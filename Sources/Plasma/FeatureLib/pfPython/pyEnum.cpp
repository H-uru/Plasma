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
/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyEnum
//
// PURPOSE: Base class stuff for enumeration support (you don't instance this
//          class
//

#include "HeadSpin.h"
#include <Python.h>
#include <structmember.h>
#include "pyGlueHelpers.h"

#include "pyEnum.h"
#include <string_theory/format>

struct EnumValue {
    PyObject_HEAD
    long value;
    char* name;
};

PyObject *NewEnumValue(char const* name, long value);

static PyObject *EnumValue_new(PyTypeObject *type, PyObject *args, PyObject *)
{
    EnumValue *self = (EnumValue*)type->tp_alloc(type, 0);
    if (self != nullptr)
    {
        char *nameTemp = nullptr;
        long value;
        if (!PyArg_ParseTuple(args, "l|s", &value, &nameTemp))
        {
            Py_DECREF(self);
            return nullptr;
        }

        if (nameTemp) // copy the value if it was passed
        {
            self->name = new char[strlen(nameTemp) + 1];
            strcpy(self->name, nameTemp);
            self->name[strlen(nameTemp)] = '\0';
        }
        else
            self->name = nullptr;
        self->value = value;
    }

    return (PyObject*)self;
}

static void EnumValue_dealloc(EnumValue *self)
{
    if (self->name)
        delete [] self->name;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *EnumValue_repr(PyObject *self)
{
    EnumValue *obj = (EnumValue*)self;
    if (obj->name == nullptr)
    {
        // no name, so just output our value
        return PyUnicode_FromFormat("%s(%ld)", Py_TYPE(obj)->tp_name, obj->value);
    }
    else
    {
        // we have a name, so output it
        return PyUnicode_FromString(obj->name);
    }
}

static PyObject* EnumValue_str(PyObject* self)
{
    EnumValue *obj = (EnumValue*)self;
    if (obj->name == nullptr)
    {
        // no name, so return our value
        return PyUnicode_FromFormat("%s(%ld)", Py_TYPE(obj)->tp_name, obj->value);
    }
    else
    {
        // just return the name
        return PyUnicode_FromString(obj->name);
    }
}

// forward def because the type object isn't defined yet
bool IsEnumValue(PyObject *obj);

Py_hash_t EnumValue_hash(PyObject *v)
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
    EnumValue *obj = nullptr;
    long other = 0;
    if (IsEnumValue(v))
    {
        obj = (EnumValue*)v;
        if (!PyLong_Check(w))
        {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        other = PyLong_AsLong(w);
    }
    else if (IsEnumValue(w))
    {
        obj = (EnumValue*)w;
        if (!PyLong_Check(v))
        {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        other = PyLong_AsLong(v);
    }
    else
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    return PyLong_FromLong(obj->value & other);
}

PyObject *EnumValue_xor(PyObject *v, PyObject *w)
{
    EnumValue *obj = nullptr;
    long other = 0;
    if (IsEnumValue(v))
    {
        obj = (EnumValue*)v;
        if (!PyLong_Check(w))
        {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        other = PyLong_AsLong(w);
    }
    else if (IsEnumValue(w))
    {
        obj = (EnumValue*)w;
        if (!PyLong_Check(v))
        {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        other = PyLong_AsLong(v);
    }
    else
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    return PyLong_FromLong(obj->value ^ other);
}

PyObject *EnumValue_or(PyObject *v, PyObject *w)
{
    EnumValue *obj = nullptr;
    long other = 0;
    if (IsEnumValue(v))
    {
        obj = (EnumValue*)v;
        if (!PyLong_Check(w))
        {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        other = PyLong_AsLong(w);
    }
    else if (IsEnumValue(w))
    {
        obj = (EnumValue*)w;
        if (!PyLong_Check(v))
        {
            Py_INCREF(Py_NotImplemented);
            return Py_NotImplemented;
        }
        other = PyLong_AsLong(v);
    }
    else
    {
        Py_INCREF(Py_NotImplemented);
        return Py_NotImplemented;
    }
    return PyLong_FromLong(obj->value | other);
}

PyObject *EnumValue_int(EnumValue *v)
{
    return PyLong_FromLong(v->value);
}

PyObject *EnumValue_long(EnumValue *v)
{
    return PyLong_FromLong((v->value));
}

PyObject *EnumValue_float(EnumValue *v)
{
    return PyFloat_FromDouble((double)(v->value));
}

// we support some of the number methods
PYTHON_START_AS_NUMBER_TABLE(EnumValue)
    nullptr,                    /*nb_add*/
    nullptr,                    /*nb_subtract*/
    nullptr,                    /*nb_multiply*/
    nullptr,                    /*nb_remainder*/
    nullptr,                    /*nb_divmod*/
    nullptr,                    /*nb_power*/
    nullptr,                    /*nb_negative*/
    nullptr,                    /*nb_positive*/
    nullptr,                    /*nb_absolute*/
    (inquiry)EnumValue_nonzero, /*nb_bool*/
    nullptr,                    /*nb_invert*/
    nullptr,                    /*nb_lshift*/
    nullptr,                    /*nb_rshift*/
    (binaryfunc)EnumValue_and,  /*nb_and*/
    (binaryfunc)EnumValue_xor,  /*nb_xor*/
    (binaryfunc)EnumValue_or,   /*nb_or*/
    (unaryfunc)EnumValue_long,  /*nb_int*/
    nullptr,                    /*nb_reserved*/
    (unaryfunc)EnumValue_float, /*nb_float*/
    nullptr,                    /*nb_inplace_add*/
    nullptr,                    /*nb_inplace_subtract*/
    nullptr,                    /*nb_inplace_multiply*/
    nullptr,                    /*nb_inplace_remainder*/
    nullptr,                    /*nb_inplace_power*/
    nullptr,                    /*nb_inplace_lshift*/
    nullptr,                    /*nb_inplace_rshift*/
    nullptr,                    /*nb_inplace_and*/
    nullptr,                    /*nb_inplace_xor*/
    nullptr,                    /*nb_inplace_or*/
    nullptr,                    /* nb_floor_divide */
    nullptr,                    /* nb_true_divide */
    nullptr,                    /* nb_inplace_floor_divide */
    nullptr,                    /* nb_inplace_true_divide */
    (unaryfunc)EnumValue_long,  /* nb_index */
    nullptr,                    /* nb_matrix_multiply */
    nullptr,                    /* nb_inplace_matrix_multiply */
PYTHON_END_AS_NUMBER_TABLE;

PYTHON_RICH_COMPARE_DEFINITION(EnumValue, v, w, compareType)
{
    long i = 0;
    long j = 0;
    if (IsEnumValue(v)) {
        i = ((EnumValue*)v)->value;
        if (PyLong_Check(w))
            j = PyLong_AsLong(w);
        else if (PyFloat_Check(w))
            j = (long)PyFloat_AsDouble(w);
        else if (IsEnumValue(w))
            j = ((EnumValue*)w)->value;
        else
            PYTHON_RETURN_NOT_IMPLEMENTED;

    } else if (IsEnumValue(w)) {
        j = ((EnumValue*)w)->value;
        if (PyLong_Check(v))
            i = PyLong_AsLong(v);
        else if (PyFloat_Check(v))
            i = (long)PyFloat_AsDouble(v);
        else if (IsEnumValue(v))
            i = ((EnumValue*)v)->value;
        else
            PYTHON_RETURN_NOT_IMPLEMENTED;
    } else {
        PYTHON_RETURN_NOT_IMPLEMENTED;
    }

    switch (compareType) {
    case Py_LT:
        PYTHON_RETURN_BOOL(i < j);
    case Py_LE:
        PYTHON_RETURN_BOOL(i <= j);
    case Py_EQ:
        PYTHON_RETURN_BOOL(i == j);
    case Py_NE:
        PYTHON_RETURN_BOOL(i != j);
    case Py_GT:
        PYTHON_RETURN_BOOL(i > j);
    case Py_GE:
        PYTHON_RETURN_BOOL(i >= j);
    DEFAULT_FATAL(compareType);
    }
}

PYTHON_NO_INIT_DEFINITION(EnumValue)

PYTHON_START_METHODS_TABLE(EnumValue)
PYTHON_END_METHODS_TABLE;

PYTHON_TYPE_START(EnumValue)
    "PlasmaConstants.EnumValue",
    sizeof(EnumValue),                  /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)EnumValue_dealloc,      /* tp_dealloc */
    PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET,
    nullptr,                            /* tp_getattr */
    nullptr,                            /* tp_setattr */
    nullptr,                            /* tp_as_async */
    EnumValue_repr,                     /* tp_repr */
    PYTHON_DEFAULT_AS_NUMBER(EnumValue),/* tp_as_number */
    nullptr,                            /* tp_as_sequence */
    nullptr,                            /* tp_as_mapping */
    EnumValue_hash,                     /* tp_hash */
    nullptr,                            /* tp_call */
    EnumValue_str,                      /* tp_str */
    nullptr,                            /* tp_getattro */
    nullptr,                            /* tp_setattro */
    nullptr,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_BASETYPE,              /* tp_flags */
    "A basic enumeration value",        /* tp_doc */
    nullptr,                            /* tp_traverse */
    nullptr,                            /* tp_clear */
    EnumValue_richCompare,              /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    nullptr,                            /* tp_iter */
    nullptr,                            /* tp_iternext */
    PYTHON_DEFAULT_METHODS_TABLE(EnumValue),    /* tp_methods */
    nullptr,                            /* tp_members */
    nullptr,                            /* tp_getset */
    nullptr,                            /* tp_base */
    nullptr,                            /* tp_dict */
    nullptr,                            /* tp_descr_get */
    nullptr,                            /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    PYTHON_DEFAULT_INIT(EnumValue),     /* tp_init */
    nullptr,                            /* tp_alloc */
    EnumValue_new,                      /* tp_new */
    nullptr,                            /* tp_free */
    nullptr,                            /* tp_is_gc */
    nullptr,                            /* tp_bases */
    nullptr,                            /* tp_mro */
    nullptr,                            /* tp_cache */
    nullptr,                            /* tp_subclasses */
    nullptr,                            /* tp_weaklist */
    nullptr,                            /* tp_del */
    0,                                  /* tp_version_tag */
    nullptr,                            /* tp_finalize */
    PYTHON_TP_VECTORCALL_PRINT
PYTHON_TYPE_END;

bool IsEnumValue(PyObject *obj)
{
    return PyObject_TypeCheck(obj, &EnumValue_type);
}

PyObject *NewEnumValue(char const* name, long value)
{
    PyObject *tempArgs = nullptr;
    if (name)
        tempArgs = Py_BuildValue("(ls)", value, name); // args are value, name
    else
        tempArgs = Py_BuildValue("(l)", value); // args are value only
    EnumValue *newObj = (EnumValue*)EnumValue_type.tp_new(&EnumValue_type, tempArgs, nullptr);
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
    if (self != nullptr)
    {
        self->values = PyDict_New();
        if (self->values == nullptr)
        {
            Py_DECREF(self);
            return nullptr;
        }
        self->lookup = PyDict_New();
        if (self->lookup == nullptr)
        {
            Py_DECREF(self);
            return nullptr;
        }
        self->reverseLookup = PyDict_New();
        if (self->reverseLookup == nullptr)
        {
            Py_DECREF(self);
            return nullptr;
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
    self->values = nullptr;
    Py_XDECREF(self->lookup);
    self->lookup = nullptr;
    Py_XDECREF(self->reverseLookup);
    self->reverseLookup = nullptr;

    return 0;
}

static void Enum_dealloc(Enum *self)
{
    Enum_clear(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *Enum_getattro(PyObject *self, PyObject *attr_name)
{
    if (!PyUnicode_Check(attr_name))
    {
        PyErr_SetString(PyExc_TypeError, "getattro expects a string argument");
        PYTHON_RETURN_ERROR;
    }
    Enum *theEnum = (Enum*)self;
    PyObject *item = PyDict_GetItem(theEnum->lookup, attr_name);
    if (!item)
    {
        PyErr_Clear(); // wasn't in the dictionary, check to see if they want the values or lookup dict
        const char *name = PyUnicode_AsUTF8(attr_name);
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
    if (!PyUnicode_Check(attr_name))
    {
        PyErr_SetString(PyExc_TypeError, "setattro expects a string argument");
        return -1;;
    }
    Enum *theEnum = (Enum*)self;
    PyObject *item = PyDict_GetItem(theEnum->lookup, attr_name);
    if (!item)
    {
        PyErr_Clear(); // wasn't in the dictionary, check to see if they want the values or lookup dict
        const char *name = PyUnicode_AsUTF8(attr_name);
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
    "PlasmaConstants.Enum",
    sizeof(Enum),                       /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)Enum_dealloc,           /* tp_dealloc */
    PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET,
    nullptr,                            /* tp_getattr */
    nullptr,                            /* tp_setattr */
    nullptr,                            /* tp_compare */
    nullptr,                            /* tp_repr */
    nullptr,                            /* tp_as_number */
    nullptr,                            /* tp_as_sequence */
    nullptr,                            /* tp_as_mapping */
    nullptr,                            /* tp_hash */
    nullptr,                            /* tp_call */
    nullptr,                            /* tp_str */
    Enum_getattro,                      /* tp_getattro */
    Enum_setattro,                      /* tp_setattro */
    nullptr,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
    | Py_TPFLAGS_BASETYPE
    | Py_TPFLAGS_HAVE_GC,               /* tp_flags */
    "Enum base class",                  /* tp_doc */
    (traverseproc)Enum_traverse,        /* tp_traverse */
    (inquiry)Enum_clear,                /* tp_clear */
    nullptr,                            /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    nullptr,                            /* tp_iter */
    nullptr,                            /* tp_iternext */
    PYTHON_DEFAULT_METHODS_TABLE(Enum), /* tp_methods */
    nullptr,                            /* tp_members */
    nullptr,                            /* tp_getset */
    nullptr,                            /* tp_base */
    nullptr,                            /* tp_dict */
    nullptr,                            /* tp_descr_get */
    nullptr,                            /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    PYTHON_DEFAULT_INIT(Enum),          /* tp_init */
    nullptr,                            /* tp_alloc */
    Enum_new,                           /* tp_new */
    nullptr,                            /* tp_free */
    nullptr,                            /* tp_is_gc */
    nullptr,                            /* tp_bases */
    nullptr,                            /* tp_mro */
    nullptr,                            /* tp_cache */
    nullptr,                            /* tp_subclasses */
    nullptr,                            /* tp_weaklist */
    nullptr,                            /* tp_del */
    0,                                  /* tp_version_tag */
    nullptr,                            /* tp_finalize */
    PYTHON_TP_VECTORCALL_PRINT
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
    if (m == nullptr)
        return;

    Enum *newEnum = (Enum*)Enum_type.tp_new(&Enum_type, nullptr, nullptr);
    if (newEnum == nullptr)
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

        ST::string enumValueName = ST::format("{}.{}", name, key);

        PyObject *newValue = NewEnumValue(enumValueName.c_str(), value);
        PyObject *valueObj = PyLong_FromLong((long)value);
        PyObject* keyObj = PyUnicode_FromStdString(key);
        PyDict_SetItem(valuesDict, valueObj, newValue);
        PyDict_SetItem(lookupDict, keyObj, newValue);
        PyDict_SetItem(reverseLookupDict, newValue, keyObj);
        Py_DECREF(keyObj);
        Py_DECREF(valueObj);
        Py_DECREF(newValue);
    }

    PyModule_AddObject(m, name, (PyObject*)newEnum);
}
