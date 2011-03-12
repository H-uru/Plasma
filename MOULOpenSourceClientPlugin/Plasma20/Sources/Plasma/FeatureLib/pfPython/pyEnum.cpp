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
/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyEnum
//
// PURPOSE: Base class stuff for enumeration support (you don't instance this
//			class
//

#include "pyEnum.h"

#include <python.h>
#include "structmember.h"
#include "pyGlueHelpers.h"

#if HS_BUILD_FOR_MAC
#include <stdio.h>
#include <bxString.h>
#endif

struct EnumValue {
	PyObject_HEAD
	long value;
	char* name;
};

PyObject *NewEnumValue(char const* name, long value);

static PyObject *EnumValue_new(PyTypeObject *type, PyObject *args, PyObject *)
{
	EnumValue *self = (EnumValue*)type->tp_alloc(type, 0);
	if (self != NULL)
	{
		char *nameTemp = NULL;
		long value;
		if (!PyArg_ParseTuple(args, "l|s", &value, &nameTemp))
		{
			Py_DECREF(self);
			return NULL;
		}

		if (nameTemp) // copy the value if it was passed
		{
			self->name = TRACKED_NEW char[strlen(nameTemp) + 1];
			strcpy(self->name, nameTemp);
			self->name[strlen(nameTemp)] = '\0';
		}
		else
			self->name = NULL;
		self->value = value;
	}

	return (PyObject*)self;
}

static void EnumValue_dealloc(EnumValue *self)
{
	if (self->name)
		delete [] self->name;
	self->ob_type->tp_free((PyObject*)self);
}

static int EnumValue_print(PyObject *self, FILE *fp, int flags)
{
	// convert this object to a string
	PyObject *strObject = (flags & Py_PRINT_RAW) ? self->ob_type->tp_str(self) : self->ob_type->tp_repr(self);
	if (strObject == NULL)
		return -1; // failure

	const char* text = PyString_AsString(strObject);
	if (text == NULL)
		return -1;

	fprintf(fp, text); // and print it to the file
	return 0;
}

static PyObject *EnumValue_repr(PyObject *self)
{
	EnumValue *obj = (EnumValue*)self;
	if (obj->name == NULL)
	{
		// no name, so just output our value
		return PyString_FromFormat("%s(%ld)", obj->ob_type->tp_name, obj->value);
	}
	else
	{
		// we have a name, so output it
		return PyString_FromString(obj->name);
	}
}

static PyObject* EnumValue_str(PyObject* self)
{
	EnumValue *obj = (EnumValue*)self;
	if (obj->name == NULL)
	{
		// no name, so return our value
		return PyString_FromFormat("%s(%ld)", obj->ob_type->tp_name, obj->value);
	}
	else
	{
		// just return the name
		return PyString_FromString(obj->name);
	}
}

// forward def because the type object isn't defined yet
bool IsEnumValue(PyObject *obj);

long EnumValue_hash(PyObject *v)
{
	// stolen from python's int class
	if (!IsEnumValue(v))
	{
		PyErr_SetString(PyExc_TypeError, "hash was passed a non enum value (this would be weird)");
		return 0;
	}
	long x = ((EnumValue*)v)->value;
	if (x == -1)
		x = -2;
	return x;
}

int EnumValue_nonzero(EnumValue *v)
{
	return v->value != 0;
}

PyObject *EnumValue_and(PyObject *v, PyObject *w)
{
	EnumValue *obj = NULL;
	long other = 0;
	if (IsEnumValue(v))
	{
		obj = (EnumValue*)v;
		if (!PyInt_Check(w))
		{
			Py_INCREF(Py_NotImplemented);
			return Py_NotImplemented;
		}
		other = PyInt_AsLong(w);
	}
	else if (IsEnumValue(w))
	{
		obj = (EnumValue*)w;
		if (!PyInt_Check(v))
		{
			Py_INCREF(Py_NotImplemented);
			return Py_NotImplemented;
		}
		other = PyInt_AsLong(v);
	}
	else
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	return PyInt_FromLong(obj->value & other);
}

