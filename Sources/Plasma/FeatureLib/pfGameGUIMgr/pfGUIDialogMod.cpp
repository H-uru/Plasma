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
//  pfGUIDialogMod Definition                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include "pfGameGUIMgr.h"

#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"
#include "plViewTransform.h"

#include "pfGUIControlMod.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogMod.h"
#include "pfGUIDialogNotifyProc.h"
#include "pfGUIListElement.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plUoid.h"
#include "pnMessage/plRefMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plAnimCmdMsg.h"
#include "plPipeline/plDebugText.h"
#include "plScene/plPostEffectMod.h"
#include "plScene/plSceneNode.h"

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIDialogMod::pfGUIDialogMod()
    : fRenderMod(), fNext(), fPrevPtr(), fEnabled(),
      fControlOfInterest(), fFocusCtrl(), fMousedCtrl(),
      fTagID(), fHandler(), fVersion(), fProcReceiver(),
      fDragMode(), fDragReceptive(), fDragTarget(),
      fDragSource(), fColorScheme(new pfGUIColorScheme())
{ }

pfGUIDialogMod::~pfGUIDialogMod()
{
    // Call the handler's destroy if there is one
    if( fHandler )
        fHandler->OnDestroy();

    // Unregister us with the Game GUI manager
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        plGenRefMsg *refMsg = new plGenRefMsg( mgrKey, plRefMsg::kOnRemove, 0, pfGameGUIMgr::kDlgModRef );
        refMsg->SetRef( this );
        refMsg->Send();
    }

    SetHandler(nullptr);

    hsRefCnt_SafeUnRef( fColorScheme );
    fColorScheme = nullptr;
}

//// ScreenToWorldPoint //////////////////////////////////////////////////////
//  Sometimes it just sucks not having access to the pipeline at just the
//  right time.

void    pfGUIDialogMod::ScreenToWorldPoint( float x, float y, float z, hsPoint3 &outPt )
{
    plViewTransform view = fRenderMod->GetViewTransform();
    view.SetScreenSize( 1, 1 );

    outPt = view.ScreenToWorld( hsPoint3( x, y, z ) );
}

//// WorldToScreenPoint //////////////////////////////////////////////////////
//  Given a point in world-space, translates it into screen coordinates 
//  (with range 0-1, origin top-left).

