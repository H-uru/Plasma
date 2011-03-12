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

#ifndef plAnimCmdMsg_inc
#define plAnimCmdMsg_inc

#include "../pnMessage/plMessageWithCallbacks.h"
#include "hsBitVector.h"
#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "../plInterp/plAnimEaseTypes.h"
#include "../plInterp/plAnimTimeConvert.h"

class plAGAnimInstance;

class plAnimCmdMsg : public plMessageWithCallbacks
{
protected:
	char *fAnimName;
	char *fLoopName;

private:
	void IInit() { fBegin=fEnd=fLoopBegin=fLoopEnd=fSpeed=fSpeedChangeRate=fTime=0; fAnimName=fLoopName=nil;}
public:
	plAnimCmdMsg()
		: plMessageWithCallbacks(nil, nil, nil) { IInit(); }
	plAnimCmdMsg(const plKey &s, 
				const plKey &r, 
				const double* t)
		: plMessageWithCallbacks(s, r, t) { IInit(); }
	virtual ~plAnimCmdMsg();

	CLASSNAME_REGISTER( plAnimCmdMsg );
	GETINTERFACE_ANY( plAnimCmdMsg, plMessageWithCallbacks );

	// When adding a command, add a check for it in CmdChangesAnimTime if appropriate
	enum ModCmds
	{
		kContinue=0,
		kStop,
		kSetLooping,
		kUnSetLooping,
		kSetBegin,
		kSetEnd,
		kSetLoopEnd,
		kSetLoopBegin,
		kSetSpeed,
		kGoToTime,
		kSetBackwards,
		kSetForewards,
		kToggleState,
		kAddCallbacks,
		kRemoveCallbacks,
		kGoToBegin,
		kGoToEnd,
		kGoToLoopBegin,
		kGoToLoopEnd,
		kIncrementForward,
		kIncrementBackward,
		kRunForward,
		kRunBackward,
		kPlayToTime,
		kPlayToPercentage,
		kFastForward,
		kGoToPercent,
		kNumCmds,
	};

	hsBitVector		fCmd;

	hsBool Cmd(int n) const { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd();
	void SetAnimName(const char *name);
	const char *GetAnimName();
	hsBool CmdChangesAnimTime(); // Will this command cause an update to the current anim time?

	void SetLoopName(const char *name);
	const char *GetLoopName();

	hsScalar fBegin;
	hsScalar fEnd;
	hsScalar fLoopEnd;
	hsScalar fLoopBegin;
	hsScalar fSpeed;
	hsScalar fSpeedChangeRate;
	hsScalar fTime;

	// IO
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

// plAnimCmdMsg is intented for animation commands sent to a plAnimTimeConvert. Commands that only apply to the
// AG (Animation Graph) system go here.

class plAGCmdMsg : public plMessage
{
protected:
	char *fAnimName;

private:
	void IInit() { fBlend = fAmp = 0;
				   fAnimName=nil;}
public:
	plAGCmdMsg()
		: plMessage(nil, nil, nil) { IInit(); }
	plAGCmdMsg(const plKey &s, 
			   const plKey &r, 
			   const double* t)
		: plMessage(s, r, t) { IInit(); }
	virtual ~plAGCmdMsg();

	CLASSNAME_REGISTER( plAGCmdMsg );
	GETINTERFACE_ANY( plAGCmdMsg, plMessage );

	enum ModCmds
	{
		kSetBlend,
		kSetAmp,
		kSetAnimTime,
	};

	hsBitVector		fCmd;

	hsBool Cmd(int n) const { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd() { fCmd.Clear(); }
	void SetAnimName(const char *name);
	const char *GetAnimName();

	hsScalar fBlend;
	hsScalar fBlendRate;
	hsScalar fAmp;
	hsScalar fAmpRate;
	hsScalar fAnimTime;

	// IO
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

class plAGInstanceCallbackMsg : public plEventCallbackMsg
{
public:
	plAGInstanceCallbackMsg() : plEventCallbackMsg(), fInstance(nil) {}
	plAGInstanceCallbackMsg(plKey receiver, CallbackEvent e, int idx=0, hsScalar t=0, Int16 repeats=-1, UInt16 user=0) :
	  plEventCallbackMsg(receiver, e, idx, t, repeats, user), fInstance(nil) {}

	CLASSNAME_REGISTER( plAGInstanceCallbackMsg );
	GETINTERFACE_ANY( plAGInstanceCallbackMsg, plEventCallbackMsg );

	// These aren't meant to go across the net, so no IO necessary.
	void Read(hsStream* stream, hsResMgr* mgr) {}
	void Write(hsStream* stream, hsResMgr* mgr) {}

	plAGAnimInstance *fInstance;
};

class plAGDetachCallbackMsg : public plEventCallbackMsg
{
protected:
	char *fAnimName;

public:
	plAGDetachCallbackMsg() : plEventCallbackMsg(), fAnimName(nil) {}
	plAGDetachCallbackMsg(plKey receiver, CallbackEvent e, int idx=0, hsScalar t=0, Int16 repeats=-1, UInt16 user=0) :
						  plEventCallbackMsg(receiver, e, idx, t, repeats, user), fAnimName(nil) {}
	virtual ~plAGDetachCallbackMsg();

	CLASSNAME_REGISTER( plAGDetachCallbackMsg );
	GETINTERFACE_ANY( plAGDetachCallbackMsg, plEventCallbackMsg );
	
	// These aren't meant to go across the net, so no IO necessary.
	void Read(hsStream* stream, hsResMgr* mgr) {}
	void Write(hsStream* stream, hsResMgr* mgr) {}
	
	void SetAnimName(const char *name);
	char *GetAnimName();
};


#endif // plAnimCmdMsg_inc
