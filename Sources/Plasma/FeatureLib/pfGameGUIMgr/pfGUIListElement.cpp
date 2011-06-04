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
//	pfGUIListElement Class Definitions										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIListElement.h"
#include "pfGameGUIMgr.h"

#include "pfGUIPopUpMenu.h"		// For skins

#include "../plGImage/plDynamicTextMap.h"
#include "../plGImage/hsCodecManager.h"
#include "../plPipeline/plDebugText.h"		// To quickly and hackily get the screen size in pixels
#include "hsResMgr.h"


//////////////////////////////////////////////////////////////////////////////
//// Base Stuff //////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void	pfGUIListElement::Read( hsStream *s, hsResMgr *mgr )
{
	fSelected = s->ReadBool();
}

void	pfGUIListElement::Write( hsStream *s, hsResMgr *mgr )
{
	s->WriteBool( fSelected );
}

//////////////////////////////////////////////////////////////////////////////
//// pfGUIListText ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIListText::pfGUIListText() : pfGUIListElement( kText )
{
	fText = nil;
	fJustify = kLeftJustify;
}

pfGUIListText::pfGUIListText( const char *text ) : pfGUIListElement( kText )
{
	fText = hsStringToWString(text);
	fJustify = kLeftJustify;
}

pfGUIListText::pfGUIListText( const wchar_t *text ) : pfGUIListElement( kText )
{
	fText = TRACKED_NEW wchar_t[ wcslen( text ) + 1 ];
	wcscpy( fText, text );
	fJustify = kLeftJustify;
}

pfGUIListText::~pfGUIListText()
{
	delete [] fText;
}

//// Virtuals ////////////////////////////////////////////////////////////////

void	pfGUIListText::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIListElement::Read( s, mgr );

	char *text = s->ReadSafeString();
	fText = hsStringToWString(text);
	delete [] text;
}

void	pfGUIListText::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIListElement::Write( s, mgr );

	char *text = hsWStringToString(fText);
	s->WriteSafeString(text);
	delete [] text;
}

hsBool	pfGUIListText::Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
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

void	pfGUIListText::GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
{
	*width = textGen->CalcStringWidth( GetText(), height );
	if( height != nil )
	{ 
		if( *height == 0 )
			*height = 10;		// Never allow zero height elements
		else
			*height += 0;		// Add one pixel on each side for padding (or not, 3.21.02 mcn)
	}
}

int		pfGUIListText::CompareTo( pfGUIListElement *rightSide )
{
	pfGUIListText *text = (pfGUIListText *)rightSide;
	
	if( text->fType != kText )
		return -2;

	return wcscmp( GetText(), text->GetText() );
}

void	pfGUIListText::SetText( const char *text )
{
	wchar_t *wText = hsStringToWString(text);
	SetText(wText);
	delete [] wText;
}

void	pfGUIListText::SetText( const wchar_t *text )
{
	delete [] fText;
	if( text != nil )
	{
		fText = TRACKED_NEW wchar_t[ wcslen( text ) + 1 ];
		wcscpy( fText, text );
	}
	else
		fText = nil;
}

void	pfGUIListText::SetJustify( JustifyTypes justify )
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

pfGUIListPicture::pfGUIListPicture() : pfGUIListElement( kPicture )
{
	fBorderSize = 2;
	fMipmapKey = nil;
}

pfGUIListPicture::pfGUIListPicture( plKey mipKey, hsBool respectAlpha ) : pfGUIListElement( kPicture )
{
	fBorderSize = 2;
	fMipmapKey = mipKey;
	fRespectAlpha = respectAlpha;

	plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
	if( mip != nil && mip->IsCompressed() )
	{
		// Gotta make and grab an uncompressed one
		plMipmap *uncompBuffer = hsCodecManager::Instance().CreateUncompressedMipmap( mip, hsCodecManager::k32BitDepth );
		char str[ 512 ];
		sprintf( str, "%s_uncomp", mip->GetKeyName() );
		fMipmapKey = hsgResMgr::ResMgr()->NewKey( str, uncompBuffer, fMipmapKey->GetUoid().GetLocation() );
		fMipmapKey->RefObject();
	}
}

pfGUIListPicture::~pfGUIListPicture()
{
	fMipmapKey->UnRefObject();
	fMipmapKey = nil;
}

//// Virtuals ////////////////////////////////////////////////////////////////

void	pfGUIListPicture::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIListElement::Read( s, mgr );

}

void	pfGUIListPicture::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIListElement::Write( s, mgr );

}

hsBool	pfGUIListPicture::Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
{
	if( fSelected )
		textGen->FillRect( x, y, maxWidth, maxHeight, fColors->fSelBackColor );

	plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
	if( mip != nil )
	{
		if( mip->GetWidth() + fBorderSize + fBorderSize > maxWidth || mip->GetHeight() + fBorderSize + fBorderSize > maxHeight )
			return false;

		textGen->DrawImage( x + fBorderSize, y + fBorderSize, mip, fRespectAlpha ? plDynamicTextMap::kImgBlend : plDynamicTextMap::kImgNoAlpha );
	}

	return true;
}

