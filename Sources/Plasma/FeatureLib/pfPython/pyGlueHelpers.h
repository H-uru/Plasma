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
#ifndef _pyGlueHelpers_h_
#define _pyGlueHelpers_h_

namespace ST { class string; }
class pyKey;
typedef struct _object PyObject;
typedef struct _typeobject PyTypeObject;
typedef struct PyMethodDef PyMethodDef;

// Useful string functions
ST::string PyUnicode_AsSTString(PyObject* obj);
int PyUnicode_STStringConverter(PyObject* obj, void* str);
int PyUnicode_PlFileNameDecoder(PyObject* obj, void* str);

PyObject* PyUnicode_FromSTString(const ST::string& str);
#define PyUnicode_FromSTString(x) PyUnicode_FromStringAndSize((x).c_str(), (x).size())
#define PyUnicode_FromStdString(x) PyUnicode_FromStringAndSize((x).c_str(), (x).size())

// A set of macros to take at least some of the tediousness out of creating straight python glue code

/////////////////////////////////////////////////////////////////////
// Python class definition macros
/////////////////////////////////////////////////////////////////////

// This defines the basic PyObject we need for python classes
#define PYTHON_CLASS_DEFINITION(pythonClassName, glueClassName) \
struct pythonClassName \
{ \
    PyObject_HEAD \
    glueClassName *fThis; \
};

// This makes sure that our python new function can access our constructors
#define PYTHON_CLASS_NEW_FRIEND(pythonClassName) friend PyObject *pythonClassName##_new(PyTypeObject *type, PyObject *args, PyObject *keywords)

#define PYTHON_CLASS_VAULT_NODE_NEW_DEFINITION \
    static PyObject* New(hsRef<RelVaultNode> vaultNode=nullptr);

// This defines the basic new function for a class
#define PYTHON_CLASS_NEW_DEFINITION static PyObject *New()

#define PYTHON_CLASS_NEW_IMPL(pythonClassName, glueClassName) \
PyObject *glueClassName::New() \
{ \
    pythonClassName *newObj = (pythonClassName*)pythonClassName##_type.tp_new(&pythonClassName##_type, nullptr, nullptr); \
    return (PyObject*)newObj; \
}

#define PYTHON_CLASS_VAULT_NODE_NEW_IMPL(pythonClassName, glueClassName) \
PyObject* glueClassName::New(hsRef<RelVaultNode> nfsNode) \
{ \
    pythonClassName* newObj = (pythonClassName*)pythonClassName##_type.tp_new(&pythonClassName##_type, nullptr, nullptr); \
    if (nfsNode) \
        newObj->fThis->fNode = std::move(nfsNode); \
    return (PyObject*)newObj; \
}

// This defines the basic check function for a class
#define PYTHON_CLASS_CHECK_DEFINITION static bool Check(PyObject *obj)

