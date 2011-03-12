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
#pragma warning(disable: 4284)
#include "HeadSpin.h"
#include "hsTypes.h"
#include "hsWindowHndl.h"
#include "plClient.h"
#include "hsStream.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plKeyFinder.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plScene/plSceneNode.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../pfCamera/plVirtualCamNeu.h"
#include "hsTimer.h"
#include "../plPipeline/hsG3DDeviceSelector.h"
#include "../plFile/plEncryptedStream.h"
#include "../plFile/plFileUtils.h"
#include "../plInputCore/plInputManager.h"
#include "../plInputCore/plInputInterfaceMgr.h"
#include "../plInputCore/plInputDevice.h"
#include "../plPhysX/plSimulationMgr.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plScene/plRelevanceMgr.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "../pfAudio/plListener.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../plMessage/plRoomLoadNotifyMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../plMessage/plTransitionMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plPreloaderMsg.h"
#include "../plMessage/plNetCommMsgs.h"
#include "../plMessage/plAgeLoadedMsg.h"

#include "../pfConsole/pfConsoleEngine.h"
#include "../pfConsole/pfConsole.h"
#include "../pfConsole/pfConsoleDirSrc.h"
#include "../plScene/plPageTreeMgr.h"
#include "../plScene/plVisMgr.h"
#include "../plFile/hsFiles.h"

#include "../pfKI/pfKI.h"

#include "../plAudio/plAudioSystem.h"
#include "../plAudio/plAudioCaps.h"

#include "../plStatGather/plProfileManagerFull.h"

#include "plPipeline.h"
#include "../plPipeline/plPipelineCreate.h"
#include "../plPipeline/plPipeDebugFlags.h"
#include "../plPipeline/plTransitionMgr.h"
#include "../plPipeline/plCaptureRender.h"
#include "../plPipeline/plDynamicEnvMap.h"
#include "../plNetClient/plLinkEffectsMgr.h"
#include "../plAvatar/plAvatarClothing.h"
#include "../plAvatar/plArmatureMod.h"
#include "../pnMessage/plProxyDrawMsg.h"

#include "../plScene/plRenderRequest.h"
#include "../plDrawable/plAccessGeometry.h"
#include "plPipeResReq.h"
#include "../plDrawable/plVisLOSMgr.h"

#include "../plGImage/plBitmap.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plProgressMgr/plProgressMgr.h"
#include "../plPipeline/plDTProgressMgr.h"
#include "../plPipeline/plBinkPlayer.h"
#include "../plMessage/plMovieMsg.h"

#include "../plSDL/plSDL.h"

#include "../pnDispatch/plDispatch.h"
#include "../pnDispatch/plDispatchLogBase.h"
#include "../pfGameGUIMgr/pfGameGUIMgr.h"
#include "../pfPython/cyMisc.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plRenderRequestMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "../plModifier/plSimpleModifier.h"
#include "plAudible.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../pnMessage/plSoundMsg.h"
#include "../pnMessage/plAudioSysMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../plAgeLoader/plResPatcher.h"
#include "../pfPython/cyPythonInterface.h"
#include "../plUnifiedTime/plClientUnifiedTime.h"
#include "../pfAnimation/plAnimDebugList.h"
#include "../pfGameGUIMgr/pfGUICtrlGenerator.h"

#include "../plGImage/plWinFontCache.h"
#include "../plGImage/plFontCache.h"

#include "../pfJournalBook/pfJournalBook.h"

#include "../plAvatar/plAGAnimInstance.h"
#include "../plAgeLoader/plAgeLoader.h"
#include "../plClientKey/plClientKey.h"

#include "../CoreLib/plQuality.h"
#include "../plGLight/plShadowCaster.h"

#include "../plNetClient/plNetLinkingMgr.h"
#include "../plNetCommon/plNetCommonConstants.h"
#include "../plNetGameLib/plNetGameLib.h"

#include "../pfSecurePreloader/pfSecurePreloader.h"
#include "../pfLocalizationMgr/pfLocalizationMgr.h"

#include "../pfCsrSrv/pfCsrSrv.h"

#include "plTweak.h"

#define MSG_LOADING_BAR

// static hsVector3 gAbsDown(0,0,-hsScalar1);

static plDispatchBase* gDisp = nil;
static plTimerCallbackManager* gTimerMgr = nil;
static plAudioSystem* gAudio = nil;

hsBool plClient::fDelayMS = false;

plClient* plClient::fInstance=nil;

static hsTArray<HMODULE>		fLoadedDLLs;

plClient::plClient()
: fPipeline(nil),
	fDone(false),
	fQuitIntro(false),
	fWindowHndl(nil),
	fInputManager(nil),
	fConsole(nil),
	fCurrentNode(nil),
	fNewCamera(nil),
	fpAuxInitDir(nil),
	fTransitionMgr(nil),
	fLinkEffectsMgr(nil),
	fProgressBar(nil),
	fGameGUIMgr(nil),
	fKIGUIGlue(nil),
	fWindowActive(false),
	fAnimDebugList(nil),
	fClampCap(-1),
	fQuality(0),
	fPageMgr(nil),
	fFontCache(nil),
	fHoldLoadRequests(false),
	fNumLoadingRooms(0),
	fNumPostLoadMsgs(0),
	fPostLoadMsgInc(0.f),
	fPatchGlobalAges(false)
{
#ifndef PLASMA_EXTERNAL_RELEASE
	bPythonDebugConnected = false;
#endif

	hsStatusMessage("Constructing client\n");
	plClient::SetInstance(this);
	// gNextRoom[0] = '\0';

	// Setup the timer. These can be overriden with console commands.
	hsTimer::SetRealTime(true);
#ifdef HS_DEBUGGING
//	hsTimer::SetRealTime(false);
	hsTimer::SetTimeClamp(0.1f);
#else // HS_DEBUGGING
//	hsTimer::SetRealTime(true);
	hsTimer::SetTimeClamp(0);
#endif // HS_DEBUGGING
	
	IDetectAudioVideoSettings();		// need to do this before the console is created

	/// allow console commands to start working early
	// Create the console engine
	fConsoleEngine = TRACKED_NEW pfConsoleEngine();
	
	// create network mgr before console runs
	plNetClientMgr::SetInstance(TRACKED_NEW plNetClientMgr);
	plAgeLoader::SetInstance(TRACKED_NEW plAgeLoader);

	// Use it to parse the init directory
	wchar initFolder[MAX_PATH];
	PathGetInitDirectory(initFolder, arrsize(initFolder));
	pfConsoleDirSrc		dirSrc( fConsoleEngine, initFolder, L"*.ini" );
	
#ifndef PLASMA_EXTERNAL_RELEASE
	// internal builds also parse the local init folder
	dirSrc.ParseDirectory( L"init", L"*.ini" );
#endif

	/// End of console stuff
}

plClient::~plClient()
{
	hsStatusMessage("Destructing client\n");

	plClient::SetInstance( nil );

	delete fPageMgr;
	delete [] fpAuxInitDir;
}

#include "../plGImage/plAVIWriter.h"
#include "../pfCharacter/pfMarkerMgr.h"

hsBool plClient::Shutdown()
{
	plSynchEnabler ps(false);	// disable dirty state tracking during shutdown	
	delete fProgressBar;

	CsrSrvShutdown();

	// Just in case, clear this out (trying to fix a crash bug where this is still active at shutdown)
	plDispatch::SetMsgRecieveCallback(nil);

	// Let the resmanager know we're going to be shutting down.
	hsgResMgr::ResMgr()->BeginShutdown();

	// Must kill off all movies before shutting down audio.
	IKillMovies();

	plgAudioSys::Activate(false);
	plBinkPlayer::DeInit();
	//
	// Get any proxies to commit suicide.
	plProxyDrawMsg* nuke = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kAllTypes
											| plProxyDrawMsg::kDestroy);
	plgDispatch::MsgSend(nuke);

	if (plAVIWriter::IsInitialized())
		plAVIWriter::Instance().Shutdown();

	hsStatusMessage( "Shutting down client...\n" );

	// First, before anybody else goes away, write out our key mappings
	if( plInputInterfaceMgr::GetInstance() )
		plInputInterfaceMgr::GetInstance()->WriteKeyMap();

	// tell Python that its ok to shutdown
	PythonInterface::WeAreInShutdown();	

	// Shutdown the journalBook API
	pfJournalBook::SingletonShutdown();

	/// Take down the KI
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	if( mgr )
		mgr->UnloadDialog( "KIBlackBar" );	// unload the blackbar which will bootstrap in the rest of the KI dialogs

	// Take down our GUI control generator
	pfGUICtrlGenerator::Instance().Shutdown();

	if (plNetClientMgr::GetInstance())
	{	
		plNetClientMgr::GetInstance()->Shutdown();
		plNetClientMgr::GetInstance()->UnRegisterAs(kNetClientMgr_KEY);		// deletes NetClientMgr instance
		plNetClientMgr::SetInstance(nil);
	}
	
	if (plAgeLoader::GetInstance())
	{	
		plAgeLoader::GetInstance()->Shutdown();
		plAgeLoader::GetInstance()->UnRegisterAs(kAgeLoader_KEY);			// deletes instance
		plAgeLoader::SetInstance(nil);
	}
	
	if (pfSecurePreloader::IsInstanced())
	{
		pfSecurePreloader::GetInstance()->Shutdown();
		// pfSecurePreloader handles its own fixed key unregistration
	}

	if (fInputManager)
	{
		fInputManager->UnRegisterAs(kInput_KEY);
		fInputManager = nil;
	}

	if( fGameGUIMgr != nil )
	{
		fGameGUIMgr->UnRegisterAs( kGameGUIMgr_KEY );
		fGameGUIMgr = nil;
	}

	for (int i = 0; i < fRooms.Count(); i++)
	{
		plSceneNode *sn = fRooms[i].fNode;
		GetKey()->Release(sn->GetKey());
	}
	fRooms.Reset();
	fRoomsLoading.clear();

	// Shutdown plNetClientMgr

	plAccessGeometry::DeInit();

	delete fPipeline;
	fPipeline = nil;

	if (plSimulationMgr::GetInstance())
		plSimulationMgr::Shutdown();
	plAvatarMgr::ShutDown();
	plRelevanceMgr::DeInit();
	
	if (fPageMgr)
		fPageMgr->Reset();

	if( fKIGUIGlue != nil )
	{
		fKIGUIGlue->UnRegisterAs( kKIGUIGlue_KEY );
		fKIGUIGlue = nil;
	}

	if( fTransitionMgr != nil )
	{
		fTransitionMgr->UnRegisterAs( kTransitionMgr_KEY );
		fTransitionMgr = nil;
	}
	
	delete fConsoleEngine;
	fConsoleEngine = nil;

	if (fLinkEffectsMgr)
	{
		fLinkEffectsMgr->UnRegisterAs( kLinkEffectsMgr_KEY);
		fLinkEffectsMgr=nil;
	}

	plClothingMgr::DeInit();

	if( fFontCache != nil )
	{
		fFontCache->UnRegisterAs( kFontCache_KEY );
		fFontCache = nil;
	}

	pfMarkerMgr::Shutdown();

	delete fAnimDebugList;

