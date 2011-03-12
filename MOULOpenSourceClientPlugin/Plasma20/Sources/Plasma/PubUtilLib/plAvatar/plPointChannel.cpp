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
#include "plPointChannel.h"
#include "plScalarChannel.h"
#include "hsResMgr.h"

#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../plInterp/plController.h"
#include "../plInterp/plAnimTimeConvert.h"
#include "../plGLight/plLightInfo.h"

//////////////
// PLPOINTSRCE
//////////////

// CTOR
plPointChannel::plPointChannel()
: plAGChannel(), fResult(0, 0, 0)
{
}

// DTOR
plPointChannel::~plPointChannel()
{
}

// VALUE
// default behaviour is just to return our result (constant value)
const hsPoint3 & plPointChannel::Value(double time)
{
	return fResult;
}

// VALUE(point, time)
void plPointChannel::Value(hsPoint3 &point, double time)
{
	point = Value(time);
}

// MAKECOMBINE
plAGChannel * plPointChannel::MakeCombine(plAGChannel *channelA)
{
	return nil;
}

// MAKEBLEND
plAGChannel * plPointChannel::MakeBlend(plAGChannel * channelB, plScalarChannel * channelBias, int blendPriority)
{
	return TRACKED_NEW plPointBlend(this, (plPointChannel *)channelB, channelBias);
}

// MAKEZEROSTATE
plAGChannel * plPointChannel::MakeZeroState()
{
	return TRACKED_NEW plPointConstant(Value(0));
}

