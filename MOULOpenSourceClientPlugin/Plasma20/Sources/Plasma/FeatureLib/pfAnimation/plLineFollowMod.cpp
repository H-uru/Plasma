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
#include "plLineFollowMod.h"
#include "plStereizer.h"
#include "../plInterp/plAnimPath.h"
#include "hsResMgr.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "plgDispatch.h"
#include "../plMessage/plListenerMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "hsBounds.h"
#include "plPipeline.h"
#include "hsFastMath.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"
#include "hsTimer.h"

plLineFollowMod::plLineFollowMod()
:	fPath(nil),
	fPathParent(nil),
	fRefObj(nil),
	fFollowMode(kFollowListener),
	fFollowFlags(kNone),
	fOffset(0),
	fOffsetClamp(0),
	fSpeedClamp(0)
{
	fSearchPos.Set(0,0,0);
}

plLineFollowMod::~plLineFollowMod()
{
	delete fPath;
}

void plLineFollowMod::SetSpeedClamp(hsScalar fps)
{
	fSpeedClamp = fps;
	if( fSpeedClamp > 0 )
	{
		fFollowFlags |= kSpeedClamp;
	}
	else
	{
		fFollowFlags &= ~kSpeedClamp;
	}

}

void plLineFollowMod::SetOffsetFeet(hsScalar f)
{
	fOffset = f;
	if( fOffset != 0 )
	{
		fFollowFlags &= ~kOffsetAng;
		fFollowFlags |= kOffsetFeet;
	}
	else
	{
		fFollowFlags &= ~kOffset;
	}
}

void plLineFollowMod::SetForceToLine(hsBool on)
{
	if( on )
		fFollowFlags |= kForceToLine;
	else
		fFollowFlags &= ~kForceToLine;
}

void plLineFollowMod::SetOffsetDegrees(hsScalar f)
{
	fOffset = hsScalarDegToRad(f);
	if( fOffset != 0 )
	{
		fFollowFlags &= ~kOffsetFeet;
		fFollowFlags |= kOffsetAng;
		fTanOffset = tanf(f);
	}
	else
	{
		fFollowFlags &= ~kOffset;
	}
}

void plLineFollowMod::SetOffsetClamp(hsScalar f)
{
	fOffsetClamp = f;
	if( fOffsetClamp > 0 )
	{
		fFollowFlags |= kOffsetClamp;
	}
	else
	{
		fFollowFlags &= ~kOffsetClamp;
	}
}

void plLineFollowMod::SetPath(plAnimPath* path)
{
	delete fPath;
	fPath = path;
}

void plLineFollowMod::Read(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Read(stream, mgr);

	fPath = plAnimPath::ConvertNoRef(mgr->ReadCreatable(stream));

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefParent), plRefFlags::kPassiveRef);

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefObject), plRefFlags::kPassiveRef);

	int n = stream->ReadSwap32();
	while(n--)
	{
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefStereizer), plRefFlags::kPassiveRef);
	}

	UInt32 f = stream->ReadSwap32();
	SetFollowMode(FollowMode(f & 0xffff));

	fFollowFlags = (UInt16)((f >> 16) & 0xffff);

	if( fFollowFlags & kOffset )
	{
		fOffset = stream->ReadSwapScalar();
	}
	if( fFollowFlags & kOffsetAng )
	{
		fTanOffset = tanf(fOffset);
	}
	if( fFollowFlags & kOffsetClamp )
	{
		fOffsetClamp = stream->ReadSwapScalar();
	}
	if( fFollowFlags & kSpeedClamp )
	{
		fSpeedClamp = stream->ReadSwapScalar();
	}
}

