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

#ifndef plCollisionDetector_inc
#define plCollisionDetector_inc

#include "plDetectorModifier.h"
#include "hsGeometry3.h"
#include <list.h>
#include <set>
class plMessage;
class plCameraMsg;
class plArmatureMod;
class plActivatorMsg;
class plEvalMsg;

class plCollisionDetector : public plDetectorModifier
{
protected:
	Int8	fType;
	hsBool  fBumped, fTriggered;

	plArmatureMod* IGetAvatarModifier(plKey key);
	bool IIsDisabledAvatar(plKey key);

public:
	enum
	{
		kTypeEnter	= 0x01,
		kTypeExit	= 0x02,
		kTypeAny	= 0x04,
		kTypeUnEnter	= 0x08,
		kTypeUnExit		= 0x10,
		kTypeBump		= 0x20,
	};

	plCollisionDetector() : fType(0), fTriggered(false), fBumped(false){;}
	virtual ~plCollisionDetector(){;}
	
	virtual hsBool MsgReceive(plMessage* msg);

	CLASSNAME_REGISTER( plCollisionDetector );
	GETINTERFACE_ANY( plCollisionDetector, plDetectorModifier );

	virtual void SetType(Int8 i) { fType |= i; }

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

// sub type for object-in-volume detectors
class plObjectInVolumeDetector : public plCollisionDetector
{
public:
	class plCollisionBookKeepingInfo
	{
		friend plObjectInVolumeDetector;
		public:
			plCollisionBookKeepingInfo(plKey& hit)
			{
				hitter=hit;
				enters=0;
				exits=0;
			}
			~plCollisionBookKeepingInfo()
			{
				hitter=nil;
			}
		protected:
			plKey hitter;
			int enters,exits;
			bool fSubStepCurState;
	};
protected:
	virtual void ITrigger(plKey hitter, bool entering, bool immediate=false);
	//virtual void ISendSavedTriggerMsgs();
	virtual void IHandleEval(plEvalMsg* pEval);
	bool	fWaitingForEval;
	
	plActivatorMsg* fSavedActivatorMsg;
	
	typedef std::list<plCollisionBookKeepingInfo*> bookKeepingList;
	bookKeepingList fCollisionList;
	typedef std::set<plKey> ResidentSet;
	ResidentSet fCurrentResidents;

public:
	
	plObjectInVolumeDetector()
	{
		fWaitingForEval=false;fSavedActivatorMsg=nil;
		
	}
	plObjectInVolumeDetector(Int8 i){fType = i;fWaitingForEval=false;fSavedActivatorMsg=nil;}
	virtual ~plObjectInVolumeDetector(){;}
	
	virtual hsBool MsgReceive(plMessage* msg);

	CLASSNAME_REGISTER(plObjectInVolumeDetector);
	GETINTERFACE_ANY(plObjectInVolumeDetector, plCollisionDetector);

	virtual void SetTarget(plSceneObject* so);

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

class plObjectInVolumeAndFacingDetector : public plObjectInVolumeDetector
{
protected:
	hsScalar fFacingTolerance;
	bool fNeedWalkingForward;

	bool fAvatarInVolume;
	bool fTriggered;

	void ICheckForTrigger();

public:
	plObjectInVolumeAndFacingDetector();
	virtual ~plObjectInVolumeAndFacingDetector();
	
	virtual hsBool MsgReceive(plMessage* msg);

	CLASSNAME_REGISTER(plObjectInVolumeAndFacingDetector);
	GETINTERFACE_ANY(plObjectInVolumeAndFacingDetector, plObjectInVolumeDetector);

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	// Export only
	void SetFacingTolerance(int degrees);
	void SetNeedWalkingForward(bool v) { fNeedWalkingForward = v; }
};

// sub-type for camera command regions

class plCameraRegionDetector : public plObjectInVolumeDetector
{
protected:
	hsTArray<plCameraMsg*>	fMessages;
	bool	fIsInside;
	bool	fSavingSendMsg;
	bool	fSavedMsgEnterFlag;
	int		fNumEvals;
	int		fLastEnterEval;
	int		fLastExitEval;

