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
//	pfGUIDialogMod Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"
#include "pfGUIControlMod.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogNotifyProc.h"
#include "pfGUIListElement.h"
#include "../plScene/plPostEffectMod.h"

#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plScene/plSceneNode.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "../plStatusLog/plStatusLog.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "plViewTransform.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIDialogMod::pfGUIDialogMod() : fRenderMod( nil ), fNext( nil ), fPrevPtr( nil )
{
	memset( fName, 0, sizeof( fName ) );
	fEnabled = false;
	fControlOfInterest = nil;
	fFocusCtrl = nil;
	fMousedCtrl = nil;
	fTagID = 0;
	fHandler = nil;
	fVersion = 0;

	fDragMode = false;
	fDragReceptive = false;
	fDragTarget = nil;
	fProcReceiver = nil;

	fColorScheme = TRACKED_NEW pfGUIColorScheme();
}

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
		plGenRefMsg *refMsg = TRACKED_NEW plGenRefMsg( mgrKey, plRefMsg::kOnRemove, 0, pfGameGUIMgr::kDlgModRef );
		refMsg->SetRef( this );
		plgDispatch::MsgSend( refMsg );
	}

	SetHandler( nil );

	hsRefCnt_SafeUnRef( fColorScheme );
	fColorScheme = nil;
}

//// ScreenToWorldPoint //////////////////////////////////////////////////////
//	Sometimes it just sucks not having access to the pipeline at just the
//	right time.

void	pfGUIDialogMod::ScreenToWorldPoint( hsScalar x, hsScalar y, hsScalar z, hsPoint3 &outPt )
{
	plViewTransform view = fRenderMod->GetViewTransform();
	view.SetScreenSize( 1, 1 );

	outPt = view.ScreenToWorld( hsPoint3( x, y, z ) );
}

//// WorldToScreenPoint //////////////////////////////////////////////////////
//	Given a point in world-space, translates it into screen coordinates 
//	(with range 0-1, origin top-left).

hsPoint3	pfGUIDialogMod::WorldToScreenPoint( const hsPoint3 &inPt )
{
	plViewTransform view = fRenderMod->GetViewTransform();
	view.SetScreenSize( 1, 1 );

	hsPoint3 tempPt = view.WorldToScreen( inPt );
	tempPt.fZ = view.WorldToCamera( inPt ).fZ;
	return tempPt;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIDialogMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return false;
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIDialogMod::MsgReceive( plMessage *msg )
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
						plAnimCmdMsg	*animMsg = TRACKED_NEW plAnimCmdMsg( GetKey(), fRenderMod->GetKey(), nil );
						animMsg->SetCmd( plAnimCmdMsg::kContinue );
						plgDispatch::MsgSend( animMsg );
					}
				}
				else if( ref->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
				{
					plAnimCmdMsg	*animMsg = TRACKED_NEW plAnimCmdMsg( GetKey(), fRenderMod->GetKey(), nil );
					animMsg->SetCmd( plAnimCmdMsg::kStop );
					plgDispatch::MsgSend( animMsg );

					fRenderMod = nil;
				}
				break;

			case kControlRef:
				if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				{
					if( ref->fWhich >= fControls.GetCount() )
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
					if( ref->fWhich >= fControls.GetCount() )
					{
						hsAssert( false, "Bad index in unreffing a control on a GUI dialog." );
					}
					else
					{
						if( fControls[ ref->fWhich ] != nil )
							fControls[ ref->fWhich ]->ISetDialog( nil );
						fControls[ ref->fWhich ] = nil;
					}
				}
				break;
		}
		return true;
	}

	return plSingleModifier::MsgReceive( msg );
}

//// AddControl //////////////////////////////////////////////////////////////

void		pfGUIDialogMod::AddControl( pfGUIControlMod *ctrl )
{
	fControls.Append( ctrl );
	ctrl->ISetDialog( this );
	ctrl->CalcInitialBounds();
}

//// AddControlOnExport //////////////////////////////////////////////////////

void		pfGUIDialogMod::AddControlOnExport( pfGUIControlMod *ctrl )
{
	fControls.Append( ctrl );
	hsgResMgr::ResMgr()->AddViaNotify( ctrl->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, fControls.GetCount() - 1, pfGUIDialogMod::kControlRef ), plRefFlags::kActiveRef );
}

//// SetEnabled //////////////////////////////////////////////////////////////

