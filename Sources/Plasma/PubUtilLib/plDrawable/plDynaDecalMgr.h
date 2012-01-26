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

#ifndef plDynaDecalMgr_inc
#define plDynaDecalMgr_inc

#include "pnNetCommon/plSynchedObject.h"
#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsStlUtils.h"


class plParticleSystem;

class plPrintShape;
class plDynaDecalEnableMsg;
class plDynaDecal;
class plDrawableSpans;
class plGBufferGroup;
class plIcicle;
class hsGMaterial;
class plBitmap;
class plMipmap;
class plSceneObject;
class plArmatureMod;

class hsStream;
class hsResMgr;
class plMessage;

class plCutter;
class plCutoutPoly;
class plFlatGridMesh;

class plDrawVisList;
class plRenderLevel;

class plAccessSpan;
class plAuxSpan;
class plDecalVtxFormat;

class plPipeline;

// plDynaDecalInfo - information we store specific to what we've
// done about a specific avatar part or scene object.
class plDynaDecalInfo
{
public:
    enum
    {
        kNone           = 0x0,
        kImmersed       = 0x1,
        kActive         = 0x2
    };

    plKey       fKey;

    double      fLastTime;
    hsPoint3    fLastPos;
    double      fWetTime;
    float    fWetLength;
    uint32_t      fFlags;

    plDynaDecalInfo& Init(const plKey& key);
};

typedef std::map< uintptr_t, plDynaDecalInfo, std::less<uintptr_t> > plDynaDecalMap;

// plDynaDecalMgr
// Primary responsibilities:
//  Allocation of adequate buffer space in plGBufferGroup
//  Setup of decal materials
//  Allocation of auxSpans
//  Receive lists of polys, translate into drawable tris
//  Create DynaDecals and destroy them when they expire.
//  Assign vertex and index subsets to DynaDecals
//  Call Update on DynaDecals
class plDynaDecalMgr : public plSynchedObject
{
public:
    enum DynaRefType
    {
        kRefMatPreShade,
        kRefMatRTShade,
        kRefTarget,
        kRefAvatar,
        kRefPartyObject,
        kRefParticles, 
        kRefNextAvailable   = 10
    };
protected:
    static hsBool               fDisableAccumulate;
    static hsBool               fDisableUpdate;

    plDynaDecalMap              fDecalMap;

    hsTArray<plDynaDecal*>      fDecals;

    hsTArray<plGBufferGroup*>   fGroups;

    plCutter*                   fCutter;

    hsTArray<plAuxSpan*>        fAuxSpans;

    hsGMaterial*                fMatPreShade;
    hsGMaterial*                fMatRTShade;

    hsTArray<plSceneObject*>    fTargets;

    hsTArray<plSceneObject*>    fPartyObjects;
    hsTArray<plParticleSystem*> fParticles;

    float                    fPartyTime;

    uint16_t                      fMaxNumVerts;
    uint16_t                      fMaxNumIdx;

    uint32_t                      fWaitOnEnable;
    
    float                    fWetLength;
    float                    fRampEnd;
    float                    fDecayStart;
    float                    fLifeSpan;
    float                    fIntensity;

    float                    fGridSizeU;
    float                    fGridSizeV;

    hsVector3                   fScale;

    // some temp calculated stuff
    float                    fInitAtten; 
    // These 4 are in normalized units [0..1], not feet.
    float                    fMinDepth;
    float                    fMinDepthRange;
    float                    fMaxDepth;
    float                    fMaxDepthRange;

    hsTArray<uint32_t>            fPartIDs;
    hsTArray<plKey>             fNotifies;

    const plPrintShape* IGetPrintShape(const plKey& objKey) const;
    const plPrintShape* IGetPrintShape(plArmatureMod* avMod, uint32_t id) const;

    virtual hsBool      IHandleEnableMsg(const plDynaDecalEnableMsg* enaMsg);
    void                INotifyActive(plDynaDecalInfo& info, const plKey& armKey, uint32_t id) const;
    void                INotifyInactive(plDynaDecalInfo& info, const plKey& armKey, uint32_t id) const;
    hsBool              IWetParts(const plDynaDecalEnableMsg* enaMsg);
    hsBool              IWetPart(uint32_t id, const plDynaDecalEnableMsg* enaMsg);
    void                IWetInfo(plDynaDecalInfo& info, const plDynaDecalEnableMsg* enaMsg) const;
    float            IHowWet(plDynaDecalInfo& info, double t) const;
    plDynaDecalInfo&    IGetDecalInfo(uintptr_t id, const plKey& key);
    void                IRemoveDecalInfo(uint32_t id);
    void                IRemoveDecalInfos(const plKey& key);

    hsGMaterial*        ISetAuxMaterial(plAuxSpan* aux, hsGMaterial* mat, hsBool rtLit);
    void                IAllocAuxSpan(plAuxSpan* aux, uint32_t maxNumVerts, uint32_t maxNumIdx);
    plAuxSpan*          IGetAuxSpan(plDrawableSpans* targ, int iSpan, hsGMaterial* mat, uint16_t numVerts, uint16_t numIdx);
    hsBool              IMakeAuxRefs(plPipeline* pipe);

