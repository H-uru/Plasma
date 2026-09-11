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

#include "pyGUIControlListBox.h"

#include <string_theory/string_stream>

#include "plGImage/plDynamicTextMap.h"

#include "pfGameGUIMgr/pfGUIListBoxMod.h"
#include "pfGameGUIMgr/pfGUIListElement.h"

#include "pyColor.h"
#include "pyGlueHelpers.h"
#include "pyImage.h"
#include "pyKey.h"

// a special class for different coloured list items
//
class pfColorListElement : public pfGUIListText
{
    protected:
        hsColorRGBA         fTextColor1;
        hsColorRGBA         fTextColor2;
        ST::string          fString1;
        ST::string          fString2;
        uint32_t            fInheritAlpha;
        int32_t             fOverrideFontSize;  // size of font to use (if -1 then just use scheme setting)

    public:
        enum InheritTypes
        {
            kNoInherit = 0,
            kInheritFromNormal,
            kInheritFromSelect,
            kSelectDetermined,
            kSelectUseGUIColor,
        };

        pfColorListElement(ST::string string1, hsColorRGBA color1, ST::string string2, hsColorRGBA color2, uint32_t inheritalpha, int32_t fontsize = -1) 
            : pfGUIListText(), fTextColor1(color1), fTextColor2(color2), fString1(std::move(string1)), fString2(std::move(string2)),
              fInheritAlpha(inheritalpha), fOverrideFontSize(fontsize)
        {}

        pfColorListElement(ST::string string1, hsColorRGBA color1, uint32_t inheritalpha, int32_t fontsize = -1)
            : pfGUIListText(), fTextColor1(color1), fTextColor2(), fString1(std::move(string1)), fInheritAlpha(inheritalpha),
              fOverrideFontSize(fontsize)
        {}

        void SetText(ST::string text) override
        {
            fString1 = std::move(text);
        }

        bool    Draw(plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) override
        {
            hsColorRGBA textColor1;
            textColor1 = fTextColor1;
            hsColorRGBA textColor2;
            textColor2 = fTextColor2;

            textGen->SetJustify( (plDynamicTextMap::Justify)fJustify );
            if( fInheritAlpha == kInheritFromNormal )
            {
                textColor1.a = fColors->fForeColor.a;
                textColor2.a = fColors->fForeColor.a;
            }
            else if ( fInheritAlpha == kInheritFromSelect )
            {
                textColor1.a = fColors->fSelForeColor.a;
                textColor2.a = fColors->fSelForeColor.a;
            }
            else if ( fInheritAlpha == kSelectDetermined )
            {
                if ( fSelected )
                {
                    textColor1.a = fColors->fSelForeColor.a;
                    textColor2.a = fColors->fSelForeColor.a;
                }
                else
                {
                    textColor1.a = fColors->fForeColor.a;
                    textColor2.a = fColors->fForeColor.a;
                }
            }
            else if ( fInheritAlpha == kSelectUseGUIColor)
            {
                if ( fSelected )
                {
                    textColor1.r = fColors->fSelForeColor.r;
                    textColor2.r = fColors->fSelForeColor.r;
                    textColor1.g = fColors->fSelForeColor.g;
                    textColor2.g = fColors->fSelForeColor.g;
                    textColor1.b = fColors->fSelForeColor.b;
                    textColor2.b = fColors->fSelForeColor.b;
                    textColor1.a = fColors->fSelForeColor.a;
                    textColor2.a = fColors->fSelForeColor.a;
                }
                else
                {
                    textColor1.a = fColors->fForeColor.a;
                    textColor2.a = fColors->fForeColor.a;
                }
            }

            // draw the first string
            if (!fString1.empty())
            {
                if ( fOverrideFontSize != -1 )
                    textGen->SetFont( fColors->fFontFace, (uint16_t)fOverrideFontSize, fColors->fFontFlags );
                textGen->SetTextColor( textColor1, fColors->fTransparent && fColors->fBackColor.a == 0.f );
                textGen->DrawWrappedString( x + 2, y, fString1, maxWidth - 4, maxHeight );
                uint16_t width, height;
                textGen->CalcWrappedStringSize(fString1, &width, &height);
                x += 2 + width;
                if (fString2.empty())
                    y += height;
                if ( fOverrideFontSize != -1 )
                    textGen->SetFont( fColors->fFontFace, fColors->fFontSize, fColors->fFontFlags );
            }

            // draw the second string
            if (!fString2.empty())
            {
                textGen->SetTextColor( textColor2, fColors->fTransparent && fColors->fBackColor.a == 0.f );
                textGen->DrawWrappedString( x + 2, y, fString2, maxWidth - 4 - x, maxHeight );
            }

            return true;
        }

