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

#include "pnSceneObject/plObjInterface.h"
#include "hsMatrix44.h"
#include "hsColorRGBA.h"
#include "plIntersect/plVolumeIsect.h"
#include "hsBitVector.h"

class hsStream;
class hsResMgr;
class plSpaceTree;

class hsGDeviceRef;
class hsGMaterial;
class plDrawableSpans;
class plLayerInterface;
class plPipeline;
class plDrawable;
class plSoftVolume;
class hsBounds3Ext;
class plVisRegion;

class plRenderRequest;
class plRenderTarget;

class plLightProxy;

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
    hsTArray<plVisRegion*>      fVisRegions;

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

    hsBool                      fRegisteredForRenderMsg;

    // Small shadow section
    hsBitVector                 fSlaveBits;

    virtual void                IMakeIsect() = 0;
    virtual plVolumeIsect*      IGetIsect() = 0;
    virtual void                IRefresh();
    
    virtual const hsMatrix44&   IGetWorldToProj() const { return fWorldToProj; }

    void                        IAddVisRegion(plVisRegion* reg);
    void                        IRemoveVisRegion(plVisRegion* reg);

    virtual void ISetSceneNode(plKey node);

    void                        ICheckMaxStrength();
public:
    plLightInfo();
    virtual ~plLightInfo();

    CLASSNAME_REGISTER( plLightInfo );
    GETINTERFACE_ANY( plLightInfo, plObjInterface );

    void SetDeviceRef(hsGDeviceRef* ref);
    hsGDeviceRef* GetDeviceRef() const { return fDeviceRef; }

    // Dirty state is local to this machine, so shouldn't be in the network synchronized properties.
    hsBool  IsDirty() const { return 0 != (fVolFlags & kVolDirty); }
    void    SetDirty(hsBool on=true) { if(on)fVolFlags |= kVolDirty; else fVolFlags &= ~kVolDirty; }    

    hsBool  IsEmpty() const { return 0 != (fVolFlags & kVolEmpty); }
    void    SetEmpty(hsBool on=true) { if(on)fVolFlags |= kVolEmpty; else fVolFlags &= ~kVolEmpty; }

    hsBool  IsZero() const { return 0 != (fVolFlags & kVolZero); }
    void    SetZero(hsBool on) { if(on)fVolFlags |= kVolZero; else fVolFlags &= ~kVolZero; }

    inline hsBool   IsIdle() const;

    hsBool  OverAll() const { return GetProperty(kLPOverAll); }

    hsBool IsShadowCaster() const { return GetProperty(kLPCastShadows); }
    void SetShadowCaster(hsBool on) { SetProperty(kLPCastShadows, on); }

    void Refresh() { if( IsDirty() ) { IRefresh(); SetDirty(false); } }
    virtual void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const;

    hsBool AffectsBound(const hsBounds3Ext& bnd) { return IGetIsect() ? IGetIsect()->Test(bnd) != kVolumeCulled : true; }
    void GetAffectedForced(const plSpaceTree* space, hsBitVector& list, hsBool charac);
    void GetAffected(const plSpaceTree* space, hsBitVector& list, hsBool charac);
    const hsTArray<int16_t>& GetAffected(plSpaceTree* space, const hsTArray<int16_t>& visList, hsTArray<int16_t>& litList, hsBool charac);
    hsBool InVisSet(const hsBitVector& visSet) const { return fVisSet.Overlap(visSet); }
    hsBool InVisNot(const hsBitVector& visNot) const { return fVisNot.Overlap(visNot); }

    void SetAmbient(const hsColorRGBA& c) { fAmbient = c; SetDirty(); }
    void SetDiffuse(const hsColorRGBA& c) { fDiffuse = c; SetDirty(); }
    void SetSpecular(const hsColorRGBA& c);

    const hsColorRGBA& GetAmbient() const { return fAmbient; }
    const hsColorRGBA& GetDiffuse() const { return fDiffuse; }
    const hsColorRGBA& GetSpecular() const { return fSpecular; }

    plLayerInterface*   GetProjection() const { return fProjection; }

    virtual void SetProperty(int prop, hsBool on);

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
    virtual const hsMatrix44& GetLocalToWorld() const;
    virtual const hsMatrix44& GetWorldToLocal() const;
    virtual const hsMatrix44& GetLightToWorld() const;
    virtual const hsMatrix44& GetWorldToLight() const;

    virtual int32_t   GetNumProperties() const { return kNumProps; }

    const plSoftVolume* GetSoftVolume() const { return fSoftVolume; }

    virtual hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const = 0;

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    virtual hsBool      MsgReceive(plMessage* msg);

    virtual void        Unlink( void );
    virtual void        Link( plLightInfo **back );
    virtual plLightInfo *GetNext( void ) { return fNextDevPtr; }
    virtual hsBool      IsLinked( void ) { return ( fNextDevPtr != nil || fPrevDevPtr != nil ) ? true : false; }

    // New shadow
    void                ClearSlaveBits() { fSlaveBits.Clear(); }
    void                SetSlaveBit(int which) { fSlaveBits.SetBit(which); }
    const hsBitVector&  GetSlaveBits() const { return fSlaveBits; }

    // These two should only be called internally and on export/convert
    virtual plKey GetSceneNode() const;

    // Export only. At runtime, the LocalToLight should be considered const.
    void                SetLocalToLight(const hsMatrix44& l2lt, const hsMatrix44& lt2l);

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo) { return addTo; }

};

