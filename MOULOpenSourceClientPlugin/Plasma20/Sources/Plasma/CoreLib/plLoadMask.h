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

#ifndef plLoadMask_inc
#define plLoadMask_inc

#include "hsTypes.h"

template <class T> class hsTArray;
class hsStream;

class plLoadMask
{
public:
	enum {
		kMaxCap = 1
	};
protected:
	static UInt8	fGlobalQuality;
	static UInt8	fGlobalCapability;
	union {
		UInt8			fQuality[kMaxCap+1];
		UInt16			fMask;
	};

	static void SetGlobalQuality(int q) { fGlobalQuality = IBitToMask(q); }
	static void SetGlobalCapability(int c) { if( c > kMaxCap ) c = kMaxCap; else if( c < 0 ) c = 0; fGlobalCapability = c; }

	static UInt8 IBitToMask(int b) { hsAssert(b<8, "LoadMask: bit too large for byte"); return (1 << b); }

	friend class plQuality;
public:
	// Change this to a for loop on kMaxCap+1 if we ever get more caps.
	plLoadMask() { fQuality[0] = fQuality[1] = 0xff; }
	plLoadMask(UInt8 qLo, UInt8 qHi) { fQuality[0] = qLo; fQuality[1] = qHi; }
	~plLoadMask() {}

	hsBool		DontLoad() const { return !(fQuality[fGlobalCapability] & fGlobalQuality); }

	hsBool		NeverLoads() const { return !(fQuality[0] && fQuality[1]); }

	hsBool		IsUsed() const { return (fQuality[0] != UInt8(-1)) || (fQuality[1] != UInt8(-1));	}

	hsBool		MatchesQuality(int q) const { return (IBitToMask(q) & (fQuality[0] | fQuality[1])) != 0; }
	hsBool		MatchesCapability(int c) const { return fQuality[c] != 0; }
	hsBool		MatchesQualityAndCapability(int q, int c) const { return IBitToMask(q) & fQuality[c]; }

	hsBool		MatchesCurrentQuality() const { return MatchesQuality(fGlobalQuality); }
	hsBool		MatchesCurrentCapability() const { return MatchesCapability(fGlobalCapability); }
	hsBool		MatchesCurrent() const { return !DontLoad(); }

	UInt8		GetQualityMask(int cap) const { return fQuality[cap]; }

	plLoadMask&		SetMask(UInt8 lo, UInt8 hi) { fQuality[0] = lo; fQuality[1] = hi; return *this; }
	plLoadMask&		SetNever() { return SetMask(0,0); }
	plLoadMask&		SetAlways() { return SetMask(UInt8(-1), UInt8(-1)); }

	plLoadMask& operator|=(const plLoadMask& m) { fMask |= m.fMask; return *this; }
	plLoadMask& operator&=(const plLoadMask& m) { fMask &= m.fMask; return *this; }

	int operator==(const plLoadMask& m) const { return fMask == m.fMask; }
	int operator!=(const plLoadMask& m) const { return !(*this == m); }

	// Only useful for sorting.
	int operator<(const plLoadMask& m) const { return fMask < m.fMask; }

	friend plLoadMask operator|(const plLoadMask& lhs, const plLoadMask& rhs) { plLoadMask r(lhs); r |= rhs; return r; }
	friend plLoadMask operator&(const plLoadMask& lhs, const plLoadMask& rhs) { plLoadMask r(lhs); r &= rhs; return r; }

	void		Read(hsStream* s);
	void		Write(hsStream* s) const;

	static const plLoadMask		kAlways;


	// Input lists are in order of preference, i.e. if rep[0] and rep[1] are both loadable based
	// on the current settings, only rep[0] will be loaded. This implies some rules
	// to avoid wasted reps (reps that would never get loaded). Basically:
	//	if( i < j ) then
	//		(quals[i] > quals[j]) || (caps[i] > caps[j])
	//
	// It doesn't break anything if that condition isn't met, it just means
	// the latter one will never get loaded (have a load mask of zero).
	// So, just to be a pal, we'll detect that condition and return based on it.
	// i.e. Return true on invalid input (something will never get loaded).
	// 
	// You can also pre-validate your input with ValidateReps, and/or validate
	// the output with ValidateMasks. The return value is a bitmask of which
	// items in the list had problems, so return of zero means A-OK.
	//
	static hsBool ComputeRepMasks(int num, const int quals[], const int caps[], plLoadMask masks[]);
	static UInt32 ValidateReps(int num, const int quals[], const int caps[]);
	static UInt32 ValidateMasks(int num, plLoadMask masks[]);
};

#endif // plLoadMask_inc
