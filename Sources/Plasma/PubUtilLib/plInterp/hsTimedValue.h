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

#ifndef hsTimedValue_inc
#define hsTimedValue_inc

#include "hsStream.h"

template <class T>
class hsTimedValue {
public:
	enum {
		kIdle			= 0x1,
		kInstant		= 0x2
	};
protected:
	UInt32					fFlags;
	hsScalar				fDuration;
	hsScalar				fStartTime;

	T						fValue;
	T						fGoal;
	T						fFrom;

public:
	hsTimedValue() : fFlags(kIdle|kInstant), fDuration(0) {}
	hsTimedValue(const T& v) : fFlags(kIdle|kInstant), fDuration(0) { SetValue(v); }

	UInt32 GetFlags() { return fFlags; }

	void SetDuration(hsScalar duration);
	hsScalar GetDuration() const { return fDuration; }

	hsBool32 operator==(const hsTimedValue<T>& v);
	hsTimedValue<T>& operator=(const T& v) { SetValue(v); return *this; }
	hsTimedValue<T>& operator+=(const T& v) { SetValue(v + fValue); return *this; }

	void SetTempValue(const T& v) { fValue = v; }
	void SetValue(const T& v) { fFrom = fGoal = fValue = v; fFlags |= kIdle; }
	const T& GetValue() const { return fValue; }

	void SetGoal(const T& g) { fGoal = g; }
	const T& GetGoal() const { return fGoal; }

	void Reset() { fFlags |= (kIdle | kInstant); }

	void StartClock(hsScalar s);
	hsScalar GetStartTime() const { return fStartTime; }

	const T& GetFrom() const { return fFrom; }

	void Update(hsScalar s);

	void WriteScalar(hsStream* s, hsScalar currSecs);
	void Write(hsStream* s, hsScalar currSecs);

	void ReadScalar(hsStream* s, hsScalar currSecs);
	void Read(hsStream* s, hsScalar currSecs);
};

template <class T>
void hsTimedValue<T>::WriteScalar(hsStream* s, hsScalar currSecs)
{
	s->WriteSwap32(fFlags);

	s->WriteSwapScalar(fValue);
	
	if( !(fFlags & kIdle) )
	{
		s->WriteSwapScalar(fDuration);
		s->WriteSwapScalar(currSecs - fStartTime);

		s->WriteSwapScalar(fGoal);
		s->WriteSwapScalar(fFrom);
	}
}

template <class T>
void hsTimedValue<T>::Write(hsStream* s, hsScalar currSecs)
{
	s->WriteSwap32(fFlags);

	fValue.Write(s);
	
	if( !(fFlags & kIdle) )
	{
		s->WriteSwapScalar(fDuration);
		s->WriteSwapScalar(currSecs - fStartTime);

		fGoal.Write(s);
		fFrom.Write(s);
	}
}

template <class T>
void hsTimedValue<T>::ReadScalar(hsStream* s, hsScalar currSecs)
{
	fFlags = s->ReadSwap32();

	fValue = s->ReadSwapScalar();

	if( !(fFlags & kIdle) )
	{
		fDuration = s->ReadSwapScalar();
		fStartTime = currSecs - s->ReadSwapScalar();

		fGoal = s->ReadSwapScalar();
		fFrom = s->ReadSwapScalar();
	}
}

template <class T>
void hsTimedValue<T>::Read(hsStream* s, hsScalar currSecs)
{
	fFlags = s->ReadSwap32();

	fValue.Read(s);

	if( !(fFlags & kIdle) )
	{
		fDuration = s->ReadSwapScalar();
		fStartTime = currSecs - s->ReadSwapScalar();

		fGoal.Read(s);
		fFrom.Read(s);
	}
}

template <class T>
void hsTimedValue<T>::SetDuration(hsScalar duration) 
{ 
	fDuration = duration; 
	if( fDuration > 0 )
		fFlags &= ~kInstant;
	else
		fFlags |= kInstant;
}

template <class T>
hsBool32 hsTimedValue<T>::operator==(const hsTimedValue<T>& v)
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
void hsTimedValue<T>::StartClock(hsScalar s)
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
void hsTimedValue<T>::Update(hsScalar s)
{
	if( fFlags & kIdle )
		return;

	hsAssert(fDuration > 0, "Instant should always be idle");

	hsScalar interp = (s - fStartTime) / fDuration;

	if( interp >= hsScalar1 )
	{
		fValue = fGoal;
		interp = hsScalar1;
		fFlags |= kIdle;
	}
	else
		fValue = fFrom + (fGoal - fFrom) * interp;
}



#endif // hsTimedValue_inc
