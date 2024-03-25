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
#ifndef HSMATRIX44_inc
#define  HSMATRIX44_inc

#include "HeadSpin.h"
#include "hsCpuID.h"
#include "hsGeometry3.h"

#include <string_theory/formatter>

class hsQuat;

////////////////////////////////////////////////////////////////////////////
class hsStream;
struct hsMatrix44 {
    enum {
        kIsIdent    = 0x1
    };

    enum {
        kRight  = 0,
        kUp,
        kView
    };
    float            fMap[4][4];
    union
    {
        uint8_t      alignment[16];
        uint32_t     fFlags;
    };

    hsMatrix44() : fFlags(0) {}
    hsMatrix44(const hsScalarTriple &translate, const hsQuat &rotate);

    void DecompRigid(hsScalarTriple &translate, hsQuat &rotate) const;

    [[nodiscard]]
    static const hsMatrix44& IdentityMatrix();

    // worldToCameras and cameraToWorlds are arrays of 6 matrices. Returned are LEFT,RIGHT,FRONT,BACK,TOP,BOTTOM.
    static void MakeEnvMapMatrices(const hsPoint3& pos, hsMatrix44* worldToCameras, hsMatrix44* cameraToWorlds);
    static void MakeCameraMatrices(const hsPoint3& from, const hsPoint3& at, const hsVector3& up, hsMatrix44& worldToCamera, hsMatrix44& cameraToWorld);

    // Concat transform
    hsMatrix44&     Translate(const hsVector3 *);
    hsMatrix44&     Scale(const hsVector3 *);
    hsMatrix44&     Rotate(int axis, float radians);

    hsMatrix44&     Reset(bool asIdent=true) 
    {
        fMap[0][0] = 1.0f; fMap[0][1] = 0.0f; fMap[0][2] = 0.0f; fMap[0][3]  = 0.0f;
        fMap[1][0] = 0.0f; fMap[1][1] = 1.0f; fMap[1][2] = 0.0f; fMap[1][3]  = 0.0f;
        fMap[2][0] = 0.0f; fMap[2][1] = 0.0f; fMap[2][2] = 1.0f; fMap[2][3]  = 0.0f;
        fMap[3][0] = 0.0f; fMap[3][1] = 0.0f; fMap[3][2] = 0.0f; fMap[3][3]  = 1.0f;

        fFlags = asIdent ? kIsIdent : 0;
        return *this;
    }

    // Create matrix from scratch
    hsMatrix44&     MakeTranslateMat(const hsVector3 *trans);
    hsMatrix44&     MakeScaleMat(const hsVector3 *scale);
    hsMatrix44&     MakeRotateMat(int axis, float radians);
    hsMatrix44&     Make(const hsPoint3* from, const hsPoint3* at, 
                        const hsVector3* up);   // Not a camera matrix
    hsMatrix44&     MakeUpPreserving(const hsPoint3* from, const hsPoint3* at, 
                        const hsVector3* up);   // Not a camera matrix
    // Camera matrix
    hsMatrix44&     MakeCamera(const hsPoint3* from, const hsPoint3* at,
                        const hsVector3* up);
    hsMatrix44&     MakeCameraUpPreserving(const hsPoint3* from, const hsPoint3* at,
                        const hsVector3* up);

    bool            GetParity() const;
    float           GetDeterminant() const;
    hsMatrix44*     GetInverse(hsMatrix44* inverse) const;
    hsMatrix44*     GetTranspose(hsMatrix44* inverse) const;
    hsMatrix44*     GetAdjoint(hsMatrix44* adjoint) const;
    hsVector3*      GetTranslate(hsVector3 *pt) const;
    hsPoint3*       GetTranslate(hsPoint3 *pt) const 
        {   return (hsPoint3*)GetTranslate((hsVector3*)pt); }
    [[nodiscard]]
    const hsPoint3  GetTranslate() const { return hsPoint3(fMap[0][3], fMap[1][3], fMap[2][3]); }
    void            GetAxis(hsVector3* view, hsVector3 *up, hsVector3* right);
    void            GetAxisFromCamera(hsVector3* view, hsVector3 *up, hsVector3* right);

    const hsVector3 GetAxis(int i) const;

    // Change component of matrix
    hsMatrix44&     SetTranslate(const hsScalarTriple *);
    hsMatrix44&     SetScale(const hsVector3 *);
    hsMatrix44&     SetRotate(int axis, float radians);

    hsVector3       RemoveScale();      // returns old scale
    void MakeXRotation(float radians);
    void MakeYRotation(float radians);
    void MakeZRotation(float radians);


    [[nodiscard]]
    hsPoint3 operator*(const hsPoint3& p) const;
    [[nodiscard]]
    hsVector3 operator*(const hsVector3& p) const;
    [[nodiscard]]
    hsMatrix44 operator *(const hsMatrix44& other) const {
        
#ifdef HS_BUILD_FOR_APPLE
        return mult_accelerate(*this, other);
#else
        return mat_mult.call(*this, other);
#endif
        
    }

    hsPoint3*           MapPoints(long count, hsPoint3 points[]) const;

    bool  IsIdentity();
    void  NotIdentity() { fFlags &= ~kIsIdent; }

    bool Compare(const hsMatrix44& rhs, float tolerance) const;
    bool operator==(const hsMatrix44& ss) const;
    bool operator!=(const hsMatrix44& ss) const { return !(ss == *this); }

    void Read(hsStream *stream);
    void Write(hsStream *stream);

private:
    //  CPU-optimized functions
    typedef hsMatrix44(*mat_mult_ptr)(const hsMatrix44&, const hsMatrix44&);
    static hsCpuFunctionDispatcher<mat_mult_ptr> mat_mult;

    static hsMatrix44 mult_fpu(const hsMatrix44& a, const hsMatrix44& b);
    static hsMatrix44 mult_sse3(const hsMatrix44& a, const hsMatrix44& b);
#ifdef HS_BUILD_FOR_APPLE
    static hsMatrix44 mult_accelerate(const hsMatrix44 &a, const hsMatrix44 &b);
#endif
};

ST_DECL_FORMAT_TYPE(const hsMatrix44&);

////////////////////////////////////////////////////////////////////////////
#endif
