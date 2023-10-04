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

#include <vector>

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "plAnimComponent.h"
#include "MaxMain/plMaxNode.h"
#include "resource.h"

#include <iparamm2.h>
#include <windowsx.h>


#include "plParticleComponents.h"
#include "plNotetrackAnim.h"

#include "pnSceneObject/plSceneObject.h"
#include "plScene/plSceneNode.h"

#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/hsMaterialConverter.h"
#include "MaxConvert/plMeshConverter.h"
#include "MaxConvert/hsControlConverter.h"

#include "MaxPlasmaMtls/Materials/plParticleMtl.h"

#include "MaxExport/plErrorMsg.h"

#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "plInterp/plController.h"
#include "plInterp/hsInterp.h"
#include "plInterp/plAnimEaseTypes.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxCompat.h"
#include "pnKeyedObject/plKey.h"
#include "plSurface/hsGMaterial.h"
#include "plDrawable/plGBufferGroup.h"

#include "plParticleSystem/plParticleSystem.h"
#include "plParticleSystem/plParticleEmitter.h"
#include "plParticleSystem/plParticleEffect.h"
#include "plParticleSystem/plParticleGenerator.h"
#include "plParticleSystem/plParticleApplicator.h"
#include "plParticleSystem/plConvexVolume.h"
#include "plParticleSystem/plBoundInterface.h"

#include "plAnimation/plScalarChannel.h"
#include "plAnimation/plAGAnim.h"

#include "pnSceneObject/plDrawInterface.h"

#include "plGLight/plLightInfo.h"
#include "plLightGrpComponent.h"

void DummyCodeIncludeFuncParticles()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// stuff for plParticleComponent

const TCHAR* plParticleCoreComponent::GenStrings[] = // line these up with the generation types enum.
{
    _T("Point Source"),
    _T("Mesh"),
    _T("One Per Vertex")
};

bool plParticleCoreComponent::IsParticleSystemComponent(plComponentBase *comp)
{
    if (comp->ClassID() == PARTICLE_SYSTEM_COMPONENT_CLASS_ID)
        return true;

    return false;
}

bool plParticleCoreComponent::NodeHasSystem(plMaxNode *pNode)
{
    int i;
    for (i = 0; i < pNode->NumAttachedComponents(); i++)
    {
        if (plParticleCoreComponent::IsParticleSystemComponent(pNode->GetAttachedComponent(i)))
            return true;
    }
    return false;
}

bool plParticleCoreComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    GetParamVals( pNode );
    pNode->SetForceLocal(true);
    pNode->SetDrawable(false);
    pNode->SetParticleRelated(true);

    Mtl *maxMaterial = hsMaterialConverter::Instance().GetBaseMtl(pNode);
    plConvert &convert = plConvert::Instance();
    if (!hsMaterialConverter::IsParticleMat(maxMaterial))
    {
        maxMaterial = nullptr;
        pNode->SetMtl(nullptr);
        if (pErrMsg->Set(!(convert.fWarned & plConvert::kWarnedBadMaterialOnParticle), pNode->GetName(),
        "Only \"Plasma Particle\" materials (not in a multi-material) may be applied to particle system objects."
        " Using a default material for now.").CheckAskOrCancel())
        {
            convert.fWarned |= plConvert::kWarnedBadMaterialOnParticle;
        }
        pErrMsg->Set(false);
    }

    // Moving this from Convert so the DrawInterface will appear sooner. Other components expect
    // the interfaces to be fully set up by the Convert pass.
    plSceneNode *sNode = plSceneNode::ConvertNoRef( pNode->GetRoomKey()->GetObjectPtr() );
    plDrawInterface *di = new plDrawInterface;
    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), di, pNode->GetLocation(), pNode->GetLoadMask());
    hsgResMgr::ResMgr()->AddViaNotify( di->GetKey(), new plObjRefMsg(pNode->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );
    pNode->SetDISceneNodeSpans(di, true);

    return true;
}

