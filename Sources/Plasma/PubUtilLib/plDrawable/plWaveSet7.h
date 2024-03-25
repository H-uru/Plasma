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

#ifndef plWaveSet7_inc
#define plWaveSet7_inc

#include <vector>

#include "hsGeometry3.h"
#include "pnEncryption/plRandom.h"
#include "hsBounds.h"
#include "plStatusLog/plStatusLog.h"

#include "plFixedWaterState7.h"

#include "plWaveSetBase.h"

class hsStream;
class hsResMgr;
class plAccessVtxSpan;
class plMessage;
class hsGMaterial;
class plDrawableSpans;
class plRenderMsg;
class plArmatureUpdateMsg;
class plGenRefMsg;
class plAuxSpan;
class plDynaDecalMgr;
class plGBufferGroup;
class plBitmap;
class plMipmap;
class plLayer;
class plRenderRequest;
class plRenderRequestMsg;
class plRenderTarget;
class plShader;
class plPipeline;
class plRipVSConsts;
class plStatusLog;
class plGraphPlate;

class plWorldWaveData7
{
public:
    hsPoint3        fDir;
    float        fLength;

    float        fFreq;
    float        fPhase;
    float        fAmplitude;
};

class plWorldWave7 : public plWorldWaveData7
{
public:

    inline void Accumulate(hsPoint3& accumPos, hsVector3& accumNorm) const;
};


class plWaveSet7 : public plWaveSetBase
{
public:
    // Props inc by 1 (bit shift in bitvector).
    enum plDrawProperties {
        kDisable                = 0,

        kNumProps               // last in the list
    };
    // Flags, also in a bitvector, so far unused in the multimodifier
    enum {
        kHasRefObject           = 16,
        kHasBuoys               = 17
    };
    enum {
        kNumWaves       = 4
    };
    enum {
        kRefDynaDecalMgr,
        kRefBuoy,
        kRefBumpMat,
        kRefBumpDraw,
        kRefBumpVShader,
        kRefBumpPShader,
        kRefBiasVShader,
        kRefBiasPShader,
        kRefRipVShader,
        kRefRipPShader,
        kRefShoreVShader,
        kRefShorePShader,
        kRefFixedVShader,
        kRefFixedPShader,
        kRefGraphShoreTex,
        kRefBubbleShoreTex,
        kRefEdgeShoreTex,
        kRefGraphShoreMat,
        kRefGraphShoreDraw,
        kRefGraphVShader,
        kRefGraphPShader,
        kRefGraphShoreRT,
        kRefShore,
        kRefDecal,
        kRefDecVShader,
        kRefDecPShader,
        kRefEnvMap,
        kRefCosineLUT,
        kRefRefObj
    };

    enum {
        kGraphShorePasses = 3
    };

    enum {
        kNumWindDep = 5
    };
    enum {
        kNumTexWaves = 16
    };
    enum {
        kBumpPerPass = 4
    };
    enum {
        kNumBumpShaders = kNumTexWaves / kBumpPerPass
    };
    enum {
        kCompositeSize = 256
    };

    enum {
        kUpdateWaveKs       = 0x1,
        kRemakeBubble       = 0x2,
        kRemakeEdge         = 0x4,
        kReRenderEnvMap     = 0x8,
        kReInitWaves        = 0x10
    };

protected:

    double fCurrTime;
    double fLastTime;

    plStatusLog*    fStatusLog;
    plGraphPlate*   fStatusGraph;

    uint32_t          fTrialUpdate;

    plFixedWaterState7      fState;

    float        fScrunchLen;
    float        fScrunchScale;

    hsVector3       fWindDir;

    float        fMinLen;
    float        fMaxLen;
    float        fFreqScale;

    float        fTransCountDown;
    int             fTransistor;
    float        fTransDel;
    
    float        fTexTransCountDown;
    int             fTexTrans;
    float        fTexTransDel;
    float        fTexWaveFade[kNumTexWaves];

    plWorldWave7    fWorldWaves[kNumWaves];
    float        fFreqMod[kNumWaves];

    plRandom        fRand;

    plKey               fSceneNode;

    std::vector<plDynaDecalMgr*>    fDecalMgrs;

    std::vector<plSceneObject*>     fBuoys;
    std::vector<plSceneObject*>     fShores;
    std::vector<plSceneObject*>     fDecals;
    plSceneObject*                  fRefObj;

