// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2020 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <deploymentinfo.h>
#include <hash.h> // for signet block challenge hash
#include <util/system.h>

#include <chain/chainparamsimport.h>

#include <assert.h>
#include <iostream>
//#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

// Falcon
#include <key/keyutil.h>

int64_t CChainParams::GetCoinYearReward(int64_t nTime) const
{
    static const int64_t nSecondsInYear = 365 * 24 * 60 * 60;

    if (strNetworkID != "regtest") {
        // After HF2: 8%, 8%, 7%, 7%, 6%
        if (nTime >= consensus.exploit_fix_2_time) {
            int64_t nPeriodsSinceHF2 = (nTime - consensus.exploit_fix_2_time) / (nSecondsInYear * 2);
            if (nPeriodsSinceHF2 >= 0 && nPeriodsSinceHF2 < 2) {
                return (8 - nPeriodsSinceHF2) * CENT;
            }
            return 6 * CENT;
        }

        // Y1 5%, Y2 4%, Y3 3%, Y4 2%, ... YN 2%
        int64_t nYearsSinceGenesis = (nTime - genesis.nTime) / nSecondsInYear;
        if (nYearsSinceGenesis >= 0 && nYearsSinceGenesis < 3) {
            return (5 - nYearsSinceGenesis) * CENT;
        }
    }

    return nCoinYearReward;
};

bool CChainParams::PushTreasuryFundSettings(int64_t time_from, TreasuryFundSettings &settings)
{
    if (settings.nMinTreasuryStakePercent < 0 or settings.nMinTreasuryStakePercent > 100) {
        throw std::runtime_error("minstakepercent must be in range [0, 100].");
    }

    vTreasuryFundSettings.emplace_back(time_from, settings);

    return true;
};

int64_t CChainParams::GetProofOfStakeReward(const CBlockIndex *pindexPrev, int64_t nFees) const
{
    int64_t nSubsidy;

    nSubsidy = (pindexPrev->nMoneySupply / COIN) * GetCoinYearReward(pindexPrev->nTime) / (365 * 24 * (60 * 60 / nTargetSpacing));

    return nSubsidy + nFees;
};

int64_t CChainParams::GetMaxSmsgFeeRateDelta(int64_t smsg_fee_prev) const
{
    return (smsg_fee_prev * consensus.smsg_fee_max_delta_percent) / 1000000;
};

bool CChainParams::CheckImportCoinbase(int nHeight, uint256 &hash) const
{
    for (auto &cth : Params().vImportedCoinbaseTxns) {
        if (cth.nHeight != (uint32_t)nHeight) {
            continue;
        }
        if (hash == cth.hash) {
            return true;
        }
        return error("%s - Hash mismatch at height %d: %s, expect %s.", __func__, nHeight, hash.ToString(), cth.hash.ToString());
    }

    return error("%s - Unknown height.", __func__);
};


const TreasuryFundSettings *CChainParams::GetTreasuryFundSettings(int64_t nTime) const
{
    for (auto i = vTreasuryFundSettings.crbegin(); i != vTreasuryFundSettings.crend(); ++i) {
        if (nTime > i->first) {
            return &i->second;
        }
    }

    return nullptr;
};

bool CChainParams::IsBech32Prefix(const std::vector<unsigned char> &vchPrefixIn) const
{
    for (auto &hrp : bech32Prefixes)  {
        if (vchPrefixIn == hrp) {
            return true;
        }
    }

    return false;
};

bool CChainParams::IsBech32Prefix(const std::vector<unsigned char> &vchPrefixIn, CChainParams::Base58Type &rtype) const
{
    for (size_t k = 0; k < MAX_BASE58_TYPES; ++k) {
        auto &hrp = bech32Prefixes[k];
        if (vchPrefixIn == hrp) {
            rtype = static_cast<CChainParams::Base58Type>(k);
            return true;
        }
    }

    return false;
};

bool CChainParams::IsBech32Prefix(const char *ps, size_t slen, CChainParams::Base58Type &rtype) const
{
    for (size_t k = 0; k < MAX_BASE58_TYPES; ++k) {
        const auto &hrp = bech32Prefixes[k];
        size_t hrplen = hrp.size();
        if (hrplen > 0
            && slen > hrplen
            && strncmp(ps, (const char*)&hrp[0], hrplen) == 0) {
            rtype = static_cast<CChainParams::Base58Type>(k);
            return true;
        }
    }

    return false;
};

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

const std::pair<const char*, CAmount> regTestOutputs[] = {
    std::make_pair("585c2b3914d9ee51f8e710304e386531c3abcc82", 10000 * COIN),
    std::make_pair("c33f3603ce7c46b423536f0434155dad8ee2aa1f", 10000 * COIN),
    std::make_pair("72d83540ed1dcf28bfaca3fa2ed77100c2808825", 10000 * COIN),
    std::make_pair("69e4cc4c219d8971a253cd5db69a0c99c4a5659d", 10000 * COIN),
    std::make_pair("eab5ed88d97e50c87615a015771e220ab0a0991a", 10000 * COIN),
    std::make_pair("119668a93761a34a4ba1c065794b26733975904f", 10000 * COIN),
    std::make_pair("6da49762a4402d199d41d5778fcb69de19abbe9f", 10000 * COIN),
    std::make_pair("27974d10ff5ba65052be7461d89ef2185acbe411", 10000 * COIN),
    std::make_pair("89ea3129b8dbf1238b20a50211d50d462a988f61", 10000 * COIN),
    std::make_pair("3baab5b42a409b7c6848a95dfd06ff792511d561", 10000 * COIN),

    std::make_pair("649b801848cc0c32993fb39927654969a5af27b0", 5000 * COIN),
    std::make_pair("d669de30fa30c3e64a0303cb13df12391a2f7256", 5000 * COIN),
    std::make_pair("f0c0e3ebe4a1334ed6a5e9c1e069ef425c529934", 5000 * COIN),
    std::make_pair("27189afe71ca423856de5f17538a069f22385422", 5000 * COIN),
    std::make_pair("0e7f6fe0c4a5a6a9bfd18f7effdd5898b1f40b80", 5000 * COIN),
};
const size_t nGenesisOutputsRegtest = sizeof(regTestOutputs) / sizeof(regTestOutputs[0]);

