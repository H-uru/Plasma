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
#ifndef _pyGameScore_h_
#define _pyGameScore_h_

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyGameScore
//
// PURPOSE: a wrapper class to provide access to a game score
//

#include "HeadSpin.h"
#include "pyGlueHelpers.h"

class pfGameScore;

class pyGameScore
{
private:
    pfGameScore * fScore;

public:
    pyGameScore();
    pyGameScore(pfGameScore * score);
    ~pyGameScore();

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGameScore);
    static PyObject* New(pfGameScore* score);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameScore object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameScore); // converts a PyObject to a pyGameScore (throws error if not correct type)

    static void     AddPlasmaClasses(PyObject *m);
    static void     AddPlasmaConstantsClasses(PyObject *m);

    uint32_t        GetOwnerID() const;
    int32_t         GetGameType() const;
    int32_t         GetPoints() const;
    plString        GetGameName() const;

    void            AddPoints(int32_t numPoints, pyKey& rcvr);
    void            Delete();
    void            TransferPoints(pyGameScore* dest, pyKey& rcvr);
    void            TransferPoints(pyGameScore* dest, int32_t numPoints, pyKey& rcvr);
    void            SetPoints(int32_t numPoints, pyKey& rcvr);

    static void     CreateAgeScore(const plString& name, uint32_t type, int32_t points, pyKey& rcvr);
    static void     CreateGlobalScore(const plString& name, uint32_t type, int32_t points, pyKey& rcvr);
    static void     CreatePlayerScore(const plString& name, uint32_t type, int32_t points, pyKey& rcvr);
    static void     CreateScore(uint32_t ownerId, const plString& name, uint32_t type, int32_t points, pyKey& rcvr);

    static void     FindAgeScores(const plString& name, pyKey& rcvr);
    static void     FindGlobalScores(const plString& name, pyKey& rcvr);
    static void     FindPlayerScores(const plString& name, pyKey& rcvr);
    static void     FindScores(uint32_t ownerId, const plString& name, pyKey& rcvr);
};

#endif
