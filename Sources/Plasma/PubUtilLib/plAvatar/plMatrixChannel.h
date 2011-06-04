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
#ifndef PLMATRIXCHANNEL_INC
#define PLMATRIXCHANNEL_INC

// local
#include "plScalarChannel.h"

// global
#include "hsTypes.h"		// you need types to include Matrix
#include "hsMatrix44.h"
#include "../plTransform/hsAffineParts.h"

// local prototypes
class plQuatChannel;
class plPointChannel;

// external prototypes
class plController;
class hsAffineParts;
class plAnimTimeConvert;
class plMatrixChannelApplicator;
class plControllerCacheInfo;

//////////////////
// PLMATRIXCHANNEL
//////////////////
// an animation channel that outputs matrices
class plMatrixChannel : public plAGChannel
{
protected:
	hsMatrix44 fResult;
	hsAffineParts fAP;

public:
	plMatrixChannel();
	virtual ~plMatrixChannel();

	// AG PROTOCOL
	virtual const hsMatrix44 & Value(double time, bool peek = false);
	virtual void Value(hsMatrix44 &matrix, double time, bool peek = false);
	virtual const hsAffineParts & AffineValue(double time, bool peek = false);

	// combine it (allocates combine object)
	virtual plAGChannel * MakeCombine(plAGChannel * channelB);

	// blend it (allocates blend object)
	virtual plAGChannel * MakeBlend(plAGChannel * channelB, plScalarChannel * channelBias, int blendPriority);
	
	// const eval at time zero
	virtual plAGChannel * MakeZeroState();
	// make a timeScale instance
	virtual plAGChannel * MakeTimeScale(plScalarChannel *timeSource);

	virtual plAGPinType GetPinType() { return kAGPinTransform; };

	virtual void Dump(int indent, bool optimized, double time);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plMatrixChannel );
	GETINTERFACE_ANY( plMatrixChannel, plAGChannel );
};

///////////////////
// PLMATRIXCONSTANT
///////////////////
// A matrix source that just keeps handing out the same value
class plMatrixConstant : public plMatrixChannel
{
public:
	plMatrixConstant();
	plMatrixConstant(const hsMatrix44 &value);
	virtual ~plMatrixConstant();

	void Set(const hsMatrix44 &value);
	
	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plMatrixConstant );
	GETINTERFACE_ANY( plMatrixConstant, plMatrixChannel );

	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *s, hsResMgr *mgr);
};

////////////////////
// PLMATRIXTIMESCALE
////////////////////
// Adapts the time scale before passing it to the next channel in line.
// Use to instance animations while allowing each instance to run at different speeds.
class plMatrixTimeScale : public plMatrixChannel
{
protected:
	plScalarChannel *fTimeSource;
	plMatrixChannel *fChannelIn;

public:
	plMatrixTimeScale();
	plMatrixTimeScale(plMatrixChannel *channel, plScalarChannel *timeSource);
	virtual ~plMatrixTimeScale();

	virtual hsBool IsStoppedAt(double time);
	virtual const hsMatrix44 & Value(double time, bool peek = false);
	virtual const hsAffineParts & AffineValue(double time, bool peek = false);

	virtual plAGChannel * Detach(plAGChannel * channel);

	virtual void Dump(int indent, bool optimized, double time);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plMatrixTimeScale );
	GETINTERFACE_ANY( plMatrixTimeScale, plMatrixChannel );
};

////////////////
// PLMATRIXBLEND
////////////////
// blends two matrices into one with weighting
class plMatrixBlend : public plMatrixChannel
{
protected:
	plMatrixChannel * fChannelA;
	plMatrixChannel * fOptimizedA;
	plMatrixChannel * fChannelB;
	plMatrixChannel * fOptimizedB;
	plScalarChannel * fChannelBias;
	int fPriority;

public:
	// xTORs
	plMatrixBlend();
	plMatrixBlend(plMatrixChannel * channelA, plMatrixChannel * channelB, plScalarChannel * channelBias, int priority);
	virtual ~plMatrixBlend();

