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
#include "plFollowMod.h"
#include "plgDispatch.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plMessage/plListenerMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plRefMsg.h"
#include "hsResMgr.h"
#include "plPipeline.h"

plFollowMod::plFollowMod()
:	fLeader(nil), fMode(kPosition), fLeaderType(kLocalPlayer), fLeaderSet(false)
{
}

plFollowMod::~plFollowMod()
{
}

#include "plProfile.h"
plProfile_CreateTimer("FollowMod", "RenderSetup", FollowMod);

hsBool plFollowMod::MsgReceive(plMessage* msg)
{
	plRenderMsg* rend = plRenderMsg::ConvertNoRef(msg);
	if( rend )
	{
		plProfile_BeginLap(FollowMod, this->GetKey()->GetUoid().GetObjectName());
		fLeaderL2W = rend->Pipeline()->GetCameraToWorld();
		fLeaderW2L = rend->Pipeline()->GetWorldToCamera();
		fLeaderSet = true;
		plProfile_EndLap(FollowMod, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}
	plListenerMsg* list = plListenerMsg::ConvertNoRef(msg);
	if( list )
	{
		hsVector3 pos;
		pos.Set(list->GetPosition().fX, list->GetPosition().fY, list->GetPosition().fZ);
		fLeaderL2W.MakeTranslateMat(&pos);
		fLeaderW2L.MakeTranslateMat(&-pos);
		fLeaderSet = true;

		return true;
	}

	plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
	if( ref )
	{
		switch( ref->fType )
		{
		case kRefLeader:

			if( ref->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fLeader = plSceneObject::ConvertNoRef(ref->GetRef());
			else if( ref->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
				fLeader = nil;
			return true;
		default:
			hsAssert(false, "Unknown ref type to FollowMod");
			break;
		}
	}

	return plSingleModifier::MsgReceive(msg);
}

hsBool plFollowMod::ICheckLeader()
{
	switch( fLeaderType )
	{
	case kLocalPlayer:
		{
			plSceneObject* player = plSceneObject::ConvertNoRef(plNetClientApp::GetInstance()->GetLocalPlayer());
			if( player )
			{
				fLeaderL2W = player->GetLocalToWorld();
				fLeaderW2L = player->GetWorldToLocal();
				fLeaderSet = true;
			}
			else
				fLeaderSet = false;
		}
		break;
	case kObject:
		if( fLeader )
		{
			fLeaderL2W = fLeader->GetLocalToWorld();
			fLeaderW2L = fLeader->GetWorldToLocal();
			fLeaderSet = true;
		}
		else
			fLeaderSet = false;
		break;
	case kCamera:
		break;
	case kListener:
		break;
	}
	return fLeaderSet;
}

void plFollowMod::IMoveTarget()
{
	if( fMode == kFullTransform )
	{
		GetTarget()->SetTransform(fLeaderL2W, fLeaderW2L);
		return;
	}

	hsMatrix44 l2w = GetTarget()->GetLocalToWorld();
	hsMatrix44 w2l = GetTarget()->GetWorldToLocal();

	if( fMode & kRotate )
	{
		int i, j;
		for( i = 0; i < 3; i++ )
		{
			for( j = 0; j < 3; j++ )
			{
				l2w.fMap[i][j] = fLeaderL2W.fMap[i][j];
				w2l.fMap[i][j] = fLeaderW2L.fMap[i][j];
			}
		}
	}

	if( fMode & kPosition )
	{
		hsMatrix44 invMove;
		invMove.Reset();

		hsPoint3 newPos = fLeaderL2W.GetTranslate();
		hsPoint3 newInvPos = fLeaderW2L.GetTranslate();

		hsPoint3 oldPos = l2w.GetTranslate();

		// l2w = newPosMat * -oldPosMat * l2w
		// so w2l = w2l * inv-oldPosMat * invNewPosMat 
		if( fMode & kPositionX )
		{
			l2w.fMap[0][3] = newPos.fX;
			invMove.fMap[0][3] = oldPos.fX - newPos.fX;
		}

		if( fMode & kPositionY )
		{
			l2w.fMap[1][3] = newPos.fY;
			invMove.fMap[1][3] = oldPos.fY - newPos.fY;
		}

		if( fMode & kPositionZ )
		{
			l2w.fMap[2][3] = newPos.fZ;
			invMove.fMap[2][3] = oldPos.fZ - newPos.fZ;
		}

		invMove.NotIdentity();

		// InvMove must happen after rotation.
		w2l = w2l * invMove;

	}

	l2w.NotIdentity();
	w2l.NotIdentity();

#ifdef HS_DEBUGGING
	//MFHORSE hackola
	hsMatrix44 inv;
	l2w.GetInverse(&inv);
#endif // HS_DEBUGGING

	GetTarget()->SetTransform(l2w, w2l);
}

hsBool plFollowMod::IEval(double secs, hsScalar del, UInt32 dirty)
{
	if( ICheckLeader() )
		IMoveTarget();
	return true;
}

void plFollowMod::SetTarget(plSceneObject* so) 
{ 
	plSingleModifier::SetTarget(so);
	if( fTarget )
		Activate();
	else
		Deactivate();
}

void plFollowMod::Activate()
{
	switch( fLeaderType )
	{
	case kLocalPlayer:
		break;
	case kObject:
		break;
	case kCamera:
		plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
		break;
	case kListener:
		plgDispatch::Dispatch()->RegisterForExactType(plListenerMsg::Index(), GetKey());
		break;
	}
	if( fTarget )
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void plFollowMod::Deactivate()
{
	switch( fLeaderType )
	{
	case kLocalPlayer:
		if( fTarget )
			plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
		break;
	case kObject:
		if( fTarget )
			plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
		break;
	case kCamera:
		plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
		break;
	case kListener:
		plgDispatch::Dispatch()->UnRegisterForExactType(plListenerMsg::Index(), GetKey());
		break;
	}
}

void plFollowMod::Read(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Read(stream, mgr);

	fLeaderType = FollowLeaderType(stream->ReadByte());
	fMode = stream->ReadByte();

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefLeader), plRefFlags::kActiveRef);

	// If active?
	Activate();
}

void plFollowMod::Write(hsStream* stream, hsResMgr* mgr)
{
	plSingleModifier::Write(stream, mgr);

	stream->WriteByte(fLeaderType);
	stream->WriteByte(fMode);

	mgr->WriteKey(stream, fLeader);
}

