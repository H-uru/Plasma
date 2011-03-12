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

//#include "hsTypes.h"
//#include "hsGeometry3.h"
//#include "plgDispatch.h"
//#include "../pnSceneObject/plDrawInterface.h"
//#include "../pnSceneObject/plCoordinateInterface.h"
//#include "hsBounds.h"
#include "plSpawnModifier.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plAvatar/plAvatarMgr.h"
//#include "../pnMessage/plTimeMsg.h"
//#include "../pnKeyedObject/plKey.h"

//#include "../plMessage/plSpawnRequestMsg.h"
//#include "../plMessage/plSpawnModMsg.h"

//hsBool plSpawnModifier::MsgReceive(plMessage* msg)
//{
//	plSpawnRequestMsg* pSpawnMsg = plSpawnRequestMsg::ConvertNoRef(msg);
//	if (pSpawnMsg)
//	{
//		fTargets.GetCount();
//		for (int i=0; i < GetNumTargets(); i++)
//		{
//			plSpawnModMsg* pMsg = NEW(plSpawnModMsg);
//			pMsg->AddReceiver( pSpawnMsg->GetSender() );
//			//pMsg->fPos= GetTarget(i)->GetDrawInterface()->GetWorldBounds().GetCenter();
//			pMsg->fPos= GetTarget(i)->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
//			pMsg->fObj = GetTarget(i)->GetKey()->GetUoid();
//			plgDispatch::MsgSend( pMsg );
//		}
//		return true;
//	}
//	return plMultiModifier::MsgReceive(msg);
//}

void plSpawnModifier::AddTarget(plSceneObject* so)
{
	plMultiModifier::AddTarget(so);
	plAvatarMgr::GetInstance()->AddSpawnPoint(this);
//	plgDispatch::Dispatch()->RegisterForExactType(plSpawnRequestMsg::Index(), GetKey());
}

void plSpawnModifier::RemoveTarget(plSceneObject* so)
{
	plMultiModifier::RemoveTarget(so);
	hsAssert(fTargets.GetCount() == 0, "Spawn modifier has multiple targets. Matt.");

	plAvatarMgr::GetInstance()->RemoveSpawnPoint(this);
}

void plSpawnModifier::Read(hsStream *stream, hsResMgr *mgr)
{
	plMultiModifier::Read(stream, mgr);
}

void plSpawnModifier::Write(hsStream *stream, hsResMgr *mgr)
{
	plMultiModifier::Write(stream, mgr);
}
