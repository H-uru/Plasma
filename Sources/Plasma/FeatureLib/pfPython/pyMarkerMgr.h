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
#ifndef pyMarkerMgr_h_inc
#define pyMarkerMgr_h_inc

//////////////////////////////////////////////////////////////////////
//
// pyMarkerMgr   - a wrapper class to provide interface to the pfMarkerMgr stuff
//
//////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "pyGlueDefinitions.h"

class pyPoint3;

class pyMarkerMgr
{
protected:
    pyMarkerMgr() {}

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptMarkerMgr);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerMgr object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerMgr); // converts a PyObject to a pyMarkerMgr (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaConstantsClasses(PyObject *m);

    void AddMarker(pyPoint3* pos, uint32_t id, bool justCreated);
    void RemoveMarker(uint32_t id);
    void RemoveAllMarkers();

    void SetSelectedMarker(uint32_t markerID);
    uint32_t GetSelectedMarker();
    void ClearSelectedMarker();

    void SetMarkersRespawn(bool respawn);
    bool GetMarkersRespawn();

    void CaptureQuestMarker(uint32_t id, bool captured); // for QUEST games (no teams)
    void CaptureTeamMarker(uint32_t id, int team); // for TEAM games (0 = not captured)
    
    // Shows your markers locally, so you can see where they are
    void ShowMarkersLocal();
    void HideMarkersLocal();
    bool AreLocalMarkersShowing();
};


#endif // pyMarkerMgr_h_inc
