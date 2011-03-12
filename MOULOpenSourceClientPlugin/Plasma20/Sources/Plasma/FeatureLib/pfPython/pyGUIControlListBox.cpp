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
//////////////////////////////////////////////
//
//
///////////////////////////////////////////////

#include "pyKey.h"

#include "../pfGameGUIMgr/pfGUIListBoxMod.h"
#include "../pfGameGUIMgr/pfGUIListElement.h"
#include "../pfGameGUIMgr/pfGUIDialogMod.h"
#include "../plGImage/plDynamicTextMap.h"

#include "pyGUIControlListBox.h"
#include "pyGUIDialog.h"
#include "pyColor.h"
#include "pyImage.h"

// a special class for different coloured list items
//
class pfColorListElement : public pfGUIListText
{
	protected:
		hsColorRGBA			fTextColor1;
		hsColorRGBA			fTextColor2;
		wchar_t				*fString1;
		wchar_t				*fString2;
		UInt32				fInheritAlpha;
		Int32				fOverrideFontSize;	// size of font to use (if -1 then just use scheme setting)

	public:
		enum InheritTypes
		{
			kNoInherit = 0,
			kInheritFromNormal,
			kInheritFromSelect,
			kSelectDetermined,
			kSelectUseGUIColor,
		};

		pfColorListElement( const char *string1, hsColorRGBA color1, const char *string2, hsColorRGBA color2, UInt32 inheritalpha, Int32 fontsize=-1 )
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

		pfColorListElement( const wchar_t *string1, hsColorRGBA color1, const wchar_t *string2, hsColorRGBA color2, UInt32 inheritalpha, Int32 fontsize=-1 )
		{
			if ( string1 )
			{
				fString1 = TRACKED_NEW wchar_t[wcslen(string1)+1];
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
				fString2 = TRACKED_NEW wchar_t[wcslen(string2)+1];
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
				fString1 = TRACKED_NEW wchar_t[wcslen(text)+1];
				wcscpy(fString1,text);
			}
			else
				fString1 = nil;
		}

		virtual hsBool	Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
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
					textGen->SetFont( fColors->fFontFace, (UInt16)fOverrideFontSize, fColors->fFontFlags );
				textGen->SetTextColor( textColor1, fColors->fTransparent && fColors->fBackColor.a == 0.f );
				textGen->DrawWrappedString( x + 2, y, fString1, maxWidth - 4, maxHeight );
				UInt16 width, height;
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

		virtual void	GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
		{
			hsBool wemade_string = false;
			wchar_t* thestring;
			if ( fString1 && fString2 )
			{
				thestring = TRACKED_NEW wchar_t[ wcslen( fString1 ) + wcslen( fString2 ) + 3 ];
				swprintf( thestring, L"%s %s", fString1, fString2 );
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
				textGen->SetFont( fColors->fFontFace, (UInt16)fOverrideFontSize, fColors->fFontFlags );
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

		virtual int		CompareTo( pfGUIListElement *rightSide )
		{
			return -2;
		}

};

class pfListTextInBox : public pfGUIListText
{
	protected:
		UInt32				fMinWidth;
		UInt32				fMinHeight;

	public:
		pfListTextInBox::pfListTextInBox( const char *text, UInt32 min_width=0, UInt32 min_height=0 ) : pfGUIListText( text )
		{
			fMinWidth = min_width;
			fMinHeight = min_height;
			fJustify = pfGUIListText::kCenter;
		}

		pfListTextInBox::pfListTextInBox( const wchar_t *text, UInt32 min_width=0, UInt32 min_height=0 ) : pfGUIListText( text )
		{
			fMinWidth = min_width;
			fMinHeight = min_height;
			fJustify = pfGUIListText::kCenter;
		}

		virtual void	GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
		{
			*width = textGen->CalcStringWidth( GetText(), height );
			if ( *width < fMinWidth )
				*width = (UInt16)fMinWidth;
			if( height != nil )
			{ 
				if( *height == 0 )
					*height = 10;		// Never allow zero height elements
				else
					*height += 0;		// Add one pixel on each side for padding (or not, 3.21.02 mcn)
				if ( *height < fMinHeight )
					*height = (UInt16)fMinHeight;
			}
		}
};

class pfListPictureInBox : public pfGUIListPicture
{
	protected:
		UInt32				fSrcX;
		UInt32				fSrcY;
		UInt32				fSrcWidth;
		UInt32				fSrcHeight;

	public:
		pfListPictureInBox::pfListPictureInBox( plKey mipKey, UInt32 x, UInt32 y, UInt32 width, UInt32 height, hsBool respectAlpha ) : pfGUIListPicture( mipKey,respectAlpha )
		{
			fSrcX = x;
			fSrcY = y;
			fSrcWidth = width;
			fSrcHeight = height;
		}

		virtual hsBool Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
		{
			plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
			if( mip != nil )
			{
				if( fSrcWidth + fBorderSize + fBorderSize > maxWidth || fSrcHeight + fBorderSize + fBorderSize > maxHeight )
					return false;

				if( fSelected )
					textGen->FillRect( x, y, (UInt16)fSrcWidth, (UInt16)fSrcHeight, fColors->fSelForeColor );
// hack!!!! This is to get the non-selected items to show up....
//  ... they need some kinda background to alpha to <whatever>
				else
				{
					if ( fRespectAlpha )
					{
						hsColorRGBA backcolor = fColors->fBackColor;
						backcolor.a = 1.0;
						textGen->FillRect( x, y, (UInt16)fSrcWidth, (UInt16)fSrcHeight, backcolor );
					}
				}
// end of hack

				textGen->DrawClippedImage( x + fBorderSize, y + fBorderSize, mip, (UInt16)fSrcX, (UInt16)fSrcY, (UInt16)fSrcWidth, (UInt16)fSrcHeight, 
											fRespectAlpha ? plDynamicTextMap::kImgBlend : plDynamicTextMap::kImgNoAlpha );
			}

			return true;
		}

		virtual void GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
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
				*width = (UInt16)(fSrcWidth + fBorderSize + fBorderSize);
				if( height != nil )
					*height = (UInt16)(fSrcHeight + fBorderSize + fBorderSize);
			}
		}
};

