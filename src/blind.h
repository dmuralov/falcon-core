// Copyright (c) 2017-2021 The Falcon Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#ifndef FALCON_BLIND_H
#define FALCON_BLIND_H

#include <secp256k1.h>
#include <secp256k1_bulletproofs.h>
#include <stdint.h>
#include <vector>

#include <amount.h>

class uint256;

extern secp256k1_context *secp256k1_ctx_blind;
extern secp256k1_scratch_space *blind_scratch;
extern secp256k1_bulletproof_generators *blind_gens;

int SelectRangeProofParameters(uint64_t nValueIn, uint64_t &minValue, int &exponent, int &nBits);

int GetRangeProofInfo(const std::vector<uint8_t> &vRangeproof, int &rexp, int &rmantissa, CAmount &min_value, CAmount &max_value);

void LoadRCTBlacklist(const int64_t indices[], size_t num_indices);
void LoadRCTWhitelist(const int64_t indices[], size_t num_indices);
void LoadCTWhitelist(const unsigned char *data, size_t data_length);
void LoadCTTaintedFilter(const unsigned char *data, size_t data_length);
void LoadBlindedOutputFilters();
bool IsFrozenBlindOutput(const uint256 &txid);  // tainted && !whitelisted
bool IsBlacklistedAnonOutput(int64_t anon_index);
bool IsWhitelistedAnonOutput(int64_t anon_index);

void ECC_Start_Blinding();
void ECC_Stop_Blinding();

#endif  // FALCON_BLIND_H
