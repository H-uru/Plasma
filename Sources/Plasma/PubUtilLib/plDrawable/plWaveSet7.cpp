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

#include "hsTypes.h"
#include "hsFastMath.h"
#include "hsTimer.h"

#include "plWaveSet7.h"
#include "plWaveSetShaderConsts.h"
#include "plRipVSConsts.h"

#include "plAccessGeometry.h"
#include "plAccessSpan.h"
#include "plAccessVtxSpan.h"

#include "plAuxSpan.h"
#include "plDynaDecal.h"
#include "plDynaRippleVSMgr.h"

#include "../plMessage/plRenderMsg.h"
#include "../pnMessage/plTimeMsg.h"

#include "../pnMessage/plObjRefMsg.h"

#include "plgDispatch.h"
#include "plPipeline.h"

#include "hsResMgr.h"

#include "../pnSceneObject/plDrawInterface.h"

#include "plPhysical.h"
#include "../plMessage/plSimInfluenceMsg.h"

#include "../plSurface/hsGMaterial.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plDrawableGenerator.h"

#include "../plMessage/plAvatarMsg.h"
#include "../plAvatar/plArmatureMod.h"

#include "../plGImage/plMipmap.h"
#include "../plGImage/plCubicEnvironmap.h"
#include "../plSurface/plLayer.h"
#include "../plMessage/plLayRefMsg.h"

#include "../plSurface/plShader.h"

#include "../plPipeline/plRenderTarget.h"
#include "../plScene/plRenderRequest.h"
#include "../plMessage/plRenderRequestMsg.h"
#include "../plScene/plPageTreeMgr.h"

#include "../plPipeline/plDynamicEnvMap.h"

#include "../plGImage/plBumpMapGen.h"

#include "../plMessage/plMatRefMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"

#include "plTweak.h"

#ifndef PLASMA_EXTERNAL_RELEASE
#include "../plStatusLog/plStatusLog.h"
#include "../plPipeline/plPlates.h"
#endif // PLASMA_EXTERNAL_RELEASE

using namespace plShaderID;

///////////////////////////////////////////////////////////////////////
#include "plProfile.h"
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#define TEST_ENVSPH

// #define TEST_UVWS

static const hsScalar kPiOverTwo = hsScalarPI * 0.5f;

static const hsScalar kGravConst = 30.f;

static const hsScalar kOOEightNsqPI = 1.f / (8.f * hsScalarPI * 4.f * 4.f);
static hsScalar currOOEightNsqPI = kOOEightNsqPI;

static inline hsScalar FreqToLen(hsScalar f) { return 2.f * hsScalarPI / f; }
static inline hsScalar LenToFreq(hsScalar l) { return 2.f * hsScalarPI / l; }

static inline hsScalar MPH2FPS(hsScalar f) { return f * 5280.f / 3600.f; }
static inline hsScalar FPS2MPH(hsScalar f) { return f / 5280.f * 3600.f; }

plCONST(hsScalar) kTimeClamp(0.3f);

inline void plWorldWave7::Accumulate(hsPoint3& accumPos, hsVector3& accumNorm) const
{
	hsScalar dist = accumPos.fX * fDir.fX + accumPos.fY * fDir.fY;

	dist *= fFreq;
	dist += fPhase;

	hsScalar s, c;
	hsFastMath::SinCosAppr(dist, s, c);
//	s += 1.f;

	// Add scaled local Z instead? So decals can move up and down non-vertically?
	// Thing is, for the height is still based on the unperterbed position, so when
	// we move it laterally, the height is wrong. Maybe not too wrong, but still wrong.
	// Same for normal.
	accumPos.fZ += s * fAmplitude; 

	c *= -fFreq * fAmplitude;
	accumNorm.fX += fDir.fX * c;
	accumNorm.fY += fDir.fY * c;
}

#ifndef PLASMA_EXTERNAL_RELEASE
inline void plWaveSet7::GraphLen(hsScalar len) const
{
	if( fStatusGraph )
	{
		hsScalar maxLen = TexState().fMaxLength * kCompositeSize / State().fRippleScale;
		Int32 val = Int32(len / maxLen * 100.f);
		fStatusGraph->AddData(val);
	}
}

inline void plWaveSet7::IRestartGraph() const
{
	if( fStatusGraph )
		fStatusGraph->ClearData();
}

void plWaveSet7::StartGraph()
{
	delete fStatusGraph;
	plPlateManager::Instance().CreateGraphPlate(&fStatusGraph);
	fStatusGraph->SetSize(0.25f, 0.25f);
	fStatusGraph->SetDataRange(0, 100, kNumTexWaves);
	fStatusGraph->SetVisible(true);
	fStatusGraph->SetPosition(0.75f, -0.75f);
}

void plWaveSet7::StopGraph()
{
	delete fStatusGraph;
	fStatusGraph = nil;
}

inline void plWaveSet7::LogF(const char *format, ...) const
{
	if( fStatusLog )
	{
		va_list args;
		va_start(args,format);
		fStatusLog->AddLineV(format, args);
		va_end(args);
	}
}

inline void plWaveSet7::LogF(UInt32 color, const char *format, ...) const
{
	if( fStatusLog )
	{
		va_list args;
		va_start(args,format);
		fStatusLog->AddLineV(color, format, args);
		va_end(args);
	}
}

inline void plWaveSet7::IRestartLog() const
{
	if( fStatusLog )
		fStatusLog->Clear();
}

void plWaveSet7::StartLog()
{
	delete fStatusLog;
	fStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(kNumTexWaves, "TexWaves", 
							plStatusLog::kDontWriteFile | plStatusLog::kDeleteForMe | plStatusLog::kFilledBackground);
}

void plWaveSet7::StopLog()
{
	delete fStatusLog;
	fStatusLog = nil;
}
#else // PLASMA_EXTERNAL_RELEASE
inline void plWaveSet7::GraphLen(hsScalar len) const
{
}

inline void plWaveSet7::IRestartGraph() const
{
}

void plWaveSet7::StartGraph()
{
}

void plWaveSet7::StopGraph()
{
}

inline void plWaveSet7::LogF(const char *format, ...) const
{
}

inline void plWaveSet7::LogF(UInt32 color, const char *format, ...) const
{
}

inline void plWaveSet7::IRestartLog() const
{
}

void plWaveSet7::StartLog()
{
}

void plWaveSet7::StopLog()
{
}
#endif // PLASMA_EXTERNAL_RELEASE


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enough of that, WaveSet (system manager) follows
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
plWaveSet7::plWaveSet7()
:	fLastTime(0),
	fCurrTime(0),
	fScrunchLen(8.f),
	fScrunchScale(2.f),
	fFreqScale(1.f),
	fRipVShader(nil),
	fRipPShader(nil),
	fShoreVShader(nil),
	fShorePShader(nil),
	fFixedVShader(nil),
	fFixedPShader(nil),
	fBiasVShader(nil),
	fBiasPShader(nil),
	fBumpMat(nil),
	fBumpDraw(nil),
	fBumpReq(nil),
	fBumpReqMsg(nil),
	fEnvMap(nil),
	fRefObj(nil),
	fCosineLUT(nil),
	fGraphShoreTex(nil),
	fBubbleShoreTex(nil),
	fEdgeShoreTex(nil),
	fMaxLen(0),

	fTrialUpdate(0),
	fTransistor(-1),
	fTransCountDown(30.f),
	fTransDel(0),

	fTexTrans(0),
	fTexTransCountDown(0),
	fTexTransDel(0),

	fStatusLog(nil),
	fStatusGraph(nil)
{
	IInitState();
	IInitWaveConsts();

	int i;
	for( i = 0; i < kNumWaves; i++ )
	{
		fFreqMod[i] = 1.f;
		IInitWave(i);
	}
	for( i = 0; i < 4; i++ )
	{
		fFixedLayers[i] = nil;
	}
	for( i = 0; i < kNumBumpShaders; i++ )
	{
		fBumpVShader[i] = nil;
		fBumpPShader[i] = nil;
	}
	fBiasLayer[0] = nil;
	fBiasLayer[1] = nil;
	for( i = 0; i < kNumTexWaves; i++ )
	{
		fBumpLayers[i] = nil;
		fTexWaveFade[i] = 1.f;
	}
	for( i = 0; i < kGraphShorePasses; i++ )
	{
		fGraphVShader[i] = nil;
		fGraphPShader[i] = nil;

		fGraphShoreMat[i] = nil;
		fGraphShoreRT[i] = nil;
		fGraphShoreDraw[i] = nil;
		fGraphReq[i] = nil;
		fGraphReqMsg[i] = nil;
	}
	for( i = 0; i < kNumDecalVShaders; i++ )
		fDecalVShaders[i] = nil;
	for( i = 0; i < kNumDecalPShaders; i++ )
		fDecalPShaders[i] = nil;
}

plWaveSet7::~plWaveSet7()
{
	delete fStatusLog;

	delete fBumpReqMsg;
	delete fBumpReq;

	int i;
	for( i = 0; i < kGraphShorePasses; i++ )
	{
		delete fGraphReqMsg[i];
		delete fGraphReq[i];
	}
}

void plWaveSet7::Read(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Read(stream, mgr);

	fMaxLen = stream->ReadSwapScalar();

	fState.Read(stream);
	IUpdateWindDir(0);

	int n = stream->ReadSwap32();
	int i;
	for( i = 0; i < n; i++ )
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefShore), plRefFlags::kPassiveRef);
	}
	n = stream->ReadSwap32();
	for( i = 0; i < n; i++ )
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefDecal), plRefFlags::kPassiveRef);
	}
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefEnvMap), plRefFlags::kActiveRef);

	if( HasFlag(kHasRefObject) )
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefRefObj), plRefFlags::kPassiveRef);
	}

	ISetupTextureWaves();

	for( i = 0; i < kNumWaves; i++ )
	{
		fFreqMod[i] = 1.f;
		IInitWave(i);
	}

	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());
}

void plWaveSet7::Write(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Write(stream, mgr);

	stream->WriteSwapScalar(fMaxLen);

	fState.Write(stream);

	stream->WriteSwap32(fShores.GetCount());
	int i;
	for( i = 0; i < fShores.GetCount(); i++ )
	{
		mgr->WriteKey(stream, fShores[i]);
	}
	stream->WriteSwap32(fDecals.GetCount());
	for( i = 0; i < fDecals.GetCount(); i++ )
	{
		mgr->WriteKey(stream, fDecals[i]);
	}
	mgr->WriteKey(stream, fEnvMap);

	if( HasFlag(kHasRefObject) )
	{
		mgr->WriteKey(stream, fRefObj);
	}
}

hsBool plWaveSet7::MsgReceive(plMessage* msg)
{
	plEvalMsg* update = plEvalMsg::ConvertNoRef(msg);
	if( update )
	{
		if (fFixedVShader == nil)
		{
			ICheckTargetMaterials();
			ICheckShoreMaterials();
			ICheckDecalMaterials();
		}
		IRestartLog();
		IRestartGraph();

		plCONST(hsBool) reRender(false);
		if( reRender || (fTrialUpdate & kReRenderEnvMap) )
		{
			plDynamicEnvMap* envMap = plDynamicEnvMap::ConvertNoRef(fEnvMap);
			if( envMap )
			{
				envMap->ReRender();

				fTrialUpdate &= ~kReRenderEnvMap;
			}
		}

		hsScalar dt = update->DelSeconds();
		if( dt > kTimeClamp )
			dt = kTimeClamp;

		IUpdateWindDir(dt);

		IFloatBuoys(dt);
		return true;
	}

	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
	if( rend )
	{
		if( !IAnyBoundsVisible(rend->Pipeline()) )
			return true;

		fCurrTime = hsTimer::GetSysSeconds();
		// Can't just use GetDelSysSeconds() or else we lose time if we skip a frame render because of high FPS.
		hsScalar dt = fLastTime > 0 ? hsScalar(fCurrTime - fLastTime) : hsTimer::GetDelSysSeconds();
		if( dt > kTimeClamp )
			dt = kTimeClamp;

		IUpdateRefObject();

		IUpdateLayers(dt);

		IUpdateWaves(dt);

		hsMatrix44 l2w;
		hsMatrix44 w2l;
		IUpdateShaders(rend->Pipeline(), l2w, w2l);
		IUpdateGraphShaders(rend->Pipeline(), dt);

		fLastTime = fCurrTime;
		return true;
	}

	plAgeLoadedMsg* ageLoaded = plAgeLoadedMsg::ConvertNoRef(msg);
	if( (ageLoaded && ageLoaded->fLoaded) || plInitialAgeStateLoadedMsg::ConvertNoRef(msg) )
	{
		ICheckTargetMaterials();
		ICheckShoreMaterials();
		ICheckDecalMaterials();
		return true;
	}

	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace) )
		{
			IOnReceive(refMsg);
		}
		else
		{
			IOnRemove(refMsg);
		}
		return true;
	}

	return plMultiModifier::MsgReceive(msg);
}

hsBool plWaveSet7::IAnyBoundsVisible(plPipeline* pipe) const
{
	int i;
	for( i = 0; i < fTargBnds.GetCount(); i++ )
	{
		if( pipe->TestVisibleWorld(fTargBnds[i]) )
			return true;
	}
	return false;
}

hsBool plWaveSet7::IOnReceive(plGenRefMsg* refMsg)
{
	switch( refMsg->fType )
	{
	case kRefRefObj:
		fRefObj = plSceneObject::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefCosineLUT:
		fCosineLUT = plMipmap::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefEnvMap:
		fEnvMap = plBitmap::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefShore:
		fShores.Append(plSceneObject::ConvertNoRef(refMsg->GetRef()));
		return true;
	case kRefDecal:
		fDecals.Append(plSceneObject::ConvertNoRef(refMsg->GetRef()));
		return true;
	case kRefDecVShader:
		fDecalVShaders[refMsg->fWhich] = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefDecPShader:
		fDecalPShaders[refMsg->fWhich] = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefGraphShoreRT:
		fGraphShoreRT[refMsg->fWhich] = plRenderTarget::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefGraphVShader:
		fGraphVShader[refMsg->fWhich] = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefGraphPShader:
		fGraphPShader[refMsg->fWhich] = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefGraphShoreTex:
		fGraphShoreTex = plMipmap::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBubbleShoreTex:
		fBubbleShoreTex = plMipmap::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefEdgeShoreTex:
		fEdgeShoreTex = plMipmap::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefGraphShoreMat:
		fGraphShoreMat[refMsg->fWhich] = hsGMaterial::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefGraphShoreDraw:
		fGraphShoreDraw[refMsg->fWhich] = plDrawableSpans::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBiasVShader:
		fBiasVShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBiasPShader:
		fBiasPShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBumpVShader:
		fBumpVShader[refMsg->fWhich] = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBumpPShader:
		fBumpPShader[refMsg->fWhich] = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefRipVShader:
		fRipVShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefRipPShader:
		fRipPShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefShoreVShader:
		fShoreVShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefShorePShader:
		fShorePShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefFixedVShader:
		fFixedVShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefFixedPShader:
		fFixedPShader = plShader::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBumpDraw:
		fBumpDraw = plDrawableSpans::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefBumpMat:
		fBumpMat = hsGMaterial::ConvertNoRef(refMsg->GetRef());
		return true;
	case kRefDynaDecalMgr:
		{
			plDynaDecalMgr* dyna = plDynaDecalMgr::ConvertNoRef(refMsg->GetRef());
			if( fDecalMgrs.Find(dyna) == fDecalMgrs.kMissingIndex )
			{
				fDecalMgrs.Append(dyna);
			}
		}
		return true;
	case kRefBuoy:
		{
			plSceneObject* so = plSceneObject::ConvertNoRef(refMsg->GetRef());
			if( fBuoys.Find(so) == fBuoys.kMissingIndex )
			{
				IShiftCenter(so);
				fBuoys.Append(so);
			}
		}
		return true;
	}
	return false;
}

hsBool plWaveSet7::IOnRemove(plGenRefMsg* refMsg)
{
	switch( refMsg->fType )
	{
	case kRefRefObj:
		fRefObj = nil;
		return true;
	case kRefCosineLUT:
		fCosineLUT = nil;
		return true;
	case kRefEnvMap:
		fEnvMap = nil;
		return true;
	case kRefShore:
		{
			plSceneObject* shore = (plSceneObject*)refMsg->GetRef();
			int idx = fShores.Find(shore);
			if( idx != fShores.kMissingIndex )
				fShores.Remove(idx);
		}
		return true;
	case kRefDecal:
		{
			plSceneObject* decal = (plSceneObject*)refMsg->GetRef();
			int idx = fDecals.Find(decal);
			if( idx != fDecals.kMissingIndex )
				fDecals.Remove(idx);
		}
		return true;
	case kRefDecVShader:
		fDecalVShaders[refMsg->fWhich] = nil;
		return true;
	case kRefDecPShader:
		fDecalPShaders[refMsg->fWhich] = nil;
		return true;
	case kRefGraphShoreRT:
		fGraphShoreRT[refMsg->fWhich] = nil;
		return true;
	case kRefGraphVShader:
		fGraphVShader[refMsg->fWhich] = nil;
		return true;
	case kRefGraphPShader:
		fGraphPShader[refMsg->fWhich] = nil;
		return true;
	case kRefGraphShoreTex:
		fGraphShoreTex = nil;
		return true;
	case kRefBubbleShoreTex:
		fBubbleShoreTex = nil;
		return true;
	case kRefEdgeShoreTex:
		fEdgeShoreTex = nil;
		return true;
	case kRefGraphShoreMat:
		fGraphShoreMat[refMsg->fWhich] = nil;
		return true;
	case kRefGraphShoreDraw:
		fGraphShoreDraw[refMsg->fWhich] = nil;
		return true;
	case kRefBiasVShader:
		fBiasVShader = nil;
		return true;
	case kRefBiasPShader:
		fBiasPShader = nil;
		return true;
	case kRefBumpVShader:
		fBumpVShader[refMsg->fWhich] = nil;
		return true;
	case kRefBumpPShader:
		fBumpPShader[refMsg->fWhich] = nil;
		return true;
	case kRefRipVShader:
		fRipVShader = nil;
		return true;
	case kRefRipPShader:
		fRipPShader = nil;
		return true;
	case kRefShoreVShader:
		fShoreVShader = nil;
		return true;
	case kRefShorePShader:
		fShorePShader = nil;
		return true;
	case kRefFixedVShader:
		fFixedVShader = nil;
		return true;
	case kRefFixedPShader:
		fFixedPShader = nil;
		return true;
	case kRefBumpDraw:
		fBumpDraw = nil;
		return true;
	case kRefBumpMat:
		fBumpMat = nil;
		return true;
	case kRefDynaDecalMgr:
		{
			plDynaDecalMgr* dyna = (plDynaDecalMgr*)refMsg->GetRef();
			int idx = fDecalMgrs.Find(dyna);
			if( fDecalMgrs.kMissingIndex != idx )
				fDecalMgrs.Remove(idx);
		}
		return true;
	case kRefBuoy:
		{
			plSceneObject* so = (plSceneObject*)refMsg->GetRef();
			int idx = fBuoys.Find(so);
			if( fBuoys.kMissingIndex != idx )
				fBuoys.Remove(idx);
		}
		return true;
	}
	return false;
}

void plWaveSet7::SetState(const plFixedWaterState7& state, hsScalar dur)
{ 
	fState.Set(state, dur);

	if( fFixedLayers[0] )
	{
		plLayer* lay = plLayer::ConvertNoRef(fFixedLayers[0]->BottomOfStack());
		lay->SetRuntimeColor(state.fWaterTint);
	}
}