class pfListPictureInBoxWithSwatches : public pfListPictureInBox
{
	protected:
		hsColorRGBA				fPColor, fSColor;

	public:

		static UInt16	fSwatchSize, fSwatchOffset;

		pfListPictureInBoxWithSwatches::pfListPictureInBoxWithSwatches( plKey mipKey, UInt32 x, UInt32 y, 
																			UInt32 width, UInt32 height, 
																			hsBool respectAlpha,
																			const hsColorRGBA &primaryColor, const hsColorRGBA &secondaryColor )
											: pfListPictureInBox( mipKey, x, y, width, height, respectAlpha )
		{
			fPColor = primaryColor;
			fSColor = secondaryColor;
		}

		virtual hsBool Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
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
UInt16	pfListPictureInBoxWithSwatches::fSwatchSize = 16;
UInt16	pfListPictureInBoxWithSwatches::fSwatchOffset = 5;



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

Int32 pyGUIControlListBox::GetSelection( void )
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

void pyGUIControlListBox::SetSelection( Int32 item )
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

void pyGUIControlListBox::RemoveElement( UInt16 index )
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

UInt16 pyGUIControlListBox::GetNumElements( void )
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

void pyGUIControlListBox::SetElement( UInt16 idx, const char* text )
{
	wchar_t *wText = hsStringToWString(text);
	SetElementW(idx,wText);
	delete [] wText;
}

void pyGUIControlListBox::SetElementW( UInt16 idx, std::wstring text )
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

void pyGUIControlListBox::SetStringJustify( UInt16 idx, UInt32 justify)
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


std::string pyGUIControlListBox::GetElement( UInt16 idx )
{
	std::wstring wRetVal = GetElementW(idx);
	char *temp = hsWStringToString(wRetVal.c_str());
	std::string retVal = temp;
	delete [] temp;
	return retVal;
}

std::wstring pyGUIControlListBox::GetElementW( UInt16 idx )
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

Int16 pyGUIControlListBox::AddString( const char *string )
{
	wchar_t *wString = hsStringToWString(string);
	Int16 retVal = AddStringW(wString);
	delete [] wString;
	return retVal;
}

Int16 pyGUIControlListBox::AddStringW( std::wstring string )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfGUIListText *element = TRACKED_NEW pfGUIListText( string.c_str() );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

Int16	pyGUIControlListBox::AddImage( pyImage& image, hsBool respectAlpha )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfGUIListPicture *element = TRACKED_NEW pfGUIListPicture(image.GetKey(),respectAlpha);
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}


Int16 pyGUIControlListBox::FindString( const char *toCompareTo )
{
	wchar_t *wToCompareTo = hsStringToWString(toCompareTo);
	Int16 retVal = FindStringW(wToCompareTo);
	delete [] wToCompareTo;
	return retVal;
}

Int16 pyGUIControlListBox::FindStringW( std::wstring toCompareTo )
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

Int16 pyGUIControlListBox::AddTextWColor( const char *str, pyColor& textcolor, UInt32 inheritalpha)
{
	wchar_t *wStr = hsStringToWString(str);
	Int16 retVal = AddTextWColorW(wStr,textcolor,inheritalpha);
	delete [] wStr;
	return retVal;
}

Int16 pyGUIControlListBox::AddTextWColorW( std::wstring str, pyColor& textcolor, UInt32 inheritalpha)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfColorListElement *element = TRACKED_NEW pfColorListElement( str.c_str(), textcolor.getColor(),nil,hsColorRGBA(),inheritalpha );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

Int16 pyGUIControlListBox::AddTextWColorWSize( const char *str, pyColor& textcolor, UInt32 inheritalpha, Int32 fontsize)
{
	wchar_t *wStr = hsStringToWString(str);
	Int16 retVal = AddTextWColorWSizeW(wStr,textcolor,inheritalpha,fontsize);
	delete [] wStr;
	return retVal;
}

