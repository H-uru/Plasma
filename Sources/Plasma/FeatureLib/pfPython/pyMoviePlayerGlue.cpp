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

#include "pyMoviePlayer.h"

#include "pfMessage/pfMovieEventMsg.h"

#include "pyColor.h"
#include "pyEnum.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptMoviePlayer, pyMoviePlayer);

PYTHON_DEFAULT_NEW_DEFINITION(ptMoviePlayer, pyMoviePlayer)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMoviePlayer)

PYTHON_INIT_DEFINITION(ptMoviePlayer, args, keywords)
{
    ST::string movieName;
    PyObject* selfKeyObj = nullptr;
    if (!PyArg_ParseTuple(args, "O&O", PyUnicode_STStringConverter, &movieName, &selfKeyObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a string and ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    if (!pyKey::Check(selfKeyObj))
    {
        PyErr_SetString(PyExc_TypeError, "__init__ expects a string and ptKey");
        PYTHON_RETURN_INIT_ERROR;
    }
    pyKey* selfKey = pyKey::ConvertFrom(selfKeyObj);
    self->fThis->MakeMovie(movieName, *selfKey);
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptMoviePlayer, setCenter, args)
{
    float x, y;
    if (!PyArg_ParseTuple(args, "ff", &x, &y))
    {
        PyErr_SetString(PyExc_TypeError, "setCenter expects two floats");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetCenter(x, y);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMoviePlayer, setScale, args)
{
    float width, height;
    if (!PyArg_ParseTuple(args, "ff", &width, &height))
    {
        PyErr_SetString(PyExc_TypeError, "setScale expects two floats");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetScale(width, height);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMoviePlayer, setColor, args)
{
    PyObject* colorObj = nullptr;
    if (!PyArg_ParseTuple(args, "O", &colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "setColor expects a ptColor");
        PYTHON_RETURN_ERROR;
    }
    if (!pyColor::Check(colorObj))
    {
        PyErr_SetString(PyExc_TypeError, "setColor expects a ptColor");
        PYTHON_RETURN_ERROR;
    }
    pyColor* color = pyColor::ConvertFrom(colorObj);
    self->fThis->SetColor(*color);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMoviePlayer, setVolume, args)
{
    float volume;
    if (!PyArg_ParseTuple(args, "f", &volume))
    {
        PyErr_SetString(PyExc_TypeError, "setVolume expects a float");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetVolume(volume);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMoviePlayer, setOpacity, args)
{
    float opacity;
    if (!PyArg_ParseTuple(args, "f", &opacity))
    {
        PyErr_SetString(PyExc_TypeError, "setOpacity expects a float");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetOpacity(opacity);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptMoviePlayer, play, Play)
PYTHON_BASIC_METHOD_DEFINITION(ptMoviePlayer, playPaused, PlayPaused)
PYTHON_BASIC_METHOD_DEFINITION(ptMoviePlayer, pause, Pause)
PYTHON_BASIC_METHOD_DEFINITION(ptMoviePlayer, resume, Resume)
PYTHON_BASIC_METHOD_DEFINITION(ptMoviePlayer, stop, Stop)

PYTHON_START_METHODS_TABLE(ptMoviePlayer)
    PYTHON_METHOD(ptMoviePlayer, setCenter, "Params: x,y\nSets the center of the movie"),
    PYTHON_METHOD(ptMoviePlayer, setScale, "Params: width,height\nSets the width and height scale of the movie"),
    PYTHON_METHOD(ptMoviePlayer, setColor, "Params: color\nSets the color of the movie"),
    PYTHON_METHOD(ptMoviePlayer, setVolume, "Params: volume\nSet the volume of the movie"),
    PYTHON_METHOD(ptMoviePlayer, setOpacity, "Params: opacity\nSets the opacity of the movie"),
    PYTHON_BASIC_METHOD(ptMoviePlayer, play, "Plays the movie"),
    PYTHON_BASIC_METHOD(ptMoviePlayer, playPaused, "Plays movie, but pauses at first frame"),
    PYTHON_BASIC_METHOD(ptMoviePlayer, pause, "Pauses the movie"),
    PYTHON_BASIC_METHOD(ptMoviePlayer, resume, "Resumes movie after pausing"),
    PYTHON_BASIC_METHOD(ptMoviePlayer, stop, "Stops the movie"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptMoviePlayer, "Params: movieName,selfKey\nAccessor class to play in the MoviePlayer");

// required functions for PyObject interoperability
PyObject *pyMoviePlayer::New(const ST::string& movieName, pyKey& selfKey)
{
    ptMoviePlayer *newObj = (ptMoviePlayer*)ptMoviePlayer_type.tp_new(&ptMoviePlayer_type, nullptr, nullptr);
    newObj->fThis->MakeMovie(movieName, selfKey);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptMoviePlayer, pyMoviePlayer)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMoviePlayer, pyMoviePlayer)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyMoviePlayer::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptMoviePlayer);
    PYTHON_CLASS_IMPORT_END(m);
}

void pyMoviePlayer::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtMovieEventReason)
    PYTHON_ENUM_ELEMENT(PtMovieEventReason, kMovieDone, pfMovieEventMsg::kMovieDone)
    PYTHON_ENUM_END(m, PtMovieEventReason)
}
