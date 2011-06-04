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
#ifndef _plNotifyMsg_h_
#define _plNotifyMsg_h_

#include "plMessage.h"
#include "hsResMgr.h"
#include "../pnModifier/plSingleModifier.h"
#include "hsUtils.h"
#include "hsGeometry3.h"


//
//  repeatable Event data in Notify message
//
// Updated 4.13.2002 mcn - Unions are all well and good, but only work on simple data types, and since
// plKey now isn't so simple...

class proEventData
{
public:
	// keep this enum synchronized with the list at the top of PlasmaTypes.py
	enum eventType
	{
		kCollision=1,
		kPicked =2,
		kControlKey=3,
		kVariable=4,
		kFacing=5,
		kContained=6,
		kActivate=7,
		kCallback=8,
		kResponderState=9,
		kMultiStage=10,
		kSpawned=11,
		kClickDrag=12,
		kCoop=13,
		kOfferLinkingBook=14,
		kBook=15,
		kClimbingBlockerHit=16,
		kNone
	};

	proEventData( Int32 evtType = kNone ) : fEventType( evtType )
	{
	}
	virtual ~proEventData() {}

	enum dataType
	{
		kNumber=1,
		kKey,
		kNotta
	};

	// what the multstage event types are for multistage.fEvent
	enum multiStageEventType
	{
		kEnterStage=1,
		kBeginingOfLoop,
		kAdvanceNextStage,
		kRegressPrevStage,
		kNothing
	};

	Int32 fEventType;		// what type of event (evenType enum)

	static proEventData* ICreateEventDataType(Int32 type);

	static proEventData* Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	static proEventData* ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);

protected:
	virtual void IInit() {}
	virtual void IDestruct() {}
	virtual void IRead(hsStream* stream, hsResMgr* mgr) {}
	virtual void IWrite(hsStream* stream, hsResMgr* mgr) {}

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr) {}
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr) {}
};

// The following macro makes it easy (or easier) to define new event data types
#define proEventType(type) class pro##type##EventData : public proEventData { public: pro##type##EventData() : proEventData( k##type ) {IInit();} virtual ~pro##type##EventData(){IDestruct();}

proEventType(Collision)
	hsBool	fEnter;		// entering? (false is exit)
	plKey	fHitter;	// collision hitter (other probably player)
	plKey	fHittee;	// collision hittee (probably us)

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Picked)
	plKey	fPicker;	// who picked it (problably player)
	plKey	fPicked;	// object that was picked
	hsBool	fEnabled;	// pick (true) or un-pick (false)
	hsPoint3 fHitPoint;	// where on the picked object that it was picked on

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Spawned)
	plKey	fSpawner;	// who spawned it (typicall plNPCSpawnMod)
	plKey	fSpawnee;	// what was spawned (typically a scene object with an armature mod)

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(ControlKey)
	Int32	fControlKey;	// what control key was hit
	hsBool	fDown;			// was the key going down (false if going up)

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Variable)
	char*	fName;			// name of variable
	Int32	fDataType;		// type of data

	// Can't be a union, sadly, but it isn't that much of a waste of space...
	hsScalar	fNumber;	// if its a number
	plKey		fKey;		// if its a plKey (pointer to something)


protected:
	virtual void IInit();
	virtual void IDestruct();
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Facing)
	plKey		fFacer; // what was facing
	plKey		fFacee; // what was being faced
	hsScalar	dot;	 // the dot prod of their view vectors
	hsBool		enabled; // Now meets facing requirement (true) or no longer meets requirement (false)

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Contained)
	plKey	fContained;	// who's inside of 
	plKey	fContainer;	// this containing volume
	hsBool	fEntering;	// entering volume (true) or exiting (false)

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Activate)
	hsBool fActive;		// Whether or not to use the data in this field
	hsBool fActivate;	// the data

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};		

proEventType(Callback)
	Int32	fEventType;  // enumerated in plEventCallbackMsg.h

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(ResponderState)
	Int32	fState;		// what state the responder should be switched to before triggering

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(MultiStage)
	Int32	fStage;	
	Int32	fEvent;
	plKey	fAvatar;	// who was running the stage

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Coop)
	UInt32	fID;		// player ID of the initiator
	UInt16	fSerial;	// serial number for the initiator
protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};


proEventType(ClickDrag)
	plKey picker; // always the local avatar in this case
	plKey picked;
	hsScalar animPos; // 0.0 to 1.0 animation percentage
};

proEventType(OfferLinkingBook)
	plKey offerer; // the avatar offering the linking book to you
	int targetAge; // the age the book is for - taken from konstant list of age buttons in xLinkingBookPopupGUI.py
	int offeree; // who we are offering the book to
protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(Book)
	UInt32	fEvent;		// The type of event. See pfJournalBook.h for enumsu
	UInt32	fLinkID;	// The link ID of the image clicked, if an image link event, otherwise unused
protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};

proEventType(ClimbingBlockerHit)
	plKey	fBlockerKey;	// collision hittee (probably us)

protected:
	virtual void IRead(hsStream* stream, hsResMgr* mgr);
	virtual void IWrite(hsStream* stream, hsResMgr* mgr);

	virtual void IReadVersion(hsStream* s, hsResMgr* mgr);
	virtual void IWriteVersion(hsStream* s, hsResMgr* mgr);
};


/////////////////////////////////////////////////////////////////////////////
//
//  MESSAGE    : plNotifyMsg
//  PARAMETERS : none
//
//  PURPOSE    : This is the message that notifies someone (either a responder or activator)
//             : that some event or transition of state has happened
//
//
class plNotifyMsg : public plMessage
{
protected:
	virtual void IInit();

public:
	plNotifyMsg() { IInit(); }
	plNotifyMsg(const plKey &s, const plKey &r);
	~plNotifyMsg();

	CLASSNAME_REGISTER( plNotifyMsg );
	GETINTERFACE_ANY( plNotifyMsg, plMessage );

	// data area
	enum notificationType
	{
		kActivator=0,
		kVarNotification,
		kNotifySelf,

		kResponderFF,			// Fast forward
		kResponderChangeState,	// Change state without triggering
	};
	Int32		fType;				// what type of notification
	hsScalar	fState;				// state of the notifier 0.0=false, 1.0=true
	Int32		fID;				// special ID mostly for responder State transitions
	hsTArray<proEventData*> fEvents;// list of events with data

	void SetType(notificationType type) { fType = type; }
	void SetState(hsScalar state) { fState = state; }

	// event records for the notify message
	void AddEvent( proEventData* ed);
	void AddCollisionEvent( hsBool enter, const plKey &other, const plKey &self, hsBool onlyOneCollision=true );
	void AddPickEvent( const plKey &other, const plKey& self, hsBool enabled, hsPoint3 hitPoint );
	void AddControlKeyEvent( Int32 key, hsBool down );
	void AddVariableEvent( const char* name, hsScalar number );
	void AddVariableEvent( const char *name, const plKey &key);
	void AddFacingEvent( const plKey &other, const plKey &self, hsScalar dot, hsBool enabled);
	void AddContainerEvent( const plKey &container, const plKey &contained, hsBool entering);
	void AddActivateEvent( hsBool activate );
	void AddCallbackEvent( Int32 event );
	void AddResponderStateEvent( Int32 state );
	void AddMultiStageEvent( Int32 stage, Int32 event, const plKey& avatar );
	void AddCoopEvent(UInt32 id, UInt16 serial);
	void AddSpawnedEvent (const plKey &spawner, const plKey &spawned);
	void AddClickDragEvent(const plKey& dragger, const plKey& dragee, hsScalar animPos);
	void AddOfferBookEvent(const plKey& offerer, int targetAge, int offeree);
	void AddBookEvent( UInt32 event, UInt32 linkID = 0 );
	void AddHitClimbingBlockerEvent(const plKey &blocker);
	proEventData* FindEventRecord( Int32 eventtype );
	Int32 GetEventCount() { return fEvents.Count(); }
	proEventData* GetEventRecord(Int32 i) { return fEvents[i]; }
	void ClearEvents();

	// Searches the event records for an event triggered by an avatar, and returns that key
	plKey GetAvatarKey();

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
	
	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};



#endif // _plNotifyMsg_h_
