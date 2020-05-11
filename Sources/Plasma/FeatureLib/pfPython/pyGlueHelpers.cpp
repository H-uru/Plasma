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

#include <Python.h>
#include <string_theory/string>
#include "pyGlueHelpers.h"
#pragma hdrstop

#include "plFileSystem.h"

ST::string PyUnicode_AsSTString(PyObject* obj)
{
    if (PyUnicode_Check(obj)) {
#if (Py_UNICODE_SIZE == 2)
        return ST::string::from_utf16(reinterpret_cast<const char16_t *>(PyUnicode_AsUnicode(obj)));
#elif (Py_UNICODE_SIZE == 4)
        return ST::string::from_utf32(reinterpret_cast<const char32_t *>(PyUnicode_AsUnicode(obj)));
#else
#       error "Py_UNICODE is an unexpected size"
#endif
    }

    return ST::null;
}

int PyUnicode_STStringConverter(PyObject* obj, void* str)
{
    if (PyUnicode_Check(obj)) {
        *reinterpret_cast<ST::string*>(str) = PyUnicode_AsSTString(obj);
        return 1;
    }
    return 0;
}

int PyUnicode_PlFileNameDecoder(PyObject* obj, void* fn)
{
    if (PyUnicode_Check(obj)) {
        *reinterpret_cast<plFileName*>(fn) = PyUnicode_AsSTString(obj);
        return 1;
    }

    PyObject* fsConvert;
    if (PyUnicode_FSDecoder(obj, &fsConvert)) {
        *reinterpret_cast<plFileName*>(fn) = PyUnicode_AsSTString(fsConvert);
        Py_DECREF(fsConvert);
        return 1;
    }
    return 0;
}
