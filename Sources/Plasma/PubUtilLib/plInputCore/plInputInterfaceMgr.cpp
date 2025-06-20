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

#include "plAvatarInputInterface.h"
#include "plSceneInputInterface.h"
#include "plDebugInputInterface.h"
#include "plTelescopeInputInterface.h"

#include <string_theory/stdio>

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

#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plProfile.h"
#include "plResMgr/plLocalization.h"

plProfile_CreateTimer("Input", "Update", Input);

//// plCtrlCmd ///////////////////////////////////////////////////////////////

void plCtrlCmd::Write(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32((uint32_t)fControlCode);
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
    if (plInputInterfaceMgr::GetInstance() != nullptr)
        plInputInterfaceMgr::GetInstance()->SetDefaultKeyCatcher(nullptr);
}

//// Statics /////////////////////////////////////////////////////////////////

plInputInterfaceMgr *plInputInterfaceMgr::fInstance = nullptr;


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
    hsAssert(fInstance == nullptr, "Attempting to create two input interface managers!");
    fInstance = this;

    fCurrentFocus = nullptr;
    fDefaultCatcher = nullptr;
}

plInputInterfaceMgr::~plInputInterfaceMgr()
{
    Shutdown();
    fInstance = nullptr;
}

//// Init ////////////////////////////////////////////////////////////////////

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

//  WriteKeyMap();

    for (plInputInterface* iface : fInterfaces)
    {
        iface->Shutdown();
        hsRefCnt_SafeUnRef(iface);
    }
    fInterfaces.clear();

    for (plCtrlCmd* ctrlMsg : fMessageQueue)
        delete ctrlMsg;
    fMessageQueue.clear();

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
    auto iter = fInterfaces.cbegin();
    for (; iter != fInterfaces.cend(); ++iter)
    {
        if ((*iter)->GetPriorityLevel() < iface->GetPriorityLevel())
            break;
    }

    fInterfaces.insert(iter, iface);
    hsRefCnt_SafeRef( iface );
    iface->Init( this );
    iface->ISetMessageQueue( &fMessageQueue );
}

void    plInputInterfaceMgr::IRemoveInterface( plInputInterface *iface )
{
    auto iter = std::find(fInterfaces.begin(), fInterfaces.end(), iface);
    if (iter != fInterfaces.end())
    {
        (*iter)->Shutdown();

        // Drop all outstanding messages that originate from the interface being removed.
        // Otherwise, IEval will crash once it tries to process these messages.
        // This is relevant e. g. when pressing Esc/Backspace quickly multiple times
        // to exit a dialog (such as the new avatar intro):
        // if multiple B_CONTROL_EXIT_GUI_MODE messages are queued up,
        // one message may close the last dialog and thus make the pfGameUIInputInterface disappear,
        // and then the source pointer of the other messages will become dangling before they could be processed.
        for (auto ctrlMsgIt = fMessageQueue.begin(); ctrlMsgIt != fMessageQueue.end();) {
            if ((*ctrlMsgIt)->GetSource() == *iter) {
                delete *ctrlMsgIt;
                ctrlMsgIt = fMessageQueue.erase(ctrlMsgIt);
            } else {
                ++ctrlMsgIt;
            }
        }

        hsRefCnt_SafeUnRef(*iter);
        fInterfaces.erase(iter);
    }
}

/// reset clickable state //////////////////////////////////////////////////////////////////////////

void plInputInterfaceMgr::ResetClickableState()
{
    // look for the scene input interface
    for (plInputInterface* iface : fInterfaces)
        iface->ResetClickableState();
}

//// IUpdateCursor ///////////////////////////////////////////////////////////

