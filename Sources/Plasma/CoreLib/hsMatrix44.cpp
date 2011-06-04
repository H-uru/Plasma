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
#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsQuat.h"
#include "hsMatrix44.h"
#include "hsStream.h"

static hsMatrix44 myIdent = hsMatrix44().Reset();
const hsMatrix44& hsMatrix44::IdentityMatrix() { return myIdent; }

/*
	For the rotation:
         ¦        2     2                                      ¦
         ¦ 1 - (2Y  + 2Z )   2XY + 2ZW         2XZ - 2YW       ¦
         ¦                                                     ¦
         ¦                          2     2                    ¦
     M = ¦ 2XY - 2ZW         1 - (2X  + 2Z )   2YZ + 2XW       ¦
         ¦                                                     ¦
         ¦                                            2     2  ¦
         ¦ 2XZ + 2YW         2YZ - 2XW         1 - (2X  + 2Y ) ¦
         ¦                                                     ¦

	The translation is far too complex to discuss here. ;^)
*/
hsMatrix44::hsMatrix44(const hsScalarTriple &translate, const hsQuat &rotate)
{
	float xx = rotate.fX * rotate.fX;
	float xy = rotate.fX * rotate.fY;
	float xz = rotate.fX * rotate.fZ;
	float xw = rotate.fX * rotate.fW;

	float yy = rotate.fY * rotate.fY;
	float yz = rotate.fY * rotate.fZ;
	float yw = rotate.fY * rotate.fW;

	float zz = rotate.fZ * rotate.fZ;
	float zw = rotate.fZ * rotate.fW;

	fMap[0][0] = 1 - 2 * ( yy + zz ); fMap[0][1] =     2 * ( xy - zw ); fMap[0][2] =     2 * ( xz + yw ); fMap[0][3] = translate.fX;
	fMap[1][0] =     2 * ( xy + zw ); fMap[1][1] = 1 - 2 * ( xx + zz ); fMap[1][2] =     2 * ( yz - xw ); fMap[1][3] = translate.fY;
	fMap[2][0] =     2 * ( xz - yw ); fMap[2][1] =     2 * ( yz + xw ); fMap[2][2] = 1 - 2 * ( xx + yy ); fMap[2][3] = translate.fZ;
	fMap[3][0] = fMap[3][1] = fMap[3][2] = 0.0f;
	fMap[3][3] = 1.0f;
	NotIdentity();
}

void hsMatrix44::DecompRigid(hsScalarTriple &translate, hsQuat &rotate) const
{
	translate = GetTranslate();
	rotate.QuatFromMatrix44(*this);
}



#if 0

hsMatrix44& hsMatrix44::Reset()
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			fMap[i][j] = (i==j) ? hsScalar1 : 0;
		}
	}
	return *this;
}
#endif