void plLineFollowMod::Write(hsStream* stream, hsResMgr* mgr)
{
	plMultiModifier::Write(stream, mgr);

	mgr->WriteCreatable(stream, fPath);

	mgr->WriteKey(stream, fPathParent); 

	mgr->WriteKey(stream, fRefObj); 

	stream->WriteSwap32(fStereizers.GetCount());
	int i;
	for( i = 0; i < fStereizers.GetCount(); i++ )
		mgr->WriteKey(stream, fStereizers[i]->GetKey());

	UInt32 f = UInt32(fFollowMode) | (UInt32(fFollowFlags) << 16);
	stream->WriteSwap32(f);

	if( fFollowFlags & kOffset )
		stream->WriteSwapScalar(fOffset);
	if( fFollowFlags & kOffsetClamp )
		stream->WriteSwapScalar(fOffsetClamp);
	if( fFollowFlags & kSpeedClamp )
		stream->WriteSwapScalar(fSpeedClamp);
}


#include "plProfile.h"
plProfile_CreateTimer("LineFollow", "RenderSetup", LineFollow);

hsBool plLineFollowMod::MsgReceive(plMessage* msg)
{
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
		{
			plSceneObject* obj = plSceneObject::ConvertNoRef(refMsg->GetRef());
			if( kRefParent == refMsg->fType )
				fPathParent = obj;
			else if( kRefObject == refMsg->fType )
				fRefObj = obj;
			else if( kRefStereizer == refMsg->fType )
			{
				plStereizer* ster = plStereizer::ConvertNoRef(refMsg->GetRef());
				int idx = fStereizers.Find(ster);
				if( idx == fStereizers.kMissingIndex )
					fStereizers.Append(ster);
			}
		}
		else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
		{
			if( kRefParent == refMsg->fType )
				fPathParent = nil;
			else if( kRefObject == refMsg->fType )
				fRefObj = nil;
			else if( kRefStereizer == refMsg->fType )
			{
				plStereizer* ster = (plStereizer*)(refMsg->GetRef());
				int idx = fStereizers.Find(ster);
				if( idx != fStereizers.kMissingIndex )
					fStereizers.Remove(idx);
			}
		}
		return true;
	}
	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
	if( rend )
	{
		plProfile_BeginLap(LineFollow, this->GetKey()->GetUoid().GetObjectName());
		hsPoint3 oldPos = fSearchPos;
		fSearchPos = rend->Pipeline()->GetViewPositionWorld();
		ICheckForPop(oldPos, fSearchPos);
		plProfile_EndLap(LineFollow, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}
	plListenerMsg* list = plListenerMsg::ConvertNoRef(msg);
	if( list )
	{
		hsPoint3 oldPos = fSearchPos;
		fSearchPos = list->GetPosition();
		ICheckForPop(oldPos, fSearchPos);

		ISetupStereizers(list);
		return true;
	}
	plPlayerPageMsg* pPMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pPMsg)
	{
		if (pPMsg->fPlayer == plNetClientMgr::GetInstance()->GetLocalPlayerKey() && !pPMsg->fUnload)
		{
			fRefObj = (plSceneObject*)pPMsg->fPlayer->GetObjectPtr();
		}
		return true;
	}

	return plMultiModifier::MsgReceive(msg);
}

void plLineFollowMod::SetFollowMode(FollowMode f) 
{ 
	IUnRegister();
	fFollowMode = f; 
	IRegister();

	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plLineFollowMod::IUnRegister()
{
	switch( fFollowMode )
	{
	case kFollowObject:
		break;
	case kFollowListener:
		plgDispatch::Dispatch()->UnRegisterForExactType(plListenerMsg::Index(), GetKey());
		break;
	case kFollowCamera:
		plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
		break;
	case kFollowLocalAvatar:
		plgDispatch::Dispatch()->UnRegisterForExactType(plPlayerPageMsg::Index(), GetKey());
		break;

	}
}

void plLineFollowMod::IRegister()
{
	switch( fFollowMode )
	{
	case kFollowObject:
		break;
	case kFollowListener:
		plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey());
		break;
	case kFollowCamera:
		plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
		break;
	case kFollowLocalAvatar:
		{	
			if (plNetClientApp::GetInstance() && plNetClientApp::GetInstance()->GetLocalPlayer())
				fRefObj = ((plSceneObject*)plNetClientApp::GetInstance()->GetLocalPlayer());
			plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
			break;
		}
	}
}

