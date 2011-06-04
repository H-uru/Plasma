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
#include "plViewFaceModifier.h"
#include "plgDispatch.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "hsFastMath.h"
#include "plPipeline.h"

#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plListenerMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plAvatar/plAvBrainHuman.h"
#include "../plAvatar/plArmatureMod.h"

plViewFaceModifier::plViewFaceModifier()
:	fFacePoint(0,0,0),
	fLastDirY(0,1.f,0),
	fScale(1.f,1.f,1.f),
	fOffset(0,0,0)
{
	fOrigLocalToParent.Reset();
	fOrigParentToLocal.Reset();

	SetFlag(kFaceCam); // default
}

plViewFaceModifier::~plViewFaceModifier()
{
}

void plViewFaceModifier::SetOrigTransform(const hsMatrix44& l2p, const hsMatrix44& p2l)
{
	fOrigLocalToParent = l2p;
	fOrigParentToLocal = p2l;
}

void plViewFaceModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);

	fScale.Read(s);

	fOrigLocalToParent.Read(s);
	fOrigParentToLocal.Read(s);

	if( HasFlag(kFaceObj) )
		mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefFaceObj), plRefFlags::kPassiveRef);

	fOffset.Read(s);

	if( HasFlag(kMaxBounds) )
		fMaxBounds.Read(s);
}

void plViewFaceModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Write(s, mgr);

	fScale.Write(s);

	fOrigLocalToParent.Write(s);
	fOrigParentToLocal.Write(s);

	if( HasFlag(kFaceObj) )
		mgr->WriteKey(s, fFaceObj); 

	fOffset.Write(s);

	if( HasFlag(kMaxBounds) )
		fMaxBounds.Write(s);
}

void plViewFaceModifier::SetMaxBounds(const hsBounds3Ext& bnd)
{
	SetFlag(kMaxBounds);
	fMaxBounds = bnd;
}

void plViewFaceModifier::SetTarget(plSceneObject* so)
{
	plSingleModifier::SetTarget(so);

	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
	if( HasFlag(kFaceList) )
		plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey());
	if( HasFlag(kFacePlay) )
		plgDispatch::Dispatch()->RegisterForExactType(plArmatureUpdateMsg::Index(), GetKey());
}

hsBool plViewFaceModifier::IEval(double secs, hsScalar del, UInt32 dirty)
{
	return false;
}