#if 0 // Havok reeks
hsMatrix44 operator*(const hsMatrix44& a, const hsMatrix44& b)
{
	hsMatrix44 c;

	if( a.fFlags & b.fFlags & hsMatrix44::kIsIdent )
	{
		c.Reset();
		return c;
	}

	if( a.fFlags & hsMatrix44::kIsIdent )
		return b;
	if( b.fFlags & hsMatrix44::kIsIdent )
		return a;

#if HS_BUILD_FOR_PS2
	MulMatrixVU0(a.fMap, b.fMap, c.fMap);
#else
	c.fMap[0][0] = hsScalarMul(a.fMap[0][0], b.fMap[0][0]) + hsScalarMul(a.fMap[0][1], b.fMap[1][0]) + hsScalarMul(a.fMap[0][2], b.fMap[2][0]) + hsScalarMul(a.fMap[0][3], b.fMap[3][0]);
	c.fMap[0][1] = hsScalarMul(a.fMap[0][0], b.fMap[0][1]) + hsScalarMul(a.fMap[0][1], b.fMap[1][1]) + hsScalarMul(a.fMap[0][2], b.fMap[2][1]) + hsScalarMul(a.fMap[0][3], b.fMap[3][1]);
	c.fMap[0][2] = hsScalarMul(a.fMap[0][0], b.fMap[0][2]) + hsScalarMul(a.fMap[0][1], b.fMap[1][2]) + hsScalarMul(a.fMap[0][2], b.fMap[2][2]) + hsScalarMul(a.fMap[0][3], b.fMap[3][2]);
	c.fMap[0][3] = hsScalarMul(a.fMap[0][0], b.fMap[0][3]) + hsScalarMul(a.fMap[0][1], b.fMap[1][3]) + hsScalarMul(a.fMap[0][2], b.fMap[2][3]) + hsScalarMul(a.fMap[0][3], b.fMap[3][3]);

	c.fMap[1][0] = hsScalarMul(a.fMap[1][0], b.fMap[0][0]) + hsScalarMul(a.fMap[1][1], b.fMap[1][0]) + hsScalarMul(a.fMap[1][2], b.fMap[2][0]) + hsScalarMul(a.fMap[1][3], b.fMap[3][0]);
	c.fMap[1][1] = hsScalarMul(a.fMap[1][0], b.fMap[0][1]) + hsScalarMul(a.fMap[1][1], b.fMap[1][1]) + hsScalarMul(a.fMap[1][2], b.fMap[2][1]) + hsScalarMul(a.fMap[1][3], b.fMap[3][1]);
	c.fMap[1][2] = hsScalarMul(a.fMap[1][0], b.fMap[0][2]) + hsScalarMul(a.fMap[1][1], b.fMap[1][2]) + hsScalarMul(a.fMap[1][2], b.fMap[2][2]) + hsScalarMul(a.fMap[1][3], b.fMap[3][2]);
	c.fMap[1][3] = hsScalarMul(a.fMap[1][0], b.fMap[0][3]) + hsScalarMul(a.fMap[1][1], b.fMap[1][3]) + hsScalarMul(a.fMap[1][2], b.fMap[2][3]) + hsScalarMul(a.fMap[1][3], b.fMap[3][3]);
	
	c.fMap[2][0] = hsScalarMul(a.fMap[2][0], b.fMap[0][0]) + hsScalarMul(a.fMap[2][1], b.fMap[1][0]) + hsScalarMul(a.fMap[2][2], b.fMap[2][0]) + hsScalarMul(a.fMap[2][3], b.fMap[3][0]);
	c.fMap[2][1] = hsScalarMul(a.fMap[2][0], b.fMap[0][1]) + hsScalarMul(a.fMap[2][1], b.fMap[1][1]) + hsScalarMul(a.fMap[2][2], b.fMap[2][1]) + hsScalarMul(a.fMap[2][3], b.fMap[3][1]);
	c.fMap[2][2] = hsScalarMul(a.fMap[2][0], b.fMap[0][2]) + hsScalarMul(a.fMap[2][1], b.fMap[1][2]) + hsScalarMul(a.fMap[2][2], b.fMap[2][2]) + hsScalarMul(a.fMap[2][3], b.fMap[3][2]);
	c.fMap[2][3] = hsScalarMul(a.fMap[2][0], b.fMap[0][3]) + hsScalarMul(a.fMap[2][1], b.fMap[1][3]) + hsScalarMul(a.fMap[2][2], b.fMap[2][3]) + hsScalarMul(a.fMap[2][3], b.fMap[3][3]);
	
	c.fMap[3][0] = hsScalarMul(a.fMap[3][0], b.fMap[0][0]) + hsScalarMul(a.fMap[3][1], b.fMap[1][0]) + hsScalarMul(a.fMap[3][2], b.fMap[2][0]) + hsScalarMul(a.fMap[3][3], b.fMap[3][0]);
	c.fMap[3][1] = hsScalarMul(a.fMap[3][0], b.fMap[0][1]) + hsScalarMul(a.fMap[3][1], b.fMap[1][1]) + hsScalarMul(a.fMap[3][2], b.fMap[2][1]) + hsScalarMul(a.fMap[3][3], b.fMap[3][1]);
	c.fMap[3][2] = hsScalarMul(a.fMap[3][0], b.fMap[0][2]) + hsScalarMul(a.fMap[3][1], b.fMap[1][2]) + hsScalarMul(a.fMap[3][2], b.fMap[2][2]) + hsScalarMul(a.fMap[3][3], b.fMap[3][2]);
	c.fMap[3][3] = hsScalarMul(a.fMap[3][0], b.fMap[0][3]) + hsScalarMul(a.fMap[3][1], b.fMap[1][3]) + hsScalarMul(a.fMap[3][2], b.fMap[2][3]) + hsScalarMul(a.fMap[3][3], b.fMap[3][3]);
#endif

	return c;
}

