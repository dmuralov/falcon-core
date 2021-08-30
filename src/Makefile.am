# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

# Pattern rule to print variables, e.g. make print-top_srcdir
print-%: FORCE
	@echo '$*'='$($*)'

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS) $(GPROF_LDFLAGS) $(SANITIZER_LDFLAGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS) $(HARDENED_CXXFLAGS) $(WARN_CXXFLAGS) $(NOWARN_CXXFLAGS) $(ERROR_CXXFLAGS) $(GPROF_CXXFLAGS) $(SANITIZER_CXXFLAGS)
AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(HARDENED_CPPFLAGS)
AM_CFLAGS = $(AM_CXXFLAGS)
AM_LIBTOOLFLAGS = --preserve-dup-deps
PTHREAD_FLAGS = $(PTHREAD_CFLAGS) $(PTHREAD_LIBS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

BITCOIN_INCLUDES=-I$(builddir) -I$(srcdir)/secp256k1/include $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS)

BITCOIN_INCLUDES += $(UNIVALUE_CFLAGS)

LIBFALCON_SERVER=libfalcon_server.a
LIBFALCON_COMMON=libfalcon_common.a
LIBFALCON_CONSENSUS=libfalcon_consensus.a
LIBFALCON_CLI=libfalcon_cli.a
LIBFALCON_UTIL=libfalcon_util.a
LIBFALCON_CRYPTO_BASE=crypto/libfalcon_crypto_base.a
LIBFALCONQT=qt/libfalconqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la
LIBFALCON_SMSG=libfalcon_smsg.a
LIBFALCON_MNEMONIC=libfalcon_mnemonic.a

if ENABLE_ZMQ
LIBFALCON_ZMQ=libfalcon_zmq.a
endif
if BUILD_BITCOIN_LIBS
LIBFALCONCONSENSUS=libfalconconsensus.la
endif
if ENABLE_WALLET
LIBFALCON_WALLET=libfalcon_wallet.a
LIBFALCON_WALLET_TOOL=libfalcon_wallet_tool.a
endif
if ENABLE_USBDEVICE
LIBFALCON_USBDEVICE=libfalcon_usbdevice.a
endif

