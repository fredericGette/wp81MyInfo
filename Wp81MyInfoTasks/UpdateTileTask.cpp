//
// UpdateTileTask.cpp
// Implementation of the UpdateTileTask class.
//

#include "pch.h"
#include "UpdateTileTask.h"
#include "InfoTile.h"

using namespace Wp81MyInfoTasks;

using namespace Windows::ApplicationModel::Background;

void UpdateTileTask::Run(IBackgroundTaskInstance^ taskInstance)
{
	(void) taskInstance;	// Unused parameter

	// Synchronous work: no deferral needed.
	InfoTile::Update();
}
