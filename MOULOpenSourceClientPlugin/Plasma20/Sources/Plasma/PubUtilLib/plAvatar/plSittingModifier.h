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
#ifndef plSittingModifier_inc
#define plSittingModifier_inc

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

#include "../pnModifier/plSingleModifier.h"	// base class
#include "../pnKeyedobject/plKey.h"			// for the notification keys
#include "hsTemplates.h"					// for the array they're kept in

/////////////////////////////////////////////////////////////////////////////////////////
//
// DECLARATIONS
//
/////////////////////////////////////////////////////////////////////////////////////////
class plNotifyMsg;
class plAvBrainGeneric;
class plArmatureMod;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plSittingModifier : public plSingleModifier
{
public:	
	enum
	{
		kApproachFront	= 0x01,
		kApproachLeft	= 0x02,
		kApproachRight	= 0x04,
		kApproachRear	= 0x08,
		kApproachMask	= kApproachFront | kApproachLeft | kApproachRight | kApproachRear,
		kDisableForward = 0x10,
	};
	
	UInt8 fMiscFlags;	

	plSittingModifier();
	plSittingModifier(bool hasFront, bool hasLeft, bool hasRight);
	virtual ~plSittingModifier();

	CLASSNAME_REGISTER( plSittingModifier );
	GETINTERFACE_ANY( plSittingModifier, plSingleModifier );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage *msg);

	void AddNotifyKey(plKey key) { fNotifyKeys.Append(key); }

	virtual void Trigger(const plArmatureMod *avMod, plNotifyMsg *enterNotify, plNotifyMsg *exitNotify);
	virtual void UnTrigger();

protected:
	/** We've been triggered: go ahead and send the seek and brain tasks to the 
		triggering avatar. */
	hsBool IEmitCommand(plKey playerKey, plMessage *enterCallback, plMessage *exitCallback);

	/** Set up generic notification messages which were passed in by the responder / 
		max authoring stuff. */
	void ISetupNotify(plNotifyMsg *notifyMsg, plNotifyMsg *originalNotify);

	/** Figure out which approach we should use to the sit target, and add the relevant
		stages to the brain. */
	plAvBrainGeneric * IBuildSitBrain(plKey avModKey, plKey seekKey,char **pAnimName, plNotifyMsg *enterNotify, plNotifyMsg *exitNotify);

	/** Unused. */
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return true; }

	/** An array of keys to objects that are interested in receiving our sit messages. */
	hsTArray<plKey> fNotifyKeys;

	/** The chair in question is in use. It will untrigger when the avatar leaves it. */
	//hsBool			fTriggered;
	plKey			fTriggeredAvatarKey;
};



#endif plSittingModifier_inc
