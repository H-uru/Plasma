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

#ifndef plCameraMsg_inc
#define plCameraMsg_inc

//
// camera message class
//
#include "plMessage.h"
#include "hsBitVector.h"
#include "hsGeometry3.h"

class plSceneObject;
class plPipeline;
class hsStream;
class hsResMgr;

class plCameraConfig
{
public:

    plCameraConfig()
        : fAccel(),fDecel(),
          fVel(), fFPAccel(),
          fFPDecel(), fFPVel(),
          fFOVw(), fFOVh(),
          fType(), fWorldspace()
    { }

    plCameraConfig(int flags)
        : fAccel(), fDecel(),
        fVel(), fFPAccel(),
        fFPDecel(), fFPVel(),
        fFOVw(), fFOVh(),
        fType(flags), fWorldspace()
    { }

    enum
    {
        kOffset     = 0x0001,
        kSpeeds     = 0x0002,
        kFOV        = 0x0004,
    };

    int          fType;

    hsPoint3     fOffset;
    float        fAccel;
    float        fDecel;
    float        fVel;
    float        fFPAccel;
    float        fFPDecel;
    float        fFPVel;
    float        fFOVw, fFOVh;   
    bool         fWorldspace;

    void Read(hsStream* stream);
    void Write(hsStream* stream);

};

class plCameraTargetFadeMsg : public plMessage
{
protected:

    plKey           fSubject;
    bool            fFadeOut;

public:
    
    plKey   GetSubjectKey() const { return fSubject; }
    
    void SetSubjectKey(const plKey &x) { fSubject = x; }
    bool FadeOut() const { return fFadeOut; }
    void SetFadeOut(bool b) { fFadeOut = b; }

    
    plCameraTargetFadeMsg() { }

    CLASSNAME_REGISTER(plCameraTargetFadeMsg);
    GETINTERFACE_ANY(plCameraTargetFadeMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

};

class plCameraMsg : public plMessage
{
protected:

    plKey           fNewCam;
    plKey           fTriggerer;
    double          fTransTime;
    plSceneObject*  fSubject;
    plPipeline*     fPipe;
    plCameraConfig  fConfig;

    bool            fActivated;

public:
    
    plKey   GetNewCam() const { return fNewCam; }
    plKey   GetTriggerer() const { return fTriggerer; }
    double  GetTransTime() const { return fTransTime; }
    plSceneObject*  GetSubject() const { return fSubject; }
    plPipeline* GetPipeline() const { return fPipe; }
    bool   GetActivated() const { return fActivated; }
    plCameraConfig* GetConfig() { return &fConfig; }

    void SetNewCam(const plKey &x) { fNewCam = x; }
    void SetTriggerer(const plKey &x) { fTriggerer = x; }
    void SetTransTime(double x) { fTransTime = x; }
    void SetSubject(plSceneObject* x) { fSubject = x; }
    void SetPipeline(plPipeline* x) { fPipe = x; }
    void SetActivated(bool x) { fActivated = x; }
        
    plCameraMsg();
    plCameraMsg(const plKey &s, 
                    const plKey &r, 
                    const double* t);
    
    CLASSNAME_REGISTER(plCameraMsg);
    GETINTERFACE_ANY(plCameraMsg, plMessage);

    enum ModCmds
    {
        kSetSubject = 0,
        kCameraMod,
        kSetAsPrimary,
        kTransitionTo,
        kPush,
        kPop,
//      kSetOffset,
//      kRegionOffset,
        kEntering,
//      kSetFirstPerson,
//      kRegionFirstPerson,
//      kRegionPush,
        kCut,
//      kModDestroy,
        kResetOnEnter,
        kResetOnExit,
        kChangeParams,
        kWorldspace,
        kCreateNewDefaultCam,
        kRegionPushCamera,
        kRegionPopCamera,
        kRegionPushPOA,
        kRegionPopPOA,
        kFollowLocalPlayer,
        kResponderTrigger,
        kSetFOV,
        kAddFOVKeyframe,
        kStartZoomIn,
        kStartZoomOut,
        kStopZoom,
        kSetAnimated,
        kPythonOverridePush,
        kPythonOverridePop,
        kPythonOverridePushCut,
        kPythonSetFirstPersonOverrideEnable,
        kPythonUndoFirstPerson,
        kUpdateCameras,
        kResponderSetThirdPerson,
        kResponderUndoThirdPerson,
        kNonPhysOn,
        kNonPhysOff,
        kResetPanning,
        kRefreshFOV,
        kNumCmds
    };

    hsBitVector     fCmd;

    bool Cmd(int n) const { return fCmd.IsBitSet(n); }
    void SetCmd(int n) { fCmd.SetBit(n); }
    void ClearCmd() { fCmd.Clear(); }
    void ClearCmd(int n) { fCmd.ClearBit(n); }

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

};


class plIfaceFadeAvatarMsg : public plMessage
{
protected:

    plKey           fSubject;
    bool            fFadeOut;
    bool            fEnable, fDisable;
    
public:
    
    plKey   GetSubjectKey() const { return fSubject; }
    
    void SetSubjectKey(const plKey &x) { fSubject = x; }
    bool FadeOut() const { return fFadeOut; }
    void SetFadeOut(bool b) { fFadeOut = b; }
    void Enable() { fEnable = true; }
    void Disable() { fDisable = true; }
    bool GetEnable() const { return fEnable; }
    bool GetDisable() const { return fDisable; }

    plIfaceFadeAvatarMsg() : fEnable(false),fDisable(false) { }

    CLASSNAME_REGISTER(plIfaceFadeAvatarMsg);
    GETINTERFACE_ANY(plIfaceFadeAvatarMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

};

#endif // plCameraMsg_inc
