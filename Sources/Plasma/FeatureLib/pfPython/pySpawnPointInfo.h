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
#ifndef pySpawnPointInfo_h_inc
#define pySpawnPointInfo_h_inc

#include "plNetCommon/plSpawnPointInfo.h"
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pySpawnPointInfo   - a wrapper class to provide interface to the plSpawnPointInfo
//
//////////////////////////////////////////////////////////////////////

class pySpawnPointInfo
{
protected:
    pySpawnPointInfo();
    pySpawnPointInfo( const plSpawnPointInfo & info ): fInfo( info ) {}
    pySpawnPointInfo(const ST::string& title, const ST::string& spawnPt);

public:
    plSpawnPointInfo    fInfo;

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptSpawnPointInfo);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(const plSpawnPointInfo& info);
    static PyObject *New(const ST::string& title, const ST::string& spawnPt);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a ptSpawnPointInfo object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySpawnPointInfo); // converts a PyObject to a pySpawnPointInfo (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaMethods(PyObject* m);

    plSpawnPointInfo & SpawnPoint() { return fInfo; }
    void    SetTitle(const ST::string& v) { fInfo.SetTitle(v); }
    ST::string GetTitle() const { return fInfo.GetTitle(); }
    void    SetName(const ST::string& v) { fInfo.SetName(v); }
    ST::string GetName() const { return fInfo.GetName(); }
    void    SetCameraStack(const ST::string& v) { fInfo.SetCameraStack(v); }
    ST::string GetCameraStack() const { return fInfo.GetCameraStack(); }

    static PyObject* GetDefaultSpawnPoint();
};

class pySpawnPointInfoRef
{
private:
    static plSpawnPointInfo fDefaultSPInfo; // created so a default constructor could be made for python, do NOT use

protected:
    pySpawnPointInfoRef(): fInfo(fDefaultSPInfo) {} // only used by python glue, do NOT call directly
    pySpawnPointInfoRef( plSpawnPointInfo & info ): fInfo( info ) {}

public:
    plSpawnPointInfo & fInfo;

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptSpawnPointInfoRef);
    static PyObject *New(plSpawnPointInfo& info);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a ptSpawnPointInfoRef object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySpawnPointInfoRef); // converts a PyObject to a pySpawnPointInfoRef (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    plSpawnPointInfo & SpawnPoint() { return fInfo; }
    void    SetTitle(const ST::string& v) { fInfo.SetTitle(v); }
    ST::string GetTitle() const { return fInfo.GetTitle(); }
    void    SetName(const ST::string& v) { fInfo.SetName(v); }
    ST::string GetName() const { return fInfo.GetName(); }
    void    SetCameraStack(const ST::string& v) { fInfo.SetCameraStack(v); }
    ST::string GetCameraStack() const { return fInfo.GetCameraStack(); }
};

#endif // pySpawnPointInfo_h_inc
