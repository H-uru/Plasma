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

#ifndef _pyObjectRef_h_
#define _pyObjectRef_h_

#include <Python.h>
#include "HeadSpin.h"

struct pyObjectNewRef_Type{};
constexpr pyObjectNewRef_Type pyObjectNewRef;

/** RAII reference count helper for Python objects. */
class pyObjectRef
{
    PyObject* fPyObject;

public:
    pyObjectRef() : fPyObject() { }

    /** Steals ownership of this object reference. */
    pyObjectRef(PyObject* object) : fPyObject(object) { }

    /** Increments the reference count of this object. */
    pyObjectRef(PyObject* object, pyObjectNewRef_Type)
        : fPyObject(object)
    {
        Py_XINCREF(object);
    }

    pyObjectRef(std::nullptr_t, pyObjectNewRef_Type) = delete;

    pyObjectRef(const pyObjectRef& copy)
        : fPyObject(copy.fPyObject)
    {
        Py_XINCREF(fPyObject);
    }

    pyObjectRef(pyObjectRef&& move) noexcept
        : fPyObject(move.fPyObject)
    {
        move.fPyObject = nullptr;
    }

    ~pyObjectRef()
    {
        Py_XDECREF(fPyObject);
    }

    /**
     * \brief Returns the managed PyObject pointer.
     * \remarks This is an explicit operation to prevent a temporary implicit PyObject pointer from
     *          being used to construct a new pyObjectRef.
     */
    PyObject* Get() const
    {
        return fPyObject;
    }

    PyObject* Release()
    {
        PyObject* temp = fPyObject;
        fPyObject = nullptr;
        return temp;
    }

    void SetPyNone()
    {
        Py_INCREF(Py_None);
        Py_XDECREF(fPyObject);
        fPyObject = Py_None;
    }

    pyObjectRef& operator =(const pyObjectRef& copy)
    {
        Py_XINCREF(copy.fPyObject);
        Py_XDECREF(fPyObject);
        fPyObject = copy.fPyObject;
        return *this;
    }

    /** Steals ownership of the provided object reference. */
    pyObjectRef& operator =(PyObject* rhs)
    {
        Py_XDECREF(fPyObject);
        fPyObject = rhs;
        return *this;
    }

    pyObjectRef& operator =(pyObjectRef&& rhs) noexcept
    {
        Py_XDECREF(fPyObject);
        fPyObject = rhs.fPyObject;
        rhs.fPyObject = nullptr;
        return *this;
    }

    bool operator ==(const pyObjectRef& rhs) const { return (fPyObject == rhs.fPyObject); }
    bool operator ==(const PyObject* rhs) const { return (fPyObject == rhs); }
    operator bool() const { return (fPyObject != nullptr); }
};

#endif
