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
#include "HeadSpin.h"
#include "hsTimer.h"
#include "plResponderModifier.h"
#include "plResponderSDLModifier.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "plPhysical.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnNetCommon/plNetApp.h"

// for localOnly cmd check
#include "plMessage/plLinkToAgeMsg.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plSoundMsg.h"

#include "plMessage/plResponderMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pfMessage/plArmatureEffectMsg.h"

#include "plStatusLog/plStatusLog.h"

#include "plMessage/plTimerCallbackMsg.h"
#include "plTimerCallbackManager.h"

#include "plMessage/plSimStateMsg.h"
//#include "plHavok1\plHKPhysical.h"
//#include "plHavok1\plHKSubWorld.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarMgr.h"

#include "plPipeline/plDebugText.h"


//#ifdef HS_DEBUGGING
#define STATUS_LOG
//#endif

#ifdef STATUS_LOG
#define ResponderLog(x) x
#else
#define ResponderLog(x)
#endif

void plResponderEnableMsg::Read(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgRead(stream, mgr);
    fEnable = stream->ReadBool();
}

void plResponderEnableMsg::Write(hsStream* stream, hsResMgr* mgr)
{
    plMessage::IMsgWrite(stream, mgr);
    stream->WriteBool(fEnable);
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

plResponderModifier::plResponderModifier() : 
    fCurState(0), 
    fCurCommand(-1), 
    fEnabled(true), 
    fFlags(0), 
    fEnter(false),
    fResponderSDLMod(nil), 
    fGotFirstLoad(false),
    fNotifyMsgFlags(0)
{
}

plResponderModifier::~plResponderModifier()
{
    delete fResponderSDLMod;
    fResponderSDLMod=nil;

    for (int i = 0; i < fStates.Count(); i++)
    {
        for (int j = 0; j < fStates[i].fCmds.Count(); j++ )
            hsRefCnt_SafeUnRef(fStates[i].fCmds[j].fMsg);
    }
}

bool plResponderModifier::MsgReceive(plMessage* msg)
{
    plNotifyMsg* pNMsg = plNotifyMsg::ConvertNoRef(msg);
    if (pNMsg)
    {
        if (pNMsg->fType == plNotifyMsg::kResponderFF)
        {
            ISetResponderStateFromNotify(pNMsg);
            IFastForward(true);
        }
        else if (pNMsg->fType == plNotifyMsg::kResponderChangeState)
        {
            ISetResponderStateFromNotify(pNMsg);
            DirtySynchState(kSDLResponder, 0);
        }
        else
        {
            // assumes state of 0 means untriggered and state of 1 is triggered
            if ((pNMsg->fState != 0 && (fFlags & kDetectTrigger)) ||
                (pNMsg->fState == 0 && (fFlags & kDetectUnTrigger)))
            {
                Trigger(pNMsg);
                DirtySynchState(kSDLResponder, 0);
            }
        }

        return true;
    }

    plResponderEnableMsg *pEnableMsg = plResponderEnableMsg::ConvertNoRef(msg);
    if (pEnableMsg)
    {
        fEnabled = pEnableMsg->fEnable;
        DirtySynchState(kSDLResponder, 0);
        return true;
    }

    plEventCallbackMsg *pEventMsg = plEventCallbackMsg::ConvertNoRef(msg);
    plTimerCallbackMsg *timerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
    if (pEventMsg || timerMsg)
    {
        int32_t waitID = pEventMsg ? pEventMsg->fUser : timerMsg->fID;

        if (waitID >= 0)
        {
            // Flag that this callback completed and try sending in case any commands were waiting on this
            fCompletedEvents.SetBit(waitID);

            ResponderLog(ILog(plStatusLog::kWhite, "Got callback from command %d(id:%d)", ICmdFromWait((int8_t)waitID)+1, waitID));

            IContinueSending();
            DirtySynchState(kSDLResponder, 0);
        }
        // The is one of the stop callbacks we generated for debug mode
        else if (fDebugAnimBox)
            IDebugAnimBox(false);

        return true;
    }

    // pass sdl msg to sdlMod
    plSDLModifierMsg* sdlMsg = plSDLModifierMsg::ConvertNoRef(msg);
    if (sdlMsg && fResponderSDLMod)
    {
        if (fResponderSDLMod->MsgReceive(sdlMsg))
            return true;    // msg handled
    }

    return plSingleModifier::MsgReceive(msg);
}

void plResponderModifier::AddCommand(plMessage* pMsg, int state)
{
    fStates[state].fCmds.Append(plResponderCmd(pMsg, -1));
}

void plResponderModifier::AddCallback(int8_t state, int8_t cmd, int8_t callback)
{
    fStates[state].fWaitToCmd[callback] = cmd;
}

//
// Decide if this cmd should only be run locally.
// If we are triggered remotely (netRequest==true), then
// we don't want to execute localOnly cmds (like cameraChanges)
//
bool plResponderModifier::IIsLocalOnlyCmd(plMessage* cmd)
{
    if (plLinkToAgeMsg::ConvertNoRef(cmd))  // don't want to page out any rooms
        return true;
    if (plCameraMsg::ConvertNoRef(cmd))     // don't want to change our camera
        return true;

    plSoundMsg *snd = plSoundMsg::ConvertNoRef( cmd );
    if( snd != nil && snd->Cmd( plSoundMsg::kIsLocalOnly ) )
        return true;

    return false;
}

void plResponderModifier::ISetResponderState(int8_t state)
{
    // make sure that it is a valid state to switch to
    if (state >= 0 && state < fStates.Count())
    {
        fCurState = state;
    }
    else
    {
        ResponderLog(ILog(plStatusLog::kRed, "Invalid state %d specified, will default to current state", state));
    }
}

void plResponderModifier::ISetResponderStateFromNotify(plNotifyMsg* msg)
{
    // set the state of the responder IF they want it to be
    proResponderStateEventData* event = (proResponderStateEventData*)msg->FindEventRecord(proEventData::kResponderState);
    if (event != nil)
        ISetResponderState((int8_t)(event->fState));
}

void plResponderModifier::Trigger(plNotifyMsg *msg)
{
#if 0
    plNetClientApp::GetInstance()->DebugMsg("RM: Responder {} is triggering, num cmds={}, enabled={}, curCmd={}, t={f}\n",
        GetKeyName(), fStates[fCurState].fCmds.GetCount(),
        fEnabled, fCurCommand, hsTimer::GetSysSeconds());
#endif

    // If we're not in the middle of sending, reset and start sending commands
    if (fCurCommand == int8_t(-1) && fEnabled)
    {
        ResponderLog(ILog(plStatusLog::kGreen, "Trigger"));

        fNotifyMsgFlags = msg->GetAllBCastFlags();
        fTriggerer = msg->GetSender();
        fPlayerKey = msg->GetAvatarKey();

        ISetResponderStateFromNotify(msg);

        proCollisionEventData *cEvent = (proCollisionEventData *)msg->FindEventRecord(proEventData::kCollision);
        fEnter = (cEvent ? cEvent->fEnter : false);

        fCompletedEvents.Reset();
        fCurCommand = 0;

        DirtySynchState(kSDLResponder, 0);

        IContinueSending();
    }
    else
    {
        ResponderLog(ILog(plStatusLog::kRed, "Rejected Trigger, %s", !fEnabled ? "responder disabled" : "responder is running"));
    }
}

bool plResponderModifier::IContinueSending()
{
    // If we haven't been started, exit
    if (fCurCommand == int8_t(-1))
        return false;

    plResponderState& state = fStates[fCurState];

    while (fCurCommand < state.fCmds.Count())
    {
        plMessage *msg = state.fCmds[fCurCommand].fMsg;
        if (msg)
        {
            // If this command needs to wait, and it's condition hasn't been met yet, exit
            int8_t wait = state.fCmds[fCurCommand].fWaitOn;
            if (wait != -1 && !fCompletedEvents.IsBitSet(wait))
            {
                ResponderLog(ILog(plStatusLog::kWhite, "Command %d is waiting for command %d(id:%d)", int8_t(fCurCommand)+1, ICmdFromWait(wait)+1, wait));
                return false;
            }

            if (!(fNotifyMsgFlags & plMessage::kNetNonLocal)|| !IIsLocalOnlyCmd(msg))
            {
                // make sure outgoing msgs inherit net flags as part of cascade
                uint32_t msgFlags = msg->GetAllBCastFlags();
                plNetClientApp::InheritNetMsgFlags(fNotifyMsgFlags, &msgFlags, true);
                msg->SetAllBCastFlags(msgFlags);

                // If this is a responder message, let it know which player triggered this
                if (plResponderMsg* responderMsg = plResponderMsg::ConvertNoRef(msg))
                {
                    responderMsg->fPlayerKey = fPlayerKey;
                }
                else if (plNotifyMsg* notifyMsg = plNotifyMsg::ConvertNoRef(msg))
                {
                    bool foundCollision = false;

                    // If we find a collision event, this message is meant to trigger a multistage
                    for (int i = 0; i < notifyMsg->GetEventCount(); i++)
                    {
                        proEventData* event = notifyMsg->GetEventRecord(i);
                        if (event->fEventType == proEventData::kCollision)
                        {
                            proCollisionEventData* collisionEvent = (proCollisionEventData*)event;
                            collisionEvent->fHitter = fPlayerKey;
                            foundCollision = true;
                        }
                    }

                    // No collision event, this message is for notifying the triggerer
                    if (!foundCollision)
                    {
                        notifyMsg->ClearReceivers();
                        notifyMsg->AddReceiver(fTriggerer);
                    }

                    notifyMsg->SetSender(GetKey());
                }
                else if (plLinkToAgeMsg* linkMsg = plLinkToAgeMsg::ConvertNoRef(msg))
                {
                    if (linkMsg->GetNumReceivers() == 0)
                    {
                        plUoid netUoid(kNetClientMgr_KEY);
                        plKey netKey = hsgResMgr::ResMgr()->FindKey(netUoid);
                        hsAssert(netKey,"NetClientMgr not found");
                        linkMsg->AddReceiver(netKey);
                    }
                }
                else if (plArmatureEffectStateMsg* stateMsg = plArmatureEffectStateMsg::ConvertNoRef(msg))
                {
                    stateMsg->ClearReceivers();
                    stateMsg->AddReceiver(fPlayerKey);
                    stateMsg->fAddSurface = fEnter;
                }
                else if (plSubWorldMsg* swMsg = plSubWorldMsg::ConvertNoRef(msg))
                {
                    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
                    if(avatar)
                    {
                        swMsg->AddReceiver(avatar->GetKey());
                    }
                }

                // If we're in anim debug mode, check if this is an anim play
                // message so we can put up the cue
                if (fDebugAnimBox)
                {
                    plAnimCmdMsg* animMsg = plAnimCmdMsg::ConvertNoRef(msg);
                    if (animMsg && animMsg->Cmd(plAnimCmdMsg::kContinue))
                        IDebugPlayMsg(animMsg);
                }


                if (plTimerCallbackMsg *timerMsg = plTimerCallbackMsg::ConvertNoRef(msg))
                {
                    hsRefCnt_SafeRef(timerMsg);
                    plgTimerCallbackMgr::NewTimer(timerMsg->fTime, timerMsg);
                }
                else
                {
                    hsRefCnt_SafeRef(msg);
                    plgDispatch::MsgSend(msg);
                }
            }
        }

        fCurCommand++;
        DirtySynchState(kSDLResponder, 0);
    }

    // Make sure all callbacks we need to wait on are done before allowing a state switch or restart
    for (int i = 0; i < state.fNumCallbacks; i++)
    {
        if (!fCompletedEvents.IsBitSet(i))
        {
            ResponderLog(ILog(plStatusLog::kWhite, "Can't reset, waiting for command %d(id:%d)", ICmdFromWait(i)+1, i));
            return false;
        }
    }

    ResponderLog(ILog(plStatusLog::kGreen, "Reset"));

    fCurCommand = -1;
    ISetResponderState(state.fSwitchToState);
    DirtySynchState(kSDLResponder, 0);
    
    return true;
}

int8_t plResponderModifier::ICmdFromWait(int8_t waitIdx)
{
    WaitToCmd& waitToCmd = fStates[fCurState].fWaitToCmd;
    if (waitToCmd.find(waitIdx) != waitToCmd.end())
        return waitToCmd[waitIdx];
    return -1;
}

void plResponderModifier::Restore()
{
    // If we're the first player in and we're loading old state where this responder
    // was running, fast forward it
    if (plNetClientApp::GetInstance()->GetJoinOrder() == 0 && fCurCommand != -1 && !fGotFirstLoad)
    {
        fGotFirstLoad = true;
        IFastForward(false);
        return;
    }

    ResponderLog(ILog(plStatusLog::kGreen, "Load SDL State"));
    
    fGotFirstLoad = true;

    plResponderState& state = fStates[fCurState];
    for (int i = 0; i < state.fNumCallbacks; i++)
    {
        if (!fCompletedEvents[i])
        {
            int cmdIdx = state.fWaitToCmd[i];

            plResponderCmd& cmd = state.fCmds[cmdIdx];

            //
            // If it's a callback message (anim or sound), just send the callbacks again
            //
            plMessageWithCallbacks* callbackMsg = plMessageWithCallbacks::ConvertNoRef(cmd.fMsg);
            if (callbackMsg)
            {
                // Create a new message for just the callbacks
                plMessageWithCallbacks* newCallbackMsg = nil;

                if (plAnimCmdMsg* animMsg = plAnimCmdMsg::ConvertNoRef(callbackMsg))
                {
                    plAnimCmdMsg* newAnimMsg = new plAnimCmdMsg;
                    newAnimMsg->SetCmd(plAnimCmdMsg::kAddCallbacks);
                    newCallbackMsg = newAnimMsg;
                    ResponderLog(ILog(plStatusLog::kGreen, "Restoring anim callback"));
                }
                else if (plSoundMsg* soundMsg = plSoundMsg::ConvertNoRef(callbackMsg))
                {
                    plSoundMsg* newSoundMsg = new plSoundMsg;
                    newSoundMsg->SetCmd(plSoundMsg::kAddCallbacks);
                    newCallbackMsg = newSoundMsg;
                    ResponderLog(ILog(plStatusLog::kGreen, "Restoring sound callback"));
                }

                // Setup the sender and receiver
                newCallbackMsg->SetSender(callbackMsg->GetSender());
                for (int iReceiver = 0; i < callbackMsg->GetNumReceivers(); i++)
                    newCallbackMsg->AddReceiver(callbackMsg->GetReceiver(iReceiver));

                // Add the callbacks
                int numCallbacks = callbackMsg->GetNumCallbacks();
                for (int iCallback = 0; iCallback < numCallbacks; iCallback++)
                {
                    plMessage* callback = callbackMsg->GetCallback(iCallback);
//                  hsRefCnt_SafeRef(callback); AddCallback will ref this for us.
                    newCallbackMsg->AddCallback(callback);
                }

                newCallbackMsg->Send();
            }
        }
    }
}

#include "plCreatableIndex.h"

plMessage* plResponderModifier::IGetFastForwardMsg(plMessage* msg, bool python)
{
    if (!msg)
        return nil;

    if (plAnimCmdMsg* animMsg = plAnimCmdMsg::ConvertNoRef(msg))
    {
        if (animMsg->Cmd(plAnimCmdMsg::kContinue) ||
            animMsg->Cmd(plAnimCmdMsg::kAddCallbacks))
        {
            plAnimCmdMsg* newAnimMsg = new plAnimCmdMsg;
            newAnimMsg->fCmd                = animMsg->fCmd;
            newAnimMsg->fBegin              = animMsg->fBegin;
            newAnimMsg->fEnd                = animMsg->fEnd;
            newAnimMsg->fLoopEnd            = animMsg->fLoopEnd;
            newAnimMsg->fLoopBegin          = animMsg->fLoopBegin;
            newAnimMsg->fSpeed              = animMsg->fSpeed;
            newAnimMsg->fSpeedChangeRate    = animMsg->fSpeedChangeRate;
            newAnimMsg->fTime               = animMsg->fTime;
            newAnimMsg->SetAnimName(animMsg->GetAnimName());
            newAnimMsg->SetLoopName(animMsg->GetLoopName());

            // Remove the callbacks
            newAnimMsg->fCmd.SetBit(plAnimCmdMsg::kAddCallbacks, false);

            if (newAnimMsg->Cmd(plAnimCmdMsg::kContinue))
            {
                newAnimMsg->fCmd.SetBit(plAnimCmdMsg::kContinue, false);
                newAnimMsg->fCmd.SetBit(plAnimCmdMsg::kFastForward, true);
            }

            for (int i = 0; i < animMsg->GetNumReceivers(); i++)
                newAnimMsg->AddReceiver(animMsg->GetReceiver(i));

            ResponderLog(ILog(plStatusLog::kWhite, "FF Animation Play Msg"));
            return newAnimMsg;
        }

        ResponderLog(ILog(plStatusLog::kWhite, "FF Animation Non-Play Msg"));
        hsRefCnt_SafeRef(msg);
        return msg;
    }
    else if(plSoundMsg *soundMsg = plSoundMsg::ConvertNoRef(msg))
    {
        if( fFlags & kSkipFFSound )
        {
            return nil;
        }
        if(soundMsg->Cmd(plSoundMsg::kPlay) ||
            soundMsg->Cmd(plSoundMsg::kToggleState)  ||
            soundMsg->Cmd(plAnimCmdMsg::kAddCallbacks))
        {
            plSoundMsg *newSoundMsg = new plSoundMsg;
            newSoundMsg->fCmd = soundMsg->fCmd;
            newSoundMsg->fBegin = soundMsg->fBegin;
            newSoundMsg->fEnd = soundMsg->fEnd;
            newSoundMsg->fLoop = soundMsg->fLoop;
            newSoundMsg->fSpeed = soundMsg->fSpeed;
            newSoundMsg->fTime = soundMsg->fTime;
            newSoundMsg->fIndex = soundMsg->fIndex;
            newSoundMsg->fRepeats = soundMsg->fRepeats;
            newSoundMsg->fPlaying = soundMsg->fPlaying;
            newSoundMsg->fNameStr = soundMsg->fNameStr;
            newSoundMsg->fVolume = soundMsg->fVolume;

            // Remove the callbacks
            newSoundMsg->fCmd.SetBit(plSoundMsg::kAddCallbacks, false);

            if(newSoundMsg->Cmd(plSoundMsg::kPlay))
            {
                newSoundMsg->fCmd.SetBit(plSoundMsg::kPlay, false);
                newSoundMsg->fCmd.SetBit(plSoundMsg::kFastForwardPlay);
                ResponderLog(ILog(plStatusLog::kWhite, "FF Sound Play Msg"));
            }
            else if(newSoundMsg->Cmd(plSoundMsg::kToggleState))
            {
                newSoundMsg->fCmd.SetBit(plSoundMsg::kToggleState, false);
                newSoundMsg->fCmd.SetBit(plSoundMsg::kFastForwardToggle);
                ResponderLog(ILog(plStatusLog::kWhite, "FF Sound Toggle State Msg"));
            }
            for (int i = 0; i < soundMsg->GetNumReceivers(); i++)
                newSoundMsg->AddReceiver(soundMsg->GetReceiver(i));

            return newSoundMsg;
        }
        ResponderLog(ILog(plStatusLog::kWhite, "FF Sound Non-Play/Toggle Msg"));
        hsRefCnt_SafeRef(msg);
        return msg;
    }
    else if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plExcludeRegionMsg))
    {
        ResponderLog(ILog(plStatusLog::kWhite, "FF Exclude Region Msg"));
        hsRefCnt_SafeRef(msg);
        return msg;
    }
    else if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plEnableMsg))
    {
        ResponderLog(ILog(plStatusLog::kWhite, "FF Visibility/Detector Enable Msg"));
        hsRefCnt_SafeRef(msg);
        return msg;
    }
    else if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plResponderEnableMsg))
    {
        ResponderLog(ILog(plStatusLog::kWhite, "FF Responder Enable Msg"));
        hsRefCnt_SafeRef(msg);
        return msg;
    }
    else if (msg->ClassIndex() == CLASS_INDEX_SCOPED(plSimSuppressMsg))
    {
        ResponderLog(ILog(plStatusLog::kWhite, "FF Physical Enable Msg"));
        hsRefCnt_SafeRef(msg);
        return msg;
    }

    return nil;
}

