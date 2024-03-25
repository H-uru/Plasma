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
//  plDynamicTextMsg                                                        //
//  Message wrapper for commands to plDynamicTextMap.                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "hsResMgr.h"

#include <utility>

#include "plDynamicTextMsg.h"

void    plDynamicTextMsg::SetTextColor( hsColorRGBA &c, bool blockRGB )
{
    hsAssert( ( fCmd & kColorCmds ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~kColorCmds;
    fCmd |= kSetTextColor; 
    fColor = c;
    fBlockRGB = blockRGB;
}

void    plDynamicTextMsg::SetFont(ST::string face, int16_t size, bool isBold)
{
    hsAssert( ( fCmd & ( kPosCmds | kStringCmds | kFlagCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~( kPosCmds | kStringCmds | kFlagCmds );
    fCmd |= kSetFont; 
    fString = std::move(face);
    fX = size;
    fFlags = (uint32_t)isBold;
}

void    plDynamicTextMsg::SetLineSpacing( int16_t spacing )
{
    fCmd |= kSetLineSpacing;
    fLineSpacing = spacing;
}

void    plDynamicTextMsg::SetJustify( uint8_t justifyFlags )
{
    hsAssert( ( fCmd & ( kFlagCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~( kFlagCmds );
    fCmd |= kSetJustify; 
    fFlags = (uint32_t)justifyFlags;
}

void    plDynamicTextMsg::FillRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, hsColorRGBA &c )
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

void    plDynamicTextMsg::FrameRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, hsColorRGBA &c )
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

void    plDynamicTextMsg::DrawString( int16_t x, int16_t y, const ST::string& text )
{
    hsAssert( ( fCmd & ( kStringCmds | kPosCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~( kStringCmds | kPosCmds );
    fCmd |= kDrawString; 

    fString = text;
    fX = x;
    fY = y;
}

void    plDynamicTextMsg::DrawClippedString( int16_t x, int16_t y, uint16_t clipLeft, uint16_t clipTop, uint16_t clipRight, uint16_t clipBottom, const ST::string& text )
{
    hsAssert( ( fCmd & ( kStringCmds | kPosCmds | kRectCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~( kStringCmds | kPosCmds | kRectCmds );
    fCmd |= kDrawClippedString; 

    fString = text;
    fX = x;
    fY = y;

    fLeft = clipLeft;
    fTop = clipTop;
    fRight = clipRight;
    fBottom = clipBottom;
}

void    plDynamicTextMsg::DrawWrappedString( int16_t x, int16_t y, uint16_t wrapWidth, uint16_t wrapHeight, const ST::string& text )
{
    hsAssert( ( fCmd & ( kStringCmds | kPosCmds | kRectCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~( kStringCmds | kPosCmds | kRectCmds );
    fCmd |= kDrawWrappedString; 

    fString = text;
    fX = x;
    fY = y;

    fRight = wrapWidth;
    fBottom = wrapHeight;
}

void    plDynamicTextMsg::DrawImage( int16_t x, int16_t y, plKey &image, bool respectAlpha )
{
    hsAssert( ( fCmd & ( kPosCmds | kFlagCmds ) ) == 0, "Attempting to issue conflicting drawText commands" );
    fCmd &= ~( kPosCmds | kFlagCmds );
    fCmd |= kDrawImage; 

    fImageKey = image;
    fX = x;
    fY = y;
    fFlags = (uint32_t)respectAlpha;
}

void    plDynamicTextMsg::DrawClippedImage( int16_t x, int16_t y, plKey &image, uint16_t clipX, uint16_t clipY, uint16_t clipWidth, uint16_t clipHeight, bool respectAlpha )
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
    fFlags = (uint32_t)respectAlpha;
}

void    plDynamicTextMsg::Read( hsStream *s, hsResMgr *mgr ) 
{ 
    plMessage::IMsgRead( s, mgr ); 

    s->ReadLE16(&fCmd);
    s->ReadLE16(&fX);
    s->ReadLE16(&fY);

    s->ReadLE16(&fLeft);
    s->ReadLE16(&fTop);
    s->ReadLE16(&fRight);
    s->ReadLE16(&fBottom);

    fClearColor.Read( s );
    fColor.Read( s );

    fString = s->ReadSafeWString();
    fImageKey = mgr->ReadKey( s );

    s->ReadLE32(&fFlags);

    fBlockRGB = s->ReadBOOL();
    s->ReadLE16(&fLineSpacing);
}

void    plDynamicTextMsg::Write( hsStream *s, hsResMgr *mgr ) 
{ 
    plMessage::IMsgWrite( s, mgr ); 

#ifdef HS_DEBUGGING
    if (fCmd & (kDrawImage | kDrawClippedImage))
    {
        hsAssert(fImageKey != nullptr, "plDynamicTextMsg::Write: Must set imageKey for draw operation");
    }
#endif

    s->WriteLE16(fCmd);
    s->WriteLE16(fX);
    s->WriteLE16(fY);
    
    s->WriteLE16(fLeft);
    s->WriteLE16(fTop);
    s->WriteLE16(fRight);
    s->WriteLE16(fBottom);

    fClearColor.Write( s );
    fColor.Write( s );

    s->WriteSafeWString(fString);

    mgr->WriteKey( s, fImageKey );

    s->WriteLE32(fFlags);

    s->WriteBOOL(fBlockRGB);
    s->WriteLE16(fLineSpacing);
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
        s->ReadLE16(&fCmd);
    if (contentFlags.IsBitSet(kDynTextMsgX))
        s->ReadLE16(&fX);
    if (contentFlags.IsBitSet(kDynTextMsgY))
        s->ReadLE16(&fY);
    if (contentFlags.IsBitSet(kDynTextMsgLeft))
        s->ReadLE16(&fLeft);
    if (contentFlags.IsBitSet(kDynTextMsgTop))
        s->ReadLE16(&fTop);
    if (contentFlags.IsBitSet(kDynTextMsgRight))
        s->ReadLE16(&fRight);
    if (contentFlags.IsBitSet(kDynTextMsgBottom))
        s->ReadLE16(&fBottom);
    if (contentFlags.IsBitSet(kDynTextMsgClearColor))
        fClearColor.Read( s );
    if (contentFlags.IsBitSet(kDynTextMsgColor))
        fColor.Read( s );
    if (contentFlags.IsBitSet(kDynTextMsgString))
        fString = s->ReadSafeWString();
    if (contentFlags.IsBitSet(kDynTextMsgImageKey))
        fImageKey = mgr->ReadKey( s );
    if (contentFlags.IsBitSet(kDynTextMsgFlags))
        s->ReadLE32(&fFlags);
    if (contentFlags.IsBitSet(kDynTextMsgBlockRGB))
        fBlockRGB = s->ReadBOOL();
    if (contentFlags.IsBitSet(kDynTextMsgLineSpacing))
        s->ReadLE16(&fLineSpacing);
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
    s->WriteLE16(fCmd);
    // kDynTextMsgX
    s->WriteLE16(fX);
    // kDynTextMsgY
    s->WriteLE16(fY);
    
    // kDynTextMsgLeft
    s->WriteLE16(fLeft);
    // kDynTextMsgTop
    s->WriteLE16(fTop);
    // kDynTextMsgRight
    s->WriteLE16(fRight);
    // kDynTextMsgBottom
    s->WriteLE16(fBottom);

    // kDynTextMsgClearColor
    fClearColor.Write( s );
    // kDynTextMsgColor
    fColor.Write( s );

    // kDynTextMsgString
    s->WriteSafeWString( fString );
    // kDynTextMsgImageKey
    mgr->WriteKey( s, fImageKey );

    // kDynTextMsgFlags
    s->WriteLE32(fFlags);

    // kDynTextMsgBlockRGB
    s->WriteBOOL( fBlockRGB );
    // kDynTextMsgLineSpacing
    s->WriteLE16(fLineSpacing);
}

