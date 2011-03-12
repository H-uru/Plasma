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
#include "pyColor.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptColor, pyColor);

PYTHON_DEFAULT_NEW_DEFINITION(ptColor, pyColor)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptColor)

PYTHON_INIT_DEFINITION(ptColor, args, keywords)
{
	char *kwlist[] = {"red", "green", "blue", "alpha", NULL};
	PyObject* redObj = NULL;
	PyObject* greenObj = NULL;
	PyObject* blueObj = NULL;
	PyObject* alphaObj = NULL;
	float red = 0.0f, green = 0.0f, blue = 0.0f, alpha = 0.0f;
	if (!PyArg_ParseTupleAndKeywords(args, keywords, "|OOOO", kwlist, &redObj, &greenObj, &blueObj, &alphaObj))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects four optional floats");
		PYTHON_RETURN_INIT_ERROR;
	}
	// extra code to allow ints if people are lazy
	if (redObj)
	{
		if (PyFloat_Check(redObj))
			red = (float)PyFloat_AsDouble(redObj);
		else if (PyInt_Check(redObj))
			red = (float)PyInt_AsLong(redObj);
		else
		{
			PyErr_SetString(PyExc_TypeError, "__init__ expects four optional floats");
			PYTHON_RETURN_INIT_ERROR;
		}
	}
	if (greenObj)
	{
		if (PyFloat_Check(greenObj))
			green = (float)PyFloat_AsDouble(greenObj);
		else if (PyInt_Check(greenObj))
			green = (float)PyInt_AsLong(greenObj);
		else
		{
			PyErr_SetString(PyExc_TypeError, "__init__ expects four optional floats");
			PYTHON_RETURN_INIT_ERROR;
		}
	}
	if (blueObj)
	{
		if (PyFloat_Check(blueObj))
			blue = (float)PyFloat_AsDouble(blueObj);
		else if (PyInt_Check(blueObj))
			blue = (float)PyInt_AsLong(blueObj);
		else
		{
			PyErr_SetString(PyExc_TypeError, "__init__ expects four optional floats");
			PYTHON_RETURN_INIT_ERROR;
		}
	}
	if (alphaObj)
	{
		if (PyFloat_Check(alphaObj))
			alpha = (float)PyFloat_AsDouble(alphaObj);
		else if (PyInt_Check(alphaObj))
			alpha = (float)PyInt_AsLong(alphaObj);
		else
		{
			PyErr_SetString(PyExc_TypeError, "__init__ expects four optional floats");
			PYTHON_RETURN_INIT_ERROR;
		}
	}

	self->fThis->setRed(red);
	self->fThis->setGreen(green);
	self->fThis->setBlue(blue);
	self->fThis->setAlpha(alpha);

	PYTHON_RETURN_INIT_OK;
}

PYTHON_RICH_COMPARE_DEFINITION(ptColor, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyColor::Check(obj1) || !pyColor::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptColor object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyColor *color1 = pyColor::ConvertFrom(obj1);
	pyColor *color2 = pyColor::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*color1) == (*color2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*color1) != (*color2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptColor object");
	PYTHON_RCOMPARE_ERROR;
}
// quick and easy macro for making element get functions
#define GET_ELEMENT_FUNC(funcName) \
	PYTHON_METHOD_DEFINITION_NOARGS(ptColor, funcName) \
{ \
	return PyFloat_FromDouble((double)self->fThis->funcName()); \
}

GET_ELEMENT_FUNC(getRed)
GET_ELEMENT_FUNC(getGreen)
GET_ELEMENT_FUNC(getBlue)
GET_ELEMENT_FUNC(getAlpha)

