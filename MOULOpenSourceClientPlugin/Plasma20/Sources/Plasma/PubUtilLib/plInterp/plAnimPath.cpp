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

#include "hsTypes.h"
#include "plAnimPath.h"
#include "plController.h"
#include "hsFastMath.h"
#include "hsResMgr.h"

const hsScalar kSmallDelTime = 1.e-2f;	
const hsScalar kInvSmallDelTime = 1.f / kSmallDelTime;
const hsScalar kTerminateDelTime = 1.e-3f;
const hsScalar kTerminateDelDistSq = .1f;

plAnimPath::plAnimPath()
: fController(nil), fLength(0), fMinDistSq(0), 
	fAnimPathFlags(0)
{
	fLocalToWorld.Reset();
	fWorldToLocal.Reset();
	Reset();
}

plAnimPath::~plAnimPath()
{
	delete fController;
}

void plAnimPath::Reset()
{
	SetCurTime(0);
}

void plAnimPath::SetCurTime(hsScalar t, UInt32 calcFlags)
{
	fTime = t;
	if( !fController )
	{
		fPos.Set(0,0,0);
		fXform.Reset();
		fVel.Set(0,0,0);
		fAccel.Set(0,0,0);
		return;
	}

	hsScalar t0, t1, t2;

	if( t < kSmallDelTime )
	{
		t0 = t;
		t1 = t + kSmallDelTime;
		t2 = t + 2 * kSmallDelTime;
	}
	else if( t > fLength - kSmallDelTime )
	{
		t0 = t - 2 * kSmallDelTime;
		t1 = t - kSmallDelTime;
		t2 = t;
	}
	else
	{
		t0 = t - kSmallDelTime;
		t1 = t;
		t2 = t + kSmallDelTime;
	}


	if (!(calcFlags & kCalcPosOnly))
	{
		hsPoint3 pos[3];

		fController->Interp(t0, &fParts);
		pos[0].Set(fParts.fT.fX, fParts.fT.fY, fParts.fT.fZ);

		fController->Interp(t1, &fParts);
		pos[1].Set(fParts.fT.fX, fParts.fT.fY, fParts.fT.fZ);

		fController->Interp(t2, &fParts);
		pos[2].Set(fParts.fT.fX, fParts.fT.fY, fParts.fT.fZ);

		fVel.Set(pos+1, pos+0);
		fVel *= kInvSmallDelTime;
		fVel = fLocalToWorld * fVel;
		fAccel.Set(&(pos[2] - pos[1]), &(pos[1] - pos[0]));
		fAccel *= kInvSmallDelTime * kInvSmallDelTime;
		fAccel = fLocalToWorld * fAccel;
	}

	fController->Interp(t, &fParts);
	fParts.ComposeMatrix(&fXform);
	fXform = fLocalToWorld * fXform;
	fXform.GetTranslate(&fPos);

}

void plAnimPath::ICalcBounds()
{
	if( !fController )
		return;

	plController* pc = fController->GetPosController();

	int i;
	hsPoint3 pos;
	hsTArray<hsScalar> keyTimes;
	pc->GetKeyTimes(keyTimes);
	fCenter.Set(0,0,0);
	for( i = 0; i < keyTimes.GetCount() ; i++ )
	{
		pc->Interp(keyTimes[i], &pos);
		fCenter += pos;
	}
	fCenter *= hsScalarInvert((hsScalar)keyTimes.GetCount());

	fRadius = 0;
	for( i = 0; i < keyTimes.GetCount(); i++ )
	{
		pc->Interp(keyTimes[i], &pos);
		hsScalar rad = (pos - fCenter).Magnitude();
		if( rad > fRadius )
			fRadius = rad;
	}
}

hsScalar plAnimPath::ICalcTotalLength()
{
	if( !(fController && fController->GetPosController()) )
		return 0;

	fLength = fController->GetPosController()->GetLength();	

	return fLength;
}