hsVector3 operator*(const hsMatrix44& m, const hsVector3& p)
{	
	if( m.fFlags & hsMatrix44::kIsIdent )
		return p;

	hsVector3 rVal;

#if HS_BUILD_FOR_PS2
	MulVectorVU0(m.fMap, (MATRIX4) &p, (MATRIX4) &rVal);
#else
	rVal.fX = hsScalarMul(p.fX, m.fMap[0][0]) + hsScalarMul(p.fY, m.fMap[0][1]) + hsScalarMul(p.fZ, m.fMap[0][2]);
	rVal.fY = hsScalarMul(p.fX, m.fMap[1][0]) + hsScalarMul(p.fY, m.fMap[1][1]) + hsScalarMul(p.fZ, m.fMap[1][2]);
	rVal.fZ = hsScalarMul(p.fX, m.fMap[2][0]) + hsScalarMul(p.fY, m.fMap[2][1]) + hsScalarMul(p.fZ, m.fMap[2][2]);
#endif

	return rVal;
}
#else // Havok reeks
hsMatrix44 hsMatrix44::operator*(const hsMatrix44& b) const
{
	hsMatrix44 c;

	if( fFlags & b.fFlags & hsMatrix44::kIsIdent )
	{
		c.Reset();
		return c;
	}

	if( fFlags & hsMatrix44::kIsIdent )
		return b;
	if( b.fFlags & hsMatrix44::kIsIdent )
		return *this;

#if HS_BUILD_FOR_PS2
	MulMatrixVU0(fMap, b.fMap, c.fMap);
#else
	c.fMap[0][0] = hsScalarMul(fMap[0][0], b.fMap[0][0]) + hsScalarMul(fMap[0][1], b.fMap[1][0]) + hsScalarMul(fMap[0][2], b.fMap[2][0]) + hsScalarMul(fMap[0][3], b.fMap[3][0]);
	c.fMap[0][1] = hsScalarMul(fMap[0][0], b.fMap[0][1]) + hsScalarMul(fMap[0][1], b.fMap[1][1]) + hsScalarMul(fMap[0][2], b.fMap[2][1]) + hsScalarMul(fMap[0][3], b.fMap[3][1]);
	c.fMap[0][2] = hsScalarMul(fMap[0][0], b.fMap[0][2]) + hsScalarMul(fMap[0][1], b.fMap[1][2]) + hsScalarMul(fMap[0][2], b.fMap[2][2]) + hsScalarMul(fMap[0][3], b.fMap[3][2]);
	c.fMap[0][3] = hsScalarMul(fMap[0][0], b.fMap[0][3]) + hsScalarMul(fMap[0][1], b.fMap[1][3]) + hsScalarMul(fMap[0][2], b.fMap[2][3]) + hsScalarMul(fMap[0][3], b.fMap[3][3]);

	c.fMap[1][0] = hsScalarMul(fMap[1][0], b.fMap[0][0]) + hsScalarMul(fMap[1][1], b.fMap[1][0]) + hsScalarMul(fMap[1][2], b.fMap[2][0]) + hsScalarMul(fMap[1][3], b.fMap[3][0]);
	c.fMap[1][1] = hsScalarMul(fMap[1][0], b.fMap[0][1]) + hsScalarMul(fMap[1][1], b.fMap[1][1]) + hsScalarMul(fMap[1][2], b.fMap[2][1]) + hsScalarMul(fMap[1][3], b.fMap[3][1]);
	c.fMap[1][2] = hsScalarMul(fMap[1][0], b.fMap[0][2]) + hsScalarMul(fMap[1][1], b.fMap[1][2]) + hsScalarMul(fMap[1][2], b.fMap[2][2]) + hsScalarMul(fMap[1][3], b.fMap[3][2]);
	c.fMap[1][3] = hsScalarMul(fMap[1][0], b.fMap[0][3]) + hsScalarMul(fMap[1][1], b.fMap[1][3]) + hsScalarMul(fMap[1][2], b.fMap[2][3]) + hsScalarMul(fMap[1][3], b.fMap[3][3]);
	
	c.fMap[2][0] = hsScalarMul(fMap[2][0], b.fMap[0][0]) + hsScalarMul(fMap[2][1], b.fMap[1][0]) + hsScalarMul(fMap[2][2], b.fMap[2][0]) + hsScalarMul(fMap[2][3], b.fMap[3][0]);
	c.fMap[2][1] = hsScalarMul(fMap[2][0], b.fMap[0][1]) + hsScalarMul(fMap[2][1], b.fMap[1][1]) + hsScalarMul(fMap[2][2], b.fMap[2][1]) + hsScalarMul(fMap[2][3], b.fMap[3][1]);
	c.fMap[2][2] = hsScalarMul(fMap[2][0], b.fMap[0][2]) + hsScalarMul(fMap[2][1], b.fMap[1][2]) + hsScalarMul(fMap[2][2], b.fMap[2][2]) + hsScalarMul(fMap[2][3], b.fMap[3][2]);
	c.fMap[2][3] = hsScalarMul(fMap[2][0], b.fMap[0][3]) + hsScalarMul(fMap[2][1], b.fMap[1][3]) + hsScalarMul(fMap[2][2], b.fMap[2][3]) + hsScalarMul(fMap[2][3], b.fMap[3][3]);
	
	c.fMap[3][0] = hsScalarMul(fMap[3][0], b.fMap[0][0]) + hsScalarMul(fMap[3][1], b.fMap[1][0]) + hsScalarMul(fMap[3][2], b.fMap[2][0]) + hsScalarMul(fMap[3][3], b.fMap[3][0]);
	c.fMap[3][1] = hsScalarMul(fMap[3][0], b.fMap[0][1]) + hsScalarMul(fMap[3][1], b.fMap[1][1]) + hsScalarMul(fMap[3][2], b.fMap[2][1]) + hsScalarMul(fMap[3][3], b.fMap[3][1]);
	c.fMap[3][2] = hsScalarMul(fMap[3][0], b.fMap[0][2]) + hsScalarMul(fMap[3][1], b.fMap[1][2]) + hsScalarMul(fMap[3][2], b.fMap[2][2]) + hsScalarMul(fMap[3][3], b.fMap[3][2]);
	c.fMap[3][3] = hsScalarMul(fMap[3][0], b.fMap[0][3]) + hsScalarMul(fMap[3][1], b.fMap[1][3]) + hsScalarMul(fMap[3][2], b.fMap[2][3]) + hsScalarMul(fMap[3][3], b.fMap[3][3]);
#endif

	return c;
}

