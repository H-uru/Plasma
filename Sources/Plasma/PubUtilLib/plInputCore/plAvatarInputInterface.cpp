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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plAvatarInputInterface													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
//#define LIMIT_VOICE_CHAT 1
#endif

#include "hsConfig.h"
#include "hsWindows.h"
#include "hsTypes.h"
#include "plAvatarInputInterface.h"

#include "../pnInputCore/plKeyMap.h"
#include "../plMessage/plInputEventMsg.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"

#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plProxyDrawMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"

// DEHACK
// used to run debug drawing stuff only; should never be checked in with this enabled
 #if 0
#include "../FeatureLib/pfCamera/plVirtualCam.h"
#include "../plDrawable/plDrawableSpans.h"
 #endif

#include "../plAudio/plVoiceChat.h"
#include "plInputDevice.h"
#include "plInputManager.h"
#include "hsResMgr.h"
#include "plgDispatch.h"

#include "hsConfig.h"
#include "hsMatrix44.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "../pnNetCommon/plNetApp.h"

//// Constructor/Destructor //////////////////////////////////////////////////

plAvatarInputInterface	*plAvatarInputInterface::fInstance = nil;

plAvatarInputInterface::plAvatarInputInterface()
{
	fInstance = this;
	fMouseDisabled = false;
	fCurrentCursor = kCursorUp;
	fCursorOpacity = 1.f;
	fCursorTimeout = 0;
	fCursorFadeDelay = 3.f;
	f3rdPerson = true;
	fInputMap = nil;
	ISetBasicMode(); // Must be after 3rdPerson and fInputMap are set.
	SetEnabled( true );			// Always enabled

	// Add our control codes to our control map. Do NOT add the key bindings yet.
	// Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
	// This part basically declares us master of the bindings for these commands.
	
	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!

#ifndef LIMIT_VOICE_CHAT
	// only allow mapping of 'PushToTalk in online versions'
	fControlMap->AddCode( S_PUSH_TO_TALK,					kControlFlagNormal | kControlFlagNoRepeat );
#endif
	fControlMap->AddCode( S_SET_FIRST_PERSON_MODE,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_EXIT_MODE,				kControlFlagNormal | kControlFlagNoRepeat );

	fControlMap->AddCode( B_CAMERA_ZOOM_IN,					kControlFlagNormal );
	fControlMap->AddCode( B_CAMERA_ZOOM_OUT,				kControlFlagNormal );

	fControlMap->AddCode( B_CONTROL_MODIFIER_FAST,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_MODIFIER_STRAFE,		kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_MOVE_FORWARD,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_MOVE_BACKWARD,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_ROTATE_LEFT,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_ROTATE_RIGHT,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_STRAFE_LEFT,			kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_STRAFE_RIGHT,			kControlFlagNormal | kControlFlagNoRepeat );
		

	fControlMap->AddCode( B_CONTROL_ALWAYS_RUN,				kControlFlagToggle | kControlFlagUpEvent | kControlFlagNoRepeat );


	fControlMap->AddCode( B_CONTROL_JUMP,					kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_DIVE,					kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_IGNORE_AVATARS,			kControlFlagNormal | kControlFlagNoRepeat );

	fControlMap->AddConsoleCommand( "Game.EnterChatMode" );
	fControlMap->AddConsoleCommand( "Game.Emote.wave" );
	fControlMap->AddConsoleCommand( "Game.Emote.laugh" );
	fControlMap->AddConsoleCommand( "Game.Emote.clap" );
	fControlMap->AddConsoleCommand( "Game.Emote.dance" );
	fControlMap->AddConsoleCommand( "Game.Emote.talk" );
	fControlMap->AddConsoleCommand( "Game.Emote.sneeze" );
	fControlMap->AddConsoleCommand( "Game.Emote.sit" );
	fControlMap->AddConsoleCommand( "Keyboard.ResetBindings" );
	
	fControlMap->AddConsoleCommand( "Game.KIOpenKI" );
	fControlMap->AddConsoleCommand( "Game.KIHelp" );
	fControlMap->AddConsoleCommand( "Game.KICreateMarker" );
	fControlMap->AddConsoleCommand( "Game.KICreateMarkerFolder" );
	fControlMap->AddConsoleCommand( "Game.KIOpenYeeshaBook" );
	fControlMap->AddConsoleCommand( "Game.KIToggleMini" );
	fControlMap->AddConsoleCommand( "Game.KIPutAway" );
	fControlMap->AddConsoleCommand( "Game.KIChatPageUp" );
	fControlMap->AddConsoleCommand( "Game.KIChatPageDown" );
	fControlMap->AddConsoleCommand( "Game.KIChatToStart" );
	fControlMap->AddConsoleCommand( "Game.KIChatToEnd" );
	fControlMap->AddConsoleCommand( "Game.KIUpSizeFont" );
	fControlMap->AddConsoleCommand( "Game.KIDownSizeFont" );
	fControlMap->AddConsoleCommand( "Game.KITakePicture" );
	fControlMap->AddConsoleCommand( "Game.KICreateJournal" );

#ifndef PLASMA_EXTERNAL_RELEASE
	fControlMap->AddCode( B_CONTROL_TOGGLE_PHYSICAL,		kControlFlagDownEvent | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_MOVE_UP,				kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_CONTROL_MOVE_DOWN,				kControlFlagNormal | kControlFlagNoRepeat );
	fControlMap->AddCode( B_TOGGLE_DRIVE_MODE,				kControlFlagDownEvent | kControlFlagNoRepeat );
	fControlMap->AddConsoleCommand( "NextStatusLog" );
#endif

	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!
}

