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

#ifndef plCollisionDetector_inc
#define plCollisionDetector_inc

#include "plDetectorModifier.h"
#include "hsGeometry3.h"
#include <list>
#include <set>
class plMessage;
class plCameraMsg;
class plArmatureMod;
class plActivatorMsg;
class plEvalMsg;

class plCollisionDetector : public plDetectorModifier
{
protected:
    int8_t    fType;
    bool    fBumped, fTriggered;

    plArmatureMod* IGetAvatarModifier(plKey key);
    bool IIsDisabledAvatar(plKey key);

public:
    enum
    {
        kTypeEnter  = 0x01,
        kTypeExit   = 0x02,
        kTypeAny    = 0x04,
        kTypeUnEnter    = 0x08,
        kTypeUnExit     = 0x10,
        kTypeBump       = 0x20,
    };

    plCollisionDetector() : fType(0), fTriggered(false), fBumped(false){ }
    plCollisionDetector(int8_t type) : fType(type), fTriggered(false), fBumped(false) { }
    virtual ~plCollisionDetector(){;}
    
    virtual bool MsgReceive(plMessage* msg);

    CLASSNAME_REGISTER( plCollisionDetector );
    GETINTERFACE_ANY( plCollisionDetector, plDetectorModifier );

    virtual void SetType(int8_t i) { fType |= i; }

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);
};

// sub type for object-in-volume detectors
class plObjectInVolumeDetector : public plCollisionDetector
{
protected:
    virtual void ITrigger(plKey hitter, bool entering, bool immediate=false);
    virtual void ISendSavedTriggerMsgs();
    
    plActivatorMsg* fSavedActivatorMsg;
    uint32_t fNumEvals;
    uint32_t fLastEnterEval;
    uint32_t fLastExitEval;

public:
    
    plObjectInVolumeDetector() 
        : plCollisionDetector(), fSavedActivatorMsg(nil), fNumEvals(0), fLastEnterEval(0), fLastExitEval(0) 
    { }

    plObjectInVolumeDetector(int8_t type) 
        : plCollisionDetector(type), fSavedActivatorMsg(nil), fNumEvals(0), fLastEnterEval(0), fLastExitEval(0) 
    { }

    virtual ~plObjectInVolumeDetector() { }
    
    virtual bool MsgReceive(plMessage* msg);

    CLASSNAME_REGISTER(plObjectInVolumeDetector);
    GETINTERFACE_ANY(plObjectInVolumeDetector, plCollisionDetector);

    virtual void SetTarget(plSceneObject* so);

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);
};

class plObjectInVolumeAndFacingDetector : public plObjectInVolumeDetector
{
protected:
    float fFacingTolerance;
    bool fNeedWalkingForward;

    bool fAvatarInVolume;
    bool fTriggered;

    void ICheckForTrigger();

public:
    plObjectInVolumeAndFacingDetector();
    virtual ~plObjectInVolumeAndFacingDetector();
    
    virtual bool MsgReceive(plMessage* msg);

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
    typedef std::vector<plCameraMsg*> plCameraMsgVec;

    plCameraMsgVec  fMessages;
    bool    fIsInside;
    bool    fSavingSendMsg;
    bool    fSavedMsgEnterFlag;

    virtual void ITrigger(plKey hitter, bool entering, bool immediate=false);
    virtual void ISendSavedTriggerMsgs();
public:
    plCameraRegionDetector()
        : plObjectInVolumeDetector(), fIsInside(false), fSavingSendMsg(false)
    { }
    ~plCameraRegionDetector();

    virtual bool MsgReceive(plMessage* msg);
    void AddMessage(plCameraMsg* pMsg) { fMessages.push_back(pMsg); }

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
    bool fOnExit;

public:
    enum
    {
        kSubworld = 0,
    };
    plSubworldRegionDetector() : fSub(nil), fOnExit(false){;}
    ~plSubworldRegionDetector();
    
    virtual bool MsgReceive(plMessage* msg);
    void SetSubworldKey(plKey pKey) { fSub = pKey; }
    void SetTriggerOnExit(bool b) { fOnExit = b; }

    CLASSNAME_REGISTER( plSubworldRegionDetector );
    GETINTERFACE_ANY( plSubworldRegionDetector, plCollisionDetector );

    void Read(hsStream* stream, hsResMgr* mgr);
    void Write(hsStream* stream, hsResMgr* mgr);
};

// sub-type for panic link regions

class plPanicLinkRegion : public plCollisionDetector
{
public:
    bool fPlayLinkOutAnim;
    
    plPanicLinkRegion() : fPlayLinkOutAnim(true) { }

    
    virtual bool MsgReceive(plMessage* msg);
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

    virtual bool MsgReceive(plMessage *msg);
    CLASSNAME_REGISTER( plSimpleRegionSensor );
    GETINTERFACE_ANY( plSimpleRegionSensor, plSingleModifier);

    virtual void Write(hsStream *stream, hsResMgr *mgr);
    virtual void Read(hsStream *stream, hsResMgr *mgr);

    virtual bool IEval(double secs, float del, uint32_t dirty);
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
    bool MsgReceive(plMessage *msg);
};
class plRidingAnimatedPhysicalDetector: public plSimpleRegionSensor
{
public:
    plRidingAnimatedPhysicalDetector(){}
    plRidingAnimatedPhysicalDetector(plMessage *enterMsg, plMessage *exitMsg) : plSimpleRegionSensor(enterMsg, exitMsg) {}
    virtual ~plRidingAnimatedPhysicalDetector(){}
    virtual bool MsgReceive(plMessage *msg);
    CLASSNAME_REGISTER( plRidingAnimatedPhysicalDetector );
    GETINTERFACE_ANY( plRidingAnimatedPhysicalDetector, plSimpleRegionSensor);
};
#endif //plCollisionDetector_inc
