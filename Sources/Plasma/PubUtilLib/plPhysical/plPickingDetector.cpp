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
#include "HeadSpin.h"
#include "plPickingDetector.h"
#include "plMessage/plActivatorMsg.h"
#include "plMessage/plPickedMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plFakeOutMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "plgDispatch.h"


bool plPickingDetector::MsgReceive(plMessage* msg)
{
    
    plObjRefMsg* refMsg = plObjRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        if( refMsg->fType == plObjRefMsg::kModifier) 
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                plModifier* mod = plModifier::ConvertNoRef(refMsg->GetRef());
                SetRemote(mod);
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                SetRemote(nil);
            }
        }
        return true;
    }
    
    plPickedMsg* pPMsg = plPickedMsg::ConvertNoRef(msg);
    if (pPMsg)
    {
        for (int i = 0; i < fReceivers.Count(); i++)
        {
            plActivatorMsg* pMsg = new plActivatorMsg;
            pMsg->AddReceiver( fReceivers[i] );
            if (pPMsg->fPicked)
                pMsg->SetTriggerType( plActivatorMsg::kPickedTrigger );
            else
                pMsg->SetTriggerType( plActivatorMsg::kUnPickedTrigger );
            // pass on the hit point
            pMsg->fHitPoint = pPMsg->fHitPoint;

            if (fProxyKey)
                pMsg->fPickedObj = fProxyKey;
            else
                pMsg->fPickedObj = GetTarget()->GetKey();

            // assume that since this is something that was PICKED that it was done by the local player.
            plKey locPlayerKey = plNetClientApp::GetInstance()->GetLocalPlayerKey();
            if (locPlayerKey)
                pMsg->fHitterObj = locPlayerKey;

            pMsg->SetSender(GetKey());
            plgDispatch::MsgSend( pMsg );
            hsStatusMessageF("%s sending activate message to %s\n",GetKey()->GetName().c_str(), fReceivers[i]->GetName().c_str());
        }
    }

    if (RemoteMod() && RemoteMod()->MsgReceive(msg))
        return true;

    return plDetectorModifier::MsgReceive(msg);
}

