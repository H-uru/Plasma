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
#ifndef plControlDefinition_inc
#define plControlDefinition_inc

#include "plControlEventCodes.h"
#include "plKeyDef.h"
#include "hsGeometry3.h"

// flags for control event codes
enum 
{
    kControlFlagNormal              =   0x00000001,
    kControlFlagNoRepeat            =   0x00000002,
    kControlFlagDownEvent           =   0x00000004, 
    kControlFlagUpEvent             =   0x00000008,
    kControlFlagToggle              =   0x00000010,
    kControlFlagXAxisEvent          =   0x00000020,
    kControlFlagYAxisEvent          =   0x00000040,
    kControlFlagRangePos            =   0x00000080,
    kControlFlagRangeNeg            =   0x00000100,
    //                              =   0x00000200,
    //                              =   0x00000400,
    kControlFlagConsoleCommand      =   0x00000800,
    kControlFlagLeftButton          =   0x00001000,
    kControlFlagRightButton         =   0x00002000,
    kControlFlagLeftButtonEx        =   0x00004000,
    kControlFlagRightButtonEx       =   0x00008000,
    kControlFlagBoxDisable          =   0x00010000,
    kControlFlagInBox               =   0x00020000,
    kControlFlagCapsLock            =   0x00040000,
    kControlFlagNetPropagate        =   0x00080000,  // Not supported anymore, but save the flag.
    kControlFlagLeftButtonUp        =   0x00100000,
    kControlFlagRightButtonUp       =   0x00200000,
    kControlFlagRightButtonRepeat   =   0x00400000,
    kControlFlagLeftButtonRepeat    =   0x00800000,
    kControlFlagNoDeactivate        =   0x01000000,
    kControlFlagDelta               =   0x02000000,
    kControlFlagMiddleButton        =   0x04000000,
    kControlFlagMiddleButtonEx      =   0x08000000,
    kControlFlagMiddleButtonRepeat  =   0x10000000,
    kControlFlagMiddleButtonUp      =   0x20000000,
};

// mouse button flags
enum
{
    kLeftButtonDown     = 0x0001,
    kLeftButtonUp       = 0x0002,
    kRightButtonDown    = 0x0004,
    kRightButtonUp      = 0x0008,
    kMiddleButtonDown   = 0x0010,
    kMiddleButtonUp     = 0x0020,
    kWheelPos           = 0x0040,
    kWheelNeg           = 0x0080,
    KWheelButtonDown    = 0x0100,
    kWheelButtonUp      = 0x0200,
    kLeftButtonRepeat   = 0x0400,
    kRightButtonRepeat  = 0x0800,
    kMiddleButtonRepeat = 0x1000,
    kLeftButtonDblClk   = 0x2000,
    kRightButtonDblClk  = 0x4000,
    kAnyButtonDown      = kLeftButtonDown | kRightButtonDown | kMiddleButtonDown,
};

// mouse cursor flags
enum
{
    kMouseNormal    = 0x0000,
    kMouseClickable = 0x0001,
};


struct Win32keyConvert
{
    uint32_t  fVKey;
    const char*   fKeyName;
};

struct CommandConvert
{
    ControlEventCode fCode;
    const char* fDesc;
};


struct plMouseInfo
{
    plMouseInfo(ControlEventCode _code, uint32_t _flags, hsPoint4 _box, const char* _desc)
    {
        fCode = _code;
        fControlFlags = _flags;
        fBox = _box;
        fControlDescription = _desc;
    }
    plMouseInfo(ControlEventCode _code, uint32_t _flags, float pt1, float pt2, float pt3, float pt4, const char* _desc)
    {
        fCode = _code;
        fControlFlags = _flags;
        fBox.Set(pt1,pt2,pt3,pt4);
        fControlDescription = _desc;
    }
    ControlEventCode    fCode;
    uint32_t            fControlFlags;
    hsPoint4            fBox;
    const char*         fControlDescription;
};


#endif // plControlDefinition_inc 