void plAnimPath::SetController(plCompoundController* tmc)
{
	hsAssert(tmc, "Bad (nil) controller for AnimPath");
	hsAssert(tmc->GetPosController(), "Bad controller for AnimPath");
	fController = tmc;
	ICalcTotalLength();
	ICalcBounds();
}

void plAnimPath::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	fLocalToWorld = l2w;
	fWorldToLocal = w2l;
}

void plAnimPath::Read(hsStream* stream, hsResMgr* mgr)
{
	fAnimPathFlags=stream->ReadSwap32();

	delete fController;
	fController = plCompoundController::ConvertNoRef(mgr->ReadCreatable(stream));

	ICalcBounds();

	fParts.Read(stream);

	fLocalToWorld.Read(stream);
	fWorldToLocal.Read(stream);

	fLength = stream->ReadSwapScalar();
    fMinDistSq = stream->ReadSwapScalar();
    
    Reset();
}

void plAnimPath::Write(hsStream* stream, hsResMgr* mgr)
{
	stream->WriteSwap32(fAnimPathFlags);

    mgr->WriteCreatable(stream, fController);
    
    fParts.Write(stream);
    
    fLocalToWorld.Write(stream);
	fWorldToLocal.Write(stream);

	stream->WriteSwapScalar(fLength);
	stream->WriteSwapScalar(fMinDistSq);
}

hsBool plAnimPath::OutOfRange(hsPoint3 &worldPt, hsScalar range) const
{
	hsPoint3 pt = fWorldToLocal * worldPt;

	hsScalar radius = (pt - fCenter).Magnitude() - fRadius;
	return( radius > range );
}

hsScalar plAnimPath::GetExtremePoint(hsPoint3 &worldPt) const
{
	if( !fController )
		return 0;

	hsPoint3 pt = fWorldToLocal * worldPt;

	plController *pc = fController->GetPosController();

	hsScalar minDistSq = 1.e33f;
	hsScalar minTime = 0, delTime = 0;
	// start search by using the time of the closest ctrl point
	int i;
	hsTArray<hsScalar> keyTimes;
	pc->GetKeyTimes(keyTimes);
	for( i = 0; i < keyTimes.GetCount(); i++ )
	{
		hsScalar t = keyTimes[i];
		hsPoint3 pos;
		pc->Interp(t, &pos);	// handles easing
		hsScalar distSq = (pt - pos).MagnitudeSquared();
		if( distSq < minDistSq )
		{
			minDistSq = distSq;
			minTime = t;
			if( 0 == i )
				delTime = keyTimes[i+1] - t;
			else if( keyTimes.GetCount() - 1 == i )
				delTime = t - keyTimes[i - 1];
			else
			{
				hsScalar fore = keyTimes[i + 1] - t;
				hsScalar back = t - keyTimes[i - 1];
				delTime = hsMaximum(fore, back);
			}
		}
	}

	return GetExtremePoint(minTime, delTime, worldPt);
}

hsScalar plAnimPath::GetExtremePoint(hsScalar lastTime, hsScalar delTime, hsPoint3 &worldPt) const
{
	if( !fController )
		return 0;

	hsPoint3 pt = fWorldToLocal * worldPt;

	IInitInterval(lastTime, delTime, pt);
	return ICheckInterval(pt);
}

