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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plAvatarInputInterface                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
//#define LIMIT_VOICE_CHAT 1
#endif

#include "HeadSpin.h"
#include "plAvatarInputInterface.h"

#include "pnInputCore/plKeyMap.h"
#include "plMessage/plInputEventMsg.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"

#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnMessage/plProxyDrawMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"

#include "plAudio/plVoiceChat.h"
#include "plInputDevice.h"
#include "plInputManager.h"
#include "hsResMgr.h"
#include "plgDispatch.h"

#include "hsMatrix44.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"

//// Constructor/Destructor //////////////////////////////////////////////////

plAvatarInputInterface  *plAvatarInputInterface::fInstance = nullptr;

plAvatarInputInterface::plAvatarInputInterface()
{
    fInstance = this;
    fMouseDisabled = false;
    fCurrentCursor = kCursorUp;
    fCursorOpacity = 1.f;
    fCursorTimeout = 0;
    fCursorFadeDelay = 3.f;
    f3rdPerson = true;
    fInputMap = nullptr;
    ISetBasicMode(); // Must be after 3rdPerson and fInputMap are set.
    SetEnabled( true );         // Always enabled

    // Add our control codes to our control map. Do NOT add the key bindings yet.
    // Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
    // This part basically declares us master of the bindings for these commands.
    
    // IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
    // RestoreDefaultKeyMappings()!!!!

#ifndef LIMIT_VOICE_CHAT
    // only allow mapping of 'PushToTalk in online versions'
    fControlMap->AddCode( S_PUSH_TO_TALK,                   kControlFlagNormal | kControlFlagNoRepeat );
#endif
    fControlMap->AddCode( S_SET_FIRST_PERSON_MODE,          kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_EXIT_MODE,              kControlFlagNormal | kControlFlagNoRepeat );

    fControlMap->AddCode( B_CAMERA_ZOOM_IN,                 kControlFlagNormal );
    fControlMap->AddCode( B_CAMERA_ZOOM_OUT,                kControlFlagNormal );

    fControlMap->AddCode( B_CONTROL_MODIFIER_FAST,          kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_MODIFIER_STRAFE,        kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_MOVE_FORWARD,           kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_MOVE_BACKWARD,          kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_ROTATE_LEFT,            kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_ROTATE_RIGHT,           kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_STRAFE_LEFT,            kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_STRAFE_RIGHT,           kControlFlagNormal | kControlFlagNoRepeat );
        

    fControlMap->AddCode( B_CONTROL_ALWAYS_RUN,             kControlFlagToggle | kControlFlagUpEvent | kControlFlagNoRepeat );


    fControlMap->AddCode( B_CONTROL_JUMP,                   kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_DIVE,                   kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_IGNORE_AVATARS,         kControlFlagNormal | kControlFlagNoRepeat );

    fControlMap->AddConsoleCommand(ST_LITERAL("Game.EnterChatMode"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.wave"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.laugh"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.clap"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.dance"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.talk"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.sneeze"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.Emote.sit"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Keyboard.ResetBindings"));
    
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIOpenKI"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIHelp"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KICreateMarker"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KICreateMarkerFolder"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIOpenYeeshaBook"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIToggleMini"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIPutAway"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIChatPageUp"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIChatPageDown"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIChatToStart"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIChatToEnd"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIUpSizeFont"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KIDownSizeFont"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KITakePicture"));
    fControlMap->AddConsoleCommand(ST_LITERAL("Game.KICreateJournal"));

#ifndef PLASMA_EXTERNAL_RELEASE
    fControlMap->AddCode( B_CONTROL_TOGGLE_PHYSICAL,        kControlFlagDownEvent | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_MOVE_UP,                kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_CONTROL_MOVE_DOWN,              kControlFlagNormal | kControlFlagNoRepeat );
    fControlMap->AddCode( B_TOGGLE_DRIVE_MODE,              kControlFlagDownEvent | kControlFlagNoRepeat );

    // These by default conflict with text characters, so we deactivate the bindings until they are needed.
    IDisableControl(B_CONTROL_MOVE_UP);
    IDisableControl(B_CONTROL_MOVE_DOWN);

    fControlMap->AddConsoleCommand(ST_LITERAL("NextStatusLog"));
#endif

    // IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
    // RestoreDefaultKeyMappings()!!!!
}

plAvatarInputInterface::~plAvatarInputInterface()
{
    delete fInputMap;
}

//// Init/Shutdown ///////////////////////////////////////////////////////////

void    plAvatarInputInterface::Init( plInputInterfaceMgr *manager )
{
    plInputInterface::Init( manager );
}

void    plAvatarInputInterface::Shutdown()
{
}

void plAvatarInputInterface::CameraInThirdPerson(bool state)
{
    if (state != f3rdPerson)
    {
        f3rdPerson = state;

        if (fInputMap->IsBasic())
        {
            ISetBasicMode();
        }
    }
}

