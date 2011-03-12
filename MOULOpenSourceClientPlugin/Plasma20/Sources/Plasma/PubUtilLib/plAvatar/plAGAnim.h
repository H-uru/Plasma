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
#ifndef PLAGANIM_INC
#define PLAGANIM_INC

/** \file plAGAnim.h
    \brief The animation class for the AniGraph animation system

	\ingroup Avatar
	\ingroup AniGraph
*/
#pragma warning(disable: 4786)		// don't care if mangled names are longer than 255 characters

#include "../pnNetCommon/plSynchedObject.h"
#include "hsStlUtils.h"
#include "hsStlSortUtils.h"

class plTMController;
class hsAffineParts;
class plAnimTimeConvert;
struct hsMatrix44;
class plEmoteAnim;
class plAGApplicator;
class plAGChannel;

/** \class	plAGAnim
			This class holds reusable animation data. A single plAGAnim can be instanced
			any number of times simultaneously.
			In order to use a plAGAnim, you need a plAGMasterMod, which is a special type of
			modifier which can be attached to multiple scene objects. A master mod is typically
			applied to the root of a scene branch, but there is no requirement that all the scene
			objects animated be children.
			Each plAGAnim has a number of channels, each of which can animate a single parameter
			of a single object. Channels are parametric, i.e. they can carry any data type.
			A plAGAnim carries a name for the animation, and each channel is named as well.
			Instancing a plAGAnim is done via a plAGAnimInstance.
	\sa plAGAnimInstance plAGMasterMod plAGModifier
 */
class plAGAnim : public plSynchedObject
{
public:
	/** How much of the body does this emote use? This is handy information for 
		figuring out whether you can, say, wave while sitting down. */
	enum BodyUsage {
		kBodyUnknown,
		kBodyUpper,
		kBodyFull,
		kBodyLower,
		kBodyMax,
		kForceSize = 0xff
	};

	plAGAnim();
	/** Construct with name, start time, and end time (within the max note track)
	 */
	plAGAnim(const char *name, double begin, double end);
	/** Destruct, freeing the underlying animation data. */
	virtual ~plAGAnim();

	/** Return the total of number of channels supplied by this animation.
	    An object being animated by this animation does not have to have this
		many channels; any which are not available will be ignored.
		This is syntactic sugar: GetApplicatorCount will return the exact
	    same number, but some code is only interested in the channels and not
	    the applicators. */
	int GetChannelCount() const;

	/** Return the ith channel of the animation. Ordering is arbitrary but consistent.
		It's currently breadth first base on the export algorithm, but don't count on this
		remaining true. */
	plAGChannel * GetChannel(int i) const;

	/** Get the name of the channel having the given index. Useful for talking to an
	    an animation before it is applied and finding out what channels it's going to
		affect. */
	virtual const char * GetChannelName(int index);

	/** Get channel by name. This corresponds to the name of the scene object this channel
		will be attached to when the animation is applied.
		This function is fairly slow and shouldn't be used often. */
	plAGChannel * GetChannel(const char *name) const;

	/** Return the number of applicators held by this animation. An applicator is used
	    to attach a channel to a sceneobject. */
	int GetApplicatorCount() const;

	/** Return the ith applicator in the channel.
	    Order is arbitrary but consistent, corresponding to processing order in the exporter. */
	plAGApplicator * GetApplicator(int i) const;		// get applicator by index
	
	/** Add an applicator -- which must have a channel behind it.
	    Applicators are translator object which take the output of a
		channel and apply it to a scene object. */
	int AddApplicator(plAGApplicator * app);

	/** Remove the ith applicator and its associated channel. Existing applicators
	    will be renumbered. */
	hsBool RemoveApplicator(int appNum);

	/** The name of the animation. This name is used in the avatar manager to reference
	    animations. Animations are generally indexed by name when they are loaded
		by the avatar or from script, but most of the functions which take an animation
		name (such as AttachAnimation on the plAGMasterMod) will also take an pointer
		to a plAGAnim. */
	virtual const char * GetName() const { return fName; }