hsBool plLineFollowMod::IEval(double secs, hsScalar del, UInt32 dirty)
{
	if( !fPath )
		return false;

	ISetPathTransform();

	if( !IGetSearchPos() )
		return false;

	hsMatrix44 tgtXfm;
	IGetTargetTransform(fSearchPos, tgtXfm);

	if( fFollowFlags & kOffset )
		IOffsetTargetTransform(tgtXfm);

	int i;
	for( i = 0; i < GetNumTargets(); i++ )
	{
		ISetTargetTransform(i, tgtXfm);
	}
	return true;
}

hsBool plLineFollowMod::IOffsetTargetTransform(hsMatrix44& tgtXfm)
{
	hsPoint3 tgtPos = tgtXfm.GetTranslate();

	hsVector3 tgt2src(&fSearchPos, &tgtPos);
	hsScalar t2sLen = tgt2src.Magnitude();
	hsFastMath::NormalizeAppr(tgt2src);

	hsVector3 out;
	out.Set(-tgt2src.fY, tgt2src.fX, 0); // (0,0,1) X (tgt2src)

	if( fFollowFlags & kOffsetAng )
	{
		hsScalar del = t2sLen * fTanOffset;
		if( fFollowFlags & kOffsetClamp ) 
		{
			if( del > fOffsetClamp )
				del = fOffsetClamp;
			else if( del < -fOffsetClamp )
				del = -fOffsetClamp;
		}
		out *= del;
	}
	else if( fFollowFlags & kOffsetFeet )
	{
		out *= fOffset;
	}
	else
		out.Set(0,0,0);

	if( fFollowFlags & kForceToLine )
	{
		hsPoint3 newSearch = tgtPos;
		newSearch += out;
		IGetTargetTransform(newSearch, tgtXfm);
	}
	else
	{
		tgtXfm.fMap[0][3] += out[0];
		tgtXfm.fMap[1][3] += out[1];
		tgtXfm.fMap[2][3] += out[2];
	}

	return true;
}

hsBool plLineFollowMod::IGetTargetTransform(hsPoint3& searchPos, hsMatrix44& tgtXfm)
{
	hsScalar t = fPath->GetExtremePoint(searchPos);
	if( fFollowFlags & kFullMatrix )
	{
		fPath->SetCurTime(t, plAnimPath::kNone);
		fPath->GetMatrix44(&tgtXfm);
	}
	else
	{
		fPath->SetCurTime(t, plAnimPath::kCalcPosOnly);
		hsPoint3 pos;
		fPath->GetPosition(&pos);
		tgtXfm.MakeTranslateMat((hsVector3*)&pos);
	}

	return true;
}

void plLineFollowMod::ISetPathTransform()
{
	if( fPathParent && fPathParent->GetCoordinateInterface() )
	{
		hsMatrix44 l2w = fPathParent->GetCoordinateInterface()->GetLocalToWorld();
		hsMatrix44 w2l = fPathParent->GetCoordinateInterface()->GetWorldToLocal();
		
		fPath->SetTransform(l2w, w2l);
	}
}

void plLineFollowMod::ICheckForPop(const hsPoint3& oldPos, const hsPoint3& newPos)
{
	hsVector3 del(&oldPos, &newPos);

	hsScalar elapsed = hsTimer::GetDelSysSeconds();
	hsScalar speedSq = 0.f;
	if (elapsed > 0.f)
		speedSq = del.MagnitudeSquared() / elapsed;

	const hsScalar kMaxSpeedSq = 30.f * 30.f; // (feet per sec)^2
	if( speedSq > kMaxSpeedSq )
		fFollowFlags |= kSearchPosPop;
	else
		fFollowFlags &= ~kSearchPosPop;
}

