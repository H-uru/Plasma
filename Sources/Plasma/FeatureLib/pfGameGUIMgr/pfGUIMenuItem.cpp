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
//  pfGUIMenuItem Definition                                                //
//                                                                          //
//  The type of button that knows how to party.                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIMenuItem.h"

#include "HeadSpin.h"
#include "hsResMgr.h"

#include "pfGameGUIMgr.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogMod.h"
#include "pfGUIPopUpMenu.h"

#include "pnMessage/plRefMsg.h"

#include "plGImage/plDynamicTextMap.h"
#include "plInputCore/plInputInterface.h"

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIMenuItem::pfGUIMenuItem()
    : fName(), fReportingHover(), fSkinBuffersUpdated(true), fHowToSkin()
{
    fSkin = nullptr;
}

pfGUIMenuItem::~pfGUIMenuItem()
{
    SetSkin(nullptr, kTop);
}

void    pfGUIMenuItem::SetName(ST::string name)
{
    fName = std::move(name);
    IUpdate();
}

//// SetSkin /////////////////////////////////////////////////////////////////

void    pfGUIMenuItem::SetSkin( pfGUISkin *skin, HowToSkin s )
{
    // Just a function wrapper for SendRef
    if (fSkin != nullptr)
        GetKey()->Release( fSkin->GetKey() );

    if (skin != nullptr)
        hsgResMgr::ResMgr()->SendRef( skin->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );

    fHowToSkin = s;

    fSkinBuffersUpdated = false;
}

//// IPostSetUpDynTextMap ////////////////////////////////////////////////////
//  Draw our initial image on the dynTextMap

void    pfGUIMenuItem::IPostSetUpDynTextMap()
{
}


//// IGetDesiredExtraDTMRoom /////////////////////////////////////////////////
//  Overridden so we can enlarge our DTMap by 3 vertically, to use the extra
//  space as basically a double buffer for our skinning

void    pfGUIMenuItem::IGrowDTMDimsToDesiredSize( uint16_t &width, uint16_t &height )
{
    height *= 3;
}

//// IUpdateSkinBuffers //////////////////////////////////////////////////////
//  Redraws the double buffers for the two skin images we keep hidden in the
//  DTMap, so we don't have to re-composite them every time we draw the
//  control.

void    pfGUIMenuItem::IUpdateSkinBuffers()
{
    if( fSkinBuffersUpdated )
        return;
    if (fSkin == nullptr)
        return;
    if (fDynTextMap == nullptr)
        return;
    if (fSkin->GetTexture() == nullptr)
        return;

    uint16_t y = fDynTextMap->GetVisibleHeight();

    IUpdateSingleSkinBuffer( y, false );
    IUpdateSingleSkinBuffer( y << 1, true );

    fSkinBuffersUpdated = true;
}

//// IUpdateSingleSkinBuffer /////////////////////////////////////////////////
//  Broken down functionality for the above function

