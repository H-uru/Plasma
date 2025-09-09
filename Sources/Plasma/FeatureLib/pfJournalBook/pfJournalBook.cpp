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
//  pfJournalBook Class                                                     //
//  A generic, high-level, abstract method of creating various Myst-like    //
//  books within the game with very little effort, while ensuring that they //
//  all remain consistent in appearance and operability.                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfJournalBook.h"

#include <cwchar>

#include "HeadSpin.h"
#include "hsGDeviceRef.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "pcSmallRect.h"
#include "hsTimer.h"
#include "plTimerCallbackManager.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plAgeLoader/plAgeLoader.h"
#include "plGImage/plDynamicTextMap.h"
#include "plGImage/plFont.h"
#include "plGImage/plJPEG.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plPNG.h"
#include "plInputCore/plInputInterface.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plLayRefMsg.h"
#include "plMessage/plMatRefMsg.h"
#include "plMessage/plTimerCallbackMsg.h"
#include "plResMgr/plKeyFinder.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"
#include "plSurface/plLayerInterface.h"

#include "pfGameGUIMgr/pfGUIButtonMod.h"
#include "pfGameGUIMgr/pfGUICheckBoxCtrl.h"
#include "pfGameGUIMgr/pfGUIClickMapCtrl.h"
#include "pfGameGUIMgr/pfGUIControlMod.h"
#include "pfGameGUIMgr/pfGUIDialogHandlers.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"
#include "pfGameGUIMgr/pfGUIDynDisplayCtrl.h"
#include "pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"
#include "pfGameGUIMgr/pfGUIProgressCtrl.h"
#include "pfMessage/pfGUINotifyMsg.h"
#include "pfSurface/plLayerAVI.h"

//////////////////////////////////////////////////////////////////////////////
//// Do we show linking rects? ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static bool s_ShowLinkRects = false;
static hsColorRGBA s_LinkRectColor{ 0.f, 0.f, 1.f, 1.f };

//////////////////////////////////////////////////////////////////////////////
//// pfEsHTMLChunk Class /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class pfEsHTMLChunk
{
    public:
        struct TextLink_Type {};
        static constexpr TextLink_Type TextLink{};

        enum Type : uint8_t
        {
            kEmpty = 0,
            kParagraph,
            kImage,
            kPageBreak,
            kFontChange,
            kMargin,
            kCover, // Just a placeholder, never actually used after compile time
            kBook,  // another placeholder
            kDecal,
            kMovie,
            kEditable, // placeholder, ver 3.0
            kTextLink,
        };

        ST::string fText; // Paragraph text, or face name
        plKey   fImageKey;  // Key of image
        uint8_t   fFontSize;
        uint32_t  fFlags;
        Type   fType;
        uint32_t  fEventID;

        hsColorRGBA fColor;

        uint16_t      fAbsoluteX, fAbsoluteY;

        float    fCurrOpacity;   // For SFX images
        float    fSFXTime;       // For SFX images
        float    fMinOpacity, fMaxOpacity;   

        hsColorRGBA fCurrColor;
        hsColorRGBA fOffColor, fOnColor;

        bool    fNoResizeImg;
        int16_t   fLineSpacing;
        bool    fTintDecal;
        bool    fLoopMovie;
        bool    fOnCover; // if true, the movie is on the cover
        uint8_t   fMovieIndex; // the index of the movie in the source code, used for identification

        enum Flags
        {
            kFontBold   = 0x00000001,
            kFontItalic = 0x00000002,
            kFontRegular= 0x00000004,       // 'cause 0 means "style not defined"
            kFontMask   = kFontBold | kFontItalic | kFontRegular,

            kFontColor  = 0x00000008,
            kFontSpacing= 0x00000010,

            kCenter     = 0x00000001,
            kLeft       = 0x00000002,
            kRight      = 0x00000003,
            kAlignMask  = 0x00000003,

            kBlendAlpha = 0x00000004,
            kCanLink    = 0x00000008,
            kFloating   = 0x00000010,
            kGlowing    = 0x00000020,
            kActAsCB    = 0x00000040,   // Cause the image to act in a checkbox-like fashion. 
                                        // Min opacity turns into "off opacity" and max opacity
                                        // is "on opacity"
            kChecked    = 0x00000080,   // Only for kActAsCB, set if it's currently "checked"
            kTranslucent= 0x00000100,   // is the image translucent? if so, use fCurrOpacity
            kValidEvent = 0x00000200,
        };

        // Paragraph constructor
        pfEsHTMLChunk( ST::string text )
            : fType(kParagraph), fFlags(kLeft), fText(std::move(text)),
              fFontSize(), fImageKey(), fEventID(), fSFXTime(),
              fAbsoluteX(), fAbsoluteY(), fNoResizeImg(), fLineSpacing(),
              fCurrOpacity(1.f), fMinOpacity(), fMaxOpacity(1.f),
              fTintDecal(), fLoopMovie(true), fOnCover(), fMovieIndex(-1)

        {
            fColor.Set(0.f, 0.f, 0.f, 1.f);
            fCurrColor.Set(0.f, 0.f, 0.f, 1.f);
            fOffColor.Set(0.f, 0.f, 0.f, 1.f);
            fOnColor.Set(0.f, 0.f, 0.f, 1.f);
        }

        // Image constructor (used for decals and movies too)
        pfEsHTMLChunk( plKey imageKey, uint32_t alignFlags )
            : fType(kImage), fFlags(alignFlags), fText(),
              fFontSize(), fImageKey(std::move(imageKey)), fEventID(), fSFXTime(),
              fAbsoluteX(), fAbsoluteY(), fNoResizeImg(), fLineSpacing(),
              fCurrOpacity(1.f), fMinOpacity(), fMaxOpacity(1.f),
              fTintDecal(), fLoopMovie(true), fOnCover(), fMovieIndex(-1)
        {
            fColor.Set(0.f, 0.f, 0.f, 1.f);
            fCurrColor.Set(0.f, 0.f, 0.f, 1.f);
            fOffColor.Set(0.f, 0.f, 0.f, 1.f);
            fOnColor.Set(0.f, 0.f, 0.f, 1.f);
        }

        // Page break constructor
        pfEsHTMLChunk()
            : fType(kPageBreak), fFlags(), fText(),
              fFontSize(), fImageKey(), fEventID(), fSFXTime(),
              fAbsoluteX(), fAbsoluteY(), fNoResizeImg(), fLineSpacing(),
              fCurrOpacity(1.f), fMinOpacity(), fMaxOpacity(1.f),
              fTintDecal(), fLoopMovie(true), fOnCover(), fMovieIndex(-1)
        {
            fColor.Set(0.f, 0.f, 0.f, 1.f);
            fCurrColor.Set(0.f, 0.f, 0.f, 1.f);
            fOffColor.Set(0.f, 0.f, 0.f, 1.f);
            fOnColor.Set(0.f, 0.f, 0.f, 1.f);
        }

        // Font change constructor
        pfEsHTMLChunk( ST::string face, uint8_t size, uint32_t fontFlags )
            : fType(kFontChange), fFlags(fontFlags), fText(std::move(face)),
              fFontSize(size), fImageKey(), fEventID(), fSFXTime(),
              fAbsoluteX(), fAbsoluteY(), fNoResizeImg(), fLineSpacing(),
              fCurrOpacity(1.f), fMinOpacity(), fMaxOpacity(1.f),
              fTintDecal(), fLoopMovie(true), fOnCover(), fMovieIndex(-1)
        {
            fColor.Set(0.f, 0.f, 0.f, 1.f);
            fCurrColor.Set(0.f, 0.f, 0.f, 1.f);
            fOffColor.Set(0.f, 0.f, 0.f, 1.f);
            fOnColor.Set(0.f, 0.f, 0.f, 1.f);
        }

        // Text link constructor
        pfEsHTMLChunk(TextLink_Type)
            : fType(kTextLink), fFlags(), fText(),
              fFontSize(), fImageKey(), fEventID(), fSFXTime(),
              fAbsoluteX(), fAbsoluteY(), fNoResizeImg(), fLineSpacing(),
              fCurrOpacity(1.f), fMinOpacity(), fMaxOpacity(1.f),
              fTintDecal(), fLoopMovie(true), fOnCover(), fMovieIndex(-1)
        {
            fColor.Set(0.f, 0.f, 1.f, 1.f);
            fCurrColor.Set(0.f, 0.f, 0.f, 1.f);
            fOffColor.Set(0.f, 0.f, 0.f, 1.f);
            fOnColor.Set(0.f, 0.f, 0.f, 1.f);
        }

        ~pfEsHTMLChunk() { }
};

//////////////////////////////////////////////////////////////////////////////
//// Visible Link Stuff //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class pfJournalVisibleLink
{
    pfEsHTMLChunk* fChunk;
    pcSmallRect    fLinkRect;

public:
    pfJournalVisibleLink(
        pfEsHTMLChunk* chunk,
        int16_t x, int16_t y,
        int16_t width, int16_t height
    ) : fChunk(chunk), fLinkRect(x, y, width, height) {}

    bool Contains(int16_t x, int16_t y) const
    {
        return fLinkRect.Contains(x, y);
    }

    void ClearRect()
    {
        fLinkRect.Set(0, 0, 0, 0);
    }

    bool HasEventID() const
    {
        return hsCheckBits(fChunk->fFlags, pfEsHTMLChunk::kValidEvent);
    }

    uint32_t GetEventID() const
    {
        hsAssert(HasEventID(), "Trying to get the EventID of a non-event, eh?");
        return fChunk->fEventID;
    }

    const ST::string& GetHRef() const
    {
        return fChunk->fText;
    }

    bool IsCheckbox() const
    {
        return hsCheckBits(fChunk->fFlags, pfEsHTMLChunk::kActAsCB);
    }

    bool IsChecked() const
    {
        hsAssert(IsCheckbox(), "Trying to see if a non-checkbox is checked?");
        return hsCheckBits(fChunk->fFlags, pfEsHTMLChunk::kChecked);
    }

    void SetChecked(bool value)
    {
        hsAssert(IsCheckbox(), "Trying to check a non-checkbox?");
        hsChangeBits(fChunk->fFlags, pfEsHTMLChunk::kChecked, value);
    }
};

//////////////////////////////////////////////////////////////////////////////
//// Our Template Dialog Handler /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class pfJournalDlgProc : public pfGUIDialogProc
{
    protected:

        pfBookData  *fBook;

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

        void    DoSomething(pfGUIControlMod *ctrl) override
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
        void    OnInit() override
        {
        }

        // Called before the dialog is shown, always after OnInit()
        void    OnShow() override
        {
        }

        // Called before the dialog is hidden
        void    OnHide() override
        {
        }

        // Called on the dialog's destructor, before it's unregistered with the game GUI manager
        void    OnDestroy() override
        {
        }

        // Called when the dialog's focused control changes
        void    OnCtrlFocusChange(pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl) override
        {
        }

        // Called when the key bound to a GUI event is pressed. Only called on the top modal dialog
        void    OnControlEvent(ControlEvt event) override
        {
            if( event == kExitMode )
            {
                if( fBook->fCurrentlyOpen )
                    fBook->fCurrBook->CloseAndHide();
                else
                    fBook->fCurrBook->Hide();
            }
        }

        void    HandleExtendedEvent(pfGUIControlMod *ctrl, uint32_t event) override
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
                            if (fBook->fCurrBook->IFindCurrVisibleLink(false, true) != -1)
                                fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorPoised/*Hand*/ );
                            else if(( fBook->fCurrBook->fCurrentPage > 1 )&&( fBook->fCurrBook->fAllowTurning ))
                                fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorLeft );
                            else if ((fBook->fCurrBook->fAreEditing) && !(fBook->fLeftEditCtrl->ShowingBeginningOfBuffer())) // if we have more buffer to show
                                fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorLeft );
                            else
                                fBook->fLeftPageMap->SetCustomCursor( plInputInterface::kCursorUp );
                        }
                    }
                    else if (event == pfGUIClickMapCtrl::kMouseDragged)
                    {
                        if (fBook->fCurrBook->IFindCurrVisibleLink(false, true) != -1)
                            fBook->fLeftPageMap->SetCustomCursor(plInputInterface::kCursorClicked);
                        else
                            fBook->fLeftPageMap->SetCustomCursor(plInputInterface::kCursorUp);
                    }
                }
                else if( ctrl == fBook->fRightPageMap )
                {
                    if( event == pfGUIClickMapCtrl::kMouseHovered )
                    {
                        if (fBook->fCurrBook)
                        {
                            // Update our custom cursor on the map
                            if (fBook->fCurrBook->IFindCurrVisibleLink(true, true) != -1)
                                fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorPoised/*Hand*/ );
                            else if((fBook->fCurrBook->fAreWeShowing) && ( fBook->fCurrBook->fCurrentPage + 2 <= fBook->fCurrBook->fLastPage )&&( fBook->fCurrBook->fAllowTurning ))
                                fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorRight );
                            else if((fBook->fCurrBook->fAreEditing) && !(fBook->fRightEditCtrl->ShowingEndOfBuffer())) // if we have more buffer to show
                                fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorRight );
                            else
                                fBook->fRightPageMap->SetCustomCursor( plInputInterface::kCursorUp );
                        }
                    }
                    else if (event == pfGUIClickMapCtrl::kMouseDragged)
                    {
                        if (fBook->fCurrBook->IFindCurrVisibleLink(true, true) != -1)
                            fBook->fRightPageMap->SetCustomCursor(plInputInterface::kCursorClicked);
                        else
                            fBook->fRightPageMap->SetCustomCursor(plInputInterface::kCursorUp);
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

    void OnEndOfControlList(int32_t cursorPos) override { bookData->HitEndOfControlList(cursorPos); }
    void OnBeginningOfControlList(int32_t cursorPos) override { bookData->HitBeginningOfControlList(cursorPos); }
};

