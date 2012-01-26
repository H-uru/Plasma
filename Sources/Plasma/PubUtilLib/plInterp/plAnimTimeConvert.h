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
#ifndef plAnimTimeConvert_inc
#define plAnimTimeConvert_inc

#include "pnFactory/plCreatable.h"
#include "hsTemplates.h"
#include "pnNetCommon/plSynchedValue.h"

#pragma warning (disable: 4284) 

class plSynchedObject;
class plAnimCmdMsg;
class plEventCallbackMsg;
class plATCEaseCurve;
class plATCState;
class plATCAnim;
class plAGMasterMod;
class plAGAnimInstance;

class plAnimTimeConvert : public plCreatable
{
    friend class plAnimTimeConvertSDLModifier;
    friend class plAGAnimInstance;

protected:
    uint16_t                  fFlags;
    float                fBegin;
    float                fEnd;
    float                fLoopEnd;
    float                fLoopBegin;
    float                fSpeed;
    float                fCurrentAnimTime;
    float                fWrapTime;
    double                  fLastEvalWorldTime;
    // Do not change fLastEvalWorldTime anywhere except WorldToAnimTime()

    plSynchedObject*                            fOwner;
    double                                      fLastStateChange;

    typedef std::list<plATCState *> plATCStateList;
    plATCStateList fStates;
    
    hsTArray<float>              fStopPoints;
    hsTArray<plEventCallbackMsg*>   fCallbackMsgs;

    /////////////////////////
    // Ease In/Out stuff

    plATCEaseCurve *fEaseInCurve;
    plATCEaseCurve *fEaseOutCurve;
    plATCEaseCurve *fSpeedEaseCurve;
    plATCEaseCurve *fCurrentEaseCurve; // One of the above, or nil

    //
    /////////////////////////

    float fInitialBegin;
    float fInitialEnd;

    static float ICalcEaseTime(const plATCEaseCurve *curve, double start, double end);
    void            IClearSpeedEase();

    void        ICheckTimeCallbacks(float frameStart, float frameStop);
    hsBool      ITimeInFrame(float secs, float start, float stop);
    void        ISendCallback(int i);

    plAnimTimeConvert& IStop(double time, float animTime);
    hsBool IIsStoppedAt(const double &wSecs, const uint32_t &flags, const plATCEaseCurve *curve) const;
    plAnimTimeConvert& IProcessStateChange(double worldTime, float animTime = -1);
    void IFlushOldStates();
    void IClearAllStates();
    plATCState *IGetState(double wSecs) const;
    plATCState *IGetLatestState() const;
    
    plAnimTimeConvert& SetFlag(uint8_t f, hsBool on) { if(on)fFlags |= f; else fFlags &= ~f; return *this; }

public:
    plAnimTimeConvert();
    virtual ~plAnimTimeConvert();

    void Init(plATCAnim *anim, plAGAnimInstance *instance, plAGMasterMod *master);

    CLASSNAME_REGISTER( plAnimTimeConvert );
    GETINTERFACE_ANY( plAnimTimeConvert, plCreatable );

    void SetOwner(plSynchedObject* o);
    const plSynchedObject* GetOwner() const { return fOwner; }

    // ALL WorldToAnimTime functions are only valid if called with a time >= fLastEvalWorldTime.
    hsBool      IsStoppedAt(double wSecs) const;
    float    WorldToAnimTime(double wSecs);
    float    WorldToAnimTimeNoUpdate(double wSecs) const; // convert time but don't fire triggers or set state
    
protected:
    static float IWorldToAnimTimeNoUpdate(double wSecs, plATCState *state);
    float IWorldToAnimTimeBeforeState(double wSecs) const;

public:
    void SetBegin(float s) { fBegin = s; }
    void SetEnd(float s) { fEnd = s; }
    void SetSpeed(float goal, float rate = 0);
    void SetLoopPoints(float begin, float end) { SetLoopBegin(begin); SetLoopEnd(end); }
    void SetLoopBegin(float s) { fLoopBegin = s; }
    void SetLoopEnd(float s) { fLoopEnd = s; }
    void SetEase(hsBool easeIn, uint8_t inType, float minLength, float maxLength, float inLength);
    void SetCurrentEaseCurve(int x);    // 0=nil, 1=easeIn, 2=easeOut, 3=speed

    float GetBegin() const { return fBegin; }
    float GetEnd() const { return fEnd; }
    float GetLoopBegin() const { return fLoopBegin; }
    float GetLoopEnd() const { return fLoopEnd; }
    float GetSpeed() const { return fSpeed; }
    hsTArray<float> &GetStopPoints() { return fStopPoints; }
    float GetBestStopDist(float min, float max, float norm, float time) const;
    int GetCurrentEaseCurve() const;    // returns  0=nil, 1=easeIn, 2=easeOut, 3=speed

    void ResizeStates(int cnt);
    void ResetWrap();

    plAnimTimeConvert& ClearFlags() { fFlags = kNone; return *this; }
    hsBool GetFlag(uint8_t f) const { return (fFlags & f) ? true : false; }

    plAnimTimeConvert& InitStop(); // Called when initializing an anim that doesn't autostart
    plAnimTimeConvert& Stop(hsBool on);
    plAnimTimeConvert& Stop(double s = -1.0);
    plAnimTimeConvert& Start(double s = -1.0);
    plAnimTimeConvert& PlayToTime(float time);
    plAnimTimeConvert& PlayToPercentage(float percent); // zero to one.
        
