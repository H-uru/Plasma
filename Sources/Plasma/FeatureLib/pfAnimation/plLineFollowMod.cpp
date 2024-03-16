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

#include "plLineFollowMod.h"
#include "plStereizer.h"

#include "hsBounds.h"
#include "hsFastMath.h"
#include "plgDispatch.h"
#include "plPipeline.h"
#include "plProfile.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plInterp/plAnimPath.h"
#include "plMessage/plListenerMsg.h"
#include "plMessage/plRenderMsg.h"

plProfile_CreateTimer("LineFollow", "RenderSetup", LineFollow);

plLineFollowMod::~plLineFollowMod()
{
    delete fPath;
}

void plLineFollowMod::SetSpeedClamp(float fps)
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

void plLineFollowMod::SetOffsetFeet(float f)
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

void plLineFollowMod::SetForceToLine(bool on)
{
    if( on )
        fFollowFlags |= kForceToLine;
    else
        fFollowFlags &= ~kForceToLine;
}

void plLineFollowMod::SetOffsetDegrees(float f)
{
    fOffset = hsDegreesToRadians(f);
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

void plLineFollowMod::SetOffsetClamp(float f)
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

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefParent), plRefFlags::kPassiveRef);

    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefObject), plRefFlags::kPassiveRef);

    int n = stream->ReadLE32();
    while(n--)
    {
        mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefStereizer), plRefFlags::kPassiveRef);
    }

    uint32_t f = stream->ReadLE32();
    SetFollowMode(FollowMode(f & 0xffff));

    fFollowFlags = (uint16_t)((f >> 16) & 0xffff);

    if( fFollowFlags & kOffset )
    {
        fOffset = stream->ReadLEFloat();
    }
    if( fFollowFlags & kOffsetAng )
    {
        fTanOffset = tanf(fOffset);
    }
    if( fFollowFlags & kOffsetClamp )
    {
        fOffsetClamp = stream->ReadLEFloat();
    }
    if( fFollowFlags & kSpeedClamp )
    {
        fSpeedClamp = stream->ReadLEFloat();
    }
}

void plLineFollowMod::Write(hsStream* stream, hsResMgr* mgr)
{
    plMultiModifier::Write(stream, mgr);

    mgr->WriteCreatable(stream, fPath);

    mgr->WriteKey(stream, fPathParent); 

    mgr->WriteKey(stream, fRefObj); 

    stream->WriteLE32((uint32_t)fStereizers.size());
    for (plStereizer* ster : fStereizers)
        mgr->WriteKey(stream, ster->GetKey());

    uint32_t f = uint32_t(fFollowMode) | (uint32_t(fFollowFlags) << 16);
    stream->WriteLE32(f);

    if( fFollowFlags & kOffset )
        stream->WriteLEFloat(fOffset);
    if( fFollowFlags & kOffsetClamp )
        stream->WriteLEFloat(fOffsetClamp);
    if( fFollowFlags & kSpeedClamp )
        stream->WriteLEFloat(fSpeedClamp);
}

