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
    static const hsScalar kRealSmall;

    hsBounds() : fType(kBoundsUninitialized) { };

    hsBounds& MakeEmpty() { fType = kBoundsEmpty; return *this; }
    hsBounds& MakeFull()    { fType = kBoundsFull; return *this; }
    hsBoundsType GetType() const { return fType; }

    //
    // These set type to kBounds Normal
    // 
    virtual void    Reset(const hsBounds3*) = 0;

    virtual hsBool IsInside(const hsPoint3* pos) const =0;  // Only valid for kBounds Normal

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
    mutable UInt32      fBounds3Flags;
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
    virtual void    Reset(const hsBounds3*);
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
    hsScalar GetMaxDim() const;     // Computes the answer
    const hsPoint3&  GetCenter() const; // Computes the answer if not already there
    virtual hsBool IsInside(const hsPoint3* pos) const; // ok for full/empty
    virtual void TestPlane(const hsVector3 &n, hsPoint2 &depth) const;
    virtual void TestPlane(const hsPlane3 *p, hsPoint2 &depth) const;
    virtual hsBool ClosestPoint(const hsPoint3& p, hsPoint3& inner, hsPoint3& outer) const;

    // Test according to my axes only, doesn't check other's axes
    // neg, pos, zero == disjoint, I contain other, overlap
    virtual Int32 TestBound(const hsBounds3& other) const; 

    static hsScalar ClosestPointToLine(const hsPoint3 *p, const hsPoint3 *v0, const hsPoint3 *v1, hsPoint3 *out);
    static hsScalar ClosestPointToInfiniteLine(const hsPoint3* p, const hsVector3* v, hsPoint3* out);

    virtual void Read(hsStream*);
    virtual void Write(hsStream*);
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

inline hsScalar hsBounds3::GetMaxDim() const
{ 
    hsAssert(kBoundsNormal == fType, "Invalid type for GetMaxDim");
    return hsMaximum(hsMaximum(fMaxs.fX-fMins.fX, fMaxs.fY-fMins.fY), fMaxs.fZ-fMins.fZ);
}

//
// A convex region specified by a series of planes.
//
class hsBoundsOriented : public hsBounds
{
private:
    hsBool      fCenterValid;
    hsPoint3    fCenter;
    hsPlane3    *fPlanes;
    UInt32      fNumPlanes;
public:
    hsBoundsOriented() : fPlanes(nil),fNumPlanes(0),fCenterValid(false) {}
    virtual ~hsBoundsOriented() {   if (fPlanes) delete [] fPlanes; }

    // Center is not computed by the class, it must be set by the creator of the class.
    void    SetCenter(const hsPoint3* c)    { fCenter=*c; fCenterValid = true; }
    void    SetCenter(const hsBounds3* b)   { hsBounds3 bb=*b; fCenter=bb.GetCenter(); fCenterValid = true; }
    void    SetCenter(const hsBoundsOriented* b)    { fCenter=b->GetCenter(); fCenterValid = true; }
    hsPoint3  GetCenter() const;

    void    SetNumberPlanes(UInt32 n);

    hsPlane3* GetPlane(int i)   { return &fPlanes[i]; }
    int     GetNumPlanes()  { return fNumPlanes; }

    //
    // These set type to kBounds Normal
    // 
    virtual void    Reset(const hsBounds3*);
    void    SetPlane(UInt32 i, hsPlane3 *p);

    //
    // Only valid for kBounds Normal
    //
    virtual hsBool IsInside(const hsPoint3* pos) const;
    virtual void TestPlane(const hsVector3 &n, hsPoint2 &depth) const; // Complain and refuse

    virtual void Write(hsStream *stream);
    virtual void Read(hsStream *stream);
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
    mutable UInt32          fExtFlags;
    hsPoint3        fCorner;
    hsVector3       fAxes[3];
    mutable hsPoint2        fDists[3];
    mutable hsScalar        fRadius;

    hsBool IAxisIsZero(UInt32 i) const { return (fExtFlags & (1 << (20+i))) != 0; };
    void IMakeSphere() const;
    void IMakeDists() const;
    void IMakeMinsMaxs();
public:
    hsBounds3Ext() : fExtFlags(kAxisAligned) {};

    hsBounds3Ext(const hsBounds3 &b);
    hsBounds3Ext &operator=(const hsBounds3 &b);
    hsBounds3Ext(const hsBounds3Ext &pRHS) { Reset(&pRHS); }
    hsBounds3Ext &operator=(const hsBounds3Ext &pRHS ) 
    {   if (&pRHS != this)  Reset(&pRHS);   return *this; }

