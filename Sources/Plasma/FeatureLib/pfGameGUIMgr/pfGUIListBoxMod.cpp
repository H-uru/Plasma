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
//  pfGUIListBoxMod Definition                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pfGUIListBoxMod.h"
#include "pfGUIListElement.h"
#include "pfGameGUIMgr.h"
#include "pfGUIUpDownPairMod.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogMod.h"

#include "pnMessage/plRefMsg.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plAvatar/plAGModifier.h"
#include "plGImage/plDynamicTextMap.h"
#include "plInputCore/plInputInterface.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


#define kIndentAmount       16


//// plSmallRect Stuff ///////////////////////////////////////////////////////

void    pfGUIListBoxMod::plSmallRect::Set( int16_t l, int16_t t, int16_t r, int16_t b ) 
{
    fLeft = l;
    fTop = t;
    fRight = r; 
    fBottom = b; 
}

bool    pfGUIListBoxMod::plSmallRect::Contains( int16_t x, int16_t y )
{
    if( x < fLeft || x > fRight || y < fTop || y > fBottom )
        return false;

    return true;
}

//// Wee Little Control Proc for scrolling ///////////////////////////////////

class pfScrollProc : public pfGUICtrlProcObject
{
    protected:

        pfGUIListBoxMod *fParent;

    public:

        pfScrollProc( pfGUIListBoxMod *parent ) : fParent( parent ) {}

        virtual void    DoSomething( pfGUIControlMod *ctrl )
        {
            // Do a check here to make sure we actually changed scroll
            // positions--if not, we don't want to update, since that'll be 
            // slow as hell
            int newScrollPos = (int)fParent->fScrollControl->GetMax() - (int)fParent->fScrollControl->GetCurrValue();
            if( newScrollPos != fParent->fScrollPos )
            {
                fParent->IUpdate();
                fParent->HandleExtendedEvent( pfGUIListBoxMod::kScrollPosChanged );
            }
        }
};

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIListBoxMod::pfGUIListBoxMod()
{
    SetFlag( kWantsInterest );
    fCurrClick = -1;
    fCurrHover = -1;
    fModsAtDragTime = 0;
    fScrollControl = nil;
    fCheckScroll = false;
    fSingleSelElement = -1;
    fScrollRangeUpdateDeferred = false;
    fScrollPos = 0;
    fLocked = false;
    fReadyToRoll = false;
    fClicking = false;

    fScrollProc = nil;
}

pfGUIListBoxMod::~pfGUIListBoxMod()
{
    ClearAllElements();

    if( fScrollProc && fScrollProc->DecRef() )
        delete fScrollProc;
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIListBoxMod::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIListBoxMod::MsgReceive( plMessage *msg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if( refMsg != nil )
    {
        if( refMsg->fType == kRefScrollCtrl )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fScrollControl = pfGUIValueCtrl::ConvertNoRef( refMsg->GetRef() );
                fScrollControl->SetHandler( fScrollProc );
                fScrollControl->SetStep( 1.f );
                ICalcScrollRange();
            }
            else
                fScrollControl = nil;
            return true;
        }
        else if( refMsg->fType == kRefDynTextMap )
        {
            // Capture this so we can reset our flag, but do NOT just return from it!
            if( !( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) ) )
                fReadyToRoll = false;
        }
    }

    return pfGUIControlMod::MsgReceive( msg );
}

//// IPostSetUpDynTextMap ////////////////////////////////////////////////////

void    pfGUIListBoxMod::IPostSetUpDynTextMap( void )
{
    pfGUIColorScheme *scheme = GetColorScheme();
    fDynTextMap->SetFont( scheme->fFontFace, scheme->fFontSize, scheme->fFontFlags, 
                            !HasFlag( kXparentBgnd ));

    ICalcWrapStarts();
    ICalcScrollRange();
    fReadyToRoll = true;
}

//// IUpdate /////////////////////////////////////////////////////////////////

