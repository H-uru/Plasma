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
#ifndef PLAVBRAINGENERIC_H
#define PLAVBRAINGENERIC_H

#include "hsStlUtils.h"
#include "plAvBrainGeneric.h"

/** \class plAvBrainCoop
	This is currently no different whatsoever from plAvBrainGeneric,
	but it's quite possible that it will need to vary, so we're
	keeping it around for the moment. */
class plAvBrainCoop : public plAvBrainGeneric
{
public:

	// used only by the class factory...
	plAvBrainCoop();

	// use this constructor for a host brain; it sets up the unique ID
	plAvBrainCoop(UInt32 exitFlags, float fadeIn, float fadeOut, MoveMode moveMode, plKey guestKey);
	
	// use this constructor for the guest brain, when you already have the unique ID
	plAvBrainCoop(UInt32 exitFlags, float fadeIn, float fadeOut, MoveMode moveMode,
				  UInt32 initiatorID, UInt16 initiatorSerial, plKey hostKey);

	hsBool MsgReceive(plMessage *msg);
	virtual bool RelayNotifyMsg(plNotifyMsg *msg);
	void EnableGuestClick();

	// rtti
	CLASSNAME_REGISTER( plAvBrainCoop );
	GETINTERFACE_ANY( plAvBrainCoop, plAvBrainGeneric);

	// i/o
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

	// stuff
	UInt32 GetInitiatorID();
	UInt16 GetInitiatorSerial();

	virtual plKey GetRecipient();
	virtual void SetRecipient(plKey &recipient);
	
private:
	UInt32 fInitiatorID;
	UInt16 fInitiatorSerial;

	plKey fGuestKey;	// only filled out if we are the host
	plKey fHostKey;		// only filled out if we are the guest

	bool fWaitingForClick;

	std::vector<plKey> fRecipients; // we have an array for a slight hack so relto book sharing works
};	

#endif