void plAvatarInputInterface::IDeactivateCommand(plMouseInfo *info)
{
    if (IHasControlFlag(info->fCode) && !(info->fControlFlags & (kControlFlagNoDeactivate | kControlFlagToggle)))
    {
        // The mapping is currently on, it's ok to deactivate, and it's not a toggle command                    
        plCtrlCmd* pCmd = new plCtrlCmd( this );
        pCmd->fNetPropagateToPlayers = info->fControlFlags & kControlFlagNetPropagate;
        pCmd->fControlActivated = false;
        pCmd->fControlCode = info->fCode;
        IClearControlFlag(pCmd->fControlCode);
        fMessageQueue->emplace_back(pCmd);
    }
}

//// IChangeInputMaps ////////////////////////////////////////////////////////

void plAvatarInputInterface::IChangeInputMaps( plAvatarInputMap *newMap )
{
    newMap->fButtonState = fInputMap ? fInputMap->fButtonState : 0; 

    if (fInputMap)
    {
        for (plMouseInfo *info : fInputMap->fMouseMap->fMap)
            IDeactivateCommand(info);
        delete fInputMap;
    }

    fInputMap = newMap;
//  fInputMap->fButtonState = 0;
//  Reset();
}

void plAvatarInputInterface::ISetSuspendMovementMode()
{
    IChangeInputMaps(new plSuspendedMovementMap());
    fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetLadderMap()
{
    IChangeInputMaps(new plLadderControlMap());
    fCurrentCursor = kCursorUp;
}


void plAvatarInputInterface::ISetPreLadderMap()
{
    IChangeInputMaps(new plLadderMountMap());
    fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetPostLadderMap()
{
    IChangeInputMaps(new plLadderDismountMap());
    fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetBasicMode()
{
    plAvatarInputMap *map;
    if (!f3rdPerson)
        map = new plBasicFirstPersonControlMap();
    else
        map = new plBasicThirdPersonControlMap();

    IChangeInputMaps(map);
    fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetMouseWalkMode(ControlEventCode code)
{
    if (code == S_SET_WALK_BACK_MODE)
        IChangeInputMaps(new pl3rdWalkBackwardMap());
    else if (code == S_SET_WALK_BACK_LB_MODE)
        IChangeInputMaps(new pl3rdWalkBackwardLBMap());
    else
        IChangeInputMaps(new pl3rdWalkForwardMap());

    fCurrentCursor = kCursorHidden;
}

//// ClearKeyMap ///////////////////////////////////////////////
void plAvatarInputInterface::ClearKeyMap()
{ 
    // Note: we might be clearing our key bindings, but we still want to be owners of the commands,
    if (fControlMap != nullptr)
    {
        fControlMap->UnmapAllBindings();
        
        // Still want this one tho
        fControlMap->BindKeyToConsoleCmd(plCtrlShiftKeyCombo(KEY_0), ST_LITERAL("Keyboard.ResetBindings"));
    }
}

//// RestoreDefaultKeyMappings ///////////////////////////////////////////////

void    plAvatarInputInterface::RestoreDefaultKeyMappings()
{
    if (fControlMap == nullptr)
        return;

    fControlMap->UnmapAllBindings();

#ifndef LIMIT_VOICE_CHAT
    fControlMap->BindKey( KEY_TAB,                      S_PUSH_TO_TALK );
#endif
    fControlMap->BindKey( KEY_F1,                       S_SET_FIRST_PERSON_MODE );
    fControlMap->BindKey( plCtrlKeyCombo( KEY_F ),      S_SET_FIRST_PERSON_MODE );
    fControlMap->BindKey( KEY_BACKSPACE,                B_CONTROL_EXIT_MODE );
    fControlMap->BindKey( KEY_ESCAPE,                   B_CONTROL_EXIT_MODE );

    fControlMap->BindKey( KEY_NUMPAD_ADD,               B_CAMERA_ZOOM_IN );
    fControlMap->BindKey( KEY_NUMPAD_SUBTRACT,          B_CAMERA_ZOOM_OUT );

    fControlMap->BindKey( KEY_SHIFT,                    B_CONTROL_MODIFIER_FAST );
    fControlMap->BindKey( KEY_ALT,                      B_CONTROL_MODIFIER_STRAFE );
    fControlMap->BindKey( KEY_UP,                       B_CONTROL_MOVE_FORWARD );
    fControlMap->BindKey( KEY_DOWN,                     B_CONTROL_MOVE_BACKWARD );
    fControlMap->BindKey( KEY_LEFT,                     B_CONTROL_ROTATE_LEFT );
    fControlMap->BindKey( KEY_RIGHT,                    B_CONTROL_ROTATE_RIGHT );
    fControlMap->BindKey( KEY_COMMA,                    B_CONTROL_STRAFE_LEFT );
    fControlMap->BindKey( KEY_PERIOD,                   B_CONTROL_STRAFE_RIGHT );

// This is now hard-coded to capslock
//  fControlMap->BindKey( KEY_CAPSLOCK,                 B_CONTROL_ALWAYS_RUN );

    fControlMap->BindKey( KEY_SPACE,                    B_CONTROL_JUMP );
//  fControlMap->BindKey( KEY_D,                        B_CONTROL_DIVE );
    fControlMap->BindKey( KEY_DELETE,                   B_CONTROL_IGNORE_AVATARS );

    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_1), ST_LITERAL("Game.Emote.wave"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_2), ST_LITERAL("Game.Emote.laugh"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_3), ST_LITERAL("Game.Emote.clap"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_4), ST_LITERAL("Game.Emote.dance"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_5), ST_LITERAL("Game.Emote.talk"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_6), ST_LITERAL("Game.Emote.sneeze"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_7), ST_LITERAL("Game.Emote.sit"));

    fControlMap->BindKeyToConsoleCmd(plCtrlShiftKeyCombo(KEY_0), ST_LITERAL("Keyboard.ResetBindings"));
    
    // KI shortcut keyboard commands
    fControlMap->BindKeyToConsoleCmd(KEY_F2,                              ST_LITERAL("Game.KIOpenKI"));
    fControlMap->BindKeyToConsoleCmd(KEY_F3,                              ST_LITERAL("Game.KIOpenYeeshaBook"));
    fControlMap->BindKeyToConsoleCmd(KEY_F4,                              ST_LITERAL("Game.KIHelp"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_HOME),            ST_LITERAL("Game.KIToggleMini"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_END),             ST_LITERAL("Game.KIPutAway"));
    fControlMap->BindKeyToConsoleCmd(KEY_PAGEUP,                          ST_LITERAL("Game.KIChatPageUp"));
    fControlMap->BindKeyToConsoleCmd(KEY_PAGEDOWN,                        ST_LITERAL("Game.KIChatPageDown"));
    fControlMap->BindKeyToConsoleCmd(KEY_HOME,                            ST_LITERAL("Game.KIChatToStart"));
    fControlMap->BindKeyToConsoleCmd(KEY_END,                             ST_LITERAL("Game.KIChatToEnd"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_NUMPAD_ADD),      ST_LITERAL("Game.KIUpSizeFont"));
    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_NUMPAD_SUBTRACT), ST_LITERAL("Game.KIDownSizeFont"));
    fControlMap->BindKeyToConsoleCmd(KEY_F5,                              ST_LITERAL("Game.KITakePicture"));
    fControlMap->BindKeyToConsoleCmd(KEY_F6,                              ST_LITERAL("Game.KICreateJournal"));
    fControlMap->BindKeyToConsoleCmd(KEY_F7,                              ST_LITERAL("Game.KICreateMarker"));
    fControlMap->BindKeyToConsoleCmd(KEY_F8,                              ST_LITERAL("Game.KICreateMarkerFolder"));

#ifndef PLASMA_EXTERNAL_RELEASE
    fControlMap->BindKey( plCtrlKeyCombo( KEY_P ),      B_CONTROL_TOGGLE_PHYSICAL );
    fControlMap->BindKey( KEY_U,                        B_CONTROL_MOVE_UP );
    fControlMap->BindKey( KEY_H,                        B_CONTROL_MOVE_DOWN );
    fControlMap->BindKey( plCtrlKeyCombo( KEY_D ),      B_TOGGLE_DRIVE_MODE );

    fControlMap->BindKeyToConsoleCmd(plCtrlKeyCombo(KEY_L), ST_LITERAL("NextStatusLog"));
#endif
}

