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
//////////////////////////////////////////////
//
//
///////////////////////////////////////////////

#include "pyKey.h"

#include "pfGameGUIMgr/pfGUIListBoxMod.h"
#include "pfGameGUIMgr/pfGUIListElement.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"
#include "plGImage/plDynamicTextMap.h"

#include "pyGUIControlListBox.h"
#include "pyGUIDialog.h"
#include "pyColor.h"
#include "pyImage.h"

// a special class for different coloured list items
//
class pfColorListElement : public pfGUIListText
{
    protected:
        hsColorRGBA         fTextColor1;
        hsColorRGBA         fTextColor2;
        wchar_t             *fString1;
        wchar_t             *fString2;
        uint32_t              fInheritAlpha;
        int32_t               fOverrideFontSize;  // size of font to use (if -1 then just use scheme setting)

    public:
        enum InheritTypes
        {
            kNoInherit = 0,
            kInheritFromNormal,
            kInheritFromSelect,
            kSelectDetermined,
            kSelectUseGUIColor,
        };

        pfColorListElement( const char *string1, hsColorRGBA color1, const char *string2, hsColorRGBA color2, uint32_t inheritalpha, int32_t fontsize=-1 )
        {
            if ( string1 )
            {
                fString1 = hsStringToWString(string1);
                fText = nil;
            }
            else
            {
                fString1 = nil;
                fText = L"";
            }
            fTextColor1 = color1;
            if (string2)
                fString2 = hsStringToWString(string2);
            else
                fString2 = nil;
            fTextColor2 = color2;
            fInheritAlpha = inheritalpha;
            fJustify = kLeftJustify;
            fOverrideFontSize = fontsize;
        }

        pfColorListElement( const wchar_t *string1, hsColorRGBA color1, const wchar_t *string2, hsColorRGBA color2, uint32_t inheritalpha, int32_t fontsize=-1 )
        {
            if ( string1 )
            {
                fString1 = new wchar_t[wcslen(string1)+1];
                wcscpy(fString1,string1);
                fText = nil;
            }
            else
            {
                fString1 = nil;
                fText = L"";
            }
            fTextColor1 = color1;
            if (string2)
            {
                fString2 = new wchar_t[wcslen(string2)+1];
                wcscpy(fString2,string2);
            }
            else
                fString2 = nil;
            fTextColor2 = color2;
            fInheritAlpha = inheritalpha;
            fJustify = kLeftJustify;
            fOverrideFontSize = fontsize;
        }

        virtual ~pfColorListElement()
        {
            if ( fString1 )
            {
                delete [] fString1;
                fString1 = nil;
                fText = nil;
            }
            if ( fString2 )
                delete [] fString2;
        }

        virtual void SetText( const char *text )
        {
            if ( fString1 )
                delete [] fString1;

            if( text != nil )
                fString1 = hsStringToWString(text);
            else
                fString1 = nil;
        }

        virtual void SetText( const wchar_t *text )
        {
            if ( fString1 )
                delete [] fString1;

            if( text != nil )
            {
                fString1 = new wchar_t[wcslen(text)+1];
                wcscpy(fString1,text);
            }
            else
                fString1 = nil;
        }

