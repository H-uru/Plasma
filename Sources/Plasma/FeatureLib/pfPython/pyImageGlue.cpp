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
#include "pyImage.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptImage, pyImage);

PYTHON_DEFAULT_NEW_DEFINITION(ptImage, pyImage)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptImage)

PYTHON_INIT_DEFINITION(ptImage, args, keywords)
{
	PyObject* keyObj = NULL;
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

PYTHON_RICH_COMPARE_DEFINITION(ptImage, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyImage::Check(obj1) || !pyImage::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptImage object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyImage *img1 = pyImage::ConvertFrom(obj1);
	pyImage *img2 = pyImage::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*img1) == (*img2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*img1) != (*img2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptImage object");
	PYTHON_RCOMPARE_ERROR;
}

#ifndef BUILDING_PYPLASMA
PYTHON_METHOD_DEFINITION(ptImage, getPixelColor, args)
{
	float x, y;
	if (!PyArg_ParseTuple(args, "ff", &x, &y))
	{
		PyErr_SetString(PyExc_TypeError, "getPixelColor expects two floats");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetPixelColor(x, y);
}

PYTHON_METHOD_DEFINITION(ptImage, getColorLoc, args)
{
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "getColorLoc expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "getColorLoc expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	return self->fThis->GetColorLoc(*color);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptImage, getWidth)
{
	return PyLong_FromUnsignedLong(self->fThis->GetWidth());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptImage, getHeight)
{
	return PyLong_FromUnsignedLong(self->fThis->GetHeight());
}

PYTHON_METHOD_DEFINITION(ptImage, saveAsJPEG, args)
{
	PyObject* filenameObj;
	unsigned char quality = 75;
	if (!PyArg_ParseTuple(args, "O|b", &filenameObj, &quality))
	{
		PyErr_SetString(PyExc_TypeError, "saveAsJPEG expects a string and a unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(filenameObj))
	{
		int strLen = PyUnicode_GetSize(filenameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)filenameObj, text, strLen);
		text[strLen] = L'\0';
		self->fThis->SaveAsJPEG(text, quality);
		delete [] text;
		PYTHON_RETURN_NONE;
	}
	else if (PyString_Check(filenameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(filenameObj);
		wchar_t* wText = hsStringToWString(text);
		self->fThis->SaveAsJPEG(wText, quality);
		delete [] wText;
		PYTHON_RETURN_NONE;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "saveAsJPEG expects a string and a unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
}
#endif // BUILDING_PYPLASMA

PYTHON_START_METHODS_TABLE(ptImage)
#ifndef BUILDING_PYPLASMA
	PYTHON_METHOD(ptImage, getPixelColor, "Params: x,y\nReturns the ptColor at the specified location (float from 0 to 1)"),
	PYTHON_METHOD(ptImage, getColorLoc, "Params: color\nReturns the ptPoint3 where the specified color is located"),
	PYTHON_METHOD_NOARGS(ptImage, getWidth, "Returns the width of the image"),
	PYTHON_METHOD_NOARGS(ptImage, getHeight, "Returns the height of the image"),
	PYTHON_METHOD(ptImage, saveAsJPEG, "Params: filename,quality=75\nSaves this image to disk as a JPEG file"),
#endif
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptImage_COMPARE			PYTHON_NO_COMPARE
#define ptImage_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptImage_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptImage_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptImage_STR				PYTHON_NO_STR
#define ptImage_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptImage)
#define ptImage_GETSET			PYTHON_NO_GETSET
#define ptImage_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptImage, "Params: imgKey\nPlasma image class");

// required functions for PyObject interoperability
#ifndef BUILDING_PYPLASMA
PyObject *pyImage::New(plMipmap* mipmap)
{
	ptImage *newObj = (ptImage*)ptImage_type.tp_new(&ptImage_type, NULL, NULL);
	newObj->fThis->fMipmap = mipmap;
	newObj->fThis->fMipMapKey = mipmap->GetKey();
	if (mipmap->GetKey())
		newObj->fThis->fMipMapKey->RefObject();
	return (PyObject*)newObj;
}
#endif

PyObject *pyImage::New(plKey mipmapKey)
{
	ptImage *newObj = (ptImage*)ptImage_type.tp_new(&ptImage_type, NULL, NULL);
	newObj->fThis->fMipMapKey = mipmapKey;
	return (PyObject*)newObj;
}

PyObject *pyImage::New(pyKey& mipmapKey)
{
	ptImage *newObj = (ptImage*)ptImage_type.tp_new(&ptImage_type, NULL, NULL);
	newObj->fThis->fMipMapKey = mipmapKey.getKey();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptImage, pyImage)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptImage, pyImage)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyImage::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptImage);
	PYTHON_CLASS_IMPORT_END(m);
}

#ifndef BUILDING_PYPLASMA
PYTHON_GLOBAL_METHOD_DEFINITION(PtLoadJPEGFromDisk, args, "Params: filename,width,height\nThe image will be resized to fit the width and height arguments. Set to 0 if resizing is not desired.\nReturns a pyImage of the specified file.")
{
	PyObject* filenameObj;
	unsigned short width, height;
	if (!PyArg_ParseTuple(args, "Ohh", &filenameObj, &width, &height))
	{
		PyErr_SetString(PyExc_TypeError, "PtLoadJPEGFromDisk expects a string and two unsigned shorts");
		PYTHON_RETURN_ERROR;
	}

	if (PyUnicode_Check(filenameObj))
	{
		int strLen = PyUnicode_GetSize(filenameObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)filenameObj, text, strLen);
		text[strLen] = L'\0';
		PyObject* ret = pyImage::LoadJPEGFromDisk(text, width, height);
		delete [] text;
		return ret;
	}
	else if (PyString_Check(filenameObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(filenameObj);
		wchar_t* wText = hsStringToWString(text);
		PyObject* ret = pyImage::LoadJPEGFromDisk(wText, width, height);
		delete [] wText;
		return ret;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "saveAsJPEG expects a string and a unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
}
#endif

void pyImage::AddPlasmaMethods(std::vector<PyMethodDef> &methods)
{
#ifndef BUILDING_PYPLASMA
	PYTHON_GLOBAL_METHOD(methods, PtLoadJPEGFromDisk);
#endif
}