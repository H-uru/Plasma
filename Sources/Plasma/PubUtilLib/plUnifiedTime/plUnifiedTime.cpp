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
#include <float.h>
#include "plUnifiedTime.h"
#include "hsStlUtils.h"
#include "hsWindows.h"

#if HS_BUILD_FOR_UNIX
#include <sys/time.h>
#include <unistd.h>
#endif

#if HS_BUILD_FOR_WIN32
#include <sys/timeb.h>	// for timeb
#endif

#include "hsUtils.h"

#include <time.h>
#include "hsStream.h"


#if HS_BUILD_FOR_WIN32
//
// Converts Windows Time to Unified Time
//
#define MAGICWINDOWSOFFSET ((__int64)11644473600)  // magic number, taken from Python Source
//
hsBool plUnifiedTime::SetFromWinFileTime(const FILETIME ft)
{
    // FILETIME resolution seems to be 0.01 sec

	__int64 ff,ffsecs;
    ff = *(__int64*)(&ft);
	ffsecs = ff/(__int64)10000000;

	if (ffsecs >= MAGICWINDOWSOFFSET)  // make sure we won't end up negatice
	{
		fSecs = (UInt32)(ffsecs-MAGICWINDOWSOFFSET);
		fMicros = (UInt32)(ff % 10000000)/10;
		return true;
	}
	else
		// before the UNIX Epoch
		return false;
}

//
// Sets the unified time to the current UTC time
//
hsBool plUnifiedTime::SetToUTC()
{
	FILETIME ft;

    GetSystemTimeAsFileTime(&ft);   /* 100 ns blocks since 01-Jan-1641 */
	return SetFromWinFileTime(ft);
}
#elif HS_BUILD_FOR_UNIX

//
// Sets the unified time to the current UTC time
//
hsBool plUnifiedTime::SetToUTC()
{
	struct timeval tv;
	
	// get the secs and micros from Jan 1, 1970
	int ret = gettimeofday(&tv, nil);
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
	struct tm * tm = nil;
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

plUnifiedTime::plUnifiedTime(plUnifiedTime_CtorNow,int mode)
{
	SetMode((Mode)mode);
	ToCurrentTime();
}


plUnifiedTime::plUnifiedTime(const timeval & tv)
:	fMode(kGmt)
{
	*this = tv;
}

plUnifiedTime::plUnifiedTime(int mode, const struct tm & src)
:	fMode((Mode)mode)
{
	*this = src;
}

plUnifiedTime::plUnifiedTime(time_t t)
:	fMode(kGmt)
{
	*this = t;
}

plUnifiedTime::plUnifiedTime(unsigned long t)
:	fMode(kGmt)
{
	*this = t;
}

plUnifiedTime::plUnifiedTime(int year, int month, int day, int hour, int min, int sec, unsigned long usec, int dst)
:	fMode(kGmt)
{
	SetTime(year,month,day,hour,min,sec,usec,dst);
}

plUnifiedTime::plUnifiedTime(int mode, const char * buf, const char * fmt)
:	fMode((Mode)mode)
{
	FromString(buf,fmt);
}

plUnifiedTime::plUnifiedTime(const plUnifiedTime & src)
:	fMode(src.fMode)
{
	*this = src;
}

plUnifiedTime::plUnifiedTime(const plUnifiedTime * src)
:	fMode(src->fMode)
{
	*this = *src;
}

plUnifiedTime plUnifiedTime::GetCurrentTime(Mode mode)
{
	plUnifiedTime t;
	t.SetMode(mode);
	t.ToCurrentTime();
	return t;
}


const plUnifiedTime & plUnifiedTime::operator=(const plUnifiedTime & src)
{
	fSecs = src.fSecs;
	fMicros = src.fMicros;
	fMode = src.fMode;
	return *this;
}

const plUnifiedTime & plUnifiedTime::operator=(const plUnifiedTime * src)
{
	return operator=(*src);
}

const plUnifiedTime & plUnifiedTime::operator=(time_t src)
{
	fSecs = src;
	fMicros = 0;
	return *this;
}

const plUnifiedTime & plUnifiedTime::operator=(unsigned long src)
{
	fSecs = src;
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
	fSecs = mktime(&atm);	// this won't work after 2030 something, sorry
	return *this;
}

void plUnifiedTime::SetSecsDouble(double secs)
{
	hsAssert(secs>=0, "plUnifiedTime::SetSecsDouble negative time");
	double x,y;
	x = modf(secs,&y);
	fSecs = (UInt32)y;
	fMicros = (UInt32)(x*1000000);
}

void plUnifiedTime::FromMillis(UInt32 millis)
{
	fSecs = millis/1000;
	fMicros = 0;
}


void plUnifiedTime::ToCurrentTime()
{
	SetToUTC();
}

hsBool plUnifiedTime::SetGMTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec, int dst)
{
	if( !SetTime( year, month, day, hour, minute, second, usec, dst ) )
		return false;

	fSecs -= IGetLocalTimeZoneOffset();
	fMode = kGmt;

	return true;
}