void    pfGUIListBoxMod::IUpdate( void )
{
    int         i, j;
    uint16_t      x, y, width, height, maxHeight;


    if( !fReadyToRoll )
        return;
    if( fScrollRangeUpdateDeferred )
    {
        fScrollRangeUpdateDeferred = false;
        ICalcScrollRange();
    }

    fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );
    if( fElements.GetCount() == 0 )
    {
        fDynTextMap->FlushToHost();
        return;
    }

    if( fLocked )
    {
        hsStatusMessage( "WARNING: Forcing unlock on GUI list box due to call to IUpdate()\n" );
        UnlockList();
    }

    if( fScrollControl != nil )
    {
        // We reverse the value, since we want the "up" button to scroll "up", which actually
        // *decreases* the scrollPos
        fScrollPos = (int)fScrollControl->GetMax() - (int)fScrollControl->GetCurrValue();

        if( fCheckScroll )
        {
            // If this is set, we just did something to change the selection to a single item.
            // Since it makes sense to try to ensure this item is on the screen, we thus do so here.
            // We don't want to do this every time, because we could be simply clicking on the
            // scroll bars, in which case we DON'T want to do this check, since it would be
            // fighting against the user, which is a big UI design no-no.
            // To do this check, we search on either side of our scroll range (we can only
            // search past after we draw, unfortunately). If there's a selected item out
            // of our range, then we best move the scrollPos to move it into view
            for( j = 0; j < fScrollPos; j++ )
            {
                int end = ( j < fWrapStartIdxs.GetCount() - 1 ) ? fWrapStartIdxs[ j + 1 ] : fElements.GetCount();

                bool anySelected = false;
                for( i = fWrapStartIdxs[ j ]; i < end; i++ )
                    anySelected |= fElements[ i ]->IsSelected();

                if( anySelected )
                {
                    // Shit. Move the scrollPos up to this item at the very least
                    fScrollPos = j;
                    fScrollControl->SetCurrValue( (float)( (int)fScrollControl->GetMax() - fScrollPos ) );
                    fCheckScroll = false;
                    break;
                }
            }
        }
    }
    else
        fScrollPos = 0;

    fElementBounds.SetCountAndZero( fElements.GetCount() );

    if( !HasFlag( kScrollLeftToRight ) )
    {
        for( j = fScrollPos, y = 4; j < fWrapStartIdxs.GetCount() && y < fDynTextMap->GetVisibleHeight() - 8; j++ )
        {
            int end = ( j < fWrapStartIdxs.GetCount() - 1 ) ? fWrapStartIdxs[ j + 1 ] : fElements.GetCount();

            for( maxHeight = 0, i = fWrapStartIdxs[ j ], x = 0; i < end; i++ )
            {
                if( HasFlag( kGrowLeavesAndProcessOxygen ) && fElements[ i ]->IsCollapsed() )
                    continue;       // Skip collapsed items

                fElements[ i ]->GetSize( fDynTextMap, &width, &height );

                if( !HasFlag( kAllowMultipleElementsPerRow ) )
                    width = fDynTextMap->GetVisibleWidth();

                if( y + height >= fDynTextMap->GetVisibleHeight() - 8 )
                    height = fDynTextMap->GetVisibleHeight() - 8 - y;

                if( HasFlag( kGrowLeavesAndProcessOxygen ) )
                {
                    x += fElements[ i ]->GetIndentLevel() * kIndentAmount;
                    if( x + width > fDynTextMap->GetVisibleWidth() )
                        width = fDynTextMap->GetVisibleWidth() - x;
                }

                if( !fElements[ i ]->Draw( fDynTextMap, x, y, width, height ) )
                {
                    // Couldn't draw, so force it to be at the end of the scroll range
                    y = fDynTextMap->GetVisibleHeight() - 8;
                    j--;
                    break;
                }

                maxHeight = ( height > maxHeight ) ? height : maxHeight;
                fElementBounds[ i ].Set( x, y, x + width - 1, y + height - 1 );
                x += width;
            }

            y += maxHeight;
        }
    }
    else
    {
        for( j = fScrollPos, x = 4; j < fWrapStartIdxs.GetCount() && x < fDynTextMap->GetVisibleWidth() - 8; j++ )
        {
            int end = ( j < fWrapStartIdxs.GetCount() - 1 ) ? fWrapStartIdxs[ j + 1 ] : fElements.GetCount();

            for( maxHeight = 0, i = fWrapStartIdxs[ j ], y = 0; i < end; i++ )
            {
                fElements[ i ]->GetSize( fDynTextMap, &width, &height );

                if( !HasFlag( kAllowMultipleElementsPerRow ) )
                    height = fDynTextMap->GetVisibleHeight();

                if( x + width >= fDynTextMap->GetVisibleWidth() - 8 )
                    width = fDynTextMap->GetVisibleWidth() - 8 - x;

                if( !fElements[ i ]->Draw( fDynTextMap, x, y, width, height ) )
                {
                    // Couldn't draw, so force it to be at the end of the scroll range
                    x = fDynTextMap->GetVisibleWidth() - 8;
                    j--;
                    break;
                }

                maxHeight = ( width > maxHeight ) ? width : maxHeight;
                fElementBounds[ i ].Set( x, y, x + width - 1, y + height - 1 );
                y += height;
            }

            x += maxHeight;
        }
    }

    // Second part of our scroll check--if we got here, then there was nothing selected
    // before the viewing area. So check the rest
    if( fCheckScroll && fScrollControl != nil )
    {
        fCheckScroll = false;
        for( ; j < fWrapStartIdxs.GetCount(); j++ )
        {
            int end = ( j < fWrapStartIdxs.GetCount() - 1 ) ? fWrapStartIdxs[ j + 1 ] : fElements.GetCount();

            bool anySelected = false;
            for( i = fWrapStartIdxs[ j ]; i < end; i++ )
                anySelected |= fElements[ i ]->IsSelected();

            if( anySelected )
            {
                fScrollPos = j;
                fScrollControl->SetCurrValue( (float)( (int)fScrollControl->GetMax() - fScrollPos ) );
                IUpdate();      // Gotta update again, since we just changed the scrollPos after the fact
                return;
            }
        }
    }

    fDynTextMap->FlushToHost();
}

