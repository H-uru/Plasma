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
#include "plPluginClient.h"

#include "../../../Plasma/PubUtilLib/plPipeline/hsG3DDeviceSelector.h"

#include "hsThread.h"
#include "hsSTLStream.h"

#include "../pnKeyedObject/plUoid.h"
#include "../plResMgr/plUpdatableResManager.h"
#include "../plStatusLog/plStatusLog.h"

// Needed for IEnableProxies
#include "plPipeline.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../pnMessage/plProxyDrawMsg.h"
#include "plgDispatch.h"

#define LOG_SCENEVIWER

static plUpdatableResManager *GetResMgr()
{
	return (plUpdatableResManager*)hsgResMgr::ResMgr();
}

plUpdatableClient::plUpdatableClient() : fPipeName(nil), fUpdateSignal(nil), fDataPath(nil), fActive(true), fDirty(false)
{
#ifdef LOG_SCENEVIWER
	fLog = plStatusLogMgr::GetInstance().CreateStatusLog(25, "SceneViewer",
		plStatusLog::kDontWriteFile | plStatusLog::kFilledBackground | plStatusLog::kAlignToTop);
#endif // LOG_SCENEVIWER
}

plUpdatableClient::~plUpdatableClient()
{
	if (fUpdateSignal)
	{
		delete fUpdateSignal;
		fUpdateSignal = nil;
	}

	if (fLog)
	{
		delete fLog;
		fLog = nil;
	}
}

void plUpdatableClient::InitUpdate(const char *semaphoreName, const char *pipeName, const char *dir)
{
	fUpdateSignal = TRACKED_NEW hsSemaphore(0, semaphoreName);
	fPipeName = pipeName;
	fDataPath = dir;
}

hsG3DDeviceModeRecord plUpdatableClient::ILoadDevMode(const char* devModeFile)
{
	// Get the usual devmode
	hsG3DDeviceModeRecord dmr = plClient::ILoadDevMode(devModeFile);

	// Override the mode with a windowed one
	hsG3DDeviceMode *mode = (hsG3DDeviceMode*)dmr.GetMode();
	mode->SetColorDepth(0);

	return hsG3DDeviceModeRecord(*dmr.GetDevice(), *mode);
}

#include "../../../Plasma/Apps/plClient/plClientUpdateFormat.h"
#include "../pnKeyedObject/plKey.h"

void plUpdatableClient::IGetUpdate()
{
	// If the semaphore is signaled an update is ready
	if (fUpdateSignal && fUpdateSignal->Wait(0))
	{
		hsNamedPipeStream s;
		s.Open(fPipeName, "r");

#ifdef LOG_SCENEVIWER
		fLog->Clear();
		static int numUpdates = 0;
		numUpdates++;
		fLog->AddLineF(plStatusLog::kBlue, "SceneViewer Update #%d", numUpdates);
#endif // LOG_SCENEVIWER

		UInt8 type = s.ReadByte();

		if (type == ClientUpdate::kShutdown)
		{
			#ifdef LOG_SCENEVIWER
			fLog->AddLine("Client shutdown");
			#endif // LOG_SCENEVIWER

			PostMessage(GetWindowHandle(), WM_SYSCOMMAND, SC_CLOSE, 0);
		}
		else if (type == ClientUpdate::kUpdate)
		{
			fDirty = true;
			
			IEnableProxies(false);

			int i;

			//
			// Delete the deleted keys
			//
			int numDeleted = s.ReadSwap32();
			std::vector<plKey*> delKeys;
			delKeys.reserve(numDeleted);

			for (i = 0; i < numDeleted; i++)
			{
				plUoid uoid;
				uoid.Read(&s);
				plKey *key = hsgResMgr::ResMgr()->FindKey(uoid);
				hsAssert(key, "Key to delete not found");
				if (key)
				{
					#ifdef LOG_SCENEVIWER
					fLog->AddLineF("Remove: %s", key->GetName());
					#endif // LOG_SCENEVIWER

					GetResMgr()->RemoveObject(key, false);
					delKeys.push_back(key);
				}
			}

			GetResMgr()->DelayLoad(true);

			//
			// Read in the changed spans
			//
			hsStatusMessage("ReadChangedSpans\n");
			GetResMgr()->ReadChangedSpans(&s);

			//
			// Read in the new keys and objects
			//
			int numNew = s.ReadSwap32();
			for (i = 0; i < numNew; i++)
			{
				plCreatable *cre = GetResMgr()->ReadCreatable(&s);

				hsKeyedObject *ko = hsKeyedObject::ConvertNoRef(cre);

				#ifdef LOG_SCENEVIWER
				if (ko)
					fLog->AddLineF("Read: %s", ko->GetKey()->GetName());
				else
					fLog->AddLine("Read: (null)");
				#endif // LOG_SCENEVIWER
			}

			GetResMgr()->DelayLoad(false);

			// Clear out any objects that were never reloaded (really deleted)
			for (i = 0; i < delKeys.size(); i++)
			{
				plKey *key = delKeys[i];
				if (!key->ObjectIsLoaded())
				{
					#ifdef LOG_SCENEVIWER
					fLog->AddLineF("Key deleted: %s", key->GetName());
					#endif // LOG_SCENEVIWER

					GetResMgr()->RemoveObject(key);
				}
			}

			IEnableProxies(true);
		}

		s.Close();
	}
}