void	pfGUIDialogMod::SetEnabled( hsBool e )
{
	if( e == fEnabled )
		return;

	fEnabled = e;

	if( fHandler != nil )
	{
		if( fEnabled )
			fHandler->OnShow();
		else
			fHandler->OnHide();
	}

	if ( !fEnabled )
	{
		// if we are being hidden then there should be no controls that have interest
		fControlOfInterest = nil;
		// also we can purge the dynaText images on the controls
		int i;
		for( i = 0; i < fControls.GetCount(); i++ )
		{
			if( fControls[ i ] == nil )
				continue;
			fControls[ i ]->PurgeDynaTextMapImage();
		}
	}

	if( fRenderMod != nil )
	{
		plAnimCmdMsg	*animMsg = TRACKED_NEW plAnimCmdMsg( GetKey(), fRenderMod->GetKey(), nil );
		if( fEnabled )
		{
			animMsg->SetCmd( plAnimCmdMsg::kContinue );

			// Update the bounds on all controls that we own
			UpdateAllBounds();
		}
		else
			animMsg->SetCmd( plAnimCmdMsg::kStop );
		plgDispatch::MsgSend( animMsg );
	}

}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIDialogMod::Read( hsStream *s, hsResMgr *mgr )
{
	plSingleModifier::Read(s, mgr);

	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRenderModRef ), plRefFlags::kActiveRef );

	s->Read( sizeof( fName ), fName );

	UInt32	i, count = s->ReadSwap32();
	fControls.SetCountAndZero( count );
	for( i = 0; i < count; i++ )
		mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kControlRef ), plRefFlags::kActiveRef );

	// Register us with the Game GUI manager
	plUoid lu( kGameGUIMgr_KEY );
	plKey mgrKey = hsgResMgr::ResMgr()->FindKey( lu );
	if( mgrKey )
	{
		plGenRefMsg *refMsg = TRACKED_NEW plGenRefMsg( mgrKey, plRefMsg::kOnCreate, 0, pfGameGUIMgr::kDlgModRef );
		hsgResMgr::ResMgr()->AddViaNotify( GetKey(), refMsg, plRefFlags::kPassiveRef );		
	}

	s->ReadSwap( &fTagID );

	fProcReceiver = mgr->ReadKey( s );
	if( fProcReceiver != nil )
		SetHandler( TRACKED_NEW pfGUIDialogNotifyProc( fProcReceiver ) );

	s->ReadSwap( &fVersion );

	fColorScheme->Read( s );

	fSceneNodeKey = mgr->ReadKey( s );
}

void	pfGUIDialogMod::Write( hsStream *s, hsResMgr *mgr )
{
	UInt32	i;


	plSingleModifier::Write( s, mgr );

	mgr->WriteKey( s, fRenderMod->GetKey() );
	s->Write( sizeof( fName ), fName );

	s->WriteSwap32( fControls.GetCount() );
	for( i = 0; i < fControls.GetCount(); i++ )
		mgr->WriteKey( s, fControls[ i ]->GetKey() );

	s->WriteSwap( fTagID );

	mgr->WriteKey( s, fProcReceiver );

	s->WriteSwap( fVersion );

	fColorScheme->Write( s );

	mgr->WriteKey( s, fSceneNodeKey );
}

plKey	pfGUIDialogMod::GetSceneNodeKey( void )
{
	if( fSceneNodeKey != nil )
		return fSceneNodeKey;

	// Attempt to grab it
	if( GetTarget() != nil && GetTarget()->GetSceneNode() != nil )
		return ( fSceneNodeKey = GetTarget()->GetSceneNode() );

	return nil;
}

//// UpdateInterestingThings /////////////////////////////////////////////////
//	Really. We go through and make sure every control marked as interesting
//	still has the mouse inside it and vice versa.

void	pfGUIDialogMod::UpdateInterestingThings( hsScalar mouseX, hsScalar mouseY, UInt8 modifiers, hsBool modalPreset )
{
	int			i;
	hsPoint3	mousePoint;


	mousePoint.Set( mouseX, mouseY, 0.f );

	for( i = 0; i < fControls.GetCount(); i++ )
	{
		if( fControls[ i ] == nil )
			continue;

		// if there was a modal present and we are not modal, then everything is unInteresting!
		if ( modalPreset && !HasFlag(pfGUIDialogMod::kModal) )
		{
			if( fControls[ i ]->IsInteresting() )
				fControls[ i ]->SetInteresting( false );
		}
		else
		{
			if( !fControls[ i ]->HasFlag( pfGUIControlMod::kIntangible ) && fControls[ i ]->PointInBounds( mousePoint ) || fControls[ i ] == fControlOfInterest )
			{
				if( !fControls[ i ]->IsInteresting() )
					fControls[ i ]->SetInteresting( true );
			}
			else
			{
				if( fControls[ i ]->IsInteresting() )
					fControls[ i ]->SetInteresting( false );
			}
		}
	}
}

