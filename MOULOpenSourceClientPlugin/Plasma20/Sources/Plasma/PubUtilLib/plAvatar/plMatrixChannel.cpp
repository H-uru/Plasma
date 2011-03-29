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

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

// havok (must be first)
//#include <hkmath/quaternion.h>
//#include <hkmath/quaternion.h>

// singular
#include "plMatrixChannel.h"

// local
#include "plQuatChannel.h"
#include "plPointChannel.h"

// global
#include "hsResMgr.h"
#include "plProfile.h"
#include "hsTimer.h"

// other
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../plInterp/plController.h"
#include "../plInterp/plAnimTimeConvert.h"
#include "../plInterp/hsInterp.h"
#include "../plTransform/hsAffineParts.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// PROFILING GIBBLIES
//
/////////////////////////////////////////////////////////////////////////////////////////

plProfile_Extern(AffineValue);
plProfile_Extern(AffineInterp);
plProfile_Extern(AffineBlend);
plProfile_Extern(AffineCompose);
plProfile_Extern(MatrixApplicator);

/////////////////////////////////////////////////////////////////////////////////////////
//
// plMatrixChannel
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------
// -----
plMatrixChannel::plMatrixChannel()
: plAGChannel()
{
}

// dtor ---------------------------
// -----
plMatrixChannel::~plMatrixChannel()
{
}

// Value --------------------------------------------------------
// ------
const hsMatrix44 & plMatrixChannel::Value(double time, bool peek)
{
	return fResult;
}

// AffineValue -----------------------------------------------------------
// ------------
const hsAffineParts & plMatrixChannel::AffineValue(double time, bool peek)
{
	return fAP;
}

// Value --------------------------------------------------------------
// ------
void plMatrixChannel::Value(hsMatrix44 &matrix, double time, bool peek)
{
	matrix = Value(time);
}

// MakeCombine -----------------------------------------------
// ------------
plAGChannel * plMatrixChannel::MakeCombine(plAGChannel *other)
{
	return nil;
}

// MakeBlend ---------------------------------------------------
// ----------
plAGChannel * plMatrixChannel::MakeBlend(plAGChannel * channelB,
										 plScalarChannel * channelBias,
										 int blendPriority)
{
	plMatrixChannel * matChanB = plMatrixChannel::ConvertNoRef(channelB);
	plAGChannel * result = this;		// if the blend fails, we keep our position in the graph

	if (matChanB)
	{
		result = TRACKED_NEW plMatrixBlend(this, matChanB, channelBias, blendPriority);
	}
	return result;
}

// MakeZeroState -----------------------------
// --------------
plAGChannel * plMatrixChannel::MakeZeroState()
{
	return TRACKED_NEW plMatrixConstant(Value(0));
}

// MakeTimeScale --------------------------------------------------------
// --------------
plAGChannel * plMatrixChannel::MakeTimeScale(plScalarChannel *timeSource)
{
	return TRACKED_NEW plMatrixTimeScale(this, timeSource);
}

