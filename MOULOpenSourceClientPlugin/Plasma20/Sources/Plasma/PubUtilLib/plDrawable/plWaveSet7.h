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

#ifndef plWaveSet7_inc
#define plWaveSet7_inc


#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "../plMath/plRandom.h"
#include "hsBounds.h"

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
	hsPoint3		fDir;
	hsScalar		fLength;

	hsScalar		fFreq;
	hsScalar		fPhase;
	hsScalar		fAmplitude;
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
		kDisable				= 0,

		kNumProps				// last in the list
	};
	// Flags, also in a bitvector, so far unused in the multimodifier
	enum {
		kHasRefObject			= 16
	};
	enum {
		kNumWaves		= 4
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
		kUpdateWaveKs		= 0x1,
		kRemakeBubble		= 0x2,
		kRemakeEdge			= 0x4,
		kReRenderEnvMap		= 0x8,
		kReInitWaves		= 0x10
	};

protected:

	double fCurrTime;
	double fLastTime;

	plStatusLog*	fStatusLog;
	plGraphPlate*	fStatusGraph;

	UInt32			fTrialUpdate;

	plFixedWaterState7		fState;

	hsScalar		fScrunchLen;
	hsScalar		fScrunchScale;

	hsVector3		fWindDir;

	hsScalar		fMinLen;
	hsScalar		fMaxLen;
	hsScalar		fFreqScale;

	hsScalar		fTransCountDown;
	int				fTransistor;
	hsScalar		fTransDel;
	
	hsScalar		fTexTransCountDown;
	int				fTexTrans;
	hsScalar		fTexTransDel;
	hsScalar		fTexWaveFade[kNumTexWaves];

	plWorldWave7	fWorldWaves[kNumWaves];
	hsScalar		fFreqMod[kNumWaves];

	plRandom		fRand;

	plKey				fSceneNode;

	hsTArray<plDynaDecalMgr*>		fDecalMgrs;

	hsTArray<plSceneObject*>		fBuoys; 
	hsTArray<plSceneObject*>		fShores;
	hsTArray<plSceneObject*>		fDecals;
	plSceneObject*					fRefObj;

	hsTArray<hsBounds3Ext>			fTargBnds;

	plLayer*						fBiasLayer[2];
	plLayer*						fBumpLayers[kNumTexWaves];
	hsGMaterial*					fBumpMat;
	plDrawableSpans*				fBumpDraw;
	plRenderRequest*				fBumpReq;
	plRenderRequestMsg*				fBumpReqMsg;
	plMipmap*						fCosineLUT;

	plShader*						fBumpVShader[kNumBumpShaders];
	plShader*						fBumpPShader[kNumBumpShaders];

	plShader*						fBiasVShader;
	plShader*						fBiasPShader;

	plBitmap*						fEnvMap;
	UInt32							fEnvSize;
	hsScalar						fEnvRefresh;

	plLayer*						fFixedLayers[4];

	plShader*						fRipVShader;
	plShader*						fRipPShader;

	plShader*						fShoreVShader;
	plShader*						fShorePShader;

	plShader*						fFixedVShader;
	plShader*						fFixedPShader;

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
	plShader*						fDecalVShaders[kNumDecalVShaders];
	plShader*						fDecalPShaders[kNumDecalPShaders];

	// Graph shore stuff
	plMipmap*						fGraphShoreTex;
	plMipmap*						fBubbleShoreTex;
	plMipmap*						fEdgeShoreTex;

	hsGMaterial*					fGraphShoreMat[kGraphShorePasses];
	plDrawableSpans*				fGraphShoreDraw[kGraphShorePasses];
	plRenderTarget*					fGraphShoreRT[kGraphShorePasses];

	plRenderRequest*				fGraphReq[kGraphShorePasses];
	plRenderRequestMsg*				fGraphReqMsg[kGraphShorePasses];

	plShader*						fGraphVShader[kGraphShorePasses];
	plShader*						fGraphPShader[kGraphShorePasses];

	class GraphState
	{
	public:
		float		fAge;
		float		fInvLife;
		float		fUOff;

		float		fFreq[4];
		float		fPhase[4];
		float		fAmp[4];
	};

	GraphState						fGraphState[kGraphShorePasses];

	class WaveK
	{
	public:
		// fK is the number of times the sine wave repeats across the texture. Must be an integer
		// fS/fK is the base X component of the direction of the wave, with Y = 1.f - X. Note that X^2 + Y^2 != 1.
		// fD allows the wave to get more off the Y direction 
		// So the X component will be Int(fS + fD*dispersion) / fK, because it must be an integer ratio to
		// preserve tiling. Also, (fS + fD) must be <= fK (for the Y normalization).
		// See the notes.
		float	fS;
		float	fK;
		float	fD;
	};

	WaveK			fWaveKs[kNumTexWaves];

	class TexWaveDesc
	{
	public:
		hsScalar	fPhase;
		hsScalar	fAmp;
		hsScalar	fLen;
		hsScalar	fFreq;
		hsScalar	fDirX;
		hsScalar	fDirY;
		hsScalar	fRotScale00;
		hsScalar	fRotScale01;
	};
	TexWaveDesc		fTexWaves[kNumTexWaves];

	class TexWaveWindDep
	{
	public:
		hsScalar		fWindSpeed;

		hsScalar		fHeight;
		hsScalar		fSpecular;
	};

	TexWaveWindDep	fWindDeps[kNumWindDep];

	void			IInitWaveConsts();
	void			IInitState();

	inline void		IScrunch(hsPoint3& pos, hsVector3& norm) const;

	void			ICalcWindow(hsScalar dt);
	void			ICalcScale();
	void			IUpdateWaves(hsScalar dt);
	void			IUpdateWave(hsScalar dt, int i);
	hsBool			IAnyBoundsVisible(plPipeline* pipe) const;

	void			IInitWave(int i);
	void			IReInitWaves();

	void			IUpdateRefObject();
	void			IUpdateWindDir(hsScalar dt);

	void			IShiftCenter(plSceneObject* so) const;
	void			IFloatBuoys(hsScalar dt);
	void			IFloatBuoy(hsScalar dt, plSceneObject* so);

	// Bookkeeping
	void	IAddTarget(const plKey& key);
	void	IRemoveTarget(const plKey& key);

	void	ISetWindSpeed(hsScalar s);

	hsBool		IOnReceive(plGenRefMsg* refMsg);
	hsBool		IOnRemove(plGenRefMsg* refMsg);

	hsBool				ITransContinue(hsScalar dt);
	void				IStartTransition(hsScalar dt);
	hsScalar			ITransitionDelay() const;
	void				ITransition(hsScalar dt);

	hsBool				ITransTexContinue(hsScalar dt);
	void				IStartTexTransition(hsScalar dt);
	void				ITransTex(hsScalar dt);

	void				IInitTexWave(int i);
	void				ISetupTextureWaves();

	void				IUpdateLayers(hsScalar dt);
	void				IUpdateBumpLayers(hsScalar dt);
	
	plRenderRequest*	ICreateRenderRequest(plRenderTarget* rt, plDrawableSpans* draw, hsScalar pri);
	void				ISubmitRenderRequests();

	plRenderTarget*		ICreateTransferRenderTarget(const char* name, int size);
	plDrawableSpans*	ICreateClearDrawable(plDrawableSpans* drawable, hsGMaterial* mat);
	
	void				IAddBumpBiasShaders(plLayer* layer);
	plMipmap*			ICreateBiasNoiseMap();
	void				IAddBumpBiasLayer(hsGMaterial* mat);

	plMipmap*			ICreateBumpBitmapFFP(hsScalar amp, hsScalar dx, hsScalar dy) const;
	hsGMaterial*		ICreateBumpLayersFFP();
	plMipmap*			ICreateBumpMipmapPS();
	plLayer*			ICreateBumpLayerPS(plMipmap* mipMap, hsGMaterial* bumpMat, int which);
	hsGMaterial*		ICreateBumpLayersPS();
	plDrawableSpans*	ICreateBumpDrawable();

	plLayer*			ICreateTotalEnvLayer(plBitmap* envMap, hsGMaterial* mat, int which, const char* pref);
	plLayer*			ICreateTotalLayer(plBitmap* bm, hsGMaterial* mat, int which, const char* suff);

	hsGMaterial*		ICreateFixedMatPS(hsGMaterial* mat, const int numUVWs);
	void				ICreateFixedMat(hsGMaterial* mat, const int numUVWs);
	void				ICheckTargetMaterials();

	plDrawableSpans*	ICreateGraphDrawable(plDrawableSpans* drawable, hsGMaterial* mat, int nWid);
	plDrawableSpans*	ICreateEmptyGraphDrawable(const char* name, UInt32 ref, int wich);
	hsGMaterial*		ICreateEmptyMaterial(const char* name, UInt32 ref, int which);
	plLayer*			ICreateBlankLayer(const char* name, int suff);
	plMipmap*			ICreateBlankTex(const char* name, int width, int height, UInt32 ref);
	plMipmap*			ICreateGraphShoreTex(int width, int height);
	plMipmap*			ICreateBubbleShoreTex(int width, int height);
	void				IRefillBubbleShoreTex();
	plMipmap*			ICreateEdgeShoreTex(int width, int height);
	void				IRefillEdgeShoreTex();
	void				ISetAsTexture(plLayer* lay, plBitmap* tex);
	void				ICreateGraphShoreLayer(hsGMaterial* mat, int iPass);
	void				ICreateGraphBubbleLayer(hsGMaterial* mat, int iPass);
	void				ICreateGraphEdgeLayer(hsGMaterial* mat, int iPass);
	void				ICreateGraphShoreMaterials();
	plRenderTarget*		ISetupGraphShoreRenderReq(int which);
	void				IMakeShoreLayer(hsGMaterial* mat, int which);
	void				ISetupShoreLayers(hsGMaterial* mat);
	void				ISetupGraphShore(hsGMaterial* mat);
	void				ICheckShoreMaterial(plSceneObject* so);
	void				ICheckShoreMaterials();
	void				ICheckDecalMaterial(plSceneObject* so);
	void				ICheckDecalMaterials();
	void				ISetupDecal(hsGMaterial* mat);
	void				ICheckDecalEnvLayers(hsGMaterial* mat);

	void				IAddGraphPShader(hsGMaterial* mat, int iPass);
	void				IAddGraphVShader(hsGMaterial* mat, int iPass);
	void				IUpdateGraphShader(hsScalar dt, int iPass);
	void				IInitGraph(int iPass);
	void				IShuffleDownGraphs(int iPass);

	// type is either plLayRefMsg::kVertexShader or plLayRefMsg::kPixelShader.
	void				IAddShaderToLayers(hsGMaterial* mat, int iFirst, int iLast, UInt8 type, plShader* shader);

	void				IAddBumpPixelShader(hsGMaterial* mat, int iShader, int iFirst, int iLast);
	void				IAddBumpVertexShader(hsGMaterial* mat, int iShader, int iFirst, int iLast);

	void				IAddRipVertexShader(hsGMaterial* mat, const plRipVSConsts& ripConsts);
	void				IAddRipPixelShader(hsGMaterial* mat, const plRipVSConsts& ripConsts);

	void				IAddShoreVertexShader(hsGMaterial* mat);
	void				IAddShorePixelShader(hsGMaterial* mat);

	void				IAddFixedVertexShader(hsGMaterial* mat, const int numUVWs);
	void				IAddFixedPixelShader(hsGMaterial* mat);

	plShader*			IGetDecalPShader(hsGMaterial* mat);
	plShader*			ICreateDecalPShader(DecalPType t);
	plShader*			IGetDecalVShader(hsGMaterial* mat);
	plShader*			ICreateDecalVShader(DecalVType t);

	void				IUpdateShaders(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);

	void				IUpdateBiasVShader();
	void				IUpdateBumpPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateBumpVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateRipPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateRipVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateShoreVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateFixedVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateFixedPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);
	void				IUpdateGraphShaders(plPipeline* pipe, hsScalar dt);
	void				IUpdateDecVShader(int t, plPipeline* pipe);
	void				IUpdateDecVShaders(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual int IShoreRef() const { return kRefShore; }
	virtual int IDecalRef() const { return kRefDecal; }

	inline void LogF(const char *format, ...) const;
	inline void LogF(UInt32 color, const char *format, ...) const;
	inline void IRestartLog() const;
	inline void GraphLen(hsScalar len) const;
	inline void IRestartGraph() const;

public:
	plWaveSet7();
	virtual ~plWaveSet7();

	CLASSNAME_REGISTER( plWaveSet7 );
	GETINTERFACE_ANY( plWaveSet7, plWaveSetBase );

	virtual hsBool MsgReceive(plMessage* msg);

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

	Int32		GetNumProperties() const { return kNumProps; }

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	hsScalar			EvalPoint(hsPoint3& pos, hsVector3& norm);

	// Getters and Setters for Python twiddling
	//
	// First a way to set new values. The secs parameter says how long to take
	// blending to the new value from the current value.
	//
	// Geometric wave parameters. These are all safe to twiddle at any time or speed.
	// The new settings take effect as new waves are spawned.
	void SetGeoMaxLength(hsScalar s, hsScalar secs=0) { fState.fGeoState.fMaxLength.Set(s, secs); }
	void SetGeoMinLength(hsScalar s, hsScalar secs=0) { fState.fGeoState.fMinLength.Set(s, secs); }
	void SetGeoAmpOverLen(hsScalar s, hsScalar secs=0) { fState.fGeoState.fAmpOverLen.Set(s, secs); }
	void SetGeoChop(hsScalar s, hsScalar secs=0) { fState.fGeoState.fChop.Set(s, secs); }
	void SetGeoAngleDev(hsScalar s, hsScalar secs=0) { fState.fGeoState.fAngleDev.Set(s, secs); }

	// Texture wave parameters. Safe to twiddle any time or speed.
	// The new settings take effect as new waves are spawned.
	void SetTexMaxLength(hsScalar s, hsScalar secs=0) { fState.fTexState.fMaxLength.Set(s, secs); }
	void SetTexMinLength(hsScalar s, hsScalar secs=0) { fState.fTexState.fMinLength.Set(s, secs); }
	void SetTexAmpOverLen(hsScalar s, hsScalar secs=0) { fState.fTexState.fAmpOverLen.Set(s, secs); }
	void SetTexChop(hsScalar s, hsScalar secs=0) { fState.fTexState.fChop.Set(s, secs); }
	void SetTexAngleDev(hsScalar s, hsScalar secs=0) { fState.fTexState.fAngleDev.Set(s, secs); }

	// The size in feet of one tile of the ripple texture. If you change this (I don't 
	// recommend it), you need to change it very slowly or it will look very stupid.
	void SetRippleScale(hsScalar s, hsScalar secs=0) { fState.fRippleScale.Set(s, secs); }

	// The direction the wind is blowing (waves will be more or less perpindicular to wind dir).
	// Change somewhat slowly, like over 30 seconds.
	void SetWindDir(const hsVector3& s, hsScalar secs=0) { fState.fWindDir.Set(s, secs); }

	// Change these gently, effect is immediate.
	void SetSpecularNoise(hsScalar s, hsScalar secs=0) { hsVector3 spec = fState.fSpecVec; spec[plFixedWaterState7::kNoise] = s; fState.fSpecVec.Set(spec, secs); }
	void SetSpecularStart(hsScalar s, hsScalar secs=0) { hsVector3 spec = fState.fSpecVec; spec[plFixedWaterState7::kSpecStart] = s; fState.fSpecVec.Set(spec, secs); }
	void SetSpecularEnd(hsScalar s, hsScalar secs=0) { hsVector3 spec = fState.fSpecVec; spec[plFixedWaterState7::kSpecEnd] = s; fState.fSpecVec.Set(spec, secs); }

	// Water Height is overriden if the ref object is animated.
	void SetWaterHeight(hsScalar s, hsScalar secs=0) { fState.fWaterHeight.Set(s, secs); }

	// Water Offset and DepthFalloff are complicated, and not immediately interesting to animate.
	void SetWaterOffset(const hsVector3& s, hsScalar secs=0) { fState.fWaterOffset.Set(s, secs); }
		void SetOpacOffset(hsScalar s, hsScalar secs=0) { hsVector3 off = fState.fWaterOffset; off.fX = s; fState.fWaterOffset.Set(off, secs); }
		void SetReflOffset(hsScalar s, hsScalar secs=0) { hsVector3 off = fState.fWaterOffset; off.fY = s; fState.fWaterOffset.Set(off, secs); }
		void SetWaveOffset(hsScalar s, hsScalar secs=0) { hsVector3 off = fState.fWaterOffset; off.fZ = s; fState.fWaterOffset.Set(off, secs); }
	void SetDepthFalloff(const hsVector3& s, hsScalar secs=0) { fState.fDepthFalloff.Set(s, secs); }
		void SetOpacFalloff(hsScalar s, hsScalar secs=0) { hsVector3 off = fState.fDepthFalloff; off.fX = s; fState.fDepthFalloff.Set(off, secs); }
		void SetReflFalloff(hsScalar s, hsScalar secs=0) { hsVector3 off = fState.fDepthFalloff; off.fY = s; fState.fDepthFalloff.Set(off, secs); }
		void SetWaveFalloff(hsScalar s, hsScalar secs=0) { hsVector3 off = fState.fDepthFalloff; off.fZ = s; fState.fDepthFalloff.Set(off, secs); }

	// Max and Min Atten aren't very interesting, and will probably go away.
	void SetMaxAtten(const hsVector3& s, hsScalar secs=0) { fState.fMaxAtten.Set(s, secs); }
	void SetMinAtten(const hsVector3& s, hsScalar secs=0) { fState.fMinAtten.Set(s, secs); }

	// Skipping the shore parameters, because they are never used.

	// Water colors, adjust slowly, effect is immediate.
	void SetWaterTint(const hsColorRGBA& s, hsScalar secs=0) { fState.fWaterTint.Set(s, secs); }
		void SetWaterRGB(const hsVector3& col, hsScalar secs=0) { hsColorRGBA rgb; rgb.Set(col.fX, col.fY, col.fZ, GetWaterOpacity()); SetWaterTint(rgb, secs); }
		void SetWaterOpacity(hsScalar s, hsScalar secs=0) { hsColorRGBA col = GetWaterTint(); col.a = s; SetWaterTint(col, secs); }
	void SetSpecularTint(const hsColorRGBA& s, hsScalar secs=0) { fState.fSpecularTint.Set(s, secs); }
		void SetSpecularRGB(const hsVector3& col, hsScalar secs=0) { hsColorRGBA rgb; rgb.Set(col.fX, col.fY, col.fZ, GetSpecularMute()); SetSpecularTint(rgb, secs); }
		void SetSpecularMute(hsScalar s, hsScalar secs=0) { hsColorRGBA col = GetSpecularTint(); col.a = s; SetSpecularTint(col, secs); }

	// The environment map is essentially projected onto a sphere. Moving the center of
	// the sphere north will move the reflections north, changing the radius of the
	// sphere effects parallax in the obvious way.
	void SetEnvCenter(const hsPoint3& s, hsScalar secs=0) { fState.fEnvCenter.Set(s, secs); }
	void SetEnvRadius(hsScalar s, hsScalar secs=0) { fState.fEnvRadius.Set(s, secs); }

	// Now a way to get current values. See the accompanying Setter for notes on
	// what the parameter means.
	//
	hsScalar GetGeoMaxLength() const { return fState.fGeoState.fMaxLength; }
	hsScalar GetGeoMinLength() const { return fState.fGeoState.fMinLength; }
	hsScalar GetGeoAmpOverLen() const { return fState.fGeoState.fAmpOverLen; }
	hsScalar GetGeoChop() const { return fState.fGeoState.fChop; }
	hsScalar GetGeoAngleDev() const { return fState.fGeoState.fAngleDev; }

	hsScalar GetTexMaxLength() const { return fState.fTexState.fMaxLength; }
	hsScalar GetTexMinLength() const { return fState.fTexState.fMinLength; }
	hsScalar GetTexAmpOverLen() const { return fState.fTexState.fAmpOverLen; }
	hsScalar GetTexChop() const { return fState.fTexState.fChop; }
	hsScalar GetTexAngleDev() const { return fState.fTexState.fAngleDev; }

	hsScalar GetRippleScale() const { return fState.fRippleScale; }

	hsVector3 GetWindDir() const { return fState.fWindDir; }

	hsScalar GetSpecularNoise() const { hsVector3 spec = fState.fSpecVec; return spec[plFixedWaterState7::kNoise]; }
	hsScalar GetSpecularStart() const { hsVector3 spec = fState.fSpecVec; return spec[plFixedWaterState7::kSpecStart]; }
	hsScalar GetSpecularEnd() const { hsVector3 spec = fState.fSpecVec; return spec[plFixedWaterState7::kSpecEnd]; }

	hsScalar GetWaterHeight() const { return fState.fWaterHeight; }

	hsVector3 GetWaterOffset() const { return fState.fWaterOffset; }
		hsScalar GetOpacOffset() const { hsVector3 off = fState.fWaterOffset; return off.fX; }
		hsScalar GetReflOffset() const { hsVector3 off = fState.fWaterOffset; return off.fY; }
		hsScalar GetWaveOffset() const { hsVector3 off = fState.fWaterOffset; return off.fZ; }
	hsVector3 GetDepthFalloff() const { return fState.fDepthFalloff; }
		hsScalar GetOpacFalloff() const { hsVector3 off = fState.fDepthFalloff; return off.fX; }
		hsScalar GetReflFalloff() const { hsVector3 off = fState.fDepthFalloff; return off.fY; }
		hsScalar GetWaveFalloff() const { hsVector3 off = fState.fDepthFalloff; return off.fZ; }

	hsVector3 GetMaxAtten() const { return fState.fMaxAtten; }
	hsVector3 GetMinAtten() const { return fState.fMinAtten; }

	hsColorRGBA GetWaterTint() const { return fState.fWaterTint; }
		hsVector3 GetWaterRGB() const { hsColorRGBA col = GetWaterTint(); return hsVector3(col.r, col.g, col.b); }
		hsScalar GetWaterOpacity() const { return GetWaterTint().a; }
	hsColorRGBA GetSpecularTint() const { return fState.fSpecularTint; }
		hsVector3 GetSpecularRGB() const { hsColorRGBA col = GetSpecularTint(); return hsVector3(col.r, col.g, col.b); }
		hsScalar GetSpecularMute() const { return GetSpecularTint().a; }

	hsPoint3 GetEnvCenter() const { return fState.fEnvCenter; }
	hsScalar GetEnvRadius() const { return fState.fEnvRadius; }

	// Export/debugging functions. For runtime, use message interface (plGenRefMsg, plWaveMsg).
	void		AddTarget(const plKey& key);
	void		RemoveTarget(const plKey& key);
	void		AddShoreTest(plKey& key);

	void		SetRefObject(plSceneObject* refObj);

	void			SetSceneNode(const plKey& key);
	plKey			GetSceneNode() const { return fSceneNode; }

	void			AddDynaDecalMgr(plKey& key);
	void			RemoveDynaDecalMgr(plKey& key);

	void			AddBuoy(plKey soKey);
	void			RemoveBuoy(plKey soKey);

	virtual hsBool			SetupRippleMat(hsGMaterial* mat, const plRipVSConsts& ripConsts);

	virtual hsScalar		GetHeight() const { return State().fWaterHeight; }

	const plFixedWaterState7::WaveState& GeoState() const { return State().fGeoState; }
	const plFixedWaterState7::WaveState& TexState() const { return State().fTexState; }
	const plFixedWaterState7& State() const { return fState; }
	void			SetState(const plFixedWaterState7& state, hsScalar dur);

	void			SetEnvSize(UInt32 s) { fEnvSize = s; }
	UInt32			GetEnvSize() const { return fEnvSize; }

	void StopLog();
	void StartLog();
	hsBool Logging() const { return fStatusLog != nil; }
	void StartGraph();
	void StopGraph();
	hsBool Graphing() const { return fStatusGraph != nil; }
};

#endif // plWaveSet7_inc
