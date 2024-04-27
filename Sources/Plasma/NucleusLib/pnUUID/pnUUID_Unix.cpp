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

// Check UUID size
static_assert(sizeof(plUUID) == sizeof(uuid_t), "plUUID and uuid_t types differ in size");

struct plUUIDHelper
{
    static inline void CopyToPlasma(plUUID* dst, const uuid_t& src)
    {
        memcpy(dst->fData, src, sizeof(plUUID));

        // plUUIDs need to always be encoded in little endian, but the uuid_t struct is big endian
        // This isn't guaranteed to be 32-bit-aligned, so casting to uint16_t/uint32_t pointers is UB
        std::swap(dst->fData[0], dst->fData[3]);
        std::swap(dst->fData[1], dst->fData[2]);
        std::swap(dst->fData[4], dst->fData[5]);
        std::swap(dst->fData[6], dst->fData[7]);
    }

    static inline void CopyToNative(uuid_t& dst, const plUUID* src)
    {
        memcpy(dst, src->fData, sizeof(plUUID));

        // uuid_t struct is big endian, but plUUIDs are always encoded in little endian
        // This isn't guaranteed to be 32-bit-aligned, so casting to uint16_t/uint32_t pointers is UB
        std::swap(dst[0], dst[3]);
        std::swap(dst[1], dst[2]);
        std::swap(dst[4], dst[5]);
        std::swap(dst[6], dst[7]);
    }
};


void plUUID::Clear()
{
    uuid_t g;
    uuid_clear( g );
    plUUIDHelper::CopyToPlasma( this, g );
}

bool plUUID::IsNull() const
{
    uuid_t g;
    plUUIDHelper::CopyToNative( g, this );
    return ( uuid_is_null( g )!=0 );
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
    if (!str) {
        return false;
    }

    uuid_t g;
    if (uuid_parse(const_cast<char*>(str), g) != 0) {
        return false;
    }

    plUUIDHelper::CopyToPlasma(this, g);
    return true;
}

bool plUUID::ToString(ST::string& out) const
{
    uuid_t g;
    plUUIDHelper::CopyToNative(g, this);
    char buf[40];
    uuid_unparse_lower(g, buf);
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

#endif