PyObject *EnumValue_xor(PyObject *v, PyObject *w)
{
	EnumValue *obj = NULL;
	long other = 0;
	if (IsEnumValue(v))
	{
		obj = (EnumValue*)v;
		if (!PyInt_Check(w))
		{
			Py_INCREF(Py_NotImplemented);
			return Py_NotImplemented;
		}
		other = PyInt_AsLong(w);
	}
	else if (IsEnumValue(w))
	{
		obj = (EnumValue*)w;
		if (!PyInt_Check(v))
		{
			Py_INCREF(Py_NotImplemented);
			return Py_NotImplemented;
		}
		other = PyInt_AsLong(v);
	}
	else
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	return PyInt_FromLong(obj->value ^ other);
}

PyObject *EnumValue_or(PyObject *v, PyObject *w)
{
	EnumValue *obj = NULL;
	long other = 0;
	if (IsEnumValue(v))
	{
		obj = (EnumValue*)v;
		if (!PyInt_Check(w))
		{
			Py_INCREF(Py_NotImplemented);
			return Py_NotImplemented;
		}
		other = PyInt_AsLong(w);
	}
	else if (IsEnumValue(w))
	{
		obj = (EnumValue*)w;
		if (!PyInt_Check(v))
		{
			Py_INCREF(Py_NotImplemented);
			return Py_NotImplemented;
		}
		other = PyInt_AsLong(v);
	}
	else
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	return PyInt_FromLong(obj->value | other);
}

PyObject *EnumValue_int(EnumValue *v)
{
	return PyInt_FromLong(v->value);
}

PyObject *EnumValue_long(EnumValue *v)
{
	return PyLong_FromLong((v->value));
}

PyObject *EnumValue_float(EnumValue *v)
{
	return PyFloat_FromDouble((double)(v->value));
}

PyObject *EnumValue_oct(EnumValue *v)
{
	char buf[100];
	long x = v->value;
	if (x < 0)
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	if (x == 0)
		strcpy(buf, "0");
	else
		_snprintf(buf, sizeof(buf), "0%lo", x);
	return PyString_FromString(buf);
}

PyObject *EnumValue_hex(EnumValue *v)
{
	char buf[100];
	long x = v->value;
	if (x < 0)
	{
		Py_INCREF(Py_NotImplemented);
		return Py_NotImplemented;
	}
	_snprintf(buf, sizeof(buf), "0x%lx", x);
	return PyString_FromString(buf);
}

int EnumValue_coerce(PyObject **pv, PyObject **pw)
{
	if (PyInt_Check(*pw))
	{
		long x = PyInt_AsLong(*pw);
		*pw = NewEnumValue(NULL, x);
		Py_INCREF(*pv);
		return 0;
	}
	else if (PyLong_Check(*pw))
	{
		double x = PyLong_AsDouble(*pw);
		if (x == -1.0 && PyErr_Occurred())
			return -1;
		*pw = NewEnumValue(NULL, (long)x);
		Py_INCREF(*pv);
		return 0;
	}
	else if (PyFloat_Check(*pw))
	{
		double x = PyFloat_AsDouble(*pw);
		*pw = NewEnumValue(NULL, (long)x);
		Py_INCREF(*pv);
		return 0;
	}
	else if (IsEnumValue(*pw))
	{
		Py_INCREF(*pv);
		Py_INCREF(*pw);
		return 0;
	}
	return 1; // Can't do it
}

// we support some of the number methods
PYTHON_START_AS_NUMBER_TABLE(EnumValue)
	0,							/*nb_add*/
	0,							/*nb_subtract*/
	0,							/*nb_multiply*/
	0,							/*nb_divide*/
	0,							/*nb_remainder*/
	0,							/*nb_divmod*/
	0,							/*nb_power*/
	0,							/*nb_negative*/
	0,							/*nb_positive*/
	0,							/*nb_absolute*/
	(inquiry)EnumValue_nonzero,	/*nb_nonzero*/
	0,							/*nb_invert*/
	0,							/*nb_lshift*/
	0,							/*nb_rshift*/
	(binaryfunc)EnumValue_and,	/*nb_and*/
	(binaryfunc)EnumValue_xor,	/*nb_xor*/
	(binaryfunc)EnumValue_or,	/*nb_or*/
	(coercion)EnumValue_coerce,	/*nb_coerce*/
	(unaryfunc)EnumValue_int,	/*nb_int*/
	(unaryfunc)EnumValue_long,	/*nb_long*/
	(unaryfunc)EnumValue_float,	/*nb_float*/
	(unaryfunc)EnumValue_oct,	/*nb_oct*/
	(unaryfunc)EnumValue_hex, 	/*nb_hex*/
	0,							/*nb_inplace_add*/
	0,							/*nb_inplace_subtract*/
	0,							/*nb_inplace_multiply*/
	0,							/*nb_inplace_divide*/
	0,							/*nb_inplace_remainder*/
	0,							/*nb_inplace_power*/
	0,							/*nb_inplace_lshift*/
	0,							/*nb_inplace_rshift*/
	0,							/*nb_inplace_and*/
	0,							/*nb_inplace_xor*/
	0,							/*nb_inplace_or*/
	0,							/* nb_floor_divide */
	0,							/* nb_true_divide */
	0,							/* nb_inplace_floor_divide */
	0,							/* nb_inplace_true_divide */
