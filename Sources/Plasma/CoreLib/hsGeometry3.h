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
#ifndef hsGGeometry3Defined
#define hsGGeometry3Defined

#include "hsTypes.h"
struct hsVector3;
struct hsPoint3;
struct hsScalarTriple;
class hsStream;

#if HS_BUILD_FOR_PS2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>

/**** vu0 inline ****/
#if 1
#define	inline_asm inline	/* inline */
#else
#define	inline_asm			/* not inline */
#endif

/******* HeadSpin *******/
typedef float	hsScalar;

/* -------------------------------------------------------------------------------- */
/* return(sqrt(x)) */
inline_asm hsScalar SqrtVU0(hsScalar x)
{
	register hsScalar ret;

	asm volatile(" \
	mfc1	$8,%1 \
	qmtc2	$8,vf4 \
	vsqrt	Q,vf4x \
	vwaitq \
	cfc2	$2,$vi22 \
	mtc1	$2,%0 \
	" :"=f" (ret) : "f" (x), "0" (ret) : "$2", "$8", "memory");

	return ret;
}

/* -------------------------------------------------------------------------------- */
/* return(1 / a) */
inline_asm hsScalar ScalarInvertVU0(hsScalar a)
{
	register hsScalar ret;

	asm volatile(" \
	mfc1	$8,%1 \
	qmtc2	$8,vf2 \
	vdiv	Q,vf0w,vf2x \
	vwaitq \
	cfc2	$2,$vi22 \
	mtc1	$2,%0 \
	" :"=f" (ret) : "f" (a), "0" (ret) : "$2", "$8", "memory");

	return ret;
}

/* -------------------------------------------------------------------------------- */
/* return(a * b) */
inline_asm hsScalar ScalarMulVU0(hsScalar a, hsScalar b)
{
	register hsScalar ret;

	asm volatile(" \
	mfc1	$8,%1 \
	qmtc2	$8,vf2 \
	mfc1	$9,%2 \
	qmtc2	$9,vf3 \
	vmul.x	vf3,vf2,vf3 \
	qmfc2   $2 ,vf3 \
	mtc1    $2,%0 \
	" :"=f" (ret) : "f" (a), "f" (b), "0" (ret) : "$2", "$8", "$9", "memory");

	return ret;
}

#endif	// PS2

/*
	If value is already close to hsScalar1, then this is a good approx. of 1/sqrt(value)
*/
static inline hsScalar hsInvSqrt(hsScalar value)
{
	hsScalar	guess;
	hsScalar	threeOverTwo = hsScalar1 + hsScalarHalf;

	value = hsScalarDiv2(value);
	guess = threeOverTwo - value;		// with initial guess = 1.0

	// repeat this line for better approx
	guess = hsScalarMul(guess, threeOverTwo - hsScalarMul(hsScalarMul(value, guess), guess));
	guess = hsScalarMul(guess, threeOverTwo - hsScalarMul(hsScalarMul(value, guess), guess));

	return guess;
}

/////////////////////////////////////////////////////////////////////////////////////////////
struct hsScalarTriple
{
//protected:
//	hsScalarTriple() : fX(privateData[0]), fY(privateData[1]), fZ(privateData[2]) {}
//	hsScalarTriple(hsScalar x, hsScalar y, hsScalar z) 
//		: fX(privateData[0]), fY(privateData[1]), fZ(privateData[2]) { fX = x, fY = y, fZ = z; }
//	
//	union {
//		u_long128	privateTemp;
//		hsScalar	privateData[4];
//	};
//public:
//
//	int operator=(const hsScalarTriple& o) { privateTemp = o.privateTemp; }
//	hsScalarTriple(const hsScalarTriple& o) : fX(privateData[0]), fY(privateData[1]), fZ(privateData[2]) 
//				{ *this = o; }
//
//	hsScalar& fX;
//	hsScalar& fY;
//	hsScalar& fZ;
protected:
	hsScalarTriple() {}
	hsScalarTriple(hsScalar x, hsScalar y, hsScalar z) : fX(x), fY(y), fZ(z) {}
public:
	hsScalar	fX, fY, fZ;

	hsScalarTriple* 	Set(hsScalar x, hsScalar y, hsScalar z) { fX= x; fY = y; fZ = z;  	return this;}
	hsScalarTriple* 	Set(const hsScalarTriple *p) { fX = p->fX; fY = p->fY; fZ = p->fZ; 	return this;}
	
