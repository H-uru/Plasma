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

#ifdef HS_BUILD_FOR_WIN32

#include <rpc.h>

// Check UUID size
static_assert(sizeof(plUUID) == sizeof(GUID), "plUUID and Win32 GUID types differ in size");

void plUUID::Clear()
{
    UuidCreateNil( (GUID *)this );
}

int plUUID::CompareTo( const plUUID * v ) const
{
    RPC_STATUS s;
    return UuidCompare( (GUID *)this, (GUID *)v, &s );
}

bool plUUID::IsEqualTo( const plUUID * v ) const
{
    return ( CompareTo( v )==0 );
}

bool plUUID::IsNull() const
{
    RPC_STATUS s;
    return 1 == UuidIsNil( (GUID *)this, &s );
}

bool plUUID::FromString( const char * str )
{
    Clear();
    if ( !str )
        return false;
    return RPC_S_OK == UuidFromString( (unsigned char *)str, (GUID *)this );
}

bool plUUID::ToString( ST::string & out ) const
{
    out = "";
    unsigned char * ubuf;
    RPC_STATUS s;
    s = UuidToString( (GUID *) this, &ubuf );
    bool success = ( s==RPC_S_OK );
    if ( success )
        out = ST::string::from_latin_1( (char*)ubuf );
    RpcStringFree( &ubuf );
    return success;
}

// static
plUUID plUUID::Generate()
{
    hsAssert(sizeof(plUUID) >= sizeof(GUID), "plUUID size");
    plUUID result;
    UuidCreate( (GUID *)&result );
    return result;
}

#endif
