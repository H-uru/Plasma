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
//  pfGUIButtonMod Definition                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIButtonMod.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pfGameGUIMgr.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogMod.h"
#include "pfGUIDraggableMod.h"

#include "pnMessage/plRefMsg.h"

#include "plInputCore/plInputInterface.h"
#include "plMessage/plAnimCmdMsg.h"

//// Control Proc For Managing the Draggable /////////////////////////////////

class pfGUIButtonDragProc : public pfGUICtrlProcObject
{
    protected:

        pfGUICtrlProcObject *fOrigProc;

        pfGUIButtonMod      *fParent;
        pfGUIDraggableMod   *fDraggable;
        bool                fReportDrag;

    public:

        pfGUIButtonDragProc( pfGUIButtonMod *parent, pfGUIDraggableMod *draggable, pfGUICtrlProcObject *origProc, bool reportDrag )
        {
            fParent = parent;
            fDraggable = draggable;
            fOrigProc = origProc;
            fReportDrag = reportDrag;
        }

        void    DoSomething(pfGUIControlMod *ctrl) override
        {
            // The draggable was let up, so now we stop dragging, disable the draggable again, and pass
            // on the event to our original proc
            if (fOrigProc != nullptr && fParent->IsTriggering())
                fOrigProc->DoSomething( ctrl );
            if (!fParent->IsButtonDown())
                fParent->StopDragging( false );
        }

        void    HandleExtendedEvent(pfGUIControlMod *ctrl, uint32_t event) override
        {
            if( event == pfGUIDraggableMod::kDragging )
            {
                // First test if we're inside our button (if so, we stop dragging)
                if( fParent->PointInBounds( fDraggable->GetLastMousePt() ) )
                {
                    // Cancel the drag
                    fParent->StopDragging( true );
                    return;
                }

                if( !fReportDrag )
                    return;
            }
            
            if (fOrigProc != nullptr)
                fOrigProc->HandleExtendedEvent( ctrl, event );
        }

        void    UserCallback(uint32_t userValue) override
        {
            if (fOrigProc != nullptr)
                fOrigProc->UserCallback( userValue );
        }
};


void    pfGUIButtonMod::StopDragging( bool cancel )
{
    fDraggable->StopDragging( cancel );
    fDraggable->SetVisible( false );
    fDraggable->SetHandler( fOrigHandler );
    fOrigHandler = nullptr;

    if( !fOrigReportedDrag )
        fDraggable->ClearFlag( pfGUIDraggableMod::kReportDragging );

    // Steal interest back
    fDialog->SetControlOfInterest( this );
}

void    pfGUIButtonMod::StartDragging()
{
    fOrigReportedDrag = fDraggable->HasFlag( pfGUIDraggableMod::kReportDragging );
    fDraggable->SetFlag( pfGUIDraggableMod::kReportDragging );

    fOrigHandler = fDraggable->GetHandler();
    fDraggable->SetVisible( true );
    fDraggable->SetHandler( new pfGUIButtonDragProc( this, fDraggable, fOrigHandler, fOrigReportedDrag ) );
    fDraggable->HandleMouseDown( fOrigMouseDownPt, 0 );
}

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIButtonMod::pfGUIButtonMod()
    : fDraggable(), fOrigHandler(), fClicking(), fTriggering(),
      fNotifyType(kNotifyOnUp), fOrigReportedDrag()
{
    SetFlag(kWantsInterest);
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIButtonMod::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIButtonMod::MsgReceive( plMessage *msg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if (refMsg != nullptr && refMsg->fType == kRefDraggable)
    {
        if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
        {
            fDraggable = pfGUIDraggableMod::ConvertNoRef( refMsg->GetRef() );
            fDraggable->SetVisible( false );        // Disable until we're dragging
        }
        else
            fDraggable = nullptr;
        return true;
    }

    return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIButtonMod::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);

    fAnimationKeys.clear();
    uint32_t count = s->ReadLE32();
    fAnimationKeys.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        fAnimationKeys.emplace_back(mgr->ReadKey(s));
    fAnimName = s->ReadSafeString();

    fMouseOverAnimKeys.clear();
    count = s->ReadLE32();
    fMouseOverAnimKeys.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        fMouseOverAnimKeys.emplace_back(mgr->ReadKey(s));
    fMouseOverAnimName = s->ReadSafeString();

    fNotifyType = s->ReadLE32();
    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDraggable ), plRefFlags::kActiveRef );
}

void    pfGUIButtonMod::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Write( s, mgr );

    s->WriteLE32((uint32_t)fAnimationKeys.size());
    for (const plKey& key : fAnimationKeys)
        mgr->WriteKey(s, key);
    s->WriteSafeString( fAnimName );

    s->WriteLE32((uint32_t)fMouseOverAnimKeys.size());
    for (const plKey& key : fMouseOverAnimKeys)
        mgr->WriteKey(s, key);
    s->WriteSafeString( fMouseOverAnimName );

    s->WriteLE32( fNotifyType );

    mgr->WriteKey(s, fDraggable != nullptr ? fDraggable->GetKey() : nullptr);

}