//// ICalcWrapStarts /////////////////////////////////////////////////////////
//  Calculates the starting indexes for each row of items. Call whenever you
//  update the element list.

void    pfGUIListBoxMod::ICalcWrapStarts( void )
{
    uint16_t      i, x, y, maxHeight, width, height;


    fWrapStartIdxs.Reset();

    if( HasFlag( kAllowMultipleElementsPerRow ) )
    {
        if( !HasFlag( kScrollLeftToRight ) )
        {
            for( i = 0, x = (uint16_t)-1, y = 4, maxHeight = 0; i < fElements.GetCount(); i++ )
            {
                fElements[ i ]->GetSize( fDynTextMap, &width, &height );

                if( x + width >= fDynTextMap->GetVisibleWidth() )
                {
                    x = 0;
                    y += maxHeight;
                    maxHeight = 0;
                    fWrapStartIdxs.Append( i );
                }
                x += width;
                maxHeight = ( height > maxHeight ) ? height : maxHeight;
            }
        }
        else
        {
            for( i = 0, y = (uint16_t)-1, x = 4, maxHeight = 0; i < fElements.GetCount(); i++ )
            {
                fElements[ i ]->GetSize( fDynTextMap, &width, &height );
                if( y + height >= fDynTextMap->GetVisibleHeight() )
                {
                    y = 0;
                    x += maxHeight;
                    maxHeight = 0;
                    fWrapStartIdxs.Append( i );
                }
                y += height;
                maxHeight = ( width > maxHeight ) ? width : maxHeight;
            }
        }
    }
    else
    {
        for( i = 0; i < fElements.GetCount(); i++ )
        {
            if( HasFlag( kGrowLeavesAndProcessOxygen ) && fElements[ i ]->IsCollapsed() )
                continue;

            fWrapStartIdxs.Append( i );
        }
    }
}

//// ICalcScrollRange ////////////////////////////////////////////////////////
//  Call whenever you update the element list

void    pfGUIListBoxMod::ICalcScrollRange( void )
{
    uint16_t      ctrlHt, ctrlWd, height, width, maxHeight;
    int         i, j, end;


    if( !fReadyToRoll )
    {
        fScrollRangeUpdateDeferred = true;
        return;
    }

    // Basically, the scroll range is 0 to (numElements-1), but since if we scroll
    // that far, we won't see anything left except the last element, we have to calculate
    // how many elements away from the end we can get before we see blank space. Which means
    // counting up from the end until we have passed the height of our control.

    end = fElements.GetCount();
    if( !HasFlag( kScrollLeftToRight ) )
    {
        if( HasFlag( kGrowLeavesAndProcessOxygen ) )
        {
            // OK, that really isn't the end, the end will the count of non-collapsed elements. Haha.
            for( i = 0, end = 0; i < fElements.GetCount(); i++ )
            {
                if( !fElements[ i ]->IsCollapsed() )
                    end++;
            }
        }

        for( i = fWrapStartIdxs.GetCount() - 1, height = 0; i >= 0; i-- )
        {
            // Get the max height of this row
            maxHeight = 0;
            for( j = fWrapStartIdxs[ i ]; j < end; j++ )
            {
                fElements[ j ]->GetSize( fDynTextMap, &width, &ctrlHt );
                maxHeight = ( ctrlHt > maxHeight ) ? ctrlHt : maxHeight;
            }
            end = fWrapStartIdxs[ i ];
            height += maxHeight;

            if( height > fDynTextMap->GetVisibleHeight() - 8 )
                break;
        }
    }
    else
    {
        for( i = fWrapStartIdxs.GetCount() - 1, width = 0; i >= 0; i-- )
        {
            // Get the max width of this column
            maxHeight = 0;
            for( j = fWrapStartIdxs[ i ]; j < end; j++ )
            {
                fElements[ j ]->GetSize( fDynTextMap, &ctrlWd, &ctrlHt );
                maxHeight = ( ctrlWd > maxHeight ) ? ctrlWd : maxHeight;
            }
            end = fWrapStartIdxs[ i ];
            width += maxHeight;

            if( width > fDynTextMap->GetVisibleWidth() - 8 )
                break;
        }
    }

    // Note: i went one past, so our scroll range is really 0 to i+1
    if( fScrollControl )
    {
        // Because we reverse the position on draw, we have to do some special trick here
        // to make sure the reversed position stays the same
        fScrollPos = (int)fScrollControl->GetMax() - (int)fScrollControl->GetCurrValue();
        if( fScrollPos >= fWrapStartIdxs.GetCount() )
            fScrollPos = fWrapStartIdxs.GetCount() - 1;

        if( i < 0 )
            // Smaller than the viewing area, so we don't scroll at all
            fScrollControl->SetRange( 0.f, 0.f );
        else
            fScrollControl->SetRange( 0.f, (float)( i + 1 ) );

        fScrollControl->SetCurrValue( (float)( (int)fScrollControl->GetMax() - fScrollPos ) );
    }
}