    virtual void Reset(const hsBounds3Ext *b);
    virtual void Reset(const hsBounds3 *b);
    virtual void Reset(const hsPoint3 *p);
    virtual void Reset(int n, const hsPoint3 *p);

    virtual void Union(const hsPoint3 *p);
    virtual void Union(const hsBounds3 *b);

    virtual void Union(const hsVector3 *v); // smears the bounds in given direction
    virtual void MakeSymmetric(const hsPoint3* p); // Expands bounds to be symmetric about p
    virtual void InscribeSphere();
    virtual void Unalign();

    virtual void Transform(const hsMatrix44 *m);
    virtual void Translate(const hsVector3 &v);

    virtual hsScalar GetRadius() const;
    virtual void GetAxes(hsVector3 *fAxis0, hsVector3 *fAxis1, hsVector3 *fAxis2) const;
    virtual hsPoint3 *GetCorner(hsPoint3 *c) const { *c = (fExtFlags & kAxisAligned ? fMins : fCorner); return c; }
    virtual void GetCorners(hsPoint3 *b) const;
    virtual hsBool ClosestPoint(const hsPoint3& p, hsPoint3& inner, hsPoint3& outer) const;

    virtual hsBool IsInside(const hsPoint3* pos) const; // ok for full/empty

    virtual void TestPlane(const hsVector3 &n, hsPoint2 &depth) const; 
    virtual Int32 TestPoints(int n, const hsPoint3 *pList) const; // pos,neg,zero == allout, allin, cut

    // Test according to my axes only, doesn't check other's axes
    // neg, pos, zero == disjoint, I contain other, overlap
    virtual Int32 TestBound(const hsBounds3Ext& other) const; 

    virtual void TestPlane(const hsVector3 &n, const hsVector3 &myVel, hsPoint2 &depth) const; 
    virtual void TestPlane(const hsPlane3 *p, const hsVector3 &myVel, hsPoint2 &depth) const; 
    virtual Int32 TestPoints(int n, const hsPoint3 *pList, const hsVector3 &ptVel) const; // pos,neg,zero == allout, allin, cut
    virtual hsBool ISectBB(const hsBounds3Ext &other, const hsVector3 &myVel) const;
    virtual hsBool ISectBB(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;
    virtual hsBool ISectABB(const hsBounds3Ext &other, const hsVector3 &myVel) const;
    virtual hsBool ISectBS(const hsBounds3Ext &other, const hsVector3 &myVel) const;

    virtual Int32 IClosestISect(const hsBounds3Ext& other, const hsVector3& myVel,
                                  hsScalar* tClose, hsScalar* tImpact) const;
    virtual hsBool ISectBoxBS(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;
    virtual hsBool ISectBSBox(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;
    virtual hsBool ISectBoxBS(const hsBounds3Ext &other, const hsVector3 &myVel) const;
    virtual hsBool ISectBSBS(const hsBounds3Ext &other, const hsVector3 &myVel, hsHitInfoExt *hit) const;

    virtual hsBool ISectLine(const hsPoint3* from, const hsPoint3* to) const;
    virtual hsBool ISectCone(const hsPoint3* from, const hsPoint3* to, hsScalar radius) const;
    virtual hsBool ISectRayBS(const hsPoint3& from, const hsPoint3& to, hsPoint3& at) const;

    virtual void Read(hsStream *s);
    virtual void Write(hsStream *s);
};

inline hsScalar hsBounds3Ext::GetRadius() const
{
    if( !(fExtFlags & kSphereSet) )
        IMakeSphere();
    return fRadius;
}

class hsHitInfoExt {
public:
    hsScalar        fDepth;
    hsVector3       fNormal;
    hsVector3       fDelPos;

    const hsBounds3Ext* fBoxBnd;
    const hsBounds3Ext* fOtherBoxBnd;
    const hsPoint3*     fRootCenter;

    hsHitInfoExt(const hsPoint3 *ctr, const hsVector3& offset) { fRootCenter=ctr; fDelPos=offset; };

    void Set(const hsBounds3Ext *m, const hsVector3* n, hsScalar d)
    { fDepth = d; fBoxBnd = m; fNormal = *n; fOtherBoxBnd = nil; }
    void Set(const hsBounds3Ext *m, const hsBounds3Ext *o, const hsVector3 &norm, hsScalar d)
    { fDepth = d; fBoxBnd = m, fOtherBoxBnd = o; fNormal = norm; }
};
#endif // hsBounds_inc
