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
#ifndef PL_AVG_H
#define PL_AVG_H

#include "HeadSpin.h"
#include "hsThread.h"
#include "hsStlUtils.h"


// A Time based Value Averaging class
//  implemented in a ring buffer
// Values are averaged over the RingLen
//  independant of frame time
// The ring buffer will grow to accomadate
//  as many samples as added during the Len
// Only Accurate to kPercision (0.001) of whatever units
//  of time are used

template <class T> class TimeBasedAvgRing
{
/* T must be a simple type

  If T is not a floating point type then rounding
  loss may occur when samples are interpolated
  at window boundries
*/
private:
    static const float kPercision;

    template <class S> class Item
    {
    private:
        T fValue;
        double fTime;
    public:
        Item() { Reset(); }
        Item(const T val, const double time) : fValue(val), fTime(time) { }
        void Reset() { fValue = 0; fTime = 0; }
        void SetValue(const T val) { fValue = val; }
        void SetTime(const double time) { fTime = time; }
        T GetValue() const { return fValue; }
        double GetTime() const { return fTime; }
    };
    typedef std::list< Item<T> > TimeList;
    typedef typename TimeList::iterator TimeListIterator; // .NET added typename to be C++ ISO compliant - JL

    TimeList fList;
    float fLen;  // in time
    float fAvg;
    float fMaxAvg;
    double fTotal;
    TimeListIterator fRingStart, fRingEnd;
    hsMutex fLock;
public:
    TimeBasedAvgRing():fLen(0.f),fAvg(0.f),fMaxAvg(0.f),fTotal(0.0) {}

    void SetRingLen(const float len) { fLen = len; }
    float GetRingLen() const { return fLen; }
    void AddItem(T value, double time);
    float GetAvg() const { return fAvg; }
    double GetTotal() const { return fTotal; }
    float GetMaxAvg() const { return fMaxAvg; }
    void ResetMaxAvg() { fMaxAvg=fAvg; }
    void Reset() { fRingStart=fRingEnd; ResetMaxAvg(); }
};


#endif  // PL_AVG_H