	/** Return the length of the animation; end - start. */
	virtual hsScalar GetLength() const { return fEnd - fStart; }

	/** Hacky function to extend the length of the animation to some minimum
	    length. Does nothing if the animation is already longer than this. */
	void ExtendToLength(hsScalar length);

	/** Return the start time for the beginning of the animation. The animation
	   contains no data prior to this time.
	   In practice, this always returns 0.0f and this function may be deprecated. */
	virtual hsScalar GetStart() const { return fStart; }
	void SetStart(hsScalar start) { fStart = start; }

	/** Return the end time of the animation. Since start is typically 0, this usually
	   serves as the length of the animation as well. */
	virtual hsScalar GetEnd() const { return fEnd; }
	void SetEnd(hsScalar end) { fEnd = end; }
	
	
	/** Returns true if any applicator on the arg anim tries to use the
		same pin (on the same object) as we do. */
	hsBool SharesPinsWith(const plAGAnim *anim) const;


	// PLASMA PROTOCOL
	// rtti
	CLASSNAME_REGISTER( plAGAnim );
	GETINTERFACE_ANY( plAGAnim, plSynchedObject );

	// *** temp hack to manage animation instances
	/** Add the animation by name to a global static registry.
	    This functionality will possibly be added to the resource
	    manager. */
	static void AddAnim(const char * name, plAGAnim *anim);
	/** See if there is an animation with the given name in the
	    global animation registry. */
	static plAGAnim *FindAnim(const char *name);
	/** Remove the given animation from the registry. */
	static hsBool RemoveAnim(const char *name);
	/** Clear the animation cache. Used when resetting the client
	    to a vanilla state, as when clearing the scene while
	    exporting. */
	static void ClearAnimationRegistry();
	/** Debugging utility. Prints out a list of all the animations
	    in the registry */
	static void DumpAnimationRegistry();

	// persistance
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

protected:
	typedef std::vector<plAGApplicator*> ApplicatorVec;
	ApplicatorVec fApps;		/// our animation channels
	float fBlend;				/// requested blend factor

	hsScalar fStart;			/// the start time of the beginning of the animation (usually 0)
	hsScalar fEnd;				/// the end time of the animation

	char *fName;				/// the name of our animation

	// ??? Can this be moved to the resource manager? If it can manage an efficient
	//     string-based namespace per class, we could get rid of this.
	typedef std::map<const char *, plAGAnim *, stringISorter> plAnimMap;	//
	static plAnimMap fAllAnims;	/// map of animation names to animations

	typedef std::map<const char *, plEmoteAnim *, stringISorter> plEmoteMap;
	static plEmoteMap fAllEmotes;
};

///////////////
// PLATCANIM
///////////////

/** \class plATCAnim
	The most common subclass of plAGAnim.
	Represents an animation with a standard AnimTimeConvert
	(i.e. stop/start/loop/etc animation)
	*/
class plATCAnim : public plAGAnim
{
public:


	plATCAnim();
	/** Construct with name, start time, and end time (within the max note track)
	    Default is to start automatically, not loop, with no ease curves. */
	plATCAnim(const char *name, double begin, double end);
	/** Destruct, freeing the underlying animation data. */
	virtual ~plATCAnim();

	/** Returns the initial position of the "playback head" for this animation.
	    Animations are not required to start at their actual beginning but can,
		for instance, start in the middle, play to the end, and then loop to the
		beginning or to their loop start point. */
	virtual hsScalar GetInitial() const { return fInitial; }
	void SetInitial(hsScalar initial) { fInitial = initial;	}
	
	/** Does this animation start automatically when it's applied? */
	virtual bool GetAutoStart() const { return fAutoStart; }
	void SetAutoStart(hsBool start) { fAutoStart = (start != 0); }

	/** If the animation loops, this is where it will restart the loop. Note that
	    loops do not have to start at the beginning of the animation. */
	virtual hsScalar GetLoopStart() const { return fLoopStart; }
	void SetLoopStart(hsScalar start) { fLoopStart = start; }

