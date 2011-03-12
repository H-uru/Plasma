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
//	pfJournalBook Class 													//
//	A generic, high-level, abstract method of creating various Myst-like	//
//	books within the game with very little effort, while ensuring that they	//
//	all remain consistent in appearance and operability.					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "pfJournalBook.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "hsResMgr.h"
#include "pcSmallRect.h"
#include "plgDispatch.h"
#include "../pfGameGUIMgr/pfGUIDialogMod.h"
#include "../pfGameGUIMgr/pfGUIControlMod.h"
#include "../pfGameGUIMgr/pfGUICheckBoxCtrl.h"
#include "../pfGameGUIMgr/pfGUIDialogHandlers.h"
#include "../pfGameGUIMgr/pfGUIDynDisplayCtrl.h"
#include "../pfGameGUIMgr/pfGUIClickMapCtrl.h"
#include "../pfGameGUIMgr/pfGUIButtonMod.h"
#include "../pfGameGUIMgr/pfGUIProgressCtrl.h"
#include "../pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"

#include "../pfMessage/pfGUINotifyMsg.h"
#include "../plGImage/plMipmap.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../plMessage/plLayRefMsg.h"
#include "../plMessage/plMatRefMsg.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/hsGMaterial.h"
#include "../plAgeLoader/plAgeLoader.h"
#include "../pfSurface/plLayerBink.h"

// So we can do image searches in our local age
#include "../plNetClient/plNetClientMgr.h"
#include "../plResMgr/plKeyFinder.h"

// For notify sends
#include "../pnMessage/plNotifyMsg.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "../plMessage/plTimerCallbackMsg.h"

// For custom cursors
#include "../plInputCore/plInputInterface.h"

// For measuring text
#include "../plGImage/plFont.h"

// For SFX
#include "hsTimer.h"



//////////////////////////////////////////////////////////////////////////////
//// pfEsHTMLChunk Class /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class pfEsHTMLChunk
{
	public:

		std::wstring fText;	// Paragraph text, or face name
		plKey	fImageKey;	// Key of image
		UInt8	fFontSize;
		UInt32	fFlags;
		UInt8	fType;
		UInt32	fEventID;

		pcSmallRect	fLinkRect;		// Used only for image chunks, and only when stored in the fVisibleLinks array

		hsColorRGBA	fColor;

		UInt16		fAbsoluteX, fAbsoluteY;

		hsScalar	fCurrOpacity;	// For SFX images
		hsScalar	fSFXTime;		// For SFX images
		hsScalar	fMinOpacity, fMaxOpacity;	

		hsColorRGBA	fCurrColor;
		hsColorRGBA	fOffColor, fOnColor;

		bool	fNoResizeImg;
		Int16	fLineSpacing;
		bool	fTintDecal;
		bool	fLoopMovie;
		bool	fOnCover; // if true, the movie is on the cover
		UInt8	fMovieIndex; // the index of the movie in the source code, used for identification

		enum Flags
		{
			kFontBold	= 0x00000001,
			kFontItalic = 0x00000002,
			kFontRegular= 0x00000004,		// 'cause 0 means "style not defined"
			kFontMask	= kFontBold | kFontItalic | kFontRegular,

			kFontColor	= 0x00000008,
			kFontSpacing= 0x00000010,

			kCenter		= 0x00000001,
			kLeft		= 0x00000002,
			kRight		= 0x00000003,
			kAlignMask	= 0x00000003,

			kBlendAlpha	= 0x00000004,
			kCanLink	= 0x00000008,
			kFloating	= 0x00000010,
			kGlowing	= 0x00000020,
			kActAsCB	= 0x00000040,	// Cause the image to act in a checkbox-like fashion. 
										// Min opacity turns into "off opacity" and max opacity
										// is "on opacity"
			kChecked	= 0x00000080,	// Only for kActAsCB, set if it's currently "checked"
			kTranslucent= 0x00000100	// is the image translucent? if so, use fCurrOpacity
		};

		enum Types
		{
			kEmpty = 0,
			kParagraph,
			kImage,
			kPageBreak,
			kFontChange,
			kMargin,
			kCover,			// Just a placeholder, never actually used after compile time
			kBook,			// another placeholder
			kDecal,
			kMovie,
			kEditable		// placeholder, ver 3.0
		};

		// Paragraph constructor
		pfEsHTMLChunk( const wchar_t *text )
		{
			fType = kParagraph;
			if (text)
				fText = text;
			else
				fText = L"";
			fFlags = kLeft;
			fFontSize = 0;
			fImageKey = nil;
			fEventID = 0;
			fColor.Set( 0.f, 0.f, 0.f, 1.f );
			fAbsoluteX = fAbsoluteY = 0;
			fCurrOpacity = 1.f;
			fMinOpacity = 0.f;
			fMaxOpacity = 1.f;
			fNoResizeImg = false;
			fLineSpacing = 0;
			fTintDecal = false;
			fLoopMovie = true;
			fOnCover = false;
			fMovieIndex = -1;
		}

		// Image constructor (used for decals and movies too)
		pfEsHTMLChunk( plKey imageKey, UInt32 alignFlags )
		{
			fType = kImage;
			fText = L"";
			fFlags = alignFlags;
			fFontSize = 0;
			fImageKey = imageKey;
			fEventID = 0;
			fColor.Set( 0.f, 0.f, 0.f, 1.f );
			fAbsoluteX = fAbsoluteY = 0;
			fCurrOpacity = 1.f;
			fMinOpacity = 0.f;
			fMaxOpacity = 1.f;
			fNoResizeImg = false;
			fLineSpacing = 0;
			fTintDecal = false;
			fLoopMovie = true;
			fOnCover = false;
			fMovieIndex = -1;
		}

		// Page break constructor
		pfEsHTMLChunk()
		{
			fType = kPageBreak;
			fText = L"";
			fImageKey = nil;
			fFontSize = 0;
			fFlags = 0;
			fEventID = 0;
			fColor.Set( 0.f, 0.f, 0.f, 1.f );
			fAbsoluteX = fAbsoluteY = 0;
			fCurrOpacity = 1.f;
			fMinOpacity = 0.f;
			fMaxOpacity = 1.f;
			fNoResizeImg = false;
			fLineSpacing = 0;
			fTintDecal = false;
			fLoopMovie = true;
			fOnCover = false;
			fMovieIndex = -1;
		}

		// Font change constructor
		pfEsHTMLChunk( const wchar_t *face, UInt8 size, UInt32 fontFlags )
		{
			fType = kFontChange;
			if (face)
				fText = face;
			else
				fText = L"";
			fFontSize = size;
			fFlags = fontFlags;
			fImageKey = nil;
			fEventID = 0;
			fColor.Set( 0.f, 0.f, 0.f, 1.f );
			fAbsoluteX = fAbsoluteY = 0;
			fCurrOpacity = 1.f;
			fMinOpacity = 0.f;
			fMaxOpacity = 1.f;
			fNoResizeImg = false;
			fLineSpacing = 0;
			fTintDecal = false;
			fLoopMovie = true;
			fOnCover = false;
			fMovieIndex = -1;
		}

		~pfEsHTMLChunk() {}
};

//////////////////////////////////////////////////////////////////////////////
//// Our Template Dialog Handler /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class pfJournalDlgProc : public pfGUIDialogProc
{
	protected:

		pfBookData	*fBook;

	public:

		enum TagIDs
		{
			kTagBookCover = 100,
			kTagLeftDTMap = 101,
			kTagRightDTMap = 102,
			kTagTurnFrontDTMap = 103,
			kTagTurnBackDTMap = 104,
			kTagTurnPageCtrl = 105,
			kTagLeftPageBtn = 106,
			kTagRightPageBtn = 107,
			kTagOutsideBookBtn = 108,
			kTagCoverLayer = 109,
			kTagLeftCornerBtn = 110,
			kTagRightCornerBtn = 111,
			kTagWidthCBDummy = 112,
			kTagHeightCBDummy = 113,
			kTagLeftEditCtrl = 120,
			kTagRightEditCtrl = 121,
			kTagTurnFrontEditCtrl = 122,
			kTagTurnBackEditCtrl = 123
		};

		pfJournalDlgProc( pfBookData *book ) : fBook( book )
		{
		}

		virtual ~pfJournalDlgProc()
		{
		}

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			if ( fBook && fBook->fCurrBook )
			{
				if( ctrl->GetTagID() == kTagBookCover )
				{
					fBook->fCurrBook->Open();
				}
				else if( ctrl->GetTagID() == kTagLeftPageBtn )
				{
					// only turn pages if the book is actually open
					if( fBook->fCurrentlyOpen )
						fBook->fCurrBook->IHandleLeftSideClick();
				}
				else if( ctrl->GetTagID() == kTagRightPageBtn )
				{
					// only turn pages if the book is actually open
					if( fBook->fCurrentlyOpen )
						fBook->fCurrBook->IHandleRightSideClick();
				}
				else if( ctrl->GetTagID() == kTagOutsideBookBtn )
				{
					if( fBook->fCurrentlyOpen )
						fBook->fCurrBook->CloseAndHide();
					else
						fBook->fCurrBook->Hide();
				}
			}
		}

		// Called on dialog init (i.e. first showing, before OnShow() is called), only ever called once
		virtual void	OnInit( void )
		{
		}

		// Called before the dialog is shown, always after OnInit()
		virtual void	OnShow( void )
		{
		}

		// Called before the dialog is hidden
		virtual void	OnHide( void )
		{
		}

		// Called on the dialog's destructor, before it's unregistered with the game GUI manager
		virtual void	OnDestroy( void )
		{
		}

		// Called when the dialog's focused control changes
		virtual void	OnCtrlFocusChange( pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl )
		{
		}

		// Called when the key bound to a GUI event is pressed. Only called on the top modal dialog
		virtual void	OnControlEvent( ControlEvt event )
		{
			if( event == kExitMode )
			{
				if( fBook->fCurrentlyOpen )
					fBook->fCurrBook->CloseAndHide();
				else
					fBook->fCurrBook->Hide();
			}
		}

		virtual void	HandleExtendedEvent( pfGUIControlMod *ctrl, UInt32 event )
		{
			if (fBook)
			{
				if( ctrl == fBook->fLeftPageMap )
				{
					if( event == pfGUIClickMapCtrl::kMouseHovered )
					{
						if (fBook->fCurrBook)
						{
							// Update our custom cursor on the map
							Int32 idx = fBook->fCurrBook->IFindCurrVisibleLink( false, true );
							if( idx != -1 )
								fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorPoised/*Hand*/ );
							else if(( fBook->fCurrBook->fCurrentPage > 1 )&&( fBook->fCurrBook->fAllowTurning ))
								fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorLeft );
							else if ((fBook->fCurrBook->fAreEditing) && !(fBook->fLeftEditCtrl->ShowingBeginningOfBuffer())) // if we have more buffer to show
								fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorLeft );
							else
								fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorUp );
						}
					}
				}
				else if( ctrl == fBook->fRightPageMap )
				{
					if( event == pfGUIClickMapCtrl::kMouseHovered )
					{
						if (fBook->fCurrBook)
						{
							// Update our custom cursor on the map
							Int32 idx = fBook->fCurrBook->IFindCurrVisibleLink( true, true );
							if( idx != -1 )
								fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorPoised/*Hand*/ );
							else if((fBook->fCurrBook->fAreWeShowing) && ( fBook->fCurrBook->fCurrentPage + 2 <= fBook->fCurrBook->fLastPage )&&( fBook->fCurrBook->fAllowTurning ))
								fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorRight );
							else if((fBook->fCurrBook->fAreEditing) && !(fBook->fRightEditCtrl->ShowingEndOfBuffer())) // if we have more buffer to show
								fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorRight );
							else
								fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorUp );
						}
					}
				}
			}
		}
};

//// Multiline edit handler class ////////////////////////////////////////////

class pfBookMultiLineEditProc : public pfGUIMultiLineEditProc
{
private:
	pfBookData *bookData;
public:
	pfBookMultiLineEditProc(pfBookData *owner) { bookData = owner; }
	virtual ~pfBookMultiLineEditProc() {}

	virtual OnEndOfControlList(Int32 cursorPos) { bookData->HitEndOfControlList(cursorPos); }
	virtual OnBeginningOfControlList(Int32 cursorPos) { bookData->HitBeginningOfControlList(cursorPos); }
};

//// Book data class /////////////////////////////////////////////////////////

pfBookData::pfBookData(const char *guiName /* = nil */)
{
	fCurrBook = nil;
	fDialog = nil;
	fCoverButton = fTurnPageButton = nil;
	fLeftPageMap = fRightPageMap = nil;
	fCoverLayer = nil;
	fCoverMaterial = nil;
	UInt16 i;
	for (i=0; i<4; i++)
		fPageMaterials[i] = nil;
	fLeftCorner = fRightCorner = nil;
	fWidthCtrl = fHeightCtrl = nil;
	fCurrSFXPages = kNoSides;
	fBaseSFXTime = 0.f;
	fResetSFXFlag = false;
	fSFXUpdateFlip = false;
	fCurrentlyTurning = false;

	fRightEditCtrl = fLeftEditCtrl = nil;
	fTurnFrontEditCtrl = fTurnBackEditCtrl = nil;
	fEditable = false;
	fAdjustCursorTo = -1;
	
	if (guiName)
		fGUIName = guiName;
	else
		fGUIName = "BkBook";
}

pfBookData::~pfBookData()
{
	RegisterForSFX( kNoSides );
}

void pfBookData::LoadGUI()
{
	// has the dialog been loaded yet?
	if (!pfGameGUIMgr::GetInstance()->IsDialogLoaded(fGUIName.c_str()))
		// no then load and set handler
		pfGameGUIMgr::GetInstance()->LoadDialog(fGUIName.c_str(), GetKey(), "GUI");
	else
		// yes then just set the handler
		pfGameGUIMgr::GetInstance()->SetDialogToNotify(fGUIName.c_str(), GetKey());
}