#define PYTHON_CLASS_CHECK_IMPL(pythonClassName, glueClassName) \
bool glueClassName::Check(PyObject *obj) \
{ \
    return PyObject_TypeCheck(obj, &pythonClassName##_type); \
}

// This defines the basic convert from function for a class
#define PYTHON_CLASS_CONVERT_FROM_DEFINITION(glueClassName) static glueClassName *ConvertFrom(PyObject *obj)

#define PYTHON_CLASS_CONVERT_FROM_IMPL(pythonClassName, glueClassName) \
glueClassName *glueClassName::ConvertFrom(PyObject *obj) \
{ \
    if (!Check(obj)) \
    { \
        PyErr_SetString(PyExc_TypeError, "object is not a " #pythonClassName); \
        return nullptr; \
    } \
    return ((pythonClassName*)obj)->fThis; \
}

/////////////////////////////////////////////////////////////////////
// Python type definition macros
/////////////////////////////////////////////////////////////////////

// This starts off the type definition (however most of the data still needs to be filled in by hand
#define PYTHON_TYPE_START(pythonClassName) \
static PyTypeObject pythonClassName##_type = { \
    PyVarObject_HEAD_INIT(nullptr, 0)

// and easy terminator to make things look pretty
#define PYTHON_TYPE_END }

// the default new function definition
#define PYTHON_DEFAULT_NEW_DEFINITION(pythonClassName, glueClassName) \
PyObject *pythonClassName##_new(PyTypeObject *type, PyObject *, PyObject *) \
{ \
    pythonClassName *self; \
    self = (pythonClassName*)type->tp_alloc(type, 0); \
    if (self != nullptr) \
    { \
        self->fThis = new glueClassName(); \
        if (self->fThis == nullptr) \
        { \
            Py_DECREF(self); \
            return nullptr; \
        } \
    } \
\
    return (PyObject*)self; \
}

#define PYTHON_DEFAULT_NEW(pythonClassName) pythonClassName##_new

// the default dealloc function definition
#define PYTHON_DEFAULT_DEALLOC_DEFINITION(pythonClassName) \
void pythonClassName##_dealloc(pythonClassName *self) \
{ \
    delete self->fThis; \
    Py_TYPE(self)->tp_free((PyObject*)self); \
}

#define PYTHON_DEFAULT_DEALLOC(pythonClassName) (destructor)pythonClassName##_dealloc

// init function defines
#define PYTHON_RETURN_INIT_ERROR return -1;
#define PYTHON_RETURN_INIT_OK return 0;

// basic no init function stuff, for when you don't want the class to be created by python
#define PYTHON_NO_INIT_DEFINITION(pythonClassName) \
int pythonClassName##___init__(pythonClassName *, PyObject *, PyObject *) \
{ \
    PyErr_SetString(PyExc_RuntimeError, "Cannot create " #pythonClassName " objects from Python"); \
    PYTHON_RETURN_INIT_ERROR; \
}

#define PYTHON_INIT_DEFINITION(pythonClassName, argsVar, keywordsVar) \
int pythonClassName##___init__(pythonClassName *self, PyObject *args, PyObject *keywords)

#define PYTHON_DEFAULT_INIT(pythonClassName) (initproc)pythonClassName##___init__

// message table default name
#define PYTHON_DEFAULT_METHODS_TABLE(pythonClassName) pythonClassName##_methods

// tp_print moved to the end, and two new vectorcall fields inserted in Python 3.8...
// tp_print gone in Python 3.9...
#if PY_VERSION_HEX >= 0x03080000
#   define PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET 0
#   if PY_VERSION_HEX >= 0x03090000
#       define PYTHON_TP_VECTORCALL_PRINT nullptr,
#   else
#       define PYTHON_TP_VECTORCALL_PRINT nullptr, nullptr,
#   endif
#else
#   define PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET nullptr
#   define PYTHON_TP_VECTORCALL_PRINT
#endif

// most glue classes can get away with this default structure
#define PLASMA_DEFAULT_TYPE(pythonClassName, docString) \
PYTHON_TYPE_START(pythonClassName) \
    "Plasma." #pythonClassName,         /* tp_name */ \
    sizeof(pythonClassName),            /* tp_basicsize */ \
    0,                                  /* tp_itemsize */ \
    PYTHON_DEFAULT_DEALLOC(pythonClassName),    /* tp_dealloc */ \
    PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET, \
    nullptr,                            /* tp_getattr */ \
    nullptr,                            /* tp_setattr */ \
    nullptr,                            /* tp_as_async */ \
    nullptr,                            /* tp_repr */ \
    nullptr,                            /* tp_as_number */ \
    nullptr,                            /* tp_as_sequence */ \
    nullptr,                            /* tp_as_mapping */ \
    nullptr,                            /* tp_hash */ \
    nullptr,                            /* tp_call */ \
    nullptr,                            /* tp_str */ \
    nullptr,                            /* tp_getattro */ \
    nullptr,                            /* tp_setattro */ \
    nullptr,                            /* tp_as_buffer */ \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */ \
    docString,                          /* tp_doc */ \
    nullptr,                            /* tp_traverse */ \
    nullptr,                            /* tp_clear */ \
    nullptr,                            /* tp_richcompare */ \
    0,                                  /* tp_weaklistoffset */ \
    nullptr,                            /* tp_iter */ \
    nullptr,                            /* tp_iternext */ \
    PYTHON_DEFAULT_METHODS_TABLE(pythonClassName),  /* tp_methods */ \
    nullptr,                            /* tp_members */ \
    nullptr,                            /* tp_getset */ \
    nullptr,                            /* tp_base */ \
    nullptr,                            /* tp_dict */ \
    nullptr,                            /* tp_descr_get */ \
    nullptr,                            /* tp_descr_set */ \
    0,                                  /* tp_dictoffset */ \
    PYTHON_DEFAULT_INIT(pythonClassName),   /* tp_init */ \
    nullptr,                            /* tp_alloc */ \
    PYTHON_DEFAULT_NEW(pythonClassName),/* tp_new */ \
    nullptr,                            /* tp_free */ \
    nullptr,                            /* tp_is_gc */ \
    nullptr,                            /* tp_bases */ \
    nullptr,                            /* tp_mro */ \
    nullptr,                            /* tp_cache */ \
    nullptr,                            /* tp_subclasses */ \
    nullptr,                            /* tp_weaklist */ \
    nullptr,                            /* tp_del */ \
    0,                                  /* tp_version_tag */ \
    nullptr,                            /* tp_finalize */ \
    PYTHON_TP_VECTORCALL_PRINT \
PYTHON_TYPE_END

// default rich compare function name
#define PYTHON_DEFAULT_RICH_COMPARE(pythonClassName) pythonClassName##_richCompare
#define PYTHON_NO_RICH_COMPARE nullptr

// default as_ table names
#define PYTHON_DEFAULT_AS_NUMBER(pythonClassName) &pythonClassName##_as_number
#define PYTHON_NO_AS_NUMBER nullptr
#define PYTHON_DEFAULT_AS_SEQUENCE(pythonClassName) &pythonClassName##_as_sequence
#define PYTHON_NO_AS_SEQUENCE nullptr
#define PYTHON_DEFAULT_AS_MAPPING(pythonClassName) &pythonClassName##_as_mapping
#define PYTHON_NO_AS_MAPPING nullptr

// str function
#define PYTHON_DEFAULT_STR(pythonClassName) (reprfunc)pythonClassName##_str
#define PYTHON_NO_STR nullptr

// get/set attro
#define PYTHON_DEFAULT_GETATTRO PyObject_GenericGetAttr
#define PYTHON_NO_GETATTRO nullptr
#define PYTHON_DEFAULT_SETATTRO PyObject_GenericSetAttr
#define PYTHON_NO_SETATTRO nullptr

// get/set table default name
#define PYTHON_DEFAULT_GETSET(pythonClassName) pythonClassName##_getseters
#define PYTHON_NO_GETSET nullptr

// default base pointer
#define PYTHON_DEFAULT_BASE_TYPE(glueBaseClass) glueBaseClass::type_ptr
#define PYTHON_NO_BASE nullptr

// for glue functions that need custom stuff, you need to define the macros yourself
#define PLASMA_CUSTOM_TYPE(pythonClassName, docString) \
    PYTHON_TYPE_START(pythonClassName) \
    "Plasma." #pythonClassName,         /* tp_name */ \
    sizeof(pythonClassName),            /* tp_basicsize */ \
    0,                                  /* tp_itemsize */ \
    PYTHON_DEFAULT_DEALLOC(pythonClassName),    /* tp_dealloc */ \
    PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET, \
    nullptr,                            /* tp_getattr */ \
    nullptr,                            /* tp_setattr */ \
    nullptr,                            /* tp_as_async */ \
    nullptr,                            /* tp_repr */ \
    pythonClassName##_AS_NUMBER,        /* tp_as_number */ \
    pythonClassName##_AS_SEQUENCE,      /* tp_as_sequence */ \
    pythonClassName##_AS_MAPPING,       /* tp_as_mapping */ \
    nullptr,                            /* tp_hash */ \
    nullptr,                            /* tp_call */ \
    pythonClassName##_STR,              /* tp_str */ \
    pythonClassName##_GETATTRO,         /* tp_getattro */ \
    pythonClassName##_SETATTRO,         /* tp_setattro */ \
    nullptr,                            /* tp_as_buffer */ \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */ \
    docString,                          /* tp_doc */ \
    nullptr,                            /* tp_traverse */ \
    nullptr,                            /* tp_clear */ \
    pythonClassName##_RICH_COMPARE,     /* tp_richcompare */ \
    0,                                  /* tp_weaklistoffset */ \
    nullptr,                            /* tp_iter */ \
    nullptr,                            /* tp_iternext */ \
    PYTHON_DEFAULT_METHODS_TABLE(pythonClassName),  /* tp_methods */ \
    nullptr,                            /* tp_members */ \
    pythonClassName##_GETSET,           /* tp_getset */ \
    pythonClassName##_BASE,             /* tp_base */ \
    nullptr,                            /* tp_dict */ \
    nullptr,                            /* tp_descr_get */ \
    nullptr,                            /* tp_descr_set */ \
    0,                                  /* tp_dictoffset */ \
    PYTHON_DEFAULT_INIT(pythonClassName),   /* tp_init */ \
    nullptr,                            /* tp_alloc */ \
    PYTHON_DEFAULT_NEW(pythonClassName),/* tp_new */ \
    nullptr,                            /* tp_free */ \
    nullptr,                            /* tp_is_gc */ \
    nullptr,                            /* tp_bases */ \
    nullptr,                            /* tp_mro */ \
    nullptr,                            /* tp_cache */ \
    nullptr,                            /* tp_subclasses */ \
    nullptr,                            /* tp_weaklist */ \
    nullptr,                            /* tp_del */ \
    0,                                  /* tp_version_tag */ \
    nullptr,                            /* tp_finalize */ \
    PYTHON_TP_VECTORCALL_PRINT \
PYTHON_TYPE_END

// for conviencence when we just need a base class
#define PLASMA_DEFAULT_TYPE_WBASE(pythonClassName, glueBaseClass, docString) \
PYTHON_TYPE_START(pythonClassName) \
    "Plasma." #pythonClassName,         /* tp_name */ \
    sizeof(pythonClassName),            /* tp_basicsize */ \
    0,                                  /* tp_itemsize */ \
    PYTHON_DEFAULT_DEALLOC(pythonClassName),    /* tp_dealloc */ \
    PYTHON_TP_PRINT_OR_VECTORCALL_OFFSET, \
    nullptr,                            /* tp_getattr */ \
    nullptr,                            /* tp_setattr */ \
    nullptr,                            /* tp_compare */ \
    nullptr,                            /* tp_repr */ \
    nullptr,                            /* tp_as_number */ \
    nullptr,                            /* tp_as_sequence */ \
    nullptr,                            /* tp_as_mapping */ \
    nullptr,                            /* tp_hash */ \
    nullptr,                            /* tp_call */ \
    nullptr,                            /* tp_str */ \
    nullptr,                            /* tp_getattro */ \
    nullptr,                            /* tp_setattro */ \
    nullptr,                            /* tp_as_buffer */ \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */ \
    docString,                          /* tp_doc */ \
    nullptr,                            /* tp_traverse */ \
    nullptr,                            /* tp_clear */ \
    nullptr,                            /* tp_richcompare */ \
    0,                                  /* tp_weaklistoffset */ \
    nullptr,                            /* tp_iter */ \
    nullptr,                            /* tp_iternext */ \
    PYTHON_DEFAULT_METHODS_TABLE(pythonClassName),  /* tp_methods */ \
    nullptr,                            /* tp_members */ \
    nullptr,                            /* tp_getset */ \
    glueBaseClass::type_ptr,            /* tp_base */ \
    nullptr,                            /* tp_dict */ \
    nullptr,                            /* tp_descr_get */ \
    nullptr,                            /* tp_descr_set */ \
    0,                                  /* tp_dictoffset */ \
    PYTHON_DEFAULT_INIT(pythonClassName),   /* tp_init */ \
    nullptr,                            /* tp_alloc */ \
    PYTHON_DEFAULT_NEW(pythonClassName),/* tp_new */ \
    nullptr,                            /* tp_free */ \
    nullptr,                            /* tp_is_gc */ \
    nullptr,                            /* tp_bases */ \
    nullptr,                            /* tp_mro */ \
    nullptr,                            /* tp_cache */ \
    nullptr,                            /* tp_subclasses */ \
    nullptr,                            /* tp_weaklist */ \
    nullptr,                            /* tp_del */ \
    0,                                  /* tp_version_tag */ \
    nullptr,                            /* tp_finalize */ \
    PYTHON_TP_VECTORCALL_PRINT \
PYTHON_TYPE_END

// small macros so that the type object can be accessed outside the glue file (for subclassing)
#define PYTHON_EXPOSE_TYPE static PyTypeObject* type_ptr
#define PYTHON_EXPOSE_TYPE_DEFINITION(pythonClass, glueClass) PyTypeObject* glueClass::type_ptr = &pythonClass##_type

/////////////////////////////////////////////////////////////////////
// Python class import macros
/////////////////////////////////////////////////////////////////////

// called at the beginning of the class definition function to grab the module
#define PYTHON_CLASS_IMPORT_START(m) \
if (m == nullptr) \
    return; \
\
Py_INCREF(m)

// called at the end of the class definition function to release the module
#define PYTHON_CLASS_IMPORT_END(m) Py_DECREF(m)

// called for each class you want to add to the module
#define PYTHON_CLASS_IMPORT(m, pythonClassName) \
if (PyType_Ready(&pythonClassName##_type) < 0) \
    return; \
    \
Py_INCREF(&pythonClassName##_type); \
PyModule_AddObject(m, #pythonClassName, (PyObject*)&pythonClassName##_type)

/////////////////////////////////////////////////////////////////////
// Python method macros
/////////////////////////////////////////////////////////////////////

// Handles the three types of methods python uses
#define PYTHON_METHOD_DEFINITION(pythonClassName, methodName, argsVar) \
    static PyObject *pythonClassName##_##methodName(pythonClassName *self, PyObject *argsVar)
#define PYTHON_METHOD_DEFINITION_NOARGS(pythonClassName, methodName) \
    static PyObject *pythonClassName##_##methodName(pythonClassName *self)
#define PYTHON_METHOD_DEFINITION_STATIC(pythonClassName, methodName, argsVar) \
    static PyObject *pythonClassName##_##methodName(PyObject*, PyObject *argsVar)
#define PYTHON_METHOD_DEFINITION_STATIC_WKEY(pythonClassName, methodName, argsVar, keywordsVar) \
    static PyObject *pythonClassName##_##methodName(PyObject*, PyObject *argsVar, PyObject *keywordsVar)
#define PYTHON_METHOD_DEFINITION_WKEY(pythonClassName, methodName, argsVar, keywordsVar) \
    static PyObject *pythonClassName##_##methodName(pythonClassName *self, PyObject *argsVar, PyObject *keywordsVar)

// A very basic function, just calls the class method and returns None
#define PYTHON_BASIC_METHOD_DEFINITION(pythonClassName, methodName, classMethodName) \
static PyObject *pythonClassName##_##methodName(pythonClassName *self) \
{ \
    self->fThis->classMethodName(); \
    PYTHON_RETURN_NONE; \
}

// Different basic return types
#define PYTHON_RETURN_ERROR { return nullptr; }
#define PYTHON_RETURN_NONE {Py_INCREF(Py_None); return Py_None;}
#define PYTHON_RETURN_BOOL(testValue)                     \
{                                                         \
    PyObject* retVal = (testValue) ? Py_True : Py_False;  \
    Py_INCREF(retVal);                                    \
    return retVal;                                        \
}                                                         //
#define PYTHON_RETURN_NOT_IMPLEMENTED {Py_INCREF(Py_NotImplemented); return Py_NotImplemented;}

// method table start
#define PYTHON_START_METHODS_TABLE(pythonClassName) static PyMethodDef pythonClassName##_methods[] = {

// method table end (automatically adds sentinal value)
#define PYTHON_END_METHODS_TABLE { nullptr } }

// basic method
#define PYTHON_METHOD(pythonClassName, methodName, docString) \
    {#methodName, (PyCFunction)pythonClassName##_##methodName, METH_VARARGS, docString}

// method with no arguments
#define PYTHON_METHOD_NOARGS(pythonClassName, methodName, docString) \
    {#methodName, (PyCFunction)pythonClassName##_##methodName, METH_NOARGS, docString}

// static method with arguments
#define PYTHON_METHOD_STATIC(pythonClassName, methodName, docString) \
    {#methodName, (PyCFunction)pythonClassName##_##methodName, METH_STATIC | METH_VARARGS, docString}

// static method with keywords
#define PYTHON_METHOD_STATIC_WKEY(pythonClassName, methodName, docString) \
    {#methodName, (PyCFunction)pythonClassName##_##methodName, METH_STATIC | METH_VARARGS | METH_KEYWORDS, docString}

// method with keywords
#define PYTHON_METHOD_WKEY(pythonClassName, methodName, docString) \
    {#methodName, (PyCFunction)pythonClassName##_##methodName, METH_VARARGS | METH_KEYWORDS, docString}

// really basic method
#define PYTHON_BASIC_METHOD(pythonClassName, methodName, docString) \
    {#methodName, (PyCFunction)pythonClassName##_##methodName, METH_NOARGS, docString}

/////////////////////////////////////////////////////////////////////
// Get/set macros
/////////////////////////////////////////////////////////////////////

// setter defines
#define PYTHON_RETURN_SET_ERROR return -1;
#define PYTHON_RETURN_SET_OK return 0;

// getter function definition
#define PYTHON_GET_DEFINITION(pythonClassName, attribName) \
    static PyObject *pythonClassName##_get##attribName(pythonClassName *self, void *closure)

// setter function definition
#define PYTHON_SET_DEFINITION(pythonClassName, attribName, valueVarName) \
    static int pythonClassName##_set##attribName(pythonClassName *self, PyObject *valueVarName, void *closure)

// read-only setter function
#define PYTHON_SET_DEFINITION_READONLY(pythonClassName, attribName) \
int pythonClassName##_set##attribName(PyObject *self, PyObject *value, void *closure) \
{ \
    PyErr_SetString(PyExc_RuntimeError, #attribName " is read-only"); \
    PYTHON_RETURN_SET_ERROR; \
}

// starts off the get/set table
#define PYTHON_START_GETSET_TABLE(pythonClassName) static PyGetSetDef pythonClassName##_getseters[] = {

// and easy terminator to make things look pretty (automatically adds sentinel value)
#define PYTHON_END_GETSET_TABLE { nullptr } }

// the get/set definition
#define PYTHON_GETSET(pythonClassName, attribName, docString) {#attribName, \
    (getter)pythonClassName##_get##attribName, (setter)pythonClassName##_set##attribName, \
    docString, nullptr }

/////////////////////////////////////////////////////////////////////
// as_ table macros
/////////////////////////////////////////////////////////////////////

// as_number table
#define PYTHON_START_AS_NUMBER_TABLE(pythonClassName) static PyNumberMethods pythonClassName##_as_number = {
#define PYTHON_END_AS_NUMBER_TABLE }

// as_sequence table
#define PYTHON_START_AS_SEQUENCE_TABLE(pythonClassName) static PySequenceMethods pythonClassName##_as_sequence = {
#define PYTHON_END_AS_SEQUENCE_TABLE }

// as_mapping table
#define PYTHON_START_AS_MAPPING_TABLE(pythonClassName) static PyMappingMethods pythonClassName##_as_mapping = {
#define PYTHON_END_AS_MAPPING_TABLE }

/////////////////////////////////////////////////////////////////////
// Compare/Rich compare functions
/////////////////////////////////////////////////////////////////////

// rich compare
#define PYTHON_RICH_COMPARE_DEFINITION(pythonClassName, obj1, obj2, compareType) PyObject *pythonClassName##_richCompare(PyObject *obj1, PyObject *obj2, int compareType)
#define PYTHON_RCOMPARE_TRUE return PyLong_FromLong((long)1)
#define PYTHON_RCOMPARE_FALSE return PyLong_FromLong((long)0)
#define PYTHON_RCOMPARE_ERROR return PyLong_FromLong((long)-1)

/////////////////////////////////////////////////////////////////////
// str functions
/////////////////////////////////////////////////////////////////////

// str function
#define PYTHON_STR_DEFINITION(pythonClassName) PyObject *pythonClassName##_str(pythonClassName *self)

/////////////////////////////////////////////////////////////////////
// Global function macros (for those functions that aren't in a class)
/////////////////////////////////////////////////////////////////////

// Global method
#define PYTHON_GLOBAL_METHOD_DEFINITION(methodName, argsVar, docString) \
static PyObject *methodName(PyObject*, PyObject*); /* forward declaration so struct can go here */ \
static PyMethodDef methodName##_method = {#methodName, methodName, METH_VARARGS, docString}; \
static PyObject *methodName(PyObject *self, PyObject *argsVar) /* and now for the actual function */

// Global method with no arguments
#define PYTHON_GLOBAL_METHOD_DEFINITION_NOARGS(methodName, docString) \
static PyObject *methodName(PyObject*); /* forward declaration so struct can go here */ \
static PyMethodDef methodName##_method = {#methodName, (PyCFunction)methodName, METH_NOARGS, docString}; \
static PyObject *methodName(PyObject *self) /* and now for the actual function */

// Global method with keywords
#define PYTHON_GLOBAL_METHOD_DEFINITION_WKEY(methodName, argsVar, keywordsVar, docString) \
static PyObject *methodName(PyObject*, PyObject*, PyObject*); /* forward declaration so struct can go here */ \
static PyMethodDef methodName##_method = {#methodName, (PyCFunction)methodName, METH_VARARGS | METH_KEYWORDS, docString}; \
static PyObject *methodName(PyObject *self, PyObject *argsVar, PyObject *keywordsVar) /* and now for the actual function */

// Basic global method
#define PYTHON_BASIC_GLOBAL_METHOD_DEFINITION(methodName, classMethodName, docString) \
static PyObject *methodName(PyObject*); /* forward declaration so struct can go here */ \
static PyMethodDef methodName##_method = {#methodName, (PyCFunction)methodName, METH_NOARGS, docString}; \
static PyObject *methodName(PyObject *self) /* and now for the actual function */ \
{ \
    classMethodName(); \
    PYTHON_RETURN_NONE; \
}

#define PYTHON_START_GLOBAL_METHOD_TABLE(name) \
    { \
        static PyMethodDef name##_globalMethods[] = {

// this goes in the definition function
#define PYTHON_GLOBAL_METHOD(methodName) methodName##_method,

// not necessary, but for continuity with the NOARGS function definition above
#define PYTHON_GLOBAL_METHOD_NOARGS(methodName) methodName##_method,

// not necessary, but for continuity with the WKEY function definition above
#define PYTHON_GLOBAL_METHOD_WKEY(methodName) methodName##_method,

// not necessary, but for continuity with the BASIC function definition above
#define PYTHON_BASIC_GLOBAL_METHOD(methodName) methodName##_method,

#define PYTHON_END_GLOBAL_METHOD_TABLE(moduleVarName, name) \
            { nullptr, nullptr, 0, nullptr } \
        }; \
        if (PyModule_AddFunctions(moduleVarName, name##_globalMethods) < 0) \
            return; \
    }

/////////////////////////////////////////////////////////////////////
// Enum glue (these should all be inside a function)
/////////////////////////////////////////////////////////////////////

// the start of an enum block
#define PYTHON_ENUM_START(enumName) std::vector<std::tuple<ST::string, Py_ssize_t>> enumName##_enumValues{

// for each element of the enum
#define PYTHON_ENUM_ELEMENT(enumName, elementName, elementValue) std::make_tuple(ST_LITERAL(#elementName), (Py_ssize_t)elementValue),

// to finish off and define the enum
#define PYTHON_ENUM_END(m, enumName) }; pyEnum::MakeEnum(m, #enumName, enumName##_enumValues);

#endif // _pyGlueHelpers_h_