void plResponderModifier::IFastForward(bool python)
{
    ResponderLog(ILog(plStatusLog::kGreen, "Fast Forward"));

    fCurCommand = 0;

    plResponderState& state = fStates[fCurState];

    while (fCurCommand < state.fCmds.Count())
    {
        plMessage *msg = state.fCmds[fCurCommand].fMsg;
        msg = IGetFastForwardMsg(msg, python);
        if (msg)
            plgDispatch::MsgSend(msg);

        fCurCommand++;
    }

    ResponderLog(ILog(plStatusLog::kGreen, "Reset"));

    fCurCommand = -1;
    ISetResponderState(state.fSwitchToState);

    plSynchEnabler enable(true);
    DirtySynchState(kSDLResponder, 0);
}

void plResponderModifier::Read(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Read(stream, mgr);

    int8_t numStates = stream->ReadByte();
    fStates.SetCount(numStates);
    for (int8_t i = 0; i < numStates; i++)
    {
        plResponderState& state = fStates[i];
        state.fNumCallbacks = stream->ReadByte();
        state.fSwitchToState = stream->ReadByte();

        int8_t j;

        int8_t numCmds = stream->ReadByte();
        state.fCmds.SetCount(numCmds);
        for (j = 0; j < numCmds; j++)
        {
            plResponderCmd& cmd = state.fCmds[j];

            plMessage* pMsg = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
            cmd.fMsg = pMsg;
            cmd.fWaitOn = stream->ReadByte();
        }

        state.fWaitToCmd.clear();
        int8_t mapSize = stream->ReadByte();
        for (j = 0; j < mapSize; j++)
        {
            int8_t wait = stream->ReadByte();
            int8_t cmd = stream->ReadByte();
            state.fWaitToCmd[wait] = cmd;
        }
    }

    ISetResponderState(stream->ReadByte());
    fEnabled = stream->ReadBool();
    fFlags = stream->ReadByte();

    // attach responderSDLMod
    delete fResponderSDLMod;
    fResponderSDLMod = new plResponderSDLModifier;
    fResponderSDLMod->SetResponder(this);
}

