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
#ifndef PLQUATCHANNEL_INC
#define PLQUATCHANNEL_INC

#include "hsQuat.h"

#include "plAGChannel.h"
#include "plAGApplicator.h"

// PLQUATCHANNEL
/////////////
// A source of animated hsQuat data
class plQuatChannel : public plAGChannel
{
protected:
	hsQuat fResult;

public:
	plQuatChannel();
	virtual ~plQuatChannel();

	// AG PROTOCOL
	virtual const hsQuat & Value(double time);
	virtual void Value(hsQuat &quaternion, double time);

	// can this channel combine with the given channel?
	virtual hsBool CanCombine(plAGChannel * channelB);
	// combine it (allocates combine object)
	virtual plAGChannel * MakeCombine(plAGChannel * channelB);

	// const eval at time zero
	virtual plAGChannel * MakeZeroState();
	// make a timeScale instance
	virtual plAGChannel * MakeTimeScale(plScalarChannel *timeSource);

	// blend it (allocates blend object)
	virtual plAGChannel * MakeBlend(plAGChannel * channelB, plScalarChannel * channelBias, int blendPriority);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plQuatChannel );
	GETINTERFACE_ANY( plQuatChannel, plAGChannel );
};

// PLQUATCONSTANT
////////////
// A quat channel that just keeps handing out the same quaternion
class plQuatConstant : public plQuatChannel
{
public:
	plQuatConstant();
	plQuatConstant(const hsQuat &quaternion);
	virtual ~plQuatConstant();

	void Set(const hsQuat &the_Quat) { fResult = the_Quat; }

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plQuatConstant );
	GETINTERFACE_ANY( plQuatConstant, plQuatChannel );

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);
};

////////////////////
// PLQUATTIMESCALE
////////////////////
// Adapts the time scale before passing it to the next channel in line.
// Use to instance animations while allowing each instance to run at different speeds.
class plQuatTimeScale : public plQuatChannel
{
protected:
	plScalarChannel *fTimeSource;
	plQuatChannel *fChannelIn;

public:
	plQuatTimeScale();
	plQuatTimeScale(plQuatChannel *channel, plScalarChannel *timeSource);
	virtual ~plQuatTimeScale();

	virtual hsBool IsStoppedAt(double time);
	virtual const hsQuat & Value(double time);

	virtual plAGChannel * Detach(plAGChannel * channel);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plQuatTimeScale );
	GETINTERFACE_ANY( plQuatTimeScale, plQuatChannel );
};

// PLQUATBLEND
//////////////
// Combines two quaternion sources into one
class plQuatBlend : public plQuatChannel
{
public:
	plQuatBlend();
	plQuatBlend(plQuatChannel *channelA, plQuatChannel *channelB, plScalarChannel *channelBias);
	virtual ~plQuatBlend();

	// GETTERS AND SETTERS FOR CHANNELS
	const plQuatChannel * GetQuatA() const { return fQuatA; }
	void SetQuatA(plQuatChannel *the_QuatA) { fQuatA = the_QuatA; }

	const plQuatChannel * GetQuatB() const { return fQuatB; }
	void SetQuatB(plQuatChannel *the_QuatB) { fQuatB = the_QuatB; }

	const plScalarChannel * GetChannelBias() const { return fChannelBias; }
	void SetChannelBias(plScalarChannel *channel) { fChannelBias = channel; }

	//float GetBlend() const { return fBlend; }
	//void SetBlend(float the_blend) { fBlend = the_blend; }
	virtual hsBool IsStoppedAt(double time);

	// AG PROTOCOL
	virtual const hsQuat &Value(double time);

	// remove the specified channel from our graph
	virtual plAGChannel * Detach(plAGChannel * channel);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plQuatBlend );
	GETINTERFACE_ANY( plQuatBlend, plQuatChannel );

protected:
	plQuatChannel *fQuatA;
	plQuatChannel *fQuatB;
	plScalarChannel *fChannelBias;

};


////////////////////////////
// 
// Channel Applicator classes

class plQuatChannelApplicator : public plAGApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plQuatChannelApplicator );
	GETINTERFACE_ANY( plQuatChannelApplicator, plAGApplicator );
};


#endif // PLQUATCHANNEL_INC
