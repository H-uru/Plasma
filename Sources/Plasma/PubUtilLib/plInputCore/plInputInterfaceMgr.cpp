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
//  plInputInterfaceMgr.cpp - The manager of all input interface layers     //
//                                                                          //
//// History /////////////////////////////////////////////////////////////////
//                                                                          //
//  2.20.02 mcn - Created.                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plInputInterfaceMgr.h"
#include "plInputInterface.h"
#include "plInputDevice.h"      // For mouse device stuff

#include "pnInputCore/plKeyMap.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "pnMessage/plClientMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "pnMessage/plPlayerPageMsg.h"

#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"

#include "pnNetCommon/plNetApp.h"
#include "plNetClient/plNetClientMgr.h"

#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plProfile.h"
#include "plResMgr/plLocalization.h"

plProfile_CreateTimer("Input", "Update", Input);

//// plCtrlCmd ///////////////////////////////////////////////////////////////

void plCtrlCmd::Write(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32( fControlCode );
    stream->WriteBool( fControlActivated );
    fPt.Write(stream);

    // write cmd/string
    plMsgCStringHelper::Poke(fCmd, stream);
}

void plCtrlCmd::Read(hsStream* stream, hsResMgr* mgr)
{
    fControlCode = (ControlEventCode)stream->ReadLE32();
    fControlActivated = stream->ReadBool();
    fPt.Read(stream);

    // read cmd/string
    plMsgCStringHelper::Peek(fCmd, stream);
}

//// plDefaultKeyCatcher /////////////////////////////////////////////////////

plDefaultKeyCatcher::~plDefaultKeyCatcher()
{
    if( plInputInterfaceMgr::GetInstance() != nil )
        plInputInterfaceMgr::GetInstance()->SetDefaultKeyCatcher( nil );
}

//// Statics /////////////////////////////////////////////////////////////////

plInputInterfaceMgr *plInputInterfaceMgr::fInstance = nil;


//// Constructor/Destructor //////////////////////////////////////////////////

plInputInterfaceMgr::plInputInterfaceMgr()
{
    fClickEnabled = false;
    fCurrentCursor = -1;
    fCursorOpacity = 1.f;
    fForceCursorHidden = false;
    fForceCursorHiddenCount = 0;
    InitDefaultKeyMap();

#if 0
    // make sure we don't miss control keys on remote players
    SetSynchFlagsBit(plSynchedObject::kSendReliably);
#endif
    hsAssert( fInstance == nil, "Attempting to create two input interface managers!" );
    fInstance = this;

    fCurrentFocus = nil;
    fDefaultCatcher = nil;
}

plInputInterfaceMgr::~plInputInterfaceMgr()
{
    Shutdown();
    fInstance = nil;
}

//// Init ////////////////////////////////////////////////////////////////////

#include "plAvatarInputInterface.h"
#include "plSceneInputInterface.h"
#include "plDebugInputInterface.h"
#include "plTelescopeInputInterface.h"