//#ifndef PLASMA_EXTERNAL_RELEASE
	if( fConsole != nil )
	{
		// UnRegisterAs destroys the object for us
		fConsole->UnRegisterAs( kConsoleObject_KEY );
		fConsole = nil;
	}
//#endif

	PythonInterface::finiPython();
	
	if (fNewCamera)
		fNewCamera->UnRegisterAs( kVirtualCamera1_KEY );

	// mark the listener for death.
	// there's no need to keep this around...
	plUoid lu(kListenerMod_KEY);
	plKey pLKey = hsgResMgr::ResMgr()->FindKey(lu);
	if (pLKey)
	{	
		plListener* pLMod = plListener::ConvertNoRef(pLKey->GetObjectPtr());
		if (pLMod)
			pLMod->UnRegisterAs(kListenerMod_KEY);
	}

	plgAudioSys::Shutdown();

	if (pfLocalizationMgr::InstanceValid())
		pfLocalizationMgr::Shutdown();
	
	ShutdownDLLs();

	plVisLOSMgr::DeInit();

	delete fPageMgr;
	fPageMgr = nil;
	plGlobalVisMgr::DeInit();

#ifdef TRACK_AG_ALLOCS
	DumpAGAllocs();
#endif // TRACK_AG_ALLOCS

	// This will destruct the client. Do it last.
	UnRegisterAs(kClient_KEY);
	

	return false;
}

void plClient::InitAuxInits()
{
	// Use another init directory specified in Command line Arg -i
	if (fpAuxInitDir)
		pfConsoleDirSrc		dirSrc( fConsoleEngine, fpAuxInitDir, "*.ini" );
}

void plClient::InitInputs()
{
	hsStatusMessage("InitInputs client\n");
	fInputManager = TRACKED_NEW plInputManager( fWindowHndl );
	fInputManager->CreateInterfaceMod(fPipeline);
	fInputManager->RegisterAs( kInput_KEY );
	plgDispatch::Dispatch()->RegisterForExactType(plIMouseXEventMsg::Index(), fInputManager->GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plIMouseYEventMsg::Index(), fInputManager->GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plIMouseBEventMsg::Index(), fInputManager->GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), fInputManager->GetKey());
	plInputDevice* pKeyboard = TRACKED_NEW plKeyboardDevice();
	fInputManager->AddInputDevice(pKeyboard);
	
	plInputDevice* pMouse = TRACKED_NEW plMouseDevice();
	fInputManager->AddInputDevice(pMouse);

	if( fWindowActive )
		fInputManager->Activate( true );
}

void plClient::ISetGraphicsDefaults()
{
	// couldn't find display mode set defaults write to ini file
	wchar graphicsIniFile[MAX_PATH];
	PathGetInitDirectory(graphicsIniFile, arrsize(graphicsIniFile));
	PathAddFilename(graphicsIniFile, graphicsIniFile, L"graphics.ini", arrsize(graphicsIniFile));
	IWriteDefaultGraphicsSettings(graphicsIniFile);
	plPipeline::fInitialPipeParams.Windowed = plPipeline::fDefaultPipeParams.Windowed;
	plPipeline::fInitialPipeParams.AntiAliasingAmount = plPipeline::fDefaultPipeParams.AntiAliasingAmount;
	plPipeline::fInitialPipeParams.AnisotropicLevel = plPipeline::fDefaultPipeParams.AnisotropicLevel;
	plPipeline::fInitialPipeParams.TextureQuality = plPipeline::fDefaultPipeParams.TextureQuality;
	plPipeline::fInitialPipeParams.VSync = plPipeline::fDefaultPipeParams.VSync;
	plShadowCaster::EnableShadowCast(plPipeline::fDefaultPipeParams.Shadows ? true : false);
	plQuality::SetQuality(plPipeline::fDefaultPipeParams.VideoQuality);
	if( (fClampCap >= 0) && (fClampCap < plQuality::GetCapability()) )
		plQuality::SetCapability(fClampCap);
	plDynamicCamMap::SetEnabled(plPipeline::fDefaultPipeParams.PlanarReflections ? true : false);
}

hsBool plClient::InitPipeline()
{
	hsStatusMessage("InitPipeline client\n");
	HWND hWnd = fWindowHndl;
	
	hsG3DDeviceModeRecord dmr;
	hsG3DDeviceSelector devSel;
	devSel.Enumerate(hWnd);
	devSel.RemoveUnusableDevModes(true);

	if (!devSel.GetDefault(&dmr))
	{
		hsMessageBox("No suitable rendering devices found.","Plasma", hsMessageBoxNormal, hsMessageBoxIconError);
		return true;
	}

	hsG3DDeviceRecord *rec = (hsG3DDeviceRecord *)dmr.GetDevice();
	int res = -1;

	if(!plPipeline::fInitialPipeParams.Windowed)
	{
		// find our resolution if we're not in windowed mode
		for ( int i = 0; i < rec->GetModes().GetCount(); i++ )
		{
			hsG3DDeviceMode *mode = rec->GetMode(i);
			if ((mode->GetWidth() == plPipeline::fInitialPipeParams.Width) &&
				(mode->GetHeight() == plPipeline::fInitialPipeParams.Height) &&
				(mode->GetColorDepth() == plPipeline::fInitialPipeParams.ColorDepth))
			{
				res = i;
				break;
			}
		}
		if(res != -1)
		{
			// found it set it as the current mode.
			dmr = hsG3DDeviceModeRecord(*rec, *rec->GetMode(res));
		}
		else
		{
			ISetGraphicsDefaults();
		}
	}

	if(plPipeline::fInitialPipeParams.TextureQuality == -1)
	{
		plPipeline::fInitialPipeParams.TextureQuality = dmr.GetDevice()->GetCap(hsG3DDeviceSelector::kCapsPixelShader) ? 2 : 1;
	}
	else
	{
		// clamp value to range
		if(plPipeline::fInitialPipeParams.TextureQuality > 2) plPipeline::fInitialPipeParams.TextureQuality = 2;
		if(plPipeline::fInitialPipeParams.TextureQuality < 0) plPipeline::fInitialPipeParams.TextureQuality = 0;
		plBitmap::SetGlobalLevelChopCount(2 - plPipeline::fInitialPipeParams.TextureQuality);
	}

	plPipeline *pipe = plPipelineCreate::CreatePipeline( hWnd, &dmr );
	if( pipe->GetErrorString() != nil )
	{
		ISetGraphicsDefaults();
#ifdef PLASMA_EXTERNAL_RELEASE
		hsMessageBox("There was an error initializing the video card.\nSetting defaults.", "Error", hsMessageBoxNormal);
#else
		hsMessageBox( pipe->GetErrorString(), "Error creating pipeline", hsMessageBoxNormal );
#endif
		delete pipe;
		devSel.GetDefault(&dmr);
		pipe = plPipelineCreate::CreatePipeline( hWnd, &dmr );
		if(pipe->GetErrorString() != nil)
		{
			// not much else we can do
			return true;
		}
	}
	fPipeline = pipe;

	hsVector3 up;
	hsPoint3 from, at;
	from.Set(0, 0, 10.f);
	at.Set(0, 20.f, 10.f);
	up.Set(0,0,-1.f);
	hsMatrix44 cam;
	cam.MakeCamera(&from,&at,&up);

	float	yon = 500.0f;

	pipe->SetFOV( 60.f, hsIntToScalar( 60.f * pipe->Height() / pipe->Width() ) );
	pipe->SetDepth( 0.3f, yon );

	hsMatrix44 id;
	id.Reset();

	pipe->SetWorldToCamera( cam, id );
	pipe->RefreshMatrices();

	// Do this so we're still black before we show progress bars, but the correct color coming out of 'em
	fClearColor.Set( 0.f, 0.f, 0.f, 1.f );
	pipe->SetClear(&fClearColor);
	pipe->ClearRenderTarget();

	plAccessGeometry::Init(pipe);

	if( fPipeline )
		fPipeline->LoadResources();

	return false;
}

//============================================================================
void	plClient::SetClearColor( hsColorRGBA &color )
{
	fClearColor = color;
	if( fPipeline != nil )
	{
		fPipeline->SetClear(&fClearColor, nil);
	}
}

//============================================================================
void plClient::IDispatchMsgReceiveCallback()
{
	if (fInstance->fProgressBar)
		fInstance->fProgressBar->Increment(1);

	static char buf[30];
	sprintf(buf, "Msg %d", fInstance->fNumPostLoadMsgs);
	fInstance->IIncProgress(fInstance->fPostLoadMsgInc, buf);

	fInstance->fNumPostLoadMsgs++;
}


