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

#include "plVersion.h"

#include "pnKeyedObject/plUoid.h"

static uint32_t       sCurrPageInfoVersion = 6;

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

    SetStrings( src.fAge, src.fPage );
    fMajorVersion = src.fMajorVersion;
    fClassVersions = src.fClassVersions;
    fChecksum = src.fChecksum;
    fDataStart = src.fDataStart;
    fIndexStart = src.fIndexStart;
}

void    plPageInfo::SetStrings(const ST::string& age, const ST::string& page)
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

    // 5 is the earliest version since we began working again on the P20 codebase in Sep 2005,
    // after Uru's online component was cancelled in Feb 2004, so I've removed support for
    // anything prior to that to clean things up a bit.
    uint32_t version = s->ReadLE32();
    if (version > sCurrPageInfoVersion || version < 5)
    {
        hsAssert( false, "Invalid header version in plPageInfo::Read()" );
        return;
    }
    if (version >= 5)
    {
        fLocation.Read( s );
        fAge = s->ReadSafeString();
        if (version < 6)
            s->ReadSafeString(); // fChapter was never used, and always "District".
        fPage = s->ReadSafeString();

        s->ReadLE16(&fMajorVersion);

        if (version < 6)
        {
            (void)s->ReadLE16(); // unused: major version
            (void)s->ReadLE32(); // unused: release version.  This was always zero... yanked.
            (void)s->ReadLE32(); // unused: flags
        }

        s->ReadLE32(&fChecksum);
        s->ReadLE32(&fDataStart);
        s->ReadLE32(&fIndexStart);
    }

    if (version >= 6)
    {
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
}

void    plPageInfo::Write( hsStream *s )
{
    s->WriteLE32( sCurrPageInfoVersion );
    fLocation.Write( s );
    s->WriteSafeString( fAge );
    s->WriteSafeString( fPage );
    s->WriteLE16(fMajorVersion);
    s->WriteLE32(fChecksum);
    s->WriteLE32(fDataStart);
    s->WriteLE32(fIndexStart);
    uint16_t numClassVersions = uint16_t(fClassVersions.size());
    s->WriteLE16(numClassVersions);
    for (uint16_t i = 0; i < numClassVersions; i++)
    {
        ClassVersion& cv = fClassVersions[i];
        s->WriteLE16(cv.Class);
        s->WriteLE16(cv.Version);
    }
}

//// IsValid /////////////////////////////////////////////////////////////////
//  Just a simple test for now.

bool    plPageInfo::IsValid() const
{
    return fLocation.IsValid();
}
