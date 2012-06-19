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

#include <Python.h>
#include "pyKey.h"
#pragma hdrstop

#include "pyGrassShader.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptGrassShader, pyGrassShader);

PYTHON_DEFAULT_NEW_DEFINITION(ptGrassShader, pyGrassShader)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGrassShader)

PYTHON_INIT_DEFINITION(ptGrassShader, args, keywords)
{
    PyObject *keyObject = NULL;
    if (!PyArg_ParseTuple(args, "O", &keyObject))
    {
        PyErr_SetString(PyExc_TypeError, "init expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    if (!pyKey::Check(keyObject))
    {
        PyErr_SetString(PyExc_TypeError, "init expects a ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }

    pyKey *key = pyKey::ConvertFrom(keyObject);
    self->fThis->SetKey(key->getKey());

    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptGrassShader, setWaveDistortion, args)
{
    int waveNum;
    PyObject *tupleObject = NULL;
    if (!PyArg_ParseTuple(args, "iO", &waveNum, &tupleObject))
    {
        PyErr_SetString(PyExc_TypeError, "setWaveDistortion expects a integer and tuple of floats");
        PYTHON_RETURN_ERROR;
    }
    if (!PyTuple_Check(tupleObject))
    {
        PyErr_SetString(PyExc_TypeError, "setWaveDistortion expects a integer and tuple of floats");
        PYTHON_RETURN_ERROR;
    }

    int len = PyTuple_Size(tupleObject);
    std::vector<float> vecArgs;
    for (int curArg = 0; curArg < len; curArg++)
    {
        PyObject *arg = PyTuple_GetItem(tupleObject, curArg);
        if (!PyFloat_Check(arg))
        {
            PyErr_SetString(PyExc_TypeError, "setWaveDistortion expects a integer and tuple of floats");
            PYTHON_RETURN_ERROR;
        }
        vecArgs.push_back((float)PyFloat_AsDouble(arg));
    }

    self->fThis->SetWaveDistortion(waveNum, vecArgs);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGrassShader, setWaveDirection, args)
{
    int waveNum;
    PyObject *tupleObject = NULL;
    if (!PyArg_ParseTuple(args, "iO", &waveNum, &tupleObject))
    {
        PyErr_SetString(PyExc_TypeError, "setWaveDirection expects a integer and tuple of floats");
        PYTHON_RETURN_ERROR;
    }
    if (!PyTuple_Check(tupleObject))
    {
        PyErr_SetString(PyExc_TypeError, "setWaveDirection expects a integer and tuple of floats");
        PYTHON_RETURN_ERROR;
    }

    int len = PyTuple_Size(tupleObject);
    std::vector<float> vecArgs;
    for (int curArg = 0; curArg < len; curArg++)
    {
        PyObject *arg = PyTuple_GetItem(tupleObject, curArg);
        if (!PyFloat_Check(arg))
        {
            PyErr_SetString(PyExc_TypeError, "setWaveDirection expects a integer and tuple of floats");
            PYTHON_RETURN_ERROR;
        }
        vecArgs.push_back((float)PyFloat_AsDouble(arg));
    }

    self->fThis->SetWaveDirection(waveNum, vecArgs);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGrassShader, setWaveSpeed, args)
{
    int waveNum;
    float speed;
    if (!PyArg_ParseTuple(args, "if", &waveNum, &speed))
    {
        PyErr_SetString(PyExc_TypeError, "setWaveSpeed expects an integer and a float");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetWaveSpeed(waveNum, speed);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptGrassShader, getWaveDistortion, args)
{
    int waveNum;
    if (!PyArg_ParseTuple(args, "i", &waveNum))
    {
        PyErr_SetString(PyExc_TypeError, "getWaveDistortion expects an integer");
        PYTHON_RETURN_ERROR;
    }

    std::vector<float> vecArgs = self->fThis->GetWaveDistortion(waveNum);
    PyObject *retVal = PyTuple_New(vecArgs.size());
    for (int curArg = 0; curArg < vecArgs.size(); curArg++)
        PyTuple_SetItem(retVal, curArg, PyFloat_FromDouble((double)vecArgs[curArg]));
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptGrassShader, getWaveDirection, args)
{
    int waveNum;
    if (!PyArg_ParseTuple(args, "i", &waveNum))
    {
        PyErr_SetString(PyExc_TypeError, "getWaveDirection expects an integer");
        PYTHON_RETURN_ERROR;
    }

    std::vector<float> vecArgs = self->fThis->GetWaveDirection(waveNum);
    PyObject *retVal = PyTuple_New(vecArgs.size());
    for (int curArg = 0; curArg < vecArgs.size(); curArg++)
        PyTuple_SetItem(retVal, curArg, PyFloat_FromDouble((double)vecArgs[curArg]));
    return retVal;
}

PYTHON_METHOD_DEFINITION(ptGrassShader, getWaveSpeed, args)
{
    int waveNum;
    if (!PyArg_ParseTuple(args, "i", &waveNum))
    {
        PyErr_SetString(PyExc_TypeError, "getWaveDirection expects an integer");
        PYTHON_RETURN_ERROR;
    }
    return PyFloat_FromDouble((double)self->fThis->GetWaveSpeed(waveNum));
}

PYTHON_BASIC_METHOD_DEFINITION(ptGrassShader, resetWaves, ResetWaves)

PYTHON_START_METHODS_TABLE(ptGrassShader)
    PYTHON_METHOD(ptGrassShader, setWaveDistortion, "Params: waveNum, distortion\nSets the wave waveNum's distortion as a tuple of x,y,z. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"),
    PYTHON_METHOD(ptGrassShader, setWaveDirection, "Params: waveNum, direction\nSets the wave waveNum's direction as a tuple of x,y. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"),
    PYTHON_METHOD(ptGrassShader, setWaveSpeed, "Params: waveNum, speed\nSets the wave waveNum's speed as a float. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"),

    PYTHON_METHOD(ptGrassShader, getWaveDistortion, "Params: waveNum\nGets the wave waveNum's distortion as a tuple of x,y,z. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"),
    PYTHON_METHOD(ptGrassShader, getWaveDirection, "Params: waveNum\nGets the wave waveNum's direction as a tuple of x,y. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"),
    PYTHON_METHOD(ptGrassShader, getWaveSpeed, "Params: waveNum\nGets the wave waveNum's speed as a float. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"),

    PYTHON_BASIC_METHOD(ptGrassShader, resetWaves, "Resets wave data to 0"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGrassShader, "Params: key\nPlasma Grass Shader class");

// required functions for PyObject interoperability
PyObject *pyGrassShader::New(plKey key)
{
    ptGrassShader *newObj = (ptGrassShader*)ptGrassShader_type.tp_new(&ptGrassShader_type, NULL, NULL);
    newObj->fThis->SetKey(key);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGrassShader, pyGrassShader)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGrassShader, pyGrassShader)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGrassShader::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGrassShader);
    PYTHON_CLASS_IMPORT_END(m);
}