hsPoint3    pfGUIDialogMod::WorldToScreenPoint( const hsPoint3 &inPt )
{
    plViewTransform view = fRenderMod->GetViewTransform();
    view.SetScreenSize( 1, 1 );

    hsPoint3 tempPt = view.WorldToScreen( inPt );
    tempPt.fZ = view.WorldToCamera( inPt ).fZ;
    return tempPt;
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIDialogMod::IEval( double secs, float del, uint32_t dirty )
{
    return false;
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIDialogMod::MsgReceive( plMessage *msg )
{
    plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
    if( ref )
    {
        switch( ref->fType )
        {
            case kRenderModRef:
                if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                {
                    fRenderMod = plPostEffectMod::ConvertNoRef( ref->GetRef() );
                    fRenderMod->EnableLightsOnRenderRequest();

                    if( fEnabled )
                    {
                        plAnimCmdMsg    *animMsg = new plAnimCmdMsg(GetKey(), fRenderMod->GetKey(), nullptr);
                        animMsg->SetCmd( plAnimCmdMsg::kContinue );
                        animMsg->Send();
                    }
                }
                else if( ref->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
                {
                    plAnimCmdMsg    *animMsg = new plAnimCmdMsg(GetKey(), fRenderMod->GetKey(), nullptr);
                    animMsg->SetCmd( plAnimCmdMsg::kStop );
                    animMsg->Send();

                    fRenderMod = nullptr;
                }
                break;

            case kControlRef:
                if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                {
                    if ((size_t)ref->fWhich >= fControls.size())
                    {
                        hsAssert( false, "Bad index in reffing a control on a GUI dialog" );
                    }
                    else
                    {
                        pfGUIControlMod *oldCtrl = fControls[ ref->fWhich ];

                        fControls[ ref->fWhich ] = pfGUIControlMod::ConvertNoRef( ref->GetRef() );
                        fControls[ ref->fWhich ]->ISetDialog( this );
                        if( oldCtrl != fControls[ ref->fWhich ] )
                            // They're equal on export time, when we DON'T want to be updating the bounds
                            fControls[ ref->fWhich ]->CalcInitialBounds();

                        if( fControls[ ref->fWhich ]->HasFlag( pfGUIControlMod::kInheritProcFromDlg ) )
                            fControls[ ref->fWhich ]->ISetHandler( fHandler );
                    }
                }

                else if( ref->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
                {
                    if ((size_t)ref->fWhich >= fControls.size())
                    {
                        hsAssert( false, "Bad index in unreffing a control on a GUI dialog." );
                    }
                    else
                    {
                        if (fControls[ref->fWhich] != nullptr)
                            fControls[ref->fWhich]->ISetDialog(nullptr);
                        fControls[ref->fWhich] = nullptr;
                    }
                }
                break;
        }
        return true;
    }

    return plSingleModifier::MsgReceive( msg );
}

//// AddControl //////////////////////////////////////////////////////////////

void        pfGUIDialogMod::AddControl( pfGUIControlMod *ctrl )
{
    fControls.emplace_back(ctrl);
    ctrl->ISetDialog( this );
    ctrl->CalcInitialBounds();
}

//// AddControlOnExport //////////////////////////////////////////////////////

void        pfGUIDialogMod::AddControlOnExport( pfGUIControlMod *ctrl )
{
    fControls.emplace_back(ctrl);
    hsgResMgr::ResMgr()->AddViaNotify(ctrl->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, fControls.size() - 1, pfGUIDialogMod::kControlRef), plRefFlags::kActiveRef);
}

//// SetEnabled //////////////////////////////////////////////////////////////

void    pfGUIDialogMod::SetEnabled( bool e )
{
    if( e == fEnabled )
        return;

    fEnabled = e;

    if (fHandler != nullptr)
    {
        if( fEnabled )
            fHandler->OnShow();
        else
            fHandler->OnHide();
    }

    if ( !fEnabled )
    {
        // if we are being hidden then there should be no controls that have interest
        fControlOfInterest = nullptr;
        // also we can purge the dynaText images on the controls
        for (pfGUIControlMod* ctrl : fControls)
        {
            if (ctrl == nullptr)
                continue;
            ctrl->PurgeDynaTextMapImage();
        }
    }

    if (fRenderMod != nullptr)
    {
        plAnimCmdMsg    *animMsg = new plAnimCmdMsg(GetKey(), fRenderMod->GetKey(), nullptr);
        if( fEnabled )
        {
            animMsg->SetCmd( plAnimCmdMsg::kContinue );

            // Update the bounds on all controls that we own
            UpdateAllBounds();
        }
        else
            animMsg->SetCmd( plAnimCmdMsg::kStop );
        animMsg->Send();
    }

}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIDialogMod::Read( hsStream *s, hsResMgr *mgr )
{
    plSingleModifier::Read(s, mgr);

    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRenderModRef ), plRefFlags::kActiveRef );

    char nameBuf[128];
    s->Read(sizeof(nameBuf), nameBuf);
    nameBuf[sizeof(nameBuf) - 1] = 0;
    fName = ST::string(nameBuf);

    uint32_t count = s->ReadLE32();
    fControls.assign(count, nullptr);
    for (uint32_t i = 0; i < count; i++)
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kControlRef ), plRefFlags::kActiveRef );

    // Register us with the Game GUI manager
    plUoid lu( kGameGUIMgr_KEY );
    plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
    if( mgrKey )
    {
        plGenRefMsg *refMsg = new plGenRefMsg( mgrKey, plRefMsg::kOnCreate, 0, pfGameGUIMgr::kDlgModRef );
        hsgResMgr::ResMgr()->AddViaNotify( GetKey(), refMsg, plRefFlags::kPassiveRef );     
    }

    s->ReadLE32(&fTagID);

    fProcReceiver = mgr->ReadKey( s );
    if (fProcReceiver != nullptr)
        SetHandler( new pfGUIDialogNotifyProc( fProcReceiver ) );

    s->ReadLE32(&fVersion);

    fColorScheme->Read( s );

    fSceneNodeKey = mgr->ReadKey( s );
}

