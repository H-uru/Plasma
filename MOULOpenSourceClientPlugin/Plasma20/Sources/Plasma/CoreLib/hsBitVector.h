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

#ifndef hsBitVector_inc
#define hsBitVector_inc

#include "hsTypes.h"

template <class T> class hsTArray;
class hsStream;

class hsBitVector {

protected:
	UInt32*					fBitVectors;
	UInt32					fNumBitVectors;

	void		IGrow(UInt32 newNumBitVectors);

	friend		class hsBitIterator;
public:
	hsBitVector(const hsBitVector& other);
	hsBitVector(UInt32 which) : fBitVectors(nil), fNumBitVectors(0) { SetBit(which); }
	hsBitVector(int b, ...); // list of one or more integer bits to set. -1 (or any negative) terminates the list (e.g. hsBitVector(0,1,4,-1);
	hsBitVector(const hsTArray<Int16>& list); // sets bit for each int in list
	hsBitVector() : fBitVectors(nil), fNumBitVectors(0) {}
	virtual ~hsBitVector() { Reset(); }

	hsBitVector& Reset() { delete [] fBitVectors; fBitVectors = nil; fNumBitVectors = 0; return *this; }
	hsBitVector& Clear(); // everyone clear, but no dealloc
	hsBitVector& Set(int upToBit=-1); // WARNING - see comments at function

	int operator==(const hsBitVector& other) const; // unset (ie uninitialized) bits are clear, 
	int operator!=(const hsBitVector& other) const { return !(*this == other); }
	hsBitVector& operator=(const hsBitVector& other); // will wind up identical

	hsBool ClearBit(UInt32 which) { return SetBit(which, 0); } // returns previous state
	hsBool SetBit(UInt32 which, hsBool on = true); // returns previous state
	hsBool IsBitSet(UInt32 which) const; // returns current state
	hsBool ToggleBit(UInt32 which); // returns previous state
	hsBitVector& RemoveBit(UInt32 which); // removes bit, sliding higher bits down to fill the gap.

	friend inline int Overlap(const hsBitVector& lhs, const hsBitVector& rhs) { return lhs.Overlap(rhs); }
	hsBool Overlap(const hsBitVector& other) const;
	hsBool Empty() const;

	hsBool operator[](UInt32 which) const { return IsBitSet(which); }

	friend inline hsBitVector operator&(const hsBitVector& lhs, const hsBitVector& rhs); // See Overlap()
	friend inline hsBitVector operator|(const hsBitVector& lhs, const hsBitVector& rhs);
	friend inline hsBitVector operator^(const hsBitVector& lhs, const hsBitVector& rhs);
	friend inline hsBitVector operator-(const hsBitVector& lhs, const hsBitVector& rhs); // return lhs w/ rhs's bits turned off

	hsBitVector& operator&=(const hsBitVector& other); // See Overlap()
	hsBitVector& operator|=(const hsBitVector& other);
	hsBitVector& operator^=(const hsBitVector& other);
	hsBitVector& operator-=(const hsBitVector& other); // return me w/ other's bits turned off

	hsBitVector& Compact();
	hsBitVector& SetSize(UInt32 numBits) { ClearBit(numBits+1); return *this; }
	UInt32 GetSize() { return fNumBitVectors << 5; }

	// integer level access
	UInt32 GetNumBitVectors() const { return fNumBitVectors; }
	UInt32 GetBitVector(int i) const { return fBitVectors[i]; }
	void SetNumBitVectors(UInt32 n) { Reset(); fNumBitVectors=n; fBitVectors = TRACKED_NEW UInt32[n]; }
	void SetBitVector(int i, UInt32 val) { fBitVectors[i]=val; }

	// Do dst.SetCount(0), then add each set bit's index into dst, returning dst.
	hsTArray<Int16>& Enumerate(hsTArray<Int16>& dst) const;
	// this->Clear(), then set all bits listed in src, returning *this.
	hsBitVector& FromList(const hsTArray<Int16>& src);

	void Read(hsStream* s);
	void Write(hsStream* s) const;
};

