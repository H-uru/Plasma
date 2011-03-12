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
/** \file plAGMasterMod.h
    \brief The animation class for the AniGraph animation system

    \ingroup Avatar
    \ingroup AniGraph
*/
#ifndef PLAGMASTERMOD_INC
#define PLAGMASTERMOD_INC

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
#include "../pnModifier/plModifier.h"
#include "plAGChannel.h"
#include "plAvDefs.h"
#include "../pnKeyedObject/plMsgForwarder.h"

// templates
#include "hsStlUtils.h"
#include "hsStlSortUtils.h"


class plAGModifier;
class plAGAnimInstance;
class plAGAnim;
class plATCAnim;
class plAGMasterSDLModifier;

////////////////
//
// PLAGMASTERMOD
//
////////////////
/** \class plAGMasterMod
	A modifier which can apply animations to scene objects. 
	Works together with a plAGModifier, which can apply animation to a
	single scene object. A master modifier hooks up to a family of
	ag modifiers to do its job.
	An animation (plAGAnim) can have many different channels, but the
	user can just pass it to a single master mod to apply it to all the
	channels owned by that master. The master will take care of hooking
	each channel up to an ag modifier.
	The goal is to make an animation a conceptually (and practically) simple
	object to work with, whether or not it has multiple channels.

	\sa plAGAnim, plAGAnimInstance, plAGModifier
	*/
class plAGMasterMod : public plModifier
{
	friend class plAGMasterSDLModifier;
public:
	/** Default constructor. Primarily for use by the class factory. */
	plAGMasterMod();
	/** Free the name and any other miscellany. */
	virtual ~plAGMasterMod();

	/** Find an individual plAGModifier of the given name under our control. */
	plAGModifier * GetChannelMod(const char * name, hsBool dontCache = false) const;

	/** \name Managing Animations */
	// \{
	// AGANIM PROTOCOL
	/** Attach the given animation object with the given blend factor.
	    If there's no animation already attached to blend with, the
	    animation will be attached at full strength. */
	plAGAnimInstance *AttachAnimationBlended(plAGAnim *anim, hsScalar blendFactor = 0,
											 UInt16 blendPriority = kAGMedBlendPriority,
											 hsBool cache = false);

	/** Look up the given animation by name and attach it
		with the given blend factor. */
	plAGAnimInstance *AttachAnimationBlended(const char *name, hsScalar blendFactor = 0,
											 UInt16 blendPriority = kAGMedBlendPriority,
											 hsBool cache = false);

	/** Play a simple anim (one that doesn't affect root) once and auto detach. 
		Intended for Zandi's facial animations that run seperate from the behaviors. */
	void PlaySimpleAnim(const char *name);
											 
	/** Detach the given animation instance. Does nothing
	    if the instance is not managed by this master mod. */
	void DetachAnimation(plAGAnimInstance *instance);
	void DetachAllAnimations();

	/** Detach the given animation by name. Searches for
	    any instances derived from animations with the
	    given name and removes them. */
	void DetachAnimation(const char *name);
	// \}

	/** Print the current animation stack to the console.
	    Will list all the animations and their blend strengths.
	    Animations later in the list will mask animations earlier
	    in the list. */
	void DumpCurrentAnims(const char *header);

	/** Find and return any animation instances with the
		given name on this master modifer. */
	plAGAnimInstance *FindAnimInstance(const char *name);

	/** Return the Ith animation instance, based on blend
	    order. Of dubious utility, but, y'know. */
	plAGAnimInstance *GetAnimInstance(int i);

	/** Attach the animation if it's not already attached. If
	    it is attached, return the instance.
	    Note that if it's attached by this function, it 
	    will be on top of the stack, but if it was already
	    attached, it could be anywhere, including buried under
	    a bunch of other animations. If it's important that it be
	    on top of the stack, you may need to detach it first. */
	plAGAnimInstance *FindOrAttachInstance(const char *name, hsScalar blendFactor);

