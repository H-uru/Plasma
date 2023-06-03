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
#ifndef pyPlayer_h
#define pyPlayer_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyPlayer
//
// PURPOSE: Class wrapper for Python to the player data
//

#include "pyGlueHelpers.h"
#include "pnKeyedObject/plKey.h"
#include <string_theory/string>

class pyKey;

class pyPlayer
{
protected:
    plKey           fAvatarKey;
    ST::string      fPlayerName;
    uint32_t        fPlayerID;
    float           fDistSq;            // from local player, temp
    bool            fIsCCR;
    bool            fIsServer;

    pyPlayer(); // only used by python glue, do NOT call
    pyPlayer(pyKey& avKey, const ST::string& pname, uint32_t pid, float distsq);
    pyPlayer(plKey avKey, const ST::string& pname, uint32_t pid, float distsq);
    // another way to create a player with just a name and number
    pyPlayer(const ST::string& pname, uint32_t pid);
public:
    void Init(plKey avKey, const ST::string& pname, uint32_t pid, float distsq); // used by python glue, do NOT call

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptPlayer);
    static PyObject *New(pyKey& avKey, const ST::string& pname, uint32_t pid, float distsq);
    static PyObject *New(plKey avKey, const ST::string& pname, uint32_t pid, float distsq);
    static PyObject *New(const ST::string& pname, uint32_t pid);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyPlayer object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyPlayer); // converts a PyObject to a pyPlayer (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // override the equals to operator
    bool operator==(const pyPlayer &player) const
    {
        // only thing that needs testing is the playerid, which is unique for all
        if ( ((pyPlayer*)this)->GetPlayerID() == player.GetPlayerID() )
            return true;
        else
            return false;
    }
    bool operator!=(const pyPlayer &player) const { return !(player == *this);    }

    // for C++ access
    plKey GetKey() const { return fAvatarKey; }

    // for python access
    ST::string GetPlayerName() const { return fPlayerName; }
    uint32_t GetPlayerID() const  { return fPlayerID; }

    float GetDistSq() const { return fDistSq; }

    void SetCCRFlag(bool state) { fIsCCR = state; }
    bool IsCCR() const { return fIsCCR; }

    void SetServerFlag(bool state) { fIsServer = state; }
    bool IsServer() const { return fIsServer; }

};

#endif  // pyPlayer_h