void    pfGUIDialogMod::Write( hsStream *s, hsResMgr *mgr )
{
    plSingleModifier::Write( s, mgr );

    mgr->WriteKey( s, fRenderMod->GetKey() );

    char name[128]{ 0 };
    memcpy(name, fName.c_str(), std::min(sizeof(name), fName.size()));
    name[sizeof(name) - 1] = 0;

    s->Write( sizeof( name ), name );

    s->WriteLE32((uint32_t)fControls.size());
    for (pfGUIControlMod* ctrl : fControls)
        mgr->WriteKey(s, ctrl->GetKey());

    s->WriteLE32(fTagID);

    mgr->WriteKey( s, fProcReceiver );

    s->WriteLE32(fVersion);

    fColorScheme->Write( s );

    mgr->WriteKey( s, fSceneNodeKey );
}

plKey   pfGUIDialogMod::GetSceneNodeKey()
{
    if (fSceneNodeKey != nullptr)
        return fSceneNodeKey;

    // Attempt to grab it
    if (GetTarget() != nullptr && GetTarget()->GetSceneNode() != nullptr)
        return ( fSceneNodeKey = GetTarget()->GetSceneNode() );

    return nullptr;
}

//// UpdateInterestingThings /////////////////////////////////////////////////
//  Really. We go through and make sure every control marked as interesting
//  still has the mouse inside it and vice versa.

void    pfGUIDialogMod::UpdateInterestingThings( float mouseX, float mouseY, uint8_t modifiers, bool modalPreset )
{
    hsPoint3    mousePoint(mouseX, mouseY, 0.f);

    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl == nullptr)
            continue;

        // if there was a modal present and we are not modal, then everything is unInteresting!
        if ( modalPreset && !HasFlag(pfGUIDialogMod::kModal) )
        {
            if (ctrl->IsInteresting())
                ctrl->SetInteresting(false);
        }
        else
        {
            if (!ctrl->HasFlag(pfGUIControlMod::kIntangible) && ctrl->PointInBounds(mousePoint)
                || ctrl == fControlOfInterest)
            {
                if (!ctrl->IsInteresting())
                    ctrl->SetInteresting(true);
            }
            else
            {
                if (ctrl->IsInteresting())
                    ctrl->SetInteresting(false);
            }
        }
    }
}

//// HandleMouseEvent ////////////////////////////////////////////////////////

