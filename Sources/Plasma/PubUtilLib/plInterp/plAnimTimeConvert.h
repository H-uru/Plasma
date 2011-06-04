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
#ifndef plAnimTimeConvert_inc
#define plAnimTimeConvert_inc

#include "../pnFactory/plCreatable.h"
#include "hsTemplates.h"
#include "../pnNetCommon/plSynchedValue.h"

#pragma warning (disable: 4284) 

class plSynchedObject;
class plAnimCmdMsg;
class plEventCallbackMsg;
class plATCEaseCurve;
class plATCState;
class plATCAnim;
class plAGMasterMod;

class plAnimTimeConvert : public plCreatable
{
	friend class plAnimTimeConvertSDLModifier;
	friend class plAGAnimInstance;

protected:
	UInt16					fFlags;
	hsScalar				fBegin;
	hsScalar				fEnd;
	hsScalar				fLoopEnd;
	hsScalar				fLoopBegin;
	hsScalar				fSpeed;
	hsScalar				fCurrentAnimTime;
	hsScalar				fWrapTime;
	double					fLastEvalWorldTime;
	// Do not change fLastEvalWorldTime anywhere except WorldToAnimTime()

	plSynchedObject*							fOwner;
	double										fLastStateChange;

	typedef std::list<plATCState *> plATCStateList;
	plATCStateList fStates;
	
	hsTArray<hsScalar>				fStopPoints;
	hsTArray<plEventCallbackMsg*>	fCallbackMsgs;

	/////////////////////////
	// Ease In/Out stuff

	plATCEaseCurve *fEaseInCurve;
	plATCEaseCurve *fEaseOutCurve;
	plATCEaseCurve *fSpeedEaseCurve;
	plATCEaseCurve *fCurrentEaseCurve; // One of the above, or nil

	//
	/////////////////////////

	hsScalar fInitialBegin;
	hsScalar fInitialEnd;

	static hsScalar	ICalcEaseTime(const plATCEaseCurve *curve, double start, double end);
	void			IClearSpeedEase();

	void		ICheckTimeCallbacks(hsScalar frameStart, hsScalar frameStop);
	hsBool		ITimeInFrame(hsScalar secs, hsScalar start, hsScalar stop);
	void		ISendCallback(int i);

	plAnimTimeConvert& IStop(double time, hsScalar animTime);
	hsBool IIsStoppedAt(const double &wSecs, const UInt32 &flags, const plATCEaseCurve *curve) const;
	plAnimTimeConvert& IProcessStateChange(double worldTime, hsScalar animTime = -1);
	void IFlushOldStates();
	void IClearAllStates();
	plATCState *IGetState(double wSecs) const;
	plATCState *IGetLatestState() const;
	
	plAnimTimeConvert& SetFlag(UInt8 f, hsBool on) { if(on)fFlags |= f; else fFlags &= ~f; return *this; }

public:
	plAnimTimeConvert();
	virtual ~plAnimTimeConvert();

	void Init(plATCAnim *anim, plAGAnimInstance *instance, plAGMasterMod *master);

	CLASSNAME_REGISTER( plAnimTimeConvert );
	GETINTERFACE_ANY( plAnimTimeConvert, plCreatable );

	void SetOwner(plSynchedObject* o);
	const plSynchedObject* GetOwner() const { return fOwner; }

	// ALL WorldToAnimTime functions are only valid if called with a time >= fLastEvalWorldTime.
	hsBool		IsStoppedAt(double wSecs) const;
	hsScalar	WorldToAnimTime(double wSecs);
	hsScalar	WorldToAnimTimeNoUpdate(double wSecs) const; // convert time but don't fire triggers or set state
	
protected:
	static hsScalar IWorldToAnimTimeNoUpdate(double wSecs, plATCState *state);
	hsScalar IWorldToAnimTimeBeforeState(double wSecs) const;

public:
	void SetBegin(hsScalar s) { fBegin = s; }
	void SetEnd(hsScalar s) { fEnd = s; }
	void SetSpeed(hsScalar goal, hsScalar rate = 0);
	void SetLoopPoints(hsScalar begin, hsScalar end) { SetLoopBegin(begin); SetLoopEnd(end); }
	void SetLoopBegin(hsScalar s) { fLoopBegin = s; }
	void SetLoopEnd(hsScalar s) { fLoopEnd = s; }
	void SetEase(hsBool easeIn, UInt8 inType, hsScalar minLength, hsScalar maxLength, hsScalar inLength);
	void SetCurrentEaseCurve(int x);	// 0=nil, 1=easeIn, 2=easeOut, 3=speed

	hsScalar GetBegin() const { return fBegin; }
	hsScalar GetEnd() const { return fEnd; }
	hsScalar GetLoopBegin() const { return fLoopBegin; }
	hsScalar GetLoopEnd() const { return fLoopEnd; }
	hsScalar GetSpeed() const { return fSpeed; }
	hsTArray<hsScalar> &GetStopPoints() { return fStopPoints; }
	hsScalar GetBestStopDist(hsScalar min, hsScalar max, hsScalar norm, hsScalar time) const;
	int GetCurrentEaseCurve() const;	// returns  0=nil, 1=easeIn, 2=easeOut, 3=speed

	void ResizeStates(int cnt);
	void ResetWrap();

	plAnimTimeConvert& ClearFlags() { fFlags = kNone; return *this; }
	hsBool GetFlag(UInt8 f) const { return (fFlags & f) ? true : false; }