void plWaveSet7::IUpdateWaves(hsScalar dt)
{
	ITransition(dt);
	ITransTex(dt);
	ICalcScale();
	fScrunchLen = 1.e33f;

	if( fTrialUpdate & kReInitWaves )
	{
		IReInitWaves();
		ISetupTextureWaves();
		ICreateBumpMipmapPS();
	}

	int i;
	for( i = 0; i < kNumWaves; i++ )
	{
		IUpdateWave(dt, i);
	}
}

// return true if we've finished this transition.
hsBool plWaveSet7::ITransContinue(hsScalar dt)
{
	hsScalar currFade = (fFreqMod[fTransistor] += fTransDel * dt);

	if( currFade <= 0 )
	{
		// Done fading down, time to fade back up.
		fTransDel = -fTransDel;
		fFreqMod[fTransistor] = fTransDel * dt;

		// Reinit the wave. This gives a chance to introduce more randomization,
		// as well as react to changing weather conditions.
		IInitWave(fTransistor);

		return false;
	}
	
	if( currFade >= 1.f )
	{
		fFreqMod[fTransistor] = 1.f;
		fTransDel = 0;

		return true;
	}

	return false;
}

void plWaveSet7::IStartTransition(hsScalar dt)
{
	// select the next wave for transitioning
	if( ++fTransistor >= kNumWaves )
		fTransistor = 0;

	// set the transFade to be fading down.
	plCONST(hsScalar) kTransDel(0.5f);
	fTransDel = -kTransDel;
}

hsScalar plWaveSet7::ITransitionDelay() const
{
	plCONST(hsScalar) kTransDelay(2.f);
	return kTransDelay;
}

void plWaveSet7::ITransition(hsScalar dt)
{
	// If we're in a transition, keep transitioning till it's done.
	if( fTransDel != 0 )
	{
		if( ITransContinue(dt) )
			fTransCountDown = ITransitionDelay();
	}
	// else if our transition countdown has gone to zero, start a new one.
	else if( (fTransCountDown -= dt) <= 0 )
	{
		IStartTransition(dt);
	}

}

hsBool plWaveSet7::ITransTexContinue(hsScalar dt)
{
	hsScalar currFade = (fTexWaveFade[fTexTrans] += fTexTransDel * dt);

	if( currFade <= 0 )
	{
		// Done fading down, time to fade back up.
		fTexTransDel = -fTexTransDel;
		fTexWaveFade[fTexTrans] = fTexTransDel * dt;

		// ReInit the wave
		IInitTexWave(fTexTrans);

		return false;
	}

	if( currFade >= 1.f )
	{
		// This one is back to full on, time to pick on another one.
		fTexWaveFade[fTexTrans] = 1.f;
		fTexTransDel = 0;

		return true;
	}

	return false;
}

void plWaveSet7::IStartTexTransition(hsScalar dt)
{
	if( ++fTexTrans >= kNumTexWaves )
		fTexTrans = 0;

	plConst(hsScalar) kTexTransDel(4.f);
	fTexTransDel = -kTexTransDel;
}

void plWaveSet7::ITransTex(hsScalar dt)
{

	// If we're in a transition, keep with it.
	if( fTexTransDel != 0 )
	{
		plConst(hsScalar) kTexTransDelay(0);
		if( ITransTexContinue(dt) )
			fTexTransCountDown = kTexTransDelay;
	}
	// else if it's time to start another...
	else if( (fTexTransCountDown -= dt) <= 0 )
	{
		IStartTexTransition(dt);
	}
}

void plWaveSet7::ICalcScale()
{
}

void plWaveSet7::IUpdateWave(hsScalar dt, int i)
{
	plWorldWave7& wave = fWorldWaves[i];

	hsScalar len = FreqToLen(wave.fFreq);

	hsScalar speed = hsFastMath::InvSqrtAppr(len / (2.f * hsScalarPI * kGravConst));

	static hsScalar speedHack = 1.f;
	speed *= speedHack;
	wave.fPhase += speed * dt;
//	wave.fPhase = fmod( speed * t, 2.f * hsScalarPI);

	hsScalar amp = GeoState().fAmpOverLen * len / hsScalar(kNumWaves);

	amp *= fFreqMod[i] * fFreqScale;

	wave.fAmplitude = amp;

}

void plWaveSet7::IReInitWaves()
{
	int i;
	for( i = 0; i < kNumWaves; i++ )
	{
		IInitWave(i);
	}
	fTransDel = 0;
	fTransCountDown = ITransitionDelay();
	fTrialUpdate &= ~kReInitWaves;
}


void plWaveSet7::IInitWave(int i)
{
	plWorldWave7& wave = fWorldWaves[i];

	wave.fLength = GeoState().fMinLength + fRand.RandZeroToOne() * (GeoState().fMaxLength - GeoState().fMinLength);

	hsScalar len = wave.fLength;

	wave.fFreq = LenToFreq(len);

	wave.fPhase = 0;

	wave.fAmplitude = 0;

	// Figure out the direction based on wind direction.
	// Even waves go in the wind direction,
	// odd waves go opposite direction
	plConst(hsScalar) kMinRotDeg(15.f);
	plConst(hsScalar) kMaxRotDeg(180.f);
	hsVector3 dir = fWindDir;

	hsScalar rotBase = GeoState().fAngleDev;

	hsScalar rads = rotBase * fRand.RandMinusOneToOne();
	hsScalar rx = hsScalar(cosf(rads));
	hsScalar ry = hsScalar(sinf(rads));

	hsScalar x = dir.fX;
	hsScalar y = dir.fY;
	dir.fX = x * rx + y * ry;
	dir.fY = x * -ry + y * rx;

	wave.fDir.Set(dir.fX, dir.fY, 0);
}

inline void plWaveSet7::IScrunch(hsPoint3& pos, hsVector3& norm) const
{
	pos.fX += -norm.fX * fScrunchLen;
	pos.fY += -norm.fY * fScrunchLen;

	norm.fX *= fScrunchScale;
	norm.fY *= fScrunchScale;

	hsFastMath::NormalizeAppr(norm);
}

hsScalar plWaveSet7::EvalPoint(hsPoint3& pos, hsVector3& norm)
{
	hsPoint3 accumPos;
	hsVector3 accumNorm;
	accumPos.Set(pos.fX, pos.fY, State().fWaterHeight);
	accumNorm.Set(0,0,0);

	int i;
	for( i = 0; i < kNumWaves; i++ )
		fWorldWaves[i].Accumulate(accumPos, accumNorm);

	accumNorm.fZ = 1.f;

	hsFastMath::NormalizeAppr(accumNorm);

	IScrunch(accumPos, accumNorm);

	// Project original pos along Z onto the plane tangent at accumPos with norm accumNorm
	hsScalar t = hsVector3(&accumPos, &pos).InnerProduct(accumNorm);
	t /= accumNorm.fZ;

	pos.fZ += t;

	norm = accumNorm;

	return pos.fZ;
}

void plWaveSet7::IUpdateWindDir(hsScalar dt)
{
	fWindDir = -State().fWindDir;
	hsFastMath::NormalizeAppr(fWindDir);
}

void plWaveSet7::IUpdateRefObject()
{
	if( fRefObj )
	{
		hsMatrix44 l2w = fRefObj->GetLocalToWorld();
		
		hsScalar h = l2w.fMap[2][3];

		hsScalar x = -l2w.fMap[0][1];
		hsScalar y = -l2w.fMap[1][1];

		fState.fWaterHeight = h;

		fState.fWindDir = hsVector3(x, y, 0.f);
	}
}

void plWaveSet7::IFloatBuoy(hsScalar dt, plSceneObject* so)
{
	// Compute force based on world bounds
	hsBounds3Ext wBnd = so->GetDrawInterface()->GetWorldBounds();
	hsBounds3Ext lBnd = so->GetDrawInterface()->GetLocalBounds();

	hsPoint3 pos(wBnd.GetCenter());
	hsPoint3 surfPos(pos);
	hsVector3 surfNorm;

	EvalPoint(surfPos, surfNorm);

	// Direction of impulse is surfNorm. Magnitude is proportional to depth
	// (in an approximation lazy hackish way).
	hsPoint2 boxDepth;
	wBnd.TestPlane(surfNorm, boxDepth);
	hsScalar surfDepth = surfNorm.InnerProduct(surfPos);
	hsScalar depth = surfDepth - boxDepth.fX;

	if( depth < 0 )
		return;

	if( depth > wBnd.GetMaxs().fZ - wBnd.GetMins().fZ )
		depth = wBnd.GetMaxs().fZ - wBnd.GetMins().fZ;

	// We really want the cross section area as facing into the water,
	// but life is full of little disappointments.
	hsScalar area = (wBnd.GetMaxs().fX - wBnd.GetMins().fX) * (wBnd.GetMaxs().fY - wBnd.GetMins().fY);

	hsScalar volume = area * depth;

	plCONST(hsScalar) kWaterDensity(1.0f);
	hsScalar forceMag = volume * kWaterDensity;

	// surfNorm is now the impulse vector. But where to apply it.
	// Don't currently have anything informative from the physical to use.
	// So, let's fake something for the moment.

	plKey physKey = so->GetSimulationInterface()->GetPhysical()->GetKey();

//	plImpulseMsg* iMsg = TRACKED_NEW plImpulseMsg(GetKey(), physKey, hsVector3(0, 0, 1.f) * forceMag * dt);
//	iMsg->Send();

#if 0
	plCONST(hsScalar) kRotScale(1.f);
	hsVector3 rotAx = hsVector3(0, 0, 1.f) % surfNorm;
	rotAx *= kRotScale * dt * volume;

	plAngularImpulseMsg* aMsg = TRACKED_NEW plAngularImpulseMsg(GetKey(), physKey, rotAx);
	aMsg->Send();
#endif

	plCONST(hsScalar) kDampener(0.1f);
	plCONST(hsScalar) kBaseDamp(0.1f);
	if( wBnd.GetMaxs().fZ > wBnd.GetMins().fZ )
	{
		// Remember, we've already limited depth to be <= Max.fZ - Min.fZ;
		hsScalar damp = depth / (wBnd.GetMaxs().fZ - wBnd.GetMins().fZ);
		damp *= kDampener;
		damp += kBaseDamp;

//		plDampMsg* dMsg = TRACKED_NEW plDampMsg(GetKey(), physKey, damp);
//		dMsg->Send();
	}
}

void plWaveSet7::IFloatBuoys(hsScalar dt)
{
	int i;
	for( i = 0; i < fBuoys.GetCount(); i++ )
	{
		if( fBuoys[i] && fBuoys[i]->GetSimulationInterface() && fBuoys[i]->GetSimulationInterface()->GetPhysical() && fBuoys[i]->GetDrawInterface() )
		{
			IFloatBuoy(dt, fBuoys[i]);
		}
	}
}

void plWaveSet7::IShiftCenter(plSceneObject* so) const
{
	// HACKAGE
	if( 0 && so->GetSimulationInterface() && so->GetSimulationInterface()->GetPhysical() && so->GetDrawInterface() )
	{
		hsPoint3 center = so->GetDrawInterface()->GetWorldBounds().GetCenter();
		hsPoint3 pos = so->GetLocalToWorld().GetTranslate();
		hsVector3 offset(&pos, &center);
//		plShiftMassMsg* msg = TRACKED_NEW plShiftMassMsg(GetKey(), so->GetSimulationInterface()->GetPhysical()->GetKey(), offset);
//		msg->Send();
	}
}

void plWaveSet7::ICheckTargetMaterials()
{
	hsBounds3Ext targBnd;
	targBnd.MakeEmpty();
	int i;
	for( i = 0; i < GetNumTargets(); i++ )
	{
		plSceneObject* so = GetTarget(i);
		if( !so )
			continue;

		const plDrawInterface* di = so->GetDrawInterface();
		if( !di )
			continue;

		hsTArray<plAccessSpan> src;
		plAccessGeometry::Instance()->OpenRO(di, src, false);

		const int numUVWs = src.GetCount() && src[0].AccessVtx().HasUVWs() ? src[0].AccessVtx().NumUVWs() : 0;
		int j;
		for( j = 0; j < src.GetCount(); j++ )
		{
			hsAssert(src[j].AccessVtx().NumUVWs() == numUVWs, "Must have same number uvws on each water mesh");
			ICreateFixedMat(src[j].GetMaterial(), numUVWs); // no-op if it's already setup.

			targBnd.Union(&src[j].GetWorldBounds());
		}

		plAccessGeometry::Instance()->Close(src);

		for( j = 0; j < di->GetNumDrawables(); j++ )
		{
			plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(j));
			if( dr )
			{
				dr->SetNativeProperty(di->GetDrawableMeshIndex(j), plSpan::kPropRunTimeLight | plSpan::kPropSkipProjection | plSpan::kPropNoShadow | plSpan::kLiteVtxNonPreshaded | plSpan::kPropReverseSort, true);
			}
		}
	}
	if( targBnd.GetType() == kBoundsNormal )
	{
		if( !fTargBnds.GetCount() )
			fTargBnds.SetCount(1);
		
		plConst(hsScalar) kMaxWaveHeight(5.f);

		hsPoint3 p;
		p = targBnd.GetMins();
		p.fZ = GetHeight() - kMaxWaveHeight;
		fTargBnds[0].Reset(&p);
		p = targBnd.GetMaxs();
		p.fZ = GetHeight() + kMaxWaveHeight;
		fTargBnds[0].Union(&p);
	}
}


void plWaveSet7::IAddTarget(const plKey& key)
{
	plObjRefMsg* refMsg = TRACKED_NEW plObjRefMsg(key, plRefMsg::kOnRequest, -1, plObjRefMsg::kModifier);
	hsgResMgr::ResMgr()->AddViaNotify( GetKey(), refMsg, plRefFlags::kActiveRef);
}

void plWaveSet7::IRemoveTarget(const plKey& key)
{
	plObjRefMsg* refMsg = TRACKED_NEW plObjRefMsg(key, plRefMsg::kOnRemove, -1, plObjRefMsg::kModifier);
	refMsg->SetRef(this);
	refMsg->Send();
}

void plWaveSet7::AddTarget(const plKey& key)
{
	IAddTarget(key);
}

void plWaveSet7::RemoveTarget(const plKey& key)
{
	IRemoveTarget(key);
}

void plWaveSet7::SetRefObject(plSceneObject* refObj)
{
	fFlags.SetBit(kHasRefObject, refObj != nil);

	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefRefObj);
	hsgResMgr::ResMgr()->SendRef(refObj, msg, plRefFlags::kPassiveRef);
}

void plWaveSet7::AddBuoy(plKey soKey)
{
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefBuoy);
	hsgResMgr::ResMgr()->AddViaNotify(soKey, msg, plRefFlags::kPassiveRef);
}

void plWaveSet7::RemoveBuoy(plKey soKey)
{
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRemove, 0, kRefBuoy);
	msg->SetRef(soKey->ObjectIsLoaded());
	msg->Send();
}



/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

// This is very evil. I can't believe I'm doing this. Stop me, please.
class plFilterMask
{
	protected:
		int				fExt;
		hsScalar		**fMask;

	public:

		plFilterMask( hsScalar sig );
		virtual ~plFilterMask();

		int		Begin() const { return -fExt; }
		int		End() const { return fExt; }

		hsScalar	Mask( int i, int j ) const { return fMask[ i ][ j ]; }
};
// End evil.

void plWaveSet7::IInitState()
{
	plConst(hsScalar) kWaterTable(-10.f);
	plConst(hsScalar) kWaterOffset[3] = { 3.f, 3.f, 0.f };
	plConst(hsScalar) kMaxAtten[3] = { 1.f, 1.f, 1.f };
	plConst(hsScalar) kMinAtten[3] = { 0.f, 0.f, 0.f };
	plConst(hsScalar) kDepthFalloff[3] = { 4.f, 4.f, 6.f };

	plConst(hsScalar) kGeoMinLen(3.f);
	plConst(hsScalar) kGeoMaxLen(8.f);
	plConst(hsScalar) kGeoAmpOverLen(0.1f);
	plConst(hsScalar) kGeoAngleDev(30.f * hsScalarPI / 180.f);
	plConst(hsScalar) kGeoChop(1.f);

	plConst(hsScalar) kTexMinLen(4.f);
	plConst(hsScalar) kTexMaxLen(30.f);
	plConst(hsScalar) kTexAmpOverLen(0.1f);
	plConst(hsScalar) kTexAngleDev(30.f * hsScalarPI / 180.f);
	plConst(hsScalar) kTexChop(1.f);

	plFixedWaterState7 state;

	state.fWindDir = hsVector3(0, 1.f, 0);

	state.fGeoState.fMaxLength = kGeoMaxLen;
	state.fGeoState.fMinLength = kGeoMinLen;
	state.fGeoState.fAmpOverLen = kGeoAmpOverLen;
	state.fGeoState.fChop = kGeoChop;
	state.fGeoState.fAngleDev = kGeoAngleDev;
	
	state.fTexState.fMaxLength = kTexMaxLen;
	state.fTexState.fMinLength = kTexMinLen;
	state.fTexState.fAmpOverLen = kTexAmpOverLen;
	state.fTexState.fChop = kTexChop;
	state.fTexState.fAngleDev = kTexAngleDev;
	
	state.fRippleScale = 25.f;

	plConst(hsScalar) kNoise(1.f);
	plConst(hsScalar) kSpecStart(50.f);
	plConst(hsScalar) kSpecEnd(10000.f);

	hsVector3 spec;
	spec[state.kNoise] = kNoise;
	spec[state.kSpecStart] = kSpecStart;
	spec[state.kSpecEnd] = kSpecEnd;
	state.fSpecVec = spec;

	state.fWaterHeight = kWaterTable;
	state.fWaterOffset = hsVector3(kWaterOffset[0], kWaterOffset[1], kWaterOffset[2]);
	state.fMaxAtten = hsVector3(kMaxAtten[0], kMaxAtten[1], kMaxAtten[2]);
	state.fMinAtten = hsVector3(kMinAtten[0], kMinAtten[1], kMinAtten[2]);
	state.fDepthFalloff = hsVector3(kDepthFalloff[0], kDepthFalloff[1], kDepthFalloff[2]);

	state.fWispiness = 0;
	state.fShoreTint = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f);
	state.fMaxColor = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f);
	state.fMinColor = hsColorRGBA().Set(0.2f, 0.4f, 0.4f, 0.6f);
	state.fEdgeOpac = 1.f;
	state.fEdgeRadius = 1.f;

	state.fPeriod = 3.f;
	state.fFingerLength = 1.f;

	state.fWaterTint = hsColorRGBA().Set(0.1f, 0.2f, 0.2f, 1.f);
	state.fSpecularTint = hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f);

	state.fEnvCenter = hsPoint3(0,0,0);
	state.fEnvRadius = 500.f;
	state.fEnvRefresh = 0.f;

	fState = state;

	IUpdateWindDir(0);
}


void plWaveSet7::IInitWaveConsts()
{
	// See header for what fS, fK and fD actually mean.
	WaveK	waveKs[] = {
		// fS		fK		fD
		{ 1.f,		5.f,	0.f },
		{ 1.f,		10.f,	2.f },
		{ -1.f,		20.f,	-9.f }, 
		{ 1.f,		30.f,	16.f }
	};
	int i;
	for( i = 0; i < kNumTexWaves; i++ )
		fWaveKs[i] = waveKs[i];

	TexWaveWindDep windDeps[] = {
		// WindSpeed, Height, Specular
		{ 0.f,		0.01f,		1.f },
		{ 5.f,		0.05f,		0.8f },
		{ 10.f,		0.05f,		0.7f },
		{ 15.f,		0.1f,		0.6f },
		{ 20.f,		0.1f,		0.5f },
		{ 25.f,		0.1f,		0.3f }
	};

	for( i = 0; i < kNumWindDep; i++ )
		fWindDeps[i] = windDeps[i];

	ISetupTextureWaves();
}

