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
#ifndef pyGameMgrMsg_h
#define pyGameMgrMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyGameMgrMsg
//
// PURPOSE: Class wrapper for game manager messages
//

#include "../pyGlueHelpers.h"
#include <string>

class pfGameMgrMsg;

class pyGameMgrMsg
{
protected:
    pfGameMgrMsg* message;

    pyGameMgrMsg();
    pyGameMgrMsg(pfGameMgrMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_EXPOSE_TYPE; // so we can subclass
    PYTHON_CLASS_NEW_FRIEND(ptGameMgrMsg);
    static PyObject* New(pfGameMgrMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameMgrMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameMgrMsg); // converts a PyObject to a pyGameMgrMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);
    static void AddPlasmaConstantsClasses(PyObject* m);

    int GetType() const;

    PyObject* UpcastToInviteReceivedMsg() const; // returns ptGameMgrInviteReceivedMsg
    PyObject* UpcastToInviteRevokedMsg() const; // returns ptGameMgrInviteRevokedMsg
};

///////////////////////////////////////////////////////////////////////////////
class pyGameMgrInviteReceivedMsg : public pyGameMgrMsg
{
protected:
    pyGameMgrInviteReceivedMsg();
    pyGameMgrInviteReceivedMsg(pfGameMgrMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGameMgrInviteReceivedMsg);
    static PyObject* New(pfGameMgrMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameMgrInviteReceivedMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameMgrInviteReceivedMsg); // converts a PyObject to a pyGameMgrInviteReceivedMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);

    unsigned long InviterID() const;
    std::wstring GameTypeID() const;
    unsigned long NewGameID() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyGameMgrInviteRevokedMsg : public pyGameMgrMsg
{
protected:
    pyGameMgrInviteRevokedMsg();
    pyGameMgrInviteRevokedMsg(pfGameMgrMsg* msg);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGameMgrInviteRevokedMsg);
    static PyObject* New(pfGameMgrMsg* msg);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameMgrInviteRevokedMsg object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameMgrInviteRevokedMsg); // converts a PyObject to a pyGameMgrInviteRevokedMsg (throws error if not correct type)

    static void AddPlasmaClasses(PyObject* m);

    unsigned long InviterID() const;
    std::wstring GameTypeID() const;
    unsigned long NewGameID() const;
};

#endif  // pyGameMgrMsg_h
