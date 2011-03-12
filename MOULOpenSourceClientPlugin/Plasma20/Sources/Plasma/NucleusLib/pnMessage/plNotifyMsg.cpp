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
/////////////////////////////////////////////////////////////////////////////
//
//  MESSAGE    : plNotifyMsg
//  PARAMETERS : none
//
//  PURPOSE    : This is the message that notifies someone (either a responder or activator)
//             : that some event or transition of state has happened
//
//

#include "plNotifyMsg.h"


plNotifyMsg::plNotifyMsg(const plKey &s, const plKey &r)
{
	fSender = s;
	AddReceiver(r);
	IInit();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IInit and ~plNotifyMsg
//  PARAMETERS : none
//
//  PURPOSE    : Initialization (called from constructors) and destructor
//
void plNotifyMsg::IInit()
{ 
	SetBCastFlag(plMessage::kNetPropagate);
	fType = kActivator;
	fState = 0.0f;			// start state at (completely) false
	fID = 0;
}

plNotifyMsg::~plNotifyMsg()
{
	ClearEvents();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddEvent
//  PARAMETERS : ed	    - pointer to event that needs to be added (be recreating)
//
//  PURPOSE    : Add an event record to this notify message
//
//
void plNotifyMsg::AddEvent( proEventData* ed )
{
	switch ( ed->fEventType )
	{
		case proEventData::kCollision:
			{
				proCollisionEventData *evt = (proCollisionEventData *)ed;
				AddCollisionEvent(evt->fEnter, evt->fHitter, evt->fHittee );
			}
			break;

		case proEventData::kSpawned:
			{
				proSpawnedEventData *evt = (proSpawnedEventData *)ed;
				AddSpawnedEvent( evt->fSpawner, evt->fSpawnee );
			}
			break;

		case proEventData::kPicked:
			{
				proPickedEventData *evt = (proPickedEventData *)ed;
				AddPickEvent( evt->fPicker, evt->fPicked, evt->fEnabled, evt->fHitPoint );
			}
			break;

		case proEventData::kContained:
			{
				proContainedEventData *evt = (proContainedEventData *)ed;
				AddContainerEvent( evt->fContained, evt->fContainer, evt->fEntering );
			}
			break;
		
		case proEventData::kCallback:
			{
				proCallbackEventData *evt = (proCallbackEventData *)ed;
				AddCallbackEvent( evt->fEventType );
			}
			break;

		case proEventData::kResponderState:
			{
				proResponderStateEventData *evt = (proResponderStateEventData *)ed;
				AddResponderStateEvent( evt->fState );
			}
			break;

		case proEventData::kMultiStage:
			{
				proMultiStageEventData *evt = (proMultiStageEventData *)ed;
				AddMultiStageEvent( evt->fStage, evt->fEvent, evt->fAvatar );
			}
			break;
			
		case proEventData::kCoop:
			{
				proCoopEventData *evt = (proCoopEventData *)ed;
				AddCoopEvent( evt->fID, evt->fSerial);
			}

		case proEventData::kControlKey:
			{
				proControlKeyEventData *evt = (proControlKeyEventData *)ed;
				AddControlKeyEvent( evt->fControlKey, evt->fDown );
			}
			break;

		case proEventData::kFacing:
			{
				proFacingEventData *evt = (proFacingEventData *)ed;
				AddFacingEvent( evt->fFacer, evt->fFacee, evt->dot, evt->enabled );
			}
			break;
		
		case proEventData::kActivate:
			{
				proActivateEventData *evt = (proActivateEventData *)ed;
				AddActivateEvent( evt->fActivate );
			}
			break;

		case proEventData::kVariable:
			{
				proVariableEventData *evt = (proVariableEventData *)ed;
				switch (evt->fDataType)
				{
					case proEventData::kNumber:
						AddVariableEvent(evt->fName, evt->fNumber);
						break;
					case proEventData::kKey:
						AddVariableEvent(evt->fName, evt->fKey);
						break;
				}
			}
			break;
		case proEventData::kClickDrag:
			{
				proClickDragEventData* evt = (proClickDragEventData*)ed;
				AddClickDragEvent(evt->picker, evt->picked, evt->animPos);
			}
			break;
		case proEventData::kOfferLinkingBook:
			{
				proOfferLinkingBookEventData* evt = (proOfferLinkingBookEventData*)ed;
				AddOfferBookEvent(evt->offerer, evt->targetAge, evt->offeree);
			}
		case proEventData::kBook:
			{
				proBookEventData* evt = (proBookEventData*)ed;
				AddBookEvent( evt->fEvent, evt->fLinkID );
			}
		case proEventData::kClimbingBlockerHit:
			{
				proClimbingBlockerHitEventData* evt = (proClimbingBlockerHitEventData*)ed;
				AddHitClimbingBlockerEvent(evt->fBlockerKey);
			}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddCollisionEvent
//  PARAMETERS : enter  - true for when entering collision and false when exiting collision
//             : other  - the plKey of the other object involved (Hitter)
//             : self   - the plKey to probably us (Hittee)
//
//  PURPOSE    : Add a collision event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddCollisionEvent( hsBool enter, const plKey &other, const plKey &self, hsBool onlyOneCollision )
{
	// if this is the normal case of there can only be one collision, then get rid of any others
	if ( onlyOneCollision )
	{
		// remove records that are like the one being added
		int num_recs = fEvents.GetCount();
		if ( num_recs > 0 )
		{
			int i;
			for ( i=0; i<num_recs; i++ )
			{
				// see if its the same type 
				proEventData* pEDTest = fEvents.Get(i);
				if ( pEDTest->fEventType == proEventData::kCollision )
				{
					// remove it
					delete fEvents[i];
					fEvents.Remove(i);
					// then jump out.. the count is no longer good and anyway there should only be one of the same type
					break;
				}
			}
		}
	}

	// create the collision event record
	proCollisionEventData* pED = TRACKED_NEW proCollisionEventData;
	pED->fEnter = enter;
	pED->fHitter = other;
	pED->fHittee = self;
	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddCallbackEvent
//  PARAMETERS : event  - the event type, as enumerated in plEventCallbackMsg.h
//
//
void plNotifyMsg::AddCallbackEvent( Int32 event )
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kCallback )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the collision event record
	proCallbackEventData* pED = TRACKED_NEW proCallbackEventData;
	pED->fEventType = event;
	fEvents.Append(pED);	// then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddResponderStateEvent
//  PARAMETERS : state  - the state for the responder to switch to before triggering
//
//
void plNotifyMsg::AddResponderStateEvent( Int32 state )
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kResponderState )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the collision event record
	proResponderStateEventData* pED = TRACKED_NEW proResponderStateEventData;
	pED->fState = state;
	fEvents.Append(pED);	// then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddMultiStageEvent
//  PARAMETERS : stage   - the stage the multistage behavior is on
//             : event   - what was the event that happened
//
//
void plNotifyMsg::AddMultiStageEvent( Int32 stage, Int32 event, const plKey& avatar )
{
	// we can have multi events of this type
	// create the mutlistage event record
	proMultiStageEventData* pED = TRACKED_NEW proMultiStageEventData;
	pED->fStage = stage;
	pED->fEvent = event;
	pED->fAvatar = avatar;
	fEvents.Append(pED);	// then add it to the list of event records
}

void plNotifyMsg::AddCoopEvent(UInt32 id, UInt16 serial)
{
	proCoopEventData *pED = TRACKED_NEW proCoopEventData;
	pED->fID = id;
	pED->fSerial = serial;
	fEvents.Append(pED);
}

void plNotifyMsg::AddSpawnedEvent (const plKey &spawner, const plKey &spawnee)
{
	proSpawnedEventData* pED = TRACKED_NEW proSpawnedEventData();
	pED->fSpawner = spawner;
	pED->fSpawnee = spawnee;
	fEvents.Append(pED);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddActivateEvent
//  PARAMETERS : activate  - true or false
//
//  PURPOSE    : Sometimes you just want a yes or no
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddActivateEvent( hsBool activate )
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kActivate )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the collision event record
	proActivateEventData* pED = TRACKED_NEW proActivateEventData;
	pED->fActive = true;
	pED->fActivate = activate;
	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddPickEvent
//  PARAMETERS : other  - the plKey of the other object involved (Picker)
//             : self   - the plKey to probably us (Picked)
//
//  PURPOSE    : Add a pick event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddPickEvent( const plKey &other, const plKey& self, hsBool enabled, hsPoint3 hitPoint )
{

	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kPicked )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the pick event record
	proPickedEventData* pED = TRACKED_NEW proPickedEventData;
	pED->fPicker = other;
	pED->fPicked = self;
	pED->fEnabled = enabled;
	pED->fHitPoint = hitPoint;

	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddContainerEvent
//  PARAMETERS : container  - the plKey of the object contained 
//             : contained   - the plKey of the containing volume
//
//  PURPOSE    : Add a container event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddContainerEvent( const plKey &container, const plKey &contained, hsBool entering )
{

	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kContained )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the pick event record
	proContainedEventData* pED = TRACKED_NEW proContainedEventData;
	pED->fContained = contained;
	pED->fContainer = container;
	pED->fEntering = entering;
	fEvents.Append(pED);	// then add it to the list of event records
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddFacingEvent
//  PARAMETERS : other  - the plKey of the other object involved (Facer)
//             : self   - the plKey to probably us (Facee)
//			   : dot    - the dot prod. of the facing angle
//
//  PURPOSE    : Add a facing event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddFacingEvent( const plKey &other, const plKey &self, hsScalar dot, hsBool enabled )
{

	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kFacing )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the pick event record
	proFacingEventData* pED = TRACKED_NEW proFacingEventData;
	pED->fFacer = other;
	pED->fFacee = self;
	pED->dot = dot;
	pED->enabled = enabled;
	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddControlKeyEvent
//  PARAMETERS : id     - identification, could be the controlkey that was hit
//             : down   - whether the control is going down or up in action
//
//  PURPOSE    : Add a control key event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddControlKeyEvent( Int32 key, hsBool down )
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kControlKey )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the control key event record
	proControlKeyEventData* pED = TRACKED_NEW proControlKeyEventData;
	pED->fControlKey = key;
	pED->fDown = down;
	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddVariableEvent
//  PARAMETERS : name    - name of the variable
//             : number  - the value of the variable as a number
//
//  PURPOSE    : Add a variable event record to this notify message
//
void plNotifyMsg::AddVariableEvent( const char* name, hsScalar number )
{
	// create the control key event record
	proVariableEventData* pED = TRACKED_NEW proVariableEventData;
	pED->fName = hsStrcpy(nil,name);
//	pED->fName = (char*)name;
	pED->fDataType = proEventData::kNumber;
	pED->fNumber = number;
	fEvents.Append(pED);	// then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddVariableEvent
//  PARAMETERS : name    - name of the variable
//             : number  - the value of the variable as a number
//
//  PURPOSE    : Add a variable event record to this notify message
//
void plNotifyMsg::AddVariableEvent( const char* name, const plKey &key )
{
	// create the control key event record
	proVariableEventData* pED = TRACKED_NEW proVariableEventData;
	pED->fName = hsStrcpy(nil,name);
//	pED->fName = (char*)name;
	pED->fDataType = proEventData::kKey;
	pED->fKey = key;
	fEvents.Append(pED);	// then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddClickDragEvent
//  PARAMETERS : picker     - always the local player
//             : pickee    - the click/draggable
//			   : animPos - 0.0 to 1.0 % of the way we are through the animation
//
//  PURPOSE    : Add a click/drag event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddClickDragEvent( const plKey& dragger, const plKey& dragee, hsScalar animPos )
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kClickDrag )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the control key event record
	proClickDragEventData* pED = TRACKED_NEW proClickDragEventData;
	pED->picked = dragee;
	pED->picker = dragger;
	pED->animPos = animPos;
	fEvents.Append(pED);	// then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddOfferBookEvent
//  PARAMETERS : offerer   - the book offerer - the local player on the sender's machine, a remote avatar at the receiver's end
//             : targetAge - the age we are offering.  this # is taken from the konstant list of age link panels in xLinkingBookPopupGUI.py
//
//  PURPOSE    : Add an OfferBookEvent - note this message should NOT EVER be locally delivered - Networked ONLY and only to a specific net transport member.
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddOfferBookEvent(const plKey& offerer, int targetAge, int offeree)
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kOfferLinkingBook )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the control key event record
	proOfferLinkingBookEventData* pED = TRACKED_NEW proOfferLinkingBookEventData;
	pED->offerer = offerer;
	pED->targetAge = targetAge;
	pED->offeree = offeree;
	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddBookEvent
//  PARAMETERS : event - The type of event we are, as defined by the enum in pfJournalBook.h
//             : linkID - For image link event types, the link ID as defined in the esHTML source. Otherwise, unused.
//
//  PURPOSE    : Add an OfferBookEvent - note this message should NOT EVER be locally delivered - Networked ONLY and only to a specific net transport member.
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddBookEvent( UInt32 event, UInt32 linkID /*=0*/)
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kBook )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the control key event record
	proBookEventData* pED = TRACKED_NEW proBookEventData;
	pED->fEvent = event;
	pED->fLinkID = linkID;
	fEvents.Append(pED);	// then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddClimbingBlockerHit
//  PARAMETERS : fBlockerKey - the key of the blocker we hit
//
//  PURPOSE    : this is to notify python we hit a specific climbing blocker 
//             : 
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddHitClimbingBlockerEvent(const plKey &blocker)
{
	// remove records that are like the one being added
	int num_recs = fEvents.GetCount();
	if ( num_recs > 0 )
	{
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == proEventData::kClimbingBlockerHit )
			{
				// remove it
				delete fEvents[i];
				fEvents.Remove(i);
				// then jump out.. the count is no longer good and anyway there should only be one of the same type
				break;
			}
		}
	}

	// create the control key event record
	proClimbingBlockerHitEventData* pED = TRACKED_NEW proClimbingBlockerHitEventData;
	pED->fBlockerKey = blocker;
	fEvents.Append(pED);	// then add it to the list of event records
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindEventRecord
//  PARAMETERS : eventtype  -  the event type record that we are looking for
//
//  PURPOSE    : Find the first record in the event records that is of type eventtype
//
proEventData* plNotifyMsg::FindEventRecord( Int32 eventtype )
{
	// make sure that its a legal event type
	if ( eventtype >= 0 && eventtype < proEventData::kNone )
	{
		// loop thru the event records looking for what they want
		int num_recs = fEvents.GetCount();
		int i;
		for ( i=0; i<num_recs; i++ )
		{
			// see if its the same type 
			proEventData* pEDTest = fEvents.Get(i);
			if ( pEDTest->fEventType == eventtype )
				return pEDTest;
		}
	}
	return nil;
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ClearEvents
//  PARAMETERS : none
//
//  PURPOSE    : clear all the event records
//
void plNotifyMsg::ClearEvents()
{
	// clean up fEvent records
	int i;
	for( i = 0; i < fEvents.GetCount(); i++ )
		delete fEvents[i];
	fEvents.Reset();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Read
//  PARAMETERS : stream  - where to read the data from
//             : mgr     - resource manager (for special help)
//
//  PURPOSE    : Read object from stream
//
void plNotifyMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	// read in the static data
	fType = stream->ReadSwap32();
	stream->ReadSwap(&fState);
	fID = stream->ReadSwap32();
	// read in the variable part of the message
	Int32 numberEDs = stream->ReadSwap32();
	fEvents.SetCountAndZero(numberEDs);
	if ( numberEDs > 0 )
	{
		int i;
		for ( i=0 ; i < numberEDs ; i++ )
		{
			proEventData* pED = proEventData::Read( stream, mgr );
			fEvents[i] = pED;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Write
//  PARAMETERS : stream  - where to write the data to
//             : mgr     - resource manager (for special help)
//
//  PURPOSE    : Write object from stream
//
void plNotifyMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	// write static data
	stream->WriteSwap32(fType);
	stream->WriteSwap(fState);
	stream->WriteSwap32(fID);
	// then write the variable data
	Int32 numberEDs = fEvents.Count();
	stream->WriteSwap32(numberEDs);
	if ( numberEDs > 0 )
	{
		// write out each record
		int i;
		for ( i=0 ; i < numberEDs; i++ )
		{
			fEvents[i]->Write(stream,mgr);
		}
	}

}

enum NotifyMsgFlags
{
	kNotifyMsgType,
	kNotifyMsgState,
	kNotifyMsgID,
	kNotifyMsgEDs,
};

#include "../pnNetCommon/plNetApp.h"

void plNotifyMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kNotifyMsgType))
		fType = s->ReadSwap32();

	if (contentFlags.IsBitSet(kNotifyMsgState))
		s->ReadSwap(&fState);

	if (contentFlags.IsBitSet(kNotifyMsgID))
		fID = s->ReadSwap32();

	if (contentFlags.IsBitSet(kNotifyMsgEDs))
	{
		// read in the variable part of the message
		Int32 numberEDs = s->ReadSwap32();
		fEvents.SetCountAndZero(numberEDs);
		if (numberEDs > 0)
		{
			for (int i = 0; i < numberEDs ; i++)
			{
				proEventData* pED = proEventData::ReadVersion(s, mgr);
				fEvents[i] = pED;
			}
		}
	}

	plKey avKey = GetAvatarKey();
	if (plNetClientApp::GetInstance() && avKey == plNetClientApp::GetInstance()->GetLocalPlayerKey())
	{
		SetBCastFlag(plMessage::kNetStartCascade, true);
		SetBCastFlag(plMessage::kNetNonLocal | plMessage::kNetPropagate, false);
	}
}

void plNotifyMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kNotifyMsgType);
	contentFlags.SetBit(kNotifyMsgState);
	contentFlags.SetBit(kNotifyMsgID);
	contentFlags.SetBit(kNotifyMsgEDs);
	contentFlags.Write(s);

	// kNotifyMsgType
	s->WriteSwap32(fType);

	// kNotifyMsgState
	s->WriteSwap(fState);

	// kNotifyMsgID
	s->WriteSwap32(fID);

	// kNotifyMsgEDs
	Int32 numberEDs = fEvents.Count();
	s->WriteSwap32(numberEDs);
	if (numberEDs > 0)
	{
		// write out each record
		for (int i = 0; i < numberEDs; i++)
		{
			fEvents[i]->WriteVersion(s, mgr);
		}
	}
}


plKey plNotifyMsg::GetAvatarKey()
{
	for (int i = 0; i < fEvents.GetCount(); i++)
	{
		proEventData *event = fEvents[i];

		switch (event->fEventType)
		{
		case proEventData::kCollision:
			return ( (proCollisionEventData *)event )->fHitter;

		case proEventData::kPicked:
			return ( (proPickedEventData *)event )->fPicker;
		
		case proEventData::kSpawned:
			return ( (proSpawnedEventData *)event )->fSpawnee;

		case proEventData::kMultiStage:
			return ( (proMultiStageEventData *)event )->fAvatar;
		}
	}

	return nil;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


proEventData* proEventData::ICreateEventDataType(Int32 type)
{
	switch (type)
	{
	case kCollision:		return TRACKED_NEW proCollisionEventData;
	case kPicked:			return TRACKED_NEW proPickedEventData;
	case kControlKey:		return TRACKED_NEW proControlKeyEventData;
	case kVariable:			return TRACKED_NEW proVariableEventData;
	case kFacing:			return TRACKED_NEW proFacingEventData;
	case kContained:		return TRACKED_NEW proContainedEventData;
	case kActivate:			return TRACKED_NEW proActivateEventData;
	case kCallback:			return TRACKED_NEW proCallbackEventData;
	case kResponderState:	return TRACKED_NEW proResponderStateEventData;
	case kMultiStage:		return TRACKED_NEW proMultiStageEventData;
	case kCoop:				return TRACKED_NEW proCoopEventData;
	case kSpawned:			return TRACKED_NEW proSpawnedEventData;
	case kOfferLinkingBook: return TRACKED_NEW proOfferLinkingBookEventData;
	case kBook:				return TRACKED_NEW proBookEventData;
	case kClimbingBlockerHit: return TRACKED_NEW proClimbingBlockerHitEventData;
	}

	return nil;
}

//// proEventData::Read /////////////////////////////////////////////////////
//	Static function on proEventData that reads in a derived event data type
//	from the given stream and returns it.

proEventData* proEventData::Read( hsStream *stream, hsResMgr *mgr )
{
	Int32 evtType = stream->ReadSwap32();

	proEventData* data = ICreateEventDataType(evtType);

	if (data != nil)
		data->IRead(stream, mgr);

	return data;
}

void proEventData::Write(hsStream *stream, hsResMgr *mgr)
{
	stream->WriteSwap32(fEventType);
	IWrite(stream, mgr);
}

enum proEventDataFlags
{
	kProEventDataType,
};

proEventData* proEventData::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProEventDataType))
	{
		Int32 evtType = s->ReadSwap32();

		proEventData* data = ICreateEventDataType(evtType);

		if (data != nil)
			data->IReadVersion(s, mgr);

		return data;
	}

	return nil;
}