void plWaveSet7::ISetupTextureWaves()
{
	// Fill in with sine waves.
	//	Start with 1/4 length of map = size / 4
	//	Go to 4 times length of pixel = 4
	//	Dir = U +- iLen/(numLen/2) * V
	//
	// For each mip level
	//		For each pixel
	//			For each wavelen
	//				Add height (sine) to alpha, normal (cosine) to color
	//			Scrunch
	//		For each wavelen
	//			// Half the wavelen to match higher mip level
	//			// If a wavelen falls below min (4), discard it and smaller lens
	//			len /= 2
	//			if( len < 4 )
	//			{
	//				numLen = iLen
	//				break;
	//			}
	//

	// Set up our table of amplitudes, wavelengths and wave directions

	// our rotation/scale in our texture transform will be
	// scale = waveKs.fK
	// dir.x * scale, dir.y * scale
	// -dir.y * scale, dir.x * scale
	// So to tile, dir.x * scale and dir.y * scale must be integers.
	// We select a randomized direction and a wavelength, then
	// round the scaled direction components to be integral, then
	// figure out what wavelength we actually came up with.
	int i;
	for( i = 0; i < kNumTexWaves; i++ )
	{
		IInitTexWave(i);
	}
	fTrialUpdate &= ~kReInitWaves;

	return;
}

void plWaveSet7::IInitTexWave(int i)
{
	hsScalar rads = fRand.RandMinusOneToOne() * TexState().fAngleDev;
	hsScalar dx = sin(rads);
	hsScalar dy = cos(rads);


	hsScalar tx = dx;
	dx = fWindDir.fY * dx - fWindDir.fX * dy;
	dy = fWindDir.fX * tx + fWindDir.fY * dy;

	hsScalar maxLen = TexState().fMaxLength * kCompositeSize / State().fRippleScale;
	hsScalar minLen = TexState().fMinLength * kCompositeSize / State().fRippleScale;
	hsScalar len = hsScalar(i) / hsScalar(kNumTexWaves-1) * (maxLen - minLen) + minLen;

	hsScalar reps = hsScalar(kCompositeSize) / len;

	dx *= reps;
	dy *= reps;
	dx = float(int(dx >= 0 ? dx + 0.5f : dx - 0.5f));
	dy = float(int(dy >= 0 ? dy + 0.5f : dy - 0.5f));

	fTexWaves[i].fRotScale00 = dx;
	fTexWaves[i].fRotScale01 = dy;

	hsScalar effK = hsFastMath::InvSqrt(dx*dx + dy*dy);
	fTexWaves[i].fLen = hsScalar(kCompositeSize) * effK;
	fTexWaves[i].fFreq = hsScalarPI * 2.f / fTexWaves[i].fLen;
	fTexWaves[i].fAmp = fTexWaves[i].fLen * TexState().fAmpOverLen;
	fTexWaves[i].fPhase = fRand.RandZeroToOne();
	
	fTexWaves[i].fDirX = dx * effK;
	fTexWaves[i].fDirY = dy * effK;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void plWaveSet7::SetSceneNode(const plKey& key)
{
	fSceneNode = key;
}

void plWaveSet7::AddDynaDecalMgr(plKey& key)
{
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefDynaDecalMgr);
	hsgResMgr::ResMgr()->AddViaNotify(key, msg, plRefFlags::kPassiveRef);

	msg = TRACKED_NEW plGenRefMsg(key, plRefMsg::kOnRequest, 0, plDynaRippleVSMgr::kRefWaveSetBase);
	hsgResMgr::ResMgr()->AddViaNotify(GetKey(), msg, plRefFlags::kPassiveRef);
}

void plWaveSet7::RemoveDynaDecalMgr(plKey& key)
{
	plGenRefMsg* msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRemove, 0, kRefDynaDecalMgr);
	msg->SetRef(key->ObjectIsLoaded());
	msg->Send();

	msg = TRACKED_NEW plGenRefMsg(key, plRefMsg::kOnRemove, 0, plDynaRippleVSMgr::kRefWaveSetBase);
	msg->SetRef(this);
	msg->Send();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void plWaveSet7::IUpdateLayers(hsScalar dt)
{
	IUpdateBumpLayers(dt);

	ISubmitRenderRequests();
}

void plWaveSet7::IUpdateBumpLayers(hsScalar dt)
{
	plCONST(hsScalar) speedHack(1.f / 3.f);
	int i;
	for( i = 0; i < kNumTexWaves; i++ )
	{
		hsScalar speed = hsFastMath::InvSqrtAppr(fTexWaves[i].fLen / (2.f * hsScalarPI * kGravConst)) * speedHack;
		fTexWaves[i].fPhase -= dt * speed;
		fTexWaves[i].fPhase -= int(fTexWaves[i].fPhase);

		if( fBumpLayers[i] )
		{
			hsMatrix44 xfm = fBumpLayers[i]->GetTransform();

			xfm.fMap[0][0] = fTexWaves[i].fRotScale00;
			xfm.fMap[0][1] = fTexWaves[i].fRotScale01;
			xfm.fMap[1][0] = -fTexWaves[i].fRotScale01;
			xfm.fMap[1][1] = fTexWaves[i].fRotScale00;

			xfm.fMap[0][3] = fTexWaves[i].fPhase;
			fBumpLayers[i]->SetTransform(xfm);
		}

		LogF("%.4d - L%8.4f R(%5.3f, %5.3f)", i, fTexWaves[i].fLen, fTexWaves[i].fRotScale00, fTexWaves[i].fRotScale01);
		GraphLen(fTexWaves[i].fLen);
	}
}

void plWaveSet7::ISubmitRenderRequests()
{
	if( fBumpReqMsg )
		fBumpReqMsg->SendAndKeep();

	int i;
	for( i = 0; i < kGraphShorePasses; i++ )
	{
		if( fGraphReqMsg[i] )
			fGraphReqMsg[i]->SendAndKeep();
	}
}

plMipmap* plWaveSet7::ICreateBumpBitmapFFP(hsScalar amp, hsScalar dx, hsScalar dy) const
{
	return nil;
}

hsGMaterial* plWaveSet7::ICreateBumpLayersFFP()
{
	// Create kNumTexWaves bitmaps, each containing a single cycle of values:
	//		mag = 0.25 * -Amplitude * cos(i/sz*2*Pi)
	//		r = mag * dx * 0.5 + 0.5
	//		g = mag * dy * 0.5 + 0.5
	//		b = 0.7 * 0.25 * 0.5 + 0.5
	//		a = 1.f
	//
	//	Where:
	//		Amplitude == wave.amp * wave.freq * kNormHack(3.0)
	//		dx, dy == wave.fDir
	//

	// Create a blank material

	// Create base layer
	//		Blend mode is none
	//		Transform scales to wave[0].freq
	//			No rotation, translation
	//	Add bitmap 0.

	// For layers 1..kNumTexWaves
	//		Create blank layer
	//			Blend mode is ADDSIGNED
	//			Transform scales to wave[i].freq
	//			No rotation or translation.
	//		Add bitmap i;

	// NOTE: if we use the fixed function pipeline to composite
	//		these into the CompositeMaterial, then they all need to
	//		be scaled down to avoid saturation on summation.
	//		Doing the composite with a pixel shader would allow the
	//		textures to be stored in full 256 range, and then scaled
	//		down during the summation, after interpolation. Would
	//		that be better? Not sure. Pixel shader registers only have
	//		a range of [-1..1], and it's hinted to be 8-bit fixed point.
	//		So we'd still have to scale down texture value (losing precision),
	//		then add it in.
	//
	//		Also, for pixel shaded compositing, we can just store the cosine
	//		values in each layer, with no rotation in the bitmap values, just
	//		in the layer transform. No rotation would make for a cleaner
	//		interpolation (not interpolating diagonally across the waves).
	//		How much difference is that? Dunno. But it would also allow the
	//		bitmap to hold the full range [-1..1]=>[0..255] of cosine values,
	//		scaling them down with constants. The pixel shader would look like:
	//
	//			tex		t0;
	//			tex		t1;
	//			tex		t2;
	//			tex		t3;
	//
	//			mul		r0, t0_bx2, c0;
	//			mad		r0, t1_bx2, c1, r0;
	//			mad		r0, t2_bx2, c2, r0;
	//			mad		r0, t3_bx2, c3, r0;
	//			// Now bias it back into range [0..1] for output.
	//			mad		r0, r0, c4, c5;		// c4 = (0.5, 0.5, 0.5, 1), c5 = (0.5, 0.5, 0.5, 0)
	//			// or add_d2	r0, r0, c0.b; // except this is likely to saturate before the divide. Dunno.
	//
	//		where the c[i] constants would be:
	//			c[i] = -wave[i].amp * wave[i].freq * kSomeNormHack * (wave[i].fDir.x, wave[i].fDir.y, 0, 1)
	//		except c[0].b = 0.7;
	//		In fact, we could cut it down to 2 textures using alpha replicate, except we need different
	//		transforms for each (for animation). So screw that.
	//
	//		Note that the above won't run on a ps.1.0 pixel shader, because the combined number of tex and
	//		arith instructions is 9. Could make a version that was okay for ps.1.0 by doing:
	//
	//		mad		r0, t0_bias, c0, c4; // t0[-0.5..0.5]*c0 + 0.5, which is [0..1]
	//		mad		r0, t1_bias, c1, r0; // + t1[-0.5..0.5]*c1
	//		mad		r0, t2_bias, c2, r0; // + t2[-0.5..0.5]*c2
	//		mad		r0, t3_bias, c3, r0; // + t3[-0.5..0.5]*c3
	//
	//		which doesn't need a final bias instruction because it remains biased all the way through
	//		at the cost of 1 bit of precision.
	//		Whatever.
	//		

	// return material;
	return nil;
}

plMipmap* plWaveSet7::ICreateBiasNoiseMap()
{
	const int size = kCompositeSize >> 2;
	plMipmap* mipMap = TRACKED_NEW plMipmap(
		size, size,
		plMipmap::kARGB32Config,
		1, 
		plMipmap::kUncompressed,
		plMipmap::UncompressedInfo::kRGB8888);

	char buff[256];
	sprintf(buff, "%s_%s", GetKey()->GetName(), "BiasBitPS");
	hsgResMgr::ResMgr()->NewKey(buff, mipMap, GetKey()->GetUoid().GetLocation());

	int i;
	for( i = 0; i < size; i++ )
	{
		int j;
		for( j = 0; j < size; j++ )
		{
			hsScalar x = fRand.RandMinusOneToOne();
			hsScalar y = fRand.RandMinusOneToOne();

			UInt8 r = UInt8((x * 0.5f + 0.5f) * 255.999f);
			UInt8 g = UInt8((y * 0.5f + 0.5f) * 255.999f);

//			r = g = 0xff; // SATURATE

			UInt32* val = mipMap->GetAddr32(i, j);
			*val = (0xff << 24)
				| (r << 16)
				| (g << 8)
				| 0xff;
		}
	}
	// For bonus points, modulate by a VERY coherent noise function.
	return mipMap;
}

plMipmap* plWaveSet7::ICreateBumpMipmapPS()
{
//	const int sizeV = kCompositeSize;
	const int sizeV = 1;

	const kNumLevels = 8; // must be log2(kCompositeSize)
	hsAssert(kCompositeSize == (1 << kNumLevels), "Mismatch on size and num mip levels");

	if( !fCosineLUT )
	{
		plMipmap* mipMap = TRACKED_NEW plMipmap(
		kCompositeSize, sizeV,
		plMipmap::kARGB32Config,
		kNumLevels, 
		plMipmap::kUncompressed,
		plMipmap::UncompressedInfo::kRGB8888);

		char buff[256];
		sprintf(buff, "%s_%s", GetKey()->GetName(), "BumpBitPS");
		hsgResMgr::ResMgr()->NewKey(buff, mipMap, GetKey()->GetUoid().GetLocation());

		hsgResMgr::ResMgr()->SendRef(mipMap->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefCosineLUT), plRefFlags::kActiveRef);
	}
	hsAssert(fCosineLUT, "Failed to make cosine lookup table");

	int k;
	for( k = 0; k < kNumLevels; k++ )
	{
		fCosineLUT->SetCurrLevel(k);
		
		const int sizeU = kCompositeSize >> k;

		int i;
		for( i = 0; i < sizeU; i++ )
		{
			hsScalar y = hsScalar(i);
			hsScalar dist = hsScalar(i) / hsScalar(sizeU-1) * 2.f * hsScalarPI;
			hsScalar c = cos(dist);
			hsScalar s = sin(dist);
			s *= 0.5f;
			s += 0.5f;
			s = hsScalar(pow(s, TexState().fChop));
			c *= s;
			UInt8 cosDist = UInt8((c * 0.5 + 0.5) * 255.999f);
			int j;
			for( j = 0; j < sizeV; j++ )
			{
				UInt32* val = fCosineLUT->GetAddr32(i, j);
				*val = (0xff << 24)
					| (cosDist << 16)
					| (cosDist << 8)
					| 0xff;
			}
		}
	}
	fCosineLUT->MakeDirty();

	return fCosineLUT;
}

void plWaveSet7::IAddBumpBiasLayer(hsGMaterial* mat)
{
	if( !fBiasLayer[0] )
	{
		plMipmap* mipMap = ICreateBiasNoiseMap();

		int i;
		for( i = 0; i < 2; i++ )
		{
			plLayer* layer = TRACKED_NEW plLayer;
			char buff[256];
			sprintf(buff, "%s_%s_%d", GetKey()->GetName(), "Bias", i);
			hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

			layer->SetBlendFlags(hsGMatState::kBlendAdd);
			layer->SetZFlags(hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite);
			layer->SetShadeFlags(hsGMatState::kShadeReallyNoFog
				| hsGMatState::kShadeNoProjectors
				| hsGMatState::kShadeNoShade);
			layer->SetClampFlags(0);
			layer->SetMiscFlags(0);
			layer->SetMiscFlags(i ? 0 : hsGMatState::kMiscRestartPassHere);

			layer->SetAmbientColor(hsColorRGBA().Set(0.25f, 0.25f, 0.f, 0.f));
			layer->SetRuntimeColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 0.f));
			layer->SetOpacity(0.f);

			layer->SetUVWSrc(0);

			plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
			hsgResMgr::ResMgr()->SendRef(mipMap->GetKey(), refMsg, plRefFlags::kActiveRef);

			IAddBumpBiasShaders(layer);

			fBiasLayer[i] = layer; // SHOULD BE REFFED (SO SHOULD fBumpLayers)
		}
	}

	mat->AddLayerViaNotify(fBiasLayer[0]);
	mat->AddLayerViaNotify(fBiasLayer[1]);

}

plLayer* plWaveSet7::ICreateBumpLayerPS(plMipmap* mipMap, hsGMaterial* bumpMat, int which)
{
	plLayer* layer = TRACKED_NEW plLayer;
	char buff[256];
	sprintf(buff, "%s_%s_%d", GetKey()->GetName(), "BumpLayerPS", which);
	hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

	layer->SetBlendFlags(which ? hsGMatState::kBlendAdd : 0);
	layer->SetZFlags(hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite);
	layer->SetShadeFlags(hsGMatState::kShadeReallyNoFog
		| hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	layer->SetClampFlags(0);
	layer->SetMiscFlags(0);
	if( (which / kBumpPerPass) * kBumpPerPass == which )
		layer->SetMiscFlags(hsGMatState::kMiscRestartPassHere);

	layer->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	layer->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	layer->SetOpacity(1.f);

	layer->SetUVWSrc(0);

	// Set up the transform.
	hsMatrix44 xfm;
	xfm.Reset();
	xfm.NotIdentity();

	xfm.fMap[0][0] = fTexWaves[which].fRotScale00;
	xfm.fMap[0][1] = fTexWaves[which].fRotScale01;
	xfm.fMap[1][0] = -fTexWaves[which].fRotScale01;
	xfm.fMap[1][1] = fTexWaves[which].fRotScale00;
	layer->SetTransform(xfm);

	bumpMat->AddLayerViaNotify(layer);

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(mipMap->GetKey(), refMsg, plRefFlags::kActiveRef);

	return layer;
}

hsGMaterial* plWaveSet7::ICreateBumpLayersPS()
{
	if( fBumpMat )
		return fBumpMat;

	// Create four bitmaps, each containing a single cycle of values:
	//		mag = cos(i/sz*2*Pi)
	//		r = 0
	//		g = mag* 0.5 + 0.5
	//		b = 1.f
	//		a = 1.f
	//
	//	Where:
	//		Amplitude == wave.amp * wave.freq * kNormHack(3.0)
	//

	// Create a blank material
	hsGMaterial* bumpMat = TRACKED_NEW hsGMaterial;
	char buff[256];
	sprintf(buff, "%s_%s", GetKey()->GetName(), "BumpMatPS");
	hsgResMgr::ResMgr()->NewKey(buff, bumpMat, GetKey()->GetUoid().GetLocation());

	plMipmap* mipMap = ICreateBumpMipmapPS();

	// Create base layer
	//		Blend mode is none
	//		Transform scales to wave[0].freq
	//			No translation
	//			Rotation is by wave[0].fDir
	//	Add bitmap 0.
	int i;
	for( i = 0; i < kNumBumpShaders; i++ )
	{
		int nBegin = bumpMat->GetNumLayers();

		int j;
		for( j = 0; j < kBumpPerPass; j++ )
			fBumpLayers[i*kBumpPerPass + j] = ICreateBumpLayerPS(mipMap, bumpMat, i*kBumpPerPass + j);

		IAddBumpVertexShader(bumpMat, i, nBegin, nBegin + kBumpPerPass-1);
		IAddBumpPixelShader(bumpMat, i, nBegin, nBegin + kBumpPerPass-1);

	}
	IAddBumpBiasLayer(bumpMat);


	// Need to add this via notify to ourselves.
	hsgResMgr::ResMgr()->SendRef(bumpMat->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefBumpMat), plRefFlags::kActiveRef);

	ICreateBumpDrawable();

	return fBumpMat = bumpMat;
}

void plWaveSet7::IAddBumpBiasShaders(plLayer* layer)
{
	if( !fBiasVShader )
	{
		plShader* vShader = TRACKED_NEW plShader;

		char buff[256];
		sprintf(buff, "%s_BiasVS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
		vShader->SetIsPixelShader(false);
		
		vShader->SetNumConsts(plBiasVS::kNumConsts);

		vShader->SetVector(plBiasVS::kTexU0,
			1.f,
			0.f,
			0.f,
			0.f);
		vShader->SetVector(plBiasVS::kTexV0,
			0.f,
			1.f,
			0.f,
			0.f);

		vShader->SetVector(plBiasVS::kTexU1,
			1.f,
			0.f,
			0.f,
			0.f);
		vShader->SetVector(plBiasVS::kTexV1,
			0.f,
			1.f,
			0.f,
			0.f);

		vShader->SetVector(plBiasVS::kNumbers,
			0.f,
			0.5f,
			1.f,
			2.f);

		hsVector3 specVec = State().fSpecVec;
		hsScalar biasScale = 0.5f * specVec[State().kNoise] / (hsScalar(kNumBumpShaders) + specVec[State().kNoise]);
		vShader->SetVector(plBiasVS::kScaleBias,
			biasScale,
			biasScale,
			0.f,
			1.f);

		vShader->SetInputFormat(1);
		vShader->SetOutputFormat(0);

//		static const plShaderDecl vDecl("sha/vs_BiasNormals.inl");
//		vShader->SetDecl(&vDecl);
		vShader->SetDecl(plShaderTable::Decl(vs_BiasNormals));

		hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefBiasVShader), plRefFlags::kActiveRef);


		fBiasVShader = vShader;
	}

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kVertexShader);	
	hsgResMgr::ResMgr()->SendRef(fBiasVShader->GetKey(), refMsg, plRefFlags::kActiveRef);


	if( !fBiasPShader )
	{
		plShader* pShader = TRACKED_NEW plShader;

		char buff[256];
		sprintf(buff, "%s_BiasPS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
		pShader->SetIsPixelShader(true);
		
		pShader->SetNumConsts(plBiasPS::kNumConsts);

		pShader->SetInputFormat(0);
		pShader->SetOutputFormat(0);

//		static const plShaderDecl pDecl("sha/ps_BiasNormals.inl");
//		pShader->SetDecl(&pDecl);
		pShader->SetDecl(plShaderTable::Decl(ps_BiasNormals));

		hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefBiasPShader), plRefFlags::kActiveRef);

		fBiasPShader = pShader;
	}

	refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kPixelShader);	
	hsgResMgr::ResMgr()->SendRef(fBiasPShader->GetKey(), refMsg, plRefFlags::kActiveRef);

}