void pfGUIListBoxMod::PurgeDynaTextMapImage()
{
    if ( fDynTextMap != nil )
        fDynTextMap->PurgeImage();
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIListBoxMod::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);

    fScrollControl = nil;
    if( s->ReadBool() )
    {
        fScrollProc = new pfScrollProc( this );
        fScrollProc->IncRef();
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefScrollCtrl ), plRefFlags::kActiveRef );
    }

    if( HasFlag( kDisableSelection ) )
        ClearFlag( kWantsInterest );

}

void    pfGUIListBoxMod::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Write( s, mgr );

    if( fScrollControl != nil )
    {
        s->WriteBool( true );
        mgr->WriteKey( s, fScrollControl->GetKey() );
    }
    else
        s->WriteBool( false );
}

//// FilterMousePosition /////////////////////////////////////////////////////
//  Tests the mouse point and decides whether we still want to accept input
//  based on the position. This allows us to act etheral (i.e. pass mouse
//  messages through) when the mouse is over an empty portion of the list.

bool    pfGUIListBoxMod::FilterMousePosition( hsPoint3 &mousePt )
{
    if( !HasFlag( kAllowMousePassThrough ) )
        return true;

    int32_t hover = fCurrClick = IGetItemFromPoint( mousePt );
    if( hover == -1 )
        return false;

    return true;
}

//// HandleMouseDown /////////////////////////////////////////////////////////
//  What we do: normal click deselects all and selects the item clicked on
//  (if any). Shift-click and ctrl-click avoids the deselect and toggles
//  the item clicked on.

void    pfGUIListBoxMod::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    int32_t   i;

    int lastSelection = -1;
    if (HasFlag(kForbidNoSelection))
    {
        // grab the last item we had selected just in case they clicked outside the list of selectable objects
        for (i=0; i<fElements.GetCount(); i++)
            if (fElements[i]->IsSelected())
            {
                lastSelection = i;
                break;
            }
    }
    
    if( HasFlag(kSingleSelect) || ( !( modifiers & ( pfGameGUIMgr::kShiftDown | pfGameGUIMgr::kCtrlDown ) ) && !HasFlag( kHandsOffMultiSelect ) ) )
    {
        // Deselect everyone!
        for( i = 0; i < fElements.GetCount(); i++ )
            fElements[ i ]->SetSelected( false );
        fSingleSelElement = -1;
    }
    else if( modifiers & pfGameGUIMgr::kShiftDown )
    {
        IFindSelectionRange( &fMinSel, &fMaxSel );
    }

    fCurrHover = fCurrClick = IGetItemFromPoint( mousePt );
    if( fCurrClick != -1 )
    {
        if( ( ( modifiers & pfGameGUIMgr::kShiftDown ) && !HasFlag( kSingleSelect ) ) )
        {
            // Select our new range, deselect everything outside
            if( fCurrClick <= fMaxSel )
            {
                ISelectRange( 0, (int8_t)(fCurrClick - 1), false );
                ISelectRange( (int8_t)fCurrClick, (int8_t)fMaxSel, true );
                ISelectRange( (int8_t)(fMaxSel + 1), -1, false );
            }
            else if( fCurrClick >= fMinSel )
            {
                ISelectRange( 0, (int8_t)(fMinSel - 1), false );
                ISelectRange( (int8_t)fMinSel, (int8_t)fCurrClick, true );
                ISelectRange( (int8_t)(fCurrClick + 1), -1, false );
            }
            fElements[ fCurrClick ]->SetSelected( true );
        }
        else
        {
            fElements[ fCurrClick ]->SetSelected( true );
            fSingleSelElement = fCurrClick;
        }
    }
    else
    {
        if (HasFlag(kForbidNoSelection) && (lastSelection != -1))
        {
            fElements[ lastSelection ]->SetSelected(true);
            fSingleSelElement = lastSelection;
        }
    }

    // We want drag/up events to be processed with the modifiers we had on
    // mouse down, not the current ones. So store 'em for reference
    fModsAtDragTime = modifiers;
    fClicking = true;
    IUpdate();
}