hsBool plLineFollowMod::IGetSearchPos()
{
	hsPoint3 oldPos = fSearchPos;
	if( kFollowObject == fFollowMode )
	{
		if( !fRefObj )
			return false;

		if( fRefObj->GetCoordinateInterface() )
		{
			fSearchPos = fRefObj->GetCoordinateInterface()->GetWorldPos();
			ICheckForPop(oldPos, fSearchPos);
			return true;
		}
		else if( fRefObj->GetDrawInterface() )
		{
			fSearchPos = fRefObj->GetDrawInterface()->GetWorldBounds().GetCenter();
			ICheckForPop(oldPos, fSearchPos);
			return true;
		}
		return false;
	}
	else 
	if (fFollowMode == kFollowLocalAvatar)
	{	
		if (!fRefObj)
			return false;
		if( fRefObj->GetCoordinateInterface() )
		{
			fSearchPos = fRefObj->GetCoordinateInterface()->GetWorldPos();
			ICheckForPop(oldPos, fSearchPos);
			return true;
		}
		else if( fRefObj->GetDrawInterface() )
		{
			fSearchPos = fRefObj->GetDrawInterface()->GetWorldBounds().GetCenter();
			ICheckForPop(oldPos, fSearchPos);
			return true;
		}
		return false;
	}
	return true;
}

hsMatrix44 plLineFollowMod::IInterpMatrices(const hsMatrix44& m0, const hsMatrix44& m1, hsScalar parm)
{
	hsMatrix44 retVal;
	int i, j;
	for( i = 0; i < 3; i++ )
	{
		for( j = 0; j < 4; j++ )
		{
			retVal.fMap[i][j] = m0.fMap[i][j] * (1.f - parm) + m1.fMap[i][j] * parm;
		}
	}
	retVal.fMap[3][0] = retVal.fMap[3][1] = retVal.fMap[3][2] = 0;
	retVal.fMap[3][3] = 1.f;
	retVal.NotIdentity();
	return retVal;
}

hsMatrix44 plLineFollowMod::ISpeedClamp(plCoordinateInterface* ci, const hsMatrix44& unclTgtXfm)
{
	// If our search position has popped, or delsysseconds is zero, just return as is.
	if( (fFollowFlags & kSearchPosPop) || !(hsTimer::GetDelSysSeconds() > 0) )
		return unclTgtXfm;

	const hsMatrix44 currL2W = ci->GetLocalToWorld();
	const hsPoint3 oldPos = currL2W.GetTranslate();
	const hsPoint3 newPos = unclTgtXfm.GetTranslate();
	const hsVector3 del(&newPos, &oldPos);
	hsScalar elapsed = hsTimer::GetDelSysSeconds();
	hsScalar speed = 0.f;
	if (elapsed > 0.f)
		speed = del.Magnitude() / elapsed;

	if( speed > fSpeedClamp )
	{
		hsScalar parm = fSpeedClamp / speed;

		hsMatrix44 clTgtXfm = IInterpMatrices(currL2W, unclTgtXfm, parm);

		return clTgtXfm;
	}

	return unclTgtXfm;
}

void plLineFollowMod::ISetTargetTransform(int iTarg, const hsMatrix44& unclTgtXfm)
{
	plCoordinateInterface* ci = IGetTargetCoordinateInterface(iTarg);
	if( ci )
	{
		hsMatrix44 tgtXfm = fFollowFlags & kSpeedClamp ? ISpeedClamp(ci, unclTgtXfm) : unclTgtXfm;
		if( fFollowFlags & kFullMatrix )
		{
			// This branch currently never gets taken. If it ever does,
			// we should probably optimize out this GetInverse() (depending
			// on how often it gets taken).
			const hsMatrix44& l2w = tgtXfm;
			hsMatrix44 w2l;
			l2w.GetInverse(&w2l);

			ci->SetTransform(l2w, w2l);
		}
		else
		{
			hsMatrix44 l2w = ci->GetLocalToWorld();
			hsMatrix44 w2l = ci->GetWorldToLocal();

			hsPoint3 pos = tgtXfm.GetTranslate();
			hsPoint3 oldPos = l2w.GetTranslate();
			
			l2w.SetTranslate(&pos);

			hsMatrix44 xlate;
			xlate.Reset();
			xlate.SetTranslate(&oldPos);
			w2l = w2l * xlate;
			xlate.SetTranslate(&-pos);
			w2l = w2l * xlate;

			ci->SetTransform(l2w, w2l);
		}
		hsPoint3 newPos = tgtXfm.GetTranslate();
		int i;
		for( i = 0; i < fStereizers.GetCount(); i++ )
		{
			if( fStereizers[i] )
			{
				fStereizers[i]->SetWorldInitPos(newPos);
				fStereizers[i]->Stereize();
			}
		}
	}
}