bool        pfGUIDialogMod::HandleMouseEvent( pfGameGUIMgr::EventType event, float mouseX, float mouseY,
                                                uint8_t modifiers )
{
    hsPoint3    mousePoint;

#ifdef HS_DEBUGGING  // Debugging bounds rects
static bool     showBounds = false;

    if( showBounds )
    {
        uint32_t sW, sH;
        plDebugText::Instance().GetScreenSize(&sW,&sH);
        for (pfGUIControlMod* ctrl : fControls)
        {
            if (ctrl == nullptr)
                continue;
            if (ctrl->HasFlag(pfGUIControlMod::kIntangible))
                continue;

            if (!ctrl->fBoundsPoints.empty())
            {
                const hsBounds3 &bnds = ctrl->GetBounds();
                plDebugText::Instance().Draw3DBorder( (uint16_t)(sW * bnds.GetMins().fX),
                                        (uint16_t)(sH * bnds.GetMins().fY),
                                        (uint16_t)(sW * bnds.GetMaxs().fX),
                                        (uint16_t)(sH * bnds.GetMaxs().fY), 0x3000ffff, 0x3000ffff );

                uint32_t color = 0xffff0000;
                for (size_t j = 0; j < ctrl->fBoundsPoints.size(); j++)
                {
//                  color = 0xff000000 | ( ( j * 16 ) << 16 );
                    float x = sW * ctrl->fBoundsPoints[j].fX;
                    float y = sH * ctrl->fBoundsPoints[j].fY;
                    plDebugText::Instance().DrawRect( (uint16_t)(x - 2), (uint16_t)(y - 2), (uint16_t)(x + 2), (uint16_t)(y + 2), color );
                    char str[24];
                    snprintf(str, std::size(str), "%zu", j);
                    plDebugText::Instance().DrawString( (uint16_t)(x + 8), (uint16_t)(y - 8), str, color );
                }
            }
            else
            {
                const hsBounds3 &bnds = ctrl->GetBounds();
                plDebugText::Instance().Draw3DBorder( (uint16_t)(sW * bnds.GetMins().fX),
                                        (uint16_t)(sH * bnds.GetMins().fY),
                                        (uint16_t)(sW * bnds.GetMaxs().fX),
                                        (uint16_t)(sH * bnds.GetMaxs().fY), 0x300000ff, 0x300000ff );
            }
        }
    }
#endif

    mousePoint.Set( mouseX, mouseY, 0.f );

    if( fDragMode )
    {
        IHandleDrag( mousePoint, event, modifiers );
        return true;        // We ALWAYS handle events if we're in drag mode
    }

    if (fControlOfInterest != nullptr)
    {
        // A particular control already has interest--pass messages directly to it no matter what
        fMousedCtrl = fControlOfInterest;
    }
    else
    {
        fMousedCtrl = nullptr;
        float smallestZ = 1.e30f;
        for (pfGUIControlMod* ctrl : fControls)
        {
            if (ctrl != nullptr && !ctrl->HasFlag(pfGUIControlMod::kIntangible) && ctrl->PointInBounds(mousePoint)
                && ctrl->IsVisible() && ctrl->IsEnabled())
            {
                if (ctrl->GetScreenMinZ() < smallestZ)
                {
                    if (ctrl->FilterMousePosition(mousePoint))
                    {
                        fMousedCtrl = ctrl;
                        smallestZ = ctrl->GetScreenMinZ();
                    }
                }
            }
        }
    }

    if (fMousedCtrl != nullptr)
    {
#ifdef HS_DEBUGGING  // Debugging bounds rects
        if (showBounds)
        {
            const hsBounds3 &bnds = fMousedCtrl->GetBounds();
            plDebugText::Instance().DrawString((uint16_t)(bnds.GetMins().fX), (uint16_t)(bnds.GetMins().fY),
                                               fMousedCtrl->GetKeyName().c_str(), (uint32_t)0xffffff00);
        }
#endif

        if( event == pfGameGUIMgr::kMouseDown )
        {
            if( fMousedCtrl->HasFlag( pfGUIControlMod::kWantsInterest ) )
                fControlOfInterest = fMousedCtrl;

            fMousedCtrl->HandleMouseDown( mousePoint, modifiers );

            // Clicking on a control (mouse down) also sets focus to that control. Unlike
            // control-of-interest, this does NOT get reset until a new control is clicked on
            if( fFocusCtrl != fMousedCtrl )
            {
                if (fHandler != nullptr)
                    fHandler->OnCtrlFocusChange( fFocusCtrl, fMousedCtrl );

                if (fFocusCtrl != nullptr)
                    fFocusCtrl->SetFocused( false );
                fFocusCtrl = fMousedCtrl;
                fFocusCtrl->SetFocused( true );
            }
        }
        else if( event == pfGameGUIMgr::kMouseUp )
        {
            fMousedCtrl->HandleMouseUp( mousePoint, modifiers );

            // Controls lose interest on mouse up
            fControlOfInterest = nullptr;
        }
        else if( event == pfGameGUIMgr::kMouseMove )
            fMousedCtrl->HandleMouseHover( mousePoint, modifiers );
        else if( event == pfGameGUIMgr::kMouseDrag )
            fMousedCtrl->HandleMouseDrag( mousePoint, modifiers );
        else if( event == pfGameGUIMgr::kMouseDblClick )
            fMousedCtrl->HandleMouseDblClick( mousePoint, modifiers );

        return true;
    }
    // Clicked on nobody, make sure we lose focus on any controls
    if (fFocusCtrl != nullptr && event == pfGameGUIMgr::kMouseDown)
    {
        if (fHandler != nullptr)
            fHandler->OnCtrlFocusChange(fFocusCtrl, nullptr);

        if (fFocusCtrl != nullptr) // The handler call could've changed it
            fFocusCtrl->SetFocused( false );
        fFocusCtrl = nullptr;
    }

    return false;
}

