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
#include "HeadSpin.h"
#include <windowsx.h>

#include "max.h"
#include "resource.h"
#include "plComponent.h"
#include "plComponentReg.h"
#include "plParticleComponents.h"
#include "plAnimComponent.h"
#include "plNotetrackAnim.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../plScene/plSceneNode.h"
#include "plgDispatch.h"

#include "../MaxConvert/plConvert.h"
#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxConvert/plMeshConverter.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxMain/plMaxNode.h"
#include "../MaxPlasmaMtls/Materials/plParticleMtl.h"

#include "../MaxExport/plErrorMsg.h"

#include "hsResMgr.h"

#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../plInterp/plController.h"
#include "../plInterp/hsInterp.h"
#include "../plInterp/plAnimEaseTypes.h"
#include "../MaxMain/plMaxNode.h"
#include "../pnKeyedObject/plKey.h"

#include "../plSurface/hsGMaterial.h"
#include "../plPipeline/plGBufferGroup.h"

#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plParticleEmitter.h"
#include "../plParticleSystem/plParticleEffect.h"
#include "../plParticleSystem/plParticleGenerator.h"
#include "../plParticleSystem/plParticleApplicator.h"
#include "../plParticleSystem/plConvexVolume.h"
#include "../plParticleSystem/plBoundInterface.h"

#include "../plAvatar/plScalarChannel.h"
#include "../plAvatar/plAGAnim.h"

#include "../pnSceneObject/plDrawInterface.h"

#include "../plGLight/plLightInfo.h"
#include "plLightGrpComponent.h"

void DummyCodeIncludeFuncParticles()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// stuff for plParticleComponent

const char *plParticleCoreComponent::GenStrings[] = // line these up with the generation types enum.
{
	"Point Source",
	"Mesh",
	"One Per Vertex"
};

hsBool plParticleCoreComponent::IsParticleSystemComponent(plComponentBase *comp)
{
	if (comp->ClassID() == PARTICLE_SYSTEM_COMPONENT_CLASS_ID)
		return true;

	return false;
}

hsBool plParticleCoreComponent::NodeHasSystem(plMaxNode *pNode)
{
	int i;
	for (i = 0; i < pNode->NumAttachedComponents(); i++)
	{
		if (plParticleCoreComponent::IsParticleSystemComponent(pNode->GetAttachedComponent(i)))
			return true;
	}
	return false;
}

hsBool plParticleCoreComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	GetParamVals( pNode );
	pNode->SetForceLocal(true);
	pNode->SetDrawable(false);
	pNode->SetParticleRelated(true);

	Mtl *maxMaterial = hsMaterialConverter::Instance().GetBaseMtl(pNode);
	plConvert &convert = plConvert::Instance();
	if (!hsMaterialConverter::IsParticleMat(maxMaterial))
	{
		maxMaterial = nil;
		pNode->SetMtl(NULL);
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
	plSceneNode	*sNode = plSceneNode::ConvertNoRef( pNode->GetRoomKey()->GetObjectPtr() );
	plDrawInterface *di = TRACKED_NEW plDrawInterface;
	hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), di, pNode->GetLocation(), pNode->GetLoadMask());
	hsgResMgr::ResMgr()->AddViaNotify( di->GetKey(), TRACKED_NEW plObjRefMsg(pNode->GetSceneObject()->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );
	pNode->SetDISceneNodeSpans(di, true);

	return true;
}