//============================================================================
hsBool plClient::MsgReceive(plMessage* msg)
{
	if (plGenRefMsg * genRefMsg = plGenRefMsg::ConvertNoRef(msg)) {
		// do nothing, we just use the client's key to ref vault image nodes.
		return true;
	}
	
	plClientRefMsg* pRefMsg = plClientRefMsg::ConvertNoRef(msg);
	if (pRefMsg)
	{
		switch(pRefMsg->fType)
		{
		case plClientRefMsg::kLoadRoom :
			#ifndef PLASMA_EXTERNAL_RELEASE
			plStatusLog::AddLineS( "pageouts.log", ".. ClientRefMsg received for room %s", pRefMsg->GetRef() != nil ? pRefMsg->GetRef()->GetKey()->GetUoid().GetObjectName() : "nilref" );
			#endif

			// was it that the room was loaded?
			if (hsCheckBits(pRefMsg->GetContext(), plRefMsg::kOnCreate))
				IRoomLoaded(plSceneNode::Convert(pRefMsg->GetRef()), false);
			// or was it that the room was unloaded?
			else if (hsCheckBits(pRefMsg->GetContext(), plRefMsg::kOnDestroy))
				IRoomUnloaded(plSceneNode::Convert(pRefMsg->GetRef()));
			#ifndef PLASMA_EXTERNAL_RELEASE
			else
				plStatusLog::AddLineS("pageouts.log", "..    refMsg is UNHANDLED");
			#endif
			break;

		case plClientRefMsg::kLoadRoomHold:
			if (hsCheckBits(pRefMsg->GetContext(), plRefMsg::kOnCreate))
				IRoomLoaded(plSceneNode::Convert(pRefMsg->GetRef()), true);
			break;

			//
			// Manually add room.
			// Add to pageMgr, but don't load the entire room.
			//
		case plClientRefMsg::kManualRoom:
			{
				if (pRefMsg->GetContext() & plRefMsg::kOnCreate ||
					pRefMsg->GetContext() & plRefMsg::kOnRequest)
				{
					hsBool found=false;
					plSceneNode *pNode = plSceneNode::ConvertNoRef(pRefMsg->GetRef()); 
					int i;
					for (i = 0; i < fRooms.Count(); i++)
					{
						if (fRooms[i].fNode->GetKey() == pRefMsg->GetSender())
						{
							found=true;
							break;
						}
					}
					if (!found)
					{					
						if (pNode)
						{
							fRooms.Append( plRoomRec( pNode, 0 ) );
							fPageMgr->AddNode(pNode);
						}
					}
				}
				else
				{
					plSceneNode* node = plSceneNode::ConvertNoRef(pRefMsg->GetRef());
					if(node)
					{
						int i;
						for (i = 0; i < fRooms.Count(); i++)
						{
							if (fRooms[i].fNode->GetKey() == node->GetKey())
							{
								fRooms.Remove(i);
								break;
							}
						}
						fPageMgr->RemoveNode(node);
					}
				}
			}
			break;
			
		}
	}

	plClientMsg* pMsg = plClientMsg::ConvertNoRef(msg);
	if (pMsg)
	{
		switch(pMsg->GetClientMsgFlag())
		{
		case plClientMsg::kQuit:
			SetDone(true);
			break;

		case plClientMsg::kLoadRoom:
		case plClientMsg::kLoadRoomHold:
			{
				IQueueRoomLoad(pMsg->GetRoomLocs(), (pMsg->GetClientMsgFlag() == plClientMsg::kLoadRoomHold));
				if (!fHoldLoadRequests)
					ILoadNextRoom();
			}
			break;

		case plClientMsg::kUnloadRoom:
			IUnloadRooms(pMsg->GetRoomLocs());
			break;

		case plClientMsg::kLoadNextRoom:
			ILoadNextRoom();
			break;

		// Load optimizations: messages to pre-load and un-load all the keys in a given age
		case plClientMsg::kLoadAgeKeys:
			{
				plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();
				mgr->LoadAgeKeys( pMsg->GetAgeName() );
			}
			break;

		case plClientMsg::kReleaseAgeKeys:
			{
				plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();
				mgr->DropAgeKeys( pMsg->GetAgeName() );
			}
			break;
			
		case plClientMsg::kDisableRenderScene:
			{
				plClient::GetInstance()->SetFlag( plClient::kFlagDBGDisableRender, true );
			}
			break;
		case plClientMsg::kEnableRenderScene:
			{
				plClient::GetInstance()->SetFlag( plClient::kFlagDBGDisableRender, false );
			}
			break;

		case plClientMsg::kResetGraphicsDevice:
			{
				ResetDisplayDevice(pMsg->fGraphicsSettings.fWidth, pMsg->fGraphicsSettings.fHeight, pMsg->fGraphicsSettings.fColorDepth, pMsg->fGraphicsSettings.fWindowed, pMsg->fGraphicsSettings.fNumAASamples, pMsg->fGraphicsSettings.fMaxAnisoSamples, pMsg->fGraphicsSettings.fVSync);
			}
			break;

		case plClientMsg::kSetGraphicsDefaults:
			{
				ISetGraphicsDefaults();
				ResetDisplayDevice(plPipeline::fDefaultPipeParams.Width, plPipeline::fDefaultPipeParams.Height, plPipeline::fDefaultPipeParams.ColorDepth, plPipeline::fDefaultPipeParams.Windowed,
					plPipeline::fDefaultPipeParams.AntiAliasingAmount, plPipeline::fDefaultPipeParams.AnisotropicLevel, plPipeline::fDefaultPipeParams.VSync, true);
			}
			break;

		}
		return true;
	}
	plRenderRequestMsg* rendReq = plRenderRequestMsg::ConvertNoRef(msg);
	if( rendReq )
	{
		IAddRenderRequest(rendReq->Request());
		return true;
	}
	plEventCallbackMsg* callback = plEventCallbackMsg::ConvertNoRef(msg);
	if( callback )
	{
		char str[256];
		sprintf(str, "Callback event from %s\n", callback->GetSender()
					? callback->GetSender()->GetName()
					: "Unknown");
		hsStatusMessage(str);
		static int gotten = 0;
		if( ++gotten > 5 )
		{
			plSimpleModifier* simpMod = plSimpleModifier::ConvertNoRef(callback->GetSender()->ObjectIsLoaded());
			plAudible* aud = plAudible::ConvertNoRef(callback->GetSender()->ObjectIsLoaded());
			if( simpMod )
			{
				plAnimCmdMsg* cmd = TRACKED_NEW plAnimCmdMsg;
				cmd->AddReceiver(simpMod->GetKey());
				cmd->SetCmd(plAnimCmdMsg::kRemoveCallbacks);
				cmd->AddCallback(callback);
				plgDispatch::MsgSend(cmd);
				hsRefCnt_SafeUnRef(callback);
			}
			else if( aud )
			{
				plSoundMsg* cmd = TRACKED_NEW plSoundMsg;
				cmd->AddReceiver(aud->GetKey());
				cmd->SetCmd(plSoundMsg::kRemoveCallbacks);
				cmd->AddCallback(callback);
				plgDispatch::MsgSend(cmd);
				hsRefCnt_SafeUnRef(callback);
			}
			hsStatusMessage("Removed\n");
			gotten = 0;
		}
		return true;
	}
	plMovieMsg* mov = plMovieMsg::ConvertNoRef(msg);
	if( mov )
	{
		return IHandleMovieMsg(mov);
	}

	plLinkEffectsTriggerMsg* linkFX = plLinkEffectsTriggerMsg::ConvertNoRef(msg);
	if (linkFX)
	{
		if (!linkFX->IsLeavingAge())
		{
#ifdef MSG_LOADING_BAR
			// Temporary stat gathering stuff
			#if 0//ndef PLASMA_EXTERNAL_RELEASE
			hsUNIXStream s;
			s.Open("Messages.txt", "at");
			static bool firstLog = true;
			if (firstLog)
			{
				firstLog = false;
				s.WriteString("------------------------------------\n");
			}
			char buf[256];
			sprintf(buf, "%s %d\n", plAgeLoader::GetInstance()->GetCurrAgeFilename(), fNumPostLoadMsgs);
			s.WriteString(buf);
			s.Close();
			#endif
#endif
		}
		return true;
	}

	//============================================================================
	// plNetCommAuthMsg
	//============================================================================
	if (plNetCommAuthMsg * authCommMsg = plNetCommAuthMsg::ConvertNoRef(msg)) {
		IHandleNetCommAuthMsg(authCommMsg);
		return true;
	}

	//============================================================================
	// plPreloaderMsg
	//============================================================================
	if (plPreloaderMsg * preloaderMsg = plPreloaderMsg::ConvertNoRef(msg)) {
		IHandlePreloaderMsg(preloaderMsg);
		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}

//============================================================================
hsBool plClient::IHandleMovieMsg(plMovieMsg* mov)
{
	if( !(mov->GetFileName() && *mov->GetFileName()) )
		return true;

	int i;
	i = fMovies.GetCount();
	if( !(mov->GetCmd() & plMovieMsg::kMake) )
	{
		for( i = 0; i < fMovies.GetCount(); i++ )
		{
			if( !stricmp(mov->GetFileName(), fMovies[i]->GetFileName()) )
				break;
		}
	}
	if( i == fMovies.GetCount() )
	{

		fMovies.Append(TRACKED_NEW plBinkPlayer);
		fMovies[i]->SetFileName(mov->GetFileName());
	}

	if( mov->GetCmd() & plMovieMsg::kAddCallbacks )
	{
		int j;
		for( j = 0; j < mov->GetNumCallbacks(); j++ )
			fMovies[i]->AddCallback(mov->GetCallback(j));
	}
	if( mov->GetCmd() & plMovieMsg::kMove )
		fMovies[i]->SetPosition(mov->GetCenter());
	if( mov->GetCmd() & plMovieMsg::kScale )
		fMovies[i]->SetScale(mov->GetScale());
	if( mov->GetCmd() & plMovieMsg::kColorAndOpacity )
		fMovies[i]->SetColor(mov->GetColor());
	if( mov->GetCmd() & plMovieMsg::kColor )
	{
		hsColorRGBA c = fMovies[i]->GetColor();
		c.Set(mov->GetColor().r, mov->GetColor().g, mov->GetColor().b, c.a);
		fMovies[i]->SetColor(c);
	}
	if( mov->GetCmd() & plMovieMsg::kOpacity )
	{
		hsColorRGBA c = fMovies[i]->GetColor();
		c.a = mov->GetColor().a;
		fMovies[i]->SetColor(c);
	}
	if( mov->GetCmd() & plMovieMsg::kFadeIn )
	{
		fMovies[i]->SetFadeFromColor(mov->GetFadeInColor());
		fMovies[i]->SetFadeFromTime(mov->GetFadeInSecs());
	}
	if( mov->GetCmd() & plMovieMsg::kFadeOut )
	{
		fMovies[i]->SetFadeToColor(mov->GetFadeOutColor());
		fMovies[i]->SetFadeToTime(mov->GetFadeOutSecs());
	}
	if( mov->GetCmd() & plMovieMsg::kVolume )
		fMovies[i]->SetVolume(mov->GetVolume());

	if( mov->GetCmd() & plMovieMsg::kStart )
		fMovies[i]->Start(fPipeline, fWindowHndl);
	if( mov->GetCmd() & plMovieMsg::kPause )
		fMovies[i]->Pause(true);
	if( mov->GetCmd() & plMovieMsg::kResume )
		fMovies[i]->Pause(false);
	if( mov->GetCmd() & plMovieMsg::kStop )
		fMovies[i]->Stop();

	// If a movie has lost its filename, it means something went horribly wrong
	// with playing it and it has shutdown. Or we just stopped it. Either way, 
	// we need to clear it out of our list.
	if( !(fMovies[i]->GetFileName() && *fMovies[i]->GetFileName()) )
	{
		delete fMovies[i];
		fMovies.Remove(i);
	}

	return true;
}

int plClient::IFindRoomByLoc(const plLocation& loc)
{
	for (int i = 0; i < fRooms.Count(); i++)
	{
		if (fRooms[i].fNode->GetKey()->GetUoid().GetLocation() == loc)
			return i;
	}

	return -1;
}

bool plClient::IIsRoomLoading(const plLocation& loc)
{
	for (int i = 0; i < fRoomsLoading.size(); i++)
	{
		if (fRoomsLoading[i] == loc)
			return true;
	}
	return false;
}

void plClient::SetHoldLoadRequests(bool hold)
{
	fHoldLoadRequests = hold;
	if (!fHoldLoadRequests)
		ILoadNextRoom();
}

#include "../plResMgr/plPageInfo.h"

void plClient::IQueueRoomLoad(const std::vector<plLocation>& locs, bool hold)
{
	bool allSameAge = true;
	const char* lastAgeName = nil;

	UInt32 numRooms = 0;
	for (int i = 0; i < locs.size(); i++)
	{
		const plLocation& loc = locs[i];

		const plPageInfo* info = plKeyFinder::Instance().GetLocationInfo(loc);
		bool alreadyLoaded = (IFindRoomByLoc(loc) != -1);
		bool isLoading = IIsRoomLoading(loc);
		if (!info || alreadyLoaded || isLoading)
		{
			#ifdef HS_DEBUGGING
			if (!info)
				hsStatusMessageF("Ignoring LoadRoom request for location 0x%x because we can't find the location", loc.GetSequenceNumber());
			else if (alreadyLoaded)
				hsStatusMessageF("Ignoring LoadRoom request for %s-%s, since room is already loaded", info->GetAge(), info->GetPage());
			else if (isLoading)
				hsStatusMessageF("Ignoring LoadRoom request for %s-%s, since room is currently loading", info->GetAge(), info->GetPage());
			#endif

			continue;
		}

		fLoadRooms.push_back(TRACKED_NEW LoadRequest(loc, hold));

		if (!lastAgeName || hsStrEQ(info->GetAge(), lastAgeName))
			lastAgeName = info->GetAge();
		else
			allSameAge = false;

//		hsStatusMessageF("+++ Loading room %s-%s", info.GetAge(), info.GetPage());
		numRooms++;
	}

	if (numRooms == 0)
		return;

	fNumLoadingRooms += numRooms;
}

void plClient::ILoadNextRoom()
{
	LoadRequest* req = nil;

	while (!fLoadRooms.empty())
	{
		req = fLoadRooms.front();
		fLoadRooms.pop_front();

		bool alreadyLoaded = (IFindRoomByLoc(req->loc) != -1);
		hsBool isLoading = IIsRoomLoading(req->loc);
		if (alreadyLoaded || isLoading)
		{
			delete req;
			req = nil;
			fNumLoadingRooms--;
		}
		else
			break;
	}

	if (req)
	{
		plClientRefMsg* pRefMsg = TRACKED_NEW plClientRefMsg(GetKey(),
			plRefMsg::kOnCreate, -1,
			req->hold ? plClientRefMsg::kLoadRoomHold : plClientRefMsg::kLoadRoom);

		fRoomsLoading.push_back(req->loc); // flag the location as currently loading

		// PageInPage is not guaranteed to finish synchronously, just FYI
		plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();
		mgr->PageInRoom(req->loc, plSceneNode::Index(), pRefMsg);

		delete req;

		plClientMsg* nextRoom = TRACKED_NEW plClientMsg(plClientMsg::kLoadNextRoom);
		nextRoom->Send(GetKey());
	}
}

void plClient::IUnloadRooms(const std::vector<plLocation>& locs)
{
	for (int i = 0; i < locs.size(); i++)
	{
		const plLocation& loc = locs[i];

		if (!loc.IsValid())
			continue;

		plKey nodeKey = nil;

		// First, look in our room list. It *should* be there, which allows us to avoid a
		// potential nasty reload-find in the resMgr.
		int roomIdx = IFindRoomByLoc(loc);
		if (roomIdx != -1)
			nodeKey = fRooms[roomIdx].fNode->GetKey();

		if (nodeKey == nil)
		{
			nodeKey = plKeyFinder::Instance().FindSceneNodeKey(loc);
		}

		if (nodeKey != nil)
		{
			plSceneNode* node = plSceneNode::ConvertNoRef(nodeKey->ObjectIsLoaded());
			if (node)
			{
				#ifndef PLASMA_EXTERNAL_RELEASE
				plStatusLog::AddLineS("pageouts.log", "SceneNode for %s loaded; Removing node", node->GetKey()->GetUoid().GetObjectName());
				#endif
				fPageMgr->RemoveNode(node);
			}
			else
			{
				#ifndef PLASMA_EXTERNAL_RELEASE
				plStatusLog::AddLineS("pageouts.log", "SceneNode for %s NOT loaded", nodeKey->GetUoid().GetObjectName());
				#endif
			}
			GetKey()->Release(nodeKey);		// release notify interest in scene node
		
			UInt32 recFlags = 0;
			if (roomIdx != -1)
			{
				recFlags = fRooms[roomIdx].fFlags;
				fRooms.Remove(roomIdx);
			}

			if (node == fCurrentNode)
				fCurrentNode = nil;

			#ifndef PLASMA_EXTERNAL_RELEASE
			plStatusLog::AddLineS("pageouts.log", "Telling netClientMgr about paging out %s", nodeKey->GetUoid().GetObjectName());
			#endif

			if (plNetClientMgr::GetInstance() != nil)
			{
				// Don't care really about the message that just came in, we care whether it was really held or not
				if (!hsCheckBits(recFlags, plRoomRec::kHeld))
					plAgeLoader::GetInstance()->StartPagingOutRoom(&nodeKey, 1);
				// Tell NetClientManager not to expect any pageout info on this guy, since he was held
				else
					plAgeLoader::GetInstance()->IgnorePagingOutRoom(&nodeKey, 1);
			}
		}
		else
		{
			#ifndef PLASMA_EXTERNAL_RELEASE
//			plStatusLog::AddLineS("pageouts.log", "++ Can't find node key for paging out room %s, loc 0x%x",
//				pMsg->GetRoomName() != nil ? pMsg->GetRoomName() : "",
//				loc.GetSequenceNumber());
			#endif
		}
	}
}

void plClient::IRoomLoaded(plSceneNode* node, bool hold)
{
	fCurrentNode = node; 
	// make sure we don't already have this room in the list:
	hsBool bAppend = true;
	for (int i = 0; i < fRooms.Count(); i++)
	{
		if (fRooms[i].fNode == fCurrentNode)
		{	
			bAppend = false;
			break;
		}
	}
	if (bAppend)
	{
		if (hold)
		{
			fRooms.Append(plRoomRec(fCurrentNode, plRoomRec::kHeld));
		}
		else
		{
			fRooms.Append(plRoomRec(fCurrentNode, 0));
			fPageMgr->AddNode(fCurrentNode);
		}
	}

	fNumLoadingRooms--;
	
	// Shut down the progress bar if that was the last room
	if (fProgressBar != nil && fNumLoadingRooms <= 0)
	{
#ifdef MSG_LOADING_BAR
		if (!hold)
		{
			struct AgeMsgCount { const char* AgeName; int NumMsgs; };
			static AgeMsgCount ageMsgCount[] =
			{
				{ "BahroCave",			2600	},
				{ "BaronCityOffice",	670		},
				{ "city",				269000	},
				{ "Cleft",				11000	},
				{ "Garden",				19700	},
				{ "Garrison",			28800	},
				{ "Gira",				3300	},
				{ "Kadish",				19700	},
				{ "Neighborhood",		19900	},
				{ "Nexus",				1400	},
				{ "Personal",			20300	},
				{ "Teledahn",			48000	}
			};

			char name[256];
			strcpy(name, &fProgressBar->GetTitle()[strlen("Loading ")]);
			name[strlen(name)-3] = '\0';

			// Get the precalculated value for how many messages will be
			// sent out before the screen actually fades in
			int numMsgs = 0;
			for (int i = 0; i < sizeof(ageMsgCount)/sizeof(AgeMsgCount); i++)
			{
				if (hsStrEQ(ageMsgCount[i].AgeName, name))
				{
					numMsgs = ageMsgCount[i].NumMsgs;
					break;
				}
			}

			fNumPostLoadMsgs = 0;

			// The last 10% of the age loading bar is for messages, so adjust
			// our progress bar increment to fill the bar fully when all
			// messages have been sent
			float max = fProgressBar->GetMax();
			float amtLeft = max - (max * 0.9f);
			fPostLoadMsgInc = (numMsgs != 0) ? amtLeft / numMsgs : 0;
			
#ifndef PLASMA_EXTERNAL_RELEASE
			if (plDispatchLogBase::IsLogging())
				plDispatchLogBase::GetInstance()->LogStatusBarChange(fProgressBar->GetTitle(), "displaying messages");
#endif // PLASMA_EXTERNAL_RELEASE
#endif
		}
	}

	hsRefCnt_SafeUnRef(fCurrentNode);
	plKey pRmKey = fCurrentNode->GetKey();
	plAgeLoader::GetInstance()->FinishedPagingInRoom(&pRmKey, 1);
	// *** this used to call "ActivateNode" (in physics) which wasn't implemented.
	// *** we should make this "turn on" physics for the selected node
	// *** depending on what guarantees we can make about the load state -- anything useful?

	// now tell all those who are interested that a room was loaded
	if (!hold)
	{
		plRoomLoadNotifyMsg* loadmsg = TRACKED_NEW plRoomLoadNotifyMsg;
		loadmsg->SetRoom(pRmKey);
		loadmsg->SetWhatHappen(plRoomLoadNotifyMsg::kLoaded);
		plgDispatch::MsgSend(loadmsg);
	}
	else
		hsStatusMessageF("Done loading hold room %s, t=%f\n", pRmKey->GetName(), hsTimer::GetSeconds());

	plLocation loc = pRmKey->GetUoid().GetLocation();
	for (int i = 0; i < fRoomsLoading.size(); i++)
	{
		if (fRoomsLoading[i] == loc)
		{
			fRoomsLoading.erase(fRoomsLoading.begin() + i);
			break;
		}
	}
	
	if (!fNumLoadingRooms)
		IStopProgress();
}

//============================================================================
void plClient::IRoomUnloaded(plSceneNode* node)
{
	#ifndef PLASMA_EXTERNAL_RELEASE
	plStatusLog::AddLineS("pageouts.log", "..    refMsg is onDestroy");
	#endif

	fCurrentNode = node; 
	hsRefCnt_SafeUnRef(fCurrentNode);
	plKey pRmKey = fCurrentNode->GetKey();
	if (plAgeLoader::GetInstance())
		plAgeLoader::GetInstance()->FinishedPagingOutRoom(&pRmKey, 1);

	// tell all those who are interested that a room was unloaded
	plRoomLoadNotifyMsg* loadmsg = TRACKED_NEW plRoomLoadNotifyMsg;
	loadmsg->SetRoom(pRmKey);
	loadmsg->SetWhatHappen(plRoomLoadNotifyMsg::kUnloaded);
	plgDispatch::MsgSend(loadmsg);
}

void plClient::IReadKeyedObjCallback(plKey key)
{
	fInstance->IIncProgress(1, key->GetName());
}

//============================================================================
void plClient::IProgressMgrCallbackProc(plOperationProgress * progress)
{
	if(!fInstance)
		return;

	fInstance->fMessagePumpProc();
	fInstance->IDraw();
}

//============================================================================
void plClient::IIncProgress (hsScalar byHowMuch, const char * text)
{
	if (fProgressBar) {
#ifndef PLASMA_EXTERNAL_RELEASE
		fProgressBar->SetStatusText( text );
#endif
		fProgressBar->Increment( byHowMuch );
	}
}

//============================================================================
void	plClient::IStartProgress( const char *title, hsScalar len )
{
	if (fProgressBar)
	{
		fProgressBar->SetLength(fProgressBar->GetMax()+len);
	}
	else
	{
		fProgressBar = plProgressMgr::GetInstance()->RegisterOperation(len, title, plProgressMgr::kNone, false, true);
#ifndef PLASMA_EXTERNAL_RELEASE
		if (plDispatchLogBase::IsLogging())
			plDispatchLogBase::GetInstance()->LogStatusBarChange(fProgressBar->GetTitle(), "starting");
#endif // PLASMA_EXTERNAL_RELEASE

		((plResManager*)hsgResMgr::ResMgr())->SetProgressBarProc(IReadKeyedObjCallback);
		plDispatch::SetMsgRecieveCallback(IDispatchMsgReceiveCallback);

		fLastProgressUpdate = 0.f;
	}
	// Workaround for NVidia driver bug, showing up as BCO not there first time.
	// See Mantis bug 0014590.
	if( fPipeline )
		fPipeline->LoadResources();
}


//============================================================================
void	plClient::IStopProgress( void )
{
	if (fProgressBar)
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		if (plDispatchLogBase::IsLogging())
			plDispatchLogBase::GetInstance()->LogStatusBarChange(fProgressBar->GetTitle(), "done");
#endif // PLASMA_EXTERNAL_RELEASE

		plDispatch::SetMsgRecieveCallback(nil);
		((plResManager*)hsgResMgr::ResMgr())->SetProgressBarProc(IReadKeyedObjCallback);
		delete fProgressBar;
		fProgressBar = nil;

		plPipeResReq::Request();

		fFlags.SetBit(kFlagGlobalDataLoaded);		
		if (fFlags.IsBitSet(kFlagAsyncInitComplete))
			ICompleteInit();
	}
}