        void    GetSize(plDynamicTextMap *textGen, uint16_t *width, uint16_t *height) override
        {
            ST::string_stream ss;
            if (!fString1.empty())
                ss << fString1;
            if (!fString1.empty() && !fString2.empty())
                ss << ' ';
            if (!fString2.empty())
                ss << fString2;
            ST::string thestring = ss.to_string();
            *width = textGen->GetVisibleWidth() - 4;

            if ( fOverrideFontSize != -1 )
                textGen->SetFont( fColors->fFontFace, (uint16_t)fOverrideFontSize, fColors->fFontFlags );
            textGen->CalcWrappedStringSize( thestring, width, height );
            if ( fOverrideFontSize != -1 )
                textGen->SetFont( fColors->fFontFace, fColors->fFontSize, fColors->fFontFlags );

            if (height != nullptr)
                *height += 0;
            *width += 4;
        }

        int     CompareTo(pfGUIListElement *rightSide) override
        {
            return -2;
        }

};

class pfListTextInBox : public pfGUIListText
{
    protected:
        uint32_t              fMinWidth;
        uint32_t              fMinHeight;

    public:
        pfListTextInBox( ST::string text, uint32_t min_width=0, uint32_t min_height=0 ) : pfGUIListText( std::move(text) )
        {
            fMinWidth = min_width;
            fMinHeight = min_height;
            fJustify = pfGUIListText::kCenter;
        }

        void    GetSize(plDynamicTextMap *textGen, uint16_t *width, uint16_t *height) override
        {
            *width = textGen->CalcStringWidth( GetText(), height );
            if ( *width < fMinWidth )
                *width = (uint16_t)fMinWidth;
            if (height != nullptr)
            { 
                if( *height == 0 )
                    *height = 10;       // Never allow zero height elements
                else
                    *height += 0;       // Add one pixel on each side for padding (or not, 3.21.02 mcn)
                if ( *height < fMinHeight )
                    *height = (uint16_t)fMinHeight;
            }
        }
};

class pfListPictureInBox : public pfGUIListPicture
{
    protected:
        uint32_t              fSrcX;
        uint32_t              fSrcY;
        uint32_t              fSrcWidth;
        uint32_t              fSrcHeight;

    public:
        pfListPictureInBox( plKey mipKey, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool respectAlpha ) : pfGUIListPicture( std::move(mipKey), respectAlpha )
        {
            fSrcX = x;
            fSrcY = y;
            fSrcWidth = width;
            fSrcHeight = height;
        }

