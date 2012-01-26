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

#ifndef plCutter_inc
#define plCutter_inc

#include "pnFactory/plCreatable.h"

#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "hsBounds.h"
#include "plIntersect/plVolumeIsect.h"
#include "hsColorRGBA.h"

struct hsPoint3;
struct hsVector3;

class plPrintCollect;
class plAccTriIterator;
class plAccessSpan;

class plCutoutHit
{
public:
    hsPoint3    fPos;
    hsVector3   fNorm;
};

class plCutoutVtx
{
public:
    plCutoutVtx& Init(const hsPoint3& p, const hsVector3& n, const hsColorRGBA& c) { fPos = p; fNorm = n; fColor = c; return *this; }

    hsPoint3    fPos;
    hsVector3   fNorm;
    hsColorRGBA fColor;
    hsPoint3    fUVW;
};

class plCutoutPoly
{
public:
    hsTArray<plCutoutVtx>       fVerts;

    hsBool                      fBaseHasAlpha;
};

class plCutoutMiniVtx
{
public:
    hsPoint3    fPos;
    hsPoint3    fUVW;
};

class plFlatGridMesh
{
public:
    hsTArray<plCutoutMiniVtx>   fVerts;
    hsTArray<uint16_t>            fIdx;

    void Reset() { fVerts.SetCount(0); fIdx.SetCount(0); }
};

class plCutter : public plCreatable
{
protected:

    // Permanent attributes
    float fLengthU;
    float fLengthV;
    float fLengthW;

    // Internal cached stuff
    float        fDistU;
    float        fDistV;
    float        fDistW;
    hsVector3       fDirU;
    hsVector3       fDirV;
    hsVector3       fDirW;
    hsVector3       fBackDir;

    hsBounds3Ext    fWorldBounds;
    plBoundsIsect   fIsect;

    void            IConstruct(hsTArray<plCutoutPoly>& dst, hsTArray<plCutoutVtx>& poly, hsBool baseHasAlpha) const;
    hsBool          IPolyClip(hsTArray<plCutoutVtx>& poly, const hsPoint3 vPos[]) const;
    
    inline void     ICutoutVtxHiU(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxHiV(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxHiW(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxLoU(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxLoV(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxLoW(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxMidV(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxMidU(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;
    inline void     ICutoutVtxMidW(const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;

    hsBool          IFindHitPoint(const hsTArray<plCutoutVtx>& inPoly, plCutoutHit& hit) const;

    inline void     ISetPosNorm(float parm, const plCutoutVtx& inVtx, const plCutoutVtx& outVtx, plCutoutVtx& dst) const;

    void            ICutoutTransformed(plAccessSpan& src, hsTArray<plCutoutPoly>& dst) const;
    void            ICutoutConstHeight(plAccessSpan& src, hsTArray<plCutoutPoly>& dst) const;
    void            ICutoutTransformedConstHeight(plAccessSpan& src, hsTArray<plCutoutPoly>& dst) const;


public:
    plCutter() {}
    virtual ~plCutter() {}

    CLASSNAME_REGISTER( plCutter );
    GETINTERFACE_ANY( plCutter, plCreatable );


    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    hsBool      FindHitPoints(const hsTArray<plCutoutPoly>& src, hsTArray<plCutoutHit>& hits) const;
    hsBool      FindHitPointsConstHeight(const hsTArray<plCutoutPoly>& src, hsTArray<plCutoutHit>& hits, float height) const;

    void        Set(const hsPoint3& pos, const hsVector3& dir, const hsVector3& out, hsBool flip=false);

    void        Cutout(plAccessSpan& src, hsTArray<plCutoutPoly>& dst) const;
    hsBool      CutoutGrid(int nWid, int nLen, plFlatGridMesh& dst) const;

    void        SetLength(const hsVector3& s) { fLengthU = s.fX; fLengthV = s.fY; fLengthW = s.fZ; }
    float    GetLengthU() const { return fLengthU; }
    float    GetLengthV() const { return fLengthV; }
    float    GetLengthW() const { return fLengthW; }

    const hsBounds3Ext& GetWorldBounds() const { return fWorldBounds; }
    plBoundsIsect& GetIsect() { return fIsect; }
    hsVector3 GetBackDir() const { return fBackDir; }

    static hsBool MakeGrid(int nWid, int nLen, const hsPoint3& center, const hsVector3& halfU, const hsVector3& halfV, plFlatGridMesh& grid);

};

#endif // plCutter_inc
