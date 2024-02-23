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

#include "pyGlueHelpers.h"
#include "pyKey.h"

#include "cyParticleSys.h"


// glue functions
PYTHON_CLASS_DEFINITION(ptParticle, cyParticleSys);

PYTHON_DEFAULT_NEW_DEFINITION(ptParticle, cyParticleSys)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptParticle)

PYTHON_NO_INIT_DEFINITION(ptParticle)

PYTHON_METHOD_DEFINITION(ptParticle, netForce, args)
{
    char forceFlag;
    if (!PyArg_ParseTuple(args, "b", &forceFlag))
    {
        PyErr_SetString(PyExc_TypeError, "netForce requires a boolean argument");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetNetForce(forceFlag != 0);
    PYTHON_RETURN_NONE;
}

// define a nice little marco to do the grunt work, since all the functions are identical
#define PARTICLE_FUNC(funcName, classFunc) \
    PYTHON_METHOD_DEFINITION(ptParticle, funcName, args) \
    { \
        float val; \
        if (!PyArg_ParseTuple(args, "f", &val)) \
        { \
            PyErr_SetString(PyExc_TypeError, #funcName " expects a float"); \
            PYTHON_RETURN_ERROR; \
        } \
        self->fThis->classFunc(val); \
        PYTHON_RETURN_NONE; \
    }

// now make all the functions using the above macro
PARTICLE_FUNC(setParticlesPerSecond, SetParticlesPerSecond)
PARTICLE_FUNC(setInitPitchRange, SetInitPitchRange)
PARTICLE_FUNC(setInitYawRange, SetInitYawRange)
PARTICLE_FUNC(setVelocityMinimum, SetVelMin)
PARTICLE_FUNC(setVelocityMaximum, SetVelMax)
PARTICLE_FUNC(setWidthSize, SetXSize)
PARTICLE_FUNC(setHeightSize, SetYSize)
PARTICLE_FUNC(setScaleMinimum, SetScaleMin)
PARTICLE_FUNC(setScaleMaximum, SetScaleMax)
PARTICLE_FUNC(setGeneratorLife, SetGenLife)
PARTICLE_FUNC(setParticleLifeMinimum, SetPartLifeMin)
PARTICLE_FUNC(setParticleLifeMaximum, SetPartLifeMax)

PYTHON_START_METHODS_TABLE(ptParticle)
    PYTHON_METHOD(ptParticle, netForce, "Params: forceFlag\nSpecify whether this object needs to use messages that are forced to the network\n"
                "- This is to be used if your Python program is running on only one client\n"
                "Such as a game master, only running on the client that owns a particular object"),

    PYTHON_METHOD(ptParticle, setParticlesPerSecond, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setInitPitchRange, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setInitYawRange, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setVelocityMinimum, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setVelocityMaximum, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setWidthSize, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setHeightSize, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setScaleMinimum, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setScaleMaximum, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setGeneratorLife, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setParticleLifeMinimum, "Params: value\nNEEDS DOCSTRING"),
    PYTHON_METHOD(ptParticle, setParticleLifeMaximum, "Params: value\nNEEDS DOCSTRING"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptParticle, "Plasma particle system class");

// required functions for PyObject interoperability
PyObject *cyParticleSys::New(PyObject *sender, PyObject *recvr)
{
    ptParticle *newObj = (ptParticle*)ptParticle_type.tp_new(&ptParticle_type, nullptr, nullptr);
    if (sender != nullptr)
    {
        newObj->fThis->SetSender(pyKey::ConvertFrom(sender)->getKey());
    }
    if (recvr != nullptr)
    {
        newObj->fThis->AddRecvr(pyKey::ConvertFrom(recvr)->getKey());
    }
    newObj->fThis->SetNetForce(false);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptParticle, cyParticleSys)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptParticle, cyParticleSys)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void cyParticleSys::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptParticle);
    PYTHON_CLASS_IMPORT_END(m);
}
