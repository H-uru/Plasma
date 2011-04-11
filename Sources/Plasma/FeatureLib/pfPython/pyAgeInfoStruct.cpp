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

#include "pyAgeInfoStruct.h"

///////////////////////////////////////////////////////////////////////////

pyAgeInfoStruct::pyAgeInfoStruct()
{
}

pyAgeInfoStruct::pyAgeInfoStruct(plAgeInfoStruct * info)
{
	fAgeInfo.CopyFrom( info );
}

pyAgeInfoStruct::~pyAgeInfoStruct()
{
}

bool pyAgeInfoStruct::operator==(const pyAgeInfoStruct &other) const
{
	return fAgeInfo.IsEqualTo( other.GetAgeInfo() );
}

/////////////////////////////////////////////////////////////////////

void pyAgeInfoStruct::CopyFrom( const pyAgeInfoStruct & other )
{
	fAgeInfo.CopyFrom( other.GetAgeInfo() );
}

void pyAgeInfoStruct::CopyFromRef( const pyAgeInfoStructRef & other )
{
	fAgeInfo.CopyFrom( other.GetAgeInfo() );
}

const char * pyAgeInfoStruct::GetAgeFilename() const
{
	return fAgeInfo.GetAgeFilename();
}

void pyAgeInfoStruct::SetAgeFilename( const char * v )
{
	fAgeInfo.SetAgeFilename( v );
}

const char * pyAgeInfoStruct::GetAgeInstanceName() const
{
	return fAgeInfo.GetAgeInstanceName();
}

void pyAgeInfoStruct::SetAgeInstanceName( const char * v )
{
	fAgeInfo.SetAgeInstanceName( v );
}

const char * pyAgeInfoStruct::GetAgeUserDefinedName() const
{
	return fAgeInfo.GetAgeUserDefinedName();
}

void pyAgeInfoStruct::SetAgeUserDefinedName( const char * v )
{
	fAgeInfo.SetAgeUserDefinedName( v );
}

const char * pyAgeInfoStruct::GetAgeDescription() const
{
	return fAgeInfo.GetAgeDescription();
}

void pyAgeInfoStruct::SetAgeDescription( const char * v )
{
	fAgeInfo.SetAgeDescription( v );
}

const char * pyAgeInfoStruct::GetAgeInstanceGuid() const
{
	fAgeInstanceGuidStr = fAgeInfo.GetAgeInstanceGuid()->AsStdString();
	return fAgeInstanceGuidStr.c_str();
}

void pyAgeInfoStruct::SetAgeInstanceGuid( const char * guid )
{
	fAgeInfo.SetAgeInstanceGuid( &plUUID( guid ) );
}

Int32 pyAgeInfoStruct::GetAgeSequenceNumber() const
{
	return fAgeInfo.GetAgeSequenceNumber();
}

void pyAgeInfoStruct::SetAgeSequenceNumber( Int32 v )
{
	fAgeInfo.SetAgeSequenceNumber( v );
}

Int32 pyAgeInfoStruct::GetAgeLanguage() const
{
	return fAgeInfo.GetAgeLanguage();
}

void pyAgeInfoStruct::SetAgeLanguage( Int32 v )
{
	fAgeInfo.SetAgeLanguage( v );
}

const char * pyAgeInfoStruct::GetDisplayName() const
{
	Int32 seq = GetAgeSequenceNumber();
	if ( seq>0 )
		xtl::format( fDisplayName, "%s (%d) %s", GetAgeUserDefinedName(), seq, GetAgeInstanceName() );
	else
		xtl::format( fDisplayName, "%s %s", GetAgeUserDefinedName(), GetAgeInstanceName() );
	return fDisplayName.c_str();
}


/////////////////////////////////////////////////////////////////////

plAgeInfoStruct pyAgeInfoStructRef::fDefaultAgeInfo; // created so a default constructor could be made for python. Do NOT use

void pyAgeInfoStructRef::CopyFrom( const pyAgeInfoStruct & other )
{
	fAgeInfo.CopyFrom( other.GetAgeInfo() );
}

void pyAgeInfoStructRef::CopyFromRef( const pyAgeInfoStructRef & other )
{
	fAgeInfo.CopyFrom( other.GetAgeInfo() );
}

const char * pyAgeInfoStructRef::GetAgeFilename() const
{
	return fAgeInfo.GetAgeFilename();
}

void pyAgeInfoStructRef::SetAgeFilename( const char * v )
{
	fAgeInfo.SetAgeFilename( v );
}

const char * pyAgeInfoStructRef::GetAgeInstanceName() const
{
	return fAgeInfo.GetAgeInstanceName();
}

void pyAgeInfoStructRef::SetAgeInstanceName( const char * v )
{
	fAgeInfo.SetAgeInstanceName( v );
}

const char * pyAgeInfoStructRef::GetAgeUserDefinedName() const
{
	return fAgeInfo.GetAgeUserDefinedName();
}

void pyAgeInfoStructRef::SetAgeUserDefinedName( const char * v )
{
	fAgeInfo.SetAgeUserDefinedName( v );
}

const char * pyAgeInfoStructRef::GetAgeInstanceGuid() const
{
	fAgeInstanceGuidStr = fAgeInfo.GetAgeInstanceGuid()->AsStdString();
	return fAgeInstanceGuidStr.c_str();
}

void pyAgeInfoStructRef::SetAgeInstanceGuid( const char * guid )
{
	fAgeInfo.SetAgeInstanceGuid( &plUUID( guid ) );
}

Int32 pyAgeInfoStructRef::GetAgeSequenceNumber() const
{
	return fAgeInfo.GetAgeSequenceNumber();
}

void pyAgeInfoStructRef::SetAgeSequenceNumber( Int32 v )
{
	fAgeInfo.SetAgeSequenceNumber( v );
}

const char * pyAgeInfoStructRef::GetDisplayName() const
{
	Int32 seq = GetAgeSequenceNumber();
	if ( seq>0 )
		xtl::format( fDisplayName, "%s (%d) %s", GetAgeUserDefinedName(), seq, GetAgeInstanceName() );
	else
		xtl::format( fDisplayName, "%s %s", GetAgeUserDefinedName(), GetAgeInstanceName() );
	return fDisplayName.c_str();
}