//// HandleMouseUp ///////////////////////////////////////////////////////////

void    pfGUIListBoxMod::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
    fClicking = false;
    if( !( fModsAtDragTime & ( pfGameGUIMgr::kShiftDown | pfGameGUIMgr::kCtrlDown ) ) && !HasFlag( kHandsOffMultiSelect ) )
    {
        // No modifiers--simply select whatever item we're on
        int32_t   item = IGetItemFromPoint( mousePt );
        if( item != fCurrClick )
        {
            if( fCurrClick >= 0 && fCurrClick < fElements.GetCount() )
                fElements[ fCurrClick ]->SetSelected( false );
            fCurrHover = fCurrClick = item;
            if( fCurrClick >= 0 && fCurrClick < fElements.GetCount() )
                fElements[ fCurrClick ]->SetSelected( true );
            else if (HasFlag(kForbidNoSelection) && fCurrClick == -1)
            {
                fElements[fSingleSelElement]->SetSelected(true);
                fCurrClick = fSingleSelElement;
            }
            fCheckScroll = true;
            fSingleSelElement = fCurrClick;
            IUpdate();
        }
        else
        {
            // We didn't change selection, so go ahead and pass the click on to
            // the item, in case it wants to do something
            if( fCurrClick >= 0 && fCurrClick < fElements.GetCount() )
            {
                hsPoint3 localPt = mousePt;
                IScreenToLocalPt( localPt );

                uint16_t lX = (uint16_t)(( localPt.fX * ( fDynTextMap->GetVisibleWidth() - 1 ) ) - fElementBounds[ fCurrClick ].fLeft);
                uint16_t lY = (uint16_t)(( localPt.fY * ( fDynTextMap->GetVisibleHeight() - 1 ) )- fElementBounds[ fCurrClick ].fTop);

                if( fElements[ fCurrClick ]->MouseClicked( lX, lY ) )
                {
                    ICalcWrapStarts();
                    ICalcScrollRange();
                    IUpdate();
                }
            }
        }
    }

    DoSomething();  // Change in selection triggers our DoSomething() call
}

//// HandleMouseHover ////////////////////////////////////////////////////////
//  Just updates our currHover item so we can update the mouse appropriately

void    pfGUIListBoxMod::HandleMouseHover( hsPoint3 &mousePt, uint8_t modifiers )
{
    fCurrHover = IGetItemFromPoint( mousePt );
}

//// HandleMouseDrag /////////////////////////////////////////////////////////

void    pfGUIListBoxMod::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
    int     i;


    if( ( fModsAtDragTime & pfGameGUIMgr::kShiftDown && !HasFlag( kSingleSelect ) ) )
    {
        // Select our new range, deselect everything outside
        if( fCurrClick <= fMaxSel )
        {
            ISelectRange( 0, (int8_t)(fCurrClick - 1), false );
            ISelectRange( (int8_t)fCurrClick, (int8_t)fMaxSel, true );
            ISelectRange( (int8_t)(fMaxSel + 1), -1, false );
        }
        else if( fCurrClick >= fMinSel )
        {
            ISelectRange( 0, (int8_t)(fMinSel - 1), false );
            ISelectRange( (int8_t)fMinSel, (int8_t)fCurrClick, true );
            ISelectRange( (int8_t)(fCurrClick + 1), -1, false );
        }
        IUpdate();
    }
    else if( !( fModsAtDragTime & ( pfGameGUIMgr::kShiftDown | pfGameGUIMgr::kCtrlDown ) ) )
    {
        // No modifiers--simply select whatever item we're on
        if( HasFlag( kDragAndDropCapable ) )
        {
            // If we're drag and drop capable, then a mouse drag function results in
            // the dialog entering Drag Mode(tm). Basically, we tell our parent dialog
            // which items to drag and it'll run with the rest
            fDialog->ClearDragList();
            for( i = 0; i < fElements.GetCount(); i++ )
            {
                if( fElements[ i ]->IsSelected() )
                    fDialog->AddToDragList( fElements[ i ] );
            }
            fDialog->EnterDragMode( this );
        }
        else
        {
            int32_t   item = IGetItemFromPoint( mousePt );
            if( item != fCurrClick )
            {
                if( fCurrClick >= 0 && fCurrClick < fElements.GetCount() )
                    fElements[ fCurrClick ]->SetSelected( false );
                fCurrHover = fCurrClick = item;
                if( fCurrClick >= 0 && fCurrClick < fElements.GetCount() )
                    fElements[ fCurrClick ]->SetSelected( true );
                fCheckScroll = true;
                fSingleSelElement = fCurrClick;
                IUpdate();
            }
        }
    }
}