inline hsBitVector::hsBitVector(const hsBitVector& other)
{
	if( 0 != (fNumBitVectors = other.fNumBitVectors) )
	{		
		fBitVectors = TRACKED_NEW UInt32[fNumBitVectors];
		int i;
		for( i = 0; i < fNumBitVectors; i++ )
			fBitVectors[i] = other.fBitVectors[i];
	}
	else
		fBitVectors = nil;
}

inline hsBool hsBitVector::Empty() const
{
	int i;
	for( i = 0; i < fNumBitVectors; i++ )
	{
		if( fBitVectors[i] )
			return false;
	}
	return true;
}

inline hsBool hsBitVector::Overlap(const hsBitVector& other) const
{
	if( fNumBitVectors > other.fNumBitVectors )
		return other.Overlap(*this);

	int i;
	for( i = 0; i < fNumBitVectors; i++ )
	{
		if( fBitVectors[i] & other.fBitVectors[i] )
			return true;
	}
	return false;
}

inline hsBitVector& hsBitVector::operator=(const hsBitVector& other)
{
	if( this != &other )
	{
		if( fNumBitVectors < other.fNumBitVectors )
		{
			Reset();
			fNumBitVectors = other.fNumBitVectors;
			fBitVectors = TRACKED_NEW UInt32[fNumBitVectors];
		}
		else
		{
			Clear();
		}

		int i;
		for( i = 0; i < other.fNumBitVectors; i++ )
			fBitVectors[i] = other.fBitVectors[i];
	}
	return *this;
}

inline int hsBitVector::operator==(const hsBitVector& other) const
{
	if( fNumBitVectors < other.fNumBitVectors )
		return other.operator==(*this);
	int i;
	for( i = 0; i < other.fNumBitVectors; i++ )
		if( fBitVectors[i] ^ other.fBitVectors[i] )
			return false;
	for( ; i < fNumBitVectors; i++ )
		if( fBitVectors[i] )
			return false;
	return true;
}

inline hsBitVector& hsBitVector::operator&=(const hsBitVector& other)
{
	if( this == &other )
		return *this;

	if( fNumBitVectors > other.fNumBitVectors )
	{
		fNumBitVectors = other.fNumBitVectors;
	}
	int i;
	for( i = 0; i < fNumBitVectors; i++ )
		fBitVectors[i] &= other.fBitVectors[i];
	return *this;
}

inline hsBitVector& hsBitVector::operator|=(const hsBitVector& other)
{
	if( this == &other )
		return *this;

	if( fNumBitVectors < other.fNumBitVectors )
	{
		IGrow(other.fNumBitVectors);
	}
	int i;
	for( i = 0; i < other.fNumBitVectors; i++ )
		fBitVectors[i] |= other.fBitVectors[i];
	return *this;
}

inline hsBitVector& hsBitVector::operator^=(const hsBitVector& other)
{
	if( this == &other )
	{
		Clear();
		return *this;
	}

	if( fNumBitVectors < other.fNumBitVectors )
	{
		IGrow(other.fNumBitVectors);
	}
	int i;
	for( i = 0; i < other.fNumBitVectors; i++ )
		fBitVectors[i] ^= other.fBitVectors[i];
	return *this;
}

inline hsBitVector& hsBitVector::operator-=(const hsBitVector& other)
{
	if( this == &other )
	{
		Clear();
		return *this;
	}

	int minNum = fNumBitVectors < other.fNumBitVectors ? fNumBitVectors : other.fNumBitVectors;
	int i;
	for( i = 0; i < minNum; i++ )
		fBitVectors[i] &= ~other.fBitVectors[i];
	return *this;
}

inline hsBitVector operator&(const hsBitVector& rhs, const hsBitVector& lhs)
{
	hsBitVector ret(rhs);
	return ret &= lhs;
}

inline hsBitVector operator|(const hsBitVector& rhs, const hsBitVector& lhs)
{
	hsBitVector ret(rhs);
	return ret |= lhs;
}

inline hsBitVector operator^(const hsBitVector& rhs, const hsBitVector& lhs)
{
	hsBitVector ret(rhs);
	return ret ^= lhs;
}