plAvatarInputInterface::~plAvatarInputInterface()
{
	delete fInputMap;
}

//// Init/Shutdown ///////////////////////////////////////////////////////////

void	plAvatarInputInterface::Init( plInputInterfaceMgr *manager )
{
	plInputInterface::Init( manager );
}

void	plAvatarInputInterface::Shutdown( void )
{
}

void plAvatarInputInterface::CameraInThirdPerson(hsBool state)
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
		plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
		pCmd->fNetPropagateToPlayers = info->fControlFlags & kControlFlagNetPropagate;
		pCmd->fControlActivated = false;
		pCmd->fControlCode = info->fCode;
		IClearControlFlag(pCmd->fControlCode);
		fMessageQueue->Append(pCmd);
	}
}

//// IChangeInputMaps ////////////////////////////////////////////////////////

void plAvatarInputInterface::IChangeInputMaps( plAvatarInputMap *newMap )
{
	newMap->fButtonState = fInputMap ? fInputMap->fButtonState : 0;	

	if (fInputMap)
	{
		int i;
		for (i = 0; i < fInputMap->fMouseMap->fMap.GetCount(); i++)
		{
			plMouseInfo *info = fInputMap->fMouseMap->fMap[i];
			IDeactivateCommand(info);
		}
		delete fInputMap;
	}

	fInputMap = newMap;
//	fInputMap->fButtonState = 0;
//	Reset();
}

