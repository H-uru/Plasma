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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#pragma warning(disable: 4284)
#include "HeadSpin.h"
#include "hsWindows.h"
#include "plClient.h"
#include "hsStream.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plKeyFinder.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnMessage/plRefMsg.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "plScene/plSceneNode.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plClientMsg.h"
#include "pfCamera/plVirtualCamNeu.h"
#include "hsTimer.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plFile/plEncryptedStream.h"
#include "plInputCore/plInputManager.h"
#include "plInputCore/plInputInterfaceMgr.h"
#include "plInputCore/plInputDevice.h"
#include "plPhysX/plSimulationMgr.h"
#include "plNetClient/plNetClientMgr.h"
#include "plAvatar/plAvatarMgr.h"
#include "plScene/plRelevanceMgr.h"
#include "plTimerCallbackManager.h"
#include "pfAudio/plListener.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "plMessage/plRoomLoadNotifyMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plCameraMsg.h"
#include "plMessage/plTransitionMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plNetCommMsgs.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plResPatcherMsg.h"

#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfConsole/pfConsole.h"
#include "pfConsole/pfConsoleDirSrc.h"
#include "plScene/plPageTreeMgr.h"
#include "plScene/plVisMgr.h"

#include "plAudio/plAudioSystem.h"

#include "plStatGather/plProfileManagerFull.h"

#include "plPipeline.h"
#include "plPipeDebugFlags.h"
#include "plPipeline/plPipelineCreate.h"
#include "plPipeline/plTransitionMgr.h"
#include "plPipeline/plCaptureRender.h"
#include "plPipeline/plDynamicEnvMap.h"
#include "plNetClient/plLinkEffectsMgr.h"
#include "plAvatar/plAvatarClothing.h"
#include "plAvatar/plArmatureMod.h"
#include "pnMessage/plProxyDrawMsg.h"

#include "plScene/plRenderRequest.h"
#include "plDrawable/plAccessGeometry.h"
#include "plPipeResReq.h"
#include "plDrawable/plVisLOSMgr.h"

#include "plGImage/plBitmap.h"

#include "plStatusLog/plStatusLog.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plPipeline/plDTProgressMgr.h"
#include "pfMoviePlayer/plMoviePlayer.h"
#include "plMessage/plMovieMsg.h"

#include "plSDL/plSDL.h"

#include "pnDispatch/plDispatch.h"
#include "pnDispatch/plDispatchLogBase.h"
#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "pfPython/cyMisc.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plRenderRequestMsg.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "plModifier/plSimpleModifier.h"
#include "plAudible.h"
#include "plMessage/plAnimCmdMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "pnMessage/plAudioSysMsg.h"
#include "plMessage/plRenderMsg.h"
#include "plAgeLoader/plResPatcher.h"
#include "pfPython/cyPythonInterface.h"
#include "plUnifiedTime/plClientUnifiedTime.h"
#include "pfAnimation/plAnimDebugList.h"
#include "pfGameGUIMgr/pfGUICtrlGenerator.h"

#include "plGImage/plFontCache.h"

#include "pfJournalBook/pfJournalBook.h"

#include "plAnimation/plAGAnimInstance.h"
#include "plAgeLoader/plAgeLoader.h"

#include "plQuality.h"
#include "plGLight/plShadowCaster.h"

#include "plNetClient/plNetLinkingMgr.h"
#include "plNetCommon/plNetCommonConstants.h"
#include "plNetGameLib/plNetGameLib.h"

#include "pfLocalizationMgr/pfLocalizationMgr.h"
#include "pfPatcher/plManifests.h"

#include "plTweak.h"

#define MSG_LOADING_BAR

// static hsVector3 gAbsDown(0,0,-1.f);

static plDispatchBase* gDisp = nil;
static plTimerCallbackManager* gTimerMgr = nil;

#ifdef HS_BUILD_FOR_WIN32
extern ITaskbarList3* gTaskbarList;
#endif

bool plClient::fDelayMS = false;

plClient* plClient::fInstance=nil;

static hsTArray<HMODULE>        fLoadedDLLs;

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
    fWindowActive(false),
    fAnimDebugList(nil),
    fClampCap(-1),
    fQuality(0),
    fPageMgr(nil),
    fFontCache(nil),
    fHoldLoadRequests(false),
    fNumLoadingRooms(0),
    fNumPostLoadMsgs(0),
    fPostLoadMsgInc(0.f)
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
//  hsTimer::SetRealTime(false);
    hsTimer::SetTimeClamp(0.1f);
#else // HS_DEBUGGING
//  hsTimer::SetRealTime(true);
    hsTimer::SetTimeClamp(0);
#endif // HS_DEBUGGING
    
    IDetectAudioVideoSettings();        // need to do this before the console is created

    /// allow console commands to start working early
    // Create the console engine
    fConsoleEngine = new pfConsoleEngine();
    
    // create network mgr before console runs
    plNetClientMgr::SetInstance(new plNetClientMgr);
    plAgeLoader::SetInstance(new plAgeLoader);

    // Use it to parse the init directory
    plFileName initFolder = plFileSystem::GetInitPath();
    pfConsoleDirSrc dirSrc(fConsoleEngine, initFolder, "*.ini");

#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds also parse the local init folder
    dirSrc.ParseDirectory("init", "*.ini");
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

#include "plGImage/plAVIWriter.h"
#include "pfCharacter/pfMarkerMgr.h"

template<typename T>
static void IUnRegisterAs(T*& ko, plFixedKeyId id)
{
    if (ko) {
        ko->UnRegisterAs(id);
        ko = nullptr;
    }
}