	virtual void ITrigger(plKey hitter, bool entering, bool immediate=false);
	virtual void ISendSavedTriggerMsgs();
	virtual void IHandleEval(plEvalMsg* pEval);
public:
	plCameraRegionDetector(){ fIsInside = false; fSavingSendMsg = false; }
	~plCameraRegionDetector();

	virtual hsBool MsgReceive(plMessage* msg);
	void AddMessage(plCameraMsg* pMsg) { fMessages.Append(pMsg); }

	CLASSNAME_REGISTER( plCameraRegionDetector );
	GETINTERFACE_ANY( plCameraRegionDetector, plCollisionDetector );

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};


// sub-type for subworld regions

class plSubworldRegionDetector : public plCollisionDetector
{
protected:
	plKey fSub;
	hsBool fOnExit;

public:
	enum
	{
		kSubworld = 0,
	};
	plSubworldRegionDetector() : fSub(nil), fOnExit(false){;}
	~plSubworldRegionDetector();
	
	virtual hsBool MsgReceive(plMessage* msg);
	void SetSubworldKey(plKey pKey) { fSub = pKey; }
	void SetTriggerOnExit(hsBool b) { fOnExit = b; }

	CLASSNAME_REGISTER( plSubworldRegionDetector );
	GETINTERFACE_ANY( plSubworldRegionDetector, plCollisionDetector );

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

// sub-type for panic link regions

class plPanicLinkRegion : public plCollisionDetector
{
public:
	hsBool fPlayLinkOutAnim;
	
	plPanicLinkRegion() : fPlayLinkOutAnim(true) {;}
	~plPanicLinkRegion(){;}

	
	virtual hsBool MsgReceive(plMessage* msg);
	CLASSNAME_REGISTER( plPanicLinkRegion );
	GETINTERFACE_ANY( plPanicLinkRegion, plCollisionDetector );

	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);	
};


/** \Class plSimpleRegionSensor
	A dead-simple interface for a collision region. Holds one message that it
	sends to anyone who enters, and another message that it sends to anyone
	who exits.
	We may want to tie this into the plCollisionDetector so that it could be
	integrated with responders.
*/
class plSimpleRegionSensor : public plSingleModifier
{
public:
	plSimpleRegionSensor();
	plSimpleRegionSensor(plMessage *enterMsg, plMessage *exitMsg);
	virtual ~plSimpleRegionSensor();

	virtual hsBool MsgReceive(plMessage *msg);
	CLASSNAME_REGISTER( plSimpleRegionSensor );
	GETINTERFACE_ANY( plSimpleRegionSensor, plSingleModifier);

	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *stream, hsResMgr *mgr);

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
protected:
	plMessage *fEnterMsg;
	plMessage *fExitMsg;
};

// This class really just exists so that I can hunt for it specifically by index
// (and not accidentally get some other SimpleRegionSensor).
class plSwimDetector : public plSimpleRegionSensor
{
public:
	plSwimDetector() {}
	plSwimDetector(plMessage *enterMsg, plMessage *exitMsg) : plSimpleRegionSensor(enterMsg, exitMsg) {}
	virtual ~plSwimDetector() {}

	CLASSNAME_REGISTER( plSwimDetector );
	GETINTERFACE_ANY( plSwimDetector, plSimpleRegionSensor);
	
	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *stream, hsResMgr *mgr);	
	hsBool MsgReceive(plMessage *msg);
};
class plRidingAnimatedPhysicalDetector: public plSimpleRegionSensor
{
public:
	plRidingAnimatedPhysicalDetector(){}
	plRidingAnimatedPhysicalDetector(plMessage *enterMsg, plMessage *exitMsg) : plSimpleRegionSensor(enterMsg, exitMsg) {}
	virtual ~plRidingAnimatedPhysicalDetector(){}
	virtual hsBool MsgReceive(plMessage *msg);
	CLASSNAME_REGISTER( plRidingAnimatedPhysicalDetector );
	GETINTERFACE_ANY( plRidingAnimatedPhysicalDetector, plSimpleRegionSensor);
};
#endif plCollisionDetector_inc
