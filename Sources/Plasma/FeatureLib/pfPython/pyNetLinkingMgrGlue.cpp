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

#include "pyNetLinkingMgr.h"

#include <string_theory/string>

#include "plNetCommon/plNetCommon.h"

#include "pyAgeLinkStruct.h"
#include "pyEnum.h"
#include "pyGlueHelpers.h"

#ifndef BUILDING_PYPLASMA

// glue functions
PYTHON_CLASS_DEFINITION(ptNetLinkingMgr, pyNetLinkingMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptNetLinkingMgr, pyNetLinkingMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptNetLinkingMgr)

PYTHON_INIT_DEFINITION(ptNetLinkingMgr, args, keywords)
{
    PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetLinkingMgr, isEnabled)
{
    PYTHON_RETURN_BOOL(self->fThis->IsEnabled());
}

PYTHON_METHOD_DEFINITION(ptNetLinkingMgr, setEnabled, args)
{
    char enable;
    if (!PyArg_ParseTuple(args, "b", &enable))
    {
        PyErr_SetString(PyExc_TypeError, "setEnabled expects a boolean");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->SetEnabled(enable != 0);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_WKEY(ptNetLinkingMgr, linkToAge, args, kwargs)
{
    const char* kwlist[] = { "ageLink", "anim", "linkInSfx", "linkOutSfx", nullptr };
    PyObject* ageLinkObj = nullptr;
    ST::string linkAnim;
    bool linkInSfx = true;
    bool linkOutSfx = true;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O&bb", const_cast<char **>(kwlist),
                                     &ageLinkObj, PyUnicode_STStringConverter, &linkAnim, &linkInSfx, &linkOutSfx))
    {
        PyErr_SetString(PyExc_TypeError, "linkToAge expects a ptAgeLinkStruct and an optional link anim name");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeLinkStruct::Check(ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "linkToAge expects a ptAgeLinkStruct and an optional link anim name");
        PYTHON_RETURN_ERROR;
    }
    pyAgeLinkStruct* ageLink = pyAgeLinkStruct::ConvertFrom(ageLinkObj);
    self->fThis->LinkToAge(*ageLink, linkAnim, linkInSfx, linkOutSfx);
    PYTHON_RETURN_NONE;
}

PYTHON_BASIC_METHOD_DEFINITION(ptNetLinkingMgr, linkToMyPersonalAge, LinkToMyPersonalAge)
PYTHON_BASIC_METHOD_DEFINITION(ptNetLinkingMgr, linkToMyPersonalAgeWithYeeshaBook, LinkToMyPersonalAgeWithYeeshaBook)
PYTHON_BASIC_METHOD_DEFINITION(ptNetLinkingMgr, linkToMyNeighborhoodAge, LinkToMyNeighborhoodAge)