bool plClient::Shutdown()
{
    plSynchEnabler ps(false);   // disable dirty state tracking during shutdown 
    delete fProgressBar;

    // Just in case, clear this out (trying to fix a crash bug where this is still active at shutdown)
    plDispatch::SetMsgRecieveCallback(nil);

    // Let the resmanager know we're going to be shutting down.
    hsgResMgr::ResMgr()->BeginShutdown();

    // Must kill off all movies before shutting down audio.
    IKillMovies();

    plgAudioSys::Activate(false);

    // Get any proxies to commit suicide.
    plProxyDrawMsg* nuke = new plProxyDrawMsg(plProxyDrawMsg::kAllTypes
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
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
    if( mgr )
        mgr->UnloadDialog( "KIBlackBar" );  // unload the blackbar which will bootstrap in the rest of the KI dialogs

    // Take down our GUI control generator
    pfGUICtrlGenerator::Instance().Shutdown();

    if (plNetClientMgr* nc = plNetClientMgr::GetInstance())
        nc->Shutdown();
    if (plAgeLoader* al = plAgeLoader::GetInstance())
        al->Shutdown();

    IUnRegisterAs(fInputManager, kInput_KEY);
    IUnRegisterAs(fGameGUIMgr, kGameGUIMgr_KEY);

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

    IUnRegisterAs(fTransitionMgr, kTransitionMgr_KEY);

    delete fConsoleEngine;
    fConsoleEngine = nil;

    IUnRegisterAs(fLinkEffectsMgr, kLinkEffectsMgr_KEY);

    plClothingMgr::DeInit();

    IUnRegisterAs(fFontCache, kFontCache_KEY);

    pfMarkerMgr::Shutdown();

    delete fAnimDebugList;

    IUnRegisterAs(fConsole, kConsoleObject_KEY);

    PythonInterface::finiPython();

    IUnRegisterAs(fNewCamera, kVirtualCamera1_KEY);

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
        pfConsoleDirSrc     dirSrc( fConsoleEngine, fpAuxInitDir, "*.ini" );
}

void plClient::InitInputs()
{
    hsStatusMessage("InitInputs client\n");
    fInputManager = new plInputManager( fWindowHndl );
    fInputManager->CreateInterfaceMod(fPipeline);
    fInputManager->RegisterAs( kInput_KEY );
    plgDispatch::Dispatch()->RegisterForExactType(plIMouseXEventMsg::Index(), fInputManager->GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plIMouseYEventMsg::Index(), fInputManager->GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plIMouseBEventMsg::Index(), fInputManager->GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), fInputManager->GetKey());
    plInputDevice* pKeyboard = new plKeyboardDevice();
    fInputManager->AddInputDevice(pKeyboard);
    
    plInputDevice* pMouse = new plMouseDevice();
    fInputManager->AddInputDevice(pMouse);

    if( fWindowActive )
        fInputManager->Activate( true );
}