//// HandleMouseDblClick /////////////////////////////////////////////////////

void    pfGUIListBoxMod::HandleMouseDblClick( hsPoint3 &mousePt, uint8_t modifiers )
{
    if( !HasFlag( kGrowLeavesAndProcessOxygen ) )
        return;     // We only care about double clicks if we make oxygen

    int32_t item = IGetItemFromPoint( mousePt );
    if( item != -1 )
    {
        if( fElements[ item ]->GetType() == pfGUIListElement::kTreeRoot )
        {
            pfGUIListTreeRoot *root = (pfGUIListTreeRoot *)fElements[ item ];

            root->ShowChildren( !root->IsShowingChildren() );
            if( !fLocked )
            {
                ICalcWrapStarts();
                ICalcScrollRange();
                IUpdate();
            }
        }
    }
}

//// IGetItemFromPoint ///////////////////////////////////////////////////////

int32_t   pfGUIListBoxMod::IGetItemFromPoint( hsPoint3 &mousePt )
{
    if( !fBounds.IsInside( &mousePt ) )
        return -1;

    hsPoint3    localPt = mousePt; // despite getting a ref to the point (why?) we do NOT want to modify it
    IScreenToLocalPt( localPt );
    uint32_t      i;
    int16_t       clickItem = -1 , clickY = (int16_t)( localPt.fY * ( fDynTextMap->GetVisibleHeight() - 1 ) );
    int16_t       clickX = (int16_t)( localPt.fX * ( fDynTextMap->GetVisibleWidth() - 1 ) );

    // We have a nice array that has the starting (top) Y's of each visible element. So we just
    // check against that.
    uint32_t startAt = 0;
    // make sure that we have a valid fScrollPos
    if ( fScrollPos != -1 )
    {
        // make sure that there is an Idx at fScrollPos
        if ( fWrapStartIdxs.GetCount() > fScrollPos )
        {
            startAt = fWrapStartIdxs[ fScrollPos ];
            clickItem = -1;
        }
    }
    for( i = startAt; i < fElements.GetCount(); i++ )
    {
        if( i<fElementBounds.GetCount() && fElementBounds[ i ].Contains( clickX, clickY ) )
        {
            clickItem = (int16_t)i;
            break;
        }
    }

    if( clickItem > fElements.GetCount() - 1 )
        clickItem = -1;

    return clickItem;
}

//// IFindSelectionRange /////////////////////////////////////////////////////
//  Find the min and max item that's selected. Returns -1 for both if nobody
//  is selected

void    pfGUIListBoxMod::IFindSelectionRange( int32_t *min, int32_t *max )
{
    int32_t       i;


    *min = *max = -1;

    for( i = 0; i < fElements.GetCount(); i++ )
    {
        if( fElements[ i ]->IsSelected() )
        {
            *min = i;
            break;
        }
    }

    for( i = fElements.GetCount() - 1; i >= 0; i-- )
    {
        if( fElements[ i ]->IsSelected() )
        {
            *max = i;
            break;
        }
    }
}

//// ISelectRange ////////////////////////////////////////////////////////////

void    pfGUIListBoxMod::ISelectRange( int8_t min, int8_t max, bool select )
{
    int16_t   i;


    if( max == -1 )
        max = fElements.GetCount() - 1;

    for( i = min; i <= max; i++ )
        fElements[ i ]->SetSelected( select );
}

//// SetSelection ////////////////////////////////////////////////////////////