void plWaveSet7::IAddBumpVertexShader(hsGMaterial* mat, int iShader, int iFirst, int iLast)
{
	if( !fBumpVShader[0] )
	{
		int iBase = 0;
		while( iBase < kNumTexWaves )
		{
			int iShader = iBase / kBumpPerPass;

			plShader* vShader = TRACKED_NEW plShader;
			char buff[256];
			sprintf(buff, "%s_BumpVS_%d", GetKey()->GetName(), iShader);
			hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
			vShader->SetIsPixelShader(false);
			
			vShader->SetNumConsts(plBumpVS::kNumConsts);

			vShader->SetVector(plBumpVS::kNumbers,
				0.f,
				0.5f,
				1.f,
				2.f);

			vShader->SetInputFormat(1);
			vShader->SetOutputFormat(0);

			vShader->SetNumPipeConsts(kBumpPerPass);
			int i;
			for( i = 0; i < kBumpPerPass; i++ )
			{
				vShader->SetPipeConst(i, static_cast<plPipeConst::Type>(plPipeConst::kTex1x4_0 + i), plBumpVS::kUXform0 + i);
			}

			vShader->SetDecl(plShaderTable::Decl(vs_CompCosines));

			hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, iShader, kRefBumpVShader), plRefFlags::kActiveRef);

			fBumpVShader[iShader] = vShader;

			iBase += kBumpPerPass;
		}
	}

	IAddShaderToLayers(mat, iFirst, iLast, plLayRefMsg::kVertexShader, fBumpVShader[iShader]);
}

void plWaveSet7::IAddBumpPixelShader(hsGMaterial* mat, int iShader, int iFirst, int iLast)
{
	if( !fBumpPShader[0] )
	{
		int iBase = 0;
		while( iBase < kNumTexWaves )
		{
			int iShader = iBase / kBumpPerPass;

			plShader* pShader = TRACKED_NEW plShader;
			char buff[256];
			sprintf(buff, "%s_BumpPS_%d", GetKey()->GetName(), iShader);
			hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
			pShader->SetIsPixelShader(true);
			
			pShader->SetNumConsts(plBumpPS::kNumConsts);

			int iLay;
			for( iLay = 0; iLay < kBumpPerPass; iLay++ )
			{
				pShader->SetVector(plBumpPS::kWave0,
					-fTexWaves[iBase + iLay].fDirX * (1.f / hsScalar(kBumpPerPass)),
					-fTexWaves[iBase + iLay].fDirY * (1.f / hsScalar(kBumpPerPass)),
					1.f,
					1.f);
			}

			pShader->SetInputFormat(0);
			pShader->SetOutputFormat(0);

		//	pShader->SetShaderFileName("sha/ps_CompCosines.inl");
		//	pShader->SetShaderFileName("sha/ps_TestPos.inl");
//			static const plShaderDecl moreDecl("sha/ps_MoreCosines.inl");
//			pShader->SetDecl(&moreDecl);

			pShader->SetDecl(plShaderTable::Decl(ps_MoreCosines));

			pShader->SetVector(plBumpPS::kHalfOne, 0.25f, 0.25f, 0.25f, 1.f);

			hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, iShader, kRefBumpPShader), plRefFlags::kActiveRef);

			fBumpPShader[iShader] = pShader;

			iBase += kBumpPerPass;
		}
	}

	IAddShaderToLayers(mat, iFirst, iLast, plLayRefMsg::kPixelShader, fBumpPShader[iShader]);
}

plDrawableSpans* plWaveSet7::ICreateBumpDrawable()
{
	fBumpDraw = TRACKED_NEW plDrawableSpans;
	char buff[256];
	sprintf(buff, "%s_BumpDraw", GetKey()->GetName());
	hsgResMgr::ResMgr()->NewKey(buff, fBumpDraw, GetKey()->GetUoid().GetLocation());

	ICreateClearDrawable(fBumpDraw, fBumpMat);
	
	hsgResMgr::ResMgr()->SendRef(fBumpDraw->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefBumpDraw), plRefFlags::kActiveRef);

	return fBumpDraw;
}

plDrawableSpans* plWaveSet7::ICreateClearDrawable(plDrawableSpans* drawable, hsGMaterial* mat)
{
//	drawable->SetNativeProperty(plDrawable::kPropVolatile, true);

	hsPoint3		pos[4];
	hsVector3		norm[4];
	hsPoint3		uvw[4];

	pos[0].fX = -1.f;
	pos[0].fY = -1.f;
	pos[0].fZ = 0.5f;

	norm[0].Set(0.f, 0.f, -1.f);

	uvw[0].fX = 0.5f / kCompositeSize;
	uvw[0].fY = 0.5f / kCompositeSize;
	uvw[0].fZ = 0;

	// P1
	pos[1] = pos[0];
	pos[1].fY += 2.f;

	norm[1] = norm[0];

	uvw[1] = uvw[0];
	uvw[1].fY += 1.f;

	// P2
	pos[2] = pos[0];
	pos[2].fX += 2.f;
	pos[2].fY += 2.f;

	norm[2] = norm[0];

	uvw[2] = uvw[0];
	uvw[2].fX += 1.f;
	uvw[2].fY += 1.f;
	
	// P3
	pos[3] = pos[0];
	pos[3].fX += 2.f;

	norm[3] = norm[0];

	uvw[3] = uvw[0];
	uvw[3].fX += 1.f;

	UInt16 idx[6];
	idx[0] = 1;
	idx[1] = 0;
	idx[2] = 2;
	
	idx[3] = 2;
	idx[4] = 0;
	idx[5] = 3;


	plDrawableGenerator::GenerateDrawable( 4, pos, norm, 
														uvw, 1,
														nil, false, nil,
														6, idx, 
														mat, 
														hsMatrix44::IdentityMatrix(), 
														false,
														nil,
														drawable);

	return drawable;
}

plRenderRequest* plWaveSet7::ICreateRenderRequest(plRenderTarget* rt, plDrawableSpans* draw, hsScalar pri)
{
	plRenderRequest* req = TRACKED_NEW plRenderRequest;

	static plPageTreeMgr emptyMgr;
	req->SetPageTreeMgr(&emptyMgr);

	req->SetClearDrawable(draw);

	req->SetOrthogonal(true);

	req->SetLocalTransform(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());
	req->SetCameraTransform(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());

	req->SetHither(0);
	req->SetYon(2.f);
	req->SetSizeX(2.f);
	req->SetSizeY(2.f);
	
	req->SetPriority(pri);

	req->SetRenderTarget(rt);

	return req;
}

plRenderTarget* plWaveSet7::ICreateTransferRenderTarget(const char* name, int size)
{
	UInt16 flags = plRenderTarget::kIsTexture | plRenderTarget::kIsOrtho;
	UInt8 bitDepth = 32;
	UInt8 zDepth = 0;
	UInt8 stencilDepth = 0;
	
	plRenderTarget* rt = TRACKED_NEW plRenderTarget(flags, size, size, bitDepth, zDepth, stencilDepth);

	char buff[256];
	sprintf(buff, "%s_%s", GetKey()->GetName(), name);
	hsgResMgr::ResMgr()->NewKey(buff, rt, GetKey()->GetUoid().GetLocation());

	return rt;
}

plLayer* plWaveSet7::ICreateTotalLayer(plBitmap* bm, hsGMaterial* mat, int which, const char* suff)
{
	plLayer* layer = mat->GetNumLayers() > which ? plLayer::ConvertNoRef(mat->GetLayer(which)->BottomOfStack()) : nil;
	if( !layer )
	{
		layer = TRACKED_NEW plLayer;
		char buff[256];
		sprintf(buff, "%s_%sLayerPS_%d", GetKey()->GetName(), suff, which);
		hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

		layer->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
		layer->SetRuntimeColor(State().fWaterTint);
		layer->SetOpacity(1.f);

		mat->AddLayerViaNotify(layer);
	}

	layer->SetBlendFlags(which ? hsGMatState::kBlendAddSigned : hsGMatState::kBlendAlpha);
	layer->SetZFlags(which ? hsGMatState::kZNoZWrite : 0);
	layer->SetShadeFlags(hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade);
	layer->SetClampFlags(0);
	layer->SetMiscFlags(hsGMatState::kMiscTwoSided);

	layer->SetUVWSrc(which);

	// Set up the transform.
	hsMatrix44 xfm;
	xfm.Reset();

	layer->SetTransform(xfm);

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(bm->GetKey(), refMsg, plRefFlags::kActiveRef);

	return layer;
}

plLayer* plWaveSet7::ICreateTotalEnvLayer(plBitmap* envMap, hsGMaterial* mat, int which, const char* pref)
{
	plLayer* layer = TRACKED_NEW plLayer;
	char buff[256];
	sprintf(buff, "%s_%s_%s_%d", GetKey()->GetName(), pref, "EnvLayerPS", which);
	hsgResMgr::ResMgr()->NewKey(buff, layer, GetKey()->GetUoid().GetLocation());

	layer->SetBlendFlags(which ? hsGMatState::kBlendAddSigned : 0);
	layer->SetZFlags(which ? hsGMatState::kZNoZWrite : 0);
	layer->SetShadeFlags(hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade);
	layer->SetClampFlags(0);
	layer->SetMiscFlags(0);

	layer->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	layer->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	layer->SetOpacity(1.f);

	layer->SetUVWSrc(which);

	// Set up the transform.
	hsMatrix44 xfm;
	xfm.Reset();

	layer->SetTransform(xfm);

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);
	hsgResMgr::ResMgr()->SendRef(envMap->GetKey(), refMsg, plRefFlags::kActiveRef);

	mat->AddLayerViaNotify(layer);

	return layer;
}

hsGMaterial* plWaveSet7::ICreateFixedMatPS(hsGMaterial* mat, const int numUVWs)
{
	hsAssert(mat, "Nil mat should have been filtered out before calling this");

	// Check if we've already done this one.
	if( (mat->GetNumLayers() == 4)
		&&(mat->GetLayer(3) == fFixedLayers[3]) )
		return mat;

	// First, strip off whatever's on there now.
	// If this is the 
	int i;
	for( i = mat->GetNumLayers()-1; i > 0; i-- )
	{
		plMatRefMsg* refMsg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnRemove, i, plMatRefMsg::kLayer);
		hsgResMgr::ResMgr()->SendRef(mat->GetLayer(i)->GetKey(), refMsg, plRefFlags::kActiveRef);
	}

	plRenderTarget* rt = ICreateTransferRenderTarget("CompRT", kCompositeSize);

	if( mat->GetNumLayers() && mat->GetLayer(0) )
	{
		fState.fWaterTint = mat->GetLayer(0)->GetRuntimeColor();
	}

	fFixedLayers[0] = ICreateTotalLayer(rt, mat, 0, "Fix");
	fFixedLayers[1] = ICreateTotalLayer(rt, mat, 1, "Fix");
	fFixedLayers[2] = ICreateTotalLayer(rt, mat, 2, "Fix");
	fFixedLayers[3] = ICreateTotalEnvLayer(fEnvMap, mat, 3, "Fix");


	fBumpReq = ICreateRenderRequest(rt, fBumpDraw, -100.f);
	fBumpReqMsg = TRACKED_NEW plRenderRequestMsg(GetKey(), fBumpReq);

	IAddFixedVertexShader(mat, numUVWs);
	IAddFixedPixelShader(mat);
	
	return mat;
}

void plWaveSet7::ICreateFixedMat(hsGMaterial* mat, const int numUVWs)
{
	if( !mat )
		return;

	if( !fEnvMap )
	{
		plDynamicEnvMap* env = TRACKED_NEW plDynamicEnvMap((UInt16)fEnvSize, (UInt16)fEnvSize, 32);
		hsgResMgr::ResMgr()->NewKey(GetKey()->GetName(), env, GetKey()->GetUoid().GetLocation());
		fEnvMap = env;
		env->SetPosition(hsPoint3(0, 0, 50.f));
		env->SetPosition(State().fEnvCenter);
		env->SetYon(10000.f);
		env->SetRefreshRate(State().fEnvRefresh);
		env->Init();
		env->ReRender();
	}

	ICreateBumpLayersPS();
	ICreateFixedMatPS(mat, numUVWs);
}


void plWaveSet7::IAddShoreVertexShader(hsGMaterial* mat)
{
	if( !fShoreVShader )
	{

		plShader* vShader = TRACKED_NEW plShader;

		char buff[256];
		sprintf(buff, "%s_ShoreVS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
		vShader->SetIsPixelShader(false);
		
		vShader->SetNumConsts(plShoreVS::kNumConsts);

		vShader->SetInputFormat(1); // This should really be one!!!
		vShader->SetOutputFormat(0);

		vShader->SetVector(plShoreVS::kSinConsts, 1.f, -1.f/6.f, 1.f/120.f, -1.f/5040.f);
		vShader->SetVector(plShoreVS::kCosConsts, 1.f, -1.f/2.f, 1.f/24.f, -1.f/720.f);
		vShader->SetVector(plShoreVS::kPiConsts, 1.f / (8.f*hsScalarPI*4.f*4.f), hsScalarPI/2.f, hsScalarPI, hsScalarPI*2.f);
		vShader->SetVector(plShoreVS::kNumericConsts, 0, 0.5f, 1.f, 2.f);

		plConst(hsScalar) kK1(0.5f);
		plConst(hsScalar) kK2(1.5f);
		hsScalar negK1OverK2Sq = -kK1 / (kK2 * kK2);
		vShader->SetVector(plShoreVS::kIncline, negK1OverK2Sq, kK1, 0.f, 0.f);

		vShader->SetNumPipeConsts(5);
		vShader->SetPipeConst(0, plPipeConst::kWorldToNDC, plShoreVS::kWorldToNDC);
		vShader->SetPipeConst(1, plPipeConst::kTex3x4_0, plShoreVS::kTex0Transform);
		vShader->SetPipeConst(2, plPipeConst::kLocalToWorld, plShoreVS::kLocalToWorld);
		vShader->SetPipeConst(3, plPipeConst::kLayRuntime, plShoreVS::kShoreTint);
		vShader->SetPipeConst(4, plPipeConst::kFogSet, plShoreVS::kFogSet);

//		vShader->SetShaderFileName("sha/vs_Shore.inl");
//		vShader->SetShaderFileName("sha/vs_ShoreSteep.inl");
//		vShader->SetShaderFileName("sha/vs_ShoreSucks.inl");
//		vShader->SetShaderFileName("sha/vs_ShoreLeave.inl");
//		vShader->SetShaderFileName("sha/vs_ShoreLeave6.inl");
//		vShader->SetDecl(plShaderTable::Decl(vs_ShoreLeave6));

//		static const plShaderDecl decl("sha/vs_ShoreLeave7.inl");
//		vShader->SetDecl(&decl);
		vShader->SetDecl(plShaderTable::Decl(vs_ShoreLeave7));

		hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefShoreVShader), plRefFlags::kActiveRef);

		fShoreVShader = vShader;

	}

	IAddShaderToLayers(mat, 0, mat->GetNumLayers()-1, plLayRefMsg::kVertexShader, fShoreVShader);

}

void plWaveSet7::IAddShorePixelShader(hsGMaterial* mat)
{
	if( !fShorePShader )
	{
		plShader* pShader = TRACKED_NEW plShader;

		char buff[256];
		sprintf(buff, "%s_ShorePS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
		pShader->SetIsPixelShader(true);

//		pShader->SetShaderFileName("sha/ps_ShoreSucks.inl");
//		pShader->SetShaderFileName("sha/ps_ShoreLeave6.inl");
		pShader->SetDecl(plShaderTable::Decl(ps_ShoreLeave6));

		hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefShorePShader), plRefFlags::kActiveRef);

		fShorePShader = pShader;
	}

	IAddShaderToLayers(mat, 0, mat->GetNumLayers()-1, plLayRefMsg::kPixelShader, fShorePShader);
}

void plWaveSet7::IAddFixedVertexShader(hsGMaterial* mat, const int numUVWs)
{
	if( !fFixedVShader )
	{

		plShader* vShader = TRACKED_NEW plShader;

		char buff[256];
		sprintf(buff, "%s_FixedVS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
		vShader->SetIsPixelShader(false);
		
		vShader->SetNumConsts(plFixedVS7::kNumConsts);

		vShader->SetInputFormat(numUVWs);
		vShader->SetOutputFormat(0);

		vShader->SetVector(plFixedVS7::kSinConsts, 1.f, -1.f/6.f, 1.f/120.f, -1.f/5040.f);
		vShader->SetVector(plFixedVS7::kCosConsts, 1.f, -1.f/2.f, 1.f/24.f, -1.f/720.f);
		vShader->SetVector(plFixedVS7::kPiConsts, 1.f / (8.f*hsScalarPI*4.f*4.f), hsScalarPI/2.f, hsScalarPI, hsScalarPI*2.f);
		vShader->SetVector(plFixedVS7::kNumericConsts, 0, 0.5f, 1.f, 2.f);

		vShader->SetNumPipeConsts(5);
		vShader->SetPipeConst(0, plPipeConst::kWorldToNDC, plFixedVS7::kWorldToNDC);
		vShader->SetPipeConst(1, plPipeConst::kCamPosWorld, plFixedVS7::kCameraPos);
		vShader->SetPipeConst(2, plPipeConst::kLocalToWorld, plFixedVS7::kLocalToWorld);
		vShader->SetPipeConst(3, plPipeConst::kLayRuntime, plFixedVS7::kWaterTint);
		vShader->SetPipeConst(4, plPipeConst::kFogSet, plFixedVS7::kFogSet);


//		static const plShaderDecl decl("sha/vs_WaveFixedFin7.inl");
//		vShader->SetDecl(&decl);
		vShader->SetDecl(plShaderTable::Decl(vs_WaveFixedFin7));


//		vShader->SetShaderFileName("sha/vs_WaveFixedFin6.inl");
//		vShader->SetShaderFileName("sha/vs_WaveFixedFin.inl");
//		vShader->SetShaderFileName("sha/vs_TestPos.inl");

		hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefFixedVShader), plRefFlags::kActiveRef);

		fFixedVShader = vShader;

	}

	IAddShaderToLayers(mat, 0, mat->GetNumLayers()-1, plLayRefMsg::kVertexShader, fFixedVShader);

}

// type is either plLayRefMsg::kVertexShader or plLayRefMsg::kPixelShader.
void plWaveSet7::IAddShaderToLayers(hsGMaterial* mat, int iFirst, int iLast, UInt8 type, plShader* shader)
{
	if( iFirst < 0 )
		iFirst = 0;
	if( UInt32(iLast) >= mat->GetNumLayers() )
		iLast = mat->GetNumLayers()-1;
	int i;
	for( i = iFirst; i <= iLast; i++ )
	{
		plLayer* layer = plLayer::ConvertNoRef(mat->GetLayer(i)->BottomOfStack());
		if( layer 
			&& (layer->GetVertexShader() != shader) 
			&& (layer->GetPixelShader() != shader) )
		{
			plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(layer->GetKey(), plRefMsg::kOnCreate, 0, type);	
			hsgResMgr::ResMgr()->SendRef(shader->GetKey(), refMsg, plRefFlags::kActiveRef);

//			layer->SetShadeFlags(layer->GetShadeFlags() | hsGMatState::kShadeReallyNoFog);
		}
	}
}

