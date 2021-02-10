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

#include "plLOSDispatch.h"

#include "plgDispatch.h"
#include "plProfile.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plPhysicalControllerCore.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plMessage/plRenderMsg.h"
#include "plModifier/plLogicModifier.h"
#include "plStatusLog/plStatusLog.h"

plProfile_CreateTimer("LineOfSight", "Simulation", LineOfSight);

plLOSDispatch::plLOSDispatch()
    : fDebugDisplay()
{
    RegisterAs(kLOSObject_KEY);
    plgDispatch::Dispatch()->RegisterForExactType(plLOSRequestMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

plLOSDispatch::~plLOSDispatch()
{
    plgDispatch::Dispatch()->UnRegisterForExactType(plLOSRequestMsg::Index(), GetKey());
    plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
}

bool plLOSDispatch::MsgReceive(plMessage* msg)
{
    plLOSRequestMsg* requestMsg = plLOSRequestMsg::ConvertNoRef(msg);
    if (requestMsg) {
        plProfile_BeginTiming(LineOfSight);

        plKey worldKey = requestMsg->fWorldKey;
        if (!worldKey) {
            plArmatureMod* av = plAvatarMgr::GetInstance()->GetLocalAvatar();
            if (av && av->GetController())
                worldKey = av->GetController()->GetSubworld();
        }

        auto hitMsg = std::make_unique<plLOSHitMsg>(GetKey(), requestMsg->GetSender(), requestMsg->fRequestID);
        hitMsg->fNoHit = !IRaycast(requestMsg->fFrom, requestMsg->fTo, worldKey, requestMsg->fRequestType,
                                   requestMsg->GetTestType() == plLOSRequestMsg::kTestClosest,
                                   hitMsg->fObj, hitMsg->fHitPoint, hitMsg->fNormal, hitMsg->fDistance);
        if (!hitMsg->fNoHit) {
            // If we have a cull db, adjust the length of the raycast to be from the
            // original point to the object we hit.  If we find anything from the cull
            // db in there, the cast fails.
            if (requestMsg->GetCullDB() != plSimDefs::kLOSDBNone) {
                plKey cullObj;
                hsPoint3 cullPoint;
                hsVector3 cullNormal;
                float cullDistance;
                bool cullHit = IRaycast(requestMsg->fFrom,
                                        hitMsg->fHitPoint,
                                        worldKey,
                                        requestMsg->fCullDB,
                                        requestMsg->GetTestType() == plLOSRequestMsg::kTestClosest,
                                        cullObj,
                                        cullPoint,
                                        cullNormal,
                                        cullDistance);
                if (cullHit) {
                    fRequests.emplace_back(requestMsg->GetRequestName(), requestMsg->GetRequestID(),
                                           LOSResult::kCull, hitMsg->fObj, cullObj);
                    if (requestMsg->GetReportType() == plLOSRequestMsg::kReportMiss ||
                        requestMsg->GetReportType() == plLOSRequestMsg::kReportHitOrMiss) {
                        hitMsg = std::make_unique<plLOSHitMsg>(GetKey(), requestMsg->GetSender(), requestMsg->fRequestID);
                        hitMsg->fNoHit = true;
                    } else {
                        hitMsg.reset();
                    }
                } else {
                    fRequests.emplace_back(requestMsg->GetRequestName(), requestMsg->GetRequestID(),
                                           LOSResult::kHit, hitMsg->fObj);
                }
            } else {
                fRequests.emplace_back(requestMsg->GetRequestName(), requestMsg->GetRequestID(),
                                       LOSResult::kHit, hitMsg->fObj);
            }

            if (hitMsg && (requestMsg->GetReportType() == plLOSRequestMsg::kReportHit ||
                           requestMsg->GetReportType() == plLOSRequestMsg::kReportHitOrMiss))
                hitMsg.release()->Send();

        } else {
            fRequests.emplace_back(requestMsg->GetRequestName(), requestMsg->GetRequestID(), LOSResult::kMiss);
            if (requestMsg->GetReportType() == plLOSRequestMsg::kReportMiss ||
                requestMsg->GetReportType() == plLOSRequestMsg::kReportHitOrMiss) {
                plLOSHitMsg* missMsg = new plLOSHitMsg(GetKey(), requestMsg->GetSender(), requestMsg->fRequestID);
                missMsg->fNoHit = true;
                missMsg->Send();
            }
        }

        plProfile_EndTiming(LineOfSight);
        return true;
    }

    if (plRenderMsg::ConvertNoRef(msg)) {
        if (!fDebugDisplay) {
            fDebugDisplay = plStatusLogMgr::GetInstance().CreateStatusLog(32, "Line of Sight",
                                                                          plStatusLog::kDontWriteFile |
                                                                          plStatusLog::kDeleteForMe |
                                                                          plStatusLog::kFilledBackground);
        }

        fDebugDisplay->Clear();
        fDebugDisplay->AddLineF("Num LOS Requests: {}", fRequests.size());
        fDebugDisplay->AddLine(plStatusLog::kGreen, "Hits");
        fDebugDisplay->AddLine(plStatusLog::kYellow, "Culled Hits");
        fDebugDisplay->AddLine(plStatusLog::kRed, "Misses");
        fDebugDisplay->AddLine("--------------------");
        fDebugDisplay->AddLine("\n");
        for (const auto& i : fRequests) {
            uint32_t color = plStatusLog::kWhite;
            switch (i.fResult) {
            case LOSResult::kHit:
                color = plStatusLog::kGreen;
                break;
            case LOSResult::kCull:
                color = plStatusLog::kYellow;
                break;
            case LOSResult::kMiss:
                color = plStatusLog::kRed;
                break;
            }

            fDebugDisplay->AddLine(color, i.fName);

            ST::string_stream ss;
            if (i.fHit)
                ss << "Hit: [" << i.fHit->GetName() << "] ";
            if (i.fCull)
                ss << "Cull: [" << i.fCull->GetName() << "] ";
            if (ss.size())
                fDebugDisplay->AddLineF(color, "    {}", ss.to_string());
        }

        fRequests.clear();
        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}

bool plLOSDispatch::ITestHit(const plSceneObject* so) const
{
    for (size_t i = 0; i < so->GetNumModifiers(); ++i) {
        if (const plLogicModifier* logicmod = plLogicModifier::ConvertNoRef(so->GetModifier(i))) {
            if (logicmod->Disabled())
                return false;
        }
    }
    return true;
}