    std::vector<hsBounds3Ext>       fTargBnds;

    plLayer*                        fBiasLayer[2];
    plLayer*                        fBumpLayers[kNumTexWaves];
    hsGMaterial*                    fBumpMat;
    plDrawableSpans*                fBumpDraw;
    plRenderRequest*                fBumpReq;
    plRenderRequestMsg*             fBumpReqMsg;
    plMipmap*                       fCosineLUT;

    plShader*                       fBumpVShader[kNumBumpShaders];
    plShader*                       fBumpPShader[kNumBumpShaders];

    plShader*                       fBiasVShader;
    plShader*                       fBiasPShader;

    plBitmap*                       fEnvMap;
    uint32_t                          fEnvSize;
    float                        fEnvRefresh;

    plLayer*                        fFixedLayers[4];

    plShader*                       fRipVShader;
    plShader*                       fRipPShader;

    plShader*                       fShoreVShader;
    plShader*                       fShorePShader;

    plShader*                       fFixedVShader;
    plShader*                       fFixedPShader;

    enum DecalVType {
        kDecalV1Lay,
        kDecalV2Lay11,
        kDecalV2Lay12,
        kDecalVEnv,
        
        kNumDecalVShaders
    };
    enum DecalPType {
        kDecalPBB,
        kDecalPaB,
        kDecalPaM,
        kDecalPaA,
        kDecalPAB,
        kDecalPAM,
        kDecalPAA,
        kDecalPMB,
        kDecalPMM,
        kDecalPMA,
        kDecalPEnv,

        kNumDecalPShaders
    };
    plShader*                       fDecalVShaders[kNumDecalVShaders];
    plShader*                       fDecalPShaders[kNumDecalPShaders];

    // Graph shore stuff
    plMipmap*                       fGraphShoreTex;
    plMipmap*                       fBubbleShoreTex;
    plMipmap*                       fEdgeShoreTex;

    hsGMaterial*                    fGraphShoreMat[kGraphShorePasses];
    plDrawableSpans*                fGraphShoreDraw[kGraphShorePasses];
    plRenderTarget*                 fGraphShoreRT[kGraphShorePasses];

    plRenderRequest*                fGraphReq[kGraphShorePasses];
    plRenderRequestMsg*             fGraphReqMsg[kGraphShorePasses];

    plShader*                       fGraphVShader[kGraphShorePasses];
    plShader*                       fGraphPShader[kGraphShorePasses];

    class GraphState
    {
    public:
        float       fAge;
        float       fInvLife;
        float       fUOff;

        float       fFreq[4];
        float       fPhase[4];
        float       fAmp[4];
    };

    GraphState                      fGraphState[kGraphShorePasses];

    class TexWaveDesc
    {
    public:
        float    fPhase;
        float    fAmp;
        float    fLen;
        float    fFreq;
        float    fDirX;
        float    fDirY;
        float    fRotScale00;
        float    fRotScale01;
    };
    TexWaveDesc     fTexWaves[kNumTexWaves];

    class TexWaveWindDep
    {
    public:
        float        fWindSpeed;

        float        fHeight;
        float        fSpecular;
    };

    TexWaveWindDep  fWindDeps[kNumWindDep];

    void            IInitWaveConsts();
    void            IInitState();

    inline void     IScrunch(hsPoint3& pos, hsVector3& norm) const;

    void            ICalcWindow(float dt);
    void            ICalcScale();
    void            IUpdateWaves(float dt);
    void            IUpdateWave(float dt, int i);
    bool            IAnyBoundsVisible(plPipeline* pipe) const;

    void            IInitWave(int i);
    void            IReInitWaves();

    void            IUpdateRefObject();
    void            IUpdateWindDir(float dt);

    void            IShiftCenter(plSceneObject* so) const;
    void            IFloatBuoys(float dt);
    void            IFloatBuoy(float dt, plSceneObject* so);

    // Bookkeeping
    void    IAddTarget(const plKey& key);
    void    IRemoveTarget(const plKey& key);

    void    ISetWindSpeed(float s);

    bool        IOnReceive(plGenRefMsg* refMsg);
    bool        IOnRemove(plGenRefMsg* refMsg);

    bool                ITransContinue(float dt);
    void                IStartTransition(float dt);
    float            ITransitionDelay() const;
    void                ITransition(float dt);