hsVector3 hsMatrix44::operator*(const hsVector3& p) const
{	
	if( fFlags & hsMatrix44::kIsIdent )
		return p;

	hsVector3 rVal;

#if HS_BUILD_FOR_PS2
	MulVectorVU0(fMap, (MATRIX4) &p, (MATRIX4) &rVal);
#else
	rVal.fX = hsScalarMul(p.fX, fMap[0][0]) + hsScalarMul(p.fY, fMap[0][1]) + hsScalarMul(p.fZ, fMap[0][2]);
	rVal.fY = hsScalarMul(p.fX, fMap[1][0]) + hsScalarMul(p.fY, fMap[1][1]) + hsScalarMul(p.fZ, fMap[1][2]);
	rVal.fZ = hsScalarMul(p.fX, fMap[2][0]) + hsScalarMul(p.fY, fMap[2][1]) + hsScalarMul(p.fZ, fMap[2][2]);
#endif

	return rVal;
}
#endif // Havok reeks

#if 0 // Havok reeks
int operator==(const hsMatrix44& s, const hsMatrix44& t)
{
	if( s.fFlags & t.fFlags & hsMatrix44::kIsIdent )
	{
		return true;
	}

	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if (s.fMap[i][j] != t.fMap[i][j])
                return false;
		}
	}

    return true;
}
#else // Havok reeks
int hsMatrix44::operator==(const hsMatrix44& ss) const
{
	if( ss.fFlags & fFlags & hsMatrix44::kIsIdent )
	{
		return true;
	}

	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if (ss.fMap[i][j] != fMap[i][j])
                return false;
		}
	}

    return true;
}
#endif // Havok reeks

hsMatrix44& hsMatrix44::Scale(const hsVector3* scale)
{
#if HS_BUILD_FOR_PS2
	MulScaleVU0(fMap, (MATRIX4 *) scale);
#else
	fMap[0][0] *= scale->fX;
	fMap[0][1] *= scale->fX;
	fMap[0][2] *= scale->fX;
	fMap[0][3] *= scale->fX;

	fMap[1][0] *= scale->fY;
	fMap[1][1] *= scale->fY;
	fMap[1][2] *= scale->fY;
	fMap[1][3] *= scale->fY;

	fMap[2][0] *= scale->fZ;
	fMap[2][1] *= scale->fZ;
	fMap[2][2] *= scale->fZ;
	fMap[2][3] *= scale->fZ;
#endif
	NotIdentity();
	return *this;
}

hsVector3 hsMatrix44::RemoveScale()
{
	hsVector3 xCol(fMap[0][0], fMap[0][1], fMap[0][2]);
	float xLen = xCol.Magnitude();
	hsVector3 yCol(fMap[1][0], fMap[1][1], fMap[1][2]);
	float yLen = yCol.Magnitude();
	hsVector3 zCol(fMap[2][0], fMap[2][1], fMap[2][2]);
	float zLen = zCol.Magnitude();

	fMap[0][0] /= xLen;
	fMap[0][1] /= xLen;
	fMap[0][2] /= xLen;
	fMap[1][0] /= yLen;
	fMap[1][1] /= yLen;
	fMap[1][2] /= yLen;
	fMap[2][0] /= zLen;
	fMap[2][1] /= zLen;
	fMap[2][2] /= zLen;

	hsVector3 oldScale(xLen, yLen, zLen);
	return oldScale;
}

hsMatrix44& hsMatrix44::Translate(const hsVector3* pt)
{
#if HS_BUILD_FOR_PS2
	TranslateVU0(fMap, (MATRIX4 *) pt);			/* SUNSOFT */
#else
	for (int i =0; i < 3; i++)
	{
		fMap[i][3] += (*pt)[i];
	}
#endif
	NotIdentity();
	return *this;
}
hsMatrix44& hsMatrix44::SetTranslate(const hsScalarTriple* pt)
{
	for (int i =0; i < 3; i++)
	{
		fMap[i][3] = (*pt)[i];
	}
	NotIdentity();
	return *this;
}
hsMatrix44& 	hsMatrix44::MakeRotateMat(int axis, hsScalar radians)
{
	Reset();
	SetRotate(axis, radians);
	NotIdentity();
	return *this;
}

hsMatrix44& 	hsMatrix44::Rotate(int axis, hsScalar radians)
{
	hsMatrix44 rMat;
	rMat.MakeRotateMat(axis, radians);
	*this = rMat * *this;
	return *this;
}

hsMatrix44& 	hsMatrix44::SetRotate(int axis, hsScalar radians)
{
	hsScalar s = hsSine(radians);
	hsScalar c = hsCosine(radians);
	int c1,c2;
	switch (axis)
	{
	case 0:
		c1 = 1;
		c2 = 2;
		break;
	case 1:
		c1 = 0;
		c2 = 2;
		break;
	case 2:
		c1 = 0;
		c2 = 1;
		break;
	}
	fMap[c1][c1] = c;
	fMap[c2][c2] = c;
	fMap[c1][c2] = s;
	fMap[c2][c1] = -s;

	NotIdentity();

	return *this;
}

