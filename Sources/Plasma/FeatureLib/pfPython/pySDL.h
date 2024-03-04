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
#ifndef _pySDL_h_
#define _pySDL_h_

//////////////////////////////////////////////////////////////////////
//
// pySDL - python subset of the plSDL library.
//
//////////////////////////////////////////////////////////////////////

#include <vector>

#include "HeadSpin.h"

#include "pyGlueDefinitions.h"

class plKey;
class plSimpleStateVariable;
class plStateDataRecord;
namespace ST { class string; }

// pySDL -- this thing really only exists for the constants
class pySDL
{
public:
    static void AddPlasmaConstantsClasses(PyObject *m);
};


// pySDLStateDataRecord
class pySDLStateDataRecord
{
private:
    plStateDataRecord * fRec;

protected:
    pySDLStateDataRecord();
    pySDLStateDataRecord( plStateDataRecord * rec );

public:
    ~pySDLStateDataRecord();

    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptSDLStateDataRecord);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(plStateDataRecord* rec);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pySDLStateDataRecord object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySDLStateDataRecord); // converts a PyObject to a pySDLStateDataRecord (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    plStateDataRecord * GetRec() const;

    /////////////////////
    PyObject * FindVar( const ST::string & name ) const; // returns pySimpleStateVariable
    ST::string GetName() const;
    std::vector<ST::string> GetVarList();
    void SetFromDefaults(bool timeStampNow);
};

// pySimpleStateVariable
class pySimpleStateVariable
{
private:
    plSimpleStateVariable * fVar;

protected:
    pySimpleStateVariable();
    pySimpleStateVariable( plSimpleStateVariable * var );

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptSimpleStateVariable);
    PYTHON_CLASS_NEW_DEFINITION;
    static PyObject *New(plSimpleStateVariable* var);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pySimpleStateVariable object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(pySimpleStateVariable); // converts a PyObject to a pySimpleStateVariable (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    plSimpleStateVariable * GetVar() const;

    /////////////////////
    bool    SetByte( uint8_t v, int idx=0 );
    bool    SetShort( short v, int idx=0 );
    bool    SetFloat( float v, int idx=0 );
    bool    SetDouble( double v, int idx=0 );
    bool    SetInt( int v, int idx=0 );
    bool    SetString( const ST::string& v, int idx=0 );
    bool    SetBool(bool v, int idx=0 );
    uint8_t GetByte( int idx=0 ) const;
    short   GetShort( int idx=0 ) const;
    int     GetInt( int idx=0 ) const;
    float   GetFloat( int idx=0 ) const;
    double  GetDouble( int idx=0 ) const;           
    bool    GetBool( int idx=0 ) const;
    ST::string GetString( int idx=0 ) const;
    plKey   GetKey( int idx=0 ) const;

    int     GetType() const;
    ST::string GetDisplayOptions() const;
    ST::string GetDefault() const;
    bool    IsAlwaysNew() const;
    bool    IsInternal() const;
    bool    IsUsed() const;

};


#endif //   _pySDL_h_
