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
      Cyan Worlds, I
      nc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsMatrix44.h"


#include "HeadSpin.h"
#include "hsGeometry3.h"
#include "hsQuat.h"
#include "hsStream.h"

#include <cmath>
#include <string_theory/format>

#ifdef HS_BUILD_FOR_APPLE
#include <Accelerate/Accelerate.h>
#endif

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
    rotate = hsQuat::QuatFromMatrix44(*this);
}

#ifdef HS_BUILD_FOR_APPLE
hsMatrix44 hsMatrix44::mult_accelerate(const hsMatrix44 &a, const hsMatrix44 &b)
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

    vDSP_mmul(const_cast<float*>(a.fMap[0]), 1, const_cast<float*>(b.fMap[0]), 1, c.fMap[0], 1, 4, 4, 4);

    return c;
}
#endif

hsMatrix44 hsMatrix44::mult_fpu(const hsMatrix44 &a, const hsMatrix44 &b)
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

    c.fMap[0][0] = (a.fMap[0][0] * b.fMap[0][0]) + (a.fMap[0][1] * b.fMap[1][0]) + (a.fMap[0][2] * b.fMap[2][0]) + (a.fMap[0][3] * b.fMap[3][0]);
    c.fMap[0][1] = (a.fMap[0][0] * b.fMap[0][1]) + (a.fMap[0][1] * b.fMap[1][1]) + (a.fMap[0][2] * b.fMap[2][1]) + (a.fMap[0][3] * b.fMap[3][1]);
    c.fMap[0][2] = (a.fMap[0][0] * b.fMap[0][2]) + (a.fMap[0][1] * b.fMap[1][2]) + (a.fMap[0][2] * b.fMap[2][2]) + (a.fMap[0][3] * b.fMap[3][2]);
    c.fMap[0][3] = (a.fMap[0][0] * b.fMap[0][3]) + (a.fMap[0][1] * b.fMap[1][3]) + (a.fMap[0][2] * b.fMap[2][3]) + (a.fMap[0][3] * b.fMap[3][3]);

    c.fMap[1][0] = (a.fMap[1][0] * b.fMap[0][0]) + (a.fMap[1][1] * b.fMap[1][0]) + (a.fMap[1][2] * b.fMap[2][0]) + (a.fMap[1][3] * b.fMap[3][0]);
    c.fMap[1][1] = (a.fMap[1][0] * b.fMap[0][1]) + (a.fMap[1][1] * b.fMap[1][1]) + (a.fMap[1][2] * b.fMap[2][1]) + (a.fMap[1][3] * b.fMap[3][1]);
    c.fMap[1][2] = (a.fMap[1][0] * b.fMap[0][2]) + (a.fMap[1][1] * b.fMap[1][2]) + (a.fMap[1][2] * b.fMap[2][2]) + (a.fMap[1][3] * b.fMap[3][2]);
    c.fMap[1][3] = (a.fMap[1][0] * b.fMap[0][3]) + (a.fMap[1][1] * b.fMap[1][3]) + (a.fMap[1][2] * b.fMap[2][3]) + (a.fMap[1][3] * b.fMap[3][3]);

    c.fMap[2][0] = (a.fMap[2][0] * b.fMap[0][0]) + (a.fMap[2][1] * b.fMap[1][0]) + (a.fMap[2][2] * b.fMap[2][0]) + (a.fMap[2][3] * b.fMap[3][0]);
    c.fMap[2][1] = (a.fMap[2][0] * b.fMap[0][1]) + (a.fMap[2][1] * b.fMap[1][1]) + (a.fMap[2][2] * b.fMap[2][1]) + (a.fMap[2][3] * b.fMap[3][1]);
    c.fMap[2][2] = (a.fMap[2][0] * b.fMap[0][2]) + (a.fMap[2][1] * b.fMap[1][2]) + (a.fMap[2][2] * b.fMap[2][2]) + (a.fMap[2][3] * b.fMap[3][2]);
    c.fMap[2][3] = (a.fMap[2][0] * b.fMap[0][3]) + (a.fMap[2][1] * b.fMap[1][3]) + (a.fMap[2][2] * b.fMap[2][3]) + (a.fMap[2][3] * b.fMap[3][3]);

    c.fMap[3][0] = (a.fMap[3][0] * b.fMap[0][0]) + (a.fMap[3][1] * b.fMap[1][0]) + (a.fMap[3][2] * b.fMap[2][0]) + (a.fMap[3][3] * b.fMap[3][0]);
    c.fMap[3][1] = (a.fMap[3][0] * b.fMap[0][1]) + (a.fMap[3][1] * b.fMap[1][1]) + (a.fMap[3][2] * b.fMap[2][1]) + (a.fMap[3][3] * b.fMap[3][1]);
    c.fMap[3][2] = (a.fMap[3][0] * b.fMap[0][2]) + (a.fMap[3][1] * b.fMap[1][2]) + (a.fMap[3][2] * b.fMap[2][2]) + (a.fMap[3][3] * b.fMap[3][2]);
    c.fMap[3][3] = (a.fMap[3][0] * b.fMap[0][3]) + (a.fMap[3][1] * b.fMap[1][3]) + (a.fMap[3][2] * b.fMap[2][3]) + (a.fMap[3][3] * b.fMap[3][3]);

    return c;
}

