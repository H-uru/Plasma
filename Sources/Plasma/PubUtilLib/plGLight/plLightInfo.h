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

#ifndef plLightInfo_inc
#define plLightInfo_inc

#include <vector>

#include "hsBitVector.h"
#include "hsColorRGBA.h"
#include "hsMatrix44.h"

#include "pnSceneObject/plObjInterface.h"

class hsBounds3Ext;
class plConeIsect;
class plDrawable;
class plDrawableSpans;
class hsGDeviceRef;
class hsGMaterial;
class plLayerInterface;
class plLightProxy;
class plParallelIsect;
class plPipeline;
class hsResMgr;
class plRenderRequest;
class plRenderTarget;
class plSoftVolume;
class plSpaceTree;
class plSphereIsect;
class hsStream;
class plVisRegion;
class plVolumeIsect;

class plLightInfo : public plObjInterface
{
public:
    enum {
        kDisable            = 0x0,  // prop 0 is always disable, declared in plObjInterface
        kLPObsolete,
        kLPCastShadows,
        kLPMovable,
        kLPHasIncludes,
        kLPIncludesChars,
        kLP_OBSOLECTE_0, // OLD
        kLPOverAll,
        kLPHasSpecular,             // This is the same as a non-black specular color, but we use it here to make it faster to check
        kLPShadowOnly,
        kLPShadowLightGroup,
        kLPForceProj,

        kNumProps
    };
    enum LIRefType
    {
        kProjection = 0,
        k_OBSOLECTE_REF_0, // OLD
        k_OBSOLECTE_REF_1, // OLD
        k_OBSOLECTE_REF_2, // OLD
        kSoftVolume,
        kVisRegion
    };
protected:
    // Volatile properties that shouldn't be network propagated.
    enum VolatileFlags {
        kVolNone        = 0x0,
        kVolDirty       = 0x1,
        kVolEmpty       = 0x2,
        kVolZero        = 0x4
    };
    uint8_t                       fVolFlags;

    hsBitVector                 fVisSet;
    hsBitVector                 fVisNot;
    std::vector<plVisRegion*>   fVisRegions;

    plLightInfo**               fPrevDevPtr;
    plLightInfo*                fNextDevPtr;
    hsGDeviceRef*               fDeviceRef;

    plLayerInterface*           fProjection;
    hsMatrix44                  fWorldToProj;

    hsColorRGBA                 fAmbient;
    hsColorRGBA                 fDiffuse;
    hsColorRGBA                 fSpecular;

    hsMatrix44                  fLightToLocal;
    hsMatrix44                  fLocalToLight;

    hsMatrix44                  fLocalToWorld;
    hsMatrix44                  fWorldToLocal;

    hsMatrix44                  fLightToWorld;
    hsMatrix44                  fWorldToLight;

    plKey                       fSceneNode;

    plLightProxy*               fProxyGen;

    plSoftVolume*               fSoftVolume;

    float                    fMaxStrength;

    bool                        fRegisteredForRenderMsg;

    // Small shadow section
    hsBitVector                 fSlaveBits;

    virtual void                IMakeIsect() = 0;
    virtual plVolumeIsect*      IGetIsect() const = 0;
    virtual void                IRefresh();
    
    virtual const hsMatrix44&   IGetWorldToProj() const { return fWorldToProj; }

    void                        IAddVisRegion(plVisRegion* reg);
    void                        IRemoveVisRegion(plVisRegion* reg);

    void ISetSceneNode(const plKey& node) override;

    void                        ICheckMaxStrength();
public:
    plLightInfo();
    virtual ~plLightInfo();

    CLASSNAME_REGISTER( plLightInfo );
    GETINTERFACE_ANY( plLightInfo, plObjInterface );

    void SetDeviceRef(hsGDeviceRef* ref);
    hsGDeviceRef* GetDeviceRef() const { return fDeviceRef; }

    // Dirty state is local to this machine, so shouldn't be in the network synchronized properties.
    bool    IsDirty() const { return 0 != (fVolFlags & kVolDirty); }
    void    SetDirty(bool on=true) { if(on)fVolFlags |= kVolDirty; else fVolFlags &= ~kVolDirty; }    

