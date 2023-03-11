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

#include <Python.h>
#include <string_theory/format>
#include <string_theory/string_stream>

#include "pyAgeInfoStruct.h"

#include "pnEncryption/plChecksum.h"
#include "pnUUID/pnUUID.h"

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

ST::string pyAgeInfoStruct::GetAgeFilename() const
{
    return fAgeInfo.GetAgeFilename();
}

void pyAgeInfoStruct::SetAgeFilename( const ST::string & v )
{
    fAgeInfo.SetAgeFilename( v );
}

ST::string pyAgeInfoStruct::GetAgeInstanceName() const
{
    return fAgeInfo.GetAgeInstanceName();
}

void pyAgeInfoStruct::SetAgeInstanceName( const ST::string & v )
{
    fAgeInfo.SetAgeInstanceName( v );
}

ST::string pyAgeInfoStruct::GetAgeUserDefinedName() const
{
    return fAgeInfo.GetAgeUserDefinedName();
}

void pyAgeInfoStruct::SetAgeUserDefinedName( const ST::string & v )
{
    fAgeInfo.SetAgeUserDefinedName( v );
}

ST::string pyAgeInfoStruct::GetAgeDescription() const
{
    return fAgeInfo.GetAgeDescription();
}

void pyAgeInfoStruct::SetAgeDescription( const ST::string & v )
{
    fAgeInfo.SetAgeDescription( v );
}

ST::string pyAgeInfoStruct::GetAgeInstanceGuid() const
{
    fAgeInstanceGuidStr = fAgeInfo.GetAgeInstanceGuid()->AsString();
    return fAgeInstanceGuidStr;
}

void pyAgeInfoStruct::SetAgeInstanceGuid(const ST::string& guid)
{
    if (!guid.empty() && guid[0] == '@')
    {
        // if it starts with an @ then do a meta kind of GUID
        ST::string curInst = fAgeInfo.GetAgeInstanceName();
        ST::string y = curInst + guid;

        plMD5Checksum hash;
        hash.Start();
        hash.AddTo(y.size(), (uint8_t*)y.c_str());
        hash.Finish();

        const char* md5sum = hash.GetAsHexString();
        ST::string_stream ss;
        for (size_t i = 0; i < 16; i++) {
            ss << md5sum[2*i];
            ss << md5sum[(2*i)+1];

            if (i == 3 || i == 5 || i == 7 || i == 9)
                ss << '-';
        }
        plUUID instanceGuid(ss.to_string());
        fAgeInfo.SetAgeInstanceGuid(&instanceGuid);
    }
    else {
        plUUID temp(guid);
        fAgeInfo.SetAgeInstanceGuid( &temp );
    }
}

int32_t pyAgeInfoStruct::GetAgeSequenceNumber() const
{
    return fAgeInfo.GetAgeSequenceNumber();
}

void pyAgeInfoStruct::SetAgeSequenceNumber( int32_t v )
{
    fAgeInfo.SetAgeSequenceNumber( v );
}

int32_t pyAgeInfoStruct::GetAgeLanguage() const
{
    return fAgeInfo.GetAgeLanguage();
}

void pyAgeInfoStruct::SetAgeLanguage( int32_t v )
{
    fAgeInfo.SetAgeLanguage( v );
}

ST::string pyAgeInfoStruct::GetDisplayName() const
{
    ST::string instance = GetAgeInstanceName();
    ST::string user = GetAgeUserDefinedName();
    bool namesEqual = (user.compare_i(instance) == 0); // Ae'gura Ae'gura

    if (namesEqual)
        return instance;
    else
    {
        int32_t seq = GetAgeSequenceNumber();
        if (seq > 0)
            return ST::format("{} ({}) {}", user, seq, instance);
        else
            return ST::format("{} {}", user, instance);
    }
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

ST::string pyAgeInfoStructRef::GetAgeFilename() const
{
    return fAgeInfo.GetAgeFilename();
}

void pyAgeInfoStructRef::SetAgeFilename( const ST::string & v )
{
    fAgeInfo.SetAgeFilename( v );
}

ST::string pyAgeInfoStructRef::GetAgeInstanceName() const
{
    return fAgeInfo.GetAgeInstanceName();
}

void pyAgeInfoStructRef::SetAgeInstanceName( const ST::string & v )
{
    fAgeInfo.SetAgeInstanceName( v );
}

ST::string pyAgeInfoStructRef::GetAgeUserDefinedName() const
{
    return fAgeInfo.GetAgeUserDefinedName();
}

void pyAgeInfoStructRef::SetAgeUserDefinedName( const ST::string & v )
{
    fAgeInfo.SetAgeUserDefinedName( v );
}

ST::string pyAgeInfoStructRef::GetAgeInstanceGuid() const
{
    fAgeInstanceGuidStr = fAgeInfo.GetAgeInstanceGuid()->AsString();
    return fAgeInstanceGuidStr;
}

void pyAgeInfoStructRef::SetAgeInstanceGuid(const ST::string& guid)
{
    plUUID tmp(guid);
    fAgeInfo.SetAgeInstanceGuid( &tmp );
}

int32_t pyAgeInfoStructRef::GetAgeSequenceNumber() const
{
    return fAgeInfo.GetAgeSequenceNumber();
}

void pyAgeInfoStructRef::SetAgeSequenceNumber( int32_t v )
{
    fAgeInfo.SetAgeSequenceNumber( v );
}

ST::string pyAgeInfoStructRef::GetDisplayName() const
{
    ST::string instance = GetAgeInstanceName();
    ST::string user = GetAgeUserDefinedName();
    bool namesEqual = (user.compare_i(instance) == 0); // Ae'gura Ae'gura

    if (namesEqual)
        return instance;
    else
    {
        int32_t seq = GetAgeSequenceNumber();
        if (seq > 0)
            return ST::format("{} ({}) {}", user, seq, instance);
        else
            return ST::format("{} {}", user, instance);
    }
}