	/** If the animation loops, this is the end point of the loop. After passing
		this point, the animation will cycle around to GetLoopStart */
	virtual hsScalar GetLoopEnd() const { return fLoopEnd; }
	void SetLoopEnd(hsScalar end) { fLoopEnd = end; }

	/** Does this animation loop?. Note that there may be multiple loop segments defined
	    within a given animation. */
	virtual bool GetLoop() const { return fLoop; }
	void SetLoop(hsBool loop) { fLoop = (loop != 0); }

	/** Set the curve type for easing in. Easing is an optional feature which allows you
		to make an animation slow down gradually when you stop it.
		The types are defined in plAnimEaseTypes.h
		*/
	virtual UInt8 GetEaseInType() const { return fEaseInType; }
	void SetEaseInType(UInt8 type) { fEaseInType = type; }

	/** Set the length of time the ease-in should take. */
	virtual hsScalar GetEaseInLength() const { return fEaseInLength; }
	/** Set the length of time the ease-in should take. */
	void SetEaseInLength(hsScalar length) { fEaseInLength = length; }

	/** The minimum value used at the start of the ease in. */
	virtual hsScalar GetEaseInMin() const { return fEaseInMin; }
	/** The minimum value used at the start of the ease in. */
	void SetEaseInMin(hsScalar length) { fEaseInMin = length; }
	
	/** The maximum value reached at the end of the ease in. */
	virtual hsScalar GetEaseInMax() const { return fEaseInMax; }
	/** The maximum value reached at the end of the ease in. */
	void SetEaseInMax(hsScalar length) { fEaseInMax = length; }

	/** The curve type for the ease out. */
	virtual UInt8 GetEaseOutType() const { return fEaseOutType; }
	/** The curve type for the ease out. */
	void SetEaseOutType(UInt8 type) { fEaseOutType = type; }
	
	/** The length of time for the ease out. */
	virtual hsScalar GetEaseOutLength() const { return fEaseOutLength; }
	/** The length of time for the ease out. */
	void SetEaseOutLength(hsScalar length) { fEaseOutLength = length; }

	/** Minimum value reached in ease-out */
	virtual hsScalar GetEaseOutMin() const { return fEaseOutMin; }
	/** Minimum value reached in ease-out */
	void SetEaseOutMin(hsScalar length) { fEaseOutMin = length; }

	/** Maximum value reached in ease-in */
	virtual hsScalar GetEaseOutMax() const { return fEaseOutMax; }
	/** Maximum value reached in ease-in */
	void SetEaseOutMax(hsScalar length) { fEaseOutMax = length; }

	/** Animations can have multiple defined loop segments; these
	    are selected using animation control messages.
	    Each loop segment is named using markers in the notetrack. */
	void AddLoop(const char *name, float start, float end);
	/** Get the loop having the given name.
		\param start will return the start time of the loop.
		\param end will hold the end time of the loop */
	bool GetLoop(const char *name, float &start, float &end) const;
	/** Lets you get a loop by index instead of name. */
	bool GetLoop(UInt32 num, float &start, float &end) const;
	/** Returns the number of loops defined on this anim. */
	UInt32 GetNumLoops() const;

	/** Add a marker to the animation. Markers can be used
	    for callbacks or for goto comands. A marker is a simple
	    name/time tuple. */
	void AddMarker(const char *name, float time);
	/** Returns the time value of the marker named by name. */
	float GetMarker(const char *name) const;
	void CopyMarkerNames(std::vector<char*> &out);
	/** Add a stop point to the animation. A stop point is a 
	    "detent" for playback - if the animation is stopping
	    near a stop point and fading out, the stop point will
	    override the fade, so that the animation stops precisely
	    at the defined time. */
	void AddStopPoint(hsScalar time);
	/** Return the number of stop points defined for this animation. */
	UInt32 NumStopPoints();
	/** Get the time corresponding to the given stop point. Stop points
	    are numbered in the order they were added. */
	hsScalar GetStopPoint(UInt32 i);
	/** Function to check for a zero-length loop, and set it to
		the anim's start/end instead */
	void CheckLoop();