        bool Draw(plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) override
        {
            plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
            if (mip != nullptr)
            {
                if( fSrcWidth + fBorderSize + fBorderSize > maxWidth || fSrcHeight + fBorderSize + fBorderSize > maxHeight )
                    return false;

                if( fSelected )
                    textGen->FillRect( x, y, (uint16_t)fSrcWidth, (uint16_t)fSrcHeight, fColors->fSelForeColor );
// hack!!!! This is to get the non-selected items to show up....
//  ... they need some kinda background to alpha to <whatever>
                else
                {
                    if ( fRespectAlpha )
                    {
                        hsColorRGBA backcolor = fColors->fBackColor;
                        backcolor.a = 1.0;
                        textGen->FillRect( x, y, (uint16_t)fSrcWidth, (uint16_t)fSrcHeight, backcolor );
                    }
                }
// end of hack

                textGen->DrawClippedImage( x + fBorderSize, y + fBorderSize, mip, (uint16_t)fSrcX, (uint16_t)fSrcY, (uint16_t)fSrcWidth, (uint16_t)fSrcHeight, 
                                            fRespectAlpha ? plDynamicTextMap::kImgBlend : plDynamicTextMap::kImgNoAlpha );
            }

            return true;
        }

        void GetSize(plDynamicTextMap *textGen, uint16_t *width, uint16_t *height) override
        {
            plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
            if (mip == nullptr)
            {
                *width = 16;
                if (height != nullptr)
                    *height = 16;
            }
            else
            {
                *width = (uint16_t)(fSrcWidth + fBorderSize + fBorderSize);
                if (height != nullptr)
                    *height = (uint16_t)(fSrcHeight + fBorderSize + fBorderSize);
            }
        }
};

class pfListPictureInBoxWithSwatches : public pfListPictureInBox
{
    protected:
        hsColorRGBA             fPColor, fSColor;

    public:

        static uint16_t   fSwatchSize, fSwatchOffset;

        pfListPictureInBoxWithSwatches( plKey mipKey, uint32_t x, uint32_t y, 
                                                                            uint32_t width, uint32_t height, 
                                                                            bool respectAlpha,
                                                                            const hsColorRGBA &primaryColor, const hsColorRGBA &secondaryColor )
                                            : pfListPictureInBox( std::move(mipKey), x, y, width, height, respectAlpha )
        {
            fPColor = primaryColor;
            fSColor = secondaryColor;
        }

        bool Draw(plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) override
        {
            if( !pfListPictureInBox::Draw( textGen, x, y, maxWidth, maxHeight ) )
                return false;

            // Draw two color swatches
            if( maxWidth > fSwatchSize * 2 + 1 + fSwatchOffset )
            {
                // Secondary on right
                x = x + maxWidth - fSwatchOffset - fSwatchSize;
                y = y + maxHeight - fSwatchOffset - fSwatchSize;
                textGen->FillRect( x, y, fSwatchSize, fSwatchSize, fSColor );

                // Primary before it
                x -= fSwatchSize + 1;
                textGen->FillRect( x, y, fSwatchSize, fSwatchSize, fPColor );
            }
            return true;
        }
};
uint16_t  pfListPictureInBoxWithSwatches::fSwatchSize = 16;
uint16_t  pfListPictureInBoxWithSwatches::fSwatchOffset = 5;



pyGUIControlListBox::pyGUIControlListBox(pyKey& gckey) : pyGUIControl(gckey)
{
}

pyGUIControlListBox::pyGUIControlListBox(plKey objkey) : pyGUIControl(std::move(objkey))
{
}

bool pyGUIControlListBox::IsGUIControlListBox(const plKey& key)
{
    if ( key && pfGUIListBoxMod::ConvertNoRef(key->ObjectIsLoaded()) )
        return true;
    return false;
}

int32_t pyGUIControlListBox::GetSelection()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            return plbmod->GetSelection();
    }
    return -1;
}

void pyGUIControlListBox::SetSelection( int32_t item )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->SetSelection(item);
    }
}

void pyGUIControlListBox::RemoveElement( uint16_t index )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->RemoveElement(index);
    }
}

void pyGUIControlListBox::ClearAllElements()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ClearAllElements();
    }
}

uint16_t pyGUIControlListBox::GetNumElements()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            return plbmod->GetNumElements();
    }
    return 0;
}