void plAvatarInputInterface::ISetSuspendMovementMode()
{
	IChangeInputMaps(TRACKED_NEW plSuspendedMovementMap());
	fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetLadderMap()
{
	IChangeInputMaps(TRACKED_NEW plLadderControlMap());
	fCurrentCursor = kCursorUp;
}


void plAvatarInputInterface::ISetPreLadderMap()
{
	IChangeInputMaps(TRACKED_NEW plLadderMountMap());
	fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetPostLadderMap()
{
	IChangeInputMaps(TRACKED_NEW plLadderDismountMap());
	fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetBasicMode()
{
	plAvatarInputMap *map;
	if (!f3rdPerson)
		map = TRACKED_NEW plBasicFirstPersonControlMap();
	else
		map = TRACKED_NEW plBasicThirdPersonControlMap();

	IChangeInputMaps(map);
	fCurrentCursor = kCursorUp;
}

void plAvatarInputInterface::ISetMouseWalkMode(ControlEventCode code)
{
	if (code == S_SET_WALK_BACK_MODE)
		IChangeInputMaps(TRACKED_NEW pl3rdWalkBackwardMap());
	else if (code == S_SET_WALK_BACK_LB_MODE)
		IChangeInputMaps(TRACKED_NEW pl3rdWalkBackwardLBMap());
	else
		IChangeInputMaps(TRACKED_NEW pl3rdWalkForwardMap());

	fCurrentCursor = kCursorHidden;
}

//// ClearKeyMap ///////////////////////////////////////////////
void plAvatarInputInterface::ClearKeyMap()
{ 
	// Note: we might be clearing our key bindings, but we still want to be owners of the commands,
	if( fControlMap != nil )
	{
		fControlMap->UnmapAllBindings();
		
		// Still want this one tho
		fControlMap->BindKeyToConsoleCmd( plCtrlShiftKeyCombo( KEY_0 ), "Keyboard.ResetBindings" );
	}
}

//// RestoreDefaultKeyMappings ///////////////////////////////////////////////

void	plAvatarInputInterface::RestoreDefaultKeyMappings( void )
{
	if( fControlMap == nil )
		return;

	fControlMap->UnmapAllBindings();

#ifndef LIMIT_VOICE_CHAT
	fControlMap->BindKey( KEY_TAB,						S_PUSH_TO_TALK );
#endif
	fControlMap->BindKey( KEY_F1,						S_SET_FIRST_PERSON_MODE );
	fControlMap->BindKey( plCtrlKeyCombo( KEY_F ),		S_SET_FIRST_PERSON_MODE );
	fControlMap->BindKey( KEY_BACKSPACE,				B_CONTROL_EXIT_MODE );
	fControlMap->BindKey( KEY_ESCAPE,					B_CONTROL_EXIT_MODE );

	fControlMap->BindKey( KEY_NUMPAD_ADD,				B_CAMERA_ZOOM_IN );
	fControlMap->BindKey( KEY_NUMPAD_SUBTRACT,			B_CAMERA_ZOOM_OUT );

	fControlMap->BindKey( KEY_SHIFT,					B_CONTROL_MODIFIER_FAST );
	fControlMap->BindKey( KEY_Z,						B_CONTROL_MODIFIER_STRAFE );
	fControlMap->BindKey( KEY_UP,						B_CONTROL_MOVE_FORWARD );
	fControlMap->BindKey( KEY_DOWN,						B_CONTROL_MOVE_BACKWARD );
	fControlMap->BindKey( KEY_LEFT,						B_CONTROL_ROTATE_LEFT );
	fControlMap->BindKey( KEY_RIGHT,					B_CONTROL_ROTATE_RIGHT );
	fControlMap->BindKey( KEY_COMMA,					B_CONTROL_STRAFE_LEFT );
	fControlMap->BindKey( KEY_PERIOD,					B_CONTROL_STRAFE_RIGHT );

// This is now hard-coded to capslock
//	fControlMap->BindKey( KEY_CAPSLOCK,					B_CONTROL_ALWAYS_RUN );

	fControlMap->BindKey( KEY_SPACE,					B_CONTROL_JUMP );
//	fControlMap->BindKey( KEY_D,						B_CONTROL_DIVE );
	fControlMap->BindKey( KEY_DELETE,					B_CONTROL_IGNORE_AVATARS );

	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_1 ),	"Game.Emote.wave" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_2 ),	"Game.Emote.laugh" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_3 ),	"Game.Emote.clap" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_4 ),	"Game.Emote.dance" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_5 ),	"Game.Emote.talk" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_6 ),	"Game.Emote.sneeze" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_7 ),	"Game.Emote.sit" );

	fControlMap->BindKeyToConsoleCmd( plCtrlShiftKeyCombo( KEY_0 ),	"Keyboard.ResetBindings" );
	
	// KI shortcut keyboard commands
	fControlMap->BindKeyToConsoleCmd( KEY_F2,									"Game.KIOpenKI" );
	fControlMap->BindKeyToConsoleCmd( KEY_F3,									"Game.KIOpenYeeshaBook" );
	fControlMap->BindKeyToConsoleCmd( KEY_F4,									"Game.KIHelp" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_HOME ),				"Game.KIToggleMini" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_END ),				"Game.KIPutAway" );
	fControlMap->BindKeyToConsoleCmd( KEY_PAGEUP,								"Game.KIChatPageUp" );
	fControlMap->BindKeyToConsoleCmd( KEY_PAGEDOWN,								"Game.KIChatPageDown" );
	fControlMap->BindKeyToConsoleCmd( KEY_HOME,									"Game.KIChatToStart" );
	fControlMap->BindKeyToConsoleCmd( KEY_END,									"Game.KIChatToEnd" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_NUMPAD_ADD ),			"Game.KIUpSizeFont" );
	fControlMap->BindKeyToConsoleCmd( plCtrlKeyCombo( KEY_NUMPAD_SUBTRACT ),	"Game.KIDownSizeFont" );
	fControlMap->BindKeyToConsoleCmd( KEY_F5,									"Game.KITakePicture" );
	fControlMap->BindKeyToConsoleCmd( KEY_F6,									"Game.KICreateJournal" );
	fControlMap->BindKeyToConsoleCmd( KEY_F7,									"Game.KICreateMarker" );
	fControlMap->BindKeyToConsoleCmd( KEY_F8,									"Game.KICreateMarkerFolder" );

#ifndef PLASMA_EXTERNAL_RELEASE
	fControlMap->BindKey( plShiftKeyCombo( KEY_P ),		B_CONTROL_TOGGLE_PHYSICAL );
	fControlMap->BindKey( KEY_U,						B_CONTROL_MOVE_UP );
	fControlMap->BindKey( KEY_H,						B_CONTROL_MOVE_DOWN );
	fControlMap->BindKey( plShiftKeyCombo( KEY_C ),		B_TOGGLE_DRIVE_MODE );
	
	fControlMap->BindKeyToConsoleCmd( KEY_L,			"NextStatusLog" );
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