//// HandleMouseEvent ////////////////////////////////////////////////////////

#ifdef HS_DEBUGGING		 // Debugging bounds rects
#include "../plPipeline/plDebugText.h"
#endif

hsBool		pfGUIDialogMod::HandleMouseEvent( pfGameGUIMgr::EventType event, hsScalar mouseX, hsScalar mouseY,
												UInt8 modifiers )
{
	hsPoint3	mousePoint;
	UInt32		i;

	pfGUIControlMod	*oldInterestingCtrl = nil;
	hsScalar		smallestZ;

#ifdef HS_DEBUGGING	 // Debugging bounds rects
static bool		showBounds = false;

	if( showBounds )
	{
		UInt32 sW, sH;
		plDebugText::Instance().GetScreenSize(&sW,&sH);
		for( i = 0; i < fControls.GetCount(); i++ )
		{
			if( fControls[ i ] == nil )
				continue;
			if( fControls[ i ]->HasFlag( pfGUIControlMod::kIntangible ) )
				continue;

			if( fControls[ i ]->fBoundsPoints.GetCount() > 0 )
			{
				const hsBounds3 &bnds = fControls[ i ]->GetBounds();
				plDebugText::Instance().Draw3DBorder( (UInt16)(sW * bnds.GetMins().fX),
										(UInt16)(sH * bnds.GetMins().fY),
										(UInt16)(sW * bnds.GetMaxs().fX),
										(UInt16)(sH * bnds.GetMaxs().fY), 0x3000ffff, 0x3000ffff );

				UInt32 color = 0xffff0000;
				for( int j = 0; j < fControls[ i ]->fBoundsPoints.GetCount(); j++ )
				{
//					color = 0xff000000 | ( ( j * 16 ) << 16 );
					float x = sW * fControls[ i ]->fBoundsPoints[ j ].fX;
					float y = sH * fControls[ i ]->fBoundsPoints[ j ].fY;
					plDebugText::Instance().DrawRect( (UInt16)(x - 2), (UInt16)(y - 2), (UInt16)(x + 2), (UInt16)(y + 2), color );
					char str[ 16 ];
					itoa( j, str, 10 );
					plDebugText::Instance().DrawString( (UInt16)(x + 8), (UInt16)(y - 8), str, color );
				}
			}
			else
			{
				const hsBounds3 &bnds = fControls[ i ]->GetBounds();
				plDebugText::Instance().Draw3DBorder( (UInt16)(sW * bnds.GetMins().fX),
										(UInt16)(sH * bnds.GetMins().fY),
										(UInt16)(sW * bnds.GetMaxs().fX),
										(UInt16)(sH * bnds.GetMaxs().fY), 0x300000ff, 0x300000ff );
			}
		}
	}
#endif

	mousePoint.Set( mouseX, mouseY, 0.f );

	if( fDragMode )
	{
		IHandleDrag( mousePoint, event, modifiers );
		return true;		// We ALWAYS handle events if we're in drag mode
	}

	oldInterestingCtrl = fMousedCtrl;
	if( fControlOfInterest != nil )
	{
		// A particular control already has interest--pass messages directly to it no matter what
		fMousedCtrl = fControlOfInterest;
	}
	else
	{
		for( i = 0, fMousedCtrl = nil, smallestZ = 1.e30f; i < fControls.GetCount(); i++ )
		{
			if( fControls[ i ] != nil && !fControls[ i ]->HasFlag( pfGUIControlMod::kIntangible ) && fControls[ i ]->PointInBounds( mousePoint ) && fControls[ i ]->IsVisible() && fControls[ i ]->IsEnabled() )
			{
				if( fControls[ i ]->GetScreenMinZ() < smallestZ )
				{
					if( fControls[ i ]->FilterMousePosition( mousePoint ) )
					{
						fMousedCtrl = fControls[ i ];
						smallestZ = fControls[ i ]->GetScreenMinZ();
					}
				}
			}
		}
	}

	if( fMousedCtrl != nil )
	{
#ifdef HS_DEBUGGING	 // Debugging bounds rects
if( showBounds )
{
	const hsBounds3 &bnds = fMousedCtrl->GetBounds();
	plDebugText::Instance().DrawString( (UInt16)(bnds.GetMins().fX), (UInt16)(bnds.GetMins().fY), fMousedCtrl->GetKeyName(), (UInt32)0xffffff00 );
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
				if( fHandler != nil )
					fHandler->OnCtrlFocusChange( fFocusCtrl, fMousedCtrl );

				if( fFocusCtrl != nil )
					fFocusCtrl->SetFocused( false );
				fFocusCtrl = fMousedCtrl;
				fFocusCtrl->SetFocused( true );
			}
		}
		else if( event == pfGameGUIMgr::kMouseUp )
		{
			fMousedCtrl->HandleMouseUp( mousePoint, modifiers );

			// Controls lose interest on mouse up
			fControlOfInterest = nil;
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
	if( fFocusCtrl != nil && event == pfGameGUIMgr::kMouseDown )
	{
		if( fHandler != nil )
			fHandler->OnCtrlFocusChange( fFocusCtrl, nil );

		if( fFocusCtrl != nil )	// The handler call could've changed it
			fFocusCtrl->SetFocused( false );
		fFocusCtrl = nil;
	}

	return false;
}

//// HandleKeyEvent //////////////////////////////////////////////////////////

hsBool		pfGUIDialogMod::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers )
{
	// Only process if a control has focus...
	if( fFocusCtrl != nil )
	{
		// And guess what, it's up to that control to process it! Gee, how easy...
		return fFocusCtrl->HandleKeyEvent( event, key, modifiers );
	}
	return false;
}

