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

#include "pyVaultImageNode.h"
#include "pyImage.h"

#include "plVault/plVault.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptVaultImageNode, pyVaultImageNode);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultImageNode, pyVaultImageNode)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultImageNode)

PYTHON_INIT_DEFINITION(ptVaultImageNode, args, keywords)
{
    int n = 0;
    if (!PyArg_ParseTuple(args, "|i", &n))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects an optional int");
        PYTHON_RETURN_INIT_ERROR;
    }
    // we don't really do anything? Not according to the associated constructor. Odd...
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, setTitle, args)
{
    PyObject* textObj;
    if (!PyArg_ParseTuple(args, "O", &textObj))
    {
        PyErr_SetString(PyExc_TypeError, "setTitle expects a unicode string");
        PYTHON_RETURN_ERROR;
    }
    if (PyUnicode_Check(textObj))
    {
        wchar_t* title = PyUnicode_AsWideCharString(textObj, nullptr);
        self->fThis->Image_SetTitle(title);
        PyMem_Free(title);
        PYTHON_RETURN_NONE;
    }
    PyErr_SetString(PyExc_TypeError, "setTitle expects a unicode string");
    PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultImageNode, getTitle)
{
    return PyUnicode_FromSTString(self->fThis->Image_GetTitle());
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, setImage, args)
{
    PyObject* imageObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &imageObj))
    {
        PyErr_SetString(PyExc_TypeError, "setImage expects a ptImage");
        PYTHON_RETURN_ERROR;
    }
    if (!pyImage::Check(imageObj))
    {
        PyErr_SetString(PyExc_TypeError, "setImage expects a ptImage");
        PYTHON_RETURN_ERROR;
    }
    pyImage* image = pyImage::ConvertFrom(imageObj);
    self->fThis->Image_SetImage(*image);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultImageNode, getImage)
{
    return self->fThis->Image_GetImage();
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, setImageFromBuf, args)
{
    PyObject* buf = nullptr;
    if (!PyArg_ParseTuple(args, "O", &buf))
    {
        PyErr_SetString(PyExc_TypeError, "setImageFromBuf expects a buffer object");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetImageFromBuf(buf);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptVaultImageNode, setImageFromScrShot, SetImageFromScrShot)

PYTHON_START_METHODS_TABLE(ptVaultImageNode)
    PYTHON_METHOD(ptVaultImageNode, setTitle, "Params: title\nSets the title (caption) of this image node"),
    PYTHON_METHOD_NOARGS(ptVaultImageNode, getTitle, "Returns the title (caption) of this image node"),
    PYTHON_METHOD(ptVaultImageNode, setImage, "Params: image\nSets the image(ptImage) of this image node"),
    PYTHON_METHOD_NOARGS(ptVaultImageNode, getImage, "Returns the image(ptImage) of this image node"),
    PYTHON_METHOD(ptVaultImageNode, setImageFromBuf, "Params: buf\nSets our image from a buffer"),
    PYTHON_BASIC_METHOD(ptVaultImageNode, setImageFromScrShot, "Grabs a screenshot and stuffs it into this node"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultImageNode, pyVaultNode, "Params: n=0\nPlasma vault image node");

// required functions for PyObject interoperability
PYTHON_CLASS_VAULT_NODE_NEW_IMPL(ptVaultImageNode, pyVaultImageNode)

PYTHON_CLASS_CHECK_IMPL(ptVaultImageNode, pyVaultImageNode)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultImageNode, pyVaultImageNode)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultImageNode::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptVaultImageNode);
    PYTHON_CLASS_IMPORT_END(m);
}
