// Copyright (c) 2018-2021 The Falcon Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FALCON_USBDEVICE_RPCUSBDEVICE_H
#define FALCON_USBDEVICE_RPCUSBDEVICE_H

class CRPCCommand;
class CRPCTable;

Span<const CRPCCommand> GetDeviceWalletRPCCommands();

void RegisterUSBDeviceRPC(CRPCTable &t);

#endif // FALCON_USBDEVICE_RPCUSBDEVICE_H
