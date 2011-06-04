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
/** \file plAGChannel.h
    \brief The animation class for the AniGraph animation system
	WHAT'S AG STAND FOR?
	AG stands for animation graph. It's a directed acyclic graph of animation data sources and operations.

	WHAT'S A CHANNEL?
	A channel is a stream of animation data.
	A channel is a node in an animation graph.
	Animation data can come from anything; keyframes, IK, physics, etc.
	Channels are often grouped together using blending nodes.
	Blending nodes are also a type of channel.
	See where I'm going with this?

	HOW IS THIS DIFFERENT FROM PLCONTROLLER?
	It's very similar, but it's designed to be extremely lightweight; animation graphs are set up and torn down
	quickly and with great frequency. They are not persistent, and their only state (besides cache)
	is their interconnectedness.

	HOW DO THEY GET SAVED?
	Their client must recreate them at read/load time. Since they are stateless, all information necessary
	to recreate them is necessarily held by the client.

	ARE THEY REFERENCE COUNTED?
	No. The entire graph is owned by the creator.
	Deleting the top node of a graph deletes the entire graph.
	If we decide we want to share subgraphs, we'll add reference counting -- easy since cycles are illegal

	HOW DO THEY INTEGRATE WITH PLCONTROLLERS?
	Once we decide the animation graph approach is workable and effective, we'll combine the plController
	concept with the plAGChannel concept.
	Until then, there will be a handful of channel types that adapt controllers into animation graphs.

	WHAT DOES "COMBINE" MEAN?
	Merging more than one types of animation data into a single output that preserves the value
	of both.

	WHAT DOES "BLEND" MEAN?
	Merging two similar types of animation data using a weight to "fade" from one to the other

	WHAT IS ZEROSTATE?
	This is a static channel that always returns the value at time zero. It is used as a reference for animation
	scaling (amplitude).

	WHAT'S AN APPLICATOR?
	In order to cut down on the number of derived channel classes for different Value() and Apply() behaviors,
	applicators were created. At the end of each graph is an applicator, which specifies what should be done
	with the result of a call to Value().

	\ingroup Avatar
	\ingroup AniGraph
*/

#ifndef PLAGCHANNEL_H
#define PLAGCHANNEL_H

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

#include "../pnFactory/plCreatable.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plAGModifier;
class plAnimTimeConvert;
class plAGChannel;
class plScalarChannel;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

/** \class plAGChannel
    An object that emits data of a specific type. Fundamental building
    block of the animation graph. */
class plAGChannel : public plCreatable
{
public:

	// -- methods --
	/** Default constructor for the base class. */
	plAGChannel();

	/** Free an allocated name. Does not release any upstream nodes. */
	virtual ~plAGChannel();

	// AG PROTOCOL
	/** Combine the given channel with this channel, allocating and returning
	    a new node which does the combination. It's up to the caller to 
	    manage the lifetime of the new node. */
	virtual plAGChannel * MakeCombine(plAGChannel * channelB);

	/** Blend the given channel with this channel, using a third channel (which 
		must output a float/scalar value) to crossfade between the two.
		As the blendChannel varies, the blend will vary. */
	virtual plAGChannel * MakeBlend(plAGChannel * channelB, plScalarChannel * blendChannel, int blendPriority);

	/** Create a "static clone" of this channel which always returns this channel's
		value at time zero. */
	virtual plAGChannel * MakeZeroState() = 0;
	
	/** If we're potentially sharing this channel with other plAGMasterMods, we'll
		want to insert a channel in the graph for cache info. This function returns
		either the cache channel (replacing us) or ourself. */
	virtual plAGChannel * MakeCacheChannel(plAnimTimeConvert *atc) { return this; }

	/** Create a new channel which converts global time to local time
	    and attach it downstream from this channel. This allows you to
	    convert an animation from one timespace to another - critical for
	    blending.
	    local-time-animation <-- timescale <-- world-time-animation */
	virtual plAGChannel * MakeTimeScale(plScalarChannel *timeSource) = 0;

	/** Is the animation moving at the given world time? Takes into account
	    start/stop messages that haven't been applied yet, ease curves, etc. */
	virtual hsBool IsStoppedAt(double wSecs) { return true; }

	/** Detach the given channel from our graph. If this is the channel in
	    question, returns any upstream channels so they can be reattached.
	    If this is not the channel in question, passes the request upstream
	    and does any reattachment necessary. */
	virtual plAGChannel * Detach(plAGChannel * channel);
	
	/** Return the optimized version of this channel. May be a completely
		different channel; will collapse out inactive subgraphs. */
	virtual plAGChannel * Optimize(double time);

	// \{
	/** The name of the channel is used to dynamically attach to sub-parts of an
		object. */
	virtual const char * GetName() { return fName; };
	virtual void SetName(char * name) { fName = name; };
	// \}

	// PLASMA PROTOCOL
	// rtti
	CLASSNAME_REGISTER( plAGChannel );
	GETINTERFACE_ANY( plAGChannel, plCreatable );

	// persistence
	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *s, hsResMgr *mgr);

protected:
	const char * fName;

};

#endif PLAGCHANNEL_H