        virtual hsBool  Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight )
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
            if (fString1)
            {
                if ( fOverrideFontSize != -1 )
                    textGen->SetFont( fColors->fFontFace, (uint16_t)fOverrideFontSize, fColors->fFontFlags );
                textGen->SetTextColor( textColor1, fColors->fTransparent && fColors->fBackColor.a == 0.f );
                textGen->DrawWrappedString( x + 2, y, fString1, maxWidth - 4, maxHeight );
                uint16_t width, height;
                textGen->CalcWrappedStringSize(fString1,&width,&height);
                x += 2 + width;
                if ( fString2 == nil )
                    y += height;
                if ( fOverrideFontSize != -1 )
                    textGen->SetFont( fColors->fFontFace, fColors->fFontSize, fColors->fFontFlags );
            }

            // draw the second string
            if ( fString2 )
            {
                textGen->SetTextColor( textColor2, fColors->fTransparent && fColors->fBackColor.a == 0.f );
                textGen->DrawWrappedString( x + 2, y, fString2, maxWidth - 4 - x, maxHeight );
            }

            return true;
        }

        virtual void    GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height )
        {
            hsBool wemade_string = false;
            wchar_t* thestring;
            if ( fString1 && fString2 )
            {
                size_t length = wcslen( fString1 ) + wcslen( fString2 ) + 3;
                thestring = new wchar_t[ length ];
                hsSnwprintf( thestring, length, L"%s %s", fString1, fString2 );
                wemade_string = true;
            }
            else if (fString1)
                thestring = fString1;
            else if (fString2)
                thestring = fString2;
            else
                thestring = nil;
            *width = textGen->GetVisibleWidth() - 4;

            if ( fOverrideFontSize != -1 )
                textGen->SetFont( fColors->fFontFace, (uint16_t)fOverrideFontSize, fColors->fFontFlags );
            textGen->CalcWrappedStringSize( thestring, width, height );
            if ( fOverrideFontSize != -1 )
                textGen->SetFont( fColors->fFontFace, fColors->fFontSize, fColors->fFontFlags );

            if( height != nil )
                *height += 0;
            *width += 4;
            // clean up thestring if we made it
            if ( wemade_string )
                delete [] thestring;
        }

        virtual int     CompareTo( pfGUIListElement *rightSide )
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
        pfListTextInBox( const char *text, uint32_t min_width=0, uint32_t min_height=0 ) : pfGUIListText( text )
        {
            fMinWidth = min_width;
            fMinHeight = min_height;
            fJustify = pfGUIListText::kCenter;
        }

        pfListTextInBox( const wchar_t *text, uint32_t min_width=0, uint32_t min_height=0 ) : pfGUIListText( text )
        {
            fMinWidth = min_width;
            fMinHeight = min_height;
            fJustify = pfGUIListText::kCenter;
        }

        virtual void    GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height )
        {
            *width = textGen->CalcStringWidth( GetText(), height );
            if ( *width < fMinWidth )
                *width = (uint16_t)fMinWidth;
            if( height != nil )
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
        pfListPictureInBox( plKey mipKey, uint32_t x, uint32_t y, uint32_t width, uint32_t height, hsBool respectAlpha ) : pfGUIListPicture( mipKey,respectAlpha )
        {
            fSrcX = x;
            fSrcY = y;
            fSrcWidth = width;
            fSrcHeight = height;
        }

        virtual hsBool Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight )
        {
            plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
            if( mip != nil )
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

        virtual void GetSize( plDynamicTextMap *textGen, uint16_t *width, uint16_t *height )
        {
            plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
            if( mip == nil )
            {
                *width = 16;
                if( height != nil )
                    *height = 16;
            }
            else
            {
                *width = (uint16_t)(fSrcWidth + fBorderSize + fBorderSize);
                if( height != nil )
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
                                                                            hsBool respectAlpha,
                                                                            const hsColorRGBA &primaryColor, const hsColorRGBA &secondaryColor )
                                            : pfListPictureInBox( mipKey, x, y, width, height, respectAlpha )
        {
            fPColor = primaryColor;
            fSColor = secondaryColor;
        }

        virtual hsBool Draw( plDynamicTextMap *textGen, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight )
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

pyGUIControlListBox::pyGUIControlListBox(plKey objkey) : pyGUIControl(objkey)
{
}

hsBool pyGUIControlListBox::IsGUIControlListBox(pyKey& gckey)
{
    if ( gckey.getKey() && pfGUIListBoxMod::ConvertNoRef(gckey.getKey()->ObjectIsLoaded()) )
        return true;
    return false;
}

int32_t pyGUIControlListBox::GetSelection( void )
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

void pyGUIControlListBox::Refresh( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->Refresh();
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

void pyGUIControlListBox::ClearAllElements( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ClearAllElements();
    }
}

uint16_t pyGUIControlListBox::GetNumElements( void )
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

void pyGUIControlListBox::SetElement( uint16_t idx, const char* text )
{
    wchar_t *wText = hsStringToWString(text);
    SetElementW(idx,wText);
    delete [] wText;
}

void pyGUIControlListBox::SetElementW( uint16_t idx, std::wstring text )
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
                    letext->SetText(text.c_str());
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


std::string pyGUIControlListBox::GetElement( uint16_t idx )
{
    std::wstring wRetVal = GetElementW(idx);
    char *temp = hsWStringToString(wRetVal.c_str());
    std::string retVal = temp;
    delete [] temp;
    return retVal;
}

std::wstring pyGUIControlListBox::GetElementW( uint16_t idx )
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
    return L"";
}

int16_t pyGUIControlListBox::AddString( const char *string )
{
    wchar_t *wString = hsStringToWString(string);
    int16_t retVal = AddStringW(wString);
    delete [] wString;
    return retVal;
}

int16_t pyGUIControlListBox::AddStringW( std::wstring string )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListText *element = new pfGUIListText( string.c_str() );
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t   pyGUIControlListBox::AddImage( pyImage& image, hsBool respectAlpha )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListPicture *element = new pfGUIListPicture(image.GetKey(),respectAlpha);
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}


int16_t pyGUIControlListBox::FindString( const char *toCompareTo )
{
    wchar_t *wToCompareTo = hsStringToWString(toCompareTo);
    int16_t retVal = FindStringW(wToCompareTo);
    delete [] wToCompareTo;
    return retVal;
}

int16_t pyGUIControlListBox::FindStringW( std::wstring toCompareTo )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            return plbmod->FindString(toCompareTo.c_str());
    }
    return -1;
}

