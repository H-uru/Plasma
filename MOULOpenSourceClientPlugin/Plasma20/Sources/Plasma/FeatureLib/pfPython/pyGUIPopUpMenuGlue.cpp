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
#include "pyGUIPopUpMenu.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptGUIPopUpMenu, pyGUIPopUpMenu);

PYTHON_DEFAULT_NEW_DEFINITION(ptGUIPopUpMenu, pyGUIPopUpMenu)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGUIPopUpMenu)

PYTHON_INIT_DEFINITION(ptGUIPopUpMenu, args, keywords)
{
	PyObject* arg1 = NULL;
	PyObject* arg2 = NULL;
	PyObject* arg3 = NULL;
	PyObject* arg4 = NULL;
	if (!PyArg_ParseTuple(args, "O|OOO", &arg1, &arg2, &arg3, &arg4))
	{
		PyErr_SetString(PyExc_TypeError, "__init__ expects one of three argument lists:\n"
					"1 - ptKey\n"
					"2 - string, float, float\n"
					"3 - string, ptGUIPopUpMenu, float, float");
		PYTHON_RETURN_INIT_ERROR;
	}
	if (pyKey::Check(arg1))
	{
		// arg list 1
		if (arg2 || arg3 || arg4)
		{
			// too many arguments
			PyErr_SetString(PyExc_TypeError, "__init__ expects one of three argument lists:\n"
				"1 - ptKey\n"
				"2 - string, float, float\n"
				"3 - string, ptGUIPopUpMenu, float, float");
			PYTHON_RETURN_INIT_ERROR;
		}
		pyKey* key = pyKey::ConvertFrom(arg1);
		self->fThis->setup(key->getKey());
		PYTHON_RETURN_INIT_OK;
	}
	else if (PyString_Check(arg1))
	{
		// arg list 2 or 3
		char* name = PyString_AsString(arg1);
		if (PyFloat_Check(arg2))
		{
			// arg list 2
			if ((!PyFloat_Check(arg3)) || arg4)
			{
				// invalid types or too many arguments
				PyErr_SetString(PyExc_TypeError, "__init__ expects one of three argument lists:\n"
					"1 - ptKey\n"
					"2 - string, float, float\n"
					"3 - string, ptGUIPopUpMenu, float, float");
				PYTHON_RETURN_INIT_ERROR;
			}
			float originX = (float)PyFloat_AsDouble(arg2);
			float originY = (float)PyFloat_AsDouble(arg3);
			self->fThis->setup(name, originX, originY);
			PYTHON_RETURN_INIT_OK;
		}
		else if (pyGUIPopUpMenu::Check(arg2))
		{
			// arg list 3
			if ((!PyFloat_Check(arg3)) || (!PyFloat_Check(arg4)))
			{
				// invalid types
				PyErr_SetString(PyExc_TypeError, "__init__ expects one of three argument lists:\n"
					"1 - ptKey\n"
					"2 - string, float, float\n"
					"3 - string, ptGUIPopUpMenu, float, float");
				PYTHON_RETURN_INIT_ERROR;
			}
			pyGUIPopUpMenu* parent = pyGUIPopUpMenu::ConvertFrom(arg2);
			float originX = (float)PyFloat_AsDouble(arg3);
			float originY = (float)PyFloat_AsDouble(arg4);
			self->fThis->setup(name, *parent, originX, originY);
			PYTHON_RETURN_INIT_OK;
		}
	}
	// invalid type
	PyErr_SetString(PyExc_TypeError, "__init__ expects one of three argument lists:\n"
		"1 - ptKey\n"
		"2 - string, float, float\n"
		"3 - string, ptGUIPopUpMenu, float, float");
	PYTHON_RETURN_INIT_ERROR;
}

