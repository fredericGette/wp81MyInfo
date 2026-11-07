//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace Wp81MyInfo;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Phone::UI::Input;
using namespace Windows::Networking::Connectivity;
using namespace Wp81MyInfoTasks;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
	InitializeComponent();

	HardwareButtons::BackPressed += ref new EventHandler<BackPressedEventArgs ^>(this, &MainPage::HardwareButtons_BackPressed);

	// Refresh the addresses when the network changes while the application is running.
	NetworkInformation::NetworkStatusChanged += ref new NetworkStatusChangedEventHandler(this, &MainPage::OnNetworkStatusChanged);

	// Refresh the addresses when the user comes back to the application.
	Application::Current->Resuming += ref new EventHandler<Object^>(this, &MainPage::OnResuming);
}

void MainPage::HardwareButtons_BackPressed(Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs^ e)
{
	// Return to the addresses.
	if (!IsStateInfo())
	{
		//Indicate the back button press is handled so the app does not exit
		e->Handled = true;

		StateInfo();
	}
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

	RefreshInfo();
}

void Wp81MyInfo::Debug(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[100];
	vsnprintf_s(buffer, sizeof(buffer), format, args);

	OutputDebugStringA(buffer);

	va_end(args);
}

void MainPage::OnNetworkStatusChanged(Object^ sender)
{
	// Not called on the UI thread.
	Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
	{
		RefreshInfo();
	}));
}

void MainPage::OnResuming(Object^ sender, Object^ e)
{
	// Not called on the UI thread.
	Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
	{
		RefreshInfo();
	}));
}

/// <summary>
/// Display the current addresses and update the tile.
/// </summary>
void MainPage::RefreshInfo()
{
	String^ wifiIp = InfoTile::GetWiFiIP();
	if (wifiIp != nullptr)
	{
		Debug("Wifi interface IP:"); OutputDebugString(wifiIp->Data()); Debug("\n");
		TextBlockIp->Text = wifiIp;
	}
	else
	{
		Debug("No Wifi interface IP\n");
		TextBlockIp->Text = L"No Wi-Fi";
	}

	String^ bluetoothAddress = InfoTile::GetBluetoothAddress();
	if (bluetoothAddress != nullptr)
	{
		TextBlockBt->Text = bluetoothAddress;
		TextBlockBtError->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	}
	else
	{
		String^ error = InfoTile::GetBluetoothError();
		Debug("No Bluetooth address:"); OutputDebugString(error->Data()); Debug("\n");
		TextBlockBt->Text = L"Unknown";
		TextBlockBtError->Text = error;
		TextBlockBtError->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}

	try
	{
		InfoTile::Update();
		auto now = (ref new Calendar())->GetDateTime();
		TextBlockUpdated->Text = ref new String(L"Tile updated at ") + DateTimeFormatter::LongTime->Format(now);
	}
	catch (Platform::Exception^ e)
	{
		OutputDebugString(e->Message->Data());
		TextBlockUpdated->Text = L"Tile update failed.";
	}
}

void Wp81MyInfo::MainPage::AppBarButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ b = (Button^)sender;
	if (b->Tag->ToString() == "Refresh")
	{
		RefreshInfo();
	}
	else if (b->Tag->ToString() == "Help")
	{
		// Want to see the help page.
		StateHelp();
	}
}

////////////////////////////////////

void Wp81MyInfo::MainPage::StateInfo()
{
	PanelInfo->Visibility = Windows::UI::Xaml::Visibility::Visible;
	PanelHelp->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	RefreshAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
	HelpAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Visible;
}

void Wp81MyInfo::MainPage::StateHelp()
{
	PanelInfo->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	PanelHelp->Visibility = Windows::UI::Xaml::Visibility::Visible;
	RefreshAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	HelpAppBarButton->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
}

bool Wp81MyInfo::MainPage::IsStateInfo()
{
	return PanelInfo->Visibility == Windows::UI::Xaml::Visibility::Visible;
}