// CPU-optimized functions requiring dispatch
hsCpuFunctionDispatcher<hsMatrix44::mat_mult_ptr> hsMatrix44::mat_mult {
    &hsMatrix44::mult_fpu,
    nullptr,            // SSE1
    nullptr,            // SSE2
    &hsMatrix44::mult_sse3
};

hsPoint3 hsMatrix44::operator*(const hsPoint3& p) const
{
    if (fFlags & hsMatrix44::kIsIdent)
        return p;

    hsPoint3 rVal((p.fX * fMap[0][0]) + (p.fY * fMap[0][1]) + (p.fZ * fMap[0][2]) + fMap[0][3],
                  (p.fX * fMap[1][0]) + (p.fY * fMap[1][1]) + (p.fZ * fMap[1][2]) + fMap[1][3],
                  (p.fX * fMap[2][0]) + (p.fY * fMap[2][1]) + (p.fZ * fMap[2][2]) + fMap[2][3]);
    return rVal;
}

hsVector3 hsMatrix44::operator*(const hsVector3& p) const
{
    if( fFlags & hsMatrix44::kIsIdent )
        return p;

    hsVector3 rVal;

    rVal.fX = (p.fX * fMap[0][0]) + (p.fY * fMap[0][1]) + (p.fZ * fMap[0][2]);
    rVal.fY = (p.fX * fMap[1][0]) + (p.fY * fMap[1][1]) + (p.fZ * fMap[1][2]);
    rVal.fZ = (p.fX * fMap[2][0]) + (p.fY * fMap[2][1]) + (p.fZ * fMap[2][2]);

    return rVal;
}

bool hsMatrix44::Compare(const hsMatrix44& rhs, float tolerance) const
{
    return
        (fabs(fMap[0][0] - rhs.fMap[0][0]) < tolerance) &&
        (fabs(fMap[0][1] - rhs.fMap[0][1]) < tolerance) &&
        (fabs(fMap[0][2] - rhs.fMap[0][2]) < tolerance) &&
        (fabs(fMap[0][3] - rhs.fMap[0][3]) < tolerance) &&

        (fabs(fMap[1][0] - rhs.fMap[1][0]) < tolerance) &&
        (fabs(fMap[1][1] - rhs.fMap[1][1]) < tolerance) &&
        (fabs(fMap[1][2] - rhs.fMap[1][2]) < tolerance) &&
        (fabs(fMap[1][3] - rhs.fMap[1][3]) < tolerance) &&

        (fabs(fMap[2][0] - rhs.fMap[2][0]) < tolerance) &&
        (fabs(fMap[2][1] - rhs.fMap[2][1]) < tolerance) &&
        (fabs(fMap[2][2] - rhs.fMap[2][2]) < tolerance) &&
        (fabs(fMap[2][3] - rhs.fMap[2][3]) < tolerance) &&

        (fabs(fMap[3][0] - rhs.fMap[3][0]) < tolerance) &&
        (fabs(fMap[3][1] - rhs.fMap[3][1]) < tolerance) &&
        (fabs(fMap[3][2] - rhs.fMap[3][2]) < tolerance) &&
        (fabs(fMap[3][3] - rhs.fMap[3][3]) < tolerance);
}