void plAvatarInputInterface::SetLadderMode()
{
    ISetPreLadderMap();
}

void plAvatarInputInterface::ClearLadderMode()
{
    ISetBasicMode();
}

void plAvatarInputInterface::SuspendMouseMovement()
{
    ISetSuspendMovementMode();
}

void plAvatarInputInterface::EnableMouseMovement()
{
    ISetBasicMode();
}

void plAvatarInputInterface::EnableJump(bool val)
{
    EnableControl(val, B_CONTROL_JUMP);
}

void plAvatarInputInterface::EnableForwardMovement(bool val)
{
    EnableControl(val, B_CONTROL_MOVE_FORWARD);
}

void plAvatarInputInterface::EnableControl(bool val, ControlEventCode code)
{
    if (val)
        IEnableControl(code);
    else
        IDisableControl(code);
}

void plAvatarInputInterface::ForceAlwaysRun(bool val)
{
    plCtrlCmd *pCmd = new plCtrlCmd( this );
    pCmd->fControlCode = B_CONTROL_ALWAYS_RUN;
    pCmd->fControlActivated = val;
    pCmd->fNetPropagateToPlayers = false;
        
    fMessageQueue->emplace_back(pCmd);
}   

//// IEval ///////////////////////////////////////////////////////////////////
//  Gets called once per IUpdate(), just like normal IEval()s

bool plAvatarInputInterface::IEval( double secs, float del, uint32_t dirty )
{
    fCursorTimeout += del;
    if( fCursorTimeout > fCursorFadeDelay )
    {
        if( fCursorTimeout > fCursorFadeDelay + 2.f )
            fCursorOpacity = 0.f;
        else
            fCursorOpacity = 1.f - ( ( fCursorTimeout - fCursorFadeDelay ) / 2.f );
    }
    else
        fCursorOpacity = 1.f;

    return true;
}

//// IHandleCtrlCmd //////////////////////////////////////////////////////////

