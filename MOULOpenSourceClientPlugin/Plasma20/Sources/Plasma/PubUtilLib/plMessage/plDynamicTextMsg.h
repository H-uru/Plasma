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
//	plDynamicTextMsg Header													//
//	Message wrapper for commands to plDynamicTextMap.						//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDynamicTextMsg_h
#define _plDynamicTextMsg_h

#include "../pnMessage/plMessage.h"
#include "hsColorRGBA.h"

class plDynamicTextMap;

class plDynamicTextMsg : public plMessage
{
	friend class plDynamicTextMap;

protected:
	UInt16		fCmd;

	// Position (fX is also used for font size)
	Int16		fX, fY;	

	// A rectangle
	UInt16		fLeft, fTop, fRight, fBottom;

	// Colors
	hsColorRGBA	fClearColor;
	hsColorRGBA	fColor;

	// String
	wchar_t		*fString;

	// Mipmap
	plKey		fImageKey;

	// Misc flags field
	UInt32		fFlags;
	
	hsBool		fBlockRGB;
	Int16		fLineSpacing;

public:
	plDynamicTextMsg() : plMessage( nil, nil, nil ) { fCmd = 0; fString = nil; fImageKey = nil; fFlags = 0; fBlockRGB = false; }
	~plDynamicTextMsg() { delete [] fString; }

	CLASSNAME_REGISTER( plDynamicTextMsg );
	GETINTERFACE_ANY( plDynamicTextMsg, plMessage );

	enum Commands
	{
		kClear				= 0x0001,
		kSetTextColor		= 0x0002,
		kSetFont			= 0x0004,
		kFillRect			= 0x0008,
		kFrameRect			= 0x0010,
		kDrawString			= 0x0020,
		kDrawClippedString	= 0x0040,
		kDrawWrappedString	= 0x0080,
		kFlush				= 0x0100,
		kDrawImage			= 0x0200,
		kSetJustify			= 0x0400,
		kDrawClippedImage	= 0x0800,
		kSetLineSpacing		= 0x1000,
		kPurgeImage			= 0x2000,

		// Don't use these--just masks for internal use
		kColorCmds			= kSetTextColor | kFillRect | kFrameRect,
		kStringCmds			= kSetFont | kDrawString | kDrawClippedString | kDrawWrappedString,
		kRectCmds			= kFillRect | kFrameRect | kDrawClippedString | kDrawWrappedString | kDrawClippedImage,
		kPosCmds			= kSetFont | kDrawClippedString | kDrawWrappedString | kDrawImage | kDrawClippedImage,
		kFlagCmds			= kSetFont | kDrawImage | kSetJustify | kDrawClippedImage 
	};

	// Commands
	void	ClearToColor( hsColorRGBA &c ) { fCmd |= kClear; fClearColor = c; }
	void	Flush( void ) { fCmd |= kFlush; }
	void	PurgeImage( void ) { fCmd |= kPurgeImage; }

	// The following are mutually exclusive commands 'cause they share some parameters
	void	SetTextColor( hsColorRGBA &c, hsBool blockRGB = false );
	void	SetFont( const char *face, Int16 size, hsBool isBold = false );
	void	SetLineSpacing( Int16 spacing );
	void	FillRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, hsColorRGBA &c );
	void	FrameRect( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom, hsColorRGBA &c );
	void	DrawString( Int16 x, Int16 y, const char *text );
	void	DrawString( Int16 x, Int16 y, const wchar_t *text );
	void	DrawClippedString( Int16 x, Int16 y, UInt16 clipLeft, UInt16 clipTop, UInt16 clipRight, UInt16 clipBottom, const char *text );
	void	DrawClippedString( Int16 x, Int16 y, UInt16 clipLeft, UInt16 clipTop, UInt16 clipRight, UInt16 clipBottom, const wchar_t *text );
	void	DrawWrappedString( Int16 x, Int16 y, UInt16 wrapWidth, UInt16 wrapHeight, const char *text );
	void	DrawWrappedString( Int16 x, Int16 y, UInt16 wrapWidth, UInt16 wrapHeight, const wchar_t *text );
	void	DrawImage( Int16 x, Int16 y, plKey &image, hsBool respectAlpha = false );
	void	DrawClippedImage( Int16 x, Int16 y, plKey &image, UInt16 clipX, UInt16 clipY, UInt16 clipWidth, UInt16 clipHeight, hsBool respectAlpha = false );
	void	SetJustify( UInt8 justifyFlags );
	// IO
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
	
	// WriteVersion writes the current version of this creatable and ReadVersion will read in
	// any previous version.
	virtual void ReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void WriteVersion(hsStream* s, hsResMgr* mgr);
};

#endif // _plDynamicTextMsg_h