	hsScalar InnerProduct(const hsScalarTriple &p) const;
	hsScalar InnerProduct(const hsScalarTriple *p) const;

//	hsScalarTriple LERP(hsScalarTriple &other, hsScalar t);
#if HS_SCALAR_IS_FIXED
	 hsScalar		Magnitude() const;
	 hsScalar		MagnitudeSquared() const;
#else

#if HS_BUILD_FOR_PS2
	 hsScalar		Magnitude() const;
#else
	 hsScalar		Magnitude() const { return hsSquareRoot(MagnitudeSquared()); }
#endif
	 hsScalar		MagnitudeSquared() const { return (fX * fX + fY * fY + fZ * fZ); }
#endif

	 hsBool IsEmpty() const { return fX == 0 && fY == 0 && fZ == 0; }

	hsScalar	operator[](int i) const;
	hsScalar&	operator[](int i);
	
	void Read(hsStream *stream);
	void Write(hsStream *stream) const;

} ATTRIBUTE_FOR_PS2;	/* SUNSOFT */


#if HS_BUILD_FOR_PS2
inline hsScalar hsScalarTriple::Magnitude() const
{
	MATRIX4	m;
	m[0] = fX;
	m[1] = fY;
	m[2] = fZ;
	return MagnitudeVU0(m);
}
#endif


///////////////////////////////////////////////////////////////////////////
inline hsScalar& hsScalarTriple::operator[] (int i) 
{
	hsAssert(i >=0 && i <3, "Bad index for hsScalarTriple::operator[]");
	 return *(&fX + i); 
}
inline hsScalar hsScalarTriple::operator[] (int i) const
{
	hsAssert(i >=0 && i <3, "Bad index for hsScalarTriple::operator[]");
	 return *(&fX + i); 
}
inline hsScalar hsScalarTriple::InnerProduct(const hsScalarTriple &p) const
{
	hsScalar tmp = fX*p.fX;
	tmp += fY*p.fY;
	tmp += fZ*p.fZ;
	return tmp;
}
inline hsScalar hsScalarTriple::InnerProduct(const hsScalarTriple *p) const
{
	hsScalar tmp = fX*p->fX;
	tmp += fY*p->fY;
	tmp += fZ*p->fZ;
	return tmp;
}

//inline hsScalarTriple hsScalarTriple::LERP(hsScalarTriple &other, hsScalar t)
//{
//	hsScalarTriple p = other - this;
//	p = p / t;
//	return this + p;
//}



/////////////////////////////////////////////////////////////////////////////////////////////
struct hsPoint3  : public hsScalarTriple {	
	hsPoint3() {};
	hsPoint3(hsScalar x, hsScalar y, hsScalar z) : hsScalarTriple(x,y,z) {}
	explicit hsPoint3(const hsScalarTriple& p) : hsScalarTriple(p) {}
	
	hsPoint3*	 Set(hsScalar x, hsScalar y, hsScalar z) { return (hsPoint3*)this->hsScalarTriple::Set(x,y,z);}
	hsPoint3*	 Set(const hsScalarTriple* p) { return (hsPoint3*)this->hsScalarTriple::Set(p) ;}

	friend inline hsPoint3 operator+(const hsPoint3& s, const hsPoint3& t);
	friend inline hsPoint3 operator+(const hsPoint3& s, const hsVector3& t);
	friend inline hsPoint3 operator-(const hsPoint3& s, const hsPoint3& t);
	friend inline hsPoint3 operator-(const hsPoint3& s);
	friend inline hsPoint3 operator*(const hsScalar& s, const hsPoint3& t);
	friend inline hsPoint3 operator*(const hsPoint3& t, const hsScalar& s);
	friend inline hsPoint3 operator/(const hsPoint3& t, const hsScalar& s);
	hsBool operator==(const hsPoint3& ss) const
	{
			return (ss.fX == fX && ss.fY == fY && ss.fZ == fZ);
	}
	hsBool operator!=(const hsPoint3& ss) const { return !(*this == ss); }
	hsPoint3 &operator+=(const hsScalarTriple &s) { fX += s.fX; fY += s.fY; fZ += s.fZ; return *this; }
	hsPoint3 &operator*=(const hsScalar s) { fX *= s; fY *= s; fZ *= s; return *this; }
} ATTRIBUTE_FOR_PS2;	/* SUNSOFT */


/////////////////////////////////////////////////////////////////////////////////////////////

struct hsVector3 : public hsScalarTriple {	

	hsVector3() {};
	hsVector3(hsScalar x, hsScalar y, hsScalar z) : hsScalarTriple(x,y,z) {}
	explicit hsVector3(const hsScalarTriple& p) : hsScalarTriple(p) { }
	hsVector3(const hsPoint3 *p1, const hsPoint3 *p2) { 
		fX = p1->fX - p2->fX, fY= p1->fY - p2->fY, fZ = p1->fZ - p2->fZ; }

