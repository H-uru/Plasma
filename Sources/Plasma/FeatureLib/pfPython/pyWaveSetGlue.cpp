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
#include "pyWaveSet.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptWaveSet, pyWaveSet);

PYTHON_DEFAULT_NEW_DEFINITION(ptWaveSet, pyWaveSet)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptWaveSet)

PYTHON_INIT_DEFINITION(ptWaveSet, args, keywords)
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

// some macros to make all this easier
// ----------------------------------------------------------------------------

#define WAVESET_FLOAT_DEF(funcSuffix) \
PYTHON_METHOD_DEFINITION(ptWaveSet, set##funcSuffix, args) \
{ \
	float s, secs = 0; \
	if (!PyArg_ParseTuple(args, "f|f", &s, &secs)) \
	{ \
		PyErr_SetString(PyExc_TypeError, "set" #funcSuffix " expects a float and an optional float"); \
		PYTHON_RETURN_ERROR; \
	} \
	self->fThis->Set##funcSuffix(s, secs); \
	PYTHON_RETURN_NONE; \
} \
PYTHON_METHOD_DEFINITION_NOARGS(ptWaveSet, get##funcSuffix) \
{ \
	return PyFloat_FromDouble(self->fThis->Get##funcSuffix()); \
}

#define WAVESET_FLOAT(funcSuffix) \
PYTHON_METHOD(ptWaveSet, set##funcSuffix, "Params: s, secs = 0\nSets the attribute to s over secs time"), \
PYTHON_METHOD_NOARGS(ptWaveSet, get##funcSuffix, "Returns the attribute's value")

#define WAVESET_OBJ_DEF(funcSuffix, pyObjType, cObjType) \
PYTHON_METHOD_DEFINITION(ptWaveSet, set##funcSuffix, args) \
{ \
	PyObject* sObj = NULL; \
	float secs = 0; \
	if (!PyArg_ParseTuple(args, "O|f", &sObj, &secs)) \
	{ \
		PyErr_SetString(PyExc_TypeError, "set" #funcSuffix " expects a " #pyObjType " and an optional float"); \
		PYTHON_RETURN_ERROR; \
	} \
	if (!cObjType::Check(sObj)) \
	{ \
		PyErr_SetString(PyExc_TypeError, "set" #funcSuffix " expects a " #pyObjType " and an optional float"); \
		PYTHON_RETURN_ERROR; \
	} \
	cObjType* s = cObjType::ConvertFrom(sObj); \
	self->fThis->Set##funcSuffix(*s, secs); \
	PYTHON_RETURN_NONE; \
} \
PYTHON_METHOD_DEFINITION_NOARGS(ptWaveSet, get##funcSuffix) \
{ \
	return self->fThis->Get##funcSuffix(); \
}

#define WAVESET_OBJ(funcSuffix) \
PYTHON_METHOD(ptWaveSet, set##funcSuffix, "Params: s, secs = 0\nSets the attribute to s over secs time"), \
PYTHON_METHOD_NOARGS(ptWaveSet, get##funcSuffix, "Returns the attribute's value")

// ----------------------------------------------------------------------------
// now onto the glue functions

WAVESET_FLOAT_DEF(GeoMaxLength)
WAVESET_FLOAT_DEF(GeoMinLength)
WAVESET_FLOAT_DEF(GeoAmpOverLen)
WAVESET_FLOAT_DEF(GeoChop)
WAVESET_FLOAT_DEF(GeoAngleDev)

WAVESET_FLOAT_DEF(TexMaxLength)
WAVESET_FLOAT_DEF(TexMinLength)
WAVESET_FLOAT_DEF(TexAmpOverLen)
WAVESET_FLOAT_DEF(TexChop)
WAVESET_FLOAT_DEF(TexAngleDev)

WAVESET_FLOAT_DEF(RippleScale)
WAVESET_OBJ_DEF(WindDir, ptVector3, pyVector3)

WAVESET_FLOAT_DEF(SpecularNoise)
WAVESET_FLOAT_DEF(SpecularStart)
WAVESET_FLOAT_DEF(SpecularEnd)

WAVESET_FLOAT_DEF(WaterHeight)

WAVESET_OBJ_DEF(WaterOffset, ptVector3, pyVector3)
WAVESET_FLOAT_DEF(OpacOffset)
WAVESET_FLOAT_DEF(ReflOffset)
WAVESET_FLOAT_DEF(WaveOffset)

WAVESET_OBJ_DEF(DepthFalloff, ptVector3, pyVector3)
WAVESET_FLOAT_DEF(OpacFalloff)
WAVESET_FLOAT_DEF(ReflFalloff)
WAVESET_FLOAT_DEF(WaveFalloff)

WAVESET_OBJ_DEF(MaxAtten, ptVector3, pyVector3)
WAVESET_OBJ_DEF(MinAtten, ptVector3, pyVector3)

WAVESET_OBJ_DEF(WaterTint, ptColor, pyColor)
WAVESET_FLOAT_DEF(WaterOpacity)
WAVESET_OBJ_DEF(SpecularTint, ptColor, pyColor)
WAVESET_FLOAT_DEF(SpecularMute)

WAVESET_OBJ_DEF(EnvCenter, ptPoint3, pyPoint3)
WAVESET_FLOAT_DEF(EnvRadius)

PYTHON_START_METHODS_TABLE(ptWaveSet)
	WAVESET_FLOAT(GeoMaxLength),
	WAVESET_FLOAT(GeoMinLength),
	WAVESET_FLOAT(GeoAmpOverLen),
	WAVESET_FLOAT(GeoChop),
	WAVESET_FLOAT(GeoAngleDev),

	WAVESET_FLOAT(TexMaxLength),
	WAVESET_FLOAT(TexMinLength),
	WAVESET_FLOAT(TexAmpOverLen),
	WAVESET_FLOAT(TexChop),
	WAVESET_FLOAT(TexAngleDev),

	WAVESET_FLOAT(RippleScale),
	WAVESET_OBJ(WindDir),

	WAVESET_FLOAT(SpecularNoise),
	WAVESET_FLOAT(SpecularStart),
	WAVESET_FLOAT(SpecularEnd),

	WAVESET_FLOAT(WaterHeight),

	WAVESET_OBJ(WaterOffset),
	WAVESET_FLOAT(OpacOffset),
	WAVESET_FLOAT(ReflOffset),
	WAVESET_FLOAT(WaveOffset),

	WAVESET_OBJ(DepthFalloff),
	WAVESET_FLOAT(OpacFalloff),
	WAVESET_FLOAT(ReflFalloff),
	WAVESET_FLOAT(WaveFalloff),

	WAVESET_OBJ(MaxAtten),
	WAVESET_OBJ(MinAtten),

	WAVESET_OBJ(WaterTint),
	WAVESET_FLOAT(WaterOpacity),
	WAVESET_OBJ(SpecularTint),
	WAVESET_FLOAT(SpecularMute),

	WAVESET_OBJ(EnvCenter),
	WAVESET_FLOAT(EnvRadius),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptWaveSet, "Params:key\nCreates a new ptWaveSet");

// required functions for PyObject interoperability
PyObject *pyWaveSet::New(plKey key)
{
	ptWaveSet *newObj = (ptWaveSet*)ptWaveSet_type.tp_new(&ptWaveSet_type, NULL, NULL);
	newObj->fThis->fWaterKey = key;
	return (PyObject*)newObj;
}

PyObject *pyWaveSet::New(pyKey &key)
{
	ptWaveSet *newObj = (ptWaveSet*)ptWaveSet_type.tp_new(&ptWaveSet_type, NULL, NULL);
	newObj->fThis->fWaterKey = key.getKey();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptWaveSet, pyWaveSet)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptWaveSet, pyWaveSet)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyWaveSet::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptWaveSet);
	PYTHON_CLASS_IMPORT_END(m);
}