	plAnimTimeConvert& InitStop(); // Called when initializing an anim that doesn't autostart
	plAnimTimeConvert& Stop(hsBool on);
	plAnimTimeConvert& Stop(double s = -1.0);
	plAnimTimeConvert& Start(double s = -1.0);
	plAnimTimeConvert& PlayToTime(hsScalar time);
	plAnimTimeConvert& PlayToPercentage(hsScalar percent); // zero to one.
		
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
	hsScalar CurrentAnimTime() const { return fCurrentAnimTime; }
	void SetCurrentAnimTime(hsScalar s, hsBool jump = false);

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	hsBool HandleCmd(plAnimCmdMsg* msg);
	void AddCallback(plEventCallbackMsg* pMsg);
	void RemoveCallback(plEventCallbackMsg* pMsg);
	void ClearCallbacks();
	void EnableCallbacks(hsBool val);
	
	enum plAnimTimeFlags {
		kNone			= 0x0,
		kStopped		= 0x1,
		kLoop			= 0x2,
		kBackwards		= 0x4,
		kWrap			= 0x8,
		kNeedsReset		= 0x10,
		kEasingIn		= 0x20,
		kForcedMove		= 0x40,
		kNoCallbacks	= 0x80,

		kFlagsMask		= 0xff
	};
	
	enum plEaseCurveType {
		kEaseNone,
		kEaseIn,
		kEaseOut,
		kEaseSpeed,
	};

};

// Rules for happy ease curves:
//		1. Any time value between 0 and fLength is kosher.
//		2. Velocity values accepted/returned are in the range [fStartSpeed, fSpeed]
//			(some tolerance for values REALLY close to the limit, to account for floating-point inaccuracy)

class plATCEaseCurve : public plCreatable
{
protected:
	hsScalar fStartSpeed;
	hsScalar fMinLength;
	hsScalar fMaxLength;
	hsScalar fNormLength;

public:
	CLASSNAME_REGISTER( plATCEaseCurve );
	GETINTERFACE_ANY( plATCEaseCurve, plCreatable );

	double fBeginWorldTime;
	hsScalar fLength;
	hsScalar fSpeed; // The anim's target ("full") speed.

	static plATCEaseCurve *CreateEaseCurve(UInt8 type, hsScalar minLength, hsScalar maxLength, hsScalar normLength, 
										   hsScalar startSpeed, hsScalar goalSpeed);

	double GetEndWorldTime() const { return fBeginWorldTime + fLength; } 

	virtual plATCEaseCurve *Clone() const = 0;
	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);
	
	virtual void RecalcToSpeed(hsScalar startSpeed, hsScalar goalSpeed, hsBool preserveRate = false);
	virtual void SetLengthOnRate(hsScalar rate);
	virtual void SetLengthOnDistance(hsScalar dist) = 0;
	virtual hsScalar PositionGivenTime(hsScalar time) const = 0;
	virtual hsScalar VelocityGivenTime(hsScalar time) const = 0;
	virtual hsScalar TimeGivenVelocity(hsScalar velocity) const = 0;
	virtual hsScalar GetMinDistance();
	virtual hsScalar GetMaxDistance();
	virtual hsScalar GetNormDistance();
};

class plConstAccelEaseCurve : public plATCEaseCurve
{
public:
	plConstAccelEaseCurve();
	plConstAccelEaseCurve(hsScalar minLength, hsScalar maxLength, hsScalar length, 
						  hsScalar startSpeed, hsScalar goalSpeed);

	CLASSNAME_REGISTER( plConstAccelEaseCurve );
	GETINTERFACE_ANY( plConstAccelEaseCurve, plATCEaseCurve );

	virtual plATCEaseCurve *Clone() const;
	virtual void SetLengthOnDistance(hsScalar dist);
	virtual hsScalar PositionGivenTime(hsScalar time) const;
	virtual hsScalar VelocityGivenTime(hsScalar time) const;
	virtual hsScalar TimeGivenVelocity(hsScalar velocity) const;
};

class plSplineEaseCurve : public plATCEaseCurve
{
public:
	plSplineEaseCurve();
	plSplineEaseCurve(hsScalar minLength, hsScalar maxLength, hsScalar length, 
					  hsScalar startSpeed, hsScalar goalSpeed);

	CLASSNAME_REGISTER( plSplineEaseCurve );
	GETINTERFACE_ANY( plSplineEaseCurve, plATCEaseCurve );

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);

	virtual plATCEaseCurve *Clone() const;
	virtual void RecalcToSpeed(hsScalar startSpeed, hsScalar goalSpeed, hsBool preserveRate = false); 
	virtual void SetLengthOnDistance(hsScalar dist);
	virtual hsScalar PositionGivenTime(hsScalar time) const;
	virtual hsScalar VelocityGivenTime(hsScalar time) const;
	virtual hsScalar TimeGivenVelocity(hsScalar velocity) const;

	hsScalar fCoef[4];
};

class plATCState
{
public:
	plATCState() : fEaseCurve(nil) {}
	~plATCState() { delete fEaseCurve; }

	void Read(hsStream *s, hsResMgr *mgr);
	void Write(hsStream *s, hsResMgr *mgr);

	double fStartWorldTime;
	hsScalar fStartAnimTime;

	UInt8 fFlags;
	hsScalar fBegin;
	hsScalar fEnd;
	hsScalar fLoopBegin;
	hsScalar fLoopEnd;
	hsScalar fSpeed;
	hsScalar fWrapTime;
	plATCEaseCurve *fEaseCurve;
};


#endif // plAnimTimeConvert_inc