void    pfGUIMenuItem::IUpdateSingleSkinBuffer( uint16_t y, bool sel )
{
    hsAssert(fSkin != nullptr && fDynTextMap != nullptr, "Invalid pointers in IUpdateSingleSkinBuffer()");


    // Note: add 1 to the visible height so we get enough overlap to take care of mipmapping issues
    uint16_t              x = 0, totWidth = fDynTextMap->GetVisibleWidth();
    uint16_t              totHeight = y + fDynTextMap->GetVisibleHeight();
    pfGUISkin::pfSRect  element;


    totWidth -= fSkin->GetElement( pfGUISkin::kRightSpan ).fWidth;
    if( fHowToSkin == kTop )
    {
        // Draw up-left corner
        element = fSkin->GetElement( pfGUISkin::kUpLeftCorner );
        fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, element.fWidth, element.fHeight, plDynamicTextMap::kImgSprite );
        x += element.fWidth;

        element = fSkin->GetElement( pfGUISkin::kTopSpan );
        for( ; x < totWidth; )
        {
            uint16_t wid = element.fWidth;
            if( x + wid > totWidth )
                wid = totWidth - x;
            fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, wid, element.fHeight, plDynamicTextMap::kImgSprite );
            x += wid;
        }

        element = fSkin->GetElement( pfGUISkin::kUpRightCorner );
        fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, element.fWidth, element.fHeight, plDynamicTextMap::kImgSprite );

        y += element.fHeight;
    }
    else if( fHowToSkin == kBottom )
    {
        // Clip some space for now
        totHeight -= fSkin->GetElement( pfGUISkin::kLowerLeftCorner ).fHeight;
    }
    
    // Group drawing by skin elements for caching performance
    uint16_t startY = y;
    x = 0;
    element = fSkin->GetElement( pfGUISkin::kLeftSpan );
    for( ; y < totHeight; )
    {
        uint16_t ht = element.fHeight;
        if( y + ht > totHeight )
            ht = totHeight - y;
        fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, element.fWidth, ht, plDynamicTextMap::kImgSprite );
        y += ht;
    }

    x += element.fWidth;
    if( sel )
        element = fSkin->GetElement( pfGUISkin::kSelectedFill );
    else
        element = fSkin->GetElement( pfGUISkin::kMiddleFill );
    for( ; x < totWidth; )
    {
        uint16_t wid = element.fWidth;
        if( x + wid > totWidth )
            wid = totWidth - x;

        for( y = startY; y < totHeight; )
        {
            uint16_t ht = element.fHeight;
            if( y + ht > totHeight )
                ht = totHeight - y;
            fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, wid, ht, plDynamicTextMap::kImgSprite );
            y += ht;
        }

        x += wid;
    }

    element = fSkin->GetElement( pfGUISkin::kRightSpan );
    for( y = startY; y < totHeight; )
    {
        uint16_t ht = element.fHeight;
        if( y + ht > totHeight )
            ht = totHeight - y;
        fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, element.fWidth, ht, plDynamicTextMap::kImgSprite );
        y += ht;
    }

    if( fHowToSkin == kBottom )
    {
        x = 0;

        // Draw lower-left corner
        element = fSkin->GetElement( pfGUISkin::kLowerLeftCorner );
        fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, element.fWidth, element.fHeight, plDynamicTextMap::kImgSprite );
        x += element.fWidth;

        element = fSkin->GetElement( pfGUISkin::kBottomSpan );
        for( ; x < totWidth; )
        {
            uint16_t wid = element.fWidth;
            if( x + wid > totWidth )
                wid = totWidth - x;
            fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, wid, element.fHeight, plDynamicTextMap::kImgSprite );
            x += wid;
        }

        element = fSkin->GetElement( pfGUISkin::kLowerRightCorner );
        fDynTextMap->DrawClippedImage( x, y, fSkin->GetTexture(), element.fX, element.fY, element.fWidth, element.fHeight, plDynamicTextMap::kImgSprite );

        y += element.fHeight;
    }
}

//// IUpdate /////////////////////////////////////////////////////////////////