    bool                ITransTexContinue(float dt);
    void                IStartTexTransition(float dt);
    void                ITransTex(float dt);

    void                IInitTexWave(int i);
    void                ISetupTextureWaves();

    void                IUpdateLayers(float dt);
    void                IUpdateBumpLayers(float dt);
    
    plRenderRequest*    ICreateRenderRequest(plRenderTarget* rt, plDrawableSpans* draw, float pri);
    void                ISubmitRenderRequests();

    plRenderTarget*     ICreateTransferRenderTarget(const char* name, int size);
    plDrawableSpans*    ICreateClearDrawable(plDrawableSpans* drawable, hsGMaterial* mat);
    
    void                IAddBumpBiasShaders(plLayer* layer);
    plMipmap*           ICreateBiasNoiseMap();
    void                IAddBumpBiasLayer(hsGMaterial* mat);

    plMipmap*           ICreateBumpBitmapFFP(float amp, float dx, float dy) const;
    hsGMaterial*        ICreateBumpLayersFFP();
    plMipmap*           ICreateBumpMipmapPS();
    plLayer*            ICreateBumpLayerPS(plMipmap* mipMap, hsGMaterial* bumpMat, int which);
    hsGMaterial*        ICreateBumpLayersPS();
    plDrawableSpans*    ICreateBumpDrawable();

    plLayer*            ICreateTotalEnvLayer(plBitmap* envMap, hsGMaterial* mat, int which, const char* pref);
    plLayer*            ICreateTotalLayer(plBitmap* bm, hsGMaterial* mat, int which, const char* suff);

    hsGMaterial*        ICreateFixedMatPS(hsGMaterial* mat, const int numUVWs);
    void                ICreateFixedMat(hsGMaterial* mat, const int numUVWs);
    void                ICheckTargetMaterials();

    plDrawableSpans*    ICreateGraphDrawable(plDrawableSpans* drawable, hsGMaterial* mat, int nWid);
    plDrawableSpans*    ICreateEmptyGraphDrawable(const char* name, uint32_t ref, int wich);
    hsGMaterial*        ICreateEmptyMaterial(const char* name, uint32_t ref, int which);
    plLayer*            ICreateBlankLayer(const char* name, int suff);
    plMipmap*           ICreateBlankTex(const char* name, int width, int height, uint32_t ref);
    plMipmap*           ICreateGraphShoreTex(int width, int height);
    plMipmap*           ICreateBubbleShoreTex(int width, int height);
    void                IRefillBubbleShoreTex();
    plMipmap*           ICreateEdgeShoreTex(int width, int height);
    void                IRefillEdgeShoreTex();
    void                ISetAsTexture(plLayer* lay, plBitmap* tex);
    void                ICreateGraphShoreLayer(hsGMaterial* mat, int iPass);
    void                ICreateGraphBubbleLayer(hsGMaterial* mat, int iPass);
    void                ICreateGraphEdgeLayer(hsGMaterial* mat, int iPass);
    void                ICreateGraphShoreMaterials();
    plRenderTarget*     ISetupGraphShoreRenderReq(int which);
    void                IMakeShoreLayer(hsGMaterial* mat, int which);
    void                ISetupShoreLayers(hsGMaterial* mat);
    void                ISetupGraphShore(hsGMaterial* mat);
    void                ICheckShoreMaterial(plSceneObject* so);
    void                ICheckShoreMaterials();
    void                ICheckDecalMaterial(plSceneObject* so);
    void                ICheckDecalMaterials();
    void                ISetupDecal(hsGMaterial* mat);
    void                ICheckDecalEnvLayers(hsGMaterial* mat);

    void                IAddGraphPShader(hsGMaterial* mat, size_t iPass);
    void                IAddGraphVShader(hsGMaterial* mat, size_t iPass);
    void                IUpdateGraphShader(float dt, size_t iPass);
    void                IInitGraph(size_t iPass);
    void                IShuffleDownGraphs(size_t iPass);

    // type is either plLayRefMsg::kVertexShader or plLayRefMsg::kPixelShader.
    void                IAddShaderToLayers(hsGMaterial* mat, int iFirst, int iLast, uint8_t type, plShader* shader);

    void                IAddBumpPixelShader(hsGMaterial* mat, int iShader, int iFirst, int iLast);
    void                IAddBumpVertexShader(hsGMaterial* mat, int iShader, int iFirst, int iLast);

