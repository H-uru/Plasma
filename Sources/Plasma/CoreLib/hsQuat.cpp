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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsQuat.h"

#include "HeadSpin.h"
#include "hsFastMath.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsStream.h"


//
// Quaternion class.
// For conversion to and from euler angles, see hsEuler.cpp,h.
// 

//
// Construct quat from angle (in radians) and axis of rotation
//
hsQuat::hsQuat(float rad, const hsVector3* axis)
{
    // hsAssert(rad >= -hsConstants::pi<float> && rad <= hsConstants::pi<float>,
    //          "Quat: Angle should be between -PI and PI");

    fW = cos(rad*0.5f);

    float s = sin(rad*0.5f);
    fX = axis->fX*s;
    fY = axis->fY*s;
    fZ = axis->fZ*s;
}

hsQuat hsQuat::Inverse() const
{
    hsQuat q2 = Conjugate();
    float msInv = 1.0f/q2.MagnitudeSquared();
    return (q2 * msInv);
}

hsPoint3 hsQuat::Rotate(const hsScalarTriple* v) const
{
    hsQuat qInv = Inverse();
    hsQuat qVec(v->fX, v->fY, v->fZ, 0);
    hsQuat t = qInv * qVec;
    hsQuat res = (t * (*this));
    //hsAssert(fabs(res.fW)<1e-5, "Error rotating vector");
    return hsPoint3(res.fX, res.fY, res.fZ);
}

void hsQuat::SetAngleAxis(const float rad, const hsVector3 &axis)
{
    fW = cos(rad*0.5f);

    float s = sin(rad*0.5f);
    fX = axis.fX*s;
    fY = axis.fY*s;
    fZ = axis.fZ*s; 
}

//
// Might want to Normalize before calling this
//
void hsQuat::GetAngleAxis(float *rad, hsVector3 *axis) const
{
    hsAssert((fW >= -1) && (fW <= 1), "Invalid acos argument");
    float ac = acos(fW);
    *rad = 2.0f * ac;

    float s = sin(ac);
    if (s != 0.0f)
    {
        float invS = 1.0f/s;
        axis->Set(fX*invS, fY*invS, fZ*invS);
    }
    else
        axis->Set(0,0,0);
}

//
//
//
float hsQuat::MagnitudeSquared()
{
    return (fX*fX + fY*fY + fZ*fZ + fW*fW);
}

//
//
//
float hsQuat::Magnitude()
{
    return sqrt(MagnitudeSquared());
}

//
//
//
void hsQuat::Normalize()
{
    float invMag = 1.0f/Magnitude();
    fX *= invMag;
    fY *= invMag;
    fZ *= invMag;
    fW *= invMag;
}

//
//
//
void hsQuat::NormalizeIfNeeded()
{

    float magSquared = MagnitudeSquared();
    if (magSquared == 1.0f)
        return;

    float invMag = 1.0f/sqrt(magSquared);
    fX *= invMag;
    fY *= invMag;
    fZ *= invMag;
    fW *= invMag;
}

//
// This is for a RHS.
// The quat should be normalized first.
// 
void hsQuat::MakeMatrix(hsMatrix44 *mat) const
{
    // mf horse - this is transpose of both what
    // Gems says and what i'm expecting to come
    // out of it, so i'm flipping it.
    mat->fMap[0][0] = 1.0f - 2.0f*fY*fY - 2.0f*fZ*fZ;
    mat->fMap[0][1] = 2.0f*fX*fY - 2.0f*fW*fZ;
    mat->fMap[0][2] = 2.0f*fX*fZ + 2.0f*fW*fY;
    mat->fMap[0][3] = 0.0f;

    mat->fMap[1][0] = 2.0f*fX*fY + 2.0f*fW*fZ;
    mat->fMap[1][1] = 1.0f - 2.0f*fX*fX - 2.0f*fZ*fZ;
    mat->fMap[1][2] = 2.0f*fY*fZ - 2.0f*fW*fX;
    mat->fMap[1][3] = 0.0f;

    mat->fMap[2][0] = 2.0f*fX*fZ - 2.0f*fW*fY;
    mat->fMap[2][1] = 2.0f*fY*fZ + 2.0f*fW*fX;
    mat->fMap[2][2] = 1.0f - 2.0f*fX*fX - 2.0f*fY*fY;
    mat->fMap[2][3] = 0.0f;

    mat->fMap[3][0] = 0.0f;
    mat->fMap[3][1] = 0.0f;
    mat->fMap[3][2] = 0.0f;
    mat->fMap[3][3] = 1.0f;
 
#if 0
    mat->fMap[0][0] = fW*fW + fX*fX - fY*fY - fZ*fZ;
    mat->fMap[1][0] = 2.0f*fX*fY - 2.0f*fW*fZ;
    mat->fMap[2][0] = 2.0f*fX*fZ + 2.0f*fW*fY;
    mat->fMap[3][0] = 0.0f;

    mat->fMap[0][1] = 2.0f*fX*fY + 2.0f*fW*fZ;
    mat->fMap[1][1] = fW*fW - fX*fX + fY*fY - fZ*fZ;
    mat->fMap[2][1] = 2.0f*fY*fZ - 2.0f*fW*fX;
    mat->fMap[3][1] = 0.0f;

    mat->fMap[0][2] = 2.0f*fX*fZ - 2.0f*fW*fY;
    mat->fMap[1][2] = 2.0f*fY*fZ + 2.0f*fW*fX;
    mat->fMap[2][2] = fW*fW - fX*fX - fY*fY + fZ*fZ;
    mat->fMap[3][2] = 0.0f;

    mat->fMap[0][3] = 0.0f;
    mat->fMap[1][3] = 0.0f;
    mat->fMap[2][3] = 0.0f;
    mat->fMap[3][3] = fW*fW + fX*fX + fY*fY + fZ*fZ;
#endif

    mat->NotIdentity();
}