hsScalar plAnimPath::ICheckInterval(hsPoint3 &pt) const
{
	if( fDelTime <= kTerminateDelTime &&
		hsVector3(&fCurPos, &fPrevPos).MagnitudeSquared() < kTerminateDelDistSq)
	{
		return IBestTime();
	}

	if( fThisTime < 0 )
		return 0;

	if( fThisTime > fLength )
		return fLength;

	if( GetFarthest() )
	{
		if( (fLastDistSq > fThisDistSq)&&(fLastDistSq >= fNextDistSq) )
			return IShiftBack(pt);

		if( (fNextDistSq > fThisDistSq)&&(fNextDistSq >= fLastDistSq) )
			return IShiftFore(pt);

		if( (fThisDistSq >= fLastDistSq)&&(fLastDistSq >= fNextDistSq) )
			return ISubDivBack(pt);

		if( (fThisDistSq >= fNextDistSq)&&(fNextDistSq >= fLastDistSq) )
			return ISubDivFore(pt);
	}
	else
	{
		if( (fLastDistSq < fThisDistSq)&&(fLastDistSq <= fNextDistSq) )
			return IShiftBack(pt);

		if( (fNextDistSq < fThisDistSq)&&(fNextDistSq <= fLastDistSq) )
			return IShiftFore(pt);

		if( (fThisDistSq <= fLastDistSq)&&(fLastDistSq <= fNextDistSq) )
			return ISubDivBack(pt);

		if( (fThisDistSq <= fNextDistSq)&&(fNextDistSq <= fLastDistSq) )
			return ISubDivFore(pt);
	}

	hsAssert(false, "Shouldn't have gotten here");
	return 0;
}

void plAnimPath::IInitInterval(hsScalar time, hsScalar delTime, hsPoint3 &pt) const
{
	plController* pc = fController->GetPosController();

	hsPoint3 pos;

	fDelTime = delTime;
	if( fDelTime <= kTerminateDelTime )
		fDelTime = kTerminateDelTime * 2;
	else
	if( fDelTime > fLength * 0.5f )
		fDelTime = fLength * 0.5f;

	fThisTime = time;
	if( fThisTime < fDelTime )
		fThisTime = fDelTime;
	else if( fThisTime > fLength - fDelTime )
		fThisTime = fLength - fDelTime;
	pc->Interp(fThisTime, &pos);
	fPrevPos=fCurPos=pos;
	fThisDistSq = (pos - pt).MagnitudeSquared();
	
	fNextTime = fThisTime + delTime;
	if( fNextTime > fLength )
		fNextTime = fLength;
	if (!(GetAnimPathFlags() & kFavorBwdSearch))
	{
		pc->Interp(fNextTime, &pos);
		fNextDistSq = (pos - pt).MagnitudeSquared();
	}
	else
	{
		fNextDistSq = 1.e33f;
	}

	fLastTime = fThisTime - delTime;
	if( fLastTime < 0 )
		fLastTime = 0;
	if (!(GetAnimPathFlags() & kFavorFwdSearch))
	{
		pc->Interp(fLastTime, &pos);
		fLastDistSq = (pos - pt).MagnitudeSquared();
	}
	else
	{
		fLastDistSq = 1.e33f;
	}

	if( fMinDistSq != 0 )
	{
		fThisDistSq -= fMinDistSq;
		if( fThisDistSq < 0 )
			fThisDistSq = -fThisDistSq;

		fNextDistSq -= fMinDistSq;
		if( fNextDistSq < 0 )
			fNextDistSq = -fNextDistSq;

		fLastDistSq -= fMinDistSq;
		if( fLastDistSq < 0 )
			fLastDistSq = -fLastDistSq;
	}
}

hsScalar plAnimPath::ISubDivBack(hsPoint3 &pt) const
{
	fNextTime = fThisTime;
	fNextDistSq = fThisDistSq;

	fDelTime *= 0.5f;

	fThisTime -= fDelTime;
	if (fThisTime<0)
		fThisTime = GetWrap() ? fLength + fThisTime : 0;

	plController* pc = fController->GetPosController();
	hsPoint3 pos;
	pc->Interp(fThisTime, &pos);
	fThisDistSq = (pos - pt).MagnitudeSquared() - fMinDistSq;
	if( fThisDistSq < 0 )
		fThisDistSq = -fThisDistSq;

	fPrevPos=fCurPos;
	fCurPos=pos;

	return ICheckInterval(pt);
}

