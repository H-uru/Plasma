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
#include "pySDL.h"
#include "..\plSDL\plSDL.h"
#include "pyEnum.h"
#include "pyKey.h"

#include <python.h>

void pySDL::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtSDLReadWriteOptions);
	PYTHON_ENUM_ELEMENT(PtSDLReadWriteOptions, kDirtyOnly,				plSDL::kDirtyOnly);
	PYTHON_ENUM_ELEMENT(PtSDLReadWriteOptions, kSkipNotificationInfo,	plSDL::kSkipNotificationInfo);
	PYTHON_ENUM_ELEMENT(PtSDLReadWriteOptions, kBroadcast,				plSDL::kBroadcast);
	//PYTHON_ENUM_ELEMENT(PtSDLReadWriteOptions, kWriteTimeStamps,		plSDL::kWriteTimeStamps);
	PYTHON_ENUM_ELEMENT(PtSDLReadWriteOptions, kTimeStampOnRead,		plSDL::kTimeStampOnRead);
	//PYTHON_ENUM_ELEMENT(PtSDLReadWriteOptions, kTimeStampDirtyOnRead,	plSDL::kTimeStampDirtyOnRead);
	PYTHON_ENUM_END(m, PtSDLReadWriteOptions);
	
	PYTHON_ENUM_START(PtSDLVarType);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kNone,			plVarDescriptor::kNone);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kInt,				plVarDescriptor::kInt);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kFloat,			plVarDescriptor::kFloat);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kBool,			plVarDescriptor::kBool);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kString32,		plVarDescriptor::kString32);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kKey,				plVarDescriptor::kKey);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kStateDescriptor,	plVarDescriptor::kStateDescriptor);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kCreatable,		plVarDescriptor::kCreatable);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kDouble,			plVarDescriptor::kDouble);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kTime,			plVarDescriptor::kTime);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kVector3,			plVarDescriptor::kVector3);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kPoint3,			plVarDescriptor::kPoint3);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kRGB,				plVarDescriptor::kRGB);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kRGBA,			plVarDescriptor::kRGBA);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kQuaternion,		plVarDescriptor::kQuaternion);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kByte,			plVarDescriptor::kByte);
	PYTHON_ENUM_ELEMENT(PtSDLVarType, kShort,			plVarDescriptor::kShort);
	PYTHON_ENUM_END(m, PtSDLVarType);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptSDLStateDataRecord, pySDLStateDataRecord);

PYTHON_DEFAULT_NEW_DEFINITION(ptSDLStateDataRecord, pySDLStateDataRecord)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSDLStateDataRecord)

PYTHON_INIT_DEFINITION(ptSDLStateDataRecord, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptSDLStateDataRecord, findVar, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "findVar expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->FindVar(name);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSDLStateDataRecord, getName)
{
	return PyString_FromString(self->fThis->GetName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSDLStateDataRecord, getVarList)
{
	std::vector<std::string> vars = self->fThis->GetVarList();
	PyObject* varList = PyList_New(vars.size());
	for (int i = 0; i < vars.size(); i++)
		PyList_SetItem(varList, i, PyString_FromString(vars[i].c_str()));
	return varList;
}

PYTHON_METHOD_DEFINITION(ptSDLStateDataRecord, setFromDefaults, args)
{
	char timeStampNow;
	if (!PyArg_ParseTuple(args, "b", &timeStampNow))
	{
		PyErr_SetString(PyExc_TypeError, "setFromDefaults expects a boolean");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetFromDefaults(timeStampNow != 0);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptSDLStateDataRecord)
	PYTHON_METHOD(ptSDLStateDataRecord, findVar, "Params: name\nFinds and returns the specified ptSimpleStateVariable"),
	PYTHON_METHOD_NOARGS(ptSDLStateDataRecord, getName, "Returns our record's name"),
	PYTHON_METHOD_NOARGS(ptSDLStateDataRecord, getVarList, "Returns the names of the vars we hold as a list of strings"),
	PYTHON_METHOD(ptSDLStateDataRecord, setFromDefaults, "Params: timeStampNow\nSets all our vars to their defaults"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptSDLStateDataRecord, "Basic SDL state data record class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptSDLStateDataRecord, pySDLStateDataRecord)

PyObject *pySDLStateDataRecord::New(plStateDataRecord* rec)
{
	ptSDLStateDataRecord* newObj = (ptSDLStateDataRecord*)ptSDLStateDataRecord_type.tp_new(&ptSDLStateDataRecord_type, NULL, NULL);
	newObj->fThis->fRec = rec;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSDLStateDataRecord, pySDLStateDataRecord)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSDLStateDataRecord, pySDLStateDataRecord)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySDLStateDataRecord::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSDLStateDataRecord);
	PYTHON_CLASS_IMPORT_END(m);
}

// glue functions
PYTHON_CLASS_DEFINITION(ptSimpleStateVariable, pySimpleStateVariable);

PYTHON_DEFAULT_NEW_DEFINITION(ptSimpleStateVariable, pySimpleStateVariable)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptSimpleStateVariable)