/*****************************************************************************
*
*   
*
***/

extern	hsBool	gDataServerLocal;

#include "plQuality.h"
#include "plLoadMask.h"

#if 0
class LoginNetClientCommCallback : public plNetClientComm::Callback
{
public:
	enum Op {kAuth, kCreatePlayer, kGetPlayerList, kLeave, kDeletePlayer};

	LoginNetClientCommCallback() : plNetClientComm::Callback(), fNumCurrentOps(0)
	{}

	virtual void OperationStarted( UInt32 context )
	{
		fNumCurrentOps++;
	}
	virtual void OperationComplete( UInt32 context, int resultCode )
	{
		if (context == kAuth)
		{
			if (hsSucceeded(resultCode))
			{
				plClient::GetInstance()->fAuthPassed = true;
			}
		}
		else if (context == kGetPlayerList)
		{
			if ( hsSucceeded( resultCode ) )
			{
				UInt32 numPlayers = fCbArgs.GetInt(0);
				UInt32 pId = -1;
				std::string pName;

				for (UInt32 i = 0; i < numPlayers; i++)
				{
					pId = fCbArgs.GetInt((UInt16)(i*3+1));
					pName = fCbArgs.GetString((UInt16)(i*3+2));

					if (pName == plClient::GetInstance()->fUsername)
					{
						plClient::GetInstance()->fPlayerID = pId;
						break;
					}
				}
			}
		}
		else if (context == kCreatePlayer)
		{
			if (hsSucceeded(resultCode))
                plClient::GetInstance()->fPlayerID = fCbArgs.GetInt(0);
		}
		else if (context == kDeletePlayer)
		{
			if (hsSucceeded(resultCode))
				plClient::GetInstance()->fPlayerID = -1;
		}

		fNumCurrentOps--;
	}

