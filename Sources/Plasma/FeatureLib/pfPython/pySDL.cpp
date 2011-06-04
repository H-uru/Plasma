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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "pySDL.h"
#include "../plSDL/plSDL.h"

///////////////////////////////////////////////////////////////////////////

pySDLStateDataRecord::pySDLStateDataRecord()
: fRec( nil )
{
}

pySDLStateDataRecord::pySDLStateDataRecord( plStateDataRecord * rec )
: fRec( rec )
{
}

pySDLStateDataRecord::~pySDLStateDataRecord() {
	DEL(fRec);
}

plStateDataRecord * pySDLStateDataRecord::GetRec() const
{
	return fRec;
}

PyObject * pySDLStateDataRecord::FindVar( const char * name ) const
{
	if ( !fRec )
		PYTHON_RETURN_NONE;

	plSimpleStateVariable * var = fRec->FindVar( name );
	if ( !var )
		PYTHON_RETURN_NONE;

	return pySimpleStateVariable::New( var );
}

const char *pySDLStateDataRecord::GetName() const
{
	if (!fRec)
		return nil;
	const plStateDescriptor *stateDesc = fRec->GetDescriptor();
	return stateDesc->GetName();
}

std::vector<std::string> pySDLStateDataRecord::GetVarList()
{
	std::vector<std::string> retVal;
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
: fVar( nil )
{
}

pySimpleStateVariable::pySimpleStateVariable( plSimpleStateVariable * var )
: fVar( var )
{
}

plSimpleStateVariable *	pySimpleStateVariable::GetVar() const
{
	return fVar;
}

bool pySimpleStateVariable::SetByte( byte v, int idx )
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

bool pySimpleStateVariable::SetString( const char * v, int idx )
{
	if ( !fVar )
		return false;
	return fVar->Set( v, idx );
}

bool pySimpleStateVariable::SetBool( bool v, int idx )
{
	if ( !fVar )
		return false;
	return fVar->Set( v, idx );
}

byte pySimpleStateVariable::GetByte( int idx ) const
{
	byte v = 0;
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

int	pySimpleStateVariable::GetInt( int idx ) const
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

const char * pySimpleStateVariable::GetString( int idx ) const
{
	fString = "";
	if ( fVar )
	{
		char v[256];
		if ( fVar->Get( v, idx ) )
			fString = v;
	}
	return fString.c_str();
}

plKey pySimpleStateVariable::GetKey( int idx ) const
{
	plKey theKey = nil;
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

const char *pySimpleStateVariable::GetDisplayOptions() const
{
	if (!fVar)
		return nil;
	plVarDescriptor *varDesc = fVar->GetVarDescriptor();
	return varDesc->GetDisplayOptions();
}

const char *pySimpleStateVariable::GetDefault() const
{
	if (!fVar)
		return nil;
	plVarDescriptor *varDesc = fVar->GetVarDescriptor();
	return varDesc->GetDefault();
}

bool pySimpleStateVariable::IsInternal() const
{
	if (!fVar)
		return nil;
	plVarDescriptor *varDesc = fVar->GetVarDescriptor();
	return varDesc->IsInternal();
}

bool pySimpleStateVariable::IsAlwaysNew() const
{
	if (!fVar)
		return nil;
	plVarDescriptor *varDesc = fVar->GetVarDescriptor();
	return varDesc->IsAlwaysNew();
}