PYTHON_RICH_COMPARE_DEFINITION(ptGUIPopUpMenu, obj1, obj2, compareType)
{
	if ((obj1 == Py_None) || (obj2 == Py_None) || !pyGUIPopUpMenu::Check(obj1) || !pyGUIPopUpMenu::Check(obj2))
	{
		// if they aren't the same type, they don't match, obviously (we also never equal none)
		if (compareType == Py_EQ)
			PYTHON_RCOMPARE_FALSE;
		else if (compareType == Py_NE)
			PYTHON_RCOMPARE_TRUE;
		else
		{
			PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUIPopUpMenu object");
			PYTHON_RCOMPARE_ERROR;
		}
	}
	pyGUIPopUpMenu *menu1 = pyGUIPopUpMenu::ConvertFrom(obj1);
	pyGUIPopUpMenu *menu2 = pyGUIPopUpMenu::ConvertFrom(obj2);
	if (compareType == Py_EQ)
	{
		if ((*menu1) == (*menu2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	else if (compareType == Py_NE)
	{
		if ((*menu1) != (*menu2))
			PYTHON_RCOMPARE_TRUE;
		PYTHON_RCOMPARE_FALSE;
	}
	PyErr_SetString(PyExc_NotImplementedError, "invalid comparison for a ptGUIPopUpMenu object");
	PYTHON_RCOMPARE_ERROR;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getKey)
{
	return self->fThis->getObjPyKey();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getTagID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetTagID());
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, enable, args)
{
	char enableFlag = 1;
	if (!PyArg_ParseTuple(args, "|b", &enableFlag))
	{
		PyErr_SetString(PyExc_TypeError, "enable expects an optional boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetEnabled(enableFlag != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIPopUpMenu, disable, Disable)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, isEnabled)
{
	PYTHON_RETURN_BOOL(self->fThis->IsEnabled());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getName)
{
	return PyString_FromString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getVersion)
{
	return PyLong_FromUnsignedLong(self->fThis->GetVersion());
}

PYTHON_BASIC_METHOD_DEFINITION(ptGUIPopUpMenu, show, Show)
PYTHON_BASIC_METHOD_DEFINITION(ptGUIPopUpMenu, hide, Hide)

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getForeColor)
{
	return self->fThis->GetForeColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getSelectColor)
{
	return self->fThis->GetSelColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getBackColor)
{
	return self->fThis->GetBackColor();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGUIPopUpMenu, getBackSelectColor)
{
	return self->fThis->GetBackSelColor();
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, setForeColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setForeColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetForeColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, setSelectColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setSelectColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetSelColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, setBackColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setBackColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetBackColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, setBackSelectColor, args)
{
	float r, g, b, a;
	if (!PyArg_ParseTuple(args, "ffff", &r, &g, &b, &a))
	{
		PyErr_SetString(PyExc_TypeError, "setBackSelectColor expects four floats");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetBackSelColor(r, g, b, a);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, addConsoleCmdItem, args)
{
	char* name;
	char* consoleCmd;
	if (!PyArg_ParseTuple(args, "ss", &name, &consoleCmd))
	{
		PyErr_SetString(PyExc_TypeError, "addConsoleCmdItem expects two strings");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddConsoleCmdItem(name, consoleCmd);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, addConsoleCmdItemW, args)
{
	wchar_t* name;
	char* consoleCmd;
	if (!PyArg_ParseTuple(args, "us", &name, &consoleCmd))
	{
		PyErr_SetString(PyExc_TypeError, "addConsoleCmdItemW expects a unicode string and a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddConsoleCmdItemW(name, consoleCmd);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, addNotifyItem, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "addNotifyItem expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddNotifyItem(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, addNotifyItemW, args)
{
	wchar_t* name;
	if (!PyArg_ParseTuple(args, "u", &name))
	{
		PyErr_SetString(PyExc_TypeError, "addNotifyItemW expects a unicode string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddNotifyItemW(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, addSubMenuItem, args)
{
	char* name;
	PyObject* subMenuObj = NULL;
	if (!PyArg_ParseTuple(args, "sO", &name, &subMenuObj))
	{
		PyErr_SetString(PyExc_TypeError, "addSubMenuItem expects a string and a ptGUIPopUpMenu");
		PYTHON_RETURN_ERROR;
	}
	if (!pyGUIPopUpMenu::Check(subMenuObj))
	{
		PyErr_SetString(PyExc_TypeError, "addSubMenuItem expects a string and a ptGUIPopUpMenu");
		PYTHON_RETURN_ERROR;
	}
	pyGUIPopUpMenu* subMenu = pyGUIPopUpMenu::ConvertFrom(subMenuObj);
	self->fThis->AddSubMenuItem(name, *subMenu);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGUIPopUpMenu, addSubMenuItemW, args)
{
	wchar_t* name;
	PyObject* subMenuObj = NULL;
	if (!PyArg_ParseTuple(args, "uO", &name, &subMenuObj))
	{
		PyErr_SetString(PyExc_TypeError, "addSubMenuItemW expects a unicode string and a ptGUIPopUpMenu");
		PYTHON_RETURN_ERROR;
	}
	if (!pyGUIPopUpMenu::Check(subMenuObj))
	{
		PyErr_SetString(PyExc_TypeError, "addSubMenuItemW expects a unicode string and a ptGUIPopUpMenu");
		PYTHON_RETURN_ERROR;
	}
	pyGUIPopUpMenu* subMenu = pyGUIPopUpMenu::ConvertFrom(subMenuObj);
	self->fThis->AddSubMenuItemW(name, *subMenu);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptGUIPopUpMenu)
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getKey, "Returns this menu's key"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getTagID, "Returns this menu's tag id"),
	PYTHON_METHOD(ptGUIPopUpMenu, enable, "Params: state=1\nEnables/disables this menu"),
	PYTHON_BASIC_METHOD(ptGUIPopUpMenu, disable, "Disables this menu"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, isEnabled, "Returns whether this menu is enabled or not"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getName, "Returns this menu's name"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getVersion, "UNKNOWN"),
	PYTHON_BASIC_METHOD(ptGUIPopUpMenu, show, "Shows this menu"),
	PYTHON_BASIC_METHOD(ptGUIPopUpMenu, hide, "Hides this menu"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getForeColor, "Returns the foreground color"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getSelectColor, "Returns the selection color"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getBackColor, "Returns the background color"),
	PYTHON_METHOD_NOARGS(ptGUIPopUpMenu, getBackSelectColor, "Returns the background selection color"),
	PYTHON_METHOD(ptGUIPopUpMenu, setForeColor, "Params: r,g,b,a\nSets the foreground color"),
	PYTHON_METHOD(ptGUIPopUpMenu, setSelectColor, "Params: r,g,b,a\nSets the selection color"),
	PYTHON_METHOD(ptGUIPopUpMenu, setBackColor, "Params: r,g,b,a\nSets the background color"),
	PYTHON_METHOD(ptGUIPopUpMenu, setBackSelectColor, "Params: r,g,b,a\nSets the selection background color"),
	PYTHON_METHOD(ptGUIPopUpMenu, addConsoleCmdItem, "Params: name,consoleCmd\nAdds a new item to the menu that fires a console command"),
	PYTHON_METHOD(ptGUIPopUpMenu, addConsoleCmdItemW, "Params: name,consoleCmd\nUnicode version of addConsoleCmdItem"),
	PYTHON_METHOD(ptGUIPopUpMenu, addNotifyItem, "Params: name\nAdds a new item ot the mneu"),
	PYTHON_METHOD(ptGUIPopUpMenu, addNotifyItemW, "Params: name\nUnicode version of addNotifyItem"),
	PYTHON_METHOD(ptGUIPopUpMenu, addSubMenuItem, "Params: name,subMenu\nAdds a submenu to this menu"),
	PYTHON_METHOD(ptGUIPopUpMenu, addSubMenuItemW, "Params: name,subMenu\nUnicode version of addSubMenuItem"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptGUIPopUpMenu_COMPARE		PYTHON_NO_COMPARE
#define ptGUIPopUpMenu_AS_NUMBER	PYTHON_NO_AS_NUMBER
#define ptGUIPopUpMenu_AS_SEQUENCE	PYTHON_NO_AS_SEQUENCE
#define ptGUIPopUpMenu_AS_MAPPING	PYTHON_NO_AS_MAPPING
#define ptGUIPopUpMenu_STR			PYTHON_NO_STR
#define ptGUIPopUpMenu_RICH_COMPARE	PYTHON_DEFAULT_RICH_COMPARE(ptGUIPopUpMenu)
#define ptGUIPopUpMenu_GETSET		PYTHON_NO_GETSET
#define ptGUIPopUpMenu_BASE			PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptGUIPopUpMenu, "Params: arg1,arg2=None,arg3=None,arg4=None\nTakes three diferent argument lists:\n"
			"gckey\n"
			"name,screenOriginX,screenOriginY\n"
			"name,parent,screenOriginX,screenOriginY");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptGUIPopUpMenu, pyGUIPopUpMenu)

PyObject *pyGUIPopUpMenu::New(pyKey& gckey)
{
	ptGUIPopUpMenu *newObj = (ptGUIPopUpMenu*)ptGUIPopUpMenu_type.tp_new(&ptGUIPopUpMenu_type, NULL, NULL);
	newObj->fThis->fGCkey = gckey.getKey();
	newObj->fThis->fBuiltMenu = nil;
	return (PyObject*)newObj;
}

PyObject *pyGUIPopUpMenu::New(plKey objkey)
{
	ptGUIPopUpMenu *newObj = (ptGUIPopUpMenu*)ptGUIPopUpMenu_type.tp_new(&ptGUIPopUpMenu_type, NULL, NULL);
	newObj->fThis->fGCkey = objkey;
	newObj->fThis->fBuiltMenu = nil;
	return (PyObject*)newObj;
}

PyObject *pyGUIPopUpMenu::New(const char *name, hsScalar screenOriginX, hsScalar screenOriginY, const plLocation &destLoc /* = plLocation::kGlobalFixedLoc */)
{
	ptGUIPopUpMenu *newObj = (ptGUIPopUpMenu*)ptGUIPopUpMenu_type.tp_new(&ptGUIPopUpMenu_type, NULL, NULL);
	newObj->fThis->setup(name, screenOriginX, screenOriginY, destLoc);
	return (PyObject*)newObj;
}

PyObject *pyGUIPopUpMenu::New(const char *name, pyGUIPopUpMenu &parent, hsScalar screenOriginX, hsScalar screenOriginY)
{
	ptGUIPopUpMenu *newObj = (ptGUIPopUpMenu*)ptGUIPopUpMenu_type.tp_new(&ptGUIPopUpMenu_type, NULL, NULL);
	newObj->fThis->setup(name, parent, screenOriginX, screenOriginY);
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGUIPopUpMenu, pyGUIPopUpMenu)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGUIPopUpMenu, pyGUIPopUpMenu)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGUIPopUpMenu::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptGUIPopUpMenu);
	PYTHON_CLASS_IMPORT_END(m);
}