bool plParticleCoreComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{   
    int32_t i, j, k;

    plLocation nodeLoc = node->GetKey()->GetUoid().GetLocation();
    ST::string objName = node->GetKey()->GetName();

    plSceneObject *sObj = node->GetSceneObject();
    plParticleSystem *sys = new plParticleSystem();

    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), sys, nodeLoc, node->GetLoadMask());   

    // Add material and lifespan animated params.
    Mtl *maxMaterial = hsMaterialConverter::Instance().GetBaseMtl(node);
    std::vector<hsGMaterial *> matArray;
    hsMaterialConverter::Instance().GetMaterialArray(maxMaterial, node, matArray);
    hsGMaterial* particleMat = matArray[0];
    
    plController *ambientCtl = nullptr;
    plController *diffuseCtl = nullptr;
    plController *opacityCtl = nullptr;
    plController *widthCtl = nullptr;
    plController *heightCtl = nullptr;
    hsControlConverter& cc = hsControlConverter::Instance();

    if (hsMaterialConverter::IsParticleMat(maxMaterial)) // Exporter will yell if this is false, but we have to handle it
    {
        plParticleMtl *particleMtl = (plParticleMtl *)maxMaterial; // We check beforehand that this is ok
        SetParticleStats(particleMtl);
        ambientCtl = cc.MakeColorController(particleMtl->GetAmbColorController(), node);
        diffuseCtl = cc.MakeColorController(particleMtl->GetColorController(), node);
        opacityCtl = cc.MakeScalarController(particleMtl->GetOpacityController(), node);
        widthCtl = cc.MakeScalarController(particleMtl->GetWidthController(), node);
        heightCtl = cc.MakeScalarController(particleMtl->GetHeightController(), node);
    }

    float genLife        = -1;
    float partLifeMin, partLifeMax;
    float pps            = fUserInput.fPPS; 
    hsPoint3 pos;
    float pitch          = hsConstants::pi<float>;
    float yaw            = 0; 
    float angleRange     = hsDegreesToRadians(fUserInput.fConeAngle);
    float velMin         = fUserInput.fVelocityMin;
    float velMax         = fUserInput.fVelocityMax;
    float xSize          = fUserInput.fHSize;
    float ySize          = fUserInput.fVSize;
    float scaleMin       = fUserInput.fScaleMin / 100.0f;
    float scaleMax       = fUserInput.fScaleMax / 100.0f;
    float gravity        = fUserInput.fGravity / 100.0f;
    float drag           = fUserInput.fDrag / 100.f;
    float windMult       = fUserInput.fWindMult / 100.f;
    float massRange      = fUserInput.fMassRange;
    float rotRange       = hsDegreesToRadians(fUserInput.fRotRange);

    uint32_t xTiles = fUserInput.fXTiles; 
    uint32_t yTiles = fUserInput.fYTiles; 
    uint32_t maxEmitters = 1 + GetEmitterReserve();
    uint32_t maxTotalParticles = 0;   

    // Need to do this even when immortal, so that maxTotalParticles is computed correctly.
    partLifeMin = fUserInput.fLifeMin;
    partLifeMax = fUserInput.fLifeMax;
    plLeafController *ppsCtl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kPPS)), node);
    if (ppsCtl != nullptr && ppsCtl->GetLength() > 0)
    {
        // Simulate just the birth across the curve and record the max
        float frameDelta = (1.f / MAX_FRAMES_PER_SEC); 
        float avgLife = (partLifeMax + partLifeMin) / 2;
        uint32_t count = node->NumAttachedComponents();
        uint32_t lifeTicks = uint32_t(avgLife / frameDelta);
        float *birth = new float[lifeTicks];

        // Find any anim components attached to the same node.
        for (i = 0; i < count; i++)
        {
            if (!plAnimComponentBase::IsAnimComponent(node->GetAttachedComponent(i)))
                continue;

            float maxAnimParticles = 0;

            plAnimComponentBase *comp = (plAnimComponentBase *)node->GetAttachedComponent(i);
            plATCAnim *anim = plATCAnim::ConvertNoRef(comp->fAnims[node]);

            // If it's an ATC anim, we can be aggressive in determining the max
            if (anim)
            {           
                float curAnimParticles = 0;

                float loopStart, loopEnd;

                for (j = -1; j < (int32_t)anim->GetNumLoops(); j++)
                {
                    // Initialize our birth counters
                    for (k = 0; k < lifeTicks; k++)
                        birth[k] = 0;

                    if (j == -1)
                    {
                        loopStart = anim->GetStart();
                        loopEnd = anim->GetEnd();
                    }
                    else
                        anim->GetLoop(j, loopStart, loopEnd);

                    float loopLength = loopEnd - loopStart;

                    if (loopLength == 0) // It's the default "(Entire Animation)"
                        loopLength = ppsCtl->GetLength();

                    uint32_t loopTicks = uint32_t(loopLength * MAX_FRAMES_PER_SEC);

                    uint32_t startTick = uint32_t(loopStart * MAX_FRAMES_PER_SEC);
                    for (uint32_t tick = 0; tick < loopTicks + lifeTicks; tick++)
                    {
                        curAnimParticles -= birth[tick % lifeTicks] * frameDelta;
                        float birthStart = 0.f;
                        float birthEnd = 0.f;
                        ppsCtl->Interp(((tick % loopTicks) + startTick) * frameDelta, &birthStart);
                        ppsCtl->Interp(((tick % loopTicks) + startTick + 1) * frameDelta, &birthEnd);
                        birth[tick % lifeTicks] = (birthStart + birthEnd) / 2;
                        curAnimParticles += birth[tick % lifeTicks] * frameDelta;
                        if (curAnimParticles > maxAnimParticles)
                            maxAnimParticles = curAnimParticles;
                    }
                }
            }
            else // No info on the animation. Assume the worst.
            {
                float maxPps = 0;
                int i;
                for (i = 1; i < ppsCtl->GetNumKeys(); i++)
                {
                    float curVal = 0;
                    hsScalarKey *key = ppsCtl->GetScalarKey(i);
                    if (key)
                        curVal = key->fValue;
                    
                    hsBezScalarKey *bezKey = ppsCtl->GetBezScalarKey(i);
                    if (bezKey)
                        curVal = bezKey->fValue;

                    if( curVal > maxPps )
                        maxPps = curVal;
                }
                maxAnimParticles = maxPps * (partLifeMax - (partLifeMax - partLifeMin) / 2);
            }
            
            if (maxTotalParticles < maxAnimParticles)
                maxTotalParticles = (uint32_t)maxAnimParticles;
        }
        delete [] birth;
    }
    else
    {
        maxTotalParticles = uint32_t(pps * (partLifeMax - (partLifeMax - partLifeMin) / 2));
    }
    maxTotalParticles *=  maxEmitters;

    delete ppsCtl;
    ppsCtl = nullptr;
    
    uint32_t maxAllowedParticles = plGBufferGroup::kMaxNumIndicesPerBuffer / 6;
    if (maxTotalParticles > maxAllowedParticles)
    {
        char text[512];
        sprintf(text, "This particle system requires a buffer for %d particles. "
                      "The max allowed for a single system is %d. Capping this system "
                      "at the max. If you need more, create a 2nd particle system "
                      "and balance out the birthrates.", 
                      maxTotalParticles, maxAllowedParticles);

        plConvert &convert = plConvert::Instance();
        if (pErrMsg->Set(!(convert.fWarned & plConvert::kWarnedTooManyParticles), node->GetName(), text).CheckAskOrCancel())
        {
            convert.fWarned |= plConvert::kWarnedTooManyParticles;
        }
        pErrMsg->Set(false);
        maxTotalParticles = maxAllowedParticles;
    }
    
    if (fUserInput.fImmortal)
    {
        partLifeMin     = -1.0;
        partLifeMax     = -1.0;
    }

    // Figure out the appropriate generator to add
    plParticleGenerator *generator = nullptr;
    uint32_t sources;
    float *pitchArray;
    float *yawArray;
    hsPoint3 *pointArray;
    hsVector3 *dirArray;
    if (fUserInput.fGenType == kGenPoint)
    {
        sources = 1;
        pitchArray = new float[sources];
        yawArray = new float[sources];
        pointArray = new hsPoint3[sources];
        pitchArray[0] = pitch;
        yawArray[0] = yaw;
        pointArray[0].Set(0, 0, 0);
        plSimpleParticleGenerator *gen = new plSimpleParticleGenerator();
        gen->Init(genLife, partLifeMin, partLifeMax, pps, sources, pointArray, pitchArray, yawArray, angleRange, 
                  velMin, velMax, xSize, ySize, scaleMin, scaleMax, massRange, rotRange);
        generator = gen;
    }
    else if (fUserInput.fGenType == kGenMesh)
    {
        std::vector<hsVector3> normals;
        std::vector<hsPoint3> pos;
        plMeshConverter::Instance().StuffPositionsAndNormals(node, &pos, &normals);
        sources = (uint32_t)normals.size();
        pitchArray = new float[sources];
        yawArray = new float[sources];
        pointArray = new hsPoint3[sources];
        for (uint32_t i = 0; i < sources; i++)
        {
            plParticleGenerator::ComputePitchYaw(pitchArray[i], yawArray[i], normals[i]);
            pointArray[i] = pos[i];
        }
        plSimpleParticleGenerator *gen = new plSimpleParticleGenerator();
        gen->Init(genLife, partLifeMin, partLifeMax, pps, sources, pointArray, pitchArray, yawArray, angleRange, 
                  velMin, velMax, xSize, ySize, scaleMin, scaleMax, massRange, rotRange);
        generator = gen;
    }
    else // One per vertex
    {
        std::vector<hsVector3> normals;
        std::vector<hsPoint3> pos;
        plMeshConverter::Instance().StuffPositionsAndNormals(node, &pos, &normals);
        sources = (uint32_t)normals.size();

        pointArray = new hsPoint3[sources];
        dirArray = new hsVector3[sources];
        for (uint32_t i = 0; i < sources; i++)
        {
            dirArray[i] = normals[i];
            pointArray[i] = pos[i];
        }

        plOneTimeParticleGenerator *gen = new plOneTimeParticleGenerator();
        gen->Init(float(sources), pointArray, dirArray, xSize, ySize, scaleMin, scaleMax, rotRange);
        generator = gen;
        maxTotalParticles = sources;
        gravity = 0.f;
    }

    // Init and attach to the scene object
    sys->Init(xTiles, yTiles, maxTotalParticles, maxEmitters, ambientCtl, diffuseCtl, opacityCtl, 
              widthCtl, heightCtl);
    hsgResMgr::ResMgr()->AddViaNotify( particleMat->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, 0 ), plRefFlags::kActiveRef );
    hsgResMgr::ResMgr()->AddViaNotify( sys->GetKey(), new plObjRefMsg( sObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );

    // Set up normals and orientation
    uint32_t miscFlags = 0;
    switch(fUserInput.fNormal)
    {
    case plParticleMtl::kNormalViewFacing:
        miscFlags |= plParticleEmitter::kNormalViewFacing;
        break;
    case plParticleMtl::kNormalUp:
    case plParticleMtl::kEmissive: // For emissive, we don't care about the normal. This choice makes us
                                   // not waste time computing one.
        miscFlags |= plParticleEmitter::kNormalUp;
        break;
    case plParticleMtl::kNormalNearestLight:
        miscFlags |= plParticleEmitter::kNormalNearestLight;
        break;
    case plParticleMtl::kNormalFromCenter:
        miscFlags |= plParticleEmitter::kNormalFromCenter;
        break;
    case plParticleMtl::kNormalVelUpVel:
        miscFlags |= plParticleEmitter::kNormalVelUpVel;
        break;
    }

    switch(fUserInput.fOrientation)
    {
    case plParticleMtl::kOrientVelocity:
        miscFlags |= plParticleEmitter::kOrientationVelocityBased;
        break;
    case plParticleMtl::kOrientUp:
        miscFlags |= plParticleEmitter::kOrientationUp;
        break;
    case plParticleMtl::kOrientVelStretch:
        miscFlags |= plParticleEmitter::kOrientationVelocityStretch;
        break;
    case plParticleMtl::kOrientVelFlow:
        miscFlags |= plParticleEmitter::kOrientationVelocityFlow;
        break;
    }
    
    if (fUserInput.fGenType == kGenOnePerVertex &&
        (miscFlags & plParticleEmitter::kOrientationVelocityMask))
    {
        char text[256];
        sprintf(text, "This particle system has an orientation that's based on velocity "
                      "(see the Particle Material), which doesn't work with OnePerVertex "
                      "generation. No particles from this system will be visible.");

        plConvert &convert = plConvert::Instance();
        if (pErrMsg->Set(!(convert.fWarned & plConvert::kWarnedParticleVelAndOnePer), node->GetName(), text).CheckAskOrCancel())
        {
            convert.fWarned |= plConvert::kWarnedParticleVelAndOnePer;
        }
        pErrMsg->Set(false);
    }
    if( maxEmitters > 1 )
        miscFlags |= plParticleEmitter::kOnReserve;

    sys->AddEmitter( maxTotalParticles, generator, miscFlags );
    sys->SetGravity(gravity);
    sys->SetDrag(drag);
    sys->SetWindMult(windMult);
    sys->SetPreSim(fUserInput.fPreSim);

    // Finally, any attached effects.
    for (i = 0; i < node->NumAttachedComponents(); i++)
    {
        plComponentBase *comp = node->GetAttachedComponent(i);
        if (plParticleEffectComponent::IsParticleEffectComponent(comp))
            ((plParticleEffectComponent *)comp)->AddToParticleSystem(sys, node);
        if (comp->ClassID() == LIGHTGRP_COMP_CID)
            IHandleLights((plLightGrpComponent*)comp, sys);
    }
    
    if (fCompPB->GetInt(ParamID(kFollowSystem)))
    {
        plParticleFollowSystemEffect *effect = new plParticleFollowSystemEffect;
        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());
        hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectMisc ), plRefFlags::kActiveRef );
    }

    return true;
}