void    plInputInterfaceMgr::Init()
{
    RegisterAs( kInputInterfaceMgr_KEY );

    plgDispatch::Dispatch()->RegisterForType( plInputIfaceMgrMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->RegisterForType( plInputEventMsg::Index(), GetKey() );

    plgDispatch::Dispatch()->RegisterForType( plEvalMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plPlayerPageMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plCmdIfaceModMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plClientMsg::Index(), GetKey() );
    
    /// Hacks (?) for now
    plAvatarInputInterface *avatar = new plAvatarInputInterface();
    IAddInterface( avatar );
    hsRefCnt_SafeUnRef( avatar );

    plSceneInputInterface *scene = new plSceneInputInterface();
    IAddInterface( scene );
    hsRefCnt_SafeUnRef( scene );

    plDebugInputInterface *camDrive = new plDebugInputInterface();
    IAddInterface( camDrive );
    hsRefCnt_SafeUnRef( camDrive );
    
}

//// Shutdown ////////////////////////////////////////////////////////////////

void    plInputInterfaceMgr::Shutdown()
{
    int i;


//  WriteKeyMap();

    for( i = 0; i < fInterfaces.GetCount(); i++ )
    {
        fInterfaces[ i ]->Shutdown();
        hsRefCnt_SafeUnRef( fInterfaces[ i ] );
    }
    fInterfaces.Reset();

    for( i = 0; i < fMessageQueue.GetCount(); i++ )
        delete fMessageQueue[ i ];
    fMessageQueue.Reset();

    plgDispatch::Dispatch()->UnRegisterForType( plInputIfaceMgrMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->UnRegisterForType( plInputEventMsg::Index(), GetKey() );

    plgDispatch::Dispatch()->UnRegisterForType( plEvalMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->UnRegisterForExactType( plPlayerPageMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->UnRegisterForExactType( plCmdIfaceModMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->UnRegisterForExactType( plClientMsg::Index(), GetKey() );
    
    UnRegisterAs( kInputInterfaceMgr_KEY );
}

//// IAdd/RemoveInterface ////////////////////////////////////////////////////
//  Each interface has a "priority level", i.e. where in the list it should be.
//  Doing it this way allows us to keep the manager unaware of the number or 
//  types of interfaces.

void    plInputInterfaceMgr::IAddInterface( plInputInterface *iface )
{
    int     i;


    for( i = 0; i < fInterfaces.GetCount(); i++ )
    {
        if( fInterfaces[ i ]->GetPriorityLevel() < iface->GetPriorityLevel() )
            break;
    }

    fInterfaces.Insert( i, iface );
    hsRefCnt_SafeRef( iface );
    iface->Init( this );
    iface->ISetMessageQueue( &fMessageQueue );
}

void    plInputInterfaceMgr::IRemoveInterface( plInputInterface *iface )
{
    int     idx = fInterfaces.Find( iface );
    if( idx != fInterfaces.kMissingIndex )
    {
        fInterfaces[ idx ]->Shutdown();
        hsRefCnt_SafeUnRef( fInterfaces[ idx ] );
        fInterfaces.Remove( idx );
    }
}

/// reset clickable state //////////////////////////////////////////////////////////////////////////

void plInputInterfaceMgr::ResetClickableState()
{
    // look for the scene input interface
    for(int i = 0; i < fInterfaces.GetCount(); i++ )
        fInterfaces[i]->ResetClickableState();
}

//// IUpdateCursor ///////////////////////////////////////////////////////////

void    plInputInterfaceMgr::IUpdateCursor( int32_t newCursor )
{
    const char*     mouseCursorResID;


    fCurrentCursor = newCursor;
    if( fCurrentCursor == plInputInterface::kCursorHidden )
        plMouseDevice::HideCursor();
    else
    {
        plMouseDevice::ShowCursor();
            
        switch( fCurrentCursor )
        {
            case plInputInterface::kCursorUp:                   mouseCursorResID = CURSOR_UP;                   break;
            case plInputInterface::kCursorLeft:                 mouseCursorResID = CURSOR_LEFT;                 break;
            case plInputInterface::kCursorRight:                mouseCursorResID = CURSOR_RIGHT;                break;
            case plInputInterface::kCursorDown:                 mouseCursorResID = CURSOR_DOWN;                 break;
            case plInputInterface::kCursorPoised:               mouseCursorResID = CURSOR_POISED;               break;
            case plInputInterface::kCursorClicked:              mouseCursorResID = CURSOR_CLICKED;              break;
            case plInputInterface::kCursorUnClicked:            mouseCursorResID = CURSOR_POISED;               break;
            case plInputInterface::kCursorOpen:                 mouseCursorResID = CURSOR_OPEN;                 break;
            case plInputInterface::kCursorGrab:                 mouseCursorResID = CURSOR_GRAB;                 break;
            case plInputInterface::kCursorArrow:                mouseCursorResID = CURSOR_ARROW;                break;
            case plInputInterface::kCursor4WayDraggable:        mouseCursorResID = CURSOR_4WAY_OPEN;            break;
            case plInputInterface::kCursor4WayDragging:         mouseCursorResID = CURSOR_4WAY_CLOSED;          break;
            case plInputInterface::kCursorUpDownDraggable:      mouseCursorResID = CURSOR_UPDOWN_OPEN;          break;
            case plInputInterface::kCursorUpDownDragging:       mouseCursorResID = CURSOR_UPDOWN_CLOSED;        break;
            case plInputInterface::kCursorLeftRightDraggable:   mouseCursorResID = CURSOR_LEFTRIGHT_OPEN;       break;
            case plInputInterface::kCursorLeftRightDragging:    mouseCursorResID = CURSOR_LEFTRIGHT_CLOSED;     break;
            case plInputInterface::kCursorOfferBook:            mouseCursorResID = CURSOR_OFFER_BOOK;           break;
            case plInputInterface::kCursorOfferBookHilite:      mouseCursorResID = CURSOR_OFFER_BOOK_HI;        break;
            case plInputInterface::kCursorOfferBookClicked:     mouseCursorResID = CURSOR_OFFER_BOOK_CLICKED;   break;
            case plInputInterface::kCursorClickDisabled:        mouseCursorResID = CURSOR_CLICK_DISABLED;       break;
            case plInputInterface::kCursorHand:                 mouseCursorResID = CURSOR_HAND;                 break;
            case plInputInterface::kCursorUpward:               mouseCursorResID = CURSOR_UPWARD;               break;
            default:                                            mouseCursorResID = CURSOR_OPEN;                 break;
                
        }

        
        plMouseDevice::NewCursor( mouseCursorResID );
    }
}

//// IEval ///////////////////////////////////////////////////////////////////
//  Inherited from plSingleModifier, gets called once per IUpdate() loop. 

bool plInputInterfaceMgr::IEval( double secs, float del, uint32_t dirty )
{
    const char *inputEval = "Eval";
    plProfile_BeginLap(Input, inputEval);
    int     i;


    // Let all our layers eval
    for( i = 0; i < fInterfaces.GetCount(); i++ )
        fInterfaces[ i ]->IEval( secs, del, dirty );

    // Handle our message queue now
    for( i = 0; i < fMessageQueue.Count(); i++ )
    {
        // Can its layer handle it?
        if( !fMessageQueue[ i ]->GetSource()->IHandleCtrlCmd( fMessageQueue[ i ] ) )
        {
            // Nope, just dispatch it like normal
            plControlEventMsg* pMsg = new plControlEventMsg;
            for (int j = 0; j < fReceivers.Count(); j++)
                pMsg->AddReceiver( fReceivers[ j ] );
            pMsg->SetControlActivated( fMessageQueue[i]->fControlActivated );
            pMsg->SetControlCode( fMessageQueue[i]->fControlCode );
            pMsg->SetControlPct(fMessageQueue[i]->fPct);
            pMsg->SetTurnToPt( fMessageQueue[i]->fPt );
            pMsg->SetCmdString(fMessageQueue[i]->GetCmdString());       
            pMsg->SetSender( GetKey() );    
            plgDispatch::MsgSend( pMsg );

            ///////////////////////////////////////////////////////
            //      send same msg over network to players
            ///////////////////////////////////////////////////////

            if (fMessageQueue[i]->fNetPropagateToPlayers)
            {
                pMsg = new plControlEventMsg;
                for (int j = 0; j < fReceivers.Count(); j++)
                    if (fReceivers[j] == plNetClientApp::GetInstance()->GetLocalPlayerKey())
                        pMsg->AddReceiver( fReceivers[j] );
                if (pMsg->GetNumReceivers())
                {
                    pMsg->SetControlActivated( fMessageQueue[i]->fControlActivated );
                    pMsg->SetControlCode( fMessageQueue[i]->fControlCode );
                    pMsg->SetControlPct(fMessageQueue[i]->fPct);
                    pMsg->SetTurnToPt( fMessageQueue[i]->fPt );
                    pMsg->SetCmdString(fMessageQueue[i]->GetCmdString());
                    pMsg->SetSender( GetKey() );
                    pMsg->SetBCastFlag(plMessage::kNetPropagate | plMessage::kPropagateToModifiers | 
                        plMessage::kNetUseRelevanceRegions);    // bcast only to other players who care about the region I'm in
                    pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);

                    plgDispatch::MsgSend( pMsg );
                }
                else
                    delete pMsg;
            }
        }
    }

    // Clear the message queue
    for( i = 0; i < fMessageQueue.Count(); i++ )
        delete fMessageQueue[ i ];
    fMessageQueue.SetCount( 0 );

    plProfile_EndLap(Input, inputEval);
    return true;
}

void plInputInterfaceMgr::ForceCursorHidden( bool requestedState )
{
    if ( requestedState )
    {
        fForceCursorHiddenCount++;
        fForceCursorHidden = requestedState;
    }
    else
    {
        fForceCursorHiddenCount--;

// this happens way too often to leave in
//      hsAssert(fForceCursorHiddenCount>=0,"ForceCursorHidded: unhiding more times than hidden" );

#define OnlyHideCursorOnLast
#ifdef OnlyHideCursorOnLast
        // is this is the last person... then really unforce hidding the mouse cursor
        if ( fForceCursorHiddenCount <= 0 )
        {
#endif //OnlyHideCursorOnLast

            fForceCursorHidden = requestedState;
            fForceCursorHiddenCount = 0;

#ifdef OnlyHideCursorOnLast
        }
#endif //OnlyHideCursorOnLast
    }

}

bool plInputInterfaceMgr::ICheckCursor(plInputInterface *iFace)
{
    if( iFace->IsEnabled() && iFace->HasInterestingCursorID() )
    {
        if( iFace->GetCurrentCursorID() != fCurrentCursor )
            IUpdateCursor( iFace->GetCurrentCursorID() );
        if( iFace->GetCurrentCursorOpacity() != fCursorOpacity )
        {
            fCursorOpacity = iFace->GetCurrentCursorOpacity();
            plMouseDevice::SetCursorOpacity( fCursorOpacity );
        }
        return true;
    }
    return false;
}           

//// MsgReceive //////////////////////////////////////////////////////////////

bool    plInputInterfaceMgr::MsgReceive( plMessage *msg )
{
    int     i;


    plEvalMsg *pEvalMsg = plEvalMsg::ConvertNoRef( msg );
    if( pEvalMsg )
    {
        IEval( pEvalMsg->GetTimeStamp(), pEvalMsg->DelSeconds(), false );
        return true;
    }

    plInputEventMsg *ieMsg = plInputEventMsg::ConvertNoRef( msg );
    if( ieMsg != nil )
    {
        const char *inputIEM = "InputEventMsg";
        plProfile_BeginLap(Input, inputIEM);
        bool handled = false;
        uint32_t missedInputStartIdx = 0;
        plInputInterface *oldCurrentFocus = fCurrentFocus;

        // Current focus (if there is one) gets first crack
        if( fCurrentFocus )
        {
            if( fCurrentFocus->IsEnabled() )
            {
                handled = (fCurrentFocus->ProcessKeyBindings(ieMsg) || fCurrentFocus->InterpretInputEvent(ieMsg));
            }
        }

        if (!handled)
        {
            // Walk our stack
            for( i = 0; i < fInterfaces.GetCount(); i++ )
            {
                if( fInterfaces[ i ]->IsEnabled() && fInterfaces[ i ] != oldCurrentFocus)
                {
                    // Try the key bindings first (common for all layers)
                    if( fInterfaces[ i ]->ProcessKeyBindings( ieMsg ) || fInterfaces[ i ]->InterpretInputEvent( ieMsg ))
                    {
                        handled = true;
                        break;
                    }
                }
            }

            if( !handled )
            {
                // Fell all the way through the stack...must've been a very uninteresting message...
                if( plKeyEventMsg::ConvertNoRef( ieMsg ) && fDefaultCatcher != nil )
                {
                    // But somebody loves those keys :)
                    fDefaultCatcher->HandleKeyEvent( plKeyEventMsg::ConvertNoRef( ieMsg ) );
                }
            }
            missedInputStartIdx = i + 1;
        }

        // Notify the rest of the interfaces in the stack that they missed the event ("lost focus", as it were)
        for (i = missedInputStartIdx; i < fInterfaces.GetCount(); i++)
            if (fInterfaces[i] != oldCurrentFocus)
                fInterfaces[i]->MissedInputEvent(ieMsg);

        // Now we re-walk to see who's the new interested party. Note that we have to re-walk
        // because a key down may have changed some layer's interest in the cursor
        if( !fForceCursorHidden )
        {
            bool cursorHandled = false;
            if (fCurrentFocus)
                cursorHandled = ICheckCursor(fCurrentFocus);

            if (!cursorHandled)
            {
                for( i = 0; i < fInterfaces.GetCount(); i++ )
                {
                    if (ICheckCursor(fInterfaces[i]))
                    {
                        cursorHandled = true;
                        break;
                    }
                }
                if (!cursorHandled)
                {
                    // NOBODY is interested in the mouse, so set to our default cursor
                    IUpdateCursor( plInputInterface::kCursorUp );
                    fCursorOpacity = 1.f;
                    plMouseDevice::SetCursorOpacity( fCursorOpacity );
                }
            }
        }
        else
        {
            // Special debug flag to force the cursor to be hidden
            if( fCursorOpacity != 0.f )
            {
                fCursorOpacity = 0.f;
                plMouseDevice::SetCursorOpacity( fCursorOpacity );
            }
        }
        plProfile_EndLap(Input, inputIEM);      
        return true;
    }

    plInputIfaceMgrMsg *mgrMsg = plInputIfaceMgrMsg::ConvertNoRef( msg );
    if( mgrMsg != nil )
    {
        if( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kAddInterface )
        {
            IAddInterface( mgrMsg->GetIFace() );
            return true;
        }
        else if( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kRemoveInterface )
        {
            IRemoveInterface( mgrMsg->GetIFace() );
            return true;
        }
        else if( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kEnableClickables )
        {
            fClickEnabled = true;
        }
        else if( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kDisableClickables )
        {
            fClickEnabled = false;
        }
    }

    plPlayerPageMsg *pPMsg = plPlayerPageMsg::ConvertNoRef( msg );
    if( pPMsg != nil && !pPMsg->fUnload)
    {
        if( pPMsg->fPlayer == plNetClientMgr::GetInstance()->GetLocalPlayerKey() )
            fReceivers.Append( pPMsg->fPlayer );
        else
        {
            int idx = fReceivers.Find( pPMsg->fPlayer );
            if( idx != fReceivers.kMissingIndex )
                fReceivers.Remove( idx );
        }
    }
    
    plCmdIfaceModMsg *pCMsg = plCmdIfaceModMsg::ConvertNoRef( msg );
    if( pCMsg )
    {
        if( pCMsg->Cmd( plCmdIfaceModMsg::kAdd ) )
        {   
            for( int i = 0; i < fReceivers.Count(); i++ )
            {
                if( fReceivers[i] == pCMsg->GetSender() )
                    return true;
            }
            fReceivers.Append( pCMsg->GetSender() );
            return true;
        }
        else
        if( pCMsg->Cmd( plCmdIfaceModMsg::kRemove ) )
        {
            for( int i = 0; i < fReceivers.Count(); i++ )
            {
                if( fReceivers[ i ] == pCMsg->GetSender() )
                {
                    fReceivers.Remove( i );
                    break;
                }
            }
            return true;
        }
    }

    plClientMsg *cMsg = plClientMsg::ConvertNoRef(msg);
    if (cMsg && cMsg->GetClientMsgFlag() == plClientMsg::kInitComplete)
    {
        // Backwards compatability hack:
        // We've loaded in the user prefs for input. If they bind movement 
        // to an arrow, or numpad, and the other binding is free, automatically 
        // bind the other one.
        plKeyMap *map = plAvatarInputInterface::GetInstance()->fControlMap;
        map->HandleAutoDualBinding(KEY_UP, KEY_NUMPAD8);
        map->HandleAutoDualBinding(KEY_DOWN, KEY_NUMPAD2);
        map->HandleAutoDualBinding(KEY_LEFT, KEY_NUMPAD4);
        map->HandleAutoDualBinding(KEY_RIGHT, KEY_NUMPAD6);     

        plgDispatch::Dispatch()->UnRegisterForExactType( plClientMsg::Index(), GetKey() );      
        return true;
    }
    // Wasn't one we want. Was it one that one of our interfaces wanted?
    for( i = 0; i < fInterfaces.GetCount(); i++ )
    {
        if( fInterfaces[ i ]->MsgReceive( msg ) )
            return true;
    }

    // Nothing, pass on...
    return plSingleModifier::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    plInputInterfaceMgr::Read( hsStream* s, hsResMgr* mgr )
{
    plSingleModifier::Read( s, mgr );
}

void    plInputInterfaceMgr::Write( hsStream* s, hsResMgr* mgr )
{
    plSingleModifier::Write( s, mgr );
}


//////////////////////////////////////////////////////////////////////////////
//// Key Maps ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// IGetRoutedKeyMap ////////////////////////////////////////////////////////
        
plKeyMap    *plInputInterfaceMgr::IGetRoutedKeyMap( ControlEventCode code )
{
    int                 i;


    for( i = 0; i < fInterfaces.GetCount(); i++ )
    {
        if( fInterfaces[ i ]->IOwnsControlCode( code ) )
            return fInterfaces[ i ]->fControlMap;
    }

    return nil;
}

//// IUnbind /////////////////////////////////////////////////////////////////
//  Unmaps any mappings with the given key. This prevents you from mapping
//  a single key to multiple commands. Currently inactive because some people
//  think it's a good idea to be able to do that.

#define ALLOW_MULTIPLE_CMDS_PER_KEY 1

void    plInputInterfaceMgr::IUnbind( const plKeyCombo &key )
{
#if !(ALLOW_MULTIPLE_CMDS_PER_KEY)
    int                 i;


    for( i = 0; i < fInterfaces.GetCount(); i++ )
        fInterfaces[ i ]->fControlMap->UnmapKey( key );
#endif
}

void plInputInterfaceMgr::ClearAllKeyMaps()
{
    for (int i = 0; i < fInterfaces.Count(); i++)
        fInterfaces[i]->ClearKeyMap();
}

//// Binding Routers /////////////////////////////////////////////////////////

void    plInputInterfaceMgr::BindAction( const plKeyCombo &key, ControlEventCode code )
{
    plKeyMap *map = IGetRoutedKeyMap( code );
    if( map != nil )
    {
        // Use default prefs
        map->EnsureKeysClear( key, plKeyCombo::kUnmapped );
        map->BindKey( key, code );
    }
    RefreshInterfaceKeyMaps();
}

void    plInputInterfaceMgr::BindAction( const plKeyCombo &key1, const plKeyCombo &key2,
                                        ControlEventCode code )
{
    plKeyMap *map = IGetRoutedKeyMap( code );
    if( map != nil )
    {
        // Force the bindings to each key, since the user specified both
        map->EnsureKeysClear( key1, key2 );
        map->BindKey( key1, code, plKeyMap::kFirstAlways );
        map->BindKey( key2, code, plKeyMap::kSecondAlways );
    }
    RefreshInterfaceKeyMaps();
}

const plKeyBinding* plInputInterfaceMgr::FindBinding( ControlEventCode code )\
{
    plKeyMap *map = IGetRoutedKeyMap( code );
    if( map != nil )
    {
        // Use default prefs
        return map->FindBinding(code);
    }

    return nil;
}

void plInputInterfaceMgr::BindConsoleCmd( const plKeyCombo &key, const char *cmd, plKeyMap::BindPref pref /*= kNoPreference*/  )
{
// not sure why this is not for external...since its done thru the different interfaces?
//#ifdef PLASMA_EXTERNAL_RELEASE
//  return;
//#endif

    plKeyMap *map = IGetRoutedKeyMap( B_CONTROL_CONSOLE_COMMAND );
    if( map != nil )
    {
        // Default prefs again
        map->EnsureKeysClear( key, plKeyCombo::kUnmapped );
        // BindKeyToConsoleCmd only works if the console command in question has already been assigned
        // to this map. Oftentimes, this isn't true when the user is binding console commands, so go ahead
        // and add the command to this map. If it's already added, this call will just quietly fail and
        // we're ok to continue
        map->AddConsoleCommand( cmd );
        map->BindKeyToConsoleCmd( key, cmd, pref );
    }
    RefreshInterfaceKeyMaps();
}

const plKeyBinding* plInputInterfaceMgr::FindBindingByConsoleCmd( const char *cmd )
{
    plKeyMap *map = IGetRoutedKeyMap( B_CONTROL_CONSOLE_COMMAND );
    if( map != nil )
    {
        return map->FindConsoleBinding(cmd);
    }
    return nil;
}

//// InitDefaultKeyMap ///////////////////////////////////////////////////////

void    plInputInterfaceMgr::InitDefaultKeyMap()
{
    int     i;

    for( i = 0; i < fInterfaces.GetCount(); i++ )
        fInterfaces[ i ]->RestoreDefaultKeyMappings();

    RefreshInterfaceKeyMaps();
}

//// RefreshInterfaceKeyMaps /////////////////////////////////////////////////

void    plInputInterfaceMgr::RefreshInterfaceKeyMaps()
{
    int     i;


    for( i = 0; i < fInterfaces.GetCount(); i++ )
        fInterfaces[ i ]->RefreshKeyMap();
}

//// WriteKeyMap /////////////////////////////////////////////////////////////

void    plInputInterfaceMgr::WriteKeyMap()
{
#ifdef PLASMA_EXTERNAL_RELEASE
    return;
#endif
    plFileName fname = plFileName::Join("init", "keyboard.fni");
    FILE* gKeyFile = plFileSystem::Open(fname, "wt");

    if (gKeyFile)
    {
        fprintf(gKeyFile, "# To remap a control to a new key,\n");
        fprintf(gKeyFile, "# just change the key listed. \n");
        fprintf(gKeyFile, "# The list of available commands is at the bottom.\n");
        fprintf(gKeyFile, "# For keys with multi-character names \n");
        fprintf(gKeyFile, "# like F1 or Backspace, be sure to enter \n");
        fprintf(gKeyFile, "# the key name as it appears in the list \n");
        fprintf(gKeyFile, "# at the end of the file. \n");
        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Be sure to put quotes around the actual command\n");
        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# To add modifiers to a key mapping (like ctrl or shift)\n");
        fprintf(gKeyFile, "# append _C for ctrl or _S for Shift (or both)\n");
        fprintf(gKeyFile, "# to the actual name of the key.\n");
        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# For example, to map Control-Shift-W to Walk Forward\n");
        fprintf(gKeyFile, "# your key mapping entry would look like this:\n");
        fprintf(gKeyFile, "# Keyboard.BindAction \tW_C_S \t\"Walk Forward\"\n");
        fprintf(gKeyFile, "# This also works for console command bindings\n");

        
        fprintf(gKeyFile, "# Keyboard.BindAction \t\tKey1\tKey2\t\t\t\tControl\n");
        fprintf(gKeyFile, "#\n");
//      fprintf(gKeyFile, "Keyboard.ClearBindings\n");
        int i;
        
        for( i = 0; i < fInterfaces.GetCount(); i++ )
            IWriteNonConsoleCmdKeys( fInterfaces[ i ]->fControlMap, gKeyFile );

        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Console command bindings:\n");
        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Keyboard.BindConsoleCmd \tKey\t\t\tCommand\n");
        fprintf(gKeyFile, "#\n");
        
        for( i = 0; i < fInterfaces.GetCount(); i++ )
            IWriteConsoleCmdKeys( fInterfaces[ i ]->fControlMap, gKeyFile );

        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Available game commands:\n");
        fprintf(gKeyFile, "#\n");
        
        for( int j = 0; plKeyMap::fCmdConvert[ j ].fCode != END_CONTROLS; j++ )
        {
            if( stricmp( plKeyMap::fCmdConvert[ j ].fDesc, "Run Modifier" ) == 0)
                continue;
            fprintf( gKeyFile, "#  %s\n", plKeyMap::fCmdConvert[ j ].fDesc );
        }

        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Key name list (for a-z or 0-9 just use the character)\n");
        fprintf(gKeyFile, "#\n");

        Win32keyConvert* keyConvert = &plKeyMap::fKeyConversionEnglish[0];
        switch (plLocalization::GetLanguage())
        {
            case plLocalization::kFrench:
                keyConvert = &plKeyMap::fKeyConversionFrench[0];
                break;
            case plLocalization::kGerman:
                keyConvert = &plKeyMap::fKeyConversionGerman[0];
                break;
            //case plLocalization::kSpanish:
            //  keyConvert = &plKeyMap::fKeyConversionSpanish[0];
            //  break;
            //case plLocalization::kItalian:
            //  keyConvert = &plKeyMap::fKeyConversionItalian[0];
            //  break;
            // default is English
        }
        for (i = 0; keyConvert[i].fVKey != 0xffffffff; i++)
        {   
//              if (stricmp(fKeyMap->fKeyConversion[i].fKeyName, "Shift") == 0)
//                  continue;
            fprintf(gKeyFile, "#  %s\n", keyConvert[i].fKeyName);
        }
        fclose(gKeyFile);
    }
}

void plInputInterfaceMgr::SetCurrentFocus(plInputInterface *focus)
{
    fCurrentFocus = focus;
}

void plInputInterfaceMgr::ReleaseCurrentFocus(plInputInterface *focus)
{
    if (fCurrentFocus == focus)
        fCurrentFocus = nil;
}


//// IKeyComboToString ///////////////////////////////////////////////////////
//  Uses static string, so don't call twice and expect the first result to
//  be still valid!

const char  *plInputInterfaceMgr::IKeyComboToString( const plKeyCombo &combo )
{
    static char     str[ 64 ];
    bool            unmapped = false;


    if( combo == plKeyCombo::kUnmapped )
        sprintf( str, "(unmapped)" );
    else
    {
        const char *c = plKeyMap::ConvertVKeyToChar( combo.fKey );
        if( c != nil )
            strncpy( str, c, sizeof( str ) );
        else
        {
            if( isalnum( combo.fKey ) )
            {
                str[ 0 ] = (char)combo.fKey;
                str[ 1 ] = 0;
            }
            else
            {
                strcpy( str, "(unmapped)" );
                unmapped = true;
            }
        }
        if( !unmapped )
        {
            if( combo.fFlags & plKeyCombo::kCtrl )
                strcat( str, "_C" );
            if( combo.fFlags & plKeyCombo::kShift )
                strcat( str, "_S" );
        }
    }

    return str;
}

//// IWriteNonConsoleCmdKeys /////////////////////////////////////////////////

void    plInputInterfaceMgr::IWriteNonConsoleCmdKeys( plKeyMap *keyMap, FILE *keyFile )
{
    int i;


    for( i = 0; i < keyMap->GetNumBindings(); i++ )
    {
        const plKeyBinding &binding = keyMap->GetBinding( i );

        if( binding.GetCode() == B_CONTROL_CONSOLE_COMMAND )
            continue;

        char    key1[ 64 ];
        strcpy( key1, IKeyComboToString( binding.GetKey1() ) );

        const char *key2 = IKeyComboToString( binding.GetKey2() );

        const char *desc = plInputMap::ConvertControlCodeToString( binding.GetCode() );

        fprintf( keyFile, "Keyboard.BindAction \t\t%s\t%s\t\t\t\t\"%s\"\n", key1, key2, desc );
    }

}

//// IWriteConsoleCmdKeys ////////////////////////////////////////////////////

void    plInputInterfaceMgr::IWriteConsoleCmdKeys( plKeyMap *keyMap, FILE *keyFile )
{
    int     i;


    for( i = 0; i < keyMap->GetNumBindings(); i++ )
    {
        const plKeyBinding &binding = keyMap->GetBinding( i );

        if( binding.GetCode() != B_CONTROL_CONSOLE_COMMAND )
            continue;

        // Our bindConsoleCmd console command (echo echo) only takes 1 key combo, not 2,
        // so as not to confuse people. Or something. So if we got two bindings, we print
        // 2 commands, which is perfectly valid
//      if( binding.GetKey1() != plKeyCombo::kUnmapped )
//      {
            const char *key = IKeyComboToString( binding.GetKey1() );
            fprintf( keyFile, "Keyboard.BindConsoleCmd\t%s\t\t\t\"%s\"\n", key, binding.GetExtendedString() );
//      }

        if( binding.GetKey2() != plKeyCombo::kUnmapped )
        {
            const char *key = IKeyComboToString( binding.GetKey2() );
            fprintf( keyFile, "Keyboard.BindConsoleCmd\t%s\t\t\t\"%s\"\n", key, binding.GetExtendedString() );
        }
    }
}
