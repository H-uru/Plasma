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

#ifndef hsBounds_inc
#define hsBounds_inc

#include "hsGeometry3.h"
#include "hsPoint2.h"
#include "hsMatrix44.h"

///////////////////////////////////////////////////////////////////////////////
// BOUNDS
///////////////////////////////////////////////////////////////////////////////

enum  hsBoundsType
{
    kBoundsNormal,
    kBoundsFull,
    kBoundsEmpty,
    kBoundsUninitialized
};

//
// Abstract base class
//
class hsBounds3;
class hsBounds
{
protected:
    hsBoundsType fType;
public:
    static const float kRealSmall;

    hsBounds() : fType(kBoundsUninitialized) { };

    hsBounds& MakeEmpty() { fType = kBoundsEmpty; return *this; }
    hsBounds& MakeFull()    { fType = kBoundsFull; return *this; }
    hsBoundsType GetType() const { return fType; }

    //
    // These set type to kBounds Normal
    // 
    virtual void    Reset(const hsBounds3*) = 0;

    virtual bool    IsInside(const hsPoint3* pos) const =0;  // Only valid for kBounds Normal

    virtual void Read(hsStream*);
    virtual void Write(hsStream*);
};

//
//
//
class hsBounds3 : public hsBounds
{
public:
    enum {
        kCenterValid        = 0x1,
        kIsSphere           = 0x2
    };
protected:
    mutable uint32_t      fBounds3Flags;
    hsPoint3            fMins;
    hsPoint3            fMaxs;
    mutable hsPoint3    fCenter;

    void ICalcCenter() const;
public:
    hsBounds3() : fBounds3Flags(0) {}
    hsBounds3(const hsBounds3 &pRHS) : fBounds3Flags(0) { Reset(&pRHS); }
    hsBounds3 &operator=(const hsBounds3 &pRHS ) 
    {   if (&pRHS != this)  Reset(&pRHS);   return *this; }

    //
    // These set type to kBounds Normal
    // 
            void    Reset(const hsBounds3*) override;
    virtual void    Reset(const hsPoint3 *p);
    virtual void    Reset(int n, const hsPoint3 *p);
    virtual void    Union(const hsPoint3 *p);
    virtual void    Union(const hsBounds3 *b);
    virtual void    Union(const hsVector3 *v); // smears the bounds in given direction
    virtual void    MakeSymmetric(const hsPoint3* p); // Expands bounds to be symmetric about p
    virtual void    InscribeSphere();
    
    virtual void    Transform(const hsMatrix44*);
    
    //
    // Only valid for kBounds Normal
    //
    virtual void GetCorners(hsPoint3 *b) const;
    const hsPoint3& GetMins() const;
    const hsPoint3& GetMaxs() const;
    float GetMaxDim() const;     // Computes the answer
    const hsPoint3&  GetCenter() const; // Computes the answer if not already there
    bool IsInside(const hsPoint3* pos) const override; // ok for full/empty
    virtual void TestPlane(const hsVector3 &n, hsPoint2 &depth) const;
    virtual void TestPlane(const hsPlane3 *p, hsPoint2 &depth) const;
    virtual bool ClosestPoint(const hsPoint3& p, hsPoint3& inner, hsPoint3& outer) const;

    // Test according to my axes only, doesn't check other's axes
    // neg, pos, zero == disjoint, I contain other, overlap
    virtual int32_t TestBound(const hsBounds3& other) const; 

    static float ClosestPointToLine(const hsPoint3 *p, const hsPoint3 *v0, const hsPoint3 *v1, hsPoint3 *out);
    static float ClosestPointToInfiniteLine(const hsPoint3* p, const hsVector3* v, hsPoint3* out);

    void Read(hsStream*) override;
    void Write(hsStream*) override;
};