void plParticleCoreComponent::IHandleLights(plLightGrpComponent* liGrp, plParticleSystem* sys)
{
    const std::vector<plLightInfo*>& liInfo = liGrp->GetLightInfos();
    for (plLightInfo* light : liInfo)
        sys->AddLight(light->GetKey());
}

bool plParticleCoreComponent::AddToAnim(plAGAnim *anim, plMaxNode *node)
{
    bool result = false;
    plController *ctl;
    hsControlConverter& cc = hsControlConverter::Instance();

    float start, end;
    if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
    {
        start = end = -1;
    }
    else
    {
        start = anim->GetStart();
        end = anim->GetEnd();
    }

    if (fCompPB->GetInt(kGenType) != kGenOnePerVertex)
    {
        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kLifeMin)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleLifeMinApplicator *app = new plParticleLifeMinApplicator();
            app->SetChannelName(node->GetName());
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }

        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kLifeMax)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleLifeMaxApplicator *app = new plParticleLifeMaxApplicator();
            app->SetChannelName(node->GetName());
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }

        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kPPS)), node, start, end);
        if (ctl != nullptr)
        {
            plParticlePPSApplicator *app = new plParticlePPSApplicator();
            app->SetChannelName(node->GetName());
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }

        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kConeAngle)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleAngleApplicator *app = new plParticleAngleApplicator();
            app->SetChannelName(node->GetName());
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }

        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kVelocityMin)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleVelMinApplicator *app = new plParticleVelMinApplicator();
            app->SetChannelName(node->GetName());
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }

        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kVelocityMax)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleVelMaxApplicator *app = new plParticleVelMaxApplicator();
            app->SetChannelName(node->GetName());
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }
        
        /*
        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kGravity)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleGravityApplicator *app = new plParticleGravityApplicator();
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }

        ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kDrag)), node, start, end);
        if (ctl != nullptr)
        {
            plParticleDragApplicator *app = new plParticleDragApplicator();
            plAnimComponentBase::SetupCtl(anim, ctl, app, node);
            result = true;      
        }
        */
    }

    ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kScaleMin)), node, start, end);
    if (ctl != nullptr)
    {
        plParticleScaleMinApplicator *app = new plParticleScaleMinApplicator();
        app->SetChannelName(node->GetName());
        plAnimComponentBase::SetupCtl(anim, ctl, app, node);
        result = true;      
    }

    ctl = cc.MakeScalarController(GetParamBlock2Controller(fCompPB, ParamID(kScaleMax)), node, start, end);
    if (ctl != nullptr)
    {
        plParticleScaleMaxApplicator *app = new plParticleScaleMaxApplicator();
        app->SetChannelName(node->GetName());
        plAnimComponentBase::SetupCtl(anim, ctl, app, node);
        result = true;      
    }

    return result;
}

void plParticleCoreComponent::SetParticleStats(plParticleMtl *mtl)
{
    IParamBlock2 *pb = mtl->GetParamBlockByID(plParticleMtl::kBlkBasic);
    fUserInput.fHSize = pb->GetFloat(plParticleMtl::kWidth);
    fUserInput.fVSize = pb->GetFloat(plParticleMtl::kHeight);
    fUserInput.fXTiles = pb->GetInt(plParticleMtl::kXTiles);
    fUserInput.fYTiles = pb->GetInt(plParticleMtl::kYTiles);
    fUserInput.fNormal = pb->GetInt(plParticleMtl::kNormal);
    fUserInput.fOrientation = pb->GetInt(plParticleMtl::kOrientation);
}