hsBool pfBookData::MsgReceive(plMessage *pMsg)
{
	plGenRefMsg *ref = plGenRefMsg::ConvertNoRef(pMsg);
	if(ref != nil)
	{
		if(ref->fType == kRefDialog)
		{
			if(ref->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
			{
				pfGUIDialogMod *temp = pfGUIDialogMod::ConvertNoRef(ref->GetRef());
				if (temp != nil) // sanity check
					fDialog = temp;
			}
			/*else
			{
				fDialog = nil;
				fCoverButton = nil;
			}*/
			return true;
		}
		else if(ref->fType == kRefDefaultCover)
		{
			if(ref->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
				fDefaultCover = plMipmap::ConvertNoRef(ref->GetRef());
			else
				fDefaultCover = nil;
			return true;
		}
	}

	plEventCallbackMsg *callback = plEventCallbackMsg::ConvertNoRef( pMsg );
	if( callback != nil )
	{
		// Our callback message to tell us the page is done flipping
		if( callback->fUser & 0x08 )
		{
			// make sure that we still have a current book
			if (fCurrBook)
			{
				// Or actually maybe it's that we're done closing and should hide now,
				// produced from a CloseAndHide()
				if( callback->fEvent == kStop )
					fCurrBook->Hide();
				else
					fCurrBook->IFinishShow( ( callback->fUser & 0x01 ) ? true : false );
			}
		}
		else if( fCurrentlyTurning )
		{
			if( callback->fUser & 0x04 )
				IFillUncoveringPage( (hsBool)( callback->fUser & 0x01 ) ? true : false ); 
			else if( callback->fUser & 0x02 )
				StartTriggeredFlip( (hsBool)( callback->fUser & 0x01 ) ? true : false );
			else
				IFinishTriggeredFlip( (hsBool)( callback->fUser & 0x01 ) ? true : false );
		}
		return true;
	}

	pfGUINotifyMsg *notify = pfGUINotifyMsg::ConvertNoRef(pMsg);
	if(notify != nil)
	{
		// The only time we should get this is when the dialog loads; after that, we hijack
		// the dialog proc with our own
		IInitTemplate(pfGUIDialogMod::ConvertNoRef(notify->GetSender()->ObjectIsLoaded()));
		return true;
	}

	plTimeMsg *time = plTimeMsg::ConvertNoRef( pMsg );
	if( time != nil && fCurrSFXPages != kNoSides && !fCurrentlyTurning && fCurrentlyOpen )
	{
		IHandleSFX( (hsScalar)time->DSeconds() );
		return true;		
	}

	plTimerCallbackMsg* timerMsg = plTimerCallbackMsg::ConvertNoRef(pMsg);
	if (timerMsg)
	{
		if (timerMsg->fID == 99) // the flip animation is about to end, hide the page to prevent flickering
		{
			// the right side was uncovered, so the left side needs to be hidden
			fLeftEditCtrl->SetVisible(false);
		}
		else if (timerMsg->fID == 98)
		{
			// the left side was uncovered, so the right side needs to be hidden
			fRightEditCtrl->SetVisible(false);
		}
	}

	return hsKeyedObject::MsgReceive(pMsg);
}

void pfBookData::IInitTemplate(pfGUIDialogMod *templateDlg)
{
	hsAssert(templateDlg != nil, "Nil template in pfBookData::IInitTemplate()!");

	// Init and ref our fDialog pointer
	hsgResMgr::ResMgr()->SendRef(templateDlg->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefDialog), plRefFlags::kPassiveRef);  

	// Hijack the dialog proc with our own
	templateDlg->SetHandlerForAll(TRACKED_NEW pfJournalDlgProc(this));

	// Find our animation keys
																
	// And other interesting pointers
	fCoverButton = pfGUICheckBoxCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagBookCover));
	fTurnPageButton = pfGUICheckBoxCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnPageCtrl));
	fLeftPageMap = pfGUIClickMapCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagLeftPageBtn));
	fRightPageMap = pfGUIClickMapCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagRightPageBtn));
	fLeftCorner = pfGUIButtonMod::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagLeftCornerBtn));
	fRightCorner = pfGUIButtonMod::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagRightCornerBtn));
	fWidthCtrl = pfGUIProgressCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagWidthCBDummy));
	fHeightCtrl = pfGUIProgressCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagHeightCBDummy));

	fTurnPageButton->SetEnabled(false);
	fCoverButton->DontPlaySounds();	// dont let checkbox play sounds, journal will take care of that.

	fCoverLayer = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagCoverLayer))->GetLayer(0);
	fCoverMaterial = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagCoverLayer))->GetMaterial(0);

	fPageMaterials[kLeftPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagLeftDTMap))->GetMaterial(0);
	fPageMaterials[kRightPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagRightDTMap))->GetMaterial(0);
	fPageMaterials[kTurnFrontPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnFrontDTMap))->GetMaterial(0);
	fPageMaterials[kTurnBackPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnBackDTMap))->GetMaterial(0);

	// Grab and ref the default cover mipmap
	plLayer *lay = plLayer::ConvertNoRef(fCoverLayer);
	if((lay != nil)&&(lay->GetTexture() != nil))
		hsgResMgr::ResMgr()->AddViaNotify(lay->GetTexture()->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefDefaultCover), plRefFlags::kPassiveRef);  

	fLeftPageMap->SetFlag(pfGUIClickMapCtrl::kReportHovering);
	fRightPageMap->SetFlag(pfGUIClickMapCtrl::kReportHovering);

	fLeftEditCtrl = pfGUIMultiLineEditCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagLeftEditCtrl));
	if (fLeftEditCtrl)
	{
		fLeftEditCtrl->SetEnabled(false); // disable the edit controls initially, we can turn them on later
		fLeftEditCtrl->SetVisible(false);
	}
	fRightEditCtrl = pfGUIMultiLineEditCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagRightEditCtrl));
	if (fRightEditCtrl)
	{
		fRightEditCtrl->SetEnabled(false);
		fRightEditCtrl->SetVisible(false);
	}

	fTurnFrontEditCtrl = pfGUIMultiLineEditCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnFrontEditCtrl));
	if (fTurnFrontEditCtrl)
	{
		fTurnFrontEditCtrl->SetEnabled(false);
		fTurnFrontEditCtrl->SetVisible(false);
	}

	fTurnBackEditCtrl = pfGUIMultiLineEditCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnBackEditCtrl));
	if (fTurnBackEditCtrl)
	{
		fTurnBackEditCtrl->SetEnabled(false);
		fTurnBackEditCtrl->SetVisible(false);
	}
	// if all the edit controls are here, we are editable, so set up the initial link
	if (fLeftEditCtrl && fRightEditCtrl && fTurnFrontEditCtrl && fTurnBackEditCtrl)
	{
		fEditable = true;
		fLeftEditCtrl->SetNext(fRightEditCtrl);
		fLeftEditCtrl->SetEventProc(TRACKED_NEW pfBookMultiLineEditProc(this));
		fTurnFrontEditCtrl->SetNext(fTurnBackEditCtrl); // these are always back to back
	}
}

//// IGetDTMap ///////////////////////////////////////////////////////////////
// Just a quick helper 

plDynamicTextMap *pfBookData::GetDTMap(UInt32 which)
{
	pfGUIDynDisplayCtrl *display = pfGUIDynDisplayCtrl::ConvertNoRef(fDialog->GetControlFromTag(which));
	return display->GetMap(0);
}

//// GetEditCtrl /////////////////////////////////////////////////////////////

pfGUIMultiLineEditCtrl *pfBookData::GetEditCtrl(UInt32 which)
{
	switch (which)
	{
	case pfJournalDlgProc::kTagLeftEditCtrl:
		return fLeftEditCtrl;
	case pfJournalDlgProc::kTagRightEditCtrl:
		return fRightEditCtrl;
	case pfJournalDlgProc::kTagTurnFrontEditCtrl:
		return fTurnFrontEditCtrl;
	case pfJournalDlgProc::kTagTurnBackEditCtrl:
		return fTurnBackEditCtrl;
	default:
		return nil;
	}
}

//// IRegisterForSFX /////////////////////////////////////////////////////////
// Registers (or unregisters) for time messages so we can process special FX 
// if we need to

void pfBookData::RegisterForSFX(WhichSide whichPages)
{
	if( whichPages == fCurrSFXPages)
		return;

	if(whichPages != kNoSides)
	{
		plgDispatch::Dispatch()->RegisterForExactType(plTimeMsg::Index(), GetKey());
	}
	else
		plgDispatch::Dispatch()->UnRegisterForExactType(plTimeMsg::Index(), GetKey());
	
	fCurrSFXPages = whichPages;
}

//// IHandleSFX //////////////////////////////////////////////////////////////
// Process SFX for this frame. Note: if ANYTHING is wrong (page starts not
// calced, pointers bad, etc) just bail, since the SFX are just for visual
// flair and not really needed.

void pfBookData::IHandleSFX(hsScalar currTime, WhichSide whichSide /*= kNoSides*/)
{
	if(fCurrBook == nil)
		return;
	if(whichSide == kNoSides)
	{
		if(fResetSFXFlag)
		{
			fBaseSFXTime=currTime;
			fResetSFXFlag=false;
		}

		fSFXUpdateFlip = !fSFXUpdateFlip;

		// Slightly recursive here to help us out a bit
		if(fSFXUpdateFlip&&(fCurrSFXPages & kLeftSide))
			IHandleSFX(currTime, kLeftSide);
		else if(!fSFXUpdateFlip &&(fCurrSFXPages & kRightSide))
			IHandleSFX(currTime, kRightSide);
		return;
	}

	// Update all SFX images for this page first
	hsScalar deltaT = currTime - fBaseSFXTime;

	UInt32 idx, inc = (whichSide == kLeftSide) ? 0 : 1;
	if(fCurrBook->fPageStarts.GetCount() <= fCurrBook->fCurrentPage + inc + 1)
		return;

	bool stillWant = false;
	for(idx = fCurrBook->fPageStarts[fCurrBook->fCurrentPage + inc]; idx < fCurrBook->fPageStarts[fCurrBook->fCurrentPage + inc + 1]; idx++)
	{
		pfEsHTMLChunk *chunk = fCurrBook->fHTMLSource[idx];
		
		if(chunk->fFlags & pfEsHTMLChunk::kGlowing)
		{
			// Glow SFX: animate opacity based on time offset
			UInt8 isOdd = 0;
			hsScalar newDelta = deltaT;
			while(newDelta > chunk->fSFXTime)
			{
				isOdd = ~isOdd;
				newDelta -= chunk->fSFXTime;
			}

			// If we're not odd, then we're decreasing in opacity, else we're increasing
			if(isOdd)
				newDelta = chunk->fSFXTime - newDelta;

			chunk->fCurrOpacity = chunk->fMaxOpacity - ((chunk->fMaxOpacity - chunk->fMinOpacity)*(newDelta / chunk->fSFXTime));
			stillWant = true;
		}
		else if(chunk->fFlags & pfEsHTMLChunk::kActAsCB)
		{
			// If our opacity doesn't match our checked state, slowly fade to it
			hsColorRGBA	inc;
			inc.Set(0.1f, 0.1f, 0.1f, 0.1f);

			hsColorRGBA &want = (chunk->fFlags & pfEsHTMLChunk::kChecked) ? chunk->fOnColor : chunk->fOffColor;
			if(want != chunk->fCurrColor)
			{
#define COMPARE_ME( wnt, curr ) \
				if( wnt > curr + 0.1f )					\
				{										\
					curr += 0.1f;	stillWant = true;	\
				}										\
				else if( wnt < curr - 0.1f )			\
				{										\
					curr -= 0.1f;	stillWant = true;	\
				}										\
				else									\
					curr = wnt;

				COMPARE_ME( want.r, chunk->fCurrColor.r )
				COMPARE_ME( want.g, chunk->fCurrColor.g )
				COMPARE_ME( want.b, chunk->fCurrColor.b )
				COMPARE_ME( want.a, chunk->fCurrColor.a )
			}
		}
	}

	// All done updating that page. Now render it!
	fCurrBook->IRenderPage( fCurrBook->fCurrentPage + inc, ( whichSide == kLeftSide ) ? pfJournalDlgProc::kTagLeftDTMap : pfJournalDlgProc::kTagRightDTMap );

	if( !stillWant )
	{
		// Done with FX for this page, so unregister for FX on this page now
		RegisterForSFX((WhichSide)(fCurrSFXPages & ~whichSide));
	}
}

//// IFillUncoveringPage /////////////////////////////////////////////////////
// Yet another step in the page flip, to make SURE we're already showing the
// turning page before we fill in the page behind it

void pfBookData::IFillUncoveringPage(hsBool rightSide)
{
	// only show the turning page if the book is open
	if ( CurrentlyOpen() )
		fTurnPageButton->SetVisible(true);
	// make sure there is a current book
	if (fCurrBook)
	{
		if (fCurrBook->fAreEditing)
		{
			int id;
			UpdatePageCorners(rightSide ? kRightSide : kLeftSide);
			if (rightSide)
			{
				fTurnBackEditCtrl->ForceUpdate(); // force everything that is changing to update
				fTurnBackEditCtrl->SetVisible(true); // and make sure everything is showing
				fTurnFrontEditCtrl->ForceUpdate();
				fTurnFrontEditCtrl->SetVisible(true);
				fRightEditCtrl->ForceUpdate();
				fRightEditCtrl->SetVisible(true);
				// The left edit ctrl doesn't update until the page flip animation is done
				id = 99;
			}
			else
			{
				fTurnFrontEditCtrl->ForceUpdate();
				fTurnFrontEditCtrl->SetVisible(true);
				fTurnBackEditCtrl->ForceUpdate();
				fTurnBackEditCtrl->SetVisible(true);
				fLeftEditCtrl->ForceUpdate();
				fLeftEditCtrl->SetVisible(true);
				// The right edit ctrl doesn't update until the page flip animation is done
				id = 98;
			}
			
			// create a timer so we can hide the old left or right turn page right before the animation finishes to prevent flicker
			plTimerCallbackMsg* pTimerMsg = TRACKED_NEW plTimerCallbackMsg(GetKey(),id);
			plgTimerCallbackMgr::NewTimer( .5, pTimerMsg ); // .5 found by trial and error
			return; // the gui controls render everything for us, so ignoring this request
		}
		if(rightSide)
			fCurrBook->IRenderPage(fCurrBook->fCurrentPage + 1, pfJournalDlgProc::kTagRightDTMap);
		else
			fCurrBook->IRenderPage(fCurrBook->fCurrentPage, pfJournalDlgProc::kTagLeftDTMap);
	}

	// Update the page corner we're flipping away from
	UpdatePageCorners(rightSide ? kRightSide : kLeftSide);
}

//// ITriggerPageFlip ////////////////////////////////////////////////////////
// Triggers the start of the page-flipping animation, as well as sets up the callback for when it's finished

void pfBookData::ITriggerPageFlip(hsBool flipBackwards, hsBool immediate)
{
	// Hack here: since we don't have an official interface to select these directly
	// in MAX, we just use a GUI check box to grab them for us, even though we never
	// actually use the functionality of the checkbox itself
	const hsTArray<plKey> &keys = fTurnPageButton->GetAnimationKeys();
	const char *animName = fTurnPageButton->GetAnimationName();

	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
	if (immediate)
	{
		msg->SetCmd(plAnimCmdMsg::kGoToEnd);
	}
	else
	{
		msg->SetCmd(plAnimCmdMsg::kContinue);
		msg->SetCmd(plAnimCmdMsg::kSetForewards);
	}
	msg->SetAnimName(flipBackwards ? "backward" : "forward");
	msg->AddReceivers(keys);
	
	// Here's the whole reason why we're not just checking the checkbox: so we can attach a callback
	// so we know when the animation completes. Pretty sad, huh? Poor checkbox.
	plEventCallbackMsg *eventMsg = TRACKED_NEW plEventCallbackMsg;
	eventMsg->AddReceiver(GetKey());
	eventMsg->fRepeats = 0;
	if (immediate)
	{
		eventMsg->fUser = ((!flipBackwards) ? 0x01 : 0x00) | 0x02;
		eventMsg->fEvent = kSingleFrameAdjust;
	}
	else
	{
		eventMsg->fUser = (flipBackwards ? 0x01 : 0x00);
		eventMsg->fEvent = kStop;
	}
	msg->SetCmd(plAnimCmdMsg::kAddCallbacks);
	msg->AddCallback(eventMsg);
	hsRefCnt_SafeUnRef(eventMsg);
	if (!immediate)
	{
		// We want a second callback to tell us when, indeed, the page has started turning
		// and is thus visible and thus we can actually, really, honestly, safely fill in the
		// page behind it
		eventMsg = TRACKED_NEW plEventCallbackMsg;
		eventMsg->AddReceiver(GetKey());
		eventMsg->fRepeats = 0;
		eventMsg->fUser = !flipBackwards ? (0x04 | 0x01) : 0x04;
		eventMsg->fEvent = kBegin;	// Should cause it to be triggered once it seeks at the start of the command
		msg->AddCallback(eventMsg);
		hsRefCnt_SafeUnRef(eventMsg);
	}
	fCurrentlyTurning = true;

	msg->Send();
}

//// StartTriggeredFlip /////////////////////////////////////////////////////
// Finishes the start of the triggered page flip (once we're sure the 
// animation is at the new frame)

void pfBookData::StartTriggeredFlip(hsBool flipBackwards)
{
	if(flipBackwards)
	{
		ITriggerPageFlip(true, false);
	}
	else
	{
		ITriggerPageFlip(false, false);
	}
}

//// Kill the page flipping cause, we're closing the book

void pfBookData::KillPageFlip()
{
	if ( fCurrentlyTurning )
	{
		//ITriggerPageFlip(false, true);
		fTurnPageButton->SetVisible(false);
	}
}

//// IFinishTriggeredFlip ////////////////////////////////////////////////////
// Finishes the triggered page flip, on callback