const std::pair<const char*, CAmount> genesisOutputs[] = {
    std::make_pair("62a62c80e0b41f2857ba83eb438d5caa46e36bcb",7017084118),
    std::make_pair("c515c636ae215ebba2a98af433a3fa6c74f84415",221897417980),
    std::make_pair("711b5e1fd0b0f4cdf92cb53b00061ef742dda4fb",120499999),
    std::make_pair("20c17c53337d80408e0b488b5af7781320a0a311",18074999),
    std::make_pair("aba8c6f8dbcf4ecfb598e3c08e12321d884bfe0b",92637054909),
    std::make_pair("1f3277a84a18f822171d720f0132f698bcc370ca",3100771006662),
    std::make_pair("8fff14bea695ffa6c8754a3e7d518f8c53c3979a",465115650998),
    std::make_pair("e54967b4067d91a777587c9f54ee36dd9f1947c4",669097504996),
    std::make_pair("7744d2ac08f2e1d108b215935215a4e66d0262d2",802917005996),
    std::make_pair("a55a17e86246ea21cb883c12c709476a09b4885c",267639001997),
    std::make_pair("4e00dce8ab44fd4cafa34839edf8f68ba7839881",267639001997),
    std::make_pair("702cae5d2537bfdd5673ac986f910d6adb23510a",254257051898),
    std::make_pair("b19e494b0033c5608a7d153e57d7fdf3dfb51bb7",1204260290404),
    std::make_pair("6909b0f1c94ea1979ed76e10a5a49ec795a8f498",1204270995964),
    std::make_pair("05a06af3b29dade9f304244d934381ac495646c1",236896901156),
    std::make_pair("557e2b3205719931e22853b27920d2ebd6147531",155127107700),
    std::make_pair("ad16fb301bd21c60c5cb580b322aa2c61b6c5df2",115374999),
    std::make_pair("182c5cfb9d17aa8d8ff78940135ca8d822022f32",17306249),
    std::make_pair("b8a374a75f6d44a0bd1bf052da014efe564ae412",133819500998),
    std::make_pair("fadee7e2878172dad55068c8696621b1788dccb3",133713917412),
    std::make_pair("eacc4b108c28ed73b111ff149909aacffd2cdf78",173382671567),
    std::make_pair("dd87cc0b8e0fc119061f33f161104ce691d23657",245040727620),
    std::make_pair("1c8b0435eda1d489e9f0a16d3b9d65182f885377",200226012806),
    std::make_pair("15a724f2bc643041cb35c9475cd67b897d62ca52",436119839355),
    std::make_pair("626f86e9033026be7afbb2b9dbe4972ef4b3e085",156118097804),
    std::make_pair("a4a73d99269639541cb7e845a4c6ef3e3911fcd6",108968353176),
    std::make_pair("27929b31f11471aa4b77ca74bb66409ff76d24a2",126271503135),
    std::make_pair("2d6248888c7f72cc88e4883e4afd1025c43a7f0e",35102718156),
    std::make_pair("25d8debc253f5c3f70010f41c53348ed156e7baa",80306152234),
};
const size_t nGenesisOutputs = sizeof(genesisOutputs) / sizeof(genesisOutputs[0]);

const std::pair<const char*, CAmount> genesisOutputsTestnet[] = {
    std::make_pair("46a064688dc7beb5f70ef83569a0f15c7abf4f28",7017084118),
    std::make_pair("9c97b561ac186bd3758bf690036296d36b1fd019",221897417980),
    std::make_pair("118a92e28242a73244fb03c96b7e1429c06f979f",120499999),
    std::make_pair("cae4bf990ce39624e2f77c140c543d4b15428ce7",18074999),
    std::make_pair("9d6b7b5874afc100eb82a4883441a73b99d9c306",92637054909),
    std::make_pair("f989e2deedb1f09ed10310fc0d7da7ebfb573326",3100771006662),
    std::make_pair("4688d6701fb4ae2893d3ec806e6af966faf67545",465115650998),
    std::make_pair("40e07b038941fb2616a54a498f763abae6d4f280",669097504996),
    std::make_pair("c43f7c57448805a068a440cc51f67379ca946264",802917005996),
    std::make_pair("98b7269dbf0c2e3344fb41cd60e75db16d6743a6",267639001997),
    std::make_pair("85dceec8cdbb9e24fe07af783e4d273d1ae39f75",267639001997),
    std::make_pair("ddc05d332b7d1a18a55509f34c786ccb65bbffbc",245040727620),
    std::make_pair("8b04d0b2b582c986975414a01cb6295f1c33d0e9",1204260290404),
    std::make_pair("1e9ff4c3ac6d0372963e92a13f1e47409eb62d37",1204270995964),
    std::make_pair("687e7cf063cd106c6098f002fa1ea91d8aee302a",236896901156),
    std::make_pair("dc0be0edcadd4cc97872db40bb8c2db2cebafd1c",155127107700),
    std::make_pair("21efcbfe37045648180ac68b406794bde77f9983",115374999),
    std::make_pair("deaf53dbfbc799eed1171269e84c733dec22f517",17306249),
    std::make_pair("200a0f9dba25e00ea84a4a3a43a7ea6983719d71",133819500998),
    std::make_pair("2d072fb1a9d1f7dd8df0443e37e9f942eab58680",133713917412),
    std::make_pair("0850f3b7caf3b822bb41b9619f8edf9b277402d0",173382671567),
    std::make_pair("ec62fbd782bf6f48e52eea75a3c68a4c3ab824c0",254257051898),
    std::make_pair("c6dcb0065e98f5edda771c594265d61e38cf63a0",200226012806),
    std::make_pair("e5f9a711ccd7cb0d2a70f9710229d0d0d7ef3bda",436119839355),
    std::make_pair("cae1527d24a91470aeb796f9d024630f301752ef",156118097804),
    std::make_pair("604f36860d79a9d72b827c99409118bfe16711bd",108968353176),
    std::make_pair("f02e5891cef35c9c5d9a770756b240aba5ba3639",126271503135),
    std::make_pair("8251b4983be1027a17dc3b977502086f08ba8910",35102718156),
    std::make_pair("b991d98acde28455ecb0193fefab06841187c4e7",80306152234),
};
const size_t nGenesisOutputsTestnet = sizeof(genesisOutputsTestnet) / sizeof(genesisOutputsTestnet[0]);