LIBFALCON_CRYPTO= $(LIBFALCON_CRYPTO_BASE)
if ENABLE_SSE41
LIBFALCON_CRYPTO_SSE41 = crypto/libfalcon_crypto_sse41.a
LIBFALCON_CRYPTO += $(LIBFALCON_CRYPTO_SSE41)
endif
if ENABLE_AVX2
LIBFALCON_CRYPTO_AVX2 = crypto/libfalcon_crypto_avx2.a
LIBFALCON_CRYPTO += $(LIBFALCON_CRYPTO_AVX2)
endif
if ENABLE_SHANI
LIBFALCON_CRYPTO_SHANI = crypto/libfalcon_crypto_shani.a
LIBFALCON_CRYPTO += $(LIBFALCON_CRYPTO_SHANI)
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*.h) $(wildcard secp256k1/src/*.c) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBFALCON_CRYPTO) \
  $(LIBFALCON_UTIL) \
  $(LIBFALCON_COMMON) \
  $(LIBFALCON_CONSENSUS) \
  $(LIBFALCON_SERVER) \
  $(LIBFALCON_CLI) \
  $(LIBFALCON_IPC) \
  $(LIBFALCON_WALLET) \
  $(LIBFALCON_WALLET_TOOL) \
  $(LIBFALCON_USBDEVICE) \
  $(LIBFALCON_SMSG) \
  $(LIBFALCON_MNEMONIC) \
  $(LIBFALCON_ZMQ)

lib_LTLIBRARIES = $(LIBFALCONCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_BITCOIND
  bin_PROGRAMS += falcond
endif

if BUILD_BITCOIN_NODE
  bin_PROGRAMS += falcon-node
endif

if BUILD_BITCOIN_CLI
  bin_PROGRAMS += falcon-cli
endif

if BUILD_BITCOIN_TX
  bin_PROGRAMS += falcon-tx
endif

if ENABLE_WALLET
if BUILD_BITCOIN_WALLET
  bin_PROGRAMS += falcon-wallet
endif
endif

if BUILD_BITCOIN_UTIL
  bin_PROGRAMS += falcon-util
endif

.PHONY: FORCE check-symbols check-security
# bitcoin core #
BITCOIN_CORE_H = \
  addrdb.h \
  rctindex.h \
  addrman.h \
  attributes.h \
  banman.h \
  base58.h \
  bech32.h \
  blockencodings.h \
  blockfilter.h \
  bloom.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  chain/chainparamsimport.h \
  chain/ct_tainted.h \
  chain/tx_blacklist.h \
  chain/tx_whitelist.h \
  checkqueue.h \
  clientversion.h \
  coins.h \
  compat.h \
  compat/assumptions.h \
  compat/byteswap.h \
  compat/cpuid.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  consensus/consensus.h \
  consensus/tx_check.h \
  consensus/tx_verify.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  dbwrapper.h \
  deploymentinfo.h \
  deploymentstatus.h \
  external_signer.h \
  flatfile.h \
  fs.h \
  httprpc.h \
  httpserver.h \
  i2p.h \
  index/base.h \
  index/blockfilterindex.h \
  index/coinstatsindex.h \
  index/disktxpos.h \
  index/txindex.h \
  indirectmap.h \
  init.h \
  anon.h \
  blind.h \
  init/common.h \
  interfaces/chain.h \
  interfaces/echo.h \
  interfaces/handler.h \
  interfaces/init.h \
  interfaces/ipc.h \
  interfaces/node.h \
  interfaces/wallet.h \
  key.h \
  key/stealth.h \
  key/extkey.h \
  key/types.h \
  key/keyutil.h \
  key_io.h \
  logging.h \
  logging/timer.h \
  mapport.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  net_permissions.h \
  net_processing.h \
  net_types.h \
  netaddress.h \
  netbase.h \
  netmessagemaker.h \
  node/blockstorage.h \
  node/coin.h \
  node/coinstats.h \
  node/context.h \
  node/psbt.h \
  node/transaction.h \
  node/ui_interface.h \
  node/utxo_snapshot.h \
  noui.h \
  outputtype.h \
  policy/feerate.h \
  policy/fees.h \
  policy/packages.h \
  policy/policy.h \
  policy/rbf.h \
  policy/settings.h \
  pow.h \
  pos/kernel.h \
  pos/miner.h \
  protocol.h \
  psbt.h \
  random.h \
  randomenv.h \
  reverse_iterator.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/mining.h \
  rpc/net.h \
  rpc/protocol.h \
  rpc/rawtransaction_util.h \
  rpc/register.h \
  rpc/request.h \
  rpc/server.h \
  rpc/util.h \
  rpc/rpcutil.h \
  scheduler.h \
  script/descriptor.h \
  script/keyorigin.h \
  script/sigcache.h \
  script/sign.h \
  script/signingprovider.h \
  script/standard.h \
  script/ismine.h \
  shutdown.h \
  signet.h \
  streams.h \
  smsg/db.h \
  smsg/crypter.h \
  smsg/net.h \
  smsg/smessage.h \
  smsg/rpcsmessage.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadinterrupt.h \
  threadsafety.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  txorphanage.h \
  txrequest.h \
  undo.h \
  util/asmap.h \
  util/bip32.h \
  util/bytevectorhash.h \
  util/check.h \
  util/epochguard.h \
  util/error.h \
  util/fees.h \
  util/getuniquepath.h \
  util/golombrice.h \
  util/hash_type.h \
  util/hasher.h \
  util/macros.h \
  util/message.h \
  util/moneystr.h \
  util/rbf.h \
  util/readwritefile.h \
  util/serfloat.h \
  util/settings.h \
  util/sock.h \
  util/spanparsing.h \
  util/string.h \
  util/system.h \
  util/thread.h \
  util/threadnames.h \
  util/time.h \
  util/tokenpipe.h \
  util/trace.h \
  util/translation.h \
  util/ui_change_type.h \
  util/url.h \
  util/vector.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  wallet/bdb.h \
  wallet/coincontrol.h \
  wallet/coinselection.h \
  wallet/context.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/dump.h \
  wallet/external_signer_scriptpubkeyman.h \
  wallet/feebumper.h \
  wallet/fees.h \
  wallet/ismine.h \
  wallet/load.h \
  wallet/receive.h \
  wallet/rpcwallet.h \
  wallet/salvage.h \
  wallet/scriptpubkeyman.h \
  wallet/spend.h \
  wallet/sqlite.h \
  wallet/transaction.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  wallet/wallettool.h \
  wallet/walletutil.h \
  walletinitinterface.h \
  wallet/hdwalletdb.h \
  wallet/hdwallettypes.h \
  wallet/hdwallet.h \
  wallet/rpchdwallet.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h \
  zmq/zmqrpc.h \
  zmq/zmqutil.h \
  usbdevice/usbdevice.h \
  usbdevice/rpcusbdevice.h \
  insight/addressindex.h \
  insight/spentindex.h \
  insight/timestampindex.h \
  insight/balanceindex.h \
  insight/csindex.h \
  insight/insight.h \
  insight/rpc.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libfalcon_util_a-clientversion.$(OBJEXT): obj/build.h

ipc/capnp/libfalcon_ipc_a-ipc.$(OBJEXT): $(libfalcon_ipc_mpgen_input:=.h)

# server: shared between bitcoind and bitcoin-qt
# Contains code accessing mempool and chain state that is meant to be separated
# from wallet and gui code (see node/README.md). Shared code should go in
# libbitcoin_common or libbitcoin_util libraries, instead.
libfalcon_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(NATPMP_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libfalcon_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_server_a_SOURCES = \
  addrdb.cpp \
  addrman.cpp \
  banman.cpp \
  blockencodings.cpp \
  blockfilter.cpp \
  chain.cpp \
  consensus/tx_verify.cpp \
  dbwrapper.cpp \
  deploymentstatus.cpp \
  flatfile.cpp \
  httprpc.cpp \
  httpserver.cpp \
  i2p.cpp \
  index/base.cpp \
  index/blockfilterindex.cpp \
  index/coinstatsindex.cpp \
  index/txindex.cpp \
  init.cpp \
  mapport.cpp \
  miner.cpp \
  net.cpp \
  net_processing.cpp \
  node/blockstorage.cpp \
  node/coin.cpp \
  node/coinstats.cpp \
  node/context.cpp \
  node/interfaces.cpp \
  node/psbt.cpp \
  node/transaction.cpp \
  node/ui_interface.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/packages.cpp \
  policy/rbf.cpp \
  policy/settings.cpp \
  pow.cpp \
  pos/kernel.cpp \
  rest.cpp \
  rpc/anon.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  rpc/client.cpp \
  rpc/rpcutil.cpp \
  rpc/util.cpp \
  script/sigcache.cpp \
  shutdown.cpp \
  signet.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  txorphanage.cpp \
  txrequest.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  insight/insight.cpp \
  insight/rpc.cpp \
  $(BITCOIN_CORE_H)

if ENABLE_WALLET
libfalcon_server_a_SOURCES += wallet/init.cpp
endif
if !ENABLE_WALLET
libfalcon_server_a_SOURCES += dummywallet.cpp
endif

if ENABLE_ZMQ
libfalcon_zmq_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(ZMQ_CFLAGS)
libfalcon_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp \
  zmq/zmqrpc.cpp \
  zmq/zmqutil.cpp
endif

if ENABLE_USBDEVICE
libfalcon_usbdevice_a_CPPFLAGS = $(BITCOIN_INCLUDES) $(AM_CPPFLAGS) $(PROTOBUF_CFLAGS)
libfalcon_usbdevice_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_usbdevice_a_CFLAGS = -DHAVE_HIDAPI $(AM_CFLAGS) $(PIE_FLAGS)
libfalcon_usbdevice_a_SOURCES = \
  usbdevice/usbwrapper.cpp \
  usbdevice/usbwrapper.h \
  usbdevice/ledger/btchipApdu.h \
  usbdevice/ledger/ledgerLayer.h \
  usbdevice/ledger/ledgerLayer.c \
  usbdevice/ledger/dongleCommHidHidapi.h \
  usbdevice/ledger/dongleCommHidHidapi.c \
  usbdevice/usbdevice.cpp \
  usbdevice/debugdevice.h \
  usbdevice/debugdevice.cpp \
  usbdevice/ledgerdevice.h \
  usbdevice/ledgerdevice.cpp \
  usbdevice/trezor/messages.proto \
  usbdevice/trezor/messages-common.proto \
  usbdevice/trezor/messages-management.proto \
  usbdevice/trezor/messages-bitcoin.proto \
  usbdevice/trezordevice.h \
  usbdevice/trezordevice.cpp \
  usbdevice/rpcusbdevice.cpp
nodist_libfalcon_usbdevice_a_SOURCES = \
  usbdevice/trezor/messages.pb.h \
  usbdevice/trezor/messages.pb.cc \
  usbdevice/trezor/messages-common.pb.h \
  usbdevice/trezor/messages-common.pb.cc \
  usbdevice/trezor/messages-management.pb.h \
  usbdevice/trezor/messages-management.pb.cc \
  usbdevice/trezor/messages-bitcoin.pb.h \
  usbdevice/trezor/messages-bitcoin.pb.cc
BUILT_SOURCES = $(nodist_libfalcon_usbdevice_a_SOURCES)
endif

# wallet: shared between bitcoind and bitcoin-qt, but only linked
# when wallet enabled
libfalcon_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(SQLITE_CFLAGS)
libfalcon_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_wallet_a_SOURCES = \
  wallet/interfaces.cpp \
  wallet/rpchdwallet.cpp \
  wallet/hdwallet.cpp \
  wallet/hdwallettypes.cpp \
  wallet/hdwalletdb.cpp \
  wallet/coincontrol.cpp \
  wallet/context.cpp \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/dump.cpp \
  wallet/external_signer_scriptpubkeyman.cpp \
  wallet/feebumper.cpp \
  wallet/fees.cpp \
  wallet/interfaces.cpp \
  wallet/load.cpp \
  wallet/receive.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/scriptpubkeyman.cpp \
  wallet/spend.cpp \
  wallet/transaction.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  blind.cpp \
  key/stealth.cpp \
  pos/kernel.cpp \
  pos/miner.cpp \
  policy/rbf.cpp \
  wallet/walletutil.cpp \
  wallet/coinselection.cpp \
  $(BITCOIN_CORE_H)

if USE_SQLITE
libfalcon_wallet_a_SOURCES += wallet/sqlite.cpp
endif
if USE_BDB
libfalcon_wallet_a_SOURCES += wallet/bdb.cpp wallet/salvage.cpp
endif

libfalcon_wallet_tool_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_wallet_tool_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_wallet_tool_a_SOURCES = \
  wallet/wallettool.cpp \
  $(BITCOIN_CORE_H)

# crypto primitives library
crypto_libfalcon_crypto_base_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libfalcon_crypto_base_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libfalcon_crypto_base_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha_poly_aead.h \
  crypto/chacha_poly_aead.cpp \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hkdf_sha256_32.cpp \
  crypto/hkdf_sha256_32.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/poly1305.h \
  crypto/poly1305.cpp \
  crypto/muhash.h \
  crypto/muhash.cpp \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha3.cpp \
  crypto/sha3.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  crypto/siphash.cpp \
  crypto/siphash.h

if USE_ASM
crypto_libfalcon_crypto_base_a_SOURCES += crypto/sha256_sse4.cpp
endif

crypto_libfalcon_crypto_sse41_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libfalcon_crypto_sse41_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libfalcon_crypto_sse41_a_CXXFLAGS += $(SSE41_CXXFLAGS)
crypto_libfalcon_crypto_sse41_a_CPPFLAGS += -DENABLE_SSE41
crypto_libfalcon_crypto_sse41_a_SOURCES = crypto/sha256_sse41.cpp

crypto_libfalcon_crypto_avx2_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libfalcon_crypto_avx2_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libfalcon_crypto_avx2_a_CXXFLAGS += $(AVX2_CXXFLAGS)
crypto_libfalcon_crypto_avx2_a_CPPFLAGS += -DENABLE_AVX2
crypto_libfalcon_crypto_avx2_a_SOURCES = crypto/sha256_avx2.cpp

crypto_libfalcon_crypto_shani_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libfalcon_crypto_shani_a_CPPFLAGS = $(AM_CPPFLAGS)
crypto_libfalcon_crypto_shani_a_CXXFLAGS += $(SHANI_CXXFLAGS)
crypto_libfalcon_crypto_shani_a_CPPFLAGS += -DENABLE_SHANI
crypto_libfalcon_crypto_shani_a_SOURCES = crypto/sha256_shani.cpp

# consensus: shared between all executables that validate any consensus rules.
libfalcon_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/tx_check.cpp \
  consensus/validation.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/bitcoinconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  span.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  util/strencodings.cpp \
  util/strencodings.h \
  version.h

# common: shared between bitcoind, and bitcoin-qt and non-server tools
libfalcon_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_common_a_SOURCES = \
  base58.cpp \
  bech32.cpp \
  bloom.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  deploymentinfo.cpp \
  external_signer.cpp \
  init/common.cpp \
  key.cpp \
  anon.cpp \
  blind.cpp \
  key/keyutil.cpp \
  key/extkey.cpp \
  key/stealth.cpp \
  smsg/crypter.cpp \
  key_io.cpp \
  merkleblock.cpp \
  netaddress.cpp \
  netbase.cpp \
  net_permissions.cpp \
  outputtype.cpp \
  policy/feerate.cpp \
  policy/policy.cpp \
  protocol.cpp \
  psbt.cpp \
  rpc/rawtransaction_util.cpp \
  rpc/external_signer.cpp \
  rpc/util.cpp \
  scheduler.cpp \
  script/descriptor.cpp \
  script/sign.cpp \
  script/signingprovider.cpp \
  script/standard.cpp \
  warnings.cpp \
  $(BITCOIN_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libfalcon_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_util_a_SOURCES = \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  fs.cpp \
  interfaces/echo.cpp \
  interfaces/handler.cpp \
  interfaces/init.cpp \
  logging.cpp \
  random.cpp \
  randomenv.cpp \
  rpc/request.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util/asmap.cpp \
  util/bip32.cpp \
  util/bytevectorhash.cpp \
  util/error.cpp \
  util/fees.cpp \
  util/getuniquepath.cpp \
  util/hasher.cpp \
  util/sock.cpp \
  util/system.cpp \
  util/message.cpp \
  util/moneystr.cpp \
  util/rbf.cpp \
  util/readwritefile.cpp \
  util/settings.cpp \
  util/thread.cpp \
  util/threadnames.cpp \
  util/serfloat.cpp \
  util/spanparsing.cpp \
  util/strencodings.cpp \
  util/string.cpp \
  util/time.cpp \
  util/tokenpipe.cpp \
  $(BITCOIN_CORE_H)

if USE_LIBEVENT
libfalcon_util_a_SOURCES += util/url.cpp
endif

libfalcon_smsg_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_smsg_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_smsg_a_CFLAGS = $(AM_CFLAGS) $(PIE_FLAGS)
libfalcon_smsg_a_SOURCES = \
  lz4/lz4.h \
  lz4/lz4.c \
  xxhash/xxhash.h \
  xxhash/xxhash.c \
  interfaces/handler.cpp \
  smsg/crypter.cpp \
  smsg/keystore.h \
  smsg/keystore.cpp \
  smsg/db.cpp \
  smsg/smessage.cpp \
  smsg/rpcsmessage.cpp


libfalcon_mnemonic_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_mnemonic_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_mnemonic_a_CFLAGS = $(AM_CFLAGS) $(PIE_FLAGS)
libfalcon_mnemonic_a_SOURCES = \
  unilib/uninorms.h \
  unilib/utf8.h \
  unilib/uninorms.cpp \
  unilib/utf8.cpp \
  key/wordlists/chinese_simplified.h \
  key/wordlists/chinese_traditional.h \
  key/wordlists/english.h \
  key/wordlists/french.h \
  key/wordlists/japanese.h \
  key/wordlists/spanish.h \
  key/wordlists/italian.h \
  key/wordlists/korean.h \
  key/wordlists/czech.h \
  key/mnemonic.h \
  key/mnemonic.cpp \
  rpc/mnemonic.cpp

if GLIBC_BACK_COMPAT
libfalcon_util_a_SOURCES += compat/glibc_compat.cpp
AM_LDFLAGS += $(COMPAT_LDFLAGS)
endif

# cli: shared between bitcoin-cli and bitcoin-qt
libfalcon_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libfalcon_cli_a_SOURCES = \
  compat/stdin.h \
  compat/stdin.cpp \
  rpc/client.cpp \
  $(BITCOIN_CORE_H)


nodist_libfalcon_util_a_SOURCES = $(srcdir)/obj/build.h
#

# bitcoind & bitcoin-node binaries #
falcon_daemon_sources = bitcoind.cpp
falcon_bin_cppflags = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
falcon_bin_cxxflags = $(AM_CXXFLAGS) $(PIE_FLAGS)
falcon_bin_ldflags = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
falcon_daemon_sources += bitcoind-res.rc
endif

falcon_bin_ldadd = \
  $(LIBFALCON_WALLET) \
  $(LIBFALCON_COMMON) \
  $(LIBFALCON_UTIL) \
  $(LIBUNIVALUE) \
  $(LIBFALCON_ZMQ) \
  $(LIBFALCON_CONSENSUS) \
  $(LIBFALCON_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1) \
  $(LIBFALCON_SMSG) \
  $(LIBFALCON_MNEMONIC) \
  $(LIBFALCON_USBDEVICE)

falcon_bin_ldadd += $(BOOST_LIBS) $(BDB_LIBS) $(MINIUPNPC_LIBS) $(NATPMP_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS) $(SQLITE_LIBS)
if ENABLE_USBDEVICE
falcon_bin_ldadd += $(USB_LIBS) $(HIDAPI_LIBS) $(PROTOBUF_LIBS)
endif

falcond_SOURCES = $(falcon_daemon_sources) init/bitcoind.cpp
falcond_CPPFLAGS = $(falcon_bin_cppflags)
falcond_CXXFLAGS = $(falcon_bin_cxxflags)
falcond_LDFLAGS = $(falcon_bin_ldflags)
falcond_LDADD = $(LIBFALCON_SERVER) $(falcon_bin_ldadd)

falcon_node_SOURCES = $(falcon_daemon_sources) init/bitcoin-node.cpp
falcon_node_CPPFLAGS = $(falcon_bin_cppflags)
falcon_node_CXXFLAGS = $(falcon_bin_cxxflags)
falcon_node_LDFLAGS = $(falcon_bin_ldflags)
falcon_node_LDADD = $(LIBFALCON_SERVER) $(falcon_bin_ldadd) $(LIBFALCON_IPC) $(LIBMULTIPROCESS_LIBS)

# bitcoin-cli binary #
falcon_cli_SOURCES = bitcoin-cli.cpp
falcon_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES) $(EVENT_CFLAGS)
falcon_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
falcon_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
falcon_cli_SOURCES += bitcoin-cli-res.rc
endif

falcon_cli_LDADD = \
  $(LIBFALCON_CLI) \
  $(LIBUNIVALUE) \
  $(LIBFALCON_UTIL) \
  $(LIBFALCON_CRYPTO)

falcon_cli_LDADD += $(BOOST_LIBS) $(EVENT_LIBS)
#

# bitcoin-tx binary #
falcon_tx_SOURCES = bitcoin-tx.cpp
falcon_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
falcon_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
falcon_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
falcon_tx_SOURCES += bitcoin-tx-res.rc
endif

falcon_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBFALCON_COMMON) \
  $(LIBFALCON_UTIL) \
  $(LIBFALCON_CONSENSUS) \
  $(LIBFALCON_CRYPTO) \
  $(LIBSECP256K1)

falcon_tx_LDADD += $(BOOST_LIBS)
#

# bitcoin-wallet binary #
falcon_wallet_SOURCES = bitcoin-wallet.cpp
falcon_wallet_CPPFLAGS = $(falcon_bin_cppflags)
falcon_wallet_CXXFLAGS = $(falcon_bin_cxxflags)
falcon_wallet_LDFLAGS = $(falcon_bin_ldflags)
#falcon_wallet_LDADD = $(LIBFALCON_WALLET_TOOL) $(falcon_bin_ldadd)
falcon_wallet_LDADD = \
  $(LIBFALCON_WALLET_TOOL) \
  $(LIBFALCON_WALLET) \
  $(LIBFALCON_SERVER) \
  $(LIBFALCON_COMMON) \
  $(LIBFALCON_CONSENSUS) \
  $(LIBFALCON_UTIL) \
  $(LIBFALCON_CRYPTO) \
  $(LIBFALCON_SMSG) \
  $(LIBFALCON_MNEMONIC) \
  $(LIBFALCON_USBDEVICE) \
  $(LIBFALCON_ZMQ) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1) \
  $(LIBUNIVALUE)
falcon_wallet_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(MINIUPNPC_LIBS) $(ZMQ_LIBS) $(SQLITE_LIBS)
if ENABLE_USBDEVICE
falcon_wallet_LDADD += $(USB_LIBS) $(HIDAPI_LIBS) $(PROTOBUF_LIBS)
endif

if TARGET_WINDOWS
falcon_wallet_SOURCES += bitcoin-wallet-res.rc
endif
#

# bitcoin-util binary #
falcon_util_SOURCES = bitcoin-util.cpp
falcon_util_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
falcon_util_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
falcon_util_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS) $(PTHREAD_FLAGS)

if TARGET_WINDOWS
falcon_util_SOURCES += bitcoin-util-res.rc
endif

falcon_util_LDADD = \
  $(LIBFALCON_COMMON) \
  $(LIBFALCON_UTIL) \
  $(LIBUNIVALUE) \
  $(LIBFALCON_CONSENSUS) \
  $(LIBFALCON_CRYPTO) \
  $(LIBSECP256K1)

falcon_util_LDADD += $(BOOST_LIBS)
#

# bitcoinconsensus library #
if BUILD_BITCOIN_LIBS
include_HEADERS = script/falconconsensus.h
libfalconconsensus_la_SOURCES = support/cleanse.cpp $(crypto_libfalcon_crypto_base_a_SOURCES) $(libfalcon_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libfalconconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libfalconconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libfalconconsensus_la_LIBADD = $(LIBSECP256K1) $(BOOST_LIBS)
libfalconconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_BITCOIN_INTERNAL
libfalconconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crc32c/src/*.gcda crc32c/src/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += index/*.gcda index/*.gcno
CLEANFILES += interfaces/*.gcda interfaces/*.gcno
CLEANFILES += node/*.gcda node/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += rpc/*.gcda rpc/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += util/*.gcda util/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno
CLEANFILES += obj/build.h

EXTRA_DIST = $(CTAES_DIST)


config/bitcoin-config.h: config/stamp-h1
	@$(MAKE) -C $(top_builddir) $(subdir)/$(@)
config/stamp-h1: $(top_srcdir)/$(subdir)/config/bitcoin-config.h.in $(top_builddir)/config.status
	$(AM_V_at)$(MAKE) -C $(top_builddir) $(subdir)/$(@)
$(top_srcdir)/$(subdir)/config/bitcoin-config.h.in:  $(am__configure_deps)
	$(AM_V_at)$(MAKE) -C $(top_srcdir) $(subdir)/config/bitcoin-config.h.in

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -f config.h
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

check-symbols: $(bin_PROGRAMS)
if TARGET_DARWIN
	@echo "Checking macOS dynamic libraries..."
	$(AM_V_at) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

if TARGET_WINDOWS
	@echo "Checking Windows dynamic libraries..."
	$(AM_V_at) $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

if TARGET_LINUX
	@echo "Checking glibc back compat..."
	$(AM_V_at) CPPFILT='$(CPPFILT)' $(PYTHON) $(top_srcdir)/contrib/devtools/symbol-check.py $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) $(PYTHON) $(top_srcdir)/contrib/devtools/security-check.py $(bin_PROGRAMS)
endif

if ENABLE_USBDEVICE
%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<
endif

libfalcon_ipc_mpgen_input = \
  ipc/capnp/echo.capnp \
  ipc/capnp/init.capnp
EXTRA_DIST += $(libfalcon_ipc_mpgen_input)
%.capnp:

if BUILD_MULTIPROCESS
LIBFALCON_IPC=libfalcon_ipc.a
libfalcon_ipc_a_SOURCES = \
  ipc/capnp/context.h \
  ipc/capnp/init-types.h \
  ipc/capnp/protocol.cpp \
  ipc/capnp/protocol.h \
  ipc/context.h \
  ipc/exception.h \
  ipc/interfaces.cpp \
  ipc/process.cpp \
  ipc/process.h \
  ipc/protocol.h
libfalcon_ipc_a_CPPFLAGS = $(AM_CPPFLAGS) $(BITCOIN_INCLUDES)
libfalcon_ipc_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS) $(LIBMULTIPROCESS_CFLAGS)

include $(MPGEN_PREFIX)/include/mpgen.mk
libfalcon_ipc_mpgen_output = \
  $(libfalcon_ipc_mpgen_input:=.c++) \
  $(libfalcon_ipc_mpgen_input:=.h) \
  $(libfalcon_ipc_mpgen_input:=.proxy-client.c++) \
  $(libfalcon_ipc_mpgen_input:=.proxy-server.c++) \
  $(libfalcon_ipc_mpgen_input:=.proxy-types.c++) \
  $(libfalcon_ipc_mpgen_input:=.proxy-types.h) \
  $(libfalcon_ipc_mpgen_input:=.proxy.h)
nodist_libfalcon_ipc_a_SOURCES = $(libfalcon_ipc_mpgen_output)
CLEANFILES += $(libfalcon_ipc_mpgen_output)
endif

if EMBEDDED_LEVELDB
include Makefile.crc32c.include
include Makefile.leveldb.include
endif

include Makefile.test_util.include
include Makefile.test_fuzz.include

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