hsBool plParticleCoreComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{	
	Int32 i, j, k;

	plLocation nodeLoc = node->GetKey()->GetUoid().GetLocation();
	const char *objName = node->GetKey()->GetName();

	plSceneObject *sObj = node->GetSceneObject();
	plParticleSystem *sys = TRACKED_NEW plParticleSystem();

	hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), sys, nodeLoc, node->GetLoadMask());	

	// Add material and lifespan animated params.
	Mtl *maxMaterial = hsMaterialConverter::Instance().GetBaseMtl(node);
	hsTArray<hsGMaterial *> matArray;
	hsMaterialConverter::Instance().GetMaterialArray(maxMaterial, node, matArray);
	hsGMaterial* particleMat = matArray[0];
	
	plController *ambientCtl = nil;
	plController *diffuseCtl = nil;
	plController *opacityCtl = nil;	
	plController *widthCtl = nil;
	plController *heightCtl = nil;
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

	hsScalar genLife		= -1;
	hsScalar partLifeMin, partLifeMax;
	hsScalar pps			= fUserInput.fPPS; 
	hsPoint3 pos(0, 0, 0);
	hsScalar pitch			= PI;
	hsScalar yaw			= 0; 
	hsScalar angleRange		= fUserInput.fConeAngle * PI / 180.f;
	hsScalar velMin			= fUserInput.fVelocityMin;
	hsScalar velMax			= fUserInput.fVelocityMax;
	hsScalar xSize			= fUserInput.fHSize;
	hsScalar ySize			= fUserInput.fVSize;
	hsScalar scaleMin		= fUserInput.fScaleMin / 100.0f;
	hsScalar scaleMax		= fUserInput.fScaleMax / 100.0f;
	hsScalar gravity		= fUserInput.fGravity / 100.0f;
	hsScalar drag			= fUserInput.fDrag / 100.f;
	hsScalar windMult		= fUserInput.fWindMult / 100.f;
	hsScalar massRange		= fUserInput.fMassRange;
	hsScalar rotRange		= fUserInput.fRotRange * PI / 180.f;

	UInt32 xTiles = fUserInput.fXTiles; 
	UInt32 yTiles = fUserInput.fYTiles; 
	UInt32 maxEmitters = 1 + GetEmitterReserve();
	UInt32 maxTotalParticles = 0;	

	// Need to do this even when immortal, so that maxTotalParticles is computed correctly.
	partLifeMin = fUserInput.fLifeMin;
	partLifeMax = fUserInput.fLifeMax;
	plLeafController *ppsCtl = cc.MakeScalarController(fCompPB->GetController(ParamID(kPPS)), node);
	if (ppsCtl != nil && ppsCtl->GetLength() > 0)
	{
		// Simulate just the birth across the curve and record the max
		hsScalar frameDelta = (1.f / MAX_FRAMES_PER_SEC); 
		hsScalar avgLife = (partLifeMax + partLifeMin) / 2;
		UInt32 count = node->NumAttachedComponents();
		UInt32 lifeTicks = avgLife / frameDelta;
		hsScalar *birth = TRACKED_NEW hsScalar[lifeTicks];

		// Find any anim components attached to the same node.
		for (i = 0; i < count; i++)
		{
			if (!plAnimComponentBase::IsAnimComponent(node->GetAttachedComponent(i)))
				continue;

			hsScalar maxAnimParticles = 0;

			plAnimComponentBase *comp = (plAnimComponentBase *)node->GetAttachedComponent(i);
			plATCAnim *anim = plATCAnim::ConvertNoRef(comp->fAnims[node]);

			// If it's an ATC anim, we can be aggressive in determining the max
			if (anim)
			{			
				hsScalar curAnimParticles = 0;

				hsScalar loopStart, loopEnd;

				for (j = -1; j < (Int32)anim->GetNumLoops(); j++)
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

					hsScalar loopLength = loopEnd - loopStart;

					if (loopLength == 0) // It's the default "(Entire Animation)"
						loopLength = ppsCtl->GetLength();

					UInt32 loopTicks = loopLength * MAX_FRAMES_PER_SEC;

					UInt32 startTick = loopStart * MAX_FRAMES_PER_SEC;
					UInt32 tick;
					for (tick = 0; tick < loopTicks + lifeTicks; tick++)
					{
						curAnimParticles -= birth[tick % lifeTicks] * frameDelta;
						hsScalar birthStart = 0.f;
						hsScalar birthEnd = 0.f;
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
				hsScalar maxPps = 0;
				int i;
				for (i = 1; i < ppsCtl->GetNumKeys(); i++)
				{
					hsScalar curVal = 0;
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
				maxTotalParticles = (UInt32)maxAnimParticles;
		}
		delete [] birth;
	}
	else
	{
		maxTotalParticles = pps * (partLifeMax - (partLifeMax - partLifeMin) / 2);
	}
	maxTotalParticles *=  maxEmitters;

	delete ppsCtl;
	ppsCtl = nil;
	
	UInt32 maxAllowedParticles = plGBufferGroup::kMaxNumIndicesPerBuffer / 6;
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
		partLifeMin		= -1.0;
		partLifeMax		= -1.0;
	}

	// Figure out the appropriate generator to add
	plParticleGenerator *generator = nil;
	UInt32 sources;
	hsScalar *pitchArray;
	hsScalar *yawArray;
	hsPoint3 *pointArray;
	hsVector3 *dirArray;
	if (fUserInput.fGenType == kGenPoint)
	{
		sources = 1;
		pitchArray = TRACKED_NEW hsScalar[sources];
		yawArray = TRACKED_NEW hsScalar[sources];
		pointArray = TRACKED_NEW hsPoint3[sources];
		pitchArray[0] = pitch;
		yawArray[0] = yaw;
		pointArray[0].Set(0, 0, 0);
		plSimpleParticleGenerator *gen = TRACKED_NEW plSimpleParticleGenerator();
		gen->Init(genLife, partLifeMin, partLifeMax, pps, sources, pointArray, pitchArray, yawArray, angleRange, 
				  velMin, velMax, xSize, ySize, scaleMin, scaleMax, massRange, rotRange);
		generator = gen;
	}
	else if (fUserInput.fGenType == kGenMesh)
	{
		hsTArray<hsVector3> normals;
		hsTArray<hsPoint3> pos;
		plMeshConverter::Instance().StuffPositionsAndNormals(node, &pos, &normals);
		sources = normals.GetCount();
		pitchArray = TRACKED_NEW hsScalar[sources];
		yawArray = TRACKED_NEW hsScalar[sources];
		pointArray = TRACKED_NEW hsPoint3[sources];
		int i;
		for (i = 0; i < sources; i++)
		{
			plParticleGenerator::ComputePitchYaw(pitchArray[i], yawArray[i], normals.Get(i));
			pointArray[i] = pos.Get(i);
		}
		plSimpleParticleGenerator *gen = TRACKED_NEW plSimpleParticleGenerator();
		gen->Init(genLife, partLifeMin, partLifeMax, pps, sources, pointArray, pitchArray, yawArray, angleRange, 
				  velMin, velMax, xSize, ySize, scaleMin, scaleMax, massRange, rotRange);
		generator = gen;
	}
	else // One per vertex
	{
		hsTArray<hsVector3> normals;
		hsTArray<hsPoint3> pos;
		plMeshConverter::Instance().StuffPositionsAndNormals(node, &pos, &normals);
		sources = normals.GetCount();

		pointArray = TRACKED_NEW hsPoint3[sources];
		dirArray = TRACKED_NEW hsVector3[sources];
		int i;
		for (i = 0; i < sources; i++)
		{
			dirArray[i] = normals.Get(i);
			pointArray[i] = pos.Get(i);
		}

		plOneTimeParticleGenerator *gen = TRACKED_NEW plOneTimeParticleGenerator();
		gen->Init(sources, pointArray, dirArray, xSize, ySize, scaleMin, scaleMax, rotRange);
		generator = gen;
		maxTotalParticles = sources;
		gravity = 0.f;
	}

	// Init and attach to the scene object
	sys->Init(xTiles, yTiles, maxTotalParticles, maxEmitters, ambientCtl, diffuseCtl, opacityCtl, 
			  widthCtl, heightCtl);
	hsgResMgr::ResMgr()->AddViaNotify( particleMat->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, 0 ), plRefFlags::kActiveRef );
	hsgResMgr::ResMgr()->AddViaNotify( sys->GetKey(), TRACKED_NEW plObjRefMsg( sObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );

	// Set up normals and orientation
	UInt32 miscFlags = 0;
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
		plParticleFollowSystemEffect *effect = TRACKED_NEW plParticleFollowSystemEffect;
		hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());
		hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectMisc ), plRefFlags::kActiveRef );
	}

	return true;
}