//// HandleKeyPress //////////////////////////////////////////////////////////

hsBool		pfGUIDialogMod::HandleKeyPress( char key, UInt8 modifiers )
{
	// Same deal as HandleKeyPress. Only problem is, we needed the msg to translate
	// to a char, so it had to be done up at the mgr level (sadly)
	// Only process if a control has focus...

	if( fFocusCtrl != nil )
	{
		return fFocusCtrl->HandleKeyPress( key, modifiers );
	}

	return false;
}

//// SetFocus ////////////////////////////////////////////////////////////////

void	pfGUIDialogMod::SetFocus( pfGUIControlMod *ctrl )
{
	if( ctrl != nil && ctrl->fDialog != this )
	{
		if( fHandler != nil )
			fHandler->OnCtrlFocusChange( fFocusCtrl, nil );

		if( fFocusCtrl != nil )
			fFocusCtrl->SetFocused( false );
		fFocusCtrl = nil;

		ctrl->fDialog->SetFocus( ctrl );
	}
	else if( ctrl != fFocusCtrl )
	{
		if( fFocusCtrl != nil )
			fFocusCtrl->SetFocused( false );

		if( fHandler != nil )
			fHandler->OnCtrlFocusChange( fFocusCtrl, ctrl );

		fFocusCtrl = ctrl;
		if( fFocusCtrl != nil )
			fFocusCtrl->SetFocused( true );
	}
}

//// Show/Hide ///////////////////////////////////////////////////////////////

void	pfGUIDialogMod::Show( void )
{
	pfGameGUIMgr::GetInstance()->ShowDialog( this );
}

void	pfGUIDialogMod::ShowNoReset( void )
{
	pfGameGUIMgr::GetInstance()->ShowDialog( this, false );
}

void	pfGUIDialogMod::Hide( void )
{
	pfGameGUIMgr::GetInstance()->HideDialog( this );
}

//// GetControlFromTag ///////////////////////////////////////////////////////

pfGUIControlMod	*pfGUIDialogMod::GetControlFromTag( UInt32 tagID )
{
	int		i;

	int ctrlCount = fControls.GetCount();

	for( i = 0; i < ctrlCount; i++ )
	{
		pfGUIControlMod *ctrl = fControls[i];
		if( ctrl && ctrl->GetTagID() == tagID )
			return fControls[ i ];
	}

	return nil;
}

//// SetControlOfInterest ////////////////////////////////////////////////////

void	pfGUIDialogMod::SetControlOfInterest( pfGUIControlMod *c )
{
	fControlOfInterest = c;
}

//// SetHandler //////////////////////////////////////////////////////////////

void	pfGUIDialogMod::SetHandler( pfGUIDialogProc *hdlr )
{
	int		i;


	if( fHandler && fHandler->DecRef() )
		delete fHandler;

	fHandler = hdlr;
	if( fHandler != nil )
	{
		fHandler->IncRef();
		fHandler->SetDialog( this );
	}

	// We also set the handler for any controls that are flagged to inherit
	// from the parent dialog. Note that SetHandlerForAll() can thus be
	// seen as a function that forces this flag (temporarily) on all controls
	for( i = 0; i < fControls.GetCount(); i++ )
	{
		// Test for nil controls since we get this also on destruct
		if( fControls[ i ] != nil && fControls[ i ]->HasFlag( pfGUIControlMod::kInheritProcFromDlg ) )
			fControls[ i ]->ISetHandler( hdlr );
	}
}

