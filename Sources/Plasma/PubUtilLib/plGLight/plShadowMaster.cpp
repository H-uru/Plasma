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
#include "plShadowMaster.h"
#include "plShadowSlave.h"

#include "plLightInfo.h"
#include "plShadowCaster.h"

#include "plIntersect/plVolumeIsect.h"
#include "plMessage/plShadowCastMsg.h"
#include "plMessage/plRenderMsg.h"

#include "plDrawable/plDrawableSpans.h"

#include "plScene/plVisMgr.h"

#include "hsBounds.h"
#include "plgDispatch.h"
#include "plPipeline.h"
#include "hsFastMath.h"

#include "plTweak.h"

uint32_t plShadowMaster::fGlobalMaxSize = 512;
float plShadowMaster::fGlobalMaxDist = 160.f; // PERSPTEST
// float plShadowMaster::fGlobalMaxDist = 100000.f; // PERSPTEST
float plShadowMaster::fGlobalVisParm = 1.f;

void plShadowMaster::SetGlobalShadowQuality(float s) 
{ 
    if( s < 0 )
        s = 0;
    else if( s > 1.f )
        s = 1.f;
    fGlobalVisParm = s;
}

void plShadowMaster::SetGlobalMaxSize(uint32_t s) 
{ 
    const uint32_t kMaxMaxGlobalSize = 512;
    const uint32_t kMinMaxGlobalSize = 32;

    // Make sure it's a power of two.
    if( ((s-1) & ~s) != (s-1) )
    {
        int i;
        for( i = 31; i >= 0; i-- )
        {
            if( (1 << i) & s )
                break;
        }
        s = 1 << i;
    }

    if( s > kMaxMaxGlobalSize )
        s = kMaxMaxGlobalSize;
    if( s < kMinMaxGlobalSize )
        s = kMinMaxGlobalSize;

    fGlobalMaxSize = s; 
}

plShadowMaster::plShadowMaster()
:   fAttenDist(0),
    fMaxDist(0),
    fMinDist(0),
    fMaxSize(256),
    fMinSize(256),
    fPower(1.f),
    fLightInfo()
{
}

plShadowMaster::~plShadowMaster()
{
    Deactivate();
}

void plShadowMaster::Read(hsStream* stream, hsResMgr* mgr)
{
    plObjInterface::Read(stream, mgr);

    fAttenDist = stream->ReadLEFloat();

    fMaxDist = stream->ReadLEFloat();
    fMinDist = stream->ReadLEFloat();

    fMaxSize = stream->ReadLE32();
    fMinSize = stream->ReadLE32();

    fPower = stream->ReadLEFloat();

    Activate();
}

void plShadowMaster::Write(hsStream* stream, hsResMgr* mgr)
{
    plObjInterface::Write(stream, mgr);

    stream->WriteLEFloat(fAttenDist);

    stream->WriteLEFloat(fMaxDist);
    stream->WriteLEFloat(fMinDist);

    stream->WriteLE32(fMaxSize);
    stream->WriteLE32(fMinSize);

    stream->WriteLEFloat(fPower);
}