inline void hsBounds3::ICalcCenter() const
{ 
    hsAssert(kBoundsNormal == fType, "Invalid type for ICalcCenter");
    fCenter = ((fMins + fMaxs) / 2.0); 
    fBounds3Flags |= kCenterValid;
}
inline void hsBounds3::GetCorners(hsPoint3 *b) const
{
    hsAssert(kBoundsNormal == fType, "Invalid type for GetCorners");
    for(int i = 0; i < 8; i++)
    {
        b[i][0] = (i & 0x1) ? fMins[0] : fMaxs[0];
        b[i][1] = (i & 0x2) ? fMins[1] : fMaxs[1];
        b[i][2] = (i & 0x4) ? fMins[2] : fMaxs[2];
    }
}
inline const hsPoint3& hsBounds3::GetMins() const
{   
    hsAssert(kBoundsNormal == fType, "Invalid type for GetMins");
    return fMins;
}
inline const hsPoint3& hsBounds3::GetMaxs() const
{   
    hsAssert(kBoundsNormal == fType, "Invalid type for GetMaxs");
    return fMaxs;
}

inline const hsPoint3& hsBounds3::GetCenter() const
{ 
    hsAssert(kBoundsNormal == fType, "Invalid type for GetCenter");
    if(!(fBounds3Flags & kCenterValid))
        ICalcCenter();
    return fCenter; 
}

//
// A convex region specified by a series of planes.
//
class hsBoundsOriented : public hsBounds
{
private:
    bool        fCenterValid;
    hsPoint3    fCenter;
    hsPlane3    *fPlanes;
    uint32_t    fNumPlanes;
public:
    hsBoundsOriented() : fCenterValid(), fPlanes(), fNumPlanes() {}
    virtual ~hsBoundsOriented() {   if (fPlanes) delete [] fPlanes; }

    // Center is not computed by the class, it must be set by the creator of the class.
    void    SetCenter(const hsPoint3* c)    { fCenter=*c; fCenterValid = true; }
    void    SetCenter(const hsBounds3* b)   { hsBounds3 bb=*b; fCenter=bb.GetCenter(); fCenterValid = true; }
    void    SetCenter(const hsBoundsOriented* b)    { fCenter=b->GetCenter(); fCenterValid = true; }
    hsPoint3  GetCenter() const;

    void    SetNumberPlanes(uint32_t n);

    hsPlane3* GetPlane(int i)   { return &fPlanes[i]; }
    int     GetNumPlanes()  { return fNumPlanes; }

    //
    // These set type to kBounds Normal
    // 
    void    Reset(const hsBounds3*) override;
    void    SetPlane(uint32_t i, hsPlane3 *p);

    //
    // Only valid for kBounds Normal
    //
    bool IsInside(const hsPoint3* pos) const override;
    virtual void TestPlane(const hsVector3 &n, hsPoint2 &depth) const; // Complain and refuse

    void Write(hsStream *stream) override;
    void Read(hsStream *stream) override;
};

class hsHitInfoExt;
class hsBounds3Ext : public hsBounds3 {
protected:
    enum {
        kAxisAligned            =0x1,
        kSphereSet              =0x2,
        kDistsSet               =0x4,
        kAxisZeroZero           =(1<<20),
        kAxisOneZero            =(1<<21),
        kAxisTwoZero            =(1<<22)
    };
    mutable uint32_t          fExtFlags;
    hsPoint3        fCorner;
    hsVector3       fAxes[3];
    mutable hsPoint2        fDists[3];
    mutable float        fRadius;

    bool IAxisIsZero(uint32_t i) const { return (fExtFlags & (1 << (20+i))) != 0; };
    void IMakeSphere() const;
    void IMakeDists() const;
    void IMakeMinsMaxs();
public:
    hsBounds3Ext() : fExtFlags(kAxisAligned), fDists(), fRadius() {};

    hsBounds3Ext(const hsBounds3 &b);
    hsBounds3Ext &operator=(const hsBounds3 &b);
    hsBounds3Ext(const hsBounds3Ext &pRHS) : hsBounds3() { Reset(&pRHS); }
    hsBounds3Ext &operator=(const hsBounds3Ext &pRHS ) 
    {   if (&pRHS != this)  Reset(&pRHS);   return *this; }

    virtual void Reset(const hsBounds3Ext *b);
    void Reset(const hsBounds3 *b) override;
    void Reset(const hsPoint3 *p) override;
    void Reset(int n, const hsPoint3 *p) override;