hsBool plViewFaceModifier::IFacePoint(plPipeline* pipe, const hsPoint3& at)
{
#if 1 // BOUNDSTEST
	extern int mfCurrentTest;
	if( mfCurrentTest != 101 )
	if( HasFlag(kMaxBounds) )
	{
		if( !pipe->TestVisibleWorld(fMaxBounds) )
			return false;
	}
#endif // BOUNDSTEST

	if( !(GetTarget() && GetTarget()->GetCoordinateInterface()) )
		return false;
		
	hsMatrix44 worldToLocal = fOrigParentToLocal;	// parentToLocal
	if( GetTarget()->GetCoordinateInterface()->GetParent() && GetTarget()->GetCoordinateInterface()->GetParent() )
	{
		hsMatrix44 m;
		worldToLocal = worldToLocal *  GetTarget()->GetCoordinateInterface()->GetParent()->GetWorldToLocal();
	}
	
	hsPoint3 localAt = worldToLocal * at;
	hsScalar len = localAt.MagnitudeSquared();
	if( len <= 0 )
		return false;
	len = -hsFastMath::InvSqrtAppr(len);
	
	hsVector3 dirX, dirY, dirZ;
	dirZ.Set(localAt.fX * len, localAt.fY * len, localAt.fZ * len);
	
	if( HasFlag(kPivotFace) )
	{
		dirY.Set(0.f, 0.f, 1.f);
		dirX = dirY % dirZ;
		dirX = hsFastMath::NormalizeAppr(dirX);
		dirY = dirZ % dirX;
	}
	else if( HasFlag(kPivotFavorY) )
	{
		dirY.Set(0.f, 1.f, 0.f);
		dirX = dirY % dirZ;
		dirX = hsFastMath::NormalizeAppr(dirX);
		dirY = dirZ % dirX;
	}
	else if( HasFlag(kPivotY) )
	{
		dirY.Set(0.f, 1.f, 0.f);
		dirX = dirY % dirZ;
		dirX = hsFastMath::NormalizeAppr(dirX);
		dirZ = dirX % dirY;
	}
	else if( HasFlag(kPivotTumble) )
	{
		dirY = fLastDirY;
		dirX = dirY % dirZ;
		dirX = hsFastMath::NormalizeAppr(dirX);
		dirY = dirZ % dirX;
		fLastDirY = dirY;
	}
	else
	{
		hsAssert(false, "I've no idea what you're getting at here in ViewFace land");
	}

	hsMatrix44 x;
	hsMatrix44 xInv;

	xInv.fMap[0][0] = x.fMap[0][0] = dirX[0];
	xInv.fMap[1][0] = x.fMap[0][1] = dirY[0];
	xInv.fMap[2][0] = x.fMap[0][2] = dirZ[0];
	xInv.fMap[3][0] = x.fMap[0][3] = 0;
	
	xInv.fMap[0][1] = x.fMap[1][0] = dirX[1];
	xInv.fMap[1][1] = x.fMap[1][1] = dirY[1];
	xInv.fMap[2][1] = x.fMap[1][2] = dirZ[1];
	xInv.fMap[3][1] = x.fMap[1][3] = 0;
	
	xInv.fMap[0][2] = x.fMap[2][0] = dirX[2];
	xInv.fMap[1][2] = x.fMap[2][1] = dirY[2];
	xInv.fMap[2][2] = x.fMap[2][2] = dirZ[2];
	xInv.fMap[3][2] = x.fMap[2][3] = 0;
	
	x.fMap[3][0] = x.fMap[3][1] = x.fMap[3][2] = 0;
	xInv.fMap[0][3] = xInv.fMap[1][3] = xInv.fMap[2][3] = 0;
	
	xInv.fMap[3][3] = x.fMap[3][3] = hsScalar1;
	
	x.NotIdentity();
	xInv.NotIdentity();

	if( HasFlag(kScale) )
	{
		x.fMap[0][0] *= fScale.fX;
		x.fMap[0][1] *= fScale.fX;
		x.fMap[0][2] *= fScale.fX;

		x.fMap[1][0] *= fScale.fY;
		x.fMap[1][1] *= fScale.fY;
		x.fMap[1][2] *= fScale.fY;

		x.fMap[2][0] *= fScale.fZ;
		x.fMap[2][1] *= fScale.fZ;
		x.fMap[2][2] *= fScale.fZ;

		hsScalar inv = 1.f / fScale.fX;
		xInv.fMap[0][0] *= inv;
		xInv.fMap[1][0] *= inv;
		xInv.fMap[2][0] *= inv;

		inv = 1.f / fScale.fY;
		xInv.fMap[0][1] *= inv;
		xInv.fMap[1][1] *= inv;
		xInv.fMap[2][1] *= inv;

		inv = 1.f / fScale.fZ;
		xInv.fMap[0][2] *= inv;
		xInv.fMap[1][2] *= inv;
		xInv.fMap[2][2] *= inv;
	}

	hsMatrix44 l2p = fOrigLocalToParent * x;
	hsMatrix44 p2l = xInv * fOrigParentToLocal;

	if( l2p != IGetTargetCoordinateInterface(0)->GetLocalToParent() ) // TERRORDAN
	{
		IGetTargetCoordinateInterface(0)->SetLocalToParent(l2p, p2l);
		IGetTargetCoordinateInterface(0)->FlushTransform(false);
	}

	return true;
}



#include "plProfile.h"
plProfile_CreateTimer("ViewFacing", "RenderSetup", ViewFace);