	// PLASMA PROTOCOL
	// rtti
	CLASSNAME_REGISTER( plATCAnim );
	GETINTERFACE_ANY( plATCAnim, plAGAnim );

	// persistance
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

protected:
	hsScalar fInitial;			/// the position of the playback head 
	bool fAutoStart;			/// does the animation start automatically?
	hsScalar fLoopStart;		/// when wrapping a loop, start here
	hsScalar fLoopEnd;			/// when you reach this point, loop back
	bool fLoop;					/// do we loop?

	UInt8 fEaseInType;			/// the type (none/linear/spline) of our ease-in curve, if any
	UInt8 fEaseOutType;			/// the type (none/linear/spline) of our ease-out curve, if any
	hsScalar fEaseInLength;		/// the length of time our ease-in curve takes
	hsScalar fEaseInMin;		/// minimum (initial) value of our ease-in
	hsScalar fEaseInMax;		/// maximum (final) value of our ease-in 
	hsScalar fEaseOutLength;	/// the length of time our ease-out curve takes
	hsScalar fEaseOutMin;		/// minimum (final) value of our ease-out
	hsScalar fEaseOutMax;		/// maximum (initial) value of our ease-out

	// a map from segment names to times
	typedef std::map<const char *, float, stringSorter> MarkerMap;
	MarkerMap fMarkers;

	typedef std::map<const char *, std::pair<float,float>, stringSorter> LoopMap;
	LoopMap fLoops;

	typedef std::vector<hsScalar> ScalarMap;
	ScalarMap fStopPoints;		/// vector of stop points
};



/** \class plEmoteAnim
	An animation to be used for emotes.
	Automatically registers so that it can be played from the chat field.
	*/
class plEmoteAnim : public plATCAnim
{
public:
	
	plEmoteAnim();
	plEmoteAnim(const char *animName, double begin, double end, float fadeIn, float fadeOut, BodyUsage bodyUsage);

	BodyUsage GetBodyUsage() const;
	hsScalar GetFadeIn() const;
	hsScalar GetFadeOut() const;

	CLASSNAME_REGISTER( plEmoteAnim );
	GETINTERFACE_ANY( plEmoteAnim, plATCAnim );

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

protected:
	BodyUsage	fBodyUsage;		// how much of the body is used by this emote?
	hsScalar	fFadeIn;		// how fast to fade in the emote
	hsScalar	fFadeOut;		// how fast to fade out the emote
};

//////////////////
// PLAGEGLOBALANIM
//////////////////

/** \class plAgeGlobalAnim
	An animation that bases its current position on a variable that global to the age,
	like weather, time of day, etc.
	*/

class plAgeGlobalAnim : public plAGAnim
{
public:
	plAgeGlobalAnim();
	/** Construct with name, start time, and end time (within the max note track)
	  */
	plAgeGlobalAnim(const char *name, double begin, double end);
	/** Destruct, freeing the underlying animation data. */
	virtual ~plAgeGlobalAnim();
	
	const char * GetGlobalVarName() const { return fGlobalVarName; }
	void SetGlobalVarName(char *name);

	// PLASMA PROTOCOL
	// rtti
	CLASSNAME_REGISTER( plAgeGlobalAnim );
	GETINTERFACE_ANY( plAgeGlobalAnim, plAGAnim );

	// persistance
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

protected:
	char *fGlobalVarName;	// Name of the SDL variable we animate on.
};

// USEFUL HELPER FUNCTIONS
bool GetStartToEndTransform(const plAGAnim *anim, hsMatrix44 *startToEnd, hsMatrix44 *endToStart, const char *channelName);
bool GetRelativeTransform(const plAGAnim *anim, double timeA, double timeB, hsMatrix44 *a2b, hsMatrix44 *b2a, const char *channelName);



#endif
