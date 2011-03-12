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

#ifndef plAnimPath_inc
#define plAnimPath_inc

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "../plTransform/hsAffineParts.h"
#include "../pnFactory/plCreatable.h"

class plCompoundController;

class plAnimPath : public plCreatable
{
public:
	enum Flags
	{
		kNone			= 0x0,
		kFavorFwdSearch	= 0x1,		// only move fwd on the curve when searching
		kFavorBwdSearch = 0x2,		// only move bwd on the curve when searching
		kCalcPosOnly	= 0x4,		// only compute pos when calling SetCurTime()
		kFarthest		= 0x8,
		kWrap			= 0x10,
		kIncrement		= 0x20,		// find the nearest / farthest point, but increment toward it
	};
protected:
	// The final product info
	hsMatrix44					fXform;
	hsPoint3					fPos;
	hsVector3					fVel;
	hsVector3					fAccel;
	hsScalar					fTime; // presumably seconds

	// The paramters (and options) for this curve.
	UInt32						fAnimPathFlags;		// currently set at runtime only
	hsScalar					fMinDistSq;
	hsScalar					fLength; // presumably seconds

	// Controller stuff only works in local space.
	hsMatrix44					fLocalToWorld;
	hsMatrix44					fWorldToLocal;

	// Bounding sphere available for ignoring out of range
	hsPoint3					fCenter;
	hsScalar					fRadius;

	plCompoundController*		fController;

	hsAffineParts				fParts;

	// These are temps during a search. They're here to avoid recalc.
	mutable hsScalar					fLastTime;
	mutable hsScalar					fLastDistSq;
	mutable hsScalar					fThisTime;
	mutable hsScalar					fThisDistSq;
	mutable hsScalar					fNextTime;
	mutable hsScalar					fNextDistSq;
	mutable hsScalar					fDelTime;
	mutable hsPoint3					fPrevPos, fCurPos;

	void						ICalcBounds();
	hsScalar					ICalcTotalLength();
	hsScalar					IShiftFore(hsPoint3 &pt) const;
	hsScalar					IShiftBack(hsPoint3 &pt) const;
	hsScalar					ISubDivFore(hsPoint3 &pt) const;
	hsScalar					ISubDivBack(hsPoint3 &pt) const;
	void						IInitInterval(hsScalar time, hsScalar delTime, hsPoint3 &pt) const;
	hsScalar					ICheckInterval(hsPoint3 &pt) const;
	hsScalar					IBestTime() const { return fLastDistSq < fThisDistSq 
														? (fLastDistSq < fNextDistSq 
															? fLastTime
															: fNextTime)
														: (fThisDistSq < fNextDistSq
															? fThisTime
															: fNextTime); }

	// Visualization helper
	void IMakeSegment(hsTArray<UInt16>& idx, hsTArray<hsPoint3>& pos,
								  hsPoint3& p1, hsPoint3& p2);
	
	// For computing arclen
	struct ArcLenDeltaInfo
	{
		hsScalar	fT;
		hsScalar	fArcLenDelta;	// arc len distance from prev sample point (array entry)
		ArcLenDeltaInfo(hsScalar t, hsScalar del) : fT(t),fArcLenDelta(del) {}
		ArcLenDeltaInfo() : fT(0),fArcLenDelta(0) {}
	};
	hsTArray<ArcLenDeltaInfo>	fArcLenDeltas;
public:
	plAnimPath();
	virtual ~plAnimPath();

	CLASSNAME_REGISTER( plAnimPath );
	GETINTERFACE_ANY( plAnimPath, plCreatable );
	
    void Reset();

	void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
	const hsMatrix44& GetLocalToWorld() const { return fLocalToWorld; }
	const hsMatrix44& GetWorldToLocal() const { return fWorldToLocal; }

	// Visualization helper
	void MakeDrawList(hsTArray<UInt16>& idx, hsTArray<hsPoint3>& pos);

	void SetAnimPathFlags(UInt32 f) { fAnimPathFlags=f; }
	UInt32 GetAnimPathFlags() const { return fAnimPathFlags; }

	void SetWrap(hsBool on) { if(on)fAnimPathFlags |= kWrap; else fAnimPathFlags &= ~kWrap; }
	hsBool GetWrap() const { return 0 != (fAnimPathFlags & kWrap); }

	void SetFarthest(hsBool on) { if(on)fAnimPathFlags |= kFarthest; else fAnimPathFlags &= ~kFarthest; }
	hsBool GetFarthest() const { return 0 != (fAnimPathFlags & kFarthest); }

	void SetCurTime(hsScalar t, UInt32 calcFlags=0);
	hsScalar GetCurTime() const { return fTime; }

	void SetController(plCompoundController* tmc);
	plCompoundController* GetController() const { return fController; }
	hsScalar GetLength() const { return fLength; } // seconds

	void SetMinDistance(hsScalar d) { fMinDistSq = d*d; }
	hsScalar GetMinDistance() const { return hsSquareRoot(fMinDistSq); }

	hsMatrix44* GetMatrix44(hsMatrix44* xOut) const { *xOut = fXform; return xOut; }
	hsPoint3*	GetPosition(hsPoint3* pOut) const { *pOut = fPos; return pOut; }
	hsVector3*	GetVelocity(hsVector3* vOut) const { *vOut = fVel; return vOut; }
	hsVector3*	GetDirection(hsVector3* dOut) const { dOut->Set(fXform.fMap[0][2], fXform.fMap[1][2], fXform.fMap[2][2]); return dOut; }
	hsVector3*	GetUp(hsVector3* uOut) const { uOut->Set(fXform.fMap[0][1], fXform.fMap[1][1], fXform.fMap[2][1]); return uOut; }
	hsVector3*	GetAcceleration(hsVector3* aOut) const { *aOut = fAccel; return aOut; }
	
	hsBool OutOfRange(hsPoint3 &pt, hsScalar range) const;
	const hsAffineParts* Parts() const { return &fParts; }
	void InitParts(const hsAffineParts& p) { fParts = p; }

	hsScalar GetExtremePoint(hsPoint3 &worldPt) const; // Exhaustive search
	hsScalar GetExtremePoint(hsScalar lastTime, hsScalar delTime, hsPoint3 &worldPt) const; // Incremental search

	// for arclen usage
	void ComputeArcLenDeltas(Int32 numSamples=256);
	hsScalar GetLookAheadTime(hsScalar startTime, hsScalar arcLength, hsBool bwd, Int32* startSrchIdx);

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);
};

#endif plAnimPath_inc
