//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace Wp81MyInfo
{
	/// <summary>
	/// Display the Wi-Fi IP address and the Bluetooth MAC address of the phone, and update the tile of the application.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

		void HardwareButtons_BackPressed(Object ^ sender, Windows::Phone::UI::Input::BackPressedEventArgs ^ e);

	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		void AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnNetworkStatusChanged(Platform::Object^ sender);
		void OnResuming(Platform::Object^ sender, Platform::Object^ e);
		void RefreshInfo();
		void StateInfo();
		void StateHelp();
		bool IsStateInfo();
	};

	void Debug(const char * format, ...);

}