void plWaveSet7::IAddFixedPixelShader(hsGMaterial* mat)
{
	if( !fFixedPShader )
	{
		plShader* pShader = TRACKED_NEW plShader;
		char buff[256];
		sprintf(buff, "%s_FixedPS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
		pShader->SetIsPixelShader(true);
		
//		pShader->SetNumConsts(plFixedPS::kNumConsts);
		pShader->SetNumConsts(0);
		
		pShader->SetInputFormat(0);
		pShader->SetOutputFormat(0);
		
//		pShader->SetShaderFileName("sha/ps_WaveFixed.inl");
//		pShader->SetShaderFileName("sha/ps_TestPos.inl");
		pShader->SetDecl(plShaderTable::Decl(ps_WaveFixed));
		
		hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefFixedPShader), plRefFlags::kActiveRef);
		
		fFixedPShader = pShader;
	}
	
	IAddShaderToLayers(mat, 0, mat->GetNumLayers()-1, plLayRefMsg::kPixelShader, fFixedPShader);

}

void plWaveSet7::IAddRipVertexShader(hsGMaterial* mat, const plRipVSConsts& ripConsts)
{
	if( !fRipVShader )
	{
		plShader* vShader = TRACKED_NEW plShader;
		char buff[256];
		sprintf(buff, "%s_RipVS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
		vShader->SetIsPixelShader(false);
		
		vShader->SetInputFormat(1); // This should really be one!!!
		vShader->SetOutputFormat(0);

		vShader->SetNumConsts(plRipVS::kNumConsts);

		vShader->SetVector(plRipVS::kSinConsts, 1.f, -1.f/6.f, 1.f/120.f, -1.f/5040.f);
		vShader->SetVector(plRipVS::kCosConsts, 1.f, -1.f/2.f, 1.f/24.f, -1.f/720.f);
		vShader->SetVector(plRipVS::kPiConsts, 1.f / (8.f*hsScalarPI*4.f*4.f), hsScalarPI/2.f, hsScalarPI, hsScalarPI*2.f);
		vShader->SetVector(plRipVS::kNumericConsts, 0, 0.5f, 1.f, 2.f);

		hsVector3 waterOffset = State().fWaterOffset;
		vShader->SetVector(plRipVS::kWaterLevel, 
			State().fWaterHeight + waterOffset.fX, 
			State().fWaterHeight + waterOffset.fY, 
			State().fWaterHeight + waterOffset.fZ, 
			State().fWaterHeight);

		hsVector3 maxAtten = State().fMaxAtten;
		hsVector3 minAtten = State().fMinAtten;
		hsVector3 depthFalloff = State().fDepthFalloff;
		vShader->SetVector(plRipVS::kDepthFalloff,
			(maxAtten.fX - minAtten.fX) / depthFalloff.fX,
			(maxAtten.fY - minAtten.fY) / depthFalloff.fY,
			(maxAtten.fZ - minAtten.fZ) / depthFalloff.fZ,
			1.f
			);
		vShader->SetVector(plRipVS::kMinAtten,
			minAtten.fX,
			minAtten.fY,
			minAtten.fZ,
			0.f
			);


		// Set up the ones passed in from the dynarippleVSmgr.
		vShader->SetVector(plRipVS::kTexConsts,
			ripConsts.fC1U,
			ripConsts.fC2U,
			ripConsts.fC1V,
			ripConsts.fC2V);

		vShader->SetVector(plRipVS::kLifeConsts,
			ripConsts.fInitAtten,
			0, // current time, we'll fill that in later
			ripConsts.fLife,
			1.f / (ripConsts.fLife - ripConsts.fDecay));

		plConst(hsScalar) kRipBias(0.1);
		vShader->SetVector(plRipVS::kRampBias,
			ripConsts.fRamp,
			1.f / ripConsts.fRamp,
			kRipBias,
			0); // Last one still unused.

		vShader->SetNumPipeConsts(4);
		vShader->SetPipeConst(0, plPipeConst::kWorldToNDC, plRipVS::kWorldToNDC);
		vShader->SetPipeConst(1, plPipeConst::kCamPosWorld, plRipVS::kCameraPos);
		vShader->SetPipeConst(2, plPipeConst::kLocalToWorld, plRipVS::kLocalToWorld);
		vShader->SetPipeConst(3, plPipeConst::kFogSet, plRipVS::kFogSet);

//		vShader->SetShaderFileName("sha/vs_WaveRip.inl");
//		vShader->SetDecl(plShaderTable::Decl(vs_WaveRip));

//		static const plShaderDecl decl("sha/vs_WaveRip7.inl");
//		vShader->SetDecl(&decl);
		vShader->SetDecl(plShaderTable::Decl(vs_WaveRip7));

		hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefRipVShader), plRefFlags::kActiveRef);

		hsAssert(vShader == fRipVShader, "Should have been set by SendRef");
	}

	IAddShaderToLayers(mat, 0, mat->GetNumLayers()-1, plLayRefMsg::kVertexShader, fRipVShader);

}

void plWaveSet7::IAddRipPixelShader(hsGMaterial* mat, const plRipVSConsts& ripConsts)
{
	if( !fRipPShader )
	{
		plShader* pShader = TRACKED_NEW plShader;
		char buff[256];
		sprintf(buff, "%s_RipPS", GetKey()->GetName());
		hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
		pShader->SetIsPixelShader(true);
		
		pShader->SetNumConsts(0);

		pShader->SetInputFormat(0);
		pShader->SetOutputFormat(0);

//		pShader->SetShaderFileName("sha/ps_WaveRip.inl");
		pShader->SetDecl(plShaderTable::Decl(ps_WaveRip));

		hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefRipPShader), plRefFlags::kActiveRef);

		hsAssert(pShader == fRipPShader, "Should have been set by SendRef");
	}

	IAddShaderToLayers(mat, 0, mat->GetNumLayers()-1, plLayRefMsg::kPixelShader, fRipPShader);

}

plShader* plWaveSet7::ICreateDecalVShader(DecalVType t)
{
	if( !fDecalVShaders[t] )
	{
		static const char* fname[kNumDecalVShaders] = {
			"vs_WaveDec1Lay_7",
			"vs_WaveDec2Lay11_7",
			"vs_WaveDec2Lay12_7",
			"vs_WaveDecEnv_7"
		};
		static const plShaderDecl shaderDecls[kNumDecalVShaders] = {
			plShaderDecl("sha/vs_WaveDec1Lay_7.inl"),
			plShaderDecl("sha/vs_WaveDec2Lay11_7.inl"),
			plShaderDecl("sha/vs_WaveDec2Lay12_7.inl"),
			plShaderDecl("sha/vs_WaveDecEnv_7.inl")
		};

		static const plShaderID::ID shaderIDs[kNumDecalVShaders] = {
			vs_WaveDec1Lay_7,
			vs_WaveDec2Lay11_7,
			vs_WaveDec2Lay12_7,
			vs_WaveDecEnv_7
		};
		static const int numUVWs[kNumDecalVShaders] = {
			1,
			1,
			2,
			3
		};
		static const int numLayXfms[kNumDecalVShaders] = {
			1,
			2,
			2,
			1
		};


		plShader* vShader = TRACKED_NEW plShader;
		char buff[256];
		sprintf(buff, "%s_%s", GetKey()->GetName(), fname[t]);
		hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
		vShader->SetIsPixelShader(false);
		
		vShader->SetInputFormat(numUVWs[t]); // This should really be one!!!
		vShader->SetOutputFormat(0);

		vShader->SetNumConsts(plWaveDecVS::kNumConsts);

		vShader->SetVector(plWaveDecVS::kSinConsts, 1.f, -1.f/6.f, 1.f/120.f, -1.f/5040.f);
		vShader->SetVector(plWaveDecVS::kCosConsts, 1.f, -1.f/2.f, 1.f/24.f, -1.f/720.f);
		vShader->SetVector(plWaveDecVS::kPiConsts, 1.f / (8.f*hsScalarPI*4.f*4.f), hsScalarPI/2.f, hsScalarPI, hsScalarPI*2.f);
		vShader->SetVector(plWaveDecVS::kNumericConsts, 0, 0.5f, 1.f, 2.f);

		hsVector3 waterOffset = State().fWaterOffset;
		vShader->SetVector(plWaveDecVS::kWaterLevel, 
			State().fWaterHeight + waterOffset.fX, 
			State().fWaterHeight + waterOffset.fY, 
			State().fWaterHeight + waterOffset.fZ, 
			State().fWaterHeight);

		hsVector3 maxAtten = State().fMaxAtten;
		hsVector3 minAtten = State().fMinAtten;
		hsVector3 depthFalloff = State().fDepthFalloff;
		vShader->SetVector(plWaveDecVS::kDepthFalloff,
			(maxAtten.fX - minAtten.fX) / depthFalloff.fX,
			(maxAtten.fY - minAtten.fY) / depthFalloff.fY,
			(maxAtten.fZ - minAtten.fZ) / depthFalloff.fZ,
			1.f
			);
		vShader->SetVector(plWaveDecVS::kMinAtten,
			minAtten.fX,
			minAtten.fY,
			minAtten.fZ,
			0.f
			);

		plConst(hsScalar) kBias(0.1);
		vShader->SetVector(plWaveDecVS::kBias,
			kBias,
			0,
			0,
			0); // Last one still unused.

		const int kNumPipe = 5;
		vShader->SetNumPipeConsts(kNumPipe + numLayXfms[t]);
		vShader->SetPipeConst(0, plPipeConst::kWorldToNDC, plWaveDecVS::kWorldToNDC);
		vShader->SetPipeConst(1, plPipeConst::kLocalToWorld, plWaveDecVS::kLocalToWorld);
		vShader->SetPipeConst(2, plPipeConst::kLayRuntime, plWaveDecVS::kMatColor);
		vShader->SetPipeConst(3, plPipeConst::kCamPosWorld, plWaveDecVS::kCameraPos);
		vShader->SetPipeConst(4, plPipeConst::kFogSet, plWaveDecVS::kFogSet);
		int i;
		for( i = 0; i < numLayXfms[t]; i++ )
		{
			vShader->SetPipeConst(kNumPipe + i, plPipeConst::Type(plPipeConst::kTex2x4_0+i), plWaveDecVS::kTex0Transform + i);
		}

//		vShader->SetDecl(&shaderDecls[t]);
		vShader->SetDecl(plShaderTable::Decl(shaderIDs[t]));

		hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, t, kRefDecVShader), plRefFlags::kActiveRef);

		hsAssert(vShader == fDecalVShaders[t], "Should have been set by SendRef");
	}
	return fDecalVShaders[t];
}

plShader* plWaveSet7::IGetDecalVShader(hsGMaterial* mat)
{
	if( mat->GetLayer(0)->GetShadeFlags() & hsGMatState::kShadeEnvironMap )
		return ICreateDecalVShader(kDecalVEnv);

	switch( mat->GetNumLayers() )
	{
	case 1:
		return ICreateDecalVShader(kDecalV1Lay);
	case 2:
		hsAssert(mat->GetLayer(0)->GetUVWSrc() == 0, "First layer must use uvw channel 1");
		hsAssert(mat->GetLayer(1)->GetUVWSrc() < 2, "Second layer must use uvw channel 1 or 2");
		switch( mat->GetLayer(1)->GetUVWSrc() )
		{
		case 0:
			return ICreateDecalVShader(kDecalV2Lay11);
		case 1:
			return ICreateDecalVShader(kDecalV2Lay12);
		default:
			return nil;
		}
		break;
	default:
		hsAssert(false, "Only 1 or 2 layers currently supported");
	}
	return nil;
}

plShader* plWaveSet7::ICreateDecalPShader(DecalPType t)
{
	if( !fDecalPShaders[t] )
	{
		static const char* fname[kNumDecalPShaders] = {
			"ps_CbaseAbase",
			"ps_CalphaAbase",
			"ps_CalphaAMult",
			"ps_CalphaAadd",
			"ps_CaddAbase",
			"ps_CaddAMult",
			"ps_CaddAAdd",
			"ps_CmultAbase",
			"ps_CmultAMult",
			"ps_CmultAAdd",
			"ps_WaveDecEnv"
		};
		static const plShaderID::ID shaderIDs[kNumDecalPShaders] = {
			ps_CbaseAbase,
			ps_CalphaAbase,
			ps_CalphaAMult,
			ps_CalphaAadd,
			ps_CaddAbase,
			ps_CaddAMult,
			ps_CaddAAdd,
			ps_CmultAbase,
			ps_CmultAMult,
			ps_CmultAAdd,
			ps_WaveDecEnv
		};

		plShader* pShader = TRACKED_NEW plShader;

		char buff[256];
		sprintf(buff, "%s_%s", GetKey()->GetName(), fname[t]);
		hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
		pShader->SetIsPixelShader(true);

//		sprintf(buff, "sha/%s.inl", fname[t]);
//		pShader->SetShaderFileName(buff);
		pShader->SetDecl(plShaderTable::Decl(shaderIDs[t]));

		hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, t, kRefDecPShader), plRefFlags::kActiveRef);

		hsAssert(fDecalPShaders[t] == pShader, "Should have been set by SendRef");
	}
	return fDecalPShaders[t];

}

plShader* plWaveSet7::IGetDecalPShader(hsGMaterial* mat)
{
	if( mat->GetLayer(0)->GetShadeFlags() & hsGMatState::kShadeEnvironMap )
		return ICreateDecalPShader(kDecalPEnv);

	hsAssert(mat->GetNumLayers() < 3, "Only 2 layers supported on water decal");
	if( mat->GetNumLayers() >= 3 )
		return nil;

	if( mat->GetNumLayers() == 1 )
		return ICreateDecalPShader(kDecalPBB);

	DecalPType t = kNumDecalPShaders; // Initialize to illegal value.

	switch( mat->GetLayer(1)->GetBlendFlags() & (hsGMatState::kBlendMask | hsGMatState::kBlendAlphaMult | hsGMatState::kBlendAlphaAdd) )
	{
	case 0:
	case hsGMatState::kBlendTest:
	case hsGMatState::kBlendAlpha:
		t = kDecalPaB;
		break;
	case hsGMatState::kBlendAdd:
		t = kDecalPAB;
		break;
	case hsGMatState::kBlendMult:
		t = kDecalPMB;
		break;
	case hsGMatState::kBlendAlpha | hsGMatState::kBlendAlphaAdd:
		t = kDecalPaA;
		break;
	case hsGMatState::kBlendAdd | hsGMatState::kBlendAlphaAdd:
		t = kDecalPAA;
		break;
	case hsGMatState::kBlendMult | hsGMatState::kBlendAlphaAdd:
		t = kDecalPMA;
		break;
	case hsGMatState::kBlendAlpha | hsGMatState::kBlendAlphaMult:
		t = kDecalPaM;
		break;
	case hsGMatState::kBlendAdd | hsGMatState::kBlendAlphaMult:
		t = kDecalPAM;
		break;
	case hsGMatState::kBlendMult | hsGMatState::kBlendAlphaMult:
		t = kDecalPMM;
		break;

	default:
		hsAssert(false, "Unsupported layer blend mode");
		return nil;
	}

	return ICreateDecalPShader(t);
}

void plWaveSet7::IUpdateShaders(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	IUpdateBiasVShader();

	IUpdateBumpVShader(pipe, l2w, w2l);

	IUpdateBumpPShader(pipe, l2w, w2l);

	IUpdateRipPShader(pipe, l2w, w2l);

	IUpdateRipVShader(pipe, l2w, w2l);

	IUpdateShoreVShader(pipe, l2w, w2l);

	IUpdateFixedVShader(pipe, l2w, w2l);

	IUpdateFixedPShader(pipe, l2w, w2l);

	IUpdateDecVShaders(pipe, l2w, w2l);
}

void plWaveSet7::IUpdateBumpPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	plCONST(Int32)		skip(0);
	int i;
	for( i = 0; i < kNumBumpShaders; i++ )
	{

		int j;
		for( j = 0; j < kBumpPerPass; j++ )
		{
			int iTex = i*kBumpPerPass + j;

			hsVector3 specVec = State().fSpecVec;
			hsScalar scale = 1.f / (hsScalar(kNumBumpShaders) + specVec[State().kNoise]);

			hsScalar maxLen = TexState().fMaxLength * kCompositeSize / State().fRippleScale;
			hsScalar rescale = fTexWaves[iTex].fLen / maxLen;

			hsScalar bias = 0.5f * scale;
			fBumpPShader[i]->SetVector(plBumpPS::kHalfOne, scale, scale, 1.f, 1.f);
			fBumpPShader[i]->SetVector(plBumpPS::kBias, bias, bias, 1.f, 1.f);

			hsScalar layScale = skip & (1 << iTex) ? 0.f : (1.f / hsScalar(kBumpPerPass));
			layScale *= fTexWaveFade[iTex];

			fBumpPShader[i]->SetVector(plBumpPS::kWave0 + j,
				-fTexWaves[iTex].fDirX * layScale,
				-fTexWaves[iTex].fDirY * layScale,
				1.f,
				1.f);
		}
	}

}

void plWaveSet7::IUpdateBumpVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
}

static inline hsScalar IRound(hsScalar f)
{
	return hsScalar(int(f + (f > 0 ? 0.5f : -0.5f)));
}

void plWaveSet7::IUpdateBiasVShader()
{

	if( fBiasVShader )
	{
		// Can't just use GetDelSysSeconds() or else we lose time if we skip a frame render because of high FPS.
		hsScalar dt = fLastTime > 0 ? hsScalar(fCurrTime - fLastTime) : hsTimer::GetDelSysSeconds();
		plConst(hsScalar) kRate(-0.1f);
		hsScalar tx = kRate * dt;
		hsScalar ty = kRate * dt;
		plConst(hsScalar) kScaleU(4.f);
		plConst(hsScalar) kScaleV(1.f);
		tx += fBiasVShader->GetFloat(plBiasVS::kTexU0, 3);
		tx -= hsScalar(int(tx));

		ty += fBiasVShader->GetFloat(plBiasVS::kTexV0, 3);
		ty -= hsScalar(int(ty));

		hsScalar scale = 1.f + (4.f - 1.f) * TexState().fAngleDev/hsScalarPI;

		hsScalar m00 = IRound(fWindDir.fY * scale);
		hsScalar m01 = IRound(fWindDir.fX * scale);
		hsScalar m10 = IRound(-fWindDir.fX * 4.f);
		hsScalar m11 = IRound(fWindDir.fY * 4.f);

		fBiasVShader->SetVector(plBiasVS::kTexU0,
			m00,
			m01,
			0,
			tx);
		fBiasVShader->SetVector(plBiasVS::kTexV0,
			m10,
			m11,
			0,
			ty);

		plConst(hsScalar) kUpperNoiseOffU(0.f);
		plConst(hsScalar) kUpperNoiseOffV(0.3f);
		fBiasVShader->SetVector(plBiasVS::kTexU1,
			m00,
			m01,
			0,
			-tx + kUpperNoiseOffU);
		fBiasVShader->SetVector(plBiasVS::kTexV1,
			m10,
			m11,
			0,
			ty + kUpperNoiseOffV);

		hsVector3 specVec = State().fSpecVec;
		hsScalar biasScale = 0.5f * specVec[State().kNoise] / (hsScalar(kNumBumpShaders) + specVec[State().kNoise]);
		fBiasVShader->SetVector(plBiasVS::kScaleBias,
			biasScale,
			biasScale,
			0.f,
			1.f);
	}
}