hsScalar plAnimPath::ISubDivFore(hsPoint3 &pt) const
{
	fLastTime = fThisTime;
	fLastDistSq = fThisDistSq;

	fDelTime *= 0.5f;

	fThisTime += fDelTime;
	if (fThisTime>fLength)
		fThisTime = GetWrap() ? fThisTime-fLength : fLength;

	plController* pc = fController->GetPosController();
	hsPoint3 pos;
	pc->Interp(fThisTime, &pos);
	fThisDistSq = (pos - pt).MagnitudeSquared() - fMinDistSq;
	if( fThisDistSq < 0 )
		fThisDistSq = -fThisDistSq;

	fPrevPos=fCurPos;
	fCurPos=pos;

	return ICheckInterval(pt);
}

hsScalar plAnimPath::IShiftBack(hsPoint3 &pt) const
{
	if( !GetWrap() && (fLastTime <= 0) )
		return ISubDivBack(pt);

	fNextTime = fThisTime;
	fNextDistSq = fThisDistSq;

	fThisTime = fLastTime;
	fThisDistSq = fLastDistSq;

	fLastTime -= fDelTime;
	if( fLastTime < 0 )
		fLastTime = GetWrap() ? fLength + fLastTime : 0;
	plController* pc = fController->GetPosController();
	hsPoint3 pos;
	pc->Interp(fLastTime, &pos);
	fLastDistSq = (pos - pt).MagnitudeSquared() - fMinDistSq;
	if( fLastDistSq < 0 )
		fLastDistSq = -fLastDistSq;

	fPrevPos=fCurPos;
	fCurPos=pos;

	return ICheckInterval(pt);
}

hsScalar plAnimPath::IShiftFore(hsPoint3 &pt) const
{
	if( !GetWrap() &&(fNextTime >= fLength) )
		return ISubDivFore(pt);

	fLastTime = fThisTime;
	fLastDistSq = fThisDistSq;

	fThisTime = fNextTime;
	fThisDistSq = fNextDistSq;

	fNextTime += fDelTime;
	if( fNextTime > fLength )
		fNextTime = GetWrap() ? fNextTime - fLength : fLength;
	plController* pc = fController->GetPosController();
	hsPoint3 pos;
	pc->Interp(fNextTime, &pos);
	fNextDistSq = (pos - pt).MagnitudeSquared() - fMinDistSq;
	if( fNextDistSq < 0 )
		fNextDistSq = -fNextDistSq;

	fPrevPos=fCurPos;
	fCurPos=pos;

	return ICheckInterval(pt);
}

//
// wireframe debug draw method.  
// doesn't use any fancy subdivision or curvature measure when drawing.
// Changes current time.
//
void plAnimPath::IMakeSegment(hsTArray<UInt16>& idx, hsTArray<hsPoint3>& pos,
							  hsPoint3& p1, hsPoint3& p2)
{
	hsVector3 del(&p2, &p1);
	hsVector3 up;
	up.Set(0,0,1.f);

	const hsScalar kOutLength = 0.25f;

	hsVector3 a = del % up;
	hsScalar magSq = a.MagnitudeSquared();
	if( magSq < 1.e-3f )
	{
		a.Set(kOutLength, 0, 0);
	}
	else
	{
		a *= hsFastMath::InvSqrtAppr(magSq);
		a *= kOutLength;
	}

	hsVector3 b = a % del;
	hsFastMath::Normalize(b);
	b *= kOutLength;

	hsPoint3 p1out, p2out;

	int baseIdx = pos.GetCount();

	pos.Append(p1);
	pos.Append(p2);

	p1out = p1;
	p1out += a;
	p2out = p2;
	p2out += a;

	pos.Append(p1out);
	pos.Append(p2out);

	p1out += -2.f * a;
	p2out += -2.f * a;

	pos.Append(p1out);
	pos.Append(p2out);

	p1out = p1;
	p1out += b;
	p2out = p2;
	p2out += b;

	pos.Append(p1out);
	pos.Append(p2out);

	p1out += -2.f * b;
	p2out += -2.f * b;

	pos.Append(p1out);
	pos.Append(p2out);

	int i;
	for( i = 0; i < 4; i++ )
	{
		int outIdx = baseIdx + 2 + i * 2;
		idx.Append(baseIdx);
		idx.Append(baseIdx + 1);
		idx.Append(baseIdx + outIdx);

		idx.Append(baseIdx + outIdx);
		idx.Append(baseIdx + 1);
		idx.Append(baseIdx + outIdx + 1);
	}
}

