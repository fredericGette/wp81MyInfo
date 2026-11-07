//
// UpdateTileTask.h
// Background task which updates the tile when the network changes (and periodically).
//

#pragma once

namespace Wp81MyInfoTasks
{
	public ref class UpdateTileTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
	{
	public:
		virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
	};
}
