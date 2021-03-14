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

#ifndef plTimedValue_inc
#define plTimedValue_inc

#include "hsTimer.h"
#include "hsStream.h"

// plTimedValue
// To use, replace your member var of type T with plTimedValue<T>.
// You can then pretty much treat it as normal. To set it to interpolate
// to a new value over secs seconds, use Set(newVal, secs).
// For I/O, see plTimedSimple and plTimedCompound.
template <class T> class plTimedValue
{
protected:
    T           fGoal;
    T           fInit;
    double      fStart;
    float    fInvSecs;

public:
    plTimedValue() : fGoal(), fInit(), fStart(), fInvSecs() { }
    plTimedValue(const plTimedValue<T>& o) { Set(o, 0.f); }
    plTimedValue(const T& v) { Set(v, 0.f); }

    plTimedValue<T>& operator=(const plTimedValue<T>& o) { return Set(o, 0.f); }
    plTimedValue<T>& operator=(const T& v) { return Set(v, 0.f); }

    plTimedValue<T>& Set(const T& v, float secs=0);

    operator T () const { return Value(); }

    T Value() const;

};

// Read/Writable version of plTimedValue, for intrinsic types (e.g. int, float, bool).
template <class T> class plTimedSimple : public plTimedValue<T>
{
public:
    plTimedSimple<T>& operator=(const plTimedValue<T>& o) { return Set(o, 0.f); }
    plTimedSimple<T>& operator=(const T& v) { return Set(v, 0.f); }

    plTimedSimple<T>& Set(const T& v, float secs=0) { plTimedValue<T>::Set(v, secs); return *this; }

    inline void Read(hsStream* s);
    inline void Write(hsStream* s) const;
};

// Read/Writable version of plTimedValue, for compound types (e.g. hsVector3, hsColorRGBA).
// May be any type that has Read(hsStream*)/Write(hsStream*) defined.
template <class T> class plTimedCompound : public plTimedValue<T>
{
public:
    plTimedCompound<T>& operator=(const plTimedValue<T>& o) { return Set(o, 0.f); }
    plTimedCompound<T>& operator=(const T& v) { return Set(v, 0.f); }

    plTimedCompound<T>& Set(const T& v, float secs=0) { plTimedValue<T>::Set(v, secs); return *this; }

    void Read(hsStream* s);
    void Write(hsStream* s) const;
};

template <class T> 
plTimedValue<T>& plTimedValue<T>::Set(const T& v, float secs)
{
    if( secs <= 0 )
    {
        fGoal = fInit = v;
        fInvSecs = 0;
    }
    else
    {
        fInit = Value();
        fStart = hsTimer::GetSysSeconds();
        fInvSecs = 1.f / secs;
        fGoal = v;
    }
    return *this;
}

template <class T> 
T plTimedValue<T>::Value() const
{
    if( fInvSecs > 0 )
    {
        float t = (float)((hsTimer::GetSysSeconds() - fStart) * fInvSecs);
        hsAssert(t >= 0, "Moving back in time");

        if( t < 1.f )
            return fGoal * t + fInit * (1.f - t);

    }
    return fGoal;
}


template <>
void plTimedSimple<float>::Read(hsStream* s)
{
    float val = s->ReadLEFloat();
    Set(val, 0.f);
}

template <>
void plTimedSimple<float>::Write(hsStream* s) const
{
    float val = this->Value();
    s->WriteLEFloat(val);
}

template <class T> 
void plTimedCompound<T>::Read(hsStream* s)
{
    T val;
    val.Read(s);
    Set(val, 0.f);
}

template <class T> 
void plTimedCompound<T>::Write(hsStream* s) const
{
    T val = this->Value();
    val.Write(s);
}

#endif // plTimedValue_inc