void	pfGUIListPicture::GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
{
	plMipmap *mip = plMipmap::ConvertNoRef( fMipmapKey->ObjectIsLoaded() );
	if( mip == nil )
	{
		*width = 16;
		if( height != nil )
			*height = 16;
	}

	*width = (UInt16)(mip->GetWidth() + fBorderSize + fBorderSize);
	if( height != nil )
		*height = (UInt16)(mip->GetHeight() + fBorderSize + fBorderSize);
}

int		pfGUIListPicture::CompareTo( pfGUIListElement *rightSide )
{
	pfGUIListPicture *text = (pfGUIListPicture *)rightSide;
	
	if( text->fType != kPicture )
		return -2;

	return -2;
}

//////////////////////////////////////////////////////////////////////////////
//// pfGUIListTreeRoot ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIListTreeRoot::pfGUIListTreeRoot() : pfGUIListElement( kTreeRoot )
{
	fText = nil;
	fShowChildren = true;
}

pfGUIListTreeRoot::pfGUIListTreeRoot( const char *text ) : pfGUIListElement( kTreeRoot )
{
	fText = hsStringToWString(text);
}

pfGUIListTreeRoot::pfGUIListTreeRoot( const wchar_t *text ) : pfGUIListElement( kTreeRoot )
{
	fText = TRACKED_NEW wchar_t[ wcslen( text ) + 1 ];
	wcscpy( fText, text );
}

pfGUIListTreeRoot::~pfGUIListTreeRoot()
{
	delete [] fText;
}

//// Virtuals ////////////////////////////////////////////////////////////////

void	pfGUIListTreeRoot::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIListElement::Read( s, mgr );

	char *temp = s->ReadSafeString();
	fText = hsStringToWString(temp);
	delete [] temp;
}

void	pfGUIListTreeRoot::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIListElement::Write( s, mgr );

	char *temp = hsWStringToString(fText);
	s->WriteSafeString( temp );
	delete [] temp;
}

hsBool	pfGUIListTreeRoot::Draw( plDynamicTextMap *textGen, UInt16 x, UInt16 y, UInt16 maxWidth, UInt16 maxHeight )
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

	if( fSkin != nil )
	{
		const pfGUISkin::pfSRect &r = fSkin->GetElement( fShowChildren ? pfGUISkin::kTreeButtonOpen : pfGUISkin::kTreeButtonClosed );

		Int16 e = ( maxHeight - r.fHeight );
		if( e < 0 )
			e = 0;
		e >>= 1;

		textGen->DrawClippedImage( x + 2, y + e, fSkin->GetTexture(), r.fX, r.fY, r.fWidth, r.fHeight, plDynamicTextMap::kImgSprite );
		x += r.fWidth + 4;
	}

	textGen->DrawClippedString( x + 4, y, GetTitle(), maxWidth - 8, maxHeight );
	return true;
}

hsBool	pfGUIListTreeRoot::MouseClicked( UInt16 localX, UInt16 localY )
{
	if( fSkin != nil )
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

void	pfGUIListTreeRoot::GetSize( plDynamicTextMap *textGen, UInt16 *width, UInt16 *height )
{
	*width = textGen->CalcStringWidth( GetTitle(), height );
	if( height != nil )
	{ 
		if( *height == 0 )
			*height = 10;		// Never allow zero height elements
		else
			*height += 0;		// Add one pixel on each side for padding (or not, 3.21.02 mcn)

		if( fSkin != nil )
		{
			UInt16 h = fSkin->GetElement( pfGUISkin::kTreeButtonClosed ).fHeight;
			if( *height < h )
				*height = h;
		}
	}

	if( fSkin != nil )
		*width += fSkin->GetElement( pfGUISkin::kTreeButtonClosed ).fWidth;
}

int		pfGUIListTreeRoot::CompareTo( pfGUIListElement *rightSide )
{
	pfGUIListTreeRoot *text = (pfGUIListTreeRoot *)rightSide;
	
	if( text->fType != kTreeRoot )
		return -2;

	return wcscmp( GetTitle(), text->GetTitle() );
}

void	pfGUIListTreeRoot::SetTitle( const char *text )
{
	wchar_t *wText = hsStringToWString(text);
	SetTitle(wText);
	delete [] wText;
}

void	pfGUIListTreeRoot::SetTitle( const wchar_t *text )
{
	delete [] fText;
	if( text != nil )
	{
		fText = TRACKED_NEW wchar_t[ wcslen( text ) + 1 ];
		wcscpy( fText, text );
	}
	else
		fText = nil;
}

void	pfGUIListTreeRoot::AddChild( pfGUIListElement *el )
{
	fChildren.Append( el );
	el->SetIndentLevel( GetIndentLevel() + 1 );
	el->SetCollapsed( !fShowChildren );
}

void	pfGUIListTreeRoot::RemoveChild( UInt32 idx )
{
	fChildren.Remove( idx );
}

void	pfGUIListTreeRoot::ShowChildren( hsBool s )
{
	UInt32 i;


	fShowChildren = s;
	for( i = 0; i < fChildren.GetCount(); i++ )
		fChildren[ i ]->SetCollapsed( !s );
}

void	pfGUIListTreeRoot::SetCollapsed( hsBool c )
{
	UInt32 i;

	
	pfGUIListElement::SetCollapsed( c );
	for( i = 0; i < fChildren.GetCount(); i++ )
		fChildren[ i ]->SetCollapsed( c ? true : !fShowChildren );
}