bool    plAvatarInputInterface::IHandleCtrlCmd( plCtrlCmd *cmd )
{
    switch( cmd->fControlCode )
    {
        case S_SET_CURSOR_UPWARD:       
            if( cmd->fControlActivated )
                fCurrentCursor = kCursorUpward;         
            return true;
        case S_SET_CURSOR_UP:       
            if( cmd->fControlActivated )
                fCurrentCursor = kCursorUp;         
            return true;
        case S_SET_CURSOR_DOWN:     
            if( cmd->fControlActivated )
                fCurrentCursor = kCursorDown;           
            return true;
        case S_SET_CURSOR_RIGHT:        
            if( cmd->fControlActivated )
                fCurrentCursor = kCursorRight;          
            return true;
        case S_SET_CURSOR_LEFT:     
            if( cmd->fControlActivated )
                fCurrentCursor = kCursorLeft;           
            return true;
        case S_SET_CURSOR_HIDDEN:       
            if( cmd->fControlActivated )
                fCurrentCursor = kCursorHidden; 
            else
                fCurrentCursor = kCursorUp;
            return true;
        case S_SET_LADDER_CONTROL:      
            if( cmd->fControlActivated )
                ISetLadderMap();            
            return true;
#if 0
        case S_SET_FIRST_PERSON_MODE:
            if( cmd->fControlActivated )
                IChangeInputMaps( new plFirstPersonControlMap() );
            return true;
#endif          
        case S_SET_BASIC_MODE:
            if( cmd->fControlActivated )
            {
                ISetBasicMode();                
            }
            return true;


        case S_SET_WALK_MODE:

            if( cmd->fControlActivated )
            {
                bool abort = false;
                for (plCtrlCmd* ctrlMsg : *fMessageQueue)
                {
                    if (ctrlMsg->fControlCode == S_SET_WALK_MODE && !ctrlMsg->fControlActivated)
                    {   
                        abort = true;
                        break;
                    }
                }
                if (abort)
                    return true;
                ISetMouseWalkMode(S_SET_WALK_MODE);
            }
            return true;

        case S_SET_WALK_BACK_MODE:
            
            if( cmd->fControlActivated )
            {
                bool abort = false;
                for (plCtrlCmd* ctrlMsg : *fMessageQueue)
                {
                    if (ctrlMsg->fControlCode == S_SET_WALK_BACK_MODE && !ctrlMsg->fControlActivated)
                    {   
                        abort = true;
                        break;
                    }
                }
                if (abort)
                    return true;
                ISetMouseWalkMode(S_SET_WALK_BACK_MODE);
            }
            return true;

        case S_SET_WALK_BACK_LB_MODE:
            
            if( cmd->fControlActivated )
            {
                bool abort = false;
                for (plCtrlCmd* ctrlMsg : *fMessageQueue)
                {
                    if (ctrlMsg->fControlCode == S_SET_WALK_BACK_MODE && !ctrlMsg->fControlActivated)
                    {
                        abort = true;
                        break;
                    }
                }
                if (abort)
                    return true;
                ISetMouseWalkMode(S_SET_WALK_BACK_LB_MODE);
            }
            return true;
            
        case S_INCREASE_MIC_VOL:
            plVoiceRecorder::IncreaseRecordingThreshhold();
            return true;

        case S_DECREASE_MIC_VOL:
            plVoiceRecorder::DecreaseRecordingThreshhold();
            return true;

    /*          case B_CONTROL_ACTION:
            {
                if (fMessageQueue[i]->fControlActivated)
                {   
                    // send a 'picked' message to the picked object
                    plPickedMsg* pPickedMsg = new plPickedMsg;
                    pPickedMsg->AddReceiver(fCurrentClickable);
                    plgDispatch::MsgSend(pPickedMsg);
                }
                else
                {
                    // send an 'unpicked message'
                    plPickedMsg* pPickedMsg = new plPickedMsg;
                    pPickedMsg->AddReceiver(fCurrentClickable);
                    pPickedMsg->fPicked = false;
                    plgDispatch::MsgSend(pPickedMsg);
                }
            }
            break;
    */

        default:
            break;
    } 

    return false;
}

bool plAvatarInputInterface::CursorInBox(plMouseEventMsg* pMsg, hsPoint4 box)
{
    return ( pMsg->GetXPos() >= box.fX && pMsg->GetXPos() <= box.fY && pMsg->GetYPos() >= box.fZ && pMsg->GetYPos() <= box.fW ); 
}

void plAvatarInputInterface::Reset()
{
    fControlFlags.Clear();
    fKeyControlFlags.Clear();
    fDisabledControls.Clear();
}

void plAvatarInputInterface::ClearMouseCursor()
{
    IClearControlFlag(S_SET_CURSOR_UPWARD);
    IClearControlFlag(S_SET_CURSOR_UP);
    IClearControlFlag(S_SET_CURSOR_DOWN);
    IClearControlFlag(S_SET_CURSOR_LEFT);
    IClearControlFlag(S_SET_CURSOR_RIGHT);
}

bool    plAvatarInputInterface::MsgReceive( plMessage *msg )
{
    plCmdIfaceModMsg *pCMsg = plCmdIfaceModMsg::ConvertNoRef( msg );
    if( pCMsg )
    {
        if (pCMsg->Cmd(plCmdIfaceModMsg::kDisableControlCode))
        {
            IDisableControl(pCMsg->fControlCode);
            return true;
        }
        else
        if (pCMsg->Cmd(plCmdIfaceModMsg::kEnableControlCode))
        {
            IEnableControl(pCMsg->fControlCode);
            return true;
        }
        return false;
    }
    return false;
}