    plAnimTimeConvert& Loop(hsBool on);
    plAnimTimeConvert& Loop() { return Loop(true); }
    plAnimTimeConvert& NoLoop() { return Loop(false); }

    plAnimTimeConvert& Backwards(hsBool on);
    plAnimTimeConvert& Backwards();
    plAnimTimeConvert& Forewards();

    hsBool IsStopped() const { return 0 != (fFlags & kStopped); }
    hsBool IsLooped() const { return 0 != (fFlags & kLoop); }
    hsBool IsBackwards() const { return 0 != (fFlags & kBackwards); }
    hsBool IsForewards() const { return !(fFlags & kBackwards); }

    double LastEvalWorldTime() const { return fLastEvalWorldTime; }
    float CurrentAnimTime() const { return fCurrentAnimTime; }
    void SetCurrentAnimTime(float s, hsBool jump = false);

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);

    hsBool HandleCmd(plAnimCmdMsg* msg);
    void AddCallback(plEventCallbackMsg* pMsg);
    void RemoveCallback(plEventCallbackMsg* pMsg);
    void ClearCallbacks();
    void EnableCallbacks(hsBool val);
    
    enum plAnimTimeFlags {
        kNone           = 0x0,
        kStopped        = 0x1,
        kLoop           = 0x2,
        kBackwards      = 0x4,
        kWrap           = 0x8,
        kNeedsReset     = 0x10,
        kEasingIn       = 0x20,
        kForcedMove     = 0x40,
        kNoCallbacks    = 0x80,

        kFlagsMask      = 0xff
    };
    
    enum plEaseCurveType {
        kEaseNone,
        kEaseIn,
        kEaseOut,
        kEaseSpeed,
    };

};

// Rules for happy ease curves:
//      1. Any time value between 0 and fLength is kosher.
//      2. Velocity values accepted/returned are in the range [fStartSpeed, fSpeed]
//          (some tolerance for values REALLY close to the limit, to account for floating-point inaccuracy)

class plATCEaseCurve : public plCreatable
{
protected:
    float fStartSpeed;
    float fMinLength;
    float fMaxLength;
    float fNormLength;

public:
    CLASSNAME_REGISTER( plATCEaseCurve );
    GETINTERFACE_ANY( plATCEaseCurve, plCreatable );

    double fBeginWorldTime;
    float fLength;
    float fSpeed; // The anim's target ("full") speed.

    static plATCEaseCurve *CreateEaseCurve(uint8_t type, float minLength, float maxLength, float normLength, 
                                           float startSpeed, float goalSpeed);

    double GetEndWorldTime() const { return fBeginWorldTime + fLength; } 

    virtual plATCEaseCurve *Clone() const = 0;
    virtual void Read(hsStream *s, hsResMgr *mgr);
    virtual void Write(hsStream *s, hsResMgr *mgr);
    
    virtual void RecalcToSpeed(float startSpeed, float goalSpeed, hsBool preserveRate = false);
    virtual void SetLengthOnRate(float rate);
    virtual void SetLengthOnDistance(float dist) = 0;
    virtual float PositionGivenTime(float time) const = 0;
    virtual float VelocityGivenTime(float time) const = 0;
    virtual float TimeGivenVelocity(float velocity) const = 0;
    virtual float GetMinDistance();
    virtual float GetMaxDistance();
    virtual float GetNormDistance();
};

class plConstAccelEaseCurve : public plATCEaseCurve
{
public:
    plConstAccelEaseCurve();
    plConstAccelEaseCurve(float minLength, float maxLength, float length, 
                          float startSpeed, float goalSpeed);

    CLASSNAME_REGISTER( plConstAccelEaseCurve );
    GETINTERFACE_ANY( plConstAccelEaseCurve, plATCEaseCurve );

    virtual plATCEaseCurve *Clone() const;
    virtual void SetLengthOnDistance(float dist);
    virtual float PositionGivenTime(float time) const;
    virtual float VelocityGivenTime(float time) const;
    virtual float TimeGivenVelocity(float velocity) const;
};

class plSplineEaseCurve : public plATCEaseCurve
{
public:
    plSplineEaseCurve();
    plSplineEaseCurve(float minLength, float maxLength, float length, 
                      float startSpeed, float goalSpeed);

    CLASSNAME_REGISTER( plSplineEaseCurve );
    GETINTERFACE_ANY( plSplineEaseCurve, plATCEaseCurve );

    virtual void Read(hsStream *s, hsResMgr *mgr);
    virtual void Write(hsStream *s, hsResMgr *mgr);

    virtual plATCEaseCurve *Clone() const;
    virtual void RecalcToSpeed(float startSpeed, float goalSpeed, hsBool preserveRate = false); 
    virtual void SetLengthOnDistance(float dist);
    virtual float PositionGivenTime(float time) const;
    virtual float VelocityGivenTime(float time) const;
    virtual float TimeGivenVelocity(float velocity) const;

    float fCoef[4];
};

class plATCState
{
public:
    plATCState() : fEaseCurve(nil) {}
    ~plATCState() { delete fEaseCurve; }

    void Read(hsStream *s, hsResMgr *mgr);
    void Write(hsStream *s, hsResMgr *mgr);

    double fStartWorldTime;
    float fStartAnimTime;

    uint8_t fFlags;
    float fBegin;
    float fEnd;
    float fLoopBegin;
    float fLoopEnd;
    float fSpeed;
    float fWrapTime;
    plATCEaseCurve *fEaseCurve;
};


#endif // plAnimTimeConvert_inc
