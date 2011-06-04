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
#ifndef plOSMsg_inc
#define plOSMsg_inc

#include <windows.h>

//
// This enum wraps all of the OS messages
// that we care about for this particular
// platform - add as necessary...
//

// for Win32:

enum plOSMsg
{
	KEYDOWN			= WM_KEYDOWN,
	KEYUP			= WM_KEYUP,
	MOUSEMOVE		= WM_MOUSEMOVE,
	L_BUTTONDN		= WM_LBUTTONDOWN,
	L_BUTTONUP		= WM_LBUTTONUP,
	R_BUTTONDN		= WM_RBUTTONDOWN,
	R_BUTTONUP		= WM_RBUTTONUP,
	MOUSEWHEEL		= 0x020A,
	L_BUTTONDBLCLK	= WM_LBUTTONDBLCLK,
	R_BUTTONDBLCLK	= WM_RBUTTONDBLCLK,
	SYSKEYDOWN		= WM_SYSKEYDOWN,
	SYSKEYUP		= WM_SYSKEYUP,
	M_BUTTONDN		= WM_MBUTTONDOWN,
	M_BUTTONUP		= WM_MBUTTONUP,
};


//
// generic structure that we can use to describe
// the state of the mouse on any platform.
//
//

struct plMouseState
{
	enum
	{
		kLeftButton		= 	0x0001,
		kRightButton	=	0x0002,
		kMiddleButton	=	0x0004,	
	};
	float	fX;
	float	fY;
	UInt32	fButtonState;
};


#endif // plOSMsg 