// MAKETIMESCALE
plAGChannel * plPointChannel::MakeTimeScale(plScalarChannel *timeSource)
{
	return TRACKED_NEW plPointTimeScale(this, timeSource);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plPointConstant
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------------
// -----
plPointConstant::plPointConstant()
: plPointChannel()
{
}

// ctor -----------------------------------------------
// -----
plPointConstant::plPointConstant(const hsPoint3 &point)
{
	fResult = point;
}

// dtor ---------------------------
// -----
plPointConstant::~plPointConstant()
{
}

void plPointConstant::Read(hsStream *stream, hsResMgr *mgr)
{
	plPointChannel::Read(stream, mgr);
	fResult.Read(stream);
}

void plPointConstant::Write(hsStream *stream, hsResMgr *mgr)
{
	plPointChannel::Write(stream, mgr);
	fResult.Write(stream);
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// PLPOINTTIMESCALE
//
// Insert into the graph when you need to change the speed or direction of time
// Also serves as a handy instancing node, since it just passes its data through.
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor ----------------------------
// -----
plPointTimeScale::plPointTimeScale()
: fTimeSource(nil), fChannelIn(nil)
{
}

// ctor --------------------------------------------------------------------------------
// -----
plPointTimeScale::plPointTimeScale(plPointChannel *channel, plScalarChannel *timeSource)
: fChannelIn(channel),
  fTimeSource(timeSource)
{
}

// dtor -----------------------------
// -----
plPointTimeScale::~plPointTimeScale()
{
}

// IsStoppedAt ---------------------------
// ------------
plPointTimeScale::IsStoppedAt(double time)
{
	return fTimeSource->IsStoppedAt(time);
}

// Value --------------------------------------------
// ------
const hsPoint3 & plPointTimeScale::Value(double time)
{
	fResult = fChannelIn->Value(fTimeSource->Value(time));

	return fResult;
}

// Detach ---------------------------------------------------
// -------
plAGChannel * plPointTimeScale::Detach(plAGChannel * channel)
{
	plAGChannel *result = this;
	
	fChannelIn = plPointChannel::ConvertNoRef(fChannelIn->Detach(channel));
	
	if(!fChannelIn || channel == this)
		result = nil;
	
	if(result != this)
		delete this;
	
	return result;
	
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// plPointBlend
//
/////////////////////////////////////////////////////////////////////////////////////////

// ctor --------------------
// -----
plPointBlend::plPointBlend()
: fPointA(nil),
  fPointB(nil)
{
}

// ctor ---------------------------------------------------------------------------------
// -----
plPointBlend::plPointBlend(plPointChannel *channelA, plPointChannel *channelB,
						   plScalarChannel *channelBias)
: fPointA(channelA),
  fPointB(channelB),
  fChannelBias(channelBias)
{
}

// dtor ---------------------
// -----
plPointBlend::~plPointBlend()
{
	fPointA = nil;
	fPointB = nil;
	fChannelBias = nil;
}

// IsStoppedAt ------------------------------
// ------------
hsBool plPointBlend::IsStoppedAt(double time)
{
	hsScalar blend = fChannelBias->Value(time);
	if (blend == 0)
		return fPointA->IsStoppedAt(time);
	if (blend == 1)
		return fPointB->IsStoppedAt(time);

	return (fPointA->IsStoppedAt(time) && fPointB->IsStoppedAt(time));
}

// Value ---------------------------------------
// ------
const hsPoint3 &plPointBlend::Value(double time)
{
	if (fPointA && fPointB)
	{
		hsScalar curBlend = fChannelBias->Value(time);
		if(curBlend == 0) {
			fPointA->Value(fResult, time);
		} else {
			if(curBlend == 1) {
				fPointB->Value(fResult, time);
			} else {
				const hsPoint3 &pointA = fPointA->Value(time);
				const hsPoint3 &pointB = fPointB->Value(time);
				hsPoint3 difference = pointB - pointA;

				difference *= curBlend;
	
				fResult = pointA + difference;
			}
		}
	} else {
		if (fPointA)
		{
			fResult = fPointA->Value(time);
		} else {
			if (fPointB)
			{
				fResult = fPointA->Value(time);
			}
		}
	}
	return fResult;
}

// Detach ------------------------------------------------------
// -------
plAGChannel * plPointBlend::Detach(plAGChannel *remove)
{
	plAGChannel *result = this;

	if (remove == this)
	{
		result = nil;
	} else 	{
		// it's possible that the incoming channel could reside down *all* of our
		// branches (it's a graph, not a tree,) so we always pass down all limbs
		fChannelBias = plScalarChannel::ConvertNoRef(fChannelBias->Detach(remove));
		fPointA = plPointChannel::ConvertNoRef(fPointA->Detach(remove));
		fPointB = plPointChannel::ConvertNoRef(fPointB->Detach(remove));
		if (!fChannelBias)
		{
			// No more bias channel, assume it's zero from now on, (a.k.a. We just want channelA)
			result = fPointA;
		}
		else
		{
			if(!fChannelBias)
				result = fPointA;
			else if(fPointA && !fPointB)
				result = fPointA;
			else if(fPointB && !fPointA)
				result = fPointB;
			else if(!fPointA && !fPointB)
				result = nil;
			if(result != this)
			{
				delete this;
			}
		}
	}
	return result;
}

///////////////////////////
// PLPOINTCONTROLLERCHANNEL
///////////////////////////

// CTOR
plPointControllerChannel::plPointControllerChannel()
: fController(nil)
{
}

// CTOR(name, controller)
plPointControllerChannel::plPointControllerChannel(plController *controller)
: fController(controller)
{
}

// ~DTOR()
plPointControllerChannel::~plPointControllerChannel()
{
	if(fController) {
		delete fController;
		fController = nil;
	}
}

// VALUE(time)
const hsPoint3 & plPointControllerChannel::Value(double time)
{
	return Value(time, nil);
}

// VALUE(time)
const hsPoint3 & plPointControllerChannel::Value(double time, plControllerCacheInfo *cache)
{
	fController->Interp((hsScalar)time, &fResult, cache);
	return fResult;
}

plAGChannel *plPointControllerChannel::MakeCacheChannel(plAnimTimeConvert *atc)
{
	plControllerCacheInfo *cache = fController->CreateCache();
	cache->SetATC(atc);
	return TRACKED_NEW plPointControllerCacheChannel(this, cache);
}

// WRITE(stream, mgr)
void plPointControllerChannel::Write(hsStream *stream, hsResMgr *mgr)
{
	plPointChannel::Write(stream, mgr);

	hsAssert(fController, "Trying to write plPointControllerChannel with nil controller. File will not be importable.");
	mgr->WriteCreatable(stream, fController);
}

// READ(stream, mgr)
void plPointControllerChannel::Read(hsStream *stream, hsResMgr *mgr)
{
	plPointChannel::Read(stream, mgr);
	
	fController = plController::ConvertNoRef(mgr->ReadCreatable(stream));
}

/////////////////////////////////
// PLPOINTCONTROLLERCACHECHANNEL
/////////////////////////////////

// CTOR
plPointControllerCacheChannel::plPointControllerCacheChannel()
: fControllerChannel(nil),
  fCache(nil)
{
}

// CTOR(name, controller)
plPointControllerCacheChannel::plPointControllerCacheChannel(plPointControllerChannel *controller, plControllerCacheInfo *cache)
: fControllerChannel(controller),
  fCache(cache)
{
}

// ~DTOR()
plPointControllerCacheChannel::~plPointControllerCacheChannel()
{
	delete fCache;
	fControllerChannel = nil;
}

// VALUE(time)
const hsPoint3 & plPointControllerCacheChannel::Value(double time, bool peek)
{
	return fControllerChannel->Value(time, fCache);
}

// DETACH
plAGChannel * plPointControllerCacheChannel::Detach(plAGChannel * channel)
{
	if(channel == this)
	{
		return nil;
	} else {
		plAGChannel *result = fControllerChannel->Detach(channel);
		
		if(result == fControllerChannel)
		{
			return this;
		} else {
			// if our controller channel has been detached, then detach ourselves as well.
			return result;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Channel applicators

void plPointChannelApplicator::IApply(const plAGModifier *modifier, double time)
{
	plPointChannel *pointChan = plPointChannel::ConvertNoRef(fChannel);
	hsAssert(pointChan, "Invalid channel given to plPointChannelApplicator");

	plCoordinateInterface *CI = IGetCI(modifier);

	hsMatrix44 l2p = CI->GetLocalToParent();
	const hsPoint3 &point = pointChan->Value(time);

	l2p.SetTranslate(&point);
	
	hsMatrix44 p2l;
	l2p.GetInverse(&p2l);
	CI->SetLocalToParent(l2p, p2l);
}

void plLightDiffuseApplicator::IApply(const plAGModifier *modifier, double time)
{
	plPointChannel *pointChan = plPointChannel::ConvertNoRef(fChannel);
	hsAssert(pointChan, "Invalid channel given to plLightDiffuseApplicator");

	plLightInfo *li = plLightInfo::ConvertNoRef(IGetGI(modifier, plLightInfo::Index()));

	const hsPoint3 &point = pointChan->Value(time);
	hsColorRGBA color;
	color.Set(point.fX, point.fY, point.fZ, 1.0f);
	li->SetDiffuse(color);
}

void plLightAmbientApplicator::IApply(const plAGModifier *modifier, double time)
{
	plPointChannel *pointChan = plPointChannel::ConvertNoRef(fChannel);
	hsAssert(pointChan, "Invalid channel given to plLightAmbientApplicator");

	plLightInfo *li = plLightInfo::ConvertNoRef(IGetGI(modifier, plLightInfo::Index()));

	const hsPoint3 &point = pointChan->Value(time);
	hsColorRGBA color;
	color.Set(point.fX, point.fY, point.fZ, 1.0f);
	li->SetAmbient(color);
}

void plLightSpecularApplicator::IApply(const plAGModifier *modifier, double time)
{
	plPointChannel *pointChan = plPointChannel::ConvertNoRef(fChannel);
	hsAssert(pointChan, "Invalid channel given to plLightSpecularApplicator");

	plLightInfo *li = plLightInfo::ConvertNoRef(IGetGI(modifier, plLightInfo::Index()));

	const hsPoint3 &point = pointChan->Value(time);
	hsColorRGBA color;
	color.Set(point.fX, point.fY, point.fZ, 1.0f);
	li->SetSpecular(color);
}