bool hsMatrix44::operator==(const hsMatrix44& ss) const
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

hsMatrix44& hsMatrix44::Scale(const hsVector3* scale)
{
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
    for (int i =0; i < 3; i++)
    {
        fMap[i][3] += (*pt)[i];
    }
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
hsMatrix44&     hsMatrix44::MakeRotateMat(int axis, float radians)
{
    Reset();
    SetRotate(axis, radians);
    NotIdentity();
    return *this;
}

hsMatrix44&     hsMatrix44::Rotate(int axis, float radians)
{
    hsMatrix44 rMat;
    rMat.MakeRotateMat(axis, radians);
    *this = rMat * *this;
    return *this;
}

hsMatrix44&     hsMatrix44::SetRotate(int axis, float radians)
{
    float s = sin(radians);
    float c = cos(radians);
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
    default:
        hsAssert(false, "Invalid rotation axis specified");
    }
    fMap[c1][c1] = c;
    fMap[c2][c2] = c;
    fMap[c1][c2] = s;
    fMap[c2][c1] = -s;

    NotIdentity();

    return *this;
}

void hsMatrix44::MakeXRotation(float radians)
{
    Reset();
    float s = sin(radians);
    float c = cos(radians);

    fMap[1][1] = c;
    fMap[2][2] = c;
    fMap[1][2] = s;
    fMap[2][1] = -s;
    NotIdentity();
}

void hsMatrix44::MakeYRotation(float radians)
{
    Reset();
    float s = sin(radians);
    float c = cos(radians);
    fMap[0][0] = c;
    fMap[2][2] = c;
    fMap[0][2] = -s;
    fMap[2][0] = s;
    NotIdentity();
}

void hsMatrix44::MakeZRotation(float radians)
{
    Reset();
    float s = sin(radians);
    float c = cos(radians);
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
    hsVector3 trans(f->fX, f->fY, f->fZ);
    MakeTranslateMat(&trans);

    hsVector3 back (f,at);  // Z
    back.Normalize();

    hsVector3 leftEar = *up % back; // X, LHS
    leftEar.Normalize();

#if 1
    // Ignore actual up vector
    hsVector3 topHead = leftEar % back; // Y, RHS (should be flipped)
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
    hsVector3 trans(f->fX, f->fY, f->fZ);
    MakeTranslateMat(&trans);

    hsVector3 topHead = *up;
    topHead.Normalize();

    hsVector3 back (f,at);  // Z
    back = -back;   // really front

    hsVector3 leftEar = *up % back; // X
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
void    hsMatrix44::GetAxis(hsVector3* view, hsVector3 *up, hsVector3* right)
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
    hsMatrix44 rmat;

    hsVector3 dirX = (*up) % dirZ; // Stop passing in down!!! // mf_flip_up - mf
    if (dirX.MagnitudeSquared())
        dirX.Normalize();

    if (dirZ.MagnitudeSquared())
        dirZ.Normalize();
    hsVector3 dirY = dirZ % dirX;
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
    MakeCameraMatrices(pos, hsPoint3(pos.fX - 1.f, pos.fY, pos.fZ), hsVector3(0.f, 0.f, 1.f), worldToCameras[0], cameraToWorlds[0]);

    MakeCameraMatrices(pos, hsPoint3(pos.fX + 1.f, pos.fY, pos.fZ), hsVector3(0.f, 0.f, 1.f), worldToCameras[1], cameraToWorlds[1]);

    MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY + 1.f, pos.fZ), hsVector3(0.f, 0.f, 1.f), worldToCameras[2], cameraToWorlds[2]);

    MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY - 1.f, pos.fZ), hsVector3(0.f, 0.f, 1.f), worldToCameras[3], cameraToWorlds[3]);

    MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY, pos.fZ + 1.f), hsVector3(0.f, -1.f, 0.f), worldToCameras[4], cameraToWorlds[4]);

    MakeCameraMatrices(pos, hsPoint3(pos.fX, pos.fY, pos.fZ - 1.f), hsVector3(0.f, 1.f, 0.f), worldToCameras[5], cameraToWorlds[5]);
}