void plClient::ISetGraphicsDefaults()
{
    // couldn't find display mode set defaults write to ini file
    plFileName graphicsIniFile = plFileName::Join(plFileSystem::GetInitPath(), "graphics.ini");
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

bool plClient::InitPipeline()
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

    float   yon = 500.0f;

    pipe->SetFOV(60.f, 60.f * (float)pipe->Height() / (float)pipe->Width());
    pipe->SetDepth(0.3f, yon);

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
void    plClient::SetClearColor( hsColorRGBA &color )
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
bool plClient::MsgReceive(plMessage* msg)
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
            plStatusLog::AddLineSF( "pageouts.log", ".. ClientRefMsg received for room {}", pRefMsg->GetRef() ? pRefMsg->GetRef()->GetKey()->GetUoid().GetObjectName() : ST_LITERAL("nilref") );
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
                    bool found=false;
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
                    plPipeline::fDefaultPipeParams.AntiAliasingAmount, plPipeline::fDefaultPipeParams.AnisotropicLevel, plPipeline::fDefaultPipeParams.VSync);
            }
            break;

        case plClientMsg::kFlashWindow:
            {
                FlashWindow();
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
        ST::string str = ST::format("Callback event from {}\n", callback->GetSender()
                            ? callback->GetSender()->GetName()
                            : ST_LITERAL("Unknown"));
        hsStatusMessage(str.c_str());
        static int gotten = 0;
        if( ++gotten > 5 )
        {
            plSimpleModifier* simpMod = plSimpleModifier::ConvertNoRef(callback->GetSender()->ObjectIsLoaded());
            plAudible* aud = plAudible::ConvertNoRef(callback->GetSender()->ObjectIsLoaded());
            if( simpMod )
            {
                plAnimCmdMsg* cmd = new plAnimCmdMsg;
                cmd->AddReceiver(simpMod->GetKey());
                cmd->SetCmd(plAnimCmdMsg::kRemoveCallbacks);
                cmd->AddCallback(callback);
                plgDispatch::MsgSend(cmd);
                hsRefCnt_SafeUnRef(callback);
            }
            else if( aud )
            {
                plSoundMsg* cmd = new plSoundMsg;
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
    // plResPatcherMsg
    //============================================================================
    if (plResPatcherMsg * resMsg = plResPatcherMsg::ConvertNoRef(msg)) {
        IHandlePatcherMsg(resMsg);
        return true;
    }

    //============================================================================
    // plNetCommAuthMsg
    //============================================================================
    if (plNetCommAuthMsg* authMsg = plNetCommAuthMsg::ConvertNoRef(msg)) {
        plgDispatch::Dispatch()->UnRegisterForExactType(plNetCommAuthMsg::Index(), GetKey());
        if (IS_NET_SUCCESS(authMsg->result))
            IPatchGlobalAgeFiles();
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

//============================================================================
bool plClient::IHandleMovieMsg(plMovieMsg* mov)
{
    if (mov->GetFileName().empty())
        return true;

    size_t i = fMovies.size();
    if (!(mov->GetCmd() & plMovieMsg::kMake))
    {
        for (i = 0; i < fMovies.size(); i++)
        {
            if (mov->GetFileName().compare_i(fMovies[i]->GetFileName().AsString()) == 0)
                break;
        }
    }
    if (i == fMovies.size())
    {
        fMovies.push_back(new plMoviePlayer());
        fMovies[i]->SetFileName(mov->GetFileName());
    }

    if (mov->GetCmd() & plMovieMsg::kAddCallbacks)
    {
        int j;
        for (j = 0; j < mov->GetNumCallbacks(); j++)
            fMovies[i]->AddCallback(mov->GetCallback(j));
    }
    if (mov->GetCmd() & plMovieMsg::kMove)
        fMovies[i]->SetPosition(mov->GetCenter());
    if (mov->GetCmd() & plMovieMsg::kScale)
        fMovies[i]->SetScale(mov->GetScale());
    if (mov->GetCmd() & plMovieMsg::kColorAndOpacity)
        fMovies[i]->SetColor(mov->GetColor());
    if (mov->GetCmd() & plMovieMsg::kColor)
    {
        hsColorRGBA c = fMovies[i]->GetColor();
        c.Set(mov->GetColor().r, mov->GetColor().g, mov->GetColor().b, c.a);
        fMovies[i]->SetColor(c);
    }
    if (mov->GetCmd() & plMovieMsg::kOpacity)
    {
        hsColorRGBA c = fMovies[i]->GetColor();
        c.a = mov->GetColor().a;
        fMovies[i]->SetColor(c);
    }
    if (mov->GetCmd() & plMovieMsg::kFadeIn)
    {
        fMovies[i]->SetFadeFromColor(mov->GetFadeInColor());
        fMovies[i]->SetFadeFromTime(mov->GetFadeInSecs());
    }
    if (mov->GetCmd() & plMovieMsg::kFadeOut)
    {
        fMovies[i]->SetFadeToColor(mov->GetFadeOutColor());
        fMovies[i]->SetFadeToTime(mov->GetFadeOutSecs());
    }
    if (mov->GetCmd() & plMovieMsg::kVolume)
        fMovies[i]->SetVolume(mov->GetVolume());

    if (mov->GetCmd() & plMovieMsg::kStart)
        fMovies[i]->Start();
    if (mov->GetCmd() & plMovieMsg::kPause)
        fMovies[i]->Pause(true);
    if (mov->GetCmd() & plMovieMsg::kResume)
        fMovies[i]->Pause(false);
    if (mov->GetCmd() & plMovieMsg::kStop)
        fMovies[i]->Stop();

    // If a movie has lost its filename, it means something went horribly wrong
    // with playing it and it has shutdown. Or we just stopped it. Either way,
    // we need to clear it out of our list.
    if (!fMovies[i]->GetFileName().IsValid())
    {
        delete fMovies[i];
        fMovies[i] = fMovies.back();
        fMovies.pop_back();
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

#include "plResMgr/plPageInfo.h"

void plClient::IQueueRoomLoad(const std::vector<plLocation>& locs, bool hold)
{
    bool allSameAge = true;
    ST::string lastAgeName;

    uint32_t numRooms = 0;
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
                hsStatusMessageF("Ignoring LoadRoom request for %s-%s, since room is already loaded", info->GetAge().c_str(), info->GetPage().c_str());
            else if (isLoading)
                hsStatusMessageF("Ignoring LoadRoom request for %s-%s, since room is currently loading", info->GetAge().c_str(), info->GetPage().c_str());
            #endif

            continue;
        }

        fLoadRooms.push_back(new LoadRequest(loc, hold));

        if (lastAgeName.empty() || info->GetAge() == lastAgeName)
            lastAgeName = info->GetAge();
        else
            allSameAge = false;

//      hsStatusMessageF("+++ Loading room %s-%s", info.GetAge(), info.GetPage());
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
        bool isLoading = IIsRoomLoading(req->loc);
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
        plClientRefMsg* pRefMsg = new plClientRefMsg(GetKey(),
            plRefMsg::kOnCreate, -1,
            req->hold ? plClientRefMsg::kLoadRoomHold : plClientRefMsg::kLoadRoom);

        fRoomsLoading.push_back(req->loc); // flag the location as currently loading

        // PageInPage is not guaranteed to finish synchronously, just FYI
        plResManager *mgr = (plResManager *)hsgResMgr::ResMgr();
        mgr->PageInRoom(req->loc, plSceneNode::Index(), pRefMsg);

        delete req;

        plClientMsg* nextRoom = new plClientMsg(plClientMsg::kLoadNextRoom);
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
                plStatusLog::AddLineSF("pageouts.log", "SceneNode for {} loaded; Removing node",
                                       node->GetKey()->GetUoid().GetObjectName());
                #endif
                fPageMgr->RemoveNode(node);
            }
            else
            {
                #ifndef PLASMA_EXTERNAL_RELEASE
                plStatusLog::AddLineSF("pageouts.log", "SceneNode for {} NOT loaded",
                                       nodeKey->GetUoid().GetObjectName());
                #endif
            }
            GetKey()->Release(nodeKey);     // release notify interest in scene node
        
            uint32_t recFlags = 0;
            if (roomIdx != -1)
            {
                recFlags = fRooms[roomIdx].fFlags;
                fRooms.Remove(roomIdx);
            }

            if (node == fCurrentNode)
                fCurrentNode = nil;

            #ifndef PLASMA_EXTERNAL_RELEASE
            plStatusLog::AddLineSF("pageouts.log", "Telling netClientMgr about paging out {}",
                                   nodeKey->GetUoid().GetObjectName());
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
//          plStatusLog::AddLineSF("pageouts.log", "++ Can't find node key for paging out room {}, loc 0x{x}",
//              pMsg->GetRoomName() != nil ? pMsg->GetRoomName() : "",
//              loc.GetSequenceNumber());
            #endif
        }
    }
}

void plClient::IRoomLoaded(plSceneNode* node, bool hold)
{
    fCurrentNode = node; 
    // make sure we don't already have this room in the list:
    bool bAppend = true;
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
                { "BahroCave",          2600    },
                { "BaronCityOffice",    670     },
                { "city",               269000  },
                { "Cleft",              11000   },
                { "Garden",             19700   },
                { "Garrison",           28800   },
                { "Gira",               3300    },
                { "Kadish",             19700   },
                { "Neighborhood",       19900   },
                { "Nexus",              1400    },
                { "Personal",           20300   },
                { "Teledahn",           48000   }
            };

            char name[256];
            strcpy(name, &fProgressBar->GetTitle().c_str()[strlen("Loading ")]);
            name[strlen(name)-3] = '\0';

            // Get the precalculated value for how many messages will be
            // sent out before the screen actually fades in
            int numMsgs = 0;
            for (int i = 0; i < sizeof(ageMsgCount)/sizeof(AgeMsgCount); i++)
            {
                if (strcmp(ageMsgCount[i].AgeName, name) == 0)
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
                plDispatchLogBase::GetInstance()->LogStatusBarChange(fProgressBar->GetTitle().c_str(), "displaying messages");
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
        plRoomLoadNotifyMsg* loadmsg = new plRoomLoadNotifyMsg;
        loadmsg->SetRoom(pRmKey);
        loadmsg->SetWhatHappen(plRoomLoadNotifyMsg::kLoaded);
        plgDispatch::MsgSend(loadmsg);
    }
    else
        hsStatusMessageF("Done loading hold room %s, t=%f\n", pRmKey->GetName().c_str(), hsTimer::GetSeconds());

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
    plRoomLoadNotifyMsg* loadmsg = new plRoomLoadNotifyMsg;
    loadmsg->SetRoom(pRmKey);
    loadmsg->SetWhatHappen(plRoomLoadNotifyMsg::kUnloaded);
    plgDispatch::MsgSend(loadmsg);
}

void plClient::IReadKeyedObjCallback(plKey key)
{
    fInstance->IIncProgress(1, key->GetName().c_str());
}

//============================================================================
void plClient::IProgressMgrCallbackProc(plOperationProgress * progress)
{
    if(!fInstance)
        return;

    // Increments the taskbar progress [Windows 7+]
#ifdef HS_BUILD_FOR_WIN32
    if (gTaskbarList && fInstance->GetWindowHandle())
    {
        static TBPFLAG lastState = TBPF_NOPROGRESS;
        TBPFLAG myState;

        // So, calling making these kernel calls is kind of SLOW. So, let's
        // hide that behind a userland check--this helps linking go faster!
        if (progress->IsAborting())
            myState = TBPF_ERROR;
        else if (progress->IsLastUpdate())
            myState = TBPF_NOPROGRESS;
        else if (progress->GetMax() == 0.f)
            myState = TBPF_INDETERMINATE;
        else
            myState = TBPF_NORMAL;

        if (myState == TBPF_NORMAL)
            // This sets us to TBPF_NORMAL
            gTaskbarList->SetProgressValue(fInstance->GetWindowHandle(), (ULONGLONG)progress->GetProgress(), (ULONGLONG)progress->GetMax());
        else if (myState != lastState)
            gTaskbarList->SetProgressState(fInstance->GetWindowHandle(), myState);
        lastState = myState;
    }
#endif

    fInstance->fMessagePumpProc();

    // HACK HACK HACK HACK!
    // Yes, this is the ORIGINAL, EVIL famerate limit from plClient::IDraw (except I bumped it to 60fps)
    // As it so happens, this callback is happening in the main resource loading thread
    // Without this NASTY ASS HACK, we draw after loading every KO, which starves the loader.
    // At some point, a better solution should be found... Like running the loader in a separate thread.
    static float lastDrawTime;
    static const float kMaxFrameRate = 1.f/60.f;
    float currTime = (float) hsTimer::GetSeconds();
    if ((currTime - lastDrawTime) > kMaxFrameRate)
    {
        fInstance->IDraw();
        lastDrawTime = currTime;
    }
}

//============================================================================
void plClient::IIncProgress (float byHowMuch, const char * text)
{
    if (fProgressBar) {
#ifndef PLASMA_EXTERNAL_RELEASE
        fProgressBar->SetStatusText( text );
#endif
        fProgressBar->Increment( byHowMuch );
    }
}

//============================================================================
void    plClient::IStartProgress( const char *title, float len )
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
            plDispatchLogBase::GetInstance()->LogStatusBarChange(fProgressBar->GetTitle().c_str(), "starting");
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
void    plClient::IStopProgress()
{
    if (fProgressBar)
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (plDispatchLogBase::IsLogging())
            plDispatchLogBase::GetInstance()->LogStatusBarChange(fProgressBar->GetTitle().c_str(), "done");
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

extern  bool    gDataServerLocal;

#include "plQuality.h"
#include "plLoadMask.h"

//============================================================================
bool plClient::StartInit()
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

    plgAudioSys::Init();

    RegisterAs( kClient_KEY );

    InitDLLs();
    
    plGlobalVisMgr::Init();
    fPageMgr = new plPageTreeMgr;

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
    /// Note: this can be done last because the console engine was inited first, and
    /// everything in code that works with the console does so through the console engine

    fConsole = new pfConsole();
    pfConsole::SetPipeline( fPipeline );
    fConsole->RegisterAs( kConsoleObject_KEY );     // fixedKey from plFixedKey.h
    fConsole->Init( fConsoleEngine );

    /// Init the font cache
    fFontCache = new plFontCache();

    /// Init the transition manager
    fTransitionMgr = new plTransitionMgr();
    fTransitionMgr->RegisterAs( kTransitionMgr_KEY );       // fixedKey from plFixedKey.h
    fTransitionMgr->Init();

    // Init the Age Linking effects manager
    fLinkEffectsMgr = new plLinkEffectsMgr();
    fLinkEffectsMgr->RegisterAs( kLinkEffectsMgr_KEY ); // fixedKey from plFixedKey.h
    fLinkEffectsMgr->Init();
    
    /// Init the in-game GUI manager
    fGameGUIMgr = new pfGameGUIMgr();
    fGameGUIMgr->RegisterAs( kGameGUIMgr_KEY );
    fGameGUIMgr->Init();

    plgAudioSys::Activate(true);

    //
    // Init Net before loading things
    //
    plgDispatch::Dispatch()->RegisterForExactType(plNetCommAuthMsg::Index(), GetKey());
    plNetClientMgr::GetInstance()->RegisterAs(kNetClientMgr_KEY);
    plAgeLoader::GetInstance()->Init();

    plCmdIfaceModMsg* pModMsg2 = new plCmdIfaceModMsg;
    pModMsg2->SetBCastFlag(plMessage::kBCastByExactType);
    pModMsg2->SetSender(fConsole->GetKey());
    pModMsg2->SetCmd(plCmdIfaceModMsg::kAdd);
    plgDispatch::MsgSend(pModMsg2);

    // create new virtual camera
    fNewCamera = new plVirtualCam1;
    fNewCamera->RegisterAs( kVirtualCamera1_KEY ); 
    fNewCamera->Init();
    fNewCamera->SetPipeline( GetPipeline() );

    plVirtualCam1::Refresh();
    pfGameGUIMgr::GetInstance()->SetAspectRatio( (float)fPipeline->Width() / (float)fPipeline->Height() );
    plMouseDevice::Instance()->SetDisplayResolution((float)fPipeline->Width(), (float)fPipeline->Height());
    plInputManager::SetRecenterMouse(false);

    plgDispatch::Dispatch()->RegisterForExactType(plMovieMsg::Index(), GetKey());

    // create the listener for the audio system:
    plListener* pLMod = new plListener;
    pLMod->RegisterAs(kListenerMod_KEY );

    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), pLMod->GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plAudioSysMsg::Index(), pLMod->GetKey());

    plSynchedObject::PushSynchDisabled(false);      // enable dirty tracking
    return true;
}

