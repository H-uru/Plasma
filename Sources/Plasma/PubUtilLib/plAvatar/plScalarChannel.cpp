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

// singular
#include "plScalarChannel.h"

// global
#include "hsResMgr.h"

// other
#include "../plGLight/plLightInfo.h"
#include "../plInterp/plController.h"
#include "../plInterp/plAnimTimeConvert.h"
#include "../plSDL/plSDL.h"


/////////////////////////////////////////////////////////////////////////////////////////
//
// plScalarChannel
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------
// -----
plScalarChannel::plScalarChannel()
: plAGChannel()
{
}

// dtor ---------------------------
// -----
plScalarChannel::~plScalarChannel()
{
}

// value --------------------------------------------------------
// ------
const hsScalar & plScalarChannel::Value(double time, hsBool peek)
{
	return fResult;
}

// value --------------------------------------------------------------
// ------
void plScalarChannel::Value(hsScalar &scalar, double time, hsBool peek)
{
	scalar = Value(time, peek);
}

// MakeCombine -----------------------------------------------
// ------------
plAGChannel * plScalarChannel::MakeCombine(plAGChannel *other)
{
	return nil;
}

// MakeBlend ---------------------------------------------------
// ----------
plAGChannel * plScalarChannel::MakeBlend(plAGChannel * channelB,
										 plScalarChannel * channelBias,
										 int blendPriority)
{
	plScalarChannel * chanB = plScalarChannel::ConvertNoRef(channelB);
	plScalarChannel * chanBias = plScalarChannel::ConvertNoRef(channelBias);
	plAGChannel * result = this;

	if (chanB)
	{
		result = TRACKED_NEW plScalarBlend(this, chanB, chanBias);
	} else {
		hsStatusMessage("Blend operation failed.");
	}
	return result;
}

// MakeZeroState -----------------------------
// --------------
plAGChannel * plScalarChannel::MakeZeroState()
{
	return TRACKED_NEW plScalarConstant(Value(0));
}