void hsMatrix44::MakeXRotation(hsScalar radians)
{
	Reset();
	hsScalar s = hsSine(radians);
	hsScalar c = hsCosine(radians);

	fMap[1][1] = c;
	fMap[2][2] = c;
	fMap[1][2] = s;
	fMap[2][1] = -s;
	NotIdentity();
}

void hsMatrix44::MakeYRotation(hsScalar radians)
{
	Reset();
	hsScalar s = hsSine(radians);
	hsScalar c = hsCosine(radians);
	fMap[0][0] = c;
	fMap[2][2] = c;
	fMap[0][2] = -s;
	fMap[2][0] = s;
	NotIdentity();
}

void hsMatrix44::MakeZRotation(hsScalar radians)
{
	Reset();
	hsScalar s = hsSine(radians);
	hsScalar c = hsCosine(radians);
	fMap[0][0] = c;
	fMap[1][1] = c;
	fMap[0][1] = s;
	fMap[1][0] = -s;
	NotIdentity();
}


//
// Not a camera matrix
//
hsMatrix44& hsMatrix44::Make(const hsPoint3* f, const hsPoint3* at, const hsVector3* up)
{
	MakeTranslateMat(&hsVector3(f->fX, f->fY, f->fZ));

	hsVector3 back (f,at);	// Z
	back.Normalize();

	hsVector3 leftEar = *up % back;	// X, LHS
	leftEar.Normalize();

#if 1
	// Ignore actual up vector
	hsVector3 topHead = leftEar % back;	// Y, RHS (should be flipped)
#else
	hsVector3 topHead = *up;
#endif
	topHead.Normalize();

	for(int i = 0; i < 3; i++)
	{
		fMap[i][0] = leftEar[i];
		fMap[i][1] = back[i];
		fMap[i][2] = topHead[i];
	}
	NotIdentity();
	return *this;
}

//
// Not a camera matrix
//
hsMatrix44& hsMatrix44::MakeUpPreserving(const hsPoint3* f, const hsPoint3* at, const hsVector3* up)
{
	MakeTranslateMat(&hsVector3(f->fX, f->fY, f->fZ));

	hsVector3 topHead = *up;
	topHead.Normalize();

	hsVector3 back (f,at);	// Z
	back = -back;	// really front

	hsVector3 leftEar = *up % back;	// X
	leftEar.Normalize();

	back = (topHead % leftEar);
	back.Normalize();

	for(int i = 0; i < 3; i++)
	{
		fMap[i][0] = leftEar[i];
		fMap[i][1] = back[i];
		fMap[i][2] = topHead[i];
	}
	NotIdentity();
	return *this;
}

//
// Get info from a non-camera matrix. Vectors are normalized.
//
void	hsMatrix44::GetAxis(hsVector3* view, hsVector3 *up, hsVector3* right)
{
	if (view)
		view->Set(-fMap[0][1],-fMap[1][1],-fMap[2][1]);	
	if (right)
		right->Set(-fMap[0][0],-fMap[1][0],-fMap[2][0]);
	if (up)
		up->Set(fMap[0][2], fMap[1][2], fMap[2][2]);
}

const hsVector3 hsMatrix44::GetAxis(int i) const
{
	hsVector3 ret;
	switch(i)
	{
	case kView:
		{
			ret.Set(-fMap[0][1],-fMap[1][1],-fMap[2][1]);
			break;
		}
	case kUp:
		{
			ret.Set(fMap[0][2], fMap[1][2], fMap[2][2]);
			break;
		}
	case kRight:
		{
			ret.Set(-fMap[0][0],-fMap[1][0],-fMap[2][0]);
			break;
		}
	}
	return ret;	
}


//
// Camera matrix
//
hsMatrix44& hsMatrix44::MakeCamera(const hsPoint3* from, const hsPoint3* at,
													const hsVector3* up)
{
	hsVector3 dirZ(at, from);
	hsVector3 trans( -from->fX, -from->fY, -from->fZ );
	hsVector3 dirY, dirX;
	hsMatrix44 rmat;
	
	dirX = (*up) % dirZ; // Stop passing in down!!! // mf_flip_up - mf
	if (dirX.MagnitudeSquared())
		dirX.Normalize();

	if (dirZ.MagnitudeSquared())
		dirZ.Normalize();
	dirY = dirZ % dirX;
	if (dirY.MagnitudeSquared())
		dirY.Normalize();

	this->Reset();
	this->Translate(&trans);
	rmat.Reset();
	for(int i = 0; i < 3; i++)
	{
		rmat.fMap[0][i] = -dirX[i];
		rmat.fMap[1][i] = dirY[i];
		rmat.fMap[2][i] = dirZ[i];
	}
	rmat.NotIdentity();
	*this = rmat * *this;
	return *this;
}

