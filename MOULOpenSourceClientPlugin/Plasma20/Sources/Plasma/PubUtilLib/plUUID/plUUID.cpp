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
#include "plUUID.h"
#include "hsStream.h"

COMPILER_ASSERT(msizeof(Uuid, data) == msizeof(plUUID, fData));

plUUID::plUUID()
{
	Clear();
}

plUUID::plUUID( const char * s )
{
	FromString( s );
}

plUUID::plUUID( const plUUID & other )
{
	CopyFrom( &other );
}

plUUID::plUUID( const Uuid & uuid )
{
    MemCopy(fData, uuid.data, sizeof(fData));
}

void plUUID::Read( hsStream * s)
{
	s->LogSubStreamPushDesc("plUUID");
	s->Read( sizeof( fData ), (void*)fData );
}

void plUUID::Write( hsStream * s)
{
	s->Write( sizeof( fData ), (const void*)fData );
}

plUUID::operator Uuid () const {
    Uuid uuid;
    MemCopy(uuid.data, fData, sizeof(uuid.data));
    return uuid;
}

const char * plUUID::AsString() const {
    static std::string str;
    ToStdString(str);
    return str.c_str();
}

void plUUID::CopyFrom( const plUUID * v ) {
	if (!v)
		Clear();
	else
		CopyFrom(*v);
}

void plUUID::CopyFrom( const plUUID & v ) {
    MemCopy(fData, v.fData, sizeof(fData));
}

/*****************************************************************************
*
*   plCreatableUuid
*
***/

//============================================================================
plCreatableUuid::plCreatableUuid () {
}

//============================================================================
plCreatableUuid::plCreatableUuid (const plCreatableUuid & other)
: plUUID(other)
{
}

//============================================================================
plCreatableUuid::plCreatableUuid (const plUUID & other)
: plUUID(other)
{
}
