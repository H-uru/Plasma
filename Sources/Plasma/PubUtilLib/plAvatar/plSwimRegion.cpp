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

#include "plSwimRegion.h"

#include "plArmatureMod.h"
#include "plPhysicalControllerCore.h"

#include <cmath>

#include "hsGeometry3.h"
#include "hsResMgr.h"

#include "pnMessage/plRefMsg.h"

void plSwimRegionInterface::Read(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Read(s, mgr);

    fDownBuoyancy = s->ReadLEFloat();
    fUpBuoyancy = s->ReadLEFloat();
    fMaxUpwardVel = s->ReadLEFloat();
}

void plSwimRegionInterface::Write(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Write(s, mgr);

    s->WriteLEFloat(fDownBuoyancy);
    s->WriteLEFloat(fUpBuoyancy);
    s->WriteLEFloat(fMaxUpwardVel);
}

void plSwimRegionInterface::GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, float &angularResult, float elapsed)
{
    linearResult.Set(0.f, 0.f, 0.f);
    angularResult = 0.f;
}

/////////////////////////////////////////////////////////////////////////

plSwimCircularCurrentRegion::plSwimCircularCurrentRegion() : 
    fCurrentSO(),
    fRotation(),
    fPullNearDistSq(1.f),
    fPullNearVel(0.f),
    fPullFarDistSq(1.f),
    fPullFarVel(0.f)
{
}

void plSwimCircularCurrentRegion::Read(hsStream* stream, hsResMgr* mgr)
{
    plSwimRegionInterface::Read(stream, mgr);
    
    fRotation = stream->ReadLEFloat();
    fPullNearDistSq = stream->ReadLEFloat();
    fPullNearVel = stream->ReadLEFloat();
    fPullFarDistSq = stream->ReadLEFloat();
    fPullFarVel = stream->ReadLEFloat();
    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // currentSO      
}

void plSwimCircularCurrentRegion::Write(hsStream* stream, hsResMgr* mgr)
{
    plSwimRegionInterface::Write(stream, mgr);
    
    stream->WriteLEFloat(fRotation);
    stream->WriteLEFloat(fPullNearDistSq);
    stream->WriteLEFloat(fPullNearVel);
    stream->WriteLEFloat(fPullFarDistSq);
    stream->WriteLEFloat(fPullFarVel);
    mgr->WriteKey(stream, fCurrentSO);
}

bool plSwimCircularCurrentRegion::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
        if (so)
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
                fCurrentSO = so;
            else if (refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove))
                fCurrentSO = nullptr;
            
            return true;
        }
    }

    return plSwimRegionInterface::MsgReceive(msg);
}

void plSwimCircularCurrentRegion::GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, float &angularResult, float elapsed)
{
    if (elapsed <= 0.f || fCurrentSO == nullptr || GetProperty(kDisable))
    {
        linearResult.Set(0.f, 0.f, 0.f);
        angularResult = 0.f;
        return;
    }

    hsPoint3 center = fCurrentSO->GetLocalToWorld().GetTranslate();

    plKey worldKey = physical->GetSubworld();
    if (worldKey)
    {
        plSceneObject* so = plSceneObject::ConvertNoRef(worldKey->ObjectIsLoaded());
        center = so->GetWorldToLocal() * center;
    }

    center.fZ = 0.f; // Just doing 2D

    hsPoint3 pos;
    physical->GetPositionSim(pos);
    
    hsVector3 pos2Center(center.fX - pos.fX, center.fY - pos.fY, 0.f);
    float pullVel;
    float distSq = pos2Center.MagnitudeSquared();
    if (distSq < .5)
    {
        // Don't want to pull us too close to the center, or we
        // get this annoying jitter.
        pullVel = 0.f;
    }
    else if (distSq <= fPullNearDistSq)
        pullVel = fPullNearVel;
    else if (distSq >= fPullFarDistSq)
        pullVel = fPullFarVel;
    else
        pullVel = fPullNearVel + (fPullFarVel - fPullNearVel) * (distSq - fPullNearDistSq) / (fPullFarDistSq - fPullNearDistSq);

    hsVector3 pull = pos2Center;
    pull.Normalize();
    linearResult.Set(pull.fY, -pull.fX, pull.fZ);
    
    pull *= pullVel;
    linearResult *= fRotation;
    linearResult += pull;

    hsVector3 v1 = linearResult * elapsed - pos2Center;
    hsVector3 v2 = -pos2Center;
    float invCos = v1.InnerProduct(v2) / v1.Magnitude() / v2.Magnitude();
    if (invCos > 1)
        invCos = 1;
    if (invCos < -1)
        invCos = -1;
    angularResult = acos(invCos) / elapsed;

//  hsAssert(real_finite(linearResult.fX) &&
//           real_finite(linearResult.fY) &&
//           real_finite(linearResult.fZ) &&
//           real_finite(angularResult), "Bad water current computation."); 
}   

/////////////////////////////////////////////////////////////////////////////////////

plSwimStraightCurrentRegion::plSwimStraightCurrentRegion() : 
    fCurrentSO(),
    fNearDist(1.f),
    fNearVel(0.f),
    fFarDist(1.f),
    fFarVel(0.f)
{
}

void plSwimStraightCurrentRegion::Read(hsStream* stream, hsResMgr* mgr)
{
    plSwimRegionInterface::Read(stream, mgr);
    
    fNearDist = stream->ReadLEFloat();
    fNearVel = stream->ReadLEFloat();
    fFarDist = stream->ReadLEFloat();
    fFarVel = stream->ReadLEFloat();
    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // currentSO      
}

void plSwimStraightCurrentRegion::Write(hsStream* stream, hsResMgr* mgr)
{
    plSwimRegionInterface::Write(stream, mgr);
    
    stream->WriteLEFloat(fNearDist);
    stream->WriteLEFloat(fNearVel);
    stream->WriteLEFloat(fFarDist);
    stream->WriteLEFloat(fFarVel);
    mgr->WriteKey(stream, fCurrentSO);
}

bool plSwimStraightCurrentRegion::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
        if (so)
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
                fCurrentSO = so;
            else if (refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove))
                fCurrentSO = nullptr;
            
            return true;
        }
    }

    return plSwimRegionInterface::MsgReceive(msg);
}

void plSwimStraightCurrentRegion::GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, float &angularResult, float elapsed)
{
    angularResult = 0.f;

    if (elapsed <= 0.f || GetProperty(kDisable))
    {
        linearResult.Set(0.f, 0.f, 0.f);
        return;
    }

    hsPoint3 pos;
    hsVector3 current = fCurrentSO->GetLocalToWorld() * hsVector3(0.f, 1.f, 0.f);
    hsPoint3 center = fCurrentSO->GetLocalToWorld().GetTranslate();;
    physical->GetPositionSim(pos);

    plKey worldKey = physical->GetSubworld();
    if (worldKey)
    {
        plSceneObject* so = plSceneObject::ConvertNoRef(worldKey->ObjectIsLoaded());
        hsMatrix44 w2l = so->GetWorldToLocal();
        center = w2l * center;
        current = w2l * current;
    }

    hsVector3 pos2Center(center.fX - pos.fX, center.fY - pos.fY, 0.f);
    float dist = current.InnerProduct(pos - center);
    float pullVel;
    
    if (dist <= fNearDist)
        pullVel = fNearVel;
    else if (dist >= fFarDist)
        pullVel = fFarVel;
    else
        pullVel = fNearVel + (fFarVel - fNearVel) * (dist - fNearDist) / (fFarDist - fNearDist);

    linearResult = current * pullVel;
}   