void hsMatrix44::MakeCameraMatrices(const hsPoint3& from, const hsPoint3& at, const hsVector3& up, hsMatrix44& worldToCamera, hsMatrix44& cameraToWorld)
{
	hsVector3 dirZ(&at, &from);

	hsVector3 dirX(dirZ % up);
	dirX.Normalize();

	hsVector3 dirY(dirX % dirZ);
	dirY.Normalize();

	worldToCamera.Reset(false);
	cameraToWorld.Reset(false);
	int i;
	for( i = 0; i < 3; i++ )
	{
		worldToCamera.fMap[0][i] = dirX[i];
		worldToCamera.fMap[1][i] = dirY[i];
		worldToCamera.fMap[2][i] = dirZ[i];

		cameraToWorld.fMap[i][0] = dirX[i];
		cameraToWorld.fMap[i][1] = dirY[i];
		cameraToWorld.fMap[i][2] = dirZ[i];
	}
	hsPoint3 trans = -from;
	worldToCamera.fMap[0][3] = dirX.InnerProduct(trans);
	worldToCamera.fMap[1][3] = dirY.InnerProduct(trans);
	worldToCamera.fMap[2][3] = dirZ.InnerProduct(trans);

	cameraToWorld.fMap[0][3] = from.fX;
	cameraToWorld.fMap[1][3] = from.fY;
	cameraToWorld.fMap[2][3] = from.fZ;
}

void hsMatrix44::MakeEnvMapMatrices(const hsPoint3& pos, hsMatrix44* worldToCameras, hsMatrix44* cameraToWorlds)
{
	MakeCameraMatrices(pos, hsPoint3(pos.fX - 1.f, pos.fY, pos.fZ), hsVector3(0, 0, 1.f), worldToCameras[0], cameraToWorlds[0]);

	MakeCameraMatrices(pos, hsPoint3(pos.fX + 1.f, pos.fY, pos.fZ), hsVector3(0, 0, 1.f), worldToCameras[1], cameraToWorlds[1]);

	MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY + 1.f, pos.fZ), hsVector3(0, 0, 1.f), worldToCameras[2], cameraToWorlds[2]);

	MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY - 1.f, pos.fZ), hsVector3(0, 0, 1.f), worldToCameras[3], cameraToWorlds[3]);

	MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY, pos.fZ + 1.f), hsVector3(0, -1.f, 0), worldToCameras[4], cameraToWorlds[4]);

	MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY, pos.fZ - 1.f), hsVector3(0, 1.f, 0), worldToCameras[5], cameraToWorlds[5]);
}

//
// Vectors are normalized.
//
void	hsMatrix44::GetAxisFromCamera(hsVector3* view, hsVector3 *up, hsVector3* right)
{
	if (view)
		view->Set(fMap[2][0],fMap[2][1],fMap[2][2]);	
	if (right)
		right->Set(fMap[0][0],fMap[0][1],fMap[0][2]);
	if (up)
		up->Set(fMap[1][0], fMap[1][1], fMap[1][2]);
}

//
// Camera matrix
//
hsMatrix44& hsMatrix44::MakeCameraUpPreserving(const hsPoint3* from, const hsPoint3* at,
													const hsVector3* up)
{
	hsVector3 dirZ(at, from);
	hsVector3 trans( -from->fX, -from->fY, -from->fZ );
	hsVector3 dirY( up->fX, up->fY, up->fZ );
	hsVector3 dirX;
	hsMatrix44 rmat;
	
	dirX =   dirY % dirZ;
	dirX.Normalize();

	dirY.Normalize();
	dirZ = dirX % dirY;
	dirZ.Normalize();

	this->Reset();
	this->Translate(&trans);
	rmat.Reset();
	for(int i = 0; i < 3; i++)
	{
		rmat.fMap[0][i] = -dirX[i];
		rmat.fMap[1][i] = dirY[i];
		rmat.fMap[2][i] = dirZ[i];
	}
	rmat.NotIdentity();
	*this = rmat * *this;
	return *this;
}

///////////////////////////////////////////////////////

static hsScalar GetDeterminant33(const hsMatrix44* mat)
{
	return	hsScalarMul(hsScalarMul(mat->fMap[0][0], mat->fMap[1][1]), mat->fMap[2][2]) +
			hsScalarMul(hsScalarMul(mat->fMap[0][1], mat->fMap[1][2]), mat->fMap[2][0]) +
			hsScalarMul(hsScalarMul(mat->fMap[0][2], mat->fMap[1][0]), mat->fMap[2][1]) -
			hsScalarMul(hsScalarMul(mat->fMap[0][2], mat->fMap[1][1]), mat->fMap[2][0]) -
			hsScalarMul(hsScalarMul(mat->fMap[0][1], mat->fMap[1][0]), mat->fMap[2][2]) -
			hsScalarMul(hsScalarMul(mat->fMap[0][0], mat->fMap[1][2]), mat->fMap[2][1]);
}

