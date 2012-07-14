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

#ifndef plCameraModifier_inc
#define plCameraModifier_inc

#include "pnModifier/plSingleModifier.h"
#include "hsBitVector.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"

class plPipeline;
class plKey;
class plCameraBrain1;
class plCameraMsg;
struct CamTrans
{
    // used when creating default track transitions at runtime
    CamTrans(plKey to)
    {
        fTransTo = to;

        fAccel = 60.0f;
        fDecel = 60.0f;
        fVelocity = 60.0f;
        fPOADecel = 60.0f;
        fPOAAccel = 60.0f;
        fPOAVelocity = 60.0f;

        fCutPos = false;
        fCutPOA = false;
        fIgnore = false;
    }
    plKey       fTransTo;

    bool    fCutPos;
    bool    fCutPOA;
    bool    fIgnore;
    float fAccel;
    float fDecel;
    float fVelocity;
    float fPOAAccel;
    float fPOADecel;
    float fPOAVelocity;

};

class plCameraModifier1 : public plSingleModifier
{
    enum
    {
        kRefBrain,
        kRefCut,
        kRefTrack,
        kRefCallbackMsg,
    };
protected:

    void Output();
    virtual bool IEval(double secs, float del, uint32_t dirty) { return true; }
        
public:
    
    plCameraModifier1();
    virtual ~plCameraModifier1();

    CLASSNAME_REGISTER( plCameraModifier1 );
    GETINTERFACE_ANY( plCameraModifier1, plSingleModifier );

    virtual bool MsgReceive(plMessage* msg);

    void        Initialize();
    virtual void Update();

    virtual void AddTarget(plSceneObject* so);

    void    SetBrain(plCameraBrain1* brain) { fBrain = brain; }

    plCameraBrain1* GetBrain()      { return fBrain;}

    hsPoint3        GetTargetPos() { return fFrom; }    
    hsPoint3        GetTargetPOA() { return fAt; }  
    hsPoint3        GetSubworldPos() { return fLastSubPos; }    
    hsPoint3        GetSubworldPOA() { return fLastSubPOA; }    

    
    void            SetTransform(hsPoint3 at);
    void            SetTargetPos(hsPoint3 pos) { fFrom = pos; }
    void            SetTargetPOA(hsPoint3 pos) { fAt = pos; }
    void            SetSubworldPos(hsPoint3 pos) { fLastSubPos = pos; }
    void            SetSubworldPOA(hsPoint3 pos) { fLastSubPOA = pos; }
    float        GetFOVw() { return fFOVw; }
    float        GetFOVh() { return fFOVh; }
    void            SetFOVw(float f, bool fUpdateVCam = true); 
    void            SetFOVh(float f, bool fUpdateVCam = true); 
    bool            GetInSubworld() { return fInSubLastUpdate; }
    void            InSubworld(bool b) { fInSubLastUpdate = b; }
    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);
    void AddTrans(CamTrans* t) { fTrans.Append(t); }
    int  GetNumTrans() { return fTrans.Count(); }
    CamTrans* GetTrans(int i) { return fTrans[i]; }
    void SetSubject(plSceneObject* pObj); 
    plSceneObject* GetSubject();

    virtual void Push(bool recenter = true); 
    virtual void Pop(); 

    virtual bool    GetFaded();
    virtual bool    SetFaded(bool b);

    bool    IsAnimated() { return fAnimated; }
    void SetAnimCommands(bool a, bool b, bool c)  { fStartAnimOnPush = a; fStopAnimOnPop = b; fResetAnimOnPop = c; }

private:
    hsPoint3                fFrom;
    hsPoint3                fAt;
    plCameraBrain1*         fBrain; // the 'logic' portion of the camera
    hsTArray<CamTrans*>     fTrans;
    plSceneObject*          fSubObj;
    float                fFOVw;
    float                fFOVh;
    hsTArray<plMessage*>    fMessageQueue;
    hsTArray<plCameraMsg*>  fFOVInstructions;
    bool                    fAnimated, fStartAnimOnPush, fStopAnimOnPop, fResetAnimOnPop;
    hsPoint3                fLastSubPos;
    hsPoint3                fLastSubPOA;
    bool                    fInSubLastUpdate;
    bool                    fUpdateBrainTarget; // sometimes our target isn't loaded yet, so wait to update the brain til later
};



#endif //plCameraModifier_inc
