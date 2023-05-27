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

#include <cmath>
#include <ctime>
#include "plUnifiedTime.h"
#include "hsWindows.h"
#include "hsStream.h"

#if HS_BUILD_FOR_UNIX
#include <sys/time.h>
#include <unistd.h>
#endif

#if HS_BUILD_FOR_WIN32
//
// Converts Windows Time to Unified Time
//
#define MAGICWINDOWSOFFSET ((__int64)11644473600)  // magic number, taken from Python Source
//
bool plUnifiedTime::SetFromWinFileTime(const FILETIME ft)
{
    // FILETIME resolution seems to be 0.01 sec

    __int64 ff,ffsecs;
    ff = *(__int64*)(&ft);
    ffsecs = ff/(__int64)10000000;

    if (ffsecs >= MAGICWINDOWSOFFSET)  // make sure we won't end up negatice
    {
        fSecs = (time_t)(ffsecs-MAGICWINDOWSOFFSET);
        fMicros = (uint32_t)(ff % 10000000)/10;
        return true;
    }
    else
        // before the UNIX Epoch
        return false;
}

//
// Sets the unified time to the current UTC time
//
bool plUnifiedTime::SetToUTC()
{
    FILETIME ft;

    GetSystemTimeAsFileTime(&ft);   /* 100 ns blocks since 01-Jan-1641 */
    return SetFromWinFileTime(ft);
}
#elif HS_BUILD_FOR_UNIX

//
// Sets the unified time to the current UTC time
//
bool plUnifiedTime::SetToUTC()
{
    struct timeval tv;
    
    // get the secs and micros from Jan 1, 1970
    int ret = gettimeofday(&tv, nullptr);
    if (ret == 0)
    {
        fSecs = tv.tv_sec;
        fMicros = tv.tv_usec;
        return true;
    }
    else
    {
        return false;
    }
}
#else
#error "Unified Time Not Implemented on this platform!"
#endif


struct tm * plUnifiedTime::IGetTime(const time_t * timer) const
{
    struct tm * tm = nullptr;
    switch (fMode)
    {
    case kGmt:
        tm = gmtime(timer);
        break;
    default:
        tm = localtime(timer);
    }
    if ( tm )
        tm->tm_isdst = -1;
    return tm;
}


plUnifiedTime::plUnifiedTime(const timeval & tv)
    : fMode(kGmt)
{
    *this = tv;
}

plUnifiedTime::plUnifiedTime(int mode, const struct tm & src)
    : fMode((Mode)mode), fMicros()
{
    *this = src;
}

plUnifiedTime::plUnifiedTime(time_t t)
    : fMode(kGmt)
{
    *this = t;
}

plUnifiedTime::plUnifiedTime(unsigned long t)
    : fMode(kGmt)
{
    *this = t;
}

plUnifiedTime::plUnifiedTime(int year, int month, int day, int hour, int min, int sec, unsigned long usec, int dst)
    : fMode(kGmt), fMicros()
{
    SetTime(year,month,day,hour,min,sec,usec,dst);
}

plUnifiedTime plUnifiedTime::GetCurrent(Mode mode)
{
    plUnifiedTime t;
    t.SetMode(mode);
    t.ToCurrentTime();
    return t;
}


const plUnifiedTime & plUnifiedTime::operator=(time_t src)
{
    fSecs = src;
    fMicros = 0;
    return *this;
}

const plUnifiedTime & plUnifiedTime::operator=(unsigned long src)
{
    fSecs = (time_t)src;
    fMicros = 0;
    return *this;
}

const plUnifiedTime & plUnifiedTime::operator=(const struct timeval & src)
{
    fSecs = src.tv_sec;
    fMicros = src.tv_usec;
    return *this;
}

const plUnifiedTime & plUnifiedTime::operator=(const struct tm & src)
{
    struct tm atm = src;
    fSecs = mktime(&atm);
    return *this;
}

void plUnifiedTime::SetSecsDouble(double secs)
{
    hsAssert(secs>=0, "plUnifiedTime::SetSecsDouble negative time");
    double x,y;
    x = modf(secs,&y);
    fSecs = (time_t)y;
    fMicros = (uint32_t)(x*1000000);
}


