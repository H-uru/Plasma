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
//	plDynamicTextMsg														//
//	Message wrapper for commands to plDynamicTextMap.						//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plDynamicTextMsg.h"
#include "hsResMgr.h"
#include "hsBitVector.h"

void	plDynamicTextMsg::SetTextColor( hsColorRGBA &c, hsBool blockRGB )
{
	hsAssert( ( fCmd & kColorCmds ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~kColorCmds;
	fCmd |= kSetTextColor; 
	fColor = c;
	fBlockRGB = blockRGB;
}

void	plDynamicTextMsg::SetFont( const char *face, Int16 size, hsBool isBold )
{
	hsAssert( ( fCmd & ( kPosCmds | kStringCmds | kFlagCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kPosCmds | kStringCmds | kFlagCmds );
	fCmd |= kSetFont; 
	fString = hsStringToWString( face );
	fX = size;
	fFlags = (UInt32)isBold;
}

void	plDynamicTextMsg::SetLineSpacing( Int16 spacing )
{
	fCmd |= kSetLineSpacing;
	fLineSpacing = spacing;
}

void	plDynamicTextMsg::SetJustify( UInt8 justifyFlags )
{
	hsAssert( ( fCmd & ( kFlagCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kFlagCmds );
	fCmd |= kSetJustify; 
	fFlags = (UInt32)justifyFlags;
}

void	plDynamicTextMsg::FillRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, hsColorRGBA &c )
{
	hsAssert( ( fCmd & ( kRectCmds | kColorCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kRectCmds | kColorCmds );
	fCmd |= kFillRect; 

	fLeft = left;
	fTop = top;
	fRight = right;
	fBottom = bottom;
	fColor = c;
}

void	plDynamicTextMsg::FrameRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, hsColorRGBA &c )
{
	hsAssert( ( fCmd & ( kRectCmds | kColorCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kRectCmds | kColorCmds );
	fCmd |= kFrameRect; 

	fLeft = left;
	fTop = top;
	fRight = right;
	fBottom = bottom;
	fColor = c;
}

void	plDynamicTextMsg::DrawString( Int16 x, Int16 y, const char *text )
{
	wchar_t *wString = hsStringToWString(text);
	DrawString(x,y,wString);
	delete [] wString;
}

void	plDynamicTextMsg::DrawString( Int16 x, Int16 y, const wchar_t *text )
{
	hsAssert( ( fCmd & ( kStringCmds | kPosCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kStringCmds | kPosCmds );
	fCmd |= kDrawString; 

	fString = TRACKED_NEW wchar_t[wcslen(text)+1];
	wcscpy( fString, text );
	fString[wcslen(text)] = L'\0';
	fX = x;
	fY = y;
}

void	plDynamicTextMsg::DrawClippedString( Int16 x, Int16 y, UInt16 clipLeft, UInt16 clipTop, UInt16 clipRight, UInt16 clipBottom, const char *text )
{
	wchar_t *wString = hsStringToWString(text);
	DrawClippedString(x,y,clipLeft,clipTop,clipRight,clipBottom,wString);
	delete [] wString;
}

void	plDynamicTextMsg::DrawClippedString( Int16 x, Int16 y, UInt16 clipLeft, UInt16 clipTop, UInt16 clipRight, UInt16 clipBottom, const wchar_t *text )
{
	hsAssert( ( fCmd & ( kStringCmds | kPosCmds | kRectCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kStringCmds | kPosCmds | kRectCmds );
	fCmd |= kDrawClippedString; 

	fString = TRACKED_NEW wchar_t[wcslen(text)+1];
	wcscpy( fString, text );
	fString[wcslen(text)] = L'\0';
	fX = x;
	fY = y;

	fLeft = clipLeft;
	fTop = clipTop;
	fRight = clipRight;
	fBottom = clipBottom;
}

void	plDynamicTextMsg::DrawWrappedString( Int16 x, Int16 y, UInt16 wrapWidth, UInt16 wrapHeight, const char *text )
{
	wchar_t *wString = hsStringToWString(text);
	DrawWrappedString(x,y,wrapWidth,wrapHeight,wString);
	delete [] wString;
}

void	plDynamicTextMsg::DrawWrappedString( Int16 x, Int16 y, UInt16 wrapWidth, UInt16 wrapHeight, const wchar_t *text )
{
	hsAssert( ( fCmd & ( kStringCmds | kPosCmds | kRectCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kStringCmds | kPosCmds | kRectCmds );
	fCmd |= kDrawWrappedString; 

	fString = TRACKED_NEW wchar_t[wcslen(text)+1];
	wcscpy( fString, text );
	fString[wcslen(text)] = L'\0';
	fX = x;
	fY = y;

	fRight = wrapWidth;
	fBottom = wrapHeight;
}

void	plDynamicTextMsg::DrawImage( Int16 x, Int16 y, plKey &image, hsBool respectAlpha )
{
	hsAssert( ( fCmd & ( kPosCmds | kFlagCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kPosCmds | kFlagCmds );
	fCmd |= kDrawImage; 

	fImageKey = image;
	fX = x;
	fY = y;
	fFlags = (UInt32)respectAlpha;
}

void	plDynamicTextMsg::DrawClippedImage( Int16 x, Int16 y, plKey &image, UInt16 clipX, UInt16 clipY, UInt16 clipWidth, UInt16 clipHeight, hsBool respectAlpha )
{
	hsAssert( ( fCmd & ( kPosCmds | kFlagCmds | kRectCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
	fCmd &= ~( kPosCmds | kFlagCmds | kRectCmds );
	fCmd |= kDrawClippedImage; 

	fImageKey = image;
	fX = x;
	fY = y;
	fLeft = clipX;
	fTop = clipY;
	fRight = clipWidth;
	fBottom = clipHeight;
	fFlags = (UInt32)respectAlpha;
}

void	plDynamicTextMsg::Read( hsStream *s, hsResMgr *mgr ) 
{ 
	plMessage::IMsgRead( s, mgr ); 

	s->ReadSwap( &fCmd );
	s->ReadSwap( &fX );
	s->ReadSwap( &fY );

	s->ReadSwap( &fLeft );
	s->ReadSwap( &fTop );
	s->ReadSwap( &fRight );
	s->ReadSwap( &fBottom );

	fClearColor.Read( s );
	fColor.Read( s );

	fString = s->ReadSafeWString();
	fImageKey = mgr->ReadKey( s );

	s->ReadSwap( &fFlags );

	s->ReadSwap( &fBlockRGB );
	s->ReadSwap( &fLineSpacing );
}
void	plDynamicTextMsg::Write( hsStream *s, hsResMgr *mgr ) 
{ 
	plMessage::IMsgWrite( s, mgr ); 

#ifdef HS_DEBUGGING
	if (fCmd & (kDrawImage | kDrawClippedImage))
	{
		hsAssert(fImageKey != nil, "plDynamicTextMsg::Write: Must set imageKey for draw operation");
	}
#endif

	s->WriteSwap( fCmd );
	s->WriteSwap( fX );
	s->WriteSwap( fY );
	
	s->WriteSwap( fLeft );
	s->WriteSwap( fTop );
	s->WriteSwap( fRight );
	s->WriteSwap( fBottom );

	fClearColor.Write( s );
	fColor.Write( s );

	s->WriteSafeWString( fString );
	mgr->WriteKey( s, fImageKey );

	s->WriteSwap( fFlags );

	s->WriteSwap( fBlockRGB );
	s->WriteSwap( fLineSpacing );
}

enum DynamicTextMsgFlags
{
	kDynTextMsgCmd,
	kDynTextMsgX,
	kDynTextMsgY,
	kDynTextMsgLeft,
	kDynTextMsgTop,
	kDynTextMsgRight,
	kDynTextMsgBottom,
	kDynTextMsgClearColor,
	kDynTextMsgColor,
	kDynTextMsgString,
	kDynTextMsgImageKey,
	kDynTextMsgFlags,
	kDynTextMsgBlockRGB,
	kDynTextMsgLineSpacing,
};

void plDynamicTextMsg::ReadVersion(hsStream* s, hsResMgr* mgr) 
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kDynTextMsgCmd))
		s->ReadSwap( &fCmd );
	if (contentFlags.IsBitSet(kDynTextMsgX))
		s->ReadSwap( &fX );
	if (contentFlags.IsBitSet(kDynTextMsgY))
		s->ReadSwap( &fY );
	if (contentFlags.IsBitSet(kDynTextMsgLeft))
		s->ReadSwap( &fLeft );
	if (contentFlags.IsBitSet(kDynTextMsgTop))
		s->ReadSwap( &fTop );
	if (contentFlags.IsBitSet(kDynTextMsgRight))
		s->ReadSwap( &fRight );
	if (contentFlags.IsBitSet(kDynTextMsgBottom))
		s->ReadSwap( &fBottom );
	if (contentFlags.IsBitSet(kDynTextMsgClearColor))
		fClearColor.Read( s );
	if (contentFlags.IsBitSet(kDynTextMsgColor))
		fColor.Read( s );
	if (contentFlags.IsBitSet(kDynTextMsgString))
		fString = s->ReadSafeWString();
	if (contentFlags.IsBitSet(kDynTextMsgImageKey))
		fImageKey = mgr->ReadKey( s );
	if (contentFlags.IsBitSet(kDynTextMsgFlags))
		s->ReadSwap( &fFlags );
	if (contentFlags.IsBitSet(kDynTextMsgBlockRGB))
		s->ReadSwap( &fBlockRGB );
	if (contentFlags.IsBitSet(kDynTextMsgLineSpacing))
		s->ReadSwap( &fLineSpacing );
}

void plDynamicTextMsg::WriteVersion(hsStream* s, hsResMgr* mgr) 
{ 
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kDynTextMsgCmd);
	contentFlags.SetBit(kDynTextMsgX);
	contentFlags.SetBit(kDynTextMsgY);
	contentFlags.SetBit(kDynTextMsgLeft);
	contentFlags.SetBit(kDynTextMsgTop);
	contentFlags.SetBit(kDynTextMsgRight);
	contentFlags.SetBit(kDynTextMsgBottom);
	contentFlags.SetBit(kDynTextMsgClearColor);
	contentFlags.SetBit(kDynTextMsgColor);
	contentFlags.SetBit(kDynTextMsgString);
	contentFlags.SetBit(kDynTextMsgImageKey);
	contentFlags.SetBit(kDynTextMsgFlags);
	contentFlags.SetBit(kDynTextMsgBlockRGB);
	contentFlags.SetBit(kDynTextMsgLineSpacing);
	contentFlags.Write(s);

	// kDynTextMsgCmd
	s->WriteSwap( fCmd );
	// kDynTextMsgX
	s->WriteSwap( fX );
	// kDynTextMsgY
	s->WriteSwap( fY );
	
	// kDynTextMsgLeft
	s->WriteSwap( fLeft );
	// kDynTextMsgTop
	s->WriteSwap( fTop );
	// kDynTextMsgRight
	s->WriteSwap( fRight );
	// kDynTextMsgBottom
	s->WriteSwap( fBottom );

	// kDynTextMsgClearColor
	fClearColor.Write( s );
	// kDynTextMsgColor
	fColor.Write( s );

	// kDynTextMsgString
	s->WriteSafeWString( fString );
	// kDynTextMsgImageKey
	mgr->WriteKey( s, fImageKey );

	// kDynTextMsgFlags
	s->WriteSwap( fFlags );

	// kDynTextMsgBlockRGB
	s->WriteSwap( fBlockRGB );
	// kDynTextMsgLineSpacing
	s->WriteSwap( fLineSpacing );

}

