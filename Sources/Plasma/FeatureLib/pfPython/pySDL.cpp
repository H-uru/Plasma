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

#include "pySDL.h"

#include <Python.h>
#include <string_theory/string>

#include "pnKeyedObject/plKey.h"

#include "plSDL/plSDL.h"

///////////////////////////////////////////////////////////////////////////

pySDLStateDataRecord::pySDLStateDataRecord()
: fRec()
{
}

pySDLStateDataRecord::pySDLStateDataRecord( plStateDataRecord * rec )
: fRec( rec )
{
}

pySDLStateDataRecord::~pySDLStateDataRecord() {
   delete fRec;
}

plStateDataRecord * pySDLStateDataRecord::GetRec() const
{
    return fRec;
}

PyObject * pySDLStateDataRecord::FindVar( const ST::string & name ) const
{
    if ( !fRec )
        PYTHON_RETURN_NONE;

    plSimpleStateVariable * var = fRec->FindVar( name );
    if ( !var )
        PYTHON_RETURN_NONE;

    return pySimpleStateVariable::New( var );
}

ST::string pySDLStateDataRecord::GetName() const
{
    if (!fRec)
        return ST::string();
    const plStateDescriptor *stateDesc = fRec->GetDescriptor();
    return stateDesc->GetName();
}

std::vector<ST::string> pySDLStateDataRecord::GetVarList()
{
    std::vector<ST::string> retVal;
    if (!fRec)
        return retVal;
    const plStateDescriptor *stateDesc = fRec->GetDescriptor();
    int numVars = stateDesc->GetNumVars();
    for (int i=0; i<numVars; i++)
    {
        const plVarDescriptor *varDesc = stateDesc->GetVar(i);
        retVal.push_back(varDesc->GetName());
    }
    return retVal;
}

void pySDLStateDataRecord::SetFromDefaults(bool timeStampNow)
{
    if (fRec)
    {
        fRec->SetFromDefaults(timeStampNow);
    }
}


///////////////////////////////////////////////////////////////////////////

pySimpleStateVariable::pySimpleStateVariable()
: fVar()
{
}

pySimpleStateVariable::pySimpleStateVariable( plSimpleStateVariable * var )
: fVar( var )
{
}

plSimpleStateVariable * pySimpleStateVariable::GetVar() const
{
    return fVar;
}

bool pySimpleStateVariable::SetByte( uint8_t v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v, idx );
}

bool pySimpleStateVariable::SetShort( short v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v, idx );
}

bool pySimpleStateVariable::SetFloat( float v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v, idx );
}

bool pySimpleStateVariable::SetDouble( double v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v, idx );
}

bool pySimpleStateVariable::SetInt( int v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v, idx );
}

bool pySimpleStateVariable::SetString( const ST::string& v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v.c_str(), idx );
}

bool pySimpleStateVariable::SetBool( bool v, int idx )
{
    if ( !fVar )
        return false;
    return fVar->Set( v, idx );
}

uint8_t pySimpleStateVariable::GetByte( int idx ) const
{
    uint8_t v = 0;
    if ( fVar )
        fVar->Get( &v, idx );
    return v;
}

short pySimpleStateVariable::GetShort( int idx ) const
{
    short v = 0;
    if ( fVar )
        fVar->Get( &v, idx );
    return v;
}

int pySimpleStateVariable::GetInt( int idx ) const
{
    int v = 0;
    if ( fVar )
        fVar->Get( &v, idx );
    return v;
}

float pySimpleStateVariable::GetFloat( int idx ) const
{
    float v = 0.f;
    if ( fVar )
        fVar->Get( &v, idx );
    return v;
}

double pySimpleStateVariable::GetDouble( int idx ) const
{
    double v = 0.0;
    if ( fVar )
        fVar->Get( &v, idx );
    return v;
}

bool pySimpleStateVariable::GetBool( int idx ) const
{
    bool v = false;
    if ( fVar )
        fVar->Get( &v, idx );
    return v;
}

ST::string pySimpleStateVariable::GetString( int idx ) const
{
    if ( fVar )
    {
        char v[256];
        if ( fVar->Get( v, idx ) )
            return ST::string::from_utf8(v);
    }
    return "";
}

plKey pySimpleStateVariable::GetKey( int idx ) const
{
    plKey theKey;
    if (fVar)
        fVar->Get(&theKey, idx);
    return theKey;
}

int pySimpleStateVariable::GetType() const
{
    if (!fVar)
        return plVarDescriptor::kNone;
    plVarDescriptor *varDesc = fVar->GetVarDescriptor();
    return varDesc->GetType();
}

ST::string pySimpleStateVariable::GetDisplayOptions() const
{
    if (!fVar)
        return ST::string();
    plVarDescriptor *varDesc = fVar->GetVarDescriptor();
    return varDesc->GetDisplayOptions();
}

ST::string pySimpleStateVariable::GetDefault() const
{
    if (!fVar)
        return ST::string();
    plVarDescriptor *varDesc = fVar->GetVarDescriptor();
    return varDesc->GetDefault();
}

bool pySimpleStateVariable::IsInternal() const
{
    if (!fVar)
        return false;
    plVarDescriptor *varDesc = fVar->GetVarDescriptor();
    return varDesc->IsInternal();
}

bool pySimpleStateVariable::IsAlwaysNew() const
{
    if (!fVar)
        return false;
    plVarDescriptor *varDesc = fVar->GetVarDescriptor();
    return varDesc->IsAlwaysNew();
}

bool pySimpleStateVariable::IsUsed() const
{
    if (fVar)
        return fVar->IsUsed();
    return false;
}