	hsVector3*	 Set(hsScalar x, hsScalar y, hsScalar z) { return (hsVector3*)hsScalarTriple::Set(x,y,z); }
	hsVector3*	 Set(const hsScalarTriple* p) { return (hsVector3*)hsScalarTriple::Set(p) ;}
	hsVector3*	 Set(const hsScalarTriple* p1, const hsScalarTriple* p2) { return Set(p1->fX-p2->fX,p1->fY-p2->fY,p1->fZ-p2->fZ);}

	void			Normalize()
				{
#if HS_BUILD_FOR_PS2
					hsScalar	length = this->Magnitude();
					 hsIfDebugMessage(length == 0, "Err: Normalizing hsVector3 of length 0", 0);
					if (length == 0)
						return;
					NormalizeVU0(length, (MATRIX4)this);
#else
					hsScalar	length = this->Magnitude();
//					 hsIfDebugMessage(length == 0, "Err: Normalizing hsVector3 of length 0", 0);
					if (length == 0)
						return;
					hsScalar	invMag = hsScalarInvert(length);
					
					fX = hsScalarMul(fX, invMag);
					fY = hsScalarMul(fY, invMag);
					fZ = hsScalarMul(fZ, invMag);
#endif
				}
	inline void		Renormalize()		// if the vector is already close to unit length
				{
					hsScalar	mag2 = *this * *this;
					hsIfDebugMessage(mag2 == 0, "Err: Renormalizing hsVector3 of length 0", 0);
					if (mag2 == 0)
						return;
					hsScalar	invMag = hsInvSqrt(mag2);
					
					fX = hsScalarMul(fX, invMag);
					fY = hsScalarMul(fY, invMag);
					fZ = hsScalarMul(fZ, invMag);
				}

//	hsVector3 &Sub(const hsPoint3& s, const hsPoint3& t)
//	{	Set(s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ); 
//			return *this; };
	friend inline hsVector3 operator+(const hsVector3& s, const hsVector3& t);
	friend inline hsVector3 operator-(const hsVector3& s, const hsVector3& t);
	friend inline hsVector3 operator-(const hsVector3& s);
	friend inline hsVector3 operator*(const hsScalar& s, const hsVector3& t);
	friend inline hsVector3 operator*(const hsVector3& t, const hsScalar& s);
	friend inline hsVector3 operator/(const hsVector3& t, const hsScalar& s);
	friend inline hsScalar operator*(const hsVector3& t, const hsVector3& s);
	friend  hsVector3 operator%(const hsVector3& t, const hsVector3& s);
#if 0 // Havok reeks
	friend hsBool32 operator==(const hsVector3& s, const hsVector3& t)
	{
			return (s.fX == t.fX && s.fY == t.fY && s.fZ == t.fZ);
	}
#else // Havok reeks
	hsBool operator==(const hsVector3& ss) const
	{
			return (ss.fX == fX && ss.fY == fY && ss.fZ == fZ);
	}
#endif // Havok reeks
	hsVector3 &operator+=(const hsScalarTriple &s) { fX += s.fX; fY += s.fY; fZ += s.fZ; return *this; }
	hsVector3 &operator-=(const hsScalarTriple &s) { fX -= s.fX; fY -= s.fY; fZ -= s.fZ; return *this; }
	hsVector3 &operator*=(const hsScalar s) { fX *= s; fY *= s; fZ *= s; return *this; }
	hsVector3 &operator/=(const hsScalar s) { fX /= s; fY /= s; fZ /= s; return *this; }
} ATTRIBUTE_FOR_PS2;	/* SUNSOFT */

struct hsPoint4 {	
	hsScalar	fX, fY, fZ, fW;
	hsPoint4() {}
	hsPoint4(hsScalar x, hsScalar y, hsScalar z, hsScalar w) :  fX(x), fY(y), fZ(z), fW(w) {}
	hsScalar&		operator[](int i); 
	hsScalar		operator[](int i) const;

	hsPoint4& operator=(const hsPoint3&p) { Set(p.fX, p.fY, p.fZ, hsScalar1); return *this; }

	hsPoint4*	Set(hsScalar x, hsScalar y, hsScalar z, hsScalar w) 
		{ fX = x; fY = y; fZ = z; fW = w; return this; }
} ATTRIBUTE_FOR_PS2;	/* SUNSOFT */


inline hsVector3 operator+(const hsVector3& s, const hsVector3& t)
{
	hsVector3		result;
	
	return *result.Set(s.fX + t.fX, s.fY + t.fY, s.fZ + t.fZ);
}