void plResponderModifier::Write(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Write(stream, mgr);

    int8_t numStates = fStates.GetCount();
    stream->WriteByte(numStates);
    for (int i = 0; i < numStates; i++)
    {
        plResponderState& state = fStates[i];
        stream->WriteByte(state.fNumCallbacks);
        stream->WriteByte(state.fSwitchToState);

        int8_t numCmds = state.fCmds.GetCount();
        stream->WriteByte(numCmds);
        for (int j = 0; j < numCmds; j++)
        {
            plResponderCmd& cmd = state.fCmds[j];

            mgr->WriteCreatable(stream, cmd.fMsg);
            stream->WriteByte(cmd.fWaitOn);
        }

        int8_t mapSize = state.fWaitToCmd.size();
        stream->WriteByte(mapSize);
        for (WaitToCmd::iterator it = state.fWaitToCmd.begin(); it != state.fWaitToCmd.end(); it++)
        {
            stream->WriteByte(it->first);
            stream->WriteByte(it->second);
        }
    }

    stream->WriteByte(fCurState);
    stream->WriteBool(fEnabled);
    stream->WriteByte(fFlags);
}


bool plResponderModifier::fDebugAnimBox = false;

void plResponderModifier::IDebugAnimBox(bool start)
{
    plDebugText &debugTxt = plDebugText::Instance();

    uint32_t scrnWidth, scrnHeight;
    debugTxt.GetScreenSize(&scrnWidth, &scrnHeight);

    // Box size is 1/8 screen size
    uint32_t boxSize = scrnHeight / 8;

    // Draw box in lower left corner
    if (start)
        debugTxt.DrawRect(0, (uint16_t)(scrnHeight-boxSize), (uint16_t)boxSize, (uint16_t)scrnHeight, 0, 255, 0);
    else
        debugTxt.DrawRect((uint16_t)boxSize, (uint16_t)(scrnHeight-boxSize), (uint16_t)(boxSize*2), (uint16_t)scrnHeight, 255, 0, 0);
}