void plParticleCoreComponent::IHandleLights(plLightGrpComponent* liGrp, plParticleSystem* sys)
{
	const hsTArray<plLightInfo*>& liInfo = liGrp->GetLightInfos();
	int i;
	for( i = 0; i < liInfo.GetCount(); i++ )
	{
		sys->AddLight(liInfo[i]->GetKey());
	}
}

hsBool plParticleCoreComponent::AddToAnim(plAGAnim *anim, plMaxNode *node)
{
	hsBool result = false;
	plController *ctl;
	hsControlConverter& cc = hsControlConverter::Instance();

	hsScalar start, end;
	if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
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
		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kLifeMin)), node, start, end);
		if (ctl != nil)
		{
			plParticleLifeMinApplicator *app = TRACKED_NEW plParticleLifeMinApplicator();
			app->SetChannelName(node->GetName());
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}

		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kLifeMax)), node, start, end);
		if (ctl != nil)
		{
			plParticleLifeMaxApplicator *app = TRACKED_NEW plParticleLifeMaxApplicator();
			app->SetChannelName(node->GetName());
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}

		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kPPS)), node, start, end);
		if (ctl != nil)
		{
			plParticlePPSApplicator *app = TRACKED_NEW plParticlePPSApplicator();
			app->SetChannelName(node->GetName());
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}

		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kConeAngle)), node, start, end);
		if (ctl != nil)
		{
			plParticleAngleApplicator *app = TRACKED_NEW plParticleAngleApplicator();
			app->SetChannelName(node->GetName());
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}

		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kVelocityMin)), node, start, end);
		if (ctl != nil)
		{
			plParticleVelMinApplicator *app = TRACKED_NEW plParticleVelMinApplicator();
			app->SetChannelName(node->GetName());
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}

		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kVelocityMax)), node, start, end); 
		if (ctl != nil)
		{
			plParticleVelMaxApplicator *app = TRACKED_NEW plParticleVelMaxApplicator();
			app->SetChannelName(node->GetName());
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}
		
		/*
		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kGravity)), node, start, end);
		if (ctl != nil)
		{
			plParticleGravityApplicator *app = TRACKED_NEW plParticleGravityApplicator();
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}

		ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kDrag)), node, start, end);
		if (ctl != nil)
		{
			plParticleDragApplicator *app = TRACKED_NEW plParticleDragApplicator();
			plAnimComponentBase::SetupCtl(anim, ctl, app, node);
			result = true;		
		}
		*/
	}

	ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kScaleMin)), node, start, end);
	if (ctl != nil)
	{
		plParticleScaleMinApplicator *app = TRACKED_NEW plParticleScaleMinApplicator();
		app->SetChannelName(node->GetName());
		plAnimComponentBase::SetupCtl(anim, ctl, app, node);
		result = true;		
	}

	ctl = cc.MakeScalarController(fCompPB->GetController(ParamID(kScaleMax)), node, start, end);
	if (ctl != nil)
	{
		plParticleScaleMaxApplicator *app = TRACKED_NEW plParticleScaleMaxApplicator();
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

CLASS_DESC(plParticleComponent, gParticleDesc, "Particle System",  "ParticleSystem", COMP_TYPE_PARTICLE, PARTICLE_SYSTEM_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleBk
(	

	plComponent::kBlkComp, _T("Particle"), 0, &gParticleDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PARTICLE, IDS_COMP_PARTICLE_ROLL, 0, 0, &gParticleCompDlgProc,

	//Particle Properties....

	plParticleCoreComponent::kGenType,		_T("Generation"),	TYPE_INT, 0, 0,
		p_default, 0,
		end,

	plParticleCoreComponent::kConeAngle,		_T("ConeAngle"),	TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_CONE_ANGLE,	
		p_default, 45.0,
		p_range, 0.0, 180.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_CONE, IDC_COMP_PARTICLE_CONE_SPIN, 1.0,
		end,

	plParticleCoreComponent::kVelocityMin,	_T("VelocityMin"),	TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_VELOCITY_MIN,	
		p_default, 50.0,
		p_range, 0.0, 500.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_VELMIN, IDC_COMP_PARTICLE_VELMIN_SPIN, 1.0,
		end,

	plParticleCoreComponent::kVelocityMax,	_T("VelocityMax"),	TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_VELOCITY_MAX,	
		p_default, 50.0,
		p_range, 0.0, 500.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_VELMAX, IDC_COMP_PARTICLE_VELMAX_SPIN, 1.0,
		end,

	plParticleCoreComponent::kLifeMin,		_T("LifeMin"),		TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_LIFE_MIN,	
		p_default, 10.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_LIFEMIN, IDC_COMP_PARTICLE_LIFEMIN_SPIN, 1.0,
		end,

	plParticleCoreComponent::kLifeMax,		_T("LifeMax"),		TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_LIFE_MAX,	
		p_default, 5.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_LIFEMAX, IDC_COMP_PARTICLE_LIFEMAX_SPIN, 1.0,
		end,

	plParticleCoreComponent::kImmortal, _T("Immortal"),		TYPE_BOOL,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_NODIE,
		end,

	plParticleCoreComponent::kPPS,	_T("PPS"),	TYPE_FLOAT, 	P_ANIMATABLE, IDS_PARTICLE_PPS,	
		p_default, 20.0,
		p_range, 0.0, 5000.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_PPS, IDC_COMP_PARTICLE_PPS_SPIN, 1.0,
		end,

	plParticleCoreComponent::kScaleMin,	_T("ScaleMin"),	TYPE_INT, 	P_ANIMATABLE, IDS_PARTICLE_SCALE_MIN,	
		p_default, 100,
		p_range, 1, 1000,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_COMP_PARTICLE_SCALEMIN, IDC_COMP_PARTICLE_SCALEMIN_SPIN, 1.0,
		end,

	plParticleCoreComponent::kScaleMax,	_T("ScaleMax"),	TYPE_INT, 	P_ANIMATABLE, IDS_PARTICLE_SCALE_MAX,	
		p_default, 100,
		p_range, 1, 1000,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_COMP_PARTICLE_SCALEMAX, IDC_COMP_PARTICLE_SCALEMAX_SPIN, 1.0,
		end,

	plParticleCoreComponent::kGravity,	_T("Gravity"),	TYPE_INT, 	0, 0,	
		p_default, 100,
		p_range, -100, 100,
		p_ui,	TYPE_SPINNER,	EDITTYPE_INT,	
		IDC_COMP_PARTICLE_GRAVITY, IDC_COMP_PARTICLE_GRAVITY_SPIN, 1.0,
		end,

	plParticleCoreComponent::kDrag,		_T("Drag"),		TYPE_INT, 	0, 0,	
		p_default, 0,
		p_range, 0, 1000,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_COMP_PARTICLE_DRAG, IDC_COMP_PARTICLE_DRAG_SPIN, 1.0,
		end,

	plParticleCoreComponent::kPreSim,	_T("PreSim"),	TYPE_INT, 	0, 0,	
		p_default, 0,
		p_range, 0, 100,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_COMP_PARTICLE_PRESIM, IDC_COMP_PARTICLE_PRESIM_SPIN, 1.0,
		end,

	plParticleCoreComponent::kWindMult,		_T("WindMult"),		TYPE_INT, 	0, 0,	
		p_default, 100,
		p_range, 0, 1000,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_COMP_PARTICLE_WIND_MULT, IDC_COMP_PARTICLE_WIND_MULT_SPIN, 1.0,
		end,

	plParticleCoreComponent::kMassRange,		_T("MassRange"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.f,
		p_range, 0.f, 1000.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_MASS_RANGE, IDC_COMP_PARTICLE_MASS_RANGE_SPIN, 1.0,
		end,

	plParticleCoreComponent::kRotRange,		_T("RotRange"),		TYPE_FLOAT, 	0, 0,	
		p_default, 0.f,
		p_range, 0.f, 180.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_ROT_RANGE, IDC_COMP_PARTICLE_ROT_RANGE_SPIN, 1.0,
		end,
		
	plParticleCoreComponent::kFollowSystem, _T("FollowSystem"),		TYPE_BOOL,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_FOLLOW_SYSTEM,
		end,		

	end
);

bool plParticleComponent::fAllowUnhide = false;

plParticleComponent::plParticleComponent()
:	fEmitterReserve(0)
{
	fClassDesc = &gParticleDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

// Gets values from the ParamBlock and passes them to the Convert
hsBool plParticleComponent::GetParamVals(plMaxNode *pNode)
{
	fUserInput.fGenType			= fCompPB->GetInt(kGenType);
	fUserInput.fConeAngle		= fCompPB->GetFloat(kConeAngle);
	fUserInput.fVelocityMin		= fCompPB->GetFloat(kVelocityMin);
	fUserInput.fVelocityMax		= fCompPB->GetFloat(kVelocityMax);
	fUserInput.fLifeMin			= fCompPB->GetFloat(kLifeMin);
	fUserInput.fLifeMax			= fCompPB->GetFloat(kLifeMax);
	fUserInput.fImmortal		= fCompPB->GetInt(kImmortal);
	fUserInput.fPPS				= fCompPB->GetFloat(kPPS);
	fUserInput.fScaleMin		= fCompPB->GetInt(kScaleMin);
	fUserInput.fScaleMax		= fCompPB->GetInt(kScaleMax);

	fUserInput.fGravity			= fCompPB->GetInt(kGravity);
	fUserInput.fDrag			= fCompPB->GetInt(kDrag);
	fUserInput.fWindMult		= fCompPB->GetInt(kWindMult);
	fUserInput.fMassRange		= fCompPB->GetFloat(kMassRange);
	fUserInput.fPreSim			= fCompPB->GetInt(kPreSim);
	fUserInput.fRotRange		= fCompPB->GetFloat(kRotRange);

	return true;
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

	void IValidateSpinners(TimeValue t, IParamBlock2 *pb, IParamMap2 *map, UInt32 id)
	{
		UInt32 minIndex, maxIndex;
		hsBool adjustMin;
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

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		int id = LOWORD(wParam);
		int code = HIWORD(wParam);

		IParamBlock2 *pb = map->GetParamBlock();
		HWND cbox = NULL;

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
				selection = SendMessage(GetDlgItem(hWnd, id), CB_GETCURSEL, 0, 0);
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
	void DeleteThis() {}
};
static ParticleCompDlgProc gParticleCompDlgProc;

//////////////////////////////////////////////////////////////////////////////////////////
//
// Particle Effects Base class

// Make sure any new Effect you add is accounted for here, or it won't get converted.
hsBool plParticleEffectComponent::IsParticleEffectComponent(plComponentBase *comp)
{
	if (comp->ClassID() == PARTICLE_FADE_COMPONENT_CLASS_ID ||
		comp->ClassID() == PARTICLE_VOLUME_COMPONENT_CLASS_ID ||
		comp->ClassID() == PARTICLE_WIND_COMPONENT_CLASS_ID ||
		comp->ClassID() == PARTICLE_UNIWIND_COMPONENT_CLASS_ID ||
		comp->ClassID() == PARTICLE_FLOCK_COMPONENT_CLASS_ID)
		return true;

	return false;
}

hsBool plParticleEffectComponent::SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg)
{
	fEffect = nil;
	return true;
}

hsBool plParticleEffectComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
	hsBool valid = plParticleCoreComponent::NodeHasSystem(node);
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
	plParticleFadeVolumeEffect *effect = nil;
	if( !fEffect )
	{
		effect = TRACKED_NEW plParticleFadeVolumeEffect();
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
	hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectMisc ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleFadeComponent, gParticleFadeDesc, "Fade Volume Effect",  "Fade Volume Effect", COMP_TYPE_PARTICLE, PARTICLE_FADE_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleFadeBk
(	

	plComponent::kBlkComp, _T("ParticleFade"), 0, &gParticleFadeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PARTICLE_FADE, IDS_COMP_PARTICLE_FADE_ROLL, 0, 0, NULL,

	plParticleFadeComponent::kFadeDistance,	_T("FadeDistance"),	TYPE_INT, 	P_ANIMATABLE, 0,	
		p_default, 100,
		p_range, 0, 10000,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_INT,	
		IDC_COMP_PARTICLE_FADE_DIST, IDC_COMP_PARTICLE_FADE_DIST_SPIN, 1.0,
		end,

	plParticleFadeComponent::kFadeZ,		_T("FadeInZ"),		TYPE_BOOL, 	P_ANIMATABLE, 0,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_FADEZ,
		end,

	end
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

hsBool plParticleVolumeComponent::PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
	if (!plParticleEffectComponent::PreConvert(pNode, pErrMsg))
		return false;

	fBound = nil;

	plMaxNode *source = (plMaxNode *)fCompPB->GetINode(kSourceNode);
	if (source == nil || !source->CanConvert())
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
	if (fBound != nil)
		return; // already converted it

	fBound = TRACKED_NEW plBoundInterface;
	hsgResMgr::ResMgr()->NewKey(node->GetName(), fBound, node->GetLocation(), node->GetLoadMask());
	fBound->Init(plMeshConverter::Instance().CreateConvexVolume(node));
	hsgResMgr::ResMgr()->AddViaNotify(fBound->GetKey(), TRACKED_NEW plObjRefMsg(node->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
}


void plParticleVolumeComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
	plParticleCollisionEffect *effect = nil;
	if( !fEffect )
	{
		plMaxNode *source = (plMaxNode *)fCompPB->GetINode(kSourceNode);

		if (source == nil || !source->CanConvert())
			return; // No source selected, user has already been warned.

		BuildVolume(source);
		switch( fCompPB->GetInt(kOnImpact) )
		{
		default:
		case kImpDefault:
			effect = TRACKED_NEW plParticleCollisionEffectBeat();
			break;
		case kImpDie:
			effect = TRACKED_NEW plParticleCollisionEffectDie();
			break;
		case kImpBounce:
			{
				plParticleCollisionEffectBounce* bnc = TRACKED_NEW plParticleCollisionEffectBounce();
				bnc->SetBounce(fCompPB->GetFloat(kBounceAmt) * 1.e-2f);
				bnc->SetFriction(fCompPB->GetFloat(kFrictionAmt) * 1.e-2f);
				effect = bnc;
			}
			break;
		};

		hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());	
		plSceneObject *sObj = source->GetSceneObject();
		hsgResMgr::ResMgr()->AddViaNotify( sObj->GetKey(), TRACKED_NEW plGenRefMsg( effect->GetKey(), plRefMsg::kOnCreate, 0, 0 ), plRefFlags::kPassiveRef );

		fEffect = effect;
	}
	else
	{
		effect = plParticleCollisionEffect::ConvertNoRef(fEffect);
	}
	hsAssert(effect, "Our effect pointer was wrong type?");
	hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectConstraint ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleVolumeComponent, gParticleVolumeDesc, "Collision Volume Effect",  "Collision Volume Effect", COMP_TYPE_PARTICLE, PARTICLE_VOLUME_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleVolumeBk
(	
	plComponent::kBlkComp, _T("ParticleVolume"), 0, &gParticleVolumeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PARTICLE_VOLUME, IDS_COMP_PARTICLE_VOLUME_ROLL, 0, 0, NULL,

	plParticleVolumeComponent::kSourceNode, _T("SourceINode"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_PARTICLE_VOLUME_NODE,
		p_sclassID,	GEOMOBJECT_CLASS_ID,
		//p_prompt, IDS_COMP_ONESHOT_STARTS,
		//p_accessor, &gOneShotAccessor,
		end,

	plParticleVolumeComponent::kOnImpact, _T("OnImpact"),		TYPE_INT, 		0, 0,
		p_ui,		TYPE_RADIO, 3,	IDC_COMP_PARTICLE_VOL_DEFAULT,					IDC_COMP_PARTICLE_VOL_DIE,				IDC_COMP_PARTICLE_VOL_BOUNCE,	
		p_vals,						plParticleVolumeComponent::kImpDefault,	plParticleVolumeComponent::kImpDie,	plParticleVolumeComponent::kImpBounce,		
		p_default, plParticleVolumeComponent::kImpDefault,
		end,

	plParticleVolumeComponent::kBounceAmt, _T("BounceAmt"), TYPE_FLOAT, 	0, 0,	
		p_default, 100.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_VOL_BOUNCEAMT, IDC_COMP_PARTICLE_VOL_BOUNCEAMT_SPIN, 1.0,
		end,	
	
	plParticleVolumeComponent::kFrictionAmt, _T("FrictionAmt"), TYPE_FLOAT, 	0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_VOL_FRICTIONAMT, IDC_COMP_PARTICLE_VOL_FRICTIONAMT_SPIN, 1.0,
		end,	
	
	end
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
		float rads = hsScalarDegToRad(clampAngDeg);
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
	plParticleLocalWind* effect = nil;
	if( !fEffect )
	{
		effect = TRACKED_NEW plParticleLocalWind();
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
	hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectForce ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleWindComponent, gParticleWindDesc, "Wind Effect",  "WindEffect", COMP_TYPE_PARTICLE, PARTICLE_WIND_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleWindBk
(	

	plComponent::kBlkComp, _T("ParticleWind"), 0, &gParticleWindDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PARTICLE_WIND, IDS_COMP_PARTICLE_WIND, 0, 0, NULL,

	plParticleWindComponent::kScaleX,	_T("ScaleX"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 25.f,
		p_range, 0.f, 1000.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_SCALEX, IDC_COMP_PARTICLE_WIND_SCALEX_SPIN, 1.0,
		end,

	plParticleWindComponent::kScaleY,	_T("ScaleY"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 25.f,
		p_range, 0.f, 1000.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_SCALEY, IDC_COMP_PARTICLE_WIND_SCALEY_SPIN, 1.0,
		end,

	plParticleWindComponent::kScaleZ,	_T("ScaleZ"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.f,
		p_range, 0.f, 1000.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_SCALEZ, IDC_COMP_PARTICLE_WIND_SCALEZ_SPIN, 1.0,
		end,

	plParticleWindComponent::kSpeed,	_T("Speed"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 32.f,
		p_range, -100.f, 100.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PARTICLE_WIND_SPEED, IDC_COMP_PARTICLE_WIND_SPEED_SPIN, 1.0,
		end,

	plParticleWindComponent::kStrength,	_T("Strength"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 30.f,
		p_range, 0.f, 100.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_STRENGTH, IDC_COMP_PARTICLE_WIND_STRENGTH_SPIN, 1.0,
		end,

	plParticleWindComponent::kConstancy,	_T("Constancy"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.f,
		p_range, -75.f, 300.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PARTICLE_WIND_CONSTANCY, IDC_COMP_PARTICLE_WIND_CONSTANCY_SPIN, 1.0,
		end,

	plParticleWindComponent::kSwirl,	_T("Swirl"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 100.f,
		p_range, 0.f, 100.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_SWIRL, IDC_COMP_PARTICLE_WIND_SWIRL_SPIN, 1.0,
		end,

	plParticleWindComponent::kHorizontal, _T("Horizontal"),		TYPE_BOOL,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_WIND_HORIZONTAL,
		end,

	plParticleWindComponent::kLocalize, _T("Localize"),		TYPE_BOOL,	
		p_default, TRUE,
		end,

	plParticleWindComponent::kClampAngle, _T("ClampAngle"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 180.f,
		p_range, 0.f, 180.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_CLAMPANG, IDC_COMP_PARTICLE_WIND_CLAMPANG_SPIN, 1.0,
		end,

	plParticleWindComponent::kRefObject, _T("RefObject"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_PARTICLE_WIND_REFOBJECT,
		p_prompt, IDS_COMP_CHOOSE_OBJECT,
		end,

	end
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
	plParticleUniformWind* effect = nil;
	if( !fEffect )
	{
		effect = TRACKED_NEW plParticleUniformWind();
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
	hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectForce ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleUniWindComponent, gParticleUniWindDesc, "Uniform Wind",  "UniWind", COMP_TYPE_PARTICLE, PARTICLE_UNIWIND_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleUniWindBk
(	

	plComponent::kBlkComp, _T("ParticleUniWind"), 0, &gParticleUniWindDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

	//Roll out
	IDD_COMP_PARTICLE_UNIWIND, IDS_COMP_PARTICLE_UNIWIND, 0, 0, NULL,

	plParticleUniWindComponent::kStrength,	_T("Strength"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 30.f,
		p_range, 0.f, 100.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_STRENGTH, IDC_COMP_PARTICLE_WIND_STRENGTH_SPIN, 1.0,
		end,

	plParticleUniWindComponent::kConstancy,	_T("Constancy"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 0.f,
		p_range, -75.f, 300.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_FLOAT,	
		IDC_COMP_PARTICLE_WIND_CONSTANCY, IDC_COMP_PARTICLE_WIND_CONSTANCY_SPIN, 1.0,
		end,

	plParticleUniWindComponent::kSwirl,	_T("Swirl"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 100.f,
		p_range, 0.f, 100.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_SWIRL, IDC_COMP_PARTICLE_WIND_SWIRL_SPIN, 1.0,
		end,

	plParticleUniWindComponent::kHorizontal, _T("Horizontal"),		TYPE_BOOL,	
		p_default, FALSE,
		p_ui,	TYPE_SINGLECHEKBOX, IDC_COMP_PARTICLE_WIND_HORIZONTAL,
		end,

	plParticleUniWindComponent::kMinSecs,	_T("MinSecs"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 1.f,
		p_range, 0.1f, 20.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_MINSECS, IDC_COMP_PARTICLE_WIND_MINSECS_SPIN, 1.0,
		end,

	plParticleUniWindComponent::kMaxSecs,	_T("MaxSecs"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 10.f,
		p_range, 0.1f, 30.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_MAXSECS, IDC_COMP_PARTICLE_WIND_MAXSECS_SPIN, 1.0,
		end,

	plParticleUniWindComponent::kRate,	_T("Rate"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 10.f,
		p_range, 0.1f, 50.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_RATE, IDC_COMP_PARTICLE_WIND_RATE_SPIN, 1.0,
		end,

	plParticleUniWindComponent::kClampAngle, _T("ClampAngle"),	TYPE_FLOAT, 	P_ANIMATABLE, 0,	
		p_default, 180.f,
		p_range, 0.f, 180.f,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_COMP_PARTICLE_WIND_CLAMPANG2, IDC_COMP_PARTICLE_WIND_CLAMPANG_SPIN2, 1.0,
		end,

	plParticleUniWindComponent::kRefObject, _T("RefObject"),	TYPE_INODE,		0, 0,
		p_ui,	TYPE_PICKNODEBUTTON, IDC_COMP_PARTICLE_WIND_REFOBJECT,
		p_prompt, IDS_COMP_CHOOSE_OBJECT,
		end,

	end
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

	void IValidateSpinners(TimeValue t, IParamBlock2 *pb, IParamMap2 *map, UInt32 id)
	{
		UInt32 minIndex, maxIndex;
		hsBool adjustMin;
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

	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	void DeleteThis() {}
};
static ParticleFlockEffectDlgProc gParticleFlockEffectDlgProc;



void plParticleFlockComponent::AddToParticleSystem(plParticleSystem *sys, plMaxNode *node)
{
	plParticleFlockEffect* effect = nil;
	if( !fEffect )
	{
		effect = TRACKED_NEW plParticleFlockEffect();
		hsgResMgr::ResMgr()->NewKey(IGetUniqueName(node), effect, node->GetLocation(), node->GetLoadMask());

		hsPoint3 offset(fCompPB->GetFloat(ParamID(kOffsetX)),
						fCompPB->GetFloat(ParamID(kOffsetY)),
						fCompPB->GetFloat(ParamID(kOffsetZ)));
		effect->SetTargetOffset(offset);
		effect->SetInfluenceAvgRadius(fCompPB->GetFloat(ParamID(kInfAvgDist)));
		effect->SetInfluenceRepelRadius(fCompPB->GetFloat(ParamID(kInfRepDist)));

		hsScalar goalDist = fCompPB->GetFloat(ParamID(kGoalDist));
		hsScalar fcDist = fCompPB->GetFloat(ParamID(kFullChaseDist));	
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
	hsgResMgr::ResMgr()->AddViaNotify( effect->GetKey(), TRACKED_NEW plGenRefMsg( sys->GetKey(), plRefMsg::kOnCreate, 0, plParticleSystem::kEffectForce ), plRefFlags::kActiveRef );
}

CLASS_DESC(plParticleFlockComponent, gParticleFlockDesc, "Particle Flock",  "Flock", COMP_TYPE_PARTICLE, PARTICLE_FLOCK_COMPONENT_CLASS_ID)

ParamBlockDesc2 gParticleFlockBk
(	
 
	plComponent::kBlkComp, _T("ParticleFlock"), 0, &gParticleFlockDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,
	
	//Roll out
	IDD_COMP_PARTICLE_FLOCK, IDS_COMP_PARTICLE_FLOCK, 0, 0, &gParticleFlockEffectDlgProc,
	
	plParticleFlockComponent::kOffsetX,	_T("OffsetX"),	TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_TARGET_OFFSETX, IDC_FLOCK_TARGET_OFFSETX_SPIN, 1.0,
		end,
	
	plParticleFlockComponent::kOffsetY,	_T("OffsetY"),	TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_TARGET_OFFSETY, IDC_FLOCK_TARGET_OFFSETY_SPIN, 1.0,
		end,

	plParticleFlockComponent::kOffsetZ,	_T("OffsetZ"),	TYPE_FLOAT, 0, 0,	
		p_default, 0.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_TARGET_OFFSETZ, IDC_FLOCK_TARGET_OFFSETZ_SPIN, 1.0,
		end,

	plParticleFlockComponent::kInfAvgDist,	_T("InfAvgDist"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_CONFORM_DIST, IDC_FLOCK_CONFORM_DIST_SPIN, 1.0,
		end,

	plParticleFlockComponent::kInfRepDist,	_T("InfRepDist"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_REPEL_DIST, IDC_FLOCK_REPEL_DIST_SPIN, 1.0,
		end,

	plParticleFlockComponent::kGoalDist,	_T("GoalDist"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_GOAL_DIST, IDC_FLOCK_GOAL_DIST_SPIN, 1.0,
		end,

	plParticleFlockComponent::kInfAvgStr,	_T("InfAvgStr"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_CONFORM_STR, IDC_FLOCK_CONFORM_STR_SPIN, 1.0,
		end,

	plParticleFlockComponent::kInfRepStr,	_T("InfRepStr"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_REPEL_STR, IDC_FLOCK_REPEL_STR_SPIN, 1.0,
		end,

	plParticleFlockComponent::kGoalOrbitStr,	_T("GoalStr"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_GOAL_STR, IDC_FLOCK_GOAL_STR_SPIN, 1.0,
		end,

	plParticleFlockComponent::kMaxChaseSpeed, _T("MaxSpeed"),	TYPE_FLOAT, 0, 0,	
		p_default, 100.0,
		p_range, 0.0, 999.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_MAX_SPEED, IDC_FLOCK_MAX_SPEED_SPIN, 1.0,
		end,		
	
	plParticleFlockComponent::kGoalChaseStr,	_T("GoalChaseStr"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_GOAL_CHASE_STR, IDC_FLOCK_GOAL_CHASE_STR_SPIN, 1.0,
		end,
	
	plParticleFlockComponent::kMaxOrbitSpeed, _T("MaxOrbitSpeed"),	TYPE_FLOAT, 0, 0,	
		p_default, 20.0,
		p_range, 0.0, 999.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_MAX_ORBIT_SPEED, IDC_FLOCK_MAX_ORBIT_SPEED_SPIN, 1.0,
		end,		
		
	plParticleFlockComponent::kFullChaseDist,	_T("FullChaseDist"),	TYPE_FLOAT, 0, 0,	
		p_default, 1.0,
		p_range, 0.0, 100.0,
		p_ui,	TYPE_SPINNER,	EDITTYPE_POS_FLOAT,	
		IDC_FLOCK_FULL_CHASE_DIST, IDC_FLOCK_FULL_CHASE_DIST_SPIN, 1.0,
		end,
		
	end
);

plParticleFlockComponent::plParticleFlockComponent()
{
	fClassDesc = &gParticleFlockDesc;
	fClassDesc->MakeAutoParamBlocks(this);
}

