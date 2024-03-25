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

#ifndef hsTimedValue_inc
#define hsTimedValue_inc

#include "hsStream.h"

template <class T>
class hsTimedValue {
public:
    enum {
        kIdle           = 0x1,
        kInstant        = 0x2
    };
protected:
    uint32_t                  fFlags;
    float                fDuration;
    float                fStartTime;

    T                       fValue;
    T                       fGoal;
    T                       fFrom;

public:
    hsTimedValue() : fFlags(kIdle|kInstant), fDuration(0) {}
    hsTimedValue(const T& v) : fFlags(kIdle|kInstant), fDuration(0) { SetValue(v); }

    uint32_t GetFlags() { return fFlags; }

    void SetDuration(float duration);
    float GetDuration() const { return fDuration; }

    bool32 operator==(const hsTimedValue<T>& v);
    hsTimedValue<T>& operator=(const T& v) { SetValue(v); return *this; }
    hsTimedValue<T>& operator+=(const T& v) { SetValue(v + fValue); return *this; }

    void SetTempValue(const T& v) { fValue = v; }
    void SetValue(const T& v) { fFrom = fGoal = fValue = v; fFlags |= kIdle; }
    const T& GetValue() const { return fValue; }

    void SetGoal(const T& g) { fGoal = g; }
    const T& GetGoal() const { return fGoal; }

    void Reset() { fFlags |= (kIdle | kInstant); }

    void StartClock(float s);
    float GetStartTime() const { return fStartTime; }

    const T& GetFrom() const { return fFrom; }

    void Update(float s);

    void WriteScalar(hsStream* s, float currSecs);
    void Write(hsStream* s, float currSecs);

    void ReadScalar(hsStream* s, float currSecs);
    void Read(hsStream* s, float currSecs);
};

template <class T>
void hsTimedValue<T>::WriteScalar(hsStream* s, float currSecs)
{
    s->WriteLE32(fFlags);

    s->WriteLEFloat(fValue);
    
    if( !(fFlags & kIdle) )
    {
        s->WriteLEFloat(fDuration);
        s->WriteLEFloat(currSecs - fStartTime);

        s->WriteLEFloat(fGoal);
        s->WriteLEFloat(fFrom);
    }
}

template <class T>
void hsTimedValue<T>::Write(hsStream* s, float currSecs)
{
    s->WriteLE32(fFlags);

    fValue.Write(s);
    
    if( !(fFlags & kIdle) )
    {
        s->WriteLEFloat(fDuration);
        s->WriteLEFloat(currSecs - fStartTime);

        fGoal.Write(s);
        fFrom.Write(s);
    }
}

template <class T>
void hsTimedValue<T>::ReadScalar(hsStream* s, float currSecs)
{
    fFlags = s->ReadLE32();

    fValue = s->ReadLEFloat();

    if( !(fFlags & kIdle) )
    {
        fDuration = s->ReadLEFloat();
        fStartTime = currSecs - s->ReadLEFloat();

        fGoal = s->ReadLEFloat();
        fFrom = s->ReadLEFloat();
    }
}

template <class T>
void hsTimedValue<T>::Read(hsStream* s, float currSecs)
{
    fFlags = s->ReadLE32();

    fValue.Read(s);

    if( !(fFlags & kIdle) )
    {
        fDuration = s->ReadLEFloat();
        fStartTime = currSecs - s->ReadLEFloat();

        fGoal.Read(s);
        fFrom.Read(s);
    }
}

template <class T>
void hsTimedValue<T>::SetDuration(float duration) 
{ 
    fDuration = duration; 
    if( fDuration > 0 )
        fFlags &= ~kInstant;
    else
        fFlags |= kInstant;
}

template <class T>
bool32 hsTimedValue<T>::operator==(const hsTimedValue<T>& v)
{
    if ((fFlags == v.fFlags) &&
        (fDuration == v.fDuration) &&
        (fStartTime == v.fStartTime) &&
        (fValue == v.fValue) &&
        (fGoal == v.fGoal) &&
        (fFrom == v.fFrom))
    {
        return true;
    }

    return false;
}

template <class T>
void hsTimedValue<T>::StartClock(float s)
{
    fStartTime = s;

    if( fFlags & kInstant )
    {
        fFlags |= kIdle;
        fValue = fGoal;
        return;
    }

    fFlags &= ~kIdle;

    if( fValue == fGoal )
        fFlags |= kIdle;

    fFrom = fValue;
}

template <class T>
void hsTimedValue<T>::Update(float s)
{
    if( fFlags & kIdle )
        return;

    hsAssert(fDuration > 0, "Instant should always be idle");

    float interp = (s - fStartTime) / fDuration;

    if( interp >= 1.f )
    {
        fValue = fGoal;
        interp = 1.f;
        fFlags |= kIdle;
    }
    else
        fValue = fFrom + (fGoal - fFrom) * interp;
}



#endif // hsTimedValue_inc