// Dump -------------------------------------------
// -----
void plMatrixChannel::Dump(int indent, bool optimized, double time)
{
	std::string indentStr;
	for(int i = 0; i < indent; i++)
	{
		indentStr += "- ";
	}
	hsStatusMessageF("%s matChan<%s>", indentStr.c_str(), fName);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plMatrixConstant
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------
// -----
plMatrixConstant::plMatrixConstant()
: plMatrixChannel()
{
}

plMatrixConstant::~plMatrixConstant()
{
}

plMatrixConstant::plMatrixConstant(const hsMatrix44 &value)
{
	Set(value);
}

void plMatrixConstant::Set(const hsMatrix44 &value)
{
	fResult = value;

	gemAffineParts gemParts1;
	decomp_affine(value.fMap, &gemParts1); 
	AP_SET(fAP, gemParts1);
}

void plMatrixConstant::Write(hsStream *stream, hsResMgr *mgr)
{
	plMatrixChannel::Write(stream, mgr);
	fAP.Write(stream);
}

void plMatrixConstant::Read(hsStream *stream, hsResMgr *mgr)
{
	plMatrixChannel::Read(stream, mgr);
	fAP.Read(stream);
	fAP.ComposeMatrix(&fResult);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plMatrixTimeScale
//
// Insert into the graph when you need to change the speed or direction of time
// Also serves as a handy instancing node, since it just passes its data through.
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ------------------------------
// -----
plMatrixTimeScale::plMatrixTimeScale()
: plMatrixChannel(), fTimeSource(nil), fChannelIn(nil)
{
}

// ctor ------------------------------------------------------------------------
// -----
plMatrixTimeScale::plMatrixTimeScale(plMatrixChannel *channel,
									 plScalarChannel *timeSource)
: fChannelIn(channel),
  fTimeSource(timeSource)
{
}

// dtor -------------------------------
// -----
plMatrixTimeScale::~plMatrixTimeScale()
{
}

// IsStoppedAt ----------------------------
// ------------
plMatrixTimeScale::IsStoppedAt(double time)
{
	return fTimeSource->IsStoppedAt(time);
}

// Value ----------------------------------------------------------
// ------
const hsMatrix44 & plMatrixTimeScale::Value(double time, bool peek)
{
	fResult = fChannelIn->Value(fTimeSource->Value(time, peek), peek);

	return fResult;
}

const hsAffineParts & plMatrixTimeScale::AffineValue(double time, bool peek)
{
	fAP = fChannelIn->AffineValue(fTimeSource->Value(time, peek), peek);
	return fAP;
}

// Detach ----------------------------------------------------
// -------
plAGChannel * plMatrixTimeScale::Detach(plAGChannel * detach)
{
	plAGChannel *result = this;

	// HAVE to recurse on the incoming channel in case there are cycles;
	// even if we're detaching this node it might also be further upstream
	fChannelIn = plMatrixChannel::ConvertNoRef(fChannelIn->Detach(detach));

	// If you delete a timescale, it is not replaced with its upstream node;
	// it's just gone.
	if(!fChannelIn || detach == this)
		result = nil;

	if(result != this)
		delete this;

	return result;
}

// Dump ---------------------------------------------
// -----
void plMatrixTimeScale::Dump(int indent, bool optimized, double time)
{
	std::string indentStr;
	for(int i = 0; i < indent; i++)
	{
		indentStr += "- ";
	}
	hsStatusMessageF("%s matTimeScale <%s> at time <%f>", indentStr.c_str(), fName, fTimeSource->Value(time, true));
	fChannelIn->Dump(indent + 1, optimized, time);
	
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// plMatrixBlend
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------
// -----
plMatrixBlend::plMatrixBlend()
: fChannelA(nil),
  fChannelB(nil),
  fChannelBias(nil)
{
}

// ctor ----------------------------------------------------------------------------
// -----
plMatrixBlend::plMatrixBlend(plMatrixChannel * channelA, plMatrixChannel * channelB,
							 plScalarChannel * channelBias, int priority)
: fChannelA(channelA),
  fOptimizedA(channelA),
  fChannelB(channelB),
  fOptimizedB(channelB),
  fChannelBias(channelBias),
  fPriority(priority)
{
}

// dtor -----------------------
// -----
plMatrixBlend::~plMatrixBlend()
{
	fChannelA = nil;
	fChannelB = nil;
	fChannelBias = nil;
}

// MakeBlend --------------------------------------------------
// ----------
plAGChannel * plMatrixBlend::MakeBlend(plAGChannel *newChannel,
									   plScalarChannel *channelBias,
									   int blendPriority)
{
	plMatrixChannel * newMatChan = plMatrixChannel::ConvertNoRef(newChannel);
	plAGChannel *result = this;
	int effectiveBlendPriority = (blendPriority == -1 ? fPriority : blendPriority);

	if(newMatChan)
	{
		if(effectiveBlendPriority >= fPriority)
		{
			// if the new channel has higher priority, just do it.
			result = plMatrixChannel::MakeBlend(newMatChan, channelBias, effectiveBlendPriority);
		} else {
			// we're higher priority: pass to our upstream channel
			fChannelA = plMatrixChannel::ConvertNoRef(fChannelA->MakeBlend(newChannel, channelBias, blendPriority));
			hsAssert(fChannelA, "MakeBlend returned non-matrix channel.");
			// ask our upstream channel to do the blend: it can't be atop us
			// this request will get recursively delegated until the priorities work.
		}
	}
	return result;
}

UInt16 plMatrixBlend::GetPriority() {
	return fPriority;
}

hsBool plMatrixBlend::IsStoppedAt(double time)
{
	hsScalar blend = fChannelBias->Value(time);
	if (blend == 0)
		return fChannelA->IsStoppedAt(time);
	if (blend == 1)
		return fChannelB->IsStoppedAt(time);

	return (fChannelA->IsStoppedAt(time) && fChannelB->IsStoppedAt(time));
}

// Value ------------------------------------------------------
// ------
const hsMatrix44 & plMatrixBlend::Value(double time, bool peek)
{
	const hsAffineParts &parts = AffineValue(time, peek);

	plProfile_BeginTiming(AffineCompose);
	parts.ComposeMatrix(&fResult);
	plProfile_EndTiming(AffineCompose);
	return fResult;
}

// AffineValue ---------------------------------------------------------
// ------------
const hsAffineParts & plMatrixBlend::AffineValue(double time, bool peek)
{
	const hsScalar &blend = fChannelBias->Value(time);
	if(blend == 0) {
		return fOptimizedA->AffineValue(time, peek);
	} else {
		if(blend == 1) {
			return fOptimizedB->AffineValue(time, peek);
		} else {
			const hsAffineParts &apA = fChannelA->AffineValue(time, peek);
			const hsAffineParts &apB = fChannelB->AffineValue(time, peek);

			plProfile_BeginTiming(AffineBlend);
			hsInterp::LinInterp(&apA, &apB, blend, &fAP);
			plProfile_EndTiming(AffineBlend);
		}
	}
	return fAP;
}

// Detach ----------------------------------------------
// -------
plAGChannel * plMatrixBlend::Detach(plAGChannel *remove)
{
	plAGChannel *result = this;

	// it's possible that the incoming channel could reside down *all* of our
	// branches (it's a graph, not a tree,) so we always pass down all limbs
	fChannelBias = plScalarChannel::ConvertNoRef(fChannelBias->Detach(remove));
	fChannelA = plMatrixChannel::ConvertNoRef(fChannelA->Detach(remove));
	fChannelB = plMatrixChannel::ConvertNoRef(fChannelB->Detach(remove));

	if(!fChannelBias)
		result = fChannelA;
	else if(fChannelA && !fChannelB)
		result = fChannelA;
	else if(fChannelB && !fChannelA)
		result = fChannelB;
	else if(!fChannelA && !fChannelB)
		result = nil;

	if(result != this)
		delete this;

	return result;
}

// Optimize -------------------------------------
// ---------
plAGChannel *plMatrixBlend::Optimize(double time)
{
	fOptimizedA = (plMatrixChannel *)fChannelA->Optimize(time);
	fOptimizedB = (plMatrixChannel *)fChannelB->Optimize(time);
	hsScalar blend = fChannelBias->Value(time);
	if(blend == 0.0f)
		return fOptimizedA;
	if(blend == 1.0f)
		return fOptimizedB;
	else
		return this;
}

// Dump -----------------------------------------
// -----
void plMatrixBlend::Dump(int indent, bool optimized, double time)
{
	std::string indentStr;
	for(int i = 0; i < indent; i++)
	{
		indentStr += "- ";
	}
	hsStatusMessageF("%s matBlend<%s>, bias:<%f>", indentStr.c_str(), fName, fChannelBias->Value(time, true));
	if(optimized)
	{
		fOptimizedB->Dump(indent + 1, optimized, time);
		fOptimizedA->Dump(indent + 1, optimized, time);
	} else {
		fChannelB->Dump(indent + 1, optimized, time);
		fChannelA->Dump(indent + 1, optimized, time);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plMatrixControllerChannel
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------------------------
// -----
plMatrixControllerChannel::plMatrixControllerChannel()
: plMatrixChannel(), fController(nil)
{
}

// ctor ---------------------------------------------------------------
// -----
plMatrixControllerChannel::plMatrixControllerChannel(plController *controller,
													 hsAffineParts *parts)
: fController(controller)
{
	fAP = *parts;
}

// dtor -----------------------------------------------
// -----
plMatrixControllerChannel::~plMatrixControllerChannel()
{
	if(fController) {
		delete fController;
		fController = nil;
	}
}

// Value ------------------------------------------------------------------
// ------
const hsMatrix44 & plMatrixControllerChannel::Value(double time, bool peek)
{
	return Value(time, peek, nil);
}

// Value ------------------------------------------------------------------
// ------
const hsMatrix44 & plMatrixControllerChannel::Value(double time, bool peek,
													plControllerCacheInfo *cache)
{
	plProfile_BeginTiming(AffineInterp);
	fController->Interp((hsScalar)time, &fAP, cache);
	plProfile_EndTiming(AffineInterp);

	plProfile_BeginTiming(AffineCompose);
	fAP.ComposeMatrix(&fResult);
	plProfile_EndTiming(AffineCompose);		
	return fResult;
}

// AffineValue ---------------------------------------------------------------------
// ------------
const hsAffineParts & plMatrixControllerChannel::AffineValue(double time, bool peek)
{
	return AffineValue(time, peek, nil);
}

// AffineValue ---------------------------------------------------------------------
// ------------
const hsAffineParts & plMatrixControllerChannel::AffineValue(double time, bool peek,
															 plControllerCacheInfo *cache)
{
	plProfile_BeginTiming(AffineInterp);
	fController->Interp((hsScalar)time, &fAP, cache);
	plProfile_EndTiming(AffineInterp);
	return fAP;
}

// MakeCacheChannel ------------------------------------------------------------
// -----------------
plAGChannel *plMatrixControllerChannel::MakeCacheChannel(plAnimTimeConvert *atc)
{
	plControllerCacheInfo *cache = fController->CreateCache();
	cache->SetATC(atc);
	return TRACKED_NEW plMatrixControllerCacheChannel(this, cache);
}

void plMatrixControllerChannel::Dump(int indent, bool optimized, double time)
{
	std::string indentStr;
	for(int i = 0; i < indent; i++)
	{
		indentStr += "- ";
	}
	hsStatusMessageF("%s MatController<%s>", indentStr.c_str(), fName);
}

// Write -------------------------------------------------------------
// ------
void plMatrixControllerChannel::Write(hsStream *stream, hsResMgr *mgr)
{
	plMatrixChannel::Write(stream, mgr);

	hsAssert(fController, "Trying to write plMatrixControllerChannel with nil controller. File will not be importable.");
	mgr->WriteCreatable(stream, fController);

	fAP.Write(stream);
}

// Read -------------------------------------------------------------
// ------
void plMatrixControllerChannel::Read(hsStream *stream, hsResMgr *mgr)
{
	plMatrixChannel::Read(stream, mgr);

	fController = plController::ConvertNoRef(mgr->ReadCreatable(stream));

	fAP.Read(stream);
}

/////////////////////////////////
// PLMATRIXCONTROLLERCACHECHANNEL
/////////////////////////////////

// CTOR
plMatrixControllerCacheChannel::plMatrixControllerCacheChannel()
: plMatrixChannel(), fControllerChannel(nil), fCache(nil)
{
}

// CTOR(name, controller)
plMatrixControllerCacheChannel::plMatrixControllerCacheChannel(plMatrixControllerChannel *controller, plControllerCacheInfo *cache)
: fControllerChannel(controller), fCache(cache)
{
}

// ~DTOR()
plMatrixControllerCacheChannel::~plMatrixControllerCacheChannel()
{
	delete fCache;
	fControllerChannel = nil;
}

// VALUE(time)
const hsMatrix44 & plMatrixControllerCacheChannel::Value(double time, bool peek)
{
	return fControllerChannel->Value(time, peek, fCache);
}

const hsAffineParts & plMatrixControllerCacheChannel::AffineValue(double time, bool peek)
{
	return fControllerChannel->AffineValue(time, peek, fCache);
}

// DETACH
plAGChannel * plMatrixControllerCacheChannel::Detach(plAGChannel * detach)
{
	plAGChannel *result = this;

	fControllerChannel =
		plMatrixControllerChannel::ConvertNoRef(fControllerChannel->Detach(detach));

	if(detach == this)
		result = fControllerChannel;

	if(!fControllerChannel)
		result = nil;

	if(result != this)
		delete this;

	return result;
}

/////////////////////
// PLQUATPOINTCOMBINE
/////////////////////

// CTOR
plQuatPointCombine::plQuatPointCombine()
: fQuatChannel(nil), fPointChannel(nil)
{
}

// CTOR
plQuatPointCombine::plQuatPointCombine(plQuatChannel *quatChannel, plPointChannel *pointChannel)
: fQuatChannel(quatChannel),
  fPointChannel(pointChannel)
{
}

// DTOR
plQuatPointCombine::~plQuatPointCombine()
{
	if(fQuatChannel) {
	//XXX	delete fQuatChannel;
		fQuatChannel = nil;
	}
	if(fPointChannel) {
	//XXX	delete fPointChannel;
		fPointChannel = nil;
	}
}

// VALUE(time)
const hsMatrix44 & plQuatPointCombine::Value(double time)
{
	if(fQuatChannel)
	{
		const hsQuat &quat = fQuatChannel->Value(time);
		quat.MakeMatrix(&fResult);
	} else {
		fResult.Reset();
	}
	if(fPointChannel)
	{
		const hsPoint3 &point = fPointChannel->Value(time);
		fResult.SetTranslate(&point);
	}
	return fResult;
}

const hsAffineParts & plQuatPointCombine::AffineValue(double time)
{
	// XXX Lame hack to get things to compile for now. 
	// Will fix when we actually start using this channel type.
	gemAffineParts gemParts1;
	decomp_affine(Value(time).fMap, &gemParts1); 
	AP_SET(fAP, gemParts1);

	return fAP;
}

// DETACH
plAGChannel * plQuatPointCombine::Detach(plAGChannel *channel)
{
	hsAssert(this != channel, "Can't detach combiners or blenders directly. Detach sub-channels instead.");
	if(this != channel)
	{
		// *** check the types on the replacement channels to make sure they're compatible
		fQuatChannel = (plQuatChannel *)fQuatChannel->Detach(channel);
		fPointChannel = (plPointChannel *)fPointChannel->Detach(channel);
	}
	return this;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// PLMATRIXCHANNELAPPLICATOR
//
///////////////////////////////////////////////////////////////////////////////////////////

// IAPPLY
void plMatrixChannelApplicator::IApply(const plAGModifier *mod, double time)
{
	if(fChannel)
	{
		plMatrixChannel *matChan = plMatrixChannel::ConvertNoRef(fChannel);

		if(matChan)
		{
			hsMatrix44 inverse;

			plProfile_BeginTiming(AffineValue);
			const hsAffineParts &ap = matChan->AffineValue(time);
			plProfile_EndTiming(AffineValue);
			
			hsMatrix44 result;

			plProfile_BeginTiming(AffineCompose);
			ap.ComposeMatrix(&result);
			ap.ComposeInverseMatrix(&inverse);
			//result.GetInverse(&inverse);
			plProfile_EndTiming(AffineCompose);

			plProfile_BeginTiming(MatrixApplicator);
			plCoordinateInterface *CI = IGetCI(mod);
			CI->SetLocalToParent(result, inverse);
			plProfile_EndTiming(MatrixApplicator);	
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// plMatrixDelayedCorrectionApplicator
//
///////////////////////////////////////////////////////////////////////////////////////////

const hsScalar plMatrixDelayedCorrectionApplicator::fDelayLength = 1.f; // seconds

void plMatrixDelayedCorrectionApplicator::SetCorrection(hsMatrix44 &cor)
{
	if (fIgnoreNextCorrection)
	{
		// We want the first correction we get from an avatar to be
		// instantaneous, otherwise they float over from (0, 0, 0).
		fIgnoreNextCorrection = false;
		return;
	}

	// decomp_affine seems to always give us the smaller angle quaternion,
	// which looks right visually when we interp. If certain cases become
	// visually annoying, we can check and adjust things here.
	gemAffineParts gemParts1;
	decomp_affine(cor.fMap, &gemParts1); 
	AP_SET(fCorAP, gemParts1);

	fDelayStart = hsTimer::GetSysSeconds();
}


// CANBLEND
hsBool plMatrixDelayedCorrectionApplicator::CanBlend(plAGApplicator *app)
{
	plMatrixChannelApplicator *matChannelApp = plMatrixChannelApplicator::ConvertNoRef(app);

	if( plMatrixChannelApplicator::ConvertNoRef(app) )
	{
		return true;
	}
	return false;
}

// IAPPLY
void plMatrixDelayedCorrectionApplicator::IApply(const plAGModifier *mod, double time)
{
	if(fChannel)
	{
		if(fEnabled)
		{
			plMatrixChannel *matChan = plMatrixChannel::ConvertNoRef(fChannel);
			hsAssert(matChan, "Invalid channel given to plMatrixChannelApplicator");

			plProfile_BeginTiming(MatrixApplicator);
			plCoordinateInterface *CI = IGetCI(mod);			
			const hsMatrix44 &animResult = matChan->Value(time);
			hsMatrix44 localResult;
			hsMatrix44 localInverse;

			if (time < fDelayStart + fDelayLength)
			{
				hsAffineParts identAP;
				identAP.Reset();
				hsAffineParts interpAP;
				hsMatrix44 interpResult;

				hsScalar blend = (hsScalar)((time - fDelayStart) / fDelayLength);
				hsInterp::LinInterp(&fCorAP, &identAP, blend, &interpAP);
				interpAP.ComposeMatrix(&interpResult);
				
				localResult = interpResult * animResult;
				localResult.GetInverse(&localInverse);
				CI->SetLocalToParent(localResult, localInverse);
			}
			else
			{
				animResult.GetInverse(&localInverse);
				CI->SetLocalToParent(animResult, localInverse);		
			}
			plProfile_EndTiming(MatrixApplicator);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// PLMATRIXDIFFERENCEAPPLICATOR
//
///////////////////////////////////////////////////////////////////////////////////////////

// Reset -------------------------------------
// ------
void plMatrixDifferenceApp::Reset(double time)
{
	hsAssert(fChannel,"Missing input channel when resetting.");
	if(fChannel)
	{
		plMatrixChannel *matChan = plMatrixChannel::ConvertNoRef(fChannel);
		hsAssert(matChan, "Invalid channel given to plMatrixChannelApplicator");
		if(matChan)
		{
			hsMatrix44 L2A, A2L;
			const hsAffineParts &ap = matChan->AffineValue(time);
			ap.ComposeMatrix(&L2A);				// what comes out of AffineValue is a local-to-animation
			ap.ComposeInverseMatrix(&A2L);

			fLastA2L = A2L;
			fLastL2A = L2A;
		}
	}
}

// CanBlend -----------------------------------------------
// ---------
hsBool plMatrixDifferenceApp::CanBlend(plAGApplicator *app)
{
	plMatrixChannelApplicator *matChannelApp = plMatrixChannelApplicator::ConvertNoRef(app);

	if( plMatrixChannelApplicator::ConvertNoRef(app) )
	{
		return true;
	}
	return false;
}

// *** move this somewhere real
bool CompareMatrices2(const hsMatrix44 &matA, const hsMatrix44 &matB, float tolerance)
{
	bool c00 = fabs(matA.fMap[0][0] - matB.fMap[0][0]) < tolerance;
	bool c01 = fabs(matA.fMap[0][1] - matB.fMap[0][1]) < tolerance;
	bool c02 = fabs(matA.fMap[0][2] - matB.fMap[0][2]) < tolerance;
	bool c03 = fabs(matA.fMap[0][3] - matB.fMap[0][3]) < tolerance;

	bool c10 = fabs(matA.fMap[1][0] - matB.fMap[1][0]) < tolerance;
	bool c11 = fabs(matA.fMap[1][1] - matB.fMap[1][1]) < tolerance;
	bool c12 = fabs(matA.fMap[1][2] - matB.fMap[1][2]) < tolerance;
	bool c13 = fabs(matA.fMap[1][3] - matB.fMap[1][3]) < tolerance;

	bool c20 = fabs(matA.fMap[2][0] - matB.fMap[2][0]) < tolerance;
	bool c21 = fabs(matA.fMap[2][1] - matB.fMap[2][1]) < tolerance;
	bool c22 = fabs(matA.fMap[2][2] - matB.fMap[2][2]) < tolerance;
	bool c23 = fabs(matA.fMap[2][3] - matB.fMap[2][3]) < tolerance;

	bool c30 = fabs(matA.fMap[3][0] - matB.fMap[3][0]) < tolerance;
	bool c31 = fabs(matA.fMap[3][1] - matB.fMap[3][1]) < tolerance;
	bool c32 = fabs(matA.fMap[3][2] - matB.fMap[3][2]) < tolerance;
	bool c33 = fabs(matA.fMap[3][3] - matB.fMap[3][3]) < tolerance;

	return c00 && c01 && c02 && c03 && c11 && c12 && c13 && c20 && c21 && c22 && c23 && c30 && c31 && c32 && c33;
}

// IAPPLY
void plMatrixDifferenceApp::IApply(const plAGModifier *mod, double time)
{
	plMatrixChannel *matChan = plMatrixChannel::ConvertNoRef(fChannel);
	hsAssert(matChan, "Invalid channel given to plMatrixChannelApplicator");
	hsMatrix44 L2A, A2L;

	const hsAffineParts &ap = matChan->AffineValue(time);

	plProfile_BeginTiming(AffineCompose);
	ap.ComposeMatrix(&L2A);				// what comes out of AffineValue is a local-to-animation
	ap.ComposeInverseMatrix(&A2L);
	plProfile_EndTiming(AffineCompose);

	plProfile_BeginTiming(MatrixApplicator);	
	if(fNew)					// if it's new, there's no previous frame to diff against;
	{							// cache the current and don't do anything
		fLastA2L = A2L;
		fLastL2A = L2A;

		fNew = false;
	} else {
		if( ! CompareMatrices2(fLastA2L, A2L, .0001f) && ! CompareMatrices2(fLastL2A, L2A, .0001f))
		{
			plCoordinateInterface *CI = IGetCI(mod);

			hsMatrix44 l2p = CI->GetLocalToParent();
			hsMatrix44 p2l = CI->GetParentToLocal();

			hsMatrix44 prev2Cur = fLastA2L * L2A;	// previous to current in local space
			hsMatrix44 cur2Prev = A2L * fLastL2A;	// current to previous in local space

			hsMatrix44 newL2P = l2p * prev2Cur;
			hsMatrix44 newP2L = cur2Prev * p2l;

			CI->SetLocalToParent(newL2P, newP2L);
			CI->FlushTransform();

			fLastL2A = L2A;
			fLastA2L = A2L;
		}
	}
	plProfile_EndTiming(MatrixApplicator);	
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// PLIK2APPLICATOR
//
///////////////////////////////////////////////////////////////////////////////////////////

/** A two-bone IK applicator.
	*/
class plIK2Applicator : public plMatrixChannelApplicator
{
public:

	// The latest time we were asked to evaluate. We won't actually do the evaluation
	// until the other bone is asked as well.
	double GetUpdateTime();

	void SetIsEndEffector(bool status);
	bool GetIsEndEffector();

	void SetTarget(hsPoint3 &worldPoint);

private:
	virtual void IApply(const plAGModifier *mod, double time);
	void ISolve();
	// The other bone involved in the IK solution
	plIK2Applicator *fOtherBone;
	// The latest time we were asked to evaluate. We won't actually run our
	// process until the other guy is asked to evaluate the same time.
	double	fUpdateTime;

	hsPoint3 fTarget;
	bool fIsEndEffector;
};

// GetUpdateTime ---------------
double plIK2Applicator::GetUpdateTime()
{
	return fUpdateTime;
}

void plIK2Applicator::SetIsEndEffector(bool status)
{
	fIsEndEffector = status;
}

bool plIK2Applicator::GetIsEndEffector()
{
	return fIsEndEffector;
}

void plIK2Applicator::IApply(const plAGModifier *mod, double time)
{
	fUpdateTime = time;
	if(time == fOtherBone->GetUpdateTime())
	{
		// we're both up-to-date: go ahead and solve
		ISolve();
	}
}

void plIK2Applicator::ISolve()
{

}