void plLineFollowMod::ISetupStereizers(const plListenerMsg* listMsg)
{
	int i;
	for( i = 0; i < fStereizers.GetCount(); i++ )
	{
		if( fStereizers[i] )
			fStereizers[i]->SetFromListenerMsg(listMsg);
	}
}

void plLineFollowMod::AddTarget(plSceneObject* so)
{
	plMultiModifier::AddTarget(so);

	if( so )
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plLineFollowMod::RemoveTarget(plSceneObject* so)
{
	plMultiModifier::RemoveTarget(so);
}

void plLineFollowMod::AddStereizer(const plKey& key)
{
	hsgResMgr::ResMgr()->SendRef(plKey(key), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefStereizer), plRefFlags::kPassiveRef);
}

void plLineFollowMod::RemoveStereizer(const plKey& key)
{
	hsgResMgr::ResMgr()->SendRef(plKey(key), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRemove, 0, kRefStereizer), plRefFlags::kPassiveRef);
}

// derived version of this class for rail cameras
// the difference is the rail camera just calculates 
// the desired position but does not move the target to
// it.

plRailCameraMod::plRailCameraMod() :
fCurrentTime(0.0f),
fTargetTime(0.0f),
fFarthest(false)
{
	plLineFollowMod::plLineFollowMod();
	fGoal.Set(0,0,0);
}

plRailCameraMod::~plRailCameraMod()
{
}

hsBool	plRailCameraMod::IGetTargetTransform(hsPoint3& searchPos, hsMatrix44& tgtXfm)
{
	if (fPath->GetFarthest())
	{	
		fFarthest = true;
		fPath->SetFarthest(false);
	}
	fTargetTime = fPath->GetExtremePoint(searchPos);
	fPath->SetCurTime(fTargetTime, plAnimPath::kCalcPosOnly);
	hsPoint3 pos;
	fPath->GetPosition(&pos);
	tgtXfm.MakeTranslateMat((hsVector3*)&pos);
	
	return true;
}

hsPoint3 plRailCameraMod::GetGoal(double secs, hsScalar speed)
{
	hsScalar delTime;
	int dir;
	if (fTargetTime == fCurrentTime)
		return fGoal;

	if (fTargetTime > fCurrentTime)
	{	
		dir = 1;
		delTime = fTargetTime - fCurrentTime;
	}
	else
	{	
		dir = -1;
		delTime = fCurrentTime - fTargetTime;
	}
	if (fPath->GetWrap() && delTime > fPath->GetLength() * 0.5f)
		dir *= -1;
	
	if (delTime <= (secs * speed))
		fCurrentTime = fTargetTime;
	else
		fCurrentTime += (hsScalar)((secs * speed) * dir);

	if (fPath->GetWrap())
	{
		if (fCurrentTime > fPath->GetLength())
			fCurrentTime = (fCurrentTime - fPath->GetLength());
		else
		if (fCurrentTime < 0.0f)
			fCurrentTime = fPath->GetLength() - fCurrentTime;
	}
	if (fFarthest)
		fPath->SetCurTime((fPath->GetLength() - fCurrentTime), plAnimPath::kCalcPosOnly);
	else
		fPath->SetCurTime(fCurrentTime, plAnimPath::kCalcPosOnly);
	
	fPath->GetPosition(&fGoal);
	
	fPath->SetCurTime(fTargetTime, plAnimPath::kCalcPosOnly);
	
	return fGoal;
}