// Binary operators
hsQuat hsQuat::operator-(const hsQuat &in) const
{
    return hsQuat(fX-in.fX, fY-in.fY, fZ-in.fZ, fW-in.fW);
}

hsQuat hsQuat::operator+(const hsQuat &in) const
{
    return hsQuat(fX+in.fX, fY+in.fY, fZ+in.fZ, fW+in.fW);
}

//
// Return quaternion product (this * in).  Note: order is important!
// To combine rotations, use the product (qSecond * qFirst),
// which gives the effect of rotating by qFirst then qSecond.
//
hsQuat hsQuat::operator*(const hsQuat &in) const
{
    hsQuat ret;
    ret.fW = (fW*in.fW - fX*in.fX - fY*in.fY - fZ*in.fZ);
    ret.fX = (fY*in.fZ - in.fY*fZ + fW*in.fX + in.fW*fX);
    ret.fY = (fZ*in.fX - in.fZ*fX + fW*in.fY + in.fW*fY);
    ret.fZ = (fX*in.fY - in.fX*fY + fW*in.fZ + in.fW*fZ);
    return ret;
}


// I/O
void hsQuat::Read(hsStream *stream)
{
    fX = stream->ReadLEFloat();
    fY = stream->ReadLEFloat();
    fZ = stream->ReadLEFloat();
    fW = stream->ReadLEFloat();
}

void hsQuat::Write(hsStream *stream)
{
    stream->WriteLEFloat(fX);
    stream->WriteLEFloat(fY);
    stream->WriteLEFloat(fZ);
    stream->WriteLEFloat(fW);
}


#if 0
//
// Interpolate on a sphere.
//
void hsQuat::SetFromSlerp(hsQuat *q1, hsQuat *q2, float t)
{
    hsAssert(t>=0.0 && t<= 1.0, "Quat slerp param must be between 0 an 1");
    float theta = acos(q1->Dot(*q2));

    float st = sin(theta);
    assert(st != 0.0);

    float s1 = sin(1.0-t)*theta / st;

    float s2 = sin(t)*theta / st;

    *this = (*q1) * s1 + (*q2) * s2;
}
#else
/*
 * Spherical linear interpolation of unit quaternions with spins
 */

#define EPSILON 1.0E-6          /* a tiny number */

void hsQuat::SetFromSlerp(const hsQuat &a, const hsQuat &b, float alpha, int spin)
//  double alpha;           /* interpolation parameter (0 to 1) */
//  Quaternion *a, *b;      /* start and end unit quaternions */
//  int spin;           /* number of extra spin rotations */
{
    float beta;          /* complementary interp parameter */
    float theta;         /* angle between A and B */
    float sin_t, cos_t;      /* sine, cosine of theta */
    float phi;           /* theta plus spins */
    int bflip;          /* use negation of B? */

    /* cosine theta = dot product of A and B */
    cos_t = a.Dot(b);

    /* if B is on opposite hemisphere from A, use -B instead */
    if (cos_t < 0.0)
    {
        cos_t = -cos_t;
        bflip = true;
    } 
    else
        bflip = false;

    /* if B is (within precision limits) the same as A,
     * just linear interpolate between A and B.
     * Can't do spins, since we don't know what direction to spin.
     */
    if (1.0 - cos_t < EPSILON) 
    {
        beta = 1.0f - alpha;
    } else 
    {               /* normal case */
//      hsAssert((cos_t >= -1) && (cos_t <= 1), "Invalid acos argument");
        theta   = acos(cos_t);
        phi     = theta + spin * hsConstants::pi<float>;
        sin_t   = sin(theta);
        hsAssert(sin_t != 0.0, "Invalid sin value in quat slerp");
        beta    = sin(theta - alpha*phi) / sin_t;
        alpha   = sin(alpha*phi) / sin_t;
    }

    if (bflip)
        alpha = -alpha;

    /* interpolate */
    fX = beta*a.fX + alpha*b.fX;
    fY = beta*a.fY + alpha*b.fY;
    fZ = beta*a.fZ + alpha*b.fZ;
    fW = beta*a.fW + alpha*b.fW;
}
#endif