void plWaveSet7::IUpdateFixedPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
}

void plWaveSet7::IUpdateRipPShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	// Nothing to do
}

void plWaveSet7::IUpdateRipVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( fRipVShader )
	{
		fRipVShader->SetVector(plRipVS::kFrequency,
			fWorldWaves[0].fFreq,
			fWorldWaves[1].fFreq,
			fWorldWaves[2].fFreq,
			fWorldWaves[3].fFreq);

		fRipVShader->SetVector(plRipVS::kPhase,
			fWorldWaves[0].fPhase,
			fWorldWaves[1].fPhase,
			fWorldWaves[2].fPhase,
			fWorldWaves[3].fPhase);

		fRipVShader->SetVector(plRipVS::kAmplitude,
			fWorldWaves[0].fAmplitude,
			fWorldWaves[1].fAmplitude,
			fWorldWaves[2].fAmplitude,
			fWorldWaves[3].fAmplitude);

		fRipVShader->SetVector(plRipVS::kDirectionX,
			fWorldWaves[0].fDir.fX,
			fWorldWaves[1].fDir.fX,
			fWorldWaves[2].fDir.fX,
			fWorldWaves[3].fDir.fX);

		fRipVShader->SetVector(plRipVS::kDirectionY,
			fWorldWaves[0].fDir.fY,
			fWorldWaves[1].fDir.fY,
			fWorldWaves[2].fDir.fY,
			fWorldWaves[3].fDir.fY);

		fRipVShader->SetVector(plRipVS::kWindRot,
			fWindDir.fY,
			-fWindDir.fX,
			fWindDir.fX,
			0);

		fRipVShader->SetVector(plRipVS::kLengths,
			fWorldWaves[0].fLength,
			fWorldWaves[1].fLength,
			fWorldWaves[2].fLength,
			fWorldWaves[3].fLength);

		fRipVShader->SetFloat(plRipVS::kLifeConsts, 1, float(hsTimer::GetSysSeconds()));
		float normQ[kNumWaves];
		int i; 
		for( i = 0; i < kNumWaves; i++ )
		{
			normQ[i] = GeoState().fChop / (2.f*hsScalarPI * GeoState().fAmpOverLen * kNumWaves);
		}

		fRipVShader->SetVector(plRipVS::kQADirX,
			fWorldWaves[0].fAmplitude * fWorldWaves[0].fDir.fX * normQ[0],
			fWorldWaves[1].fAmplitude * fWorldWaves[1].fDir.fX * normQ[1],
			fWorldWaves[2].fAmplitude * fWorldWaves[2].fDir.fX * normQ[2],
			fWorldWaves[3].fAmplitude * fWorldWaves[3].fDir.fX * normQ[3]);

		fRipVShader->SetVector(plRipVS::kQADirY,
			fWorldWaves[0].fAmplitude * fWorldWaves[0].fDir.fY * normQ[0],
			fWorldWaves[1].fAmplitude * fWorldWaves[1].fDir.fY * normQ[1],
			fWorldWaves[2].fAmplitude * fWorldWaves[2].fDir.fY * normQ[2],
			fWorldWaves[3].fAmplitude * fWorldWaves[3].fDir.fY * normQ[3]);

	}
}

void plWaveSet7::IUpdateDecVShaders(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	int i;
	for( i = 0; i < kNumDecalVShaders; i++ )
		IUpdateDecVShader(i, pipe);
}

void plWaveSet7::IUpdateDecVShader(int t, plPipeline* pipe)
{
	plShader* shader = fDecalVShaders[t];
	if( shader )
	{
		shader->SetVector(plWaveDecVS::kFrequency,
			fWorldWaves[0].fFreq,
			fWorldWaves[1].fFreq,
			fWorldWaves[2].fFreq,
			fWorldWaves[3].fFreq);

		shader->SetVector(plWaveDecVS::kPhase,
			fWorldWaves[0].fPhase,
			fWorldWaves[1].fPhase,
			fWorldWaves[2].fPhase,
			fWorldWaves[3].fPhase);

		shader->SetVector(plWaveDecVS::kAmplitude,
			fWorldWaves[0].fAmplitude,
			fWorldWaves[1].fAmplitude,
			fWorldWaves[2].fAmplitude,
			fWorldWaves[3].fAmplitude);

		shader->SetVector(plWaveDecVS::kDirectionX,
			fWorldWaves[0].fDir.fX,
			fWorldWaves[1].fDir.fX,
			fWorldWaves[2].fDir.fX,
			fWorldWaves[3].fDir.fX);

		shader->SetVector(plWaveDecVS::kDirectionY,
			fWorldWaves[0].fDir.fY,
			fWorldWaves[1].fDir.fY,
			fWorldWaves[2].fDir.fY,
			fWorldWaves[3].fDir.fY);

		shader->SetVector(plWaveDecVS::kLengths,
			fWorldWaves[0].fLength,
			fWorldWaves[1].fLength,
			fWorldWaves[2].fLength,
			fWorldWaves[3].fLength);

		if( t == kDecalVEnv )
		{
			hsPoint3 worldCam = pipe->GetViewTransform().GetCameraToWorld().GetTranslate();

			hsPoint3 envCenter(State().fEnvCenter);

			hsScalar envRadius = State().fEnvRadius;

			hsVector3 camToCen(&envCenter, &worldCam);
			hsScalar G = camToCen.MagnitudeSquared() - envRadius * envRadius;
			shader->SetVectorW(plWaveDecVS::kEnvAdjust, camToCen, G);

		}

		float normQ[kNumWaves];
		int i;
		for( i = 0; i < kNumWaves; i++ )
		{
			normQ[i] = GeoState().fChop / (2.f*hsScalarPI * GeoState().fAmpOverLen * kNumWaves);
		}

		shader->SetVector(plWaveDecVS::kQADirX,
			fWorldWaves[0].fAmplitude * fWorldWaves[0].fDir.fX * normQ[0],
			fWorldWaves[1].fAmplitude * fWorldWaves[1].fDir.fX * normQ[1],
			fWorldWaves[2].fAmplitude * fWorldWaves[2].fDir.fX * normQ[2],
			fWorldWaves[3].fAmplitude * fWorldWaves[3].fDir.fX * normQ[3]);

		shader->SetVector(plWaveDecVS::kQADirY,
			fWorldWaves[0].fAmplitude * fWorldWaves[0].fDir.fY * normQ[0],
			fWorldWaves[1].fAmplitude * fWorldWaves[1].fDir.fY * normQ[1],
			fWorldWaves[2].fAmplitude * fWorldWaves[2].fDir.fY * normQ[2],
			fWorldWaves[3].fAmplitude * fWorldWaves[3].fDir.fY * normQ[3]);

		shader->SetVector(plWaveDecVS::kDirXW,
			fWorldWaves[0].fDir.fX * fWorldWaves[0].fFreq,
			fWorldWaves[1].fDir.fX * fWorldWaves[1].fFreq,
			fWorldWaves[2].fDir.fX * fWorldWaves[2].fFreq,
			fWorldWaves[3].fDir.fX * fWorldWaves[3].fFreq);

		shader->SetVector(plWaveDecVS::kDirYW,
			fWorldWaves[0].fDir.fY * fWorldWaves[0].fFreq,
			fWorldWaves[1].fDir.fY * fWorldWaves[1].fFreq,
			fWorldWaves[2].fDir.fY * fWorldWaves[2].fFreq,
			fWorldWaves[3].fDir.fY * fWorldWaves[3].fFreq);

		shader->SetVector(plWaveDecVS::kWK,
			normQ[0] * fWorldWaves[0].fFreq,
			normQ[1] * fWorldWaves[1].fFreq,
			normQ[2] * fWorldWaves[2].fFreq,
			normQ[3] * fWorldWaves[3].fFreq);

		shader->SetVector(plWaveDecVS::kDirXSqKW,
			fWorldWaves[0].fDir.fX * fWorldWaves[0].fDir.fX * fWorldWaves[0].fFreq * normQ[0],
			fWorldWaves[1].fDir.fX * fWorldWaves[1].fDir.fX * fWorldWaves[1].fFreq * normQ[1],
			fWorldWaves[2].fDir.fX * fWorldWaves[2].fDir.fX * fWorldWaves[2].fFreq * normQ[2],
			fWorldWaves[3].fDir.fX * fWorldWaves[3].fDir.fX * fWorldWaves[3].fFreq * normQ[3]);

		shader->SetVector(plWaveDecVS::kDirXDirYKW,
			fWorldWaves[0].fDir.fX * fWorldWaves[0].fDir.fY * fWorldWaves[0].fFreq * normQ[0],
			fWorldWaves[1].fDir.fX * fWorldWaves[1].fDir.fY * fWorldWaves[1].fFreq * normQ[1],
			fWorldWaves[2].fDir.fX * fWorldWaves[2].fDir.fY * fWorldWaves[2].fFreq * normQ[2],
			fWorldWaves[3].fDir.fX * fWorldWaves[3].fDir.fY * fWorldWaves[3].fFreq * normQ[3]);

		shader->SetVector(plWaveDecVS::kDirYSqKW,
			fWorldWaves[0].fDir.fY * fWorldWaves[0].fDir.fY * fWorldWaves[0].fFreq * normQ[0],
			fWorldWaves[1].fDir.fY * fWorldWaves[1].fDir.fY * fWorldWaves[1].fFreq * normQ[1],
			fWorldWaves[2].fDir.fY * fWorldWaves[2].fDir.fY * fWorldWaves[2].fFreq * normQ[2],
			fWorldWaves[3].fDir.fY * fWorldWaves[3].fDir.fY * fWorldWaves[3].fFreq * normQ[3]);

	}
}

void plWaveSet7::IUpdateShoreVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( fShoreVShader )
	{

		fShoreVShader->SetVector(plShoreVS::kFrequency,
			fWorldWaves[0].fFreq,
			fWorldWaves[1].fFreq,
			fWorldWaves[2].fFreq,
			fWorldWaves[3].fFreq);

		fShoreVShader->SetVector(plShoreVS::kPhase,
			fWorldWaves[0].fPhase,
			fWorldWaves[1].fPhase,
			fWorldWaves[2].fPhase,
			fWorldWaves[3].fPhase);

		fShoreVShader->SetVector(plShoreVS::kAmplitude,
			fWorldWaves[0].fAmplitude,
			fWorldWaves[1].fAmplitude,
			fWorldWaves[2].fAmplitude,
			fWorldWaves[3].fAmplitude);

		fShoreVShader->SetVector(plShoreVS::kDirectionX,
			fWorldWaves[0].fDir.fX,
			fWorldWaves[1].fDir.fX,
			fWorldWaves[2].fDir.fX,
			fWorldWaves[3].fDir.fX);

		fShoreVShader->SetVector(plShoreVS::kDirectionY,
			fWorldWaves[0].fDir.fY,
			fWorldWaves[1].fDir.fY,
			fWorldWaves[2].fDir.fY,
			fWorldWaves[3].fDir.fY);

		hsVector3 waterOffset = State().fWaterOffset;
		fShoreVShader->SetVector(plShoreVS::kWaterLevel, 
			State().fWaterHeight + waterOffset.fX, 
			State().fWaterHeight + waterOffset.fY, 
			State().fWaterHeight + waterOffset.fZ, 
			State().fWaterHeight);

		hsVector3 maxAtten = State().fMaxAtten;
		hsVector3 minAtten = State().fMinAtten;
		hsVector3 depthFalloff = State().fDepthFalloff;
		fShoreVShader->SetVector(plShoreVS::kDepthFalloff,
			(maxAtten.fX - minAtten.fX) / depthFalloff.fX,
			(maxAtten.fY - minAtten.fY) / depthFalloff.fY,
			(maxAtten.fZ - minAtten.fZ) / depthFalloff.fZ,
			1.f
			);
		fShoreVShader->SetVector(plShoreVS::kMinAtten,
			minAtten.fX,
			minAtten.fY,
			minAtten.fZ,
			0.f
			);

		fShoreVShader->SetVector(plShoreVS::kLengths,
			fWorldWaves[0].fLength,
			fWorldWaves[1].fLength,
			fWorldWaves[2].fLength,
			fWorldWaves[3].fLength);

		plConst(hsScalar) kK1(2.f);
		plConst(hsScalar) kK2(5.f);
		hsScalar negK1OverK2Sq = -kK1 / (kK2 * kK2);
		fShoreVShader->SetVector(plShoreVS::kIncline, negK1OverK2Sq, kK1, 0.f, 0.f);

		float normQ[kNumWaves];
		int i;
		for( i = 0; i < kNumWaves; i++ )
		{
			normQ[i] = GeoState().fChop / (2.f*hsScalarPI * GeoState().fAmpOverLen * kNumWaves);
		}

		fShoreVShader->SetVector(plShoreVS::kQADirX,
			fWorldWaves[0].fAmplitude * fWorldWaves[0].fDir.fX * normQ[0],
			fWorldWaves[1].fAmplitude * fWorldWaves[1].fDir.fX * normQ[1],
			fWorldWaves[2].fAmplitude * fWorldWaves[2].fDir.fX * normQ[2],
			fWorldWaves[3].fAmplitude * fWorldWaves[3].fDir.fX * normQ[3]);

		fShoreVShader->SetVector(plShoreVS::kQADirY,
			fWorldWaves[0].fAmplitude * fWorldWaves[0].fDir.fY * normQ[0],
			fWorldWaves[1].fAmplitude * fWorldWaves[1].fDir.fY * normQ[1],
			fWorldWaves[2].fAmplitude * fWorldWaves[2].fDir.fY * normQ[2],
			fWorldWaves[3].fAmplitude * fWorldWaves[3].fDir.fY * normQ[3]);


		if( fTrialUpdate & kRemakeBubble )
			IRefillBubbleShoreTex();
		if( fTrialUpdate & kRemakeEdge )
			IRefillEdgeShoreTex();
	}
}