void plAvatarInputInterface::EnableJump(hsBool val)
{
	EnableControl(val, B_CONTROL_JUMP);
}

void plAvatarInputInterface::EnableForwardMovement(hsBool val)
{
	EnableControl(val, B_CONTROL_MOVE_FORWARD);
}

void plAvatarInputInterface::EnableControl(hsBool val, ControlEventCode code)
{
	if (val)
		IEnableControl(code);
	else
		IDisableControl(code);
}

void plAvatarInputInterface::ForceAlwaysRun(hsBool val)
{
	plCtrlCmd *pCmd = TRACKED_NEW plCtrlCmd( this );
	pCmd->fControlCode = B_CONTROL_ALWAYS_RUN;
	pCmd->fControlActivated = val;
	pCmd->fNetPropagateToPlayers = false;
		
	fMessageQueue->Append( pCmd );
}	

//// IEval ///////////////////////////////////////////////////////////////////
//	Gets called once per IUpdate(), just like normal IEval()s

hsBool plAvatarInputInterface::IEval( double secs, hsScalar del, UInt32 dirty )
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

hsBool	plAvatarInputInterface::IHandleCtrlCmd( plCtrlCmd *cmd )
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
		case S_SET_LADDER_CONTROL:		
			if( cmd->fControlActivated )
				ISetLadderMap();			
			return true;
#if 0
		case S_SET_FIRST_PERSON_MODE:
			if( cmd->fControlActivated )
				IChangeInputMaps( TRACKED_NEW plFirstPersonControlMap() );
			return true;
