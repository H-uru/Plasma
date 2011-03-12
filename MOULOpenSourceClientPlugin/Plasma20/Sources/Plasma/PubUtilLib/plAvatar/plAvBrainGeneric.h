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
#ifndef PLAVBRAINGENERIC_INC
#define PLAVBRAINGENERIC_INC

// base class
#include "plAvBrain.h"
#include "plAGAnim.h"

class plAnimStage;
class plAnimStageVec;
class plAvBrainGenericMsg;
class plHorizontalFreezeAction;
class plNotifyMsg;

/** \class plAvBrainGeneric
	The generic brain is designed to manage complex (and simple) sequences of animations
	used in detailed puzzle sequences and the like.
	It implements a model wherein each animation is wrapped in an plAnimStage object,
	which provides different means of user input, support for loop types, callbacks
	at significant events, and scripted control of stage transitions.
	Detailed object interactions such as climbing, sitting, interacting with levers,
	and the like are all handled by generic brains with varying stages.
*/
class plAvBrainGeneric : public plArmatureBrain
{
public:
	/** \enum BrainType
		Sometimes the brains are truly generic and opaque, and sometimes
		we need to be able to tell "what this brain does" at a summary level.
		This enum lists our current standard brain types.
		This data is not used internally by the brain; it's just stored
		for the user. */
	enum BrainType {
		kGeneric,				/// type was not specified
		kLadder,				/// it's a ladder brain
		kSit,					/// it's a sit brain
		kSitOnGround,			/// sit on the ground emote
		kEmote,					/// yeah yeah
		kAFK,
		kNumBrainTypes,
		kNonGeneric = kNumBrainTypes	/// If we ask the avatar what its current generic type is,
										/// we use this to signify that it's not currenty using
										/// a generic brain.
	};
	
	/** \enum ExitFlag
		The default behavior for a generic brain is to exit when all its
		stages are completed. This set of bitwise indices defines other
		conditions which can be used to exit a brain.
		Note: For now, we're restricting the SDL for this to a byte. Change that if you add
			  too many flags.
		*/
	enum ExitFlag {
		kExitNormal = 0,		// for completeness; use this when you don't have an exit condition
		kExitAnyTask  = 0x01,			// exit if anyone sends us a task
		kExitNewBrain =	0x02,			// exit if anyone sends us a new brain
		kExitAnyInput =	0x04,		// exit if we get any input control events
		kExitMaxFlag  = 0x08
	};

	enum MoveMode {
		kMoveAbsolute,				// the animations are taken to be in world coordinates
		kMoveRelative,				// the animation is played as if the starting point is the origin
		kMoveNormal,				// the avatar root is moved by translating the animation to velocity
		kMoveStandstill,			// the avatar root is held immobile
		kMaxMoveMode,
		kForceSize = 0xff
	};

	enum Mode{
		kEntering	= 0x01,			// seeking, loading, or within the "enter" animation
		kNormal		= 0x02,			// undistinguished animations are running
		kFadingIn	= 0x03,			// fading in first stage; not running
		kFadingOut	= 0x04,			// fading out last stage; not running
		kExit		= 0x05,			// exit observing animation fadeouts, etc.
		kAbort		= 0x06,			// exit immediately without updating animations
		kMaxMode	= 0x07,
		kModeSize	= 0xff
	} fMode;

	static const hsScalar kDefaultFadeIn;
	static const hsScalar kDefaultFadeOut;

	/** Default constructor for the class factory and descendants. */
	plAvBrainGeneric();

	/** Canonical constructor - use this.
		\param stages A vector of stages to use for this brain. Will be copied
					  into our internal vector.
		\param enterMessage A message to send when the brain enters begins playing.
							If you need finer control over callbacks, use stage callbacks.
		\param exitMessage A message to send when the brain exits.
		\param recipient Callbacks from the brain *and* the stages is sent to this key.
		\param exitFlags A combination of exit conditions from the ExitFlag enum */
	plAvBrainGeneric(plAnimStageVec *stages, plMessage *enterMessage, plMessage *exitMessage,
					 plKey recipient, UInt32 exitFlags, float fadeIn, float fadeOut, MoveMode moveMode);

	/** Simplified constructor
		\param exitFlags Indicates which conditions will cause the brain to exit.
		\param fadeIn Rate (in blend units per second) of initial animation fade in.
		\param fadeOut Rate (in blend units per second) of final animation fade out.
		*/
	plAvBrainGeneric(UInt32 exitFlags, float fadeIn, float fadeOut, MoveMode moveMode);	

	/** Virtual destructor */
	virtual ~plAvBrainGeneric();

	static hsBool fForce3rdPerson;

	/** Attaches our current stage and takes control of the armature. Note that
		the brain may be in a "half-finished" state -- we may be receiving some
		complex state from another client and therefore pushing on a brain that
		is in the middle of its third stage, for example. */
	virtual void Activate(plArmatureModBase *avMod);

	/** Advance the current stage and swap in a new stage if necessary. */
	virtual hsBool Apply(double timeNow, hsScalar elapsed);

	/** Remove all our stages and release control of the armature. */
	virtual void Deactivate();

	virtual plKey GetRecipient();
	virtual void SetRecipient(const plKey &recipient);

	/** Send a notify message to our recipient.
		Some brains, such as the coop, will use this opportunity to add annotations
		to the notify. */
	virtual bool RelayNotifyMsg(plNotifyMsg *msg);

	/** We're leaving the age. Clean up. */
	virtual hsBool LeaveAge();

	virtual hsBool IsRunningTask();
	
	/** Compare the names of the anims in our stages.
		Return true on a match (order matters). */
	bool MatchAnimNames(const char *names[], int count);

