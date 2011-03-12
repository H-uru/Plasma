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
// Stolen from: http://www.mvps.org/user32/webhost.cab
// No copyright notices, so I assume it's public domain -Colin

#include "hsConfig.h"
#if HS_BUILD_FOR_WIN32

#pragma once

#include <windows.h>


struct basewnd
{
	static wchar_t szClassName[];
	static LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static void Initialize(HINSTANCE hAppInstance,UINT style=0);
	
	HWND hwnd;
	ULONG mcRef;
	basewnd();
	virtual ~basewnd();
	
public:
	virtual ULONG AddRef();
	virtual ULONG Release();
	virtual BOOL HandleMessage(UINT,WPARAM,LPARAM,LRESULT*)=0;
	
public: // inline overrides
	BOOL ShowWindow(int nCmdShow){return ::ShowWindow(hwnd,nCmdShow);}
	BOOL UpdateWindow(void){return ::UpdateWindow(hwnd);}
};

#endif