//// MissedInputEvent ////////////////////////////////////////////////////////
//  If we "missed" an input event, then somebody caught it above us, thus we
//  have "lost focus" in a way. So we should stop walking/moving/whatever.
//  Should this be in the base inputInterface, since it deals with key
//  bindings? Perhaps, dunno yet. We'll see...

void    plAvatarInputInterface::MissedInputEvent( plInputEventMsg *pMsg )
{
    int                 i;


    if (plKeyEventMsg::ConvertNoRef(pMsg) == nullptr)
    {
        // We only "lose focus" if someone else grabbed a key message. Don't care about anything else.
        return;
    }

    // Disable all set control flags, EXCEPT autorun. Rrrgh.
    for( i = 0; i < fControlMap->GetNumBindings(); i++ )
    {
        const plKeyBinding &binding = fControlMap->GetBinding( i );

        if( IHasKeyControlFlag( binding.GetCode() ) && binding.GetCode() != B_CONTROL_ALWAYS_RUN )
        {
            plCtrlCmd *pCmd = new plCtrlCmd( this );
            pCmd->fControlCode = binding.GetCode();
            pCmd->fControlActivated = false;
            pCmd->SetCmdString( binding.GetExtendedString() );

            if( binding.GetCodeFlags() & kControlFlagNetPropagate )
                pCmd->fNetPropagateToPlayers = true;
            else
                pCmd->fNetPropagateToPlayers = false;

            fMessageQueue->emplace_back(pCmd);
            IClearKeyControlFlag( binding.GetCode() );
        }
    }
}

bool plAvatarInputInterface::IsEnterChatModeBound()
{
    int i;
    for ( i=0; i< fControlMap->GetNumBindings(); i++ )
    {
        const plKeyBinding &binding = fControlMap->GetBinding( i );

        ST::string extString = binding.GetExtendedString();
        if (extString == "Game.EnterChatMode")
        {
            if (binding.GetKey1() != plKeyCombo::kUnmapped )
                return true;
        }
    }
    return false;
}


//// InterpretInputEvent /////////////////////////////////////////////////////

