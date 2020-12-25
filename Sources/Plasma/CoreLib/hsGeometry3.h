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
#ifndef hsGGeometry3Defined
#define hsGGeometry3Defined

#include "HeadSpin.h"

struct hsVector3;
struct hsPoint3;
struct hsScalarTriple;
class hsStream;

/*
    If value is already close to 1.f, then this is a good approx. of 1/sqrt(value)
*/
static inline float hsInvSqrt(float value)
{
    float    guess;
    float    threeOverTwo = 1.5f;

    value /= 2.f;
    guess = threeOverTwo - value;       // with initial guess = 1.0

    // repeat this line for better approx
    guess = (guess * threeOverTwo - ((value * guess) * guess));
    guess = (guess * threeOverTwo - ((value * guess) * guess));

    return guess;
}

/////////////////////////////////////////////////////////////////////////////////////////////
struct hsScalarTriple
{
//protected:
//  hsScalarTriple() : fX(privateData[0]), fY(privateData[1]), fZ(privateData[2]) {}
//  hsScalarTriple(float x, float y, float z) 
//      : fX(privateData[0]), fY(privateData[1]), fZ(privateData[2]) { fX = x, fY = y, fZ = z; }
//  
//  union {
//      u_long128   privateTemp;
//      float    privateData[4];
//  };
//public:
//
//  int operator=(const hsScalarTriple& o) { privateTemp = o.privateTemp; }
//  hsScalarTriple(const hsScalarTriple& o) : fX(privateData[0]), fY(privateData[1]), fZ(privateData[2]) 
//              { *this = o; }
//
//  float& fX;
//  float& fY;
//  float& fZ;
protected:
    hsScalarTriple() : fX(), fY(), fZ() { }
    hsScalarTriple(float x, float y, float z) : fX(x), fY(y), fZ(z) {}
public:
    float    fX, fY, fZ;

    hsScalarTriple*     Set(float x, float y, float z) { fX= x; fY = y; fZ = z;    return this;}
    hsScalarTriple*     Set(const hsScalarTriple *p) { fX = p->fX; fY = p->fY; fZ = p->fZ;  return this;}
    
    float InnerProduct(const hsScalarTriple &p) const;
    float InnerProduct(const hsScalarTriple *p) const;

//  hsScalarTriple LERP(hsScalarTriple &other, float t);
     float       Magnitude() const;
     float       MagnitudeSquared() const { return (fX * fX + fY * fY + fZ * fZ); }

     bool IsEmpty() const { return fX == 0 && fY == 0 && fZ == 0; }

    float    operator[](int i) const;
    float&   operator[](int i);
    
    void Read(hsStream *stream);
    void Write(hsStream *stream) const;

};


///////////////////////////////////////////////////////////////////////////
inline float& hsScalarTriple::operator[] (int i) 
{
    hsAssert(i >=0 && i <3, "Bad index for hsScalarTriple::operator[]");
     return *(&fX + i); 
}
inline float hsScalarTriple::operator[] (int i) const
{
    hsAssert(i >=0 && i <3, "Bad index for hsScalarTriple::operator[]");
     return *(&fX + i); 
}
inline float hsScalarTriple::InnerProduct(const hsScalarTriple &p) const
{
    float tmp = fX*p.fX;
    tmp += fY*p.fY;
    tmp += fZ*p.fZ;
    return tmp;
}
inline float hsScalarTriple::InnerProduct(const hsScalarTriple *p) const
{
    float tmp = fX*p->fX;
    tmp += fY*p->fY;
    tmp += fZ*p->fZ;
    return tmp;
}

//inline hsScalarTriple hsScalarTriple::LERP(hsScalarTriple &other, float t)
//{
//  hsScalarTriple p = other - this;
//  p = p / t;
//  return this + p;
//}



/////////////////////////////////////////////////////////////////////////////////////////////
struct hsPoint3  : public hsScalarTriple {  
    hsPoint3() {};
    hsPoint3(float x, float y, float z) : hsScalarTriple(x,y,z) {}
    explicit hsPoint3(const hsScalarTriple& p) : hsScalarTriple(p) {}
    
    hsPoint3*    Set(float x, float y, float z) { return (hsPoint3*)this->hsScalarTriple::Set(x,y,z);}
    hsPoint3*    Set(const hsScalarTriple* p) { return (hsPoint3*)this->hsScalarTriple::Set(p) ;}