void plUnifiedTime::ToCurrentTime()
{
    SetToUTC();
}

bool plUnifiedTime::SetGMTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec)
{
    if( !SetTime( year, month, day, hour, minute, second, usec, 0 ) )
        return false;

    fSecs -= IGetLocalTimeZoneOffset();
    fMode = kGmt;

    return true;
}

bool plUnifiedTime::SetTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec, int dst)
{
    struct tm atm;
    atm.tm_sec = second;
    atm.tm_min = minute;
    atm.tm_hour = hour;
    atm.tm_mday = day;
    atm.tm_mon = month - 1;
    atm.tm_year = year - 1900;
    atm.tm_isdst = dst;
    fSecs = mktime(&atm);
    if (fSecs == -1)
        return false;
    if (fMicros >= 1000000)
        return false;
    fMicros = usec;
    fMode = kLocal;
    return true;
}

bool plUnifiedTime::GetTime(short &year, short &month, short &day, short &hour, short &minute, short &second) const
{
    struct tm* time = IGetTime(&fSecs);
    if (!time)
        return false;
    year = time->tm_year+1900;
    month = time->tm_mon+1;
    day = time->tm_mday;
    hour = time->tm_hour;
    minute = time->tm_min;
    second = time->tm_sec;
    return true;
}

const char* plUnifiedTime::Print() const
{
    static std::string s;
//  short year, month, day, hour, minute, second;
//  GetTime(year, month, day, hour, minute, second);
//
//  s = ST::format("yr {} mo {} day {} hour {} min {} sec {}",
//                 year, month, day, hour, minute, second);

    s = Format("%c");
    return s.c_str();
}

const char* plUnifiedTime::PrintWMillis() const
{
    static ST::string s;
    s = ST::format("{},s:{},ms:{}",
        Print(), (unsigned long)GetSecs(), GetMillis() );
    return s.c_str();
}

struct tm * plUnifiedTime::GetTm(struct tm * ptm) const
{
    if (ptm != nullptr)
    {
        *ptm = *IGetTime(&fSecs);
        return ptm;
    }
    else
        return IGetTime(&fSecs);
}

int plUnifiedTime::GetYear() const
{ 
    return GetTm() ? GetTm()->tm_year + 1900 : 0; 
}
int plUnifiedTime::GetMonth() const
{ 
    return GetTm() ? GetTm()->tm_mon + 1 : 0; 
}
int plUnifiedTime::GetDay() const
{ 
    return GetTm() ? GetTm()->tm_mday : 0; 
}
int plUnifiedTime::GetHour() const
{ 
    return GetTm() ? GetTm()->tm_hour : 0; 
}
int plUnifiedTime::GetMinute() const
{ 
    return GetTm() ? GetTm()->tm_min : 0; 
}
int plUnifiedTime::GetSecond() const
{ 
    return GetTm() ? GetTm()->tm_sec : 0; 
}
int plUnifiedTime::GetDayOfWeek() const
{ 
    return GetTm() ? GetTm()->tm_wday : 0; 
}
int plUnifiedTime::GetMillis() const
{ 
    return fMicros/1000;
}

#ifdef _MSC_VER
#   pragma optimize( "g", off )    // disable global optimizations
#endif
double plUnifiedTime::GetSecsDouble() const
{
    double ret = GetSecs() + GetMicros() / 1000000.0;
    return ret;
}
#ifdef _MSC_VER
#   pragma optimize( "", on )  // restore optimizations to their defaults
#endif

void plUnifiedTime::Read(hsStream* s)
{
    fSecs = (time_t)s->ReadLE32();
    s->ReadLE32(&fMicros);
    // preserve fMode
}

void plUnifiedTime::Write(hsStream* s) const
{
    s->WriteLE32((uint32_t)fSecs);
    s->WriteLE32(fMicros);
    // preserve fMode
}

const plUnifiedTime & plUnifiedTime::operator-=(const plUnifiedTime & rhs)
{
    // carry if needed
    if ((*this).fMicros < rhs.fMicros)
    {
        (*this).fSecs--;
        (*this).fMicros += 1000000;
    }
    (*this).fMicros -= rhs.fMicros;
    (*this).fSecs -= rhs.fSecs;
    return *this;
}

