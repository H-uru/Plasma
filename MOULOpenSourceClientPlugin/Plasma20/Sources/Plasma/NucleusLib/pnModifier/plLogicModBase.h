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

#ifndef plLogicModBase_inc
#define plLogicModBase_inc

#include "plSingleModifier.h"
#include "../pnNetCommon/plSynchedValue.h"
#include "hsTemplates.h"

class plConditionalObject;
class plSceneObject;
class plNotifyMsg;
class plVolumeSensorConditionalObjectNoArbitration;
class plLogicModBase : public plSingleModifier
{
public:
	enum Flags
	{
		kLocalElement	= 0,
		kReset,
		kTriggered,
		kOneShot,
		kRequestingTrigger,
		kTypeActivator,			// this LogicMod is part of an Activator Component (not a Responder)
		kMultiTrigger,
	};

protected:
	hsTArray<plMessage*>			fCommandList;
	hsTArray<plKey>					fReceiverList;
	UInt32							fCounterLimit;
	hsScalar						fTimer;
	hsBitVector						fFlags;
	UInt32							fCounter;
	plNotifyMsg*					fNotify;
	bool							fDisabled;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) {return false;}
	void IUpdateSharedState(bool triggered) const;
	hsBool IEvalCounter();
	virtual void PreTrigger(hsBool netRequest);
	virtual void Trigger(hsBool netRequest);
	virtual void UnTrigger();
	
	void CreateNotifyMsg();
	
public:
	friend plVolumeSensorConditionalObjectNoArbitration;
	plLogicModBase();
	~plLogicModBase();
	CLASSNAME_REGISTER( plLogicModBase );
	GETINTERFACE_ANY( plLogicModBase, plSingleModifier );

	void AddTarget(plSceneObject* so);
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);
	virtual hsBool VerifyConditions(plMessage* msg) { return true;}

	virtual void Reset(bool bCounterReset);

	void SetDisabled(bool disabled) { fDisabled = disabled; }
	bool Disabled() { return fDisabled; }

	plNotifyMsg* GetNotify() { return fNotify; }

	void AddCommand(plMessage* msg) { fCommandList.Append(msg); }
	void SetOneShot(hsBool b) { if (b) SetFlag(kOneShot); else ClearFlag(kOneShot); }
	void RegisterForMessageType(UInt16 hClass);

	virtual void RequestTrigger(hsBool netRequest=false);
	virtual void RequestUnTrigger() { UnTrigger(); }

	hsBool	HasFlag(int f) const { return fFlags.IsBitSet(f); }
	void	SetFlag(int f) { fFlags.SetBit(f); }
	void	ClearFlag(int which) { fFlags.ClearBit(which); }

	void AddNotifyReceiver(plKey receiver);

	// for debug purposes only!
	void ConsoleTrigger(plKey playerKey);
	void ConsoleRequestTrigger();
};



#endif // plLogicModifier_inc