//// HandleKeyEvent //////////////////////////////////////////////////////////

bool        pfGUIDialogMod::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers )
{
    // Only process if a control has focus...
    if (fFocusCtrl != nullptr)
    {
        // And guess what, it's up to that control to process it! Gee, how easy...
        return fFocusCtrl->HandleKeyEvent( event, key, modifiers );
    }
    return false;
}

//// HandleKeyPress //////////////////////////////////////////////////////////

bool        pfGUIDialogMod::HandleKeyPress( wchar_t key, uint8_t modifiers )
{
    // Same deal as HandleKeyPress. Only problem is, we needed the msg to translate
    // to a char, so it had to be done up at the mgr level (sadly)
    // Only process if a control has focus...

    if (fFocusCtrl != nullptr)
    {
        return fFocusCtrl->HandleKeyPress( key, modifiers );
    }

    return false;
}

//// SetFocus ////////////////////////////////////////////////////////////////

void    pfGUIDialogMod::SetFocus( pfGUIControlMod *ctrl )
{
    if (ctrl != nullptr && ctrl->fDialog != this)
    {
        if (fHandler != nullptr)
            fHandler->OnCtrlFocusChange(fFocusCtrl, nullptr);

        if (fFocusCtrl != nullptr)
            fFocusCtrl->SetFocused( false );
        fFocusCtrl = nullptr;

        ctrl->fDialog->SetFocus( ctrl );
    }
    else if( ctrl != fFocusCtrl )
    {
        if (fFocusCtrl != nullptr)
            fFocusCtrl->SetFocused( false );

        if (fHandler != nullptr)
            fHandler->OnCtrlFocusChange( fFocusCtrl, ctrl );

        fFocusCtrl = ctrl;
        if (fFocusCtrl != nullptr)
            fFocusCtrl->SetFocused( true );
    }
}

//// Show/Hide ///////////////////////////////////////////////////////////////

void    pfGUIDialogMod::Show()
{
    pfGameGUIMgr::GetInstance()->ShowDialog( this );
}

void    pfGUIDialogMod::ShowNoReset()
{
    pfGameGUIMgr::GetInstance()->ShowDialog( this, false );
}

void    pfGUIDialogMod::Hide()
{
    pfGameGUIMgr::GetInstance()->HideDialog( this );
}

//// GetControlFromTag ///////////////////////////////////////////////////////

pfGUIControlMod *pfGUIDialogMod::GetControlFromTag( uint32_t tagID )
{
    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl && ctrl->GetTagID() == tagID)
            return ctrl;
    }

    return nullptr;
}

//// SetControlOfInterest ////////////////////////////////////////////////////

void    pfGUIDialogMod::SetControlOfInterest( pfGUIControlMod *c )
{
    fControlOfInterest = c;
}

//// SetHandler //////////////////////////////////////////////////////////////

void    pfGUIDialogMod::SetHandler( pfGUIDialogProc *hdlr )
{
    if( fHandler && fHandler->DecRef() )
        delete fHandler;

    fHandler = hdlr;
    if (fHandler != nullptr)
    {
        fHandler->IncRef();
        fHandler->SetDialog( this );
    }

    // We also set the handler for any controls that are flagged to inherit
    // from the parent dialog. Note that SetHandlerForAll() can thus be
    // seen as a function that forces this flag (temporarily) on all controls
    for (pfGUIControlMod* ctrl : fControls)
    {
        // Test for nil controls since we get this also on destruct
        if (ctrl != nullptr && ctrl->HasFlag(pfGUIControlMod::kInheritProcFromDlg))
            ctrl->ISetHandler(hdlr);
    }
}

//// SetHandlerForAll ////////////////////////////////////////////////////////
//  Does SetHandler() for the dialog and all of its controls. Handy if you
//  have one of those all-encompasing dialog procs. :)

void    pfGUIDialogMod::SetHandlerForAll( pfGUIDialogProc *hdlr )
{
    SetHandler( hdlr );
    for (pfGUIControlMod* ctrl : fControls)
        ctrl->ISetHandler(hdlr);
}

//// SetControlHandler ///////////////////////////////////////////////////////

