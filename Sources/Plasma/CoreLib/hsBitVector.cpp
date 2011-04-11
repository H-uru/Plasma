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

#include "hsTypes.h"
#include "hsStream.h"
#include "hsBitVector.h"
#include "hsTemplates.h"

#include <stdarg.h>

hsBitVector::hsBitVector(int b, ...)
:	fBitVectors(nil),
	fNumBitVectors(0)
{
	va_list vl;

	va_start( vl, b );

	do {
		SetBit( b, true );
	} while( (b = va_arg( vl, int )) >= 0 );

	va_end( vl );
}

hsBitVector::hsBitVector(const hsTArray<Int16>& src)
:	fBitVectors(nil),
	fNumBitVectors(0)
{
	FromList(src);
}

void hsBitVector::IGrow(UInt32 newNumBitVectors)
{
	hsAssert(newNumBitVectors > fNumBitVectors, "Growing smaller");
	UInt32 *old = fBitVectors;
	fBitVectors = TRACKED_NEW UInt32[newNumBitVectors];
	int i;
	for( i = 0; i < fNumBitVectors; i++ )
		fBitVectors[i] = old[i];
	for( ; i < newNumBitVectors; i++ )
		fBitVectors[i] = 0;
	delete [] old;
	fNumBitVectors = newNumBitVectors;
}

hsBitVector& hsBitVector::Compact()
{
	if( !fBitVectors )
		return *this;

	if( fBitVectors[fNumBitVectors-1] )
		return *this;

	int hiVec = 0;
	for( hiVec = fNumBitVectors-1; (hiVec >= 0)&& !fBitVectors[hiVec]; --hiVec );
	if( hiVec >= 0 )
	{
		UInt32 *old = fBitVectors;
		fBitVectors = TRACKED_NEW UInt32[++hiVec];
		int i;
		for( i = 0; i < hiVec; i++ )
			fBitVectors[i] = old[i];
		fNumBitVectors = hiVec;
		delete [] old;
	}
	else
	{
		Reset();
	}
	return *this;
}


void hsBitVector::Read(hsStream* s)
{
	Reset();

	s->LogReadSwap(&fNumBitVectors,"NumBitVectors");
	if( fNumBitVectors )
	{
		delete [] fBitVectors;
		fBitVectors = TRACKED_NEW UInt32[fNumBitVectors];
		int i;
		for( i = 0; i < fNumBitVectors; i++ )
			s->LogReadSwap(&fBitVectors[i],"BitVector");
	}
}

void hsBitVector::Write(hsStream* s) const
{
	s->WriteSwap32(fNumBitVectors);

	int i;
	for( i = 0; i < fNumBitVectors; i++ )
		s->WriteSwap32(fBitVectors[i]);
}

hsTArray<Int16>& hsBitVector::Enumerate(hsTArray<Int16>& dst) const
{
	dst.SetCount(0);
	hsBitIterator iter(*this);
	int i = iter.Begin();
	while( i >= 0 )
	{
		dst.Append(i);
		i = iter.Advance();
	}
	return dst;
}

hsBitVector& hsBitVector::FromList(const hsTArray<Int16>& src)
{
	Clear();
	int i;
	for( i = 0; i < src.GetCount(); i++ )
		SetBit(src[i]);
	return *this;
}

//////////////////////////////////////////////////////////////////////////

int hsBitIterator::IAdvanceVec()
{
	hsAssert((fCurrVec >= 0) && (fCurrVec < fBits.fNumBitVectors), "Invalid state to advance from");

	while( (++fCurrVec < fBits.fNumBitVectors) && !fBits.fBitVectors[fCurrVec] );

	return fCurrVec < fBits.fNumBitVectors;
}

int hsBitIterator::IAdvanceBit()
{
	do 
	{
		if( ++fCurrBit > 31 )
		{
			if( !IAdvanceVec() )
				return false;
			fCurrBit = 0;
		}
	} while( !(fBits.fBitVectors[fCurrVec] & (1 << fCurrBit)) );

	return true;
}

int hsBitIterator::Advance()
{
	if( End() )
		return -1;

	if( !IAdvanceBit() )
		return fCurrVec = -1;

	return fCurrent = (fCurrVec << 5) + fCurrBit;
}

int hsBitIterator::Begin()
{
	fCurrent = -1;
	fCurrVec = -1;
	int i;
	for( i = 0; i < fBits.fNumBitVectors; i++ )
	{
		if( fBits.fBitVectors[i] )
		{
			int j;
			for( j = 0; j < 32; j++ )
			{
				if( fBits.fBitVectors[i] & (1 << j) )
				{
					fCurrVec = i;
					fCurrBit = j;

					return fCurrent = (fCurrVec << 5) + fCurrBit;
				}
			}
		}
	}
	return fCurrent;
}

