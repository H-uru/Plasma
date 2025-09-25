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

#include "pyGameScore.h"

#include <string_theory/string>

#include "pfGameScoreMgr/pfGameScoreMgr.h"

#include "pyEnum.h"
#include "pyGlueHelpers.h"
#include "pyKey.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptGameScore, pyGameScore);

PYTHON_DEFAULT_NEW_DEFINITION(ptGameScore, pyGameScore)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptGameScore)

PYTHON_NO_INIT_DEFINITION(ptGameScore)

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getOwnerID)
{
    return PyLong_FromLong(self->fThis->GetOwnerID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getPoints)
{
    return PyLong_FromLong(self->fThis->GetPoints());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getGameType)
{
    return PyLong_FromLong(self->fThis->GetGameType());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, getName)
{
    return PyUnicode_FromSTString(self->fThis->GetGameName());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptGameScore, remove)
{
    self->fThis->Delete();
    PYTHON_RETURN_NONE; // who cares about a result?
}

PYTHON_METHOD_DEFINITION(ptGameScore, addPoints, args)
{
    int32_t numPoints = 0;
    PyObject* keyObj = nullptr;
    if (!PyArg_ParseTuple(args, "i|O", &numPoints, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "addPoints expects an int and an optional key");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "addPoints expects an int and an optional key");
        PYTHON_RETURN_ERROR;
    }

    pyKey* rcvr = pyKey::ConvertFrom(keyObj);
    self->fThis->AddPoints(numPoints, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_WKEY(ptGameScore, transferPoints, args, kwargs)
{
    const char* kwlist[] = { "dest", "points", "key", nullptr };
    PyObject* destObj = nullptr;
    int32_t   points  = 0; // Hmmm... Evil?
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|iO", const_cast<char **>(kwlist),
                                     &destObj, &points, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "transferPoints expects a ptGameScore, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!(pyGameScore::Check(destObj) && pyKey::Check(keyObj)))
    {
        PyErr_SetString(PyExc_TypeError, "transferPoints expects a ptGameScore, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyGameScore* dest = pyGameScore::ConvertFrom(destObj);
    pyKey*       rcvr = pyKey::ConvertFrom(keyObj);
    if (points)
        self->fThis->TransferPoints(dest, points, *rcvr);
    else
        self->fThis->TransferPoints(dest, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION(ptGameScore, setPoints, args)
{
    int32_t numPoints = 0;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTuple(args, "i|O", &numPoints, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "setPoints expects an int and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "setPoints expects an int and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey* rcvr = pyKey::ConvertFrom(keyObj);
    self->fThis->SetPoints(numPoints, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC_WKEY(ptGameScore, createAgeScore, args, kwargs)
{
    const char* kwlist[] = { "scoreName", "type", "points", "key", nullptr };
    ST::string name;
    uint32_t type     = 0;
    int32_t points    = 0;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&I|iO", const_cast<char **>(kwlist),
                                     PyUnicode_STStringConverter, &name, &type, &points, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createAgeScore expects a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createAgeScore expects a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::CreateAgeScore(name, type, points, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC_WKEY(ptGameScore, createGlobalScore, args, kwargs)
{
    const char* kwlist[] = { "scoreName", "type", "points", "key", nullptr };
    ST::string name;
    uint32_t type     = 0;
    int32_t points    = 0;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&I|iO", const_cast<char **>(kwlist),
                                     PyUnicode_STStringConverter, &name, &type, &points, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createGlobalScore expects a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createGlobalScore expects a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::CreateGlobalScore(name, type, points, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC_WKEY(ptGameScore, createPlayerScore, args, kwargs)
{
    const char* kwlist[] = { "scoreName", "type", "points", "key", nullptr };
    ST::string name;
    uint32_t type     = 0;
    int32_t points    = 0;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&I|iO", const_cast<char **>(kwlist),
                                     PyUnicode_STStringConverter, &name, &type, &points, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createPlayerScore expects a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createPlayerScore expects a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::CreatePlayerScore(name, type, points, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC_WKEY(ptGameScore, createScore, args, kwargs)
{
    const char* kwlist[] = { "ownerID", "scoreName", "type", "points", "key", nullptr };
    uint32_t ownerID  = 0;
    ST::string name;
    uint32_t type     = 0;
    int32_t points    = 0;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "IO&I|iO", const_cast<char **>(kwlist),
                                     &ownerID, PyUnicode_STStringConverter, &name, &type, &points,
                                     &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createScore expects an int, a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "createScore expects an int, a string, an int, an optional int, and an optional ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::CreateScore(ownerID, name, type, points, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC(ptGameScore, findAgeScores, args)
{
    ST::string name;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTuple(args, "O&O", PyUnicode_STStringConverter, &name, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findAgeScores expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findAgeScores expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::FindAgeScores(name, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC(ptGameScore, findGlobalScores, args)
{
    ST::string name;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTuple(args, "O&O", PyUnicode_STStringConverter, &name, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findGlobalScores expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findGlobalScores expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::FindGlobalScores(name, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC(ptGameScore, findPlayerScores, args)
{
    ST::string name;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTuple(args, "O&O", PyUnicode_STStringConverter, &name, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findPlayerScores expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findPlayerScores expects a string and a ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::FindPlayerScores(name, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC(ptGameScore, findScores, args)
{
    uint32_t ownerId  = 0;
    ST::string name;
    PyObject* keyObj  = nullptr;
    if (!PyArg_ParseTuple(args, "IO&O", &ownerId, PyUnicode_STStringConverter, &name, &keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findScore expects an int, a string, and a ptKey");
        PYTHON_RETURN_ERROR;
    }
    if (!pyKey::Check(keyObj))
    {
        PyErr_SetString(PyExc_TypeError, "findScore expects an int, a string, and a ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::FindScores(ownerId, name, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC(ptGameScore, findAgeHighScores, args)
{
    ST::string name;
    uint32_t maxScores;
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O&IO", PyUnicode_STStringConverter, &name, &maxScores, &keyObj) ||
        !pyKey::Check(keyObj)) {
        PyErr_SetString(PyExc_TypeError, "findAgeHighScores expects a string, an int, and a ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::FindAgeHighScores(name, maxScores, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_METHOD_DEFINITION_STATIC(ptGameScore, findGlobalHighScores, args)
{
    ST::string name;
    uint32_t maxScores;
    PyObject* keyObj;
    if (!PyArg_ParseTuple(args, "O&IO", PyUnicode_STStringConverter, &name, &maxScores, &keyObj) ||
        !pyKey::Check(keyObj)) {
        PyErr_SetString(PyExc_TypeError, "findGlobalHighScores expects a string, an int, and a ptKey");
        PYTHON_RETURN_ERROR;
    }

    pyKey*   rcvr = pyKey::ConvertFrom(keyObj);
    pyGameScore::FindGlobalHighScores(name, maxScores, *rcvr);
    PYTHON_RETURN_NONE; // get result in callback
}

PYTHON_START_METHODS_TABLE(ptGameScore)
    PYTHON_METHOD_NOARGS(ptGameScore, getGameType, "Returns the score game type."),
    PYTHON_METHOD_NOARGS(ptGameScore, getName, "Returns the score game name."),
    PYTHON_METHOD_NOARGS(ptGameScore, getOwnerID, "Returns the score owner."),
    PYTHON_METHOD_NOARGS(ptGameScore, getPoints, "Returns the number of points in this score"),
    PYTHON_METHOD_NOARGS(ptGameScore, remove, "Removes this score from the server"),
    PYTHON_METHOD(ptGameScore, addPoints, "Params: points, key=None\nAdds points to the score"),
    PYTHON_METHOD_WKEY(ptGameScore, transferPoints, "Params: dest, points=0, key=None\nTransfers points from this score to another"),
    PYTHON_METHOD(ptGameScore, setPoints, "Params: numPoints, key\nSets the number of points in the score\nDon't use to add/remove points, use only to reset values!"),
    PYTHON_METHOD_STATIC_WKEY(ptGameScore, createAgeScore, "Params: scoreName, type, points=0, key=None\nCreates a new score associated with this age"),
    PYTHON_METHOD_STATIC_WKEY(ptGameScore, createGlobalScore, "Params: scoreName, type, points=0, key=None\nCreates a new global score"),
    PYTHON_METHOD_STATIC_WKEY(ptGameScore, createPlayerScore, "Params: scoreName, type, points=0, key=None\nCreates a new score associated with this player"),
    PYTHON_METHOD_STATIC_WKEY(ptGameScore, createScore, "Params: ownerID, scoreName, type, points=0, key=None\nCreates a new score for an arbitrary owner"),
    PYTHON_METHOD_STATIC(ptGameScore, findAgeScores, "Params: scoreName, key\nFinds matching scores for this age"),
    PYTHON_METHOD_STATIC(ptGameScore, findGlobalScores, "Params: scoreName, key\nFinds matching global scores"),
    PYTHON_METHOD_STATIC(ptGameScore, findPlayerScores, "Params: scoreName, key\nFinds matching player scores"),
    PYTHON_METHOD_STATIC(ptGameScore, findScores, "Params: ownerID, scoreName, key\nFinds matching scores for an arbitrary owner"),
    PYTHON_METHOD_STATIC(ptGameScore, findAgeHighScores, "Params: name, maxScores, key\nFinds the highest matching scores for the current age's owners"),
    PYTHON_METHOD_STATIC(ptGameScore, findGlobalHighScores, "Params: name, maxScores, key\nFinds the highest matching scores"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptGameScore, "Plasma Game Score");

// required functions for PyObject interoperability
PyObject* pyGameScore::New(pfGameScore* score)
{
    ptGameScore* newObj = (ptGameScore*)ptGameScore_type.tp_new(&ptGameScore_type, nullptr, nullptr);
    hsRefCnt_SafeUnRef(newObj->fThis->fScore);
    newObj->fThis->fScore = score;
    hsRefCnt_SafeRef(newObj->fThis->fScore);
    return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptGameScore, pyGameScore)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptGameScore, pyGameScore)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyGameScore::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptGameScore);
    PYTHON_CLASS_IMPORT_END(m);
}

void pyGameScore::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtGameScoreTypes)
    PYTHON_ENUM_ELEMENT(PtGameScoreTypes, kFixed, kScoreTypeFixed)
    PYTHON_ENUM_ELEMENT(PtGameScoreTypes, kAccumulative, kScoreTypeAccumulative)
    PYTHON_ENUM_ELEMENT(PtGameScoreTypes, kAccumAllowNegative, kScoreTypeAccumAllowNegative)
    PYTHON_ENUM_END(m, PtGameScoreTypes)

    PYTHON_ENUM_START(m, PtScoreRankGroups)
    PYTHON_ENUM_ELEMENT(PtScoreRankGroups, kIndividual, kScoreRankGroupIndividual)
    PYTHON_ENUM_ELEMENT(PtScoreRankGroups, kNeighborhood, kScoreRankGroupNeighborhood)
    PYTHON_ENUM_END(m, PtScoreRankGroups)

    PYTHON_ENUM_START(m, PtScoreTimePeriods)
    PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kOverall, kScoreTimePeriodOverall)
    PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kYear, kScoreTimePeriodYear)
    PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kMonth, kScoreTimePeriodMonth)
    PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kDay, kScoreTimePeriodDay)
    PYTHON_ENUM_END(m, PtScoreTimePeriods)
}