void proEventData::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProEventDataType);
	contentFlags.Write(s);

	// kProEventDataType
	s->WriteSwap32(fEventType);

	IWriteVersion(s, mgr);
}

void proCollisionEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fEnter = stream->ReadBool();
	fHitter = mgr->ReadKey(stream);
	fHittee = mgr->ReadKey(stream);
}

void proCollisionEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteBool(fEnter);
	mgr->WriteKey(stream, fHitter);
	mgr->WriteKey(stream, fHittee);
}

enum proCollisionFlags
{
	kProCollisionEnter,
	kProCollisionHitter,
	kProCollisionHittee,
};

void proCollisionEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProCollisionEnter))
		fEnter = s->ReadBool();
	if (contentFlags.IsBitSet(kProCollisionHitter))
		fHitter = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProCollisionHittee))
		fHittee = mgr->ReadKey(s);
}

void proCollisionEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProCollisionEnter);
	contentFlags.SetBit(kProCollisionHitter);
	contentFlags.SetBit(kProCollisionHittee);
	contentFlags.Write(s);

	// kProCollisionEnter
	s->WriteBool(fEnter);
	// kProCollisionHitter
	mgr->WriteKey(s, fHitter);
	// kProCollisionHittee
	mgr->WriteKey(s, fHittee);
}

void proPickedEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fPicker = mgr->ReadKey(stream);
	fPicked = mgr->ReadKey(stream);
	fEnabled = stream->ReadBool();
	fHitPoint.Read(stream);
}

void proPickedEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	mgr->WriteKey(stream, fPicker);
	mgr->WriteKey(stream, fPicked);
	stream->WriteBool(fEnabled);
	fHitPoint.Write(stream);
}

enum ProPickedFlags
{
	kProPickedPicker,
	kProPickedPicked,
	kProPickedEnabled,
	kProPickedHitPoint,
};

void proPickedEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProPickedPicker))
		fPicker = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProPickedPicked))
		fPicked = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProPickedEnabled))
		fEnabled = s->ReadBool();
	if (contentFlags.IsBitSet(kProPickedHitPoint))
		fHitPoint.Read(s);
}

void proPickedEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProPickedPicker);
	contentFlags.SetBit(kProPickedPicked);
	contentFlags.SetBit(kProPickedEnabled);
	contentFlags.SetBit(kProPickedHitPoint);
	contentFlags.Write(s);

	// kProPickedPicker
	mgr->WriteKey(s, fPicker);
	// kProPickedPicked
	mgr->WriteKey(s, fPicked);
	// kProPickedEnabled
	s->WriteBool(fEnabled);
	// kProPickedHitPoint
	fHitPoint.Write(s);
}

void proSpawnedEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fSpawner = mgr->ReadKey(stream);
	fSpawnee = mgr->ReadKey(stream);
}

void proSpawnedEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	mgr->WriteKey(stream, fSpawner);
	mgr->WriteKey(stream, fSpawnee);
}

enum ProSpawnedFlags
{
	kProSpawnedSpawner,
	kProSpawnedSpawnee,
};

void proSpawnedEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProSpawnedSpawner))
		fSpawner = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProSpawnedSpawnee))
		fSpawnee = mgr->ReadKey(s);
}

void proSpawnedEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProSpawnedSpawner);
	contentFlags.SetBit(kProSpawnedSpawnee);
	contentFlags.Write(s);

	// kProSpawnedSpawner
	mgr->WriteKey(s, fSpawner);
	// kProSpawnedSpawnee
	mgr->WriteKey(s, fSpawnee);
}

void proControlKeyEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fControlKey = stream->ReadSwap32();
	fDown = stream->ReadBool();
}
void proControlKeyEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSwap32(fControlKey);
	stream->WriteBool(fDown);
}

enum ProControlFlags
{
	kProControlKey,
	kProControlDown,
};

void proControlKeyEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProControlKey))
		fControlKey = s->ReadSwap32();
	if (contentFlags.IsBitSet(kProControlDown))
		fDown = s->ReadBool();
}
void proControlKeyEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProControlKey);
	contentFlags.SetBit(kProControlDown);
	contentFlags.Write(s);

	// kProControlKey
	s->WriteSwap32(fControlKey);
	// kProControlDown
	s->WriteBool(fDown);
}


