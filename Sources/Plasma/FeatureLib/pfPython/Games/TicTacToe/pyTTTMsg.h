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
#ifndef pyTTTMsg_h
#define pyTTTMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyTTTMsg
//
// PURPOSE: Class wrapper for TTT game messages
//
#include <Python.h>

#include "pfGameMgr/pfGameMgr.h"

#include "../../pyGlueHelpers.h"
#include "../pyGameCliMsg.h"

class pyTTTMsg : public pyGameCliMsg
{
protected:
    pyTTTMsg();
    pyTTTMsg(pfGameCliMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_EXPOSE_TYPE; // so we can subclass
    PYTHON_CLASS_NEW_FRIEND(ptTTTMsg);
    static PyObject* New(pfGameCliMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyTTTMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyTTTMsg); // converts a PyObject to a pyTTTMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);
    static void AddPlasmaConstantsClasses(PyObject* m);

    int GetTTTMsgType() const;

    PyObject* UpcastToFinalTTTMsg() const; // returns the ttt message that this really is
};

///////////////////////////////////////////////////////////////////////////////
class pyTTTGameStartedMsg : public pyTTTMsg
{
protected:
    pyTTTGameStartedMsg();
    pyTTTGameStartedMsg(pfGameCliMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptTTTGameStartedMsg);
    static PyObject* New(pfGameCliMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyTTTGameStartedMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyTTTGameStartedMsg); // converts a PyObject to a pyTTTGameStartedMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);

    bool YourTurn() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyTTTGameOverMsg : public pyTTTMsg
{
protected:
    pyTTTGameOverMsg();
    pyTTTGameOverMsg(pfGameCliMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptTTTGameOverMsg);
    static PyObject* New(pfGameCliMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyTTTGameOverMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyTTTGameOverMsg); // converts a PyObject to a pyTTTGameOverMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);

    int Result() const;
    unsigned long WinnerID() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyTTTMoveMadeMsg : public pyTTTMsg
{
protected:
    pyTTTMoveMadeMsg();
    pyTTTMoveMadeMsg(pfGameCliMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptTTTMoveMadeMsg);
    static PyObject* New(pfGameCliMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyTTTMoveMadeMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyTTTMoveMadeMsg); // converts a PyObject to a pyTTTMoveMadeMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);

    unsigned long PlayerID() const;
    int Row() const;
    int Col() const;
};

#endif // pyTTTMsg_h
