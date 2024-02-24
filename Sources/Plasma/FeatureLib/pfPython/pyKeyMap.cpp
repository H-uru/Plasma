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
//////////////////////////////////////////////////////////////////////
//
// pyKeyMap   - a wrapper class all the key mapping functions
//
//////////////////////////////////////////////////////////////////////


#include "pyKeyMap.h"

#include <string_theory/string>

#include "pnInputCore/plKeyMap.h"

#include "plInputCore/plInputInterfaceMgr.h"

// conversion functions
ST::string pyKeyMap::ConvertVKeyToChar(uint32_t vk, uint32_t flags)
{
    return plKeyMap::KeyComboToString(plKeyCombo(static_cast<plKeyDef>(vk), flags));
}

uint32_t pyKeyMap::ConvertCharToVKey(const ST::string& charVKey)
{
    return plKeyMap::StringToKeyCombo(charVKey).fKey;
}

uint32_t pyKeyMap::ConvertCharToFlags(const ST::string& charVKey)
{
    return plKeyMap::StringToKeyCombo(charVKey).fFlags;
}


uint32_t pyKeyMap::ConvertCharToControlCode(const ST::string& charCode)
{
    ControlEventCode code = plInputMap::ConvertCharToControlCode(charCode);
    return (uint32_t)code;
}

ST::string pyKeyMap::ConvertControlCodeToString(uint32_t code)
{
    return plInputMap::ConvertControlCodeToString((ControlEventCode)code);
}


// bind a key to an action
void pyKeyMap::BindKey(const ST::string& keyStr1, const ST::string& keyStr2, const ST::string& act)
{
    
    ControlEventCode code = plKeyMap::ConvertCharToControlCode( act );
    if( code == END_CONTROLS )
    {
        // error.... traceback?
        return;
    }

    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        plKeyCombo key1 = plKeyMap::StringToKeyCombo(keyStr1);
        plKeyCombo key2 = plKeyMap::StringToKeyCombo(keyStr2);
        plInputInterfaceMgr::GetInstance()->BindAction( key1, key2, code );
    }
}

// bind a key to an action
void pyKeyMap::BindKeyToConsoleCommand(const ST::string& keyStr1, const ST::string& command)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        plKeyCombo key1 = plKeyMap::StringToKeyCombo(keyStr1);
        plInputInterfaceMgr::GetInstance()->BindConsoleCmd( key1, command, plKeyMap::kFirstAlways);
    }
}

uint32_t pyKeyMap::GetBindingKey1(uint32_t code)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
        if ( keymap )
        {
            plKeyCombo key = keymap->GetKey1();
            return (uint32_t)(key.fKey);
        }
    }
    return 0;
}

uint32_t pyKeyMap::GetBindingFlags1(uint32_t code)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
        if ( keymap )
        {
            plKeyCombo key = keymap->GetKey1();
            return (uint32_t)(key.fFlags);
        }
    }
    return 0;
}

uint32_t pyKeyMap::GetBindingKey2(uint32_t code)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
        if ( keymap )
        {
            plKeyCombo key = keymap->GetKey2();
            return (uint32_t)(key.fKey);
        }
    }
    return 0;
}

uint32_t pyKeyMap::GetBindingFlags2(uint32_t code)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBinding((ControlEventCode)code);
        if ( keymap )
        {
            plKeyCombo key = keymap->GetKey2();
            return (uint32_t)(key.fFlags);
        }
    }
    return 0;
}

uint32_t pyKeyMap::GetBindingKeyConsole(const ST::string& command)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBindingByConsoleCmd(command);
        if ( keymap )
        {
            plKeyCombo key = keymap->GetKey1();
            return (uint32_t)(key.fKey);
        }
    }
    return 0;
}

uint32_t pyKeyMap::GetBindingFlagsConsole(const ST::string& command)
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
    {
        const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBindingByConsoleCmd(command);
        if ( keymap )
        {
            plKeyCombo key = keymap->GetKey1();
            return (uint32_t)(key.fFlags);
        }
    }
    return 0;
}

void pyKeyMap::WriteKeyMap()
{
    if (plInputInterfaceMgr::GetInstance() != nullptr)
        plInputInterfaceMgr::GetInstance()->WriteKeyMap();
}