//// SetHandlerForAll ////////////////////////////////////////////////////////
//	Does SetHandler() for the dialog and all of its controls. Handy if you
//	have one of those all-encompasing dialog procs. :)

void	pfGUIDialogMod::SetHandlerForAll( pfGUIDialogProc *hdlr )
{
	int		i;


	SetHandler( hdlr );
	for( i = 0; i < fControls.GetCount(); i++ )
		fControls[ i ]->ISetHandler( hdlr );
}

//// SetControlHandler ///////////////////////////////////////////////////////

void	pfGUIDialogMod::SetControlHandler( UInt32 tagID, pfGUIDialogProc *hdlr )
{
	int		i;
	for( i = 0; i < fControls.GetCount(); i++ )
	{
		if( fControls[ i ]->GetTagID() == tagID )
		{
			fControls[ i ]->SetHandler( hdlr );
			break;
		}
	}
}


//// UpdateAspectRatio ///////////////////////////////////////////////////////

void	pfGUIDialogMod::UpdateAspectRatio( void )
{
	if (fRenderMod)
	{
		// Set width fov respecting height fov
		fRenderMod->SetFovX(pfGameGUIMgr::GetInstance()->GetAspectRatio() * fRenderMod->GetFovY());
	}
	UpdateAllBounds();
}


//// UpdateAllBounds /////////////////////////////////////////////////////////

void	pfGUIDialogMod::UpdateAllBounds( void )
{
	int i;
	for( i = 0; i < fControls.GetCount(); i++ )
	{
		if( fControls[ i ] != nil )
			fControls[ i ]->UpdateBounds( nil, true );
	}
}

//// RefreshAllControls //////////////////////////////////////////////////////

void	pfGUIDialogMod::RefreshAllControls( void )
{
	int i;
	for( i = 0; i < fControls.GetCount(); i++ )
		fControls[ i ]->IUpdate();
}

//////////////////////////////////////////////////////////////////////////////
//// ListElement Drag Functions //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// ClearDragList ///////////////////////////////////////////////////////////

void	pfGUIDialogMod::ClearDragList( void )
{
	fDragElements.Reset();
}

//// AddToDragList ///////////////////////////////////////////////////////////

void	pfGUIDialogMod::AddToDragList( pfGUIListElement *e )
{
	fDragElements.Append( e );
}

//// EnterDragMode ///////////////////////////////////////////////////////////

void	pfGUIDialogMod::EnterDragMode( pfGUIControlMod *source )
{
	if( fDragElements.GetCount() > 0 )
	{
		fDragMode = true;
		fDragReceptive = false;
		fDragTarget = nil;

		fDragSource = source;
	}
}

//// IHandleDrag /////////////////////////////////////////////////////////////
//	Oooh, we're in dragging-list-elements-around mode! So completely ignore 
//	the normal way we do things; what we need to do is wait until the mouse 
//	button is up, all the while testing to see if the control we're on top of
//	is capable of receiving the elements we have. Once the mouse button is let
//	up, if the control is indeed receptive, we call its drag handler for each 
//	of our elements, and either way, exit drag mode.

void	pfGUIDialogMod::IHandleDrag( hsPoint3 &mousePoint, pfGameGUIMgr::EventType event, UInt8 modifiers )
{
	int				i;
	hsScalar		smallestZ;


	// First, see if our target control has changed
	for( i = 0, fMousedCtrl = nil, smallestZ = 1.e30f; i < fControls.GetCount(); i++ )
	{
		if( fControls[ i ]->PointInBounds( mousePoint ) && fControls[ i ]->GetBounds().GetMaxs().fZ < smallestZ )
			fMousedCtrl = fControls[ i ];
	}

	if( fMousedCtrl != fDragTarget )
	{
		// Target has changed, update our receptive flag
		fDragTarget = fMousedCtrl;
		if( fDragTarget == nil )
			fDragReceptive = false;
		else
		{
			pfGUIDropTargetProc *dropProc = fDragTarget->GetDropTargetHdlr();
			if( dropProc == nil )
				fDragReceptive = false;
			else
			{
				fDragReceptive = true;
				for( i = 0; i < fDragElements.GetCount(); i++ )
				{
					if( !dropProc->CanEat( fDragElements[ i ], fDragSource ) )
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
			for( i = 0; i < fDragElements.GetCount(); i++ )
				dropProc->Eat( fDragElements[ i ], fDragSource, fDragTarget );
		}
	}	
}

//// GetDesiredCursor ////////////////////////////////////////////////////////

UInt32		pfGUIDialogMod::GetDesiredCursor( void ) const
{
	if( fMousedCtrl != nil ) 
		return fMousedCtrl->IGetDesiredCursor();

	return 0;
}