void    pfGUIListBoxMod::SetSelection( int32_t item )
{
    if( HasFlag( kSingleSelect ) )
    {
        // Easy, only one item selected
        if( fSingleSelElement != -1 )
            fElements[ fSingleSelElement ]->SetSelected( false );

        fSingleSelElement = item;
    }
    else
        ISelectRange( 0, -1, false );

    if( item != -1 && item < fElements.GetCount() )
        fElements[ item ]->SetSelected( true );

    fCheckScroll = true;
    IUpdate();
}

void    pfGUIListBoxMod::RemoveSelection( int32_t item )
{
    // make sure the item is valid
    if ( item != -1 && item < fElements.GetCount() )
    {
        // if single select and its what is selected
        if ( HasFlag( kSingleSelect) && fSingleSelElement == item )
        {
            // then remove the selection and make nothing selected
            fElements[ fSingleSelElement ]->SetSelected( false );
            fSingleSelElement = item;
        }
        // else if multiple selection
        else
            // just remove the selection
            fElements[ item ]->SetSelected( false );
    }
    fCheckScroll = true;
    IUpdate();
}

void    pfGUIListBoxMod::AddSelection( int32_t item )
{
    // make sure the item is valid (can't add a non-selection!)
    if ( item != -1 && item < fElements.GetCount() )
    {
        // if single select then just like SetSelection
        if ( HasFlag( kSingleSelect) )
        {
            SetSelection(item);
        }
        // else if multiple selection
        else
            // just set its selection
            fElements[ item ]->SetSelected( true );
    }
    fCheckScroll = true;
    IUpdate();
}

//// HandleKeyPress //////////////////////////////////////////////////////////

bool    pfGUIListBoxMod::HandleKeyPress( wchar_t key, uint8_t modifiers )
{
    return false;
}

bool    pfGUIListBoxMod::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers )
{
    if( key == KEY_CAPSLOCK )
        return false;

    if( HasFlag( kDisableKeyActions ) )
        return false;

    if( event == pfGameGUIMgr::kKeyDown || event == pfGameGUIMgr::kKeyRepeat )
    {
        // Use arrow keys to do our dirty work
        if( key == KEY_UP )
        {
            if( fCurrClick == -1 && fElements.GetCount() > 0 )
                fCurrClick = 0;
            else while( fCurrClick > 0 )
            {
                fCurrClick--;
                if( !HasFlag( kGrowLeavesAndProcessOxygen ) || !fElements[ fCurrClick ]->IsCollapsed() )
                    break;
            }
        }
        else if( key == KEY_DOWN )
        {
            if( fCurrClick == -1 && fElements.GetCount() > 0 )
                fCurrClick = fElements.GetCount() - 1;
            else while( fCurrClick < fElements.GetCount() - 1 )
            {
                fCurrClick++;
                if( !HasFlag( kGrowLeavesAndProcessOxygen ) || !fElements[ fCurrClick ]->IsCollapsed() )
                    break;
            }
        }
        else if( key == KEY_HOME )
        {
            if( fElements.GetCount() > 0 )
                fCurrClick = 0;         
        }
        else if( key == KEY_END )
        {
            if( fElements.GetCount() > 0 )
                fCurrClick = fElements.GetCount() - 1;          
        }
        else if( key == KEY_ENTER && HasFlag( kGrowLeavesAndProcessOxygen ) )
        {
            if( fCurrClick != -1 && fElements[ fCurrClick ]->GetType() == pfGUIListElement::kTreeRoot )
            {
                pfGUIListTreeRoot *root = (pfGUIListTreeRoot *)fElements[ fCurrClick ];
                root->ShowChildren( !root->IsShowingChildren() );

                if( !fLocked )
                {
                    ICalcWrapStarts();
                    ICalcScrollRange();
                }
            }
        }
        else
        {
            return false;
        }

        ISelectRange( 0, -1, false );
        if( fCurrClick != -1 )
            fElements[ fCurrClick ]->SetSelected( true );
        fSingleSelElement = fCurrClick;

        DoSomething();  // Change in selection triggers our DoSomething() call

        fCheckScroll = true;
        IUpdate();
        return true;
    }

    return false;
}

//// ScrollToEnd /////////////////////////////////////////////////////////////

void    pfGUIListBoxMod::ScrollToEnd( void )
{
    if( fScrollControl != nil )
    {
        fScrollPos = (int)fScrollControl->GetMin();
        fScrollControl->SetCurrValue( fScrollControl->GetMax() );
    }
    IUpdate();
}

//// ScrollToBegin ///////////////////////////////////////////////////////////

void    pfGUIListBoxMod::ScrollToBegin( void )
{
    if( fScrollControl != nil )
    {
        fScrollPos = (int)fScrollControl->GetMax();
        fScrollControl->SetCurrValue( fScrollControl->GetMin() );
    }
    IUpdate();
}

