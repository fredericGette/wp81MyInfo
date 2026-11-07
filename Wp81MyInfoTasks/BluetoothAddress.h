//
// BluetoothAddress.h
// Read the Bluetooth MAC address of the phone.
//

#pragma once

#include <string>

#define bluetoothAddressSize 6

// Read the address the same way as the driver QcBluetooth8930.sys:
// the file C:\DPP\QCOM\BT.PROVISION, then the registry value BDAddr.
// address receives the most significant byte first.
// Returns false and describes the failures in error if the address can't be read.
bool ReadBluetoothAddress(unsigned char address[bluetoothAddressSize], std::wstring& error);