	bool IsActive()
	{
		return fNumCurrentOps > 0;
	}

private:
	int fNumCurrentOps;
};
#endif

//============================================================================
hsBool plClient::StartInit()
{
	hsStatusMessage("Init client\n");
	fFlags.SetBit( kFlagIniting );

	pfLocalizationMgr::Initialize("dat");

	plQuality::SetQuality(fQuality);
	if( (GetClampCap() >= 0) && (GetClampCap() < plQuality::GetCapability()) )
		plQuality::SetCapability(GetClampCap());

	/// 2.16.2001 mcn - Moved console engine init to constructor, 
	/// so we could use console commands even before the pipeline init

	plDTProgressMgr::DeclareThyself();

	// Set our callback for the progress manager so everybody else can use it
	fLastProgressUpdate = 0.f;
	plProgressMgr::GetInstance()->SetCallbackProc( IProgressMgrCallbackProc );

	// Check the registry, which deletes data files that are either corrupt or
	// have old version numbers.  If the file still exists on the file server
	// then it will be patched on-the-fly as needed (unless you're running with
	// local data of course).
	((plResManager *)hsgResMgr::ResMgr())->VerifyPages();

	// the dx8 audio system MUST be initialized
	// before the database is loaded
	HWND hWnd = fWindowHndl;
	SetForegroundWindow(fWindowHndl);

	plgAudioSys::Init(hWnd);
	gAudio = plgAudioSys::Sys();

	RegisterAs( kClient_KEY );

	InitDLLs();
	
	plGlobalVisMgr::Init();
	fPageMgr = TRACKED_NEW plPageTreeMgr;

	plVisLOSMgr::Init(fPipeline, fPageMgr);

	// init globals
	plAvatarMgr::GetInstance();
	plRelevanceMgr::Init();

	gDisp = plgDispatch::Dispatch();
	gTimerMgr = plgTimerCallbackMgr::Mgr();

	//
	// initialize input system
	//
	InitInputs();

	/// Init the console object
	///	Note: this can be done last because the console engine was inited first, and
	/// everything in code that works with the console does so through the console engine

	fConsole = TRACKED_NEW pfConsole();
	pfConsole::SetPipeline( fPipeline );
	fConsole->RegisterAs( kConsoleObject_KEY );		// fixedKey from plFixedKey.h
	fConsole->Init( fConsoleEngine );

	/// Init the font cache
	fFontCache = TRACKED_NEW plFontCache();

	/// Init the transition manager
	fTransitionMgr = TRACKED_NEW plTransitionMgr();
	fTransitionMgr->RegisterAs( kTransitionMgr_KEY );		// fixedKey from plFixedKey.h
	fTransitionMgr->Init();

	// Init the Age Linking effects manager
	fLinkEffectsMgr = TRACKED_NEW plLinkEffectsMgr();
	fLinkEffectsMgr->RegisterAs( kLinkEffectsMgr_KEY );	// fixedKey from plFixedKey.h
	fLinkEffectsMgr->Init();
	
	/// Init the in-game GUI manager
	fGameGUIMgr = TRACKED_NEW pfGameGUIMgr();
	fGameGUIMgr->RegisterAs( kGameGUIMgr_KEY );
	fGameGUIMgr->Init();

	plgAudioSys::Activate(true);

	plConst(hsScalar) delay(2.f);
	//commenting out publisher splash for MORE
	//IPlayIntroBink("avi/intro0.bik", delay, 0.f, 0.f, 1.f, 1.f, 0.75);
	//if( GetDone() ) return false;
	IPlayIntroBink("avi/intro1.bik", 0.f, 0.f, 0.f, 1.f, 1.f, 0.75);
	if( GetDone() ) return false;
	plgDispatch::Dispatch()->RegisterForExactType(plMovieMsg::Index(), GetKey());

	//
	// Init Net before loading things
	//
	plgDispatch::Dispatch()->RegisterForExactType(plNetCommAuthMsg::Index(), GetKey());
	plNetClientMgr::GetInstance()->Init();
	plAgeLoader::GetInstance()->Init();
	pfSecurePreloader::GetInstance()->Init();
	
	plCmdIfaceModMsg* pModMsg2 = TRACKED_NEW plCmdIfaceModMsg;
	pModMsg2->SetBCastFlag(plMessage::kBCastByExactType);
	pModMsg2->SetSender(fConsole->GetKey());
	pModMsg2->SetCmd(plCmdIfaceModMsg::kAdd);
	plgDispatch::MsgSend(pModMsg2);

	// create new the virtual camera
	fNewCamera = TRACKED_NEW plVirtualCam1;
	fNewCamera->RegisterAs( kVirtualCamera1_KEY ); 
	fNewCamera->Init();
	fNewCamera->SetPipeline( GetPipeline() );

	float aspectratio = (float)fPipeline->Width() / (float)fPipeline->Height();
	plVirtualCam1::SetAspectRatio(aspectratio);
	plVirtualCam1::SetFOV(plVirtualCam1::GetFOVw(), plVirtualCam1::GetFOVh());

	pfGameGUIMgr::GetInstance()->SetAspectRatio( aspectratio );
	plMouseDevice::Instance()->SetDisplayResolution((float)fPipeline->Width(), (float)fPipeline->Height());

	// create the listener for the audio system:
	plListener* pLMod = TRACKED_NEW plListener;
	pLMod->RegisterAs(kListenerMod_KEY );

	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), pLMod->GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAudioSysMsg::Index(), pLMod->GetKey());

	plSynchedObject::PushSynchDisabled(false);		// enable dirty tracking

	if (StrCmp(NetCommGetStartupAge()->ageDatasetName, "StartUp") == 0)
	{
		plNetCommAuthMsg * msg  = NEW(plNetCommAuthMsg);
		msg->result             = kNetSuccess;
		msg->param              = nil;
		msg->Send();
	}

	// 2nd half of plClient initialization occurs after
	// all network events have completed.  Async events:
	//
	// 1) Download secure files
	//
	// Continue plClient init via IOnAsyncInitComplete().

	return true;
}

//============================================================================
void	plClient::IPatchGlobalAgeFiles( void )
{
	const char * ageFiles[] = {
		"GlobalAnimations",
		"GlobalAvatars",
		"GlobalClothing",
		"GlobalMarkers",
		"GUI",
		"CustomAvatars"
	};

	for (unsigned i = 0; i < arrsize(ageFiles); ++i) {
		plResPatcher myPatcher(ageFiles[i], true);

		if (gDataServerLocal)
			break;
			
		if (!myPatcher.Update()) {
			SetDone(true);
			break;
		}
	}
}

void plClient::InitDLLs()
{
	hsStatusMessage("Init dlls client\n");
	char str[255];
	typedef void (*PInitGlobalsFunc) (hsResMgr *, plFactory *, plTimerCallbackManager *, plTimerShare*,
		plNetClientApp*);

	hsFolderIterator modDllFolder("ModDLL\\");
	while (modDllFolder.NextFileSuffix(".dll")) 
	{
		modDllFolder.GetPathAndName(str);
		HMODULE hMod = LoadLibrary(str);
		if (hMod)
		{
			PInitGlobalsFunc initGlobals = (PInitGlobalsFunc)GetProcAddress(hMod, "InitGlobals");
			initGlobals(hsgResMgr::ResMgr(), plFactory::GetTheFactory(), plgTimerCallbackMgr::Mgr(), 
				hsTimer::GetTheTimer(), plNetClientApp::GetInstance());
			fLoadedDLLs.Append(hMod);
		}
	}
}

void plClient::ShutdownDLLs()
{
	int j;
	for( j = 0; j < fLoadedDLLs.GetCount(); j++ )
	{
		BOOL ret = FreeLibrary(fLoadedDLLs[j]);
		if( !ret )
			hsStatusMessage("Failed to free lib\n");
	}
	fLoadedDLLs.Reset();
}