//// UpdateBounds ////////////////////////////////////////////////////////////

void    pfGUIButtonMod::UpdateBounds( hsMatrix44 *invXformMatrix, bool force )
{
    pfGUIControlMod::UpdateBounds( invXformMatrix, force );
    if (!fAnimationKeys.empty() || !fMouseOverAnimKeys.empty())
        fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void    pfGUIButtonMod::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    fClicking = true;
    if (!fAnimationKeys.empty())
    {
        plAnimCmdMsg *msg = new plAnimCmdMsg();
        msg->SetCmd( plAnimCmdMsg::kContinue );
        msg->SetCmd( plAnimCmdMsg::kSetForewards );
        msg->SetCmd( plAnimCmdMsg::kGoToBegin );    
        msg->SetAnimName( fAnimName );
        msg->AddReceivers( fAnimationKeys );
        msg->Send();
    }

    IPlaySound( kMouseDown );

    fOrigMouseDownPt = mousePt;
    if ( fNotifyType == kNotifyOnDown || fNotifyType == kNotifyOnUpAndDown)
    {
        fTriggering = true;
        DoSomething();
        fTriggering = false;
    }
}

void    pfGUIButtonMod::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{

    // make sure that we got the down click first
    if ( !fClicking )
        return;

    fClicking = false;
    if (!fAnimationKeys.empty())
    {
        plAnimCmdMsg *msg = new plAnimCmdMsg();
        msg->SetCmd( plAnimCmdMsg::kContinue );
        msg->SetCmd( plAnimCmdMsg::kSetBackwards );
        msg->SetCmd( plAnimCmdMsg::kGoToEnd );  
        msg->SetAnimName( fAnimName );
        msg->AddReceivers( fAnimationKeys );
        msg->Send();
    }

    IPlaySound( kMouseUp );

    // Don't run the command if the mouse is outside our bounds
    if( !fBounds.IsInside( &mousePt ) && fNotifyType != kNotifyOnUpAndDown )
        return;     

    if ( fNotifyType == kNotifyOnUp || fNotifyType == kNotifyOnUpAndDown)
        fTriggering = true;
    DoSomething();
    fTriggering = false;
}

void    pfGUIButtonMod::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
    if( !fClicking )
        return;

    if (fDraggable == nullptr)
        return;

    if( !fDraggable->IsVisible() )
    {
        // Are we outside ourselves?
        if( !PointInBounds( mousePt ) )
        {
            // Yes, start dragging
            StartDragging();

            // Hand off our interest to the draggable
            fDialog->SetControlOfInterest( fDraggable );
        }
    }
}

void    pfGUIButtonMod::HandleMouseDblClick(hsPoint3& mousePt, uint8_t modifiers)
{
    HandleExtendedEvent(kDoubleClick);
}

void    pfGUIButtonMod::SetNotifyType(int32_t kind)
{
    fNotifyType = kind;
}

int32_t   pfGUIButtonMod::GetNotifyType()
{
    return fNotifyType;
}

bool    pfGUIButtonMod::IsButtonDown()
{
    return fClicking;
}

//// SetInteresting //////////////////////////////////////////////////////////
//  Overridden to play mouse over animation when we're interesting

void    pfGUIButtonMod::SetInteresting( bool i )
{
    pfGUIControlMod::SetInteresting( i );

    if (!fMouseOverAnimKeys.empty())
    {
        plAnimCmdMsg *msg = new plAnimCmdMsg();
        msg->SetCmd( plAnimCmdMsg::kContinue );
        msg->SetCmd( fInteresting ? plAnimCmdMsg::kSetForewards : plAnimCmdMsg::kSetBackwards );
        msg->SetAnimName( fMouseOverAnimName );
        msg->AddReceivers( fMouseOverAnimKeys );
        msg->Send();
    }

    if( i )
        IPlaySound( kMouseOver );
    else
        IPlaySound( kMouseOff );
}


void    pfGUIButtonMod::SetAnimationKeys(const std::vector<plKey> &keys, const ST::string &name)
{
    fAnimationKeys = keys;
    fAnimName = name;
}

void    pfGUIButtonMod::SetMouseOverAnimKeys(const std::vector<plKey> &keys, const ST::string &name)
{
    fMouseOverAnimKeys = keys;
    fMouseOverAnimName = name;
}


//// IGetDesiredCursor ///////////////////////////////////////////////////////

uint32_t      pfGUIButtonMod::IGetDesiredCursor() const
{
    if (fHandler == nullptr)
        return 0;

    if( fClicking )
        return plInputInterface::kCursorClicked;

    return plInputInterface::kCursorPoised;
}

