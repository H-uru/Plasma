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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtBase64.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private
*
***/

static const char kEncode64[] = {
//   0000000000111111111122222222223333333333444444444455555555556666
//   0123456789012345678901234567890123456789012345678901234567890123
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
};

// Note that the decode table contains one special entry:
// The '-' character (0x2d) maps to 63 just like '/' (0x2f)
// so that URLs will work with Base64Decode when we implement them.
#define kTerminator 127
#define xx kTerminator
static const char kDecode64[] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,62,xx,63,xx,63,
    52,53,54,55,56,57,58,59,60,61,xx,xx,xx,xx,xx,xx,
    xx, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,xx,xx,xx,xx,xx,
    xx,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
    xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,
};
#undef xx

static const char kFillchar = '=';


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
unsigned Base64Encode (
    unsigned    srcChars,
    const uint8_t  srcData[],
    unsigned    dstChars,
    char *      dstData
) {
    ASSERT(srcData);
    ASSERT(dstChars >= Base64EncodeSize(srcChars));
    ASSERT(dstData);
    
    const char * dstBase = dstData;
    const uint8_t * srcTerm = srcData + srcChars;
    for (;;) switch (srcTerm - srcData) {
        case 0:
            *dstData++ = 0;
        return dstData - dstBase;

        case 1:
            *dstData++ = kEncode64[ ((srcData[0] >> 2) & 0x3f) ];
            *dstData++ = kEncode64[ ((srcData[0] << 4) & 0x30) ];
            *dstData++ = kFillchar;
            *dstData++ = kFillchar;
            *dstData++ = 0;
        return dstData - dstBase;

        case 2:
            *dstData++ = kEncode64[ ((srcData[0] >> 2) & 0x3f) ];
            *dstData++ = kEncode64[ ((srcData[0] << 4) & 0x30) + ((srcData[1] >> 4) & 0x0f) ];
            *dstData++ = kEncode64[ ((srcData[1] << 2) & 0x3c) ];
            *dstData++ = kFillchar;
            *dstData++ = 0;
        return dstData - dstBase;

        default:
            *dstData++ = kEncode64[ ((srcData[0] >> 2) & 0x3f) ];
            *dstData++ = kEncode64[ ((srcData[0] << 4) & 0x30) + ((srcData[1] >> 4) & 0x0f) ];
            *dstData++ = kEncode64[ ((srcData[1] << 2) & 0x3c) + ((srcData[2] >> 6) & 0x03) ];
            *dstData++ = kEncode64[ (srcData[2] & 0x3f) ];
            srcData   += 3;
        break;
    }
}

//============================================================================
unsigned Base64Decode (
    unsigned    srcChars,
    const char  srcData[],
    unsigned    dstChars,
    uint8_t *      dstData
) {
    ASSERT(srcData);
    ASSERT(dstChars >= Base64DecodeSize(srcChars, srcData));
    ASSERT(dstData);

    const uint8_t * dstBase = dstData;
    const char * srcTerm = srcData + srcChars;
    while (srcTerm - srcData >= 4) {

        *dstData++ = (uint8_t) (
            (kDecode64[srcData[0]] << 2 & 0xfc)
           +(kDecode64[srcData[1]] >> 4 & 0x03)
        );

        if (kDecode64[srcData[2]] == kTerminator)
            break;

        *dstData++ = (uint8_t) (
            (kDecode64[srcData[1]] << 4 & 0xf0)
           +(kDecode64[srcData[2]] >> 2 & 0x0f)
        );

        if (kDecode64[srcData[3]] == kTerminator)
            break;

        *dstData++ = (uint8_t) (
            (kDecode64[srcData[2]] << 6 & 0xc0)
           +(kDecode64[srcData[3]])
        );

        srcData += 4;
    }

    return dstData - dstBase;
}