hsBool plViewFaceModifier::MsgReceive(plMessage* msg)
{
	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);

	if( rend )
	{
		plProfile_BeginLap(ViewFace, this->GetKey()->GetUoid().GetObjectName());

		if( HasFlag(kFaceCam) )
		{
			fFacePoint = rend->Pipeline()->GetViewPositionWorld();
			if( HasFlag(kOffset) )
			{
				if( HasFlag(kOffsetLocal) )
				{
					fFacePoint += rend->Pipeline()->GetViewAcrossWorld() * fOffset.fX;
					fFacePoint += rend->Pipeline()->GetViewUpWorld() * fOffset.fY;
					fFacePoint += rend->Pipeline()->GetViewDirWorld() * fOffset.fZ;
				}
				else
				{
					fFacePoint += fOffset;
				}
			}
		}
		else
		if( HasFlag(kFaceObj) )
		{
			if( !fFaceObj )
				return true;
			fFacePoint = fFaceObj->GetLocalToWorld().GetTranslate();
			if( HasFlag(kOffset) )
			{
				if( HasFlag(kOffsetLocal) )
					fFacePoint += fFaceObj->GetLocalToWorld() * fOffset;
				else
					fFacePoint += fOffset;
			}
		}

		IFacePoint(rend->Pipeline(), fFacePoint);
		
		plProfile_EndLap(ViewFace, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}
	plArmatureUpdateMsg* armMsg = plArmatureUpdateMsg::ConvertNoRef(msg);
	if( armMsg && armMsg->IsLocal() )
	{
		const plSceneObject* head = armMsg->fArmature->FindBone(plAvBrainHuman::Head);
		if( head )
		{
			fFacePoint = head->GetLocalToWorld().GetTranslate();
			if( HasFlag(kOffset) )
			{
				if( HasFlag(kOffsetLocal) )
					fFacePoint += head->GetLocalToWorld() * fOffset;
				else
					fFacePoint += fOffset;
			}
		}

		return true;
	}
	plListenerMsg* list = plListenerMsg::ConvertNoRef(msg);
	if( list )
	{
		fFacePoint = list->GetPosition();

		if( HasFlag(kOffset) )
		{
			if( HasFlag(kOffsetLocal) )
			{
				fFacePoint += (list->GetDirection() % list->GetUp()) * fOffset.fX;
				fFacePoint += list->GetDirection() * fOffset.fY;
				fFacePoint += list->GetUp() * fOffset.fZ;
			}
			else
			{
				fFacePoint += fOffset;
			}
		}

		return true;
	}
	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			IOnReceive(refMsg);
		else
			IOnRemove(refMsg);
		return true;
	}
	return plSingleModifier::MsgReceive(msg);
}

void plViewFaceModifier::IOnReceive(plGenRefMsg* refMsg)
{
	switch(refMsg->fType)
	{
	case kRefFaceObj:
		fFaceObj = plSceneObject::ConvertNoRef(refMsg->GetRef());
		break;
	}
}

void plViewFaceModifier::IOnRemove(plGenRefMsg* refMsg)
{
	switch(refMsg->fType)
	{
	case kRefFaceObj:
		fFaceObj = nil;
		break;
	}
}

void plViewFaceModifier::ISetObject(plKey soKey)
{
	hsgResMgr::ResMgr()->SendRef(soKey, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnRequest, 0, kRefFaceObj), plRefFlags::kPassiveRef);
}

void plViewFaceModifier::SetFollowMode(FollowMode m, plKey soKey)
{
	ClearFlag(kFaceCam);
	ClearFlag(kFaceList);
	ClearFlag(kFacePlay);
	ClearFlag(kFaceObj);
	
	switch(m)
	{
	case kFollowCamera:
		SetFlag(kFaceCam);
		break;
	case kFollowListener:
		SetFlag(kFaceList);
		break;
	case kFollowPlayer:
		SetFlag(kFacePlay);
		break;
	case kFollowObject:
		SetFlag(kFaceObj);
		ISetObject(soKey);
		break;
	default:
		hsAssert(false, "Unknown follow mode");
		SetFlag(kFaceCam);
		break;
	}
}

plViewFaceModifier::FollowMode plViewFaceModifier::GetFollowMode() const
{
	if( HasFlag(kFaceCam) )
		return kFollowCamera;
	if( HasFlag(kFaceList) )
		return kFollowListener;
	if( HasFlag(kFacePlay) )
		return kFollowPlayer;
	if( HasFlag(kFaceObj) )
		return kFollowObject;

	hsAssert(false, "Have no follow mode");
	return kFollowCamera;

}

void plViewFaceModifier::SetOffset(const hsVector3& off, hsBool local)
{
	fOffset = off;
	if( local )
		SetFlag(kOffsetLocal);
	else
		ClearFlag(kOffsetLocal);
}
