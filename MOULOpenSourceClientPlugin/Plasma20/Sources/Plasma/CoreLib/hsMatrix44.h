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
#ifndef HSMATRIX44_inc
#define  HSMATRIX44_inc

#include "hsTypes.h"
#include "hsGeometry3.h"

class hsQuat;

////////////////////////////////////////////////////////////////////////////
class hsStream;
struct hsMatrix44 {
	enum {
		kIsIdent	= 0x1
	};

	enum {
		kRight	= 0,
		kUp,
		kView
	};
	hsScalar 			fMap[4][4];
	UInt32				fFlags;

	hsMatrix44() : fFlags(0) {}
	hsMatrix44(const hsScalarTriple &translate, const hsQuat &rotate);

	void DecompRigid(hsScalarTriple &translate, hsQuat &rotate) const;

	static const hsMatrix44& IdentityMatrix();

	// worldToCameras and cameraToWorlds are arrays of 6 matrices. Returned are LEFT,RIGHT,FRONT,BACK,TOP,BOTTOM.
	static void MakeEnvMapMatrices(const hsPoint3& pos, hsMatrix44* worldToCameras, hsMatrix44* cameraToWorlds);
	static void MakeCameraMatrices(const hsPoint3& from, const hsPoint3& at, const hsVector3& up, hsMatrix44& worldToCamera, hsMatrix44& cameraToWorld);

	// Concat transform
	hsMatrix44&		Translate(const hsVector3 *);
	hsMatrix44&		Scale(const hsVector3 *);
	hsMatrix44& 	Rotate(int axis, hsScalar radians);

	hsMatrix44&		Reset(hsBool asIdent=true) 
	{
		fMap[0][0] = 1.0f; fMap[0][1] = 0.0f; fMap[0][2] = 0.0f; fMap[0][3]  = 0.0f;
		fMap[1][0] = 0.0f; fMap[1][1] = 1.0f; fMap[1][2] = 0.0f; fMap[1][3]  = 0.0f;
		fMap[2][0] = 0.0f; fMap[2][1] = 0.0f; fMap[2][2] = 1.0f; fMap[2][3]  = 0.0f;
		fMap[3][0] = 0.0f; fMap[3][1] = 0.0f; fMap[3][2] = 0.0f; fMap[3][3]  = 1.0f;

		fFlags = asIdent ? kIsIdent : 0;
		return *this;
	}

	// Create matrix from scratch
	hsMatrix44&		MakeTranslateMat(const hsVector3 *trans);
	hsMatrix44&		MakeScaleMat(const hsVector3 *scale);
	hsMatrix44& 	MakeRotateMat(int axis, hsScalar radians);
	hsMatrix44&		Make(const hsPoint3* from, const hsPoint3* at, 
						const hsVector3* up);	// Not a camera matrix
	hsMatrix44&		MakeUpPreserving(const hsPoint3* from, const hsPoint3* at, 
						const hsVector3* up);	// Not a camera matrix
	// Camera matrix
	hsMatrix44&		MakeCamera(const hsPoint3* from, const hsPoint3* at,
						const hsVector3* up);
	hsMatrix44&		MakeCameraUpPreserving(const hsPoint3* from, const hsPoint3* at,
						const hsVector3* up);

	hsBool			GetParity() const;
	hsScalar		GetDeterminant() const;
	hsMatrix44*		GetInverse(hsMatrix44* inverse) const;
	hsMatrix44*		GetTranspose(hsMatrix44* inverse) const;
	hsMatrix44*		GetAdjoint(hsMatrix44* adjoint) const;
	hsVector3*		GetTranslate(hsVector3 *pt) const;
	hsPoint3*		GetTranslate(hsPoint3 *pt) const 
		{ 	return (hsPoint3*)GetTranslate((hsVector3*)pt); }
	const hsPoint3  GetTranslate() const;
	void			GetAxis(hsVector3* view, hsVector3 *up, hsVector3* right);
	void			GetAxisFromCamera(hsVector3* view, hsVector3 *up, hsVector3* right);

	const hsVector3 GetAxis(int i) const;

	// Change component of matrix
	hsMatrix44&		SetTranslate(const hsScalarTriple *);
	hsMatrix44&		SetScale(const hsVector3 *);
	hsMatrix44& 	SetRotate(int axis, hsScalar radians);

	hsVector3		RemoveScale();		// returns old scale
	void MakeXRotation(hsScalar radians);
	void MakeYRotation(hsScalar radians);
	void MakeZRotation(hsScalar radians);


#if 0 // Havok reeks
	friend hsPoint3		operator*(const hsMatrix44& m, const hsPoint3& p)
					{	
						if( m.fFlags & hsMatrix44::kIsIdent )
							return p;

						hsPoint3 rVal;
						rVal.fX = hsScalarMul(p.fX, m.fMap[0][0]) + hsScalarMul(p.fY, m.fMap[0][1]) + hsScalarMul(p.fZ, m.fMap[0][2]) + m.fMap[0][3];
						rVal.fY = hsScalarMul(p.fX, m.fMap[1][0]) + hsScalarMul(p.fY, m.fMap[1][1]) + hsScalarMul(p.fZ, m.fMap[1][2])  + m.fMap[1][3];
						rVal.fZ = hsScalarMul(p.fX, m.fMap[2][0]) + hsScalarMul(p.fY, m.fMap[2][1]) + hsScalarMul(p.fZ, m.fMap[2][2])  + m.fMap[2][3];
						return rVal;
					}
	friend  hsVector3 operator*(const hsMatrix44& m, const hsVector3& p);
	friend hsMatrix44	operator*(const hsMatrix44& a, const hsMatrix44& b);
#else // Havok reeks
	hsPoint3		operator*(const hsPoint3& p) const
					{	
						if( fFlags & hsMatrix44::kIsIdent )
							return p;

						hsPoint3 rVal;
						rVal.fX = hsScalarMul(p.fX, fMap[0][0]) + hsScalarMul(p.fY, fMap[0][1]) + hsScalarMul(p.fZ, fMap[0][2]) + fMap[0][3];
						rVal.fY = hsScalarMul(p.fX, fMap[1][0]) + hsScalarMul(p.fY, fMap[1][1]) + hsScalarMul(p.fZ, fMap[1][2])  + fMap[1][3];
						rVal.fZ = hsScalarMul(p.fX, fMap[2][0]) + hsScalarMul(p.fY, fMap[2][1]) + hsScalarMul(p.fZ, fMap[2][2])  + fMap[2][3];
						return rVal;
					}
	hsVector3 operator*(const hsVector3& p) const;
	hsMatrix44	operator*(const hsMatrix44& b) const;
#endif // Havok reeks
	
	hsPoint3*			MapPoints(long count, hsPoint3 points[]) const;
	
	hsBool			IsIdentity(void);
	void			NotIdentity() { fFlags &= ~kIsIdent; }

#if 0 // Havok reeks
	friend int operator==(const hsMatrix44& s, const hsMatrix44& t);
	friend int operator!=(const hsMatrix44& s, const hsMatrix44& t);
#else // Havok reeks
	hsBool operator==(const hsMatrix44& ss) const;
	hsBool operator!=(const hsMatrix44& ss) const { return !(ss == *this); }
#endif // Havok reeks

	void Read(hsStream *stream);
	void Write(hsStream *stream);
} ATTRIBUTE_FOR_PS2;	/* SUNSOFT */

#if 0 // Havok reeks
inline int operator!=(const hsMatrix44& s, const hsMatrix44& t)
{
    return (!(s==t));
}
#endif // Havok reeks

////////////////////////////////////////////////////////////////////////////
#endif