void plWaveSet7::IUpdateFixedVShader(plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( fFixedVShader )
	{
		fFixedVShader->SetVector(plFixedVS7::kFrequency,
			fWorldWaves[0].fFreq,
			fWorldWaves[1].fFreq,
			fWorldWaves[2].fFreq,
			fWorldWaves[3].fFreq);

		fFixedVShader->SetVector(plFixedVS7::kPhase,
			fWorldWaves[0].fPhase,
			fWorldWaves[1].fPhase,
			fWorldWaves[2].fPhase,
			fWorldWaves[3].fPhase);

		fFixedVShader->SetVector(plFixedVS7::kAmplitude,
			fWorldWaves[0].fAmplitude,
			fWorldWaves[1].fAmplitude,
			fWorldWaves[2].fAmplitude,
			fWorldWaves[3].fAmplitude);

		fFixedVShader->SetVector(plFixedVS7::kDirectionX,
			fWorldWaves[0].fDir.fX,
			fWorldWaves[1].fDir.fX,
			fWorldWaves[2].fDir.fX,
			fWorldWaves[3].fDir.fX);

		fFixedVShader->SetVector(plFixedVS7::kDirectionY,
			fWorldWaves[0].fDir.fY,
			fWorldWaves[1].fDir.fY,
			fWorldWaves[2].fDir.fY,
			fWorldWaves[3].fDir.fY);

		plCONST(hsScalar) kEnvRadius(500.f);
		hsScalar envRadius = State().fEnvRadius;

		hsPoint3 worldCam = pipe->GetViewTransform().GetCameraToWorld().GetTranslate();

		hsPoint3 envCenter(State().fEnvCenter);

		hsVector3 camToCen(&envCenter, &worldCam);
		hsScalar G = camToCen.MagnitudeSquared() - envRadius * envRadius;
		fFixedVShader->SetVectorW(plFixedVS7::kEnvAdjust, camToCen, G);

		hsScalar texScale = 1.f / State().fRippleScale;

		fFixedVShader->SetVector(plFixedVS7::kUVScale,
			texScale,
			0,
			0,
			0);

		hsScalar specAtten = State().fTexState.fAmpOverLen * hsScalarPI * 2.f;

		plCONST(hsScalar) kScaleHack(0.1f);
		hsScalar baseScale = kScaleHack;
//		baseScale *= hsScalar(kBumpPerPass) * (hsScalar(kNumBumpShaders) + State().fSpecVec[State().kNoise]);
		// Not sure what's right here. but we are currently scaling down by 1/(numBumpShaders + noise),
		// so I guess we want to scale up by that amount here. Not sure we shouldn't figuring in bumpperpass
		// on both, but at least now we're consistent.
		hsVector3 specVec = State().fSpecVec;
		baseScale *= (hsScalar(kNumBumpShaders) + specVec[State().kNoise]);
		baseScale *= (TexState().fChop + 1.f);


		hsScalar specStart = specVec[State().kSpecStart];
		hsScalar specEnd = specVec[State().kSpecEnd];
		if( specStart > specEnd )
			specEnd = specStart + 1.f;
		fFixedVShader->SetVector(plFixedVS7::kSpecAtten,
			-specEnd,
			1.f / (specStart - specEnd),
			baseScale * specAtten,
			0.f);

		hsColorRGBA envTint = State().fSpecularTint;
		if( fFixedLayers[0] && (fFixedLayers[0]->GetShadeFlags() & hsGMatState::kShadeSpecular) )
		{
			envTint *= fFixedLayers[0]->GetSpecularColor();
		}
		fFixedVShader->SetColor(plFixedVS::kEnvTint, envTint);

		fFixedVShader->SetVector(plFixedVS7::kWindRot,
			fWindDir.fY,
			-fWindDir.fX,
			fWindDir.fX,
			0);

		fFixedVShader->SetVector(plFixedVS7::kLengths,
			fWorldWaves[0].fLength,
			fWorldWaves[1].fLength,
			fWorldWaves[2].fLength,
			fWorldWaves[3].fLength);

		// These don't change often, but they can change. We're going
		// to be uploading them to the card anyway, might as well refresh
		// the sysmem shader to be safe.
		hsVector3 waterOffset = State().fWaterOffset;
		fFixedVShader->SetVector(plFixedVS7::kWaterLevel, 
			State().fWaterHeight + waterOffset.fX, 
			State().fWaterHeight + waterOffset.fY, 
			State().fWaterHeight + waterOffset.fZ, 
			State().fWaterHeight);

		hsVector3 maxAtten = State().fMaxAtten;
		hsVector3 minAtten = State().fMinAtten;
		hsVector3 depthFalloff = State().fDepthFalloff;
		fFixedVShader->SetVector(plFixedVS7::kDepthFalloff,
			(maxAtten.fX - minAtten.fX) / depthFalloff.fX,
			(maxAtten.fY - minAtten.fY) / depthFalloff.fY,
			(maxAtten.fZ - minAtten.fZ) / depthFalloff.fZ,
			1.f
			);
		fFixedVShader->SetVector(plFixedVS7::kMinAtten,
			minAtten.fX,
			minAtten.fY,
			minAtten.fZ,
			0.f
			);

		float normQ[kNumWaves];
		int i;
		for( i = 0; i < kNumWaves; i++ )
		{
			normQ[i] = GeoState().fChop / (2.f*hsScalarPI * GeoState().fAmpOverLen * kNumWaves);
		}

		fFixedVShader->SetVector(plFixedVS7::kDirXK,
			fWorldWaves[0].fDir.fX * normQ[0],
			fWorldWaves[1].fDir.fX * normQ[1],
			fWorldWaves[2].fDir.fX * normQ[2],
			fWorldWaves[3].fDir.fX * normQ[3]);

		fFixedVShader->SetVector(plFixedVS7::kDirYK,
			fWorldWaves[0].fDir.fY * normQ[0],
			fWorldWaves[1].fDir.fY * normQ[1],
			fWorldWaves[2].fDir.fY * normQ[2],
			fWorldWaves[3].fDir.fY * normQ[3]);

		fFixedVShader->SetVector(plFixedVS7::kDirXW,
			fWorldWaves[0].fDir.fX * fWorldWaves[0].fFreq,
			fWorldWaves[1].fDir.fX * fWorldWaves[1].fFreq,
			fWorldWaves[2].fDir.fX * fWorldWaves[2].fFreq,
			fWorldWaves[3].fDir.fX * fWorldWaves[3].fFreq);

		fFixedVShader->SetVector(plFixedVS7::kDirYW,
			fWorldWaves[0].fDir.fY * fWorldWaves[0].fFreq,
			fWorldWaves[1].fDir.fY * fWorldWaves[1].fFreq,
			fWorldWaves[2].fDir.fY * fWorldWaves[2].fFreq,
			fWorldWaves[3].fDir.fY * fWorldWaves[3].fFreq);

		fFixedVShader->SetVector(plFixedVS7::kWK,
			normQ[0] * fWorldWaves[0].fFreq,
			normQ[1] * fWorldWaves[1].fFreq,
			normQ[2] * fWorldWaves[2].fFreq,
			normQ[3] * fWorldWaves[3].fFreq);

		fFixedVShader->SetVector(plFixedVS7::kDirXSqKW,
			fWorldWaves[0].fDir.fX * fWorldWaves[0].fDir.fX * fWorldWaves[0].fFreq * normQ[0],
			fWorldWaves[1].fDir.fX * fWorldWaves[1].fDir.fX * fWorldWaves[1].fFreq * normQ[1],
			fWorldWaves[2].fDir.fX * fWorldWaves[2].fDir.fX * fWorldWaves[2].fFreq * normQ[2],
			fWorldWaves[3].fDir.fX * fWorldWaves[3].fDir.fX * fWorldWaves[3].fFreq * normQ[3]);

		fFixedVShader->SetVector(plFixedVS7::kDirXDirYKW,
			fWorldWaves[0].fDir.fX * fWorldWaves[0].fDir.fY * fWorldWaves[0].fFreq * normQ[0],
			fWorldWaves[1].fDir.fX * fWorldWaves[1].fDir.fY * fWorldWaves[1].fFreq * normQ[1],
			fWorldWaves[2].fDir.fX * fWorldWaves[2].fDir.fY * fWorldWaves[2].fFreq * normQ[2],
			fWorldWaves[3].fDir.fX * fWorldWaves[3].fDir.fY * fWorldWaves[3].fFreq * normQ[3]);

		fFixedVShader->SetVector(plFixedVS7::kDirYSqKW,
			fWorldWaves[0].fDir.fY * fWorldWaves[0].fDir.fY * fWorldWaves[0].fFreq * normQ[0],
			fWorldWaves[1].fDir.fY * fWorldWaves[1].fDir.fY * fWorldWaves[1].fFreq * normQ[1],
			fWorldWaves[2].fDir.fY * fWorldWaves[2].fDir.fY * fWorldWaves[2].fFreq * normQ[2],
			fWorldWaves[3].fDir.fY * fWorldWaves[3].fDir.fY * fWorldWaves[3].fFreq * normQ[3]);


	}
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void plWaveSet7::ICheckShoreMaterials()
{
	int i;
	for( i = 0; i < fShores.GetCount(); i++ )
		ICheckShoreMaterial(fShores[i]);
}

void plWaveSet7::ICheckShoreMaterial(plSceneObject* so)
{
	if( !so )
		return;

	const plDrawInterface* di = so->GetDrawInterface();
	if( !di )
		return;

	hsTArray<plAccessSpan> src;
	plAccessGeometry::Instance()->OpenRO(di, src);

	if( !src.GetCount() )
		return;

	int i;
	for( i = 0; i < src.GetCount(); i++ )
	{
		ISetupGraphShore(src[i].GetMaterial());
	}

	plAccessGeometry::Instance()->Close(src);

}

void plWaveSet7::ICheckDecalMaterials()
{
	int i;
	for( i = 0; i < fDecals.GetCount(); i++ )
		ICheckDecalMaterial(fDecals[i]);
}

void plWaveSet7::ICheckDecalMaterial(plSceneObject* so)
{
	if( !so )
		return;

	const plDrawInterface* di = so->GetDrawInterface();
	if( !di )
		return;

	hsTArray<plAccessSpan> src;
	plAccessGeometry::Instance()->OpenRO(di, src);

	if( !src.GetCount() )
		return;

	int i;
	for( i = 0; i < src.GetCount(); i++ )
	{
		ISetupDecal(src[i].GetMaterial());
	}

	plAccessGeometry::Instance()->Close(src);

}

void plWaveSet7::ICheckDecalEnvLayers(hsGMaterial* mat)
{
	// Mat needs to be set up as:
	//		Lay0 = bumpmap - uv is lookup into bumpmap
	//		Lay1 = bumpmap - uv is col0 of tangent2local
	//		Lay2 = bumpmap - uv is col1 of tangent2local
	//		Lay3 = fEnvMap - uv is col2 of tangent2local, except we'll ignore it and use the normal instead
	// Note we'll have to transpose that as part of creating tangent2world
	// Easiest way to check whether we've done this one already is checking whether
	// Lay3 == fEnvMap.

	// If we haven't done this already
	if( !fDecalVShaders[kDecalVEnv] || (mat->GetLayer(0)->GetVertexShader() != fDecalVShaders[kDecalVEnv]) )
	{
		plLayer* lay3 = nil;

		plMatRefMsg* refMsg;

		const int numLayers = mat->GetNumLayers();
		int i;
		for( i = numLayers-1; i >= 0; i-- )
		{
			plLayer* lay0 = plLayer::ConvertNoRef(mat->GetLayer(i)->BottomOfStack());
			lay0->SetBlendFlags(hsGMatState::kBlendAddColorTimesAlpha);
//			lay0->SetBlendFlags(hsGMatState::kBlendAlpha);
			lay0->SetMiscFlags(lay0->GetMiscFlags() | hsGMatState::kMiscRestartPassHere);

			// Repeat lay3 as i+1, i+2 and i+3
			// set base blend to timealphaadd

			// If we are just creating lay3, then by creating it we've added (appended) it to mat.
			// Otherwise, we need to add it as i+1.
			if( !lay3 )
			{
				lay3 = ICreateTotalEnvLayer(fEnvMap, mat, 3, "Dec");
				lay3->SetBlendFlags(hsGMatState::kBlendAlpha);
			}
			else
			{
				refMsg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnRequest, i+1, plMatRefMsg::kLayer | plMatRefMsg::kInsert);
				hsgResMgr::ResMgr()->SendRef(lay3->GetKey(), refMsg, plRefFlags::kActiveRef);
			}

			refMsg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnRequest, i+2, plMatRefMsg::kLayer | plMatRefMsg::kInsert);
			hsgResMgr::ResMgr()->SendRef(lay3->GetKey(), refMsg, plRefFlags::kActiveRef);

			refMsg = TRACKED_NEW plMatRefMsg(mat->GetKey(), plRefMsg::kOnRequest, i+3, plMatRefMsg::kLayer | plMatRefMsg::kInsert);
			hsgResMgr::ResMgr()->SendRef(lay3->GetKey(), refMsg, plRefFlags::kActiveRef);
		}
	}
}

void plWaveSet7::ISetupDecal(hsGMaterial* mat)
{
	if( mat->GetLayer(0)->GetShadeFlags() & hsGMatState::kShadeEnvironMap )
	{
		ICheckDecalEnvLayers(mat);
	}

	plShader* vShader = IGetDecalVShader(mat);
	if( mat->GetLayer(0)->GetVertexShader() != vShader )
		IAddShaderToLayers(mat, 0, -1, plLayRefMsg::kVertexShader, vShader);

	int i;
	for( i = 0; i < mat->GetNumLayers(); i++ )
	{
		plLayer* lay = plLayer::ConvertNoRef(mat->GetLayer(i)->BottomOfStack());
		if( lay )
			lay->SetZFlags(lay->GetZFlags() | hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer);
	}

	// The FFP is actually quite adequate for the required blends,
	// but may not stay that way (if we allow fancier blends and what not).
	// So we can either use the Pixel Shaders or FFP, depending on this define.
// #define MF_USE_FFP
#ifndef MF_USE_FFP
	plShader* pShader = IGetDecalPShader(mat);
	if( mat->GetLayer(0)->GetPixelShader() != pShader )
		IAddShaderToLayers(mat, 0, -1, plLayRefMsg::kPixelShader, pShader);
#endif // MF_USE_FFP
}

void plWaveSet7::AddShoreTest(plKey& key)
{
	hsgResMgr::ResMgr()->SendRef(key, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefShore), plRefFlags::kPassiveRef);

	plSceneObject* so = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
	ICheckShoreMaterial(so);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
hsBool plWaveSet7::SetupRippleMat(hsGMaterial* mat, const plRipVSConsts& ripConsts)
{
	// We'll assume that if we set the vertexshader, we set the pixelshader too.
	if( fRipVShader && (mat->GetLayer(0)->GetVertexShader() == fRipVShader) )
		return true;

	int i;
	for( i = 0; i < mat->GetNumLayers(); i++ )
	{
		plLayer* lay = plLayer::ConvertNoRef(mat->GetLayer(i)->BottomOfStack());
		if( lay )
			lay->SetZFlags(lay->GetZFlags() | hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer);
	}

	IAddRipVertexShader(mat, ripConsts);
	IAddRipPixelShader(mat, ripConsts);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

plDrawableSpans* plWaveSet7::ICreateGraphDrawable(plDrawableSpans* drawable, hsGMaterial* mat, int nWid)
{
//	drawable->SetNativeProperty(plDrawable::kPropVolatile, true);

	const int nVerts = nWid * 2;

	hsTArray<hsPoint3> pos;
	hsTArray<hsVector3> norm;

	pos.SetCount(nVerts);
	norm.SetCount(nVerts);

#ifdef TEST_UVWS
	hsTArray<hsPoint3> uvw; // Are we actually ever going to use these uvws?
	uvw.SetCount(nVerts);
#endif // TEST_UVWS

	int i;
	for( i = 0; i < nWid; i++ )
	{
		int iDn = i << 1;
		int iUp = iDn + 1;

		hsScalar delX = hsScalar(i) / hsScalar(nWid-1);

		pos[iDn].fX = delX * 2.f - 1.f;
		pos[iDn].fY = -1.f;
		pos[iDn].fZ = 0.5f;

		norm[iDn].Set(0.f, 0.f, 1.f);

#ifdef TEST_UVWS
		uvw[iDn].fX = delX;
		uvw[iDn].fY = 0.f;
		uvw[iDn].fZ = 0.f;
#endif // TEST_UVWS

		pos[iUp].fX = delX * 2.f - 1.f;
		pos[iUp].fY = 1.f;
		pos[iUp].fZ = 0.5f;

		norm[iUp].Set(1.f, 0.f, 1.f);

#ifdef TEST_UVWS
		uvw[iUp].fX = delX;
		uvw[iUp].fY = 1.f;
		uvw[iUp].fZ = 0.f;
#endif // TEST_UVWS
	}

	const int nTris = (nWid-1) * 2;

	hsTArray<UInt16> idxArr;
	idxArr.SetCount(nTris * 3);

	UInt16* idx = idxArr.AcquireArray();

	int iBase = 0;
	for( i = 0; i < nTris; i += 2 )
	{
		*idx++ = i;
		*idx++ = i + 2;
		*idx++ = i + 1;

		*idx++ = i + 1;
		*idx++ = i + 2;
		*idx++ = i + 3;
	}


	plDrawableGenerator::GenerateDrawable( nVerts, pos.AcquireArray(), norm.AcquireArray(), 
#ifndef TEST_UVWS
														nil, 0, 
#else // TEST_UVWS
														uvw.AcquireArray(), 1,
#endif // TEST_UVWS
														nil, false, nil,
														nTris * 3, idxArr.AcquireArray(), 
														mat, 
														hsMatrix44::IdentityMatrix(), 
														false,
														nil,
														drawable);

	return drawable;
}

plDrawableSpans* plWaveSet7::ICreateEmptyGraphDrawable(const char* name, UInt32 ref, int which)
{
	plDrawableSpans* drawable = TRACKED_NEW plDrawableSpans;
	char buff[256];
	sprintf(buff, "%s_%s_%d", GetKey()->GetName(), name, which);
	hsgResMgr::ResMgr()->NewKey(buff, drawable, GetKey()->GetUoid().GetLocation());

	hsgResMgr::ResMgr()->SendRef(drawable->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, which, (Int8)ref), plRefFlags::kActiveRef);

	return drawable;
}

hsGMaterial* plWaveSet7::ICreateEmptyMaterial(const char* name, UInt32 ref, int which)
{
	hsGMaterial* mat = TRACKED_NEW hsGMaterial;

	char buff[256];
	sprintf(buff, "%s_%s_%d", GetKey()->GetName(), name, which);
	hsgResMgr::ResMgr()->NewKey(buff, mat, GetKey()->GetUoid().GetLocation());

	hsgResMgr::ResMgr()->SendRef(mat->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, which, (Int8)ref), plRefFlags::kActiveRef);

	return mat;
}

plLayer* plWaveSet7::ICreateBlankLayer(const char* name, int suff)
{
	plLayer* lay = TRACKED_NEW plLayer;
	char buff[256];
	sprintf(buff, "%s_%s_%d", GetKey()->GetName(), name, suff);
	hsgResMgr::ResMgr()->NewKey(buff, lay, GetKey()->GetUoid().GetLocation());

	return lay;
}

plMipmap* plWaveSet7::ICreateBlankTex(const char* name, int width, int height, UInt32 ref)
{
	plMipmap* mipMap = TRACKED_NEW plMipmap(
		width, height,
		plMipmap::kARGB32Config,
		1, 
		plMipmap::kUncompressed,
		plMipmap::UncompressedInfo::kRGB8888);

	char buff[256];
	sprintf(buff, "%s_%s", GetKey()->GetName(), name);
	hsgResMgr::ResMgr()->NewKey(buff, mipMap, GetKey()->GetUoid().GetLocation());

	hsgResMgr::ResMgr()->SendRef(mipMap->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, (Int8)ref), plRefFlags::kActiveRef);

	return mipMap;
}

plMipmap* plWaveSet7::ICreateGraphShoreTex(int width, int height)
{
	// GraphShoreLayer has a texture with white color (possibly noised),
	// and a solid alpha.
	// Alpha is constant along U (width).
	// Alpha is 0 at base,
	//		ramps up to opaque
	//		then drops off sharply to transparent just before the top.

	// If we haven't already made one...
	if( !fGraphShoreTex )
	{
		plMipmap* mipMap = ICreateBlankTex("Graph", width, height, kRefGraphShoreTex);

		plConst(hsScalar) kRampFrac(0.4f);
		plConst(hsScalar) kTruncFrac(0.8f);
		const int rampEnd = int(kRampFrac * height + 0.5f);
		int truncEnd = int(kTruncFrac * height);
		if( truncEnd >= (height-1) )
			truncEnd = height-2;
		int j;
		for( j = 0; j < height; j++ )
		{
			UInt32 alpha = 255;
			plConst(int) kRampStart(4);
			if( j <= kRampStart )
			{
				alpha = 0;
			}
			else
			if( j - kRampStart < rampEnd )
			{
				alpha = ((j-kRampStart) * 255) / rampEnd;
			}
			else if( j > truncEnd )
			{
				alpha = 0;
			}
			UInt32 color = (alpha << 24)
					| (0xff << 16)
					| (0xff << 8)
					| 0xff;

			int i;
			for( i = 0; i < width; i++ )
			{
				UInt32* val = mipMap->GetAddr32(i, j);
				*val = color;
			}
		}
	}

	return fGraphShoreTex;
}

void plWaveSet7::IRefillBubbleShoreTex()
{
	plMipmap* mipMap = fBubbleShoreTex;
	hsAssert(mipMap, "Refilling a non-existent bubble texture");

	const int width = mipMap->GetWidth();
	const int height = mipMap->GetHeight();

	// Initialize to white opaque. 
	memset(mipMap->GetAddr32(0,0), 0xff, width*height*sizeof(UInt32));

	plConst(int) kMinNumBub(1024);
	plConst(int) kMaxNumBub(6000);
	const int kNumBub = (int)(kMinNumBub + State().fWispiness * (kMaxNumBub - kMinNumBub));
	int k;
	for( k = 0; k < kNumBub; k++ )
	{
		// Select a random location.
		int iLoc = (int)(fRand.RandZeroToOne() * width);
		int jLoc = (int)(fRand.RandZeroToOne() * height);

		// Select a random radius
		plConst(hsScalar) kMinRad(2.f);
		plConst(hsScalar) kMaxRad(5.0f);
		int radius = int(kMinRad + fRand.RandZeroToOne() * (kMaxRad - kMinRad));
		hsScalar invRadiusSq = 1.f / hsScalar(radius*radius);

		// Carve out a hole.
		int j;
		for( j = -radius; j < radius; j++ )
		{
			int jj = jLoc + j;
			if( jj < 0 )
				jj += height;
			else if( jj >= height )
				jj -= height;

			int i;
			for( i = -radius; i < radius; i++ )
			{
				int ii = iLoc + i;
				if( ii < 0 )
					ii += width;
				else if( ii >= width )
					ii -= width;

				hsScalar f = hsScalar(i*i + j*j) * invRadiusSq;
				if( f > 1.f )
					f = 1.f;
				plConst(hsScalar) kMinAlpha(0.8f);
				plConst(hsScalar) kMaxAlpha(1.f);
				f *= (kMaxAlpha - kMinAlpha);
				f += kMinAlpha;

				UInt32* val = mipMap->GetAddr32(ii, jj);
				UInt32 alpha = (*val) >> 24;
				alpha = UInt32(hsScalar(alpha) * f);
				*val &= 0x00ffffff;
				*val |= (alpha << 24);
			}
		}
	}

	const hsColorRGBA maxColor = State().fMaxColor;
	const hsColorRGBA minColor = State().fMinColor;
	int j;
	for( j = 0; j < height; j++ )
	{
		int i;
		for( i = 0; i < width; i++ )
		{
			UInt32* val = mipMap->GetAddr32(i, j);
			hsColorRGBA col;
			col.FromARGB32(*val);
			hsScalar alpha = col.a;
			col = maxColor - minColor;
			col *= alpha;
			col += minColor;

			*val = col.ToARGB32();

		}
	}
	mipMap->MakeDirty();

	fTrialUpdate &= ~kRemakeBubble;
}

plMipmap* plWaveSet7::ICreateBubbleShoreTex(int width, int height)
{
	// Bubble layer is white in color (or noised).
	// Alpha is just, well, random bubbles.
	// Tile in U and V

	// If we haven't already made one...
	if( !fBubbleShoreTex )
	{
		plMipmap* mipMap = ICreateBlankTex("Bubble", width, height, kRefBubbleShoreTex);

		IRefillBubbleShoreTex();
	}

	return fBubbleShoreTex;
}

