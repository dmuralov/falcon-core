// Copyright (c) 2017-2021 The Particl Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/util/setup_common.h>
#include <net.h>
#include <script/signingprovider.h>
#include <script/script.h>
#include <consensus/validation.h>
#include <consensus/merkle.h>
#include <consensus/tx_verify.h>
#include <key/extkey.h>
#include <pos/kernel.h>
#include <chainparams.h>
#include <blind.h>

#include <script/sign.h>
#include <policy/policy.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(falconchain_tests, FalconBasicTestingSetup)


BOOST_AUTO_TEST_CASE(oldversion_test)
{
    CBlock blk, blkOut;
    blk.nTime = 1487406900;

    CMutableTransaction txn;
    blk.vtx.push_back(MakeTransactionRef(txn));

    CDataStream ss(SER_DISK, 0);

    ss << blk;
    ss >> blkOut;

    BOOST_CHECK(blk.vtx[0]->nVersion == blkOut.vtx[0]->nVersion);
}

BOOST_AUTO_TEST_CASE(signature_test)
{
    SeedInsecureRand();
    FillableSigningProvider keystore;

    CKey k;
    InsecureNewKey(k, true);
    keystore.AddKey(k);

    CPubKey pk = k.GetPubKey();
    CKeyID id = pk.GetID();

    CMutableTransaction txn;
    txn.nVersion = FALCON_TXN_VERSION;
    txn.nLockTime = 0;

    int nBlockHeight = 22;
    OUTPUT_PTR<CTxOutData> out0 = MAKE_OUTPUT<CTxOutData>();
    out0->vData = SetCompressedInt64(out0->vData, nBlockHeight);
    txn.vpout.push_back(out0);

    CScript script = CScript() << OP_DUP << OP_HASH160 << ToByteVector(id) << OP_EQUALVERIFY << OP_CHECKSIG;
    OUTPUT_PTR<CTxOutStandard> out1 = MAKE_OUTPUT<CTxOutStandard>();
    out1->nValue = 10000;
    out1->scriptPubKey = script;
    txn.vpout.push_back(out1);

    CMutableTransaction txn2;
    txn2.nVersion = FALCON_TXN_VERSION;
    txn2.vin.push_back(CTxIn(txn.GetHash(), 0));

    std::vector<uint8_t> vchAmount(8);
    part::SetAmount(vchAmount, out1->nValue);

    SignatureData sigdata;
    BOOST_CHECK(ProduceSignature(keystore, MutableTransactionSignatureCreator(&txn2, 0, vchAmount, SIGHASH_ALL), script, sigdata));

    ScriptError serror = SCRIPT_ERR_OK;
    BOOST_CHECK(VerifyScript(txn2.vin[0].scriptSig, out1->scriptPubKey, &sigdata.scriptWitness, STANDARD_SCRIPT_VERIFY_FLAGS, MutableTransactionSignatureChecker(&txn2, 0, vchAmount, MissingDataBehavior::ASSERT_FAIL), &serror));
    BOOST_CHECK(serror == SCRIPT_ERR_OK);
}

BOOST_AUTO_TEST_CASE(falconchain_test)
{
    SeedInsecureRand();
    FillableSigningProvider keystore;

    CKey k;
    InsecureNewKey(k, true);
    keystore.AddKey(k);

    CPubKey pk = k.GetPubKey();
    CKeyID id = pk.GetID();

    CScript script = CScript() << OP_DUP << OP_HASH160 << ToByteVector(id) << OP_EQUALVERIFY << OP_CHECKSIG;

    CBlock blk;
    blk.nVersion = FALCON_BLOCK_VERSION;
    blk.nTime = 1487406900;

    CMutableTransaction txn;
    txn.nVersion = FALCON_TXN_VERSION;
    txn.SetType(TXN_COINBASE);
    txn.nLockTime = 0;
    OUTPUT_PTR<CTxOutStandard> out0 = MAKE_OUTPUT<CTxOutStandard>();
    out0->nValue = 10000;
    out0->scriptPubKey = script;
    txn.vpout.push_back(out0);


    blk.vtx.push_back(MakeTransactionRef(txn));

    bool mutated;
    blk.hashMerkleRoot = BlockMerkleRoot(blk, &mutated);
    blk.hashWitnessMerkleRoot = BlockWitnessMerkleRoot(blk, &mutated);


    CDataStream ss(SER_DISK, 0);
    ss << blk;

    CBlock blkOut;
    ss >> blkOut;

    BOOST_CHECK(blk.hashMerkleRoot == blkOut.hashMerkleRoot);
    BOOST_CHECK(blk.hashWitnessMerkleRoot == blkOut.hashWitnessMerkleRoot);
    BOOST_CHECK(blk.nTime == blkOut.nTime && blkOut.nTime == 1487406900);

    BOOST_CHECK(TXN_COINBASE == blkOut.vtx[0]->GetType());
}

