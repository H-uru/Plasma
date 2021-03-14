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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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

#include "hsResMgr.h"
#include "hsStream.h"

#include "pnNetCommon/plNetApp.h"

plNotifyMsg::plNotifyMsg(const plKey &s, const plKey &r)
{
    SetSender(s);
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
    fState = 0.0f;          // start state at (completely) false
    fID = 0;
}

plNotifyMsg::~plNotifyMsg()
{
    ClearEvents();
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddEvent
//  PARAMETERS : ed     - pointer to event that needs to be added (be recreating)
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
            break;

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
                    case proEventData::kFloat:
                        AddVariableEvent(evt->fName, evt->fNumber.f);
                        break;
                    case proEventData::kInt:
                        AddVariableEvent(evt->fName, evt->fNumber.i);
                        break;
                    case proEventData::kNull:
                        AddVariableEvent(evt->fName);
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
            break;
        case proEventData::kBook:
            {
                proBookEventData* evt = (proBookEventData*)ed;
                AddBookEvent( evt->fEvent, evt->fLinkID );
            }
            break;
        case proEventData::kClimbingBlockerHit:
            {
                proClimbingBlockerHitEventData* evt = (proClimbingBlockerHitEventData*)ed;
                AddHitClimbingBlockerEvent(evt->fBlockerKey);
            }
            break;
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
void plNotifyMsg::AddCollisionEvent( bool enter, const plKey &other, const plKey &self, bool onlyOneCollision )
{
    // if this is the normal case of there can only be one collision, then get rid of any others
    if ( onlyOneCollision )
    {
        // remove records that are like the one being added
        for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
        {
            // see if its the same type
            proEventData* pEDTest = *iter;
            if ( pEDTest->fEventType == proEventData::kCollision )
            {
                // remove it
                delete pEDTest;
                fEvents.erase(iter);
                // then jump out.. the count is no longer good and anyway there should only be one of the same type
                break;
            }
        }
    }

    // create the collision event record
    proCollisionEventData* pED = new proCollisionEventData;
    pED->fEnter = enter;
    pED->fHitter = other;
    pED->fHittee = self;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddCallbackEvent
//  PARAMETERS : event  - the event type, as enumerated in plEventCallbackMsg.h
//
//
void plNotifyMsg::AddCallbackEvent( int32_t event )
{
    // remove records that are like the one being added
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kCallback )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the collision event record
    proCallbackEventData* pED = new proCallbackEventData;
    pED->fEventType = event;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddResponderStateEvent
//  PARAMETERS : state  - the state for the responder to switch to before triggering
//
//
void plNotifyMsg::AddResponderStateEvent( int32_t state )
{
    // remove records that are like the one being added
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kResponderState )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the collision event record
    proResponderStateEventData* pED = new proResponderStateEventData;
    pED->fState = state;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddMultiStageEvent
//  PARAMETERS : stage   - the stage the multistage behavior is on
//             : event   - what was the event that happened
//
//
void plNotifyMsg::AddMultiStageEvent( int32_t stage, int32_t event, const plKey& avatar )
{
    // we can have multi events of this type
    // create the mutlistage event record
    proMultiStageEventData* pED = new proMultiStageEventData;
    pED->fStage = stage;
    pED->fEvent = event;
    pED->fAvatar = avatar;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}

void plNotifyMsg::AddCoopEvent(uint32_t id, uint16_t serial)
{
    proCoopEventData *pED = new proCoopEventData;
    pED->fID = id;
    pED->fSerial = serial;
    fEvents.emplace_back(pED);
}

void plNotifyMsg::AddSpawnedEvent (const plKey &spawner, const plKey &spawnee)
{
    proSpawnedEventData* pED = new proSpawnedEventData();
    pED->fSpawner = spawner;
    pED->fSpawnee = spawnee;
    fEvents.emplace_back(pED);
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
void plNotifyMsg::AddActivateEvent( bool activate )
{
    // remove records that are like the one being added
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kActivate )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the collision event record
    proActivateEventData* pED = new proActivateEventData;
    pED->fActive = true;
    pED->fActivate = activate;
    fEvents.emplace_back(pED);    // then add it to the list of event records
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
void plNotifyMsg::AddPickEvent( const plKey &other, const plKey& self, bool enabled, hsPoint3 hitPoint )
{
    // remove records that are like the one being added
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kPicked )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the pick event record
    proPickedEventData* pED = new proPickedEventData;
    pED->fPicker = other;
    pED->fPicked = self;
    pED->fEnabled = enabled;
    pED->fHitPoint = hitPoint;

    fEvents.emplace_back(pED);    // then add it to the list of event records
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
void plNotifyMsg::AddContainerEvent( const plKey &container, const plKey &contained, bool entering )
{
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kContained )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the pick event record
    proContainedEventData* pED = new proContainedEventData;
    pED->fContained = contained;
    pED->fContainer = container;
    pED->fEntering = entering;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddFacingEvent
//  PARAMETERS : other  - the plKey of the other object involved (Facer)
//             : self   - the plKey to probably us (Facee)
//             : dot    - the dot prod. of the facing angle
//
//  PURPOSE    : Add a facing event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddFacingEvent( const plKey &other, const plKey &self, float dot, bool enabled )
{
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kFacing )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the pick event record
    proFacingEventData* pED = new proFacingEventData;
    pED->fFacer = other;
    pED->fFacee = self;
    pED->dot = dot;
    pED->enabled = enabled;
    fEvents.emplace_back(pED);    // then add it to the list of event records
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
void plNotifyMsg::AddControlKeyEvent( int32_t key, bool down )
{
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kControlKey )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the control key event record
    proControlKeyEventData* pED = new proControlKeyEventData;
    pED->fControlKey = key;
    pED->fDown = down;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddVariableEvent
//  PARAMETERS : name    - name of the variable
//             : number  - the value of the variable as a number
//
//  PURPOSE    : Add a variable event record to this notify message
//
void plNotifyMsg::AddVariableEvent(const ST::string& name, float number)
{
    // create the control key event record
    proVariableEventData* pED = new proVariableEventData;
    pED->fName = name;
    pED->fDataType = proEventData::kFloat;
    pED->fNumber.f = number;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddVariableEvent
//  PARAMETERS : name    - name of the variable
//             : number  - the value of the variable as a number
//
//  PURPOSE    : Add a variable event record to this notify message
//
void plNotifyMsg::AddVariableEvent(const ST::string& name, int32_t number)
{
    // create the control key event record
    proVariableEventData* pED = new proVariableEventData;
    pED->fName = name;
    pED->fDataType = proEventData::kInt;
    pED->fNumber.i = number;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddVariableEvent
//  PARAMETERS : name    - name of the variable
//
//  PURPOSE    : Add a variable event record to this notify message
//
void plNotifyMsg::AddVariableEvent(const ST::string& name)
{
    // create the control key event record
    proVariableEventData* pED = new proVariableEventData;
    pED->fName = name;
    pED->fDataType = proEventData::kNull;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddVariableEvent
//  PARAMETERS : name    - name of the variable
//             : number  - the value of the variable as a number
//
//  PURPOSE    : Add a variable event record to this notify message
//
void plNotifyMsg::AddVariableEvent(const ST::string& name, const plKey &key)
{
    // create the control key event record
    proVariableEventData* pED = new proVariableEventData;
    pED->fName = name;
    pED->fDataType = proEventData::kKey;
    pED->fKey = key;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddClickDragEvent
//  PARAMETERS : picker     - always the local player
//             : pickee    - the click/draggable
//             : animPos - 0.0 to 1.0 % of the way we are through the animation
//
//  PURPOSE    : Add a click/drag event record to this notify message
//             : Remove like event records, only the last one counts
//
// NOTE: To test for duplicate record, it only checks for records of the same type
//     : Eventually, it might be wise to check if the same 'self' key also?
//
void plNotifyMsg::AddClickDragEvent( const plKey& dragger, const plKey& dragee, float animPos )
{
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kClickDrag )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the control key event record
    proClickDragEventData* pED = new proClickDragEventData;
    pED->picked = dragee;
    pED->picker = dragger;
    pED->animPos = animPos;
    fEvents.emplace_back(pED);    // then add it to the list of event records
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
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kOfferLinkingBook )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the control key event record
    proOfferLinkingBookEventData* pED = new proOfferLinkingBookEventData;
    pED->offerer = offerer;
    pED->targetAge = targetAge;
    pED->offeree = offeree;
    fEvents.emplace_back(pED);    // then add it to the list of event records
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
void plNotifyMsg::AddBookEvent( uint32_t event, uint32_t linkID /*=0*/)
{
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kBook )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the control key event record
    proBookEventData* pED = new proBookEventData;
    pED->fEvent = event;
    pED->fLinkID = linkID;
    fEvents.emplace_back(pED);    // then add it to the list of event records
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
    for (auto iter = fEvents.begin(); iter != fEvents.end(); ++iter)
    {
        // see if its the same type
        proEventData* pEDTest = *iter;
        if ( pEDTest->fEventType == proEventData::kClimbingBlockerHit )
        {
            // remove it
            delete pEDTest;
            fEvents.erase(iter);
            // then jump out.. the count is no longer good and anyway there should only be one of the same type
            break;
        }
    }

    // create the control key event record
    proClimbingBlockerHitEventData* pED = new proClimbingBlockerHitEventData;
    pED->fBlockerKey = blocker;
    fEvents.emplace_back(pED);    // then add it to the list of event records
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindEventRecord
//  PARAMETERS : eventtype  -  the event type record that we are looking for
//
//  PURPOSE    : Find the first record in the event records that is of type eventtype
//
proEventData* plNotifyMsg::FindEventRecord( int32_t eventtype )
{
    // make sure that its a legal event type
    if ( eventtype >= 0 && eventtype < proEventData::kNone )
    {
        // loop thru the event records looking for what they want
        for (proEventData* pEDTest : fEvents)
        {
            // see if its the same type 
            if ( pEDTest->fEventType == eventtype )
                return pEDTest;
        }
    }
    return nullptr;
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
    for (proEventData* pED : fEvents)
        delete pED;
    fEvents.clear();
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
    fType = stream->ReadLE32();
    stream->ReadLEFloat(&fState);
    fID = stream->ReadLE32();
    // read in the variable part of the message
    uint32_t numberEDs = stream->ReadLE32();
    fEvents.resize(numberEDs);
    for (uint32_t i = 0; i < numberEDs; i++)
        fEvents[i] = proEventData::Read(stream, mgr);
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
    stream->WriteLE32(fType);
    stream->WriteLEFloat(fState);
    stream->WriteLE32(fID);
    // then write the variable data
    stream->WriteLE32((uint32_t)fEvents.size());
    // write out each record
    for (proEventData* pED : fEvents)
        pED->Write(stream,mgr);
}

enum NotifyMsgFlags
{
    kNotifyMsgType,
    kNotifyMsgState,
    kNotifyMsgID,
    kNotifyMsgEDs,
};

void plNotifyMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
    plMessage::IMsgReadVersion(s, mgr);

    hsBitVector contentFlags;
    contentFlags.Read(s);

    if (contentFlags.IsBitSet(kNotifyMsgType))
        fType = s->ReadLE32();

    if (contentFlags.IsBitSet(kNotifyMsgState))
        s->ReadLEFloat(&fState);

    if (contentFlags.IsBitSet(kNotifyMsgID))
        fID = s->ReadLE32();

    if (contentFlags.IsBitSet(kNotifyMsgEDs))
    {
        // read in the variable part of the message
        uint32_t numberEDs = s->ReadLE32();
        fEvents.resize(numberEDs);
        for (uint32_t i = 0; i < numberEDs ; i++)
            fEvents[i] = proEventData::ReadVersion(s, mgr);
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
    s->WriteLE32(fType);

    // kNotifyMsgState
    s->WriteLEFloat(fState);

    // kNotifyMsgID
    s->WriteLE32(fID);

    // kNotifyMsgEDs
    s->WriteLE32((uint32_t)fEvents.size());
    // write out each record
    for (proEventData* pED : fEvents)
        pED->WriteVersion(s, mgr);
}


plKey plNotifyMsg::GetAvatarKey()
{
    for (proEventData *event : fEvents)
    {
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

    return nullptr;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


proEventData* proEventData::ICreateEventDataType(int32_t type)
{
    switch (type)
    {
    case kCollision:        return new proCollisionEventData;
    case kPicked:           return new proPickedEventData;
    case kControlKey:       return new proControlKeyEventData;
    case kVariable:         return new proVariableEventData;
    case kFacing:           return new proFacingEventData;
    case kContained:        return new proContainedEventData;
    case kActivate:         return new proActivateEventData;
    case kCallback:         return new proCallbackEventData;
    case kResponderState:   return new proResponderStateEventData;
    case kMultiStage:       return new proMultiStageEventData;
    case kCoop:             return new proCoopEventData;
    case kSpawned:          return new proSpawnedEventData;
    case kOfferLinkingBook: return new proOfferLinkingBookEventData;
    case kBook:             return new proBookEventData;
    case kClimbingBlockerHit: return new proClimbingBlockerHitEventData;
    }

    return nullptr;
}

//// proEventData::Read /////////////////////////////////////////////////////
//  Static function on proEventData that reads in a derived event data type
//  from the given stream and returns it.

proEventData* proEventData::Read( hsStream *stream, hsResMgr *mgr )
{
    int32_t evtType = stream->ReadLE32();

    proEventData* data = ICreateEventDataType(evtType);

    if (data != nullptr)
        data->IRead(stream, mgr);

    return data;
}

void proEventData::Write(hsStream *stream, hsResMgr *mgr)
{
    stream->WriteLE32(fEventType);
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
        int32_t evtType = s->ReadLE32();

        proEventData* data = ICreateEventDataType(evtType);

        if (data != nullptr)
            data->IReadVersion(s, mgr);

        return data;
    }

    return nullptr;
}

void proEventData::WriteVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kProEventDataType);
    contentFlags.Write(s);

    // kProEventDataType
    s->WriteLE32(fEventType);

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
    fControlKey = stream->ReadLE32();
    fDown = stream->ReadBool();
}
void proControlKeyEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32(fControlKey);
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
        fControlKey = s->ReadLE32();
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
    s->WriteLE32(fControlKey);
    // kProControlDown
    s->WriteBool(fDown);
}


void proVariableEventData::IInit()
{
    fName = ST::string();
}

void proVariableEventData::IDestruct()
{
    fName = ST::string();
}

void proVariableEventData::IReadNumber(hsStream * stream) {
    switch (fDataType)
    {
    case kFloat:
        fNumber.f = stream->ReadLEFloat();
        break;
    case kInt:
        fNumber.i = stream->ReadLE32();
        break;
    default: 
        (void)stream->ReadLE32(); //ignore
        break;
    }
}

void proVariableEventData::IWriteNumber(hsStream * stream) {
    switch (fDataType)
    {
    case kFloat:
        stream->WriteLEFloat(fNumber.f);
        break;
    case kInt:
        stream->WriteLE32(fNumber.i);
        break;
    default: 
        stream->WriteLE32(0);
        break;
    }
}

void proVariableEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
    fName = stream->ReadSafeString();
    fDataType = stream->ReadLE32();
    IReadNumber(stream);
    fKey = mgr->ReadKey(stream);
}

void proVariableEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteSafeString(fName);
    stream->WriteLE32(fDataType);
    IWriteNumber(stream);
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
        fDataType = s->ReadLE32();
    if (contentFlags.IsBitSet(kProVariableNumber))
        IReadNumber(s);
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
    s->WriteLE32(fDataType);
    // kProVariableNumber
    IWriteNumber(s);
    // kProVariableKey
    mgr->WriteKey(s, fKey);
}

void proFacingEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
    fFacer = mgr->ReadKey(stream);
    fFacee = mgr->ReadKey(stream);
    dot = stream->ReadLEFloat();
    enabled = stream->ReadBool();
}

void proFacingEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    mgr->WriteKey(stream, fFacer);
    mgr->WriteKey(stream, fFacee);
    stream->WriteLEFloat(dot);
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
        dot = s->ReadLEFloat();
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
    s->WriteLEFloat(dot);
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
    fEventType = stream->ReadLE32();
}

void proCallbackEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32(fEventType);
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
        fEventType = s->ReadLE32();
}

void proCallbackEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kProCallbackEventType);
    contentFlags.Write(s);

    // kProCallbackEventType
    s->WriteLE32(fEventType);
}

void proResponderStateEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
    fState = stream->ReadLE32();
}

void proResponderStateEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32(fState);
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
        fState = s->ReadLE32();
}

void proResponderStateEventData::IWriteVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kProResponderState);
    contentFlags.Write(s);

    // kProResponderState
    s->WriteLE32(fState);
}

void proMultiStageEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
    fStage = stream->ReadLE32();
    fEvent = stream->ReadLE32();
    fAvatar = mgr->ReadKey(stream);
}

void proMultiStageEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32(fStage);
    stream->WriteLE32(fEvent);
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
        fStage = s->ReadLE32();
    if (contentFlags.IsBitSet(kProMultiStageEvent))
        fEvent = s->ReadLE32();
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
    s->WriteLE32(fStage);
    // kProMultiStageEvent
    s->WriteLE32(fEvent);
    // kProMultiStageAvatar
    mgr->WriteKey(s, fAvatar);
}

void proCoopEventData::IRead(hsStream* stream, hsResMgr* mgr)
{
    fID = stream->ReadLE32();
    fSerial = stream->ReadLE16();
}

void proCoopEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32(fID);
    stream->WriteLE16(fSerial);
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
        fID = stream->ReadLE32();
    if(contentFlags.IsBitSet(kProCoopSerial))
        fSerial = stream->ReadLE16();
}

void proCoopEventData::IWriteVersion(hsStream* stream, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.SetBit(kProCoopID);
    contentFlags.SetBit(kProCoopSerial);
    contentFlags.Write(stream);

    stream->WriteLE32(fID);
    stream->WriteLE16(fSerial);
    
}

void proOfferLinkingBookEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    mgr->WriteKey(stream, offerer);
    stream->WriteLE32(targetAge);
    stream->WriteLE32(offeree);
}

void proOfferLinkingBookEventData::IRead(hsStream* stream,  hsResMgr* mgr)
{
    offerer = mgr->ReadKey(stream);
    targetAge = stream->ReadLE32();
    offeree = stream->ReadLE32();
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
    s->WriteLE32(targetAge);
    s->WriteLE32(offeree);
}

void proOfferLinkingBookEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.Read(s);

    if(contentFlags.IsBitSet(kProOfferOfferer))
        offerer = mgr->ReadKey(s);
    if(contentFlags.IsBitSet(kProOfferTargetAge))
        targetAge = s->ReadLE32();
    if(contentFlags.IsBitSet(kProOfferOfferee))
        offeree = s->ReadLE32();
}

void proBookEventData::IWrite(hsStream* stream, hsResMgr* mgr)
{
    stream->WriteLE32(fEvent);
    stream->WriteLE32(fLinkID);
}

void proBookEventData::IRead(hsStream* stream,  hsResMgr* mgr)
{
    fEvent = stream->ReadLE32();
    fLinkID = stream->ReadLE32();
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

    s->WriteLE32( fEvent );
    s->WriteLE32( fLinkID );
}

void proBookEventData::IReadVersion(hsStream* s, hsResMgr* mgr)
{
    hsBitVector contentFlags;
    contentFlags.Read(s);

    if(contentFlags.IsBitSet(kProBookEvent))
        fEvent = s->ReadLE32();
    if(contentFlags.IsBitSet(kProBookLinkID))
        fLinkID = s->ReadLE32();
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