hsMatrix44* hsMatrix44::GetTranspose(hsMatrix44* transp) const
{
	for(int i = 0 ; i < 4; i++)
		for(int j=0; j < 4; j++)
			transp->fMap[i][j] = fMap[j][i];
	return transp;
}


static inline hsScalar Determinant2(hsScalar a, hsScalar b,hsScalar c, hsScalar d)
{
	return hsScalarMul(a,d) - hsScalarMul(c,b);
}

static inline hsScalar Determinant3(hsScalar a, hsScalar b, hsScalar c,
                       hsScalar d, hsScalar e, hsScalar f,
                       hsScalar g, hsScalar h, hsScalar i)
{
	return hsScalarMul(a, Determinant2(e, f, h, i))
		-  hsScalarMul(b, Determinant2(d, f, g, i))
		+  hsScalarMul(c, Determinant2(d, e, g, h));
}

hsScalar hsMatrix44::GetDeterminant() const
{
#if HS_BUILD_FOR_PS2
	return (GetDeterminantVU0(fMap));
#else
    return (fMap[0][0]*Determinant3(fMap[1][1], fMap[2][1], fMap[3][1],
                                      fMap[1][2], fMap[2][2], fMap[3][2],
                                      fMap[1][3], fMap[2][3], fMap[3][3]) - 
            fMap[1][0]*Determinant3(fMap[0][1], fMap[2][1], fMap[3][1],
                                      fMap[0][2], fMap[2][2], fMap[3][2],
                                      fMap[0][3], fMap[2][3], fMap[3][3]) +
            fMap[2][0]*Determinant3(fMap[0][1], fMap[1][1], fMap[3][1],
                                      fMap[0][2], fMap[1][2], fMap[3][2],
                                      fMap[0][3], fMap[1][3], fMap[3][3]) -
            fMap[3][0]*Determinant3(fMap[0][1], fMap[1][1], fMap[2][1],
                                      fMap[0][2], fMap[1][2], fMap[2][2],
                                      fMap[0][3], fMap[1][3], fMap[2][3]));
#endif
}


hsMatrix44 *hsMatrix44::GetAdjoint(hsMatrix44 *adj) const
{
#if HS_BUILD_FOR_PS2
	GetAdjointVU0(fMap, adj->fMap);
#else
    float   a1, a2, a3, a4, b1, b2, b3, b4;
    float   c1, c2, c3, c4, d1, d2, d3, d4;
/*
 *     calculate the adjoint of a 4x4 matrix
 *
 *      Let  a   denote the minor determinant of matrix A obtained by
 *           ij
 *
 *      deleting the ith row and jth column from A.
 *
 *                    i+j
 *     Let  b   = (-1)    a
 *          ij            ji
 *
 *    The matrix B = (b  ) is the adjoint of A
 *                     ij
 */

    /* assign to individual variable names to aid  */
    /* selecting correct values  */

    a1 = fMap[0][0];
    b1 = fMap[0][1];
    c1 = fMap[0][2];
    d1 = fMap[0][3];

    a2 = fMap[1][0];
    b2 = fMap[1][1];
    c2 = fMap[1][2];
    d2 = fMap[1][3];

    a3 = fMap[2][0];
    b3 = fMap[2][1];
    c3 = fMap[2][2];
    d3 = fMap[2][3];

    a4 = fMap[3][0];
    b4 = fMap[3][1];
    c4 = fMap[3][2];
    d4 = fMap[3][3];

    /*
     * row column labeling reversed since we transpose rows & columns
     */

    adj->fMap[0][0] = Determinant3(b2, b3, b4, c2, c3, c4, d2, d3, d4);
    adj->fMap[1][0] = -Determinant3(a2, a3, a4, c2, c3, c4, d2, d3, d4);
    adj->fMap[2][0] = Determinant3(a2, a3, a4, b2, b3, b4, d2, d3, d4);
    adj->fMap[3][0] = -Determinant3(a2, a3, a4, b2, b3, b4, c2, c3, c4);

    adj->fMap[0][1] = -Determinant3(b1, b3, b4, c1, c3, c4, d1, d3, d4);
    adj->fMap[1][1] = Determinant3(a1, a3, a4, c1, c3, c4, d1, d3, d4);
    adj->fMap[2][1] = -Determinant3(a1, a3, a4, b1, b3, b4, d1, d3, d4);
    adj->fMap[3][1] = Determinant3(a1, a3, a4, b1, b3, b4, c1, c3, c4);

    adj->fMap[0][2] = Determinant3(b1, b2, b4, c1, c2, c4, d1, d2, d4);
    adj->fMap[1][2] = -Determinant3(a1, a2, a4, c1, c2, c4, d1, d2, d4);
    adj->fMap[2][2] = Determinant3(a1, a2, a4, b1, b2, b4, d1, d2, d4);
    adj->fMap[3][2] = -Determinant3(a1, a2, a4, b1, b2, b4, c1, c2, c4);

    adj->fMap[0][3] = -Determinant3(b1, b2, b3, c1, c2, c3, d1, d2, d3);
    adj->fMap[1][3] = Determinant3(a1, a2, a3, c1, c2, c3, d1, d2, d3);
    adj->fMap[2][3] = -Determinant3(a1, a2, a3, b1, b2, b3, d1, d2, d3);
    adj->fMap[3][3] = Determinant3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
#endif
	adj->NotIdentity();
    return adj;
}