void plAnimPath::MakeDrawList(hsTArray<UInt16>& idx, hsTArray<hsPoint3>& pos)
{
	hsMatrix44 resetL2W = GetLocalToWorld();
	hsMatrix44 resetW2L = GetWorldToLocal();

	hsMatrix44 ident;
	ident.Reset();
	SetTransform(ident, ident);

	hsScalar numSegs = fRadius;	// crude estimate of arclength
	if (numSegs>100)
		numSegs=100;
	hsScalar animLen = GetLength();
	hsScalar timeInc = animLen/numSegs;
	hsScalar time=0;
	hsPoint3 p1, p2;

	SetCurTime(0, kCalcPosOnly);
	GetPosition(&p1);

	time += timeInc;
	hsBool quit=false;
	while(! quit && time < animLen+timeInc)
	{
		if (time > animLen)
		{
			time = animLen;
			quit=true;
		}

		SetCurTime(time, kCalcPosOnly);
		GetPosition(&p2);

		IMakeSegment(idx, pos, p1, p2);

		time += timeInc;

		p1 = p2;
	}

	SetTransform(resetL2W, resetW2L);
}

//
// Precompute array of arclen deltas for lookahead ability.
// Changes current time!
//
void plAnimPath::ComputeArcLenDeltas(Int32 numSamples)
{
	if (fArcLenDeltas.GetCount() >= numSamples)
		return;		// already computed enough samples

	// compute arc len deltas
	fArcLenDeltas.Reset();
	fArcLenDeltas.SetCount(numSamples);
	hsScalar animLen = GetLength();
	hsScalar timeInc = animLen/(numSamples-1);	// use num-1 since we'll create the zeroth entry by hand
	hsScalar time=0;
	hsPoint3 p1, p2;

	Int32 cnt=0;

	// prime initial point
	SetCurTime(0, kCalcPosOnly);
	GetPosition(&p1);
	ArcLenDeltaInfo aldi(time, 0);
	fArcLenDeltas[cnt++]=aldi;
	time += timeInc;

	hsBool quit=false;
	while(!quit && time<animLen+timeInc)
	{
		if (time > animLen || cnt+1 == numSamples)
		{
			time = animLen;
			quit=true;
		}

		SetCurTime(time, kCalcPosOnly);
		GetPosition(&p2);

		ArcLenDeltaInfo aldi(time, hsVector3(&p2, &p1).Magnitude());
		fArcLenDeltas[cnt++]=aldi;

		time += timeInc;
		p1 = p2;
	}
	hsAssert(fArcLenDeltas.GetCount()==numSamples, "arcLenArray size wrong?");
	hsAssert(cnt==numSamples, "arcLenArray size wrong?");
}