Int16 pyGUIControlListBox::AddTextWColorWSizeW( std::wstring str, pyColor& textcolor, UInt32 inheritalpha, Int32 fontsize)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfColorListElement *element = TRACKED_NEW pfColorListElement( str.c_str(), textcolor.getColor(),nil,hsColorRGBA(),inheritalpha, fontsize );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

void pyGUIControlListBox::Add2TextWColor( const char *str1, pyColor& textcolor1,const char *str2, pyColor& textcolor2, UInt32 inheritalpha)
{
	wchar_t *wStr1 = hsStringToWString(str1);
	wchar_t *wStr2 = hsStringToWString(str2);
	Add2TextWColorW(wStr1,textcolor1,wStr2,textcolor2,inheritalpha);
	delete [] wStr2;
	delete [] wStr1;
}

void pyGUIControlListBox::Add2TextWColorW( std::wstring str1, pyColor& textcolor1, std::wstring str2, pyColor& textcolor2, UInt32 inheritalpha)
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfColorListElement *element = TRACKED_NEW pfColorListElement(str1.c_str(),textcolor1.getColor(),str2.c_str(),textcolor2.getColor(),inheritalpha );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			plbmod->AddElement( element );
		}
	}
}

Int16 pyGUIControlListBox::AddStringInBox( const char *string, UInt32 min_width, UInt32 min_height )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfListTextInBox *element = TRACKED_NEW pfListTextInBox( string, min_width, min_height );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

Int16 pyGUIControlListBox::AddStringInBoxW( std::wstring string, UInt32 min_width, UInt32 min_height )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfListTextInBox *element = TRACKED_NEW pfListTextInBox( string.c_str(), min_width, min_height );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

Int16	pyGUIControlListBox::AddImageInBox( pyImage& image, UInt32 x, UInt32 y, UInt32 width, UInt32 height, hsBool respectAlpha )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfListPictureInBox *element = TRACKED_NEW pfListPictureInBox(image.GetKey(),x,y,width,height,respectAlpha);
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

Int16	pyGUIControlListBox::AddImageAndSwatchesInBox( pyImage& image, UInt32 x, UInt32 y, UInt32 width, UInt32 height, hsBool respectAlpha,
														pyColor &primary, pyColor &secondary )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfListPictureInBoxWithSwatches *element = TRACKED_NEW pfListPictureInBoxWithSwatches( image.GetKey(),x,y,
																						width,height,respectAlpha,
																						primary.getColor(), secondary.getColor() );
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( element );
			return plbmod->AddElement( element );
		}
	}
	return -1;
}

void	pyGUIControlListBox::SetSwatchSize( UInt32 size )
{
	pfListPictureInBoxWithSwatches::fSwatchSize = (UInt16)size;
}

void	pyGUIControlListBox::SetSwatchEdgeOffset( UInt32 set )
{
	pfListPictureInBoxWithSwatches::fSwatchOffset = (UInt16)set;
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


void pyGUIControlListBox::SetScrollPos( Int32 pos )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
			plbmod->SetScrollPos(pos);
	}
}


Int32 pyGUIControlListBox::GetScrollPos( void )
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


Int32 pyGUIControlListBox::GetScrollRange( void )
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

void	pyGUIControlListBox::AddBranch( const char *name, hsBool initiallyOpen )
{
	wchar_t *wName = hsStringToWString(name);
	AddBranchW(wName,initiallyOpen);
	delete [] wName;
}

void	pyGUIControlListBox::AddBranchW( std::wstring name, hsBool initiallyOpen )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
		{
			pfGUIListTreeRoot *root = TRACKED_NEW pfGUIListTreeRoot( name.c_str() );
			root->ShowChildren( initiallyOpen );
			
			if( fBuildRoots.GetCount() > 0 )
				fBuildRoots[ fBuildRoots.GetCount() - 1 ]->AddChild( root );

			fBuildRoots.Append( root );
			plbmod->AddElement( root );
		}
	}
}

void	pyGUIControlListBox::CloseBranch( void )
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

void	pyGUIControlListBox::RemoveSelection( Int32 item )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
			plbmod->RemoveSelection(item);
	}
}

void	pyGUIControlListBox::AddSelection( Int32 item )
{
	if ( fGCkey )
	{
		// get the pointer to the modifier
		pfGUIListBoxMod* plbmod = pfGUIListBoxMod::ConvertNoRef(fGCkey->ObjectIsLoaded());
		if ( plbmod )
			plbmod->AddSelection(item);
	}
}

PyObject*	pyGUIControlListBox::GetSelectionList()
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
			UInt16 i;
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

PyObject*	pyGUIControlListBox::GetBranchList()
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
			UInt16 i;
			for ( i=0; i<numItems; i++ )
			{
				pfGUIListElement* element = plbmod->GetElement(i);
				if ( element )
				{
					if ( element->GetType() == pfGUIListElement::kTreeRoot )
					{
						pfGUIListTreeRoot* elroot = (pfGUIListTreeRoot*)element;
						UInt16 showing = elroot->IsShowingChildren();
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