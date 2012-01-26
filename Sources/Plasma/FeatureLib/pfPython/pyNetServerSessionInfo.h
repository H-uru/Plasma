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
#ifndef pyNetServerSessionInfo_h_inc
#define pyNetServerSessionInfo_h_inc

#include "HeadSpin.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "pnUUID/pnUUID.h"

#include <Python.h>
#include "pyGlueHelpers.h"

//////////////////////////////////////////////////////////////////////
//
// pyNetServerSessionInfo, pyNetServerSessionInfoRef
//      - wrapper classes to provide interface to the plNetServerSessionInfo
//
//////////////////////////////////////////////////////////////////////

class pyNetServerSessionInfo
{
private:
    plNetServerSessionInfo  fInfo;
    mutable plUUID fServerGuid; // for GetServerGuid()

protected:
    pyNetServerSessionInfo() {}
    pyNetServerSessionInfo( const plNetServerSessionInfo & info ): fInfo( info ) {}

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptNetServerSessionInfo);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(const plNetServerSessionInfo &info);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNetServerSessionInfo object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNetServerSessionInfo); // converts a PyObject to a pyNetServerSessionInfo (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    plNetServerSessionInfo & ServerInfo() { return fInfo; }

    void SetServerName(const char * val) { fInfo.SetServerName( val ); }
    void SetServerType(uint8_t val) { fInfo.SetServerType( val ); }
    void SetServerAddr(const char * val) { fInfo.SetServerAddr( val ); }
    void SetServerPort(uint16_t val) { fInfo.SetServerPort( val ); }
    void SetServerGuid(const char * val) { fServerGuid.FromString( val ); fInfo.SetServerGuid( &fServerGuid ); }
    bool HasServerName() const { return fInfo.HasServerName(); }
    bool HasServerType() const { return fInfo.HasServerType(); }
    bool HasServerAddr() const { return fInfo.HasServerAddr(); }
    bool HasServerPort() const { return fInfo.HasServerPort(); }
    bool HasServerGuid() const { return fInfo.HasServerGuid(); }
    const char *    GetServerName() const { return fInfo.GetServerName(); }
    uint8_t           GetServerType() const { return fInfo.GetServerType(); }
    const char *    GetServerAddr() const { return fInfo.GetServerAddr(); }
    uint16_t          GetServerPort() const { return fInfo.GetServerPort(); }
    const char *    GetServerGuid() const { fServerGuid.CopyFrom( fInfo.GetServerGuid() ); return fServerGuid.AsString(); }
};


class pyNetServerSessionInfoRef
{
private:
    static plNetServerSessionInfo fDefaultServerSessionInfo; // created so a default constructor could be made for python. Do NOT use

    plNetServerSessionInfo& fInfo;
    mutable plUUID fServerGuid; // for GetServerGuid()

protected:
    pyNetServerSessionInfoRef(): fInfo(fDefaultServerSessionInfo) {} // only here for the python glue... do NOT call directly
    pyNetServerSessionInfoRef( plNetServerSessionInfo& info ): fInfo( info ) {}
    plNetServerSessionInfo & ServerInfo() { return fInfo; }

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptNetServerSessionInfoRef);
    static PyObject *New(plNetServerSessionInfo &info);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyNetServerSessionInfoRef object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyNetServerSessionInfoRef); // converts a PyObject to a pyNetServerSessionInfoRef (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    void SetServerName(const char * val) { fInfo.SetServerName( val ); }
    void SetServerType(uint8_t val) { fInfo.SetServerType( val ); }
    void SetServerAddr(const char * val) { fInfo.SetServerAddr( val ); }
    void SetServerPort(uint16_t val) { fInfo.SetServerPort( val ); }
    void SetServerGuid(const char * val) { fServerGuid.FromString( val ); fInfo.SetServerGuid( &fServerGuid ); }
    bool HasServerName() const { return fInfo.HasServerName(); }
    bool HasServerType() const { return fInfo.HasServerType(); }
    bool HasServerAddr() const { return fInfo.HasServerAddr(); }
    bool HasServerPort() const { return fInfo.HasServerPort(); }
    bool HasServerGuid() const { return fInfo.HasServerGuid(); }
    const char *    GetServerName() const { return fInfo.GetServerName(); }
    uint8_t           GetServerType() const { return fInfo.GetServerType(); }
    const char *    GetServerAddr() const { return fInfo.GetServerAddr(); }
    uint16_t          GetServerPort() const { return fInfo.GetServerPort(); }
    const char *    GetServerGuid() const { fServerGuid.CopyFrom( fInfo.GetServerGuid() ); return fServerGuid.AsString(); }
};


#endif // pyNetServerSessionInfo_h_inc