static CBlock CreateGenesisBlockRegTest(uint32_t nTime, uint32_t nNonce, uint32_t nBits)
{
    const char *pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";

    CMutableTransaction txNew;
    txNew.nVersion = FALCON_TXN_VERSION;
    txNew.SetType(TXN_COINBASE);
    txNew.vin.resize(1);
    uint32_t nHeight = 0;  // bip34
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp)) << OP_RETURN << nHeight;

    txNew.vpout.resize(nGenesisOutputsRegtest);
    for (size_t k = 0; k < nGenesisOutputsRegtest; ++k) {
        OUTPUT_PTR<CTxOutStandard> out = MAKE_OUTPUT<CTxOutStandard>();
        out->nValue = regTestOutputs[k].second;
        out->scriptPubKey = CScript() << OP_DUP << OP_HASH160 << ParseHex(regTestOutputs[k].first) << OP_EQUALVERIFY << OP_CHECKSIG;
        txNew.vpout[k] = out;
    }

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = FALCON_BLOCK_VERSION;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));

    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    genesis.hashWitnessMerkleRoot = BlockWitnessMerkleRoot(genesis);

    return genesis;
}

static CBlock CreateGenesisBlockTestNet(uint32_t nTime, uint32_t nNonce, uint32_t nBits)
{
    const char *pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";

    CMutableTransaction txNew;
    txNew.nVersion = FALCON_TXN_VERSION;
    txNew.SetType(TXN_COINBASE);
    txNew.vin.resize(1);
    uint32_t nHeight = 0;  // bip34
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp)) << OP_RETURN << nHeight;

    txNew.vpout.resize(nGenesisOutputsTestnet);
    for (size_t k = 0; k < nGenesisOutputsTestnet; ++k) {
        OUTPUT_PTR<CTxOutStandard> out = MAKE_OUTPUT<CTxOutStandard>();
        out->nValue = genesisOutputsTestnet[k].second;
        out->scriptPubKey = CScript() << OP_DUP << OP_HASH160 << ParseHex(genesisOutputsTestnet[k].first) << OP_EQUALVERIFY << OP_CHECKSIG;
        txNew.vpout[k] = out;
    }

    // Foundation Fund Raiser Funds
    // rVDQRVBKnQEfNmykMSY9DHgqv8s7XZSf5R fc118af69f63d426f61c6a4bf38b56bcdaf8d069
    OUTPUT_PTR<CTxOutStandard> out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 397364 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("fc118af69f63d426f61c6a4bf38b56bcdaf8d069") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // rVDQRVBKnQEfNmykMSY9DHgqv8s7XZSf5R fc118af69f63d426f61c6a4bf38b56bcdaf8d069
    out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 296138 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("89ca93e03119d53fd9ad1e65ce22b6f8791f8a49") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // Community Initative
    // rAybJ7dx4t6heHy99WqGcXkoT4Bh3V9qZ8 340288104577fcc3a6a84b98f7eac1a54e5287ee
    out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 156675 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("89ca93e03119d53fd9ad1e65ce22b6f8791f8a49") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // Contributors Left Over Funds
    // rAvmLShYFZ78aAHhFfUFsrHMoBuPPyckm5 3379aa2a4379ae6c51c7777d72e8e0ffff71881b
    out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 216346 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("89ca93e03119d53fd9ad1e65ce22b6f8791f8a49") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // Reserved Falcon for primary round
    // rLWLm1Hp7im3mq44Y1DgyirYgwvrmRASib 9c8c6c8c698f074180ecfdb38e8265c11f2a62cf
    out = MAKE_OUTPUT<CTxOutStandard>();
    //out->nValue = 996000 * COIN;
    //out->nValue = 89998803971.7 * COIN;
    out->nValue = 8999880397 * COIN;
    out->scriptPubKey = CScript() << 1512000000 << OP_CHECKLOCKTIMEVERIFY << OP_DROP << OP_HASH160<< ParseHex("9c8c6c8c698f074180ecfdb38e8265c11f2a62cf") << OP_EQUAL; // 2017-11-30
    txNew.vpout.push_back(out);


    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = FALCON_BLOCK_VERSION;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));

    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    genesis.hashWitnessMerkleRoot = BlockWitnessMerkleRoot(genesis);

    return genesis;
}

static CBlock CreateGenesisBlockMainNet(uint32_t nTime, uint32_t nNonce, uint32_t nBits)
{
    const char *pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";

    CMutableTransaction txNew;
    txNew.nVersion = FALCON_TXN_VERSION;
    txNew.SetType(TXN_COINBASE);

    txNew.vin.resize(1);
    uint32_t nHeight = 0;  // bip34
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp)) << OP_RETURN << nHeight;

    txNew.vpout.resize(nGenesisOutputs);
    for (size_t k = 0; k < nGenesisOutputs; ++k) {
        OUTPUT_PTR<CTxOutStandard> out = MAKE_OUTPUT<CTxOutStandard>();
        out->nValue = genesisOutputs[k].second;
        out->scriptPubKey = CScript() << OP_DUP << OP_HASH160 << ParseHex(genesisOutputs[k].first) << OP_EQUALVERIFY << OP_CHECKSIG;
        txNew.vpout[k] = out;
    }

    // Foundation Fund Raiser Funds
    // RHFKJkrB4H38APUDVckr7TDwrK11N7V7mx
    OUTPUT_PTR<CTxOutStandard> out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 397364 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("5766354dcb13caff682ed9451b9fe5bbb786996c") << OP_EQUAL;
    txNew.vpout.push_back(out);

    out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 296138 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("5766354dcb13caff682ed9451b9fe5bbb786996c") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // Community Initative
    // RKKgSiQcMjbC8TABRoyyny1gTU4fAEiQz9
    out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 156675 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("6e29c4a11fd54916d024af16ca913cdf8f89cb31") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // Contributors Left Over Funds
    // RKiaVeyLUp7EmwHtCP92j8Vc1AodhpWi2U
    out = MAKE_OUTPUT<CTxOutStandard>();
    out->nValue = 216346 * COIN;
    out->scriptPubKey = CScript() << OP_HASH160 << ParseHex("727e5e75929bbf26912dd7833971d77e7450a33e") << OP_EQUAL;
    txNew.vpout.push_back(out);

    // Reserved Falcon for primary round
    // RNnoeeqBTkpPQH8d29Gf45dszVj9RtbmCu
    out = MAKE_OUTPUT<CTxOutStandard>();
    //out->nValue = 89998803971,7 * COIN;
    out->nValue = 8999880397 * COIN;
    //out->nValue = 996000 * COIN;
    out->scriptPubKey = CScript() << 1512000000 << OP_CHECKLOCKTIMEVERIFY << OP_DROP << OP_HASH160<< ParseHex("9433643b4fd5de3ebd7fdd68675f978f34585af1") << OP_EQUAL; // 2017-11-30
    txNew.vpout.push_back(out);


    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = FALCON_BLOCK_VERSION;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));

    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    genesis.hashWitnessMerkleRoot = BlockWitnessMerkleRoot(genesis);

    return genesis;
}


