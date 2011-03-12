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
#ifndef HSQUAT_inc
#define HSQUAT_inc

#include "hsGeometry3.h"

struct hsMatrix44;

//
// Quaternion class.
// For conversion to and from euler angles, see hsEuler.cpp,h.
// 
class hsQuat {

public:
	hsScalar fX,fY,fZ,fW;	

	// Constructors
	hsQuat(){}
	hsQuat(hsScalar X, hsScalar Y, hsScalar Z, hsScalar W) : 
		fX(X), fY(Y), fZ(Z), fW(W) {}
	hsQuat(const hsQuat& a) { fX = a.fX; fY = a.fY; fZ = a.fZ; fW = a.fW; } 
	hsQuat(hsScalar af[4]) { fX = af[0]; fY = af[1]; fZ = af[2]; fW = af[3]; }
	hsQuat(hsScalar rad, const hsVector3* axis);

	static hsQuat	QuatFromMatrix44(const hsMatrix44& mat);
	hsQuat&	SetFromMatrix44(const hsMatrix44& mat);
	void SetFromMatrix(const hsMatrix44 *mat);
	void SetFromSlerp(const hsQuat &q1, const hsQuat &q2, hsScalar t, int spin=0);
	void Set(hsScalar X, hsScalar Y, hsScalar Z, hsScalar W)  
		{ fX = X; fY = Y; fZ = Z; fW = W; }
	void GetAngleAxis(hsScalar *rad, hsVector3 *axis) const;
	void SetAngleAxis(const hsScalar rad, const hsVector3 &axis);
	hsPoint3 Rotate(const hsScalarTriple* v);
	
	// Access operators
	hsScalar& operator[](int i) { return (&fX)[i]; }     
	const hsScalar& operator[](int i) const { return (&fX)[i]; }  

	// Unary operators
	hsQuat operator-() const { return(hsQuat(-fX,-fY,-fZ,-fW)); } 
	hsQuat operator+() const { return *this; } 

	// Comparison
	int operator==(const hsQuat& a) const
		{ return (fX==a.fX && fY==a.fY && fZ==a.fZ && fW==a.fW); }
	void Identity() { fX = fY = fZ = (hsScalar)0.0; fW = (hsScalar) 1.0; }
	int IsIdentity() const
		{ return (fX==0.0 && fY==0.0 && fZ==0.0 && fW==1.0); }
	void Normalize();  
	void NormalizeIfNeeded();  
	void MakeMatrix(hsMatrix44 *mat) const;
	hsScalar Magnitude();
	hsScalar MagnitudeSquared();
	hsQuat Conjugate() const
		{ return hsQuat(-fX,-fY,-fZ,fW); }
	hsQuat Inverse();
	// Binary operators
	hsQuat operator-(const hsQuat&) const;
	hsQuat operator+(const hsQuat&) const;
	hsQuat operator*(const hsQuat&) const;
	hsQuat operator*(hsScalar f) const
		{ return hsQuat(fX*f,fY*f,fZ*f,fW*f); }
	hsQuat operator/(hsScalar f) const
		{ return hsQuat(fX/f,fY/f,fZ/f,fW/f); }
	hsQuat operator/(const hsQuat&) const;

	hsScalar Dot(const hsQuat &q2) const
		{	return (fX*q2.fX + fY*q2.fY + fZ*q2.fZ + fW*q2.fW); }

	// I/O
	void Read(hsStream *stream);
	void Write(hsStream *stream);

	};
#endif