void pyGUIControlListBox::SetElement( uint16_t idx, ST::string text )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListElement* le = plbmod->GetElement(idx);
            if ( le )
            {
                if ( le->GetType() == pfGUIListElement::kText )
                {
                    // if its a text element type then it should be safe to cast it to a pfGUIListText
                    pfGUIListText* letext = (pfGUIListText*)le;
                    letext->SetText(std::move(text));
                }
            }
        }
    }
}

void pyGUIControlListBox::SetStringJustify( uint16_t idx, uint32_t justify)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListElement* le = plbmod->GetElement(idx);
            if ( le )
            {
                if ( le->GetType() == pfGUIListElement::kText )
                {
                    // if its a text element type then it should be safe to cast it to a pfGUIListText
                    pfGUIListText* letext = (pfGUIListText*)le;
                    letext->SetJustify( (pfGUIListText::JustifyTypes)justify );
                }
            }
        }
    }
}


ST::string pyGUIControlListBox::GetElement( uint16_t idx )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListElement* le = plbmod->GetElement(idx);
            if ( le )
            {
                if ( le->GetType() == pfGUIListElement::kText )
                {
                    // if its a text element type then it should be safe to cast it to a pfGUIListText
                    pfGUIListText* letext = (pfGUIListText*)le;
                    return letext->GetText();
                }
                else if ( le->GetType() == pfGUIListElement::kTreeRoot )
                {
                    pfGUIListTreeRoot* elroot = (pfGUIListTreeRoot*)le;
                    return elroot->GetTitle();
                }
            }
        }
    }
    return "";
}

int16_t pyGUIControlListBox::AddString( ST::string string )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListText *element = new pfGUIListText(std::move(string));
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t   pyGUIControlListBox::AddImage( pyImage& image, bool respectAlpha )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListPicture *element = new pfGUIListPicture(image.GetKey(),respectAlpha);
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}


int16_t pyGUIControlListBox::FindString( ST::string toCompareTo )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            return plbmod->FindString(std::move(toCompareTo));
    }
    return -1;
}

int16_t pyGUIControlListBox::AddTextWColorW( ST::string str, pyColor& textcolor, uint32_t inheritalpha)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfColorListElement *element = new pfColorListElement(std::move(str), textcolor.getColor(), inheritalpha);
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t pyGUIControlListBox::AddTextWColorWSizeW( ST::string str, pyColor& textcolor, uint32_t inheritalpha, int32_t fontsize)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfColorListElement *element = new pfColorListElement(std::move(str), textcolor.getColor(), inheritalpha, fontsize);
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

void pyGUIControlListBox::Add2TextWColorW( ST::string str1, pyColor& textcolor1, ST::string str2, pyColor& textcolor2, uint32_t inheritalpha)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfColorListElement *element = new pfColorListElement(std::move(str1), textcolor1.getColor(), std::move(str2), textcolor2.getColor(), inheritalpha);
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            plbmod->AddElement( element );
        }
    }
}

int16_t pyGUIControlListBox::AddStringInBox( ST::string string, uint32_t min_width, uint32_t min_height )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfListTextInBox *element = new pfListTextInBox( std::move(string), min_width, min_height );
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t   pyGUIControlListBox::AddImageInBox( pyImage& image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool respectAlpha )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfListPictureInBox *element = new pfListPictureInBox(image.GetKey(),x,y,width,height,respectAlpha);
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t   pyGUIControlListBox::AddImageAndSwatchesInBox( pyImage& image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool respectAlpha,
                                                        pyColor &primary, pyColor &secondary )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfListPictureInBoxWithSwatches *element = new pfListPictureInBoxWithSwatches( image.GetKey(),x,y,
                                                                                        width,height,respectAlpha,
                                                                                        primary.getColor(), secondary.getColor() );
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(element);
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

void    pyGUIControlListBox::SetSwatchSize( uint32_t size )
{
    pfListPictureInBoxWithSwatches::fSwatchSize = (uint16_t)size;
}