PYTHON_METHOD_DEFINITION(ptNetLinkingMgr, linkPlayerHere, args)
{
    unsigned long pid;
    if (!PyArg_ParseTuple(args, "l", &pid))
    {
        PyErr_SetString(PyExc_TypeError, "linkPlayerHere expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->LinkPlayerHere(pid);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetLinkingMgr, linkPlayerToAge, args)
{
    PyObject* ageLinkObj = nullptr;
    unsigned long pid;
    if (!PyArg_ParseTuple(args, "Ol", &ageLinkObj, &pid))
    {
        PyErr_SetString(PyExc_TypeError, "linkPlayerToAge expects a ptAgeLinkStruct and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    if (!pyAgeLinkStruct::Check(ageLinkObj))
    {
        PyErr_SetString(PyExc_TypeError, "linkPlayerToAge expects a ptAgeLinkStruct and an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    pyAgeLinkStruct* ageLink = pyAgeLinkStruct::ConvertFrom(ageLinkObj);
    self->fThis->LinkPlayerToAge(*ageLink, pid);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptNetLinkingMgr, linkToPlayersAge, args)
{
    unsigned long pid;
    if (!PyArg_ParseTuple(args, "l", &pid))
    {
        PyErr_SetString(PyExc_TypeError, "linkToPlayersAge expects an unsigned long");
        PYTHON_RETURN_ERROR;
    }
    self->fThis->LinkToPlayersAge(pid);
    PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetLinkingMgr, getCurrAgeLink)
{
    return self->fThis->GetCurrAgeLink();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptNetLinkingMgr, getPrevAgeLink)
{
    return self->fThis->GetPrevAgeLink();
}

PYTHON_START_METHODS_TABLE(ptNetLinkingMgr)
    PYTHON_METHOD_NOARGS(ptNetLinkingMgr, isEnabled, "True if linking is enabled."),
    PYTHON_METHOD(ptNetLinkingMgr, setEnabled, "Params: enable\nEnable/Disable linking."),
    PYTHON_METHOD_WKEY(ptNetLinkingMgr, linkToAge, "Params: ageLink, linkAnim\nLinks to ageLink (ptAgeLinkStruct, string)"),
    PYTHON_BASIC_METHOD(ptNetLinkingMgr, linkToMyPersonalAge, "Link to my Personal Age"),
    PYTHON_BASIC_METHOD(ptNetLinkingMgr, linkToMyPersonalAgeWithYeeshaBook, "Link to my Personal Age with the YeeshaBook"),
    PYTHON_BASIC_METHOD(ptNetLinkingMgr, linkToMyNeighborhoodAge, "Link to my Neighborhood Age"),
    PYTHON_METHOD(ptNetLinkingMgr, linkPlayerHere, "Params: pid\nlink player(pid) to where I am"),
    PYTHON_METHOD(ptNetLinkingMgr, linkPlayerToAge, "Params: ageLink,pid\nLink player(pid) to ageLink"),
    PYTHON_METHOD(ptNetLinkingMgr, linkToPlayersAge, "Params: pid\nLink me to where player(pid) is"),
    PYTHON_METHOD_NOARGS(ptNetLinkingMgr, getCurrAgeLink, "Get the ptAgeLinkStruct for the current age"),
    PYTHON_METHOD_NOARGS(ptNetLinkingMgr, getPrevAgeLink, "Get the ptAgeLinkStruct for the previous age"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptNetLinkingMgr, "Constructor to get access to the net link manager");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptNetLinkingMgr, pyNetLinkingMgr)

PYTHON_CLASS_CHECK_IMPL(ptNetLinkingMgr, pyNetLinkingMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptNetLinkingMgr, pyNetLinkingMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyNetLinkingMgr::AddPlasmaClasses(PyObject *m)
{
    PYTHON_CLASS_IMPORT_START(m);
    PYTHON_CLASS_IMPORT(m, ptNetLinkingMgr);
    PYTHON_CLASS_IMPORT_END(m);
}

#endif // BUILDING_PYPLASMA

void pyNetLinkingMgr::AddPlasmaConstantsClasses(PyObject *m)
{
    PYTHON_ENUM_START(m, PtLinkingRules)
    PYTHON_ENUM_ELEMENT(PtLinkingRules, kBasicLink,     plNetCommon::LinkingRules::kBasicLink)
    PYTHON_ENUM_ELEMENT(PtLinkingRules, kOriginalBook,  plNetCommon::LinkingRules::kOriginalBook)
    PYTHON_ENUM_ELEMENT(PtLinkingRules, kSubAgeBook,    plNetCommon::LinkingRules::kSubAgeBook)
    PYTHON_ENUM_ELEMENT(PtLinkingRules, kOwnedBook,     plNetCommon::LinkingRules::kOwnedBook)
    PYTHON_ENUM_ELEMENT(PtLinkingRules, kVisitBook,     plNetCommon::LinkingRules::kVisitBook)
    PYTHON_ENUM_ELEMENT(PtLinkingRules, kChildAgeBook,  plNetCommon::LinkingRules::kChildAgeBook)
    PYTHON_ENUM_END(m, PtLinkingRules)
}