// quick and easy macro for making element set functions
#define SET_ELEMENT_FUNC(funcName) \
	PYTHON_METHOD_DEFINITION(ptColor, funcName, args) \
{ \
	float value; \
	if (!PyArg_ParseTuple(args, "f", &value)) \
{ \
	PyErr_SetString(PyExc_TypeError, #funcName " expects a float"); \
	PYTHON_RETURN_ERROR; \
} \
	self->fThis->funcName(value); \
	PYTHON_RETURN_NONE; \
}

SET_ELEMENT_FUNC(setRed)
SET_ELEMENT_FUNC(setGreen)
SET_ELEMENT_FUNC(setBlue)
SET_ELEMENT_FUNC(setAlpha)

// quick and easy macro for the color functions
#define COLOR_FUNC(funcName, classFunc) \
	PYTHON_METHOD_DEFINITION_NOARGS(ptColor, funcName) \
{ \
	self->fThis->classFunc(); /* set the internal color */ \
	Py_INCREF((PyObject*)self); /* incref it because we have to return a new reference */ \
	return (PyObject*)self; /* and return ourself */ \
}

COLOR_FUNC(white, White)
COLOR_FUNC(black, Black)
COLOR_FUNC(red, Red)
COLOR_FUNC(green, Green)
COLOR_FUNC(blue, Blue)
COLOR_FUNC(magenta, Magenta)
COLOR_FUNC(cyan, Cyan)
COLOR_FUNC(yellow, Yellow)
COLOR_FUNC(brown, Brown)
COLOR_FUNC(gray, Gray)
COLOR_FUNC(orange, Orange)
COLOR_FUNC(pink, Pink)
COLOR_FUNC(darkbrown, DarkBrown)
COLOR_FUNC(darkgreen, DarkGreen)
COLOR_FUNC(darkpurple, DarkPurple)
COLOR_FUNC(navyblue, NavyBlue)
COLOR_FUNC(maroon, Maroon)
COLOR_FUNC(tan, Tan)
COLOR_FUNC(slateblue, SlateBlue)
COLOR_FUNC(steelblue, SteelBlue)

PYTHON_START_METHODS_TABLE(ptColor)
	PYTHON_METHOD_NOARGS(ptColor, getRed, "Get the red component of the color"),
	PYTHON_METHOD_NOARGS(ptColor, getGreen, "Get the green component of the color"),
	PYTHON_METHOD_NOARGS(ptColor, getBlue, "Get the blue component of the color"),
	PYTHON_METHOD_NOARGS(ptColor, getAlpha, "Get the alpha blend component of the color"),
	
	PYTHON_METHOD(ptColor, setRed, "Params: red\nSet the red component of the color. 0.0 to 1.0"),
	PYTHON_METHOD(ptColor, setGreen, "Params: green\nSet the green component of the color. 0.0 to 1.0"),
	PYTHON_METHOD(ptColor, setBlue, "Params: blue\nSet the blue component of the color. 0.0 to 1.0"),
	PYTHON_METHOD(ptColor, setAlpha, "Params: alpha\nSet the alpha blend component of the color. 0.0 to 1.0"),
	
	PYTHON_METHOD_NOARGS(ptColor, white, "Sets the color to be white\n"
				"Example: white = ptColor().white()"),
	PYTHON_METHOD_NOARGS(ptColor, black, "Sets the color to be black\n"
				"Example: black = ptColor().black()"),
	PYTHON_METHOD_NOARGS(ptColor, red, "Sets the color to be red\n"
				"Example: red = ptColor().red()"),
	PYTHON_METHOD_NOARGS(ptColor, green, "Sets the color to be green\n"
				"Example: green = ptColor().green()"),
	PYTHON_METHOD_NOARGS(ptColor, blue, "Sets the color to be blue\n"
				"Example: blue = ptColor().blue()"),
	PYTHON_METHOD_NOARGS(ptColor, magenta, "Sets the color to be magenta\n"
				"Example: magenta = ptColor().magenta()"),
	PYTHON_METHOD_NOARGS(ptColor, cyan, "Sets the color to be cyan\n"
				"Example: cyan = ptColor.cyan()"),
	PYTHON_METHOD_NOARGS(ptColor, yellow, "Sets the color to be yellow\n"
				"Example: yellow = ptColor().yellow()"),
	PYTHON_METHOD_NOARGS(ptColor, brown, "Sets the color to be brown\n"
				"Example: brown = ptColor().brown()"),
	PYTHON_METHOD_NOARGS(ptColor, gray, "Sets the color to be gray\n"
				"Example: gray = ptColor().gray()"),
	PYTHON_METHOD_NOARGS(ptColor, orange, "Sets the color to be orange\n"
				"Example: orange = ptColor().orange()"),
	PYTHON_METHOD_NOARGS(ptColor, pink, "Sets the color to be pink\n"
				"Example: pink = ptColor().pink()"),
	PYTHON_METHOD_NOARGS(ptColor, darkbrown, "Sets the color to be darkbrown\n"
				"Example: darkbrown = ptColor().darkbrown()"),
	PYTHON_METHOD_NOARGS(ptColor, darkgreen, "Sets the color to be darkgreen\n"
				"Example: darkgreen = ptColor().darkgreen()"),
	PYTHON_METHOD_NOARGS(ptColor, darkpurple, "Sets the color to be darkpurple\n"
				"Example: darkpurple = ptColor().darkpurple()"),
	PYTHON_METHOD_NOARGS(ptColor, navyblue, "Sets the color to be navyblue\n"
				"Example: navyblue = ptColor().navyblue()"),
	PYTHON_METHOD_NOARGS(ptColor, maroon, "Sets the color to be maroon\n"
				"Example: maroon = ptColor().maroon()"),
	PYTHON_METHOD_NOARGS(ptColor, tan, "Sets the color to be tan\n"
				"Example: tan = ptColor().tan()"),
	PYTHON_METHOD_NOARGS(ptColor, slateblue, "Sets the color to be slateblue\n"
				"Example: slateblue = ptColor().slateblue()"),
	PYTHON_METHOD_NOARGS(ptColor, steelblue, "Sets the color to be steelblue\n"
				"Example: steelblue = ptColor().steelblue()"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptColor_COMPARE			PYTHON_NO_COMPARE
#define ptColor_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptColor_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptColor_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptColor_STR				PYTHON_NO_STR
#define ptColor_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptColor)
#define ptColor_GETSET			PYTHON_NO_GETSET
#define ptColor_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptColor, "Params: red=0, green=0, blue=0, alpha=0\nPlasma color class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptColor, pyColor)

PyObject *pyColor::New(hsScalar red, hsScalar green, hsScalar blue, hsScalar alpha)
{
	ptColor *newObj = (ptColor*)ptColor_type.tp_new(&ptColor_type, NULL, NULL);
	newObj->fThis->setRed(red);
	newObj->fThis->setGreen(green);
	newObj->fThis->setBlue(blue);
	newObj->fThis->setAlpha(alpha);
	return (PyObject*)newObj;
}

PyObject *pyColor::New(const hsColorRGBA & color)
{
	ptColor *newObj = (ptColor*)ptColor_type.tp_new(&ptColor_type, NULL, NULL);
	newObj->fThis->setColor(color);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptColor, pyColor)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptColor, pyColor)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyColor::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptColor);
	PYTHON_CLASS_IMPORT_END(m);
}