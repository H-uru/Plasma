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
#ifndef _pyKeyMap_h_
#define _pyKeyMap_h_

//////////////////////////////////////////////////////////////////////
//
// pyKeyMap   - a wrapper class all the Key mapping functions
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pyGlueHelpers.h"

class plKeyCombo;

class pyKeyMap
{
protected:
    pyKeyMap() {};

private:
    plKeyCombo IBindKeyToVKey(ST::string keyStr);


public:
    enum
    {
        kShift  = 0x01,
        kCtrl   = 0x02
    };

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptKeyMap);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyKeyMap object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyKeyMap); // converts a PyObject to a pyKeyMap (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // conversion functions
    ST::string ConvertVKeyToChar(uint32_t vk, uint32_t flags);
    uint32_t ConvertCharToVKey(const ST::string& charVKey);
    uint32_t ConvertCharToFlags(const ST::string& charVKey);

    uint32_t ConvertCharToControlCode(const ST::string& charCode);
    ST::string ConvertControlCodeToString(uint32_t code);


    // bind a key to an action
    void BindKey(const ST::string& keyStr1, const ST::string& keyStr2, const ST::string& act);
    void BindKeyToConsoleCommand(const ST::string& keyStr1, const ST::string& command);

    uint32_t GetBindingKey1(uint32_t code);
    uint32_t GetBindingFlags1(uint32_t code);
    uint32_t GetBindingKey2(uint32_t code);
    uint32_t GetBindingFlags2(uint32_t code);

    uint32_t GetBindingKeyConsole(const ST::string& command);
    uint32_t GetBindingFlagsConsole(const ST::string& command);

    void WriteKeyMap();

};

#endif // _pyKeyMap_h_
