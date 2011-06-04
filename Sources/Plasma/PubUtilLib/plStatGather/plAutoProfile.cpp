/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "plAutoProfile.h"

#include "plgDispatch.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"

#include "hsStream.h"
#include "hsTimer.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "../plMessage/plTimerCallbackMsg.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plModifier/plSpawnModifier.h"
#include "../plMessage/plConsoleMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../plAgeLoader/plAgeLoader.h"
#include "plProfileManagerFull.h"
#include "../plFile/plFileUtils.h"

#include "../plPipeline/plDebugText.h"
#include "../pnMessage/plTimeMsg.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plVault/plVault.h"

#include "../plContainer/plConfigInfo.h" // for plStringList

// For taking screenshots
#include "../plGImage/plMipmap.h"
#include "../Apps/plClient/plClient.h"
#include "../plJPEG/plJPEG.h"
#include "plPipeline.h"

#include <algorithm>

class plAutoProfileImp : public plAutoProfile
{
protected:
	plStringList fAges;
	int fNextAge;
	int fNextSpawnPoint;
	const char* fLastSpawnPointName;
	// For profiling a single age
	std::string fAgeName;
	bool fLinkedToSingleAge;
	bool fJustLinkToAges;

	UInt64 fLinkTime;

	std::string fStatusMessage;

	void INextProfile();
	bool INextAge();
	bool INextSpawnPoint();

	void IInit();
	void IShutdown();

public:
	plAutoProfileImp();

	virtual void StartProfile(const char* ageName);
	virtual void LinkToAllAges();

	virtual hsBool MsgReceive(plMessage* msg);
};

plAutoProfile* plAutoProfile::Instance()
{
	static plAutoProfileImp theInstance;
	return &theInstance;
}

////////////////////////////////////////////////////////////////////////////////

plAutoProfileImp::plAutoProfileImp() : fNextAge(0), fNextSpawnPoint(0), fLastSpawnPointName(nil), fLinkedToSingleAge(false), fJustLinkToAges(false)
{
}

void plAutoProfileImp::StartProfile(const char* ageName)
{
	if (ageName)
		fAgeName = ageName;
	else
		fAgeName = "";

	IInit();

	plProfileManagerFull::Instance().ActivateAllStats();
}

void plAutoProfileImp::LinkToAllAges()
{
	fJustLinkToAges = true;
	IInit();
}

void plAutoProfileImp::IInit()
{
	// TODO: Find a better way to grab a list of age names, since the old data server
	// no longer exists
	/*plIDataServer* dataServer = plNetClientMgr::GetInstance()->GetDataServer();
	if (!dataServer)
		return;

	dataServer->GetDatasetAges(fAges);*/

	// The first age we link into is AvatarCustomization, since we have a new avatar.
	// Coincidentally, the first age in our list is AvatarCustomization.  However,
	// sometimes linking from ACA to ACA causes a crash.  To get around that, we just
	// reverse the list, so it's last.
	std::reverse(fAges.begin(), fAges.end());

	fNextAge = 0;

	RegisterAs(kAutoProfile_KEY);

	plgDispatch::Dispatch()->RegisterForExactType(plAgeBeginLoadingMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());

	plConsoleMsg* consoleMsg = TRACKED_NEW plConsoleMsg(plConsoleMsg::kExecuteLine, "Camera.AlwaysCut true");
	consoleMsg->Send();
}

#ifdef HS_BUILD_FOR_WIN32
#include "hsWindows.h"
#include <shellapi.h>
#endif

void plAutoProfileImp::IShutdown()
{
	// KLUDGE - Copy the load timing log, in case we used it
	#define kTimingLog L"readtimings.0.log"
	#define kAgeTimingLog L"agetimings.0.log"
	wchar destPath[MAX_PATH];
	wchar sourcePath[MAX_PATH];

	PathAddFilename(destPath, plProfileManagerFull::Instance().GetProfilePath(), kTimingLog, arrsize(destPath));
	PathGetLogDirectory(sourcePath, arrsize(sourcePath));
	PathAddFilename(sourcePath, sourcePath, kTimingLog, arrsize(sourcePath));
	plFileUtils::FileCopy(sourcePath, destPath);

	PathAddFilename(destPath, plProfileManagerFull::Instance().GetProfilePath(), kAgeTimingLog, arrsize(destPath));
	PathGetLogDirectory(sourcePath, arrsize(sourcePath));
	PathAddFilename(sourcePath, sourcePath, kAgeTimingLog, arrsize(sourcePath));
	plFileUtils::FileCopy(sourcePath, destPath);

#ifdef HS_BUILD_FOR_WIN32
	ShellExecute(nil, nil, "PostRun.bat", nil, nil, SW_SHOWNORMAL);
#endif

	plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
	plgDispatch::Dispatch()->UnRegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->UnRegisterForExactType(plAgeLoadedMsg::Index(), GetKey());

	UnRegisterAs(kAutoProfile_KEY);
	// Pump the queue so we get fully unregistered
	plgDispatch::Dispatch()->MsgQueueProcess();

	plClientMsg* clientMsg = TRACKED_NEW plClientMsg(plClientMsg::kQuit);
	clientMsg->Send(hsgResMgr::ResMgr()->FindKey(kClient_KEY));
}