    void Union(const hsPoint3 *p) override;
    void Union(const hsBounds3 *b) override;

    void Union(const hsVector3 *v) override; // smears the bounds in given direction
    void MakeSymmetric(const hsPoint3* p) override; // Expands bounds to be symmetric about p
    void InscribeSphere() override;
    virtual void Unalign();

    void Transform(const hsMatrix44 *m) override;
    virtual void Translate(const hsVector3 &v);

    virtual float GetRadius() const;
    virtual void GetAxes(hsVector3 *fAxis0, hsVector3 *fAxis1, hsVector3 *fAxis2) const;
    virtual hsPoint3 *GetCorner(hsPoint3 *c) const { *c = (fExtFlags & kAxisAligned ? fMins : fCorner); return c; }
    void GetCorners(hsPoint3 *b) const override;
    bool ClosestPoint(const hsPoint3& p, hsPoint3& inner, hsPoint3& outer) const override;

    bool IsInside(const hsPoint3* pos) const override; // ok for full/empty

    void TestPlane(const hsVector3 &n, hsPoint2 &depth) const override;
    virtual int32_t TestPoints(int n, const hsPoint3 *pList) const; // pos,neg,zero == allout, allin, cut

    // Test according to my axes only, doesn't check other's axes
    // neg, pos, zero == disjoint, I contain other, overlap
    virtual int32_t TestBound(const hsBounds3Ext& other) const; 

    virtual void TestPlane(const hsVector3 &n, const hsVector3 &myVel, hsPoint2 &depth) const; 
    virtual void TestPlane(const hsPlane3 *p, const hsVector3 &myVel, hsPoint2 &depth) const; 
    virtual int32_t TestPoints(int n, const hsPoint3 *pList, const hsVector3 &ptVel) const; // pos,neg,zero == allout, allin, cut
    virtual bool ISectBB(const hsBounds3Ext &other, const hsVector3 &myVel) const;
    virtual bool ISectBB(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;
    virtual bool ISectABB(const hsBounds3Ext &other, const hsVector3 &myVel) const;
    virtual bool ISectBS(const hsBounds3Ext &other, const hsVector3 &myVel) const;

    virtual int32_t IClosestISect(const hsBounds3Ext& other, const hsVector3& myVel,
                                  float* tClose, float* tImpact) const;
    virtual bool ISectBoxBS(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;
    virtual bool ISectBSBox(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;
    virtual bool ISectBoxBS(const hsBounds3Ext &other, const hsVector3 &myVel) const;
    virtual bool ISectBSBS(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;

    virtual bool ISectLine(const hsPoint3* from, const hsPoint3* to) const;
    virtual bool ISectCone(const hsPoint3* from, const hsPoint3* to, float radius) const;
    virtual bool ISectRayBS(const hsPoint3& from, const hsPoint3& to, hsPoint3& at) const;

    void Read(hsStream *s) override;
    void Write(hsStream *s) override;
};

inline float hsBounds3Ext::GetRadius() const
{
    if( !(fExtFlags & kSphereSet) )
        IMakeSphere();
    return fRadius;
}

class hsHitInfoExt {
public:
    float        fDepth;
    hsVector3       fNormal;
    hsVector3       fDelPos;

    const hsBounds3Ext* fBoxBnd;
    const hsBounds3Ext* fOtherBoxBnd;
    const hsPoint3*     fRootCenter;

    hsHitInfoExt(const hsPoint3 *ctr, const hsVector3& offset) { fRootCenter=ctr; fDelPos=offset; };

    void Set(const hsBounds3Ext *m, const hsVector3* n, float d)
    { fDepth = d; fBoxBnd = m; fNormal = *n; fOtherBoxBnd = nullptr; }
    void Set(const hsBounds3Ext *m, const hsBounds3Ext *o, const hsVector3 &norm, float d)
    { fDepth = d; fBoxBnd = m, fOtherBoxBnd = o; fNormal = norm; }
};
#endif // hsBounds_inc