bool plAvatarInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
    if (fInputMap == nullptr)
        return false;

    plMouseMap  *mouseMap = fInputMap->fMouseMap;


    plKeyEventMsg* pKeyMsg = plKeyEventMsg::ConvertNoRef(pMsg);
    if( pKeyMsg )
    {
        // Handled by key bindings
    }

    if (fMouseDisabled)
        return false;

    plMouseEventMsg* pMouseMsg = plMouseEventMsg::ConvertNoRef(pMsg);
    if (pMouseMsg)
    {
        uint32_t oldButtonState = fInputMap->fButtonState;

        // There's no "up" event for mouse wheel scrolling, so we have to check
        // on each message whether we're still scrolling or not
        fInputMap->fButtonState &= ~kWheelNeg;
        fInputMap->fButtonState &= ~kWheelPos;

        // check for button presses...
        if (fInputMap->fButtonState & kLeftButtonDown)
        {
            fInputMap->fButtonState |= kLeftButtonRepeat;
        }
        if (fInputMap->fButtonState & kRightButtonDown)
        {
            fInputMap->fButtonState |= kRightButtonRepeat;
        }
        if (fInputMap->fButtonState & kMiddleButtonDown)
        {
            fInputMap->fButtonState |= kMiddleButtonRepeat;
        }
        if (pMouseMsg->GetButton() == kLeftButtonDown)
        {
            fInputMap->fButtonState |= kLeftButtonDown;
        }
        if (pMouseMsg->GetButton() == kLeftButtonUp)
        {
            fInputMap->fButtonState &= ~kLeftButtonDown;
            fInputMap->fButtonState &= ~kLeftButtonRepeat;
        }
        if (pMouseMsg->GetButton() == kRightButtonDown)
        {
            fInputMap->fButtonState |= kRightButtonDown;
        }
        if (pMouseMsg->GetButton() == kRightButtonUp)
        {
            fInputMap->fButtonState &= ~kRightButtonDown;
            fInputMap->fButtonState &= ~kRightButtonRepeat;
        }
        if (pMouseMsg->GetButton() == kMiddleButtonDown)
        {
            fInputMap->fButtonState |= kMiddleButtonDown;
        }
        if (pMouseMsg->GetButton() == kMiddleButtonUp)
        {
            fInputMap->fButtonState &= ~kMiddleButtonDown;
            fInputMap->fButtonState &= ~kMiddleButtonRepeat;
        }
        if (pMouseMsg->GetButton() == kWheelPos) {
            fInputMap->fButtonState |= kWheelPos;
        }
        if (pMouseMsg->GetButton() == kWheelNeg) {
            fInputMap->fButtonState |= kWheelNeg;
        }

        if (oldButtonState != fInputMap->fButtonState || pMouseMsg->GetDX() != 0.f || pMouseMsg->GetDY() != 0.f || pMouseMsg->GetWheelDelta() != 0.f)
            fCursorTimeout = 0.f;   // Reset cursor opacity timeout thingy

        /*  NOTE: I see that this interface always returns true for mouse
            messages, even if it does nothing with them. It ends up working
            because this interface is always last in the stack. Seems like
            a bad idea, but it works so far and I'm not going to change it
            unless it obviously breaks something.

            Still, since we say that we've handled any mouse message, I'm 
            taking the liberty of making things simple. If a button is down,
            we reserve focus. If not, we release it.

            If things ever change so that an interface below us expects us
            to return false for messages we don't care about, we'll have to
            be more careful about reserving focus.
        */
        if (fInputMap->fButtonState & kAnyButtonDown)
            fManager->SetCurrentFocus(this);
        else
            fManager->ReleaseCurrentFocus(this);

        for (plMouseInfo *mouseInfo : mouseMap->fMap)
        {
            // is this control already set?
            if (IHasControlFlag(mouseInfo->fCode))
            {
                // Control isn't enabled, ignore
                if (!IControlCodeEnabled(mouseInfo->fCode))
                    return true;                
                
                // can we disable this control?
                bool disable = false;
                
                // can we disable this control based on a button?
                if (mouseInfo->fControlFlags & kControlFlagLeftButton && !(fInputMap->fButtonState & kLeftButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButton && !(fInputMap->fButtonState & kRightButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagMiddleButton && !(fInputMap->fButtonState & kMiddleButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && (fInputMap->fButtonState & kLeftButtonRepeat))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && (fInputMap->fButtonState & kRightButtonRepeat))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagMiddleButtonEx && (fInputMap->fButtonState & kMiddleButtonRepeat))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && !(fInputMap->fButtonState & kLeftButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && !(fInputMap->fButtonState & kRightButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagMiddleButtonEx && !(fInputMap->fButtonState & kMiddleButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagLeftButtonRepeat && !(fInputMap->fButtonState & kLeftButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagRightButtonRepeat && !(fInputMap->fButtonState & kRightButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagMiddleButtonRepeat && !(fInputMap->fButtonState & kMiddleButtonDown))
                    disable = true;
                if (mouseInfo->fControlFlags & kControlFlagWheel && !(fInputMap->fButtonState & (kWheelPos | kWheelNeg)))
                    disable = true;

                // can we disable this control based on the cursor position?
                if (!CursorInBox(pMouseMsg, mouseInfo->fBox) && mouseInfo->fControlFlags & kControlFlagBoxDisable)
                    disable = true;

                if (disable)
                {
                    IDeactivateCommand(mouseInfo);
                    continue;
                }
                // is it a range control?  If so we need to re-send the command
                
                if ((mouseInfo->fControlFlags & kControlFlagRangePos) || (mouseInfo->fControlFlags & kControlFlagRangeNeg))
                {
                    plCtrlCmd* pCmd = new plCtrlCmd( this );
                    pCmd->fControlActivated = true;
                    pCmd->fControlCode = mouseInfo->fCode;
                    float pct = 0.0f;
                    if (mouseInfo->fControlFlags & kControlFlagRangePos)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fX - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fZ - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    else 
                    if (mouseInfo->fControlFlags & kControlFlagRangeNeg)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fY - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fW - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    pCmd->fPct = pct;
                    if (pct == 1.0f || pct == -1.0f)
                    {
                        delete pCmd;
                        break;
                    }
                    pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;
                    fMessageQueue->emplace_back(pCmd);
                }
                if (mouseInfo->fControlFlags & kControlFlagDelta)
                {
                    plCtrlCmd* pCmd = new plCtrlCmd( this );
                    pCmd->fControlActivated = true;
                    pCmd->fControlCode = mouseInfo->fCode;
                    float pct = 0.0f;

                    if (mouseInfo->fControlFlags & kControlFlagWheel)
                        pct = pMouseMsg->GetWheelDelta();
                    else if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                        pct = pMouseMsg->GetDX();
                    else
                        pct = pMouseMsg->GetDY();
                
                    if (pct == 0.f)
                    {
                        delete pCmd;
                        continue;
                    }
                    pCmd->fPct = pct;
                    pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;
                    fMessageQueue->emplace_back(pCmd);
                }
                
            }
            else // if it is an 'always if in box' command see if it's not in the box
            if ((mouseInfo->fControlFlags & kControlFlagInBox) && (!CursorInBox(pMouseMsg, mouseInfo->fBox)))
            {   
                plCtrlCmd* pCmd = new plCtrlCmd( this );
                pCmd->fControlActivated = false;
                pCmd->fControlCode = mouseInfo->fCode;
                pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;
                fMessageQueue->emplace_back(pCmd);
                continue;
            }
            else // the control is not set, see if we should set it.
            {
                // is the control disabled?
                if (fDisabledControls.IsBitSet(mouseInfo->fCode))
                    continue;
                
                // is the cursor in the appropriate box?
                if (CursorInBox(pMouseMsg, mouseInfo->fBox))
                {   
                    // do we require a button?
                    if (mouseInfo->fControlFlags & kControlFlagLeftButton && !(fInputMap->fButtonState & kLeftButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButton && !(fInputMap->fButtonState & kRightButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagMiddleButton && !(fInputMap->fButtonState & kMiddleButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && (fInputMap->fButtonState & kLeftButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && (fInputMap->fButtonState & kRightButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagMiddleButtonEx && (fInputMap->fButtonState & kMiddleButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonRepeat && !(fInputMap->fButtonState & kLeftButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonRepeat && !(fInputMap->fButtonState & kRightButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagMiddleButtonRepeat && !(fInputMap->fButtonState & kMiddleButtonRepeat))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonEx && !(fInputMap->fButtonState & kLeftButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonEx && !(fInputMap->fButtonState & kLeftButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagMiddleButtonEx && !(fInputMap->fButtonState & kMiddleButtonDown))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagLeftButtonUp && !(pMouseMsg->GetButton() == kLeftButtonUp))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagRightButtonUp && !(pMouseMsg->GetButton() == kRightButtonUp))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagMiddleButtonUp && !(pMouseMsg->GetButton() == kMiddleButtonUp))
                        continue;
                    if (mouseInfo->fControlFlags & kControlFlagWheel && !(pMouseMsg->GetButton() == kWheelPos || pMouseMsg->GetButton() == kWheelNeg))
                        continue;

                    // okay, we're in the box and either we don't require a button or our button is pressed.
                    // so set the command as 'enabled' 
                    // UNLESS it has kControlFlagInBox, which means we want it sent every frame it is in the box
                    if (!(mouseInfo->fControlFlags & kControlFlagInBox))
                        SetControlFlag(mouseInfo->fCode);
                    // issue the command
                    plCtrlCmd* pCmd = new plCtrlCmd( this );
                    pCmd->fControlActivated = true;
                    pCmd->fControlCode = mouseInfo->fCode;
                    pCmd->fNetPropagateToPlayers = mouseInfo->fControlFlags & kControlFlagNetPropagate;

                    // figure out what percent (if any)
                    float pct = 0.0f;
                    if (mouseInfo->fControlFlags & kControlFlagRangePos)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fX - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fZ - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    else 
                    if (mouseInfo->fControlFlags & kControlFlagRangeNeg)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = fabs((mouseInfo->fBox.fY - pMouseMsg->GetXPos()) / (mouseInfo->fBox.fY - mouseInfo->fBox.fX));
                        else
                            pct = fabs((mouseInfo->fBox.fW - pMouseMsg->GetYPos()) / (mouseInfo->fBox.fW - mouseInfo->fBox.fZ));
                    }
                    pCmd->fPct = pct;
                    if (pct == 1.0f || pct == -1.0f)
                    {
                        delete pCmd;
                        break;
                    }

                    if (mouseInfo->fControlFlags & kControlFlagDelta)
                    {
                        if (mouseInfo->fControlFlags & kControlFlagWheel)
                            pct = pMouseMsg->GetWheelDelta();
                        else if (mouseInfo->fControlFlags & kControlFlagXAxisEvent)
                            pct = pMouseMsg->GetDX();
                        else
                            pct = pMouseMsg->GetDY();
                        
                        pCmd->fPct = pct;
                    }
                    
                    // and add it to the list
                    fMessageQueue->emplace_back(pCmd);
                    continue;
                }
            }
        }
        return true;
    }

    return false;
}   


//////////////////////////////////////////////////////////////////////////////
//// plAvatarInputMap and derivations ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// You really want to think of input maps as various states in a state machine.
// (This is why there are 3 different maps for walking. It all depends on which
// "state" we want to jump to.) 
// 
// When you pop from one map to the next:
//     - All controls in the old map are deactivated 
//       (except kControlFlagNoDeactivate and kControlFlagToggle)
//     - Any controls in the new map that can be activated by the current state
//       will be. (i.e. you press a mouse button to switch to walk mode, hold
//       hold that button down, and the walk command in the new mode will activate)

plAvatarInputMap::plAvatarInputMap()
{
    fMouseMap = new plMouseMap;

    fButtonState = 0;
    fInterface = plAvatarInputInterface::GetInstance();
}

plAvatarInputMap::~plAvatarInputMap()
{
    delete fMouseMap;
}
        
plSuspendedMovementMap::plSuspendedMovementMap() : plAvatarInputMap()
{
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_ACTION_MOUSE,      kControlFlagLeftButtonEx, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_PICK,              kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f));
}

plBasicControlMap::plBasicControlMap() : plSuspendedMovementMap()
{
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_ROTATE_RIGHT,      kControlFlagLeftButton | kControlFlagBoxDisable,    0.95f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_ROTATE_LEFT,       kControlFlagLeftButton | kControlFlagBoxDisable,    0.0f, 0.05f,    0.0f, 1.0f));
    
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_TURN_TO,           kControlFlagLeftButtonEx, 0.05f, 0.95f, 0.0f, 0.95f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_WALK_MODE,             kControlFlagLeftButton, 0.05f, 0.95f,   0.0f, 0.95f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_WALK_BACK_LB_MODE,     kControlFlagLeftButton, 0.05f, 0.95f,   0.95f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_WALK_BACK_MODE,        kControlFlagMiddleButton,   0.05f, 0.95f,   0.0f, 1.0f)); 

    fMouseMap->AddMapping(new plMouseInfo(S_SET_CURSOR_UP,             kControlFlagNormal | kControlFlagInBox,     0.05f, 0.95f, 0.0f, 0.95f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_CURSOR_DOWN,           kControlFlagNormal | kControlFlagInBox,     0.05f, 0.95f, 0.95f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_CURSOR_RIGHT,          kControlFlagNormal | kControlFlagInBox,     0.95f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_CURSOR_LEFT,           kControlFlagNormal | kControlFlagInBox,     0.0f, 0.05f, 0.0f, 1.0f));
}

plLadderControlMap::plLadderControlMap() : plSuspendedMovementMap()
{
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_MOVE_FORWARD,      kControlFlagLeftButton | kControlFlagBoxDisable,    0.0f, 1.0f, 0.0f, 0.5f) ); 
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_MOVE_BACKWARD,     kControlFlagLeftButton | kControlFlagBoxDisable, 0.0f, 1.0f,    0.5f, 1.0f) );

    fMouseMap->AddMapping(new plMouseInfo(S_SET_CURSOR_UPWARD,         kControlFlagNormal | kControlFlagInBox,     0.0f, 1.0f, 0.0f, 0.5f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_CURSOR_DOWN,           kControlFlagNormal | kControlFlagInBox,     0.0f, 1.0f, 0.5f, 1.0f));

    fMouseMap->AddMapping(new plMouseInfo(S_SET_FREELOOK,              kControlFlagRightButton,    0.05f, 0.95f,   0.0f, 0.95f));
}