const plUnifiedTime & plUnifiedTime::operator+=(const plUnifiedTime & rhs)
{
    (*this).fMicros += rhs.fMicros;
    // carry if needed
    if ((*this).fMicros >= 1000000)
    {
        (*this).fSecs++;
        (*this).fMicros -= 1000000;
    }
    (*this).fSecs += rhs.fSecs;
    return *this;
}

bool plUnifiedTime::operator==(const plUnifiedTime & rhs) const
{
    return ((fSecs == rhs.fSecs) && (fMicros == rhs.fMicros));
}
bool plUnifiedTime::operator!=(const plUnifiedTime & rhs) const
{
    return ((fSecs != rhs.fSecs) || (fMicros != rhs.fMicros));
}
bool plUnifiedTime::operator <(const plUnifiedTime & rhs) const
{
    return ((fSecs < rhs.fSecs) || ((fSecs == rhs.fSecs) && (fMicros < rhs.fMicros)));
}
bool plUnifiedTime::operator >(const plUnifiedTime & rhs) const
{
    return ((fSecs > rhs.fSecs) || ((fSecs == rhs.fSecs) && (fMicros > rhs.fMicros)));
}
bool plUnifiedTime::operator<=(const plUnifiedTime & rhs) const
{
    return (*this<rhs) || (*this==rhs);
}
bool plUnifiedTime::operator>=(const plUnifiedTime & rhs) const
{
    return (*this>rhs) || (*this==rhs);
}


plUnifiedTime::operator timeval() const
{
#if HS_BUILD_FOR_WIN32
    // tv_secs should be a time_t, but on Windows it is a long
    struct timeval t = {(long)fSecs, (long)fMicros};
#else
    struct timeval t = {fSecs, (suseconds_t)fMicros};
#endif
    return t;
}


plUnifiedTime::operator struct tm() const
{
    return *GetTm();
}


std::string plUnifiedTime::Format(const char * fmt) const
{
    char buf[128];
    struct tm * t = IGetTime(&fSecs);
    if (t == nullptr ||
        !strftime(buf, sizeof(buf), fmt, t))
        buf[0] = '\0';
    return std::string(buf);
}


plUnifiedTime operator -(const plUnifiedTime & left, const plUnifiedTime & right)
{
    plUnifiedTime ans = left;
    ans -= right;
    return ans;
}

plUnifiedTime operator +(const plUnifiedTime & left, const plUnifiedTime & right)
{
    plUnifiedTime ans = left;
    ans += right;
    return ans;
}

bool operator <(const plUnifiedTime & time, int secs)
{
    return (time.GetSecs()<secs);
}

/// Local time zone offset stuff

int32_t   plUnifiedTime::fLocalTimeZoneOffset = -1;

int32_t   plUnifiedTime::IGetLocalTimeZoneOffset()
{
    static bool     inited = false;

    if( !inited )
    {
        inited = true;

        // Calculate the difference between local time and GMT for this system currently
        // Taken from devx.com from an article written by Danny Kalev
        // http://gethelp.devx.com/techtips/cpp_pro/10min/2001/10min1200-3.asp

        time_t  currLocalTime = time(nullptr);  // current local time

        struct tm   local = *gmtime( &currLocalTime );  // convert curr to GMT, store as tm
        
        time_t      utc = mktime( &local ); // convert GMT tm to GMT time_t

        double diffInSecs = difftime( utc, currLocalTime );

        fLocalTimeZoneOffset = (int32_t)diffInSecs;
    }

    return fLocalTimeZoneOffset;
}

//
// static helper, return difference timeA-timeB, may be negative
//
double plUnifiedTime::GetTimeDifference(const plUnifiedTime& timeA, const plUnifiedTime& timeB)
{
    bool neg = (timeB > timeA);
    plUnifiedTime timeDiff = neg ? (timeB - timeA) : (timeA - timeB);   // always positive
    double t = (float)(neg ? timeDiff.GetSecsDouble() * -1. : timeDiff.GetSecsDouble());
    return t;
}