hsBool plUnifiedTime::SetTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec, int dst)
{
	struct tm atm;
	atm.tm_sec = second;
	atm.tm_min = minute;
	atm.tm_hour = hour;
	atm.tm_mday = day;
	atm.tm_mon = month - 1;
	atm.tm_year = year - 1900;
	atm.tm_isdst = dst;
	fSecs = mktime(&atm);	// this won't work after 2030 something, sorry
	if (fSecs == -1)
		return false;
	if (fMicros >= 1000000)
		return false;
	fMicros = usec;
	fMode = kLocal;
	return true;
}

hsBool plUnifiedTime::GetTime(short &year, short &month, short &day, short &hour, short &minute, short &second) const
{
	struct tm* time = IGetTime((const time_t *)&fSecs);
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
//	short year, month, day, hour, minute, second;
//	GetTime(year, month, day, hour, minute, second);
//
//	xtl::format(s,"yr %d mo %d day %d hour %d min %d sec %d",
//		year, month, day, hour, minute, second);

	s = Format("%c");
	return s.c_str();
}

const char* plUnifiedTime::PrintWMillis() const
{
	static std::string s;
	xtl::format(s,"%s,s:%d,ms:%d",
		Print(), GetSecs(), GetMillis() );
	return s.c_str();
}