plLadderMountMap::plLadderMountMap() : plSuspendedMovementMap()
{
    fMouseMap->AddMapping(new plMouseInfo(S_SET_LADDER_CONTROL,                kControlFlagLeftButtonUp,   0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_LADDER_CONTROL,                kControlFlagRightButtonUp,  0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_LADDER_CONTROL,                kControlFlagMiddleButtonUp, 0.0f, 1.0f, 0.0f, 1.0f));
}

plLadderDismountMap::plLadderDismountMap() : plSuspendedMovementMap()
{
    fMouseMap->AddMapping(new plMouseInfo(S_SET_BASIC_MODE,                kControlFlagLeftButtonUp,   0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_BASIC_MODE,                kControlFlagRightButtonUp,  0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_BASIC_MODE,                kControlFlagMiddleButtonUp, 0.0f, 1.0f, 0.0f, 1.0f));
}




plBasicThirdPersonControlMap::plBasicThirdPersonControlMap() : plBasicControlMap()
{   
    fMouseMap->AddMapping(new plMouseInfo(S_SET_FREELOOK,              kControlFlagRightButton,                 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CAMERA_ADJUST_OFFSET,      kControlFlagWheel | kControlFlagDelta,   0.0f, 1.0f, 0.0f, 1.0f));
}   

