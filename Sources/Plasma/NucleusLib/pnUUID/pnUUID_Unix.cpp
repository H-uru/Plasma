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
#include "pnUUID.h"

#ifdef HS_BUILD_FOR_UNIX

#include <uuid/uuid.h>

struct plUUIDHelper
{
    static inline void CopyToPlasma( plUUID * dst, const uuid_t & src )
    {
        hsAssert( sizeof(uuid_t)==sizeof(dst->fData), "sizeof(uuid_t)!=sizeof(plUUID)" );
        memcpy( (void*)dst->fData, (const void *)src, sizeof( plUUID ) );
    }

    static inline void CopyToNative( uuid_t & dst, const plUUID * src )
    {
        hsAssert( sizeof(uuid_t)==sizeof(src->fData), "sizeof(uuid_t)!=sizeof(plUUID)" );
        memcpy( (void*)dst, (const void *)src->fData, sizeof( plUUID ) );
    }
};


void plUUID::Clear()
{
    uuid_t g;
    plUUIDHelper::CopyToNative( g, this );
    uuid_clear( g );
    plUUIDHelper::CopyToPlasma( this, g );
}

bool plUUID::IsNull() const
{
    uuid_t g;
    plUUIDHelper::CopyToNative( g, this );
    return ( uuid_is_null( g )!=0 );
}

void plUUID::CopyFrom( const plUUID * v )
{
    memcpy( (void*)fData, (const void*)v->fData, sizeof(fData) );
}

int plUUID::CompareTo( const plUUID * v ) const
{
    uuid_t ga, gb;
    plUUIDHelper::CopyToNative( ga, this );
    plUUIDHelper::CopyToNative( gb, v );
    int ans = uuid_compare( ga, gb );
    return ans;
}

bool plUUID::IsEqualTo( const plUUID * v ) const
{
    return ( CompareTo( v )==0 );
}

bool plUUID::FromString( const char * str )
{
    Clear();
    if ( !str )
        return false;
    uuid_t g;
    uuid_parse( str, g );
    plUUIDHelper::CopyToPlasma( this, g );
    return true;
}

bool plUUID::ToString( plString & out ) const
{
    uuid_t g;
    plUUIDHelper::CopyToNative( g, this );
    char buf[40];
    uuid_unparse( g, buf );
    out = buf;
    return true;
}

// static
plUUID plUUID::Generate()
{
    uuid_t g;
    uuid_generate( g );
    plUUID result;
    plUUIDHelper::CopyToPlasma( &result, g );
    return result;
}

#else

// dummy function to prevent a linker warning complaining about no public symbols if the
// contents of the file get compiled out via pre-processor
void _preventLNK4221WarningStub()
{
}

#endif
