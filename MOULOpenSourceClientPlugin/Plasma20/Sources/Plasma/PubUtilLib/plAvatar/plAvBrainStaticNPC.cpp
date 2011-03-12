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
//class plAvBrainStaticNPC : public plAvBrain
//{
//	
//};
//
///** \class plAvBrain
//	Virtual base class for the modular avatar & creature brains.
//	A brain is a modular object which can be installed into a plArmatureMod
//	to drive it around in the scene. Some brains are for creature ai;
//	others intepret user input. The most interesting brain for reference is
//	probably plAvBrainHuman, which implements the control system for the
//	user's avatar.
//	/sa plAvBrainHuman, plArmatureMod
//*/
//class plAvBrainStaticNPC : public plAvBrainStaticNPC
//{
//public:
//	/** Default constructor - constructs the brain but does not attach it.
//		Brains are always constructed before being Pushed onto the armature
//		they seek to control. */
//	plAvBrain();
//	
//	/** Destructor. Automatically called on a brain when it is popped from the armature. */
//	virtual ~plAvBrain();
//
//	// BRAIN PROTOCOL
//	/** This brain has just been freshly pushed onto an armature and is now
//		in primary control. Note that brains beneath it on the stack may
//		still have effect on the armature; any messages which the top
//		brain doesn't handle will propagate down to the next brain in line.
//	*/
//	virtual hsBool Activate(plArmatureMod *avMod);
//
//	/** Has the brain resolved all its load-time dependencies? This is a mechanism
//		to allow newly loading creatures to reach a known state before they
//		are asked to load secondary state or to interact with the environment.
//	*/
//	virtual hsBool IsReady();
//
//	/** This brain has just been removed from its armature and is about to be destructed. */
//	virtual hsBool Deactivate();
//
//	/** This is the top brain and it's time for it to evaluate. Called during eval
//		time for the armature modifier. Only the topmost brain gets an apply
//		call; others must do any necessary background work during MsgReceive. */
//	virtual hsBool Apply(double timeNow, hsScalar elapsed);
//
//	/** Another brain has been pushed atop this brain. Drop into the background.
//		We'll still receive any messages that the upper brain doesn't eat. */
//	virtual hsBool Suspend();
//
//	/** We were suspended, but now we're on top of the brain stack again. */
//	virtual hsBool Resume();
//
//	// \name Spawning \{
//	/** Do any necessary custom action upon spawning afresh in a new age.
//		For the human brain, this simply consists of noting that we have
//		spawned and we can stop trying to do so. */
//	virtual void Spawn(double timeNow) {};
//
//	/** Custom behavior for entering an age. Binding the camera, audio source, etc. */
//	virtual void EnterAge(hsBool reSpawn) {};
//
//	/** Custom behavior for leaving an age. Free any attachments to camera, audio, etc. */
//	virtual void LeaveAge() {};
//
//	// TASKS
//	// tasks are operations that must be done in sequence.
//	/** Push a new task onto the end of our FIFO task queue. Will not 
//		run until all the tasks ahead of it have run. 
//		\sa plAvTask
//	*/
//	virtual void QueueTask(plAvTask *task);
//
//
//	virtual void DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);
//
//
//	// PLASMOTICS
//	CLASSNAME_REGISTER( plAvBrain );
//	GETINTERFACE_ANY( plAvBrain, plCreatable );
//
//	virtual void Write(hsStream *stream, hsResMgr *mgr);
//	virtual void Read(hsStream *stream, hsResMgr *mgr);
//
//	virtual void SaveAux(hsStream *stream, hsResMgr *mgr);
//	virtual void LoadAux(hsStream *stream, hsResMgr *mgr, double time);
//
//	virtual hsBool MsgReceive(plMessage *msg);
//
//
//protected:
//	plArmatureMod	*fAvMod;			// the avatar we're controlling
//
//	// TASKS
//	// -- variables
//	typedef std::deque<plAvTask *> plAvTaskQueue;
//	plAvTaskQueue			fTaskQueue;						// FIFO queue of tasks we're working on
//	plAvTask				*fCurTask;						// the task we're working on right now
//	// -- methods
//	virtual hsBool IHandleTaskMsg(plAvTaskMsg *msg);	// respond to a task scheduling message
//	void IProcessTasks(double time, hsScalar elapsed);	// process current task and start new one if necessary
//	
//	hsBool	fSuspended;
//};
//