PYTHON_END_AS_NUMBER_TABLE;

PYTHON_COMPARE_DEFINITION(EnumValue, v, w)
{
	long i = 0;
	long j = 0;
	if (IsEnumValue(v))
	{
		i = ((EnumValue*)v)->value;
		if (PyInt_Check(w))
			j = PyInt_AsLong(w);
		else if (PyFloat_Check(w))
			j = (long)PyFloat_AsDouble(w);
		else if (PyLong_Check(w))
			j = PyLong_AsLong(w);
		else if (IsEnumValue(w))
			j = ((EnumValue*)w)->value;
		else
		{
			PyErr_SetString(PyExc_TypeError, "cannot compare EnumValue to a non number");
			return 0;
		}

	}
	else if (IsEnumValue(w))
	{
		j = ((EnumValue*)w)->value;
		if (PyInt_Check(v))
			i = PyInt_AsLong(v);
		else if (PyFloat_Check(v))
			i = (long)PyFloat_AsDouble(v);
		else if (PyLong_Check(v))
			i = PyLong_AsLong(v);
		else if (IsEnumValue(v))
			i = ((EnumValue*)v)->value;
		else
		{
			PyErr_SetString(PyExc_TypeError, "cannot compare EnumValue to a non number");
			return 0;
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError, "cannot compare EnumValue to a non number");
		return 0;
	}

	if (i < j)
		PYTHON_COMPARE_LESS_THAN;
	else if (i > j)
		PYTHON_COMPARE_GREATER_THAN;
	PYTHON_COMPARE_EQUAL;
}

PYTHON_NO_INIT_DEFINITION(EnumValue)

PYTHON_START_METHODS_TABLE(EnumValue)
PYTHON_END_METHODS_TABLE;

PYTHON_TYPE_START(EnumValue)
	0,
	"PlasmaConstants.EnumValue",
	sizeof(EnumValue),					/* tp_basicsize */
	0,									/* tp_itemsize */
	(destructor)EnumValue_dealloc,		/* tp_dealloc */
	EnumValue_print,					/* tp_print */
	0,									/* tp_getattr */
	0,									/* tp_setattr */
	PYTHON_DEFAULT_COMPARE(EnumValue),	/* tp_compare */
	EnumValue_repr,						/* tp_repr */
	PYTHON_DEFAULT_AS_NUMBER(EnumValue),/* tp_as_number */
	0,									/* tp_as_sequence */
	0,									/* tp_as_mapping */
	EnumValue_hash,						/* tp_hash */
	0,									/* tp_call */
	EnumValue_str,						/* tp_str */
	0,									/* tp_getattro */
	0,									/* tp_setattro */
	0,									/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_CHECKTYPES,			/* tp_flags */
	"A basic enumeration value",		/* tp_doc */
	0,									/* tp_traverse */
	0,									/* tp_clear */
	0,									/* tp_richcompare */
	0,									/* tp_weaklistoffset */
	0,									/* tp_iter */
	0,									/* tp_iternext */
	PYTHON_DEFAULT_METHODS_TABLE(EnumValue),	/* tp_methods */
	0,									/* tp_members */
	0,									/* tp_getset */
	0,									/* tp_base */
	0,									/* tp_dict */
	0,									/* tp_descr_get */
	0,									/* tp_descr_set */
	0,									/* tp_dictoffset */
	PYTHON_DEFAULT_INIT(EnumValue),		/* tp_init */
	0,									/* tp_alloc */
	EnumValue_new						/* tp_new */
