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
#include "plFacingConditionalObject.h"
#include "plgDispatch.h"
#include "../../NucleusLib/pnModifier/plLogicModBase.h"
#include "../plMessage/plActivatorMsg.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plFakeOutMsg.h"
#include "../pnNetCommon/plNetApp.h"

plFacingConditionalObject::plFacingConditionalObject() :
fTolerance(-1.0f),
fDirectional(false)
{
	SetSatisfied(true);
}

hsBool plFacingConditionalObject::MsgReceive(plMessage* msg)
{
	return plConditionalObject::MsgReceive(msg);
}


void plFacingConditionalObject::Write(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Write(stream, mgr);
	stream->WriteSwap(fTolerance);
	stream->WriteBool(fDirectional);
}

void plFacingConditionalObject::Read(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Read(stream, mgr);
	stream->ReadSwap(&fTolerance);
	fDirectional = stream->ReadBool();
}

hsBool plFacingConditionalObject::Verify(plMessage* msg)
{
	plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pActivateMsg && pActivateMsg->fHitterObj)
	{	
		plSceneObject* pPlayer = plSceneObject::ConvertNoRef(pActivateMsg->fHitterObj->ObjectIsLoaded());
		if (pPlayer)
		{
			hsVector3 playerView = pPlayer->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
			hsVector3 ourView;
			if (fDirectional)
				ourView = fLogicMod->GetTarget()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
			else
			{
				hsVector3 v(fLogicMod->GetTarget()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pPlayer->GetCoordinateInterface()->GetLocalToWorld().GetTranslate());
				ourView = v;
				ourView.Normalize();
			}
			hsScalar dot = playerView * ourView;
			if (dot >= fTolerance)
			{
				fLogicMod->GetNotify()->AddFacingEvent( pActivateMsg->fHitterObj, fLogicMod->GetTarget()->GetKey(), dot, true);
				return true;			
			}
			else
			{
				return false;
			}
		}
	}
	plFakeOutMsg* pFakeMsg = plFakeOutMsg::ConvertNoRef(msg);
	if (pFakeMsg && plNetClientApp::GetInstance()->GetLocalPlayerKey())
	{
		// sanity check
		if (!fLogicMod->GetTarget()->GetCoordinateInterface())
			return false;

		plSceneObject* pPlayer = plSceneObject::ConvertNoRef(plNetClientApp::GetInstance()->GetLocalPlayerKey()->ObjectIsLoaded());
		if (pPlayer)
		{
			hsVector3 playerView = pPlayer->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
			hsVector3 ourView;
			if (fDirectional)
				ourView = fLogicMod->GetTarget()->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
			else
			{
				hsVector3 v(fLogicMod->GetTarget()->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pPlayer->GetCoordinateInterface()->GetLocalToWorld().GetTranslate());
				ourView = v;
				ourView.fZ = playerView.fZ;
				ourView.Normalize();
			}
			hsScalar dot = playerView * ourView;
			if (dot >= fTolerance)
			{
				return true;			
			}
			else
			{
				if (!IsToggle())
				{
					fLogicMod->GetNotify()->AddFacingEvent( pPlayer->GetKey(), fLogicMod->GetTarget()->GetKey(), dot, false);
					fLogicMod->RequestUnTrigger();
					return false;
				}
			}
		}
	}
	return false;
}