    friend inline hsPoint3 operator+(const hsPoint3& s, const hsPoint3& t);
    friend inline hsPoint3 operator+(const hsPoint3& s, const hsVector3& t);
    friend inline hsPoint3 operator-(const hsPoint3& s, const hsPoint3& t);
    friend inline hsPoint3 operator-(const hsPoint3& s);
    friend inline hsPoint3 operator*(const float& s, const hsPoint3& t);
    friend inline hsPoint3 operator*(const hsPoint3& t, const float& s);
    friend inline hsPoint3 operator/(const hsPoint3& t, const float& s);
    bool operator==(const hsPoint3& ss) const
    {
            return (ss.fX == fX && ss.fY == fY && ss.fZ == fZ);
    }
    bool operator!=(const hsPoint3& ss) const { return !(*this == ss); }
    hsPoint3 &operator+=(const hsScalarTriple &s) { fX += s.fX; fY += s.fY; fZ += s.fZ; return *this; }
    hsPoint3 &operator*=(const float s) { fX *= s; fY *= s; fZ *= s; return *this; }
};


/////////////////////////////////////////////////////////////////////////////////////////////

struct hsVector3 : public hsScalarTriple {  

    hsVector3() {};
    hsVector3(float x, float y, float z) : hsScalarTriple(x,y,z) {}
    explicit hsVector3(const hsScalarTriple& p) : hsScalarTriple(p) { }
    hsVector3(const hsPoint3 *p1, const hsPoint3 *p2)
        : hsScalarTriple(p1->fX - p2->fX, p1->fY - p2->fY, p1->fZ - p2->fZ)
    { }

    hsVector3*   Set(float x, float y, float z) { return (hsVector3*)hsScalarTriple::Set(x,y,z); }
    hsVector3*   Set(const hsScalarTriple* p) { return (hsVector3*)hsScalarTriple::Set(p) ;}
    hsVector3*   Set(const hsScalarTriple* p1, const hsScalarTriple* p2) { return Set(p1->fX-p2->fX,p1->fY-p2->fY,p1->fZ-p2->fZ);}

    void            Normalize()
                {
                    float    length = this->Magnitude();
//                   hsIfDebugMessage(length == 0, "Err: Normalizing hsVector3 of length 0", 0);
                    if (length == 0)
                        return;
                    float    invMag = hsInvert(length);
                    
                    fX = (fX * invMag);
                    fY = (fY * invMag);
                    fZ = (fZ * invMag);
                }
    inline void     Renormalize()       // if the vector is already close to unit length
                {
                    float    mag2 = *this * *this;
                    hsIfDebugMessage(mag2 == 0, "Err: Renormalizing hsVector3 of length 0", 0);
                    if (mag2 == 0)
                        return;
                    float    invMag = hsInvSqrt(mag2);
                    
                    fX = (fX * invMag);
                    fY = (fY * invMag);
                    fZ = (fZ * invMag);
                }

//  hsVector3 &Sub(const hsPoint3& s, const hsPoint3& t)
//  {   Set(s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ); 
//          return *this; };
    friend inline hsVector3 operator+(const hsVector3& s, const hsVector3& t);
    friend inline hsVector3 operator-(const hsVector3& s, const hsVector3& t);
    friend inline hsVector3 operator-(const hsVector3& s);
    friend inline hsVector3 operator*(const float& s, const hsVector3& t);
    friend inline hsVector3 operator*(const hsVector3& t, const float& s);
    friend inline hsVector3 operator/(const hsVector3& t, const float& s);
    friend inline float operator*(const hsVector3& t, const hsVector3& s);
    friend  hsVector3 operator%(const hsVector3& t, const hsVector3& s);
    bool operator==(const hsVector3& ss) const
    {
            return (ss.fX == fX && ss.fY == fY && ss.fZ == fZ);
    }

    hsVector3 &operator+=(const hsScalarTriple &s) { fX += s.fX; fY += s.fY; fZ += s.fZ; return *this; }
    hsVector3 &operator-=(const hsScalarTriple &s) { fX -= s.fX; fY -= s.fY; fZ -= s.fZ; return *this; }
    hsVector3 &operator*=(const float s) { fX *= s; fY *= s; fZ *= s; return *this; }
    hsVector3 &operator/=(const float s) { fX /= s; fY /= s; fZ /= s; return *this; }
};