//// Book data class /////////////////////////////////////////////////////////

void pfBookData::LoadGUI()
{
    // has the dialog been loaded yet?
    if (!pfGameGUIMgr::GetInstance()->IsDialogLoaded(fGUIName))
        // no then load and set handler
        pfGameGUIMgr::GetInstance()->LoadDialog(fGUIName, GetKey());
    else
        // yes then just set the handler
        pfGameGUIMgr::GetInstance()->SetDialogToNotify(fGUIName, GetKey());
}

bool pfBookData::MsgReceive(plMessage *pMsg)
{
    plGenRefMsg *ref = plGenRefMsg::ConvertNoRef(pMsg);
    if (ref != nullptr)
    {
        if(ref->fType == kRefDialog)
        {
            if(ref->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
            {
                pfGUIDialogMod *temp = pfGUIDialogMod::ConvertNoRef(ref->GetRef());
                if (temp != nullptr) // sanity check
                    fDialog = temp;
            }
            /*else
            {
                fDialog = nullptr;
                fCoverButton = nullptr;
            }*/
            return true;
        }
        else if(ref->fType == kRefDefaultCover)
        {
            if(ref->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
                fDefaultCover = plMipmap::ConvertNoRef(ref->GetRef());
            else
                fDefaultCover = nullptr;
            return true;
        }
    }

    plEventCallbackMsg *callback = plEventCallbackMsg::ConvertNoRef( pMsg );
    if (callback != nullptr)
    {
        // Our callback message to tell us the page is done flipping
        if( callback->fUser & 0x08 )
        {
            // make sure that we still have a current book
            if (fCurrBook)
            {
                // Or actually maybe it's that we're done closing and should hide now,
                // produced from a CloseAndHide()
                if( callback->fEvent == plEventCallbackMsg::kStop )
                    fCurrBook->Hide();
                else
                    fCurrBook->IFinishShow( ( callback->fUser & 0x01 ) ? true : false );
            }
        }
        else if( fCurrentlyTurning )
        {
            if( callback->fUser & 0x04 )
                IFillUncoveringPage( (bool)( callback->fUser & 0x01 ) ? true : false ); 
            else if( callback->fUser & 0x02 )
                StartTriggeredFlip( (bool)( callback->fUser & 0x01 ) ? true : false );
            else
                IFinishTriggeredFlip( (bool)( callback->fUser & 0x01 ) ? true : false );
        }
        return true;
    }

    pfGUINotifyMsg *notify = pfGUINotifyMsg::ConvertNoRef(pMsg);
    if (notify != nullptr)
    {
        // The only time we should get this is when the dialog loads; after that, we hijack
        // the dialog proc with our own
        IInitTemplate(pfGUIDialogMod::ConvertNoRef(notify->GetSender()->ObjectIsLoaded()));
        return true;
    }

    plTimeMsg *time = plTimeMsg::ConvertNoRef( pMsg );
    if (time != nullptr && fCurrSFXPages != kNoSides && !fCurrentlyTurning && fCurrentlyOpen)
    {
        IHandleSFX( (float)time->DSeconds() );
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
    hsAssert(templateDlg != nullptr, "Nil template in pfBookData::IInitTemplate()!");

    // Init and ref our fDialog pointer
    hsgResMgr::ResMgr()->SendRef(templateDlg->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefDialog), plRefFlags::kPassiveRef);  

    // Hijack the dialog proc with our own
    templateDlg->SetHandlerForAll(new pfJournalDlgProc(this));

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
    fCoverButton->DontPlaySounds(); // dont let checkbox play sounds, journal will take care of that.

    fCoverLayer = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagCoverLayer))->GetLayer(0);
    fCoverMaterial = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagCoverLayer))->GetMaterial(0);

    fPageMaterials[kLeftPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagLeftDTMap))->GetMaterial(0);
    fPageMaterials[kRightPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagRightDTMap))->GetMaterial(0);
    fPageMaterials[kTurnFrontPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnFrontDTMap))->GetMaterial(0);
    fPageMaterials[kTurnBackPage] = pfGUIDynDisplayCtrl::ConvertNoRef(templateDlg->GetControlFromTag(pfJournalDlgProc::kTagTurnBackDTMap))->GetMaterial(0);

    // Grab and ref the default cover mipmap
    plLayer *lay = plLayer::ConvertNoRef(fCoverLayer);
    if ((lay != nullptr) && (lay->GetTexture() != nullptr))
        hsgResMgr::ResMgr()->AddViaNotify(lay->GetTexture()->GetKey(), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefDefaultCover), plRefFlags::kPassiveRef);  

    fLeftPageMap->SetFlag(pfGUIClickMapCtrl::kReportDragging);
    fLeftPageMap->SetFlag(pfGUIClickMapCtrl::kReportHovering);
    fRightPageMap->SetFlag(pfGUIClickMapCtrl::kReportDragging);
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
        fLeftEditCtrl->SetEventProc(new pfBookMultiLineEditProc(this));
        fTurnFrontEditCtrl->SetNext(fTurnBackEditCtrl); // these are always back to back
    }
}

//// IGetDTMap ///////////////////////////////////////////////////////////////
// Just a quick helper 

plDynamicTextMap *pfBookData::GetDTMap(uint32_t which)
{
    pfGUIDynDisplayCtrl *display = pfGUIDynDisplayCtrl::ConvertNoRef(fDialog->GetControlFromTag(which));
    return display->GetMap(0);
}

//// GetEditCtrl /////////////////////////////////////////////////////////////

pfGUIMultiLineEditCtrl *pfBookData::GetEditCtrl(uint32_t which)
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
        return nullptr;
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