    void                IAddRipVertexShader(hsGMaterial* mat, const plRipVSConsts& ripConsts);
    void                IAddRipPixelShader(hsGMaterial* mat, const plRipVSConsts& ripConsts);

    void                IAddShoreVertexShader(hsGMaterial* mat);
    void                IAddShorePixelShader(hsGMaterial* mat);

    void                IAddFixedVertexShader(hsGMaterial* mat, const int numUVWs);
    void                IAddFixedPixelShader(hsGMaterial* mat);

    plShader*           IGetDecalPShader(hsGMaterial* mat);
    plShader*           ICreateDecalPShader(DecalPType t);
    plShader*           IGetDecalVShader(hsGMaterial* mat);
    plShader*           ICreateDecalVShader(DecalVType t);

    void                IUpdateShaders(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);

    void                IUpdateBiasVShader();
    void                IUpdateBumpPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateBumpVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateRipPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateRipVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateShoreVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateFixedVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateFixedPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
    void                IUpdateGraphShaders(plPipeline* pipe, float dt);
    void                IUpdateDecVShader(int t, plPipeline* pipe);
    void                IUpdateDecVShaders(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);

    int IShoreRef() const override { return kRefShore; }
    int IDecalRef() const override { return kRefDecal; }

    template<typename... _Args>
    void Log(const char *format, _Args&&... args) const
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (fStatusLog)
            fStatusLog->AddLineF(format, std::forward<_Args>(args)...);
#endif
    }

    template<typename... _Args>
    void Log(uint32_t color, const char *format, _Args&&... args) const
    {
#ifndef PLASMA_EXTERNAL_RELEASE
        if (fStatusLog)
            fStatusLog->AddLineF(color, format, std::forward<_Args>(args)...);
#endif
    }

    inline void IRestartLog() const;
    inline void GraphLen(float len) const;
    inline void IRestartGraph() const;

