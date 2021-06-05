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
#ifndef __SIMPLEEXPORT_H
#define __SIMPLEEXPORT_H

#include <MaxMain/MaxAPI.h>

//
// Inlines
//
extern HINSTANCE hInstance;
extern TCHAR *GetString(int id);

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//
// Header file for headSpin 3dsMax exporter
//

class HSExport2 : public SceneExport 
{
public:
                    HSExport2();
                    ~HSExport2();
    int             ExtCount() override;                 // Number of extensions supported
    const MCHAR *   Ext(int n) override;                 // Extension #n (i.e. "HS")
    const MCHAR *   LongDesc() override;                 // Long ASCII description (i.e. "Autodesk 3D Studio File")
    const MCHAR *   ShortDesc() override;                // Short ASCII description (i.e. "3D Studio")
    const MCHAR *   AuthorName() override;               // ASCII Author name
    const MCHAR *   CopyrightMessage() override;         // ASCII Copyright message
    const MCHAR *   OtherMessage1() override;            // Other message #1
    const MCHAR *   OtherMessage2() override;            // Other message #2
    unsigned int    Version() override;                  // Version number * 100 (i.e. v3.01 = 301)
    void            ShowAbout(HWND hWnd) override;       // Show DLL's "About..." box
    int             DoExport(const MCHAR* name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0) override;

    const MCHAR*    GetName()                   { return fName; }

private:
    static bool         IProgressCallback(float percent);
    static DWORD WINAPI IProgressDummyFunc(LPVOID arg); 

    MCHAR               fName[128];
};

//------------------------------------------------------

class HSClassDesc2 : public ClassDesc
{
public:
    int             IsPublic() override { return 1; }
    void *          Create(BOOL loading = FALSE) override { return new HSExport2; }
    const MCHAR *   ClassName() override { return _M("Plasma 2.0 Scene Exporter"); }
    SClass_ID       SuperClassID() override { return SCENE_EXPORT_CLASS_ID; }
#ifdef HS_DEBUGGING
    Class_ID        ClassID() override { return Class_ID(0x547962c7, 0x520a702d); }
#else
    Class_ID        ClassID() override { return Class_ID(0x717f791f, 0x79412447); }
#endif
    const MCHAR*    Category() override { return _M("Plasma Export..."); }
};



class SimpleExportExitCallback : public ExitMAXCallback
{
public:
    BOOL Exit(HWND hWnd) override { return false; }
};

//
// Externs
//
extern int controlsInit;
extern HSClassDesc2 HSDesc;

//
// Defines
//
#define no_RAM() Alert(IDS_TH_OUTOFMEMORY)

#endif  // __SIMPLEEXPORT_H
