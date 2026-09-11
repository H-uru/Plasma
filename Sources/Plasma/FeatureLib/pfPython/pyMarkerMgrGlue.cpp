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

#include "pyMarkerMgr.h"

#include "pfMessage/pfMarkerMsg.h"

#include "pyEnum.h"
#include "pyGeometry3.h"
#include "pyGlueHelpers.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptMarkerMgr, pyMarkerMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptMarkerMgr, pyMarkerMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptMarkerMgr)

PYTHON_INIT_DEFINITION(ptMarkerMgr, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptMarkerMgr, addMarker, args)
{
    PyObject* point;
    unsigned long id;
    uint8_t justCreated;
    if (!PyArg_ParseTuple(args, "Olb", &point, &id, &justCreated) || !pyPoint3::Check(point))
    {
        PyErr_SetString(PyExc_TypeError, "addMarker expects a ptPoint3, an unsigned long, and a bool");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->AddMarker(pyPoint3::ConvertFrom(point), id, justCreated != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMarkerMgr, removeMarker, args)
{
    unsigned long id;
    if (!PyArg_ParseTuple(args, "l", &id))
    {
        PyErr_SetString(PyExc_TypeError, "removeMarker expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->RemoveMarker(id);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptMarkerMgr, removeAllMarkers, RemoveAllMarkers)

PYTHON_METHOD_DEFINITION(ptMarkerMgr, setSelectedMarker, args)
{
    unsigned long id;
    if (!PyArg_ParseTuple(args, "l", &id))
    {
        PyErr_SetString(PyExc_TypeError, "setSelectedMarker expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetSelectedMarker(id);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMgr, getSelectedMarker)
{
    // -1 means "no selected marker" -- don't return the unsigned version of that
    // Python will give us 4 billion in that case :)
    uint32_t marker = self->fThis->GetSelectedMarker();
    if (marker == static_cast<uint32_t>(-1))
        return PyLong_FromLong(-1);
    else
        return PyLong_FromUnsignedLong(marker);
}

PYTHON_BASIC_METHOD_DEFINITION(ptMarkerMgr, clearSelectedMarker, ClearSelectedMarker)

PYTHON_METHOD_DEFINITION(ptMarkerMgr, setMarkersRespawn, args)
{
    uint8_t respawn;
    if (!PyArg_ParseTuple(args, "b", &respawn))
    {
        PyErr_SetString(PyExc_TypeError, "setMarkersRespawn expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetMarkersRespawn(respawn != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMgr, getMarkersRespawn)
{
    PYTHON_RETURN_BOOL(self->fThis->GetMarkersRespawn());
}

PYTHON_METHOD_DEFINITION(ptMarkerMgr, captureQuestMarker, args)
{
    unsigned long id;
    uint8_t captured;
    if (!PyArg_ParseTuple(args, "lb", &id, &captured))
    {
        PyErr_SetString(PyExc_TypeError, "captureQuestMarker expects an unsigned long and a bool");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->CaptureQuestMarker(id, captured != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptMarkerMgr, captureTeamMarker, args)
{
    unsigned long id;
    int team;
    if (!PyArg_ParseTuple(args, "li", &id, &team))
    {
        PyErr_SetString(PyExc_TypeError, "captureTeamMarker expects an unsigned long and an int");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->CaptureTeamMarker(id, team);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptMarkerMgr, showMarkersLocal, ShowMarkersLocal)
PYTHON_BASIC_METHOD_DEFINITION(ptMarkerMgr, hideMarkersLocal, HideMarkersLocal)

PYTHON_METHOD_DEFINITION_NOARGS(ptMarkerMgr, areLocalMarkersShowing)
{
    PYTHON_RETURN_BOOL(self->fThis->AreLocalMarkersShowing());
}

PYTHON_START_METHODS_TABLE(ptMarkerMgr)
    PYTHON_METHOD(ptMarkerMgr, addMarker, "Params: x, y, z, id, justCreated\nAdd a marker in the specified location with the specified id"),
    PYTHON_METHOD(ptMarkerMgr, removeMarker, "Params: id\nRemoves the specified marker from the game"),
    PYTHON_BASIC_METHOD(ptMarkerMgr, removeAllMarkers, "Removes all markers"),
    PYTHON_METHOD(ptMarkerMgr, setSelectedMarker, "Params: id\nSets the selected marker to the one with the specified id"),
    PYTHON_METHOD_NOARGS(ptMarkerMgr, getSelectedMarker, "Returns the id of the selected marker"),
    PYTHON_BASIC_METHOD(ptMarkerMgr, clearSelectedMarker, "Unselects the selected marker"),
    PYTHON_METHOD(ptMarkerMgr, setMarkersRespawn, "Params: respawn\nSets whether markers respawn after being captured, or not"),
    PYTHON_METHOD_NOARGS(ptMarkerMgr, getMarkersRespawn, "Returns whether markers respawn after being captured, or not"),
    PYTHON_METHOD(ptMarkerMgr, captureQuestMarker, "Params: id, captured\nSets a marker as captured or not"),
    PYTHON_METHOD(ptMarkerMgr, captureTeamMarker, "Params: id, team\nSets a marker as captured by the specified team (0 = not captured)"),
    PYTHON_BASIC_METHOD(ptMarkerMgr, showMarkersLocal, "Shows the markers on your machine, so you can see where they are"),
    PYTHON_BASIC_METHOD(ptMarkerMgr, hideMarkersLocal, "Hides the markers on your machine, so you can no longer see where they are"),
    PYTHON_METHOD_NOARGS(ptMarkerMgr, areLocalMarkersShowing, "Returns true if we are showing the markers on this local machine"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptMarkerMgr, "Marker manager accessor class");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptMarkerMgr, pyMarkerMgr)

PYTHON_CLASS_CHECK_IMPL(ptMarkerMgr, pyMarkerMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptMarkerMgr, pyMarkerMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyMarkerMgr::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptMarkerMgr);
    PYTHON_CLASS_IMPORT_END(m);
}

void pyMarkerMgr::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtMarkerMsgType)
    PYTHON_ENUM_ELEMENT(PtMarkerMsgType, kMarkerCaptured,   pfMarkerMsg::kMarkerCaptured)
    PYTHON_ENUM_END(m, PtMarkerMsgType)
}
