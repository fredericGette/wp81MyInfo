//
// BluetoothAddress.cpp
// Read the Bluetooth MAC address of the phone.
// See Lumia520/content/drivers/qcbluetooth8930.md, chapter "Bluetooth address".
//

#include "pch.h"
#include <windows.h>
#include "BluetoothAddress.h"

// Content of the file:
// 00    : must be 1
// 01    : 06 (length of the address ?)
// 02-07 : Bluetooth address, most significant byte first
#define provisionFile L"C:\\DPP\\QCOM\\BT.PROVISION"
#define provisionSize 8
#define provisionVersion 1
#define provisionAddressOffset 2

// REG_BINARY, 6 bytes, least significant byte first.
#define parametersKey L"SYSTEM\\CurrentControlSet\\Services\\qcbluetooth\\Parameters"
#define addressValue L"BDAddr"

#define registryLocalMachine (reinterpret_cast<HKEY>(static_cast<ULONG_PTR>(0x80000002)))
#define registryKeyRead 0x20019
#define registryBinaryType 3

// The registry functions aren't declared for the applications: they are found at run time in KernelBase.dll.
typedef LONG(WINAPI *RegOpenKeyExWFunction)(HKEY, LPCWSTR, DWORD, DWORD, HKEY*);
typedef LONG(WINAPI *RegQueryValueExWFunction)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef LONG(WINAPI *RegCloseKeyFunction)(HKEY);

static HMODULE GetKernelBase()
{
	MEMORY_BASIC_INFORMATION info = {};
	if (VirtualQuery(reinterpret_cast<void*>(&DisableThreadLibraryCalls), &info, sizeof(info)) == 0)
	{
		return nullptr;
	}
	return reinterpret_cast<HMODULE>(info.AllocationBase);
}

static std::wstring ErrorText(const wchar_t* source, unsigned long code)
{
	return std::wstring(source) + L": error " + std::to_wstring(code);
}

// The driver replaces a missing address by 00-00-00-00-00-00.
static bool IsValid(const unsigned char* address)
{
	for (int i = 0; i < bluetoothAddressSize; ++i)
	{
		if (address[i] != 0)
		{
			return true;
		}
	}
	return false;
}

static bool ParseProvision(const unsigned char* data, DWORD size, unsigned char* address)
{
	if (size < provisionSize || data[0] != provisionVersion || !IsValid(data + provisionAddressOffset))
	{
		return false;
	}
	memcpy(address, data + provisionAddressOffset, bluetoothAddressSize);
	return true;
}

static bool ParseRegistryValue(const unsigned char* data, DWORD size, DWORD type, unsigned char* address)
{
	if (type != registryBinaryType || size != bluetoothAddressSize || !IsValid(data))
	{
		return false;
	}
	for (int i = 0; i < bluetoothAddressSize; ++i)
	{
		address[i] = data[bluetoothAddressSize - 1 - i];
	}
	return true;
}

static bool ReadFromFile(unsigned char* address, std::wstring& error)
{
	HANDLE file = CreateFile2(provisionFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
	if (file == INVALID_HANDLE_VALUE)
	{
		error = ErrorText(L"BT.PROVISION", GetLastError());
		return false;
	}

	unsigned char data[provisionSize];
	DWORD size = 0;
	BOOL success = ReadFile(file, data, sizeof(data), &size, nullptr);
	DWORD readError = GetLastError();
	CloseHandle(file);

	if (!success)
	{
		error = ErrorText(L"BT.PROVISION", readError);
		return false;
	}
	if (!ParseProvision(data, size, address))
	{
		error = L"BT.PROVISION: invalid content";
		return false;
	}
	return true;
}

static bool ReadFromRegistry(unsigned char* address, std::wstring& error)
{
	HMODULE kernelBase = GetKernelBase();
	if (kernelBase == nullptr)
	{
		error = L"BDAddr: KernelBase not found";
		return false;
	}

	auto regOpenKeyExW = reinterpret_cast<RegOpenKeyExWFunction>(GetProcAddress(kernelBase, "RegOpenKeyExW"));
	auto regQueryValueExW = reinterpret_cast<RegQueryValueExWFunction>(GetProcAddress(kernelBase, "RegQueryValueExW"));
	auto regCloseKey = reinterpret_cast<RegCloseKeyFunction>(GetProcAddress(kernelBase, "RegCloseKey"));
	if (regOpenKeyExW == nullptr || regQueryValueExW == nullptr || regCloseKey == nullptr)
	{
		error = L"BDAddr: registry functions not found";
		return false;
	}

	HKEY key = nullptr;
	LONG status = regOpenKeyExW(registryLocalMachine, parametersKey, 0, registryKeyRead, &key);
	if (status != ERROR_SUCCESS)
	{
		error = ErrorText(L"BDAddr", status);
		return false;
	}

	unsigned char data[bluetoothAddressSize];
	DWORD size = sizeof(data);
	DWORD type = 0;
	status = regQueryValueExW(key, addressValue, nullptr, &type, data, &size);
	regCloseKey(key);

	if (status != ERROR_SUCCESS)
	{
		error = ErrorText(L"BDAddr", status);
		return false;
	}
	if (!ParseRegistryValue(data, size, type, address))
	{
		error = L"BDAddr: invalid content";
		return false;
	}
	return true;
}

bool ReadBluetoothAddress(unsigned char address[bluetoothAddressSize], std::wstring& error)
{
	std::wstring fileError;
	std::wstring registryError;
	if (ReadFromFile(address, fileError) || ReadFromRegistry(address, registryError))
	{
		error.clear();
		return true;
	}
	error = fileError + L", " + registryError;
	return false;
}
