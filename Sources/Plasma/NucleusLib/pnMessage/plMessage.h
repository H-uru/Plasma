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

#ifndef plMessage_inc
#define plMessage_inc

#include "../pnFactory/plCreatable.h"
#include "../pnKeyedObject/plKey.h"
#include "hsTemplates.h"
#include "hsStlUtils.h"

class plKey;
class hsStream;

// Base class for messages only has enough info to route it
// and send it over the wire (Read/Write).
class plMessage : public plCreatable
{
public:
	typedef UInt16	plStrLen;

	enum plBCastFlags {
		kBCastNone				= 0x0,
		kBCastByType			= 0x1,	// To everyone registered for this msg type or msgs this is derived from
		kBCastUNUSED_0			= 0x2,	// Obsolete option (never used). Was BCastBySender
		kPropagateToChildren	= 0x4,	// Propagate down through SceneObject heirarchy
		kBCastByExactType		= 0x8,	// To everyone registered for this exact msg type.
		kPropagateToModifiers	= 0x10,	// Send the msg to an object and all its modifier
		kClearAfterBCast		= 0x20,	// Clear registration for this type after sending this msg
		kNetPropagate			= 0x40,	// Propagate this message over the network (remotely)
		kNetSent				= 0x80, // Internal use-This msg has been sent over the network 
		kNetUseRelevanceRegions	= 0x100, // Used along with NetPropagate to filter the msg bcast using relevance regions
		kNetForce				= 0x200, // Used along with NetPropagate to force the msg to go out (ie. ignore cascading rules)
		kNetNonLocal			= 0x400, // Internal use-This msg came in over the network (remote msg)
		kLocalPropagate			= 0x800, // Propagate this message locally (ON BY DEFAULT)
		kNetNonDeterministic	= kNetForce, // This msg is a non-deterministic response to another msg	
		kMsgWatch				= 0x1000, // Debug only - will break in dispatch before sending this msg
		kNetStartCascade		= 0x2000, // Internal use-msg is non-local and initiates a cascade of net msgs. This bit is not inherited or computed, it's a property.
		kNetAllowInterAge		= 0x4000, // If rcvr is online but not in current age, they will receive msg courtesy of pls routing.
		kNetSendUnreliable		= 0x8000,  // Don't use reliable send when net propagating
		kCCRSendToAllPlayers	= 0x10000,	// CCRs can send a plMessage to all online players.
		kNetCreatedRemotely		= 0x20000,	// kNetSent and kNetNonLocal are inherited by child messages sent off while processing a net-propped
											// parent. This flag ONLY gets sent on the actual message that went across the wire.
				
	};
private:
	bool dispatchBreak;

	friend class plDispatch;
	friend class plDispatchLog;

protected:
	plKey					fSender;
	hsTArray<plKey>			fReceivers;
	double					fTimeStamp;

	UInt32					fBCastFlags;
	std::vector<UInt32>*	fNetRcvrPlayerIDs;

	void IMsgRead(hsStream* stream, hsResMgr* mgr);		// default read implementation
	void IMsgWrite(hsStream* stream, hsResMgr* mgr);	// default write implementation

	void IMsgReadVersion(hsStream* stream, hsResMgr* mgr);
	void IMsgWriteVersion(hsStream* stream, hsResMgr* mgr);

public:
	plMessage();
	plMessage(const plKey &s,
				const plKey &r,
				const double* t);

	virtual ~plMessage();
	
	CLASSNAME_REGISTER( plMessage );
	GETINTERFACE_ANY( plMessage, plCreatable );

	// These must be implemented by all derived message classes (hence pure).
	// Derived classes should call the base-class default read/write implementation, 
	// so the derived Read() should call plMessage::IMsgRead().
	virtual void Read(hsStream* stream, hsResMgr* mgr) = 0;
	virtual void Write(hsStream* stream, hsResMgr* mgr) = 0;

	const plKey				GetSender() const { return fSender; }
	plMessage&				SetSender(const plKey &s) { fSender = s; return *this; }

	plMessage&				SetNumReceivers(int n);
	UInt32					GetNumReceivers() const ;
	const plKey&			GetReceiver(int i) const;
	plMessage&				RemoveReceiver(int i);

	plMessage&				ClearReceivers();
	plMessage&				AddReceiver(const plKey &r);
	plMessage&				AddReceivers(const hsTArray<plKey>& rList);

	hsBool					Send(const plKey r=nil, hsBool async=false); // Message will self-destruct after send.
	hsBool					SendAndKeep(const plKey r=nil, hsBool async=false); // Message won't self-destruct after send.

	const double GetTimeStamp() const { return fTimeStamp; }
	plMessage& SetTimeStamp(double t) { fTimeStamp = t; return *this; }

	hsBool HasBCastFlag(UInt32 f) const { return 0 != (fBCastFlags & f); }
	plMessage& SetBCastFlag(UInt32 f, hsBool on=true) { if( on )fBCastFlags |= f; else fBCastFlags &= ~f; return *this; }
	
	void SetAllBCastFlags(UInt32 f) { fBCastFlags=f; }
	UInt32 GetAllBCastFlags() const { return fBCastFlags; }

	void AddNetReceiver( UInt32 plrID );
	void AddNetReceivers( const std::vector<UInt32> & plrIDs );
	std::vector<UInt32>* GetNetReceivers() const { return fNetRcvrPlayerIDs; }

	// just before dispatching this message, drop into debugger	
	void SetBreakBeforeDispatch (bool on) { dispatchBreak = on; }
	bool GetBreakBeforeDispatch () const { return dispatchBreak; }
};



/////////////////////////////////////////////////////////////////
// Helpers for reading/writing these types:
//		std::string
//		xtl::istring
//		c strings (char *)
//		c arrays (type [])


/////////////////////////////////////////////////////////////////
// reads/writes your std::string field

struct plMsgStdStringHelper
{
	static int Poke(const std::string & stringref, hsStream* stream, const UInt32 peekOptions=0);
	static int PokeBig(const std::string & stringref, hsStream* stream, const UInt32 peekOptions=0);
	static int Poke(const char * buf, UInt32 bufsz, hsStream* stream, const UInt32 peekOptions=0);
	static int PokeBig(const char * buf, UInt32 bufsz, hsStream* stream, const UInt32 peekOptions=0);
	static int Peek(std::string & stringref, hsStream* stream, const UInt32 peekOptions=0);
	static int PeekBig(std::string & stringref, hsStream* stream, const UInt32 peekOptions=0);
};

/////////////////////////////////////////////////////////////////
// reads/writes your xtl::istring field

struct plMsgXtlStringHelper
{
	static int Poke(const xtl::istring & stringref, hsStream* stream, const UInt32 peekOptions=0);
	static int Peek(xtl::istring & stringref, hsStream* stream, const UInt32 peekOptions=0);
};

/////////////////////////////////////////////////////////////////
// reads/writes your char * field

struct plMsgCStringHelper
{
	static int Poke(const char * str, hsStream* stream, const UInt32 peekOptions=0);
	// deletes str and reallocates. you must delete [] str;
	static int Peek(char *& str, hsStream* stream, const UInt32 peekOptions=0);
};

/////////////////////////////////////////////////////////////////
// reads/writes your type [] field
// don't use with byte ordered types like Int16,32.
// fine for Int8, char, and IEEE formatted types like float, double.

struct plMsgCArrayHelper
{
	static int Poke(const void * buf, UInt32 bufsz, hsStream* stream, const UInt32 peekOptions=0);
	static int Peek(void * buf, UInt32 bufsz, hsStream* stream, const UInt32 peekOptions=0);
};





#endif // plMessage_inc