PYTHON_TYPE_END;

bool IsEnumValue(PyObject *obj)
{
	return PyObject_TypeCheck(obj, &EnumValue_type);
}

PyObject *NewEnumValue(char const* name, long value)
{
	PyObject *tempArgs = NULL;
	if (name)
		tempArgs = Py_BuildValue("(ls)", value, name); // args are value, name
	else
		tempArgs = Py_BuildValue("(l)", value); // args are value only
	EnumValue *newObj = (EnumValue*)EnumValue_type.tp_new(&EnumValue_type, tempArgs, NULL);
	Py_DECREF(tempArgs); // clean up the temps
	return (PyObject*)newObj;
}

// Now for the Enum base class

struct Enum
{
	PyObject_HEAD
	PyObject *values; // the values list
	PyObject *lookup; // the enum values, key is enum, value is enum value
	PyObject *reverseLookup; // the enum values, key is enum value, value is enum
};

static PyObject *Enum_new(PyTypeObject *type, PyObject *, PyObject *)
{
	Enum *self = (Enum*)type->tp_alloc(type, 0);
	if (self != NULL)
	{
		self->values = PyDict_New();
		if (self->values == NULL)
		{
			Py_DECREF(self);
			return NULL;
		}
		self->lookup = PyDict_New();
		if (self->lookup == NULL)
		{
			Py_DECREF(self);
			return NULL;
		}
		self->reverseLookup = PyDict_New();
		if (self->reverseLookup == NULL)
		{
			Py_DECREF(self);
			return NULL;
		}
	}

	return (PyObject*)self;
}

static int Enum_traverse(Enum *self, visitproc visit, void *arg)
{
	if (self->values && visit(self->values, arg) < 0)
		return -1;
	if (self->lookup && visit(self->lookup, arg) < 0)
		return -1;
	if (self->reverseLookup && visit(self->reverseLookup, arg) < 0)
		return -1;

	return 0;
}

static int Enum_clear(Enum *self)
{
	Py_XDECREF(self->values);
	self->values = NULL;
	Py_XDECREF(self->lookup);
	self->lookup = NULL;
	Py_XDECREF(self->reverseLookup);
	self->reverseLookup = NULL;

	return 0;
}