hsBool plClient::MainLoop()
{
#ifndef PLASMA_EXTERNAL_RELEASE
	if (PythonInterface::UsePythonDebugger())
	{
		PythonInterface::PythonDebugger()->Update();

		if (PythonInterface::PythonDebugger()->IsConnected())
		{
			bPythonDebugConnected = true;
			if (PythonInterface::DebuggerRequestedExit() && PythonInterface::PythonDebugger()->ExitOnStop())
				SetDone(true); // debugger requested that we stop running, so exit nicely
		}
		else
			bPythonDebugConnected = false;
	}
#endif

#ifdef PLASMA_EXTERNAL_RELEASE
	if (DebugIsDebuggerPresent())
	{
		NetCliAuthLogClientDebuggerConnect();
		SetDone(true);
	}
#endif

	if(plClient::fDelayMS)
		Sleep(5);
	
	// Reset our stats
	plProfileManager::Instance().BeginFrame();

	if (IUpdate())
		return true;
		
	if (IDraw())
		return true;
	
	plProfileManagerFull::Instance().EndFrame();
	plProfileManager::Instance().EndFrame();

	// Draw the stats
	plProfileManagerFull::Instance().Update();

	return false;
}

#include "plProfile.h"

plProfile_Extern(DrawTime);
plProfile_Extern(UpdateTime);
plProfile_CreateTimer("ResMgr", "Update", ResMgr);
plProfile_CreateTimer("DispatchQueue", "Update", DispatchQueue);
plProfile_CreateTimer("RenderSetup", "Update", RenderMsg);
plProfile_CreateTimer("Simulation", "Update", Simulation);
plProfile_CreateTimer("NetTime", "Update", UpdateNetTime);
plProfile_Extern(TimeMsg);
plProfile_Extern(EvalMsg);
plProfile_Extern(TransformMsg);
plProfile_Extern(CameraMsg);
plProfile_Extern(AnimatingPhysicals);
plProfile_Extern(StoppedAnimPhysicals);

plProfile_CreateTimer("BeginRender", "Render", BeginRender);
plProfile_CreateTimer("ClearRender", "Render", ClearRender);
plProfile_CreateTimer("PreRender", "Render", PreRender);
plProfile_CreateTimer("MainRender", "Render", MainRender);
plProfile_CreateTimer("PostRender", "Render", PostRender);
plProfile_CreateTimer("Movies", "Render", Movies);
plProfile_CreateTimer("Console", "Render", Console);
plProfile_CreateTimer("StatusLog", "Render", StatusLog);
plProfile_CreateTimer("ProgressMgr", "Render", ProgressMgr);
plProfile_CreateTimer("ScreenElem", "Render", ScreenElem);
plProfile_CreateTimer("EndRender", "Render", EndRender);


hsBool plClient::IUpdate()
{
	plProfile_BeginTiming(UpdateTime);
	
	// reset timer on first frame if realtime and not clamping, to avoid initial large delta
	if (hsTimer::GetSysSeconds()==0 && hsTimer::IsRealTime() && hsTimer::GetTimeClamp()==0)
		hsTimer::SetRealTime(true);

	plProfile_BeginTiming(DispatchQueue);
	plgDispatch::Dispatch()->MsgQueueProcess();
	plProfile_EndTiming(DispatchQueue);
	
	const char *inputUpdate = "Update";
	if (fInputManager) // Is this used anymore? Seems to always be nil.
		fInputManager->Update();

	hsTimer::IncSysSeconds();
	plClientUnifiedTime::SetSysTime();	// keep a unified time, based on sysSeconds
	// Time may have been clamped in IncSysSeconds, depending on hsTimer's current mode.

	double currTime = hsTimer::GetSysSeconds();
	hsScalar delSecs = hsTimer::GetDelSysSeconds();

	// do not change this ordering

	plProfile_BeginTiming(UpdateNetTime);
	plNetClientMgr::GetInstance()->Update(currTime);
	plProfile_EndTiming(UpdateNetTime);

	// update python 
	//plCaptureRender::Update(fPipeline);
	plCaptureRender::Update();
	cyMisc::Update(currTime);

	// This TimeMsg doesn't really do much, except somehow it flushes the dispatch
	// after the NetClientMgr updates, delivering any SelfDestruct messages in the
	// queue. This is important to prevent objects that are about to go away from
	// starting trouble during their update. So to get rid of this message, some
	// other way of flushing the dispatch after NegClientMgr's update is needed. mf	
	plProfile_BeginTiming(TimeMsg);
	plTimeMsg* msg = TRACKED_NEW plTimeMsg(nil, nil, nil, nil);
	plgDispatch::MsgSend(msg);
	plProfile_EndTiming(TimeMsg);

	plProfile_BeginTiming(EvalMsg);
	plEvalMsg* eval = TRACKED_NEW plEvalMsg(nil, nil, nil, nil);
	plgDispatch::MsgSend(eval);
	plProfile_EndTiming(EvalMsg);

	char *xFormLap1 = "Main";
	plProfile_BeginLap(TransformMsg, xFormLap1);
	plTransformMsg* xform = TRACKED_NEW plTransformMsg(nil, nil, nil, nil);
	plgDispatch::MsgSend(xform);
	plProfile_EndLap(TransformMsg, xFormLap1);

	plCoordinateInterface::SetTransformPhase(plCoordinateInterface::kTransformPhaseDelayed);	

	if (fAnimDebugList)
		fAnimDebugList->ShowReport();
	
	plProfile_BeginTiming(Simulation);
	plSimulationMgr::GetInstance()->Advance(delSecs);
	plProfile_EndTiming(Simulation);
			
	// At this point, we just register for a plDelayedTransformMsg when dirtied.
	if (!plCoordinateInterface::GetDelayedTransformsEnabled())
	{
		char *xFormLap2 = "Simulation";
		plProfile_BeginLap(TransformMsg, xFormLap2);
		xform = TRACKED_NEW plTransformMsg(nil, nil, nil, nil);
		plgDispatch::MsgSend(xform);
		plProfile_EndLap(TransformMsg, xFormLap2);
	}
	else
	{
		char *xFormLap3 = "Delayed";
		plProfile_BeginLap(TransformMsg, xFormLap3);
		xform = TRACKED_NEW plDelayedTransformMsg(nil, nil, nil, nil);
		plgDispatch::MsgSend(xform);
		plProfile_EndLap(TransformMsg, xFormLap3);
	}

	plCoordinateInterface::SetTransformPhase(plCoordinateInterface::kTransformPhaseNormal);
	
	plProfile_BeginTiming(CameraMsg);
	plCameraMsg* cameras = TRACKED_NEW plCameraMsg;
	cameras->SetCmd(plCameraMsg::kUpdateCameras);
	cameras->SetBCastFlag(plMessage::kBCastByExactType);
	plgDispatch::MsgSend(cameras);
	plProfile_EndTiming(CameraMsg);

	if (fPatchGlobalAges)
	{
		// Download or patch our global ages, if necessary
		IPatchGlobalAgeFiles();
		IOnAsyncInitComplete();

		fPatchGlobalAges = false;
	}

	return false;
}

hsBool plClient::IDrawProgress() {
	// HACK: Don't draw while we're caching some room loads, otherwise the
	// progress bar will jump around while we're calculating the size
	if (fHoldLoadRequests)
		return false;

	// Reset our stats
	plProfileManager::Instance().BeginFrame();

	plProfile_BeginTiming(DrawTime);
	if( fPipeline->BeginRender() )
	{
		return IFlushRenderRequests();
	}
	// Override the clear color to black.
	fPipeline->ClearRenderTarget(&hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));

#ifndef PLASMA_EXTERNAL_RELEASE
	fConsole->Draw( fPipeline );
#endif

	plStatusLogMgr::GetInstance().Draw();
	plProgressMgr::GetInstance()->Draw( fPipeline );
	fPipeline->RenderScreenElements();
	fPipeline->EndRender();
	plProfile_EndTiming(DrawTime);

	plProfileManager::Instance().EndFrame();

	return false;
}

hsBool plClient::IDraw()
{
	// Limit framerate
	static float lastDrawTime;
	static const float kMaxFrameRate = 1.f/30.f;
	float currTime = (float) hsTimer::GetSeconds();
	if (!fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNVPerfHUD))
	{
		// If we're using NVPerfHUD to step through draw calls,
		// We're going to have a frame delta of zero. In that
		// case we need to draw no matter what, and we don't
		// care as much about starving other threads because
		// we're presumably just debugging a graphics glitch.
		if ((currTime - lastDrawTime) < kMaxFrameRate)
			return true;
	}
	lastDrawTime = currTime;

	// If we're shutting down, don't attempt to draw.  Doing so
	// tends to cause a device reload each frame.	
	if (fDone)
		return true;
	
	if (plProgressMgr::GetInstance()->IsActive())
		return IDrawProgress();
		
	plProfile_Extern(VisEval);
	plProfile_BeginTiming(VisEval);
	plGlobalVisMgr::Instance()->Eval(fPipeline->GetViewPositionWorld());
	plProfile_EndTiming(VisEval);

	plProfile_BeginTiming(RenderMsg);
	plRenderMsg* rendMsg = TRACKED_NEW plRenderMsg(fPipeline);
	plgDispatch::MsgSend(rendMsg);
	plProfile_EndTiming(RenderMsg);

	plPreResourceMsg* preMsg = TRACKED_NEW plPreResourceMsg(fPipeline);
	plgDispatch::MsgSend(preMsg);

	// This might not be the ideal place for this, but it 
	// needs to be AFTER the plRenderMsg is sent, and
	// BEFORE BeginRender. (plRenderMsg causes construction of
	// Dynamic objects (e.g. RT's), BeginRender uses them (e.g. shadows).
	if( plPipeResReq::Check() || fPipeline->CheckResources() )
	{
		fPipeline->LoadResources();
	}

	plProfile_EndTiming(UpdateTime);

	plProfile_BeginTiming(DrawTime);

	plProfile_BeginTiming(BeginRender);
	if( fPipeline->BeginRender() )
	{
		plProfile_EndTiming(BeginRender);
		return IFlushRenderRequests();
	}
	plProfile_EndTiming(BeginRender);

	plProfile_BeginTiming(ClearRender);
	fPipeline->ClearRenderTarget();
	plProfile_EndTiming(ClearRender);

	plProfile_BeginTiming(PreRender);
	if( !fFlags.IsBitSet( kFlagDBGDisableRRequests ) )
		IProcessPreRenderRequests();
	plProfile_EndTiming(PreRender);

	plProfile_BeginTiming(MainRender);
	if( !fFlags.IsBitSet( kFlagDBGDisableRender ) )
		fPageMgr->Render(fPipeline);
	plProfile_EndTiming(MainRender);

	plProfile_BeginTiming(PostRender);
	if( !fFlags.IsBitSet( kFlagDBGDisableRRequests ) )
		IProcessPostRenderRequests();
	plProfile_EndTiming(PostRender);

	plProfile_BeginTiming(Movies);
	IServiceMovies();
	plProfile_EndTiming(Movies);

#ifndef PLASMA_EXTERNAL_RELEASE
	plProfile_BeginTiming(Console);
	fConsole->Draw( fPipeline );
	plProfile_EndTiming(Console);
