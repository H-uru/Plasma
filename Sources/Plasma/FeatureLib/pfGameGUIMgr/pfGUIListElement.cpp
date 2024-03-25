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
//  pfGUIListElement Class Definitions                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIListElement.h"

#include <string_theory/format>

#include "HeadSpin.h"
#include "hsResMgr.h"

#include "pfGameGUIMgr.h"
#include "pfGUIPopUpMenu.h"     // For skins

#include "plGImage/hsCodecManager.h"
#include "plGImage/plDynamicTextMap.h"

//////////////////////////////////////////////////////////////////////////////
//// Base Stuff //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void    pfGUIListElement::Read( hsStream *s, hsResMgr *mgr )
{
    fSelected = s->ReadBool();
}

void    pfGUIListElement::Write( hsStream *s, hsResMgr *mgr )
{
    s->WriteBool( fSelected );
}

//////////////////////////////////////////////////////////////////////////////
//// pfGUIListText ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIListText::pfGUIListText()
    : pfGUIListElement(kText), fJustify(kLeftJustify)
{
}

pfGUIListText::pfGUIListText( ST::string text )
    : pfGUIListElement(kText), fText(std::move(text)), fJustify(kLeftJustify)
{
}

//// Virtuals ////////////////////////////////////////////////////////////////

void    pfGUIListText::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIListElement::Read( s, mgr );

    fText = s->ReadSafeString();
}

void    pfGUIListText::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIListElement::Write( s, mgr );

    s->WriteSafeString(fText);
}

bool    pfGUIListText::Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight )
{
    textGen->SetJustify( (plDynamicTextMap::Justify)fJustify );
    if( fSelected )
    {
        textGen->FillRect( x, y, maxWidth, maxHeight, fColors->fSelBackColor );
        textGen->SetTextColor( fColors->fSelForeColor, fColors->fTransparent && fColors->fSelBackColor.a == 0.f );
    }
    else
    {
        // Normal back color will be cleared for us
        textGen->SetTextColor( fColors->fForeColor, fColors->fTransparent && fColors->fBackColor.a == 0.f );
    }

    textGen->DrawClippedString( x + 4, y, GetText(), maxWidth - 8, maxHeight );
    return true;
}

void    pfGUIListText::GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height )
{
    *width = textGen->CalcStringWidth( GetText(), height );
    if (height != nullptr)
    { 
        if( *height == 0 )
            *height = 10;       // Never allow zero height elements
        else
            *height += 0;       // Add one pixel on each side for padding (or not, 3.21.02 mcn)
    }
}

int     pfGUIListText::CompareTo( pfGUIListElement *rightSide )
{
    pfGUIListText *text = (pfGUIListText *)rightSide;
    
    if( text->fType != kText )
        return -2;

    return GetText().compare(text->GetText());
}

void    pfGUIListText::SetJustify( JustifyTypes justify )
{
    switch( justify )
    {
        case kRightJustify:
            fJustify = plDynamicTextMap::kRightJustify;
            break;
        case kCenter:
            fJustify = plDynamicTextMap::kCenter;
            break;
        case kLeftJustify:
        default:
            fJustify = plDynamicTextMap::kLeftJustify;
            break;
    }
}


//////////////////////////////////////////////////////////////////////////////
//// pfGUIListPicture ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor/Destructor //////////////////////////////////////////////////



pfGUIListPicture::pfGUIListPicture(plKey mipKey, bool respectAlpha)
    : pfGUIListElement(kPicture), fRespectAlpha(respectAlpha), fBorderSize(2), fMipmapKey(std::move(mipKey))
{
    plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
    if (mip != nullptr && mip->IsCompressed())
    {
        // Gotta make and grab an uncompressed one
        plMipmap *uncompBuffer = hsCodecManager::Instance().CreateUncompressedMipmap( mip, hsCodecManager::k32BitDepth );
        ST::string str = ST::format("{}_uncomp", mip->GetKeyName());
        fMipmapKey = hsgResMgr::ResMgr()->NewKey( str, uncompBuffer, fMipmapKey->GetUoid().GetLocation() );
    }

    fMipmapKey->RefObject();
}

pfGUIListPicture::~pfGUIListPicture()
{
    fMipmapKey->UnRefObject();
    fMipmapKey = nullptr;
}

//// Virtuals ////////////////////////////////////////////////////////////////

void    pfGUIListPicture::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIListElement::Read( s, mgr );

}

void    pfGUIListPicture::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIListElement::Write( s, mgr );

}

bool    pfGUIListPicture::Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight )
{
    if( fSelected )
        textGen->FillRect( x, y, maxWidth, maxHeight, fColors->fSelBackColor );

    plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
    if (mip != nullptr)
    {
        if( mip->GetWidth() + fBorderSize + fBorderSize > maxWidth || mip->GetHeight() + fBorderSize + fBorderSize > maxHeight )
            return false;

        textGen->DrawImage( x + fBorderSize, y + fBorderSize, mip, fRespectAlpha ? plDynamicTextMap::kImgBlend : plDynamicTextMap::kImgNoAlpha );
    }

    return true;
}