void pfBookData::IFinishTriggeredFlip(hsBool wasBackwards)
{
	if (fCurrBook && fCurrBook->fAreEditing) // this is handled differently when we are editing
	{
		fLeftEditCtrl->SetNext(fRightEditCtrl); // relink the original path
		if (!wasBackwards)
		{
			// adjust the starting point of the control (not needed if we weren't backwards since that was done when we started turning
			Int32 newStart = fRightEditCtrl->GetLastVisibleLine();
			fLeftEditCtrl->SetGlobalStartLine(newStart);
		}
		if (fAdjustCursorTo >= 0)
		{
			if (wasBackwards)
			{
				fRightEditCtrl->SetCursorToLoc(fAdjustCursorTo);
				fRightEditCtrl->GetOwnerDlg()->SetFocus(fRightEditCtrl);
			}
			else
			{
				fLeftEditCtrl->SetCursorToLoc(fAdjustCursorTo);
				fLeftEditCtrl->GetOwnerDlg()->SetFocus(fLeftEditCtrl);
			}
			fAdjustCursorTo = -1;
		}

		fTurnFrontEditCtrl->SetVisible(false); // hide the controls
		fTurnBackEditCtrl->SetVisible(false);

		fLeftEditCtrl->SetVisible(true);
		fRightEditCtrl->SetVisible(true);

		fLeftEditCtrl->ForceUpdate();
		fRightEditCtrl->ForceUpdate();
	}
	else if(wasBackwards)
	{
		// Grab the DTMaps for the front of the flip page and the right page, so we can
		// copy the front into the right page
		plDynamicTextMap *turnFront = GetDTMap(pfJournalDlgProc::kTagTurnFrontDTMap);
		plDynamicTextMap *right = GetDTMap(pfJournalDlgProc::kTagRightDTMap);
//		right->Swap( turnFront );
		if ( turnFront->IsValid() && right->IsValid() )
		{
			memcpy(right->GetImage(), turnFront->GetImage(), right->GetLevelSize(0));
			if(right->GetDeviceRef() != nil)
				right->GetDeviceRef()->SetDirty(true);
		}
		// we are going to attempt to re-render the left-hand page
		// sometimes, the book stutters on a page flip and screws up the book
		if (fCurrBook)
		{
			fCurrBook->IRenderPage(fCurrBook->fCurrentPage, pfJournalDlgProc::kTagLeftDTMap);
			// move the videos over
			fCurrBook->IMoveMovies(PageMaterial(kTurnFrontPage),PageMaterial(kRightPage));
		}
	}
	else
	{
		// Grab the DTMaps for the back of the flip page and the left page, so we can
		// copy the back into the left page
		plDynamicTextMap *turnBack = GetDTMap(pfJournalDlgProc::kTagTurnBackDTMap);
		plDynamicTextMap *left = GetDTMap(pfJournalDlgProc::kTagLeftDTMap);
	//		right->Swap( turnFront );
		if ( turnBack->IsValid() && left->IsValid() )
		{
			memcpy(left->GetImage(), turnBack->GetImage(), left->GetLevelSize(0));
			if(left->GetDeviceRef() != nil)
				left->GetDeviceRef()->SetDirty(true);
		}
		// we are going to attempt to re-render the right-hand page
		// sometimes, the book stutters on a page flip and screws up the book
		if (fCurrBook)
		{
			fCurrBook->IRenderPage(fCurrBook->fCurrentPage + 1, pfJournalDlgProc::kTagRightDTMap);
			// move the videos over
			fCurrBook->IMoveMovies(PageMaterial(kTurnBackPage),PageMaterial(kLeftPage));
		}
	}

	// Hide our page flipping button/checkbox/whatever
	fTurnPageButton->SetVisible(false);

	// Update page corners
	UpdatePageCorners(kBothSides);

	fCurrentlyTurning = false;

	// Start our FX once the page is done turning
	fResetSFXFlag = true;
}

//// UpdatePageCorners //////////////////////////////////////////////////////
// Enables or disables the left and right page corners, to indicate current turnage state

void pfBookData::UpdatePageCorners(WhichSide which)
{
	// make sure there is a book to update!
	if (fCurrBook)
	{
		if (!fCurrBook->fAllowTurning || !fCurrBook->fAreWeShowing)
		{
			fLeftCorner->SetVisible(false);
			fLeftCorner->SetEnabled(false);
			fRightCorner->SetVisible(false);
			fRightCorner->SetEnabled(false);
			return;
		}
		if((which == kLeftSide)||(which == kBothSides))
		{
			if (fCurrBook->fAreEditing)
				fLeftCorner->SetVisible(!fLeftEditCtrl->ShowingBeginningOfBuffer()); // only show if the control is not viewing the beginning of the buffer
			else
				fLeftCorner->SetVisible((fCurrBook->fCurrentPage >= 2) ? true : false);
			// Note: always disabled (we just go on the page click itself)
			fLeftCorner->SetEnabled(false);
		}
		if((which == kRightSide)||(which == kBothSides))
		{
			if (fCurrBook->fAreEditing)
				fRightCorner->SetVisible(!fRightEditCtrl->ShowingEndOfBuffer()); // only show if the control is not viewing the end of the buffer
			else
				fRightCorner->SetVisible((fCurrBook->fCurrentPage + 2 <= fCurrBook->fLastPage) ? true : false);
			fRightCorner->SetEnabled(false);
		}
	}
}

//// SetCurrSize ////////////////////////////////////////////////////////////
// Seeks the width and height animations to set the desired book size. Sizes are in % across the animation

void pfBookData::SetCurrSize(hsScalar w, hsScalar h)
{
	fWidthCtrl->SetCurrValue(w);
	fHeightCtrl->SetCurrValue(h);
}

//// PlayBookCloseAnim //////////////////////////////////////////////////////
//	Triggers our animation for closing or opening the book.

void pfBookData::PlayBookCloseAnim(hsBool closeIt /*= true*/, hsBool immediate /*= false*/)
{
	// Disable the book cover button if we're opening, enable otherwise
	fCoverButton->SetEnabled(closeIt);

	// Tell our cover button to check or uncheck
	fCoverButton->SetChecked(!closeIt, immediate);

	// Trigger the open (or close) sound
	if(!immediate)
		fCoverButton->PlaySound(closeIt ? pfGUICheckBoxCtrl::kMouseUp : pfGUICheckBoxCtrl::kMouseDown);

	fCurrentlyOpen = !closeIt;
}

//// Event routines from a linked multi-line edit control ////////////////////

void pfBookData::HitEndOfControlList(Int32 cursorPos)
{
	fAdjustCursorTo = cursorPos;
	if (fCurrBook)
		fCurrBook->NextPage();
}

void pfBookData::HitBeginningOfControlList(Int32 cursorPos)
{
	fAdjustCursorTo = cursorPos;
	if (fCurrBook)
		fCurrBook->PreviousPage();
}

void pfBookData::EnableEditGUI(hsBool enable/* =true */)
{
	if (fEditable)
	{
		fLeftEditCtrl->SetEnabled(enable);
		fLeftEditCtrl->SetVisible(enable);
		fRightEditCtrl->SetEnabled(enable);
		fRightEditCtrl->SetVisible(enable);
		// we don't make these editable because they are temps used for the turning page
		fTurnFrontEditCtrl->SetVisible(false); // we don't want these visible initially either
		fTurnBackEditCtrl->SetVisible(false);
	}
}

//// Our Singleton Stuff /////////////////////////////////////////////////////

//pfJournalBook	*pfJournalBook::fInstance = nil;
std::map<std::string,pfBookData*> pfJournalBook::fBookGUIs;

void	pfJournalBook::SingletonInit( void )
{
	fBookGUIs["BkBook"] = TRACKED_NEW pfBookData(); // load the default book data object
	hsgResMgr::ResMgr()->NewKey("BkBook",fBookGUIs["BkBook"],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
	fBookGUIs["BkBook"]->LoadGUI();
}

void	pfJournalBook::SingletonShutdown( void )
{
	std::map<std::string,pfBookData*>::iterator i = fBookGUIs.begin();
	while (i != fBookGUIs.end())
	{
		pfBookData *bookData = i->second;
		bookData->GetKey()->UnRefObject();
		i->second = nil;
		i++;
	}
	fBookGUIs.clear();
}

void	pfJournalBook::LoadGUI( const char *guiName )
{
	if (fBookGUIs.find(guiName) == fBookGUIs.end()) // is it already loaded?
	{ // nope, load it
		fBookGUIs[guiName] = TRACKED_NEW pfBookData(guiName);
		hsgResMgr::ResMgr()->NewKey(guiName,fBookGUIs[guiName],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
		fBookGUIs[guiName]->LoadGUI();
	}
}

void	pfJournalBook::UnloadGUI( const char *guiName )
{
	if (strcmp(guiName,"BkBook")==0)
		return; // do not allow people to unload the default book gui
	std::map<std::string,pfBookData*>::iterator loc = fBookGUIs.find(guiName);
	if (loc != fBookGUIs.end()) // make sure it's loaded
	{
		fBookGUIs[guiName]->GetKey()->UnRefObject();
		fBookGUIs[guiName] = nil;
		fBookGUIs.erase(loc);
	}
}

void	pfJournalBook::UnloadAllGUIs()
{
	std::map<std::string,pfBookData*>::iterator i = fBookGUIs.begin();
	std::vector<std::string> names;
	while (i != fBookGUIs.end())
	{
		std::string name = i->first;
		names.push_back(name); // store a list of keys
		i++;
	}
	int idx;
	for (idx = 0; idx < names.size(); idx++)
		UnloadGUI(names[idx].c_str()); // UnloadGUI won't unload BkBook
}

//// Constructor /////////////////////////////////////////////////////////////
// The constructor takes in the esHTML source for the journal, along with
// the name of the mipmap to use as the cover of the book. The callback
// key is the keyed object to send event messages to (see <img> tag).

pfJournalBook::pfJournalBook( const char *esHTMLSource, plKey coverImageKey, plKey callbackKey /*= nil*/, 
								const plLocation &hintLoc /* = plLocation::kGlobalFixedLoc */, const char *guiName /* = nil */ )
{
	if (guiName && (strcmp(guiName,"") != 0))
		fCurBookGUI = guiName;
	else
		fCurBookGUI = "BkBook";
	if (fBookGUIs.find(fCurBookGUI) == fBookGUIs.end())
	{
		fBookGUIs[fCurBookGUI] = TRACKED_NEW pfBookData(fCurBookGUI.c_str());
		hsgResMgr::ResMgr()->NewKey(fCurBookGUI.c_str(),fBookGUIs[fCurBookGUI],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
		fBookGUIs[fCurBookGUI]->LoadGUI();
	}
	
	fCurrentPage = 0;
	fLastPage = -1;
	fCoverMipKey = coverImageKey;
	fCoverFromHTML = false;
	fCallbackKey = callbackKey;
	fWidthScale = fHeightScale = 0.f;
	fPageTMargin = fPageLMargin = fPageBMargin = fPageRMargin = 16;
	fAllowTurning = true;
	fAreWeShowing = false;
	fCoverTint.Set( 0.f, 0.f, 0.f, 1.f );
	fTintFirst = true;
	fTintCover = false;
	fAreEditing = false;
	fWantEditing = false;
	fDefLoc = hintLoc;

	wchar_t *wESHTMLSource = hsStringToWString(esHTMLSource);
	fUncompiledSource = wESHTMLSource;
	ICompileSource( wESHTMLSource, hintLoc );
	delete [] wESHTMLSource;
}

pfJournalBook::pfJournalBook( const wchar_t *esHTMLSource, plKey coverImageKey, plKey callbackKey /*= nil*/, 
								const plLocation &hintLoc /* = plLocation::kGlobalFixedLoc */, const char *guiName /* = nil */ )
{
	if (guiName && (strcmp(guiName,"") != 0))
		fCurBookGUI = guiName;
	else
		fCurBookGUI = "BkBook";
	if (fBookGUIs.find(fCurBookGUI) == fBookGUIs.end())
	{
		fBookGUIs[fCurBookGUI] = TRACKED_NEW pfBookData(fCurBookGUI.c_str());
		hsgResMgr::ResMgr()->NewKey(fCurBookGUI.c_str(),fBookGUIs[fCurBookGUI],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
		fBookGUIs[fCurBookGUI]->LoadGUI();
	}
	
	fCurrentPage = 0;
	fLastPage = -1;
	fCoverMipKey = coverImageKey;
	fCoverFromHTML = false;
	fCallbackKey = callbackKey;
	fWidthScale = fHeightScale = 0.f;
	fPageTMargin = fPageLMargin = fPageBMargin = fPageRMargin = 16;
	fAllowTurning = true;
	fAreWeShowing = false;
	fCoverTint.Set( 1.f, 1.f, 1.f, 1.f );
	fTintFirst = true;
	fTintCover = false;
	fAreEditing = false;
	fWantEditing = false;
	fDefLoc = hintLoc;
	fUncompiledSource = esHTMLSource;

	ICompileSource( esHTMLSource, hintLoc );
}

pfJournalBook::~pfJournalBook()
{
	if (fBookGUIs.find(fCurBookGUI) != fBookGUIs.end()) // it might have been deleted before we got here
		if( fBookGUIs[fCurBookGUI] && fBookGUIs[fCurBookGUI]->CurBook() == this )
			Hide();

	IFreeSource();
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfJournalBook::MsgReceive( plMessage *pMsg )
{
	return hsKeyedObject::MsgReceive( pMsg );
}

void	pfJournalBook::SetGUI( const char *guiName )
{
	if (guiName && (strcmp(guiName,"") != 0))
		fCurBookGUI = guiName;
	if (fBookGUIs.find(fCurBookGUI) == fBookGUIs.end())
		fCurBookGUI = "BkBook"; // requested GUI isn't loaded, so use default GUI
	SetEditable(fWantEditing); // make sure that if we want editing, to set it
	ICompileSource(fUncompiledSource.c_str(), fDefLoc); // recompile the source to be safe
}

//// Show ////////////////////////////////////////////////////////////////////
// Shows the book, optionally starting open or closed

void	pfJournalBook::Show( hsBool startOpened /*= false */)
{
	fBookGUIs[fCurBookGUI]->StartedOpen(startOpened);
	fBookGUIs[fCurBookGUI]->CurBook(this);
	fBookGUIs[fCurBookGUI]->SetCurrSize(fWidthScale, fHeightScale);
	ILoadAllImages( false );

	hsGMaterial *cover = fBookGUIs[fCurBookGUI]->CoverMaterial();
	if( cover != nil )
	{
		hsTArray<plLayerInterface*> layers;
		plMipmap *mip = fCoverMipKey ? plMipmap::ConvertNoRef( fCoverMipKey->ObjectIsLoaded() ) : nil;
		if( mip != nil )
		{
			layers.Append(IMakeBaseLayer(mip));

			int i;
			for (i=0; i<fCoverDecals.GetCount(); i++)
			{
				if (fCoverDecals[i]->fType == pfEsHTMLChunk::kDecal)
				{
					plMipmap *decal = plMipmap::ConvertNoRef( fCoverDecals[i]->fImageKey != nil ? fCoverDecals[i]->fImageKey->ObjectIsLoaded() : nil );
					if (decal != nil)
						layers.Append(IMakeDecalLayer(fCoverDecals[i],decal,mip));
				}
				else
				{
					// it's a cover movie, not a decal, so we make a layer, thinking it's at 0,0 and a left map (which gives us the results we want)
					plLayerBink *movieLayer = IMakeMovieLayer(fCoverDecals[i],0,0,mip,pfJournalDlgProc::kTagLeftDTMap,false);
					loadedMovie *movie = TRACKED_NEW loadedMovie;
					movie->movieLayer = movieLayer;
					movie->movieChunk = fCoverDecals[i];
					fLoadedMovies.Append(movie);
					layers.Append(plLayerInterface::ConvertNoRef(movieLayer));
					fVisibleLinks.Reset(); // remove any links that the make movie layer might have added, since a cover movie can't link
				}
			}
			ISetDecalLayers(cover,layers);
		}
		else
		{
			layers.Append(IMakeBaseLayer(fBookGUIs[fCurBookGUI]->DefaultCover()));
			ISetDecalLayers(cover,layers);
		}
		// release our ref on the cover layers since the material will take care of them now
		int i;
		for (i=0; i<layers.GetCount(); i++)
			GetKey()->Release(layers[i]->GetKey());
	}

//	fInstance->IPlayBookCloseAnim( !startOpened, true );
	fBookGUIs[fCurBookGUI]->TurnPageButton()->SetVisible( false );
	ITriggerCloseWithNotify( !startOpened, true );
}

//// IFinishShow /////////////////////////////////////////////////////////////
// Finish showing the book, due to the animation being done seeking

void	pfJournalBook::IFinishShow( hsBool startOpened )
{
	fBookGUIs[fCurBookGUI]->Dialog()->Show();

	fAreWeShowing = true;

	if( startOpened )
	{
		// Render initial pages
		fCurrentPage = 0;
		fVisibleLinks.Reset();
		IRenderPage( 0, pfJournalDlgProc::kTagLeftDTMap );
		IRenderPage( 1, pfJournalDlgProc::kTagRightDTMap );

		fBookGUIs[fCurBookGUI]->UpdatePageCorners( pfBookData::kBothSides );
	}

	ISendNotify( kNotifyShow );
}

//// Hide ////////////////////////////////////////////////////////////////////

void	pfJournalBook::Hide( void )
{
	if (fBookGUIs[fCurBookGUI])
	{
		// we can only hide our own dialog
		if (fBookGUIs[fCurBookGUI]->CurBook() == this )
		{
			if (fBookGUIs[fCurBookGUI]->Dialog())
				fBookGUIs[fCurBookGUI]->Dialog()->Hide();
			fBookGUIs[fCurBookGUI]->CurBook(nil);
			ISendNotify( kNotifyHide );
			ILoadAllImages( true );
			// purge the dynaTextMaps, we're done with them for now
			IPurgeDynaTextMaps();
			// nuke the movies so they don't stay in memory (they're big!)
			int i;
			for( i = 0; i < fLoadedMovies.GetCount(); i++ )
			{
				plLayerBink *movie = fLoadedMovies[ i ]->movieLayer;
				movie->GetKey()->UnRefObject();
				delete fLoadedMovies[ i ];
			}
			fLoadedMovies.Reset();
		}
	}

}

//// Open ////////////////////////////////////////////////////////////////////
// Opens the book, optionally to the given page

void	pfJournalBook::Open( UInt32 startingPage /*= 0 */)
{
	if( !fBookGUIs[fCurBookGUI]->CurrentlyOpen() )
	{
		fBookGUIs[fCurBookGUI]->PlayBookCloseAnim( false );

		// Render initial pages
		fCurrentPage = startingPage;
		fVisibleLinks.Reset();
		IRenderPage( startingPage, pfJournalDlgProc::kTagLeftDTMap );
		IRenderPage( startingPage + 1, pfJournalDlgProc::kTagRightDTMap );

		fBookGUIs[fCurBookGUI]->UpdatePageCorners( pfBookData::kBothSides );
	}
}

//// Close ///////////////////////////////////////////////////////////////////
// Closes the book.

void	pfJournalBook::Close( void )
{
	// don't allow them to close the book if the book started open
	if( !fBookGUIs[fCurBookGUI]->StartedOpen() && fBookGUIs[fCurBookGUI]->CurrentlyOpen() )
	{
		ISendNotify( kNotifyClose );
		fBookGUIs[fCurBookGUI]->PlayBookCloseAnim( true );
	}
}

//// CloseAndHide ////////////////////////////////////////////////////////////
// Closes the book, then calls Hide() once it's done closing

void	pfJournalBook::CloseAndHide( void )
{
	// if they start with the book open, then don't allow them to close it
	if( !fBookGUIs[fCurBookGUI]->StartedOpen() && fBookGUIs[fCurBookGUI]->CurrentlyOpen() )
	{
		// if we are flipping a book then kill the page flipping animation
		if ( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )
			fBookGUIs[fCurBookGUI]->KillPageFlip();
		ISendNotify( kNotifyClose );

		ITriggerCloseWithNotify( true, false );

		// Don't hide until we get the callback!
	}
	else
		// Already closed, just hide
		Hide();
}

//// ITriggerCloseWithNotify /////////////////////////////////////////////////
// Close with a notify

void	pfJournalBook::ITriggerCloseWithNotify( hsBool closeNotOpen, hsBool immediate )
{
	// Disable the book cover button if we're opening, enable otherwise
	fBookGUIs[fCurBookGUI]->CoverButton()->SetEnabled( closeNotOpen );

	// Do the animation manually so we can get a callback
	fBookGUIs[fCurBookGUI]->CurrentlyOpen(!closeNotOpen);

	const hsTArray<plKey> &keys = fBookGUIs[fCurBookGUI]->CoverButton()->GetAnimationKeys();
	const char *animName = fBookGUIs[fCurBookGUI]->CoverButton()->GetAnimationName();

	plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
	if( !immediate )
	{
		msg->SetCmd( plAnimCmdMsg::kContinue );
		msg->SetCmd( closeNotOpen ? plAnimCmdMsg::kSetBackwards : plAnimCmdMsg::kSetForewards );
	}
	else
	{
		msg->SetCmd( plAnimCmdMsg::kStop );
		msg->SetCmd( closeNotOpen ? plAnimCmdMsg::kGoToBegin : plAnimCmdMsg::kGoToEnd );
	}
	msg->SetAnimName( animName );
	msg->AddReceivers( keys );
	
	plEventCallbackMsg *eventMsg = TRACKED_NEW plEventCallbackMsg;
	eventMsg->AddReceiver( fBookGUIs[fCurBookGUI]->GetKey() );
	eventMsg->fRepeats = 0;
	eventMsg->fUser = 0x08 | ( closeNotOpen ? 0x00 : 0x01 );	// So we know which this is for
	eventMsg->fEvent = immediate ? ( kSingleFrameEval ) : kStop;
	msg->SetCmd( plAnimCmdMsg::kAddCallbacks );
	msg->AddCallback( eventMsg );
	hsRefCnt_SafeUnRef( eventMsg );

	msg->Send();

	// Trigger the open (or close) sound
	if( !immediate )
		fBookGUIs[fCurBookGUI]->CoverButton()->PlaySound(closeNotOpen ? pfGUICheckBoxCtrl::kMouseUp : pfGUICheckBoxCtrl::kMouseDown);
}

//// NextPage ////////////////////////////////////////////////////////////////
// Advances forward one page

void	pfJournalBook::NextPage( void )
{
	if( (fBookGUIs[fCurBookGUI]->CurrentlyTurning()) || (!fAllowTurning) || (!fAreWeShowing) )
		return;

	if ((fAreEditing)&&!(fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->ShowingEndOfBuffer())) // we're editing the book, so page turning is different here
	{
		fCurrentPage += 2; // we just go to the next page, an editable book has "infinite" pages anyway

		pfGUIMultiLineEditCtrl *right = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl);
		pfGUIMultiLineEditCtrl *left = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl);
		pfGUIMultiLineEditCtrl *turnFront = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl);
		pfGUIMultiLineEditCtrl *turnBack = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl);
		// re-link the controls with the turn page in the middle
		left->SetNext(turnFront);
		turnBack->SetNext(right);

		// At this point, only the left and right pages are visible, we don't want to actually update anything until the animation
		// starts so that nothing flashes or overdraws.

		fBookGUIs[fCurBookGUI]->StartTriggeredFlip( false );
		fBookGUIs[fCurBookGUI]->TurnPageButton()->PlaySound( pfGUICheckBoxCtrl::kMouseUp );
		ISendNotify(kNotifyNextPage);
	}
	else if( fCurrentPage + 2 <= fLastPage )
	{
		fCurrentPage += 2;
		fVisibleLinks.Reset();

		// Swap the right DT map into the turn page front DTMap, then render
		// the new current page into turn page back and currPage+1 into 
		// the right DTMap
		plDynamicTextMap *turnFront = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagTurnFrontDTMap );
		plDynamicTextMap *right = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagRightDTMap );
		if ( turnFront->IsValid() && right->IsValid() )
		{
			memcpy( turnFront->GetImage(), right->GetImage(), right->GetLevelSize( 0 ) );
			if( turnFront->GetDeviceRef() != nil )
				turnFront->GetDeviceRef()->SetDirty( true );
		}
		// copy the videos over
		IMoveMovies( fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kRightPage), fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kTurnFrontPage) );
		IRenderPage( fCurrentPage, pfJournalDlgProc::kTagTurnBackDTMap );

		// This will fire a callback when it's done that'll let us continue the setup
		fBookGUIs[fCurBookGUI]->StartTriggeredFlip( false );

		// Play us a sound too, if defined on our button
		fBookGUIs[fCurBookGUI]->TurnPageButton()->PlaySound( pfGUICheckBoxCtrl::kMouseUp );

		ISendNotify( kNotifyNextPage );
	}
}