	virtual plAGChannel * plMatrixBlend::MakeBlend(plAGChannel *newChannel, plScalarChannel *channelBias, int blendPriority);

	// you cannot blend on top of a channel that has higher priority than you do.
	virtual UInt16 GetPriority();

	// SPECIFICS
	const plMatrixChannel * GetChannelA() const { return fChannelA; }
	void SetChannelA(plMatrixChannel * channel) { fChannelA = channel; }

	const plMatrixChannel * GetChannelB() const { return fChannelB; }
	void SetChannelB(plMatrixChannel * channel) { fChannelB = channel; }

	const plScalarChannel * GetChannelBias() const { return fChannelBias; }
	void SetChannelBias(plScalarChannel * channel) { fChannelBias = channel; }

	//virtual void SetBlend(float blend) { fBlend = blend; };
	//virtual float GetBlend() { return fBlend; };

	virtual hsBool IsStoppedAt(double time);

	// AG PROTOCOL
	virtual const hsMatrix44 & Value(double time, bool peek = false);
	virtual const hsAffineParts & AffineValue(double time, bool peek = false);

	// remove the specified channel from our graph
	virtual plAGChannel * Detach(plAGChannel * channel);
	virtual void Dump(int indent, bool optimized, double time);

	plAGChannel *plMatrixBlend::Optimize(double time);
	
	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plMatrixBlend );
	GETINTERFACE_ANY( plMatrixBlend, plMatrixChannel );
};

/////////////////////
// PLMATRIXCONTROLLER
/////////////////////
// converts a plController-style animation into a plMatrixChannel
class plMatrixControllerChannel : public plMatrixChannel
{
protected:
	plController	*fController;

public:
	// xTORs
	plMatrixControllerChannel();
	plMatrixControllerChannel(plController *controller, hsAffineParts *parts);
	virtual ~plMatrixControllerChannel();

	// AG PROTOCOL
	virtual const hsAffineParts & AffineValue(double time, bool peek = false);
	virtual const hsAffineParts & AffineValue(double time, bool peek, plControllerCacheInfo *cache);	
	virtual const hsMatrix44 & Value(double time, bool peek = false);
	virtual const hsMatrix44 & Value(double time, bool peek, plControllerCacheInfo *cache);
	
	virtual plAGChannel * MakeCacheChannel(plAnimTimeConvert *atc);

	virtual void Dump(int indent, bool optimized, double time);

	// PLASMA PROTOCOL
	// rtti
	CLASSNAME_REGISTER( plMatrixControllerChannel );
	GETINTERFACE_ANY( plMatrixControllerChannel, plMatrixChannel );

	// persistence
	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *s, hsResMgr *mgr);
};

/////////////////////////////////
// PLMATRIXCONTROLLERCACHECHANNEL
/////////////////////////////////
// Same as plMatrixController, but with caching info
class plMatrixControllerCacheChannel : public plMatrixChannel
{
protected:
	plControllerCacheInfo *fCache;
	plMatrixControllerChannel *fControllerChannel;
	
public:
	plMatrixControllerCacheChannel();
	plMatrixControllerCacheChannel(plMatrixControllerChannel *channel, plControllerCacheInfo *cache);
	virtual ~plMatrixControllerCacheChannel();
	
	virtual const hsMatrix44 & Value(double time, bool peek = false);
	virtual const hsAffineParts & AffineValue(double time, bool peek = false);
	
	virtual plAGChannel * Detach(plAGChannel * channel);
	
	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plMatrixControllerCacheChannel );
	GETINTERFACE_ANY( plMatrixControllerCacheChannel, plMatrixChannel );

	// Created at runtime only, so no Read/Write
};