bool plLineFollowMod::MsgReceive(plMessage* msg)
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
                auto idx = std::find(fStereizers.cbegin(), fStereizers.cend(), ster);
                if (idx == fStereizers.cend())
                    fStereizers.emplace_back(ster);
            }
        }
        else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
        {
            if( kRefParent == refMsg->fType )
                fPathParent = nullptr;
            else if( kRefObject == refMsg->fType )
                fRefObj = nullptr;
            else if( kRefStereizer == refMsg->fType )
            {
                plStereizer* ster = (plStereizer*)(refMsg->GetRef());
                auto idx = std::find(fStereizers.cbegin(), fStereizers.cend(), ster);
                if (idx != fStereizers.end())
                    fStereizers.erase(idx);
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
        if (pPMsg->fPlayer == plNetClientApp::GetInstance()->GetLocalPlayerKey() && !pPMsg->fUnload)
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

bool plLineFollowMod::IEval(double secs, float del, uint32_t dirty)
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

    for (size_t i = 0; i < GetNumTargets(); i++)
    {
        ISetTargetTransform(i, tgtXfm);
    }
    return true;
}

bool plLineFollowMod::IOffsetTargetTransform(hsMatrix44& tgtXfm)
{
    hsPoint3 tgtPos = tgtXfm.GetTranslate();

    hsVector3 tgt2src(&fSearchPos, &tgtPos);
    float t2sLen = tgt2src.Magnitude();
    hsFastMath::NormalizeAppr(tgt2src);

    hsVector3 out(-tgt2src.fY, tgt2src.fX, 0.f); // (0,0,1) X (tgt2src)

    if( fFollowFlags & kOffsetAng )
    {
        float del = t2sLen * fTanOffset;
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
        out.Set(0.f, 0.f, 0.f);

    if( fFollowFlags & kForceToLine )
    {
        hsPoint3 newSearch = tgtPos + out;
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

bool plLineFollowMod::IGetTargetTransform(hsPoint3& searchPos, hsMatrix44& tgtXfm)
{
    float t = fPath->GetExtremePoint(searchPos);
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

    float elapsed = hsTimer::GetDelSysSeconds();
    float speedSq = 0.f;
    if (elapsed > 0.f)
        speedSq = del.MagnitudeSquared() / elapsed;

    const float kMaxSpeedSq = 30.f * 30.f; // (feet per sec)^2
    if( speedSq > kMaxSpeedSq )
        fFollowFlags |= kSearchPosPop;
    else
        fFollowFlags &= ~kSearchPosPop;
}

bool plLineFollowMod::IGetSearchPos()
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

hsMatrix44 plLineFollowMod::IInterpMatrices(const hsMatrix44& m0, const hsMatrix44& m1, float parm)
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
    float elapsed = hsTimer::GetDelSysSeconds();
    float speed = 0.f;
    if (elapsed > 0.f)
        speed = del.Magnitude() / elapsed;

    if( speed > fSpeedClamp )
    {
        float parm = fSpeedClamp / speed;

        hsMatrix44 clTgtXfm = IInterpMatrices(currL2W, unclTgtXfm, parm);

        return clTgtXfm;
    }

    return unclTgtXfm;
}

void plLineFollowMod::ISetTargetTransform(size_t iTarg, const hsMatrix44& unclTgtXfm)
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
            hsPoint3 neg = -pos;
            xlate.SetTranslate(&neg);
            w2l = w2l * xlate;

            ci->SetTransform(l2w, w2l);
        }
        hsPoint3 newPos = tgtXfm.GetTranslate();
        for (plStereizer* ster : fStereizers)
        {
            if (ster)
            {
                ster->SetWorldInitPos(newPos);
                ster->Stereize();
            }
        }
    }
}

void plLineFollowMod::ISetupStereizers(const plListenerMsg* listMsg)
{
    for (plStereizer* ster : fStereizers)
    {
        if (ster)
            ster->SetFromListenerMsg(listMsg);
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
    hsgResMgr::ResMgr()->SendRef(plKey(key), new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefStereizer), plRefFlags::kPassiveRef);
}

void plLineFollowMod::RemoveStereizer(const plKey& key)
{
    hsgResMgr::ResMgr()->SendRef(plKey(key), new plGenRefMsg(GetKey(), plRefMsg::kOnRemove, 0, kRefStereizer), plRefFlags::kPassiveRef);
}

// derived version of this class for rail cameras
// the difference is the rail camera just calculates 
// the desired position but does not move the target to
// it.

plRailCameraMod::plRailCameraMod() :
plLineFollowMod(),
fCurrentTime(0.0f),
fTargetTime(0.0f),
fFarthest(false)
{
    fGoal.Set(0,0,0);
}

plRailCameraMod::~plRailCameraMod()
{
}

bool  plRailCameraMod::IGetTargetTransform(hsPoint3& searchPos, hsMatrix44& tgtXfm)
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

hsPoint3 plRailCameraMod::GetGoal(double secs, float speed)
{
    float delTime;
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
        fCurrentTime += (float)((secs * speed) * dir);

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
