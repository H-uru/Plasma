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
#include "pyKey.h"

#include "pyImageLibMod.h"
#include "pyImage.h"
#include "plGImage/plMipmap.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptImageLibMod, pyImageLibMod);

PYTHON_DEFAULT_NEW_DEFINITION(ptImageLibMod, pyImageLibMod)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptImageLibMod)

PYTHON_INIT_DEFINITION(ptImageLibMod, args, keywords)
{
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    pyKey* key = pyKey::ConvertFrom(keyObj);
    self->fThis->setKey(*key);
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptImageLibMod, getImage, args)
{
    ST::string name;
    if (!PyArg_ParseTuple(args, "O&", PyUnicode_STStringConverter, &name)) {
        PyErr_SetString(PyExc_TypeError, "getImage expects an image name");
        PYTHON_RETURN_ERROR;
    }

    return self->fThis->GetImage(name);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptImageLibMod, getImages)
{
    const std::vector<PyObject*> imageList = self->fThis->GetImages();
    PyObject* retVal = PyTuple_New(imageList.size());
    for (size_t curKey = 0; curKey < imageList.size(); curKey++)
        PyTuple_SET_ITEM(retVal, curKey, imageList[curKey]);
    return retVal;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptImageLibMod, getNames)
{
    std::vector<ST::string> nameList = self->fThis->GetImageNames();
    PyObject* retVal = PyTuple_New(nameList.size());
    for (size_t curKey = 0; curKey < nameList.size(); curKey++)
        PyTuple_SET_ITEM(retVal, curKey, PyUnicode_FromSTString(nameList[curKey]));
    return retVal;
}

PYTHON_START_METHODS_TABLE(ptImageLibMod)
    PYTHON_METHOD(ptImageLibMod, getImage, "Params: name\nReturns the ptImage with the specified name"),
    PYTHON_METHOD_NOARGS(ptImageLibMod, getImages, "Returns a tuple of the library's ptImages"),
    PYTHON_METHOD_NOARGS(ptImageLibMod, getNames, "Returns the list of image names"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptImageLibMod, "Params: ilmKey\nPlasma image library modifier class");

// required functions for PyObject interoperability
PyObject* pyImageLibMod::New(plImageLibMod* ilm)
{
    ptImageLibMod* newObj = (ptImageLibMod*)ptImageLibMod_type.tp_new(&ptImageLibMod_type, nullptr, nullptr);
    newObj->fThis->fModifier = ilm;
    newObj->fThis->fModifierKey = ilm->GetKey();
    if (ilm->GetKey())
        newObj->fThis->fModifierKey->RefObject();
    return (PyObject*)newObj;
}

PyObject* pyImageLibMod::New(plKey ilmKey)
{
    ptImageLibMod* newObj = (ptImageLibMod*)ptImageLibMod_type.tp_new(&ptImageLibMod_type, nullptr, nullptr);
    newObj->fThis->fModifierKey = std::move(ilmKey);
    return (PyObject*)newObj;
}

PyObject* pyImageLibMod::New(pyKey& ilmKey)
{
    ptImageLibMod* newObj = (ptImageLibMod*)ptImageLibMod_type.tp_new(&ptImageLibMod_type, nullptr, nullptr);
    newObj->fThis->fModifierKey = ilmKey.getKey();
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptImageLibMod, pyImageLibMod)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptImageLibMod, pyImageLibMod)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyImageLibMod::AddPlasmaClasses(PyObject* m)
{
    PYTHON_CLASS_IMPORT_START(m);
        PYTHON_CLASS_IMPORT(m, ptImageLibMod);
    PYTHON_CLASS_IMPORT_END(m);
}