class plDirectionalLightInfo : public plLightInfo
{
protected:

    virtual void                IMakeIsect() {}
    virtual plVolumeIsect*      IGetIsect() { return nil; }

public:
    plDirectionalLightInfo();
    virtual ~plDirectionalLightInfo();

    CLASSNAME_REGISTER( plDirectionalLightInfo );
    GETINTERFACE_ANY( plDirectionalLightInfo, plLightInfo );

    virtual void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const;

    hsVector3 GetWorldDirection() const;
    virtual hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const { return -GetWorldDirection(); }

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

};

class plLimitedDirLightInfo : public plDirectionalLightInfo
{
protected:

    float                    fWidth;
    float                    fHeight;
    float                    fDepth;

    plParallelIsect*            fParPlanes;

    virtual void                IMakeIsect();
    virtual plVolumeIsect*      IGetIsect() { return fParPlanes; }

    virtual void                IRefresh();

public:
    plLimitedDirLightInfo();
    virtual ~plLimitedDirLightInfo();

    CLASSNAME_REGISTER( plLimitedDirLightInfo );
    GETINTERFACE_ANY( plLimitedDirLightInfo, plDirectionalLightInfo );

    virtual void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const;

    float GetWidth() const { return fWidth; }
    float GetHeight() const { return fHeight; }
    float GetDepth() const { return fDepth; }

    void SetWidth(float w) { fWidth = w; }
    void SetHeight(float h) { fHeight = h; }
    void SetDepth(float d) { fDepth = d; }

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);
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

    virtual void                IMakeIsect();
    virtual plVolumeIsect*      IGetIsect() { return fSphere; }

    virtual void                IRefresh();


public:
    plOmniLightInfo();
    virtual ~plOmniLightInfo();

    CLASSNAME_REGISTER( plOmniLightInfo );
    GETINTERFACE_ANY( plOmniLightInfo, plLightInfo );

    virtual void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const;

    virtual hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const;

    hsBool      IsAttenuated() const { return (fAttenLinear != 0)||(fAttenQuadratic != 0) || ( fAttenCutoff != 0 ); }
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

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);

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

    virtual void                IMakeIsect();
    virtual plVolumeIsect*      IGetIsect() { return fCone; }

    virtual void                IRefresh();

public:
    plSpotLightInfo();
    virtual ~plSpotLightInfo();

    CLASSNAME_REGISTER( plSpotLightInfo );
    GETINTERFACE_ANY( plSpotLightInfo, plOmniLightInfo );

    virtual void GetStrengthAndScale(const hsBounds3Ext& bnd, float& strength, float& scale) const;

    hsVector3 GetWorldDirection() const;
    virtual hsVector3 GetNegativeWorldDirection(const hsPoint3& pos) const { return -GetWorldDirection(); }

    void SetFalloff(float f) { fFalloff = f; SetDirty(true); }
    void SetSpotInner(float rads) { fSpotInner = rads; SetDirty(true); }
    void SetSpotOuter(float rads) { fSpotOuter = rads; SetDirty(true); }

    float GetFalloff() const { return fFalloff; }
    float GetSpotInner() const { return fSpotInner; }
    float GetSpotOuter() const { return fSpotOuter; }

    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);

};

inline hsBool plLightInfo::IsIdle() const
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
