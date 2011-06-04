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
#ifndef plCoopCoordinator_h
#define plCoopCoordinator_h

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

// global
#include "../pnKeyedObject/hsKeyedObject.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plAvBrainCoop;
class plMessage;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

/** \class plCoopCoordinator
	Manages cooperation betweeen several generic brains; primarily serves as a place
	to send synchronization messages. Originally this was going to be the role of the
	'host' brain, but that didn't provide a good interface point for Python.
	This object can either be a "helper" for Python, or its functionality can be
	recoded in Python.
	*/
class plCoopCoordinator : public hsKeyedObject
{
public:
	plCoopCoordinator();
	plCoopCoordinator(plKey host, plKey guest,
					  plAvBrainCoop *hostBrain, plAvBrainCoop *guestBrain,
					  const char *synchBone, UInt32 hostOfferStage, UInt32 guestAcceptStage,
					  plMessage *guestAcceptMsg,
					  bool autoStartGuest);
	~plCoopCoordinator();

	virtual hsBool MsgReceive(plMessage *msg);

	void Run();

	UInt32 GetInitiatorID();
	UInt16 GetInitiatorSerial();

	bool IsActiveForReal();

	// rtti
	CLASSNAME_REGISTER( plCoopCoordinator );
	GETINTERFACE_ANY( plCoopCoordinator, hsKeyedObject);

	// i/o
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

protected:
	void IStartHost();
	void IStartGuest();
	void IContinueGuest();
	void IAdvanceParticipant(bool host);

	void IStartTimeout();
	void ITimeout();

	plKey fHostKey;
	plKey fGuestKey;

	plAvBrainCoop *fHostBrain;
	plAvBrainCoop *fGuestBrain;

	UInt32 fInitiatorID;
	UInt32 fInitiatorSerial;
	
	UInt32	fHostOfferStage;			// when we enter this stage, the offer is ready
	UInt32	fGuestAcceptStage;			// when we enter this stage, the offer is accepted

	plMessage *fGuestAcceptMsg;			// send this when the guest accepts

	char *fSynchBone;
	bool fAutoStartGuest;
	bool fGuestAccepted;

	bool fGuestLinked; // guest linked, so ignore the timeout timer
};

#endif // plCoopCoordinator_h