int16_t pyGUIControlListBox::AddTextWColor( const char *str, pyColor& textcolor, uint32_t inheritalpha)
{
    wchar_t *wStr = hsStringToWString(str);
    int16_t retVal = AddTextWColorW(wStr,textcolor,inheritalpha);
    delete [] wStr;
    return retVal;
}

int16_t pyGUIControlListBox::AddTextWColorW( std::wstring str, pyColor& textcolor, uint32_t inheritalpha)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfColorListElement *element = new pfColorListElement( str.c_str(), textcolor.getColor(),nil,hsColorRGBA(),inheritalpha );
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t pyGUIControlListBox::AddTextWColorWSize( const char *str, pyColor& textcolor, uint32_t inheritalpha, int32_t fontsize)
{
    wchar_t *wStr = hsStringToWString(str);
    int16_t retVal = AddTextWColorWSizeW(wStr,textcolor,inheritalpha,fontsize);
    delete [] wStr;
    return retVal;
}

int16_t pyGUIControlListBox::AddTextWColorWSizeW( std::wstring str, pyColor& textcolor, uint32_t inheritalpha, int32_t fontsize)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfColorListElement *element = new pfColorListElement( str.c_str(), textcolor.getColor(),nil,hsColorRGBA(),inheritalpha, fontsize );
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

void pyGUIControlListBox::Add2TextWColor( const char *str1, pyColor& textcolor1,const char *str2, pyColor& textcolor2, uint32_t inheritalpha)
{
    wchar_t *wStr1 = hsStringToWString(str1);
    wchar_t *wStr2 = hsStringToWString(str2);
    Add2TextWColorW(wStr1,textcolor1,wStr2,textcolor2,inheritalpha);
    delete [] wStr2;
    delete [] wStr1;
}

void pyGUIControlListBox::Add2TextWColorW( std::wstring str1, pyColor& textcolor1, std::wstring str2, pyColor& textcolor2, uint32_t inheritalpha)
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfColorListElement *element = new pfColorListElement(str1.c_str(),textcolor1.getColor(),str2.c_str(),textcolor2.getColor(),inheritalpha );
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            plbmod->AddElement( element );
        }
    }
}

int16_t pyGUIControlListBox::AddStringInBox( const char *string, uint32_t min_width, uint32_t min_height )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfListTextInBox *element = new pfListTextInBox( string, min_width, min_height );
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t pyGUIControlListBox::AddStringInBoxW( std::wstring string, uint32_t min_width, uint32_t min_height )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfListTextInBox *element = new pfListTextInBox( string.c_str(), min_width, min_height );
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t   pyGUIControlListBox::AddImageInBox( pyImage& image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, hsBool respectAlpha )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfListPictureInBox *element = new pfListPictureInBox(image.GetKey(),x,y,width,height,respectAlpha);
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
            return plbmod->AddElement( element );
        }
    }
    return -1;
}

int16_t   pyGUIControlListBox::AddImageAndSwatchesInBox( pyImage& image, uint32_t x, uint32_t y, uint32_t width, uint32_t height, hsBool respectAlpha,
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
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
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



void pyGUIControlListBox::ScrollToBegin( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ScrollToBegin();
    }
}


void pyGUIControlListBox::ScrollToEnd( void )
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


int32_t pyGUIControlListBox::GetScrollPos( void )
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


int32_t pyGUIControlListBox::GetScrollRange( void )
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


void pyGUIControlListBox::LockList( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->LockList();
    }
}


void pyGUIControlListBox::UnlockList( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->UnlockList();
    }
}

void pyGUIControlListBox::Clickable( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->ClearFlag(pfGUIControlMod::kIntangible);
    }
}

void pyGUIControlListBox::Unclickable( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
            plbmod->SetFlag(pfGUIControlMod::kIntangible);
    }
}

void    pyGUIControlListBox::AddBranch( const char *name, hsBool initiallyOpen )
{
    wchar_t *wName = hsStringToWString(name);
    AddBranchW(wName,initiallyOpen);
    delete [] wName;
}

void    pyGUIControlListBox::AddBranchW( std::wstring name, hsBool initiallyOpen )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            pfGUIListTreeRoot *root = new pfGUIListTreeRoot( name.c_str() );
            root->ShowChildren( initiallyOpen );
            
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( root );

            fBuildRoots.Append( root );
            plbmod->AddElement( root );
        }
    }
}

void    pyGUIControlListBox::CloseBranch( void )
{
    if ( fGCkey )
    {
        // get the pointer to the modifier
        pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
        if ( plbmod )
        {
            if( fBuildRoots.GetCount() > 0 )
                fBuildRoots.Remove( fBuildRoots.GetCount() - 1 );
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
                    PyObject* element = PyInt_FromLong(i);
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
                        PyTuple_SetItem(element, 0, PyInt_FromLong(i));
                        PyTuple_SetItem(element, 1, PyInt_FromLong(showing));
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