/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 12500000;
        consensus.BIP34Height = 0;
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;

        consensus.OpIsCoinstakeTime = 0x5A04EC00;       // 2017-11-10 00:00:00 UTC
        consensus.fAllowOpIsCoinstakeWithP2PKH = false;
        consensus.nPaidSmsgTime = 0x5C791EC0;           // 2019-03-01 12:00:00 UTC
        consensus.smsg_fee_time = 0x5D2DBC40;           // 2019-07-16 12:00:00 UTC
        consensus.bulletproof_time = 0x5D2DBC40;        // 2019-07-16 12:00:00 UTC
        consensus.rct_time = 0x5D2DBC40;                // 2019-07-16 12:00:00 UTC
        consensus.smsg_difficulty_time = 0x5D2DBC40;    // 2019-07-16 12:00:00 UTC
        consensus.exploit_fix_1_time = 1614268800;      // 2021-02-25 16:00:00 UTC
        consensus.exploit_fix_2_time = 1626109200;      // 2021-07-12 17:00:00 UTC

        consensus.m_frozen_anon_index = 27340;
        consensus.m_frozen_blinded_height = 884433;


        consensus.smsg_fee_period = 5040;
        consensus.smsg_fee_funding_tx_per_k = 200000;
        consensus.smsg_fee_msg_per_day_per_k = 50000;
        consensus.smsg_fee_max_delta_percent = 43;
        consensus.smsg_min_difficulty = 0x1effffff;
        consensus.smsg_difficulty_max_delta = 0xffff;

        consensus.powLimit = uint256S("000000000000bfffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        //consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1619222400; // April 24th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1628640000; // August 11th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 709632; // Approximately November 12th, 2021

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256(); //c88e750b04ef441449
        //0x0000000000000000000000000000000000000000000000c88e750b04ef441449
        consensus.defaultAssumeValid = uint256(); // 992790
         //0xea97054e60558199d5c94627116fbfa9f3be1c63d45510963d1a308fe152974b

        consensus.nMinRCTOutputDepth = 12;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x80;
        pchMessageStart[1] = 0x85;
        pchMessageStart[2] = 0xb6;
        pchMessageStart[3] = 0xba;
        nDefaultPort = 51838;
        nBIP44ID = (int)WithHardenedBit(44);
        assert(nBIP44ID == (int)0x8000002C);


        nModifierInterval = 10 * 60;    // 10 minutes
        nStakeMinConfirmations = 225;   // 225 * 2 minutes
        nTargetSpacing = 120;           // 2 minutes
        nTargetTimespan = 24 * 60;      // 24 mins

        AddImportHashesMain(vImportedCoinbaseTxns);
        SetLastImportHeight();

        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 2;
        m_assumed_chain_state_size = 2;

        genesis = CreateGenesisBlockMainNet(1630663200, 3061400, 0x1f00ffff); 
        consensus.hashGenesisBlock = genesis.GetHash();
        
        std::cout << "hsh -> " << consensus.hashGenesisBlock.ToString() << std::endl;
        std::cout << "hsh -> " << genesis.hashMerkleRoot.ToString() << std::endl;
        std::cout << "hsh -> " << genesis.hashWitnessMerkleRoot.ToString() << std::endl;
        
        
        assert(consensus.hashGenesisBlock == uint256S("0x8869fb2e36b77b7bab2f08ca8d0a520b2a7ce971b9d8c60e228df7ea7e56209d"));
        //0x0000ee0784c195317ac95623e22fddb8c7b8825dc3998e0bb924d66866eccf4c
        assert(genesis.hashMerkleRoot == uint256S("0x195420e3824cebd37ac4a89173bd0d51c3f875549777beff05ec25bf687e973e"));
        //0xc95fb023cf4bc02ddfed1a59e2b2f53edd1a726683209e2780332edf554f1e3e
        assert(genesis.hashWitnessMerkleRoot == uint256S("0x46850d793efa0fbf433ae7770010b3c1f541f05c114f63bbb469c36c1d6b8e01"));
        //0x619e94a7f9f04c8a1d018eb8bcd9c42d3c23171ebed8f351872256e36959d66c


        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as an addrfetch if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        //vSeeds.emplace_back("mainnet-seed.falcon.io");
        //vSeeds.emplace_back("dnsseed-mainnet.falcon.io");
        //vSeeds.emplace_back("mainnet.falcon.io");
        //vSeeds.emplace_back("dnsseed.tecnovert.net");


        vTreasuryFundSettings.emplace_back(0,
            TreasuryFundSettings("RJAPhgckEgRGVPZa9WoGSWW24spskSfLTQ", 10, 60));
        vTreasuryFundSettings.emplace_back(consensus.OpIsCoinstakeTime,
            TreasuryFundSettings("RBiiQBnQsVPPQkUaJVQTjsZM9K2xMKozST", 10, 60));
        vTreasuryFundSettings.emplace_back(consensus.exploit_fix_2_time,
            TreasuryFundSettings("RQYUDd3EJohpjq62So4ftcV5XZfxZxJPe9", 50, 650));


        base58Prefixes[PUBKEY_ADDRESS]     = {0x24}; // F
        base58Prefixes[SCRIPT_ADDRESS]     = {0x30}; // L
        base58Prefixes[PUBKEY_ADDRESS_256] = {0x39};
        base58Prefixes[SCRIPT_ADDRESS_256] = {0x3d};
        base58Prefixes[SECRET_KEY]         = {0x6c};
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x69, 0x6e, 0x82, 0xd1}; // PPAR
        base58Prefixes[EXT_SECRET_KEY]     = {0x8f, 0x1d, 0xae, 0xb8}; // XPAR
        base58Prefixes[STEALTH_ADDRESS]    = {0x14};
        base58Prefixes[EXT_KEY_HASH]       = {0x4b}; // X
        base58Prefixes[EXT_ACC_HASH]       = {0x17}; // A
        base58Prefixes[EXT_PUBLIC_KEY_BTC] = {0x04, 0x88, 0xB2, 0x1E}; // xpub
        base58Prefixes[EXT_SECRET_KEY_BTC] = {0x04, 0x88, 0xAD, 0xE4}; // xprv

        bech32Prefixes[PUBKEY_ADDRESS].assign       ("ph",(const char*)"ph"+2);
        bech32Prefixes[SCRIPT_ADDRESS].assign       ("pr",(const char*)"pr"+2);
        bech32Prefixes[PUBKEY_ADDRESS_256].assign   ("pl",(const char*)"pl"+2);
        bech32Prefixes[SCRIPT_ADDRESS_256].assign   ("pj",(const char*)"pj"+2);
        bech32Prefixes[SECRET_KEY].assign           ("px",(const char*)"px"+2);
        bech32Prefixes[EXT_PUBLIC_KEY].assign       ("pep",(const char*)"pep"+3);
        bech32Prefixes[EXT_SECRET_KEY].assign       ("pex",(const char*)"pex"+3);
        bech32Prefixes[STEALTH_ADDRESS].assign      ("ps",(const char*)"ps"+2);
        bech32Prefixes[EXT_KEY_HASH].assign         ("pek",(const char*)"pek"+3);
        bech32Prefixes[EXT_ACC_HASH].assign         ("pea",(const char*)"pea"+3);
        bech32Prefixes[STAKE_ONLY_PKADDR].assign    ("pcs",(const char*)"pcs"+3);

        bech32_hrp = "fw";
        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                { 5000,     uint256S("0xe786020ab94bc5461a07d744f3631a811b4ebf424fceda12274f2321883713f4")},
                { 15000,    uint256S("0xafc73ac299f2e6dd309077d230fccef547b9fc24379c1bf324dd3683b13c61c3")},
                { 30000,    uint256S("0x35d95c12799323d7b418fd64df9d88ef67ef27f057d54033b5b2f38a5ecaacbf")},
                { 91000,    uint256S("0x4d1ffaa5b51431918a0c74345e2672035c743511359ac8b1be67467b02ff884c")},
                { 112250,   uint256S("0x89e4b23471aea7a875df835d6f89613fd87ba649e7a1d8cb892917d0080ef337")},
                { 213800,   uint256S("0xfd6c0e5f7444a9e09a5fa1652db73d5b8628aeabe162529a5356be700509aa80")},
                { 303640,   uint256S("0x7cc035d7888ee6d824cec8ff01a6287a71873d874f72a5fd3706d227b88f8e99")},
                { 443228,   uint256S("0x1e2ae3edb2fa5b398c2f719d2bbb44b3089fb96170b6676c0c963f12bceba489")},
                { 583322,   uint256S("0x2be0224e40ddf4763f61ff6db088806f3ad5c6530ea7a6801b067ecbbd13fec9")},
                { 634566,   uint256S("0xc330a61e218b06d3d567c459b54e83ab682a366fc00b77d69dd78c6ed9655a2e")},
                { 777625,   uint256S("0x75b2b1412610c1ff54e49fc38222f3f45fe934b0e485ccae7b5d461b94510734")},
                { 856749,   uint256S("0x6b705dbf87345594314152841212a532753f11ec711ac81afc64f31eb048df19")},
                { 887180,   uint256S("0xf9f1e91f82e73d4781052e42c8b814b8265e0929d4c16284db3feb354bfc317c")},
                { 962370,   uint256S("0x43c3d5568f3b3467e5142f86445d5b12b923e3e5c4a1e6566d90a7fad807799c")},
                { 992790,   uint256S("0xea97054e60558199d5c94627116fbfa9f3be1c63d45510963d1a308fe152974b")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
         // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData {
            // Data from rpc: getchaintxstats 4096 ea97054e60558199d5c94627116fbfa9f3be1c63d45510963d1a308fe152974b
            /* nTime    */ 0,   // 1628237328,
            /* nTxCount */ 0,   //1176142,
            /* dTxRate  */ 0    //0.01
        };
    }

    void SetOld()
    {
        //consensus.BIP16Exception = uint256S("0x00000000000002dc756eebf4f49723ed8d30cc28a5f108eb94b1ba88ac4f9c22");
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 0;  //was 227931
        //consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0; //    was 388381        000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 0; //    was 363725        00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.CSVHeight = 419328; // 000000000000000004a1b34462cb8aeebd5799177f7a29cf28f2d1961716b5b5
        consensus.SegwitHeight = 481824; // 0000000000000000001c8018d9cb3b742ef25114f27563e3fc4a1902167f9893
        consensus.MinBIP9WarningHeight = consensus.SegwitHeight + consensus.nMinerConfirmationWindow;
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        genesis = CreateGenesisBlock(1231006505, 2083236893, 0x1d00ffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "bc";
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 12500000;
        consensus.BIP34Height = 0;
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;

        consensus.OpIsCoinstakeTime = 0;
        consensus.fAllowOpIsCoinstakeWithP2PKH = true; // TODO: clear for next testnet
        consensus.nPaidSmsgTime = 0;
        consensus.smsg_fee_time = 0x5C67FB40;           // 2019-02-16 12:00:00
        consensus.bulletproof_time = 0x5C67FB40;        // 2019-02-16 12:00:00
        consensus.rct_time = 0;
        consensus.smsg_difficulty_time = 0x5D19F5C0;    // 2019-07-01 12:00:00
        consensus.exploit_fix_1_time = 1614268800;      // 2021-02-25 16:00:00

        consensus.smsg_fee_period = 5040;
        consensus.smsg_fee_funding_tx_per_k = 200000;
        consensus.smsg_fee_msg_per_day_per_k = 50000;
        consensus.smsg_fee_max_delta_percent = 43;
        consensus.smsg_min_difficulty = 0x1effffff;
        consensus.smsg_difficulty_max_delta = 0xffff;

        consensus.powLimit = uint256S("000000000005ffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        //consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1619222400; // April 24th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1628640000; // August 11th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256(); //12c75dd4368d68ff58
        //0x000000000000000000000000000000000000000000000012c75dd4368d68ff58
        consensus.defaultAssumeValid = uint256(); // 940090
       
        //0xa7670a4ec4a80183a41c37c0bb377deb25e64d0d9f0e1b9cd69f832c315f2f31

        consensus.nMinRCTOutputDepth = 12;

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x9a;
        pchMessageStart[2] = 0x9c;
        pchMessageStart[3] = 0xae;
        nDefaultPort = 52038;
        nBIP44ID = (int)WithHardenedBit(1);

        nModifierInterval = 10 * 60;    // 10 minutes
        nStakeMinConfirmations = 225;   // 225 * 2 minutes
        nTargetSpacing = 120;           // 2 minutes
        nTargetTimespan = 24 * 60;      // 24 mins


        AddImportHashesTest(vImportedCoinbaseTxns);
        SetLastImportHeight();

        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlockTestNet(1630663200, 3061400, 0x1f00ffff);
        consensus.hashGenesisBlock = genesis.GetHash();
        
        std::cout << "hsh -> " << consensus.hashGenesisBlock.ToString() << std::endl;
        std::cout << "hsh -> " << genesis.hashMerkleRoot.ToString() << std::endl;
        std::cout << "hsh -> " << genesis.hashWitnessMerkleRoot.ToString() << std::endl;

        assert(consensus.hashGenesisBlock == uint256S("0x08e1289ed35e5c75c92388f800aa1ca56074dd24e21e837f747e3e55313811bd"));
        //0x0000594ada5310b367443ee0afd4fa3d0bbd5850ea4e33cdc7d6a904a7ec7c90
        assert(genesis.hashMerkleRoot == uint256S("0x4d0da1b41bd614359ce1c96b894fdce7858887b9a8f51fa89b8e2b7341c48ece"));
        //2c7f4d88345994e3849502061f6303d9666172e4dff3641d3472a72908eec002
        assert(genesis.hashWitnessMerkleRoot == uint256S("0138ba971393fa3d68f85493e57443357b2e8b8c02c44829a8945d8ac9609765"));
        //f9e2235c9531d5a19263ece36e82c4d5b71910d73cd0b677b81c5e50d17b6cda

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        //vSeeds.emplace_back("testnet-seed.falcon.io");
        //vSeeds.emplace_back("dnsseed-testnet.falcon.io");
        //vSeeds.emplace_back("dnsseed-testnet.tecnovert.net");

        vTreasuryFundSettings.push_back(std::make_pair(0, TreasuryFundSettings("rTvv9vsbu269mjYYEecPYinDG8Bt7D86qD", 10, 60)));

        base58Prefixes[PUBKEY_ADDRESS]     = {0x5f}; // f
        base58Prefixes[SCRIPT_ADDRESS]     = {0x7f}; // t
        base58Prefixes[PUBKEY_ADDRESS_256] = {0x77};
        base58Prefixes[SCRIPT_ADDRESS_256] = {0x7b};
        base58Prefixes[SECRET_KEY]         = {0x2e};
        base58Prefixes[EXT_PUBLIC_KEY]     = {0xe1, 0x42, 0x78, 0x00}; // ppar
        base58Prefixes[EXT_SECRET_KEY]     = {0x04, 0x88, 0x94, 0x78}; // xpar
        base58Prefixes[STEALTH_ADDRESS]    = {0x15}; // T
        base58Prefixes[EXT_KEY_HASH]       = {0x89}; // x
        base58Prefixes[EXT_ACC_HASH]       = {0x53}; // a
        base58Prefixes[EXT_PUBLIC_KEY_BTC] = {0x04, 0x35, 0x87, 0xCF}; // tpub
        base58Prefixes[EXT_SECRET_KEY_BTC] = {0x04, 0x35, 0x83, 0x94}; // tprv

        bech32Prefixes[PUBKEY_ADDRESS].assign       ("tph",(const char*)"tph"+3);
        bech32Prefixes[SCRIPT_ADDRESS].assign       ("tpr",(const char*)"tpr"+3);
        bech32Prefixes[PUBKEY_ADDRESS_256].assign   ("tpl",(const char*)"tpl"+3);
        bech32Prefixes[SCRIPT_ADDRESS_256].assign   ("tpj",(const char*)"tpj"+3);
        bech32Prefixes[SECRET_KEY].assign           ("tpx",(const char*)"tpx"+3);
        bech32Prefixes[EXT_PUBLIC_KEY].assign       ("tpep",(const char*)"tpep"+4);
        bech32Prefixes[EXT_SECRET_KEY].assign       ("tpex",(const char*)"tpex"+4);
        bech32Prefixes[STEALTH_ADDRESS].assign      ("tps",(const char*)"tps"+3);
        bech32Prefixes[EXT_KEY_HASH].assign         ("tpek",(const char*)"tpek"+4);
        bech32Prefixes[EXT_ACC_HASH].assign         ("tpea",(const char*)"tpea"+4);
        bech32Prefixes[STAKE_ONLY_PKADDR].assign    ("tpcs",(const char*)"tpcs"+4);

        bech32_hrp = "tfw";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {127620, uint256S("0xe5ab909fc029b253bad300ccf859eb509e03897e7853e8bfdde2710dbf248dd1")},
                {210920, uint256S("0x5534f546c3b5a264ca034703b9694fabf36d749d66e0659eef5f0734479b9802")},
                {312860, uint256S("0xaba2e3b2dcf1970b53b67c869325c5eefd3a107e62518fa4640ddcfadf88760d")},
                {428386, uint256S("0x08bbc92c831b864c809b575901e37aaa9aa2b2e38212594aedf2712a87267da9")},
                {534422, uint256S("0xbf0ae4652ff8d2b2479cf828e2e4ec408cf29223c2ec8a96485b1bf424e096c6")},
                {728858, uint256S("0xd71157e5a929a2aba06b23566932ffaba05d1a063b2ab71d2807b8e2efcf765c")},
                {808059, uint256S("0x89de981a2cca262ae52ff5e69a0915c9083fb7cd4aba44e39f83c12a6b6602a9")},
                {909640, uint256S("0xe2e1880d525c93e24ca2d0d494fe78624ad28c4ce778f987504582b7404bcb71")},
                {940090, uint256S("0xa7670a4ec4a80183a41c37c0bb377deb25e64d0d9f0e1b9cd69f832c315f2f31")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096 a7670a4ec4a80183a41c37c0bb377deb25e64d0d9f0e1b9cd69f832c315f2f31
            /* nTime    */ 0,   //1628236944,
            /* nTxCount */ 0,  //1000837,
            /* dTxRate  */ 0 //0.007
        };
    }
};

/**
 * Signet: test network with an additional consensus parameter (see BIP325).
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const ArgsManager& args) {
        std::vector<uint8_t> bin;
        vSeeds.clear();

        if (!args.IsArgSet("-signetchallenge")) {
            bin = ParseHex("512103ad5e0edad18cb1f0fc0d28a3d4f1f3e445640337489abb10404f2d1e086be430210359ef5021964fe22d6f8e05b2463c9540ce96883fe3b278760f048f5189f2e6c452ae");
            vSeeds.emplace_back("178.128.221.177");
            vSeeds.emplace_back("2a01:7c8:d005:390::5");
            vSeeds.emplace_back("v7ajjeirttkbnt32wpy3c6w3emwnfr3fkla7hpxcfokr3ysd3kqtzmqd.onion:38333");

            consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000008546553c03");
            consensus.defaultAssumeValid = uint256S("0x000000187d4440e5bff91488b700a140441e089a8aaea707414982460edbfe54"); // 47200
            m_assumed_blockchain_size = 1;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                // Data from RPC: getchaintxstats 4096 000000187d4440e5bff91488b700a140441e089a8aaea707414982460edbfe54
                /* nTime    */ 1626696658,
                /* nTxCount */ 387761,
                /* dTxRate  */ 0.04035946932424404,
            };
        } else {
            const auto signet_challenge = args.GetArgs("-signetchallenge");
            if (signet_challenge.size() != 1) {
                throw std::runtime_error(strprintf("%s: -signetchallenge cannot be multiple values.", __func__));
            }
            bin = ParseHex(signet_challenge[0]);

            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                0,
                0,
                0,
            };
            LogPrintf("Signet with challenge %s\n", signet_challenge[0]);
        }

        if (args.IsArgSet("-signetseednode")) {
            vSeeds = args.GetArgs("-signetseednode");
        }

        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;
        consensus.signet_challenge.assign(bin.begin(), bin.end());
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP16Exception = uint256{};
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000377ae000000000000000000000000000000000000000000000000000000");
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Activation of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        // message start is defined as the first 4 bytes of the sha256d of the block script
        CHashWriter h(SER_DISK, 0);
        h << consensus.signet_challenge;
        uint256 hash = h.GetHash();
        memcpy(pchMessageStart, hash.begin(), 4);

        nDefaultPort = 38433;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1598918400, 52613770, 0x1e0377ae, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000008819873e925422c1ff0f99f7cc9bbb232af63a077a480a3633bee1ef6"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tb";

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = false;
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams {
public:
    explicit CRegTestParams(const ArgsManager& args) {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Exception = uint256();
        consensus.BIP34Height = 2; // BIP34 activated on regtest (Block at height 1 not enforced for testing purposes)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 111; // BIP65 activated on regtest (Block at height 110 and earlier not enforced for testing purposes)
        consensus.BIP66Height = 102; // BIP66 activated on regtest (Block at height 101 and earlier not enforced for testing purposes)
        consensus.CSVHeight = 432; // CSV activated on regtest (Used in rpc activation tests)
        consensus.SegwitHeight = 0; // SEGWIT is always activated on regtest unless overridden
        consensus.MinBIP9WarningHeight = 0;

        consensus.OpIsCoinstakeTime = 0;
        consensus.fAllowOpIsCoinstakeWithP2PKH = false;
        consensus.nPaidSmsgTime = 0;
        consensus.smsg_fee_time = 0;
        consensus.bulletproof_time = 0;
        consensus.rct_time = 0;
        consensus.smsg_difficulty_time = 0;

        consensus.clamp_tx_version_time = 0;

        consensus.smsg_fee_period = 50;
        consensus.smsg_fee_funding_tx_per_k = 200000;
        consensus.smsg_fee_msg_per_day_per_k = 50000;
        consensus.smsg_fee_max_delta_percent = 4300;
        consensus.smsg_min_difficulty = 0x1f0fffff;
        consensus.smsg_difficulty_max_delta = 0xffff;

        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        consensus.nMinRCTOutputDepth = 2;

        pchMessageStart[0] = 0xb0;
        pchMessageStart[1] = 0x89;
        pchMessageStart[2] = 0x83;
        pchMessageStart[3] = 0x8d;
        nDefaultPort = 12038;
        nBIP44ID = (int)WithHardenedBit(1);


        nModifierInterval = 2 * 60;     // 2 minutes
        nStakeMinConfirmations = 12;
        nTargetSpacing = 5;             // 5 seconds
        nTargetTimespan = 16 * 60;      // 16 mins
        nStakeTimestampMask = 0;

        SetLastImportHeight();

        nPruneAfterHeight = gArgs.GetBoolArg("-fastprune", false) ? 100 : 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        UpdateActivationParametersFromArgs(args);

        genesis = CreateGenesisBlockRegTest(1487714923, 0, 0x207fffff);

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x6cd174536c0ada5bfa3b8fde16b98ae508fff6586f2ee24cf866867098f25907"));
        assert(genesis.hashMerkleRoot == uint256S("0xf89653c7208af2c76a3070d436229fb782acbd065bd5810307995b9982423ce7"));
        assert(genesis.hashWitnessMerkleRoot == uint256S("0x36b66a1aff91f34ab794da710d007777ef5e612a320e1979ac96e5f292399639"));


        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();
        //vSeeds.emplace_back("dummySeed.invalid.");

        fDefaultConsistencyChecks = true;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                //{0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            {
                110,
                {AssumeutxoHash{uint256S("0xd98b6865564c85700442c2df34cf379d97ac4c157b3eb25780ca916183620376")}, 110},
            },
            {
                200,
                {AssumeutxoHash{uint256S("0x51c8d11d8b5c1de51543c579736e786aa2736206d1e11e627568029ce092cf62")}, 200},
            },
        };

        base58Prefixes[PUBKEY_ADDRESS]     = {0x5f}; // f
        base58Prefixes[SCRIPT_ADDRESS]     = {0x7f}; // t
        base58Prefixes[PUBKEY_ADDRESS_256] = {0x77};
        base58Prefixes[SCRIPT_ADDRESS_256] = {0x7b};
        base58Prefixes[SECRET_KEY]         = {0x2e};
        base58Prefixes[EXT_PUBLIC_KEY]     = {0xe1, 0x42, 0x78, 0x00}; // ppar
        base58Prefixes[EXT_SECRET_KEY]     = {0x04, 0x88, 0x94, 0x78}; // xpar
        base58Prefixes[STEALTH_ADDRESS]    = {0x15}; // T
        base58Prefixes[EXT_KEY_HASH]       = {0x89}; // x
        base58Prefixes[EXT_ACC_HASH]       = {0x53}; // a
        base58Prefixes[EXT_PUBLIC_KEY_BTC] = {0x04, 0x35, 0x87, 0xCF}; // tpub
        base58Prefixes[EXT_SECRET_KEY_BTC] = {0x04, 0x35, 0x83, 0x94}; // tprv

        bech32Prefixes[PUBKEY_ADDRESS].assign       ("tph",(const char*)"tph"+3);
        bech32Prefixes[SCRIPT_ADDRESS].assign       ("tpr",(const char*)"tpr"+3);
        bech32Prefixes[PUBKEY_ADDRESS_256].assign   ("tpl",(const char*)"tpl"+3);
        bech32Prefixes[SCRIPT_ADDRESS_256].assign   ("tpj",(const char*)"tpj"+3);
        bech32Prefixes[SECRET_KEY].assign           ("tpx",(const char*)"tpx"+3);
        bech32Prefixes[EXT_PUBLIC_KEY].assign       ("tpep",(const char*)"tpep"+4);
        bech32Prefixes[EXT_SECRET_KEY].assign       ("tpex",(const char*)"tpex"+4);
        bech32Prefixes[STEALTH_ADDRESS].assign      ("tps",(const char*)"tps"+3);
        bech32Prefixes[EXT_KEY_HASH].assign         ("tpek",(const char*)"tpek"+4);
        bech32Prefixes[EXT_ACC_HASH].assign         ("tpea",(const char*)"tpea"+4);
        bech32Prefixes[STAKE_ONLY_PKADDR].assign    ("tpcs",(const char*)"tpcs"+4);

        bech32_hrp = "lcfw";

        chainTxData = ChainTxData{
            0,
            0,
            0
        };
    }

    void SetOld()
    {
        genesis = CreateGenesisBlock(1296688602, 2, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        /*
        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        */

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "bcrt";
    }

    /**
     * Allows modifying the Version Bits regtest parameters.
     */
    void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout, int min_activation_height)
    {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
        consensus.vDeployments[d].min_activation_height = min_activation_height;
    }
    void UpdateActivationParametersFromArgs(const ArgsManager& args);
};

