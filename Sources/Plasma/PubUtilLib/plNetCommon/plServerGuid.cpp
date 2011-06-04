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
#include "hsWindows.h"
#include "plServerGuid.h"
#include "../pnMessage/plMessage.h"
#include "../PubUtilLib/plStreamLogger/plStreamLogger.h"
#if HS_BUILD_FOR_WIN32
#include <process.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////

#ifdef HS_BUILD_FOR_WIN32

// Taken from plUnifiedTime, in turn taken from python source.
// TODO: Move this down to CoreLib someday (and rename it maybe).
#define MAGICWINDOWSOFFSET ((__int64)11644473600)
static UInt32 SecsSinceUNIXEpoch()
{
	FILETIME ft;
    GetSystemTimeAsFileTime(&ft);   /* 100 ns blocks since 01-Jan-1641 */
	__int64 ff,ffsecs;
    ff = *(__int64*)(&ft);
	ffsecs = ff/(__int64)10000000;
	return (UInt32)(ffsecs-MAGICWINDOWSOFFSET);
}

#else

static UInt32 SecsSinceUNIXEpoch()
{
	struct timeval tv;
	gettimeofday(&tv, nil);
	return tv.tv_sec;
}

#endif


////////////////////////////////////////////////////////////////////

UInt32	plServerGuid::fGuidSeed = 0;

plServerGuid::plServerGuid()
{
	Clear();
}

plServerGuid::plServerGuid( const plServerGuid & other )
{
	Clear();
	CopyFrom( other );
}

plServerGuid::plServerGuid(const char * s)
{
	Clear();
	FromString(s);
}

plServerGuid::plServerGuid(const hsWide & v)
{
	Clear();
	FromWide( v );
}

plServerGuid& plServerGuid::operator=( const plServerGuid & rhs )
{
	CopyFrom(rhs);
	return *this;
}


bool operator==(const plServerGuid & X, const plServerGuid & Y)
{
	return memcmp(X.N,Y.N,plServerGuid::kGuidBytes)==0;
}
bool operator!=(const plServerGuid & X, const plServerGuid & Y)
{
	return memcmp(X.N,Y.N,plServerGuid::kGuidBytes)!=0;
}
bool operator<(const plServerGuid & X, const plServerGuid & Y)
{
	return memcmp(X.N,Y.N,plServerGuid::kGuidBytes)<0;
}


hsWide plServerGuid::AsWide() const
{
	return fWide;
}

void plServerGuid::FromWide( const hsWide & v )
{
	fWide = v;
}


bool plServerGuid::IsSet() const
{
	return N[0]||N[1]||N[2]||N[3]||N[4]||N[5]||N[6]||N[7];
}

bool plServerGuid::IsEqualTo(const plServerGuid * other) const
{
	return (*this)==(*other);
}

const char * plServerGuid::AsString() const
{
	static char str[kGuidBytes*2+1];
	sprintf(str,"%02X%02X%02X%02X%02X%02X%02X%02X",N[0],N[1],N[2],N[3],N[4],N[5],N[6],N[7]);
	return str;
}

std::string plServerGuid::AsStdString( void ) const
{
	std::string str;
	str.resize(kGuidBytes*2+1);
	int n = sprintf(const_cast<char*>(str.data()),"%02X%02X%02X%02X%02X%02X%02X%02X",N[0],N[1],N[2],N[3],N[4],N[5],N[6],N[7]);
	str.resize(n);
	return str;
}


static unsigned char hexValues[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


bool plServerGuid::FromString(const char * s)
{
	if (!s || (s && strlen(s)!=kGuidBytes*2 ) )
	{
		Clear();
		return false;
	}
	N[0]  = hexValues[s[0]]<<4;
	N[0] += hexValues[s[1]];
	N[1]  = hexValues[s[2]]<<4;
	N[1] += hexValues[s[3]];
	N[2]  = hexValues[s[4]]<<4;
	N[2] += hexValues[s[5]];
	N[3]  = hexValues[s[6]]<<4;
	N[3] += hexValues[s[7]];
	N[4]  = hexValues[s[8]]<<4;
	N[4] += hexValues[s[9]];
	N[5]  = hexValues[s[10]]<<4;
	N[5] += hexValues[s[11]];
	N[6]  = hexValues[s[12]]<<4;
	N[6] += hexValues[s[13]];
	N[7]  = hexValues[s[14]]<<4;
	N[7] += hexValues[s[15]];
	return true;
}


void plServerGuid::Read(hsStream * s, hsResMgr*)
{
	s->LogSubStreamStart("push me");
	s->LogSubStreamPushDesc("plServerGuid");
	plMsgCArrayHelper::Peek(N,kGuidBytes,s);
	s->LogSubStreamEnd();
}

void plServerGuid::Write(hsStream * s, hsResMgr*)
{
	plMsgCArrayHelper::Poke(N,kGuidBytes,s);
}

void plServerGuid::CopyFrom(const plServerGuid & other)
{
	memcpy(N,other.N,kGuidBytes);
}

void plServerGuid::CopyFrom(const plServerGuid * other)
{
	if(other)
		memcpy(N,other->N,kGuidBytes);
	else
		Clear();
}

void plServerGuid::Clear()
{
	memset(N,0,kGuidBytes);
}

void plServerGuid::SetGuidSeed(UInt32 seed)
{
	fGuidSeed = seed;
}

plServerGuid plServerGuid::GenerateGuid()
{
//	 |  N[0] |  N[1] |  N[2] |  N[3] |  N[4] |  N[5] |  N[6] |  N[7] |
//	 43210987654321098765432109876543210987654321098765432109876543210
//	 01234567890123456789012345678901234567890123456789012345678901234
//	 64              48              32              16              0
//	 |     fGuidSeed         |    Current Time       |    Counter    |
//
//	fGuidSeed:		24 bits	(settable. default is getpid())
//	Current Time:	24 bits (seconds. ~8.5 year cycle)
//	Counter:		16 bits (always increasing per process)

	static UInt16	StaticCounter = 0;
	if (!fGuidSeed)
	{
		hsStatusMessage( "fGuidSeed not set yet. Cannot generate a reliable guid! Setting fGuidSeed=getpid()." );
//		hsAssert(fGuidSeed,"fGuidSeed not set yet. Cannot generate a reliable guid.\nIgnoring this assert will set fGuidSeed=getpid().");
		fGuidSeed = getpid();
	}

	UInt32 currTime = SecsSinceUNIXEpoch();

	plServerGuid guid;
	guid.N[0] = (UInt8)((fGuidSeed & 0x00FF0000)>>16);
	guid.N[1] = (UInt8)((fGuidSeed & 0x0000FF00)>> 8);
	guid.N[2] = (UInt8)(fGuidSeed & 0x000000FF);
	guid.N[3] = (UInt8)((currTime  & 0x00FF0000)>>16);
	guid.N[4] = (UInt8)((currTime  & 0x0000FF00)>> 8);
	guid.N[5] = (UInt8)(currTime  & 0x000000FF);
	guid.N[6] = (StaticCounter & 0xFF00)>> 8;
	guid.N[7] = (StaticCounter & 0x00FF);

	StaticCounter++;

	return guid;
}


////////////////////////////////////////////////////////////////////
// End.
