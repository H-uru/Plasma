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
#ifndef hsWideDefined
#define hsWideDefined

#include "hsTypes.h"

struct hsWide {
	Int32	fHi;
	UInt32	fLo;

	hsWide*	Set(Int32 lo) { fLo = lo; if (lo < 0) fHi = -1L; else fHi = 0; return this; }
	hsWide*	Set(Int32 hi, UInt32 lo) { fHi = hi; fLo = lo; return this; }

	inline hsBool	IsNeg() const { return fHi < 0; }
	inline hsBool	IsPos() const { return fHi > 0 || (fHi == 0 && fLo != 0); }
	inline hsBool	IsZero() const { return fHi == 0 && fLo == 0; }
	inline hsBool	IsWide() const;


	hsBool operator==(const hsWide& b) const { return fHi == b.fHi && fLo == b.fLo; }
	hsBool operator<(const hsWide& b) const { return fHi < b.fHi || fHi == b.fHi && fLo < b.fLo; }
	hsBool operator>( const hsWide& b) const { return fHi > b.fHi || fHi == b.fHi && fLo > b.fLo; }
	hsBool operator!=( const hsWide& b) const { return !( *this == b); }
	hsBool operator<=(const hsWide& b) const { return !(*this > b); }
	hsBool operator>=(const hsWide& b) const { return !(*this < b); }

	inline hsWide*	Negate();
	inline hsWide*	Add(Int32 scaler);
	inline hsWide*	Add(const hsWide* a);
	inline hsWide*	Sub(const hsWide* a);
	inline hsWide*	ShiftLeft(unsigned shift);
	inline hsWide*	ShiftRight(unsigned shift);
	inline hsWide*	RoundRight(unsigned shift);

	inline Int32	AsLong() const;				// return bits 31-0, checking for over/under flow
	inline hsFixed	AsFixed() const;			// return bits 47-16, checking for over/under flow
	inline hsFract	AsFract() const;			// return bits 61-30, checking for over/under flow

	hsWide*	Mul(Int32 a);					// this updates the wide
	hsWide*	Mul(Int32 a, Int32 b);			// this sets the wide
	hsWide*	Div(Int32 denom);				// this updates the wide
	hsWide*	Div(const hsWide* denom);		// this updates the wide

	hsFixed	FixDiv(const hsWide* denom) const;
	hsFract	FracDiv(const hsWide* denom) const;

	Int32	Sqrt() const;
	Int32	CubeRoot() const;

#if HS_CAN_USE_FLOAT
	double	AsDouble() const { return fHi * double(65536) * double(65536) + fLo; }
	hsWide* Set(double d) 
	{ 
		Int32 hi = Int32(d / double(65536) / double(65536));
		Int32 lo = Int32(d - double(hi));
		return Set(hi, lo);
	}
#endif

};

const hsWide kPosInfinity64 = { kPosInfinity32, 0xffffffff };
const hsWide kNegInfinity64 = { kNegInfinity32, 0 };

/////////////////////// Inline implementations ///////////////////////

#define	TOP2BITS(n)	(UInt32(n) >> 30)
#define	TOP3BITS(n)	(UInt32(n) >> 29)

#if HS_PIN_MATH_OVERFLOW && HS_DEBUG_MATH_OVERFLOW
	#define hsSignalMathOverflow()	hsDebugMessage("Math overflow", 0)
	#define hsSignalMathUnderflow()	hsDebugMessage("Math underflow", 0)
#else
	#define hsSignalMathOverflow()
	#define hsSignalMathUnderflow()
#endif

#define WIDE_ISNEG(hi, lo)						(Int32(hi) < 0)
#define WIDE_LESSTHAN(hi, lo, hi2, lo2)				((hi) < (hi2) || (hi) == (hi2) && (lo) < (lo2))
#define WIDE_SHIFTLEFT(outH, outL, inH, inL, shift)		do { (outH) = ((inH) << (shift)) | ((inL) >> (32 - (shift))); (outL) = (inL) << (shift); } while (0)
#define WIDE_NEGATE(hi, lo)						do { (hi) = ~(hi); if (((lo) = -Int32(lo)) == 0) (hi) += 1; } while (0) 
#define WIDE_ADDPOS(hi, lo, scaler)				do { UInt32 tmp = (lo) + (scaler); if (tmp < (lo)) (hi) += 1; (lo) = tmp; } while (0)
#define WIDE_SUBWIDE(hi, lo, subhi, sublo)			do { (hi) -= (subhi); if ((lo) < (sublo)) (hi) -= 1; (lo) -= (sublo); } while (0) 

/////////////////////// Inline implementations ///////////////////////

inline hsWide* hsWide::Negate()
{
	WIDE_NEGATE(fHi, fLo);
	
	return this;
}

inline hsWide* hsWide::Add(Int32 scaler)
{
	if (scaler >= 0)
		WIDE_ADDPOS(fHi, fLo, scaler);
	else
	{	scaler = -scaler;
		if (fLo < UInt32(scaler))
			fHi--;
		fLo -= scaler;
	}

	return this;
}

inline hsWide* hsWide::Add(const hsWide* a)
{
	UInt32	newLo = fLo + a->fLo;

	fHi += a->fHi;
	if (newLo < (fLo | a->fLo))
		fHi++;
	fLo = newLo;

	return this;
}

inline hsWide* hsWide::Sub(const hsWide* a)
{
	WIDE_SUBWIDE(fHi, fLo, a->fHi, a->fLo);

	return this;
}

inline hsWide* hsWide::ShiftLeft(unsigned shift)
{
	WIDE_SHIFTLEFT(fHi, fLo, fHi, fLo, shift);

	return this;
}

inline hsWide* hsWide::ShiftRight(unsigned shift)
{
	fLo = (fLo >> shift) | (fHi << (32 - shift));
	fHi = fHi >> shift;		// fHi >>= shift;   Treated as logical shift on CW9-WIN32, which breaks for fHi < 0

	return this;
}

inline hsWide* hsWide::RoundRight(unsigned shift)
{
	return this->Add(1L << (shift - 1))->ShiftRight(shift);
}

inline Int32 hsWide::AsLong() const
{
#if HS_PIN_MATH_OVERFLOW
	if (fHi > 0 || fHi == 0 && (Int32)fLo < 0)
	{	hsSignalMathOverflow();
		return kPosInfinity32;
	}
	if (fHi < -1L || fHi == -1L && (Int32)fLo >= 0)
	{	hsSignalMathOverflow();
		return kNegInfinity32;
	}
#endif
	return (Int32)fLo;
}

inline hsBool hsWide::IsWide() const
{
	return (fHi > 0 || fHi == 0 && (Int32)fLo < 0) || (fHi < -1L || fHi == -1L && (Int32)fLo >= 0);
}

inline hsFixed hsWide::AsFixed() const
{
	hsWide tmp = *this;

	return tmp.RoundRight(16)->AsLong();
}

inline hsFract hsWide::AsFract() const
{
	hsWide tmp = *this;

	return tmp.RoundRight(30)->AsLong();
}

#endif
