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
#ifndef _PL_UNIFIEDTIME_INC_
#define _PL_UNIFIEDTIME_INC_

#include "hsTypes.h"
#include "hsStlUtils.h"

#if HS_BUILD_FOR_WIN32
#include "hsWindows.h"
#endif

//
// Plasma Unified System Time & Data Class
// the time and date are in secs and micros from
// Jan 1, 1970 00:00:00
//

struct timeval;
class hsStream;

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

enum plUnifiedTime_CtorNow { kNow };


class plUnifiedTime //
{
public:
	enum Mode
	{
		kGmt,
		kLocal
	};

protected:
	UInt32	fSecs;
	UInt32	fMicros;
	Mode	fMode;

	static Int32	fLocalTimeZoneOffset;

	struct tm * IGetTime(const time_t * timer) const;

	static Int32	IGetLocalTimeZoneOffset( void );

public:
	plUnifiedTime() : fSecs(0),fMicros(0), fMode(kGmt) { }		// set ToEpoch() at start
	plUnifiedTime(double secsDouble) { SetSecsDouble(secsDouble); }
	plUnifiedTime(plUnifiedTime_CtorNow,int mode=kLocal);
	plUnifiedTime(const timeval & tv);
	plUnifiedTime(int mode, const struct tm & src);
	plUnifiedTime(time_t t);
	plUnifiedTime(unsigned long t);
	plUnifiedTime(int year, int month, int day, int hour, int min, int sec, unsigned long usec=0, int dst=-1);
	plUnifiedTime(int mode, const char * buf, const char * fmt);
	plUnifiedTime(const plUnifiedTime & src);
	plUnifiedTime(const plUnifiedTime * src);
	static plUnifiedTime GetCurrentTime(Mode mode=kGmt);

	// assignment
	const plUnifiedTime & operator=(const plUnifiedTime & src);
	const plUnifiedTime & operator=(const plUnifiedTime * src);
	const plUnifiedTime & operator=(const struct timeval & src);
	const plUnifiedTime & operator=(time_t src);
	const plUnifiedTime & operator=(unsigned long t);
	const plUnifiedTime & operator=(const struct tm & src);

	// getters
	UInt32 GetSecs() const { return fSecs; }
	UInt32 GetMicros() const { return fMicros; }
	double GetSecsDouble() const;  // get the secs and micros as a double floating point value
	hsBool GetTime(short &year, short &month, short &day, short &hour, short &minute, short &second) const;
	struct tm * GetTm(struct tm * ptm=nil) const;
	int GetYear() const;
	int GetMonth() const;
	int GetDay() const;
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetMillis() const;
	int GetDayOfWeek() const;
	int GetMode() const {return fMode;}	// local or gmt.
	UInt32 AsMillis();

	// setters
	void SetSecs(const UInt32 secs) { fSecs = secs; }
	void SetSecsDouble(double secs);
	void SetMicros(const UInt32 micros) { fMicros = micros; }
	hsBool SetTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec=0, int dst=-1);
	hsBool SetGMTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec=0, int dst=-1);
	hsBool SetToUTC();
	void ToCurrentTime();
	void ToEpoch() { fSecs = 0; fMicros = 0;}
	void SetMode(Mode mode) { fMode=mode;}
	void FromMillis(UInt32 millis);
#if HS_BUILD_FOR_WIN32
	hsBool SetFromWinFileTime(const FILETIME ft);
#endif
	
	// query
	bool AtEpoch() const { return fSecs == 0 && fMicros == 0;}

	void Read(hsStream* s);
	void Write(hsStream* s) const;

	// time math
	const plUnifiedTime & operator+=(const plUnifiedTime & rhs);
	const plUnifiedTime & operator-=(const plUnifiedTime & rhs);
	friend plUnifiedTime operator -(const plUnifiedTime & left, const plUnifiedTime & right);
	friend plUnifiedTime operator +(const plUnifiedTime & left, const plUnifiedTime & right);
	static double GetTimeDifference(const plUnifiedTime& timeA, const plUnifiedTime& timeB);		// handles negative

	// time compare
	bool operator==(const plUnifiedTime & rhs) const;
	bool operator!=(const plUnifiedTime & rhs) const;
	bool operator <(const plUnifiedTime & rhs) const;
	bool operator >(const plUnifiedTime & rhs) const;
	bool operator<=(const plUnifiedTime & rhs) const;
	bool operator>=(const plUnifiedTime & rhs) const;

	friend bool operator <(const plUnifiedTime & time, int secs);


	// casting
	operator time_t() const { return fSecs;}
	operator timeval() const;
	operator struct tm() const;

	// formatting (ala strftime)
	std::string Format(const char * fmt) const;

	// parsing
	bool FromString(const char * buf, const char * fmt);
	
	const char* Print() const;	// print as simple string
	const char* PrintWMillis() const;	// print as simple string w/ millis
/*
FromString: (from glibc's strptime() man page)
     Converts the character string pointed to by buf to values which are
	 stored in the ``tm'' structure pointed to by tm, using the format
	 specified by format.

     The following conversion specifications are supported:

     %%    A `%' is written. No argument is converted.
     %a    the day of week, using the locale's weekday names; either the ab-
           breviated or full name may be specified.
     %A    the same as %a.
     %b    the month, using the locale's month names; either the abbreviated
           or full name may be specified.
     %B    the same as %b.
     %c    the date and time, using the locale's date and time format.
     %C    the century number [0,99]; leading zeros are permitted but not re-
           quired.  This conversion should be used in conjunction with the %y
           conversion.
     %d    the day of month [1,31]; leading zeros are permitted but not re-
           quired.
     %D    the date as %m/%d/%y.
     %e    the same as %d.
     %h    the same as %b.
     %H    the hour (24-hour clock) [0,23]; leading zeros are permitted but
           not required.
     %I    the hour (12-hour clock) [1,12]; leading zeros are permitted but
           not required.
     %j    the day number of the year [1,366]; leading zeros are permitted but
           not required.
     %k    the same as %H.
     %l    the same as %I.
     %m    the month number [1,12]; leading zeros are permitted but not re-
           quired.
     %M    the minute [0,59]; leading zeros are permitted but not required.
     %n    any white-space, including none.
     %p    the locale's equivalent of a.m. or p.m..
     %r    the time (12-hour clock) with %p, using the locale's time format.
     %R    the time as %H:%M.
     %S    the seconds [0,61]; leading zeros are permitted but not required.
     %t    any white-space, including none.
     %T    the time as %H:%M:%S.
     %U    the week number of the year (Sunday as the first day of the week)
           as a decimal number [0,53]; leading zeros are permitted but not re-
           quired.  All days in a year preceding the first Sunday are consid-
           ered to be in week 0.
     %w    the weekday as a decimal number [0,6], with 0 representing Sunday;
           leading zeros are permitted but not required.
     %W    the week number of the year (Monday as the first day of the week)
           as a decimal number [0,53]; leading zeros are permitted but not re-
           quired.  All days in a year preceding the first Monday are consid-
           ered to be in week 0.
     %x    the date, using the locale's date format.
     %X    the time, using the locale's time format.
     %y    the year within the 20th century [69,99] or the 21st century
           [0,68]; leading zeros are permitted but not required.  If specified
           in conjunction with %C, specifies the year [0,99] within that cen-
           tury.
     %Y    the year, including the century (i.e., 1996).
*/
};


plUnifiedTime operator -(const plUnifiedTime & left, const plUnifiedTime & right);
plUnifiedTime operator +(const plUnifiedTime & left, const plUnifiedTime & right);

#endif //_PL_UNIFIEDTIME_INC_
