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

#ifndef _pyGmMarker_h_
#define _pyGmMarker_h_

#include "pyGameCli.h"

#include "pnGameMgr/pnGmMarkerConst.h"

class pfGmMarker;
class pyGmMarkerHandler;

class pyGmMarker : public pyGameCli
{
public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptGmMarker);
    PYTHON_CLASS_GMCLI_NEW_DEFINITION(pfGmMarker);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultNode object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGmMarker); // converts a PyObject to a pyVaultNode (throws error if not correct type)

protected:
    pfGmMarker* GetGameCli() const { return (pfGmMarker*)fCli; }

public:
    void StartGame() const;
    void PauseGame() const;
    void ResetGame() const;
    void ChangeGameName(const ST::string& name) const;
    void ChangeTimeLimit(uint32_t timeLimit) const;
    void DeleteGame() const;
    void AddMarker(double x, double y, double z, const ST::string& name, const ST::string& age) const;
    void DeleteMarker(uint32_t markerId) const;
    void ChangeMarkerName(uint32_t markerId, const ST::string& markerName) const;
    void CaptureMarker(uint32_t markerId) const;

public:
    static void Create(
        PyObject* handler,
        EMarkerGameType gameType,
        ST::string templateId
    );

    static bool IsSupported();

public:
    static void AddPlasmaGameClasses(PyObject* m);
    static void AddPlasmaGameConstantsClasses(PyObject* m);
};

#endif
