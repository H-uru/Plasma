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
//  plDynamicTextMsg Header                                                 //
//  Message wrapper for commands to plDynamicTextMap.                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDynamicTextMsg_h
#define _plDynamicTextMsg_h

#include "pnMessage/plMessage.h"
#include "hsColorRGBA.h"
#include <string_theory/string>

class plDynamicTextMap;

class plDynamicTextMsg : public plMessage
{
    friend class plDynamicTextMap;

protected:
    uint16_t    fCmd;

    // Position (fX is also used for font size)
    int16_t     fX, fY;

    // A rectangle
    uint16_t    fLeft, fTop, fRight, fBottom;

    // Colors
    hsColorRGBA fClearColor;
    hsColorRGBA fColor;

    // String
    ST::string  fString;

    // Mipmap
    plKey       fImageKey;

    // Misc flags field
    uint32_t    fFlags;
    
    bool        fBlockRGB;
    int16_t     fLineSpacing;

public:
    plDynamicTextMsg() :
        plMessage(nullptr, nullptr, nullptr),
        fCmd(0), fImageKey(nullptr), fFlags(0), fBlockRGB(false)
    { }

    CLASSNAME_REGISTER( plDynamicTextMsg );
    GETINTERFACE_ANY( plDynamicTextMsg, plMessage );

    enum Commands
    {
        kClear              = 0x0001,
        kSetTextColor       = 0x0002,
        kSetFont            = 0x0004,
        kFillRect           = 0x0008,
        kFrameRect          = 0x0010,
        kDrawString         = 0x0020,
        kDrawClippedString  = 0x0040,
        kDrawWrappedString  = 0x0080,
        kFlush              = 0x0100,
        kDrawImage          = 0x0200,
        kSetJustify         = 0x0400,
        kDrawClippedImage   = 0x0800,
        kSetLineSpacing     = 0x1000,
        kPurgeImage         = 0x2000,

        // Don't use these--just masks for internal use
        kColorCmds          = kSetTextColor | kFillRect | kFrameRect,
        kStringCmds         = kSetFont | kDrawString | kDrawClippedString | kDrawWrappedString,
        kRectCmds           = kFillRect | kFrameRect | kDrawClippedString | kDrawWrappedString | kDrawClippedImage,
        kPosCmds            = kSetFont | kDrawClippedString | kDrawWrappedString | kDrawImage | kDrawClippedImage,
        kFlagCmds           = kSetFont | kDrawImage | kSetJustify | kDrawClippedImage 
    };

    // Commands
    void    ClearToColor( hsColorRGBA &c ) { fCmd |= kClear; fClearColor = c; }
    void    Flush() { fCmd |= kFlush; }
    void    PurgeImage() { fCmd |= kPurgeImage; }

    // The following are mutually exclusive commands 'cause they share some parameters
    void    SetTextColor( hsColorRGBA &c, bool blockRGB = false );
    void    SetFont(ST::string face, int16_t size, bool isBold = false);
    void    SetLineSpacing( int16_t spacing );
    void    FillRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, hsColorRGBA &c );
    void    FrameRect( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, hsColorRGBA &c );
    void    DrawString( int16_t x, int16_t y, const ST::string& text );
    void    DrawClippedString( int16_t x, int16_t y, uint16_t clipLeft, uint16_t clipTop, uint16_t clipRight, uint16_t clipBottom, const ST::string& text );
    void    DrawWrappedString( int16_t x, int16_t y, uint16_t wrapWidth, uint16_t wrapHeight, const ST::string& text );
    void    DrawImage( int16_t x, int16_t y, plKey &image, bool respectAlpha = false );
    void    DrawClippedImage( int16_t x, int16_t y, plKey &image, uint16_t clipX, uint16_t clipY, uint16_t clipWidth, uint16_t clipHeight, bool respectAlpha = false );
    void    SetJustify( uint8_t justifyFlags );
    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
    
    // WriteVersion writes the current version of this creatable and ReadVersion will read in
    // any previous version.
    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};

#endif // _plDynamicTextMsg_h
