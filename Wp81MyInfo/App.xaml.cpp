//
// App.xaml.cpp
// Implementation of the App class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace Wp81MyInfo;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Animation;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

// Must match the EntryPoint declared in Package.appxmanifest.
#define taskEntryPoint "Wp81MyInfoTasks.UpdateTileTask"

// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkID=391641

/// <summary>
/// Initializes the singleton application object.  This is the first line of authored code
/// executed, and as such is the logical equivalent of main() or WinMain().
/// </summary>
App::App()
{
	InitializeComponent();
	Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
}

/// <summary>
/// Invoked when the application is launched normally by the end user. Other entry points
/// will be used when the application is launched to open a specific file, to display
/// search results, and so forth.
/// </summary>
/// <param name="e">Details about the launch request and process.</param>
void App::OnLaunched(LaunchActivatedEventArgs^ e)
{
#if _DEBUG
	if (IsDebuggerPresent())
	{
		DebugSettings->EnableFrameRateCounter = true;
	}
#endif

	auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

	// Do not repeat app initialization when the Window already has content,
	// just ensure that the window is active.
	if (rootFrame == nullptr)
	{
		// Create a Frame to act as the navigation context and associate it with
		// a SuspensionManager key
		rootFrame = ref new Frame();

		rootFrame->CacheSize = 1;

		// Set the default language
		rootFrame->Language = Windows::Globalization::ApplicationLanguages::Languages->GetAt(0);

		// Place the frame in the current Window
		Window::Current->Content = rootFrame;

		// Keep the tile up to date when the application is not running.
		RegisterBackgroundTasks();
	}

	if (rootFrame->Content == nullptr)
	{
		// Removes the turnstile navigation for startup.
		if (rootFrame->ContentTransitions != nullptr)
		{
			_transitions = ref new TransitionCollection();
			for (auto transition : rootFrame->ContentTransitions)
			{
				_transitions->Append(transition);
			}
		}

		rootFrame->ContentTransitions = nullptr;
		_firstNavigatedToken = rootFrame->Navigated += ref new NavigatedEventHandler(this, &App::RootFrame_FirstNavigated);

		// When the navigation stack isn't restored navigate to the first page,
		// configuring the new page by passing required information as a navigation
		// parameter.
		if (!rootFrame->Navigate(MainPage::typeid, e->Arguments))
		{
			throw ref new FailureException("Failed to create initial page");
		}
	}

	// Ensure the current window is active
	Window::Current->Activate();
}

/// <summary>
/// Restores the content transitions after the app has launched.
/// </summary>
void App::RootFrame_FirstNavigated(Object^ sender, NavigationEventArgs^ e)
{
	auto rootFrame = safe_cast<Frame^>(sender);

	TransitionCollection^ newTransitions;
	if (_transitions == nullptr)
	{
		newTransitions = ref new TransitionCollection();
		newTransitions->Append(ref new NavigationThemeTransition());
	}
	else
	{
		newTransitions = _transitions;
	}

	rootFrame->ContentTransitions = newTransitions;
	rootFrame->Navigated -= _firstNavigatedToken;
}

/// <summary>
/// Invoked when application execution is being suspended. Application state is saved
/// without knowing whether the application will be terminated or resumed with the contents
/// of memory still intact.
/// </summary>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
	(void) sender;	// Unused parameter
	(void) e;		// Unused parameter
}

/// <summary>
/// Register the background task which updates the tile:
/// - each time the network changes (Wi-Fi connected/disconnected, new address...)
/// - every 30 minutes, in case a network change was missed.
/// </summary>
void App::RegisterBackgroundTasks()
{
	// On Windows Phone the access must be requested again after an update of the application,
	// so always reset it.
	BackgroundExecutionManager::RemoveAccess();

	create_task(BackgroundExecutionManager::RequestAccessAsync()).then([](BackgroundAccessStatus status)
	{
		if (status != BackgroundAccessStatus::AllowedMayUseActiveRealTimeConnectivity
			&& status != BackgroundAccessStatus::AllowedWithAlwaysOnRealTimeConnectivity)
		{
			Debug("Background access denied (%d)\n", (int)status);
			return;
		}

		// Remove the previous registrations.
		for (auto registration : BackgroundTaskRegistration::AllTasks)
		{
			registration->Value->Unregister(true);
		}

		auto networkTaskBuilder = ref new BackgroundTaskBuilder();
		networkTaskBuilder->Name = "NetworkStateChangeTask";
		networkTaskBuilder->TaskEntryPoint = taskEntryPoint;
		networkTaskBuilder->SetTrigger(ref new SystemTrigger(SystemTriggerType::NetworkStateChange, false));
		networkTaskBuilder->Register();

		auto timerTaskBuilder = ref new BackgroundTaskBuilder();
		timerTaskBuilder->Name = "TimerTask";
		timerTaskBuilder->TaskEntryPoint = taskEntryPoint;
		timerTaskBuilder->SetTrigger(ref new TimeTrigger(30, false));
		timerTaskBuilder->Register();

		Debug("Background tasks registered.\n");
	}).then([](task<void> t)
	{
		try
		{
			t.get();
		}
		catch (Platform::Exception^ e)
		{
			OutputDebugString(e->Message->Data());
		}
	});
}
