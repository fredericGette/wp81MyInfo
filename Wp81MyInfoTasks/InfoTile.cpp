//
// InfoTile.cpp
// Implementation of the InfoTile class.
//

#include "pch.h"
#include <algorithm>
#include "InfoTile.h"
#include "BluetoothAddress.h"

using namespace Wp81MyInfoTasks;

using namespace Platform;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Networking;
using namespace Windows::Networking::Connectivity;
using namespace Windows::UI::Notifications;

#define noWiFiText L"No Wi-Fi"
#define unknownText L"Unknown"
#define ipPrefix L"IP: "
#define bluetoothPrefix L"BT: "
#define noBreakSpace L'\u00A0'

static std::wstring FormatAddress(const unsigned char address[bluetoothAddressSize], const wchar_t* separator)
{
	wchar_t text[32];
	swprintf_s(text, L"%02X%s%02X%s%02X%s%02X%s%02X%s%02X",
		address[0], separator, address[1], separator, address[2], separator,
		address[3], separator, address[4], separator, address[5]);
	return text;
}

// Keep a line of the tile in one piece when the tile wraps its text.
static std::wstring NoBreak(std::wstring line)
{
	std::replace(line.begin(), line.end(), L' ', noBreakSpace);
	return line;
}

/**
* see https://social.msdn.microsoft.com/Forums/sqlserver/en-US/2fda9c75-135c-4ead-9a6c-28d78a83b6e0/force-winsock2-socket-to-use-wifi-connection?forum=wpdevelop
*/
String^ InfoTile::GetWiFiIP()
{
	String^ ipAddress = nullptr;
	auto hostnames = NetworkInformation::GetHostNames();

	for (unsigned int i = 0; i < hostnames->Size; ++i)
	{
		auto hn = hostnames->GetAt(i);

		// Keep only IPv4 addresses (ignore IPv6 and DNS names).
		if (hn->Type == HostNameType::Ipv4 && hn->IPInformation != nullptr && hn->IPInformation->NetworkAdapter != nullptr)
		{
			//IanaInterfaceType == 71 => Wifi
			//IanaInterfaceType == 6 => Ethernet (Emulator)
			auto type = hn->IPInformation->NetworkAdapter->IanaInterfaceType;
			if (type == 71 || type == 6)
			{
				ipAddress = hn->DisplayName;
			}
		}
	}

	return ipAddress;
}

String^ InfoTile::GetBluetoothAddress()
{
	unsigned char address[bluetoothAddressSize];
	std::wstring error;
	if (!ReadBluetoothAddress(address, error))
	{
		return nullptr;
	}
	return ref new String(FormatAddress(address, L":").c_str());
}

String^ InfoTile::GetBluetoothError()
{
	unsigned char address[bluetoothAddressSize];
	std::wstring error;
	ReadBluetoothAddress(address, error);
	return ref new String(error.c_str());
}

void InfoTile::Update()
{
	String^ ipAddress = GetWiFiIP();
	std::wstring ipLine = std::wstring(ipPrefix) + (ipAddress != nullptr ? ipAddress->Data() : noWiFiText);

	unsigned char address[bluetoothAddressSize];
	std::wstring error;
	std::wstring squareBluetoothLine = std::wstring(bluetoothPrefix) + unknownText;
	std::wstring wideBluetoothLine = squareBluetoothLine;
	if (ReadBluetoothAddress(address, error))
	{
		// Without the colons in the square tile: its lines don't wrap and would be truncated.
		squareBluetoothLine = bluetoothPrefix + FormatAddress(address, L"");
		wideBluetoothLine = bluetoothPrefix + FormatAddress(address, L":");
	}

	// Square tile: one address per line, in regular text (this template doesn't wrap its lines).
	// Wide tile: large text, the biggest size of the templates. It's a single wrapped string: each address
	// is unbreakable, so they don't fit on the same line and the second one goes to the next line.
	// The 71x71 tile is too small for text and keeps its logo.
	std::wstring xml =
		L"<tile><visual version=\"2\">"
		L"<binding template=\"TileSquare150x150Text03\" fallback=\"TileSquareText03\">"
		L"<text id=\"1\">" + ipLine + L"</text>"
		L"<text id=\"2\">" + squareBluetoothLine + L"</text>"
		L"</binding>"
		L"<binding template=\"TileWide310x150Text03\" fallback=\"TileWideText03\">"
		L"<text id=\"1\">" + NoBreak(ipLine) + L" " + NoBreak(wideBluetoothLine) + L"</text>"
		L"</binding>"
		L"</visual></tile>";

	XmlDocument^ tileXml = ref new XmlDocument();
	tileXml->LoadXml(ref new String(xml.c_str()));

	TileUpdateManager::CreateTileUpdaterForApplication()->Update(ref new TileNotification(tileXml));
}
