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
/** \file plAnimStage.h
	An intelligent avatar animation for use in a generic brain.

    \ingroup Avatar
*/
#ifndef PL_ANIM_STAGE_INC
#define PL_ANIM_STAGE_INC

#include "hsMatrix44.h"
#include "hsStlUtils.h"
#include "../pnFactory/plCreatable.h"

class plMessage;
class plAGAnimInstance;
class plArmatureMod;
class plArmatureMod;
class hsStream;
class hsResMgr;
class plArmatureBrain;
class plDebugText;
class plMultistageBehMod;

// PLANIMSTAGE
// In a multi-stage behavior, each stage is specified in one of these classes
/** \class plAnimStage
	An animation stage is an intelligent wrapper for an avatar animation.
	It provides specific control over:
		- how the progress of the animation is controlled
		- when the animation optionally sends messages about its progress
		- whether the animation naturally progresses to the next animation after finishing
		- how many times the animation loops
		- whether the animation works in world space or a local space
	*/
class plAnimStage : public plCreatable
{
public:
	// -- forward declarations of types --
	// Points to send notifications at
	/** \enum NotifyType
		Controls when notification messages are sent by the stage.
		These can be used instead of or in combination with
		normal animation callback messages.
		Where the messages are sent is determined by the controlling
		brain.
		These are mask bits; you can have any number of them on at a time.
		*/
	enum NotifyType
	{
		kNotifyEnter   = 0x1,	/// Send when we enter the stage from
		kNotifyLoop    = 0x2,	/// Sent each time the stage loops
		kNotifyAdvance = 0x4,	/// Sent when we advance out of the stage
		kNotifyRegress = 0x8	/// Sent when we regress out of the stage
	};

	/** \enum ForwardType
		What makes the stage progress in a forward direction?
		You can only choose one of these, although a stage will always respond
		to forward movement messages. */
	enum ForwardType {
		kForwardNone,		/// move forward only in response to messages
		kForwardKey,		/// move forward when the user presses the move forward key
		kForwardAuto,		/// move forward automatically
		kForwardMax,
	} fForwardType;

	/** \enum BackType
		What makes the stage progress in a backward direction?
		You can only choose one of these, although a stage will always respond
		to backward movement message. */
	enum BackType {
		kBackNone,			/// move backward only in response to messages
		kBackKey,			/// move backward when the user presses the move backward key
		kBackAuto,			/// move backward automatically (only when entered from end)
		kBackMax,
	} fBackType;

	/** When do we advance to the next stage?
		This is typically asked when we're reached the end of the current stage,
		but for some advance types, you can interrupt the current stage and
		advance automatically on certain events. */
	enum AdvanceType {
		kAdvanceNone,		/// advance only when told to via a message
		kAdvanceOnMove,		/// advance whenever the user hits a movement key
		kAdvanceAuto,		/// advance automatically at the end of the current stage
		kAdvanceOnAnyKey,	/// advance when any key is pressed
		kAdvanceMax,
	} fAdvanceType;

	/** When do we regress to the previous stage?
		This is typically asked when we've reached the beginning of the current stage
		and we still trying to move backwards.
		For some regress types, however, you can abort this stage automatically when
		certain events occur. */
	enum RegressType {
		kRegressNone,		// regress only when told to via a message
		kRegressOnMove,		// regress whenever the user hits a movement key
		kRegressAuto,		// regress automatically when we reach the beginning of the current
							/// stage and we're still moving backwards
		kRegressOnAnyKey,	// regress when any key is pressed
		kRegressMax,
	} fRegressType;
	
	/** Default constructor, used primarily by the class factory. */
	plAnimStage();
	/** Canonical constructor.
		\param animName The name of the animation controlled by this stage.
		\param notify Flags for when to send notify messages
		\param forward How to control forward movement
		\param backward How to control backward movement
		\param regress When to regress to the previous stage
		\param advance When to advance to the next stage
		\param loops How many times to loop this stage after its initial playthrough.
					 i.e. If you only want it to play once, set loops to 0
					 To make it play forever, pass in -1 for loops.
		\param localize Align the origin of the animation to the avatar's position
						when the stage start's playing. Only relevant if the
						animation attempts to reposition the avatar by having a
						channel attached to the avatar's handle.
	*/
	plAnimStage(const char *animName,
				UInt8 notify,
				ForwardType forward,
				BackType backward,
				AdvanceType advance,
				RegressType regress,
				int loops);

	/** Most complete constructor. Specifies every aspect of the stage. Differs from
		the next-most-complete constructor by the presence of doAdvanceTo, advanceTo,
		doRegressTo, and regressTo. These parameters allow you to specify what stages
		will be played after this one, depending on which direction the stage is
		moving.
	*/
	plAnimStage(const char *animName,
				UInt8 notify,
				ForwardType forward,
				BackType back,
				AdvanceType advance,
				RegressType regress,
				int loops,
				bool doAdvanceTo,
				int advanceTo,
				bool doRegressTo,
				int regressTo);