void proVariableEventData::IInit()
{
	fName = nil;
}
void proVariableEventData::IDestruct()
{
	if ( fName != nil )
		delete [] fName;
	fName = nil;
}

void proVariableEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fName = stream->ReadSafeString();
	fDataType = stream->ReadSwap32();
	fNumber = stream->ReadSwapScalar();
	fKey = mgr->ReadKey(stream);
}

void proVariableEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSafeString(fName);
	stream->WriteSwap32(fDataType);
	stream->WriteSwapScalar(fNumber);
	mgr->WriteKey(stream, fKey);
}

enum ProVariableFlags
{
	kProVariableName,
	kProVariableDataType,
	kProVariableNumber,
	kProVariableKey,
};

void proVariableEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProVariableName))
		fName = s->ReadSafeString();
	if (contentFlags.IsBitSet(kProVariableDataType))
		fDataType = s->ReadSwap32();
	if (contentFlags.IsBitSet(kProVariableNumber))
		fNumber = s->ReadSwapScalar();
	if (contentFlags.IsBitSet(kProVariableKey))
		fKey = mgr->ReadKey(s);
}

void proVariableEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProVariableName);
	contentFlags.SetBit(kProVariableDataType);
	contentFlags.SetBit(kProVariableNumber);
	contentFlags.SetBit(kProVariableKey);
	contentFlags.Write(s);

	// kProVariableName
	s->WriteSafeString(fName);
	// kProVariableDataType
	s->WriteSwap32(fDataType);
	// kProVariableNumber
	s->WriteSwapScalar(fNumber);
	// kProVariableKey
	mgr->WriteKey(s, fKey);
}

void proFacingEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fFacer = mgr->ReadKey(stream);
	fFacee = mgr->ReadKey(stream);
	dot = stream->ReadSwapScalar();
	enabled = stream->ReadBool();
}

void proFacingEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	mgr->WriteKey(stream, fFacer);
	mgr->WriteKey(stream, fFacee);
	stream->WriteSwapScalar(dot);
	stream->WriteBool(enabled);
}

enum ProFacingFlags
{
	kProFacingFacer,
	kProFacingFacee,
	kProFacingDot,
	kProFacingEnabled,
};

void proFacingEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProFacingFacer))
		fFacer = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProFacingFacee))
		fFacee = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProFacingDot))
		dot = s->ReadSwapScalar();
	if (contentFlags.IsBitSet(kProFacingEnabled))
		enabled = s->ReadBool();
}

void proFacingEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProFacingFacer);
	contentFlags.SetBit(kProFacingFacee);
	contentFlags.SetBit(kProFacingDot);
	contentFlags.SetBit(kProFacingEnabled);
	contentFlags.Write(s);

	// kProFacingFacer
	mgr->WriteKey(s, fFacer);
	// kProFacingFacee	
	mgr->WriteKey(s, fFacee);
	// kProFacingDot	
	s->WriteSwapScalar(dot);
	// kProFacingEnabled
	s->WriteBool(enabled);
}

void proContainedEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fContained = mgr->ReadKey(stream);
	fContainer = mgr->ReadKey(stream);
	fEntering = stream->ReadBool();
}

void proContainedEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	mgr->WriteKey(stream, fContained);
	mgr->WriteKey(stream, fContainer);
	stream->WriteBool(fEntering);
}

enum ProContainedFlags
{
	kProContainedContained,
	kProContainedContainer,
	kProContainedEntering,
};

void proContainedEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProContainedContained))
		fContained = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProContainedContainer))
		fContainer = mgr->ReadKey(s);
	if (contentFlags.IsBitSet(kProContainedEntering))
		fEntering = s->ReadBool();
}

void proContainedEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProContainedContained);
	contentFlags.SetBit(kProContainedContainer);
	contentFlags.SetBit(kProContainedEntering);
	contentFlags.Write(s);

	// kProContainedContained
	mgr->WriteKey(s, fContained);
	// kProContainedContainer
	mgr->WriteKey(s, fContainer);
	// kProContainedEntering
	s->WriteBool(fEntering);
}

void proActivateEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fActive = stream->ReadBool();
	fActivate = stream->ReadBool();
}

void proActivateEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteBool(fActive);
	stream->WriteBool(fActivate);
}

enum ProActivateFlags
{
	kProActivateActive,
	kProActivateActivate,
};

void proActivateEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProActivateActive))
		fActive = s->ReadBool();
	if (contentFlags.IsBitSet(kProActivateActivate))
		fActivate = s->ReadBool();
}

void proActivateEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProActivateActive);
	contentFlags.SetBit(kProActivateActivate);
	contentFlags.Write(s);
	
	// kProActivateActive
	s->WriteBool(fActive);
	// kProActivateActivate
	s->WriteBool(fActivate);
}

void proCallbackEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fEventType = stream->ReadSwap32();
}

void proCallbackEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSwap32(fEventType);
}

enum ProCallbackFlags
{
	kProCallbackEventType,
};

void proCallbackEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProCallbackEventType))
		fEventType = s->ReadSwap32();
}

void proCallbackEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProCallbackEventType);
	contentFlags.Write(s);

	// kProCallbackEventType
	s->WriteSwap32(fEventType);
}

void proResponderStateEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fState = stream->ReadSwap32();
}

void proResponderStateEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSwap32(fState);
}

enum ProResponderFlags
{
	kProResponderState,
};

void proResponderStateEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProResponderState))
		fState = s->ReadSwap32();
}

void proResponderStateEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProResponderState);
	contentFlags.Write(s);

	// kProResponderState
	s->WriteSwap32(fState);
}

void proMultiStageEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fStage = stream->ReadSwap32();
	fEvent = stream->ReadSwap32();
	fAvatar = mgr->ReadKey(stream);
}

void proMultiStageEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSwap32(fStage);
	stream->WriteSwap32(fEvent);
	mgr->WriteKey(stream, fAvatar);
}

enum ProMultiStageFlags
{
	kProMultiStageStage,
	kProMultiStageEvent,
	kProMultiStageAvatar,
};

void proMultiStageEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProMultiStageStage))
		fStage = s->ReadSwap32();
	if (contentFlags.IsBitSet(kProMultiStageEvent))
		fEvent = s->ReadSwap32();
	if (contentFlags.IsBitSet(kProMultiStageAvatar))
		fAvatar = mgr->ReadKey(s);
}

void proMultiStageEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProMultiStageStage);
	contentFlags.SetBit(kProMultiStageEvent);
	contentFlags.SetBit(kProMultiStageAvatar);
	contentFlags.Write(s);

	// kProMultiStageStage
	s->WriteSwap32(fStage);
	// kProMultiStageEvent
	s->WriteSwap32(fEvent);
	// kProMultiStageAvatar
	mgr->WriteKey(s, fAvatar);
}

void proCoopEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fID = stream->ReadSwap32();
	fSerial = stream->ReadSwap16();
}

void proCoopEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSwap32(fID);
	stream->WriteSwap16(fSerial);
}

enum ProCoopFlags
{
	kProCoopID,
	kProCoopSerial
};

void proCoopEventData::IReadVersion(hsStream* stream, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(stream);

	if(contentFlags.IsBitSet(kProCoopID))
		fID = stream->ReadSwap32();
	if(contentFlags.IsBitSet(kProCoopSerial))
		fSerial = stream->ReadSwap16();
}

void proCoopEventData::IWriteVersion(hsStream* stream, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProCoopID);
	contentFlags.SetBit(kProCoopSerial);
	contentFlags.Write(stream);

	stream->WriteSwap32(fID);
	stream->WriteSwap16(fSerial);
	
}

void proOfferLinkingBookEventData::IWrite(hsStream* stream,	hsResMgr* mgr)
{
	mgr->WriteKey(stream, offerer);
	stream->WriteSwap32(targetAge);
	stream->WriteSwap32(offeree);
}

void proOfferLinkingBookEventData::IRead(hsStream* stream,	hsResMgr* mgr)
{
	offerer = mgr->ReadKey(stream);
	targetAge = stream->ReadSwap32();
	offeree = stream->ReadSwap32();
}

enum ProOfferFlags
{
	kProOfferOfferer,
	kProOfferTargetAge,
	kProOfferOfferee,
};

void proOfferLinkingBookEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProOfferOfferer);
	contentFlags.SetBit(kProOfferTargetAge);
	contentFlags.SetBit(kProOfferOfferee);
	contentFlags.Write(s);

	mgr->WriteKey(s, offerer);
	s->WriteSwap32(targetAge);
	s->WriteSwap32(offeree);
}

void proOfferLinkingBookEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if(contentFlags.IsBitSet(kProOfferOfferer))
		offerer = mgr->ReadKey(s);
	if(contentFlags.IsBitSet(kProOfferTargetAge))
		targetAge = s->ReadSwap32();
	if(contentFlags.IsBitSet(kProOfferOfferee))
		offeree = s->ReadSwap32();
}

void proBookEventData::IWrite(hsStream* stream,	hsResMgr* mgr)
{
	stream->WriteSwap32(fEvent);
	stream->WriteSwap32(fLinkID);
}

void proBookEventData::IRead(hsStream* stream,	hsResMgr* mgr)
{
	fEvent = stream->ReadSwap32();
	fLinkID = stream->ReadSwap32();
}

enum ProBookFlags
{
	kProBookEvent,
	kProBookLinkID,
};

void proBookEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProBookEvent);
	contentFlags.SetBit(kProBookLinkID);
	contentFlags.Write(s);

	s->WriteSwap32( fEvent );
	s->WriteSwap32( fLinkID );
}

void proBookEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if(contentFlags.IsBitSet(kProBookEvent))
		fEvent = s->ReadSwap32();
	if(contentFlags.IsBitSet(kProBookLinkID))
		fLinkID = s->ReadSwap32();
}


void proClimbingBlockerHitEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
	fBlockerKey = mgr->ReadKey(stream);
}

void proClimbingBlockerHitEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
	mgr->WriteKey(stream, fBlockerKey);
}

enum proClimbingBlockerHitFlags
{
	
	kProClimbingBlockerKey,
};

void proClimbingBlockerHitEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kProClimbingBlockerKey))
		fBlockerKey = mgr->ReadKey(s);;
}

void proClimbingBlockerHitEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
	hsBitVector contentFlags;
	contentFlags.SetBit(kProClimbingBlockerKey);
	contentFlags.Write(s);

	// kProClimbingBlockerKey
	mgr->WriteKey(s, fBlockerKey);
}
