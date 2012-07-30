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
#ifndef _pyKey_h_
#define _pyKey_h_

//////////////////////////////////////////////////////////////////////
//
// pyKey   - the wrapper class around a plKey so that Python can handle it
//
//////////////////////////////////////////////////////////////////////

#include "pnKeyedObject/plKey.h"

#include "pyGlueHelpers.h"

class plPythonFileMod;
class pySceneObject;
class plPipeline;

class pyKey
{
private:
        plKey               fKey;       // the plKey that we are holding onto
#ifndef BUILDING_PYPLASMA // pyPlasma (and other plugins) don't need all this extra junk)
        plPythonFileMod*    fPyFileMod;     // pointer to the PythonFileModifier

        bool                fNetForce;
#endif // BUILDING_PYPLASMA

protected:
    pyKey();
    pyKey(plKey key);
#ifndef BUILDING_PYPLASMA
    pyKey(plKey key, plPythonFileMod* pymod);
#endif

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptKey);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(plKey key);
    static PyObject *New(pyKey *key);
#ifndef BUILDING_PYPLASMA
    static PyObject *New(plKey key, plPythonFileMod *pyMod);
    static PyObject *New(pyKey *key, plPythonFileMod *pyMod);
#endif
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyKey object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyKey); // converts a PyObject to a pyKey (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // override the equals to operator
    bool operator==(const pyKey &key) const;
    bool operator!=(const pyKey &key) const { return !(key == *this); }

    // getter and setters
    virtual plKey getKey() { return fKey; }
    virtual void setKey(plKey key) { fKey=key; }
    virtual const char* getName() const;
#ifndef BUILDING_PYPLASMA
    PyObject* GetPySceneObject();

    virtual void SetNetForce(bool state) { fNetForce = state; }

    // methods to be sent to the plKey
    // send enable message to the plKey
    virtual void Enable() { IEnable(true); }
    // send disable message to the plKey
    virtual void Disable() { IEnable(false); }
    // if this is a modifier then get the (first) object its attached to
    virtual PyObject* GetParentObject();

    // special functions when the plKey is a pointer to the PythonModifier
    // (Only called from C++, not from Python directly)
    //
    // Was the last plNotifyMsg a locally sent?
    virtual bool WasLocalNotify();
    // Is this python file mod attached to a clone?
    virtual bool IsAttachedToClone();
    // (old style - Used in pyNotify)
    // get the notify list count
    virtual int32_t NotifyListCount();
    // (old style - Used in pyNotify)
    // get a notify list item
    virtual plKey GetNotifyListItem(int32_t i);
    // Set the dirty state on the PythonModifier
    virtual void DirtySynchState(const char* SDLStateName, uint32_t sendFlags);

    // register and unregister for control key envents
    virtual void EnableControlKeyEvents();
    virtual void DisableControlKeyEvents();

    virtual plPipeline* GetPipeline();
private:
    // build and send enable message to the plKey
    virtual void IEnable(bool state);
#endif // BUILDING_PYPLASMA
};

#endif // _pyKey_h_
