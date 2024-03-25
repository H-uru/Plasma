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
#ifndef pyAgeInfoStruct_h_inc
#define pyAgeInfoStruct_h_inc

#include <string_theory/string>

#include "plNetCommon/plNetServerSessionInfo.h"

#include "pyGlueDefinitions.h"

//////////////////////////////////////////////////////////////////////
//
// pyAgeInfoStruct   - a wrapper class to provide interface to the plAgeInfoStruct
//
//////////////////////////////////////////////////////////////////////

class pyAgeInfoStructRef;


class pyAgeInfoStruct
{
private:
    plAgeInfoStruct fAgeInfo;
    mutable ST::string fAgeInstanceGuidStr;    // for getting Age Instance GUID
    mutable ST::string fDisplayName;           // used by GetDisplayName()

protected:
    pyAgeInfoStruct();
    pyAgeInfoStruct(plAgeInfoStruct * info);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptAgeInfoStruct);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(plAgeInfoStruct *info);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeInfoStruct object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeInfoStruct); // converts a PyObject to a pyAgeInfoStruct (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    bool operator==(const pyAgeInfoStruct &other) const;
    bool operator!=(const pyAgeInfoStruct &other) const { return !(other==*this); }
    plAgeInfoStruct * GetAgeInfo() { return &fAgeInfo; }
    const plAgeInfoStruct * GetAgeInfo() const { return &fAgeInfo; }
    void    CopyFrom( const pyAgeInfoStruct & other );
    void    CopyFromRef( const pyAgeInfoStructRef & other );
    ST::string GetAgeFilename() const;
    void    SetAgeFilename( const ST::string & v );
    ST::string GetAgeInstanceName() const;
    void    SetAgeInstanceName( const ST::string & v );
    ST::string GetAgeUserDefinedName() const;
    void    SetAgeUserDefinedName( const ST::string & v );
    ST::string GetAgeDescription() const;
    void    SetAgeDescription( const ST::string & v );
    ST::string GetAgeInstanceGuid() const;
    void    SetAgeInstanceGuid(const ST::string& guid);
    int32_t   GetAgeSequenceNumber() const;
    void    SetAgeSequenceNumber( int32_t v );
    int32_t   GetAgeLanguage() const;
    void    SetAgeLanguage( int32_t v );
    ST::string GetDisplayName() const;
};

class pyAgeInfoStructRef
{
private:
    static plAgeInfoStruct fDefaultAgeInfo; // created so a default constructor could be made for python. Do NOT use

    plAgeInfoStruct & fAgeInfo;
    mutable ST::string fAgeInstanceGuidStr;   // for getting Age Instance GUID
    mutable ST::string fDisplayName;          // used by GetDisplayName()

protected:
    pyAgeInfoStructRef(): fAgeInfo( fDefaultAgeInfo ) {} // only here for the python glue... do NOT call directly
    pyAgeInfoStructRef(plAgeInfoStruct & info): fAgeInfo( info ){}

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptAgeInfoStructRef);
    static PyObject *New(plAgeInfoStruct &info);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyAgeInfoStructRef object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyAgeInfoStructRef); // converts a PyObject to a pyAgeInfoStructRef (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    plAgeInfoStruct * GetAgeInfo() { return &fAgeInfo; }
    const plAgeInfoStruct * GetAgeInfo() const { return &fAgeInfo; }
    void    CopyFrom( const pyAgeInfoStruct & other );
    void    CopyFromRef( const pyAgeInfoStructRef & other );
    ST::string GetAgeFilename() const;
    void    SetAgeFilename( const ST::string & v );
    ST::string GetAgeInstanceName() const;
    void    SetAgeInstanceName( const ST::string & v );
    ST::string GetAgeUserDefinedName() const;
    void    SetAgeUserDefinedName( const ST::string & v );
    ST::string GetAgeInstanceGuid() const;
    void    SetAgeInstanceGuid(const ST::string& guid);
    int32_t   GetAgeSequenceNumber() const;
    void    SetAgeSequenceNumber( int32_t v );
    ST::string GetDisplayName() const;
};

#endif // pyAgeInfoStruct_h_inc