#endif

	plProfile_BeginTiming(StatusLog);
	plStatusLogMgr::GetInstance().Draw();
	plProfile_EndTiming(StatusLog);

	plProfile_BeginTiming(ProgressMgr);
	plProgressMgr::GetInstance()->Draw( fPipeline );
	plProfile_EndTiming(ProgressMgr);

	fLastProgressUpdate = hsTimer::GetSeconds();

	plProfile_BeginTiming(ScreenElem);
	fPipeline->RenderScreenElements();
	plProfile_EndTiming(ScreenElem);

	plProfile_BeginTiming(EndRender);
	fPipeline->EndRender();
	plProfile_EndTiming(EndRender);

	plProfile_EndTiming(DrawTime); 

	return false;
}

void plClient::IServiceMovies()
{
	int i;
	for( i = 0; i < fMovies.GetCount(); i++ )
	{
		hsAssert(fMovies[i]->GetFileName() && *fMovies[i]->GetFileName(), "Lost our movie");
		if( !fMovies[i]->NextFrame() )
		{
			delete fMovies[i];
			fMovies.Remove(i);
			i--;
		}
	}
}

void plClient::IKillMovies()
{
	int i;
	for( i = 0; i < fMovies.GetCount(); i++ )
		delete fMovies[i];
	fMovies.Reset();
}

hsBool plClient::IPlayIntroBink(const char* movieName, hsScalar endDelay, hsScalar posX, hsScalar posY, hsScalar scaleX, hsScalar scaleY, hsScalar volume /* = 1.0 */)
{
	SetQuitIntro(false);
	plBinkPlayer player;
	player.SetPosition(posX, posY);
	player.SetScale(scaleX, scaleY);
	player.SetFileName(movieName);
	player.SetFadeToTime(endDelay);
	player.SetFadeToColor(hsColorRGBA().Set(0, 0, 0, 1.f));
	player.SetVolume(volume);
	bool firstTry = true;  // flag to make sure that we don't quit before we even start

	if( player.Start(fPipeline, fWindowHndl) )
	{
		while( true )
		{
			if( fInstance )
				fInstance->fMessagePumpProc();

			if( GetDone() )
				return true;
			if (firstTry)
			{
				firstTry = false;
				SetQuitIntro(false);
			}
			else
			{
				if( GetQuitIntro() )
					return true;
			}

			hsBool done = false;
			if( !fPipeline->BeginRender() )
			{
				fPipeline->ClearRenderTarget();
				done = !player.NextFrame();

				fPipeline->EndRender();
			}

			if( done )
				return true;
		}
		return true;
	}
	return false;
}

hsBool plClient::IFlushRenderRequests()
{
	// For those requesting ack's, we could go through and send them
	// mail telling them their request was ill-timed. But hopefully,
	// the lack of an acknowledgement will serve as notice.
	int i;
	for( i = 0; i < fPreRenderRequests.GetCount(); i++ )
		hsRefCnt_SafeUnRef(fPreRenderRequests[i]);
	fPreRenderRequests.Reset();

	for( i = 0; i < fPostRenderRequests.GetCount(); i++ )
		hsRefCnt_SafeUnRef(fPostRenderRequests[i]);
	fPostRenderRequests.Reset();

	return false;
}

void plClient::IProcessRenderRequests(hsTArray<plRenderRequest*>& reqs)
{
	int i;
	for( i = 0; i < reqs.GetCount(); i++ )
	{
		reqs[i]->Render(fPipeline, fPageMgr);
		hsRefCnt_SafeUnRef(reqs[i]);
	}
	reqs.SetCount(0);
}

void plClient::IProcessPreRenderRequests()
{
	IProcessRenderRequests(fPreRenderRequests);
}

void plClient::IProcessPostRenderRequests()
{
	IProcessRenderRequests(fPostRenderRequests);
}

void plClient::IAddRenderRequest(plRenderRequest* req)
{
	if( req->GetPriority() < 0 )
	{
		int i;
		for( i = 0; i < fPreRenderRequests.GetCount(); i++ )
		{
			if( req->GetPriority() < fPreRenderRequests[i]->GetPriority() )
				break;
		}
		fPreRenderRequests.Insert(i, req);
		hsRefCnt_SafeRef(req);
	}
	else
	{
		int i;
		for( i = 0; i < fPostRenderRequests.GetCount(); i++ )
		{
			if( req->GetPriority() < fPostRenderRequests[i]->GetPriority() )
				break;
		}
		fPostRenderRequests.Insert(i, req);
		hsRefCnt_SafeRef(req);
	}
}

hsG3DDeviceModeRecord plClient::ILoadDevMode(const char* devModeFile)
{
	hsStatusMessage("Load DevMode client\n");
	HWND hWnd = fWindowHndl;

	hsUNIXStream	stream;
	hsBool			gottaCreate = false;

	// If DevModeFind is specified, use the old method
//	if ((GetGameFlags() & kDevModeFind))
//		FindAndSaveDevMode(hWnd, devModeFile);
	// Otherwise, use the new method
	hsG3DDeviceModeRecord dmr;
	if (stream.Open(devModeFile, "rb"))
	{
		/// It's there, but is the device record valid?
		hsG3DDeviceRecord selRec;
		hsG3DDeviceMode selMode;

		selRec.Read(&stream);
		if( selRec.IsInvalid() )
		{
			hsStatusMessage( "WARNING: Old DeviceRecord found on file. Setting defaults..." );
			gottaCreate = true;
		}
		else
		{
			/// Read the rest in
			selMode.Read(&stream);

			UInt16 performance = stream.ReadSwap16();

			if( performance < 25 )
				plBitmap::SetGlobalLevelChopCount( 2 );
			else if( performance < 75 )
				plBitmap::SetGlobalLevelChopCount( 1 );
			else
				plBitmap::SetGlobalLevelChopCount( 0 );
		}
		stream.Close();

		dmr = hsG3DDeviceModeRecord(selRec, selMode);
	}
	else
		gottaCreate = true;

	if( gottaCreate )
	{

		hsG3DDeviceSelector devSel;
		devSel.Enumerate(hWnd);
		devSel.RemoveUnusableDevModes(true);

		if (!devSel.GetDefault(&dmr))
		{
			//hsAssert(0, "plGame::LoadDevMode - No acceptable hardware found");
			hsMessageBox("No suitable rendering devices found.","realMYST",hsMessageBoxNormal);
			return dmr;
		}

		if (stream.Open(devModeFile, "wb"))
		{
			dmr.GetDevice()->Write(&stream);
			dmr.GetMode()->Write(&stream);
			stream.WriteSwap16((UInt16)(0*100));
			stream.Close();
		}

	}
	return dmr;
}

void plClient::ResetDisplayDevice(int Width, int Height, int ColorDepth, hsBool Windowed, int NumAASamples, int MaxAnisotropicSamples, hsBool VSync, hsBool windowOnly)
{
	if(!fPipeline) return;
	int BorderWidth = 0, BorderHeight = 0, CaptionHeight = 0;

	WindowActivate(false);
	int ActualWidth;
	int ActualHeight;

	if( Windowed )
	{
		BorderWidth	= GetSystemMetrics( SM_CXSIZEFRAME );
		BorderHeight = GetSystemMetrics( SM_CYSIZEFRAME );
		CaptionHeight = GetSystemMetrics( SM_CYCAPTION );
		ActualWidth = Width + BorderWidth * 2;
		ActualHeight = Height + BorderHeight * 2 + CaptionHeight;
		SetWindowLong( fWindowHndl, GWL_STYLE,
			WS_POPUP | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE);
		SetWindowLong(fWindowHndl, GWL_EXSTYLE, 0);
	}
	else
	{
		SetWindowLong(fWindowHndl, GWL_STYLE,
			WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZE | WS_VISIBLE);

		SetWindowLong(fWindowHndl, GWL_EXSTYLE, WS_EX_APPWINDOW);
	}

	if(!windowOnly)
		fPipeline->ResetDisplayDevice(Width, Height, ColorDepth, Windowed, NumAASamples, MaxAnisotropicSamples, VSync);

	float aspectratio = (float)Width / (float)Height;
	plMouseDevice::Instance()->SetDisplayResolution((float)Width, (float)Height);
	pfGameGUIMgr::GetInstance()->SetAspectRatio( aspectratio );

	UINT flags = SWP_NOCOPYBITS | SWP_NOMOVE | SWP_SHOWWINDOW;
	if(Windowed)
	{
		SetWindowPos( fWindowHndl, HWND_NOTOPMOST, 0, 0, ActualWidth, ActualHeight, flags );
	}
	else
	{
		SetWindowPos( fWindowHndl, HWND_TOP, 0, 0, Width, Height, flags );
		::ClipCursor(nil);
	}

	WindowActivate(true);
}


void WriteBool(hsStream *stream, char *name, hsBool on )
{
	char command[256];
	sprintf(command, "%s %s\r\n", name, on ? "true" : "false");
	stream->WriteString(command);
}

void WriteInt(hsStream *stream, char *name, int val )
{
	char command[256];
	sprintf(command, "%s %d\r\n", name, val);
	stream->WriteString(command);
}

void WriteString(hsStream *stream, const char *name, const char *val)
{
	char command[256];
	sprintf(command, "%s %s\r\n", name, val);
	stream->WriteString(command);
}