void    pyGUIControlListBox::SetSwatchEdgeOffset( uint32_t set )
{
    pfListPictureInBoxWithSwatches::fSwatchOffset = (uint16_t)set;
}



void pyGUIControlListBox::ScrollToBegin()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ScrollToBegin();
    }
}


void pyGUIControlListBox::ScrollToEnd()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ScrollToEnd();
    }
}


void pyGUIControlListBox::SetScrollPos( int32_t pos )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->SetScrollPos(pos);
    }
}


int32_t pyGUIControlListBox::GetScrollPos()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            return plbmod->GetScrollPos();
    }
    return 0;
}


int32_t pyGUIControlListBox::GetScrollRange()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            return plbmod->GetScrollRange();
    }
    return 0;
}


void pyGUIControlListBox::LockList()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->LockList();
    }
}


void pyGUIControlListBox::UnlockList()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->UnlockList();
    }
}

void pyGUIControlListBox::Clickable()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ClearFlag(pfGUIControlMod::kIntangible);
    }
}

void pyGUIControlListBox::Unclickable()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->SetFlag(pfGUIControlMod::kIntangible);
    }
}

void    pyGUIControlListBox::AddBranch( ST::string name, bool initiallyOpen )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListTreeRoot *root = new pfGUIListTreeRoot(std::move(name));
            root->ShowChildren( initiallyOpen );
            
            if (!fBuildRoots.empty())
                fBuildRoots.back()->AddChild(root);

            fBuildRoots.emplace_back(root);
            plbmod->AddElement( root );
        }
    }
}

void    pyGUIControlListBox::CloseBranch()
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            if (!fBuildRoots.empty())
                fBuildRoots.pop_back();
        }
    }
}

void    pyGUIControlListBox::RemoveSelection( int32_t item )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->RemoveSelection(item);
    }
}

void    pyGUIControlListBox::AddSelection( int32_t item )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->AddSelection(item);
    }
}

PyObject*   pyGUIControlListBox::GetSelectionList()
{
    // create the list
    PyObject* pySL = PyList_New(0);

    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            int numItems = plbmod->GetNumElements();
            uint16_t i;
            for ( i=0; i<numItems; i++ )
            {
                pfGUIListElement* element = plbmod->GetElement(i);
                if ( element->IsSelected() )
                {
                    PyObject* element = PyLong_FromLong(i);
                    PyList_Append(pySL, element);
                    Py_XDECREF(element);
                }
            }
        }
    }
    return pySL;
}

PyObject*   pyGUIControlListBox::GetBranchList()
{
    // create the list
    PyObject* pySL = PyList_New(0);

    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            int numItems = plbmod->GetNumElements();
            uint16_t i;
            for ( i=0; i<numItems; i++ )
            {
                pfGUIListElement* element = plbmod->GetElement(i);
                if ( element )
                {
                    if ( element->GetType() == pfGUIListElement::kTreeRoot )
                    {
                        pfGUIListTreeRoot* elroot = (pfGUIListTreeRoot*)element;
                        uint16_t showing = elroot->IsShowingChildren();
                        PyObject* element = PyTuple_New(2);
                        PyTuple_SetItem(element, 0, PyLong_FromLong(i));
                        PyTuple_SetItem(element, 1, PyLong_FromLong(showing));
                        PyList_Append(pySL, element);
                        Py_XDECREF(element);
                    }
                }
            }
        }
    }
    return pySL;
}

void pyGUIControlListBox::AllowNoSelect()
{
    if (fGCkey)
    {
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (plbmod)
            plbmod->ClearFlag(pfGUIListBoxMod::kForbidNoSelection);
    }
}

void pyGUIControlListBox::DisallowNoSelect()
{
    if (fGCkey)
    {
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if (plbmod)
            plbmod->SetFlag(pfGUIListBoxMod::kForbidNoSelection);
    }
}