struct tm * plUnifiedTime::GetTm(struct tm * ptm) const
{
	if (ptm != nil)
	{
		*ptm = *IGetTime((const time_t *)&fSecs);
		return ptm;
	}
	else
		return IGetTime((const time_t *)&fSecs);
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

#pragma optimize( "g", off )	// disable global optimizations
double plUnifiedTime::GetSecsDouble() const
{
	hsDoublePrecBegin
	double ret = GetSecs() + GetMicros() / 1000000.0;
	hsDoublePrecEnd
	return ret;
}
#pragma optimize( "", on )	// restore optimizations to their defaults

UInt32 plUnifiedTime::AsMillis()
{
	return GetSecs()*1000;
}

void plUnifiedTime::Read(hsStream* s)
{
	s->LogSubStreamStart("UnifiedTime");
	s->LogReadSwap(&fSecs,"Seconds");
	s->LogReadSwap(&fMicros,"MicroSeconds");
	s->LogSubStreamEnd();
	// preserve fMode
}

void plUnifiedTime::Write(hsStream* s) const
{
	s->WriteSwap(fSecs);
	s->WriteSwap(fMicros);
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
	struct timeval t = {fSecs, fMicros};
	return t;
}


plUnifiedTime::operator struct tm() const
{
	return *GetTm();
}


std::string plUnifiedTime::Format(const char * fmt) const
{
	char buf[128];
	struct tm * t = IGetTime((const time_t *)&fSecs);
	if (t == nil ||
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


////////////////////////////////////////////////////////////////////
// FromString

#if !defined(HS_BUILD_FOR_UNIX)

namespace pvt_strptime
{
	
	//
	// based on glibc's strptime
	//
	
#define __isleap(year)	\
	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
	
#define match_char(ch1, ch2) if (ch1 != ch2) return NULL
#define match_string(cs1, s2) \
	(_strnicmp((cs1), (s2), strlen (cs1)) ? 0 : ((s2) += strlen (cs1), 1))
#define get_number(from, to, n) \
	do {									      \
    int __n = n;							      \
    val = 0;								      \
    while (*rp == ' ')							      \
	++rp;								      \
    if (*rp < '0' || *rp > '9')						      \
	return NULL;							      \
    do {								      \
	val *= 10;							      \
	val += *rp++ - '0';						      \
    } while (--__n > 0 && val * 10 <= to && *rp >= '0' && *rp <= '9');	      \
    if (val < from || val > to)						      \
	return NULL;							      \
	} while (0)
#define recursive(new_fmt) \
	(*(new_fmt) != '\0'							      \
	&& (rp = strptime_internal (rp, (new_fmt), tm, mode)) != NULL)
	
	static char const weekday_name[][10] =
	{
		"Sunday", "Monday", "Tuesday", "Wednesday",
			"Thursday", "Friday", "Saturday"
	};
	static char const ab_weekday_name[][4] =
	{
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char const month_name[][10] =
	{
		"January", "February", "March", "April", "May", "June",
			"July", "August", "September", "October", "November", "December"
	};
	static char const ab_month_name[][4] =
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
			"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
#define HERE_D_T_FMT "%m/%d/%y %H:%M:%S"
#define HERE_D_FMT "%m/%d/%y"
#define HERE_AM_STR "AM"
#define HERE_PM_STR "PM"
#define HERE_T_FMT_AMPM "%I:%M:%S %p"
#define HERE_T_FMT "%H:%M:%S"
	
	const unsigned short int __mon_yday[2][13] =
	{
		/* Normal years.  */
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
			/* Leap years.  */
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
	};
	
	
	
	static struct tm *
		time_r(
		const time_t *t,
		struct tm *tp,
		int mode)
	{
		struct tm *l = 0;
		switch (mode)
		{
		case plUnifiedTime::kGmt:
			l = gmtime(t);
			break;
		default:
			l = localtime(t);
		}
		if (! l)
			return 0;
		*tp = *l;
		tp->tm_isdst = -1;
		return tp;
	}
	
	/* Compute the day of the week.  */
	static void
		day_of_the_week (struct tm *tm)
	{
	/* We know that January 1st 1970 was a Thursday (= 4).  Compute the
	the difference between this data in the one on TM and so determine
		the weekday.  */
		int corr_year = 1900 + tm->tm_year - (tm->tm_mon < 2);
		int wday = (-473
			+ (365 * (tm->tm_year - 70))
			+ (corr_year / 4)
			- ((corr_year / 4) / 25) + ((corr_year / 4) % 25 < 0)
			+ (((corr_year / 4) / 25) / 4)
			+ __mon_yday[0][tm->tm_mon]
			+ tm->tm_mday - 1);
		tm->tm_wday = ((wday % 7) + 7) % 7;
	}
	
	/* Compute the day of the year.  */
	static void
		day_of_the_year (struct tm *tm)
	{
		tm->tm_yday = (__mon_yday[__isleap (1900 + tm->tm_year)][tm->tm_mon]
			+ (tm->tm_mday - 1));
	}
	
	
	
	static char * strptime_internal(
		const char * rp,
		const char * fmt,
		struct tm * tm,
		int mode)
	{
		const char *rp_backup;
		int cnt;
		size_t val;
		int have_I, is_pm;
		int century, want_century;
		int have_wday, want_xday;
		int have_yday;
		int have_mon, have_mday;
		
		have_I = is_pm = 0;
		century = -1;
		want_century = 0;
		
		have_wday = want_xday = have_yday = have_mon = have_mday = 0;
		
		while (*fmt != '\0')
		{
		/* A white space in the format string matches 0 more or white
			space in the input string.  */
			if (isspace (*fmt))
			{
				while (isspace (*rp))
					++rp;
				++fmt;
				continue;
			}
			
			/* Any character but `%' must be matched by the same character
			in the iput string.  */
			if (*fmt != '%')
			{
				match_char (*fmt++, *rp++);
				continue;
			}
			
			++fmt;
			/* We need this for handling the `E' modifier.  */
start_over:
			
			/* Make back up of current processing pointer.  */
			rp_backup = rp;
			
			switch (*fmt++)
			{
			case '%':
				/* Match the `%' character itself.  */
				match_char ('%', *rp++);
				break;
			case 'a':
			case 'A':
				/* Match day of week.  */
				for (cnt = 0; cnt < 7; ++cnt)
				{
					if (match_string (weekday_name[cnt], rp)
						|| match_string (ab_weekday_name[cnt], rp))
					{
						break;
					}
				}
				if (cnt == 7)
					/* Does not match a weekday name.  */
					return NULL;
				tm->tm_wday = cnt;
				have_wday = 1;
				break;
			case 'b':
			case 'B':
			case 'h':
				/* Match month name.  */
				for (cnt = 0; cnt < 12; ++cnt)
				{
					if (match_string (month_name[cnt], rp)
						|| match_string (ab_month_name[cnt], rp))
					{
						break;
					}
				}
				if (cnt == 12)
					/* Does not match a month name.  */
					return NULL;
				tm->tm_mon = cnt;
				want_xday = 1;
				break;
			case 'c':
				/* Match locale's date and time format.  */
				if (!recursive (HERE_D_T_FMT))
					return NULL;
				want_xday = 1;
				break;
			case 'C':
				/* Match century number.  */
				get_number (0, 99, 2);
				century = val;
				want_xday = 1;
				break;
			case 'd':
			case 'e':
				/* Match day of month.  */
				get_number (1, 31, 2);
				tm->tm_mday = val;
				have_mday = 1;
				want_xday = 1;
				break;
			case 'F':
				if (!recursive ("%Y-%m-%d"))
					return NULL;
				want_xday = 1;
				break;
			case 'x':
				/* Fall through.  */
			case 'D':
				/* Match standard day format.  */
				if (!recursive (HERE_D_FMT))
					return NULL;
				want_xday = 1;
				break;
			case 'k':
			case 'H':
				/* Match hour in 24-hour clock.  */
				get_number (0, 23, 2);
				tm->tm_hour = val;
				have_I = 0;
				break;
			case 'I':
				/* Match hour in 12-hour clock.  */
				get_number (1, 12, 2);
				tm->tm_hour = val % 12;
				have_I = 1;
				break;
			case 'j':
				/* Match day number of year.  */
				get_number (1, 366, 3);
				tm->tm_yday = val - 1;
				have_yday = 1;
				break;
			case 'm':
				/* Match number of month.  */
				get_number (1, 12, 2);
				tm->tm_mon = val - 1;
				have_mon = 1;
				want_xday = 1;
				break;
			case 'M':
				/* Match minute.  */
				get_number (0, 59, 2);
				tm->tm_min = val;
				break;
			case 'n':
			case 't':
				/* Match any white space.  */
				while (isspace (*rp))
					++rp;
				break;
			case 'p':
				/* Match locale's equivalent of AM/PM.  */
				if (!match_string (HERE_AM_STR, rp))
					if (match_string (HERE_PM_STR, rp))
						is_pm = 1;
					else
						return NULL;
					break;
			case 'r':
				if (!recursive (HERE_T_FMT_AMPM))
					return NULL;
				break;
			case 'R':
				if (!recursive ("%H:%M"))
					return NULL;
				break;
			case 's':
				{
				/* The number of seconds may be very high so we cannot use
				the `get_number' macro.  Instead read the number
				character for character and construct the result while
					doing this.  */
					time_t secs = 0;
					if (*rp < '0' || *rp > '9')
						/* We need at least one digit.  */
						return NULL;
					
					do
					{
						secs *= 10;
						secs += *rp++ - '0';
					}
					while (*rp >= '0' && *rp <= '9');
					
					if (time_r (&secs, tm, mode) == NULL)
						/* Error in function.  */
						return NULL;
				}
				break;
			case 'S':
				get_number (0, 61, 2);
				tm->tm_sec = val;
				break;
			case 'X':
				/* Fall through.  */
			case 'T':
				if (!recursive (HERE_T_FMT))
					return NULL;
				break;
			case 'u':
				get_number (1, 7, 1);
				tm->tm_wday = val % 7;
				have_wday = 1;
				break;
			case 'g':
				get_number (0, 99, 2);
				/* XXX This cannot determine any field in TM.  */
				break;
			case 'G':
				if (*rp < '0' || *rp > '9')
					return NULL;
					/* XXX Ignore the number since we would need some more
				information to compute a real date.  */
				do
				++rp;
				while (*rp >= '0' && *rp <= '9');
				break;
			case 'U':
			case 'V':
			case 'W':
				get_number (0, 53, 2);
				/* XXX This cannot determine any field in TM without some
				information.  */
				break;
			case 'w':
				/* Match number of weekday.  */
				get_number (0, 6, 1);
				tm->tm_wday = val;
				have_wday = 1;
				break;
			case 'y':
				/* Match year within century.  */
				get_number (0, 99, 2);
				/* The "Year 2000: The Millennium Rollover" paper suggests that
				values in the range 69-99 refer to the twentieth century.  */
				tm->tm_year = val >= 69 ? val : val + 100;
				/* Indicate that we want to use the century, if specified.  */
				want_century = 1;
				want_xday = 1;
				break;
			case 'Y':
				/* Match year including century number.  */
				get_number (0, 9999, 4);
				tm->tm_year = val - 1900;
				want_century = 0;
				want_xday = 1;
				break;
				
				goto start_over;
				
			case 'O':
				switch (*fmt++)
				{
				case 'd':
				case 'e':
					/* Match day of month using alternate numeric symbols.  */
					get_number (1, 31, 2);
					tm->tm_mday = val;
					have_mday = 1;
					want_xday = 1;
					break;
				case 'H':
				/* Match hour in 24-hour clock using alternate numeric
					symbols.  */
					get_number (0, 23, 2);
					tm->tm_hour = val;
					have_I = 0;
					break;
				case 'I':
				/* Match hour in 12-hour clock using alternate numeric
					symbols.  */
					get_number (1, 12, 2);
					tm->tm_hour = val - 1;
					have_I = 1;
					break;
				case 'm':
					/* Match month using alternate numeric symbols.  */
					get_number (1, 12, 2);
					tm->tm_mon = val - 1;
					have_mon = 1;
					want_xday = 1;
					break;
				case 'M':
					/* Match minutes using alternate numeric symbols.  */
					get_number (0, 59, 2);
					tm->tm_min = val;
					break;
				case 'S':
					/* Match seconds using alternate numeric symbols.  */
					get_number (0, 61, 2);
					tm->tm_sec = val;
					break;
				case 'U':
				case 'V':
				case 'W':
					get_number (0, 53, 2);
					/* XXX This cannot determine any field in TM without
					further information.  */
					break;
				case 'w':
					/* Match number of weekday using alternate numeric symbols.  */
					get_number (0, 6, 1);
					tm->tm_wday = val;
					have_wday = 1;
					break;
				case 'y':
					/* Match year within century using alternate numeric symbols.  */
					get_number (0, 99, 2);
					tm->tm_year = val >= 69 ? val : val + 100;
					want_xday = 1;
					break;
				default:
					return NULL;
				}
				break;
				default:
					return NULL;
	}
    }
	
	if (have_I && is_pm)
		tm->tm_hour += 12;
	
	if (century != -1)
    {
		if (want_century)
			tm->tm_year = tm->tm_year % 100 + (century - 19) * 100;
		else
			/* Only the century, but not the year.  Strange, but so be it.  */
			tm->tm_year = (century - 19) * 100;
    }
	
	if (want_xday && !have_wday)
	{
		if ( !(have_mon && have_mday) && have_yday)
		{
			/* We don't have tm_mon and/or tm_mday, compute them.  */
			int t_mon = 0;
			while (__mon_yday[__isleap(1900 + tm->tm_year)][t_mon] <= tm->tm_yday)
				t_mon++;
			if (!have_mon)
				tm->tm_mon = t_mon - 1;
			if (!have_mday)
				tm->tm_mday =
				(tm->tm_yday
				- __mon_yday[__isleap(1900 + tm->tm_year)][t_mon - 1] + 1);
		}
		day_of_the_week (tm);
	}
	if (want_xday && !have_yday)
		day_of_the_year (tm);
	
	return (char *) rp;
}


}	// namespace pvt_strptime

#endif

bool plUnifiedTime::FromString(const char * buf, const char * fmt)
{
	struct tm tm;
	tm.tm_isdst = -1;
#if !defined(HS_BUILD_FOR_UNIX)
	bool result = (pvt_strptime::strptime_internal(buf, fmt, &tm, fMode)!=nil);
#else
	bool result = (strptime(buf, fmt, &tm)!=nil);
#endif
	if (result)
		*this = tm;
	return result;
}

/// Local time zone offset stuff

Int32	plUnifiedTime::fLocalTimeZoneOffset = -1;

Int32	plUnifiedTime::IGetLocalTimeZoneOffset( void )
{
	static bool		inited = false;

	if( !inited )
	{
		inited = true;

		// Calculate the difference between local time and GMT for this system currently
		// Taken from devx.com from an article written by Danny Kalev
		// http://gethelp.devx.com/techtips/cpp_pro/10min/2001/10min1200-3.asp

		time_t	currLocalTime = time( 0	);		// current local time

		struct tm	local = *gmtime( &currLocalTime );	// convert curr to GMT, store as tm
		
		time_t		utc = mktime( &local );	// convert GMT tm to GMT time_t

		double diffInSecs = difftime( utc, currLocalTime );

		fLocalTimeZoneOffset = (Int32)diffInSecs;
	}

	return fLocalTimeZoneOffset;
}

//
// static helper, return difference timeA-timeB, may be negative
//
double plUnifiedTime::GetTimeDifference(const plUnifiedTime& timeA, const plUnifiedTime& timeB)
{
	bool neg = (timeB > timeA);
	plUnifiedTime timeDiff = neg ? (timeB - timeA) : (timeA - timeB);	// always positive
	double t = (float)(neg ? timeDiff.GetSecsDouble() * -1. : timeDiff.GetSecsDouble());
	return t;
}