public:
    plWaveSet7();
    virtual ~plWaveSet7();

    CLASSNAME_REGISTER( plWaveSet7 );
    GETINTERFACE_ANY( plWaveSet7, plWaveSetBase );

    bool MsgReceive(plMessage* msg) override;

    bool IEval(double secs, float del, uint32_t dirty) override { return false; }

    int32_t       GetNumProperties() const { return kNumProps; }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    float            EvalPoint(hsPoint3& pos, hsVector3& norm);

    // Getters and Setters for Python twiddling
    //
    // First a way to set new values. The secs parameter says how long to take
    // blending to the new value from the current value.
    //
    // Geometric wave parameters. These are all safe to twiddle at any time or speed.
    // The new settings take effect as new waves are spawned.
    void SetGeoMaxLength(float s, float secs=0) { fState.fGeoState.fMaxLength.Set(s, secs); }
    void SetGeoMinLength(float s, float secs=0) { fState.fGeoState.fMinLength.Set(s, secs); }
    void SetGeoAmpOverLen(float s, float secs=0) { fState.fGeoState.fAmpOverLen.Set(s, secs); }
    void SetGeoChop(float s, float secs=0) { fState.fGeoState.fChop.Set(s, secs); }
    void SetGeoAngleDev(float s, float secs=0) { fState.fGeoState.fAngleDev.Set(s, secs); }

    // Texture wave parameters. Safe to twiddle any time or speed.
    // The new settings take effect as new waves are spawned.
    void SetTexMaxLength(float s, float secs=0) { fState.fTexState.fMaxLength.Set(s, secs); }
    void SetTexMinLength(float s, float secs=0) { fState.fTexState.fMinLength.Set(s, secs); }
    void SetTexAmpOverLen(float s, float secs=0) { fState.fTexState.fAmpOverLen.Set(s, secs); }
    void SetTexChop(float s, float secs=0) { fState.fTexState.fChop.Set(s, secs); }
    void SetTexAngleDev(float s, float secs=0) { fState.fTexState.fAngleDev.Set(s, secs); }

    // The size in feet of one tile of the ripple texture. If you change this (I don't 
    // recommend it), you need to change it very slowly or it will look very stupid.
    void SetRippleScale(float s, float secs=0) { fState.fRippleScale.Set(s, secs); }

    // The direction the wind is blowing (waves will be more or less perpindicular to wind dir).
    // Change somewhat slowly, like over 30 seconds.
    void SetWindDir(const hsVector3& s, float secs=0) { fState.fWindDir.Set(s, secs); }

    // Change these gently, effect is immediate.
    void SetSpecularNoise(float s, float secs=0) { hsVector3 spec = fState.fSpecVec; spec[plFixedWaterState7::kNoise] = s; fState.fSpecVec.Set(spec, secs); }
    void SetSpecularStart(float s, float secs=0) { hsVector3 spec = fState.fSpecVec; spec[plFixedWaterState7::kSpecStart] = s; fState.fSpecVec.Set(spec, secs); }
    void SetSpecularEnd(float s, float secs=0) { hsVector3 spec = fState.fSpecVec; spec[plFixedWaterState7::kSpecEnd] = s; fState.fSpecVec.Set(spec, secs); }

    // Water Height is overriden if the ref object is animated.
    void SetWaterHeight(float s, float secs=0) { fState.fWaterHeight.Set(s, secs); }

    // Water Offset and DepthFalloff are complicated, and not immediately interesting to animate.
    void SetWaterOffset(const hsVector3& s, float secs=0) { fState.fWaterOffset.Set(s, secs); }
        void SetOpacOffset(float s, float secs=0) { hsVector3 off = fState.fWaterOffset; off.fX = s; fState.fWaterOffset.Set(off, secs); }
        void SetReflOffset(float s, float secs=0) { hsVector3 off = fState.fWaterOffset; off.fY = s; fState.fWaterOffset.Set(off, secs); }
        void SetWaveOffset(float s, float secs=0) { hsVector3 off = fState.fWaterOffset; off.fZ = s; fState.fWaterOffset.Set(off, secs); }
    void SetDepthFalloff(const hsVector3& s, float secs=0) { fState.fDepthFalloff.Set(s, secs); }
        void SetOpacFalloff(float s, float secs=0) { hsVector3 off = fState.fDepthFalloff; off.fX = s; fState.fDepthFalloff.Set(off, secs); }
        void SetReflFalloff(float s, float secs=0) { hsVector3 off = fState.fDepthFalloff; off.fY = s; fState.fDepthFalloff.Set(off, secs); }
        void SetWaveFalloff(float s, float secs=0) { hsVector3 off = fState.fDepthFalloff; off.fZ = s; fState.fDepthFalloff.Set(off, secs); }

    // Max and Min Atten aren't very interesting, and will probably go away.
    void SetMaxAtten(const hsVector3& s, float secs=0) { fState.fMaxAtten.Set(s, secs); }
    void SetMinAtten(const hsVector3& s, float secs=0) { fState.fMinAtten.Set(s, secs); }

    // Skipping the shore parameters, because they are never used.

    // Water colors, adjust slowly, effect is immediate.
    void SetWaterTint(const hsColorRGBA& s, float secs=0) { fState.fWaterTint.Set(s, secs); }
        void SetWaterRGB(const hsVector3& col, float secs=0) { hsColorRGBA rgb; rgb.Set(col.fX, col.fY, col.fZ, GetWaterOpacity()); SetWaterTint(rgb, secs); }
        void SetWaterOpacity(float s, float secs=0) { hsColorRGBA col = GetWaterTint(); col.a = s; SetWaterTint(col, secs); }
    void SetSpecularTint(const hsColorRGBA& s, float secs=0) { fState.fSpecularTint.Set(s, secs); }
        void SetSpecularRGB(const hsVector3& col, float secs=0) { hsColorRGBA rgb; rgb.Set(col.fX, col.fY, col.fZ, GetSpecularMute()); SetSpecularTint(rgb, secs); }
        void SetSpecularMute(float s, float secs=0) { hsColorRGBA col = GetSpecularTint(); col.a = s; SetSpecularTint(col, secs); }

    // The environment map is essentially projected onto a sphere. Moving the center of
    // the sphere north will move the reflections north, changing the radius of the
    // sphere effects parallax in the obvious way.
    void SetEnvCenter(const hsPoint3& s, float secs=0) { fState.fEnvCenter.Set(s, secs); }
    void SetEnvRadius(float s, float secs=0) { fState.fEnvRadius.Set(s, secs); }

    // Now a way to get current values. See the accompanying Setter for notes on
    // what the parameter means.
    //
    float GetGeoMaxLength() const { return fState.fGeoState.fMaxLength; }
    float GetGeoMinLength() const { return fState.fGeoState.fMinLength; }
    float GetGeoAmpOverLen() const { return fState.fGeoState.fAmpOverLen; }
    float GetGeoChop() const { return fState.fGeoState.fChop; }
    float GetGeoAngleDev() const { return fState.fGeoState.fAngleDev; }

    float GetTexMaxLength() const { return fState.fTexState.fMaxLength; }
    float GetTexMinLength() const { return fState.fTexState.fMinLength; }
    float GetTexAmpOverLen() const { return fState.fTexState.fAmpOverLen; }
    float GetTexChop() const { return fState.fTexState.fChop; }
    float GetTexAngleDev() const { return fState.fTexState.fAngleDev; }

    float GetRippleScale() const { return fState.fRippleScale; }

    hsVector3 GetWindDir() const override { return fState.fWindDir; }

    float GetSpecularNoise() const { hsVector3 spec = fState.fSpecVec; return spec[plFixedWaterState7::kNoise]; }
    float GetSpecularStart() const { hsVector3 spec = fState.fSpecVec; return spec[plFixedWaterState7::kSpecStart]; }
    float GetSpecularEnd() const { hsVector3 spec = fState.fSpecVec; return spec[plFixedWaterState7::kSpecEnd]; }

    float GetWaterHeight() const { return fState.fWaterHeight; }

    hsVector3 GetWaterOffset() const { return fState.fWaterOffset; }
        float GetOpacOffset() const { hsVector3 off = fState.fWaterOffset; return off.fX; }
        float GetReflOffset() const { hsVector3 off = fState.fWaterOffset; return off.fY; }
        float GetWaveOffset() const { hsVector3 off = fState.fWaterOffset; return off.fZ; }
    hsVector3 GetDepthFalloff() const { return fState.fDepthFalloff; }
        float GetOpacFalloff() const { hsVector3 off = fState.fDepthFalloff; return off.fX; }
        float GetReflFalloff() const { hsVector3 off = fState.fDepthFalloff; return off.fY; }
        float GetWaveFalloff() const { hsVector3 off = fState.fDepthFalloff; return off.fZ; }

    hsVector3 GetMaxAtten() const { return fState.fMaxAtten; }
    hsVector3 GetMinAtten() const { return fState.fMinAtten; }

    hsColorRGBA GetWaterTint() const { return fState.fWaterTint; }
        hsVector3 GetWaterRGB() const { hsColorRGBA col = GetWaterTint(); return hsVector3(col.r, col.g, col.b); }
        float GetWaterOpacity() const { return GetWaterTint().a; }
    hsColorRGBA GetSpecularTint() const { return fState.fSpecularTint; }
        hsVector3 GetSpecularRGB() const { hsColorRGBA col = GetSpecularTint(); return hsVector3(col.r, col.g, col.b); }
        float GetSpecularMute() const { return GetSpecularTint().a; }

    hsPoint3 GetEnvCenter() const { return fState.fEnvCenter; }
    float GetEnvRadius() const { return fState.fEnvRadius; }

    // Export/debugging functions. For runtime, use message interface (plGenRefMsg, plWaveMsg).
    void        AddTarget(const plKey& key);
    void        RemoveTarget(const plKey& key);
    void        AddShoreTest(plKey& key);

    void        SetRefObject(plSceneObject* refObj);

    void            SetSceneNode(plKey key);
    plKey           GetSceneNode() const { return fSceneNode; }

    void            AddDynaDecalMgr(const plKey& key);
    void            RemoveDynaDecalMgr(const plKey& key);

    void            AddBuoy(const plKey& soKey);
    void            RemoveBuoy(const plKey& soKey);

    bool            SetupRippleMat(hsGMaterial* mat, const plRipVSConsts& ripConsts) override;

    float        GetHeight() const override { return State().fWaterHeight; }

    const plFixedWaterState7::WaveState& GeoState() const { return State().fGeoState; }
    const plFixedWaterState7::WaveState& TexState() const { return State().fTexState; }
    const plFixedWaterState7& State() const { return fState; }
    void            SetState(const plFixedWaterState7& state, float dur);

    void            SetEnvSize(uint32_t s) { fEnvSize = s; }
    uint32_t          GetEnvSize() const { return fEnvSize; }

    void StopLog();
    void StartLog();
    bool Logging() const { return fStatusLog != nullptr; }
    void StartGraph();
    void StopGraph();
    bool Graphing() const { return fStatusGraph != nullptr; }
};

#endif // plWaveSet7_inc