    uint16_t*             IGetBaseIdxPtr(const plAuxSpan* auxSpan) const;
    plDecalVtxFormat*   IGetBaseVtxPtr(const plAuxSpan* auxSpan) const;

    virtual int         INewDecal() = 0;
    plDynaDecal*        IInitDecal(plAuxSpan* aux, double t, uint16_t numVerts, uint16_t numIdx);
    void                IKillDecal(int i);
    void                IUpdateDecals(double t);

    void                ICountIncoming(hsTArray<plCutoutPoly>& src, uint16_t& numVerts, uint16_t& numIdx) const;
    hsBool              IConvertPolysColor(plAuxSpan* auxSpan, plDynaDecal* decal, hsTArray<plCutoutPoly>& src);
    hsBool              IConvertPolysAlpha(plAuxSpan* auxSpan, plDynaDecal* decal, hsTArray<plCutoutPoly>& src);
    hsBool              IConvertPolysVS(plAuxSpan* auxSpan, plDynaDecal* decal, hsTArray<plCutoutPoly>& src);
    hsBool              IConvertPolys(plAuxSpan* auxSpan, plDynaDecal* decal, hsTArray<plCutoutPoly>& src);
    hsBool              IProcessPolys(plDrawableSpans* targ, int iSpan, double t, hsTArray<plCutoutPoly>& src);
    hsBool              IHitTestPolys(hsTArray<plCutoutPoly>& src) const;

    hsBool              IProcessGrid(plDrawableSpans* targ, int iSpan, hsGMaterial* mat, double t, const plFlatGridMesh& grid);
    hsBool              IConvertFlatGrid(plAuxSpan* auxSpan, plDynaDecal* decal, const plFlatGridMesh& grid) const;
    hsBool              ICutoutGrid(plDrawableSpans* drawable, int iSpan, hsGMaterial* mat, double secs);
    hsBool              IHitTestFlatGrid(const plFlatGridMesh& grid) const;

    hsBool              ICutoutList(hsTArray<plDrawVisList>& drawVis, double secs);
    hsBool              ICutoutObject(plSceneObject* so, double secs);
    hsBool              ICutoutTargets(double secs);

    void                ISetDepthFalloff(); // Sets from current cutter settings.

    virtual void        ICutoutCallback(const hsTArray<plCutoutPoly>& cutouts, hsBool hasWaterHeight=false, float waterHeight=0.f);

    hsGMaterial*        IConvertToEnvMap(hsGMaterial* mat, plBitmap* envMap);

    virtual void        SetKey(plKey k);

    hsVector3           IReflectDir(hsVector3 dir) const;
    hsMatrix44          IL2WFromHit(hsPoint3 pos, hsVector3 dir) const;
    hsVector3           IRandomUp(hsVector3 dir) const;
    void                IGetParticles();

public:

    plDynaDecalMgr();
    virtual ~plDynaDecalMgr();

    CLASSNAME_REGISTER( plDynaDecalMgr );
    GETINTERFACE_ANY( plDynaDecalMgr, plSynchedObject );

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    virtual hsBool MsgReceive(plMessage* msg);

    // This is public, because you need to call it after creating
    // a DynaDecalMgr on the fly. It's normally called on Read().
    void    InitAuxSpans();

    void SetScale(const hsVector3& v) { fScale = v; }
    const hsVector3& GetScale() const { return fScale; }

    void SetWaitOnEnable(hsBool on) { fWaitOnEnable = on; }
    hsBool GetWaitOnEnable() const { return fWaitOnEnable; }

    void SetWetLength(float f) { fWetLength = f; }
    void SetRampEnd(float f) { fRampEnd = f; }
    void SetDecayStart(float f) { fDecayStart = f; }
    void SetLifeSpan(float f) { fLifeSpan = f; }
    void SetIntensity(float f) { fIntensity = f; }
    float GetWetLength() const { return fWetLength; }
    float GetRampEnd() const { return fRampEnd; }
    float GetDecayStart() const { return fDecayStart; }
    float GetLifeSpan() const { return fLifeSpan; }
    float GetIntensity() const { return fIntensity; }

    void        SetPartyTime(float secs) { fPartyTime = secs; } // Duration of particle spewage
    float    GetPartyTime() const { return fPartyTime; }

    void ConvertToEnvMap(plBitmap* envMap);
    const plMipmap* GetMipmap() const;

    void AddNotify(const plKey& k) { fNotifies.Append(k); }
    uint32_t GetNumNotifies() const { return fNotifies.GetCount(); }
    const plKey& GetNotify(int i) const { return fNotifies[i]; }

    static void SetDisableAccumulate(hsBool on) { fDisableAccumulate = on; }
    static void ToggleDisableAccumulate() { fDisableAccumulate = !fDisableAccumulate; }
    static hsBool GetDisableAccumulate() { return fDisableAccumulate; }

    static void SetDisableUpdate(hsBool on) { fDisableUpdate = on; }
    static void ToggleDisableUpdate() { fDisableUpdate = !fDisableUpdate; }
    static hsBool GetDisableUpdate() { return fDisableUpdate; }
};

#endif // plDynaDecalMgr_inc