//// PreviousPage ////////////////////////////////////////////////////////////
// Same, only back

void	pfJournalBook::PreviousPage( void )
{
	if(( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )||( !fAllowTurning ))
		return;

	if (fAreEditing) // we're editing the book, so page turning is different here
	{
		if (!(fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->ShowingBeginningOfBuffer())) // make sure we don't flip past the beginning
		{
			if (fCurrentPage >= 2) // this variable can get out of whack if we open the book to a page in the middle
				fCurrentPage -= 2; // just making sure that this doesn't go below zero (and therefore wrap around)
			
			pfGUIMultiLineEditCtrl *right = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl);
			pfGUIMultiLineEditCtrl *left = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl);
			pfGUIMultiLineEditCtrl *turnFront = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl);
			pfGUIMultiLineEditCtrl *turnBack = fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl);
			// adjust the starting position of the left control to the new start
			Int32 newStartLine = left->GetFirstVisibleLine() - ((left->GetNumVisibleLines()-1)*2);
			left->SetGlobalStartLine(newStartLine);
			// re-link the controls with the turn page in the middle
			left->SetNext(turnFront);
			turnBack->SetNext(right);

			// At this point, only the left and right pages are visible, we don't want to actually update anything until the animation
			// starts so that nothing flashes or overdraws.
			
			fBookGUIs[fCurBookGUI]->StartTriggeredFlip( true );
			fBookGUIs[fCurBookGUI]->TurnPageButton()->PlaySound( pfGUICheckBoxCtrl::kMouseUp );
			ISendNotify(kNotifyPreviousPage);
		}
		else
		{
			Close();
		}
	}
	else if( fCurrentPage > 1 )
	{
		fCurrentPage -= 2;
		fVisibleLinks.Reset();

		// Swap the left DT map into the turn page back DTMap, then render
		// the new current page into the left and currPage+1 into 
		// the turn page front DTMap
		plDynamicTextMap *turnBack = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagTurnBackDTMap );
		plDynamicTextMap *left = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagLeftDTMap );
		if ( turnBack->IsValid() && left->IsValid() )
		{
			memcpy( turnBack->GetImage(), left->GetImage(), left->GetLevelSize( 0 ) );
			if( turnBack->GetDeviceRef() != nil )
				turnBack->GetDeviceRef()->SetDirty( true );
		}
		// copy the videos over
		IMoveMovies( fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kLeftPage), fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kTurnBackPage) );
		IRenderPage( fCurrentPage + 1, pfJournalDlgProc::kTagTurnFrontDTMap );

		// This will fire a callback when it's done that'll let us continue the setup
		fBookGUIs[fCurBookGUI]->StartTriggeredFlip( true );

		// Play us a sound too, if defined on our button
		fBookGUIs[fCurBookGUI]->TurnPageButton()->PlaySound( pfGUICheckBoxCtrl::kMouseUp );

		ISendNotify( kNotifyPreviousPage );
	}
	else
	{
		Close();
	}
}

//// IFindCurrVisibleLink ////////////////////////////////////////////////////
// Find the current moused link, if any

Int32	pfJournalBook::IFindCurrVisibleLink( hsBool rightNotLeft, hsBool hoverNotUp )
{
	pfGUIClickMapCtrl *ctrl = ( rightNotLeft ) ? fBookGUIs[fCurBookGUI]->RightPageMap() : fBookGUIs[fCurBookGUI]->LeftPageMap();

	hsPoint3 pt = hoverNotUp ? ctrl->GetLastMousePt() : ctrl->GetLastMouseUpPt();

	// This should be 0-1 in the context of the control, so scale to our DTMap size
	plDynamicTextMap *dtMap = fBookGUIs[fCurBookGUI]->GetDTMap( rightNotLeft ? pfJournalDlgProc::kTagRightDTMap : pfJournalDlgProc::kTagLeftDTMap );
	pt.fX *= (hsScalar)dtMap->GetWidth();
	pt.fY *= (hsScalar)dtMap->GetHeight();
	if( rightNotLeft )
	{
		// Clicks on the right side are offsetted in x by the left side's width
		dtMap = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagLeftDTMap );
		pt.fX += dtMap->GetWidth();
	}

	// Search through the list of visible hotspots
	UInt32 i;
	for( i = 0; i < fVisibleLinks.GetCount(); i++ )
	{
		if( fVisibleLinks[ i ]->fLinkRect.Contains( (Int16)pt.fX, (Int16)pt.fY ) )
		{
			// Found a visible link
			return (Int32)i;
		}
	}

	return -1;
}

//// IHandleLeftSideClick ////////////////////////////////////////////////////

void	pfJournalBook::IHandleLeftSideClick( void )
{
	if( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )
		return;

	Int32 idx = IFindCurrVisibleLink( false, false );
	if( idx != -1 )
	{
		if( fVisibleLinks[ idx ]->fFlags & pfEsHTMLChunk::kActAsCB )
			IHandleCheckClick( idx, pfBookData::kLeftSide );
		else
			ISendNotify( kNotifyImageLink, fVisibleLinks[ idx ]->fEventID );
		return;
	}

	// No link found that we're inside of, so just do the default behavior of turning the page
	PreviousPage();
}

void	pfJournalBook::IHandleRightSideClick( void )
{
	if( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )
		return;

	Int32 idx = IFindCurrVisibleLink( true, false );
	if( idx != -1 )
	{
		if( fVisibleLinks[ idx ]->fFlags & pfEsHTMLChunk::kActAsCB )
			IHandleCheckClick( idx, pfBookData::kRightSide );
		else
			ISendNotify( kNotifyImageLink, fVisibleLinks[ idx ]->fEventID );
		return;
	}

	// No link found that we're inside of, so just do the default behavior of turning the page
	NextPage();
}

//// IHandleCheckClick ///////////////////////////////////////////////////////
// Process a click on the given "check box" image

void	pfJournalBook::IHandleCheckClick( UInt32 idx, pfBookData::WhichSide which )
{
	// Special processing for checkboxes--toggle our state, switch our opacity
	// and then send a notify about our new state
	hsBool check = ( fVisibleLinks[ idx ]->fFlags & pfEsHTMLChunk::kChecked ) ? false : true;
	if( check )
	{
		fVisibleLinks[ idx ]->fFlags |= pfEsHTMLChunk::kChecked;
//		fVisibleLinks[ idx ]->fCurrOpacity = fVisibleLinks[ idx ]->fMaxOpacity;
	}
	else
	{
		fVisibleLinks[ idx ]->fFlags &= ~pfEsHTMLChunk::kChecked;
//		fVisibleLinks[ idx ]->fCurrOpacity = fVisibleLinks[ idx ]->fMinOpacity;
	}

	// Re-render the page we're on, to show the change in state
	IRenderPage( fCurrentPage + ( ( which == pfBookData::kLeftSide ) ? 0 : 1 ), ( which == pfBookData::kLeftSide ) ? pfJournalDlgProc::kTagLeftDTMap: pfJournalDlgProc::kTagRightDTMap );

	ISendNotify( check ? kNotifyImageLink : kNotifyCheckUnchecked, fVisibleLinks[ idx ]->fEventID );

	// Register for FX processing, so we can fade the checkbox in
	fBookGUIs[fCurBookGUI]->RegisterForSFX( (pfBookData::WhichSide)( fBookGUIs[fCurBookGUI]->CurSFXPages() | which ) );
}

//// GoToPage ////////////////////////////////////////////////////////////////
// For completeness...

void	pfJournalBook::GoToPage( UInt32 pageNumber )
{
	// Put us here, but only on an even page (odd pages go on the right, y'know)
	if (pageNumber < fPageStarts.Count())
		fCurrentPage = pageNumber & ~0x00000001;
	else
		fCurrentPage = 0;
	fVisibleLinks.Reset();
	IRenderPage( fCurrentPage, pfJournalDlgProc::kTagLeftDTMap );
	IRenderPage( fCurrentPage + 1, pfJournalDlgProc::kTagRightDTMap );
	fBookGUIs[fCurBookGUI]->UpdatePageCorners( pfBookData::kBothSides );
}

//// SetEditable /////////////////////////////////////////////////////////////

void	pfJournalBook::SetEditable(hsBool editable)
{
	if (fBookGUIs[fCurBookGUI]->IsEditable()) // make sure this GUI supports editing
	{
		fBookGUIs[fCurBookGUI]->EnableEditGUI(editable);
		fAreEditing = editable; // we may be editing the book, so change rendering/page flipping methods
		if (editable)
			fLastPage = 0; // setting this to 0 since editable books don't know what the last page is (it always changes)
	}
	else
		fWantEditing = editable; // we want to edit, but the gui doesn't support it, check again if the GUI changes
};