// MakeTimeScale --------------------------------------------------------
// --------------
plAGChannel * plScalarChannel::MakeTimeScale(plScalarChannel *timeSource)
{
	return TRACKED_NEW plScalarTimeScale(this, timeSource);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plScalarConstant
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------
// -----
plScalarConstant::plScalarConstant()
{
}

// ctor ------------------------------------------
// -----
plScalarConstant::plScalarConstant(hsScalar value)
{
	fResult = value;
}

// dtor -----------------------------
// -----
plScalarConstant::~plScalarConstant()
{
}

void plScalarConstant::Read(hsStream *stream, hsResMgr *mgr)
{
	plScalarChannel::Read(stream, mgr);
	fResult = stream->ReadSwapScalar();
}

void plScalarConstant::Write(hsStream *stream, hsResMgr *mgr)
{
	plScalarChannel::Write(stream, mgr);
	stream->WriteSwapScalar(fResult);
}



////////////////////
// PLSCALARTIMESCALE
////////////////////
// Insert into the graph when you need to change the speed or direction of time
// Also serves as a handy instancing node, since it just passes its data through.

// CTOR
plScalarTimeScale::plScalarTimeScale()
: fTimeSource(nil),
  fChannelIn(nil)
{
}

// CTOR (channel, converter)
plScalarTimeScale::plScalarTimeScale(plScalarChannel *channel, plScalarChannel *timeSource)
: fChannelIn(channel),
  fTimeSource(timeSource)
{
}

// DTOR
plScalarTimeScale::~plScalarTimeScale()
{
}

plScalarTimeScale::IsStoppedAt(double time)
{
	return fTimeSource->IsStoppedAt(time);
}

// VALUE
const hsScalar & plScalarTimeScale::Value(double time, hsBool peek)
{
	fResult = fChannelIn->Value(fTimeSource->Value(time, peek));

	return fResult;
}

// DETACH
plAGChannel * plScalarTimeScale::Detach(plAGChannel * detach)
{
	plAGChannel *result = this;
	
	fChannelIn = plScalarChannel::ConvertNoRef(fChannelIn->Detach(detach));
	
	if(!fChannelIn || detach == this)
		result = nil;
	
	if(result != this)
		delete this;
	
	return result;
	
}


/////////////////////////////////////////////////////////////////////////////////////////
//
// plScalarBlend
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------
// -----
plScalarBlend::plScalarBlend()
: fChannelA(nil),
  fChannelB(nil),
  fChannelBias(nil)
{
}

// ctor ----------------------------------------------------------------------------
// -----
plScalarBlend::plScalarBlend(plScalarChannel * channelA, plScalarChannel * channelB,
							 plScalarChannel * channelBias)
: fChannelA(channelA),
  fChannelB(channelB),
  fChannelBias(channelBias)
{
}

// dtor -----------------------
// -----
plScalarBlend::~plScalarBlend()
{
	fChannelA = nil;
	fChannelB = nil;
	fChannelBias = nil;
}

// IsStoppedAt -------------------------------
// ------------
hsBool plScalarBlend::IsStoppedAt(double time)
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
const hsScalar & plScalarBlend::Value(double time, hsBool peek)
{
	hsScalar curBlend = fChannelBias->Value(time, peek);
	if(curBlend == 0) {
		fChannelA->Value(fResult, time, peek);
	} else {
		if(curBlend == 1) {
			fChannelB->Value(fResult, time, peek);
		} else {
			const hsScalar &scalarA = fChannelA->Value(time, peek);
			const hsScalar &scalarB = fChannelB->Value(time, peek);
			fResult = scalarA + curBlend * (scalarB - scalarA);
		}
	}
	return fResult;
}


// Detach ----------------------------------------------
// -------
plAGChannel * plScalarBlend::Detach(plAGChannel *remove)
{
	plAGChannel *result = this;

	// it's possible that the incoming channel could reside down *all* of our
	// branches (it's a graph, not a tree,) so we always pass down all limbs
	fChannelBias = plScalarChannel::ConvertNoRef(fChannelBias->Detach(remove));
	fChannelA = plScalarChannel::ConvertNoRef(fChannelA->Detach(remove));
	fChannelB = plScalarChannel::ConvertNoRef(fChannelB->Detach(remove));

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

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLSCALARCONTROLLERCHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------------------------
// -----
plScalarControllerChannel::plScalarControllerChannel()
: fController(nil)
{
}

// ctor ----------------------------------------------------------------------------
// -----
plScalarControllerChannel::plScalarControllerChannel(plController *controller)
: fController(controller)
{
}

// dtor -----------------------------------------------
// -----
plScalarControllerChannel::~plScalarControllerChannel()
{
	if(fController) {
		delete fController;
		fController = nil;
	}
}

// Value ------------------------------------------------------------------
// ------
const hsScalar & plScalarControllerChannel::Value(double time, hsBool peek)
{
	return Value(time, peek, nil);
}

// Value ------------------------------------------------------------------
// ------
const hsScalar & plScalarControllerChannel::Value(double time, hsBool peek,
												  plControllerCacheInfo *cache)
{		
	fController->Interp((hsScalar)time, &fResult, cache);
	return fResult;
}

// MakeCacheChannel ------------------------------------------------------------
// -----------------
plAGChannel *plScalarControllerChannel::MakeCacheChannel(plAnimTimeConvert *atc)
{
	plControllerCacheInfo *cache = fController->CreateCache();
	cache->SetATC(atc);
	return TRACKED_NEW plScalarControllerCacheChannel(this, cache);
}

// Write -------------------------------------------------------------
// ------
void plScalarControllerChannel::Write(hsStream *stream, hsResMgr *mgr)
{
	plScalarChannel::Write(stream, mgr);

	hsAssert(fController, "Trying to write plScalarControllerChannel with nil controller. File will not be importable.");
	mgr->WriteCreatable(stream, fController);
}

// Read -------------------------------------------------------------
// -----
void plScalarControllerChannel::Read(hsStream *stream, hsResMgr *mgr)
{
	plScalarChannel::Read(stream, mgr);
	
	fController = plController::ConvertNoRef(mgr->ReadCreatable(stream));
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLSCALARCONTROLLERCACHECHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------------------------------------
// -----
plScalarControllerCacheChannel::plScalarControllerCacheChannel()
: fControllerChannel(nil),
  fCache(nil)
{
}

// ctor ---------------------------------------------------------------------------------------------
// -----
plScalarControllerCacheChannel::plScalarControllerCacheChannel(plScalarControllerChannel *controller,
															   plControllerCacheInfo *cache)
: fControllerChannel(controller),
  fCache(cache)
{
}

// dtor ---------------------------------------------------------
// -----
plScalarControllerCacheChannel::~plScalarControllerCacheChannel()
{
	delete fCache;
	fControllerChannel = nil;
}

// Value ---------------------------------------------------------------------
// ------
const hsScalar & plScalarControllerCacheChannel::Value(double time, bool peek)
{
	return fControllerChannel->Value(time, peek, fCache);
}

// Detach -----------------------------------------------------------------
// -------
plAGChannel * plScalarControllerCacheChannel::Detach(plAGChannel * channel)
{
	plAGChannel *result = this;

	if(channel == this)
	{
		result = nil;
	} else {
		fControllerChannel = plScalarControllerChannel::ConvertNoRef(fControllerChannel->Detach(channel));
		
		if(!fControllerChannel)
			result = nil;
	}
	if(result != this)
		delete this;

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLATCCHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------
plATCChannel::plATCChannel()
: fConvert(nil)
{
}

// ctor ----------------------------------------------
plATCChannel::plATCChannel(plAnimTimeConvert *convert)
: fConvert(convert)
{
}

// dtor ---------------------
plATCChannel::~plATCChannel()
{
}

// IsStoppedAt ------------------------------
// ------------
hsBool plATCChannel::IsStoppedAt(double time)
{
	return fConvert->IsStoppedAt(time);
}

// Value -----------------------------------------------------
// ------
const hsScalar & plATCChannel::Value(double time, hsBool peek)
{
	fResult = (peek ? fConvert->WorldToAnimTimeNoUpdate(time) : fConvert->WorldToAnimTime(time));
	return fResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLSCALARSDLCHANNEL
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------------
// -----
plScalarSDLChannel::plScalarSDLChannel()
: fLength(1), fVar(nil) 
{
	fResult = 0;
}

plScalarSDLChannel::plScalarSDLChannel(hsScalar length)
: fLength(length), fVar(nil) 
{
	fResult = 0;
}

// dtor ---------------------------------
plScalarSDLChannel::~plScalarSDLChannel()
{
}

// IsStoppedAt ------------------------------------
// ------------
hsBool plScalarSDLChannel::IsStoppedAt(double time) 
{ 
	return false; 
}

// Value -----------------------------------------------------------
// ------
const hsScalar & plScalarSDLChannel::Value(double time, hsBool peek)
{
	if (fVar)
		fVar->Get(&fResult);

	// the var will return a 0-1 value, scale to match our anim length.
	fResult *= fLength;
	return fResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// APPLICATORS
//
/////////////////////////////////////////////////////////////////////////////////////////

// IApply ------------------------------------------------------------------
// -------
void plScalarChannelApplicator::IApply(const plAGModifier *mod, double time)
{
}

// IApply --------------------------------------------------------------
// -------
void plSpotInnerApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
	hsAssert(scalarChan, "Invalid channel given to plSpotInnerApplicator");

	plSpotLightInfo *sli = plSpotLightInfo::ConvertNoRef(IGetGI(mod, plSpotLightInfo::Index()));

	const hsScalar &scalar = scalarChan->Value(time);
	sli->SetSpotInner(hsScalarDegToRad(scalar)*0.5f);
}

// IApply --------------------------------------------------------------
// -------
void plSpotOuterApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
	hsAssert(scalarChan, "Invalid channel given to plSpotInnerApplicator");

	plSpotLightInfo *sli = plSpotLightInfo::ConvertNoRef(IGetGI(mod, plSpotLightInfo::Index()));

	const hsScalar &scalar = scalarChan->Value(time);
	sli->SetSpotOuter(hsScalarDegToRad(scalar)*0.5f);
}


void plOmniApplicator::IApply(const plAGModifier *modifier, double time)
{
	plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
	hsAssert(scalarChan, "Invalid channel given to plLightOmniApplicator");

	plOmniLightInfo *oli = plOmniLightInfo::ConvertNoRef(IGetGI(modifier, plOmniLightInfo::Index()));

	oli->SetLinearAttenuation(scalarChan->Value(time));
}

void plOmniSqApplicator::IApply(const plAGModifier *modifier, double time)
{
	plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
	hsAssert(scalarChan, "Invalid channel given to plLightOmniApplicator");

	plOmniLightInfo *oli = plOmniLightInfo::ConvertNoRef(IGetGI(modifier, plOmniLightInfo::Index()));

	oli->SetQuadraticAttenuation(scalarChan->Value(time));
}

void plOmniCutoffApplicator::IApply(const plAGModifier *modifier, double time)
{
	plScalarChannel *scalarChan = plScalarChannel::ConvertNoRef(fChannel);
	hsAssert(scalarChan, "Invalid channel given to plOmniCutoffApplicator");

	plOmniLightInfo *oli = plOmniLightInfo::ConvertNoRef(IGetGI(modifier, plOmniLightInfo::Index()));

	oli->SetCutoffAttenuation( scalarChan->Value( time ) );
}