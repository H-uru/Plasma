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
#ifndef HSQUAT_inc
#define HSQUAT_inc

struct hsMatrix44;
struct hsScalarTriple;
class hsStream;
struct hsPoint3;
struct hsVector3;

//
// Quaternion class.
// For conversion to and from euler angles, see hsEuler.cpp,h.
// 
class hsQuat {

public:
    float fX,fY,fZ,fW;   

    // Constructors
    hsQuat() : fX(), fY(), fZ(), fW(1.f) { }
    hsQuat(float X, float Y, float Z, float W) : 
        fX(X), fY(Y), fZ(Z), fW(W) {}
    hsQuat(const hsQuat& a) = default;
    hsQuat(float af[4]) { fX = af[0]; fY = af[1]; fZ = af[2]; fW = af[3]; }
    hsQuat(float rad, const hsVector3* axis);

    hsQuat& operator=(const hsQuat& a) = default;

    [[nodiscard]]
    static hsQuat QuatFromMatrix44(const hsMatrix44& mat);
    hsQuat& SetFromMatrix44(const hsMatrix44& mat);
    void SetFromMatrix(const hsMatrix44 *mat);
    void SetFromSlerp(const hsQuat &q1, const hsQuat &q2, float t, int spin=0);
    void Set(float X, float Y, float Z, float W) { fX = X; fY = Y; fZ = Z; fW = W; }
    void GetAngleAxis(float *rad, hsVector3 *axis) const;
    void SetAngleAxis(const float rad, const hsVector3 &axis);

    [[nodiscard]]
    hsPoint3 Rotate(const hsScalarTriple* v) const;

    // Access operators
    float& operator[](int i) { return (&fX)[i]; }     
    const float& operator[](int i) const { return (&fX)[i]; }  

    // Unary operators
    hsQuat operator-() const { return(hsQuat(-fX,-fY,-fZ,-fW)); } 
    hsQuat operator+() const { return *this; } 

    // Comparison
    int operator==(const hsQuat& a) const
        { return (fX==a.fX && fY==a.fY && fZ==a.fZ && fW==a.fW); }
    void Identity() { fX = fY = fZ = (float)0.0; fW = (float) 1.0; }
    int IsIdentity() const
        { return (fX==0.0 && fY==0.0 && fZ==0.0 && fW==1.0); }
    void Normalize();  
    void NormalizeIfNeeded();  
    void MakeMatrix(hsMatrix44 *mat) const;

    [[nodiscard]]
    float Magnitude();

    [[nodiscard]]
    float MagnitudeSquared();

    [[nodiscard]]
    hsQuat Conjugate() const { return hsQuat(-fX,-fY,-fZ,fW); }

    [[nodiscard]]
    hsQuat Inverse() const;

    // Binary operators
    hsQuat operator-(const hsQuat&) const;
    hsQuat operator+(const hsQuat&) const;
    hsQuat operator*(const hsQuat&) const;
    hsQuat operator*(float f) const { return hsQuat(fX*f,fY*f,fZ*f,fW*f); }
    hsQuat operator/(float f) const { return hsQuat(fX/f,fY/f,fZ/f,fW/f); }
    hsQuat operator/(const hsQuat&) const;

    [[nodiscard]]
    float Dot(const hsQuat &q2) const { return (fX*q2.fX + fY*q2.fY + fZ*q2.fZ + fW*q2.fW); }

    // I/O
    void Read(hsStream *stream);
    void Write(hsStream *stream);
};

#endif
