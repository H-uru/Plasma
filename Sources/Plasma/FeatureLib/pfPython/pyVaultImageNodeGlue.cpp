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
#include "pyVaultImageNode.h"
#include "pyImage.h"

#include "../plVault/plVault.h"
#include <python.h>

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

PYTHON_METHOD_DEFINITION(ptVaultImageNode, imageSetTitle, args)
{
	char* title;
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		PyErr_SetString(PyExc_TypeError, "imageSetTitle expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Image_SetTitle(title);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultImageNode, imageGetTitle)
{
	return PyString_FromString(self->fThis->Image_GetTitle().c_str());
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, imageSetImage, args)
{
	PyObject* imageObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &imageObj))
	{
		PyErr_SetString(PyExc_TypeError, "imageSetImage expects a ptImage");
		PYTHON_RETURN_ERROR;
	}
	if (!pyImage::Check(imageObj))
	{
		PyErr_SetString(PyExc_TypeError, "imageSetImage expects a ptImage");
		PYTHON_RETURN_ERROR;
	}
	pyImage* image = pyImage::ConvertFrom(imageObj);
	self->fThis->Image_SetImage(*image);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultImageNode, imageGetImage)
{
	return self->fThis->Image_GetImage();
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, setTitle, args)
{
	char* title;
	if (!PyArg_ParseTuple(args, "s", &title))
	{
		PyErr_SetString(PyExc_TypeError, "setTitle expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->Image_SetTitle(title);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, setTitleW, args)
{
	PyObject* textObj;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "setTitleW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* title = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, title, strLen);
		title[strLen] = L'\0';
		self->fThis->Image_SetTitleW(title);
		delete [] title;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* title = PyString_AsString(textObj);
		self->fThis->Image_SetTitle(title);
		PYTHON_RETURN_NONE;
	}
	PyErr_SetString(PyExc_TypeError, "setTitleW expects a unicode string");
	PYTHON_RETURN_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultImageNode, getTitle)
{
	return PyString_FromString(self->fThis->Image_GetTitle().c_str());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultImageNode, getTitleW)
{
	std::wstring retVal = self->fThis->Image_GetTitleW();
	return PyUnicode_FromWideChar(retVal.c_str(), retVal.length());
}

PYTHON_METHOD_DEFINITION(ptVaultImageNode, setImage, args)
{
	PyObject* imageObj = NULL;
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
	PyObject* buf = NULL;
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
	// legacy glue
	PYTHON_METHOD(ptVaultImageNode, imageSetTitle, "Params: title\nLEGACY\nSets the title (caption) of this image node"),
	PYTHON_METHOD_NOARGS(ptVaultImageNode, imageGetTitle, "LEGACY\nReturns the title (caption) of this image node"),
	PYTHON_METHOD(ptVaultImageNode, imageSetImage, "Params: image\nLEGACY\nSets the image(ptImage) of this image node"),
	PYTHON_METHOD_NOARGS(ptVaultImageNode, imageGetImage, "LEGACY\nReturns the image(ptImage) of this image node"),
	// new glue
	PYTHON_METHOD(ptVaultImageNode, setTitle, "Params: title\nSets the title (caption) of this image node"),
	PYTHON_METHOD(ptVaultImageNode, setTitleW, "Params: title\nUnicode version of setTitle"),
	PYTHON_METHOD_NOARGS(ptVaultImageNode, getTitle, "Returns the title (caption) of this image node"),
	PYTHON_METHOD_NOARGS(ptVaultImageNode, getTitleW, "Unicode version of getTitle"),
	PYTHON_METHOD(ptVaultImageNode, setImage, "Params: image\nSets the image(ptImage) of this image node"),
	PYTHON_METHOD_NOARGS(ptVaultImageNode, getImage, "Returns the image(ptImage) of this image node"),
	PYTHON_METHOD(ptVaultImageNode, setImageFromBuf, "Params: buf\nSets our image from a buffer"),
	PYTHON_BASIC_METHOD(ptVaultImageNode, setImageFromScrShot, "Grabs a screenshot and stuffs it into this node"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE_WBASE(ptVaultImageNode, pyVaultNode, "Params: n=0\nPlasma vault image node");

// required functions for PyObject interoperability
PyObject *pyVaultImageNode::New(RelVaultNode* nfsNode)
{
	ptVaultImageNode *newObj = (ptVaultImageNode*)ptVaultImageNode_type.tp_new(&ptVaultImageNode_type, NULL, NULL);
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->DecRef();
	newObj->fThis->fNode = nfsNode;
	if (newObj->fThis->fNode)
		newObj->fThis->fNode->IncRef();
	return (PyObject*)newObj;
}

PyObject *pyVaultImageNode::New(int n /* =0 */)
{
	ptVaultImageNode *newObj = (ptVaultImageNode*)ptVaultImageNode_type.tp_new(&ptVaultImageNode_type, NULL, NULL);
	// oddly enough, nothing to do here
	return (PyObject*)newObj;
}

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