//============================================================================
bool plClient::BeginGame()
{
    plNetClientMgr::GetInstance()->Init();
    IPlayIntroMovie("avi/CyanWorlds.webm", 0.f, 0.f, 0.f, 1.f, 1.f, 0.75);
    if (GetDone()) return false;
    if (NetCommGetStartupAge()->ageDatasetName.compare_i("StartUp") == 0) {
        // This is needed because there is no auth step in this case
        plNetCommAuthMsg* msg = new plNetCommAuthMsg();
        msg->result = kNetSuccess;
        msg->param = nullptr;
        msg->Send();
    }
    return true;
}

//============================================================================
void    plClient::IPatchGlobalAgeFiles()
{
    plgDispatch::Dispatch()->RegisterForExactType(plResPatcherMsg::Index(), GetKey());

    plResPatcher* patcher = plResPatcher::GetInstance();
    patcher->Update(plManifest::EssentialGameManifests());
}

void plClient::InitDLLs()
{
    hsStatusMessage("Init dlls client\n");
    typedef void (*PInitGlobalsFunc) (hsResMgr *, plFactory *, plTimerCallbackManager *, plTimerShare*,
        plNetClientApp*);

    std::vector<plFileName> dlls = plFileSystem::ListDir("ModDLL", "*.dll");
    for (auto iter = dlls.begin(); iter != dlls.end(); ++iter)
    {
        HMODULE hMod = LoadLibraryW(iter->WideString().data());
        if (hMod)
        {
            PInitGlobalsFunc initGlobals = (PInitGlobalsFunc)GetProcAddress(hMod, "InitGlobals");
            (*initGlobals)(hsgResMgr::ResMgr(), plFactory::GetTheFactory(), plgTimerCallbackMgr::Mgr(),
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

bool plClient::MainLoop()
{
#if defined(HAVE_CYPYTHONIDE) && !defined(PLASMA_EXTERNAL_RELEASE)
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


bool plClient::IUpdate()
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
    plClientUnifiedTime::SetSysTime();  // keep a unified time, based on sysSeconds
    // Time may have been clamped in IncSysSeconds, depending on hsTimer's current mode.

    double currTime = hsTimer::GetSysSeconds();
    float delSecs = hsTimer::GetDelSysSeconds();

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
    plTimeMsg* msg = new plTimeMsg(nil, nil, nil, nil);
    plgDispatch::MsgSend(msg);
    plProfile_EndTiming(TimeMsg);

    plProfile_BeginTiming(EvalMsg);
    plEvalMsg* eval = new plEvalMsg(nil, nil, nil, nil);
    plgDispatch::MsgSend(eval);
    plProfile_EndTiming(EvalMsg);

    char *xFormLap1 = "Main";
    plProfile_BeginLap(TransformMsg, xFormLap1);
    plTransformMsg* xform = new plTransformMsg(nil, nil, nil, nil);
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
        xform = new plTransformMsg(nil, nil, nil, nil);
        plgDispatch::MsgSend(xform);
        plProfile_EndLap(TransformMsg, xFormLap2);
    }
    else
    {
        char *xFormLap3 = "Delayed";
        plProfile_BeginLap(TransformMsg, xFormLap3);
        xform = new plDelayedTransformMsg(nil, nil, nil, nil);
        plgDispatch::MsgSend(xform);
        plProfile_EndLap(TransformMsg, xFormLap3);
    }

    plCoordinateInterface::SetTransformPhase(plCoordinateInterface::kTransformPhaseNormal);
    
    plProfile_BeginTiming(CameraMsg);
    plCameraMsg* cameras = new plCameraMsg;
    cameras->SetCmd(plCameraMsg::kUpdateCameras);
    cameras->SetBCastFlag(plMessage::kBCastByExactType);
    plgDispatch::MsgSend(cameras);
    plProfile_EndTiming(CameraMsg);

    return false;
}

bool plClient::IDrawProgress() {
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

bool plClient::IDraw()
{
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
    plRenderMsg* rendMsg = new plRenderMsg(fPipeline);
    plgDispatch::MsgSend(rendMsg);
    plProfile_EndTiming(RenderMsg);

    plPreResourceMsg* preMsg = new plPreResourceMsg(fPipeline);
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
    for (size_t i = 0; i < fMovies.size(); i++)
    {
        if (!fMovies[i]->NextFrame())
        {
            delete fMovies[i];
            fMovies[i] = fMovies.back();
            fMovies.pop_back();
            i--;
        }
    }
}

void plClient::IKillMovies()
{
    for (size_t i = 0; i < fMovies.size(); i++)
        delete fMovies[i];
    fMovies.clear();
}

bool plClient::IPlayIntroMovie(const char* movieName, float endDelay, float posX, float posY, float scaleX, float scaleY, float volume /* = 1.0 */)
{
    SetQuitIntro(false);
    plMoviePlayer player;
    player.SetPosition(posX, posY);
    player.SetScale(scaleX, scaleY);
    player.SetFileName(movieName);
    player.SetFadeToTime(endDelay);
    player.SetFadeToColor(hsColorRGBA().Set(0, 0, 0, 1.f));
    player.SetVolume(volume);
    bool firstTry = true;  // flag to make sure that we don't quit before we even start

    if (player.Start())
    {
        while (true)
        {
            if (fInstance)
                fInstance->fMessagePumpProc();

            if (GetDone())
                return true;
            if (firstTry)
            {
                firstTry = false;
                SetQuitIntro(false);
            }
            else
            {
                if (GetQuitIntro())
                    return true;
            }

            bool done = false;
            if (!fPipeline->BeginRender())
            {
                fPipeline->ClearRenderTarget();
                done = !player.NextFrame();

                fPipeline->RenderScreenElements();
                fPipeline->EndRender();
            }

            if (done)
                return true;
        }
        return true;
    }
    return false;
}

bool plClient::IFlushRenderRequests()
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

void plClient::ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync)
{
    if(!fPipeline) return;

    WindowActivate(false);

    fPipeline->ResetDisplayDevice(Width, Height, ColorDepth, Windowed, NumAASamples, MaxAnisotropicSamples, VSync);

    ResizeDisplayDevice(Width, Height, Windowed);

    WindowActivate(true);
}

void plClient::ResizeDisplayDevice(int Width, int Height, bool Windowed)
{

    if (plMouseDevice::Instance())
        plMouseDevice::Instance()->SetDisplayResolution((float)Width, (float)Height);

    float aspectratio = (float)Width / (float)Height;
    if (pfGameGUIMgr::GetInstance())
        pfGameGUIMgr::GetInstance()->SetAspectRatio( aspectratio );

    // Direct3D no longer uses exclusive fullscreen mode, ergo, we must resize the display
    if (!Windowed)
        IChangeResolution(Width, Height);

    uint32_t winStyle, winExStyle;
    if( Windowed )
    {
        // WS_VISIBLE appears necessary to avoid leaving behind framebuffer junk when going from windowed to a smaller window
        winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
        winExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    } else {
        winStyle = WS_VISIBLE;
        winExStyle = WS_EX_APPWINDOW;
    }
    SetWindowLong(fWindowHndl, GWL_STYLE, winStyle);
    SetWindowLong(fWindowHndl, GWL_EXSTYLE, winExStyle);


    uint32_t flags = SWP_NOCOPYBITS | SWP_SHOWWINDOW | SWP_FRAMECHANGED;
    uint32_t OutsideWidth, OutsideHeight;
    if( Windowed )
    {
        RECT winRect = { 0, 0, Width, Height };
        AdjustWindowRectEx(&winRect, winStyle, false, winExStyle);
        OutsideWidth = winRect.right - winRect.left;
        OutsideHeight = winRect.bottom - winRect.top;
    } else {
        OutsideWidth = Width;
        OutsideHeight = Height;
    }
    SetWindowPos( fWindowHndl, HWND_NOTOPMOST, 0, 0, OutsideWidth, OutsideHeight, flags );
}

void plClient::IChangeResolution(int width, int height)
{
    // First, we need to be mindful that we may not be operating on the primary display device
    // I unfortunately cannot test this works as expected, but it will likely save us some cursing
    HMONITOR monitor = MonitorFromWindow(fWindowHndl, MONITOR_DEFAULTTONULL);
    if (!monitor)
        return;
    MONITORINFOEXW moninfo;
    memset(&moninfo, 0, sizeof(moninfo));
    moninfo.cbSize = sizeof(moninfo);
    GetMonitorInfoW(monitor, &moninfo);

    // Fetch a base display settings
    DEVMODEW devmode;
    memset(&devmode, 0, sizeof(devmode));
    devmode.dmSize = sizeof(devmode);
    EnumDisplaySettingsW(moninfo.szDevice, ENUM_REGISTRY_SETTINGS, &devmode);

    // Actually update the resolution
    if (width != 0 && height != 0) {
        devmode.dmPelsWidth = width;
        devmode.dmPelsHeight = height;
    }
    ChangeDisplaySettingsExW(moninfo.szDevice, &devmode, nullptr, CDS_FULLSCREEN, nullptr);
}

void WriteBool(hsStream *stream, char *name, bool on )
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
    bool devmode = true;
    hsG3DDeviceModeRecord dmr;
    hsG3DDeviceSelector devSel;
    devSel.Enumerate(fWindowHndl);
    devSel.RemoveUnusableDevModes(true);

    if (!devSel.GetDefault(&dmr))
        devmode = false;
    hsG3DDeviceRecord *rec = (hsG3DDeviceRecord *)dmr.GetDevice();
    const hsG3DDeviceMode *mode = dmr.GetMode();

    bool pixelshaders = rec->GetCap(hsG3DDeviceSelector::kCapsPixelShader);

    plPipeline::fDefaultPipeParams.ColorDepth = hsG3DDeviceSelector::kDefaultDepth;
#if defined(HS_DEBUGGING) || defined(DEBUG)
    plPipeline::fDefaultPipeParams.Windowed = true;
#else
    plPipeline::fDefaultPipeParams.Windowed = false;
#endif

    // Use current desktop resolution for fullscreen mode
    if(!plPipeline::fDefaultPipeParams.Windowed)
    {
        plPipeline::fDefaultPipeParams.Width = GetSystemMetrics(SM_CXSCREEN);
        plPipeline::fDefaultPipeParams.Height = GetSystemMetrics(SM_CYSCREEN);
    }
    else
    {
        plPipeline::fDefaultPipeParams.Width = hsG3DDeviceSelector::kDefaultWidth;
        plPipeline::fDefaultPipeParams.Height = hsG3DDeviceSelector::kDefaultHeight;
    }

    plPipeline::fDefaultPipeParams.Shadows = 1;

    // enable planar reflections if pixelshaders are available
    plPipeline::fDefaultPipeParams.PlanarReflections = 1;

    // enable 2x antialiasing and anisotropic to 2 samples if pixelshader version is greater that 2.0
    plPipeline::fDefaultPipeParams.AntiAliasingAmount = rec->GetMaxAnisotropicSamples() ? 2 : 0;
    plPipeline::fDefaultPipeParams.AnisotropicLevel = mode->GetNumFSAATypes() ? 2 : 0;

    plPipeline::fDefaultPipeParams.TextureQuality = pixelshaders ? 2 : 1;
    plPipeline::fDefaultPipeParams.VideoQuality = pixelshaders ? 2 : 1;

    plPipeline::fDefaultPipeParams.VSync = false;

    int val = 0;
    hsStream *stream = nil;
    hsUNIXStream s;
    plFileName audioIniFile = plFileName::Join(plFileSystem::GetInitPath(), "audio.ini");
    plFileName graphicsIniFile = plFileName::Join(plFileSystem::GetInitPath(), "graphics.ini");

#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds can use the local dir
    if (plFileInfo("init/audio.ini").Exists())
        audioIniFile = "init/audio.ini";
    if (plFileInfo("init/graphics.ini").Exists())
        graphicsIniFile = "init/graphics.ini";
#endif

    //check to see if audio.ini exists
    if (s.Open(audioIniFile)) {
        s.Close();
    } else {
        stream = plEncryptedStream::OpenEncryptedFileWrite(audioIniFile);
        WriteBool(stream, "Audio.Initialize",  true);
        WriteBool(stream, "Audio.UseEAX", false);
        WriteInt(stream, "Audio.SetPriorityCutoff", 6);
        WriteInt(stream, "Audio.MuteAll", false);
        WriteInt(stream, "Audio.SetChannelVolume SoundFX", 1);
        WriteInt(stream, "Audio.SetChannelVolume BgndMusic", 1);
        WriteInt(stream, "Audio.SetChannelVolume Ambience", 1);
        WriteInt(stream, "Audio.SetChannelVolume NPCVoice", 1);
        WriteInt(stream, "Audio.EnableVoiceRecording", 1);
        stream->Close();
        delete stream;
    }

    // check to see if graphics.ini exists
    if (s.Open(graphicsIniFile))
        s.Close();
    else
        IWriteDefaultGraphicsSettings(graphicsIniFile);
}

void plClient::IWriteDefaultGraphicsSettings(const plFileName& destFile)
{
    hsStream *stream = plEncryptedStream::OpenEncryptedFileWrite(destFile);

    WriteInt(stream, "Graphics.Width", plPipeline::fDefaultPipeParams.Width);
    WriteInt(stream, "Graphics.Height", plPipeline::fDefaultPipeParams.Height);
    WriteInt(stream, "Graphics.ColorDepth", plPipeline::fDefaultPipeParams.ColorDepth);
    WriteBool(stream, "Graphics.Windowed", plPipeline::fDefaultPipeParams.Windowed);
    WriteInt(stream, "Graphics.AntiAliasAmount", plPipeline::fDefaultPipeParams.AntiAliasingAmount);
    WriteInt(stream, "Graphics.AnisotropicLevel", plPipeline::fDefaultPipeParams.AnisotropicLevel );
    WriteInt(stream, "Graphics.TextureQuality", plPipeline::fDefaultPipeParams.TextureQuality);
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
        
    if (fWindowActive != active ) {
        if (fInputManager)
            fInputManager->Activate(active);
        plArmatureMod::WindowActivate(active);

        // Remember, we are no longer exclusive fullscreen, so we actually have to toggle the desktop resolution
        // whee? wait. WHEEE!
        if (fPipeline->IsFullScreen())
            IChangeResolution(active ? fPipeline->Width() : 0, active ? fPipeline->Height() : 0);
    }
    fWindowActive = active;
}

void plClient::FlashWindow()
{
#ifdef HS_BUILD_FOR_WIN32
    FLASHWINFO info;
    info.cbSize = sizeof(info);
    info.dwFlags = FLASHW_TIMERNOFG | FLASHW_ALL;
    info.hwnd = fWindowHndl;
    info.uCount = -1;
    FlashWindowEx(&info);
#endif
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

    // We'd like to do a SetHoldLoadRequests here, but the GUI stuff doesn't draw right
    // if you try to delay the loading for it.  To work around that, we allocate a
    // global loading bar in advance and set it to a big enough range that when the GUI's
    // are done loading about the right amount of it is filled.
    fNumLoadingRooms++;
    IStartProgress("Loading Global...", 0);

    /// Init the KI
    pfGameGUIMgr    *mgr = pfGameGUIMgr::GetInstance();
    mgr->LoadDialog( "KIBlackBar" );    // load the blackbar which will bootstrap in the rest of the KI dialogs

    // Init the journal book API
    pfJournalBook::SingletonInit();

    SetHoldLoadRequests(true);
    fProgressBar->SetLength(fProgressBar->GetProgress());

    plClothingMgr::Init();
    // Load in any clothing data
    ((plResManager*)hsgResMgr::ResMgr())->PageInAge("GlobalClothing");

    pfMarkerMgr::Instance();

    fAnimDebugList = new plAnimDebugList();

    /// Now parse final init files (*.fni). These are files just like ini files, only to be run
    /// after all hell has broken loose in the client.
    plFileName initFolder = plFileSystem::GetInitPath();
    pfConsoleDirSrc dirSrc(fConsoleEngine, initFolder, "net*.fni");  // connect to net first
#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds also parse the local init folder
    dirSrc.ParseDirectory("init", "net*.fni");
#endif

    dirSrc.ParseDirectory(initFolder, "*.fni");
#ifndef PLASMA_EXTERNAL_RELEASE
    // internal builds also parse the local init folder
    dirSrc.ParseDirectory("init", "*.fni");
#endif

    // run fni in the Aux Init dir
    if (fpAuxInitDir)
    {
        dirSrc.ParseDirectory(fpAuxInitDir, "net*.fni");   // connect to net first
        dirSrc.ParseDirectory(fpAuxInitDir, "*.fni");
    }

    fNumLoadingRooms--;

    ((plResManager*)hsgResMgr::ResMgr())->PageInAge("GlobalAnimations");
    SetHoldLoadRequests(false);

    // Tell the transition manager to start faded out. This is so we don't
    // get a frame or two of non-faded drawing before we do our initial fade in
    (void)(new plTransitionMsg( plTransitionMsg::kFadeOut, 0.0f, true ))->Send();

    fFlags.SetBit(kFlagAsyncInitComplete);
    if (fFlags.IsBitSet(kFlagGlobalDataLoaded))
        ICompleteInit();
}

//============================================================================
void plClient::ICompleteInit () {
    // Reset clear color on the pipeline
//  fPipeline->ClearRenderTarget( &fClearColor, &depth );

    plSimulationMgr::GetInstance()->Resume();               // start the sim at the last possible minute

    fFlags.SetBit( kFlagIniting, false );
    hsStatusMessage("Client init complete.");

    // Tell everyone we're ready to rock.
    plClientMsg* clientMsg = new plClientMsg(plClientMsg::kInitComplete);
    clientMsg->SetBCastFlag(plMessage::kBCastByType);
    clientMsg->Send();
}

//============================================================================
void plClient::IHandlePatcherMsg (plResPatcherMsg * msg) {
    plgDispatch::Dispatch()->UnRegisterForExactType(plResPatcherMsg::Index(), GetKey());

    if (!msg->Success()) {
        plNetClientApp::GetInstance()->QueueDisableNet(true, msg->GetError().c_str());
        return;
    }

    IOnAsyncInitComplete();
}