plBasicFirstPersonControlMap::plBasicFirstPersonControlMap() : plBasicControlMap()
{
    fMouseMap->AddMapping(new plMouseInfo(A_CONTROL_TURN,              kControlFlagRightButtonRepeat | kControlFlagXAxisEvent | kControlFlagDelta, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(S_SET_FREELOOK,              kControlFlagRightButton,    0.05f, 0.95f,   0.0f, 0.95f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,   kControlFlagRightButton,    0.05f, 0.95f,   0.0f, 0.95f));
}

// also used in 1st person walk mode    
pl3rdWalkMap::pl3rdWalkMap() : plAvatarInputMap()
{
    // control special to this mode.
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_MODIFIER_FAST,     kControlFlagRightButton,    0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(A_CONTROL_TURN,              kControlFlagXAxisEvent | kControlFlagDelta, 0.0f, 1.0f, 0.0f, 1.0f));

    plInputManager::SetRecenterMouse(true);     
    plMouseDevice::HideCursor();
    plInputInterfaceMgr::GetInstance()->ForceCursorHidden(true);
}

pl3rdWalkMap::~pl3rdWalkMap()
{
    plInputManager::SetRecenterMouse(false);        
    plMouseDevice::ShowCursor();
    plInputInterfaceMgr::GetInstance()->ForceCursorHidden(false);
}

pl3rdWalkForwardMap::pl3rdWalkForwardMap() : pl3rdWalkMap()
{
    fMouseMap->AddMapping(new plMouseInfo(S_SET_BASIC_MODE,            kControlFlagLeftButtonUp, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_MOVE_FORWARD,      kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,   kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f));
}

pl3rdWalkBackwardMap::pl3rdWalkBackwardMap() : pl3rdWalkMap()
{
    fMouseMap->AddMapping(new plMouseInfo(S_SET_BASIC_MODE,            kControlFlagMiddleButtonUp, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_MOVE_BACKWARD,     kControlFlagMiddleButton, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,   kControlFlagMiddleButton, 0.0f, 1.0f, 0.0f, 1.0f));
}

// same as the other backward walk map, but this one is triggered by the left mouse button.
pl3rdWalkBackwardLBMap::pl3rdWalkBackwardLBMap() : pl3rdWalkMap()
{
    fMouseMap->AddMapping(new plMouseInfo(S_SET_BASIC_MODE,            kControlFlagLeftButtonUp, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_MOVE_BACKWARD,     kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f));
    fMouseMap->AddMapping(new plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,   kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f));
}