// Detect audio/video settings and save them to their respective ini file, if ini files don't exist
void plClient::IDetectAudioVideoSettings()
{
	// Setup default pipeline settings
	hsBool devmode = true;
	hsG3DDeviceModeRecord dmr;
	hsG3DDeviceSelector devSel;
	devSel.Enumerate(fWindowHndl);
	devSel.RemoveUnusableDevModes(true);

	if (!devSel.GetDefault(&dmr))
		devmode = false;
	hsG3DDeviceRecord *rec = (hsG3DDeviceRecord *)dmr.GetDevice();
	const hsG3DDeviceMode *mode = dmr.GetMode();

	hsBool pixelshaders = rec->GetCap(hsG3DDeviceSelector::kCapsPixelShader);
	int psMajor = 0, psMinor = 0;
	rec->GetPixelShaderVersion(psMajor, psMinor);
	hsBool refDevice = false;
	if(rec->GetG3DHALorHEL() == hsG3DDeviceSelector::kHHD3DRefDev)
		refDevice = true;

	plPipeline::fDefaultPipeParams.Width = hsG3DDeviceSelector::kDefaultWidth;
	plPipeline::fDefaultPipeParams.Height = hsG3DDeviceSelector::kDefaultHeight;
	plPipeline::fDefaultPipeParams.ColorDepth = hsG3DDeviceSelector::kDefaultDepth;
#if defined(HS_DEBUGGING) || defined(DEBUG)
	plPipeline::fDefaultPipeParams.Windowed = true;
#else
	plPipeline::fDefaultPipeParams.Windowed = false;
#endif

	plPipeline::fDefaultPipeParams.Shadows = 0;
	// enable shadows if TnL is available, meaning not an intel extreme.
	if(rec->GetG3DHALorHEL() == hsG3DDeviceSelector::kHHD3DTnLHalDev)
		plPipeline::fDefaultPipeParams.Shadows = 1;

	// enable planar reflections if pixelshaders are available
	if(pixelshaders && !refDevice)
	{
	plPipeline::fDefaultPipeParams.PlanarReflections = 1;
	}
	else
	{
	plPipeline::fDefaultPipeParams.PlanarReflections = 0;
	}

	// enable 2x antialiasing and anisotropic to 2 samples if pixelshader version is greater that 2.0
	if(psMajor >= 2 && !refDevice)
	{
		plPipeline::fDefaultPipeParams.AntiAliasingAmount = rec->GetMaxAnisotropicSamples() ? 2 : 0;
		plPipeline::fDefaultPipeParams.AnisotropicLevel = mode->GetNumFSAATypes() ? 2 : 0;
	}
	else
	{
		plPipeline::fDefaultPipeParams.AntiAliasingAmount = 0;
		plPipeline::fDefaultPipeParams.AnisotropicLevel = 0;
	}

	if(refDevice)
	{
		plPipeline::fDefaultPipeParams.TextureQuality = 0;
		plPipeline::fDefaultPipeParams.VideoQuality = 0;

	}
	else
	{
		plPipeline::fDefaultPipeParams.TextureQuality = psMajor >= 2 ? 2 : 1;
		plPipeline::fDefaultPipeParams.VideoQuality = pixelshaders ? 2 : 1;
	}
	plPipeline::fDefaultPipeParams.VSync = false;

	// card specific overrides
	if(strstr(rec->GetDriverDesc(), "FX 5200"))
	{
		plPipeline::fDefaultPipeParams.AntiAliasingAmount = 0;
	}


	int val = 0;
	hsStream *stream = nil;
	hsUNIXStream s;
	wchar audioIniFile[MAX_PATH], graphicsIniFile[MAX_PATH];
	PathGetInitDirectory(audioIniFile, arrsize(audioIniFile));
	StrCopy(graphicsIniFile, audioIniFile, arrsize(audioIniFile));
	PathAddFilename(audioIniFile, audioIniFile, L"audio.ini", arrsize(audioIniFile));
	PathAddFilename(graphicsIniFile, graphicsIniFile, L"graphics.ini", arrsize(graphicsIniFile));

#ifndef PLASMA_EXTERNAL_RELEASE
	// internal builds can use the local dir
	if (PathDoesFileExist(L"init//audio.ini"))
		StrCopy(audioIniFile, L"init//audio.ini", arrsize(audioIniFile));
	if (PathDoesFileExist(L"init//graphics.ini"))
		StrCopy(graphicsIniFile, L"init//graphics.ini", arrsize(audioIniFile));
#endif

	//check to see if audio.ini exists
	if (s.Open(audioIniFile))
	{
		s.Close();
	}
	else
	{
		stream = plEncryptedStream::OpenEncryptedFileWrite(audioIniFile);

		plAudioCaps caps = plAudioCapsDetector::Detect(false, true);
		val = 6;
		if( (hsPhysicalMemory() < 256) || plProfileManager::Instance().GetProcessorSpeed() < 1350000000)
		{
			val = 3;
		}

		char deviceName[256];
		sprintf(deviceName, "\"%s\"", DEFAULT_AUDIO_DEVICE_NAME);

		WriteBool(stream, "Audio.Initialize",  caps.IsAvailable());
		WriteBool(stream, "Audio.UseEAX", false);
		WriteInt(stream, "Audio.SetPriorityCutoff", val);
		WriteInt(stream, "Audio.MuteAll", false);
		WriteInt(stream, "Audio.SetChannelVolume SoundFX", 1);
		WriteInt(stream, "Audio.SetChannelVolume BgndMusic", 1);
		WriteInt(stream, "Audio.SetChannelVolume Ambience", 1);
		WriteInt(stream, "Audio.SetChannelVolume NPCVoice", 1);
		WriteInt(stream, "Audio.EnableVoiceRecording", 1);
		WriteString(stream, "Audio.SetDeviceName", deviceName );
		stream->Close();
		delete stream;
		stream = nil;
	}
	
	// check to see if graphics.ini exists
	if (s.Open(graphicsIniFile))
	{
		s.Close();
	}
	else
	{
		IWriteDefaultGraphicsSettings(graphicsIniFile);
	}
}

void plClient::IWriteDefaultGraphicsSettings(const wchar* destFile)
{
	hsStream *stream = plEncryptedStream::OpenEncryptedFileWrite(destFile);

	WriteInt(stream, "Graphics.Width", plPipeline::fDefaultPipeParams.Width);
	WriteInt(stream, "Graphics.Height", plPipeline::fDefaultPipeParams.Height);
	WriteInt(stream, "Graphics.ColorDepth", plPipeline::fDefaultPipeParams.ColorDepth);
	WriteBool(stream, "Graphics.Windowed", plPipeline::fDefaultPipeParams.Windowed);
	WriteInt(stream, "Graphics.AntiAliasAmount", plPipeline::fDefaultPipeParams.AntiAliasingAmount);
	WriteInt(stream, "Graphics.AnisotropicLevel", plPipeline::fDefaultPipeParams.AnisotropicLevel );
	WriteInt(stream, "Graphics.TextureQuality",plPipeline::fDefaultPipeParams.TextureQuality);
	WriteInt(stream, "Quality.Level", plPipeline::fDefaultPipeParams.VideoQuality);
	WriteInt(stream, "Graphics.Shadow.Enable", plPipeline::fDefaultPipeParams.Shadows);
	WriteInt(stream, "Graphics.EnablePlanarReflections", plPipeline::fDefaultPipeParams.PlanarReflections);
	WriteBool(stream, "Graphics.EnableVSync", plPipeline::fDefaultPipeParams.VSync);
	stream->Close();
	delete stream;
	stream = nil;
}


void plClient::WindowActivate(bool active)
{
	if (GetDone())
		return;
		
	if( !fWindowActive != !active )
	{
		if( fInputManager != nil )
			fInputManager->Activate( active );

		plArmatureMod::WindowActivate( active );
	}
	fWindowActive = active;
}


//============================================================================
void plClient::IOnAsyncInitComplete () {
	// Init State Desc Language (files should now be downloaded and in place)
	plSDLMgr::GetInstance()->SetNetApp(plNetClientMgr::GetInstance());
	plSDLMgr::GetInstance()->Init( plSDL::kDisallowTimeStamping );

	PythonInterface::initPython();
	// set the pipeline for the python cyMisc module so that it can do a screen capture
	cyMisc::SetPipeline( fPipeline );

	// Load our custom fonts from our current dat directory
	fFontCache->LoadCustomFonts("dat");
	plWinFontCache::GetInstance().LoadCustomFonts("dat");

	// We'd like to do a SetHoldLoadRequests here, but the GUI stuff doesn't draw right
	// if you try to delay the loading for it.  To work around that, we allocate a
	// global loading bar in advance and set it to a big enough range that when the GUI's
	// are done loading about the right amount of it is filled.
	fNumLoadingRooms++;
	IStartProgress("Loading Global...", 0);

	/// Init the KI
	pfGameGUIMgr	*mgr = pfGameGUIMgr::GetInstance();
	mgr->LoadDialog( "KIBlackBar" );	// load the blackbar which will bootstrap in the rest of the KI dialogs

	// Init the journal book API
	pfJournalBook::SingletonInit();

	SetHoldLoadRequests(true);
	fProgressBar->SetLength(fProgressBar->GetProgress());

	plClothingMgr::Init();
	// Load in any clothing data
	((plResManager*)hsgResMgr::ResMgr())->PageInAge("GlobalClothing");

	pfMarkerMgr::Instance();

	fAnimDebugList = TRACKED_NEW plAnimDebugList();

	/// Now parse final init files (*.fni). These are files just like ini files, only to be run
	/// after all hell has broken loose in the client.
	wchar initFolder[MAX_PATH];
	PathGetInitDirectory(initFolder, arrsize(initFolder));
	pfConsoleDirSrc		dirSrc( fConsoleEngine, initFolder, L"net*.fni" );	// connect to net first
#ifndef PLASMA_EXTERNAL_RELEASE
	// internal builds also parse the local init folder
	dirSrc.ParseDirectory( L"init", L"net*.fni" );
#endif

	dirSrc.ParseDirectory( initFolder, L"*.fni" );
#ifndef PLASMA_EXTERNAL_RELEASE
	// internal builds also parse the local init folder
	dirSrc.ParseDirectory( L"init", L"*.fni" );
#endif

	// run fni in the Aux Init dir
	if (fpAuxInitDir)
	{	
		dirSrc.ParseDirectory(fpAuxInitDir, "net*.fni" );	// connect to net first
		dirSrc.ParseDirectory(fpAuxInitDir, "*.fni" );
	}

	fNumLoadingRooms--;

	((plResManager*)hsgResMgr::ResMgr())->PageInAge("GlobalAnimations");
	SetHoldLoadRequests(false);

	// Tell the transition manager to start faded out. This is so we don't
	// get a frame or two of non-faded drawing before we do our initial fade in
	(void)(TRACKED_NEW plTransitionMsg( plTransitionMsg::kFadeOut, 0.0f, true ))->Send();

	fFlags.SetBit(kFlagAsyncInitComplete);
	if (fFlags.IsBitSet(kFlagGlobalDataLoaded))
		ICompleteInit();
}

//============================================================================
void plClient::ICompleteInit () {
	// Reset clear color on the pipeline
//	fPipeline->ClearRenderTarget( &fClearColor, &depth );

	plSimulationMgr::GetInstance()->Resume();				// start the sim at the last possible minute

	fFlags.SetBit( kFlagIniting, false );
	hsStatusMessage("Client init complete.");

	// Tell everyone we're ready to rock.
	plClientMsg* clientMsg = TRACKED_NEW plClientMsg(plClientMsg::kInitComplete);
	clientMsg->SetBCastFlag(plMessage::kBCastByType);
	clientMsg->Send();

	CsrSrvInitialize();
}

//============================================================================
void plClient::IHandlePreloaderMsg (plPreloaderMsg * msg) {

	plgDispatch::Dispatch()->UnRegisterForExactType(plPreloaderMsg::Index(), GetKey());
	
	if (!msg->fSuccess) {
		char str[1024];
		StrPrintf(
			str,
			arrsize(str),
			"Secure file preloader failed"
		);
		plNetClientApp::GetInstance()->QueueDisableNet(true, str);
		return;
	}
	
	fPatchGlobalAges = true;
}

//============================================================================
void plClient::IHandleNetCommAuthMsg (plNetCommAuthMsg * msg) {

	plgDispatch::Dispatch()->UnRegisterForExactType(plNetCommAuthMsg::Index(), GetKey());

	if (IS_NET_ERROR(msg->result)) {
		char str[1024];
		StrPrintf(
			str,
			arrsize(str),
			// fmt
			"Authentication failed: NetError %u, %S.\n"
			,// values
			msg->result,
			NetErrorToString(msg->result)
		);
		plNetClientApp::GetInstance()->QueueDisableNet(true, str);
		return;
	}

	plgDispatch::Dispatch()->RegisterForExactType(plPreloaderMsg::Index(), GetKey());

	// Precache our secure files
	pfSecurePreloader::GetInstance()->RequestFileGroup(L"Python", L"pak");
	pfSecurePreloader::GetInstance()->RequestFileGroup(L"SDL", L"sdl");
	pfSecurePreloader::GetInstance()->Start();
}