static void Enum_dealloc(Enum *self)
{
	Enum_clear(self);
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *Enum_getattro(PyObject *self, PyObject *attr_name)
{
	if (!PyString_Check(attr_name))
	{
		PyErr_SetString(PyExc_TypeError, "getattro expects a string argument");
		PYTHON_RETURN_ERROR;
	}
	Enum *theEnum = (Enum*)self;
	PyObject *item = PyDict_GetItem(theEnum->lookup, attr_name);
	if (!item)
	{
		PyErr_Clear(); // wasn't in the dictionary, check to see if they want the values or lookup dict
		char *name = PyString_AsString(attr_name);
		if (strcmp(name, "values") == 0)
		{
			Py_INCREF(theEnum->values);
			return theEnum->values;
		}
		else if (strcmp(name, "lookup") == 0)
		{
			Py_INCREF(theEnum->lookup);
			return theEnum->lookup;
		}
		else if (strcmp(name, "reverseLookup") == 0)
		{
			Py_INCREF(theEnum->reverseLookup);
			return theEnum->reverseLookup;
		}
		return PyObject_GenericGetAttr(self, attr_name); // let the default method handle it
	}
	Py_INCREF(item);
	return item;
}

static int Enum_setattro(PyObject *self, PyObject *attr_name, PyObject *value)
{
	if (!PyString_Check(attr_name))
	{
		PyErr_SetString(PyExc_TypeError, "setattro expects a string argument");
		return -1;;
	}
	Enum *theEnum = (Enum*)self;
	PyObject *item = PyDict_GetItem(theEnum->lookup, attr_name);
	if (!item)
	{
		PyErr_Clear(); // wasn't in the dictionary, check to see if they want the values or lookup dict
		char *name = PyString_AsString(attr_name);
		if (strcmp(name, "values") == 0)
		{
			PyErr_SetString(PyExc_RuntimeError, "Cannot set the value attribute");
			return -1;
		}
		else if (strcmp(name, "lookup") == 0)
		{
			PyErr_SetString(PyExc_RuntimeError, "Cannot set the lookup attribute");
			return -1;
		}
		else if (strcmp(name, "reverseLookup") == 0)
		{
			PyErr_SetString(PyExc_RuntimeError, "Cannot set the reverseLookup attribute");
			return -1;
		}
		// they aren't trying to set an enum value or any of our special values, so let them
		return PyObject_GenericSetAttr(self, attr_name, value); // let the default method handle it
	}
	PyErr_SetString(PyExc_RuntimeError, "Cannot set any enum value");
	return -1;
}

PYTHON_NO_INIT_DEFINITION(Enum)

PYTHON_START_METHODS_TABLE(Enum)
PYTHON_END_METHODS_TABLE;

PYTHON_TYPE_START(Enum)
	0,
	"PlasmaConstants.Enum",
	sizeof(Enum),						/* tp_basicsize */
	0,									/* tp_itemsize */
	(destructor)Enum_dealloc,			/* tp_dealloc */
	0,									/* tp_print */
	0,									/* tp_getattr */
	0,									/* tp_setattr */
	0,									/* tp_compare */
	0,									/* tp_repr */
	0,									/* tp_as_number */
	0,									/* tp_as_sequence */
	0,									/* tp_as_mapping */
	0,									/* tp_hash */
	0,									/* tp_call */
	0,									/* tp_str */
	Enum_getattro,						/* tp_getattro */
	Enum_setattro,						/* tp_setattro */
	0,									/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT
	| Py_TPFLAGS_BASETYPE
	| Py_TPFLAGS_HAVE_GC,				/* tp_flags */
	"Enum base class",					/* tp_doc */
	(traverseproc)Enum_traverse,		/* tp_traverse */
	(inquiry)Enum_clear,				/* tp_clear */
	0,									/* tp_richcompare */
	0,									/* tp_weaklistoffset */
	0,									/* tp_iter */
	0,									/* tp_iternext */
	PYTHON_DEFAULT_METHODS_TABLE(Enum),	/* tp_methods */
	0,									/* tp_members */
	0,									/* tp_getset */
	0,									/* tp_base */
	0,									/* tp_dict */
	0,									/* tp_descr_get */
	0,									/* tp_descr_set */
	0,									/* tp_dictoffset */
	PYTHON_DEFAULT_INIT(Enum),			/* tp_init */
	0,									/* tp_alloc */
	Enum_new							/* tp_new */
PYTHON_TYPE_END;

// creates and sets up the enum base class
void pyEnum::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, EnumValue);
	PYTHON_CLASS_IMPORT(m, Enum);
	PYTHON_CLASS_IMPORT_END(m);
}

// makes an enum object using the specified name and values
void pyEnum::MakeEnum(PyObject *m, const char* name, std::map<std::string, int> values)
{
	if (m == NULL)
		return;

	Enum *newEnum = (Enum*)Enum_type.tp_new(&Enum_type, NULL, NULL);
	if (newEnum == NULL)
	{
		std::string errorStr = "Could not create enum named ";
		errorStr += name;
		PyErr_SetString(PyExc_RuntimeError, errorStr.c_str());
		return;
	}

	PyObject *valuesDict = newEnum->values;
	PyObject *lookupDict = newEnum->lookup;
	PyObject *reverseLookupDict = newEnum->reverseLookup;
	for (std::map<std::string, int>::iterator curValue = values.begin(); curValue != values.end(); curValue++)
	{
		std::string key = curValue->first;
		int value = curValue->second;

		std::string enumValueName = name;
		enumValueName += "." + key;

		PyObject *newValue = NewEnumValue(enumValueName.c_str(), value);
		PyObject *valueObj = PyInt_FromLong((long)value);
		PyObject *keyObj = PyString_FromString(key.c_str());
		PyDict_SetItem(valuesDict, valueObj, newValue);
		PyDict_SetItem(lookupDict, keyObj, newValue);
		PyDict_SetItem(reverseLookupDict, newValue, keyObj);
		Py_DECREF(keyObj);
		Py_DECREF(valueObj);
		Py_DECREF(newValue);
	}

	PyModule_AddObject(m, (char*)name, (PyObject*)newEnum);
}