	/** Simple and common constructor: moves forward automatically, advances automatically,
		no reverse, no regress.
		\param animName The name of the animation controlled by this stage.
		\param notify Flags for when to send notify messages
		*/
	plAnimStage(const char *animName, UInt8 notify);
	virtual ~plAnimStage();
	const plAnimStage& operator=(const plAnimStage& src);

	plAGAnimInstance * Attach(plArmatureMod *armature, plArmatureBrain *brain, float initialBlend, double time);
	int Detach(plArmatureMod *armature);
	void Reset(double time, plArmatureMod *avMod, bool atStart);
	void ResetAtTime(double time, float localTime, plArmatureMod *avMod);
	bool MoveRelative(double worldTime, float delta, float &overage, plArmatureMod *avMod);

	/** The name of the animation associated with this stage. */
	const char * GetAnimName();

	ForwardType GetForwardType();
	void SetForwardType(ForwardType t);
	BackType GetBackType();
	void SetBackType(BackType t);
	AdvanceType GetAdvanceType();
	void SetAdvanceType(AdvanceType t);
	RegressType GetRegressType();
	void SetRegresstype(RegressType t);
	UInt32 GetNotifyFlags();
	void SetNotifyFlags(UInt32 newFlags);
	int GetNumLoops();
	void SetNumLoops(int loops);
	int GetLoopValue();
	void SetLoopValue(int loop);
	float GetLocalTime();
	void SetLocalTime(float time, hsBool noCallbacks = false);
	float GetLength();
	
	void SetMod(plMultistageBehMod* p) { fMod = p; }	// for returning ID#'s to python scripts:
	plMultistageBehMod* GetMod() { return fMod; }
	bool GetIsAttached();
	void SetIsAttached(bool status);

	int GetNextStage(int curStage);
	int GetPrevStage(int curStage);
	plAGAnimInstance *GetAnimInstance() const { return fAnimInstance; };
	bool GetReverseOnIdle() { return fReverseOnIdle; }
	void SetReverseOnIdle(bool onOff) { fReverseOnIdle = onOff; }	
	void DumpDebug(bool active, int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	// STANDARD PLASMA PROTOCOL
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

	virtual void SaveAux(hsStream *stream, hsResMgr *mgr);
	virtual void LoadAux(hsStream *stream, hsResMgr *mgr, double time);

	CLASSNAME_REGISTER( plAnimStage );
	GETINTERFACE_ANY( plAnimStage, plCreatable);
	
protected:
	bool IMoveBackward(double worldTime, float delta, float &underrun, plArmatureMod *avMod);
	bool IMoveForward(double worldTime, float delta, float &overrun, plArmatureMod *avMod);

	bool INeedFadeIn();
	bool INeedFadeOut();

	bool ITryAdvance(plArmatureMod *avMod);
	bool ITryRegress(plArmatureMod *avMod);

	hsBool ISendNotify(UInt32 notifyMask, UInt32 notifyType, plArmatureMod *armature, plArmatureBrain *brain);

	char *fAnimName;			// the name of our animation
	UInt8 fNotify;				// flags for which events will cause notification events
	int fLoops;					// how many times will this animation loop (after initial playthrough?)

	bool fDoAdvanceTo;			// advance to a specific stage instead of n + 1
	UInt32 fAdvanceTo;			// the stage to advance to, provided fDoAdvanceTo is true
	bool fDoRegressTo;			// regress to a specific stage instaed of n - 1
	UInt32 fRegressTo;			// the stage to regress true, provided fDoRegressTo is true

	// --- these are derived & kept for bookkeeping
	plAGAnimInstance *fAnimInstance;
	plArmatureMod *fArmature;
	plArmatureBrain *fBrain;			// only valid while attached
	plMultistageBehMod *fMod;			// where we came from
	float fLocalTime;					// our time within the animation
	float fLength;						// the length of our animation?
	int fCurLoop;						// which loop are we currently in?
	bool fAttached;						// in the middle of reloading
	bool fAnimatedHandle;				// this animation moves the handle
	UInt8 fSentNotifies;				// which notifies have we sent?
	bool fReverseOnIdle;				// reverse our key interpretation if we stop. this is a special
										// case for down ladders, for which the forward button means "keep going down"
										// if you hold it down the whole time, but means "go up" if you press it 
										// again after releasing it.
	bool fDone;							// We've reached the end of the anim -- may be fading out, or paused.

};

class plAnimStageVec : public std::vector<plAnimStage*>
{
};

#endif // PL_ANIM_STAGE_INC
