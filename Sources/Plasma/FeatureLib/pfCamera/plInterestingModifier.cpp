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
#include "hsGeometry3.h"
#include "plgDispatch.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../plMessage/plInterestingPing.h"
#include "hsBounds.h"
#include "plInterestingModifier.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnKeyedObject/plKey.h"


hsScalar plInterestingModifier::fInterestRadius		= 100.0f;
hsScalar plInterestingModifier::fInterestWeight		= 1.0f;


hsBool plInterestingModifier::IEval(double secs, hsScalar del, UInt32 dirty)
{
	for (int i=0; i < GetNumTargets(); i++)
	{
		if( GetTarget(i) && GetTarget(i)->GetDrawInterface() )
		{
			const hsBounds3Ext& targBnd = GetTarget(i)->GetDrawInterface()->GetWorldBounds();
			if( targBnd.GetType() == kBoundsNormal )
			{
				plInterestingModMsg* pMsg = TRACKED_NEW plInterestingModMsg;
				pMsg->fPos= GetTarget(i)->GetDrawInterface()->GetWorldBounds().GetCenter();
				pMsg->fSize = GetTarget(i)->GetDrawInterface()->GetWorldBounds().GetMaxDim();
				pMsg->fRadius = fInterestRadius;
				pMsg->fWeight = fInterestWeight;
				pMsg->fObj = GetTarget(i)->GetKey();
				pMsg->fType = GetType();
				pMsg->SetBCastFlag( plMessage::kPropagateToModifiers );
				plgDispatch::MsgSend( pMsg );
			}
		}
	}
	return true;
}

void plInterestingModifier::AddTarget(plSceneObject* so)
{
	fTarget = so;
	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