BOOST_AUTO_TEST_CASE(varints)
{
    SeedInsecureRand();

    int start = InsecureRandRange(100);
    size_t size = 0;
    uint8_t c[128];
    std::vector<uint8_t> v;

    // Encode
    for (int i = start; i < 10000; i+=100) {
        size_t sz = GetSizeOfVarInt<VarIntMode::NONNEGATIVE_SIGNED>(i);
        BOOST_CHECK(sz = part::PutVarInt(c, i));
        BOOST_CHECK(0 == part::PutVarInt(v, i));
        BOOST_CHECK(0 == memcmp(c, &v[size], sz));
        size += sz;
        BOOST_CHECK(size == v.size());
    }
    for (uint64_t i = 0;  i < 100000000000ULL; i += 999999937) {
        BOOST_CHECK(0 == part::PutVarInt(v, i));
        size += GetSizeOfVarInt<VarIntMode::DEFAULT>(i);
        BOOST_CHECK(size == v.size());
    }

    // Decode
    size_t nB = 0, o = 0;
    for (int i = start; i < 10000; i+=100) {
        uint64_t j = (uint64_t)-1;
        BOOST_CHECK(0 == part::GetVarInt(v, o, j, nB));
        BOOST_CHECK_MESSAGE(i == (int)j, "decoded:" << j << " expected:" << i);
        o += nB;
    }
    for (uint64_t i = 0;  i < 100000000000ULL; i += 999999937) {
        uint64_t j = (uint64_t)-1;
        BOOST_CHECK(0 == part::GetVarInt(v, o, j, nB));
        BOOST_CHECK_MESSAGE(i == j, "decoded:" << j << " expected:" << i);
        o += nB;
    }
}

BOOST_AUTO_TEST_CASE(mixed_input_types)
{
    CMutableTransaction txn;
    txn.nVersion = FALCON_TXN_VERSION;
    BOOST_CHECK(txn.IsFalconVersion());

    CAmount txfee;
    int nSpendHeight = 1;
    CCoinsView viewDummy;
    CCoinsViewCache inputs(&viewDummy);

    CMutableTransaction txnPrev;
    txnPrev.nVersion = FALCON_TXN_VERSION;
    BOOST_CHECK(txnPrev.IsFalconVersion());

    CScript scriptPubKey;
    txnPrev.vpout.push_back(MAKE_OUTPUT<CTxOutStandard>(1 * COIN, scriptPubKey));
    txnPrev.vpout.push_back(MAKE_OUTPUT<CTxOutStandard>(2 * COIN, scriptPubKey));
    txnPrev.vpout.push_back(MAKE_OUTPUT<CTxOutCT>());
    txnPrev.vpout.push_back(MAKE_OUTPUT<CTxOutCT>());

    CTransaction txnPrev_c(txnPrev);
    AddCoins(inputs, txnPrev_c, 1);

    uint256 prevHash = txnPrev_c.GetHash();

    std::vector<std::pair<std::vector<int>, bool> > tests = {
        std::make_pair( (std::vector<int>) {0 }, true),
        std::make_pair( (std::vector<int>) {0, 1}, true),
        std::make_pair( (std::vector<int>) {0, 2}, false),
        std::make_pair( (std::vector<int>) {0, 1, 2}, false),
        std::make_pair( (std::vector<int>) {2}, true),
        std::make_pair( (std::vector<int>) {2, 3}, true),
        std::make_pair( (std::vector<int>) {2, 3, 1}, false),
        std::make_pair( (std::vector<int>) {-1}, true),
        std::make_pair( (std::vector<int>) {-1, -1}, true),
        std::make_pair( (std::vector<int>) {2, -1}, false),
        std::make_pair( (std::vector<int>) {0, -1}, false),
        std::make_pair( (std::vector<int>) {0, 0, -1}, false),
        std::make_pair( (std::vector<int>) {0, 2, -1}, false)
    };

    for (auto t : tests) {
        txn.vin.clear();

        for (auto ti : t.first) {
            if (ti < 0)  {
                CTxIn ai;
                ai.prevout.n = COutPoint::ANON_MARKER;
                ai.SetAnonInfo(1, 1);

                std::vector<uint8_t> vpkm, vki(33, 0);
                part::PutVarInt(vpkm, 1);
                ai.scriptWitness.stack.emplace_back(vpkm);
                ai.scriptData.stack.emplace_back(vki);
                txn.vin.push_back(ai);
                continue;
            }
            txn.vin.push_back(CTxIn(prevHash, ti));
        }

        CTransaction tx_c(txn);
        TxValidationState state;
        bool rv = Consensus::CheckTxInputs(tx_c, state, inputs, nSpendHeight, txfee);

        if (t.second) {
            BOOST_CHECK(state.GetRejectReason() != "mixed-input-types");
        } else {
            BOOST_CHECK(!rv);
            BOOST_CHECK(state.GetRejectReason() == "mixed-input-types");
        }
    }
}