void    pfGUIDialogMod::SetControlHandler( uint32_t tagID, pfGUIDialogProc *hdlr )
{
    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl->GetTagID() == tagID)
        {
            ctrl->SetHandler(hdlr);
            break;
        }
    }
}


//// UpdateAspectRatio ///////////////////////////////////////////////////////

void    pfGUIDialogMod::UpdateAspectRatio()
{
    if (fRenderMod)
    {
        // Set width fov respecting height fov
        fRenderMod->SetFovX(pfGameGUIMgr::GetInstance()->GetAspectRatio() * fRenderMod->GetFovY());
    }
    UpdateAllBounds();
}


//// UpdateAllBounds /////////////////////////////////////////////////////////

void    pfGUIDialogMod::UpdateAllBounds()
{
    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl != nullptr)
            ctrl->UpdateBounds(nullptr, true);
    }
}

//// RefreshAllControls //////////////////////////////////////////////////////

void    pfGUIDialogMod::RefreshAllControls()
{
    for (pfGUIControlMod* ctrl : fControls)
        ctrl->IUpdate();
}

//////////////////////////////////////////////////////////////////////////////
//// ListElement Drag Functions //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// ClearDragList ///////////////////////////////////////////////////////////

void    pfGUIDialogMod::ClearDragList()
{
    fDragElements.clear();
}

//// AddToDragList ///////////////////////////////////////////////////////////

void    pfGUIDialogMod::AddToDragList( pfGUIListElement *e )
{
    fDragElements.emplace_back(e);
}

//// EnterDragMode ///////////////////////////////////////////////////////////

void    pfGUIDialogMod::EnterDragMode( pfGUIControlMod *source )
{
    if (!fDragElements.empty())
    {
        fDragMode = true;
        fDragReceptive = false;
        fDragTarget = nullptr;

        fDragSource = source;
    }
}

//// IHandleDrag /////////////////////////////////////////////////////////////
//  Oooh, we're in dragging-list-elements-around mode! So completely ignore 
//  the normal way we do things; what we need to do is wait until the mouse 
//  button is up, all the while testing to see if the control we're on top of
//  is capable of receiving the elements we have. Once the mouse button is let
//  up, if the control is indeed receptive, we call its drag handler for each 
//  of our elements, and either way, exit drag mode.

void    pfGUIDialogMod::IHandleDrag( hsPoint3 &mousePoint, pfGameGUIMgr::EventType event, uint8_t modifiers )
{
    // First, see if our target control has changed
    fMousedCtrl = nullptr;
    float smallestZ = 1.e30f;
    for (pfGUIControlMod* ctrl : fControls)
    {
        if (ctrl->PointInBounds(mousePoint) && ctrl->GetBounds().GetMaxs().fZ < smallestZ)
            fMousedCtrl = ctrl;
    }

    if( fMousedCtrl != fDragTarget )
    {
        // Target has changed, update our receptive flag
        fDragTarget = fMousedCtrl;
        if (fDragTarget == nullptr)
            fDragReceptive = false;
        else
        {
            pfGUIDropTargetProc *dropProc = fDragTarget->GetDropTargetHdlr();
            if (dropProc == nullptr)
                fDragReceptive = false;
            else
            {
                fDragReceptive = true;
                for (pfGUIListElement* dragElement : fDragElements)
                {
                    if (!dropProc->CanEat(dragElement, fDragSource))
                    {
                        fDragReceptive = false;
                        break;
                    }
                }
            }
        }
    }

    if( event == pfGameGUIMgr::kMouseUp )
    {
        /// Mouse got let up--we're exiting drag mode, but can we process the drop?
        fDragMode = false;
        if( fDragReceptive )
        {
            pfGUIDropTargetProc *dropProc = fDragTarget->GetDropTargetHdlr();
            for (pfGUIListElement* dragElement : fDragElements)
                dropProc->Eat(dragElement, fDragSource, fDragTarget);
        }
    }   
}

//// GetDesiredCursor ////////////////////////////////////////////////////////

uint32_t      pfGUIDialogMod::GetDesiredCursor() const
{
    if (fMousedCtrl != nullptr)
        return fMousedCtrl->IGetDesiredCursor();

    return 0;
}