#endif			
		case S_SET_BASIC_MODE:
			if( cmd->fControlActivated )
			{
				ISetBasicMode();				
#if 0
plProxyDrawMsg* Dmsg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kCamera | plProxyDrawMsg::kDestroy);
plgDispatch::MsgSend(Dmsg);
plVirtualCam::Instance()->GetPipeline()->SetDrawableTypeMask(plVirtualCam::Instance()->GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kCameraProxy);
#endif
			}
			return true;


		case S_SET_WALK_MODE:

			if( cmd->fControlActivated )
			{
				hsBool abort = false;
				for (int i = 0; i < fMessageQueue->GetCount(); i++)
				{
					if ((*fMessageQueue)[i]->fControlCode == S_SET_WALK_MODE && !(*fMessageQueue)[i]->fControlActivated)
					{	
						abort = true;
#if 0						

					plProxyDrawMsg* Dmsg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kCamera | plProxyDrawMsg::kDestroy);
					plgDispatch::MsgSend(Dmsg);
					plVirtualCam::Instance()->GetPipeline()->SetDrawableTypeMask(plVirtualCam::Instance()->GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kCameraProxy);
#endif
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
				hsBool abort = false;
				for (int i = 0; i < fMessageQueue->GetCount(); i++)
				{
					if ((*fMessageQueue)[i]->fControlCode == S_SET_WALK_BACK_MODE && !(*fMessageQueue)[i]->fControlActivated)
					{	
						abort = true;
#if 0						
						
						plProxyDrawMsg* Dmsg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kCamera | plProxyDrawMsg::kDestroy);
						plgDispatch::MsgSend(Dmsg);
						plVirtualCam::Instance()->GetPipeline()->SetDrawableTypeMask(plVirtualCam::Instance()->GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kCameraProxy);
#endif
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
				hsBool abort = false;
				for (int i = 0; i < fMessageQueue->GetCount(); i++)
				{
					if ((*fMessageQueue)[i]->fControlCode == S_SET_WALK_BACK_MODE && !(*fMessageQueue)[i]->fControlActivated)
					{	
						abort = true;
#if 0						
						
						plProxyDrawMsg* Dmsg = TRACKED_NEW plProxyDrawMsg(plProxyDrawMsg::kCamera | plProxyDrawMsg::kDestroy);
						plgDispatch::MsgSend(Dmsg);
						plVirtualCam::Instance()->GetPipeline()->SetDrawableTypeMask(plVirtualCam::Instance()->GetPipeline()->GetDrawableTypeMask() & ~plDrawableSpans::kCameraProxy);
#endif
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

	/*			case B_CONTROL_ACTION:
			{
				if (fMessageQueue[i]->fControlActivated)
				{	
					// send a 'picked' message to the picked object
					plPickedMsg* pPickedMsg = TRACKED_NEW plPickedMsg;
					pPickedMsg->AddReceiver(fCurrentClickable);
					plgDispatch::MsgSend(pPickedMsg);
				}
				else
				{
					// send an 'unpicked message'
					plPickedMsg* pPickedMsg = TRACKED_NEW plPickedMsg;
					pPickedMsg->AddReceiver(fCurrentClickable);
					pPickedMsg->fPicked = false;
					plgDispatch::MsgSend(pPickedMsg);
				}
			}
			break;
	*/
	} 

	return false;
}

hsBool plAvatarInputInterface::CursorInBox(plMouseEventMsg* pMsg, hsPoint4 box)
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

hsBool	plAvatarInputInterface::MsgReceive( plMessage *msg )
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
//	If we "missed" an input event, then somebody caught it above us, thus we
//	have "lost focus" in a way. So we should stop walking/moving/whatever.
//	Should this be in the base inputInterface, since it deals with key
//	bindings? Perhaps, dunno yet. We'll see...

void	plAvatarInputInterface::MissedInputEvent( plInputEventMsg *pMsg )
{
	int					i;


	if( plKeyEventMsg::ConvertNoRef( pMsg ) == nil )
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
			plCtrlCmd *pCmd = TRACKED_NEW plCtrlCmd( this );
			pCmd->fControlCode = binding.GetCode();
			pCmd->fControlActivated = false;
			pCmd->SetCmdString( binding.GetExtendedString() );

			if( binding.GetCodeFlags() & kControlFlagNetPropagate )
				pCmd->fNetPropagateToPlayers = true;
			else
				pCmd->fNetPropagateToPlayers = false;

			fMessageQueue->Append( pCmd );
			IClearKeyControlFlag( binding.GetCode() );
		}
	}
}

hsBool plAvatarInputInterface::IsEnterChatModeBound()
{
	int i;
	for ( i=0; i< fControlMap->GetNumBindings(); i++ )
	{
		const plKeyBinding &binding = fControlMap->GetBinding( i );

		const char* extString = binding.GetExtendedString();
		if ( extString && strcmp("Game.EnterChatMode",extString) == 0 )
		{
			if (binding.GetKey1() != plKeyCombo::kUnmapped )
				return true;
		}
	}
	return false;
}


//// InterpretInputEvent /////////////////////////////////////////////////////

hsBool plAvatarInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
	if( fInputMap == nil )
		return false;

	plMouseMap	*mouseMap = fInputMap->fMouseMap;


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
		UInt32 oldButtonState = fInputMap->fButtonState;

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
		
		if( oldButtonState != fInputMap->fButtonState || pMouseMsg->GetDX() != 0.f || pMouseMsg->GetDY() != 0.f )
		{
			fCursorTimeout = 0.f;	// Reset cursor opacity timeout thingy
		}

		/*	NOTE: I see that this interface always returns true for mouse
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

		for (int i=0; i < mouseMap->fMap.Count(); i++)
		{
			// is this control already set?
			if (IHasControlFlag(mouseMap->fMap[i]->fCode))
			{
				// Control isn't enabled, ignore
				if (!IControlCodeEnabled(mouseMap->fMap[i]->fCode))
					return true;				
				
				// can we disable this control?
				hsBool disable = false;
				
				// can we disable this control based on a button?
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButton && !(fInputMap->fButtonState & kLeftButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButton && !(fInputMap->fButtonState & kRightButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButton && !(fInputMap->fButtonState & kMiddleButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonEx && (fInputMap->fButtonState & kLeftButtonRepeat))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonEx && (fInputMap->fButtonState & kRightButtonRepeat))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonEx && (fInputMap->fButtonState & kMiddleButtonRepeat))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonEx && !(fInputMap->fButtonState & kLeftButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonEx && !(fInputMap->fButtonState & kRightButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonEx && !(fInputMap->fButtonState & kMiddleButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonRepeat && !(fInputMap->fButtonState & kLeftButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonRepeat && !(fInputMap->fButtonState & kRightButtonDown))
					disable = true;
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonRepeat && !(fInputMap->fButtonState & kMiddleButtonDown))
					disable = true;

				// can we disable this control based on the cursor position?
				if (!CursorInBox(pMouseMsg, mouseMap->fMap[i]->fBox) && mouseMap->fMap[i]->fControlFlags & kControlFlagBoxDisable)
					disable = true;

				if (disable)
				{
					IDeactivateCommand(mouseMap->fMap[i]);
					continue;
				}
				// is it a range control?  If so we need to re-send the command
				
				if ((mouseMap->fMap[i]->fControlFlags & kControlFlagRangePos) || (mouseMap->fMap[i]->fControlFlags & kControlFlagRangeNeg))
				{
					plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
					pCmd->fControlActivated = true;
					pCmd->fControlCode = mouseMap->fMap[i]->fCode;
					hsScalar pct = 0.0f;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRangePos)
					{
						if (mouseMap->fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((mouseMap->fMap[i]->fBox.fX - pMouseMsg->GetXPos()) / (mouseMap->fMap[i]->fBox.fY - mouseMap->fMap[i]->fBox.fX));
						else
							pct = hsABS((mouseMap->fMap[i]->fBox.fZ - pMouseMsg->GetYPos()) / (mouseMap->fMap[i]->fBox.fW - mouseMap->fMap[i]->fBox.fZ));
					}
					else 
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRangeNeg)
					{
						if (mouseMap->fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((mouseMap->fMap[i]->fBox.fY - pMouseMsg->GetXPos()) / (mouseMap->fMap[i]->fBox.fY - mouseMap->fMap[i]->fBox.fX));
						else
							pct = hsABS((mouseMap->fMap[i]->fBox.fW - pMouseMsg->GetYPos()) / (mouseMap->fMap[i]->fBox.fW - mouseMap->fMap[i]->fBox.fZ));
					}
					pCmd->fPct = pct;
					if (pct == 1.0f || pct == -1.0f)
					{
						delete pCmd;
						break;
					}
					pCmd->fNetPropagateToPlayers = mouseMap->fMap[i]->fControlFlags & kControlFlagNetPropagate;
					fMessageQueue->Append(pCmd);
				}
				if (mouseMap->fMap[i]->fControlFlags & kControlFlagDelta)
				{
					plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
					pCmd->fControlActivated = true;
					pCmd->fControlCode = mouseMap->fMap[i]->fCode;
					hsScalar pct = 0.0f;

					if (mouseMap->fMap[i]->fControlFlags & kControlFlagXAxisEvent)
						pct = pMouseMsg->GetDX();
					else
						pct = pMouseMsg->GetDY();
				
					if (pct == 0.f)
					{
						delete pCmd;
						continue;
					}
					pCmd->fPct = pct;
					pCmd->fNetPropagateToPlayers = mouseMap->fMap[i]->fControlFlags & kControlFlagNetPropagate;
					fMessageQueue->Append(pCmd);
				}
				
			}
			else // if it is an 'always if in box' command see if it's not in the box
			if ( (mouseMap->fMap[i]->fControlFlags & kControlFlagInBox) && (!CursorInBox(pMouseMsg, mouseMap->fMap[i]->fBox)) )
			{	
				plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
				pCmd->fControlActivated = false;
				pCmd->fControlCode = mouseMap->fMap[i]->fCode;
				pCmd->fNetPropagateToPlayers = mouseMap->fMap[i]->fControlFlags & kControlFlagNetPropagate;
				fMessageQueue->Append(pCmd);
				continue;
			}
			else // the control is not set, see if we should set it.
			{
				// is the control disabled?
				if (fDisabledControls.IsBitSet(mouseMap->fMap[i]->fCode))
					continue;
				
				// is the cursor in the appropriate box?
				if (CursorInBox(pMouseMsg, mouseMap->fMap[i]->fBox))
				{	
					// do we require a button?
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButton && !(fInputMap->fButtonState & kLeftButtonDown))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButton && !(fInputMap->fButtonState & kRightButtonDown))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButton && !(fInputMap->fButtonState & kMiddleButtonDown))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonEx && (fInputMap->fButtonState & kLeftButtonRepeat))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonEx && (fInputMap->fButtonState & kRightButtonRepeat))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonEx && (fInputMap->fButtonState & kMiddleButtonRepeat))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonRepeat && !(fInputMap->fButtonState & kLeftButtonRepeat))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonRepeat && !(fInputMap->fButtonState & kRightButtonRepeat))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonRepeat && !(fInputMap->fButtonState & kMiddleButtonRepeat))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonEx && !(fInputMap->fButtonState & kLeftButtonDown))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonEx && !(fInputMap->fButtonState & kLeftButtonDown))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonEx && !(fInputMap->fButtonState & kMiddleButtonDown))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagLeftButtonUp && !(pMouseMsg->GetButton() == kLeftButtonUp))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRightButtonUp && !(pMouseMsg->GetButton() == kRightButtonUp))
						continue;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagMiddleButtonUp && !(pMouseMsg->GetButton() == kMiddleButtonUp))
						continue;

					// okay, we're in the box and either we don't require a button or our button is pressed.
					// so set the command as 'enabled' 
					// UNLESS it has kControlFlagInBox, which means we want it sent every frame it is in the box
					if (!(mouseMap->fMap[i]->fControlFlags & kControlFlagInBox))
						SetControlFlag(mouseMap->fMap[i]->fCode);
					// issue the command
					plCtrlCmd* pCmd = TRACKED_NEW plCtrlCmd( this );
					pCmd->fControlActivated = true;
					pCmd->fControlCode = mouseMap->fMap[i]->fCode;
					pCmd->fNetPropagateToPlayers = mouseMap->fMap[i]->fControlFlags & kControlFlagNetPropagate;

					// figure out what percent (if any)
					hsScalar pct = 0.0f;
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRangePos)
					{
						if (mouseMap->fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((mouseMap->fMap[i]->fBox.fX - pMouseMsg->GetXPos()) / (mouseMap->fMap[i]->fBox.fY - mouseMap->fMap[i]->fBox.fX));
						else
							pct = hsABS((mouseMap->fMap[i]->fBox.fZ - pMouseMsg->GetYPos()) / (mouseMap->fMap[i]->fBox.fW - mouseMap->fMap[i]->fBox.fZ));
					}
					else 
					if (mouseMap->fMap[i]->fControlFlags & kControlFlagRangeNeg)
					{
						if (mouseMap->fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = hsABS((mouseMap->fMap[i]->fBox.fY - pMouseMsg->GetXPos()) / (mouseMap->fMap[i]->fBox.fY - mouseMap->fMap[i]->fBox.fX));
						else
							pct = hsABS((mouseMap->fMap[i]->fBox.fW - pMouseMsg->GetYPos()) / (mouseMap->fMap[i]->fBox.fW - mouseMap->fMap[i]->fBox.fZ));
					}
					pCmd->fPct = pct;
					if (pct == 1.0f || pct == -1.0f)
					{
						delete pCmd;
						break;
					}

					if (mouseMap->fMap[i]->fControlFlags & kControlFlagDelta)
					{
						if (mouseMap->fMap[i]->fControlFlags & kControlFlagXAxisEvent)
							pct = pMouseMsg->GetDX();
						else
							pct = pMouseMsg->GetDY();
						
						pCmd->fPct = pct;
					}
					
					// and add it to the list
					fMessageQueue->Append(pCmd);
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
	fMouseMap = TRACKED_NEW plMouseMap;

	fButtonState = 0;
	fInterface = plAvatarInputInterface::GetInstance();
}

plAvatarInputMap::~plAvatarInputMap()
{
	delete fMouseMap;
}
		
plSuspendedMovementMap::plSuspendedMovementMap() : plAvatarInputMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_ACTION_MOUSE,		kControlFlagLeftButtonEx, 0.0f, 1.0f, 0.0f, 1.0f, "The Picked key") );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_PICK,				kControlFlagLeftButton,	0.0f, 1.0f, 0.0f, 1.0f,	"The Picked key") );
}

plBasicControlMap::plBasicControlMap() : plSuspendedMovementMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_ROTATE_RIGHT,		kControlFlagLeftButton | kControlFlagBoxDisable,	0.95f, 1.0f, 0.0f, 1.0f,		"Rotate Player Right") );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_ROTATE_LEFT,		kControlFlagLeftButton | kControlFlagBoxDisable,	0.0f, 0.05f,	0.0f, 1.0f, 	"Rotate Player Left") );
	
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_TURN_TO,			kControlFlagLeftButtonEx, 0.05f, 0.95f,	0.0f, 0.95f,		"Turn to") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_WALK_MODE,				kControlFlagLeftButton,	0.05f, 0.95f,	0.0f, 0.95f,		"Set Walk Mode") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_WALK_BACK_LB_MODE,		kControlFlagLeftButton, 0.05f, 0.95f,	0.95f, 1.0f,		"Set Walk Back LB Mode") );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_WALK_BACK_MODE,		kControlFlagMiddleButton,	0.05f, 0.95f,	0.0f, 1.0f,		"Set Walk Back Mode") ); 

	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_CURSOR_UP,				kControlFlagNormal | kControlFlagInBox,		0.05f, 0.95f, 0.0f, 0.95f,	"set cursor up") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_CURSOR_DOWN,			kControlFlagNormal | kControlFlagInBox,		0.05f, 0.95f, 0.95f, 1.0f,	"set cursor down") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_CURSOR_RIGHT,			kControlFlagNormal | kControlFlagInBox,		0.95f, 1.0f, 0.0f, 1.0f,	"set cursor right") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_CURSOR_LEFT,			kControlFlagNormal | kControlFlagInBox,		0.0f, 0.05f, 0.0f, 1.0f,	"set cursor left") ); 
}

plLadderControlMap::plLadderControlMap() : plSuspendedMovementMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_MOVE_FORWARD,		kControlFlagLeftButton | kControlFlagBoxDisable,	0.0f, 1.0f,	0.0f, 0.5f,		"Set Walk Mode") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_MOVE_BACKWARD,		kControlFlagLeftButton | kControlFlagBoxDisable, 0.0f, 1.0f,	0.5f, 1.0f,		"Set Walk Back LB Mode") );

	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_CURSOR_UPWARD,			kControlFlagNormal | kControlFlagInBox,		0.0f, 1.0f,	0.0f, 0.5f,	"set cursor up") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_CURSOR_DOWN,			kControlFlagNormal | kControlFlagInBox,		0.0f, 1.0f,	0.5f, 1.0f,	"set cursor down") ); 

	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_FREELOOK,				kControlFlagRightButton,	0.05f, 0.95f,	0.0f, 0.95f,		"Set Camera first-person z-axis panning") ); 
}


plLadderMountMap::plLadderMountMap() : plSuspendedMovementMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_LADDER_CONTROL,				kControlFlagLeftButtonUp,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Ladder Mode") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_LADDER_CONTROL,				kControlFlagRightButtonUp,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Ladder Mode") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_LADDER_CONTROL,				kControlFlagMiddleButtonUp,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Ladder Mode") ); 
}

plLadderDismountMap::plLadderDismountMap() : plSuspendedMovementMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_BASIC_MODE,				kControlFlagLeftButtonUp,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Basic Mode") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_BASIC_MODE,				kControlFlagRightButtonUp,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Basic Mode") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_BASIC_MODE,				kControlFlagMiddleButtonUp,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Basic Mode") ); 
}




plBasicThirdPersonControlMap::plBasicThirdPersonControlMap() : plBasicControlMap()
{	
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_FREELOOK,				kControlFlagRightButton,	0.0f, 1.0f,	0.0f, 1.0f,		"Freelook Mode") ); 	
}	

plBasicFirstPersonControlMap::plBasicFirstPersonControlMap() : plBasicControlMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(A_CONTROL_TURN,				kControlFlagRightButtonRepeat | kControlFlagXAxisEvent | kControlFlagDelta,	0.0f, 1.0f, 0.0f, 1.0f,		"Rotate Player") );	
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_FREELOOK,				kControlFlagRightButton,	0.05f, 0.95f,	0.0f, 0.95f,		"Set Camera first-person z-axis panning") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,	kControlFlagRightButton,	0.05f, 0.95f,	0.0f, 0.95f,		"Set Camera first-person z-axis panning") ); 
}

// also used in 1st person walk mode	
pl3rdWalkMap::pl3rdWalkMap() : plAvatarInputMap()
{
	// control special to this mode.
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_MODIFIER_FAST,		kControlFlagRightButton,	0.0f, 1.0f, 0.0f, 1.0f,	"Run Modifier"	) );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(A_CONTROL_TURN,				kControlFlagXAxisEvent | kControlFlagDelta,	0.0f, 1.0f, 0.0f, 1.0f,		"Rotate Player") );

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
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_BASIC_MODE,			kControlFlagLeftButtonUp, 0.0f, 1.0f, 0.0f, 1.0f,		"Third Person") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_MOVE_FORWARD,		kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f,		"Walk forward") );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,	kControlFlagLeftButton,	0.0f, 1.0f,	0.0f, 1.0f,		"Set Camera first-person z-axis panning") ); 	
}

pl3rdWalkBackwardMap::pl3rdWalkBackwardMap() : pl3rdWalkMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_BASIC_MODE,			kControlFlagMiddleButtonUp, 0.0f, 1.0f, 0.0f, 1.0f,		"Third Person") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_MOVE_BACKWARD,		kControlFlagMiddleButton, 0.0f, 1.0f, 0.0f, 1.0f,		"Walk backward") );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,	kControlFlagMiddleButton, 0.0f, 1.0f, 0.0f, 1.0f,		"Set Camera first-person z-axis panning") ); 
}

// same as the other backward walk map, but this one is triggered by the left mouse button.
pl3rdWalkBackwardLBMap::pl3rdWalkBackwardLBMap() : pl3rdWalkMap()
{
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(S_SET_BASIC_MODE,			kControlFlagLeftButtonUp, 0.0f, 1.0f, 0.0f, 1.0f,		"Third Person") ); 
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_MOVE_BACKWARD,		kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f,		"Walk backward") );
	fMouseMap->AddMapping( TRACKED_NEW plMouseInfo(B_CONTROL_CAMERA_WALK_PAN,	kControlFlagLeftButton, 0.0f, 1.0f, 0.0f, 1.0f,		"Set Camera first-person z-axis panning") ); 
}