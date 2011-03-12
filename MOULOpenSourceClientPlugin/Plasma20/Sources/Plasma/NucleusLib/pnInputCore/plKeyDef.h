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
#ifndef plKeyDef_inc
#define plKeyDef_inc

#include "hsConfig.h"

#if HS_BUILD_FOR_WIN32

#include "windows.h"

#define VK_BACK_QUOTE	0xc0
//
// keyboard definitions:
//
// map O.S. specific keyboard defines
// to plasma key defines here...
//
//

// for win32:

enum plKeyDef
{
	KEY_A				= 'A',
	KEY_B				= 'B',
	KEY_C				= 'C',
	KEY_D				= 'D',
	KEY_E				= 'E',
	KEY_F				= 'F',
	KEY_G				= 'G',
	KEY_H				= 'H',
	KEY_I				= 'I',
	KEY_J				= 'J',
	KEY_K				= 'K',
	KEY_L				= 'L',
	KEY_M				= 'M',
	KEY_N				= 'N',
	KEY_O				= 'O',
	KEY_P				= 'P',
	KEY_Q				= 'Q',
	KEY_R				= 'R',
	KEY_S				= 'S',
	KEY_T				= 'T',
	KEY_U				= 'U',
	KEY_V				= 'V',
	KEY_W				= 'W',
	KEY_X				= 'X',
	KEY_Y				= 'Y',
	KEY_Z				= 'Z',
	KEY_0				= 0x30,
	KEY_1				= 0x31,
	KEY_2				= 0x32,
	KEY_3				= 0x33,
	KEY_4				= 0x34,
	KEY_5				= 0x35,
	KEY_6				= 0x36,
	KEY_7				= 0x37,
	KEY_8				= 0x38,
	KEY_9				= 0x39,
	KEY_F1				= VK_F1,
	KEY_F2				= VK_F2,
	KEY_F3				= VK_F3,
	KEY_F4				= VK_F4,
	KEY_F5				= VK_F5,
	KEY_F6				= VK_F6,
	KEY_F7				= VK_F7,
	KEY_F8				= VK_F8,
	KEY_F9				= VK_F9,
	KEY_F10				= VK_F10,
	KEY_F11				= VK_F11,
	KEY_F12				= VK_F12,
	KEY_ESCAPE			= VK_ESCAPE,
	KEY_TAB				= VK_TAB,
	KEY_SHIFT			= VK_SHIFT,
	KEY_CTRL			= VK_CONTROL,
	KEY_ALT				= VK_MENU,
	KEY_UP				= VK_UP,
	KEY_DOWN			= VK_DOWN,
	KEY_LEFT			= VK_LEFT,
	KEY_RIGHT			= VK_RIGHT,
	KEY_BACKSPACE		= VK_BACK,
	KEY_ENTER			= VK_RETURN,
	KEY_PAUSE			= VK_PAUSE,
	KEY_CAPSLOCK		= VK_CAPITAL,
	KEY_PAGEUP			= VK_PRIOR,
	KEY_PAGEDOWN		= VK_NEXT,
	KEY_END				= VK_END,
	KEY_HOME			= VK_HOME,
	KEY_PRINTSCREEN		= VK_SNAPSHOT,
	KEY_INSERT			= VK_INSERT,
	KEY_DELETE			= VK_DELETE,
	KEY_NUMPAD0			= VK_NUMPAD0,		
	KEY_NUMPAD1			= VK_NUMPAD1,		
	KEY_NUMPAD2			= VK_NUMPAD2,		
	KEY_NUMPAD3			= VK_NUMPAD3,		
	KEY_NUMPAD4			= VK_NUMPAD4,		
	KEY_NUMPAD5			= VK_NUMPAD5,		
	KEY_NUMPAD6			= VK_NUMPAD6,		
	KEY_NUMPAD7			= VK_NUMPAD7,		
	KEY_NUMPAD8			= VK_NUMPAD8,		
	KEY_NUMPAD9			= VK_NUMPAD9,
	KEY_NUMPAD_MULTIPLY	= VK_MULTIPLY,
	KEY_NUMPAD_ADD		= VK_ADD,
	KEY_NUMPAD_SUBTRACT = VK_SUBTRACT,
	KEY_NUMPAD_PERIOD	= VK_DECIMAL,
	KEY_NUMPAD_DIVIDE	= VK_DIVIDE,
	KEY_SPACE			= VK_SPACE,
	KEY_COMMA			= VK_OEM_COMMA,
	KEY_PERIOD			= VK_OEM_PERIOD,
	KEY_DASH			= VK_OEM_MINUS,
	KEY_EQUAL			= VK_OEM_PLUS,
	
	// these are only good in the US of A...
	KEY_SEMICOLON		= VK_OEM_1,
	KEY_SLASH			= VK_OEM_2,
	KEY_TILDE			= VK_OEM_3,	
	KEY_LBRACKET		= VK_OEM_4,
	KEY_BACKSLASH		= VK_OEM_5,
	KEY_RBRACKET		= VK_OEM_6,
	KEY_QUOTE			= VK_OEM_7,
	KEY_UNMAPPED		= 0xffffffff,
};


#elif HS_BUILD_FOR_UNIX

enum plKeyDef
{
	KEY_UNMAPPED		= 0xffffffff,
};
#endif

#endif // plKeyDef_inc 