/////////////////////
// PLQUATPOINTCOMBINE
/////////////////////
// combines a quaternion and a point into a matrix
class plQuatPointCombine : public plMatrixChannel
{
protected:
	plQuatChannel *fQuatChannel;
	plPointChannel *fPointChannel;

public:
	// xTORs
	plQuatPointCombine();
	plQuatPointCombine(plQuatChannel *quatChannel, plPointChannel *pointChannel);
	virtual ~plQuatPointCombine();

	// SPECIFICS
	const plQuatChannel * GetQuatChannel() const { return fQuatChannel; }
	void SetQuatChannel(plQuatChannel * channel) { fQuatChannel = channel; }

	plPointChannel * GetPointChannel() const { return fPointChannel; }
	void SetPointChannel(plPointChannel * channel) { fPointChannel = channel; }

	// AG PROTOCOL
	virtual const hsMatrix44 & Value(double time);
	virtual const hsAffineParts & AffineValue(double time);

	// remove the specified channel from our graph
	virtual plAGChannel * Detach(plAGChannel * channel);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plQuatPointCombine );
	GETINTERFACE_ANY( plQuatPointCombine, plMatrixChannel );
};


/////////////////////
//
// Applicator classes

class plMatrixChannelApplicator : public plAGApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plMatrixChannelApplicator );
	GETINTERFACE_ANY( plMatrixChannelApplicator, plAGApplicator );

	virtual hsBool CanCombine(plAGApplicator *app) { return false; }
	virtual plAGPinType GetPinType() { return kAGPinTransform; }
};

// PLMATRIXDELAYEDCORRECTIONAPPLICATOR
// Used for blending in sudden location changes due to synch messages.
// This app tacks on a correction to the l2p transform
// (so l2p is set as animL2P*correction)
// interpolating this to the identity matrix over time.
class plMatrixDelayedCorrectionApplicator : public plMatrixChannelApplicator
{
protected:
	hsAffineParts fCorAP;	// AP of the correction.
	double fDelayStart;		// Start time of the delayed correction
	
	// apply our animation * our correction to the node
	virtual void IApply(const plAGModifier *mod, double time);

public:
	plMatrixDelayedCorrectionApplicator() : fDelayStart(-1000.f), fIgnoreNextCorrection(true) { fCorAP.Reset(); }
	
	CLASSNAME_REGISTER( plMatrixDelayedCorrectionApplicator );
	GETINTERFACE_ANY( plMatrixDelayedCorrectionApplicator, plMatrixChannelApplicator );

	void SetCorrection(hsMatrix44 &correction);
	virtual hsBool AutoDelete() { return false; } // should we remove it when its input channel is gone?

	// applicator arbitration...
	virtual plAGPinType GetPinType() { return kAGPinTransform; }
	virtual hsBool CanBlend(plAGApplicator *app);

	hsBool fIgnoreNextCorrection;
	static const hsScalar fDelayLength; // static var for now.	
};

// PLMATRIXDIFFERENCEAPP
// Each frame, this guy takes the difference between his current value
// and his last value and applies that to the current world
// transform of the target.
// You could also call it the Temporal Matrix Difference Applicator,
// but that sucks to type.
class plMatrixDifferenceApp : public plMatrixChannelApplicator
{
public:
	/** Forget the previous cached transform and start again */
	void Reset(double time);

	/** Should this applicator be automatically removed when its channel goes away? */
	virtual hsBool AutoDelete() { return false; }

	// applicator arbitration
	virtual plAGPinType GetPinType() { return kAGPinTransform; }
	virtual hsBool CanBlend(plAGApplicator *app);

	CLASSNAME_REGISTER(plMatrixDifferenceApp);
	GETINTERFACE_ANY(plMatrixDifferenceApp, plMatrixChannelApplicator);

protected:
	virtual void IApply(const plAGModifier *mod, double time);
	hsMatrix44 fLastL2A;		// local to animation space
	hsMatrix44 fLastA2L;		// animation space to local
	bool fNew;					// true if we haven't cached anything yet
};


#endif