    bool    IsEmpty() const { return 0 != (fVolFlags & kVolEmpty); }
    void    SetEmpty(bool on=true) { if(on)fVolFlags |= kVolEmpty; else fVolFlags &= ~kVolEmpty; }

    bool    IsZero() const { return 0 != (fVolFlags & kVolZero); }
    void    SetZero(bool on) { if(on)fVolFlags |= kVolZero; else fVolFlags &= ~kVolZero; }

    inline bool     IsIdle() const;

    bool    OverAll() const { return GetProperty(kLPOverAll); }

    bool IsShadowCaster() const { return GetProperty(kLPCastShadows); }
    void SetShadowCaster(bool on) { SetProperty(kLPCastShadows, on); }

    void Refresh() { if( IsDirty() ) { IRefresh(); SetDirty(false); } }
    virtual void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const;

    bool AffectsBound(const hsBounds3Ext& bnd);
    void GetAffectedForced(const plSpaceTree* space, hsBitVector& list, bool charac);
    void GetAffected(const plSpaceTree* space, hsBitVector& list, bool charac);
    const std::vector<int16_t>& GetAffected(plSpaceTree* space, const std::vector<int16_t>& visList, std::vector<int16_t>& litList, bool charac);
    bool InVisSet(const hsBitVector& visSet) const { return fVisSet.Overlap(visSet); }
    bool InVisNot(const hsBitVector& visNot) const { return fVisNot.Overlap(visNot); }

    void SetAmbient(const hsColorRGBA& c) { fAmbient = c; SetDirty(); }
    void SetDiffuse(const hsColorRGBA& c) { fDiffuse = c; SetDirty(); }
    void SetSpecular(const hsColorRGBA& c);

    const hsColorRGBA& GetAmbient() const { return fAmbient; }
    const hsColorRGBA& GetDiffuse() const { return fDiffuse; }
    const hsColorRGBA& GetSpecular() const { return fSpecular; }

    plLayerInterface*   GetProjection() const { return fProjection; }

    void SetProperty(int prop, bool on) override;

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;
    virtual const hsMatrix44& GetLocalToWorld() const;
    virtual const hsMatrix44& GetWorldToLocal() const;
    virtual const hsMatrix44& GetLightToWorld() const;
    virtual const hsMatrix44& GetWorldToLight() const;

    int32_t   GetNumProperties() const override { return kNumProps; }

    const plSoftVolume* GetSoftVolume() const { return fSoftVolume; }

    virtual hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const = 0;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool        MsgReceive(plMessage* msg) override;

    virtual void        Unlink();
    virtual void        Link( plLightInfo **back );
    virtual plLightInfo *GetNext() { return fNextDevPtr; }
    virtual bool        IsLinked() { return (fNextDevPtr != nullptr || fPrevDevPtr != nullptr); }

    // New shadow
    void                ClearSlaveBits() { fSlaveBits.Clear(); }
    void                SetSlaveBit(int which) { fSlaveBits.SetBit(which); }
    const hsBitVector&  GetSlaveBits() const { return fSlaveBits; }

    // These two should only be called internally and on export/convert
    plKey GetSceneNode() const override;

    // Export only. At runtime, the LocalToLight should be considered const.
    void                SetLocalToLight(const hsMatrix44& l2lt, const hsMatrix44& lt2l);

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo) { return addTo; }

};

class plDirectionalLightInfo : public plLightInfo
{
protected:

    void                IMakeIsect() override { }
    plVolumeIsect*      IGetIsect() const override { return nullptr; }

public:
    plDirectionalLightInfo();
    virtual ~plDirectionalLightInfo();

    CLASSNAME_REGISTER( plDirectionalLightInfo );
    GETINTERFACE_ANY( plDirectionalLightInfo, plLightInfo );

    void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const override;

    hsVector3 GetWorldDirection() const;
    hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const override { return -GetWorldDirection(); }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

};

class plLimitedDirLightInfo : public plDirectionalLightInfo
{
protected:

    float                    fWidth;
    float                    fHeight;
    float                    fDepth;

    plParallelIsect*            fParPlanes;

    void           IMakeIsect() override;
    plVolumeIsect* IGetIsect() const override;

