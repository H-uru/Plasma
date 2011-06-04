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
#ifndef NO_AV_MSGS
#ifndef SERVER


#ifndef plAvCoopMsg_inc
#define plAvCoopMsg_inc

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
#include "../pnMessage/plMessage.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plCoopCoordinator;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

/** plAvCoopMsg -------------------------------------------------------------------------
	These are always sent to the avatar manager, which sends the appropriate things to
	the other avatar.
	A possible future optimization would be to send them only to the involved players.
*/
class plAvCoopMsg : public plMessage
{
public:

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// TYPES
	//
	/////////////////////////////////////////////////////////////////////////////////////

	enum Command {
		kNone,
		kStartNew,
		kGuestAccepted,
		kGuestSeeked,
		kGuestSeekAbort,
		kCommandSize	= 0xffff
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// INTERFACE
	//
	/////////////////////////////////////////////////////////////////////////////////////

	// constructors
	plAvCoopMsg();
	plAvCoopMsg(Command cmd, UInt32 id, UInt16 serial);
	plAvCoopMsg(plKey sender, plCoopCoordinator *coordinateur);
	~plAvCoopMsg();

	// rtti
	CLASSNAME_REGISTER( plAvCoopMsg );
	GETINTERFACE_ANY( plAvCoopMsg, plMessage);

	// i/o
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// PUBLIC DATA MEMBERS
	//
	/////////////////////////////////////////////////////////////////////////////////////

	UInt32 fInitiatorID;
	UInt16 fInitiatorSerial;

	Command	fCommand;

	plCoopCoordinator *fCoordinator;		// optional
};

#endif

#endif // ndef SERVER
#endif // ndef NO_AV_MSGS
