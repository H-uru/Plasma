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
#include "pyKey.h"
#include "pyDynamicText.h"
#include "pyEnum.h"
#include "pyColor.h"
#include "pyImage.h"
#include "../plGImage/plDynamicTextMap.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptDynamicMap, pyDynamicText);

PYTHON_DEFAULT_NEW_DEFINITION(ptDynamicMap, pyDynamicText)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptDynamicMap)

PYTHON_INIT_DEFINITION(ptDynamicMap, args, keywords)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "|O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects an optional ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (!keyObj)
		PYTHON_RETURN_INIT_OK; // nothing to do
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects an optional ptKey");
		PYTHON_RETURN_INIT_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->AddReceiver(*key);
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, sender, args)
{
	PyObject* senderObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &senderObj))
	{
		PyErr_SetString(PyExc_TypeError, "sender expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(senderObj))
	{
		PyErr_SetString(PyExc_TypeError, "sender expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* sender = pyKey::ConvertFrom(senderObj);
	self->fThis->SetSender(*sender);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptDynamicMap, clearKeys, ClearReceivers)

PYTHON_METHOD_DEFINITION(ptDynamicMap, addKey, args)
{
	PyObject* keyObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "addKey expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	if (!pyKey::Check(keyObj))
	{
		PyErr_SetString(PyExc_TypeError, "addKey expects a ptKey");
		PYTHON_RETURN_ERROR;
	}
	pyKey* key = pyKey::ConvertFrom(keyObj);
	self->fThis->AddReceiver(*key);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, netPropagate, args)
{
	char propagateFlag;
	if (!PyArg_ParseTuple(args, "b", &propagateFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netPropagate expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetPropagate(propagateFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, netForce, args)
{
	char forceFlag;
	if (!PyArg_ParseTuple(args, "b", &forceFlag))
	{
		PyErr_SetString(PyExc_TypeError, "netForce expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetNetForce(forceFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, clearToColor, args)
{
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "clearToColor expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "clearToColor expects a ptColor");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	self->fThis->ClearToColor(*color);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptDynamicMap, flush, Flush)
PYTHON_BASIC_METHOD_DEFINITION(ptDynamicMap, purgeImage, PurgeImage)

PYTHON_METHOD_DEFINITION(ptDynamicMap, setTextColor, args)
{
	PyObject* colorObj = NULL;
	char blockRGB = 0;
	if (!PyArg_ParseTuple(args, "O|b", &colorObj, &blockRGB))
	{
		PyErr_SetString(PyExc_TypeError, "setTextColor expects a ptColor and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "setTextColor expects a ptColor and an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	self->fThis->SetTextColor2(*color, blockRGB != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, setFont, args)
{
	char* faceName;
	short fontSize;
	if (!PyArg_ParseTuple(args, "sh", &faceName, &fontSize))
	{
		PyErr_SetString(PyExc_TypeError, "setFont expects a string and a short int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetFont(faceName, fontSize);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, fillRect, args)
{
	unsigned short left, top, right, bottom;
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "hhhhO", &left, &top, &right, &bottom, &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "fillRect expects four unsigned short ints and a ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "fillRect expects four unsigned short ints and a ptColor");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	self->fThis->FillRect(left, top, right, bottom, *color);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, frameRect, args)
{
	unsigned short left, top, right, bottom;
	PyObject* colorObj = NULL;
	if (!PyArg_ParseTuple(args, "hhhhO", &left, &top, &right, &bottom, &colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "frameRect expects four unsigned short ints and a ptColor");
		PYTHON_RETURN_ERROR;
	}
	if (!pyColor::Check(colorObj))
	{
		PyErr_SetString(PyExc_TypeError, "frameRect expects four unsigned short ints and a ptColor");
		PYTHON_RETURN_ERROR;
	}
	pyColor* color = pyColor::ConvertFrom(colorObj);
	self->fThis->FrameRect(left, top, right, bottom, *color);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, setClipping, args)
{
	unsigned short left, top, right, bottom;
	if (!PyArg_ParseTuple(args, "hhhh", &left, &top, &right, &bottom))
	{
		PyErr_SetString(PyExc_TypeError, "setClipping expects four unsigned short ints");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetClipping(left, top, right, bottom);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptDynamicMap, unsetClipping, UnsetClipping)

PYTHON_METHOD_DEFINITION(ptDynamicMap, setWrapping, args)
{
	unsigned short wrapWidth, wrapHeight;
	if (!PyArg_ParseTuple(args, "hh", &wrapWidth, &wrapHeight))
	{
		PyErr_SetString(PyExc_TypeError, "setWrapping expects two unsigned short ints");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetWrapping(wrapWidth, wrapHeight);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptDynamicMap, unsetWrapping, UnsetWrapping)

PYTHON_METHOD_DEFINITION(ptDynamicMap, drawText, args)
{
	short x, y;
	char* text;
	if (!PyArg_ParseTuple(args, "hhs", &x, &y, &text))
	{
		PyErr_SetString(PyExc_TypeError, "drawText expects two short ints and a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->DrawText(x, y, text);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, drawTextW, args)
{
	short x, y;
	wchar_t* text;
	if (!PyArg_ParseTuple(args, "hhu", &x, &y, &text))
	{
		PyErr_SetString(PyExc_TypeError, "drawTextW expects two short ints and a unicode string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->DrawTextW(x, y, text);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, drawImage, args)
{
	short x, y;
	PyObject* imageObj = NULL;
	char respectAlpha;
	if (!PyArg_ParseTuple(args, "hhOb", &x, &y, &imageObj, &respectAlpha))
	{
		PyErr_SetString(PyExc_TypeError, "drawImage expects two shorts, a ptImage, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyImage::Check(imageObj))
	{
		PyErr_SetString(PyExc_TypeError, "drawImage expects two shorts, a ptImage, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	pyImage* image = pyImage::ConvertFrom(imageObj);
	self->fThis->DrawImage(x, y, *image, respectAlpha != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, drawImageClipped, args)
{
	unsigned short x, y;
	PyObject* imageObj = NULL;
	unsigned short cx, cy, cw, ch;
	char respectAlpha;
	if (!PyArg_ParseTuple(args, "hhOhhhhb", &x, &y, &imageObj, &cx, &cy, &cw, &ch, &respectAlpha))
	{
		PyErr_SetString(PyExc_TypeError, "drawImageClipped expects two shorts, a ptImage, four shorts, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	if (!pyImage::Check(imageObj))
	{
		PyErr_SetString(PyExc_TypeError, "drawImageClipped expects two shorts, a ptImage, four shorts, and a boolean");
		PYTHON_RETURN_ERROR;
	}
	pyImage* image = pyImage::ConvertFrom(imageObj);
	self->fThis->DrawImageClipped(x, y, *image, cx, cy, cw, ch, respectAlpha != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDynamicMap, getWidth)
{
	return PyInt_FromLong(self->fThis->GetWidth());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDynamicMap, getHeight)
{
	return PyInt_FromLong(self->fThis->GetHeight());
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, calcTextExtents, args)
{
	PyObject* textObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &textObj))
	{
		PyErr_SetString(PyExc_TypeError, "calcTextExtents expects a string");
		PYTHON_RETURN_ERROR;
	}

	std::wstring wText;
	if (PyUnicode_Check(textObj))
	{
		int strLen = PyUnicode_GetSize(textObj);
		wchar_t* text = TRACKED_NEW wchar_t[strLen + 1];
		PyUnicode_AsWideChar((PyUnicodeObject*)textObj, text, strLen);
		text[strLen] = L'\0';
		wText = text;
		delete [] text;
	}
	else if (PyString_Check(textObj))
	{
		// we'll allow this, just in case something goes weird
		char* text = PyString_AsString(textObj);
		wchar_t* temp = hsStringToWString(text);
		wText = temp;
		delete [] temp;
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "calcTextExtents expects a string");
		PYTHON_RETURN_ERROR;
	}

	unsigned height, width;
	self->fThis->CalcTextExtents(wText, width, height);
	PyObject* retVal = PyTuple_New(2);
	PyTuple_SetItem(retVal, 0, PyInt_FromLong(width));
	PyTuple_SetItem(retVal, 1, PyInt_FromLong(height));
	return retVal;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, setJustify, args)
{
	unsigned char justify;
	if (!PyArg_ParseTuple(args, "b", &justify))
	{
		PyErr_SetString(PyExc_TypeError, "setJustify expects a unsigned 8-bit int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetJustify(justify);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptDynamicMap, setLineSpacing, args)
{
	short spacing;
	if (!PyArg_ParseTuple(args, "h", &spacing))
	{
		PyErr_SetString(PyExc_TypeError, "setLineSpacing expects a short int");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetLineSpacing(spacing);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptDynamicMap, getImage)
{
	return pyImage::New(self->fThis->GetImage());
}

PYTHON_START_METHODS_TABLE(ptDynamicMap)
	PYTHON_METHOD(ptDynamicMap, sender, "Params: sender\nSet the sender of the message being sent to the DynamicMap"),
	PYTHON_BASIC_METHOD(ptDynamicMap, clearKeys, "Clears the receiver list"),
	PYTHON_METHOD(ptDynamicMap, addKey, "Params: key\nAdd a receiver... in other words a DynamicMap"),
	PYTHON_METHOD(ptDynamicMap, netPropagate, "Params: propagateFlag\nSpecify whether this object needs to use messages that are sent on the network\n"
				"- The default is for this to be false."),
	PYTHON_METHOD(ptDynamicMap, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
				"- This is to be used if your Python program is running on only one client\n"
				"Such as a game master, only running on the client that owns a particular object\n"
				"This only applies when NetPropagate is set to true"),
	PYTHON_METHOD(ptDynamicMap, clearToColor, "Params: color\nClear the DynamicMap to the specified color\n"
				"- 'color' is a ptColor object"),
	PYTHON_BASIC_METHOD(ptDynamicMap, flush, "Flush all the commands that were issued since the last flush()"),
	PYTHON_BASIC_METHOD(ptDynamicMap, purgeImage, "Purge the DynamicTextMap images"),
	PYTHON_METHOD(ptDynamicMap, setTextColor, "Params: color, blockRGB=0\nSet the color of the text to be written\n"
				"- 'color' is a ptColor object\n"
				"- 'blockRGB' must be true if you're trying to render onto a transparent or semi-transparent color"),
	PYTHON_METHOD(ptDynamicMap, setFont, "Params: facename,size\nSet the font of the text to be written\n"
				"- 'facename' is a string with the name of the font\n"
				"- 'size' is the point size of the font to use"),
	PYTHON_METHOD(ptDynamicMap, fillRect, "Params: left,top,right,bottom,color\nFill in the specified rectangle with a color\n"
				"- left,top,right,bottom define the rectangle\n"
				"- 'color' is a ptColor object"),	
	PYTHON_METHOD(ptDynamicMap, frameRect, "Params: left,top,right,bottom,color\nFrame a rectangle with a specified color\n"
				"- left,top,right,bottom define the rectangle\n"
				"- 'color' is a ptColor object"),
	PYTHON_METHOD(ptDynamicMap, setClipping, "Params: clipLeft,clipTop,clipRight,clipBottom\nSets the clipping rectangle\n"
				"- All drawtext will be clipped to this until the\n"
				"unsetClipping() is called"),
	PYTHON_BASIC_METHOD(ptDynamicMap, unsetClipping, "Stop the clipping of text"),
	PYTHON_METHOD(ptDynamicMap, setWrapping, "Params: wrapWidth,wrapHeight\nSet where text will be wrapped horizontally and vertically\n"
				"- All drawtext commands will be wrapped until the\n"
				"unsetWrapping() is called"),
	PYTHON_BASIC_METHOD(ptDynamicMap, unsetWrapping, "Stop text wrapping"),
	PYTHON_METHOD(ptDynamicMap, drawText, "Params: x,y,text\nDraw text at a specified location\n"
				"- x,y is the point to start drawing the text\n"
				"- 'text' is a string of the text to be drawn"),
	PYTHON_METHOD(ptDynamicMap, drawTextW, "Params: x,y,text\nUnicode version of drawText"),
	PYTHON_METHOD(ptDynamicMap, drawImage, "Params: x,y,image,respectAlphaFlag\nDraws a ptImage object on the dynamicTextmap starting at the location x,y"),
	PYTHON_METHOD(ptDynamicMap, drawImageClipped, "Params: x,y,image,cx,cy,cw,ch,respectAlphaFlag\nDraws a ptImage object clipped to cx,cy with cw(width),ch(height)"),
	PYTHON_METHOD_NOARGS(ptDynamicMap, getWidth, "Returns the width of the dynamicTextmap"),
	PYTHON_METHOD_NOARGS(ptDynamicMap, getHeight, "Returns the height of the dynamicTextmap"),
	PYTHON_METHOD(ptDynamicMap, calcTextExtents, "Params: text\nCalculates the extent of the specified text, returns it as a (width, height) tuple"),
	PYTHON_METHOD(ptDynamicMap, setJustify, "Params: justify\nSets the justification of the text. (justify is a PtJustify)"),
	PYTHON_METHOD(ptDynamicMap, setLineSpacing, "Params: spacing\nSets the line spacing (in pixels)"),
	PYTHON_METHOD_NOARGS(ptDynamicMap, getImage, "Returns a pyImage associated with the dynamicTextmap"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptDynamicMap, "Params: key=None\nCreates a ptDynamicMap object");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptDynamicMap, pyDynamicText)

PyObject *pyDynamicText::New(pyKey& key)
{
	ptDynamicMap *newObj = (ptDynamicMap*)ptDynamicMap_type.tp_new(&ptDynamicMap_type, NULL, NULL);
	newObj->fThis->fReceivers.Append(key.getKey());
	return (PyObject*)newObj;
}

PyObject *pyDynamicText::New(plKey key)
{
	ptDynamicMap *newObj = (ptDynamicMap*)ptDynamicMap_type.tp_new(&ptDynamicMap_type, NULL, NULL);
	newObj->fThis->fReceivers.Append(key);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptDynamicMap, pyDynamicText)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptDynamicMap, pyDynamicText)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyDynamicText::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptDynamicMap);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyDynamicText::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtJustify);
	PYTHON_ENUM_ELEMENT(PtJustify, kCenter, plDynamicTextMap::Justify::kCenter);
	PYTHON_ENUM_ELEMENT(PtJustify, kLeftJustify, plDynamicTextMap::Justify::kLeftJustify);
	PYTHON_ENUM_ELEMENT(PtJustify, kRightJustify, plDynamicTextMap::Justify::kRightJustify);
	PYTHON_ENUM_END(m, PtJustify);
}