hsBool plUpdatableClient::Init()
{
	if (plClient::Init())
	{
		GetResMgr()->ForceLoadDirectory(fDataPath, true);
		// Page in the SceneViewer now that our key is ready
		GetResMgr()->PageInSceneViewer();
		return true;
	}

	return false;
}

hsBool plUpdatableClient::MainLoop()
{
	IGetUpdate();

	if (fActive)
		return plClient::MainLoop();
	else
	{
		Sleep(100);
		return true;
	}
}

#include <direct.h>

hsBool plUpdatableClient::Shutdown()
{
	if (fDirty && fDataPath)
	{
		char oldCwd[MAX_PATH];
		getcwd(oldCwd, sizeof(oldCwd));

		// Even bigger hack
		char tempCrap[MAX_PATH];
		strcpy(tempCrap, fDataPath);
		tempCrap[strlen(tempCrap)-strlen("dat\\")] = '\0';
		chdir(tempCrap);

		GetResMgr()->WriteSceneViewer();

		chdir(oldCwd);
	}

	return plClient::Shutdown();
}

void plUpdatableClient::IEnableProxies(bool enable)
{
	if (enable)
	{
		// switch back on any drawable proxies
		if (fPipeline->GetDrawableTypeMask() & plDrawableSpans::kAudibleProxy)
		{	
			plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kAudible | plProxyDrawMsg::kCreate);
			plgDispatch::MsgSend(msg);
		}
		if (fPipeline->GetDrawableTypeMask() & plDrawableSpans::kOccluderProxy)
		{	
			plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kOccluder | plProxyDrawMsg::kCreate);
			plgDispatch::MsgSend(msg);
		}
		if (fPipeline->GetDrawableTypeMask() & plDrawableSpans::kPhysicalProxy)
		{	
			plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kPhysical | plProxyDrawMsg::kCreate);
			plgDispatch::MsgSend(msg);
		}
		if (fPipeline->GetDrawableTypeMask() & plDrawableSpans::kLightProxy)
		{	
			plProxyDrawMsg* msg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kLight | plProxyDrawMsg::kCreate);
			plgDispatch::MsgSend(msg);
		}
	}
	else
	{
		// notify any and all drawable proxies to stop drawing...
		plProxyDrawMsg* nuke = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kAudible
												| plProxyDrawMsg::kOccluder
												| plProxyDrawMsg::kPhysical
												| plProxyDrawMsg::kLight
												| plProxyDrawMsg::kDestroy);
		plgDispatch::MsgSend(nuke);
	}
}

void plUpdatableClient::WindowActivate(bool active)
{
	fActive = active;
}