//
// Returns time of point (at least) arcLength units away from point at startTime.
// Also sets strtSrchIdx for incremental searching.
//
hsScalar plAnimPath::GetLookAheadTime(hsScalar startTime, hsScalar arcLengthIn, hsBool bwd,
									  Int32* startSrchIdx)
{
	if (arcLengthIn==0)
		return startTime;	// default is no look ahead

	if (startTime==GetLength() && !bwd)
		return GetLength();

	if (startTime==0 && bwd)
		return 0;

	hsAssert(startSrchIdx, "nil var for startSrchIdx");

	hsScalar oldTime=fTime;

	ComputeArcLenDeltas();	// precompute first time only

	// save and change time if necessary
	if (fTime!=startTime)
		SetCurTime(startTime, kCalcPosOnly);

	// find nearest (forward) arcLen sample point, use starting srch index provided
	hsBool found=false;
	Int32 i;
	for(i=(*startSrchIdx); i<fArcLenDeltas.GetCount()-1; i++)
	{
		if (fArcLenDeltas[i].fT<=startTime && startTime<fArcLenDeltas[i+1].fT)
		{
			*startSrchIdx=i;
			found=true;
			break;
		}
	}

	if (!found)
	{
		// check if equal to last index
		if (startTime==fArcLenDeltas[fArcLenDeltas.GetCount()-1].fT)
		{
			*startSrchIdx=fArcLenDeltas.GetCount()-1;
			found=true;
		}
		else
		{
			for(i=0; i<*startSrchIdx; i++)
			{
				if (fArcLenDeltas[i].fT<=startTime && startTime<fArcLenDeltas[i+1].fT)
				{
					*startSrchIdx=i;
					found=true;
					break;
				}
			}
		}
	}

	// find distance to nearest arcLen sample point
	Int32 nearestIdx = bwd ? *startSrchIdx : *startSrchIdx+1;
	hsAssert(found, "couldn't find arcLength sample");

	hsPoint3 pos;
	GetPosition(&pos);		// startTime position

	hsPoint3 pos2;
	hsScalar endTime = fArcLenDeltas[nearestIdx].fT; 
	SetCurTime(endTime, kCalcPosOnly);
	GetPosition(&pos2);		// position at nearest sample point

	hsScalar curArcLen = hsVector3(&pos2, &pos).Magnitude();
	hsScalar curTime=0;
	hsBool quit=false;
	hsScalar timeOut = 0;
	Int32 inc = bwd ? -1 : 1;
	// now sum distance deltas until we exceed the desired arcLen
	if (curArcLen<arcLengthIn)
	{
		for(i=bwd ? nearestIdx : nearestIdx+inc; i<fArcLenDeltas.GetCount() && i>=0; i+=inc)
		{
			if (curArcLen+fArcLenDeltas[i].fArcLenDelta>arcLengthIn)
			{
				// gone tooFar
				endTime = fArcLenDeltas[i].fT;
				curTime = fArcLenDeltas[i-1].fT;
				break;
			}
			curArcLen += fArcLenDeltas[i].fArcLenDelta;
		}	
		
		if ( (i==fArcLenDeltas.GetCount() && !bwd) || (i<0 && bwd) )
		{
			quit=true;
			timeOut = bwd ? 0 : GetLength();
		}		
	}
	else
	{
		curArcLen = 0;
		curTime = startTime;
	}

	if (!quit)
	{
		// interp remaining interval

		// 1. compute necessary distToGoal
		hsScalar distToGoal = arcLengthIn-curArcLen;
		hsAssert(distToGoal, "0 length distToGoal?");
		
		// 2. compute % of dist interval which gives distToGoal
		SetCurTime(curTime, kCalcPosOnly);
		GetPosition(&pos);

		SetCurTime(endTime, kCalcPosOnly);
		GetPosition(&pos2);

		hsScalar distInterval = hsVector3(&pos2, &pos).Magnitude();
		hsScalar percent = distToGoal/distInterval;
		hsAssert(percent>=0 && percent<=1, "illegal percent value"); 

		// 3. compute interpolated time value using percent
		if (!bwd)
			timeOut = curTime + (endTime-curTime)*percent;
		else
			timeOut = endTime - (endTime-curTime)*percent;
		hsAssert((timeOut>=curTime && timeOut<=endTime), "illegal interpolated time value");
		// hsAssert(!bwd || (timeOut<=curTime && timeOut>=endTime), "bwd: illegal interpolated time value");
	}

	// restore time
	if (fTime != oldTime)
		SetCurTime(oldTime);

	hsAssert(bwd || (timeOut>startTime && timeOut<=GetLength()), "fwd: illegal look ahead time");
	hsAssert(!bwd || (timeOut<startTime && timeOut>=0), "bwd: illegal look ahead time");

	return timeOut;
}