void pfBookData::IHandleSFX(float currTime, WhichSide whichSide /*= kNoSides*/)
{
    if (fCurrBook == nullptr)
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
    float deltaT = currTime - fBaseSFXTime;

    uint32_t idx, inc = (whichSide == kLeftSide) ? 0 : 1;
    if (fCurrBook->fPageStarts.size() <= fCurrBook->fCurrentPage + inc + 1)
        return;

    bool stillWant = false;
    for(idx = fCurrBook->fPageStarts[fCurrBook->fCurrentPage + inc]; idx < fCurrBook->fPageStarts[fCurrBook->fCurrentPage + inc + 1]; idx++)
    {
        pfEsHTMLChunk *chunk = fCurrBook->fHTMLSource[idx];
        
        if(chunk->fFlags & pfEsHTMLChunk::kGlowing)
        {
            // Glow SFX: animate opacity based on time offset
            uint8_t isOdd = 0;
            float newDelta = deltaT;
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
            hsColorRGBA inc;
            inc.Set(0.1f, 0.1f, 0.1f, 0.1f);

            hsColorRGBA &want = (chunk->fFlags & pfEsHTMLChunk::kChecked) ? chunk->fOnColor : chunk->fOffColor;
            if(want != chunk->fCurrColor)
            {
#define COMPARE_ME( wnt, curr ) \
                if( wnt > curr + 0.1f )                 \
                {                                       \
                    curr += 0.1f;   stillWant = true;   \
                }                                       \
                else if( wnt < curr - 0.1f )            \
                {                                       \
                    curr -= 0.1f;   stillWant = true;   \
                }                                       \
                else                                    \
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

void pfBookData::IFillUncoveringPage(bool rightSide)
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
            plTimerCallbackMsg* pTimerMsg = new plTimerCallbackMsg(GetKey(),id);
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

void pfBookData::ITriggerPageFlip(bool flipBackwards, bool immediate)
{
    // Hack here: since we don't have an official interface to select these directly
    // in MAX, we just use a GUI check box to grab them for us, even though we never
    // actually use the functionality of the checkbox itself
    const std::vector<plKey> &keys = fTurnPageButton->GetAnimationKeys();
    ST::string animName = fTurnPageButton->GetAnimationName();

    plAnimCmdMsg *msg = new plAnimCmdMsg();
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
    plEventCallbackMsg *eventMsg = new plEventCallbackMsg;
    eventMsg->AddReceiver(GetKey());
    eventMsg->fRepeats = 0;
    if (immediate)
    {
        eventMsg->fUser = ((!flipBackwards) ? 0x01 : 0x00) | 0x02;
        eventMsg->fEvent = plEventCallbackMsg::kSingleFrameAdjust;
    }
    else
    {
        eventMsg->fUser = (flipBackwards ? 0x01 : 0x00);
        eventMsg->fEvent = plEventCallbackMsg::kStop;
    }
    msg->SetCmd(plAnimCmdMsg::kAddCallbacks);
    msg->AddCallback(eventMsg);
    hsRefCnt_SafeUnRef(eventMsg);
    if (!immediate)
    {
        // We want a second callback to tell us when, indeed, the page has started turning
        // and is thus visible and thus we can actually, really, honestly, safely fill in the
        // page behind it
        eventMsg = new plEventCallbackMsg;
        eventMsg->AddReceiver(GetKey());
        eventMsg->fRepeats = 0;
        eventMsg->fUser = !flipBackwards ? (0x04 | 0x01) : 0x04;
        eventMsg->fEvent = plEventCallbackMsg::kBegin;  // Should cause it to be triggered once it seeks at the start of the command
        msg->AddCallback(eventMsg);
        hsRefCnt_SafeUnRef(eventMsg);
    }
    fCurrentlyTurning = true;

    msg->Send();
}

//// StartTriggeredFlip /////////////////////////////////////////////////////
// Finishes the start of the triggered page flip (once we're sure the 
// animation is at the new frame)

void pfBookData::StartTriggeredFlip(bool flipBackwards)
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

void pfBookData::IFinishTriggeredFlip(bool wasBackwards)
{
    if (fCurrBook && fCurrBook->fAreEditing) // this is handled differently when we are editing
    {
        fLeftEditCtrl->SetNext(fRightEditCtrl); // relink the original path
        if (!wasBackwards)
        {
            // adjust the starting point of the control (not needed if we weren't backwards since that was done when we started turning
            int32_t newStart = fRightEditCtrl->GetLastVisibleLine();
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
//      right->Swap( turnFront );
        if ( turnFront->IsValid() && right->IsValid() )
        {
            memcpy(right->GetImage(), turnFront->GetImage(), right->GetLevelSize(0));
            if (right->GetDeviceRef() != nullptr)
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
    //      right->Swap( turnFront );
        if ( turnBack->IsValid() && left->IsValid() )
        {
            memcpy(left->GetImage(), turnBack->GetImage(), left->GetLevelSize(0));
            if (left->GetDeviceRef() != nullptr)
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

void pfBookData::SetCurrSize(float w, float h)
{
    fWidthCtrl->SetCurrValue(w);
    fHeightCtrl->SetCurrValue(h);
}

//// PlayBookCloseAnim //////////////////////////////////////////////////////
//  Triggers our animation for closing or opening the book.

void pfBookData::PlayBookCloseAnim(bool closeIt /*= true*/, bool immediate /*= false*/)
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

void pfBookData::HitEndOfControlList(int32_t cursorPos)
{
    fAdjustCursorTo = cursorPos;
    if (fCurrBook)
        fCurrBook->NextPage();
}

void pfBookData::HitBeginningOfControlList(int32_t cursorPos)
{
    fAdjustCursorTo = cursorPos;
    if (fCurrBook)
        fCurrBook->PreviousPage();
}

void pfBookData::EnableEditGUI(bool enable/* =true */)
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

//pfJournalBook *pfJournalBook::fInstance = nullptr;
std::map<ST::string,pfBookData*> pfJournalBook::fBookGUIs;

void    pfJournalBook::SingletonInit()
{
    fBookGUIs["BkBook"] = new pfBookData(); // load the default book data object
    hsgResMgr::ResMgr()->NewKey("BkBook",fBookGUIs["BkBook"],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
    fBookGUIs["BkBook"]->LoadGUI();
}

void    pfJournalBook::SingletonShutdown()
{
    std::map<ST::string,pfBookData*>::iterator i = fBookGUIs.begin();
    while (i != fBookGUIs.end())
    {
        pfBookData *bookData = i->second;
        bookData->GetKey()->UnRefObject();
        i->second = nullptr;
        i++;
    }
    fBookGUIs.clear();
}

void    pfJournalBook::LoadGUI( const ST::string &guiName )
{
    if (fBookGUIs.find(guiName) == fBookGUIs.end()) // is it already loaded?
    { // nope, load it
        fBookGUIs[guiName] = new pfBookData(guiName);
        hsgResMgr::ResMgr()->NewKey(guiName,fBookGUIs[guiName],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
        fBookGUIs[guiName]->LoadGUI();
    }
}

void    pfJournalBook::UnloadGUI( const ST::string &guiName )
{
    if (guiName.compare("BkBook")==0)
        return; // do not allow people to unload the default book gui
    auto loc = fBookGUIs.find(guiName);
    if (loc != fBookGUIs.end()) // make sure it's loaded
    {
        fBookGUIs[guiName]->GetKey()->UnRefObject();
        fBookGUIs[guiName] = nullptr;
        fBookGUIs.erase(loc);
    }
}

void    pfJournalBook::UnloadAllGUIs()
{
    std::vector<ST::string> names;
    for (const auto& [name, data] : fBookGUIs)
        names.emplace_back(name); // store a list of keys
    for (const ST::string& name : names)
        UnloadGUI(name); // UnloadGUI won't unload BkBook
}

void     pfJournalBook::ShowLinkRect(bool on)
{
    s_ShowLinkRects = on;
}

//// Constructor /////////////////////////////////////////////////////////////
// The constructor takes in the esHTML source for the journal, along with
// the name of the mipmap to use as the cover of the book. The callback
// key is the keyed object to send event messages to (see <img> tag).

pfJournalBook::pfJournalBook(ST::string esHTMLSource, plKey coverImageKey, plKey callbackKey /*= {}*/,
                             const plLocation &hintLoc /* = plLocation::kGlobalFixedLoc */, const ST::string &guiName /* = {} */)
{
    if (!guiName.empty())
        fCurBookGUI = guiName;
    else
        fCurBookGUI = "BkBook";
    if (fBookGUIs.find(fCurBookGUI) == fBookGUIs.end())
    {
        fBookGUIs[fCurBookGUI] = new pfBookData(fCurBookGUI);
        hsgResMgr::ResMgr()->NewKey(fCurBookGUI,fBookGUIs[fCurBookGUI],pfGameGUIMgr::GetInstance()->GetKey()->GetUoid().GetLocation());
        fBookGUIs[fCurBookGUI]->LoadGUI();
    }
    
    fCurrentPage = 0;
    fLastPage = -1;
    fCoverMipKey = std::move(coverImageKey);
    fCoverFromHTML = false;
    fCallbackKey = std::move(callbackKey);
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
    fUncompiledSource = std::move(esHTMLSource);

    ICompileSource( fUncompiledSource, hintLoc );
}

pfJournalBook::~pfJournalBook()
{
    if (fBookGUIs.find(fCurBookGUI) != fBookGUIs.end()) // it might have been deleted before we got here
        if( fBookGUIs[fCurBookGUI] && fBookGUIs[fCurBookGUI]->CurBook() == this )
            Hide();

    IFreeSource();
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfJournalBook::MsgReceive( plMessage *pMsg )
{
    return hsKeyedObject::MsgReceive( pMsg );
}

void    pfJournalBook::SetGUI( const ST::string &guiName )
{
    if (!guiName.empty())
        fCurBookGUI = guiName;
    if (fBookGUIs.find(fCurBookGUI) == fBookGUIs.end())
        fCurBookGUI = "BkBook"; // requested GUI isn't loaded, so use default GUI
    SetEditable(fWantEditing); // make sure that if we want editing, to set it
    ICompileSource(fUncompiledSource, fDefLoc); // recompile the source to be safe
}

//// Show ////////////////////////////////////////////////////////////////////
// Shows the book, optionally starting open or closed

void    pfJournalBook::Show( bool startOpened /*= false */)
{
    fBookGUIs[fCurBookGUI]->StartedOpen(startOpened);
    fBookGUIs[fCurBookGUI]->CurBook(this);
    fBookGUIs[fCurBookGUI]->SetCurrSize(fWidthScale, fHeightScale);
    ILoadAllImages( false );

    hsGMaterial *cover = fBookGUIs[fCurBookGUI]->CoverMaterial();
    if (cover != nullptr)
    {
        std::vector<plLayerInterface*> layers;
        plMipmap *mip = fCoverMipKey ? plMipmap::ConvertNoRef(fCoverMipKey->ObjectIsLoaded()) : nullptr;
        if (mip != nullptr)
        {
            layers.emplace_back(IMakeBaseLayer(mip));

            for (pfEsHTMLChunk* decalChunk : fCoverDecals)
            {
                if (decalChunk->fType == pfEsHTMLChunk::kDecal)
                {
                    plMipmap *decal = plMipmap::ConvertNoRef(decalChunk->fImageKey != nullptr ? decalChunk->fImageKey->ObjectIsLoaded() : nullptr);
                    if (decal != nullptr)
                        layers.emplace_back(IMakeDecalLayer(decalChunk, decal, mip));
                }
                else
                {
                    // it's a cover movie, not a decal, so we make a layer, thinking it's at 0,0 and a left map (which gives us the results we want)
                    plLayerAVI *movieLayer = IMakeMovieLayer(decalChunk, 0, 0, mip, pfJournalDlgProc::kTagLeftDTMap, false);
                    loadedMovie *movie = new loadedMovie;
                    movie->movieLayer = movieLayer;
                    movie->movieChunk = decalChunk;
                    fLoadedMovies.emplace_back(movie);
                    layers.emplace_back(plLayerInterface::ConvertNoRef(movieLayer));
                    fVisibleLinks.clear(); // remove any links that the make movie layer might have added, since a cover movie can't link
                }
            }
            ISetDecalLayers(cover,layers);
        }
        else
        {
            layers.emplace_back(IMakeBaseLayer(fBookGUIs[fCurBookGUI]->DefaultCover()));
            ISetDecalLayers(cover,layers);
        }
        // release our ref on the cover layers since the material will take care of them now
        for (plLayerInterface* layer : layers)
            GetKey()->Release(layer->GetKey());
    }

//  fInstance->IPlayBookCloseAnim( !startOpened, true );
    fBookGUIs[fCurBookGUI]->TurnPageButton()->SetVisible( false );
    ITriggerCloseWithNotify( !startOpened, true );
}

//// IFinishShow /////////////////////////////////////////////////////////////
// Finish showing the book, due to the animation being done seeking

void    pfJournalBook::IFinishShow( bool startOpened )
{
    fBookGUIs[fCurBookGUI]->Dialog()->Show();

    fAreWeShowing = true;

    if( startOpened )
    {
        // Render initial pages
        fCurrentPage = 0;
        fVisibleLinks.clear();
        IRenderPage( 0, pfJournalDlgProc::kTagLeftDTMap );
        IRenderPage( 1, pfJournalDlgProc::kTagRightDTMap );

        fBookGUIs[fCurBookGUI]->UpdatePageCorners( pfBookData::kBothSides );
    }

    ISendNotify( kNotifyShow );
}

//// Hide ////////////////////////////////////////////////////////////////////

void    pfJournalBook::Hide()
{
    if (fBookGUIs[fCurBookGUI])
    {
        // we can only hide our own dialog
        if (fBookGUIs[fCurBookGUI]->CurBook() == this )
        {
            if (fBookGUIs[fCurBookGUI]->Dialog())
                fBookGUIs[fCurBookGUI]->Dialog()->Hide();
            fBookGUIs[fCurBookGUI]->CurBook(nullptr);
            ISendNotify( kNotifyHide );
            ILoadAllImages( true );
            // purge the dynaTextMaps, we're done with them for now
            IPurgeDynaTextMaps();
            // nuke the movies so they don't stay in memory (they're big!)
            for (loadedMovie* lm : fLoadedMovies)
            {
                plLayerAVI *movie = lm->movieLayer;
                movie->GetKey()->UnRefObject();
                delete lm;
            }
            fLoadedMovies.clear();
        }
    }

}

//// Open ////////////////////////////////////////////////////////////////////
// Opens the book, optionally to the given page

void    pfJournalBook::Open( uint32_t startingPage /*= 0 */)
{
    if( !fBookGUIs[fCurBookGUI]->CurrentlyOpen() )
    {
        ISendNotify(kNotifyOpen);

        fBookGUIs[fCurBookGUI]->PlayBookCloseAnim( false );

        // Make sure we actually know where the requested page is.
        if (startingPage > fCurrentPage)
            IRecalcPageStarts(startingPage);

        // Render initial pages
        fCurrentPage = startingPage;
        fVisibleLinks.clear();
        IRenderPage( startingPage, pfJournalDlgProc::kTagLeftDTMap );
        IRenderPage( startingPage + 1, pfJournalDlgProc::kTagRightDTMap );

        fBookGUIs[fCurBookGUI]->UpdatePageCorners( pfBookData::kBothSides );
    }
}

//// Close ///////////////////////////////////////////////////////////////////
// Closes the book.

void    pfJournalBook::Close()
{
    // don't allow them to close the book if the book started open
    if( !fBookGUIs[fCurBookGUI]->StartedOpen() && fBookGUIs[fCurBookGUI]->CurrentlyOpen() )
    {
        ISendNotify( kNotifyClose, 0 );
        fBookGUIs[fCurBookGUI]->PlayBookCloseAnim( true );
    }
}

//// CloseAndHide ////////////////////////////////////////////////////////////
// Closes the book, then calls Hide() once it's done closing

void    pfJournalBook::CloseAndHide()
{
    // if they start with the book open, then don't allow them to close it
    if( !fBookGUIs[fCurBookGUI]->StartedOpen() && fBookGUIs[fCurBookGUI]->CurrentlyOpen() )
    {
        // if we are flipping a book then kill the page flipping animation
        if ( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )
            fBookGUIs[fCurBookGUI]->KillPageFlip();
        ISendNotify( kNotifyClose, 1 );

        ITriggerCloseWithNotify( true, false );

        // Don't hide until we get the callback!
    }
    else
        // Already closed, just hide
        Hide();
}

//// ITriggerCloseWithNotify /////////////////////////////////////////////////
// Close with a notify

void    pfJournalBook::ITriggerCloseWithNotify( bool closeNotOpen, bool immediate )
{
    // Disable the book cover button if we're opening, enable otherwise
    fBookGUIs[fCurBookGUI]->CoverButton()->SetEnabled( closeNotOpen );

    // Do the animation manually so we can get a callback
    fBookGUIs[fCurBookGUI]->CurrentlyOpen(!closeNotOpen);

    const std::vector<plKey> &keys = fBookGUIs[fCurBookGUI]->CoverButton()->GetAnimationKeys();
    ST::string animName = fBookGUIs[fCurBookGUI]->CoverButton()->GetAnimationName();

    plAnimCmdMsg *msg = new plAnimCmdMsg();
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
    
    plEventCallbackMsg *eventMsg = new plEventCallbackMsg;
    eventMsg->AddReceiver( fBookGUIs[fCurBookGUI]->GetKey() );
    eventMsg->fRepeats = 0;
    eventMsg->fUser = 0x08 | ( closeNotOpen ? 0x00 : 0x01 );    // So we know which this is for
    eventMsg->fEvent = immediate ? ( plEventCallbackMsg::kSingleFrameEval ) : plEventCallbackMsg::kStop;
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

void    pfJournalBook::NextPage()
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
        fVisibleLinks.clear();

        // Swap the right DT map into the turn page front DTMap, then render
        // the new current page into turn page back and currPage+1 into 
        // the right DTMap
        plDynamicTextMap *turnFront = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagTurnFrontDTMap );
        plDynamicTextMap *right = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagRightDTMap );
        if ( turnFront->IsValid() && right->IsValid() )
        {
            memcpy( turnFront->GetImage(), right->GetImage(), right->GetLevelSize( 0 ) );
            if (turnFront->GetDeviceRef() != nullptr)
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

void    pfJournalBook::PreviousPage()
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
            int32_t newStartLine = left->GetFirstVisibleLine() - ((left->GetNumVisibleLines()-1)*2);
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
        fVisibleLinks.clear();

        // Swap the left DT map into the turn page back DTMap, then render
        // the new current page into the left and currPage+1 into 
        // the turn page front DTMap
        plDynamicTextMap *turnBack = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagTurnBackDTMap );
        plDynamicTextMap *left = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagLeftDTMap );
        if ( turnBack->IsValid() && left->IsValid() )
        {
            memcpy( turnBack->GetImage(), left->GetImage(), left->GetLevelSize( 0 ) );
            if (turnBack->GetDeviceRef() != nullptr)
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

hsSsize_t pfJournalBook::IFindCurrVisibleLink(bool rightNotLeft, bool hoverNotUp)
{
    pfGUIClickMapCtrl *ctrl = ( rightNotLeft ) ? fBookGUIs[fCurBookGUI]->RightPageMap() : fBookGUIs[fCurBookGUI]->LeftPageMap();

    hsPoint3 pt = hoverNotUp ? ctrl->GetLastMousePt() : ctrl->GetLastMouseUpPt();

    // This should be 0-1 in the context of the control, so scale to our DTMap size
    plDynamicTextMap *dtMap = fBookGUIs[fCurBookGUI]->GetDTMap( rightNotLeft ? pfJournalDlgProc::kTagRightDTMap : pfJournalDlgProc::kTagLeftDTMap );
    pt.fX *= (float)dtMap->GetWidth();
    pt.fY *= (float)dtMap->GetHeight();
    if( rightNotLeft )
    {
        // Clicks on the right side are offsetted in x by the left side's width
        dtMap = fBookGUIs[fCurBookGUI]->GetDTMap( pfJournalDlgProc::kTagLeftDTMap );
        pt.fX += dtMap->GetWidth();
    }

    // Search through the list of visible hotspots
    for (size_t i = 0; i < fVisibleLinks.size(); i++)
    {
        if (fVisibleLinks[i].Contains((int16_t)pt.fX, (int16_t)pt.fY ))
        {
            // Found a visible link
            return hsSsize_t(i);
        }
    }

    return -1;
}

//// IHandleLeftSideClick ////////////////////////////////////////////////////

void    pfJournalBook::IHandleLeftSideClick()
{
    if( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )
        return;

    if (hsSsize_t idx = IFindCurrVisibleLink(false, false); idx != -1)
    {
        if (fVisibleLinks[idx].IsCheckbox()) {
            IHandleCheckClick((uint32_t)idx, pfBookData::kLeftSide);
        } else {
            if (fVisibleLinks[idx].HasEventID())
                ISendNotify(kNotifyImageLink, fVisibleLinks[idx].GetEventID());
            if (!fVisibleLinks[idx].GetHRef().empty())
                IFlipToAnchor(fVisibleLinks[idx].GetHRef());
        }
        return;
    }

    // No link found that we're inside of, so just do the default behavior of turning the page
    PreviousPage();
}

void    pfJournalBook::IHandleRightSideClick()
{
    if( fBookGUIs[fCurBookGUI]->CurrentlyTurning() )
        return;

    if (hsSsize_t idx = IFindCurrVisibleLink(true, false); idx != -1)
    {
        if (fVisibleLinks[idx].IsCheckbox()) {
            IHandleCheckClick((uint32_t)idx, pfBookData::kRightSide);
        } else {
            if (fVisibleLinks[idx].HasEventID())
                ISendNotify(kNotifyImageLink, fVisibleLinks[idx].GetEventID());
            if (!fVisibleLinks[idx].GetHRef().empty())
                IFlipToAnchor(fVisibleLinks[idx].GetHRef());
        }
        return;
    }

    // No link found that we're inside of, so just do the default behavior of turning the page
    NextPage();
}

hsSsize_t pfJournalBook::IFindAnchor(const ST::string& anchor) const
{
    for (hsSsize_t i = fHTMLSource.size() - 1; i >= 0; --i) {
        if (fHTMLSource[i]->fType != pfEsHTMLChunk::kTextLink)
            continue;
        // Match the anchor, not links to it
        if (hsTestBits(fHTMLSource[i]->fFlags, pfEsHTMLChunk::kCanLink))
            continue;
        if (fHTMLSource[i]->fText.compare_i(anchor) == 0)
            return i;
    }

    return -1;
}

void    pfJournalBook::IFlipToAnchor(const ST::string& anchor)
{
    if (fAreEditing)
        return;

    // We don't know where this anchor is in the book. We could theoretically use
    // fPageStarts to figure out which page the chunk appears on, but paragraph chunks
    // can be split as pages are turned. So, fPageStarts can't be trusted until the entire
    // book is rendered at least once. So, we need to force the issue. Thankfully, forcing
    // the issue only recalcs what has NOT been rendered yet, so we only incur a performance
    // penalty once for doing this.
    ForceCacheCalculations();

    // Now that everything is stable, we can look up the anchor.
    hsSsize_t chunkIdx = IFindAnchor(anchor);
    if (chunkIdx == -1) {
        hsAssert(0, "Trying to link to a non-existent anchor, eh?");
        return;
    }

    hsSsize_t pageNum = -1;
    for (hsSsize_t i = 0; i < fPageStarts.size(); ++i) {
        // Look for the chunk *after* the anchor - it's the one with the content
        // we care about. Unless they put the anchor at the end of the book...
        if (fPageStarts[i] > std::min((uint32_t)(chunkIdx + 1), fPageStarts.back())) {
            pageNum = i - 1;
            break;
        }
    }
    if (pageNum == -1) {
        hsAssert(0, "Ahhhhh! We couldn't find the page for this anchor!");
        return;
    }

    if ((pageNum & ~0x00000001) == fCurrentPage) {
        // Make sure everything is drawn nicely.
        GoToPage(fCurrentPage);
    } else if (pageNum < fCurrentPage) {
        fCurrentPage = pageNum & ~0x00000001;
        fVisibleLinks.clear();

        // Swap the left DT map into the turn page back DTMap, then render
        // the new current page into the left and currPage+1 into
        // the turn page front DTMap
        plDynamicTextMap* turnBack = fBookGUIs[fCurBookGUI]->GetDTMap(pfJournalDlgProc::kTagTurnBackDTMap);
        plDynamicTextMap* left = fBookGUIs[fCurBookGUI]->GetDTMap(pfJournalDlgProc::kTagLeftDTMap);
        if (turnBack->IsValid() && left->IsValid()) {
            memcpy(turnBack->GetImage(), left->GetImage(), left->GetLevelSize(0));
            if (turnBack->GetDeviceRef() != nullptr)
                turnBack->GetDeviceRef()->SetDirty(true);
        }
        // copy the videos over
        IMoveMovies(fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kLeftPage), fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kTurnBackPage));
        IRenderPage(fCurrentPage + 1, pfJournalDlgProc::kTagTurnFrontDTMap);

        // This will fire a callback when it's done that'll let us continue the setup
        fBookGUIs[fCurBookGUI]->StartTriggeredFlip(true);

        // Play us a sound too, if defined on our button
        fBookGUIs[fCurBookGUI]->TurnPageButton()->PlaySound(pfGUICheckBoxCtrl::kMouseUp);

        fBookGUIs[fCurBookGUI]->UpdatePageCorners(pfBookData::kBothSides);

        ISendNotify(kNotifyPreviousPage);
    } else {
        fCurrentPage = pageNum & ~0x00000001;
        fVisibleLinks.clear();

        // Swap the right DT map into the turn page front DTMap, then render
        // the new current page into turn page back and currPage+1 into
        // the right DTMap
        plDynamicTextMap* turnFront = fBookGUIs[fCurBookGUI]->GetDTMap(pfJournalDlgProc::kTagTurnFrontDTMap);
        plDynamicTextMap* right = fBookGUIs[fCurBookGUI]->GetDTMap(pfJournalDlgProc::kTagRightDTMap);
        if (turnFront->IsValid() && right->IsValid()) {
            memcpy(turnFront->GetImage(), right->GetImage(), right->GetLevelSize(0));
            if (turnFront->GetDeviceRef() != nullptr)
                turnFront->GetDeviceRef()->SetDirty(true);
        }
        // copy the videos over
        IMoveMovies(fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kRightPage), fBookGUIs[fCurBookGUI]->PageMaterial(pfBookData::kTurnFrontPage));
        IRenderPage(fCurrentPage, pfJournalDlgProc::kTagTurnBackDTMap);

        // This will fire a callback when it's done that'll let us continue the setup
        fBookGUIs[fCurBookGUI]->StartTriggeredFlip(false);

        // Play us a sound too, if defined on our button
        fBookGUIs[fCurBookGUI]->TurnPageButton()->PlaySound(pfGUICheckBoxCtrl::kMouseUp);

        fBookGUIs[fCurBookGUI]->UpdatePageCorners(pfBookData::kBothSides);

        ISendNotify(kNotifyNextPage);
    }
}

//// IHandleCheckClick ///////////////////////////////////////////////////////
// Process a click on the given "check box" image

void    pfJournalBook::IHandleCheckClick( uint32_t idx, pfBookData::WhichSide which )
{
    // Special processing for checkboxes--toggle our state, switch our opacity
    // and then send a notify about our new state
    bool check = !fVisibleLinks[idx].IsChecked();
    if( check )
    {
        fVisibleLinks[ idx ].SetChecked(true);
//      fVisibleLinks[ idx ]->fCurrOpacity = fVisibleLinks[ idx ]->fMaxOpacity;
    }
    else
    {
        fVisibleLinks[ idx ].SetChecked(false);
//      fVisibleLinks[ idx ]->fCurrOpacity = fVisibleLinks[ idx ]->fMinOpacity;
    }

    // Re-render the page we're on, to show the change in state
    IRenderPage( fCurrentPage + ( ( which == pfBookData::kLeftSide ) ? 0 : 1 ), ( which == pfBookData::kLeftSide ) ? pfJournalDlgProc::kTagLeftDTMap: pfJournalDlgProc::kTagRightDTMap );

    ISendNotify(check ? kNotifyImageLink : kNotifyCheckUnchecked, fVisibleLinks[idx].GetEventID());

    // Register for FX processing, so we can fade the checkbox in
    fBookGUIs[fCurBookGUI]->RegisterForSFX( (pfBookData::WhichSide)( fBookGUIs[fCurBookGUI]->CurSFXPages() | which ) );
}

//// GoToPage ////////////////////////////////////////////////////////////////
// For completeness...

void    pfJournalBook::GoToPage( uint32_t pageNumber )
{
    // Put us here, but only on an even page (odd pages go on the right, y'know)
    // (no need for a range check, going past the end simply puts you on a blank page, able to go backward but not forward)
    fCurrentPage = pageNumber & ~0x00000001;
    fVisibleLinks.clear();
    IRenderPage( fCurrentPage, pfJournalDlgProc::kTagLeftDTMap );
    IRenderPage( fCurrentPage + 1, pfJournalDlgProc::kTagRightDTMap );
    fBookGUIs[fCurBookGUI]->UpdatePageCorners( pfBookData::kBothSides );
}

//// SetEditable /////////////////////////////////////////////////////////////

void    pfJournalBook::SetEditable(bool editable)
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

void    pfJournalBook::ForceCacheCalculations()
{
    // Make sure our page starts are up-to-snuff, at least to this point
    IRecalcPageStarts( -1 );
}

//// ICompileSource //////////////////////////////////////////////////////////
// Compiles the given string of esHTML source into our compiled chunk list

bool    pfJournalBook::ICompileSource(const ST::string& source, const plLocation &hintLoc)
{
    IFreeSource();

    pfEsHTMLChunk *chunk, *lastParChunk = new pfEsHTMLChunk(ST::string());
    pfEsHTMLChunk *lastLinkChunk = nullptr;
    const char *c, *start;
    ST::string name;
    ST::string option;
    float bookWidth=1.f, bookHeight=1.f;
    uint8_t movieIndex = 0; // the index of a movie in the source (used for id purposes)

    plKey anotherKey;


    // Parse our source!
    const char *end = source.end();
    for (start = c = source.begin(); c < end;) {
        // Are we on a tag?
        uint8_t type = IGetTagType(c, end);
        if (type != pfEsHTMLChunk::kEmpty) {
            // First, end the current paragraph chunk, which is a special case 'cause its 
            // text is defined outside the tag
            if (start == c ) {
                // No actual text, just delete
                delete lastParChunk;
                lastParChunk = nullptr;
            } else if (lastParChunk) {
                lastParChunk->fText = ST::string(start, c - start);
                fHTMLSource.emplace_back(lastParChunk);
            }

            // What chunk are we making now?
            switch (type) {
                case pfEsHTMLChunk::kParagraph:
                    c += 2;
                    chunk = new pfEsHTMLChunk(ST::string());
                    chunk->fFlags = IFindLastAlignment();
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("align") == 0) {
                            if (option.compare_i("left") == 0)
                                chunk->fFlags = pfEsHTMLChunk::kLeft;
                            else if (option.compare_i("center") == 0)
                                chunk->fFlags = pfEsHTMLChunk::kCenter;
                            else if (option.compare_i("right") == 0)
                                chunk->fFlags = pfEsHTMLChunk::kRight;
                        }
                    }
                    // Append text to this one (don't add to source just yet)
                    lastParChunk = chunk;
                    break;

                case pfEsHTMLChunk::kImage:
                    c += 4;
                    chunk = new pfEsHTMLChunk(nullptr , 0);
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("align") == 0) {
                            chunk->fFlags &= ~pfEsHTMLChunk::kAlignMask;
                            if (option.compare_i("left") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kLeft;
                            else if (option.compare_i("center") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kCenter;
                            else if (option.compare_i("right") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kRight;
                        } else if (name.compare_i("src") == 0) {
                            // Name of mipmap source
                            chunk->fImageKey = IGetMipmapKey( option, hintLoc );
                        } else if (name.compare_i("link") == 0) {
                            chunk->fEventID = option.to_uint();
                            chunk->fFlags |= pfEsHTMLChunk::kCanLink;
                            chunk->fFlags |= pfEsHTMLChunk::kValidEvent;
                        } else if (name.compare_i("blend") == 0) {
                            if (option.compare_i("alpha") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kBlendAlpha;
                        } else if (name.compare_i("pos") == 0) {
                            chunk->fFlags |= pfEsHTMLChunk::kFloating;

                            ST::string comma = option.after_first(',');
                            if (!comma.empty()) {
                                chunk->fAbsoluteY = comma.to_ushort();
                            }
                            chunk->fAbsoluteX = option.before_first(',').to_ushort();
                        } else if (name.compare_i("glow") == 0) {
                            chunk->fFlags |= pfEsHTMLChunk::kGlowing;
                            chunk->fFlags &= ~pfEsHTMLChunk::kActAsCB;

                            ST::string comma = option.after_first(',');
                            if (!comma.empty()) {
                                ST::string comma2 = comma.after_first(',');
                                if (!comma2.empty()) {
                                    chunk->fMaxOpacity = comma2.to_float();
                                }
                                chunk->fMinOpacity = comma.before_first(',').to_float();
                            }
                            chunk->fSFXTime = option.before_first(',').to_float();
                        } else if (name.compare_i("opacity") == 0) {
                            chunk->fFlags |= pfEsHTMLChunk::kTranslucent;
                            chunk->fCurrOpacity = option.to_float();
                        } else if (name.compare_i("check") == 0) {
                            chunk->fFlags |= pfEsHTMLChunk::kActAsCB;
                            chunk->fFlags &= ~pfEsHTMLChunk::kGlowing;

                            ST::string comma = option.after_first(',');
                            if (!comma.empty()) {
                                ST::string comma2 = comma.after_first(',');
                                if (!comma2.empty()) {
                                    if (comma2.to_long())
                                        chunk->fFlags |= pfEsHTMLChunk::kChecked;
                                }
                                comma = comma.before_first(',');
                                uint32_t c = comma.to_uint(16);
                                if (comma.size() <= 6)
                                    c |= 0xff000000;    // Add in full alpha if none specified
                                chunk->fOffColor.FromARGB32(c);
                            }
                            option = option.before_first(',');
                            uint32_t c = option.to_uint(16);
                            if (option.size() <= 6)
                                c |= 0xff000000;    // Add in full alpha if none specified
                            chunk->fOnColor.FromARGB32(c);

                            if (chunk->fFlags & pfEsHTMLChunk::kChecked)
                                chunk->fCurrColor = chunk->fOnColor;
                            else
                                chunk->fCurrColor = chunk->fOffColor;
                        } else if (name.compare_i("resize") == 0) {
                            chunk->fNoResizeImg = (option.compare_i("no") == 0);
                        } else if (name.compare_i("href") == 0) {
                            hsSetBits(chunk->fFlags, pfEsHTMLChunk::kCanLink);
                            chunk->fText = std::move(option);
                        }
                    }

                    // Inherit link
                    if (!hsCheckBits(chunk->fFlags, pfEsHTMLChunk::kCanLink) && lastLinkChunk) {
                        hsSetBits(chunk->fFlags, pfEsHTMLChunk::kCanLink);
                        hsSetBits(chunk->fFlags, pfEsHTMLChunk::kValidEvent);
                        chunk->fEventID = lastLinkChunk->fEventID;
                    }

                    if (chunk->fImageKey)
                        fHTMLSource.emplace_back(chunk);
                    else
                        delete chunk;
                    // Start new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kCover:
                    // Don't create an actual chunk for this one, just use the "src" and 
                    // grab the mipmap key for our cover
                    c += 6;
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("src") == 0) {
                            // Name of mipmap source
                            anotherKey = IGetMipmapKey( option, hintLoc );
                            if (anotherKey) {
                                fCoverMipKey = anotherKey;
                                fCoverFromHTML = true;
                            }
                        } else if (name.compare_i("tint") == 0) {
                            fTintCover = true;
                            fCoverTint.FromARGB32(option.to_long(16) | 0xff000000);
                        } else if (name.compare_i("tintfirst") == 0) {
                            fTintFirst = (option.compare_i("no") != 0);
                        }
                    }
                    // Still gotta create a new par chunk
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kPageBreak:
                    c += 3;
                    chunk = new pfEsHTMLChunk();
                    while (IGetNextOption(c, end, name, option)) {
                    }
                    fHTMLSource.emplace_back(chunk);
                    // Start new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kFontChange:
                    c += 5;
                    chunk = new pfEsHTMLChunk(ST::string(), 0, 0);
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("style") == 0) {
                            uint8_t guiFlags = 0;
                            if (option.compare_i("b") == 0) {
                                chunk->fFlags = pfEsHTMLChunk::kFontBold;
                                guiFlags = plDynamicTextMap::kFontBold;
                            } else if (option.compare_i("i") == 0) {
                                chunk->fFlags = pfEsHTMLChunk::kFontItalic;
                                guiFlags = plDynamicTextMap::kFontItalic;
                            } else if (option.compare_i("bi") == 0) {
                                chunk->fFlags = pfEsHTMLChunk::kFontBold | pfEsHTMLChunk::kFontItalic;
                                guiFlags = plDynamicTextMap::kFontBold | plDynamicTextMap::kFontItalic;
                            } else {
                                chunk->fFlags = pfEsHTMLChunk::kFontRegular;
                            }

                            if (fBookGUIs[fCurBookGUI]->IsEditable()) {
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontStyle(guiFlags);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontStyle(guiFlags);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontStyle(guiFlags);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontStyle(guiFlags);
                            }
                        } else if (name.compare_i("face") == 0) {
                            // Name of mipmap source
                            chunk->fText = option;
                            if (fBookGUIs[fCurBookGUI]->IsEditable()) {
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontFace(option);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontFace(option);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontFace(option);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontFace(option);
                            }
                        } else if (name.compare_i("size") == 0) {
                            chunk->fFontSize = (uint8_t)option.to_ulong();
                            if (fBookGUIs[fCurBookGUI]->IsEditable()) {
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontSize(chunk->fFontSize);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontSize(chunk->fFontSize);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontSize(chunk->fFontSize);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontSize(chunk->fFontSize);
                            }
                        } else if(name.compare_i("color") == 0) {
                            chunk->fColor.FromARGB32(option.to_ulong(16) | 0xff000000);
                            chunk->fFlags |= pfEsHTMLChunk::kFontColor;
                            if (fBookGUIs[fCurBookGUI]->IsEditable()) {
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetFontColor(chunk->fColor);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetFontColor(chunk->fColor);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetFontColor(chunk->fColor);
                                fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetFontColor(chunk->fColor);
                            }
                        } else if(name.compare_i("spacing") == 0) {
                            chunk->fLineSpacing = option.to_short();
                            chunk->fFlags |= pfEsHTMLChunk::kFontSpacing;
                        }
                    }
                    fHTMLSource.emplace_back(chunk);
                    // Start new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kMargin:
                    c += 7;
                    while(IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("top") == 0)
                            fPageTMargin = option.to_uint();
                        else if (name.compare_i("left") == 0)
                            fPageLMargin = option.to_uint();
                        else if (name.compare_i("bottom") == 0)
                            fPageBMargin = option.to_uint();
                        else if (name.compare_i("right") == 0)
                            fPageRMargin = option.to_uint();
                    }
                    // set the edit controls to the margins we just set
                    if (fBookGUIs[fCurBookGUI]->IsEditable()) {
                        fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagRightEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
                        fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagLeftEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
                        fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnFrontEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
                        fBookGUIs[fCurBookGUI]->GetEditCtrl(pfJournalDlgProc::kTagTurnBackEditCtrl)->SetMargins(fPageTMargin,fPageLMargin,fPageBMargin,fPageRMargin);
                    }
                    // Start a new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kBook:
                    c += 5;
                    // don't actually create a chunk, just set the book size
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("height") == 0)
                            bookHeight = option.to_float();
                        else if (name.compare_i("width") == 0)
                            bookWidth = option.to_float();
                    }
                    fHeightScale = 1.f - bookHeight;
                    fWidthScale = 1.f - bookWidth;

                    // Still gotta create a new par chunk
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kDecal:
                    c += 6;
                    chunk = new pfEsHTMLChunk(nullptr, 0);
                    chunk->fType = pfEsHTMLChunk::kDecal;
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("align") == 0) {
                            chunk->fFlags &= ~pfEsHTMLChunk::kAlignMask;
                            if (option.compare_i("left") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kLeft;
                            else if (option.compare_i("center") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kCenter;
                            else if (option.compare_i("right") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kRight;
                        } else if (name.compare_i("src") == 0) {
                            // Name of mipmap source
                            chunk->fImageKey = IGetMipmapKey(option, hintLoc);
                        } else if (name.compare_i("pos") == 0) {
                            chunk->fFlags |= pfEsHTMLChunk::kFloating;

                            ST::string comma = option.after_first(',');
                            if (!comma.empty()) {
                                chunk->fAbsoluteY = comma.to_ushort();
                            }
                            chunk->fAbsoluteX = option.before_first(',').to_ushort();
                        } else if (name.compare_i("resize") == 0) {
                            chunk->fNoResizeImg = (option.compare_i("no") == 0);
                        } else if (name.compare_i("tint") == 0) {
                            chunk->fTintDecal = (option.compare_i("yes") == 0);
                        }
                    }
                    // add it to our cover decals list (this is tag is essentially thrown away as far as the parser cares)
                    if (chunk->fImageKey)
                        fCoverDecals.emplace_back(chunk);
                    else
                        delete chunk;
                    // Start new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kMovie:
                    c += 6;
                    chunk = new pfEsHTMLChunk(nullptr, 0);
                    chunk->fType = pfEsHTMLChunk::kMovie;
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("align") == 0) {
                            chunk->fFlags &= ~pfEsHTMLChunk::kAlignMask;
                            if (option.compare_i("left") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kLeft;
                            else if (option.compare_i("center") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kCenter;
                            else if (option.compare_i("right") == 0)
                                chunk->fFlags |= pfEsHTMLChunk::kRight;
                        } else if(name.compare_i("src") == 0) {
                            chunk->fText = option;
                        } else if(name.compare_i("link") == 0) {
                            chunk->fEventID = option.to_uint();
                            chunk->fFlags |= pfEsHTMLChunk::kCanLink;
                            chunk->fFlags |= pfEsHTMLChunk::kValidEvent;
                        } else if(name.compare_i("pos") == 0) {
                            chunk->fFlags |= pfEsHTMLChunk::kFloating;

                            ST::string comma = option.after_first(',');
                            if (!comma.empty()) {
                                chunk->fAbsoluteY = comma.to_ushort();
                            }
                            chunk->fAbsoluteX = option.before_first(',').to_ushort();
                        } else if (name.compare_i("resize") == 0) {
                            chunk->fNoResizeImg = (option.compare_i("no") == 0);
                        } else if (name.compare_i("oncover") == 0) {
                            chunk->fOnCover = (option.compare_i("yes") == 0);
                        } else if (name.compare_i("loop") == 0) {
                            chunk->fLoopMovie = option.compare_i("no") != 0;
                        } else if (name.compare_i("href") == 0) {
                            hsSetBits(chunk->fFlags, pfEsHTMLChunk::kCanLink);
                            chunk->fText = std::move(option);
                        }
                    }

                    // Inherit link
                    if (!hsCheckBits(chunk->fFlags, pfEsHTMLChunk::kCanLink) && lastLinkChunk) {
                        hsSetBits(chunk->fFlags, pfEsHTMLChunk::kCanLink);
                        hsSetBits(chunk->fFlags, pfEsHTMLChunk::kValidEvent);
                        chunk->fEventID = lastLinkChunk->fEventID;
                    }

                    chunk->fMovieIndex = movieIndex;
                    movieIndex++;
                    if (chunk->fOnCover) {
                        if (!chunk->fText.empty())
                            fCoverDecals.emplace_back(chunk);
                        else
                            delete chunk;
                    } else {
                        if (!chunk->fText.empty())
                            fHTMLSource.emplace_back(chunk);
                        else
                            delete chunk;
                    }
                    // Start new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;
                    
                case pfEsHTMLChunk::kEditable:
                    c += 9;
                    SetEditable(true);
                    chunk = new pfEsHTMLChunk();
                    while (IGetNextOption(c, end, name, option)) {
                    }
                    fHTMLSource.emplace_back(chunk);
                    // Start new paragraph chunk after this one
                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;

                case pfEsHTMLChunk::kTextLink:
                    c += 2;
                    chunk = new pfEsHTMLChunk(pfEsHTMLChunk::TextLink);
                    while (IGetNextOption(c, end, name, option)) {
                        if (name.compare_i("link") == 0) {
                            ST::conversion_result result;
                            chunk->fEventID = option.to_uint(result);
                            // Ideally, we'd do something like link=clear, but we'll just
                            // go with anything that isn't a valid integer.
                            hsChangeBits(chunk->fFlags, pfEsHTMLChunk::kCanLink, result.ok());
                            hsChangeBits(chunk->fFlags, pfEsHTMLChunk::kValidEvent, result.ok());
                        } else if (name.compare_i("color") == 0) {
                            chunk->fColor.FromARGB32(option.to_ulong(16) | 0xff000000);
                        } else if (name.compare_i("name") == 0) {
                            chunk->fText = ST::format("#{}", option);
                        } else if (name.compare_i("href") == 0) {
                            hsSetBits(chunk->fFlags, pfEsHTMLChunk::kCanLink);
                            chunk->fText = std::move(option);
                        }
                    }
                    fHTMLSource.emplace_back(chunk);

                    // For other interactable bits to inherit the link.
                    if (hsTestBits(chunk->fFlags, pfEsHTMLChunk::kCanLink))
                        lastLinkChunk = chunk;
                    else
                        lastLinkChunk = nullptr;

                    lastParChunk = new pfEsHTMLChunk(ST::string());
                    lastParChunk->fFlags = IFindLastAlignment();
                    break;
            }

            start = c;
        } else {
            // Keep looking
            c++;
        }
    }

    // Final bit goes into the last paragraph chunk we had
    if (start == c) {
        // No actual text, just delete
        delete lastParChunk;
        lastParChunk = nullptr;
    } else if (lastParChunk) {
        lastParChunk->fText = ST::string(start, c - start);

        fHTMLSource.emplace_back(lastParChunk);
    }

    // Reset a few
    fPageStarts = {0};
    if (fAreEditing)
        fLastPage = 0;
    else
        fLastPage = -1;

    return true;
}

uint8_t   pfJournalBook::IGetTagType( const char *string, const char *end )
{
    if (string == end || string[0] != '<')
        return pfEsHTMLChunk::kEmpty;

    string++;

    struct TagRec
    {
        ST::string fTag;
        uint8_t       fType;
    } tags[] = {{ ST_LITERAL("p"), pfEsHTMLChunk::kParagraph },
                { ST_LITERAL("img"), pfEsHTMLChunk::kImage },
                { ST_LITERAL("pb"), pfEsHTMLChunk::kPageBreak },
                { ST_LITERAL("font"), pfEsHTMLChunk::kFontChange },
                { ST_LITERAL("margin"), pfEsHTMLChunk::kMargin },
                { ST_LITERAL("cover"), pfEsHTMLChunk::kCover },
                { ST_LITERAL("book"), pfEsHTMLChunk::kBook },
                { ST_LITERAL("decal"), pfEsHTMLChunk::kDecal },
                { ST_LITERAL("movie"), pfEsHTMLChunk::kMovie },
                { ST_LITERAL("editable"), pfEsHTMLChunk::kEditable },
                { ST_LITERAL("a"), pfEsHTMLChunk::kTextLink },
    };

    for (const auto& tag : tags)
    {
        if (string + tag.fTag.size() < end && tag.fTag.compare_ni(string, tag.fTag.size()) == 0)
        {
            // Found tag--but only space or end tag marker allowed afterwards
            char sep = string[tag.fTag.size()];
            if (sep == '>' || sep == ' ')
                return tag.fType;
        }
    }

    return pfEsHTMLChunk::kEmpty;
}

bool    pfJournalBook::IGetNextOption( const char *&string, const char *end, ST::string& name, ST::string& option )
{
    const char *c;


    // Advance past any white space
    while (string < end && *string == ' ')
        string++;

    if (string < end && *string == '>')
    {
        string++;
        return false;
    }

    // Advance to =
    c = string;
    while (string < end && *string != '>' && *string != ' ' && *string != '=')
        string++;

    if (string == end || *string != '=')
        return false;

    // Copy name
    name = ST::string::from_utf8(c, string - c);

    // Find start of option value
    string++;
    while (string < end && *string == ' ')
        string++;

    if (string == end || *string == '>')
        return false;

    if (*string == '\"')
    {
        // Search for other quote
        string++;
        c = string;
        while (string < end && *string != '>' && *string != '\"')
            string++;

        option = ST::string::from_utf8(c, string - c);
        
        if (string < end && *string == '\"')
            string++;

        return true;
    }

    // Non-quoted token
    c = string;
    while (string < end && *string != ' ' && *string != '>')
        string++;

    option = ST::string::from_utf8(c, string - c);
    
    return true;
}

void    pfJournalBook::IFreeSource()
{
    for (pfEsHTMLChunk* chunk : fHTMLSource)
        delete chunk;
    fHTMLSource.clear();

    for (pfEsHTMLChunk* decalChunk : fCoverDecals)
        delete decalChunk;
    fCoverDecals.clear();

    for (loadedMovie* lm : fLoadedMovies)
    {
        plLayerAVI *movie = lm->movieLayer;
        movie->GetKey()->UnRefObject();
        delete lm;
    }
    fLoadedMovies.clear();
}

//// IGetMipmapKey ///////////////////////////////////////////////////////////
// Looks up the key for a mipmap given the image name. Note that the given
// location is treated as a hint; if the image isn't found in that location,
// the code will attempt to look in the currently loaded age for a matching
// image name.

plKey   pfJournalBook::IGetMipmapKey( const ST::string& name, const plLocation &loc )
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if( name.contains( '/' ) || name.contains( '\\' ) )
    {
        // For internal use only--allow local path names of PNG and JPEG images, to
        // facilitate fast prototyping
        plMipmap *mip;
        if( name.contains( ".png" ) )
            mip = plPNG::Instance().ReadFromFile( name.c_str() );
        else
            mip = plJPEG::Instance().ReadFromFile( name.c_str() );

        hsgResMgr::ResMgr()->NewKey(name, mip, loc );
        return mip->GetKey();
    }
#endif

    // Try first to find in the given location
    plUoid myUoid( loc, plMipmap::Index(), name);
    plKey key = hsgResMgr::ResMgr()->FindKey( myUoid );
    if (key != nullptr)
    {
        return key;
    }


    // Next, try our "global" pre-defined age
    const plLocation &globLoc = plKeyFinder::Instance().FindLocation( "GUI", "BkBookImages" );
    myUoid = plUoid( globLoc, plMipmap::Index(), name);
    key = hsgResMgr::ResMgr()->FindKey( myUoid );
    if (key != nullptr)
    {
        return key;
    }

    // Do a search through our current age with just the name given
    if (plNetClientApp::GetInstance() != nullptr)
    {
        ST::string thisAge = plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName();
        if (!thisAge.empty())
        {
            key = plKeyFinder::Instance().StupidSearch( thisAge, "", plMipmap::Index(), name, true );
            if (key != nullptr)
            {
                return key;
            }
        }
    }

    return nullptr;
}

//// IRenderPage /////////////////////////////////////////////////////////////
//  Takes the given page out of the source and renders it into the specified
//  DTMap (by GUI tag ID). If no page is cached after this one, also updates
//  the various cached info about page endings, etc.

void    pfJournalBook::IRenderPage( uint32_t page, uint32_t whichDTMap, bool suppressRendering /*= false*/ )
{
    if (fAreEditing)
        return; // we don't render if we are editing the book
    
    // Grab the DTMap via the GUI system
    plDynamicTextMap *dtMap = fBookGUIs[fCurBookGUI]->GetDTMap( whichDTMap );
    hsAssert(dtMap != nullptr, "Invalid DT map in IRenderPage()");

    loadedMovie *movie = nullptr;
    bool movieAlreadyLoaded = false;

    // Make sure our page starts are up-to-snuff, at least to this point
    IRecalcPageStarts( page );

    // Render!
    hsColorRGBA color;
    color.Set( 0, 0, 0, 0 );
    if( !suppressRendering )
        dtMap->ClearToColor( color );

    hsGMaterial *material = nullptr;
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
        for (size_t i = 0; i < material->GetNumLayers(); i++) // remove all plLayerMovie layers
        {
            plLayerInterface *matLayer = material->GetLayer(i);
            plLayerAVI *movie = plLayerAVI::ConvertNoRef(matLayer);
            if (movie) // if it was a movie layer
            {
                plMatRefMsg* refMsg = new plMatRefMsg(material->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer); // remove it
                hsgResMgr::ResMgr()->SendRef(material->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
            }
        }
    }

    hsAssert(page < fPageStarts.size() || page > fLastPage, "UnInitialized page start!");
    if (page <= fLastPage
        && page < fPageStarts.size())   // Added this as a crash-prevention bandaid - MT
    {
        uint32_t idx;
        uint16_t width, height, y, x, ascent, lastX, lastY;
        
        uint8_t     fontFlags, fontSize;
        ST::string  fontFace;
        hsColorRGBA fontColor;
        int16_t     fontSpacing;
        bool        needSFX = false;

        // Find the current text link
        pfEsHTMLChunk* currLinkChunk = IFindTextLink(fPageStarts[page]);

        // Find and set initial font properties
        IFindFontProps( fPageStarts[ page ], fontFace, fontSize, fontFlags, fontColor, fontSpacing );
        dtMap->SetFont( fontFace, fontSize, fontFlags, false );
        if (currLinkChunk != nullptr)
            dtMap->SetTextColor(currLinkChunk->fColor, true);
        else
            dtMap->SetTextColor(fontColor, true);
        dtMap->SetLineSpacing(fontSpacing);

        for (idx = fPageStarts[page], x = (uint16_t)fPageLMargin, y = (uint16_t)fPageTMargin;
            y < (uint16_t)(512 - fPageTMargin - fPageBMargin) && idx < fHTMLSource.size(); idx++)
        {
            if (fPageStarts.size() > page + 1 && idx == fPageStarts[page + 1])
                break;  // Just go ahead and break at the start of the next page, since we already found it

            pfEsHTMLChunk *chunk = fHTMLSource[ idx ];

            switch( chunk->fType )
            {
                case pfEsHTMLChunk::kParagraph:             
                    if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kLeft )
                    {
                        if (dtMap->GetFontJustify() != plDynamicTextMap::kLeftJustify) {
                            dtMap->SetJustify(plDynamicTextMap::kLeftJustify);
                            x = (uint16_t)fPageLMargin; // reset X if our justification changes
                        }
                    }
                    else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kRight )
                    {
                        if (dtMap->GetFontJustify() != plDynamicTextMap::kRightJustify) {
                            dtMap->SetJustify(plDynamicTextMap::kRightJustify);
                            x = (uint16_t)fPageLMargin; // reset X if our justification changes
                        }
                    }
                    else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kCenter )
                    {
                        if (dtMap->GetFontJustify() != plDynamicTextMap::kCenter) {
                            dtMap->SetJustify(plDynamicTextMap::kCenter);
                            x = (uint16_t)fPageLMargin; // reset X if our justification changes
                        }
                    }

                    dtMap->SetFirstLineIndent( (int16_t)(x - fPageLMargin) );
                    width = (uint16_t)(512 - fPageLMargin - fPageRMargin);
                    height = (uint16_t)(512 - fPageBMargin - y);
                    uint32_t lastChar;
                    dtMap->CalcWrappedStringSize( chunk->fText, &width, &height, &lastChar, &ascent, &lastX, &lastY );
                    width = (uint16_t)(512 - fPageLMargin - fPageRMargin);
                    if( !suppressRendering )
                        dtMap->DrawWrappedString( (uint16_t)fPageLMargin, y, chunk->fText, width, (uint16_t)(512 - fPageBMargin - y), &lastX, &lastY );

                    // Insert link rects without making the code too crazy.
                    // A huge assumption here... We can simply move from the (x, y) render starting position
                    // down and to the right. This _will_ break if anyone wants to add support for RTL.
                    // Additionally, if a line wraps such that there's a lot of blank space, that blank
                    // space will still be clickable. Ugh.
                    if (currLinkChunk != nullptr) {
                        int16_t maxCharHt = (int16_t)dtMap->GetCurrFont()->GetMaxCharHeight();
                        int16_t lineDelta = maxCharHt + fontSpacing;
                        pcSmallRect linkRect(x, y, 0, maxCharHt);

                         // Right page rects are offsetted to differentiate
                        int16_t xOffs = 0;
                        if (whichDTMap == pfJournalDlgProc::kTagRightDTMap || whichDTMap == pfJournalDlgProc::kTagTurnFrontDTMap)
                            xOffs = (int16_t)(dtMap->GetWidth());

                        while (linkRect.fY <= lastY) {
                            if (linkRect.fY + lineDelta >= lastY)
                                linkRect.fWidth = lastX - linkRect.fX;
                            else
                                linkRect.fWidth = 512 - fPageRMargin - linkRect.fX;

                            // Blank line at end of page could give us a zero width rect, skip that
                            if (!suppressRendering && linkRect.fWidth > 0) {
                                fVisibleLinks.emplace_back(
                                    currLinkChunk,
                                    linkRect.fX + xOffs,
                                    linkRect.fY,
                                    linkRect.fWidth, linkRect.fHeight
                                );
                                if (s_ShowLinkRects) {
                                    dtMap->FrameRect(
                                        linkRect.fX, linkRect.fY,
                                        linkRect.fWidth, linkRect.fHeight,
                                        s_LinkRectColor
                                    );
                                }
                            } else if (!suppressRendering && linkRect.fWidth > 0) {
                                fVisibleLinks.emplace_back(
                                    currLinkChunk,
                                    0, 0, 0, 0
                                );
                            }

                            linkRect.fX = (int16_t)fPageLMargin;
                            linkRect.fY += lineDelta;
                        }
                    }

                    if( lastChar == 0 )
                    {
                        // This paragraph didn't fit on this page at *all*, so just bump it to the next
                        // one artificially (the -- is to account for the for loop; see image handling below)
                        y += 512;
                        if( idx > fPageStarts[ page ] )
                            idx--;
                        break;
                    }
                    if (lastChar < chunk->fText.size())
                    {
                        // Didn't get to render the whole paragraph in this go, so we're going to cheat
                        // and split the paragraph up into two so that we can handle it properly. Note:
                        // this changes the chunk array beyond this point, so we need to invalidate the
                        // cache, but that's ok 'cause if we're doing this, it's probably invalid (or empty)
                        // anyway
                        pfEsHTMLChunk *c2 = new pfEsHTMLChunk(chunk->fText.substr(lastChar));
                        c2->fFlags = chunk->fFlags;
                        fHTMLSource.emplace(fHTMLSource.begin() + idx + 1, c2);

                        // Clip and reallocate so we don't have two copies laying around
                        chunk->fText = chunk->fText.left(lastChar);

                        // Invalidate our cache starting with the next page
                        if (fPageStarts.size() > page + 1)
                            fPageStarts.resize(page + 1);

                        y += 512;
                        break;
                    }
                    
                    x = lastX;
                    y = (uint16_t)(lastY - dtMap->GetCurrFont()->GetAscent());    // Since our text is top-justified

                    if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) != pfEsHTMLChunk::kLeft )
                    {
                        // Ending X is not guaranteed to be anything useful if we're not left justified
                        x = (uint16_t)fPageLMargin;
                    }

                    break;

                case pfEsHTMLChunk::kImage:
                    {
                        plMipmap *mip = plMipmap::ConvertNoRef(chunk->fImageKey != nullptr ? chunk->fImageKey->ObjectIsLoaded() : nullptr);
                        if (mip != nullptr)
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
                                    y += (uint16_t)(mip->GetHeight());

                                    // Wonderful, the break breaks us from the switch(), which means the for()
                                    // loops runs once more and increments idx. So this is to counter that.
                                    // (We better check tho, just to make sure nobody feeds us an extra-large 
                                    // image and sends us on an infinite loop)
                                    if( idx > fPageStarts[ page ] )
                                        idx--;
                                    break;
                                }
                                if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kLeft )
                                    x = (uint16_t)fPageLMargin;
                                else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kRight )
                                    x = (uint16_t)(512 - fPageRMargin - mip->GetWidth());
                                else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kCenter )
                                    x = (uint16_t)(256 - ( mip->GetWidth() >> 1 ));

                                IDrawMipmap( chunk, x, y, mip, dtMap, whichDTMap, suppressRendering );

                                y += (uint16_t)(mip->GetHeight());
                                x = (uint16_t)fPageLMargin;
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
                    y = (uint16_t)(512 - fPageTMargin - fPageBMargin);
                    x = (uint16_t)fPageLMargin;
                    break;

                case pfEsHTMLChunk::kFontChange:                
                    IFindFontProps( idx, fontFace, fontSize, fontFlags, fontColor, fontSpacing );
                    dtMap->SetFont( fontFace, fontSize, fontFlags, false );
                    dtMap->SetTextColor( fontColor, true );
                    dtMap->SetLineSpacing(fontSpacing);
                    break;

                case pfEsHTMLChunk::kMovie: {
                    movieAlreadyLoaded = (IMovieAlreadyLoaded(chunk) != nullptr); // have we already cached it?
                    plLayerAVI *movieLayer = IMakeMovieLayer(chunk, x, y, (plMipmap*)dtMap, whichDTMap, suppressRendering);
                    if (movieLayer)
                    {
                        // adjust the starting height of the movie if we are keeping it inline with the text
                        uint32_t movieHeight = movieLayer->GetHeight();
                        if(!(chunk->fFlags & pfEsHTMLChunk::kFloating ))
                        {
                            if( y + movieHeight >= 512 - fPageBMargin )
                            {
                                // Movie overlaps the bottom of this page, so forcibly break so we'll
                                // end up marking the page break here (note that, unlike paragraphs, we
                                // can't really break the Movie into two)
                                y += (uint16_t)movieHeight;

                                // Wonderful, the break breaks us from the switch(), which means the for()
                                // loops runs once more and increments idx. So this is to counter that.
                                // (We better check tho, just to make sure nobody feeds us an extra-large 
                                // image and sends us on an infinite loop)
                                if( idx > fPageStarts[ page ] )
                                    idx--;
                                break;
                            }
                            y += (uint16_t)movieHeight;
                            x = (uint16_t)fPageLMargin;
                        }
                        if (!movieAlreadyLoaded) // if the movie wasn't already cached, cache it
                        {
                            movie = new loadedMovie;
                            movie->movieLayer = movieLayer; // save the layer and chunk data
                            movie->movieChunk = chunk;
                            fLoadedMovies.emplace_back(movie);
                            movie = nullptr;
                            movieAlreadyLoaded = false;
                        }
                        if (material && !suppressRendering)
                            material->AddLayerViaNotify(movieLayer);
                    }
                    break;
                }

                case pfEsHTMLChunk::kTextLink:
                    if (hsCheckBits(chunk->fFlags, pfEsHTMLChunk::kCanLink)) {
                        dtMap->SetTextColor(chunk->fColor, true);
                        currLinkChunk = chunk;
                    } else {
                        dtMap->SetTextColor(fontColor, true);
                        currLinkChunk = nullptr;
                    }
                    break;
            }
        }

        if (fPageStarts.size() <= page + 1)
            fPageStarts.resize(page + 2);
        fPageStarts[ page + 1 ] = idx;

        if (idx == fHTMLSource.size())
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

