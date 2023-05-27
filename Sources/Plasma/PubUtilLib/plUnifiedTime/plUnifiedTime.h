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

#ifndef _PL_UNIFIEDTIME_INC_
#define _PL_UNIFIEDTIME_INC_

#include "HeadSpin.h"

#if HS_BUILD_FOR_WIN32
    typedef struct _FILETIME FILETIME;
#endif

//
// Plasma Unified System Time & Data Class
// the time and date are in secs and micros from
// Jan 1, 1970 00:00:00
//

struct timeval;
class hsStream;
namespace ST { class string; }

class plUnifiedTime //
{
public:
    enum Mode
    {
        kGmt,
        kLocal
    };

protected:
    time_t  fSecs;
    uint32_t  fMicros;
    Mode    fMode;

    static int32_t    fLocalTimeZoneOffset;

    struct tm * IGetTime(const time_t * timer) const;

    static int32_t    IGetLocalTimeZoneOffset();

public:
    plUnifiedTime() : fSecs(), fMicros(), fMode(kGmt) { }      // set ToEpoch() at start
    plUnifiedTime(double secsDouble) : fMode(kGmt) { SetSecsDouble(secsDouble); }
    plUnifiedTime(const timeval & tv);
    plUnifiedTime(Mode mode, struct tm src);
    plUnifiedTime(time_t t);
    plUnifiedTime(int year, int month, int day, int hour, int min, int sec, unsigned long usec=0, int dst=-1);

    static plUnifiedTime GetCurrent(Mode mode=kGmt);

    // getters
    time_t GetSecs() const { return fSecs; }
    uint32_t GetMicros() const { return fMicros; }
    double GetSecsDouble() const;  // get the secs and micros as a double floating point value
    bool GetTime(short &year, short &month, short &day, short &hour, short &minute, short &second) const;
    struct tm * GetTm(struct tm * ptm=nullptr) const;
    int GetYear() const;
    int GetMonth() const;
    int GetDay() const;
    int GetHour() const;
    int GetMinute() const;
    int GetSecond() const;
    int GetMillis() const;
    int GetDayOfWeek() const;
    Mode GetMode() const { return fMode; } // local or gmt.

    // setters
    void SetSecs(const time_t secs) { fSecs = secs; }
    void SetSecsDouble(double secs);
    void SetMicros(const uint32_t micros) { fMicros = micros; }
    bool SetTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec=0, int dst=-1);
    bool SetGMTime(short year, short month, short day, short hour, short minute, short second, unsigned long usec=0);
    bool SetToUTC();
    void ToCurrentTime();
    void ToEpoch() { fSecs = 0; fMicros = 0;}
    void SetMode(Mode mode) { fMode=mode;}
#if HS_BUILD_FOR_WIN32
    bool SetFromWinFileTime(const FILETIME ft);
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
    static double GetTimeDifference(const plUnifiedTime& timeA, const plUnifiedTime& timeB);        // handles negative

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
    ST::string Format(const char * fmt) const;
    
    ST::string Print() const; // print as simple string
    ST::string PrintWMillis() const; // print as simple string w/ millis
};


plUnifiedTime operator -(const plUnifiedTime & left, const plUnifiedTime & right);
plUnifiedTime operator +(const plUnifiedTime & left, const plUnifiedTime & right);

#endif //_PL_UNIFIEDTIME_INC_