hsMatrix44* hsMatrix44::GetInverse(hsMatrix44* inverse) const
{
	hsScalar det = GetDeterminant();
    int i,j;

    if (det == 0.0f)
	{
		inverse->Reset();
		return inverse;
	}

	det = hsScalarInvert(det);
    GetAdjoint(inverse);
#if HS_BUILD_FOR_PS2
	MatMulVU0(inverse->fMap, det);
#else
    for (i=0; i<4; i++)
        for (j=0; j<4; j++)
            inverse->fMap[i][j] *= det;
#endif
    return inverse;
}

hsMatrix44& hsMatrix44::SetScale(const hsVector3* pt)
{
	for (int i =0; i < 3; i++)
	{
		fMap[i][i] = (*pt)[i];
	}
	NotIdentity();
	return *this;
}

hsMatrix44& hsMatrix44::MakeScaleMat(const hsVector3* pt)
{
	Reset();
	SetScale(pt);
	return *this;
}

hsMatrix44 &hsMatrix44::MakeTranslateMat(const hsVector3 *trans)
{
	Reset();
	SetTranslate(trans);
	return *this;
}

hsVector3* hsMatrix44::GetTranslate(hsVector3 *pt) const
{
	for (int i =0; i < 3; i++)
	{
		(*pt)[i] = fMap[i][3];
	}
	return pt;
}

const hsPoint3 hsMatrix44::GetTranslate() const
{
	hsPoint3 pt;
	for (int i =0; i < 3; i++)
	{
		(pt)[i] = fMap[i][3];
	}
	return pt;
}


hsPoint3*  hsMatrix44::MapPoints(long count, hsPoint3 points[]) const
{
	if( !(fFlags & hsMatrix44::kIsIdent) )
	{
		int i;
		for(i = 0; i < count; i++)
		{
			points[i] = *this * points[i];	
		}
	}
	return points;
}

hsBool hsMatrix44::IsIdentity(void)
{
	hsBool retVal = true;
	int i, j;
	for( i = 0; i < 4; i++ )
	{
		for( j = 0; j < 4; j++ )
		{
#if 0 // IDENTITY_CRISIS
			if( i == j)
			{
				if (fMap[i][j] != hsScalar1) 
				{
					NotIdentity();
					retVal = false;
				}
			}
			else
			{
				if( fMap[i][j] != 0 )
				{
					NotIdentity();
					retVal = false;
				}
			}
#else // IDENTITY_CRISIS
			const hsScalar kEPS = 1.e-5f;
			if( i == j)
			{
				if( (fMap[i][j] < hsScalar1-kEPS) || (fMap[i][j] > hsScalar1+kEPS) ) 
				{
					NotIdentity();
					retVal = false;
				}
				else
				{
					fMap[i][j] = hsScalar1;
				}
			}
			else
			{
				if( (fMap[i][j] < -kEPS) || (fMap[i][j] > kEPS) )
				{
					NotIdentity();
					retVal = false;
				}
				else
				{
					fMap[i][j] = 0;
				}
			}
#endif // IDENTITY_CRISIS
		}
	}
	if( retVal )
		fFlags |= kIsIdent;
	return retVal;
}

hsBool hsMatrix44::GetParity() const
{
	if( fFlags & kIsIdent )
		return false;

	hsVector3* rows[3];
	rows[0] = (hsVector3*)&fMap[0][0];
	rows[1] = (hsVector3*)&fMap[1][0];
	rows[2] = (hsVector3*)&fMap[2][0];

	hsVector3 zeroXone = *rows[0] % *rows[1];
	return zeroXone.InnerProduct(rows[2]) < 0;
}

void hsMatrix44::Read(hsStream *stream)
{
	if (stream->ReadBool())
	{
		int i,j;
		for(i=0; i<4; i++)
			for(j=0; j<4; j++)
#if HS_SCALAR_IS_FIXED
				fMap[i][j] = stream->ReadSwap32();
#endif
#if HS_SCALAR_IS_FLOAT
				fMap[i][j] = stream->ReadSwapFloat();
#endif
		IsIdentity();
	}
	else
		Reset();
}

void hsMatrix44::Write(hsStream *stream)
{
	hsBool ident = IsIdentity();
	stream->WriteBool(!ident);
	if (!ident)
	{
		int i,j;
		for(i=0; i<4; i++)
			for(j=0; j<4; j++)
#if HS_SCALAR_IS_FIXED
				stream->WriteSwap32(fMap[i][j]);
#endif
#if HS_SCALAR_IS_FLOAT
				stream->WriteSwapFloat(fMap[i][j]);			
#endif
	}
}
