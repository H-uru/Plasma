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

#include "HeadSpin.h"
#include "hsWindows.h"

#include <bmmlib.h>
#include <iparamb2.h>
#pragma hdrstop

/** Stubbed out BitmapIO instance for deprecated bink layers **/
class plBinkBitmapIO : public BitmapIO
{
public:
    int ExtCount()                  { return 1; }
    const TCHAR* Ext(int n)         { return _T("bik"); }

    const TCHAR* LongDesc()         { return _T("DEAD: Bink File"); }
    const TCHAR* ShortDesc()        { return _T("Bink"); }

    const TCHAR* AuthorName()       { return _T("Colin Bonstead"); }
    const TCHAR* CopyrightMessage() { return _T("Copyright 2004, Cyan Inc."); }
    unsigned int Version()          { return 100; }

    int Capability()                { return 0; }
    DWORD EvaluateConfigure()       { return 0; }
    BOOL LoadConfigure(void* ptr)   { return FALSE; }
    BOOL SaveConfigure(void* ptr)   { return FALSE; }

    BMMRES GetImageInfo(BitmapInfo* fbi) { return BMMRES_INTERNALERROR; }
    BitmapStorage* Load(BitmapInfo* fbi, Bitmap* map, BMMRES* status) { return nullptr; }
    void ShowAbout(HWND hWnd)
    {
        hsMessageBoxWithOwner(hWnd, "Bink Layers removed due to license issues", "DEAD", hsMessageBoxNormal, MB_ICONEXCLAMATION);
    }
};

class BinkClassDesc : public ClassDesc2
{
public:
    int IsPublic() { return 1; }
    void* Create(BOOL loading=FALSE) { return static_cast<void*>(new plBinkBitmapIO); }

    const TCHAR* ClassName() { return "Bink"; }
    SClass_ID SuperClassID() { return BMM_IO_CLASS_ID; }
    Class_ID ClassID() { return Class_ID(0x71c75c3c, 0x206f480e); }
    const TCHAR* Category() { return "Bitmap I/O"; }
};

static BinkClassDesc BinkDesc;
ClassDesc2* GetBinkClassDesc()
{
    return &BinkDesc;
}