inline hsBitVector operator-(const hsBitVector& rhs, const hsBitVector& lhs)
{
	hsBitVector ret(rhs);
	return ret -= lhs;
}

inline hsBitVector& hsBitVector::Clear()
{
	int i;
	for( i = 0; i < fNumBitVectors; i++ )
		fBitVectors[i] = 0;
	return *this;
}

// WARNING - since the bitvector is conceptually infinitely long,
// we can't actually set all the bits. If you pass in a non-negative
// upToBit, this sets all bits up to and including that one, otherwise
// it just sets however many bits are currently allocated. You can
// assure this is as many as you want by first calling SetSize, but
// if there are more bits than the requested size, these will also
// get set. Calling Set with a non-negative upToBit will only set
// the bits from 0 to upToBit, but won't clear any higher bits.
inline hsBitVector& hsBitVector::Set(int upToBit)
{
	if( upToBit >= 0 )
	{
		UInt32 major = upToBit >> 5;
		UInt32 minor = 1 << (upToBit & 0x1f);
		if( major >= fNumBitVectors )
			IGrow(major+1);

		UInt32 i;
		for( i = 0; i < major; i++ )
			fBitVectors[i] = 0xffffffff;
		for( i = 1; i <= minor && i > 0; i <<= 1 )
			fBitVectors[major] |= i;
	}
	else
	{
		int i;
		for( i = 0; i < fNumBitVectors; i++ )
			fBitVectors[i] = 0xffffffff;
	}
	return *this;
}

inline hsBool hsBitVector::IsBitSet(UInt32 which) const
{
	UInt32 major = which >> 5;
	return 
		(major < fNumBitVectors)
		&& (0 != (fBitVectors[major] & 1 << (which & 0x1f)));
}

inline hsBool hsBitVector::SetBit(UInt32 which, hsBool on)
{
	UInt32 major = which >> 5;
	UInt32 minor = 1 << (which & 0x1f);
	if( major >= fNumBitVectors )
		IGrow(major+1);
	hsBool ret = 0 != (fBitVectors[major] & minor);
	if( ret != on )
	{
		if( on )
			fBitVectors[major] |= minor;
		else
			fBitVectors[major] &= ~minor;
	}

	return ret;
}

inline hsBool hsBitVector::ToggleBit(UInt32 which)
{
	UInt32 major = which >> 5;
	UInt32 minor = 1 << (which & 0x1f);
	if( major >= fNumBitVectors )
		IGrow(major);
	hsBool ret = 0 != (fBitVectors[major] & minor);
	if( ret )
		fBitVectors[major] &= ~minor;
	else
		fBitVectors[major] |= minor;
	return ret;
}

inline hsBitVector& hsBitVector::RemoveBit(UInt32 which)
{
	UInt32 major = which >> 5;
	if( major >= fNumBitVectors )
		return *this;
	UInt32 minor = 1 << (which & 0x1f);
	UInt32 lowMask = minor-1;
	UInt32 hiMask = ~(lowMask);

	fBitVectors[major] = (fBitVectors[major] & lowMask)
		| ((fBitVectors[major] >> 1) & hiMask);

	while( major < fNumBitVectors-1 )
	{
		if( fBitVectors[major+1] & 0x1 )
			fBitVectors[major] |= 0x80000000;
		else
			fBitVectors[major] &= ~0x80000000;

		major++;

		fBitVectors[major] >>= 1;
	}
	fBitVectors[major] &= ~0x80000000;

	return *this;
}

class hsBitIterator
{
protected:
	const hsBitVector&	fBits;

	int					fCurrent;

	int					fCurrVec;
	int					fCurrBit;

	int			IAdvanceBit();
	int			IAdvanceVec();

public:
	// Must call begin after instanciating.
	hsBitIterator(const hsBitVector& bits) : fBits(bits) {}

	int			Begin();
	int					Current() const { return fCurrent; }
	int			Advance();
	int					End() const { return fCurrVec < 0; }
};


#endif // hsBitVector_inc