//
// Vectors are normalized.
//
void    hsMatrix44::GetAxisFromCamera(hsVector3* view, hsVector3 *up, hsVector3* right)
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
    hsMatrix44 rmat;

    hsVector3 dirX =   dirY % dirZ;
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

hsMatrix44* hsMatrix44::GetTranspose(hsMatrix44* transp) const
{
    for(int i = 0 ; i < 4; i++)
        for(int j=0; j < 4; j++)
            transp->fMap[i][j] = fMap[j][i];
    return transp;
}


static inline float Determinant2(float a, float b,float c, float d)
{
    return (a * d) - (c * b);
}

static inline float Determinant3(float a, float b, float c,
                       float d, float e, float f,
                       float g, float h, float i)
{
    return (a * Determinant2(e, f, h, i))
        -  (b * Determinant2(d, f, g, i))
        +  (c * Determinant2(d, e, g, h));
}

float hsMatrix44::GetDeterminant() const
{
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
}


hsMatrix44 *hsMatrix44::GetAdjoint(hsMatrix44 *adj) const
{
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

    adj->NotIdentity();
    return adj;
}

hsMatrix44* hsMatrix44::GetInverse(hsMatrix44* inverse) const
{
    float det = GetDeterminant();
    int i,j;

    if (det == 0.0f)
    {
        inverse->Reset();
        return inverse;
    }

    det = hsInvert(det);
    GetAdjoint(inverse);

    for (i=0; i<4; i++)
        for (j=0; j<4; j++)
            inverse->fMap[i][j] *= det;

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

bool hsMatrix44::IsIdentity()
{
    bool retVal = true;
    int i, j;
    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
        {
#if 0 // IDENTITY_CRISIS
            if( i == j)
            {
                if (fMap[i][j] != 1.f) 
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
            const float kEPS = 1.e-5f;
            if( i == j)
            {
                if( (fMap[i][j] < 1.f-kEPS) || (fMap[i][j] > 1.f+kEPS) ) 
                {
                    NotIdentity();
                    retVal = false;
                }
                else
                {
                    fMap[i][j] = 1.f;
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

bool hsMatrix44::GetParity() const
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
                fMap[i][j] = stream->ReadLEFloat();
        IsIdentity();
    }
    else
        Reset();
}

void hsMatrix44::Write(hsStream *stream)
{
    bool ident = IsIdentity();
    stream->WriteBool(!ident);
    if (!ident)
    {
        int i,j;
        for(i=0; i<4; i++)
            for(j=0; j<4; j++)
                stream->WriteLEFloat(fMap[i][j]);
    }
}


ST_FORMAT_TYPE(const hsMatrix44&)
{
    ST_FORMAT_FORWARD(ST::format("hsMatrix44[[{.4f}, {.4f}, {.4f}, {.4f}]; [{.4f}, {.4f}, {.4f}, {.4f}]; [{.4f}, {.4f}, {.4f}, {.4f}]; [{.4f}, {.4f}, {.4f}, {.4f}]]",
                    value.fMap[0][0], value.fMap[0][1], value.fMap[0][2], value.fMap[0][3],
                    value.fMap[1][0], value.fMap[1][1], value.fMap[1][2], value.fMap[1][3],
                    value.fMap[2][0], value.fMap[2][1], value.fMap[2][2], value.fMap[2][3],
                    value.fMap[3][0], value.fMap[3][1], value.fMap[3][2], value.fMap[3][3]));
}