void plResponderModifier::IDebugPlayMsg(plAnimCmdMsg* msg)
{
    // Create a stop callback so we can do a cue for that too
    plEventCallbackMsg *eventMsg = new plEventCallbackMsg;
    eventMsg->AddReceiver(GetKey());
    eventMsg->fRepeats = 0;
    eventMsg->fUser = -1;
    eventMsg->fEvent = kStop;
    msg->SetCmd(plAnimCmdMsg::kAddCallbacks);
    msg->AddCallback(eventMsg);
    hsRefCnt_SafeUnRef(eventMsg);

    IDebugAnimBox(true);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef STATUS_LOG
static plStatusLog *gLog = nil;
static std::vector<ST::string> gNoLogStrings;
#endif // STATUS_LOG

void plResponderModifier::NoLogString(const char* str)
{
#ifdef STATUS_LOG
    gNoLogStrings.push_back(str);
#endif // STATUS_LOG
}

void plResponderModifier::ILog(uint32_t color, const char* format, ...)
{
#ifdef STATUS_LOG
    if (!gLog)
        gLog = plStatusLogMgr::GetInstance().CreateStatusLog(15, "Responder", plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe | plStatusLog::kDontWriteFile | plStatusLog::kAlignToTop);

    if (!format || *format == '\0')
        return;

    ST::string keyName = GetKeyName();

    // Make sure this key isn't in our list of keys to deny
    for (const auto& it : gNoLogStrings) {
        if (keyName.starts_with(it))
            return;
    }

    // Format the log text
    char buf[256];
    va_list args;
    va_start(args, format);
    int numWritten = hsVsnprintf(buf, sizeof(buf), format, args);
    hsAssert(numWritten > 0, "Buffer too small");
    va_end(args);

    // Strip the redundant part off the key name
    ST_ssize_t modPos = keyName.find("_ResponderModifier");
    if (modPos != -1)
        keyName = keyName.left(modPos);

    gLog->AddLineF(color, "{}: {}", keyName, buf);
#endif // STATUS_LOG
}