void    pfGUIMenuItem::IUpdate()
{
    if (fDynTextMap == nullptr)
        return;

    if (fSkin != nullptr)
    {
        IUpdateSkinBuffers();

        if( !fSkinBuffersUpdated )
            return;

        // Copy now from our skin buffer, plus set our text color
        uint16_t y = fDynTextMap->GetVisibleHeight();

        if( IsInteresting() )
        {
            fDynTextMap->DrawClippedImage( 0, 0, fDynTextMap, 0, y << 1, fDynTextMap->GetVisibleWidth(), y, plDynamicTextMap::kImgSprite );
            fDynTextMap->SetTextColor( GetColorScheme()->fSelForeColor );
        }
        else
        {
            fDynTextMap->DrawClippedImage( 0, 0, fDynTextMap, 0, y, fDynTextMap->GetVisibleWidth(), y, plDynamicTextMap::kImgSprite );
            fDynTextMap->SetTextColor( GetColorScheme()->fForeColor );
        }
    }
    else
    {
        if( IsInteresting() )
        {
            fDynTextMap->ClearToColor( GetColorScheme()->fSelBackColor );
            fDynTextMap->SetTextColor( GetColorScheme()->fSelForeColor );
        }
        else
        {
            fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );
            fDynTextMap->SetTextColor( GetColorScheme()->fForeColor );
        }
    }

    fDynTextMap->SetJustify( plDynamicTextMap::kLeftJustify );

    if (!fName.empty())
    {
        uint16_t ht;
        fDynTextMap->CalcStringWidth( fName, &ht );

        int16_t x = 0, y = ( fDynTextMap->GetVisibleHeight() - ht ) >> 1;
        if (fHowToSkin == kTop && fSkin != nullptr)
            y += fSkin->GetElement( pfGUISkin::kTopSpan ).fHeight >> 1;
        else if (fHowToSkin == kBottom && fSkin != nullptr)
            y -= fSkin->GetElement( pfGUISkin::kTopSpan ).fHeight >> 1;
        
        if (fSkin != nullptr)
            x += fSkin->GetBorderMargin();

        if( fClicking )
        {
            x += 2;
            y += 2;
        }

        fDynTextMap->DrawClippedString( x, y, fName, fDynTextMap->GetVisibleWidth(), fDynTextMap->GetVisibleHeight() );

        if( HasFlag( kDrawSubMenuArrow ) )
        {
            if (fSkin != nullptr)
            {
                pfGUISkin::pfSRect element;

                if( IsInteresting() )
                    element = fSkin->GetElement( pfGUISkin::kSelectedSubMenuArrow );
                else
                    element = fSkin->GetElement( pfGUISkin::kSubMenuArrow );

                y += ( ht >> 1 ) - ( element.fHeight >> 1 );
                if( y < 0 || y + element.fHeight >= fDynTextMap->GetHeight() )
                    y = 0;

                fDynTextMap->DrawClippedImage( x + fDynTextMap->GetVisibleWidth() - 2 - element.fWidth
                                                - fSkin->GetElement( pfGUISkin::kRightSpan ).fWidth, 
                                                y,
                                                fSkin->GetTexture(), element.fX, element.fY, element.fWidth, element.fHeight, plDynamicTextMap::kImgBlend );
            }
            else
            {
                fDynTextMap->SetJustify( plDynamicTextMap::kRightJustify );
                fDynTextMap->DrawString( x + fDynTextMap->GetVisibleWidth() - 2, y, ">>" );
            }
        }
    }

    fDynTextMap->FlushToHost();
}

void pfGUIMenuItem::PurgeDynaTextMapImage()
{
    if (fDynTextMap != nullptr)
        fDynTextMap->PurgeImage();
}

//// GetTextExtents //////////////////////////////////////////////////////////
//  Calculate the size of the drawn text.

void    pfGUIMenuItem::GetTextExtents( uint16_t &width, uint16_t &height )
{
    width = fDynTextMap->CalcStringWidth( fName, &height );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIMenuItem::MsgReceive( plMessage *msg )
{
    return pfGUIButtonMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIMenuItem::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIButtonMod::Read( s, mgr );
}

void    pfGUIMenuItem::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIButtonMod::Write( s, mgr );
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void    pfGUIMenuItem::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    pfGUIButtonMod::HandleMouseDown( mousePt, modifiers );
    IUpdate();
}

void    pfGUIMenuItem::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
    pfGUIButtonMod::HandleMouseUp( mousePt, modifiers );
    IUpdate();
}

void    pfGUIMenuItem::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
/*  if( !fClicking )
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
*/
    pfGUIButtonMod::HandleMouseDrag( mousePt, modifiers );
}

void    pfGUIMenuItem::HandleMouseHover( hsPoint3 &mousePt, uint8_t modifiers )
{
    pfGUIButtonMod::HandleMouseHover( mousePt, modifiers );
    if( HasFlag( kReportHovers ) )
    {
        if( PointInBounds( mousePt ) )
        {
            if (!fReportingHover && (fDialog->GetControlOfInterest() == nullptr ||
                                     fDialog->GetControlOfInterest() == this))
            {
                fReportingHover = true;
                HandleExtendedEvent( kMouseHover );
                fDialog->SetControlOfInterest( this );
            }
        }
        else if( fReportingHover )
        {
            fReportingHover = false;
            HandleExtendedEvent( kMouseExit );
            fDialog->SetControlOfInterest(nullptr);
        }
    }
}

//// SetInteresting //////////////////////////////////////////////////////////
//  Overridden to play mouse over animation when we're interesting

void    pfGUIMenuItem::SetInteresting( bool i )
{
    pfGUIButtonMod::SetInteresting( i );
    IUpdate();

    // Make sure we're not still thinking we're reporting hovers when we're not
    if( !i )
        fReportingHover = false;
}