void    plInputInterfaceMgr::IUpdateCursor( int32_t newCursor )
{
    if (newCursor == plInputInterface::kCursorHidden) {
        plMouseDevice::HideCursor();
    } else {
        if (fCurrentCursor == plInputInterface::kCursorHidden)
            plMouseDevice::ShowCursor();

        ST::string mouseCursorResID;
        switch (newCursor) {
            case plInputInterface::kCursorUp:                 mouseCursorResID = ST_LITERAL(CURSOR_UP);                 break;
            case plInputInterface::kCursorLeft:               mouseCursorResID = ST_LITERAL(CURSOR_LEFT);               break;
            case plInputInterface::kCursorRight:              mouseCursorResID = ST_LITERAL(CURSOR_RIGHT);              break;
            case plInputInterface::kCursorDown:               mouseCursorResID = ST_LITERAL(CURSOR_DOWN);               break;
            case plInputInterface::kCursorPoised:             mouseCursorResID = ST_LITERAL(CURSOR_POISED);             break;
            case plInputInterface::kCursorClicked:            mouseCursorResID = ST_LITERAL(CURSOR_CLICKED);            break;
            case plInputInterface::kCursorUnClicked:          mouseCursorResID = ST_LITERAL(CURSOR_POISED);             break;
            case plInputInterface::kCursorOpen:               mouseCursorResID = ST_LITERAL(CURSOR_OPEN);               break;
            case plInputInterface::kCursorGrab:               mouseCursorResID = ST_LITERAL(CURSOR_GRAB);               break;
            case plInputInterface::kCursorArrow:              mouseCursorResID = ST_LITERAL(CURSOR_ARROW);              break;
            case plInputInterface::kCursor4WayDraggable:      mouseCursorResID = ST_LITERAL(CURSOR_4WAY_OPEN);          break;
            case plInputInterface::kCursor4WayDragging:       mouseCursorResID = ST_LITERAL(CURSOR_4WAY_CLOSED);        break;
            case plInputInterface::kCursorUpDownDraggable:    mouseCursorResID = ST_LITERAL(CURSOR_UPDOWN_OPEN);        break;
            case plInputInterface::kCursorUpDownDragging:     mouseCursorResID = ST_LITERAL(CURSOR_UPDOWN_CLOSED);      break;
            case plInputInterface::kCursorLeftRightDraggable: mouseCursorResID = ST_LITERAL(CURSOR_LEFTRIGHT_OPEN);     break;
            case plInputInterface::kCursorLeftRightDragging:  mouseCursorResID = ST_LITERAL(CURSOR_LEFTRIGHT_CLOSED);   break;
            case plInputInterface::kCursorOfferBook:          mouseCursorResID = ST_LITERAL(CURSOR_OFFER_BOOK);         break;
            case plInputInterface::kCursorOfferBookHilite:    mouseCursorResID = ST_LITERAL(CURSOR_OFFER_BOOK_HI);      break;
            case plInputInterface::kCursorOfferBookClicked:   mouseCursorResID = ST_LITERAL(CURSOR_OFFER_BOOK_CLICKED); break;
            case plInputInterface::kCursorClickDisabled:      mouseCursorResID = ST_LITERAL(CURSOR_CLICK_DISABLED);     break;
            case plInputInterface::kCursorHand:               mouseCursorResID = ST_LITERAL(CURSOR_HAND);               break;
            case plInputInterface::kCursorUpward:             mouseCursorResID = ST_LITERAL(CURSOR_UPWARD);             break;
            default:                                          mouseCursorResID = ST_LITERAL(CURSOR_OPEN);               break;
        }

        plMouseDevice::NewCursor(mouseCursorResID);
    }

    fCurrentCursor = newCursor;
}

//// IEval ///////////////////////////////////////////////////////////////////
//  Inherited from plSingleModifier, gets called once per IUpdate() loop. 

