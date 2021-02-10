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
#ifndef _plNotifyMsg_h_
#define _plNotifyMsg_h_

#include "plMessage.h"

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

    proEventData( int32_t evtType = kNone ) : fEventType( evtType )
    {
    }
    virtual ~proEventData() {}

    enum dataType
    {
        kFloat=1,
        kKey,
        kInt,
        kNull,
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

    int32_t fEventType;       // what type of event (evenType enum)

    static proEventData* ICreateEventDataType(int32_t type);

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
#define proEventType(type)                                                  \
    class pro##type##EventData : public proEventData {                      \
    public:                                                                 \
        pro##type##EventData() : proEventData(k##type) {                    \
            IInit();                                                        \
        }                                                                   \
        virtual ~pro##type##EventData() {                                   \
            IDestruct();                                                    \
        }

proEventType(Collision)
    bool    fEnter;     // entering? (false is exit)
    plKey   fHitter;    // collision hitter (other probably player)
    plKey   fHittee;    // collision hittee (probably us)

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Picked)
    plKey   fPicker;    // who picked it (problably player)
    plKey   fPicked;    // object that was picked
    bool    fEnabled;   // pick (true) or un-pick (false)
    hsPoint3 fHitPoint; // where on the picked object that it was picked on

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Spawned)
    plKey   fSpawner;   // who spawned it (typicall plNPCSpawnMod)
    plKey   fSpawnee;   // what was spawned (typically a scene object with an armature mod)

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(ControlKey)
    int32_t   fControlKey;    // what control key was hit
    bool    fDown;          // was the key going down (false if going up)

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Variable)
    ST::string  fName;          // name of variable
    int32_t     fDataType;      // type of data

    // Can't be a union, sadly, but it isn't that much of a waste of space...
    plKey       fKey;       // if its a plKey (pointer to something)

    union {
        float f;
        int32_t i;
    } fNumber;

protected:
    void IInit() override;
    void IDestruct() override;
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;

    virtual void IReadNumber(hsStream * stream);
    virtual void IWriteNumber(hsStream * stream);
};

proEventType(Facing)
    plKey       fFacer; // what was facing
    plKey       fFacee; // what was being faced
    float    dot;     // the dot prod of their view vectors
    bool        enabled; // Now meets facing requirement (true) or no longer meets requirement (false)

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Contained)
    plKey   fContained; // who's inside of 
    plKey   fContainer; // this containing volume
    bool    fEntering;  // entering volume (true) or exiting (false)

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Activate)
    bool fActive;     // Whether or not to use the data in this field
    bool fActivate;   // the data

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Callback)
    int32_t   fEventType;  // enumerated in plEventCallbackMsg.h

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(ResponderState)
    int32_t   fState;     // what state the responder should be switched to before triggering

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(MultiStage)
    int32_t   fStage;
    int32_t   fEvent;
    plKey   fAvatar;    // who was running the stage

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Coop)
    uint32_t  fID;        // player ID of the initiator
    uint16_t  fSerial;    // serial number for the initiator
protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};


proEventType(ClickDrag)
    plKey picker; // always the local avatar in this case
    plKey picked;
    float animPos; // 0.0 to 1.0 animation percentage
};

proEventType(OfferLinkingBook)
    plKey offerer; // the avatar offering the linking book to you
    int targetAge; // the age the book is for - taken from konstant list of age buttons in xLinkingBookPopupGUI.py
    int offeree; // who we are offering the book to
protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(Book)
    uint32_t  fEvent;     // The type of event. See pfJournalBook.h for enumsu
    uint32_t  fLinkID;    // The link ID of the image clicked, if an image link event, otherwise unused
protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
};

proEventType(ClimbingBlockerHit)
    plKey   fBlockerKey;    // collision hittee (probably us)

protected:
    void IRead(hsStream* stream, hsResMgr* mgr) override;
    void IWrite(hsStream* stream, hsResMgr* mgr) override;

    void IReadVersion(hsStream* s, hsResMgr* mgr) override;
    void IWriteVersion(hsStream* s, hsResMgr* mgr) override;
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
    plNotifyMsg(const plKey& s, const plKey& r);
    ~plNotifyMsg();

    CLASSNAME_REGISTER(plNotifyMsg);
    GETINTERFACE_ANY(plNotifyMsg, plMessage);

    // data area
    enum notificationType
    {
        kActivator=0,
        kVarNotification,
        kNotifySelf,

        kResponderFF,           // Fast forward
        kResponderChangeState,  // Change state without triggering
    };
    int32_t       fType;              // what type of notification
    float    fState;             // state of the notifier 0.0=false, 1.0=true
    int32_t       fID;                // special ID mostly for responder State transitions
    std::vector<proEventData*> fEvents; // list of events with data

    void SetType(notificationType type) { fType = type; }
    void SetState(float state) { fState = state; }

    /**
     * Adds an arbitrary event to this notify message.
     * \note This copies \a ed.
     */
    void AddEvent( proEventData* ed);
    void AddCollisionEvent( bool enter, const plKey &other, const plKey &self, bool onlyOneCollision=true );
    void AddPickEvent( const plKey &other, const plKey& self, bool enabled, hsPoint3 hitPoint );
    void AddControlKeyEvent( int32_t key, bool down );
    void AddVariableEvent( const ST::string& name, float number );
    void AddVariableEvent( const ST::string& name, int32_t number );
    void AddVariableEvent( const ST::string& name );
    void AddVariableEvent( const ST::string& name, const plKey &key );
    void AddFacingEvent( const plKey &other, const plKey &self, float dot, bool enabled);
    void AddContainerEvent( const plKey &container, const plKey &contained, bool entering);
    void AddActivateEvent( bool activate );
    void AddCallbackEvent( int32_t event );
    void AddResponderStateEvent( int32_t state );
    void AddMultiStageEvent( int32_t stage, int32_t event, const plKey& avatar );
    void AddCoopEvent(uint32_t id, uint16_t serial);
    void AddSpawnedEvent (const plKey &spawner, const plKey &spawned);
    void AddClickDragEvent(const plKey& dragger, const plKey& dragee, float animPos);
    void AddOfferBookEvent(const plKey& offerer, int targetAge, int offeree);
    void AddBookEvent( uint32_t event, uint32_t linkID = 0 );
    void AddHitClimbingBlockerEvent(const plKey &blocker);
    proEventData* FindEventRecord( int32_t eventtype );
    size_t GetEventCount() { return fEvents.size(); }
    proEventData* GetEventRecord(size_t i) { return fEvents[i]; }
    void ClearEvents();

    // Searches the event records for an event triggered by an avatar, and returns that key
    plKey GetAvatarKey();

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};



#endif // _plNotifyMsg_h_