//// ForceCacheCalculations //////////////////////////////////////////////////
// Just forces a full calc of the cached info

void	pfJournalBook::ForceCacheCalculations( void )
{
	// Make sure our page starts are up-to-snuff, at least to this point
	IRecalcPageStarts( -1 );
}

// Tiny helper to convert hex values the *right* way
static UInt32	IConvertHex( const wchar_t *str )
{
	UInt32 value = 0;
	while( *str != 0 )
	{
		value <<= 4;
		switch( *str )
		{
			case L'0':			value |= 0x0;	break;
			case L'1':			value |= 0x1;	break;
			case L'2':			value |= 0x2;	break;
			case L'3':			value |= 0x3;	break;
			case L'4':			value |= 0x4;	break;
			case L'5':			value |= 0x5;	break;
			case L'6':			value |= 0x6;	break;
			case L'7':			value |= 0x7;	break;
			case L'8':			value |= 0x8;	break;
			case L'9':			value |= 0x9;	break;
			case L'a': case L'A':	value |= 0xa;	break;
			case L'b': case L'B':	value |= 0xb;	break;
			case L'c': case L'C':	value |= 0xc;	break;
			case L'd': case L'D':	value |= 0xd;	break;
			case L'e': case L'E':	value |= 0xe;	break;
			case L'f': case L'F':	value |= 0xf;	break;
		}
		str++;
	}

	return value;
}

//// ICompileSource //////////////////////////////////////////////////////////
// Compiles the given string of esHTML source into our compiled chunk list