void plShadowMaster::Activate() const
{
    plgDispatch::Dispatch()->RegisterForExactType(plShadowCastMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plShadowMaster::Deactivate() const
{
    plgDispatch::Dispatch()->UnRegisterForExactType(plShadowCastMsg::Index(), GetKey());
    plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
}

void plShadowMaster::SetMaxDist(float f)
{
    fMaxDist = f;
    fMinDist = f * 0.75f;
}


#include "plProfile.h"
plProfile_CreateTimer("ShadowMaster", "RenderSetup", ShadowMaster);
bool plShadowMaster::MsgReceive(plMessage* msg)
{
    plRenderMsg* rendMsg = plRenderMsg::ConvertNoRef(msg);
    if( rendMsg )
    {
        plProfile_BeginLap(ShadowMaster, this->GetKey()->GetUoid().GetObjectName().c_str());
        IBeginRender();
        plProfile_EndLap(ShadowMaster, this->GetKey()->GetUoid().GetObjectName().c_str());
        return true;
    }

    plShadowCastMsg* castMsg = plShadowCastMsg::ConvertNoRef(msg);
    if( castMsg )
    {
        IOnCastMsg(castMsg);
        return true;
    }

    return plObjInterface::MsgReceive(msg);
}

void plShadowMaster::IBeginRender()
{
    fSlavePool.clear();
    if( ISetLightInfo() ) 
        fLightInfo->ClearSlaveBits();
}

bool plShadowMaster::IOnCastMsg(plShadowCastMsg* castMsg)
{
//  // HACKTEST
//  return false;

    if( !fLightInfo )
        return false;

    if( !fLightInfo->InVisSet(plGlobalVisMgr::Instance()->GetVisSet())
        || fLightInfo->InVisNot(plGlobalVisMgr::Instance()->GetVisNot()) )
        return false;

    const uint8_t shadowQuality = uint8_t(plShadowMaster::GetGlobalShadowQuality() * 3.9f);
    if( !GetKey()->GetUoid().GetLoadMask().MatchesQuality(shadowQuality) )
        return false;

    plShadowCaster* caster = castMsg->Caster();

    if (caster->Spans().empty())
        return false;

    hsBounds3Ext casterBnd;
    IComputeCasterBounds(caster, casterBnd);

    float power = IComputePower(caster, casterBnd);

    static float kVisShadowPower = 1.e-1f;
    static float kMinShadowPower = 2.e-1f;
    static float kKneeShadowPower = 3.e-1f;
    if( power < kMinShadowPower )
        return false;
    if( power < kKneeShadowPower )
    {
        power -= kMinShadowPower;
        power /= kKneeShadowPower - kMinShadowPower;
        power *= kKneeShadowPower - kVisShadowPower;
        power += kVisShadowPower;
    }

    // Create ShadowSlave focused on ShadowCaster
    // ShadowSlave extent just enough to cover ShadowCaster (including nearplane)
    plShadowSlave* slave = ICreateShadowSlave(castMsg, casterBnd, power);
    if( !slave )
        return false;

    // !!!IMPORTANT
    // ShadowMaster contains 2 values for yon.
    // First value applies to ShadowMaster. Any ShadowCaster beyond this distance
    //      won't cast a shadow
    // Second value applies to ShadowSlaves. This is the distance beyond the ShadowCaster
    //      (NOT FROM SHADOW SOURCE) over which the shadow attenuates to zero
    // The effective yon for the ShadowSlave is ShadowSlaveYon + DistanceToFarthestPointOnShadowCasterBound
    //      That's the distance used for culling ShadowReceivers
    // The ShadowSlaveYon is used directly in the 

    slave->fIndex = uint32_t(-1);
    castMsg->Pipeline()->SubmitShadowSlave(slave);
    
    if( slave->fIndex == uint32_t(-1) )
    {
        IRecycleSlave(slave);
        return false;
    }
    
    fLightInfo->SetSlaveBit(slave->fIndex);
    slave->SetFlag(plShadowSlave::kObeysLightGroups, fLightInfo->GetProperty(plLightInfo::kLPShadowLightGroup) && fLightInfo->GetProperty(plLightInfo::kLPHasIncludes));
    slave->SetFlag(plShadowSlave::kIncludesChars, fLightInfo->GetProperty(plLightInfo::kLPIncludesChars));

    return true;
}


plLightInfo* plShadowMaster::ISetLightInfo()
{
    fLightInfo = nullptr;
    plSceneObject* owner = IGetOwner();
    if( owner )
    {
        fLightInfo = plLightInfo::ConvertNoRef(owner->GetGenericInterface(plLightInfo::Index()));
    }
    return fLightInfo;
}

void plShadowMaster::IComputeCasterBounds(const plShadowCaster* caster, hsBounds3Ext& casterBnd)
{
    casterBnd.MakeEmpty();
    for (const plShadowCastSpan& castSpan : caster->Spans())
    {
        plDrawableSpans* dr = castSpan.fDraw;
        uint32_t index = castSpan.fIndex;

        // Right now, the generic world bounds seems close enough, even when skinned.
        // It gets a little off on the lower LODs, but, hey, they're the lower LODs.
        // Getting something more precise will probably involve finding a cagey place
        // to stash the accurate world bounds, because we only compute them when the
        // skinned objects are visible, and we need them here before the object is
        // visibility tested.
        casterBnd.Union(&dr->GetSpan(index)->fWorldBounds);
    }
}

plShadowSlave* plShadowMaster::INextSlave(const plShadowCaster* caster)
{
    return fSlavePool.next([this, caster] { return INewSlave(caster); }).get();
}

plShadowSlave* plShadowMaster::ICreateShadowSlave(plShadowCastMsg* castMsg, const hsBounds3Ext& casterBnd, float power)
{
    const plShadowCaster* caster = castMsg->Caster();

    plShadowSlave* slave = INextSlave(caster);

    slave->Init();
    //HACKTEST
//  slave->SetFlag(plShadowSlave::kReverseCull, true);

    slave->fPower = power;
    slave->fCaster = caster;
    slave->fCasterWorldBounds = casterBnd;
    slave->fAttenDist = fAttenDist * caster->GetAttenScale();
    slave->fBlurScale = caster->GetBlurScale();

    slave->SetFlag(plShadowSlave::kSelfShadow, GetProperty(kSelfShadow) || caster->GetSelfShadow());

    // Order of these matters, since values calculated in one are
    // used by later functions. Rearrange at your own risk.
    IComputeWorldToLight(casterBnd, slave);

    IComputeBounds(casterBnd, slave);

    IComputeWidthAndHeight(castMsg, slave);

    IComputeProjections(castMsg, slave);

    IComputeLUT(castMsg, slave);

    IComputeISect(casterBnd, slave);

    // Make sure we really need this shadow. If we decide we
    // don't, the returned slave will be nil;
    slave = ILastChanceToBail(castMsg, slave);

    return slave;
}

plShadowSlave* plShadowMaster::IRecycleSlave(plShadowSlave* slave)
{
    fSlavePool.pop_back();
    return nullptr;
}

plShadowSlave* plShadowMaster::ILastChanceToBail(plShadowCastMsg* castMsg, plShadowSlave* slave)
{
    const hsBounds3Ext& wBnd = slave->fWorldBounds;

    // If the bounds of the cast shadow aren't visible, forget it.
    if( !castMsg->Pipeline()->TestVisibleWorld(wBnd) )
        return IRecycleSlave(slave);

    float maxDist = fMaxDist > 0
        ? (fGlobalMaxDist > 0
            ? std::min(fMaxDist, fGlobalMaxDist)
            : fMaxDist)
        : fGlobalMaxDist;

    plConst(float) kMinFrac(0.6f);
    maxDist *= kMinFrac + GetGlobalShadowQuality() * (1.f - kMinFrac);

    // If we haven't got a max distance at which the shadow stays visible
    // then we just need to go with it.
    if( maxDist <= 0 )
        return slave;

    plConst(float) kMinFadeFrac(0.90f);
    plConst(float) kMaxFadeFrac(0.75f);
    const float fadeFrac = kMinFadeFrac + GetGlobalShadowQuality() * (kMaxFadeFrac - kMinFadeFrac);
    float minDist = maxDist * fadeFrac;

    // So we want to fade out the shadow as it gets farther away, hopefully
    // pitching it in the distance when we couldn't see it anyway. 
    hsPoint2 depth;
    // We've been testing based on the view direction, which shows unfortunate
    // camera facing dependency (because it is camera facing dependent) with
    // large shadow casters and low shadow quality. Let's try using the vector
    // connecting the caster center with the view position. It's just as wrong,
    // but at least nothing will change when you swing the camera around.
#if 0
    wBnd.TestPlane(castMsg->Pipeline()->GetViewDirWorld(), depth);
    float eyeDist = castMsg->Pipeline()->GetViewDirWorld().InnerProduct(castMsg->Pipeline()->GetViewPositionWorld());
#else
    hsPoint3 centre = wBnd.GetCenter();
    hsPoint3 vpos = castMsg->Pipeline()->GetViewPositionWorld();
    hsVector3 dir(&centre, &vpos);
    hsFastMath::NormalizeAppr(dir);
    wBnd.TestPlane(dir, depth);
    float eyeDist = dir.InnerProduct(vpos);
#endif
    float dist = depth.fX - eyeDist;

    // If it's not far enough to be fading, just go with it as is.
    dist -= minDist;
    if( dist < 0 )
        return slave;

    dist /= maxDist - minDist;
    dist = 1.f - dist;

    // If it's totally faded out, recycle the slave and return nullptr;
    if( dist <= 0 )
        return IRecycleSlave(slave);

    slave->fPower *= dist;

    return slave;
}

// compute ShadowSlave power influenced by SoftRegion, current light intensity, and ShadowCaster.fMaxOpacity;
float plShadowMaster::IComputePower(const plShadowCaster* caster, const hsBounds3Ext& casterBnd) const
{
    float power = 0;
    if( fLightInfo && !fLightInfo->IsIdle() )
    {
        power = caster->fMaxOpacity;
        float strength, scale;
        fLightInfo->GetStrengthAndScale(casterBnd, strength, scale);
        power *= strength;
    }
    power *= fPower;
    power *= caster->GetBoost();

    return power;
}

void plShadowMaster::IComputeWidthAndHeight(plShadowCastMsg* castMsg, plShadowSlave* slave) const
{
    slave->fWidth = fMaxSize;
    slave->fHeight = fMaxSize;

    if( GetGlobalShadowQuality() <= 0.5f )
    {
        slave->fWidth >>= 1;
        slave->fHeight >>= 1;
    }
    if( castMsg->Caster()->GetLimitRes() )
    {
        slave->fWidth >>= 1;
        slave->fHeight >>= 1;
    }

    const hsBounds3Ext& wBnd = slave->fWorldBounds;

    hsPoint2 depth;
    wBnd.TestPlane(castMsg->Pipeline()->GetViewDirWorld(), depth);
    float eyeDist = castMsg->Pipeline()->GetViewDirWorld().InnerProduct(castMsg->Pipeline()->GetViewPositionWorld());
    float dist = depth.fX - eyeDist;
    if( dist < 0 )
        dist = 0;

    slave->fPriority = dist; // Might want to boost the local players priority.

    plConst(float) kShiftDist = 50.f; // PERSPTEST
//  plConst(float) kShiftDist = 5000.f; // PERSPTEST
    int iShift = int(dist / kShiftDist);
    slave->fWidth >>= iShift;
    slave->fHeight >>= iShift;

    if( slave->fWidth > fGlobalMaxSize )
        slave->fWidth = fGlobalMaxSize;
    if( slave->fHeight > fGlobalMaxSize )
        slave->fHeight = fGlobalMaxSize;

    const int kMinSize = 32;
    if( slave->fWidth < kMinSize )
        slave->fWidth = kMinSize;
    if( slave->fHeight < kMinSize )
        slave->fHeight = kMinSize;
}

void plShadowMaster::IComputeLUT(plShadowCastMsg* castMsg, plShadowSlave* slave) const
{
    // This needs to go from camera space z to lightspace z, and then translate that
    // into a lookup on U from our LUT.
    // First to get into lightspace, we transform our point by
    // worldToLight * cameraToWorld.
    // Then we want to map like
    // Map 0 => (closest = CasterBnd.Closest), 1 => (CasterBnd.Closest + FalloffDist = farthest)
    // which means a matrix looking like:
    //  0.0, 0.0, 1/(farthest - closest), -closest / (farthest - closest),
    //  0.0, 0.0, 0.0, 0.0,
    //  0.0, 0.0, 0.0, 0.0,
    //  0.0, 0.0, 0.0, 0.0,


    hsBounds3Ext bnd = slave->fCasterWorldBounds;
    bnd.Transform(&slave->fWorldToLight);
    float farthest = bnd.GetCenter().fZ + slave->fAttenDist;
    float closest = bnd.GetMins().fZ;

    // Shouldn't this always be negated?
    static hsMatrix44 lightToLut; // Static ensures initialized to all zeros.
    lightToLut.fMap[0][2] = 1/(farthest - closest); 
    lightToLut.fMap[0][3] = -closest / (farthest - closest);

    // This full matrix multiply is a little overkill. Could simplify it quite a bit...
    slave->fRcvLUT = lightToLut * slave->fWorldToLight;

    // For caster, we'll be rendering in light space, so we just need to lut off
    // cameraspace z
    // Can put bias here if needed (probably) by adding small 
    // bias to ShadowSlave.LUTXfm.fMap[0][3]. Bias magnitude would probably be at
    // least 0.5f/256.f to compensate for quantization.

    plConst(float) kSelfBias = 2.f / 256.f;
#if 0 // MF_NOSELF
    plConst(float) kOtherBias = -0.5 / 256.f;
    lightToLut.fMap[0][3] += slave->HasFlag(plShadowSlave::kSelfShadow) ? kSelfBias : kOtherBias;
#else // MF_NOSELF
    lightToLut.fMap[0][3] += kSelfBias;
#endif // MF_NOSELF

    if( slave->CastInCameraSpace() )
    {
        slave->fCastLUT = lightToLut * slave->fWorldToLight;
    }
    else
    {
        slave->fCastLUT = lightToLut;
    }

#ifdef MF_HACK_SKIPLUT
    hsMatrix44 hack;
    hack.Reset();
    hack.NotIdentity();
    hack.fMap[0][0] = 0;
    hack.fMap[0][3] = 1.f;
    slave->fCastLUT = hack;
#endif // MF_HACK_SKIPLUT
}
