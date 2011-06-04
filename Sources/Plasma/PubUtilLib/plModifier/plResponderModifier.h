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
#ifndef plResponderModifier_inc
#define plResponderModifier_inc

#include "hsStlUtils.h"
#include "../pnModifier/plSingleModifier.h"
#include "../pnMessage/plMessage.h"


class plNotifyMsg;
class plAnimCmdMsg;
class plResponderSDLModifier;
class plResponderModifier : public plSingleModifier
{
	friend class plResponderSDLModifier;
protected:
	typedef std::map<Int8,Int8> WaitToCmd;

	class plResponderCmd
	{
	public:
		plResponderCmd() : fMsg(nil), fWaitOn(-1) {}
		plResponderCmd(plMessage *msg, Int8 waitOn) : fMsg(msg), fWaitOn(waitOn) {}

		plMessage *fMsg;
		Int8 fWaitOn;		// Index into fCompletedEvents of who we're waiting on
	};
	class plResponderState
	{
	public:
		hsTArray<plResponderCmd> fCmds;
		Int8 fNumCallbacks;			// So we know how far to search into the bitvector to find out when we're done
		Int8 fSwitchToState;		// State to switch to when all commands complete
		WaitToCmd fWaitToCmd;
	};

	hsTArray<plResponderState> fStates;

	Int8 fCurState;					// The current state (first index for fCommandList)
	Int8 fCurCommand;				// The command we are currently waiting to send (or -1 if we're not sending)
	bool fNetRequest;				// Was the last trigger a net request
	hsBitVector fCompletedEvents;	// Which events that commands are waiting on have completed
	bool fEnabled;
	plKey fPlayerKey;				// The player who triggered this last
	plKey fTriggerer;				// Whoever triggered us (for sending notify callbacks)
	hsBool fEnter;					// Is our current trigger a volume enter?
	bool fGotFirstLoad;				// Have we gotten our first SDL load?

	plResponderSDLModifier* fResponderSDLMod;		// handles saving and restoring state

	enum
	{
		kDetectTrigger		= 0x1,
		kDetectUnTrigger	= 0x2,
		kSkipFFSound		= 0x4
	};
	UInt8 fFlags;
	UInt32 fNotifyMsgFlags;	// store the msg flags of the notify which triggered us

	void Trigger(plNotifyMsg *msg);
	bool IIsLocalOnlyCmd(plMessage* cmd);
	bool IContinueSending();

	Int8 ICmdFromWait(Int8 waitIdx);

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return true; }

	static bool fDebugAnimBox;	// Draws a box on screen when an animation is started
	static void IDebugAnimBox(bool start);
	void IDebugPlayMsg(plAnimCmdMsg* msg);

	// Trigger the responder (regardless of what it's doing) and "fast forward" it to the final state
	// If python is true, only run animations
	void IFastForward(bool python);
	// If the message is FF-able, returns it (or a FF-able version)
	plMessage* IGetFastForwardMsg(plMessage* msg, bool python);

	void ISetResponderStateFromNotify(plNotifyMsg* msg);
	void ISetResponderState(Int8 state);

	void ILog(UInt32 color, const char* format, ...);

	friend class plResponderComponent;
	friend class plResponderWait;

public:
	plResponderModifier();
	~plResponderModifier();

	CLASSNAME_REGISTER( plResponderModifier );
	GETINTERFACE_ANY( plResponderModifier, plSingleModifier );
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	const plResponderSDLModifier* GetSDLModifier() const { return fResponderSDLMod; }

	static bool ToggleDebugAnimBox() { return fDebugAnimBox = !fDebugAnimBox; }
	static void NoLogString(const char* str);

	// Restore callback state after load
	void Restore();
	
	const Int8 GetState() const { return fCurState; }
	//
	// Export time only
	//
	void AddCommand(plMessage* pMsg, int state=0);
	void AddCallback(Int8 state, Int8 cmd, Int8 callback);
};

// Message for changing the enable state in a responder modifier
class plResponderEnableMsg : public plMessage
{
public:
	bool fEnable;

	plResponderEnableMsg() : fEnable(true) {}
	plResponderEnableMsg(bool enable) : fEnable(enable) {}

	CLASSNAME_REGISTER(plResponderEnableMsg);
	GETINTERFACE_ANY(plResponderEnableMsg, plMessage);

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plResponderModifier_inc