hsBool	pfJournalBook::ICompileSource( const wchar_t *source, const plLocation &hintLoc )
{
	IFreeSource();


	pfEsHTMLChunk *chunk, *lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
	const wchar_t	*c, *start;
	wchar_t	name[ 128 ], option[ 256 ];
	float bookWidth=1.0, bookHeight=1.0;
	UInt8 movieIndex = 0; // the index of a movie in the source (used for id purposes)

	plKey anotherKey;


	// Parse our source!
	for( start = c = source; *c != 0; )
	{
		// Are we on a tag?
		UInt8 type = IGetTagType( c );
		if( type != pfEsHTMLChunk::kEmpty )
		{
			// First, end the current paragraph chunk, which is a special case 'cause its 
			// text is defined outside the tag
			if( start == c )
			{
				// No actual text, just delete
				delete lastParChunk;
				lastParChunk = nil;
			}
			else if( lastParChunk != nil )
			{
				UInt32 count = ((UInt32)c - (UInt32)start)/2; // wchar_t is 2 bytes
				
				wchar_t *temp = TRACKED_NEW wchar_t[ count + 1 ];
				wcsncpy( temp, start, count );
				temp[count] = L'\0';
				lastParChunk->fText = temp;
				delete [] temp;

				// Special case to remove any last trailing carriage return
//				if( count > 1 && lastParChunk->fText[ count - 1 ] == '\n' )
//					lastParChunk->fText[ count - 1 ] = 0;

				fHTMLSource.Append( lastParChunk );
			}

			// What chunk are we making now?
			switch( type )
			{
				case pfEsHTMLChunk::kParagraph:
					c += 2;
					chunk = TRACKED_NEW pfEsHTMLChunk( nil );
					chunk->fFlags = IFindLastAlignment();
					while( IGetNextOption( c, name, option ) )
					{
						if( wcsicmp( name, L"align" ) == 0 )
						{
							if( wcsicmp( option, L"left" ) == 0 )
								chunk->fFlags = pfEsHTMLChunk::kLeft;
							else if( wcsicmp( option, L"center" ) == 0 )
								chunk->fFlags = pfEsHTMLChunk::kCenter;
							else if( wcsicmp( option, L"right" ) == 0 )
								chunk->fFlags = pfEsHTMLChunk::kRight;
						}
					}
					// Append text to this one (don't add to source just yet)
					lastParChunk = chunk;
					break;

				case pfEsHTMLChunk::kImage:
					c += 4;
					chunk = TRACKED_NEW pfEsHTMLChunk( nil, 0 );
					while( IGetNextOption( c, name, option ) )
					{
						if( wcsicmp( name, L"align" ) == 0 )
						{
							chunk->fFlags &= ~pfEsHTMLChunk::kAlignMask;
							if( wcsicmp( option, L"left" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kLeft;
							else if( wcsicmp( option, L"center" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kCenter;
							else if( wcsicmp( option, L"right" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kRight;
						}
						else if( wcsicmp( name, L"src" ) == 0 )
						{
							// Name of mipmap source
							chunk->fImageKey = IGetMipmapKey( option, hintLoc );
						}
						else if( wcsicmp( name, L"link" ) == 0 )
						{
							chunk->fEventID = _wtoi( option );
							chunk->fFlags |= pfEsHTMLChunk::kCanLink;
						}
						else if( wcsicmp( name, L"blend" ) == 0 )
						{
							if( wcsicmp( option, L"alpha" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kBlendAlpha;
						}
						else if( wcsicmp( name, L"pos" ) == 0 )
						{
							chunk->fFlags |= pfEsHTMLChunk::kFloating;

							wchar_t *comma = wcschr( option, L',' );
							if( comma != nil )
							{

								chunk->fAbsoluteY = _wtoi( comma + 1 );
								*comma = 0;
							}
							chunk->fAbsoluteX = _wtoi( option );
						}
						else if( wcsicmp( name, L"glow" ) == 0 )
						{
							chunk->fFlags |= pfEsHTMLChunk::kGlowing;
							chunk->fFlags &= ~pfEsHTMLChunk::kActAsCB;

							char *cOption = hsWStringToString(option);
							char *comma = strchr( cOption, ',' );
							if( comma != nil )
							{
								char *comma2 = strchr( comma + 1, ',' );
								if( comma2 != nil )
								{
									chunk->fMaxOpacity = (hsScalar)atof( comma2 + 1 );
									*comma2 = 0;
								}
								chunk->fMinOpacity = (hsScalar)atof( comma + 1 );
								*comma = 0;
							}
							chunk->fSFXTime = (hsScalar)atof( cOption );
							delete [] cOption;
						}
						else if( wcsicmp( name, L"opacity" ) == 0 )
						{
							chunk->fFlags |= pfEsHTMLChunk::kTranslucent;
							char *cOption = hsWStringToString(option);
							chunk->fCurrOpacity = (hsScalar)atof( cOption );
							delete [] cOption;
						}
						else if( wcsicmp( name, L"check" ) == 0 )
						{
							chunk->fFlags |= pfEsHTMLChunk::kActAsCB;
							chunk->fFlags &= ~pfEsHTMLChunk::kGlowing;

							wchar_t *comma = wcschr( option, L',' );
							if( comma != nil )
							{
								wchar_t *comma2 = wcschr( comma + 1, L',' );
								if( comma2 != nil )
								{
									if( _wtoi( comma2 + 1 ) != 0 )
										chunk->fFlags |= pfEsHTMLChunk::kChecked;
									*comma2 = 0;
								}
								UInt32 c = IConvertHex( comma + 1 );
								if( wcslen( comma + 1 ) <= 6 )
									c |= 0xff000000;	// Add in full alpha if none specified
								chunk->fOffColor.FromARGB32( c );
								*comma = 0;
							}
							UInt32 c = IConvertHex( option );
							if( wcslen( option ) <= 6 )
								c |= 0xff000000;	// Add in full alpha if none specified
							chunk->fOnColor.FromARGB32( c );

							if( chunk->fFlags & pfEsHTMLChunk::kChecked )
								chunk->fCurrColor = chunk->fOnColor;
							else
								chunk->fCurrColor = chunk->fOffColor;
						}
						else if (wcsicmp(name,L"resize")==0)
						{
							if (wcsicmp(option,L"no")==0)
								chunk->fNoResizeImg = true;
						}
					}
					if( chunk->fImageKey != nil )
						fHTMLSource.Append( chunk );
					else
						delete chunk;
					// Start new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kCover:
					// Don't create an actual chunk for this one, just use the "src" and 
					// grab the mipmap key for our cover
					c += 6;
					while( IGetNextOption( c, name, option ) )
					{
						if( wcsicmp( name, L"src" ) == 0 )
						{
							// Name of mipmap source
							anotherKey = IGetMipmapKey( option, hintLoc );
							if( anotherKey != nil )
							{
								fCoverMipKey = anotherKey;
								fCoverFromHTML = true;
							}
						}
						if( wcsicmp( name, L"tint" ) == 0 )
						{
							fTintCover = true;
							fCoverTint.FromARGB32( wcstol( option, nil, 16 ) | 0xff000000 );
						}
						if( wcsicmp( name, L"tintfirst" ) == 0 )
						{
							if (wcsicmp(option,L"no")==0)
								fTintFirst = false;
						}
					}
					// Still gotta create a new par chunk
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kPageBreak:
					c += 3;
					chunk = TRACKED_NEW pfEsHTMLChunk();
					while( IGetNextOption( c, name, option ) )
					{
					}
					fHTMLSource.Append( chunk );
					// Start new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kFontChange:
					c += 5;
					chunk = TRACKED_NEW pfEsHTMLChunk( nil, 0, 0 );
					while( IGetNextOption( c, name, option ) )
					{
						if( wcsicmp( name, L"style" ) == 0 )
						{
							UInt8 guiFlags = 0;
							if( wcsicmp( option, L"b" ) == 0 )
							{
								chunk->fFlags = pfEsHTMLChunk::kFontBold;
								guiFlags = plDynamicTextMap::kFontBold;
							}
							else if( wcsicmp( option, L"i" ) == 0 )
							{
								chunk->fFlags = pfEsHTMLChunk::kFontItalic;
								guiFlags = plDynamicTextMap::kFontItalic;
							}
							else if( wcsicmp( option, L"bi" ) == 0 )
							{
								chunk->fFlags = pfEsHTMLChunk::kFontBold | pfEsHTMLChunk::kFontItalic;
								guiFlags = plDynamicTextMap::kFontBold | plDynamicTextMap::kFontItalic;
							}
							else
								chunk->fFlags = pfEsHTMLChunk::kFontRegular;
							if (fBookGUIs[fCurBookGUI]->IsEditable())
							{
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontStyle(guiFlags);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontStyle(guiFlags);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontStyle(guiFlags);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontStyle(guiFlags);
							}
						}
						else if( wcsicmp( name, L"face" ) == 0 )
						{
							// Name of mipmap source
							chunk->fText = option;
							if (fBookGUIs[fCurBookGUI]->IsEditable())
							{
								char *fontFace = hsWStringToString(option);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontFace(fontFace);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontFace(fontFace);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontFace(fontFace);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontFace(fontFace);
								delete [] fontFace;
							}
						}
						else if( wcsicmp( name, L"size" ) == 0 )
						{
							chunk->fFontSize = _wtoi( option );
							if (fBookGUIs[fCurBookGUI]->IsEditable())
							{
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontSize(chunk->fFontSize);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontSize(chunk->fFontSize);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontSize(chunk->fFontSize);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontSize(chunk->fFontSize);
							}
						}
						else if( wcsicmp( name, L"color" ) == 0 )
						{
							chunk->fColor.FromARGB32( wcstol( option, nil, 16 ) | 0xff000000 );
							chunk->fFlags |= pfEsHTMLChunk::kFontColor;
							if (fBookGUIs[fCurBookGUI]->IsEditable())
							{
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontColor(chunk->fColor);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontColor(chunk->fColor);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontColor(chunk->fColor);
								fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontColor(chunk->fColor);
							}
						}
						else if( wcsicmp( name, L"spacing" ) == 0 )
						{
							chunk->fLineSpacing = _wtoi(option);
							chunk->fFlags |= pfEsHTMLChunk::kFontSpacing;
						}
					}
					fHTMLSource.Append( chunk );
					// Start new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kMargin:
					c += 7;
					while(IGetNextOption(c,name,option))
					{
						if (wcsicmp(name,L"top") == 0)
							fPageTMargin = _wtoi(option);
						else if (wcsicmp(name,L"left") == 0)
							fPageLMargin = _wtoi(option);
						else if (wcsicmp(name,L"bottom") == 0)
							fPageBMargin = _wtoi(option);
						else if (wcsicmp(name,L"right") == 0)
							fPageRMargin = _wtoi(option);
					}
					// set the edit controls to the margins we just set
					if (fBookGUIs[fCurBookGUI]->IsEditable())
					{
						fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
						fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
						fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
						fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
					}
					// Start a new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk(nil);
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kBook:
					c += 5;
					// don't actually create a chunk, just set the book size
					while (IGetNextOption(c,name,option))
					{
						if (wcsicmp(name,L"height") == 0)
						{
							char *temp = hsWStringToString(option);
							bookHeight = (float)atof(temp);
							delete [] temp;
						}
						else if (wcsicmp(name,L"width") == 0)
						{
							char *temp = hsWStringToString(option);
							bookWidth = (float)atof(temp);
							delete [] temp;
						}
					}
					fHeightScale = 1.f - bookHeight;
					fWidthScale = 1.f - bookWidth;
					
					// Still gotta create a new par chunk
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kDecal:
					c += 6;
					chunk = TRACKED_NEW pfEsHTMLChunk( nil, 0 );
					chunk->fType = pfEsHTMLChunk::kDecal;
					while( IGetNextOption( c, name, option ) )
					{
						if( wcsicmp( name, L"align" ) == 0 )
						{
							chunk->fFlags &= ~pfEsHTMLChunk::kAlignMask;
							if( wcsicmp( option, L"left" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kLeft;
							else if( wcsicmp( option, L"center" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kCenter;
							else if( wcsicmp( option, L"right" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kRight;
						}
						else if( wcsicmp( name, L"src" ) == 0 )
						{
							// Name of mipmap source
							chunk->fImageKey = IGetMipmapKey( option, hintLoc );
						}
						else if( wcsicmp( name, L"pos" ) == 0 )
						{
							chunk->fFlags |= pfEsHTMLChunk::kFloating;

							wchar_t *comma = wcschr( option, L',' );
							if( comma != nil )
							{

								chunk->fAbsoluteY = _wtoi( comma + 1 );
								*comma = 0;
							}
							chunk->fAbsoluteX = _wtoi( option );
						}
						else if (wcsicmp(name,L"resize")==0)
						{
							if (wcsicmp(option,L"no")==0)
								chunk->fNoResizeImg = true;
						}
						else if (wcsicmp(name,L"tint")==0)
						{
							if (wcsicmp(option,L"yes")==0)
								chunk->fTintDecal = true;
						}
					}
					// add it to our cover decals list (this is tag is essentially thrown away as far as the parser cares)
					if( chunk->fImageKey != nil )
						fCoverDecals.Append( chunk );
					else
						delete chunk;
					// Start new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;

				case pfEsHTMLChunk::kMovie:
					c += 6;
					chunk = TRACKED_NEW pfEsHTMLChunk( nil, 0 );
					chunk->fType = pfEsHTMLChunk::kMovie;
					while( IGetNextOption( c, name, option ) )
					{
						if( wcsicmp( name, L"align" ) == 0 )
						{
							chunk->fFlags &= ~pfEsHTMLChunk::kAlignMask;
							if( wcsicmp( option, L"left" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kLeft;
							else if( wcsicmp( option, L"center" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kCenter;
							else if( wcsicmp( option, L"right" ) == 0 )
								chunk->fFlags |= pfEsHTMLChunk::kRight;
						}
						else if( wcsicmp( name, L"src" ) == 0 )
						{
							chunk->fText = option;
						}
						else if( wcsicmp( name, L"link" ) == 0 )
						{
							chunk->fEventID = _wtoi( option );
							chunk->fFlags |= pfEsHTMLChunk::kCanLink;
						}
						else if( wcsicmp( name, L"pos" ) == 0 )
						{
							chunk->fFlags |= pfEsHTMLChunk::kFloating;

							wchar_t *comma = wcschr( option, L',' );
							if( comma != nil )
							{

								chunk->fAbsoluteY = _wtoi( comma + 1 );
								*comma = 0;
							}
							chunk->fAbsoluteX = _wtoi( option );
						}
						else if (wcsicmp(name,L"resize")==0)
						{
							if (wcsicmp(option,L"no")==0)
								chunk->fNoResizeImg = true;
						}
						else if (wcsicmp(name,L"oncover")==0)
						{
							if (wcsicmp(option,L"yes")==0)
								chunk->fOnCover = true;
						}
						else if (wcsicmp(name,L"loop")==0)
						{
							if (wcsicmp(option,L"no")==0)
								chunk->fLoopMovie = false;
						}
					}
					chunk->fMovieIndex = movieIndex;
					movieIndex++;
					if (chunk->fOnCover)
					{
						if( chunk->fText != L"" )
							fCoverDecals.Append( chunk );
						else
							delete chunk;
					}
					else
					{
						if( chunk->fText != L"" )
							fHTMLSource.Append( chunk );
						else
							delete chunk;
					}
					// Start new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;
					
				case pfEsHTMLChunk::kEditable:
					c += 9;
					SetEditable(true);
					chunk = TRACKED_NEW pfEsHTMLChunk();
					while( IGetNextOption( c, name, option ) )
					{
					}
					fHTMLSource.Append( chunk );
					// Start new paragraph chunk after this one
					lastParChunk = TRACKED_NEW pfEsHTMLChunk( nil );
					lastParChunk->fFlags = IFindLastAlignment();
					break;
			}

			start = c;
		}
		else
		{
			// Keep looking
			c++;
		}
	}

	// Final bit goes into the last paragraph chunk we had
	if( start == c )
	{
		// No actual text, just delete
		delete lastParChunk;
		lastParChunk = nil;
	}
	else if( lastParChunk != nil )
	{
		UInt32 count = (UInt32)c - (UInt32)start;
		
		wchar_t *temp = TRACKED_NEW wchar_t[ count + 1 ];
		wcsncpy( temp, start, count + 1 );
		lastParChunk->fText = temp;
		delete [] temp;
		
		// Special case to remove any last trailing carriage return
//		if( count > 1 && lastParChunk->fText[ count - 1 ] == '\n' )
//			lastParChunk->fText[ count - 1 ] = 0;

		fHTMLSource.Append( lastParChunk );
	}

	// Reset a few
	fPageStarts.Reset();
	fPageStarts.Append( 0 );
	if (fAreEditing)
		fLastPage = 0;
	else
		fLastPage = -1;
	
	return true;
}

UInt8	pfJournalBook::IGetTagType( const wchar_t *string )
{
	if( string[ 0 ] != '<' )
		return pfEsHTMLChunk::kEmpty;

	struct TagRec
	{
		const wchar_t *fTag;
		UInt8		fType;
	} tags[] = { { L"p", pfEsHTMLChunk::kParagraph },
				{ L"img", pfEsHTMLChunk::kImage },
				{ L"pb", pfEsHTMLChunk::kPageBreak },
				{ L"font", pfEsHTMLChunk::kFontChange },
				{ L"margin", pfEsHTMLChunk::kMargin },
				{ L"cover", pfEsHTMLChunk::kCover },
				{ L"book", pfEsHTMLChunk::kBook },
				{ L"decal", pfEsHTMLChunk::kDecal },
				{ L"movie", pfEsHTMLChunk::kMovie },
				{ L"editable", pfEsHTMLChunk::kEditable },
				{ nil, pfEsHTMLChunk::kEmpty } };
				

	UInt32 i;
	for( i = 0; tags[ i ].fTag != nil; i++ )
	{
		if( wcsnicmp( string + 1, tags[ i ].fTag, wcslen( tags[ i ].fTag ) ) == 0 )
		{
			// Found tag--but only space or end tag marker allowed afterwards
			char end = (char)string[ wcslen( tags[ i ].fTag ) + 1 ];
			if( end == '>' || end == ' ' )
				return tags[ i ].fType;
		}
	}

	return pfEsHTMLChunk::kEmpty;
}

hsBool	pfJournalBook::IGetNextOption( const wchar_t *&string, wchar_t *name, wchar_t *option )
{
	const wchar_t *c;


	// Advance past any white space
	while( *string == L' ' )
		string++;

	if( *string == L'>' )
	{
		string++;
		return false;
	}

	// Advance to =
	c = string;
	while( *string != L'>' && *string != L' ' && *string != L'=' && *string != L'\0' )
		string++;

	if( *string != L'=' )
		return false;

	// Copy name
	UInt32 len = ((UInt32)string - (UInt32)c)/2; // divide length by 2 because each character is two bytes
	wcsncpy( name, c, len );
	name[len] = L'\0';

	// Find start of option value
	string++;
	while( *string == L' ' )
		string++;

	if( *string == L'\0' || *string == L'>' )
		return false;

	if( *string == L'\"' )
	{
		// Search for other quote
		string++;
		c = string;
		while( *string != L'>' && *string != L'\"' && *string != L'\0' )
			string++;

		len = ((UInt32)string - (UInt32)c)/2; // divide length by 2 because each character is two bytes
		wcsncpy( option, c, len );
		option[len] = L'\0';
		
		if( *string == L'\"' )
			string++;

		return true;
	}

	// Non-quoted token
	c = string;
	while( *string != L' ' && *string != L'>' && *string != L'\0' )
		string++;

	len = ((UInt32)string - (UInt32)c)/2; // divide length by 2 because each character is two bytes
	wcsncpy( option, c, len );
	option[len] = L'\0';
	
	return true;
}

void	pfJournalBook::IFreeSource( void )
{
	UInt32 i;
	
	for( i = 0; i < fHTMLSource.GetCount(); i++ )
		delete fHTMLSource[ i ];
	fHTMLSource.Reset();

	for( i = 0; i < fCoverDecals.GetCount(); i++ )
		delete fCoverDecals[ i ];
	fCoverDecals.Reset();

	for( i = 0; i < fLoadedMovies.GetCount(); i++ )
	{
		plLayerBink *movie = fLoadedMovies[ i ]->movieLayer;
		movie->GetKey()->UnRefObject();
		delete fLoadedMovies[ i ];
	}
	fLoadedMovies.Reset();
}

//// IGetMipmapKey ///////////////////////////////////////////////////////////
// Looks up the key for a mipmap given the image name. Note that the given
// location is treated as a hint; if the image isn't found in that location,
// the code will attempt to look in the currently loaded age for a matching
// image name.

#ifndef PLASMA_EXTERNAL_RELEASE
#include "../plJPEG/plJPEG.h"
#endif

plKey	pfJournalBook::IGetMipmapKey( const wchar_t *name, const plLocation &loc )
{
	char *cName = hsWStringToString(name);
#ifndef PLASMA_EXTERNAL_RELEASE
	if( strchr( cName, '/' ) != nil || strchr( cName, '\\' ) != nil )
	{
		// For internal use only--we allow local path names of JPEG images, to
		// facilitate fast prototyping
		plMipmap *mip = plJPEG::Instance().ReadFromFile( cName );
		hsgResMgr::ResMgr()->NewKey( cName, mip, loc );
		delete [] cName;
		return mip->GetKey();
	}
#endif

	// Try first to find in the given location
	plUoid myUoid( loc, plMipmap::Index(), cName );
	plKey key = hsgResMgr::ResMgr()->FindKey( myUoid );
	if( key != nil )
	{
		delete [] cName;
		return key;
	}


	// Next, try our "global" pre-defined age
	const plLocation &globLoc = plKeyFinder::Instance().FindLocation( "GUI", "BkBookImages" );
	myUoid = plUoid( globLoc, plMipmap::Index(), cName );
	key = hsgResMgr::ResMgr()->FindKey( myUoid );
	if( key != nil )
	{
		delete [] cName;
		return key;
	}

	// Do a search through our current age with just the name given
	if( plNetClientMgr::GetInstance() != nil )
	{
		const char *thisAge = plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName();
		if( thisAge != nil )
		{
			key = plKeyFinder::Instance().StupidSearch( thisAge, nil, plMipmap::Index(), cName, true );
			if( key != nil )
			{
				delete [] cName;
				return key;
			}
		}
	}

	delete [] cName;
	return nil;
}

//// IRenderPage /////////////////////////////////////////////////////////////
//	Takes the given page out of the source and renders it into the specified
//	DTMap (by GUI tag ID). If no page is cached after this one, also updates
//	the various cached info about page endings, etc.

void	pfJournalBook::IRenderPage( UInt32 page, UInt32 whichDTMap, hsBool suppressRendering /*= false*/ )
{
	if (fAreEditing)
		return; // we don't render if we are editing the book
	
	// Grab the DTMap via the GUI system
	plDynamicTextMap *dtMap = fBookGUIs[fCurBookGUI]->GetDTMap( whichDTMap );
	hsAssert( dtMap != nil, "Invalid DT map in IRenderPage()" );

	loadedMovie *movie = nil;
	bool movieAlreadyLoaded = false;

	// Make sure our page starts are up-to-snuff, at least to this point
	IRecalcPageStarts( page );

	// Render!
	hsColorRGBA color;
	color.Set( 0, 0, 0, 0 );
	if( !suppressRendering )
		dtMap->ClearToColor( color );

	hsGMaterial *material = nil;
	if (whichDTMap == pfJournalDlgProc::kTagLeftDTMap)
		material = fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kLeftPage);
	else if (whichDTMap == pfJournalDlgProc::kTagRightDTMap)
		material = fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kRightPage);
	else if (whichDTMap == pfJournalDlgProc::kTagTurnFrontDTMap)
		material = fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kTurnFrontPage);
	else if (whichDTMap == pfJournalDlgProc::kTagTurnBackDTMap)
		material = fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kTurnBackPage);

	if (material)
	{
		// clear any exiting layers (movies) from the material
		int i;
		for( i = 0; i < material->GetNumLayers(); i++ ) // remove all plLayerBink layers
		{
			plLayerInterface *matLayer = material->GetLayer(i);
			plLayerBink *bink = plLayerBink::ConvertNoRef(matLayer);
			if (bink) // if it was a bink layer
			{
				plMatRefMsg* refMsg = TRACKED_NEW plMatRefMsg(material->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer); // remove it
				hsgResMgr::ResMgr()->SendRef(material->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
			}
		}
	}

	hsAssert(page < fPageStarts.GetCount(), "UnInitialized page start!");
	if( page <= fLastPage 
		&& page < fPageStarts.GetCount())	// Added this as a crash-prevention bandaid - MT
	{
		UInt32 idx;
		UInt16 width, height, y, x, ascent, lastX, lastY;
		
		UInt8		fontFlags, fontSize;
		const wchar_t *fontFace;
		hsColorRGBA	fontColor;
		Int16		fontSpacing;
		hsBool		needSFX = false;

		// Find and set initial font properties
		IFindFontProps( fPageStarts[ page ], fontFace, fontSize, fontFlags, fontColor, fontSpacing );
		dtMap->SetFont( fontFace, fontSize, fontFlags, false );
		dtMap->SetTextColor( fontColor, true );
		dtMap->SetLineSpacing(fontSpacing);

		for( idx = fPageStarts[ page ], x = (UInt16)fPageLMargin, y = (UInt16)fPageTMargin;
			y < (UInt16)(512 - fPageTMargin - fPageBMargin) && idx < fHTMLSource.GetCount(); idx++ )
		{
			if( fPageStarts.GetCount() > page + 1 && idx == fPageStarts[ page + 1 ] )
				break;	// Just go ahead and break at the start of the next page, since we already found it

			pfEsHTMLChunk *chunk = fHTMLSource[ idx ];

			switch( chunk->fType )
			{
				case pfEsHTMLChunk::kParagraph:				
					if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kLeft )
					{
						dtMap->SetJustify( plDynamicTextMap::kLeftJustify );
						x = (UInt16)fPageLMargin; // reset X if our justification changes
					}
					else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kRight )
					{
						dtMap->SetJustify( plDynamicTextMap::kRightJustify );
						x = (UInt16)fPageLMargin; // reset X if our justification changes
					}
					else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kCenter )
					{
						dtMap->SetJustify( plDynamicTextMap::kCenter );
						x = (UInt16)fPageLMargin; // reset X if our justification changes
					}

					dtMap->SetFirstLineIndent( (Int16)(x - fPageLMargin) );
					width = (UInt16)(512 - fPageLMargin - fPageRMargin);
					height = (UInt16)(512 - fPageBMargin - y);
					UInt32 lastChar;
					dtMap->CalcWrappedStringSize( chunk->fText.c_str(), &width, &height, &lastChar, &ascent, &lastX, &lastY );
					width = (UInt16)(512 - fPageLMargin - fPageRMargin);
					if( !suppressRendering )
						dtMap->DrawWrappedString( (UInt16)fPageLMargin, y, chunk->fText.c_str(), width, (UInt16)(512 - fPageBMargin - y), &lastX, &lastY );

					if( lastChar == 0 )
					{
						// This paragraph didn't fit on this page at *all*, so just bump it to the next
						// one artificially (the -- is to account for the for loop; see image handling below)
						y += 512;
						if( idx > fPageStarts[ page ] )
							idx--;
						break;
					}
					if( chunk->fText[ lastChar ] != 0 )
					{
						// Didn't get to render the whole paragraph in this go, so we're going to cheat
						// and split the paragraph up into two so that we can handle it properly. Note:
						// this changes the chunk array beyond this point, so we need to invalidate the
						// cache, but that's ok 'cause if we're doing this, it's probably invalid (or empty)
						// anyway
						int fTextLen = chunk->fText.length();
						wchar_t *s = TRACKED_NEW wchar_t[fTextLen+1];
						wcscpy(s,chunk->fText.c_str());
						s[fTextLen] = L'\0';

						// Note: Makes a copy of the string
						pfEsHTMLChunk *c2 = TRACKED_NEW pfEsHTMLChunk( &s[ lastChar ] );
						c2->fFlags = chunk->fFlags;
						fHTMLSource.Insert( idx + 1, c2 );

						// Clip and reallocate so we don't have two copies laying around
						s[ lastChar ] = L'\0';
						chunk->fText = s;
						delete [] s;

						// Invalidate our cache starting with the next page
						if( fPageStarts.GetCount() > page + 1 )
							fPageStarts.SetCount( page + 1 );

						y += 512;
						break;
					}
					
					x = lastX;
					y = (UInt16)(lastY - dtMap->GetCurrFont()->GetAscent());	// Since our text is top-justified

					if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) != pfEsHTMLChunk::kLeft )
					{
						// Ending X is not guaranteed to be anything useful if we're not left justified
						x = (UInt16)fPageLMargin;
					}

					break;

				case pfEsHTMLChunk::kImage:
					{
						plMipmap *mip = plMipmap::ConvertNoRef( chunk->fImageKey != nil ? chunk->fImageKey->ObjectIsLoaded() : nil );
						if( mip != nil )
						{
							// First, determine if we need to be processing FX messages
							if( chunk->fFlags & pfEsHTMLChunk::kGlowing )
								needSFX = true;
							else if( chunk->fFlags & pfEsHTMLChunk::kActAsCB )
							{
								// If our color doesn't match our checked state, we want to be fading it in
								hsColorRGBA &want = ( chunk->fFlags & pfEsHTMLChunk::kChecked ) ? chunk->fOnColor : chunk->fOffColor;
								if( want != chunk->fCurrColor )
									needSFX = true;
							}

							if( chunk->fFlags & pfEsHTMLChunk::kFloating )
							{
								// Floating image, ignore the text flow completely and just splat the image on!
								IDrawMipmap( chunk, chunk->fAbsoluteX, chunk->fAbsoluteY, mip, dtMap, whichDTMap, suppressRendering );
							}
							else
							{
								if( y + mip->GetHeight() >= 512 - fPageBMargin )
								{
									// Mipmap overlaps the bottom of this page, so forcibly break so we'll
									// end up marking the page break here (note that, unlike paragraphs, we
									// can't really break the mipmap into two...well, OK, we could, but it 
									// wouldn't make much sense :)
									y += (UInt16)(mip->GetHeight());

									// Wonderful, the break breaks us from the switch(), which means the for()
									// loops runs once more and increments idx. So this is to counter that.
									// (We better check tho, just to make sure nobody feeds us an extra-large 
									// image and sends us on an infinite loop)
									if( idx > fPageStarts[ page ] )
										idx--;
									break;
								}
								if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kLeft )
									x = (UInt16)fPageLMargin;
								else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kRight )
									x = (UInt16)(512 - fPageRMargin - mip->GetWidth());
								else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kCenter )
									x = (UInt16)(256 - ( mip->GetWidth() >> 1 ));

								IDrawMipmap( chunk, x, y, mip, dtMap, whichDTMap, suppressRendering );

								y += (UInt16)(mip->GetHeight());
								x = (UInt16)fPageLMargin;
							}
						}
					}
					break;

				case pfEsHTMLChunk::kPageBreak:
					// Time for some creative guesswork. See, if the user put a <pb> in at the end of a page,
					// but coincidentally we already broke to a new page, then the explicit <pb> is redundant,
					// so we don't want it. However, if we're at a new page *because* of a <pb>, then a new <pb>
					// is intentional, so we DO want to process it.
					if( idx == fPageStarts[ page ] )
					{
						// Did we get here b/c the last chunk was a pb?
						if( idx > 0 && fHTMLSource[ idx - 1 ]->fType != pfEsHTMLChunk::kPageBreak )
						{
							// Nope, so we DO want to ignore it!
							continue;
						}
					}
					y = (UInt16)(512 - fPageTMargin - fPageBMargin);
					x = (UInt16)fPageLMargin;
					break;

				case pfEsHTMLChunk::kFontChange:				
					IFindFontProps( idx, fontFace, fontSize, fontFlags, fontColor, fontSpacing );
					dtMap->SetFont( fontFace, fontSize, fontFlags, false );
					dtMap->SetTextColor( fontColor, true );
					dtMap->SetLineSpacing(fontSpacing);
					break;

				case pfEsHTMLChunk::kMovie:
					movieAlreadyLoaded = (IMovieAlreadyLoaded(chunk) != nil); // have we already cached it?
					plLayerBink *movieLayer = IMakeMovieLayer(chunk, x, y, (plMipmap*)dtMap, whichDTMap, suppressRendering);
					if (movieLayer)
					{
						// adjust the starting height of the movie if we are keeping it inline with the text
						UInt32 movieHeight = 0, movieWidth = 0;
						movieHeight = movieLayer->GetHeight();
						movieWidth = movieLayer->GetWidth();
						if(!(chunk->fFlags & pfEsHTMLChunk::kFloating ))
						{
							if( y + movieHeight >= 512 - fPageBMargin )
							{
								// Movie overlaps the bottom of this page, so forcibly break so we'll
								// end up marking the page break here (note that, unlike paragraphs, we
								// can't really break the Movie into two)
								y += (UInt16)movieHeight;

								// Wonderful, the break breaks us from the switch(), which means the for()
								// loops runs once more and increments idx. So this is to counter that.
								// (We better check tho, just to make sure nobody feeds us an extra-large 
								// image and sends us on an infinite loop)
								if( idx > fPageStarts[ page ] )
									idx--;
								break;
							}
							y += (UInt16)movieHeight;
							x = (UInt16)fPageLMargin;
						}
						if (!movieAlreadyLoaded) // if the movie wasn't already cached, cache it
						{
							movie = TRACKED_NEW loadedMovie;
							movie->movieLayer = movieLayer; // save the layer and chunk data
							movie->movieChunk = chunk;
							fLoadedMovies.Append(movie);
							movie = nil;
							movieAlreadyLoaded = false;
						}
						if (material && !suppressRendering)
							material->AddLayerViaNotify(movieLayer);
					}
					break;
			}
		}

		if( fPageStarts.GetCount() <= page + 1 )
			fPageStarts.ExpandAndZero( page + 2 );
		fPageStarts[ page + 1 ] = idx;

		if( idx == fHTMLSource.GetCount() )
			fLastPage = page;

		pfBookData::WhichSide thisWhich = ( whichDTMap == pfJournalDlgProc::kTagRightDTMap ) ? pfBookData::kRightSide : ( whichDTMap == pfJournalDlgProc::kTagLeftDTMap )  ? pfBookData::kLeftSide : pfBookData::kNoSides;
		if( needSFX )
			fBookGUIs[fCurBookGUI]->RegisterForSFX( (pfBookData::WhichSide)( fBookGUIs[fCurBookGUI]->CurSFXPages() | thisWhich ) );
		else
			fBookGUIs[fCurBookGUI]->RegisterForSFX( (pfBookData::WhichSide)( fBookGUIs[fCurBookGUI]->CurSFXPages() & ~thisWhich ) );
	}

	if( !suppressRendering )
		dtMap->FlushToHost();
}

//// IMoveMovies /////////////////////////////////////////////////////////////

void	pfJournalBook::IMoveMovies( hsGMaterial *source, hsGMaterial *dest )
{
	hsTArray<plLayerBink*> moviesOnPage;
	if (source && dest)
	{
		// clear any exiting layers (movies) from the material and save them to our local array
		int i;
		for( i = 0; i < source->GetNumLayers(); i++ ) // remove all plLayerBink layers
		{
			plLayerInterface *matLayer = source->GetLayer(i);
			plLayerBink *bink = plLayerBink::ConvertNoRef(matLayer);
			if (bink) // if it was a bink layer
			{
				plMatRefMsg* refMsg = TRACKED_NEW plMatRefMsg(source->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer); // remove it
				hsgResMgr::ResMgr()->SendRef(source->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
				moviesOnPage.Append(bink);
			}
		}
		// clear the destination's movies (if it has any)
		for( i = 0; i < dest->GetNumLayers(); i++ ) // remove all plLayerBink layers
		{
			plLayerInterface *matLayer = dest->GetLayer(i);
			plLayerBink *bink = plLayerBink::ConvertNoRef(matLayer);
			if (bink) // if it was a bink layer
			{
				plMatRefMsg* refMsg = TRACKED_NEW plMatRefMsg(dest->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer); // remove it
				hsgResMgr::ResMgr()->SendRef(dest->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
			}
		}
		// put the movies we ripped off the old page onto the new one
		for( i = 0; i < moviesOnPage.GetCount(); i++ )
		{
			dest->AddLayerViaNotify(moviesOnPage[i]);
		}
	}
}

//// IDrawMipmap /////////////////////////////////////////////////////////////

void	pfJournalBook::IDrawMipmap( pfEsHTMLChunk *chunk, UInt16 x, UInt16 y, plMipmap *mip, plDynamicTextMap *dtMap, UInt32 whichDTMap, hsBool dontRender )
{
	plMipmap *copy = TRACKED_NEW plMipmap();
	copy->CopyFrom(mip);
	if (chunk->fNoResizeImg)
	{
		// book is NOT square, there is a h/w ratio of 1/0.7
		// calc new size based on how the book has been skewed
		float xScale = (fWidthScale == 0) ? 1 : 1/(1-fWidthScale);
		float yScale = (fHeightScale == 0) ? 1 : 1/(1-fHeightScale);
		yScale *= 0.7; // adjust because the book isn't square
		UInt32 width = (UInt32)(mip->GetWidth()*xScale);
		UInt32 height = (UInt32)(mip->GetHeight()*yScale);
		UInt16 xShift;
		UInt16 yShift;

		if (dtMap->GetWidth() < width) width = dtMap->GetWidth();
		if (dtMap->GetHeight() < height) height = dtMap->GetHeight();

		if (height < mip->GetHeight())
		{
			yShift = (UInt16)((mip->GetHeight()-height)/2);
			if (y+yShift+height > dtMap->GetHeight())
				y = (UInt16)(dtMap->GetHeight()-height);
			else
				y += yShift;
		}
		else
		{
			yShift = (UInt16)((height-mip->GetHeight())/2);
			if (yShift > y)
				y = 0;
			else
				y -= yShift;
		}

		if (width < mip->GetWidth())
		{
			xShift = (UInt16)((mip->GetWidth()-width)/2);
			if (x+xShift+width > dtMap->GetWidth())
				x = (UInt16)(dtMap->GetWidth()-width);
			else
				x += xShift;
		}
		else
		{
			xShift = (UInt16)((width-mip->GetWidth())/2);
			if (xShift > x)
				x = 0;
			else
				x -= xShift;
		}
		
		copy->SetCurrLevel(0); // resize the image so it will look unchanged when rendered on the altered book
		copy->ResizeNicely((UInt16)width,(UInt16)height,plMipmap::kDefaultFilter);
	}
	if( !dontRender )
	{
		plMipmap::CompositeOptions	opts;
		if( chunk->fFlags & pfEsHTMLChunk::kActAsCB )
		{
			opts.fFlags = ( chunk->fFlags & pfEsHTMLChunk::kBlendAlpha ) ? 0 : plMipmap::kBlendWriteAlpha;
			opts.fRedTint = chunk->fCurrColor.r;
			opts.fGreenTint = chunk->fCurrColor.g;
			opts.fBlueTint = chunk->fCurrColor.b;
			opts.fOpacity = (UInt8)(chunk->fCurrColor.a * 255.f);
		}
		else 
		{
			if( chunk->fFlags & pfEsHTMLChunk::kGlowing )
				opts.fFlags = ( chunk->fFlags & pfEsHTMLChunk::kBlendAlpha ) ? 0 : plMipmap::kMaskSrcAlpha;
			else if (chunk->fFlags & pfEsHTMLChunk::kTranslucent)
				opts.fFlags = plMipmap::kMaskSrcAlpha;
			else
				opts.fFlags = ( chunk->fFlags & pfEsHTMLChunk::kBlendAlpha ) ? plMipmap::kCopySrcAlpha : plMipmap::kForceOpaque;
			opts.fOpacity = (UInt8)(chunk->fCurrOpacity * 255.f);
		}
		dtMap->Composite( copy, x, y, &opts );
	}

	if( chunk->fFlags & pfEsHTMLChunk::kCanLink )
	{
		if( whichDTMap == pfJournalDlgProc::kTagRightDTMap || whichDTMap == pfJournalDlgProc::kTagTurnFrontDTMap )
			x += (UInt16)(dtMap->GetWidth());	// Right page rects are offsetted to differentiate

		if (dontRender) // if we aren't rendering then this link isn't visible, but the index still needs to be valid, so give it a rect of 0,0,0,0
			chunk->fLinkRect.Set(0,0,0,0);
		else
			chunk->fLinkRect.Set( x, y, (Int16)(copy->GetWidth()), (Int16)(copy->GetHeight()) );
		fVisibleLinks.Append( chunk );
	}
	delete copy;
}

pfJournalBook::loadedMovie *pfJournalBook::IMovieAlreadyLoaded(pfEsHTMLChunk *chunk)
{
	int i;
	for (i=0; i<fLoadedMovies.GetCount(); i++) // filename and id# must both match
	{
		if ((chunk->fText == fLoadedMovies[i]->movieChunk->fText)&&(chunk->fMovieIndex == fLoadedMovies[i]->movieChunk->fMovieIndex))
			return fLoadedMovies[i];
	}
	return nil;
}

plKey pfJournalBook::GetMovie(UInt8 index)
{
	loadedMovie *movie = IGetMovieByIndex(index);
	if (movie)
		return movie->movieLayer->GetKey();
	return plKey(nil);
}

pfJournalBook::loadedMovie *pfJournalBook::IGetMovieByIndex(UInt8 index)
{
	int i;
	for (i=0; i<fLoadedMovies.GetCount(); i++)
	{
		if (fLoadedMovies[i]->movieChunk->fMovieIndex == index)
			return fLoadedMovies[i];
	}
	return nil;
}

plLayerBink *pfJournalBook::IMakeMovieLayer(pfEsHTMLChunk *chunk, UInt16 x, UInt16 y, plMipmap *baseMipmap, UInt32 whichDTMap, hsBool dontRender)
{
	// see if it's already loaded
	loadedMovie *movie = IMovieAlreadyLoaded(chunk);
	plLayer* layer = nil;
	plLayerBink* movieLayer = nil;
	UInt16 movieWidth=0,movieHeight=0;
	if (movie)
	{
		movieLayer = movie->movieLayer;
		layer = plLayer::ConvertNoRef(movieLayer->BottomOfStack());
		movieWidth = movieLayer->GetWidth();
		movieHeight = movieLayer->GetHeight();
	}
	else
	{
		// Create the layer and register it.

		// We'll need a unique name. This is a hack, but an effective hack.
		static int uniqueSuffix = 0;
		char buff[256];

		sprintf(buff, "%s_%d_ml", GetKey()->GetName(), uniqueSuffix);
		layer = TRACKED_NEW plLayer;
		hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

		sprintf(buff, "%s_%d_m", GetKey()->GetName(), uniqueSuffix++);
		movieLayer = TRACKED_NEW plLayerBink;
		hsgResMgr::ResMgr()->NewKey(buff, movieLayer, GetKey()->GetUoid().GetLocation());
		movieLayer->GetKey()->RefObject(); // we want to own a ref so we can nuke it at will

		movieLayer->AttachViaNotify(layer);

		// Initialize it.
		char *name = hsWStringToString(chunk->fText.c_str());
		movieLayer->SetMovieName(name);
		delete [] name;
		movieLayer->Eval(0,0,0); // set up the movie

		movieWidth = movieLayer->GetWidth();
		movieHeight = movieLayer->GetHeight();

		if (movieHeight == 0 || movieWidth == 0) // problem loading the file
		{
			movieLayer->GetKey()->UnRefObject();
			return nil;
		}
	}

	if (layer)
	{
		layer->InitToDefault();

		layer->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.0f));
		layer->SetOpacity(1.0f);

		// Blend flags, movies are opaque, but they don't take the whole page, so alphamapping them
		layer->SetBlendFlags(hsGMatState::kBlendAlpha);

		// Movie shouldn't have to ZWrite
		layer->SetZFlags(hsGMatState::kZNoZWrite);

		// No special shading.
		layer->SetShadeFlags(0);

		// Clamp all textures.
		layer->SetClampFlags(hsGMatState::kClampTexture);

		// Draw passes individually.
		layer->SetMiscFlags(hsGMatState::kMiscRestartPassHere);

		// Shared UV coordinates.
		layer->SetUVWSrc(0);

		if( chunk->fFlags & pfEsHTMLChunk::kFloating )
		{
			// Floating movie, ignore the text flow completely and just splat the movie on!
			x = chunk->fAbsoluteX;
			y = chunk->fAbsoluteY;
		}

		if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kLeft )
			x = (UInt16)fPageLMargin;
		else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kRight )
			x = (UInt16)(baseMipmap->GetWidth() - fPageRMargin - movieWidth);
		else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kCenter )
			x = (UInt16)((baseMipmap->GetWidth() >> 1) - (movieWidth >> 1));

		// x and y are in pixels, need to convert to a range of 0 to 1
		float xRel = (float)x/(float)baseMipmap->GetWidth();
		float yRel = (float)y/(float)baseMipmap->GetHeight();

		// Need to convert the scaling to texture space
		float xScale = (float)baseMipmap->GetWidth()/(float)movieWidth;
		float yScale = (float)baseMipmap->GetHeight()/(float)movieHeight;

		if (chunk->fNoResizeImg)
		{
			// book is NOT square, there is a h/w ratio of 1/0.7
			// calc new size based on how the book has been skewed
			xScale *= (fWidthScale == 0) ? 1 : (1-fWidthScale);
			yScale *= (fHeightScale == 0) ? 1 : (1-fHeightScale);
			yScale *= 1/0.7; // adjust because the book isn't square
		}

		hsVector3 scaleVec(xScale,yScale,1), translateVec(-(xRel*xScale),-(yRel*yScale),0);
		hsMatrix44 scaleMat, translateMat;
		scaleMat.MakeScaleMat(&scaleVec);
		translateMat.MakeTranslateMat(&translateVec);

		hsMatrix44 flipMat;
		if (chunk->fOnCover) // cover movies need to be y flipped
		{
			hsVector3 yTransVec(0,-1,0), invertYVec(1,-1,1);
			hsMatrix44 invertY, transY;
			invertY.MakeScaleMat(&invertYVec);
			transY.MakeTranslateMat(&yTransVec);
			flipMat = invertY * transY;
		}
		else // left page movies need to be x flipped
		{
			if ((whichDTMap == pfJournalDlgProc::kTagLeftDTMap) || (whichDTMap == pfJournalDlgProc::kTagTurnBackDTMap))
			{
				hsVector3 xTransVec(-1,0,0), invertXVec(-1,1,1);
				hsMatrix44 invertX, transX;
				invertX.MakeScaleMat(&invertXVec);
				transX.MakeTranslateMat(&xTransVec);
				flipMat = invertX * transX;
			}
			else
				flipMat = hsMatrix44::IdentityMatrix();
		}
		
		hsMatrix44 xfm;
		xfm = translateMat * scaleMat * flipMat;

		layer->SetTransform(xfm);
	}
	if( chunk->fFlags & pfEsHTMLChunk::kCanLink )
	{
		if( whichDTMap == pfJournalDlgProc::kTagRightDTMap || whichDTMap == pfJournalDlgProc::kTagTurnFrontDTMap )
			x += (UInt16)(baseMipmap->GetWidth());	// Right page rects are offsetted to differentiate

		if (dontRender) // if we aren't rendering then this link isn't visible, but the index still needs to be valid, so give it a rect of 0,0,0,0
			chunk->fLinkRect.Set(0,0,0,0);
		else
			chunk->fLinkRect.Set( x, y, movieWidth, movieHeight );
		fVisibleLinks.Append( chunk );
	}

	plAnimTimeConvert &timeConvert = movieLayer->GetTimeConvert();
	timeConvert.SetBegin(0);
	timeConvert.SetEnd(movieLayer->GetLength());
	if (chunk->fLoopMovie)
	{
		timeConvert.SetLoopPoints(0,movieLayer->GetLength());
		timeConvert.Loop();
	}
	timeConvert.Start(); // start the show!

	return movieLayer;
}

plLayerInterface *pfJournalBook::IMakeBaseLayer(plMipmap *image)
{
	// Create the layer and register it.

	// We'll need a unique name. This is a hack, but an effective hack.
	static int uniqueSuffix = 0;
	char buff[256];
	sprintf(buff, "%s_%d", GetKey()->GetName(), uniqueSuffix++);

	plLayer* layer = TRACKED_NEW plLayer;
	hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

	// Initialize it.
	layer->InitToDefault();

	layer->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	if (fTintCover)
		layer->SetRuntimeColor(hsColorRGBA().Set(fCoverTint.r, fCoverTint.g, fCoverTint.b, 1.f));
	layer->SetOpacity(1.f);

	// Blend flags, opaque for the bottom layer.
	layer->SetBlendFlags(0);

	// Only the bottom layer writes it's Z value.
	layer->SetZFlags(0);

	// No special shading.
	layer->SetShadeFlags(hsGMatState::kShadeReallyNoFog);

	// Clamp all textures.
	layer->SetClampFlags(hsGMatState::kClampTexture);

	// Draw passes individually.
	layer->SetMiscFlags(hsGMatState::kMiscRestartPassHere);

	// Shared UV coordinates.
	layer->SetUVWSrc(0);

	// Set up the transform.
	hsVector3 yTransVec(0,-1,0), invertYVec(1,-1,1);
	hsMatrix44 xfm, invertY, transY, flipY;
	invertY.MakeScaleMat(&invertYVec);
	transY.MakeTranslateMat(&yTransVec);
	flipY = invertY * transY;
	xfm = flipY;

	layer->SetTransform(xfm);

	// Set the texture (assumes mipmap is non-nil).
	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(image->GetKey(), refMsg, plRefFlags::kActiveRef);

	return plLayerInterface::ConvertNoRef(layer);
}

plLayerInterface *pfJournalBook::IMakeDecalLayer(pfEsHTMLChunk *decalChunk, plMipmap *decal, plMipmap *baseMipmap)
{
	// Create the layer and register it.

	// We'll need a unique name. This is a hack, but an effective hack.
	static int uniqueSuffix = 0;
	char buff[256];
	sprintf(buff, "%s_%d_d", GetKey()->GetName(), uniqueSuffix++);

	plLayer* layer = TRACKED_NEW plLayer;
	hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

	// Initialize it.
	layer->InitToDefault();

	// tint the layer only if the decal wants to be tinted
	layer->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	if (decalChunk->fTintDecal)
		layer->SetRuntimeColor(hsColorRGBA().Set(fCoverTint.r, fCoverTint.g, fCoverTint.b, 1.f));
	layer->SetOpacity(1.f);

	// Blend flags, decals are alpha blended.
	layer->SetBlendFlags(hsGMatState::kBlendAlpha);

	// Only the bottom layer writes it's Z value.
	layer->SetZFlags(hsGMatState::kZNoZWrite);

	// No special shading.
	layer->SetShadeFlags(hsGMatState::kShadeReallyNoFog);

	// Clamp all textures.
	layer->SetClampFlags(hsGMatState::kClampTexture);

	// Draw passes individually.
	layer->SetMiscFlags(hsGMatState::kMiscRestartPassHere);

	// Shared UV coordinates.
	layer->SetUVWSrc(0);

	// Set up the transform.
	UInt16 x,y;
	x = decalChunk->fAbsoluteX;
	y = decalChunk->fAbsoluteY;

	if((decalChunk->fFlags & pfEsHTMLChunk::kAlignMask) == pfEsHTMLChunk::kLeft)
		x = (UInt16)fPageLMargin;
	else if((decalChunk->fFlags & pfEsHTMLChunk::kAlignMask) == pfEsHTMLChunk::kRight)
		x = (UInt16)(baseMipmap->GetWidth() - decal->GetWidth());
	else if((decalChunk->fFlags & pfEsHTMLChunk::kAlignMask) == pfEsHTMLChunk::kCenter)
		x = (UInt16)((baseMipmap->GetWidth() >> 1) - (decal->GetWidth() >> 1));

	// x and y are in pixels, need to convert to a range of 0 to 1
	float xRel = (float)x/(float)baseMipmap->GetWidth();
	float yRel = (float)y/(float)baseMipmap->GetHeight();

	// Need to convert the scaling to texture space
	float xScale = (float)baseMipmap->GetWidth()/(float)decal->GetWidth();
	float yScale = (float)baseMipmap->GetHeight()/(float)decal->GetHeight();

	if (decalChunk->fNoResizeImg)
	{
		// book is NOT square, there is a h/w ratio of 1/0.7
		// calc new size based on how the book has been skewed
		xScale *= (fWidthScale == 0) ? 1 : (1-fWidthScale);
		yScale *= (fHeightScale == 0) ? 1 : (1-fHeightScale);
		yScale *= 1/0.7; // adjust because the book isn't square
	}

	hsVector3 scaleVec(xScale,yScale,1), translateVec(-(xRel*xScale),-(yRel*yScale),0), yTransVec(0,-1,0), invertYVec(1,-1,1);
	hsMatrix44 scaleMat, translateMat, invertY, transY;
	scaleMat.MakeScaleMat(&scaleVec);
	translateMat.MakeTranslateMat(&translateVec);
	invertY.MakeScaleMat(&invertYVec);
	transY.MakeTranslateMat(&yTransVec);

	hsMatrix44 xfm, flipY;
	flipY = invertY * transY;
	xfm = translateMat * scaleMat * flipY;

	layer->SetTransform(xfm);

	// Set the texture (assumes mipmap is non-nil).
	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(decal->GetKey(), refMsg, plRefFlags::kActiveRef);

	return plLayerInterface::ConvertNoRef(layer);
}

void pfJournalBook::ISetDecalLayers(hsGMaterial *material,hsTArray<plLayerInterface*> layers)
{
	// First, clear out the existing layers.
	int i;
	for( i = material->GetNumLayers()-1; i >= 0; i-- )
	{
		plMatRefMsg* refMsg = TRACKED_NEW plMatRefMsg(material->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer);
		hsgResMgr::ResMgr()->SendRef(material->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
	}

	// Now append our new layers in order.
	for( i = 0; i < layers.GetCount(); i++ )
	{
		material->AddLayerViaNotify(layers[i]);
	}
}

//// IFindFontProps //////////////////////////////////////////////////////////
// Starting at the given chunk, works backwards to determine the full set of current
// font properties at that point, or assigns defaults if none were specified

void	pfJournalBook::IFindFontProps( UInt32 chunkIdx, const wchar_t *&face, UInt8 &size, UInt8 &flags, hsColorRGBA &color, Int16 &spacing )
{
	enum Which
	{
		kFace	= 0x01,
		kSize	= 0x02,
		kFlags	= 0x04,
		kColor	= 0x08,
		kSpacing= 0x10,

		kAllFound = kFace | kSize | kFlags | kColor | kSpacing
	};

	// Start empty
	UInt8	found = 0;

	// Work backwards and fill in our properties
	chunkIdx++;
	do
	{
		chunkIdx--;
		if (fHTMLSource.Count() <= chunkIdx)
			break; // apparently it's sometimes possible for fHTMLSource to be empty (parse errors?)
		pfEsHTMLChunk *chunk = fHTMLSource[ chunkIdx ];

		if( chunk->fType == pfEsHTMLChunk::kFontChange )
		{
			// What do we (still) need?
			if( !( found & kFace ) && chunk->fText != L"" )
			{
				face = chunk->fText.c_str();
				found |= kFace;
			}
			if( !( found & kSize ) && chunk->fFontSize > 0 )
			{
				size = chunk->fFontSize;
				found |= kSize;
			}
			if( !( found & kFlags ) && ( chunk->fFlags & pfEsHTMLChunk::kFontMask ) != 0 )
			{
				flags = 0;
				if( chunk->fFlags & pfEsHTMLChunk::kFontBold )
					flags |= plDynamicTextMap::kFontBold;
				if( chunk->fFlags & pfEsHTMLChunk::kFontItalic )
					flags |= plDynamicTextMap::kFontItalic;

				found |= kFlags;
			}
			if( !( found & kColor ) && ( chunk->fFlags & pfEsHTMLChunk::kFontColor ) )
			{
				color = chunk->fColor;
				found |= kColor;
			}
			if( !( found & kSpacing ) && ( chunk->fFlags & pfEsHTMLChunk::kFontSpacing ) )
			{
				spacing = chunk->fLineSpacing;
				found |= kSpacing;
			}
		}

	} while( chunkIdx != 0 && found != kAllFound );

	// Set any un-found defaults
	if( !( found & kFace ) )
		face = L"Arial";
	if( !( found & kSize ) )
		size = 24;
	if( !( found & kFlags ) )
		flags = 0;
	if( !( found & kColor ) )
		color.Set( 0.f, 0.f, 0.f, 1.f );
	if( !( found & kSpacing ) )
		spacing = 0;
}

//// IFindLastAlignment //////////////////////////////////////////////////////
// Find the last paragraph chunk and thus the last par alignment settings

UInt8	pfJournalBook::IFindLastAlignment( void ) const
{
	Int32 idx;


	for( idx = fHTMLSource.GetCount() - 1; idx >= 0; idx-- )
	{
		if( fHTMLSource[ idx ]->fType == pfEsHTMLChunk::kParagraph && fHTMLSource[ idx ]->fFlags != 0 )
			return (UInt8)(fHTMLSource[ idx ]->fFlags);
	}

	return pfEsHTMLChunk::kLeft;
}

//// IRecalcPageStarts ///////////////////////////////////////////////////////
// Ensures that all the page starts are calced up to the given page (but not including it)

void	pfJournalBook::IRecalcPageStarts( UInt32 upToPage )
{
	UInt32 page;


	// Well, sadly, we can't really calc the page starts without at least a DTMap
	// we can change things on...so we just pick one and render. Note: this WILL
	// trash the font settings on the given DTMap!

	// We assume that the stored page starts we already have are accurate, so
	// just start from there and calc onward

	for( page = fPageStarts.GetCount()-1; page < upToPage && page <= fLastPage; page++ )
	{
		// normally we would surpress rendering the pages, but that seems to have a bug in it
		// that causes lost text that the rendering doesn't have. Since it isn't very costly to
		// actually draw them all (even in large journals), we're just going to do it
		IRenderPage( page, pfJournalDlgProc::kTagTurnBackDTMap, false );
		// Reset any "visible" links since they aren't really visible
		UInt16 i;
		for (i=0; i<fVisibleLinks.Count(); i++)
		{
			fVisibleLinks[i]->fLinkRect.Set(0,0,0,0);
		}
	}
}

//// ISendNotify /////////////////////////////////////////////////////////////
// Just sends out a notify to our currently set receiver key

void	pfJournalBook::ISendNotify( UInt32 type, UInt32 linkID )
{
	if( fCallbackKey != nil )
	{
		plNotifyMsg *pMsg = TRACKED_NEW plNotifyMsg;
		pMsg->AddBookEvent( type, linkID );
		pMsg->SetBCastFlag( plMessage::kNetPropagate, false ); // don't deliver networked!
		pMsg->Send( fCallbackKey );
	}
}

//// SetBookSize /////////////////////////////////////////////////////////////
// Sets the book size scaling. 1,1 would be full size, 0,0 is the smallest size possible
// Note: internally we store these as the seek positions on our animations,
// so the incoming parameters are actually inverse of what we finally want

void	pfJournalBook::SetBookSize( hsScalar width, hsScalar height )
{
	fWidthScale = 1.f - width;
	fHeightScale = 1.f - height;

	if( fBookGUIs[fCurBookGUI]->CurBook() == this )
		fBookGUIs[fCurBookGUI]->SetCurrSize( fWidthScale, fHeightScale );
}

//// ILoadAllImages //////////////////////////////////////////////////////////
// Load (or unload) all the images for the book

void	pfJournalBook::ILoadAllImages( hsBool unload )
{
	UInt32	i;

	// load the cover
	if( fCoverFromHTML && fCoverMipKey != nil )
	{
		if( unload )
			fBookGUIs[fCurBookGUI]->GetKey()->Release( fCoverMipKey );
		else
		{
			plGenRefMsg *ref = TRACKED_NEW plGenRefMsg( fBookGUIs[fCurBookGUI]->GetKey(), plRefMsg::kOnCreate, -1, kRefImage );
			hsgResMgr::ResMgr()->AddViaNotify( fCoverMipKey, ref, plRefFlags::kActiveRef );
		}
	}

	for( i = 0; i < fHTMLSource.GetCount(); i++ )
	{
		if( fHTMLSource[ i ]->fType == pfEsHTMLChunk::kImage && fHTMLSource[ i ]->fImageKey != nil )
		{
			if( unload )
				fBookGUIs[fCurBookGUI]->GetKey()->Release( fHTMLSource[ i ]->fImageKey );
			else
			{
				plGenRefMsg *ref = TRACKED_NEW plGenRefMsg( fBookGUIs[fCurBookGUI]->GetKey(), plRefMsg::kOnCreate, -1, kRefImage );  
				hsgResMgr::ResMgr()->AddViaNotify( fHTMLSource[ i ]->fImageKey, ref, plRefFlags::kActiveRef );
			}
		}
	}
}

void pfJournalBook::IPurgeDynaTextMaps( )
{
	plDynamicTextMap* turnFront = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagTurnFrontDTMap );
	if (turnFront)
		turnFront->PurgeImage();

	plDynamicTextMap* right = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagRightDTMap );
	if (right)
		right->PurgeImage();

	plDynamicTextMap* turnBack = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagTurnBackDTMap );
	if (turnBack)
		turnBack->PurgeImage();

	plDynamicTextMap* left = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagLeftDTMap );
	if (left)
		left->PurgeImage();

	pfGUIMultiLineEditCtrl *leftEdit = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagLeftEditCtrl );
	if (leftEdit)
		leftEdit->PurgeDynaTextMapImage();

	pfGUIMultiLineEditCtrl *rightEdit = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagRightEditCtrl );
	if (rightEdit)
		rightEdit->PurgeDynaTextMapImage();

	pfGUIMultiLineEditCtrl *turnFrontEdit = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagTurnFrontEditCtrl );
	if (turnFrontEdit)
		turnFrontEdit->PurgeDynaTextMapImage();

	pfGUIMultiLineEditCtrl *turnBackEdit = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagTurnBackEditCtrl );
	if (turnBackEdit)
		turnBackEdit->PurgeDynaTextMapImage();
}

std::string pfJournalBook::GetEditableText()
{
	pfGUIMultiLineEditCtrl *left = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagLeftEditCtrl );
	if (left)
	{
		char *temp = left->GetNonCodedBuffer();
		std::string retVal = temp;
		delete [] temp;
		return retVal;
	}
	return "";
}

void pfJournalBook::SetEditableText(std::string text)
{
	pfGUIMultiLineEditCtrl *left = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagLeftEditCtrl );
	if (left)
	{
		left->SetBuffer(text.c_str());
		left->ForceUpdate();
	}
}