//
//
//
void hsQuat::SetFromMatrix(const hsMatrix44* mat)
{
    float wSq = 0.25f*(1 + mat->fMap[0][0] + mat->fMap[1][1] + mat->fMap[2][2]);
    if (wSq > EPSILON)
    {
        fW = sqrt(wSq);
        float iw4 = 1.0f/(4.0f*fW);
        fX = (mat->fMap[2][1] - mat->fMap[1][2]) * iw4;
        fY = (mat->fMap[0][2] - mat->fMap[2][0]) * iw4;
        fZ = (mat->fMap[1][0] - mat->fMap[0][1]) * iw4;
        return;
    }

    fW = 0;
    float xSq = -0.5f*(mat->fMap[1][1] + mat->fMap[2][2]);
    if (xSq > EPSILON)
    {
        fX = sqrt(xSq);
        float ix2 = 1.0f/(2.0f*fX);
        fY = mat->fMap[1][0] * ix2;
        fZ = mat->fMap[2][0] * ix2;
        return;
    }

    fX = 0;
    float ySq = 0.5f * (1 - mat->fMap[2][2]);
    if (ySq > EPSILON)
    {
        fY = sqrt(ySq);
        fZ = mat->fMap[2][1] / (2.0f*fY);
        return;
    }

    fY = 0;
    fZ = 1;
}

// 9/15/03 - Colin
// Changed to not use hsFastMath::InvSqrt, due to errors occuring at some
// specific angles that caused Havok to blow up.
hsQuat hsQuat::QuatFromMatrix44(const hsMatrix44& mat)
{
    /* This algorithm avoids near-zero divides by looking for a large component
     * - first w, then x, y, or z.  When the trace is greater than zero,
     * |w| is greater than 1/2, which is as small as a largest component can be.
     * Otherwise, the largest diagonal entry corresponds to the largest of |x|,
     * |y|, or |z|, one of which must be larger than |w|, and at least 1/2. */
    hsQuat qu;
    float tr, s;

    const int X = 0;
    const int Y = 1;
    const int Z = 2;
    const int W = 3;
    tr = mat.fMap[X][X] + mat.fMap[Y][Y]+ mat.fMap[Z][Z];
    if (tr >= 0.0) {
        s = float(sqrt(tr + 1.f));
        qu.fW = 0.5f * s;
        s = 0.5f / s;
        qu.fX = (mat.fMap[Z][Y] - mat.fMap[Y][Z]) * s;
        qu.fY = (mat.fMap[X][Z] - mat.fMap[Z][X]) * s;
        qu.fZ = (mat.fMap[Y][X] - mat.fMap[X][Y]) * s;
    } else {
        int h = X;
        if (mat.fMap[Y][Y] > mat.fMap[X][X]) 
            h = Y;
        if (mat.fMap[Z][Z] > mat.fMap[h][h]) 
            h = Z;
        switch (h) {
#define caseMacro(i,j,k,I,J,K) \
        case I:\
        s = float(sqrt( (mat.fMap[I][I] - (mat.fMap[J][J]+mat.fMap[K][K])) + 1.f )); \
        qu.i = 0.5f * s; \
        s = 0.5f / s; \
        qu.j = (mat.fMap[I][J] + mat.fMap[J][I]) * s; \
        qu.k = (mat.fMap[K][I] + mat.fMap[I][K]) * s; \
        qu.fW = (mat.fMap[K][J] - mat.fMap[J][K]) * s; \
        break
        caseMacro(fX,fY,fZ,X,Y,Z);
        caseMacro(fY,fZ,fX,Y,Z,X);
        caseMacro(fZ,fX,fY,Z,X,Y);
        }
    }
    return (qu);
}

hsQuat& hsQuat::SetFromMatrix44(const hsMatrix44& mat)
{
    return (*this = QuatFromMatrix44(mat));
}