inline hsVector3 operator-(const hsVector3& s, const hsVector3& t)
{
	hsVector3		result;
	
	return *result.Set(s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ);
}

// unary minus
inline hsVector3 operator-(const hsVector3& s)
{
	hsVector3		result;	
	return *result.Set(-s.fX, -s.fY, -s.fZ);
}

inline hsVector3 operator*(const hsVector3& s, const hsScalar& t)
{
	hsVector3		result;	
	return *result.Set(hsScalarMul(s.fX, t), hsScalarMul(s.fY, t), hsScalarMul(s.fZ, t));
}

inline hsVector3 operator/(const hsVector3& s, const hsScalar& t)
{
	hsVector3		result;	
	return *result.Set(hsScalarDiv(s.fX, t), hsScalarDiv(s.fY, t), hsScalarDiv(s.fZ, t));
}

inline hsVector3 operator*(const hsScalar& t, const hsVector3& s)
{
	hsVector3		result;
	
	return *result.Set(hsScalarMul(s.fX, t), hsScalarMul(s.fY, t), hsScalarMul(s.fZ, t));
}

inline hsScalar operator*(const hsVector3& t, const hsVector3& s)
{
	return hsScalarMul(t.fX, s.fX) + hsScalarMul(t.fY, s.fY) + hsScalarMul(t.fZ, s.fZ);
}

////////////////////////////////////////////////////////////////////////////

inline hsPoint3 operator+(const hsPoint3& s, const hsPoint3& t)
{
	hsPoint3		result;
	
	return *result.Set(s.fX + t.fX, s.fY + t.fY, s.fZ + t.fZ);
}

inline hsPoint3 operator+(const hsPoint3& s, const hsVector3& t)
{
	hsPoint3		result;
	
	return *result.Set(s.fX + t.fX, s.fY + t.fY, s.fZ + t.fZ);
}

inline hsPoint3 operator-(const hsPoint3& s, const hsPoint3& t)
{
	hsPoint3		result;
	
	return *result.Set(s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ);
}

// unary -
inline hsPoint3 operator-(const hsPoint3& s)
{
	hsPoint3		result;
	return *result.Set(-s.fX, -s.fY, -s.fZ);
}

inline hsPoint3 operator-(const hsPoint3& s, const hsVector3& t)
{
	hsPoint3		result;
	
	return *result.Set(s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ);
}

inline hsPoint3 operator*(const hsPoint3& s, const hsScalar& t)
{
	hsPoint3		result;	
	return *result.Set(hsScalarMul(s.fX, t), hsScalarMul(s.fY, t), hsScalarMul(s.fZ, t));
}

inline hsPoint3 operator/(const hsPoint3& s, const hsScalar& t)
{
	hsPoint3		result;	
	return *result.Set(hsScalarDiv(s.fX, t), hsScalarDiv(s.fY, t), hsScalarDiv(s.fZ, t));
}

inline hsPoint3 operator*(const hsScalar& t, const hsPoint3& s)
{
	hsPoint3		result;
	
	return *result.Set(hsScalarMul(s.fX, t), hsScalarMul(s.fY, t), hsScalarMul(s.fZ, t));
}

inline hsScalar hsPoint4::operator[] (int i) const
{
	hsAssert(i >=0 && i <4, "Bad index for hsPoint4::operator[]");
	 return *(&fX + i); 
}

inline hsScalar& hsPoint4::operator[] (int i)
{
	hsAssert(i >=0 && i <4, "Bad index for hsPoint4::operator[]");
	 return *(&fX + i); 
}

typedef hsPoint3 hsGUv;



struct hsPointNorm {
	hsPoint3	fPos;
	hsVector3	fNorm;

	void Read(hsStream* s) { fPos.Read(s); fNorm.Read(s); }
	void Write(hsStream* s) const { fPos.Write(s); fNorm.Write(s); }
} ATTRIBUTE_FOR_PS2;	/* SUNSOFT */


struct hsPlane3 {
	hsVector3 fN;
	hsScalar fD;

	hsPlane3() { }
	hsPlane3(const hsVector3* nrml, hsScalar d) 
	{ fN = *nrml; fD=d; }
	hsPlane3(const hsPoint3* pt, const hsVector3* nrml)
	{ fN = *nrml; fD = -pt->InnerProduct(nrml); }
	
	// create plane from a triangle (assumes clockwise winding of vertices)
	hsPlane3(const hsPoint3* pt1, const hsPoint3* pt2, const hsPoint3* pt3);

	hsVector3 GetNormal() const { return fN; }

	void Read(hsStream *stream);
	void Write(hsStream *stream) const;
} ATTRIBUTE_FOR_PS2;

#endif
