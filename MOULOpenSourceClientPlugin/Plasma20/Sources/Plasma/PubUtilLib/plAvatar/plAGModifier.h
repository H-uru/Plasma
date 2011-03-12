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

    \ingroup Avatar
    \ingroup AniGraph
*/
#ifndef PLAGMODIFIER_H
#define PLAGMODIFIER_H

#include "hsTypes.h"							// need for plSingleModifier
#include "../pnModifier/plSingleModifier.h"		// inherited

// local
#include "../plAvatar/plScalarChannel.h"

// stl
#include "hsStlUtils.h"
#include "hsStlSortUtils.h"

class plSceneObject;

class plAGAnimInstance;
class plAGAnim;
class plAvatarAnim;
class plAnimCmdMsg;
class plOneShotCallbacks;

///////////////
//
// PLAGMODIFIER
//
///////////////
/** \class plAGModifier
	Animates a single scene object.
	May apply multiple animations to that scene object, animating
	the transform with one, colors with another, etc.
	The basic model is that the ag modifier holds an array of
	applicators, each of which is the root of a tree of animation
	operations, such as a blend tree, etc. Each frame it fires
	all its applicators, or (ideally) only those that need to be fired.
	*/
class plAGModifier : public plSingleModifier
{
public:
	/** Default constructor, primarily for the class factory. */
	plAGModifier();

	/** Construct given a name. This name will be used to match up 
		incoming channels with this modifier. You may also supply an
		autoApply parameter, which indicates whether this modifier
		should apply itself every frame, or only when explicitly asked to. */
	plAGModifier(const char *name, hsBool autoApply = true);

	/** It's a destructor. Destroys the name passed into the constructor,
		and a bunch of other stuff you don't need to know anything about. */
	virtual ~plAGModifier();

	/** Get the name of the channel controlled by this modifier. */
	const char * GetChannelName() const;
	/** Change the channel name of the modifier. Will delete the previous
	    name. Will NOT remove any channels that are already attached, so 
	    you could wind up with a modifier named "Fred" and a bunch of
	    channels attached to it that were intended for "Lamont." */
	void SetChannelName(char * name);

	/** Attach a TRACKED_NEW applicator to our modifier. Will arbitrate with existing
	    modifiers if necessary, based on pin type. May destruct existing applicators. */
	plAGChannel *MergeChannel(plAGApplicator *app, plAGChannel *chan, plScalarChannel *blend,
							  plAGAnimInstance *anim, int priority);
	/** Remove the given channel. Will also remove the channel's applicator. */
	hsBool DetachChannel(plAGChannel * channel);

	/** Set the applicator for a modifier.
		\deprecated */
	void SetApplicator(plAGApplicator *app);
	/** Get the applicator for a given pintype. Note that an ag modifier can
	    only have one applicator of a given pin type attached at a time.
	    The exception is for the unknown pin type, which never conflicts
	    with any other pin type, including itself. */
	plAGApplicator *GetApplicator(plAGPinType pin) const;

	/** Apply the animation for our scene object. */
	void Apply(double time) const;

	/** Get the channel tied to our ith applicator */
	plAGChannel * GetChannel(int i) { return fApps[i]->GetChannel(); }

	void Enable(hsBool val);

	// PERSISTENCE
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

	// PLASMA PROTOCOL
	CLASSNAME_REGISTER( plAGModifier );
	GETINTERFACE_ANY( plAGModifier, plSingleModifier );
	
protected:
	typedef std::vector<plAGApplicator*> plAppTable;
	plAppTable fApps;			// the applicators (with respective channels) that we're applying to our scene object

	char *fChannelName;			// name used for matching animation channels to this modifier
	hsBool	fAutoApply;			// evaluate animation automatically during IEval call
	hsBool	fEnabled;			// if not enabled, we don't eval any of our anims

	// APPLYING THE ANIMATION
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);

	virtual hsBool IHandleCmd(plAnimCmdMsg* modMsg) { return false; } // only plAGMasterMod should handle these
	virtual void IApplyDynamic() {};	// dummy function required by base class

	// INTERNAL ACCESSORS FOR SCENE OBJECT INTERFACES
	plAudioInterface * LeakAI() const { return IGetTargetAudioInterface(0); };
	plCoordinateInterface * LeakCI() const { return IGetTargetCoordinateInterface(0); };
	plDrawInterface * LeakDI() const { return IGetTargetDrawInterface(0); };
	plSimulationInterface * LeakSI() const { return IGetTargetSimulationInterface(0); };
	plObjInterface * LeakGI(UInt32 classIdx) const { return IGetTargetGenericInterface(0, classIdx); }

	friend plAudioInterface * plAGApplicator::IGetAI(const plAGModifier * modifier) const;
	friend plCoordinateInterface * plAGApplicator::IGetCI(const plAGModifier * modifier) const;
	friend plDrawInterface * plAGApplicator::IGetDI(const plAGModifier * modifier) const;
	friend plSimulationInterface * plAGApplicator::IGetSI(const plAGModifier * modifier) const;
	friend plObjInterface * plAGApplicator::IGetGI(const plAGModifier * modifier, UInt16 classIdx) const;

};
const plModifier * FindModifierByClass(const plSceneObject *obj, int classID);


#endif

