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
#include <string_theory/format>

#ifdef HS_BUILD_FOR_APPLE
#   include <AvailabilityMacros.h>
#   include <sys/time.h>
#endif

#include "plUnifiedTime.h"

#include "hsStream.h"

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


plUnifiedTime::plUnifiedTime(const struct timespec& tv)
    : fSecs(tv.tv_sec), fMicros(tv.tv_nsec / 1000), fMode(kGmt)
{}

// Note: mktime may modify src in place, so src must be passed in by value (i. e. copied).
plUnifiedTime::plUnifiedTime(Mode mode, struct tm src)
    : fSecs(mktime(&src)), fMicros(), fMode(mode)
{}

plUnifiedTime::plUnifiedTime(time_t t)
    : fSecs(t), fMicros(), fMode(kGmt)
{}

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
    struct timespec ts;

#if defined(HS_BUILD_FOR_APPLE)
#if defined(HAVE_BUILTIN_AVAILABLE) && MAC_OS_X_VERSION_MIN_REQUIRED >= 101500
    if (__builtin_available(macOS 10.15, *)) {
        // timespec_get is only supported since macOS 10.15
        int res = timespec_get(&ts, TIME_UTC);
        hsAssert(res != 0, "timespec_get failed");
    } else
#endif
    {
        struct timeval tv;
        int res = gettimeofday(&tv, nullptr);
        hsAssert(res == 0, "gettimeofday failed");

        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec * 1000;
    }
#else
    int res = timespec_get(&ts, TIME_UTC);
    hsAssert(res != 0, "timespec_get failed");
#endif

    fSecs = ts.tv_sec;
    fMicros = ts.tv_nsec / 1000;
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

ST::string plUnifiedTime::Print() const
{
//  short year, month, day, hour, minute, second;
//  GetTime(year, month, day, hour, minute, second);
//
//  s = ST::format("yr {} mo {} day {} hour {} min {} sec {}",
//                 year, month, day, hour, minute, second);

    return Format("%c");
}

ST::string plUnifiedTime::PrintWMillis() const
{
    return ST::format("{},s:{},ms:{}",
        Print(), (unsigned long)GetSecs(), GetMillis() );
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

double plUnifiedTime::GetSecsDouble() const
{
    return GetSecs() + GetMicros() / 1000000.0;
}

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


plUnifiedTime::operator struct timespec() const
{
    struct timespec ts;
    ts.tv_sec = fSecs;
    ts.tv_nsec = fMicros * 1000;
    return ts;
}


plUnifiedTime::operator struct tm() const
{
    return *GetTm();
}


ST::string plUnifiedTime::Format(const char * fmt) const
{
    char buf[128];
    struct tm * t = IGetTime(&fSecs);
    if (t == nullptr ||
        !strftime(buf, sizeof(buf), fmt, t))
        return {};
    return ST::string::from_utf8(buf);
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
