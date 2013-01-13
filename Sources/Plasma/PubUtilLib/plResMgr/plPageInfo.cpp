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
#include "plPageInfo.h"

#include "hsStream.h"
#include "pnKeyedObject/plUoid.h"
#include "plVersion.h"

static uint32_t s_MinPageInfoVersion = 6;
static uint32_t s_CurrPageInfoVersion = 7;

//// Constructor/Destructor //////////////////////////////////////////////////
plPageInfo::plPageInfo()
{
    IInit();
}

plPageInfo::plPageInfo( const plLocation &loc )
{
    IInit();
    fLocation = loc;
}

void plPageInfo::IInit()
{
    fAge = fPage = "";
    fLocation.Invalidate();
    fFlags = 0;
    SetMajorVersion(plVersion::GetMajorVersion());
    fClassVersions.clear();
    fChecksum = 0;
    fDataStart = fIndexStart = 0;
}

plPageInfo::~plPageInfo()
{
    SetStrings("", "");
}

plPageInfo::plPageInfo( const plPageInfo &src )
{
    IInit();
    ISetFrom( src );
}

plPageInfo &plPageInfo::operator=( const plPageInfo &src )
{
    ISetFrom( src );
    return *this;
}

void    plPageInfo::ISetFrom( const plPageInfo &src )
{
    fLocation = src.fLocation;
    fFlags = src.fFlags;
    SetStrings( src.fAge, src.fPage );
    fMajorVersion = src.fMajorVersion;
    fClassVersions = src.fClassVersions;
    fChecksum = src.fChecksum;
    fDataStart = src.fDataStart;
    fIndexStart = src.fIndexStart;
}

void    plPageInfo::SetStrings(const plString& age, const plString& page)
{
    fAge = age;
    fPage = page;
}

void    plPageInfo::SetLocation( const plLocation &loc )
{
    fLocation = loc;
}

void plPageInfo::AddClassVersion(uint16_t classIdx, uint16_t version)
{
    ClassVersion cv;
    cv.Class = classIdx;
    cv.Version = version;
    fClassVersions.push_back(cv);
}

void plPageInfo::Read( hsStream *s )
{
    IInit();

    // Only support data from Myst Online: Uru Live Again and later
    // This means page info versions 6 and above. 5 is just way too old.
    uint32_t version = s->ReadLE32(); // HINT: Plasma21 reads an LE16
    if (version > s_CurrPageInfoVersion)
    {
        hsAssert(false, plString::Format("PageInfo version %d is too new", version).c_str());
        return;
    }
    else if (version < s_MinPageInfoVersion)
    {
        hsAssert(false, plString::Format("PageInfo version %d is too old", version).c_str());
        return;
    }

    fLocation.Read(s);
    fAge = s->ReadSafeString_TEMP();
    fPage = s->ReadSafeString_TEMP();
    s->LogReadLE(&fMajorVersion, "major version");
    if (version > 6)
        s->LogReadLE(&fFlags, "page flags"); // restored in 7
    s->LogReadLE(&fChecksum, "checksum");
    s->LogReadLE(&fDataStart, "data start offset");
    s->LogReadLE(&fIndexStart, "index start offset");

    uint16_t numClassVersions = s->ReadLE16();
    fClassVersions.reserve(numClassVersions);
    for (uint16_t i = 0; i < numClassVersions; i++)
    {
        ClassVersion cv;
        cv.Class = s->ReadLE16();
        cv.Version = s->ReadLE16();
        fClassVersions.push_back(cv);
    }
}

void    plPageInfo::Write( hsStream *s )
{
    // Try to maintain some semblence of compatibility. If there are no page flags,
    // let's dump out a Cyan-compatible PRP.
    if (fFlags == 0)
        s->WriteLE32( s_MinPageInfoVersion );
    else
        s->WriteLE32( s_CurrPageInfoVersion );
    fLocation.Write( s );
    s->WriteSafeString( fAge );
    s->WriteSafeString( fPage );
    s->WriteLE( fMajorVersion );
    if (fFlags != 0)
        s->WriteLE( fFlags );
    s->WriteLE( fChecksum );
    s->WriteLE( fDataStart );
    s->WriteLE( fIndexStart );

    s->WriteLE16(fClassVersions.size());
    for (auto it = fClassVersions.begin(); it != fClassVersions.end(); ++it)
    {
        s->WriteLE16((*it).Class);
        s->WriteLE16((*it).Version);
    }
}

//// IsValid /////////////////////////////////////////////////////////////////
//  Just a simple test for now.

bool    plPageInfo::IsValid( void ) const
{
    return fLocation.IsValid();
}
