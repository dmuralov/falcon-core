#!/usr/bin/env bash
#
# Copyright (c) 2018-2020 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
# Check for circular dependencies

export LC_ALL=C

EXPECTED_CIRCULAR_DEPENDENCIES=(
    "chainparamsbase -> util/system -> chainparamsbase"
    "node/blockstorage -> validation -> node/blockstorage"
    "index/blockfilterindex -> node/blockstorage -> validation -> index/blockfilterindex"
    "index/base -> validation -> index/blockfilterindex -> index/base"
    "index/coinstatsindex -> node/coinstats -> index/coinstatsindex"
    "policy/fees -> txmempool -> policy/fees"
    "qt/addresstablemodel -> qt/walletmodel -> qt/addresstablemodel"
    "qt/recentrequeststablemodel -> qt/walletmodel -> qt/recentrequeststablemodel"
    "qt/sendcoinsdialog -> qt/walletmodel -> qt/sendcoinsdialog"
    "qt/transactiontablemodel -> qt/walletmodel -> qt/transactiontablemodel"
    "txmempool -> validation -> txmempool"
    "wallet/fees -> wallet/wallet -> wallet/fees"
    "wallet/wallet -> wallet/walletdb -> wallet/wallet"
    "node/coinstats -> validation -> node/coinstats"

    # Temporary circular dependencies that allow wallet.h/wallet.cpp to be
    # split up in a MOVEONLY commit. These are removed in #21206.
    "wallet/receive -> wallet/wallet -> wallet/receive"
    "wallet/spend -> wallet/wallet -> wallet/spend"

# Falcon
    "anon -> txmempool -> anon"
    "anon -> validation -> anon"
    "consensus/tx_verify -> validation -> consensus/tx_verify"
    "insight/insight -> txdb -> insight/insight"
    "insight/insight -> txmempool -> insight/insight"
    "insight/insight -> validation -> insight/insight"
    "key/extkey -> key_io -> key/extkey"
    "key/stealth -> key_io -> key/stealth"
    "pos/kernel -> validation -> pos/kernel"
    "pos/miner -> wallet/hdwallet -> pos/miner"
    "smsg/db -> smsg/smessage -> smsg/db"
    "smsg/smessage -> validation -> smsg/smessage"
    "txdb -> validation -> txdb"
    "usbdevice/debugdevice -> usbdevice/usbdevice -> usbdevice/debugdevice"
    "usbdevice/ledgerdevice -> usbdevice/usbdevice -> usbdevice/ledgerdevice"
    "usbdevice/trezordevice -> usbdevice/usbdevice -> usbdevice/trezordevice"
    "usbdevice/usbdevice -> wallet/hdwallet -> usbdevice/usbdevice"
    "wallet/hdwallet -> wallet/hdwalletdb -> wallet/hdwallet"
    "wallet/hdwallet -> wallet/wallet -> wallet/hdwallet"
    "key/extkey -> key_io -> script/standard -> key/extkey"
    "key/stealth -> key_io -> script/standard -> key/stealth"
    "smsg/smessage -> wallet/hdwallet -> smsg/smessage"
    "net_processing -> smsg/smessage -> wallet/hdwallet -> pos/kernel -> node/transaction -> net_processing"
    "net_processing -> smsg/smessage -> node/context -> net_processing"
    "net_processing -> smsg/smessage -> net_processing"
    "net_processing -> validation -> net_processing"
)

EXIT_CODE=0

CIRCULAR_DEPENDENCIES=()

IFS=$'\n'
for CIRC in $(cd src && ../contrib/devtools/circular-dependencies.py {*,*/*,*/*/*}.{h,cpp} | sed -e 's/^Circular dependency: //'); do
    CIRCULAR_DEPENDENCIES+=( "$CIRC" )
    IS_EXPECTED_CIRC=0
    for EXPECTED_CIRC in "${EXPECTED_CIRCULAR_DEPENDENCIES[@]}"; do
        if [[ "${CIRC}" == "${EXPECTED_CIRC}" ]]; then
            IS_EXPECTED_CIRC=1
            break
        fi
    done
    if [[ ${IS_EXPECTED_CIRC} == 0 ]]; then
        echo "A new circular dependency in the form of \"${CIRC}\" appears to have been introduced."
        echo
        EXIT_CODE=1
    fi
done

for EXPECTED_CIRC in "${EXPECTED_CIRCULAR_DEPENDENCIES[@]}"; do
    IS_PRESENT_EXPECTED_CIRC=0
    for CIRC in "${CIRCULAR_DEPENDENCIES[@]}"; do
        if [[ "${CIRC}" == "${EXPECTED_CIRC}" ]]; then
            IS_PRESENT_EXPECTED_CIRC=1
            break
        fi
    done
    if [[ ${IS_PRESENT_EXPECTED_CIRC} == 0 ]]; then
        echo "Good job! The circular dependency \"${EXPECTED_CIRC}\" is no longer present."
        echo "Please remove it from EXPECTED_CIRCULAR_DEPENDENCIES in $0"
        echo "to make sure this circular dependency is not accidentally reintroduced."
        echo
        EXIT_CODE=1
    fi
done

exit ${EXIT_CODE}
