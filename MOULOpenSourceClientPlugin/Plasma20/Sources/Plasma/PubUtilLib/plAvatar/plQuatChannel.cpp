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
#include "plQuatChannel.h"
#include "plPointChannel.h"
#include "plMatrixChannel.h"

#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../plInterp/plAnimTimeConvert.h"

#include "hsMatrix44.h"

////////////////
// PLQUATCHANNEL
////////////////

// CTOR
plQuatChannel::plQuatChannel()
: plAGChannel()
{
	fResult.Identity();
}

// DTOR
plQuatChannel::~plQuatChannel()
{
}

// VALUE (time)
const hsQuat &plQuatChannel::Value(double time)
{
	return fResult;
}

// VALUE (quaternion, time)
void plQuatChannel::Value(hsQuat &quat, double time)
{
	quat = Value(time);
}

// CANCOMBINE
hsBool plQuatChannel::CanCombine(plAGChannel *channelA)
{
	return false;
	if(plPointChannel::ConvertNoRef(channelA))
	{
		return true;
	} else {
		return false;
	}
}

// MAKECOMBINE
plAGChannel * plQuatChannel::MakeCombine(plAGChannel *channelA)
{
	if(plPointChannel::ConvertNoRef(channelA))
	{
		return TRACKED_NEW plQuatPointCombine(this, (plPointChannel *)channelA);
	} else {
		return nil;
	}
}

// MAKEBLEND
plAGChannel *plQuatChannel::MakeBlend(plAGChannel *channelB, plScalarChannel *channelBias, int blendPriority)
{
	plQuatChannel *chanB = plQuatChannel::ConvertNoRef(channelB);
	plScalarChannel *chanBias = plScalarChannel::ConvertNoRef(channelBias);
	if(chanB && chanBias)
	{
		return TRACKED_NEW plQuatBlend(this, chanB, chanBias);
	} else {
		hsStatusMessageF("Blend operation failed.");
		return this;
	}
}

// MAKEZEROSTATE
plAGChannel * plQuatChannel::MakeZeroState()
{
	return TRACKED_NEW plQuatConstant(Value(0));
}

// MAKETIMESCALE
plAGChannel * plQuatChannel::MakeTimeScale(plScalarChannel *timeSource)
{
	return TRACKED_NEW plQuatTimeScale(this, timeSource);
}

/////////////////
// PLQUATCONSTANT
/////////////////

// CTOR
plQuatConstant::plQuatConstant()
: plQuatChannel()
{
}

// CTOR(name, quaternion)
plQuatConstant::plQuatConstant(const hsQuat &quaternion)
{
	fResult = quaternion;
}

// DTOR
plQuatConstant::~plQuatConstant()
{
}

void plQuatConstant::Read(hsStream *stream, hsResMgr *mgr)
{
	plQuatChannel::Read(stream, mgr);
	fResult.Read(stream);
}

void plQuatConstant::Write(hsStream *stream, hsResMgr *mgr)
{
	plQuatChannel::Write(stream, mgr);
	fResult.Write(stream);
}

////////////////////
// PLQUATTIMESCALE
////////////////////
// Insert into the graph when you need to change the speed or direction of time
// Also serves as a handy instancing node, since it just passes its data through.

// CTOR
plQuatTimeScale::plQuatTimeScale()
: fTimeSource(nil),
  fChannelIn(nil)
{
}

// CTOR (channel, converter)
plQuatTimeScale::plQuatTimeScale(plQuatChannel *channel, plScalarChannel *timeSource)
: fChannelIn(channel),
  fTimeSource(timeSource)
{
}

// DTOR
plQuatTimeScale::~plQuatTimeScale()
{
}

plQuatTimeScale::IsStoppedAt(double time)
{
	return fTimeSource->IsStoppedAt(time);
}

// VALUE
const hsQuat & plQuatTimeScale::Value(double time)
{
	fResult = fChannelIn->Value(fTimeSource->Value(time));

	return fResult;
}

// DETACH
plAGChannel * plQuatTimeScale::Detach(plAGChannel * channel)
{
	plAGChannel *result = this;

	fChannelIn = plQuatChannel::ConvertNoRef(fChannelIn->Detach(channel));

	if(!fChannelIn || channel == this)
		result = nil;

	if (result != this)
		delete this;

	return result;
}

//////////////
// PLQUATBLEND
//////////////

// CTOR
plQuatBlend::plQuatBlend()
: fQuatA(nil),
  fQuatB(nil),
  fChannelBias(nil)
{
}

// CTOR(channelA, channelB, blend)
plQuatBlend::plQuatBlend(plQuatChannel *channelA, plQuatChannel *channelB, plScalarChannel *channelBias)
: fQuatA(channelA),
  fQuatB(channelB),
  fChannelBias(channelBias)
{
}

// DTOR
plQuatBlend::~plQuatBlend()
{
	//if (fQuatA) delete fQuatA;
	//if (fQuatB) delete fQuatB;
	fQuatA = fQuatB = nil;
	fChannelBias = nil;
}

hsBool plQuatBlend::IsStoppedAt(double time)
{
	hsScalar blend = fChannelBias->Value(time);
	if (blend == 0)
		return fQuatA->IsStoppedAt(time);
	if (blend == 1)
		return fQuatB->IsStoppedAt(time);

	return (fQuatA->IsStoppedAt(time) && fQuatB->IsStoppedAt(time));
}

// VALUE(time)
const hsQuat &plQuatBlend::Value(double time)
{
	hsQuat quatA = fQuatA->Value(time);
	hsQuat quatB = fQuatB->Value(time);

	fResult.SetFromSlerp(quatA, quatB, fChannelBias->Value(time));

	return fResult;
}


// REMOVE
// Remove the given channel wherever it may be in the graph (including this node)
plAGChannel * plQuatBlend::Detach(plAGChannel *remove)
{
	plAGChannel *result = this;

	hsAssert(remove != this, "Cannot remove blenders explicitly. Remove blended source instead.");

	if (remove != this)
	{
		fChannelBias = plScalarChannel::ConvertNoRef(fChannelBias->Detach(remove));
		if (!fChannelBias)
		{
			// No more bias channel, assume it's zero from now on, (a.k.a. We just want channelA)
			result = fQuatA;
		}
		else
		{
			fQuatA = (plQuatChannel *)fQuatA->Detach(remove);
			if(fQuatA)
			{
				// channel a still here(although children may be gone); try channel b
				fQuatB = (plQuatChannel *)fQuatB->Detach(remove);

				if(!fQuatB)
				{
					result = fQuatA;	// channel b is gone: return channel a as blender's replacement
				}
			} else {
				result = fQuatB;		// channel a is gone: return channel b
			}

			if (result != this)
			{
				delete this;			// lost one of our channels: kill the blender.
			}
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////
// Applicators

void plQuatChannelApplicator::IApply(const plAGModifier *mod, double time)
{
	plQuatChannel *quatChan = plQuatChannel::ConvertNoRef(fChannel);
	hsAssert(quatChan, "Invalid channel in plQuatChannelApplicator");

	const hsQuat &rotate = quatChan->Value(time);

	plCoordinateInterface *CI = IGetCI(mod);

	hsMatrix44 l2w;
	hsMatrix44 w2l;

	rotate.MakeMatrix(&l2w);
	l2w.GetInverse(&w2l);

	CI->SetLocalToParent(l2w, w2l);
}
