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

#ifndef plVolumeIsect_inc
#define plVolumeIsect_inc

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsTemplates.h"
#include "hsBounds.h"

#include "pnFactory/plCreatable.h"

class hsBounds3Ext;

enum plVolumeCullResult
{
    kVolumeClear        = 0x0,
    kVolumeCulled       = 0x1,
    kVolumeSplit        = 0x2
};


class plVolumeIsect : public plCreatable
{
public:
    CLASSNAME_REGISTER( plVolumeIsect );
    GETINTERFACE_ANY( plVolumeIsect, plCreatable );

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) = 0;
    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const = 0;    
    virtual float            Test(const hsPoint3& pos) const = 0;

    virtual void Read(hsStream* s, hsResMgr* mgr) = 0;
    virtual void Write(hsStream* s, hsResMgr* mgr) = 0;
};

class plSphereIsect : public plVolumeIsect
{
protected:
    hsPoint3            fCenter;
    hsPoint3            fWorldCenter;
    float            fRadius;
    hsPoint3            fMins;
    hsPoint3            fMaxs;
public:
    plSphereIsect();
    virtual ~plSphereIsect();

    CLASSNAME_REGISTER( plSphereIsect );
    GETINTERFACE_ANY( plSphereIsect, plVolumeIsect );

    void SetCenter(const hsPoint3& c);
    void SetRadius(float r);

    float GetRadius() const { return fRadius; }

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;    
    virtual float            Test(const hsPoint3& pos) const; // return 0 if point inside, else "distance" from pos to volume

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plConeIsect : public plVolumeIsect
{
protected:
    hsBool              fCapped;

    float            fRadAngle;
    float            fLength;

    hsPoint3            fWorldTip;
    hsVector3           fWorldNorm;

    hsMatrix44          fWorldToNDC;
    hsMatrix44          fLightToNDC;

    hsVector3           fNorms[5];
    float            fDists[5];

    void                ISetup();
public:

    plConeIsect();
    virtual ~plConeIsect();

    CLASSNAME_REGISTER( plConeIsect );
    GETINTERFACE_ANY( plConeIsect, plVolumeIsect );

    void SetAngle(float rads);
    void SetLength(float d);

    float GetLength() const { return fCapped ? fLength : 0; }
    float GetAngle() const { return fRadAngle; }

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plCylinderIsect : public plVolumeIsect
{
protected:
    hsPoint3        fTop;
    hsPoint3        fBot;
    float        fRadius;

    hsPoint3        fWorldBot;
    hsVector3       fWorldNorm;
    float        fLength;
    float        fMin;
    float        fMax;

    void ISetupCyl(const hsPoint3& wTop, const hsPoint3& wBot, float radius);

public:
    plCylinderIsect();
    virtual ~plCylinderIsect();

    CLASSNAME_REGISTER( plCylinderIsect );
    GETINTERFACE_ANY( plCylinderIsect, plVolumeIsect );

    void SetCylinder(const hsPoint3& lTop, const hsPoint3& lBot, float radius);
    void SetCylinder(const hsPoint3& lBot, const hsVector3& axis, float radius);

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plParallelIsect : public plVolumeIsect
{
protected:
    class ParPlane
    {
    public:
        hsVector3           fNorm;
        float            fMin;
        float            fMax;

        hsPoint3            fPosOne;
        hsPoint3            fPosTwo;
    };
    hsTArray<ParPlane>      fPlanes;

public:
    plParallelIsect();
    virtual ~plParallelIsect();

    CLASSNAME_REGISTER( plParallelIsect );
    GETINTERFACE_ANY( plParallelIsect, plVolumeIsect );

    void SetNumPlanes(int n); // each plane is really two parallel planes
    uint16_t GetNumPlanes() const { return fPlanes.GetCount(); }

    void SetPlane(int which, const hsPoint3& locPosOne, const hsPoint3& locPosTwo);

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plConvexIsect : public plVolumeIsect
{
protected:
    class SinglePlane
    {
    public:
        hsVector3       fNorm;
        float        fDist;
        hsPoint3        fPos;

        hsVector3       fWorldNorm;
        float        fWorldDist;
    };

    hsTArray<SinglePlane>   fPlanes;

public:
    plConvexIsect();
    virtual ~plConvexIsect();

    CLASSNAME_REGISTER( plConvexIsect );
    GETINTERFACE_ANY( plConvexIsect, plVolumeIsect );

    void ClearPlanes() { fPlanes.SetCount(0); }
    void AddPlaneUnchecked(const hsVector3& n, float dist); // no validation here
    void AddPlane(const hsVector3& n, const hsPoint3& p);
    uint16_t GetNumPlanes() const { return fPlanes.GetCount(); }

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plBoundsIsect : public plVolumeIsect
{
protected:
    hsBounds3Ext        fLocalBounds;
    hsBounds3Ext        fWorldBounds;
public:
    plBoundsIsect();
    virtual ~plBoundsIsect();

    CLASSNAME_REGISTER( plBoundsIsect );
    GETINTERFACE_ANY( plBoundsIsect, plVolumeIsect );

    void SetBounds(const hsBounds3Ext& bnd);

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plComplexIsect : public plVolumeIsect
{
protected:
    hsTArray<plVolumeIsect*>        fVolumes;

public:
    plComplexIsect();
    virtual ~plComplexIsect();

    CLASSNAME_REGISTER( plComplexIsect );
    GETINTERFACE_ANY( plComplexIsect, plVolumeIsect );

    void AddVolume(plVolumeIsect* v); // Will capture pointer
    uint16_t GetNumVolumes() const { return fVolumes.GetCount(); }

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
};

class plUnionIsect : public plComplexIsect
{
public:
    plUnionIsect();
    ~plUnionIsect();

    CLASSNAME_REGISTER( plUnionIsect );
    GETINTERFACE_ANY( plUnionIsect, plComplexIsect );

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;
};

class plIntersectionIsect : public plComplexIsect
{
public:
    plIntersectionIsect();
    ~plIntersectionIsect();

    CLASSNAME_REGISTER( plIntersectionIsect );
    GETINTERFACE_ANY( plIntersectionIsect, plComplexIsect );

    virtual plVolumeCullResult  Test(const hsBounds3Ext& bnd) const;
    virtual float            Test(const hsPoint3& pos) const;
};

#endif // plVolumeIsect_inc
