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

#ifndef plVirtualCam1_inc
#define plVirtualCam1_inc

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"

#include "pnKeyedObject/hsKeyedObject.h"

#include <vector>

class plCameraModifier1;
class plCameraBrain1;
class plCameraProxy;
struct CamTrans;
struct hsColorRGBA;
class plDebugInputInterface;
class plDrawableSpans;
class hsGMaterial;
class plKey;
class plPipeline;
class plPlate;
class plSceneNode;
class plSceneObject;

#define POS_TRANS_OFF       0
#define POS_TRANS_FIXED     1
#define POS_TRANS_FOLLOW    2
#define POA_TRANS_OFF       3
#define POA_TRANS_FIXED     4
#define POA_TRANS_FOLLOW    5

class plVirtualCam1 : public hsKeyedObject
{
    typedef std::vector<plCameraModifier1*> plCameraVec;
    typedef std::vector<plSceneObject*>     plSOVec;

protected:
    

    void Output();
    void SetOutputFOV();
    void IUpdate();
    void INext();

public:
    enum flags
    {
        kSetFOV,
        /** Forces the next camera transition to be cut. */
        kCutNextTrans,
        kRender,
        kRegionIgnore,
        kFirstPersonEnabled,
        kResponderForced3rd,
        kScriptsForced3rd,
        kScriptsDisabled1st,
        kAvatarWalking,
        kUnPanCamera,
        kInterpPanLimits,
        kFalling,
        //kRegisteredForBehaviors, // not reliable anymore since we have a dummy avatar in the startup age
        kFirstPersonAtLinkOut,
        kJustLinkedIn,
        kFirstPersonUserSelected,
    };
    
    enum action
    {
        kPush   = 0,
        kPop,
        kReplacement,
        kBackgroundPop,
        kRefCamera,
    };
    plVirtualCam1();
    virtual ~plVirtualCam1();

    CLASSNAME_REGISTER( plVirtualCam1 );
    GETINTERFACE_ANY( plVirtualCam1, hsKeyedObject );

    void SetPipeline(plPipeline* p); 
    void Init();

    bool MsgReceive(plMessage* msg) override;
    static void SetFOV(float x, float y);
    static void SetFOV(plCameraModifier1* pCam);
    static void SetDepth(float h, float y);
    static float GetFOVw() { return fFOVw; }
    static float GetFOVh() { return fFOVh; }
    static float GetHither() { return fHither; }
    static float GetYon()    { return fYon; }
    static void  SetOffset(float x, float y, float z);
    static void Refresh();
    static float GetAspectRatio() { return fAspectRatio; }
    static void SetAspectRatio(float ratio);

    bool InTransition() { return fTransPos != POS_TRANS_OFF; }
    plCameraModifier1* GetCurrentCamera();
    plCameraModifier1* GetCurrentStackCamera();
    plCameraModifier1* GetTransitionCamera(){return fTransitionCamera;}
    bool Is1stPersonCamera();

    bool    HasMovementFlag(int f) { return fMoveFlags.IsBitSet(f); }
    void    SetMovementFlag(int f, bool on = true) { fMoveFlags.SetBit(f, on);} 
        
    hsPoint3 GetCameraPos() const { return fOutputPos; }
    hsPoint3 GetCameraPOA() const { return fOutputPOA; }
    hsVector3 GetCameraUp() const { return fOutputUp; }
    void    SetCutNextTrans(); // used when player warps into a new camera region
    void    SetCutNext();

    const hsMatrix44 GetCurrentMatrix() { return fMatrix; }
    static plVirtualCam1* Instance() { return fInstance; }
    
    size_t GetNumCameras() { return fCameraStack.size(); }
    plCameraModifier1* GetCameraNumber(size_t camNumber);
    void RebuildStack(const plKey& key);

    void SetFlags(int flag) { fFlags.SetBit(flag); }
    bool HasFlags(int flag) { return fFlags.IsBitSet(flag); }
    void ClearFlags(int flag) { fFlags.ClearBit(flag); }

    // console command stuff
    static void Next();
    static void Prev();
    static void Deactivate();
    void CameraRegions(bool b) { fFlags.SetBit(kRegionIgnore,b); }
    void LogFOV(bool b) { printFOV = b; }
    void Drive();
    void PushThirdPerson();
    
    static void AddMsgToLog(const char* msg);
    static bool IsCurrentCamera(const plCameraModifier1* mod);
    void ClearStack();

    void AddCameraLoaded(plSceneObject* pCam) { fCamerasLoaded.push_back(pCam); }
    bool RestoreFromName(const ST::string& name);
    void StartUnPan();
    // these are for console access
    static bool fUseAccelOverride, freeze, alwaysCutForColin, WalkPan3rdPerson,StayInFirstPersonForever;
    static float fDecel, fAccel, fVel;
    static float fFallTimerDelay;
    
private:

    void Reset(bool bRender);
    void PushCamera(plCameraModifier1* pCam, bool bDefault = false);
    void PopCamera(plCameraModifier1* pCam);
    void AddCameraToStack(plCameraModifier1* pCam);
    void PopAll();
    void CreateDefaultCamera(plSceneObject* subject);
    void StartTransition(CamTrans* transition);
    void RunTransition();
    void FinishTransition();
    void SetRender(bool render);
    void IHandleCameraStatusLog(plCameraModifier1* pMod, int action);
    void FreezeOutput(int frames) { fFreezeCounter = frames; } // I hate this and I hate myself for doing it
    void UnFadeAvatarIn(int frames) { fFadeCounter = frames; } // ditto
    void FirstPersonOverride();
    
    void AdjustForInput();
    void UnPanIfNeeded();
    void StartInterpPanLimits();
    void InterpPanLimits();
    
    plPipeline*         fPipe;
    hsMatrix44          fMatrix;
    hsPoint3            fOutputPos;
    hsPoint3            fOutputPOA;
    hsVector3           fOutputUp;
    int                 fTransPos;
    plDebugInputInterface*  fCameraDriveInterface;
    FILE*               foutLog;
    plCameraVec         fCameraStack;
    int                 fFreezeCounter;
    int                 fFadeCounter;
    hsBitVector         fFlags;
    plSOVec             fCamerasLoaded;
    hsBitVector         fMoveFlags;
    float               fX;
    float               fY;
    float               fXPanLimit;
    float               fZPanLimit;
    float               fXPanLimitGoal;
    float               fZPanLimitGoal;
    float               fXUnPanRate;
    float               fZUnPanRate;
    float               fXPanInterpRate;
    float               fZPanInterpRate;
    double              fUnPanEndTime;
    double              fInterpPanLimitTime;
    float               fRetainedFY;

    // built-in cameras
    plCameraModifier1*  fDriveCamera; // for driving around 
    plCameraModifier1*  fTransitionCamera; // transitions between cameras placed in scenes
    plCameraModifier1*  fPythonOverride; // a special camera pushed by python
    plCameraModifier1*  fFirstPersonOverride; // the built-in first person camera
    plCameraModifier1*  fPrevCam; // the last camera we were displaying
    plCameraModifier1*  fThirdPersonCam; // built in third person cam for ccr's when they jump about

    static float fFOVh, fFOVw;
    static float fAspectRatio;
    static float fHither, fYon;
    static plVirtualCam1* fInstance;
    static bool printFOV; 
    static float fPanResponseTime;
    bool fForceCutOnce;

};


#endif //plVirtualCam1_inc