bool plInputInterfaceMgr::IEval( double secs, float del, uint32_t dirty )
{
    const ST::string inputEval = ST_LITERAL("Eval");
    plProfile_BeginLap(Input, inputEval);


    // Let all our layers eval
    for (plInputInterface* iface : fInterfaces)
        iface->IEval(secs, del, dirty);

    // Handle our message queue now
    // NOTE: Do not use a range-based for loop here, since the size of
    // fMessageQueue might change in the middle of the loop !!!
    for (size_t i = 0; i < fMessageQueue.size(); ++i)
    {
        plCtrlCmd* ctrlMsg = fMessageQueue[i];

        // Can its layer handle it?
        if (!ctrlMsg->GetSource()->IHandleCtrlCmd(ctrlMsg))
        {
            // Nope, just dispatch it like normal
            plControlEventMsg* pMsg = new plControlEventMsg;
            for (const auto& rcKey : fReceivers)
                pMsg->AddReceiver(rcKey);
            pMsg->SetControlActivated(ctrlMsg->fControlActivated);
            pMsg->SetControlCode(ctrlMsg->fControlCode);
            pMsg->SetControlPct(ctrlMsg->fPct);
            pMsg->SetTurnToPt(ctrlMsg->fPt);
            pMsg->SetCmdString(ctrlMsg->GetCmdString());
            pMsg->SetSender( GetKey() );    
            plgDispatch::MsgSend( pMsg );

            ///////////////////////////////////////////////////////
            //      send same msg over network to players
            ///////////////////////////////////////////////////////

            if (ctrlMsg->fNetPropagateToPlayers)
            {
                pMsg = new plControlEventMsg;
                for (const plKey& rcKey : fReceivers)
                    if (rcKey == plNetClientApp::GetInstance()->GetLocalPlayerKey())
                        pMsg->AddReceiver(rcKey);
                if (pMsg->GetNumReceivers())
                {
                    pMsg->SetControlActivated(ctrlMsg->fControlActivated);
                    pMsg->SetControlCode(ctrlMsg->fControlCode);
                    pMsg->SetControlPct(ctrlMsg->fPct);
                    pMsg->SetTurnToPt(ctrlMsg->fPt);
                    pMsg->SetCmdString(ctrlMsg->GetCmdString());
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
    for (plCtrlCmd* ctrlMsg : fMessageQueue)
        delete ctrlMsg;
    fMessageQueue.clear();

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
    plEvalMsg *pEvalMsg = plEvalMsg::ConvertNoRef( msg );
    if( pEvalMsg )
    {
        IEval( pEvalMsg->GetTimeStamp(), pEvalMsg->DelSeconds(), false );
        return true;
    }

    plInputEventMsg *ieMsg = plInputEventMsg::ConvertNoRef( msg );
    if (ieMsg != nullptr)
    {
        const ST::string inputIEM = ST_LITERAL("InputEventMsg");
        plProfile_BeginLap(Input, inputIEM);
        bool handled = false;
        size_t missedInputStartIdx = 0;
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
            size_t i;
            for (i = 0; i < fInterfaces.size(); i++)
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
                if (plKeyEventMsg::ConvertNoRef(ieMsg) && fDefaultCatcher != nullptr)
                {
                    // But somebody loves those keys :)
                    fDefaultCatcher->HandleKeyEvent( plKeyEventMsg::ConvertNoRef( ieMsg ) );
                }
            }
            missedInputStartIdx = i + 1;
        }

        // Notify the rest of the interfaces in the stack that they missed the event ("lost focus", as it were)
        for (size_t i = missedInputStartIdx; i < fInterfaces.size(); i++)
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
                for (plInputInterface* iface : fInterfaces)
                {
                    if (ICheckCursor(iface))
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
    if (mgrMsg != nullptr)
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
    if (pPMsg != nullptr && !pPMsg->fUnload)
    {
        if( pPMsg->fPlayer == plNetClientApp::GetInstance()->GetLocalPlayerKey() )
            fReceivers.emplace_back(pPMsg->fPlayer);
        else
        {
            auto idx = std::find(fReceivers.cbegin(), fReceivers.cend(), pPMsg->fPlayer);
            if (idx != fReceivers.cend())
                fReceivers.erase(idx);
        }
    }
    
    plCmdIfaceModMsg *pCMsg = plCmdIfaceModMsg::ConvertNoRef( msg );
    if( pCMsg )
    {
        if( pCMsg->Cmd( plCmdIfaceModMsg::kAdd ) )
        {   
            for (const plKey& rcKey : fReceivers)
            {
                if (rcKey == pCMsg->GetSender())
                    return true;
            }
            fReceivers.emplace_back(pCMsg->GetSender());
            return true;
        }
        else
        if( pCMsg->Cmd( plCmdIfaceModMsg::kRemove ) )
        {
            auto idx = std::find(fReceivers.cbegin(), fReceivers.cend(), pCMsg->GetSender());
            if (idx != fReceivers.cend())
                fReceivers.erase(idx);
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
    for (plInputInterface* iface : fInterfaces)
    {
        if (iface->MsgReceive(msg))
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
    for (plInputInterface* iface : fInterfaces)
    {
        if (iface->IOwnsControlCode(code))
            return iface->fControlMap;
    }

    return nullptr;
}

//// IUnbind /////////////////////////////////////////////////////////////////
//  Unmaps any mappings with the given key. This prevents you from mapping
//  a single key to multiple commands. Currently inactive because some people
//  think it's a good idea to be able to do that.

#define ALLOW_MULTIPLE_CMDS_PER_KEY 1

void    plInputInterfaceMgr::IUnbind( const plKeyCombo &key )
{
#if !(ALLOW_MULTIPLE_CMDS_PER_KEY)
    for (plInputInterface* iface : fInterfaces)
        iface->fControlMap->UnmapKey(key);
#endif
}

void plInputInterfaceMgr::ClearAllKeyMaps()
{
    for (plInputInterface* iface : fInterfaces)
        iface->ClearKeyMap();
}

//// Binding Routers /////////////////////////////////////////////////////////

void    plInputInterfaceMgr::BindAction( const plKeyCombo &key, ControlEventCode code )
{
    plKeyMap *map = IGetRoutedKeyMap( code );
    if (map != nullptr)
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
    if (map != nullptr)
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
    if (map != nullptr)
    {
        // Use default prefs
        return map->FindBinding(code);
    }

    return nullptr;
}

void plInputInterfaceMgr::BindConsoleCmd(const plKeyCombo &key, const ST::string& cmd, plKeyMap::BindPref pref /*= kNoPreference*/)
{
// not sure why this is not for external...since its done thru the different interfaces?
//#ifdef PLASMA_EXTERNAL_RELEASE
//  return;
//#endif

    plKeyMap *map = IGetRoutedKeyMap( B_CONTROL_CONSOLE_COMMAND );
    if (map != nullptr)
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

const plKeyBinding* plInputInterfaceMgr::FindBindingByConsoleCmd(const ST::string& cmd)
{
    plKeyMap *map = IGetRoutedKeyMap( B_CONTROL_CONSOLE_COMMAND );
    if (map != nullptr)
    {
        return map->FindConsoleBinding(cmd);
    }
    return nullptr;
}

//// InitDefaultKeyMap ///////////////////////////////////////////////////////

void    plInputInterfaceMgr::InitDefaultKeyMap()
{
    for (plInputInterface* iface : fInterfaces)
        iface->RestoreDefaultKeyMappings();

    RefreshInterfaceKeyMaps();
}

//// RefreshInterfaceKeyMaps /////////////////////////////////////////////////

void    plInputInterfaceMgr::RefreshInterfaceKeyMaps()
{
    for (plInputInterface* iface : fInterfaces)
        iface->RefreshKeyMap();
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
        
        for (plInputInterface* iface : fInterfaces)
            IWriteNonConsoleCmdKeys(iface->fControlMap, gKeyFile);

        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Console command bindings:\n");
        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Keyboard.BindConsoleCmd \tKey\t\t\tCommand\n");
        fprintf(gKeyFile, "#\n");
        
        for (plInputInterface* iface : fInterfaces)
            IWriteConsoleCmdKeys(iface->fControlMap, gKeyFile);

        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Available game commands:\n");
        fprintf(gKeyFile, "#\n");
        
        for (const auto& [code, desc] : plKeyMap::fCmdConvert)
        {
            ST::printf(gKeyFile, "#  {}\n", desc);
        }

        fprintf(gKeyFile, "#\n");
        fprintf(gKeyFile, "# Key name list (for a-z or 0-9 just use the character)\n");
        fprintf(gKeyFile, "#\n");

        const std::map<uint32_t, ST::string>& keyConvert = plKeyMap::GetKeyConversion();
        for (const auto& [vKey, keyName] : keyConvert)
        {   
            ST::printf(gKeyFile, "#  {}\n", keyName);
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
        fCurrentFocus = nullptr;
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

        ST::string key1 = plKeyMap::KeyComboToString(binding.GetKey1());
        ST::string key2 = plKeyMap::KeyComboToString(binding.GetKey2());
        ST::string desc = plInputMap::ConvertControlCodeToString(binding.GetCode());
        ST::printf(keyFile, "Keyboard.BindAction \t\t{}\t{}\t\t\t\t\"{}\"\n", key1, key2, desc);
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
            ST::string key = plKeyMap::KeyComboToString(binding.GetKey1());
            ST::printf(keyFile, "Keyboard.BindConsoleCmd\t{}\t\t\t\"{}\"\n", key, binding.GetExtendedString());
//      }

        if( binding.GetKey2() != plKeyCombo::kUnmapped )
        {
            ST::string key2 = plKeyMap::KeyComboToString(binding.GetKey2());
            ST::printf(keyFile, "Keyboard.BindConsoleCmd\t{}\t\t\t\"{}\"\n", key2, binding.GetExtendedString());
        }
    }
}