void plWaveSet7::IRefillEdgeShoreTex()
{
	plMipmap* mipMap = fEdgeShoreTex;

	const int width = mipMap->GetWidth();
	const int height = mipMap->GetHeight();

	plConst(hsScalar) kCenter(0.8f);
	plConst(hsScalar) kRadius(0.025f);

	const int center = int(kCenter * height);

	const int radius = int(kRadius * height * State().fEdgeRadius);

	const int top = center + radius;
	const int bot = center - radius;
	const hsScalar invRadiusSq = 1.f / hsScalar(radius*radius);

	hsAssert(top < height-1, "Center too high or radius too big");

	const hsScalar maxAlpha = State().fEdgeOpac * 255.9f;
	int j;
	for( j = 0; j < height; j++ )
	{
		UInt32 alpha = 0;
		if( (j > bot)&&(j < top) )
		{
#if 0 // like x^2
			hsScalar a = hsScalar(j-center);
			a *= a;
			a *= invRadiusSq;
			a = 1.f - a;
#elif 1 // like 1/x^2
			hsScalar a = hsScalar(j-center);
			if( a < 0 )
				a = -a;
			a /= hsScalar(radius);
			a = 1.f - a;
			a *= a;
#else // like cos
			hsScalar a = hsScalar(j - center);
			a /= hsScalar(radius);
			a *= hsScalarPI;
			a = hsFastMath::CosInRange(a);
			a += 1.f;
			a *= 0.5f;
#endif

			alpha = UInt32(a * maxAlpha);
		}

		int i;
		for( i = 0; i < width; i++ )
		{
			UInt32* val = mipMap->GetAddr32(i, j);
			*val = (alpha << 24)
				| (alpha << 16)
				| (alpha << 8)
				| (alpha << 0);
		}
	}
	mipMap->MakeDirty();

	fTrialUpdate &= ~kRemakeEdge;
}

plMipmap* plWaveSet7::ICreateEdgeShoreTex(int width, int height)
{
	// Edge layer is solid white color.
	// Alpha is 0 from base almost up to where graph shore texture drops off
	// sharply. There, alpha ramps up and back down. Probably get a better look
	// from sqrt(k - d^2) (elliptical) than a linear ramp up and down.
	// Tile in U, clamp in V.

	// If we haven't already made one...
	if( !fEdgeShoreTex )
	{
		plMipmap* mipMap = ICreateBlankTex("Edge", width, height, kRefEdgeShoreTex);

		IRefillEdgeShoreTex();
	}

	return fEdgeShoreTex;
}

void plWaveSet7::ISetAsTexture(plLayer* lay, plBitmap* tex)
{
	hsAssert(lay && tex, "Trying to set nil texture or nil layer");
	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(lay->GetKey(), plRefMsg::kOnRequest, 0, plLayRefMsg::kTexture);	
	hsgResMgr::ResMgr()->SendRef(tex->GetKey(), refMsg, plRefFlags::kActiveRef);
}

void plWaveSet7::ICreateGraphShoreLayer(hsGMaterial* mat, int iPass)
{
	// GraphShoreLayer has a texture with white color (possibly noised),
	// and a solid alpha.
	// Alpha is constant along U (width).
	// Alpha is 0 at base,
	//		ramps up to opaque
	//		then drops off sharply to transparent just before the top.
	// Might as well make it pretty much 1 dimensional.
	//
	// Tile in U, clamp in V

	plLayer* lay = ICreateBlankLayer("Graph", iPass);

	// Set up it's state.
	// First pass just overwrites, from then on alpha blend onto.
	lay->SetBlendFlags(0);
	lay->SetZFlags(hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite);
	lay->SetShadeFlags(hsGMatState::kShadeReallyNoFog
		| hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	lay->SetClampFlags(hsGMatState::kClampTextureV);
	lay->SetMiscFlags(hsGMatState::kMiscRestartPassHere);

	lay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	lay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	lay->SetOpacity(1.f);

	lay->SetUVWSrc(0);

	// Now set the texture.
	plConst(int) kGraphWidth(1);
	plConst(int) kGraphHeight(512);
	plMipmap* tex = ICreateGraphShoreTex(kGraphWidth, kGraphHeight);
	ISetAsTexture(lay, tex);

	mat->AddLayerViaNotify(lay);

}

// Second layer is the bubbles
void plWaveSet7::ICreateGraphBubbleLayer(hsGMaterial* mat, int iPass)
{
	// Bubble layer is white in color (or noised).
	// Alpha is just, well, random bubbles.
	// Tile in U and V
	plLayer* lay = ICreateBlankLayer("Bubble", iPass);

	// Set up it's state.
	lay->SetBlendFlags(hsGMatState::kBlendAlpha);
	lay->SetZFlags(hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite);
	lay->SetShadeFlags(hsGMatState::kShadeReallyNoFog
		| hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	lay->SetClampFlags(0);
	lay->SetMiscFlags(0);

	lay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	lay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	lay->SetOpacity(1.f);

	lay->SetUVWSrc(0);

	// Now set the texture
	plConst(int) kBubWidth(128);
	plConst(int) kBubHeight(128);
	plMipmap* tex = ICreateBubbleShoreTex(kBubWidth, kBubHeight);
	ISetAsTexture(lay, tex);

	mat->AddLayerViaNotify(lay);
}

// Third layer is the alpha leading edge
void plWaveSet7::ICreateGraphEdgeLayer(hsGMaterial* mat, int iPass)
{
	// Edge layer is solid white color.
	// Alpha is 0 from base almost up to where graph shore texture drops off
	// sharply. There, alpha ramps up and back down. Probably get a better look
	// from sqrt(k - d^2) (elliptical) than a linear ramp up and down.
	// Tile in U, clamp in V.
	plLayer* lay = ICreateBlankLayer("Edge", iPass);

	// Set up it's state.
	lay->SetBlendFlags(hsGMatState::kBlendAlpha);
	lay->SetZFlags(hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite);
	lay->SetShadeFlags(hsGMatState::kShadeReallyNoFog
		| hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	lay->SetClampFlags(hsGMatState::kClampTextureV);
	lay->SetMiscFlags(0);

	lay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
	lay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
	lay->SetOpacity(1.f);

	lay->SetUVWSrc(0);

	// Now set the texture.
	plConst(int) kEdgeWidth(1);
	plConst(int) kEdgeHeight(512);
	plMipmap* tex = ICreateEdgeShoreTex(kEdgeWidth, kEdgeHeight);
	ISetAsTexture(lay, tex);

	mat->AddLayerViaNotify(lay);
}

void plWaveSet7::ICreateGraphShoreMaterials()
{
	int i;
	for( i = 0; i < kGraphShorePasses; i++ )
	{
		// Create our material
		// and send ourselves a ref.
		hsGMaterial* mat = ICreateEmptyMaterial("GraphShoreMat", kRefGraphShoreMat, i);
		
		// GraphShoreMat's are the materials used to generate the shore texture layers
		// which are then used on rendering the shore to the screen.

		// Create 3 layers
		// First layer is the graph shore
		ICreateGraphShoreLayer(mat, i);

		// Second layer is the bubbles
		ICreateGraphBubbleLayer(mat, i);

		// Third layer is the alpha leading edge
		ICreateGraphEdgeLayer(mat, i);

		IAddGraphVShader(mat, i);
		IAddGraphPShader(mat, i);

		IInitGraph(i);

		hsAssert(fGraphShoreMat[i] == mat, "Should have been processed in ICreateEmptyMaterial()");
	}

}

void plWaveSet7::IAddGraphVShader(hsGMaterial* mat, int iPass)
{
	if( !fGraphVShader[iPass] )
	{
		plShader* vShader = TRACKED_NEW plShader;
		char buff[256];
		sprintf(buff, "%s_GraphVS_%d", GetKey()->GetName(), iPass);
		hsgResMgr::ResMgr()->NewKey(buff, vShader, GetKey()->GetUoid().GetLocation());
		vShader->SetIsPixelShader(false);

		vShader->SetNumConsts(plGraphVS::kNumConsts);
		
		vShader->SetVector(plGraphVS::kNumericConsts, 0, 0.5f, 1.f, 2.f);
		vShader->SetVector(plGraphVS::kPiConsts, 1.f / (2.f*hsScalarPI), hsScalarPI/2.f, hsScalarPI, hsScalarPI*2.f);
		vShader->SetVector(plGraphVS::kCosConsts, 1.f, -1.f/2.f, 1.f/24.f, -1.f/720.f);

#ifndef TEST_UVWS
		vShader->SetInputFormat(0);
#else // TEST_UVWS
		vShader->SetInputFormat(1);
#endif // TEST_UVWS
		vShader->SetOutputFormat(0);

//		vShader->SetShaderFileName("sha/vs_WaveGraph.inl");
//		vShader->SetShaderFileName("sha/vs_WaveGraph2.inl");
		vShader->SetDecl(plShaderTable::Decl(vs_WaveGraph2));

		hsgResMgr::ResMgr()->SendRef(vShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, iPass, kRefGraphVShader), plRefFlags::kActiveRef);

		hsAssert(fGraphVShader[iPass] == vShader, "SendRef should have set shader");
	}

	IAddShaderToLayers(mat, 0, 2, plLayRefMsg::kVertexShader, fGraphVShader[iPass]);
}

void plWaveSet7::IAddGraphPShader(hsGMaterial* mat, int iPass)
{
	if( !fGraphPShader[iPass] )
	{
		plShader* pShader = TRACKED_NEW plShader;
		char buff[256];
		sprintf(buff, "%s_GraphPS_%d", GetKey()->GetName(), iPass);
		hsgResMgr::ResMgr()->NewKey(buff, pShader, GetKey()->GetUoid().GetLocation());
		pShader->SetIsPixelShader(true);
		
		pShader->SetNumConsts(plGraphPS::kNumConsts);

		pShader->SetInputFormat(0);
		pShader->SetOutputFormat(0);

//		pShader->SetShaderFileName("sha/ps_WaveGraph.inl");
		pShader->SetDecl(plShaderTable::Decl(ps_WaveGraph));

		hsgResMgr::ResMgr()->SendRef(pShader->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, iPass, kRefGraphPShader), plRefFlags::kActiveRef);

		hsAssert(fGraphPShader[iPass] == pShader, "SendRef should have set shader");
	}

	IAddShaderToLayers(mat, 0, 2, plLayRefMsg::kPixelShader, fGraphPShader[iPass]);
}

plRenderTarget* plWaveSet7::ISetupGraphShoreRenderReq(int which)
{
	plConst(int) kGraphSize(256);
	char name[256];
	sprintf(name, "Graph_%d", which);
	plRenderTarget* rt = ICreateTransferRenderTarget(name, kGraphSize);
	
	hsgResMgr::ResMgr()->SendRef(rt->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, which, kRefGraphShoreRT), plRefFlags::kActiveRef);

	fGraphReq[which] = ICreateRenderRequest(rt, fGraphShoreDraw[which], -100.f);
	fGraphReqMsg[which] = TRACKED_NEW plRenderRequestMsg(GetKey(), fGraphReq[which]);

	return rt;
}

void plWaveSet7::ISetupGraphShore(hsGMaterial* mat)
{
	if( !fGraphShoreRT[0] )
	{
		// Create the material
		ICreateGraphShoreMaterials();

		int i;
		for( i = 0; i < kGraphShorePasses; i++ )
		{
			// Create the mesh
			plDrawableSpans* drawable = ICreateEmptyGraphDrawable("GraphShore", kRefGraphShoreDraw, i);
			plConst(int) kGraphWidth(256);
			fGraphShoreDraw[i] = ICreateGraphDrawable(drawable, fGraphShoreMat[i], kGraphWidth);

			// Setup render requests
			// Return value is the texture we want to use for the shore line.
			fGraphShoreRT[i] = ISetupGraphShoreRenderReq(i);
		}

	}

	// If we've already done this material, we're done (it may be shared
	// with another shore mesh).
	if( fShoreVShader && mat->GetLayer(0) && (fShoreVShader == mat->GetLayer(0)->GetVertexShader()) )
		return;

	// We now have all our render target textures. Set them up as
	// our kGraphShorePasses layers on this material.
	ISetupShoreLayers(mat);

}

void plWaveSet7::IMakeShoreLayer(hsGMaterial* mat, int which)
{
	char name[512];
	if( which >= mat->GetNumLayers() )
	{
		plLayer* lay = TRACKED_NEW plLayer;
		sprintf(name, "%s_lay_%d", mat->GetKey()->GetName(), which);
		hsgResMgr::ResMgr()->NewKey(name, lay, GetKey()->GetUoid().GetLocation());

		lay->SetAmbientColor(hsColorRGBA().Set(0.f, 0.f, 0.f, 1.f));
		lay->SetRuntimeColor(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
		lay->SetOpacity(1.f);

		mat->AddLayerViaNotify(lay);
	}
}

void plWaveSet7::ISetupShoreLayers(hsGMaterial* mat)
{
	// Make sure we have exactly kGraphShorePasses layers on this mat.
	IMakeShoreLayer(mat, 0);
	IMakeShoreLayer(mat, 1);
	IMakeShoreLayer(mat, 2);

	// Make sure the state is correct for each layer.
	// And set the textures up to point at the render targets
	plLayer* lay = plLayer::ConvertNoRef(mat->GetLayer(0)->BottomOfStack());
	hsAssert(lay, "Bad first layer on material %s. Animated?");
//	lay->SetBlendFlags(hsGMatState::kBlendAlpha | hsGMatState::kBlendInvertFinalAlpha | hsGMatState::kBlendAlphaAlways);
	lay->SetBlendFlags(hsGMatState::kBlendAlpha);
	lay->SetClampFlags(hsGMatState::kClampTextureV);
	lay->SetShadeFlags(hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	lay->SetZFlags(hsGMatState::kZNoZWrite | hsGMatState::kZIncLayer);
	lay->SetMiscFlags(hsGMatState::kMiscTwoSided);
	ISetAsTexture(lay, fGraphShoreRT[0]);

	lay->SetUVWSrc(0);

	lay = plLayer::ConvertNoRef(mat->GetLayer(1));
	hsAssert(lay, "Bad second layer on material %s. Animated?");
	lay->SetBlendFlags(hsGMatState::kBlendAlpha);
	lay->SetClampFlags(hsGMatState::kClampTextureV);
	lay->SetShadeFlags(hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	lay->SetZFlags(hsGMatState::kZNoZWrite);
	lay->SetMiscFlags(hsGMatState::kMiscTwoSided);
	ISetAsTexture(lay, fGraphShoreRT[1]);

	lay->SetUVWSrc(1);

	lay = plLayer::ConvertNoRef(mat->GetLayer(2));
	hsAssert(lay, "Bad third layer on material %s. Animated?");
	lay->SetBlendFlags(hsGMatState::kBlendAlpha);
	lay->SetClampFlags(hsGMatState::kClampTextureV);
	lay->SetShadeFlags(hsGMatState::kShadeNoProjectors
		| hsGMatState::kShadeNoShade
		| hsGMatState::kShadeWhite);
	lay->SetZFlags(hsGMatState::kZNoZWrite);
	lay->SetMiscFlags(hsGMatState::kMiscTwoSided);
	ISetAsTexture(lay, fGraphShoreRT[2]);

	lay->SetUVWSrc(2);

	// Add the vertex and pixel shaders.
	IAddShoreVertexShader(mat);
	IAddShorePixelShader(mat);
}

void plWaveSet7::IInitGraph(int iPass)
{
	GraphState& gs = fGraphState[iPass];
	plShader* shader = fGraphVShader[iPass];

	// First the easy stuff.

	// Age starts off 0.
	gs.fAge = 0;

	static int lastOne = 0;

	plConst(hsScalar) kBasePeriod(3.f);
	hsScalar life = State().fPeriod * kBasePeriod * (1.f + fRand.RandZeroToOne()); 
	gs.fInvLife = (1.f + hsScalar(lastOne)/hsScalar(kGraphShorePasses-1)) / life;

	lastOne = !lastOne;

	gs.fUOff = fRand.RandZeroToOne();

	// Now the rest we have to think about a little, and
	// think about four times.
	int i;
	for( i = 0; i < 4; i++ )
	{
		// Okay, phase we don't have to think too hard about,
		// it doesn't matter as long as it's random.
		gs.fPhase[i] = fRand.RandZeroToOne() * 2.f * hsScalarPI;

		// Next up is frequency, but frequency is the hard one.
		// Remember frequency has to preserve tiling, so freq = k * 2 * PI.
		// We'd like to keep at least one big one around all the time,
		// so we'll always put a big one in first, and then a bunch of
		// smaller ones to noise it up nice.
		int k;
		if( !i )
		{
			k = fRand.RandZeroToOne() > 0.5 ? 1 : 2;
		}
		else
		{
			plConst(int) kMinFreq(3);
			plConst(int) kMaxFreq(10);

			k = kMinFreq + int((kMaxFreq-kMinFreq) * fRand.RandZeroToOne());
		}

		// Input will be in range [0..2], so we'll omit the customary 2*PI here.
		gs.fFreq[i] = k * hsScalarPI; 

		// Amplitude depends on freqency, or roughly inversely proportional
		// to frequency (randomized about linear on period).
		// Divide by 4 because that's how many oscillators we have, and they
		// are summed.
		hsScalar period = 1.f / hsScalar(k);
		plConst(hsScalar) kAmpScale(1.f / 4.f / 2.f);
		plConst(hsScalar) kMinPeriodFrac(1.f);
		plConst(hsScalar) kMaxPeriodFrac(2.f);
		period *= kMinPeriodFrac + fRand.RandZeroToOne() * (kMaxPeriodFrac - kMinPeriodFrac);
		period *= kAmpScale;
		gs.fAmp[i] = period;
	}
	// Go ahead and set the ones on the shader that won't be updated.
	shader->SetVector(plGraphVS::kPhase,
		gs.fPhase[0],
		gs.fPhase[1],
		gs.fPhase[2],
		gs.fPhase[3]);
	shader->SetVector(plGraphVS::kFrequency,
		gs.fFreq[0],
		gs.fFreq[1],
		gs.fFreq[2],
		gs.fFreq[3]);

	// Propagate all this to the shader.
	IUpdateGraphShader(0, iPass);
}

void plWaveSet7::IShuffleDownGraphs(int iPass)
{
	int i;
	for( i = iPass+1; i < kGraphShorePasses; i++ )
	{
		fGraphState[i-1] = fGraphState[i];
		fGraphVShader[i-1]->CopyConsts(fGraphVShader[i]);
	}
	IInitGraph(kGraphShorePasses-1);
}

void plWaveSet7::IUpdateGraphShader(hsScalar dt, int iPass)
{
	if( fGraphShoreDraw[iPass] )
	{
		GraphState& gs = fGraphState[iPass];
		plShader* shader = fGraphVShader[iPass];

		gs.fAge += dt;
		hsScalar rads = gs.fAge * gs.fInvLife;
		if( rads >= hsScalarPI )
		{
			// Recycle this one and restart the upper.
			IShuffleDownGraphs(iPass);
		}
		else
		{
			hsScalar sinAge = hsFastMath::SinInRange(rads);

			shader->SetVector(plGraphVS::kAmplitude, 
				gs.fAmp[0] * sinAge, 
				gs.fAmp[1] * sinAge, 
				gs.fAmp[2] * sinAge, 
				gs.fAmp[3] * sinAge);

			// Might want to tint this sometime.
			plConst(hsColorRGBA) kTint(hsColorRGBA().Set(1.f, 1.f, 1.f, 1.f));
			hsColorRGBA tint = kTint;
			tint.a *= sinAge;
			shader->SetColor(plGraphVS::kColor, tint);

			plConst(hsScalar) kCMax(1.f);
			plConst(hsScalar) kCMin(3.f);
			hsScalar cMin = kCMax + (kCMin - kCMax) * State().fFingerLength;
			plConst(hsScalar) k2ndLayerScale(2.f);
			plConst(hsScalar) k2ndLayerVoff(1.5f);
			shader->SetVector(plGraphVS::kUVWConsts,
				(kCMax - cMin) * sinAge + cMin,
				gs.fUOff,
				k2ndLayerVoff,
				k2ndLayerScale);
		}
	}
}

void plWaveSet7::IUpdateGraphShaders(plPipeline* pipe, hsScalar dt)
{
	if( fGraphShoreDraw[0] )
	{
		int i;
		for( i = kGraphShorePasses-1; i >= 0; i-- )
		{
			IUpdateGraphShader(dt, i);
		}
	}
}