BOOST_AUTO_TEST_CASE(mixed_output_types)
{
    // When sending from plain only CT or RCT outputs are valid
    ECC_Start_Blinding();

    CAmount txfee = 2000;
    int nSpendHeight = 1;
    CCoinsView viewDummy;
    CCoinsViewCache inputs(&viewDummy);

    CMutableTransaction txnPrev;
    txnPrev.nVersion = FALCON_TXN_VERSION;
    BOOST_CHECK(txnPrev.IsFalconVersion());

    CScript scriptPubKey;
    txnPrev.vpout.push_back(MAKE_OUTPUT<CTxOutStandard>(1 * COIN, scriptPubKey));

    CTransaction txnPrev_c(txnPrev);
    AddCoins(inputs, txnPrev_c, 1);
    uint256 prevHash = txnPrev_c.GetHash();

    CMutableTransaction txn;
    txn.nVersion = FALCON_TXN_VERSION;
    BOOST_CHECK(txn.IsFalconVersion());
    txn.vin.push_back(CTxIn(prevHash, 0));

    OUTPUT_PTR<CTxOutData> out_fee = MAKE_OUTPUT<CTxOutData>();
    out_fee->vData.push_back(DO_FEE);
    BOOST_REQUIRE(0 == part::PutVarInt(out_fee->vData, txfee));
    txn.vpout.push_back(out_fee);

    txn.vpout.push_back(MAKE_OUTPUT<CTxOutStandard>(1 * COIN - txfee, scriptPubKey));
    txn.vpout.push_back(MAKE_OUTPUT<CTxOutCT>());
    txn.vpout.push_back(MAKE_OUTPUT<CTxOutRingCT>());

    CTransaction tx_c(txn);
    TxValidationState state;
    state.SetStateInfo(GetTime(), nSpendHeight, Params().GetConsensus(), true /* falcon_mode */, false /* skip_rangeproof */);
    state.m_clamp_tx_version = true; // Using mainnet chainparams
    gArgs.ForceSetArg("-acceptanontxn", "1"); // TODO: remove
    gArgs.ForceSetArg("-acceptblindtxn", "1"); // TODO: remove
    BOOST_CHECK(!Consensus::CheckTxInputs(tx_c, state, inputs, nSpendHeight, txfee));
    BOOST_CHECK(state.GetRejectReason() == "bad-txns-plain-in-mixed-out");

    txn.vpout.pop_back();
    CTransaction tx_c2(txn);
    BOOST_CHECK(!Consensus::CheckTxInputs(tx_c2, state, inputs, nSpendHeight, txfee));
    BOOST_CHECK(state.GetRejectReason() != "bad-txns-plain-in-mixed-out");

    ECC_Stop_Blinding();
}

