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

#ifndef plLineFollowMod_inc
#define plLineFollowMod_inc

#include "pnModifier/plMultiModifier.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"

class plAnimPath;
class plSceneObject;
class plStereizer;
class plListenerMsg;

class plLineFollowMod : public plMultiModifier
{
public:
    enum FollowMode {
        kFollowObject,
        kFollowListener,
        kFollowCamera,
        kFollowLocalAvatar,
    };
    enum RefType {
        kRefParent,
        kRefObject,
        kRefStereizer
    };

    
    enum {
        kNone           = 0x0,
        kFullMatrix     = 0x1,
        kOffsetFeet     = 0x2,
        kOffsetAng      = 0x4,
        kOffset         = kOffsetFeet | kOffsetAng,
        kOffsetClamp    = 0x8,
        kForceToLine    = 0x10,
        kSpeedClamp     = 0x20,
        kSearchPosPop   = 0x40  // Temp flag, gets set every time the target pops in position
    };
protected:
    FollowMode          fFollowMode;
    uint16_t              fFollowFlags;

    plAnimPath*         fPath;
    plSceneObject*      fPathParent;

    plSceneObject*      fRefObj;

    mutable hsPoint3    fSearchPos;

    hsTArray<plStereizer*>      fStereizers;

    float            fTanOffset;
    float            fOffset;
    float            fOffsetClamp;
    float            fSpeedClamp;

    virtual hsBool IEval(double secs, float del, uint32_t dirty);
    
    virtual hsBool      IGetSearchPos();
    virtual void        ISetTargetTransform(int iTarg, const hsMatrix44& tgtXfm);
    virtual void        ISetPathTransform();

    virtual hsBool      IGetTargetTransform(hsPoint3& searchPos, hsMatrix44& tgtXfm);
    virtual hsBool      IOffsetTargetTransform(hsMatrix44& tgtXfm);
    virtual hsMatrix44  ISpeedClamp(plCoordinateInterface* ci, const hsMatrix44& unclTgtXfm);
    hsMatrix44          IInterpMatrices(const hsMatrix44& m0, const hsMatrix44& m1, float parm);
    void                ICheckForPop(const hsPoint3& oldPos, const hsPoint3& newPos);

    void                ISetupStereizers(const plListenerMsg* listMsg);

    void IUnRegister();
    void IRegister();

public:

    plLineFollowMod();
    ~plLineFollowMod();

    CLASSNAME_REGISTER( plLineFollowMod );
    GETINTERFACE_ANY( plLineFollowMod, plMultiModifier );
    
    virtual void Read(hsStream* stream, hsResMgr* mgr);
    virtual void Write(hsStream* stream, hsResMgr* mgr);

    // Export time stuff
    void                SetPath(plAnimPath* path);
    const plAnimPath*   GetPath() const { return fPath; }

    void                SetFollowMode(FollowMode f);
    FollowMode          GetFollowMode() const { return fFollowMode; }

    hsBool              HasOffsetFeet() const { return 0 != (fFollowFlags & kOffsetFeet); }
    hsBool              HasOffsetDegrees() const { return 0 != (fFollowFlags & kOffsetAng); }
    hsBool              HasOffset() const { return 0 != (fFollowFlags & kOffset); }
    hsBool              HasOffsetClamp() const { return 0 != (fFollowFlags & kOffsetClamp); }
    hsBool              HasSpeedClamp() const { return 0 != (fFollowFlags & kSpeedClamp); }

    void                SetOffsetFeet(float f);
    float            GetOffsetFeet() const { return fOffset; }

    void                SetOffsetDegrees(float f);
    float            GetOffsetDegrees() const { return hsRadiansToDegrees(fOffset); }

    void                SetOffsetClamp(float f);
    float            GetOffsetClamp() const { return fOffsetClamp; }

    void                SetForceToLine(hsBool on);
    hsBool              GetForceToLine() const { return 0 != (fFollowFlags & kForceToLine); }

    void                SetSpeedClamp(float feetPerSec);
    float            GetSpeedClamp() const { return fSpeedClamp; }

    hsBool              MsgReceive(plMessage* msg);

    virtual void AddTarget(plSceneObject* so);
    virtual void RemoveTarget(plSceneObject* so); 

    void        AddStereizer(const plKey& sterKey);
    void        RemoveStereizer(const plKey& sterKey);
};


class plRailCameraMod : public plLineFollowMod
{

public:

    plRailCameraMod();
    ~plRailCameraMod();

    CLASSNAME_REGISTER( plRailCameraMod );
    GETINTERFACE_ANY( plRailCameraMod, plLineFollowMod );
    
    void Init() { fCurrentTime = -1; } // twiddle ourselves so we get ready to go...
    hsPoint3 GetGoal(double secs, float speed);

protected:
    
    virtual void        ISetTargetTransform(int iTarg, const hsMatrix44& tgtXfm) {fDesiredMatrix = tgtXfm;}
    virtual hsBool      IGetTargetTransform(hsPoint3& searchPos, hsMatrix44& tgtXfm);
    
    hsMatrix44  fDesiredMatrix;
    float    fCurrentTime;
    float    fTargetTime;
    hsPoint3    fGoal;
    hsBool      fFarthest;
};
#endif // plLineFollowMod_inc