PYTHON_INIT_DEFINITION(ptSimpleStateVariable, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

// two little macros to make things a little easier
#define STATEVAR_SET(funcName, cFuncName, typeName, cTypeName, pythonTypeChar) \
PYTHON_METHOD_DEFINITION(ptSimpleStateVariable, funcName, args) \
{ \
	cTypeName val; \
	int idx = 0; \
	if (!PyArg_ParseTuple(args, #pythonTypeChar "|i", &val, &idx)) \
	{ \
		PyErr_SetString(PyExc_TypeError, #funcName " expects a " #typeName " and an optional int"); \
		PYTHON_RETURN_ERROR; \
	} \
	PYTHON_RETURN_BOOL(self->fThis->cFuncName(val, idx)); \
}

#define STATEVAR_GET(funcName, cFuncName, pythonReturnFunc) \
PYTHON_METHOD_DEFINITION(ptSimpleStateVariable, funcName, args) \
{ \
	int idx = 0; \
	if (!PyArg_ParseTuple(args, "|i", &idx)) \
	{ \
		PyErr_SetString(PyExc_TypeError, #funcName " expects an optional int"); \
		PYTHON_RETURN_ERROR; \
	} \
	return pythonReturnFunc(self->fThis->cFuncName(idx)); \
}

STATEVAR_SET(setByte, SetByte, byte, char, b)
STATEVAR_SET(setShort, SetShort, short, short, h)
STATEVAR_SET(setFloat, SetFloat, float, float, f)
STATEVAR_SET(setDouble, SetDouble, double, double, d)
STATEVAR_SET(setInt, SetInt, int, int, i)
STATEVAR_SET(setString, SetString, string, char*, s)

// setBool is special cause of the way python represents booleans
PYTHON_METHOD_DEFINITION(ptSimpleStateVariable, setBool, args)
{
	char val;
	int idx = 0;
	if (!PyArg_ParseTuple(args, "b|i", &val, &idx))
	{
		PyErr_SetString(PyExc_TypeError, "setBool expects a boolean and an optional int");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->SetBool(val != 0, idx));
}

STATEVAR_GET(getByte, GetByte, PyInt_FromLong)
STATEVAR_GET(getShort, GetShort, PyInt_FromLong)
STATEVAR_GET(getInt, GetInt, PyInt_FromLong)
STATEVAR_GET(getFloat, GetFloat, PyFloat_FromDouble)
STATEVAR_GET(getDouble, GetDouble, PyFloat_FromDouble)
STATEVAR_GET(getString, GetString, PyString_FromString)
STATEVAR_GET(getKey, GetKey, pyKey::New)

// getBool is special cause of the way python represents booleans
PYTHON_METHOD_DEFINITION(ptSimpleStateVariable, getBool, args)
{
	int idx = 0;
	if (!PyArg_ParseTuple(args, "|i", &idx))
	{
		PyErr_SetString(PyExc_TypeError, "getBool expects an optional int");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->GetBool(idx));
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSimpleStateVariable, getType)
{
	return PyInt_FromLong(self->fThis->GetType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSimpleStateVariable, getDisplayOptions)
{
	return PyString_FromString(self->fThis->GetDisplayOptions());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSimpleStateVariable, getDefault)
{
	return PyString_FromString(self->fThis->GetDefault());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSimpleStateVariable, isAlwaysNew)
{
	PYTHON_RETURN_BOOL(self->fThis->IsAlwaysNew());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptSimpleStateVariable, isInternal)
{
	PYTHON_RETURN_BOOL(self->fThis->IsInternal());
}

PYTHON_START_METHODS_TABLE(ptSimpleStateVariable)
	PYTHON_METHOD(ptSimpleStateVariable, setByte, "Params: val,idx=0\nSets a byte variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, setShort, "Params: val,idx=0\nSets a short variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, setFloat, "Params: val,idx=0\nSets a float variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, setDouble, "Params: val,idx=0\nSets a double variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, setInt, "Params: val,idx=0\nSets an int variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, setString, "Params: val,idx=0\nSets a string variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, setBool, "Params: val,idx=0\nSets a boolean variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getByte, "Params: idx=0\nReturns a byte variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getShort, "Params: idx=0\nReturns a short variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getInt, "Params: idx=0\nReturns an int variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getFloat, "Params: idx=0\nReturns a float variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getDouble, "Params: idx=0\nReturns a double variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getString, "Params: idx=0\nReturns a string variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getKey, "Params: idx=0\nReturns a plKey variable's value"),
	PYTHON_METHOD(ptSimpleStateVariable, getBool, "Params: idx=0\nReturns a boolean variable's value"),
	PYTHON_METHOD_NOARGS(ptSimpleStateVariable, getType, "Returns the variable's type"),
	PYTHON_METHOD_NOARGS(ptSimpleStateVariable, getDisplayOptions, "Returns the variable's display options"),
	PYTHON_METHOD_NOARGS(ptSimpleStateVariable, getDefault, "Returns the variable's default"),
	PYTHON_METHOD_NOARGS(ptSimpleStateVariable, isAlwaysNew, "Is this variable always new?"),
	PYTHON_METHOD_NOARGS(ptSimpleStateVariable, isInternal, "Is this an internal variable?"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptSimpleStateVariable, "Basic SDL state data record class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptSimpleStateVariable, pySimpleStateVariable)

PyObject *pySimpleStateVariable::New(plSimpleStateVariable* var)
{
	ptSimpleStateVariable* newObj = (ptSimpleStateVariable*)ptSimpleStateVariable_type.tp_new(&ptSimpleStateVariable_type, NULL, NULL);
	newObj->fThis->fVar = var;
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptSimpleStateVariable, pySimpleStateVariable)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptSimpleStateVariable, pySimpleStateVariable)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pySimpleStateVariable::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptSimpleStateVariable);
	PYTHON_CLASS_IMPORT_END(m);
}