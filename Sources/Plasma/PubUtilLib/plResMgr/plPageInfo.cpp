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
#include "plPageInfo.h"
#include "hsUtils.h"
#include "hsStream.h"
#include "../pnKeyedObject/plUoid.h"
#include "plVersion.h"

static UInt32		sCurrPageInfoVersion = 6;

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
	fAge = fPage = nil;
	fLocation.Invalidate();
	SetMajorVersion(plVersion::GetMajorVersion());
	fClassVersions.clear();
	fChecksum = 0;
	fDataStart = fIndexStart = 0;
}

plPageInfo::~plPageInfo()
{
	SetStrings( nil, nil );
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

void	plPageInfo::ISetFrom( const plPageInfo &src )
{
	fLocation = src.fLocation;

	SetStrings( src.fAge, src.fPage );
	fMajorVersion = src.fMajorVersion;
	fClassVersions = src.fClassVersions;
	fChecksum = src.fChecksum;
	fDataStart = src.fDataStart;
	fIndexStart = src.fIndexStart;
}

void	plPageInfo::SetStrings( const char *age, const char *page )
{
	delete [] fAge;
	delete [] fPage;

	fAge = ( age == nil ) ? nil : hsStrcpy( age );
	fPage = ( page == nil ) ? nil : hsStrcpy( page );
}

void	plPageInfo::SetLocation( const plLocation &loc )
{
	fLocation = loc;
}

void plPageInfo::AddClassVersion(UInt16 classIdx, UInt16 version)
{
	ClassVersion cv;
	cv.Class = classIdx;
	cv.Version = version;
	fClassVersions.push_back(cv);
}

const plLocation& plPageInfo::GetLocation() const
{
	return fLocation;
}

void plPageInfo::Read( hsStream *s )
{
	delete [] fAge;
	delete [] fPage;

	IInit();

	// 5 is the earliest version since we began working again on the P20 codebase in Sep 2005,
	// after Uru's online component was cancelled in Feb 2004, so I've removed support for
	// anything prior to that to clean things up a bit.
	UInt32 version = s->ReadSwap32();
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
			delete s->ReadSafeString(); // fChapter was never used, and always "District".
		fPage = s->ReadSafeString();

		s->ReadSwap( &fMajorVersion );

		if (version < 6)
		{
			UInt16 unusedMinorVersion;
			s->ReadSwap(&unusedMinorVersion);
			Int32 unusedReleaseVersion;
			s->ReadSwap(&unusedReleaseVersion); // This was always zero... yanked.
			UInt32 unusedFlags;
			s->ReadSwap(&unusedFlags);
		}

		s->ReadSwap( &fChecksum );
		s->ReadSwap( &fDataStart );
		s->ReadSwap( &fIndexStart );
	}

	if (version >= 6)
	{
		UInt16 numClassVersions = s->ReadSwap16();
		fClassVersions.reserve(numClassVersions);
		for (UInt16 i = 0; i < numClassVersions; i++)
		{
			ClassVersion cv;
			cv.Class = s->ReadSwap16();
			cv.Version = s->ReadSwap16();
			fClassVersions.push_back(cv);
		}
	}
}

void	plPageInfo::Write( hsStream *s )
{
	s->WriteSwap32( sCurrPageInfoVersion );
	fLocation.Write( s );
	s->WriteSafeString( fAge );
	s->WriteSafeString( fPage );
	s->WriteSwap( fMajorVersion );
	s->WriteSwap( fChecksum );
	s->WriteSwap( fDataStart );
	s->WriteSwap( fIndexStart );
	UInt16 numClassVersions = UInt16(fClassVersions.size());
	s->WriteSwap16(numClassVersions);
	for (UInt16 i = 0; i < numClassVersions; i++)
	{
		ClassVersion& cv = fClassVersions[i];
		s->WriteSwap16(cv.Class);
		s->WriteSwap16(cv.Version);
	}
}

//// IsValid /////////////////////////////////////////////////////////////////
//	Just a simple test for now.

hsBool	plPageInfo::IsValid( void ) const
{
	return fLocation.IsValid();
}