void    pfJournalBook::IMoveMovies( hsGMaterial *source, hsGMaterial *dest )
{
    std::vector<plLayerAVI*> moviesOnPage;
    if (source && dest)
    {
        // clear any exiting layers (movies) from the material and save them to our local array
        for (size_t i = 0; i < source->GetNumLayers(); i++) // remove all plLayerMovie layers
        {
            plLayerInterface *matLayer = source->GetLayer(i);
            plLayerAVI *movie = plLayerAVI::ConvertNoRef(matLayer);
            if (movie) // if it was a movie layer
            {
                plMatRefMsg* refMsg = new plMatRefMsg(source->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer); // remove it
                hsgResMgr::ResMgr()->SendRef(source->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
                moviesOnPage.emplace_back(movie);
            }
        }
        // clear the destination's movies (if it has any)
        for (size_t i = 0; i < dest->GetNumLayers(); i++) // remove all plLayerMovie layers
        {
            plLayerInterface *matLayer = dest->GetLayer(i);
            plLayerAVI *movie = plLayerAVI::ConvertNoRef(matLayer);
            if (movie) // if it was a movie layer
            {
                plMatRefMsg* refMsg = new plMatRefMsg(dest->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer); // remove it
                hsgResMgr::ResMgr()->SendRef(dest->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
            }
        }
        // put the movies we ripped off the old page onto the new one
        for (plLayerAVI* movie : moviesOnPage)
            dest->AddLayerViaNotify(movie);
    }
}

//// IDrawMipmap /////////////////////////////////////////////////////////////

void    pfJournalBook::IDrawMipmap( pfEsHTMLChunk *chunk, uint16_t x, uint16_t y, plMipmap *mip, plDynamicTextMap *dtMap, uint32_t whichDTMap, bool dontRender )
{
    hsRef<plMipmap> copy(new plMipmap(), hsStealRef);
    copy->CopyFrom(mip);
    if (chunk->fNoResizeImg)
    {
        // book is NOT square, there is a h/w ratio of 1/0.7
        // calc new size based on how the book has been skewed
        float xScale = (fWidthScale == 0) ? 1 : 1/(1-fWidthScale);
        float yScale = (fHeightScale == 0) ? 1 : 1/(1-fHeightScale);
        yScale *= 0.7f; // adjust because the book isn't square
        uint32_t width = (uint32_t)(mip->GetWidth()*xScale);
        uint32_t height = (uint32_t)(mip->GetHeight()*yScale);
        uint16_t xShift;
        uint16_t yShift;

        if (dtMap->GetWidth() < width) width = dtMap->GetWidth();
        if (dtMap->GetHeight() < height) height = dtMap->GetHeight();

        if (height < mip->GetHeight())
        {
            yShift = (uint16_t)((mip->GetHeight()-height)/2);
            if (y+yShift+height > dtMap->GetHeight())
                y = (uint16_t)(dtMap->GetHeight()-height);
            else
                y += yShift;
        }
        else
        {
            yShift = (uint16_t)((height-mip->GetHeight())/2);
            if (yShift > y)
                y = 0;
            else
                y -= yShift;
        }

        if (width < mip->GetWidth())
        {
            xShift = (uint16_t)((mip->GetWidth()-width)/2);
            if (x+xShift+width > dtMap->GetWidth())
                x = (uint16_t)(dtMap->GetWidth()-width);
            else
                x += xShift;
        }
        else
        {
            xShift = (uint16_t)((width-mip->GetWidth())/2);
            if (xShift > x)
                x = 0;
            else
                x -= xShift;
        }
        
        copy->SetCurrLevel(0); // resize the image so it will look unchanged when rendered on the altered book
        copy->ResizeNicely((uint16_t)width,(uint16_t)height,plMipmap::kDefaultFilter);
    }
    if( !dontRender )
    {
        plMipmap::CompositeOptions  opts;
        if( chunk->fFlags & pfEsHTMLChunk::kActAsCB )
        {
            opts.fFlags = ( chunk->fFlags & pfEsHTMLChunk::kBlendAlpha ) ? 0 : plMipmap::kBlendWriteAlpha;
            opts.fRedTint = chunk->fCurrColor.r;
            opts.fGreenTint = chunk->fCurrColor.g;
            opts.fBlueTint = chunk->fCurrColor.b;
            opts.fOpacity = (uint8_t)(chunk->fCurrColor.a * 255.f);
        }
        else 
        {
            if( chunk->fFlags & pfEsHTMLChunk::kGlowing )
                opts.fFlags = ( chunk->fFlags & pfEsHTMLChunk::kBlendAlpha ) ? 0 : plMipmap::kMaskSrcAlpha;
            else if (chunk->fFlags & pfEsHTMLChunk::kTranslucent)
                opts.fFlags = plMipmap::kMaskSrcAlpha;
            else
                opts.fFlags = ( chunk->fFlags & pfEsHTMLChunk::kBlendAlpha ) ? plMipmap::kCopySrcAlpha : plMipmap::kForceOpaque;
            opts.fOpacity = (uint8_t)(chunk->fCurrOpacity * 255.f);
        }
        dtMap->Composite(copy.Get(), x, y, &opts);
    }

    if( chunk->fFlags & pfEsHTMLChunk::kCanLink )
    {
        int16_t xOffs = 0;
        if( whichDTMap == pfJournalDlgProc::kTagRightDTMap || whichDTMap == pfJournalDlgProc::kTagTurnFrontDTMap )
            xOffs = (int16_t)(dtMap->GetWidth());   // Right page rects are offsetted to differentiate

        // if we aren't rendering then this link isn't visible, but the index still needs to be valid, so give it a rect of 0,0,0,0
        if (dontRender) {
            fVisibleLinks.emplace_back(chunk, 0, 0, 0, 0);
        } else {
            fVisibleLinks.emplace_back(chunk, x + xOffs, y, (int16_t)(copy->GetWidth()), (int16_t)(copy->GetHeight()));
            if (s_ShowLinkRects)
                dtMap->FrameRect(x, y, (int16_t)(copy->GetWidth()), (int16_t)(copy->GetHeight()), s_LinkRectColor);
        }
    }
}

pfJournalBook::loadedMovie *pfJournalBook::IMovieAlreadyLoaded(pfEsHTMLChunk *chunk)
{
    for (loadedMovie* lm : fLoadedMovies) // filename and id# must both match
    {
        if ((chunk->fText == lm->movieChunk->fText) && (chunk->fMovieIndex == lm->movieChunk->fMovieIndex))
            return lm;
    }
    return nullptr;
}

plKey pfJournalBook::GetMovie(uint8_t index)
{
    loadedMovie *movie = IGetMovieByIndex(index);
    if (movie)
        return movie->movieLayer->GetKey();
    return plKey();
}

pfJournalBook::loadedMovie *pfJournalBook::IGetMovieByIndex(uint8_t index)
{
    for (loadedMovie* lm : fLoadedMovies)
    {
        if (lm->movieChunk->fMovieIndex == index)
            return lm;
    }
    return nullptr;
}

plLayerAVI *pfJournalBook::IMakeMovieLayer(pfEsHTMLChunk *chunk, uint16_t x, uint16_t y, plMipmap *baseMipmap, uint32_t whichDTMap, bool dontRender)
{
    // see if it's already loaded
    loadedMovie *movie = IMovieAlreadyLoaded(chunk);
    plLayer* layer = nullptr;
    plLayerAVI* movieLayer = nullptr;
    uint16_t movieWidth=0,movieHeight=0;
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
        ST::string buff;

        buff = ST::format("{}_{}_ml", GetKey()->GetName(), uniqueSuffix);
        layer = new plLayer;
        hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

        buff = ST::format("{}_{}_m", GetKey()->GetName(), uniqueSuffix++);
        movieLayer = new plLayerAVI;
        hsgResMgr::ResMgr()->NewKey(buff, movieLayer, GetKey()->GetUoid().GetLocation());
        movieLayer->GetKey()->RefObject(); // we want to own a ref so we can nuke it at will

        movieLayer->AttachViaNotify(layer);

        // Initialize it.
        movieLayer->SetMovieName(chunk->fText);
        movieLayer->Eval(0,0,0); // set up the movie

        movieWidth = movieLayer->GetWidth();
        movieHeight = movieLayer->GetHeight();

        if (movieHeight == 0 || movieWidth == 0) // problem loading the file
        {
            movieLayer->GetKey()->UnRefObject();
            return nullptr;
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
            x = (uint16_t)fPageLMargin;
        else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kRight )
            x = (uint16_t)(baseMipmap->GetWidth() - fPageRMargin - movieWidth);
        else if( ( chunk->fFlags & pfEsHTMLChunk::kAlignMask ) == pfEsHTMLChunk::kCenter )
            x = (uint16_t)((baseMipmap->GetWidth() >> 1) - (movieWidth >> 1));

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
            yScale *= 1.f/0.7f; // adjust because the book isn't square
        }

        hsVector3 scaleVec(xScale, yScale, 1.f);
        hsVector3 translateVec(-(xRel*xScale), -(yRel*yScale), 0.f);
        hsMatrix44 scaleMat, translateMat;
        scaleMat.MakeScaleMat(&scaleVec);
        translateMat.MakeTranslateMat(&translateVec);

        hsMatrix44 flipMat;
        if (chunk->fOnCover) // cover movies need to be y flipped
        {
            hsVector3 yTransVec(0.f, -1.f, 0.f);
            hsVector3 invertYVec(1.f, -1.f, 1.f);
            hsMatrix44 invertY, transY;
            invertY.MakeScaleMat(&invertYVec);
            transY.MakeTranslateMat(&yTransVec);
            flipMat = invertY * transY;
        }
        else // left page movies need to be x flipped
        {
            if ((whichDTMap == pfJournalDlgProc::kTagLeftDTMap) || (whichDTMap == pfJournalDlgProc::kTagTurnBackDTMap))
            {
                hsVector3 xTransVec(-1.f, 0.f, 0.f);
                hsVector3 invertXVec(-1.f, 1.f, 1.f);
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
            x += (uint16_t)(baseMipmap->GetWidth());  // Right page rects are offsetted to differentiate

        // if we aren't rendering then this link isn't visible, but the index still needs to be valid, so give it a rect of 0,0,0,0
        if (dontRender) {
            fVisibleLinks.emplace_back(chunk, 0, 0, 0, 0);
        } else {
            fVisibleLinks.emplace_back(chunk, x, y, movieWidth, movieHeight);
        }
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
    ST::string buff = ST::format("{}_{}", GetKey()->GetName(), uniqueSuffix++);

    plLayer* layer = new plLayer;
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
    hsVector3 yTransVec(0.f, -1.f, 0.f);
    hsVector3 invertYVec(1.f, -1.f, 1.f);
    hsMatrix44 xfm, invertY, transY, flipY;
    invertY.MakeScaleMat(&invertYVec);
    transY.MakeTranslateMat(&yTransVec);
    flipY = invertY * transY;
    xfm = flipY;

    layer->SetTransform(xfm);

    // Set the texture (assumes mipmap is non-nil).
    plLayRefMsg* refMsg = new plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
    hsgResMgr::ResMgr()->SendRef(image->GetKey(), refMsg, plRefFlags::kActiveRef);

    return plLayerInterface::ConvertNoRef(layer);
}

plLayerInterface *pfJournalBook::IMakeDecalLayer(pfEsHTMLChunk *decalChunk, plMipmap *decal, plMipmap *baseMipmap)
{
    // Create the layer and register it.

    // We'll need a unique name. This is a hack, but an effective hack.
    static int uniqueSuffix = 0;
    ST::string buff = ST::format("{}_{}_d", GetKey()->GetName(), uniqueSuffix++);

    plLayer* layer = new plLayer;
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
    uint16_t x,y;
    x = decalChunk->fAbsoluteX;
    y = decalChunk->fAbsoluteY;

    if((decalChunk->fFlags & pfEsHTMLChunk::kAlignMask) == pfEsHTMLChunk::kLeft)
        x = (uint16_t)fPageLMargin;
    else if((decalChunk->fFlags & pfEsHTMLChunk::kAlignMask) == pfEsHTMLChunk::kRight)
        x = (uint16_t)(baseMipmap->GetWidth() - decal->GetWidth());
    else if((decalChunk->fFlags & pfEsHTMLChunk::kAlignMask) == pfEsHTMLChunk::kCenter)
        x = (uint16_t)((baseMipmap->GetWidth() >> 1) - (decal->GetWidth() >> 1));

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
        yScale *= 1.f/0.7f; // adjust because the book isn't square
    }

    hsVector3 scaleVec(xScale, yScale, 1.f);
    hsVector3 translateVec(-(xRel*xScale), -(yRel*yScale), 0.f);
    hsVector3 yTransVec(0.f, -1.f, 0.f);
    hsVector3 invertYVec(1.f, -1.f, 1.f);
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
    plLayRefMsg* refMsg = new plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
    hsgResMgr::ResMgr()->SendRef(decal->GetKey(), refMsg, plRefFlags::kActiveRef);

    return plLayerInterface::ConvertNoRef(layer);
}

void pfJournalBook::ISetDecalLayers(hsGMaterial *material, const std::vector<plLayerInterface*> &layers)
{
    // First, clear out the existing layers.
    for (hsSsize_t i = material->GetNumLayers() - 1; i >= 0; i--)
    {
        plMatRefMsg* refMsg = new plMatRefMsg(material->GetKey(), plRefMsg::kOnRemove, (int32_t)i, plMatRefMsg::kLayer);
        hsgResMgr::ResMgr()->SendRef(material->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
    }

    // Now append our new layers in order.
    for (plLayerInterface* layer : layers)
        material->AddLayerViaNotify(layer);
}

//// IFindFontProps //////////////////////////////////////////////////////////
// Starting at the given chunk, works backwards to determine the full set of current
// font properties at that point, or assigns defaults if none were specified

void    pfJournalBook::IFindFontProps( uint32_t chunkIdx, ST::string &face, uint8_t &size, uint8_t &flags, hsColorRGBA &color, int16_t &spacing )
{
    enum Which
    {
        kFace   = 0x01,
        kSize   = 0x02,
        kFlags  = 0x04,
        kColor  = 0x08,
        kSpacing= 0x10,

        kAllFound = kFace | kSize | kFlags | kColor | kSpacing
    };

    // Start empty
    uint8_t   found = 0;

    // Work backwards and fill in our properties
    chunkIdx++;
    do
    {
        chunkIdx--;
        if (fHTMLSource.size() <= chunkIdx)
            break; // apparently it's sometimes possible for fHTMLSource to be empty (parse errors?)
        pfEsHTMLChunk *chunk = fHTMLSource[ chunkIdx ];

        if( chunk->fType == pfEsHTMLChunk::kFontChange )
        {
            // What do we (still) need?
            if( !( found & kFace ) && !chunk->fText.empty() )
            {
                face = chunk->fText;
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
        face = "Arial";
    if( !( found & kSize ) )
        size = 24;
    if( !( found & kFlags ) )
        flags = 0;
    if( !( found & kColor ) )
        color.Set( 0.f, 0.f, 0.f, 1.f );
    if( !( found & kSpacing ) )
        spacing = 0;
}

//// IFindTextLink ///////////////////////////////////////////////////////////
// Starting at the given chunk, works backward to determine the current text link ID
pfEsHTMLChunk* pfJournalBook::IFindTextLink(uint32_t chunkIdx) const
{
    if (fHTMLSource.empty())
        return nullptr;

    for (hsSsize_t i = chunkIdx; i >= 0; --i) {
        if (fHTMLSource[i]->fType != pfEsHTMLChunk::kTextLink)
            continue;
        if (hsCheckBits(fHTMLSource[i]->fFlags, pfEsHTMLChunk::kCanLink))
            return fHTMLSource[i];
        else
            return nullptr;
    }

    return nullptr;
}

//// IFindLastAlignment //////////////////////////////////////////////////////
// Find the last paragraph chunk and thus the last par alignment settings

uint8_t   pfJournalBook::IFindLastAlignment() const
{
    for (auto iter = fHTMLSource.crbegin(); iter != fHTMLSource.crend(); ++iter)
    {
        const pfEsHTMLChunk* chunk = *iter;
        if (chunk->fType == pfEsHTMLChunk::kParagraph && chunk->fFlags != 0)
            return (uint8_t)(chunk->fFlags);
    }

    return pfEsHTMLChunk::kLeft;
}

//// IRecalcPageStarts ///////////////////////////////////////////////////////
// Ensures that all the page starts are calced up to the given page (but not including it)

void    pfJournalBook::IRecalcPageStarts( uint32_t upToPage )
{
    // Well, sadly, we can't really calc the page starts without at least a DTMap
    // we can change things on...so we just pick one and render. Note: this WILL
    // trash the font settings on the given DTMap!

    // We assume that the stored page starts we already have are accurate, so
    // just start from there and calc onward

    for (uint32_t page = fPageStarts.size() - 1; page < upToPage && page <= fLastPage; page++)
    {
        // normally we would surpress rendering the pages, but that seems to have a bug in it
        // that causes lost text that the rendering doesn't have. Since it isn't very costly to
        // actually draw them all (even in large journals), we're just going to do it
        IRenderPage( page, pfJournalDlgProc::kTagTurnBackDTMap, false );
        // Reset any "visible" links since they aren't really visible
        for (auto& link : fVisibleLinks)
            link.ClearRect();
    }
}

//// ISendNotify /////////////////////////////////////////////////////////////
// Just sends out a notify to our currently set receiver key

void    pfJournalBook::ISendNotify( uint32_t type, uint32_t linkID )
{
    if (fCallbackKey != nullptr)
    {
        plNotifyMsg *pMsg = new plNotifyMsg;
        pMsg->AddBookEvent( type, linkID );
        pMsg->SetBCastFlag( plMessage::kNetPropagate, false ); // don't deliver networked!
        pMsg->Send( fCallbackKey );
    }
}

//// SetBookSize /////////////////////////////////////////////////////////////
// Sets the book size scaling. 1,1 would be full size, 0,0 is the smallest size possible
// Note: internally we store these as the seek positions on our animations,
// so the incoming parameters are actually inverse of what we finally want

void    pfJournalBook::SetBookSize( float width, float height )
{
    fWidthScale = 1.f - width;
    fHeightScale = 1.f - height;

    if( fBookGUIs[fCurBookGUI]->CurBook() == this )
        fBookGUIs[fCurBookGUI]->SetCurrSize( fWidthScale, fHeightScale );
}

//// ILoadAllImages //////////////////////////////////////////////////////////
// Load (or unload) all the images for the book

void    pfJournalBook::ILoadAllImages( bool unload )
{
    // load the cover
    if (fCoverFromHTML && fCoverMipKey != nullptr)
    {
        if( unload )
            fBookGUIs[fCurBookGUI]->GetKey()->Release( fCoverMipKey );
        else
        {
            plGenRefMsg *ref = new plGenRefMsg( fBookGUIs[fCurBookGUI]->GetKey(), plRefMsg::kOnCreate, -1, kRefImage );
            hsgResMgr::ResMgr()->AddViaNotify( fCoverMipKey, ref, plRefFlags::kActiveRef );
        }
    }

    for (pfEsHTMLChunk* chunk : fHTMLSource)
    {
        if (chunk->fType == pfEsHTMLChunk::kImage && chunk->fImageKey != nullptr)
        {
            if( unload )
                fBookGUIs[fCurBookGUI]->GetKey()->Release(chunk->fImageKey);
            else
            {
                plGenRefMsg *ref = new plGenRefMsg( fBookGUIs[fCurBookGUI]->GetKey(), plRefMsg::kOnCreate, -1, kRefImage );  
                hsgResMgr::ResMgr()->AddViaNotify(chunk->fImageKey, ref, plRefFlags::kActiveRef);
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

ST::string pfJournalBook::GetEditableText()
{
    pfGUIMultiLineEditCtrl *left = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagLeftEditCtrl );
    if (left)
    {
        return left->GetNonCodedBuffer();
    }
    return {};
}

void pfJournalBook::SetEditableText(const ST::string& text)
{
    pfGUIMultiLineEditCtrl *left = fBookGUIs[fCurBookGUI]->GetEditCtrl( pfJournalDlgProc::kTagLeftEditCtrl );
    if (left)
    {
        left->SetBuffer(text);
        left->ForceUpdate();
    }
}