void        pfGUIListBoxMod::SetColorScheme( pfGUIColorScheme *newScheme )
{
    uint16_t  i;

    pfGUIControlMod::SetColorScheme( newScheme );

    for( i = 0; i < fElements.GetCount(); i++ )
    {
        fElements[ i ]->SetColorScheme( newScheme );
        fElements[ i ]->SetSkin( fSkin );
    }
}

void pfGUIListBoxMod::SetScrollPos( int32_t pos )
{
    if ( pos >= (int)fScrollControl->GetMin() && pos <= (int)fScrollControl->GetMax() )
    {
        fScrollControl->SetCurrValue( (float)pos );
        fScrollPos = (int)fScrollControl->GetMax() - pos;
    }
    IUpdate();
}

int32_t pfGUIListBoxMod::GetScrollPos( void )
{
    return (int)fScrollControl->GetCurrValue();
}

int32_t pfGUIListBoxMod::GetScrollRange( void )
{
    return (int)fScrollControl->GetMax() - (int)fScrollControl->GetMin();
}


//////////////////////////////////////////////////////////////////////////////
//// Element Manipulation ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

uint16_t  pfGUIListBoxMod::AddElement( pfGUIListElement *el )
{
    uint16_t  idx = fElements.GetCount();
    fElements.Append( el );
    el->SetColorScheme( GetColorScheme() );
    el->SetSkin( fSkin );
    if( !fLocked )
    {
        ICalcWrapStarts();
        ICalcScrollRange();
        IUpdate();
        HandleExtendedEvent( pfGUIListBoxMod::kItemAdded );
    }
    return idx;
}

void    pfGUIListBoxMod::RemoveElement( uint16_t index )
{
    // Make sure no other elements care about this one
    uint16_t i, j;
    for( i = 0; i < fElements.GetCount(); i++ )
    {
        if( fElements[ i ]->GetType() == pfGUIListElement::kTreeRoot )
        {
            pfGUIListTreeRoot *root = (pfGUIListTreeRoot *)fElements[ i ];
            for( j = 0; j < root->GetNumChildren(); )
            {
                if( root->GetChild( j ) == fElements[ index ] )
                    root->RemoveChild( j );
                else
                    j++;
            }
        }
    }

    delete fElements[ index ];
    fElements.Remove( index );

    if( index == fSingleSelElement )
        fSingleSelElement = -1;
    else if( index < fSingleSelElement )
        fSingleSelElement--;
    fCurrHover = fCurrClick = -1;

    if( !fLocked )
    {
        ICalcWrapStarts();
        ICalcScrollRange();
        IUpdate();
        HandleExtendedEvent( pfGUIListBoxMod::kItemRemoved );
    }
}

int16_t   pfGUIListBoxMod::FindElement( pfGUIListElement *toCompareTo )
{
    int     i;


    for( i = 0; i < fElements.GetCount(); i++ )
    {
        if( fElements[ i ]->CompareTo( toCompareTo ) == 0 )
            return i;
    }

    return (int16_t)-1;
}

void    pfGUIListBoxMod::ClearAllElements( void )
{
    int     i;


    for( i = 0; i < fElements.GetCount(); i++ )
        delete fElements[ i ];
    fElements.Reset();
    fSingleSelElement = -1;

    if( !fLocked )
    {
        ICalcWrapStarts();
        ICalcScrollRange();
        IUpdate();
    }

    HandleExtendedEvent( pfGUIListBoxMod::kListCleared );
}

uint16_t  pfGUIListBoxMod::AddString( const plString &string )
{
    return AddElement( new pfGUIListText( string ) );
}

int16_t   pfGUIListBoxMod::FindString( const plString &toCompareTo )
{
    pfGUIListText   text( toCompareTo );
    return FindElement( &text );
}

uint16_t  pfGUIListBoxMod::GetNumElements( void )
{
    return fElements.GetCount();
}

pfGUIListElement    *pfGUIListBoxMod::GetElement( uint16_t idx )
{
    return fElements[ idx ];
}

void    pfGUIListBoxMod::LockList( void )
{
    fLocked = true;
}

void    pfGUIListBoxMod::UnlockList( void )
{
    fLocked = false;
    ICalcWrapStarts();
    ICalcScrollRange();
    IUpdate();
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

uint32_t      pfGUIListBoxMod::IGetDesiredCursor( void ) const
{
    if( fCurrHover == -1 )
        return plInputInterface::kNullCursor;

    if( fClicking )
        return plInputInterface::kCursorClicked;

    return plInputInterface::kCursorPoised;
}

