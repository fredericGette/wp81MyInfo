//
// InfoTile.h
// Read the Wi-Fi IP address and the Bluetooth MAC address, and display them in the tile of the application.
//

#pragma once

namespace Wp81MyInfoTasks
{
	public ref class InfoTile sealed
	{
	public:
		// IPv4 address of the Wi-Fi interface, or nullptr if the phone is not connected to a Wi-Fi network.
		static Platform::String^ GetWiFiIP();

		// Bluetooth MAC address (78:92:3E:C7:7B:53), or nullptr if it can't be read.
		static Platform::String^ GetBluetoothAddress();

		// Why the Bluetooth MAC address can't be read, or an empty string if it can.
		static Platform::String^ GetBluetoothError();

		// Display the current addresses in the tile of the application.
		static void Update();
	};
}