void plAutoProfileImp::INextProfile()
{
	// Haven't linked to our first age yet, do that before we start profiling
	if (fNextAge == 0)
	{
		if (!INextAge())
			IShutdown();
	}
	else
	{
		// Log the stats for this spawn point
		if (fLastSpawnPointName)
		{
			const char * ageName = NetCommGetAge()->ageDatasetName;
			plProfileManagerFull::Instance().LogStats(ageName, fLastSpawnPointName);

			plMipmap mipmap;
			if (plClient::GetInstance()->GetPipeline()->CaptureScreen(&mipmap))
			{
				char fileName[256];
				sprintf(fileName, "%s%s_%s.jpg",
					plProfileManagerFull::Instance().GetProfilePath(),
					ageName, fLastSpawnPointName);

				plJPEG::Instance().SetWriteQuality(100);
				plJPEG::Instance().WriteToFile(fileName, &mipmap);
			}

			fLastSpawnPointName = nil;
		}

		// Try to go to the next spawn point
		if (!INextSpawnPoint())
		{
			// Link to the next age
			if (!INextAge())
			{
				// We've done all the ages, shut down
				IShutdown();
			}
		}
	}
}

bool plAutoProfileImp::INextAge()
{
	const char* ageName = nil;

	if (fAgeName.length() > 0)
	{
		if (fLinkedToSingleAge)
			return false;

		fLinkedToSingleAge = true;
		ageName = fAgeName.c_str();
	}
	else
	{
		if (fNextAge >= fAges.size())
			return false;

		ageName = fAges[fNextAge].c_str();
	}

	fNextAge++;
	fNextSpawnPoint = 0;

	plAgeLinkStruct link;
	link.GetAgeInfo()->SetAgeFilename(ageName);
	link.SetLinkingRules(plNetCommon::LinkingRules::kBasicLink);
	plNetLinkingMgr::GetInstance()->LinkToAge(&link);

	fStatusMessage = "Linking to age ";
	fStatusMessage += ageName;

	return true;
}

bool plAutoProfileImp::INextSpawnPoint()
{
	if (fJustLinkToAges)
		return false;

	const char* kPerfSpawnPrefix = "cPerf-";
	int kPerfSpawnLen = strlen(kPerfSpawnPrefix);

	// Find the next perf spawn point
	bool foundGood = false;
	while (fNextSpawnPoint < plAvatarMgr::GetInstance()->NumSpawnPoints())
	{
		const plSpawnModifier* spawnMod = plAvatarMgr::GetInstance()->GetSpawnPoint(fNextSpawnPoint);
		fLastSpawnPointName = spawnMod->GetKeyName();

		if (strncmp(fLastSpawnPointName, kPerfSpawnPrefix, kPerfSpawnLen) == 0)
		{
			fStatusMessage = "Profiling spawn point ";
			fStatusMessage += fLastSpawnPointName;

			foundGood = true;
			break;
		}
		else
			fNextSpawnPoint++;
	}

	if (!foundGood)
	{
		fLastSpawnPointName = nil;
		fStatusMessage = "No profile spawn point found";
		return false;
	}

	plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
	if (avatar)
	{
		double fakeTime = 0.0f;
		avatar->SpawnAt(fNextSpawnPoint, fakeTime);
	}

	fNextSpawnPoint++;

	plTimerCallbackMsg* timerMsg = TRACKED_NEW plTimerCallbackMsg(GetKey());
	plgTimerCallbackMgr::NewTimer(30, timerMsg);

	return true;
}

hsBool plAutoProfileImp::MsgReceive(plMessage* msg)
{
	plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
	if (evalMsg)
	{
		if (fStatusMessage.length() > 0)
			plDebugText::Instance().DrawString(10, 10, fStatusMessage.c_str());
	}

	plAgeLoadedMsg* ageLoaded = plAgeLoadedMsg::ConvertNoRef(msg);
	if (ageLoaded)
	{
		if (!ageLoaded->fLoaded)
		{
			fLinkTime = hsTimer::GetFullTickCount();
			hsStatusMessage("Age unloaded");
		}
		return true;
	}

	plInitialAgeStateLoadedMsg* ageStateLoaded = plInitialAgeStateLoadedMsg::ConvertNoRef(msg);
	if (ageStateLoaded)
	{
		if (fNextAge > 0)
		{
			fLinkTime = hsTimer::GetFullTickCount() - fLinkTime;
			float ms = hsTimer::FullTicksToMs(fLinkTime);

			hsStatusMessageF("Age %s finished load, took %.1f ms",
				fAges[fNextAge-1].c_str(),
				ms);

			plStatusLog::AddLineS("agetimings.log", "Age %s took %.1f ms",
				fAges[fNextAge-1].c_str(),
				ms);
		}

		fStatusMessage = "Age loaded.  Preparing to profile.";

		// Age is loaded, start profiling in 5 seconds (to make sure the avatar is linked in all the way)
		plTimerCallbackMsg* timerMsg = TRACKED_NEW plTimerCallbackMsg(GetKey());
		plgTimerCallbackMgr::NewTimer(5, timerMsg);
		return true;
	}

	plTimerCallbackMsg* timerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
	if (timerMsg)
	{
		INextProfile();
		return true;
	}

	// When the first age starts to load, register the stupid avatar customization variable
	// so the calibration screen won't pop up and ruin everything.  I'm sure I could try
	// and do this earlier, but I'm not sure when that player folder is set so screw it, it works here.
	plAgeBeginLoadingMsg* ageBeginLoadingMsg = plAgeBeginLoadingMsg::ConvertNoRef(msg);
	if (ageBeginLoadingMsg)
	{
		plgDispatch::Dispatch()->UnRegisterForExactType(plAgeBeginLoadingMsg::Index(), GetKey());
		VaultAddChronicleEntryAndWait(L"InitialAvCursomizationsDone", 0, L"1");
		return true;
	}

	return false;
}