void CRegTestParams::UpdateActivationParametersFromArgs(const ArgsManager& args)
{
    if (args.IsArgSet("-segwitheight")) {
        int64_t height = args.GetArg("-segwitheight", consensus.SegwitHeight);
        if (height < 0 || height >= std::numeric_limits<int>::max()) {
            throw std::runtime_error(strprintf("Activation height %ld for segwit is out of valid range.", height));
        }
        consensus.SegwitHeight = static_cast<int>(height);
    }

    if (!args.IsArgSet("-vbparams")) return;

    for (const std::string& strDeployment : args.GetArgs("-vbparams")) {
        std::vector<std::string> vDeploymentParams;
        boost::split(vDeploymentParams, strDeployment, boost::is_any_of(":"));
        if (vDeploymentParams.size() < 3 || 4 < vDeploymentParams.size()) {
            throw std::runtime_error("Version bits parameters malformed, expecting deployment:start:end[:min_activation_height]");
        }
        int64_t nStartTime, nTimeout;
        int min_activation_height = 0;
        if (!ParseInt64(vDeploymentParams[1], &nStartTime)) {
            throw std::runtime_error(strprintf("Invalid nStartTime (%s)", vDeploymentParams[1]));
        }
        if (!ParseInt64(vDeploymentParams[2], &nTimeout)) {
            throw std::runtime_error(strprintf("Invalid nTimeout (%s)", vDeploymentParams[2]));
        }
        if (vDeploymentParams.size() >= 4 && !ParseInt32(vDeploymentParams[3], &min_activation_height)) {
            throw std::runtime_error(strprintf("Invalid min_activation_height (%s)", vDeploymentParams[3]));
        }
        bool found = false;
        for (int j=0; j < (int)Consensus::MAX_VERSION_BITS_DEPLOYMENTS; ++j) {
            if (vDeploymentParams[0] == VersionBitsDeploymentInfo[j].name) {
                UpdateVersionBitsParameters(Consensus::DeploymentPos(j), nStartTime, nTimeout, min_activation_height);
                found = true;
                LogPrintf("Setting version bits activation parameters for %s to start=%ld, timeout=%ld, min_activation_height=%d\n", vDeploymentParams[0], nStartTime, nTimeout, min_activation_height);
                break;
            }
        }
        if (!found) {
            throw std::runtime_error(strprintf("Invalid deployment (%s)", vDeploymentParams[0]));
        }
    }
}

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

