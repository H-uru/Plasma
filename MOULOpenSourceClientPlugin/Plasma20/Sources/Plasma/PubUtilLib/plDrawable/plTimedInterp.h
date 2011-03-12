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

#ifndef plTimedInterp_inc
#define plTimedInterp_inc

// There's no auto update on this, the owner is
// required to call update periodically.
//
// This isn't optimized in any way, and is still
// fairly general. Adding features would most likely
// be best implemented in a derived type.
// Notably lacking features are:
//		ability to read and write, 
//		auto-eval on significant time change
//		
//
// Requires class T to have the following members
// T& operator+=(const class T& t);
// T& operator-=(const class T& t);
// T& operator*=(hsScalar scale);
// T& operator=(const class T&); // unless builtin = is adequate.
//
template <class T> class plTimedInterp
{
protected:
	T			fInit;
	T			fCurr;
	T			fTarg;

	double		fDuration;
	double		fEnd;

	const T& IEnd();
	const T& IBegin();
	const T& IEval(hsScalar parm);
public:
	plTimedInterp();
	plTimedInterp(const T& t);

	const T& Set(const T& val) { return SetTarget(val, 0, 0); }
	const T& SetTarget(const T& targ, double start, double duration);

	const T& Update(double t);

	operator T() const { return fCurr; }
	const T& Value() const { return fCurr; }
};

template <class T>
plTimedInterp<T>::plTimedInterp()
:	fDuration(0),
	fEnd(0)
{
}

template <class T>
plTimedInterp<T>::plTimedInterp(const T& t)
:	fInit(t),
	fCurr(t),
	fTarg(t),
	fDuration(0),
	fEnd(0)
{
}

template <class T>
const T& 
plTimedInterp<T>::Update(double t)
{
	if( fDuration <= 0 )
		return IEnd();

	hsScalar parm = hsScalar((fEnd - t) / fDuration);
	if( parm <= 0 )
		return IEnd();
	else if( parm >= 1.f )
		return IBegin();

	return IEval(parm);
}

template <class T>
const T& 
plTimedInterp<T>::SetTarget(const T& targ, double start, double dur)
{
	fEnd = start + dur;
	fDuration = dur;
	fTarg = targ;
	if( dur <= 0 )
		fCurr = targ;
	fInit = fCurr;
	return fCurr;
}

template <class T>
const T& 
plTimedInterp<T>::IEnd()
{
	fDuration = 0;
	fCurr = fTarg;
	return fCurr;
}

template <class T>
const T& 
plTimedInterp<T>::IBegin()
{
	return fCurr;
}

template <class T>
const T& 
plTimedInterp<T>::IEval(hsScalar parm)
{
	fCurr = fInit;
	fCurr -= fTarg;
	fCurr *= parm;
	fCurr += fTarg;
	return fCurr;
}


#endif // plTimedInterp_inc