	/** Return the number of animations available. */
	int GetNumAnimations();
	/** Return the number of animations that are privately
	    owned by this modifier.
	    Animations may be either shared in a general pool,
	    or privately owned by a mastermod. */
	int GetNumPrivateAnimations();

	int GetNumATCAnimations();
	plAGAnimInstance *GetATCAnimInstance(int i);

	/** Apply all our animations to all our parts.
		\param timeNow is the current world time
		\param elapsed is the time since the previous frame */
	void ApplyAnimations(double timeNow, hsScalar elapsed);

	/** Runs through our anims and applies them, without
		processing fades. This is used when we load in anim
		state from the server, and need to advance it to a
		certain point before enabling callbacks */
	void AdvanceAnimsToTime(double time);

	/** Change the connectivity in the graph so that inactive animations are bypassed.
	    The original connectivity information is kept, so if the activity of different
		animations is changed (such as by changing blend biases or adding new animations,
		the graph can be compiled again to the correct state. */
	void Compile(double time);

	/** We've done something that invalidates the cached connectivity in the graph.
	    Mark this for fixup. */
	void SetNeedCompile(bool needCompile);
	
	/** List the animationg graph to stdOut, with a ASCII representation of the tree
		structure. Done by recursively dumping the graph; some types of nodes will have
		more output information than others.
		*/
	void DumpAniGraph(const char *channel, bool optimized, double time);

	/** Set whether or not this is the "group master" so grouped animations will only have
		one member getting/setting sdl animation state in order to synch the anims
		*/
	void SetIsGrouped(bool grouped);
	void SetIsGroupMaster(bool master, plMsgForwarder* msgForwarder);

	// PLASMA PROTOCOL
	virtual int GetNumTargets() const { return fTarget ? 1 : 0; }
	virtual plSceneObject* GetTarget(int w) const { /* hsAssert(w < GetNumTargets(), "Bad target"); */ return fTarget; }
	virtual void AddTarget(plSceneObject * object);
	virtual void RemoveTarget(plSceneObject * object);

	hsBool plAGMasterMod::MsgReceive(plMessage* msg);

	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream * stream, hsResMgr *mgr);

	hsBool HasRunningAnims();
	hsBool DirtySynchState(const char* SDLStateName, UInt32 synchFlags);	
	
	CLASSNAME_REGISTER( plAGMasterMod );
	GETINTERFACE_ANY( plAGMasterMod, plModifier );

protected:
	// -- methods --
	plAGModifier * ICacheChannelMod(plAGModifier *mod) const;
	plAGModifier * IFindChannelMod(const plSceneObject *obj, const char *name) const;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
	
	virtual void IApplyDynamic() {};	// dummy function required by base class

	// Find markers in an anim for environment effects (footsteps)
	virtual void ISetupMarkerCallbacks(plATCAnim *anim, plAnimTimeConvert *atc) {}

	// -- members
	plSceneObject*	fTarget;

	// a map from channel names to ag modifiers within our domain
	typedef std::map<const char *, plAGModifier*, stringSorter> plChannelModMap;
	plChannelModMap fChannelMods;

	// instances which are currently attached to this master
	typedef std::vector<plAGAnimInstance*> plInstanceVector;
	plInstanceVector fAnimInstances;					// animation instances

	// animations which are owned exclusively by this master
	typedef std::vector<plAGAnim*> plAnimVector;
	plAnimVector fPrivateAnims;

	// animations that require AnimTimeConvert state to be synched
	typedef std::vector<plAGAnim*> plAnimVector;
	plInstanceVector fATCAnimInstances;
	
	hsBool fFirstEval;
	hsBool fNeedEval;
	void IRegForEval(hsBool val);

	// SDL modifier which sends/recvs dynamics state
	plAGMasterSDLModifier *fAGMasterSDLMod;	

	bool fNeedCompile;

	bool fIsGrouped;
	bool fIsGroupMaster;
	plMsgForwarder* fMsgForwarder;
	
	enum {
		kPrivateAnim,
		kPublicAnim,
	};
};

#endif // PLAGMASTERMOD_INC
