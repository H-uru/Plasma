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
#include "hsStlUtils.h"

#include "pyAgeLinkStruct.h"
#include "pySpawnPointInfo.h"

///////////////////////////////////////////////////////////////////////////


pyAgeLinkStruct::pyAgeLinkStruct()
{
}

pyAgeLinkStruct::pyAgeLinkStruct( plAgeLinkStruct * link )
{
	fAgeLink.CopyFrom( link );
}

pyAgeLinkStruct::~pyAgeLinkStruct()
{
}

bool pyAgeLinkStruct::operator==(const pyAgeLinkStruct &other) const
{
	return fAgeLink.IsEqualTo( other.GetAgeLink() );
}

/////////////////////////////////////////////////////////////////////

PyObject * pyAgeLinkStruct::GetAgeInfo()
{
	return pyAgeInfoStructRef::New( *fAgeLink.GetAgeInfo() );
}

void pyAgeLinkStruct::SetAgeInfo( pyAgeInfoStruct & info )
{
	fAgeLink.GetAgeInfo()->CopyFrom( info.GetAgeInfo() );
}

const char* pyAgeLinkStruct::GetParentAgeFilename()
{
	return fAgeLink.GetParentAgeFilename();
}

void pyAgeLinkStruct::SetParentAgeFilename( const char* parentname )
{
	fAgeLink.SetParentAgeFilename(parentname);
}

void pyAgeLinkStruct::CopyFrom( const pyAgeLinkStruct & other )
{
	fAgeLink.CopyFrom( other.GetAgeLink() );
}

void pyAgeLinkStruct::CopyFromRef( const pyAgeLinkStructRef & other )
{
	fAgeLink.CopyFrom( other.GetAgeLink() );
}

void pyAgeLinkStruct::SetLinkingRules( int v )
{
	fAgeLink.SetLinkingRules( v );
}

int pyAgeLinkStruct::GetLinkingRules() const
{
	return fAgeLink.GetLinkingRules();
}

void pyAgeLinkStruct::SetSpawnPoint( pySpawnPointInfo & v )
{
	fAgeLink.SpawnPoint() = v.SpawnPoint();
}

void pyAgeLinkStruct::SetSpawnPointRef( pySpawnPointInfoRef & v )
{
	fAgeLink.SpawnPoint() = v.SpawnPoint();
}

PyObject * pyAgeLinkStruct::GetSpawnPoint()
{
	return pySpawnPointInfoRef::New( fAgeLink.SpawnPoint() );
}


////////////////////////////////////////////////////////////////////////

plAgeLinkStruct pyAgeLinkStructRef::fDefaultLinkStruct; // created so a default constructor could be made for python. Do NOT use

PyObject * pyAgeLinkStructRef::GetAgeInfo()
{
	return pyAgeInfoStructRef::New( *fAgeLink.GetAgeInfo() );
}

void pyAgeLinkStructRef::SetAgeInfo( pyAgeInfoStruct & info )
{
	fAgeLink.GetAgeInfo()->CopyFrom( info.GetAgeInfo() );
}

void pyAgeLinkStructRef::CopyFrom( const pyAgeLinkStruct & other )
{
	fAgeLink.CopyFrom( other.GetAgeLink() );
}

void pyAgeLinkStructRef::CopyFromRef( const pyAgeLinkStructRef & other )
{
	fAgeLink.CopyFrom( other.GetAgeLink() );
}

void pyAgeLinkStructRef::SetLinkingRules( int v )
{
	fAgeLink.SetLinkingRules( v );
}

int pyAgeLinkStructRef::GetLinkingRules() const
{
	return fAgeLink.GetLinkingRules();
}

void pyAgeLinkStructRef::SetSpawnPoint( pySpawnPointInfo & v )
{
	fAgeLink.SpawnPoint() = v.SpawnPoint();
}

void pyAgeLinkStructRef::SetSpawnPointRef( pySpawnPointInfoRef & v )
{
	fAgeLink.SpawnPoint() = v.SpawnPoint();
}

PyObject * pyAgeLinkStructRef::GetSpawnPoint()
{
	return pySpawnPointInfoRef::New( fAgeLink.SpawnPoint() );
}


