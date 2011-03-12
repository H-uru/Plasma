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
#ifndef __SIMPLEEXPORT_H
#define __SIMPLEEXPORT_H

#include "hsWindows.h"
#include "../MaxMain/resource.h"
#include "Max.h"
#include "HeadSpin.h"

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
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "HS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
    virtual int		DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

    const char*			GetName()                   { return fName; }

private:
    static hsBool		IProgressCallback(hsScalar percent);
    static DWORD WINAPI IProgressDummyFunc(LPVOID arg); 

    char                fName[128];
};

//------------------------------------------------------

class HSClassDesc2 : public ClassDesc 
{
public:
    int 			IsPublic() { return 1; }
    void *			Create(BOOL loading = FALSE) { return TRACKED_NEW HSExport2; }
    const TCHAR *	ClassName() { return "Plasma 2.0 Scene Exporter"; }
    SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
#ifdef HS_DEBUGGING
    Class_ID		ClassID() { return Class_ID(0x547962c7, 0x520a702d); }
#else
    Class_ID		ClassID() { return Class_ID(0x717f791f, 0x79412447); }
#endif
    const TCHAR* 	Category() { return "Plasma Export...";  }
};



class SimpleExportExitCallback : public ExitMAXCallback
{
public:
    BOOL Exit(HWND hWnd)    { return false; }
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