bool HaveParams() {
    return globalChainParams.get();
}

const CChainParams *pParams() {
    return globalChainParams.get();
}

std::unique_ptr<CChainParams> CreateChainParams(const ArgsManager& args, const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN) {
        return std::unique_ptr<CChainParams>(new CMainParams());
    } else if (chain == CBaseChainParams::TESTNET) {
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    } else if (chain == CBaseChainParams::SIGNET) {
        return std::unique_ptr<CChainParams>(new SigNetParams(args));
    } else if (chain == CBaseChainParams::REGTEST) {
        return std::unique_ptr<CChainParams>(new CRegTestParams(args));
    }
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(gArgs, network);
}

void SetOldParams(std::unique_ptr<CChainParams> &params)
{
    if (params->NetworkID() == CBaseChainParams::MAIN) {
        return ((CMainParams*)params.get())->SetOld();
    }
    if (params->NetworkID() == CBaseChainParams::REGTEST) {
        return ((CRegTestParams*)params.get())->SetOld();
    }
};

void ResetParams(std::string sNetworkId, bool fFalconModeIn)
{
    // Hack to pass old unit tests
    globalChainParams = CreateChainParams(gArgs, sNetworkId);
    if (!fFalconModeIn) {
        SetOldParams(globalChainParams);
    }
};

CChainParams &RegtestParams()
{
    return *globalChainParams.get();
};