	/** Add the given stage to the end of the stage sequence.
		Returns the zero-based index of the stage.
	*/
	int AddStage(plAnimStage *stage);

	/** The sequence number of the *given* stage -- not the current stage. Useful
		if you have a pointer to a stage and want to find out where in the sequence
		it lives. Notice that since stage transitions can be nonlinear, there is
		no guarantee that this will be the nth stage to play, or that it will play
		at all. */
	int GetStageNum(plAnimStage *stage);

	/** Get the sequence number of the stage that's currently playing. */
	int GetCurStageNum();
	
	void SetCurStageNum(int num) { fCurStage = num; };

	/** How many stages are in this brain? */
	int GetStageCount();

	/** Retrieve a stage via a sequence number. Returns nil if no such stage exists 
		Be extremely careful about retaining this pointer, as it may be deleted at
		any time. */
	plAnimStage * GetStage(int which);

	/** Retrieve a pointer to the currently executing stage.Returns nil if no such stage exists 
		Be extremely careful about retaining this pointer, as it may be deleted at
		any time. */
	plAnimStage * GetCurStage();

	/** Set the user-specified type of the brain. Has no effect on brain behavior;
		just a poor man's dynamic type system */
	BrainType SetType(BrainType newType);

	/** Get the user-specified type of the brain. Has no effect on brain behavior;
		for external reference only. */
	BrainType GetType();

	/** Is the brain moving "generally" forward? This is a fairly poorly-defined concept
		that is only used by stages that have "auto" progress types, i.e. they
		move forward if the brain is moving forward, and move backward if the brain
		is moving backward.
		/deprecated Avoid. will probably be deleted. */
	bool GetForward() const { return fForward; }
	bool GetReverseFBControlsOnRelease() const { return fReverseFBControlsOnRelease; };
	void SetReverseFBControlsOnRelease(bool val) { fReverseFBControlsOnRelease = val; };	

	/** Returns a pointer to the message we will send when we start our brain.
		Returns nil if there is no such message *or* if it has already been sent.
		/bug This message is destructed after being delivered, so we need to 
			 nil this pointer after sending it.
	*/
	plMessage* GetStartMessage() const { return fStartMessage; }

	/** Returns a pointer to the message we will send when we finish our brain.
		Returns nil if there is no such message *or* if it has already been sent.
		/bug This message is destructed after being delivered, so we need to 
			 nil this pointer after sending it.
	*/
	plMessage* GetEndMessage() const { return fEndMessage; }

	/** Returns the bitvector holding our exit conditions.
		/sa plAvBrainGeneric::ExitFlags */
	UInt32 GetExitFlags() const { return fExitFlags; }

	plAGAnim::BodyUsage GetBodyUsage();
	void SetBodyUsage(plAGAnim::BodyUsage bodyUsage);

	Mode GetMode() { return fMode; }
	void SetMode(Mode mode) { fMode = mode; }

	float GetFadeIn() { return fFadeIn; }
	float GetFadeOut() { return fFadeOut; }

	MoveMode GetMoveMode() { return fMoveMode; }

	/** Output the brain's status to the avatar debug screen. */
	virtual void DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	// plasma protocol
	hsBool plAvBrainGeneric::MsgReceive(plMessage *msg);
	CLASSNAME_REGISTER( plAvBrainGeneric );
	GETINTERFACE_ANY( plAvBrainGeneric, plArmatureBrain );

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

protected:
	
	/////////////////////////////////////////////////////////////////////////////////////
	//
	// CODE
	//
	/////////////////////////////////////////////////////////////////////////////////////

	hsBool IHandleGenBrainMsg(const plAvBrainGenericMsg *msg);
	hsBool IHandleTaskMsg(plAvTaskMsg *msg);

	bool IBrainIsCompatible(plAvBrainGeneric *otherBrain);

	float IGetAnimDelta(double time, float elapsed);	// how far should we move our animation this frame?

	hsBool IProcessNormal(double time, float elapsed);

	hsBool IProcessFadeIn(double time, float elapsed);
	hsBool IProcessFadeOut(double time, float elapsed);

	hsBool ISwitchStages(int oldStage, int newStage, float delta, hsBool setTime, hsScalar newTime,
						 float fadeNew, hsScalar fadeOld, double worldTime);

	void IEnterMoveMode(double time);		// we've just entered and we're about to begin animating.
	void IExitMoveMode();					// we're done animating; clean up

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// DATA
	//
	/////////////////////////////////////////////////////////////////////////////////////
	 
	plKey fRecipient;				// this guy gets all new notify messages
	plAnimStageVec *fStages;		// all the stages in our animation
	int fCurStage;					// which stage are we playing? (zero-based)
	BrainType fType;				// what type of brain are we?
	UInt32 fExitFlags;				// what will cause us to exit?
	plHorizontalFreezeAction *fCallbackAction;

	bool fForward;					// are we currently moving forward or backward through the stages?
									// this is used by the "auto-" movement types in the stages

	plMessage *fStartMessage;		// send this message when our brain is activated
	plMessage *fEndMessage;			// send this message when our brain is deactivated

	bool fReverseFBControlsOnRelease;	// Tells the armature that we want to reverse the forward/backward controls
										// if both are released. The armatureMod will tell us if this has actually happened

	float fFadeIn;					// if non-zero, the rate to fade in the first animation
	float fFadeOut;					// if non-zero, the rate to fade out the last animation

	MoveMode fMoveMode;				// how are we translating animation movement to
									// movement in the world?

	plAGAnim::BodyUsage fBodyUsage;	// how much of the body does this brain use?
									// this is based strictly on compatibility; i.e. a brain
									// that uses only the lower body might still animate the
									// hands, but it is guaranteed to look right when overlaid
									// by an anim that uses only the upper body.
	
};



#endif // PLAVBRAINGENERIC_INC