    void                IRefresh() override;

public:
    plLimitedDirLightInfo()
        : fParPlanes(), fWidth(), fHeight(), fDepth()
    { }
    ~plLimitedDirLightInfo();

    CLASSNAME_REGISTER( plLimitedDirLightInfo );
    GETINTERFACE_ANY( plLimitedDirLightInfo, plDirectionalLightInfo );

    void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const override;

    float GetWidth() const { return fWidth; }
    float GetHeight() const { return fHeight; }
    float GetDepth() const { return fDepth; }

    void SetWidth(float w) { fWidth = w; }
    void SetHeight(float h) { fHeight = h; }
    void SetDepth(float d) { fDepth = d; }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    // Visualization
    plDrawableSpans*    CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo) override;
};

class plOmniLightInfo : public plLightInfo
{
protected:
    // Omni and spot
    float            fAttenConst;
    float            fAttenLinear;
    float            fAttenQuadratic;
    float            fAttenCutoff;       // Only for attenuate-but-not-really mode, 0 otherwise

    plSphereIsect*              fSphere;

    void           IMakeIsect() override;
    plVolumeIsect* IGetIsect() const override;

    void                IRefresh() override;


public:
    plOmniLightInfo();
    virtual ~plOmniLightInfo();

    CLASSNAME_REGISTER( plOmniLightInfo );
    GETINTERFACE_ANY( plOmniLightInfo, plLightInfo );

    void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const override;

    hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const override;

    bool        IsAttenuated() const { return (fAttenLinear != 0)||(fAttenQuadratic != 0) || ( fAttenCutoff != 0 ); }
    float    GetRadius() const;

    float    GetConstantAttenuation() const { return fAttenConst; }
    float    GetLinearAttenuation() const { return fAttenLinear; }
    float    GetQuadraticAttenuation() const { return fAttenQuadratic; }
    float    GetCutoffAttenuation() const { return fAttenCutoff; }
    hsPoint3    GetWorldPosition() const { return fLightToWorld.GetTranslate(); }

    void        SetConstantAttenuation(float a) { fAttenConst = a; SetDirty(true); }
    void        SetLinearAttenuation(float a) { fAttenLinear = a; SetDirty(true); }
    void        SetQuadraticAttenuation(float a) { fAttenQuadratic = a; SetDirty(true); }
    void        SetCutoffAttenuation( float a ) { fAttenCutoff = a; SetDirty( true ); }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    // Visualization
    plDrawableSpans*    CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo) override;

};

class plSpotLightInfo : public plOmniLightInfo
{
protected:
    // Valid only for spot
    float            fFalloff;

    // Note - these are half angles, D3D wants whole angles, so fSpotInner*2 etc.
    float            fSpotInner;
    float            fSpotOuter;

    float            fEffectiveFOV;

    plConeIsect*        fCone;

    void                IMakeIsect() override;
    plVolumeIsect*      IGetIsect() const override;

    void                IRefresh() override;

public:
    plSpotLightInfo()
        : fFalloff(1.f),
          fSpotInner(hsConstants::pi<float> * 0.125f),
          fSpotOuter(hsConstants::pi<float> * 0.25f),
          fCone(), fEffectiveFOV()
    { }
    ~plSpotLightInfo();

    CLASSNAME_REGISTER( plSpotLightInfo );
    GETINTERFACE_ANY( plSpotLightInfo, plOmniLightInfo );

    void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const override;

    hsVector3 GetWorldDirection() const;
    hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const override { return -GetWorldDirection(); }

    void SetFalloff(float f) { fFalloff = f; SetDirty(true); }
    void SetSpotInner(float rads) { fSpotInner = rads; SetDirty(true); }
    void SetSpotOuter(float rads) { fSpotOuter = rads; SetDirty(true); }

    float GetFalloff() const { return fFalloff; }
    float GetSpotInner() const { return fSpotInner; }
    float GetSpotOuter() const { return fSpotOuter; }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    // Visualization
    plDrawableSpans*    CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo) override;

};

inline bool plLightInfo::IsIdle() const
{
    if( GetProperty(kDisable) )
        return true;

    if( IsZero() )
        return true;

    if( IsEmpty() )
        return true;

    return false;
}

#endif // plLightInfo_inc