class ParticleCompDlgProc : public ParamMap2UserDlgProc
{
protected:
    void EnableDynGenParams(IParamMap2 *pm, bool enabled)
    {
        pm->Enable(plParticleComponent::kConeAngle, enabled);
        pm->Enable(plParticleComponent::kVelocityMin, enabled);
        pm->Enable(plParticleComponent::kVelocityMax, enabled);
        pm->Enable(plParticleComponent::kLifeMin, enabled);
        pm->Enable(plParticleComponent::kLifeMax, enabled);
        pm->Enable(plParticleComponent::kImmortal, enabled);
        pm->Enable(plParticleComponent::kPPS, enabled);
        pm->Enable(plParticleComponent::kGravity, enabled);
        pm->Enable(plParticleComponent::kPreSim, enabled);
        pm->Enable(plParticleComponent::kDrag, enabled);
    }

public:
    ParticleCompDlgProc() {}
    ~ParticleCompDlgProc() {}

    void IValidateSpinners(TimeValue t, IParamBlock2 *pb, IParamMap2 *map, uint32_t id)
    {
        uint32_t minIndex, maxIndex;
        bool adjustMin;
        switch(id)
        {
        case IDC_COMP_PARTICLE_VELMIN:
        case IDC_COMP_PARTICLE_VELMIN_SPIN:
            minIndex = plParticleCoreComponent::kVelocityMin; maxIndex = plParticleCoreComponent::kVelocityMax; adjustMin = false;
            break;
        case IDC_COMP_PARTICLE_VELMAX:
        case IDC_COMP_PARTICLE_VELMAX_SPIN:
            minIndex = plParticleCoreComponent::kVelocityMin; maxIndex = plParticleCoreComponent::kVelocityMax; adjustMin = true;
            break;
        case IDC_COMP_PARTICLE_LIFEMIN:
        case IDC_COMP_PARTICLE_LIFEMIN_SPIN:
            minIndex = plParticleCoreComponent::kLifeMin; maxIndex = plParticleCoreComponent::kLifeMax; adjustMin = false;
            break;
        case IDC_COMP_PARTICLE_LIFEMAX:
        case IDC_COMP_PARTICLE_LIFEMAX_SPIN:
            minIndex = plParticleCoreComponent::kLifeMin; maxIndex = plParticleCoreComponent::kLifeMax; adjustMin = true;
            break;
        default:
            return;
        }

        float min, max;
        min = pb->GetFloat(minIndex, t);
        max = pb->GetFloat(maxIndex, t);

        if (min > max)
        {
            if (adjustMin)
                pb->SetValue(minIndex, t, max);
            else
                pb->SetValue(maxIndex, t, min);

            map->Invalidate(minIndex);
            map->Invalidate(maxIndex);
        }
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        IParamBlock2 *pb = map->GetParamBlock();
        HWND cbox = nullptr;

        int selection;
        switch (msg)
        {
        case WM_INITDIALOG:
            int j;
            for (j = 0; j < plParticleCoreComponent::kGenNumOptions; j++)
            {
                cbox = GetDlgItem(hWnd, IDC_GEN_TYPE);
                SendMessage(cbox, CB_ADDSTRING, 0, (LPARAM)plParticleCoreComponent::GenStrings[j]);
            }
            selection = pb->GetInt(plParticleCoreComponent::kGenType);
            SendMessage(cbox, CB_SETCURSEL, selection, 0);
            EnableDynGenParams(map, selection != plParticleCoreComponent::kGenOnePerVertex);

            CheckDlgButton(hWnd, IDC_TRACKVIEW_SHOW, plParticleComponent::fAllowUnhide ? BST_CHECKED : BST_UNCHECKED);
            return TRUE;

        case WM_COMMAND:  
            if (id == IDC_GEN_TYPE)
            {
                selection = (int)SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0);
                pb->SetValue(plParticleCoreComponent::kGenType, t, selection);
                EnableDynGenParams(map, selection != plParticleCoreComponent::kGenOnePerVertex);
                return TRUE;
            }
            else if (id == IDC_COMP_PARTICLE_VELMIN || id == IDC_COMP_PARTICLE_VELMAX ||
                id == IDC_COMP_PARTICLE_LIFEMIN || id == IDC_COMP_PARTICLE_LIFEMAX)
            {
                IValidateSpinners(t, pb, map, id);
                return TRUE;
            }
            else if (id == IDC_TRACKVIEW_SHOW && code == BN_CLICKED)
            {
                plParticleComponent::fAllowUnhide = (IsDlgButtonChecked(hWnd, IDC_TRACKVIEW_SHOW) == BST_CHECKED);
                plComponentShow::Update();
                return TRUE;
            }
            break;
        case CC_SPINNER_CHANGE:
            if (id == IDC_COMP_PARTICLE_VELMIN_SPIN || id == IDC_COMP_PARTICLE_VELMAX_SPIN ||
                id == IDC_COMP_PARTICLE_LIFEMIN_SPIN || id == IDC_COMP_PARTICLE_LIFEMAX_SPIN)
            {
                IValidateSpinners(t, pb, map, id);
                return TRUE;
            }
            break;
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static ParticleCompDlgProc gParticleCompDlgProc;

CLASS_DESC(plParticleComponent, gParticleDesc, "Particle System",  "ParticleSystem", COMP_TYPE_PARTICLE, PARTICLE_SYSTEM_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleBk
(   

    plComponent::kBlkComp, _T("Particle"), 0, &gParticleDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_PARTICLE, IDS_COMP_PARTICLE_ROLL, 0, 0, &gParticleCompDlgProc,

    //Particle Properties....

    plParticleCoreComponent::kGenType,      _T("Generation"),   TYPE_INT, 0, 0,
        p_default, 0,
        p_end,

    plParticleCoreComponent::kConeAngle,        _T("ConeAngle"),    TYPE_FLOAT,     P_ANIMATABLE, IDS_PARTICLE_CONE_ANGLE,  
        p_default, 45.0,
        p_range, 0.0, 180.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_CONE, IDC_COMP_PARTICLE_CONE_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kVelocityMin,  _T("VelocityMin"),  TYPE_FLOAT,     P_ANIMATABLE, IDS_PARTICLE_VELOCITY_MIN,    
        p_default, 50.0,
        p_range, 0.0, 500.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_VELMIN, IDC_COMP_PARTICLE_VELMIN_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kVelocityMax,  _T("VelocityMax"),  TYPE_FLOAT,     P_ANIMATABLE, IDS_PARTICLE_VELOCITY_MAX,    
        p_default, 50.0,
        p_range, 0.0, 500.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_VELMAX, IDC_COMP_PARTICLE_VELMAX_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kLifeMin,      _T("LifeMin"),      TYPE_FLOAT,     P_ANIMATABLE, IDS_PARTICLE_LIFE_MIN,    
        p_default, 10.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_LIFEMIN, IDC_COMP_PARTICLE_LIFEMIN_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kLifeMax,      _T("LifeMax"),      TYPE_FLOAT,     P_ANIMATABLE, IDS_PARTICLE_LIFE_MAX,    
        p_default, 5.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_LIFEMAX, IDC_COMP_PARTICLE_LIFEMAX_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kImmortal, _T("Immortal"),     TYPE_BOOL,  0, 0,
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_NODIE,
        p_end,

    plParticleCoreComponent::kPPS,  _T("PPS"),  TYPE_FLOAT,     P_ANIMATABLE, IDS_PARTICLE_PPS, 
        p_default, 20.0,
        p_range, 0.0, 5000.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_PPS, IDC_COMP_PARTICLE_PPS_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kScaleMin, _T("ScaleMin"), TYPE_INT,   P_ANIMATABLE, IDS_PARTICLE_SCALE_MIN,   
        p_default, 100,
        p_range, 1, 1000,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT,   
        IDC_COMP_PARTICLE_SCALEMIN, IDC_COMP_PARTICLE_SCALEMIN_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kScaleMax, _T("ScaleMax"), TYPE_INT,   P_ANIMATABLE, IDS_PARTICLE_SCALE_MAX,   
        p_default, 100,
        p_range, 1, 1000,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT,   
        IDC_COMP_PARTICLE_SCALEMAX, IDC_COMP_PARTICLE_SCALEMAX_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kGravity,  _T("Gravity"),  TYPE_INT,   0, 0,   
        p_default, 100,
        p_range, -100, 100,
        p_ui,   TYPE_SPINNER,   EDITTYPE_INT,   
        IDC_COMP_PARTICLE_GRAVITY, IDC_COMP_PARTICLE_GRAVITY_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kDrag,     _T("Drag"),     TYPE_INT,   0, 0,   
        p_default, 0,
        p_range, 0, 1000,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT,   
        IDC_COMP_PARTICLE_DRAG, IDC_COMP_PARTICLE_DRAG_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kPreSim,   _T("PreSim"),   TYPE_INT,   0, 0,   
        p_default, 0,
        p_range, 0, 100,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT,   
        IDC_COMP_PARTICLE_PRESIM, IDC_COMP_PARTICLE_PRESIM_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kWindMult,     _T("WindMult"),     TYPE_INT,   0, 0,   
        p_default, 100,
        p_range, 0, 1000,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT,   
        IDC_COMP_PARTICLE_WIND_MULT, IDC_COMP_PARTICLE_WIND_MULT_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kMassRange,        _T("MassRange"),        TYPE_FLOAT,     0, 0,   
        p_default, 0.f,
        p_range, 0.f, 1000.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_MASS_RANGE, IDC_COMP_PARTICLE_MASS_RANGE_SPIN, 1.0,
        p_end,

    plParticleCoreComponent::kRotRange,     _T("RotRange"),     TYPE_FLOAT,     0, 0,   
        p_default, 0.f,
        p_range, 0.f, 180.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_ROT_RANGE, IDC_COMP_PARTICLE_ROT_RANGE_SPIN, 1.0,
        p_end,
        
    plParticleCoreComponent::kFollowSystem, _T("FollowSystem"),     TYPE_BOOL, 0, 0, 
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_FOLLOW_SYSTEM,
        p_end,        

    p_end
);

bool plParticleComponent::fAllowUnhide = false;

plParticleComponent::plParticleComponent()
:   fEmitterReserve(0)
{
    fClassDesc = &gParticleDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// Gets values from the ParamBlock and passes them to the Convert
bool plParticleComponent::GetParamVals(plMaxNode *pNode)
{
    fUserInput.fGenType         = fCompPB->GetInt(kGenType);
    fUserInput.fConeAngle       = fCompPB->GetFloat(kConeAngle);
    fUserInput.fVelocityMin     = fCompPB->GetFloat(kVelocityMin);
    fUserInput.fVelocityMax     = fCompPB->GetFloat(kVelocityMax);
    fUserInput.fLifeMin         = fCompPB->GetFloat(kLifeMin);
    fUserInput.fLifeMax         = fCompPB->GetFloat(kLifeMax);
    fUserInput.fImmortal        = fCompPB->GetInt(kImmortal);
    fUserInput.fPPS             = fCompPB->GetFloat(kPPS);
    fUserInput.fScaleMin        = (float)fCompPB->GetInt(kScaleMin);
    fUserInput.fScaleMax        = (float)fCompPB->GetInt(kScaleMax);

    fUserInput.fGravity         = (float)fCompPB->GetInt(kGravity);
    fUserInput.fDrag            = (float)fCompPB->GetInt(kDrag);
    fUserInput.fWindMult        = (float)fCompPB->GetInt(kWindMult);
    fUserInput.fMassRange       = fCompPB->GetFloat(kMassRange);
    fUserInput.fPreSim          = (float)fCompPB->GetInt(kPreSim);
    fUserInput.fRotRange        = fCompPB->GetFloat(kRotRange);

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Particle Effects Base class

// Make sure any new Effect you add is accounted for here, or it won't get converted.
bool plParticleEffectComponent::IsParticleEffectComponent(plComponentBase *comp)
{
    if (comp->ClassID() == PARTICLE_FADE_COMPONENT_CLASS_ID ||
        comp->ClassID() == PARTICLE_VOLUME_COMPONENT_CLASS_ID ||
        comp->ClassID() == PARTICLE_WIND_COMPONENT_CLASS_ID ||
        comp->ClassID() == PARTICLE_UNIWIND_COMPONENT_CLASS_ID ||
        comp->ClassID() == PARTICLE_FLOCK_COMPONENT_CLASS_ID)
        return true;

    return false;
}

bool plParticleEffectComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fEffect = nullptr;
    return true;
}

bool plParticleEffectComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    bool valid = plParticleCoreComponent::NodeHasSystem(node);
    if (!valid)
    {
        pErrMsg->Set(true, node->GetName(), "Node has a particle effect component, "
                     "but no particle system to apply it to. Ignoring component.").Show();
        pErrMsg->Set(false);
    }

    return valid;
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleFadeComponent

void plParticleFadeComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
    plParticleFadeVolumeEffect *effect = nullptr;
    if( !fEffect )
    {
        effect = new plParticleFadeVolumeEffect();
        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());
        effect->fLength = (float)fCompPB->GetInt(kFadeDistance);
        if (fCompPB->GetInt(kFadeZ))
            effect->fIgnoreZ = false;
        
        fEffect = effect;
    }
    else
    {
        effect = plParticleFadeVolumeEffect::ConvertNoRef(fEffect);
    }
    hsAssert(effect, "Our effect pointer was wrong type?");
    hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectMisc ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleFadeComponent, gParticleFadeDesc, "Fade Volume Effect",  "Fade Volume Effect", COMP_TYPE_PARTICLE, PARTICLE_FADE_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleFadeBk
(   

    plComponent::kBlkComp, _T("ParticleFade"), 0, &gParticleFadeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_PARTICLE_FADE, IDS_COMP_PARTICLE_FADE_ROLL, 0, 0, nullptr,

    plParticleFadeComponent::kFadeDistance, _T("FadeDistance"), TYPE_INT,   P_ANIMATABLE, 0,    
        p_default, 100,
        p_range, 0, 10000,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_INT,   
        IDC_COMP_PARTICLE_FADE_DIST, IDC_COMP_PARTICLE_FADE_DIST_SPIN, 1.0,
        p_end,

    plParticleFadeComponent::kFadeZ,        _T("FadeInZ"),      TYPE_BOOL,  P_ANIMATABLE, 0,    
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_FADEZ,
        p_end,

    p_end
);

plParticleFadeComponent::plParticleFadeComponent()
{
    fClassDesc = &gParticleFadeDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

////////////////////////////////////////////////////////////////////////////
//
// Convex Volume Component

void plParticleVolumeComponent::CollectNonDrawables(INodeTab& nonDrawables)
{
    INode* source = fCompPB->GetINode(kSourceNode);
    if( source )
        nonDrawables.Append(1, &source);
}

bool plParticleVolumeComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    if (!plParticleEffectComponent::PreConvert(pNode, pErrMsg))
        return false;

    fBound = nullptr;

    plMaxNode *source = (plMaxNode *)fCompPB->GetINode(kSourceNode);
    if (source == nullptr || !source->CanConvert())
    {
        pErrMsg->Set(true, pNode->GetName(), "Particle Convex Volume component has not been assigned a "
                     "node to build itself from or Volume has Ignore component on it.. Ignoring component.").Show();
        pErrMsg->Set(false);
        return false; // No source selected
    }

    source->SetForceLocal(true);
    source->SetDrawable(false);
    source->SetParticleRelated(true);

    return true;
}

void plParticleVolumeComponent::BuildVolume(plMaxNode *node)
{
    if (fBound != nullptr)
        return; // already converted it

    fBound = new plBoundInterface;
    hsgResMgr::ResMgr()->NewKey(M2ST(node->GetName()), fBound, node->GetLocation(), node->GetLoadMask());
    fBound->Init(plMeshConverter::Instance().CreateConvexVolume(node));
    hsgResMgr::ResMgr()->AddViaNotify(fBound->GetKey(), new plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
}


void plParticleVolumeComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
    plParticleCollisionEffect *effect = nullptr;
    if( !fEffect )
    {
        plMaxNode *source = (plMaxNode *)fCompPB->GetINode(kSourceNode);

        if (source == nullptr || !source->CanConvert())
            return; // No source selected, user has already been warned.

        BuildVolume(source);
        switch( fCompPB->GetInt(kOnImpact) )
        {
        default:
        case kImpDefault:
            effect = new plParticleCollisionEffectBeat();
            break;
        case kImpDie:
            effect = new plParticleCollisionEffectDie();
            break;
        case kImpBounce:
            {
                plParticleCollisionEffectBounce* bnc = new plParticleCollisionEffectBounce();
                bnc->SetBounce(fCompPB->GetFloat(kBounceAmt) * 1.e-2f);
                bnc->SetFriction(fCompPB->GetFloat(kFrictionAmt) * 1.e-2f);
                effect = bnc;
            }
            break;
        };

        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());    
        plSceneObject *sObj = source->GetSceneObject();
        hsgResMgr::ResMgr()->AddViaNotify( sObj->GetKey(), new plGenRefMsg( effect->GetKey(), plRefMsg::kOnCreate, 0, 0 ), plRefFlags::kPassiveRef );

        fEffect = effect;
    }
    else
    {
        effect = plParticleCollisionEffect::ConvertNoRef(fEffect);
    }
    hsAssert(effect, "Our effect pointer was wrong type?");
    hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectConstraint ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleVolumeComponent, gParticleVolumeDesc, "Collision Volume Effect",  "Collision Volume Effect", COMP_TYPE_PARTICLE, PARTICLE_VOLUME_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleVolumeBk
(   
    plComponent::kBlkComp, _T("ParticleVolume"), 0, &gParticleVolumeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_PARTICLE_VOLUME, IDS_COMP_PARTICLE_VOLUME_ROLL, 0, 0, nullptr,

    plParticleVolumeComponent::kSourceNode, _T("SourceINode"),  TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_PARTICLE_VOLUME_NODE,
        p_sclassID, GEOMOBJECT_CLASS_ID,
        //p_prompt, IDS_COMP_ONESHOT_STARTS,
        //p_accessor, &gOneShotAccessor,
        p_end,

    plParticleVolumeComponent::kOnImpact, _T("OnImpact"),       TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_COMP_PARTICLE_VOL_DEFAULT,                  IDC_COMP_PARTICLE_VOL_DIE,              IDC_COMP_PARTICLE_VOL_BOUNCE,   
        p_vals,                     plParticleVolumeComponent::kImpDefault, plParticleVolumeComponent::kImpDie, plParticleVolumeComponent::kImpBounce,      
        p_default, plParticleVolumeComponent::kImpDefault,
        p_end,

    plParticleVolumeComponent::kBounceAmt, _T("BounceAmt"), TYPE_FLOAT,     0, 0,   
        p_default, 100.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_VOL_BOUNCEAMT, IDC_COMP_PARTICLE_VOL_BOUNCEAMT_SPIN, 1.0,
        p_end,    
    
    plParticleVolumeComponent::kFrictionAmt, _T("FrictionAmt"), TYPE_FLOAT,     0, 0,   
        p_default, 0.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_VOL_FRICTIONAMT, IDC_COMP_PARTICLE_VOL_FRICTIONAMT_SPIN, 1.0,
        p_end,    

    p_end
);

plParticleVolumeComponent::plParticleVolumeComponent()
{
    fClassDesc = &gParticleVolumeDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleWindComponent

static hsVector3 IGetRefDir(plMaxNode* node, INode* refNode, float clampAngDeg)
{
    clampAngDeg *= 0.5f;
    float vecLen = 100.f;
    if( clampAngDeg > 1.f )
    {
        float rads = hsDegreesToRadians(clampAngDeg);
        float sinAng = sinf(rads);

        hsAssert(sinAng > 0.01, "Trig confusion?");

#if 0 
        float cosAng = cosf(rads);
        if( cosAng < 0.01f )
            vecLen = 0;
        else
            vecLen = cosAng / sinAng;
#else
        vecLen = 1.f / sinAng;
#endif

    }
    if( !refNode )
        refNode = node;

    Matrix3 nodeTM = refNode->GetNodeTM(TimeValue(0));
    Point3 dir = nodeTM.GetRow(1);
    dir = FNormalize(dir);

    hsVector3 refDir(dir.x * vecLen, dir.y * vecLen, dir.z * vecLen);

    return refDir;
}


void plParticleWindComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
    plParticleLocalWind* effect = nullptr;
    if( !fEffect )
    {
        effect = new plParticleLocalWind();
        effect->SetScale(hsVector3(fCompPB->GetFloat(kScaleX), fCompPB->GetFloat(kScaleY), fCompPB->GetFloat(kScaleZ)));
        effect->SetSpeed(fCompPB->GetFloat(kSpeed));

        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());

        effect->SetStrength(fCompPB->GetFloat(kStrength));
        effect->SetSwirl(fCompPB->GetFloat(kSwirl) * 1.e-2f);
        effect->SetHorizontal(fCompPB->GetInt(kHorizontal));
        effect->SetConstancy(fCompPB->GetFloat(kConstancy) * 1.e-2f);

        effect->SetRefDirection(IGetRefDir(node, fCompPB->GetINode(kRefObject), fCompPB->GetFloat(kClampAngle)));

        fEffect = effect;
    }
    else
    {
        effect = plParticleLocalWind::ConvertNoRef(fEffect);
    }
    hsAssert(effect, "Our effect pointer was wrong type?");
    hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectForce ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleWindComponent, gParticleWindDesc, "Wind Effect",  "WindEffect", COMP_TYPE_PARTICLE, PARTICLE_WIND_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleWindBk
(   

    plComponent::kBlkComp, _T("ParticleWind"), 0, &gParticleWindDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_PARTICLE_WIND, IDS_COMP_PARTICLE_WIND, 0, 0, nullptr,

    plParticleWindComponent::kScaleX,   _T("ScaleX"),   TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 25.f,
        p_range, 0.f, 1000.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_SCALEX, IDC_COMP_PARTICLE_WIND_SCALEX_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kScaleY,   _T("ScaleY"),   TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 25.f,
        p_range, 0.f, 1000.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_SCALEY, IDC_COMP_PARTICLE_WIND_SCALEY_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kScaleZ,   _T("ScaleZ"),   TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 0.f,
        p_range, 0.f, 1000.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_SCALEZ, IDC_COMP_PARTICLE_WIND_SCALEZ_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kSpeed,    _T("Speed"),    TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 32.f,
        p_range, -100.f, 100.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_FLOAT, 
        IDC_COMP_PARTICLE_WIND_SPEED, IDC_COMP_PARTICLE_WIND_SPEED_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kStrength, _T("Strength"), TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 30.f,
        p_range, 0.f, 100.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_STRENGTH, IDC_COMP_PARTICLE_WIND_STRENGTH_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kConstancy,    _T("Constancy"),    TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 0.f,
        p_range, -75.f, 300.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_FLOAT, 
        IDC_COMP_PARTICLE_WIND_CONSTANCY, IDC_COMP_PARTICLE_WIND_CONSTANCY_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kSwirl,    _T("Swirl"),    TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 100.f,
        p_range, 0.f, 100.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_SWIRL, IDC_COMP_PARTICLE_WIND_SWIRL_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kHorizontal, _T("Horizontal"),     TYPE_BOOL, 0, 0, 
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_WIND_HORIZONTAL,
        p_end,

    plParticleWindComponent::kLocalize, _T("Localize"),     TYPE_BOOL, 0, 0, 
        p_default, TRUE,
        p_end,

    plParticleWindComponent::kClampAngle, _T("ClampAngle"), TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 180.f,
        p_range, 0.f, 180.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_CLAMPANG, IDC_COMP_PARTICLE_WIND_CLAMPANG_SPIN, 1.0,
        p_end,

    plParticleWindComponent::kRefObject, _T("RefObject"),   TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_PARTICLE_WIND_REFOBJECT,
        p_prompt, IDS_COMP_CHOOSE_OBJECT,
        p_end,

    p_end
);

plParticleWindComponent::plParticleWindComponent()
{
    fClassDesc = &gParticleWindDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleUniWindComponent

void plParticleUniWindComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
    plParticleUniformWind* effect = nullptr;
    if( !fEffect )
    {
        effect = new plParticleUniformWind();
        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());

        effect->SetStrength(fCompPB->GetFloat(kStrength));
        effect->SetSwirl(fCompPB->GetFloat(kSwirl) * 1.e-2f);
        effect->SetHorizontal(fCompPB->GetInt(kHorizontal));
        effect->SetConstancy(fCompPB->GetFloat(kConstancy) * 1.e-2f);

        effect->SetFrequencyRange(fCompPB->GetFloat(kMinSecs), fCompPB->GetFloat(kMaxSecs));
        effect->SetFrequencyRate(fCompPB->GetFloat(kRate));

        effect->SetRefDirection(IGetRefDir(node, fCompPB->GetINode(kRefObject), fCompPB->GetFloat(kClampAngle)));

        fEffect = effect;
    }
    else
    {
        effect = plParticleUniformWind::ConvertNoRef(fEffect);
    }
    hsAssert(effect, "Our effect pointer was wrong type?");
    hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectForce ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleUniWindComponent, gParticleUniWindDesc, "Uniform Wind",  "UniWind", COMP_TYPE_PARTICLE, PARTICLE_UNIWIND_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleUniWindBk
(   

    plComponent::kBlkComp, _T("ParticleUniWind"), 0, &gParticleUniWindDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    //Roll out
    IDD_COMP_PARTICLE_UNIWIND, IDS_COMP_PARTICLE_UNIWIND, 0, 0, nullptr,

    plParticleUniWindComponent::kStrength,  _T("Strength"), TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 30.f,
        p_range, 0.f, 100.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_STRENGTH, IDC_COMP_PARTICLE_WIND_STRENGTH_SPIN, 1.0,
        p_end,

    plParticleUniWindComponent::kConstancy, _T("Constancy"),    TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 0.f,
        p_range, -75.f, 300.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_FLOAT, 
        IDC_COMP_PARTICLE_WIND_CONSTANCY, IDC_COMP_PARTICLE_WIND_CONSTANCY_SPIN, 1.0,
        p_end,

    plParticleUniWindComponent::kSwirl, _T("Swirl"),    TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 100.f,
        p_range, 0.f, 100.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_SWIRL, IDC_COMP_PARTICLE_WIND_SWIRL_SPIN, 1.0,
        p_end,

    plParticleUniWindComponent::kHorizontal, _T("Horizontal"),      TYPE_BOOL, 0, 0,  
        p_default, FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_WIND_HORIZONTAL,
        p_end,

    plParticleUniWindComponent::kMinSecs,   _T("MinSecs"),  TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 1.f,
        p_range, 0.1f, 20.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_MINSECS, IDC_COMP_PARTICLE_WIND_MINSECS_SPIN, 1.0,
        p_end,

    plParticleUniWindComponent::kMaxSecs,   _T("MaxSecs"),  TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 10.f,
        p_range, 0.1f, 30.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_MAXSECS, IDC_COMP_PARTICLE_WIND_MAXSECS_SPIN, 1.0,
        p_end,

    plParticleUniWindComponent::kRate,  _T("Rate"), TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 10.f,
        p_range, 0.1f, 50.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_RATE, IDC_COMP_PARTICLE_WIND_RATE_SPIN, 1.0,
        p_end,

    plParticleUniWindComponent::kClampAngle, _T("ClampAngle"),  TYPE_FLOAT,     P_ANIMATABLE, 0,    
        p_default, 180.f,
        p_range, 0.f, 180.f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_PARTICLE_WIND_CLAMPANG2, IDC_COMP_PARTICLE_WIND_CLAMPANG_SPIN2, 1.0,
        p_end,

    plParticleUniWindComponent::kRefObject, _T("RefObject"),    TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_PARTICLE_WIND_REFOBJECT,
        p_prompt, IDS_COMP_CHOOSE_OBJECT,
        p_end,

    p_end
);

plParticleUniWindComponent::plParticleUniWindComponent()
{
    fClassDesc = &gParticleUniWindDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleFlockComponent

class ParticleFlockEffectDlgProc : public ParamMap2UserDlgProc
{
public:
    ParticleFlockEffectDlgProc() {}
    ~ParticleFlockEffectDlgProc() {}

    void IValidateSpinners(TimeValue t, IParamBlock2 *pb, IParamMap2 *map, uint32_t id)
    {
        uint32_t minIndex, maxIndex;
        bool adjustMin;
        switch(id)
        {
        case IDC_FLOCK_GOAL_DIST:
        case IDC_FLOCK_GOAL_DIST_SPIN:
            minIndex = plParticleFlockComponent::kGoalDist; maxIndex = plParticleFlockComponent::kFullChaseDist; adjustMin = false;
            break;
        case IDC_FLOCK_FULL_CHASE_DIST:
        case IDC_FLOCK_FULL_CHASE_DIST_SPIN:
            minIndex = plParticleFlockComponent::kGoalDist; maxIndex = plParticleFlockComponent::kFullChaseDist; adjustMin = true;
            break;
        default:
            return;
        }

        float min, max;
        min = pb->GetFloat(minIndex, t);
        max = pb->GetFloat(maxIndex, t);

        if (min > max)
        {
            if (adjustMin)
                pb->SetValue(minIndex, t, max);
            else
                pb->SetValue(maxIndex, t, min);

            map->Invalidate(minIndex);
            map->Invalidate(maxIndex);
        }
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        int id = LOWORD(wParam);

        IParamBlock2 *pb = map->GetParamBlock();

        switch (msg)
        {
        case WM_COMMAND:  
        case CC_SPINNER_CHANGE:
            IValidateSpinners(t, pb, map, id);
            return TRUE;
        }
        return FALSE;
    }
    void DeleteThis() override { }
};
static ParticleFlockEffectDlgProc gParticleFlockEffectDlgProc;



void plParticleFlockComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
    plParticleFlockEffect* effect = nullptr;
    if( !fEffect )
    {
        effect = new plParticleFlockEffect();
        hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());

        hsPoint3 offset(fCompPB->GetFloat(ParamID(kOffsetX)),
                        fCompPB->GetFloat(ParamID(kOffsetY)),
                        fCompPB->GetFloat(ParamID(kOffsetZ)));
        effect->SetTargetOffset(offset);
        effect->SetInfluenceAvgRadius(fCompPB->GetFloat(ParamID(kInfAvgDist)));
        effect->SetInfluenceRepelRadius(fCompPB->GetFloat(ParamID(kInfRepDist)));

        float goalDist = fCompPB->GetFloat(ParamID(kGoalDist));
        float fcDist = fCompPB->GetFloat(ParamID(kFullChaseDist));   
        effect->SetGoalRadius(goalDist);
        effect->SetFullChaseRadius(goalDist > fcDist ? goalDist : fcDist); // Fix old data

        effect->SetConformStr(fCompPB->GetFloat(ParamID(kInfAvgStr)));
        effect->SetRepelStr(fCompPB->GetFloat(ParamID(kInfRepStr)));
        effect->SetGoalOrbitStr(fCompPB->GetFloat(ParamID(kGoalOrbitStr)));
        effect->SetGoalChaseStr(fCompPB->GetFloat(ParamID(kGoalChaseStr)));
        effect->SetMaxChaseSpeed(fCompPB->GetFloat(ParamID(kMaxChaseSpeed)));
        effect->SetMaxOrbitSpeed(fCompPB->GetFloat(ParamID(kMaxOrbitSpeed)));
        effect->SetMaxParticles(sys->GetMaxTotalParticles());

        fEffect = effect;
    }
    else
    {
        effect = plParticleFlockEffect::ConvertNoRef(fEffect);
    }
    hsAssert(effect, "Our effect pointer was wrong type?");
    hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), new plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectForce ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleFlockComponent, gParticleFlockDesc, "Particle Flock",  "Flock", COMP_TYPE_PARTICLE, PARTICLE_FLOCK_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleFlockBk
(   
 
    plComponent::kBlkComp, _T("ParticleFlock"), 0, &gParticleFlockDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,
    
    //Roll out
    IDD_COMP_PARTICLE_FLOCK, IDS_COMP_PARTICLE_FLOCK, 0, 0, &gParticleFlockEffectDlgProc,
    
    plParticleFlockComponent::kOffsetX, _T("OffsetX"),  TYPE_FLOAT, 0, 0,   
        p_default, 0.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_TARGET_OFFSETX, IDC_FLOCK_TARGET_OFFSETX_SPIN, 1.0,
        p_end,
    
    plParticleFlockComponent::kOffsetY, _T("OffsetY"),  TYPE_FLOAT, 0, 0,   
        p_default, 0.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_TARGET_OFFSETY, IDC_FLOCK_TARGET_OFFSETY_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kOffsetZ, _T("OffsetZ"),  TYPE_FLOAT, 0, 0,   
        p_default, 0.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_TARGET_OFFSETZ, IDC_FLOCK_TARGET_OFFSETZ_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kInfAvgDist,  _T("InfAvgDist"),   TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_CONFORM_DIST, IDC_FLOCK_CONFORM_DIST_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kInfRepDist,  _T("InfRepDist"),   TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_REPEL_DIST, IDC_FLOCK_REPEL_DIST_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kGoalDist,    _T("GoalDist"), TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_GOAL_DIST, IDC_FLOCK_GOAL_DIST_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kInfAvgStr,   _T("InfAvgStr"),    TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_CONFORM_STR, IDC_FLOCK_CONFORM_STR_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kInfRepStr,   _T("InfRepStr"),    TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_REPEL_STR, IDC_FLOCK_REPEL_STR_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kGoalOrbitStr,    _T("GoalStr"),  TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_GOAL_STR, IDC_FLOCK_GOAL_STR_SPIN, 1.0,
        p_end,

    plParticleFlockComponent::kMaxChaseSpeed, _T("MaxSpeed"),   TYPE_FLOAT, 0, 0,   
        p_default, 100.0,
        p_range, 0.0, 999.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_MAX_SPEED, IDC_FLOCK_MAX_SPEED_SPIN, 1.0,
        p_end,        
    
    plParticleFlockComponent::kGoalChaseStr,    _T("GoalChaseStr"), TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_GOAL_CHASE_STR, IDC_FLOCK_GOAL_CHASE_STR_SPIN, 1.0,
        p_end,
    
    plParticleFlockComponent::kMaxOrbitSpeed, _T("MaxOrbitSpeed"),  TYPE_FLOAT, 0, 0,   
        p_default, 20.0,
        p_range, 0.0, 999.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_MAX_ORBIT_SPEED, IDC_FLOCK_MAX_ORBIT_SPEED_SPIN, 1.0,
        p_end,        
        
    plParticleFlockComponent::kFullChaseDist,   _T("FullChaseDist"),    TYPE_FLOAT, 0, 0,   
        p_default, 1.0,
        p_range, 0.0, 100.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_FLOCK_FULL_CHASE_DIST, IDC_FLOCK_FULL_CHASE_DIST_SPIN, 1.0,
        p_end,

    p_end
);

plParticleFlockComponent::plParticleFlockComponent()
{
    fClassDesc = &gParticleFlockDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