void    pfGUIListPicture::GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height )
{
    plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
    if (mip == nullptr)
    {
        *width = 16;
        if (height != nullptr)
            *height = 16;
    }

    *width = (uint16_t)(mip->GetWidth() + fBorderSize + fBorderSize);
    if (height != nullptr)
        *height = (uint16_t)(mip->GetHeight() + fBorderSize + fBorderSize);
}

int     pfGUIListPicture::CompareTo( pfGUIListElement *rightSide )
{
    pfGUIListPicture *text = (pfGUIListPicture *)rightSide;
    
    if( text->fType != kPicture )
        return -2;

    return -2;
}

//////////////////////////////////////////////////////////////////////////////
//// pfGUIListTreeRoot ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Virtuals ////////////////////////////////////////////////////////////////

void    pfGUIListTreeRoot::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIListElement::Read( s, mgr );

    fText = s->ReadSafeString();
}

void    pfGUIListTreeRoot::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIListElement::Write( s, mgr );

    s->WriteSafeString(fText);
}

bool    pfGUIListTreeRoot::Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight )
{
    textGen->SetJustify( plDynamicTextMap::kLeftJustify );
    if( fSelected )
    {
        textGen->FillRect( x, y, maxWidth, maxHeight, fColors->fSelBackColor );
        textGen->SetTextColor( fColors->fSelForeColor, fColors->fTransparent && fColors->fSelBackColor.a == 0.f );
    }
    else
    {
        // Normal back color will be cleared for us
        textGen->SetTextColor( fColors->fForeColor, fColors->fTransparent && fColors->fBackColor.a == 0.f );
    }

    if (fSkin != nullptr)
    {
        const pfGUISkin::pfSRect &r = fSkin->GetElement( fShowChildren ? pfGUISkin::kTreeButtonOpen : pfGUISkin::kTreeButtonClosed );

        int16_t e = ( maxHeight - r.fHeight );
        if( e < 0 )
            e = 0;
        e >>= 1;

        textGen->DrawClippedImage( x + 2, y + e, fSkin->GetTexture(), r.fX, r.fY, r.fWidth, r.fHeight, plDynamicTextMap::kImgSprite );
        x += r.fWidth + 4;
    }

    textGen->DrawClippedString( x + 4, y, GetTitle(), maxWidth - 8, maxHeight );
    return true;
}

bool    pfGUIListTreeRoot::MouseClicked( uint16_t localX, uint16_t localY )
{
    if (fSkin != nullptr)
    {
        const pfGUISkin::pfSRect &r = fSkin->GetElement( fShowChildren ? pfGUISkin::kTreeButtonOpen : pfGUISkin::kTreeButtonClosed );

        // For now, I can't think of a clean way of getting the current visible height to this function,
        // but just testing the X value for tree controls is good enough for now. If we need Y testing for
        // other elements, I'll figure out something.
        if( localX >= 2 && localX <= 2 + r.fWidth )
        {
            ShowChildren( !fShowChildren );
            return true;
        }
    }

    return false;
}

void    pfGUIListTreeRoot::GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height )
{
    *width = textGen->CalcStringWidth( GetTitle(), height );
    if (height != nullptr)
    { 
        if( *height == 0 )
            *height = 10;       // Never allow zero height elements
        else
            *height += 0;       // Add one pixel on each side for padding (or not, 3.21.02 mcn)

        if (fSkin != nullptr)
        {
            uint16_t h = fSkin->GetElement( pfGUISkin::kTreeButtonClosed ).fHeight;
            if( *height < h )
                *height = h;
        }
    }

    if (fSkin != nullptr)
        *width += fSkin->GetElement( pfGUISkin::kTreeButtonClosed ).fWidth;
}

int     pfGUIListTreeRoot::CompareTo( pfGUIListElement *rightSide )
{
    pfGUIListTreeRoot *text = (pfGUIListTreeRoot *)rightSide;
    
    if( text->fType != kTreeRoot )
        return -2;

    return GetTitle().compare(text->GetTitle());
}

void    pfGUIListTreeRoot::AddChild( pfGUIListElement *el )
{
    fChildren.emplace_back(el);
    el->SetIndentLevel( GetIndentLevel() + 1 );
    el->SetCollapsed( !fShowChildren );
}

void    pfGUIListTreeRoot::RemoveChild(size_t idx)
{
    fChildren.erase(fChildren.begin() + idx);
}

void    pfGUIListTreeRoot::ShowChildren( bool s )
{
    fShowChildren = s;
    for (pfGUIListElement* child : fChildren)
        child->SetCollapsed(!s);
}

void    pfGUIListTreeRoot::SetCollapsed( bool c )
{
    pfGUIListElement::SetCollapsed( c );
    for (pfGUIListElement* child : fChildren)
        child->SetCollapsed(c ? true : !fShowChildren);
}