BOOST_AUTO_TEST_CASE(op_iscoinstake_tests)
{
    CKey k1, k2;
    InsecureNewKey(k1, true);
    InsecureNewKey(k2, true);
    CPubKey pk1 = k1.GetPubKey(), pk2 = k2.GetPubKey();
    CKeyID id1 = pk1.GetID(), id2 = pk2.GetID();

    CScript scriptOutA, scriptOutB;
    CScript scriptStake = CScript() << OP_DUP << OP_HASH160 << ToByteVector(id1) << OP_EQUALVERIFY << OP_CHECKSIG;
    CScript scriptSpend = CScript() << OP_DUP << OP_HASH160 << ToByteVector(id2) << OP_EQUALVERIFY << OP_CHECKSIG;

    CScript script = CScript() << OP_ISCOINSTAKE << OP_IF;
    script.append(scriptStake);
    script << OP_ELSE;
    script.append(scriptSpend);
    script << OP_ENDIF;

    int64_t cs_time = Params().GetConsensus().OpIsCoinstakeTime;
    TxoutType whichType;
    BOOST_CHECK(true == IsStandard(script, whichType, cs_time));

    // Test trailing script
    script << OP_DROP;
    script << CScriptNum(123);
    BOOST_CHECK(false == IsStandard(script, whichType, cs_time));

    // Test compacted cs_script
    CScript script2 = CScript() << OP_DUP << OP_HASH160;
    script2 << OP_ISCOINSTAKE << OP_IF << ToByteVector(id1);
    script2 << OP_ELSE << ToByteVector(id2) << OP_ENDIF << OP_EQUALVERIFY << OP_CHECKSIG;

    BOOST_CHECK(true == SplitConditionalCoinstakeScript(script2, scriptOutA, scriptOutB));
    BOOST_CHECK(scriptOutA == scriptStake);
    BOOST_CHECK(scriptOutB == scriptSpend);

    // Test nested ifs
    CScript script3_nested_if = CScript() << OP_1 << OP_IF << OP_RETURN << OP_ELSE << OP_IF << OP_RETURN << OP_ELSE << OP_1 << OP_RETURN << OP_ENDIF << OP_ENDIF;
    CScript script3 = CScript() << OP_ISCOINSTAKE << OP_IF;
    script3.append(script3_nested_if);
    script3 << OP_ELSE;
    script3.append(scriptSpend);
    script3 << OP_ENDIF;

    BOOST_CHECK(true == SplitConditionalCoinstakeScript(script3, scriptOutA, scriptOutB));
    BOOST_CHECK(scriptOutA == script3_nested_if);
    BOOST_CHECK(scriptOutB == scriptSpend);
}

BOOST_AUTO_TEST_CASE(coin_year_reward)
{
    BOOST_CHECK(Params().GetCoinYearReward(1529700000) == 5 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1531832399) == 5 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1531832400) == 4 * CENT);    // 2018-07-17 13:00:00
    BOOST_CHECK(Params().GetCoinYearReward(1563368399) == 4 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1563368400) == 3 * CENT);    // 2019-07-17 13:00:00
    BOOST_CHECK(Params().GetCoinYearReward(1594904399) == 3 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1594904400) == 2 * CENT);    // 2020-07-16 13:00:00

    size_t seconds_in_year = 60 * 60 * 24 * 365;
    BOOST_CHECK(Params().GetCoinYearReward(1626109199) == 2 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1626109200) == 8 * CENT);                                // 2021-07-12 17:00:00 UTC
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year) == 8 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year * 2 - 1) == 8 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year * 2) == 7 * CENT);          // 2023-07-12 17:00:00 UTC
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year * 3) == 7 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year * 4 - 1) == 7 * CENT);
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year * 4) == 6 * CENT);          // 2025-07-11 17:00:00 UTC
    BOOST_CHECK(Params().GetCoinYearReward(1626109200 + seconds_in_year * 6) == 6 * CENT);
}


BOOST_AUTO_TEST_SUITE_END()