struct hsPoint4 {   
    float    fX, fY, fZ, fW;
    hsPoint4() : fX(), fY(), fZ(), fW() { }
    hsPoint4(float x, float y, float z, float w) :  fX(x), fY(y), fZ(z), fW(w) {}
    float&       operator[](int i); 
    float        operator[](int i) const;

    hsPoint4& operator=(const hsPoint3&p) { Set(p.fX, p.fY, p.fZ, 1.f); return *this; }

    hsPoint4*   Set(float x, float y, float z, float w) 
        { fX = x; fY = y; fZ = z; fW = w; return this; }
};


inline hsVector3 operator+(const hsVector3& s, const hsVector3& t)
{
    return hsVector3(s.fX + t.fX, s.fY + t.fY, s.fZ + t.fZ);
}

inline hsVector3 operator-(const hsVector3& s, const hsVector3& t)
{
    return hsVector3 (s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ);
}

// unary minus
inline hsVector3 operator-(const hsVector3& s)
{
    return hsVector3(-s.fX, -s.fY, -s.fZ);
}

inline hsVector3 operator*(const hsVector3& s, const float& t)
{
    return hsVector3 (s.fX * t, s.fY * t, s.fZ * t);
}

inline hsVector3 operator/(const hsVector3& s, const float& t)
{
    return hsVector3(s.fX / t, s.fY / t, s.fZ / t);
}

inline hsVector3 operator*(const float& t, const hsVector3& s)
{
    return hsVector3(s.fX * t, s.fY * t, s.fZ * t);
}

inline float operator*(const hsVector3& t, const hsVector3& s)
{
    return (t.fX * s.fX) + (t.fY * s.fY) + (t.fZ * s.fZ);
}

////////////////////////////////////////////////////////////////////////////

inline hsPoint3 operator+(const hsPoint3& s, const hsPoint3& t)
{
    return hsPoint3(s.fX + t.fX, s.fY + t.fY, s.fZ + t.fZ);
}

inline hsPoint3 operator+(const hsPoint3& s, const hsVector3& t)
{
    return hsPoint3 (s.fX + t.fX, s.fY + t.fY, s.fZ + t.fZ);
}

inline hsPoint3 operator-(const hsPoint3& s, const hsPoint3& t)
{
    return hsPoint3(s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ);
}

// unary -
inline hsPoint3 operator-(const hsPoint3& s)
{
    return hsPoint3(-s.fX, -s.fY, -s.fZ);
}

inline hsPoint3 operator-(const hsPoint3& s, const hsVector3& t)
{
    return hsPoint3 (s.fX - t.fX, s.fY - t.fY, s.fZ - t.fZ);
}

inline hsPoint3 operator*(const hsPoint3& s, const float& t)
{
    return hsPoint3 ((s.fX * t), (s.fY * t), (s.fZ * t));
}

inline hsPoint3 operator/(const hsPoint3& s, const float& t)
{
    return hsPoint3 ((s.fX / t), (s.fY / t), (s.fZ / t));
}

inline hsPoint3 operator*(const float& t, const hsPoint3& s)
{
    return hsPoint3 ((s.fX * t), (s.fY * t), (s.fZ * t));
}

inline float hsPoint4::operator[] (int i) const
{
    hsAssert(i >=0 && i <4, "Bad index for hsPoint4::operator[]");
     return *(&fX + i); 
}

inline float& hsPoint4::operator[] (int i)
{
    hsAssert(i >=0 && i <4, "Bad index for hsPoint4::operator[]");
     return *(&fX + i); 
}

typedef hsPoint3 hsGUv;



struct hsPointNorm {
    hsPoint3    fPos;
    hsVector3   fNorm;

    void Read(hsStream* s) { fPos.Read(s); fNorm.Read(s); }
    void Write(hsStream* s) const { fPos.Write(s); fNorm.Write(s); }
};


struct hsPlane3 {
    hsVector3 fN;
    float fD;

    hsPlane3() : fD() { }
    hsPlane3(const hsVector3* nrml, float d) 
    { fN = *nrml; fD=d; }
    hsPlane3(const hsPoint3* pt, const hsVector3* nrml)
    { fN = *nrml; fD = -pt->InnerProduct(nrml); }
    
    // create plane from a triangle (assumes clockwise winding of vertices)
    hsPlane3(const hsPoint3* pt1, const hsPoint3* pt2, const hsPoint3* pt3);

    hsVector3 GetNormal() const { return fN; }

    void Read(hsStream *stream);
    void Write(hsStream *stream) const;
};

#endif
