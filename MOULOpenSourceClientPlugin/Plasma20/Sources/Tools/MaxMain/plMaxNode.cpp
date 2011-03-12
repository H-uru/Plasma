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
#include "max.h"
#include "iparamm2.h"
#include "iparamb2.h"
#include "ISkin.h"
#include "MNMath.h"

#include "plMaxNode.h"
//#include "../MaxComponent/resource.h"
#include "GlobalUtility.h"

#include "plgDispatch.h"
#include "plPluginResManager.h"
#include "plMaxNodeData.h"
#include "hsUtils.h"

#include "../MaxConvert/plConvert.h"
#include "hsTemplates.h"
#include "hsStringTokenizer.h"

#include "../MaxConvert/hsConverterUtils.h"
#include "../MaxConvert/hsControlConverter.h"
#include "../MaxConvert/plMeshConverter.h"
#include "../MaxConvert/hsMaterialConverter.h"
#include "../MaxConvert/plLayerConverter.h"
#include "../MaxConvert/UserPropMgr.h"
#include "../MaxExport/plErrorMsg.h"
#include "../MaxConvert/hsVertexShader.h"
#include "../MaxConvert/plLightMapGen.h"
#include "plMaxMeshExtractor.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"

#include "../pnKeyedObject/plKey.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../plScene/plSceneNode.h"
#include "../plPhysX/plPXPhysical.h"
#include "../plDrawable/plInstanceDrawInterface.h"
#include "../plDrawable/plSharedMesh.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plAudioInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pfAnimation/plFilterCoordInterface.h"
#include "../plParticleSystem/plBoundInterface.h"
#include "../plPhysical/plPickingDetector.h"
#include "../plModifier/plLogicModifier.h"
#include "../plModifier/plResponderModifier.h"
#include "../plModifier/plInterfaceInfoModifier.h"
#include "../pfAnimation/plLightModifier.h"
#include "../pfCharacter/plPlayerModifier.h"
#include "../plAvatar/plAGModifier.h"
#include "../plAvatar/plAGAnim.h"
#include "../plAvatar/plPointChannel.h"
#include "../plAvatar/plScalarChannel.h"
#include "../plAvatar/plAGMasterMod.h"
#include "../plMessage/plReplaceGeometryMsg.h"
#include "../plGImage/plMipmap.h"
#include "../plModifier/plSpawnModifier.h"
#include "../plInterp/plController.h"
#include "../plInterp/hsInterp.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pfAnimation/plViewFaceModifier.h" // mf horse temp hack testing to be thrown away

#include "../plScene/plOccluder.h"
#include "hsFastMath.h"


#include "../plDrawable/plDrawableSpans.h"
#include "../plDrawable/plGeometrySpan.h"
#include "../plPipeline/plFogEnvironment.h"

#include "../plGLight/plLightInfo.h"
#include "../plGLight/plLightKonstants.h"
#include "../plSurface/plLayerInterface.h"
#include "../plSurface/plLayer.h"
#include "../plSurface/hsGMaterial.h"

#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plIntRefMsg.h"

#include "../MaxExport/plExportProgressBar.h"

#include "../MaxPlasmaMtls/Materials/plDecalMtl.h"

#include "../MaxComponent/plComponentTools.h"
#include "../MaxComponent/plComponent.h"
#include "../MaxComponent/plComponentExt.h"
#include "../MaxComponent/plFlexibilityComponent.h"
#include "../MaxComponent/plLightMapComponent.h"
#include "../MaxComponent/plXImposter.h"
#include "../MaxComponent/plMiscComponents.h"

#include "../plParticleSystem/plParticleSystem.h"
#include "../plParticleSystem/plParticleEmitter.h"
#include "../plParticleSystem/plParticleEffect.h"
#include "../plParticleSystem/plParticleGenerator.h"
#include "../plParticleSystem/plConvexVolume.h"

#include "../MaxPlasmaLights/plRealTimeLightBase.h"
#include "../MaxPlasmaLights/plRTProjDirLight.h"


extern UserPropMgr gUserPropMgr;

hsBool ThreePlaneIntersect(const hsVector3& norm0, const hsPoint3& point0, 
						 const hsVector3& norm1, const hsPoint3& point1, 
						 const hsVector3& norm2, const hsPoint3& point2, hsPoint3& loc);

// Begin external component toolbox ///////////////////////////////////////////////////////////////
static plKey ExternAddModifier(plMaxNodeBase *node, plModifier *mod)
{
	return nil;//((plMaxNode*)node)->AddModifier(mod);
}

static plKey ExternGetNewKey(const char *name, plModifier *mod, plLocation loc)
{
	return nil;//hsgResMgr::ResMgr()->NewKey(name, mod, loc);
}

// In plResponderComponent (for no apparent reason).
int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const char *segName, hsTArray<plKey>& keys);
// In plAudioComponents
int GetSoundNameAndIdx(plComponentBase *comp, plMaxNodeBase *node, const char*& name);

#include "../MaxComponent/plAnimComponent.h"

static const char *GetAnimCompAnimName(plComponentBase *comp)
{
	if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
		return ((plAnimComponentBase*)comp)->GetAnimName();
	return nil;
}

static plKey GetAnimCompModKey(plComponentBase *comp, plMaxNodeBase *node)
{
	if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
		return ((plAnimComponentBase*)comp)->GetModKey((plMaxNode*)node);
	return nil;
}

plComponentTools gComponentTools(ExternAddModifier, 
								ExternGetNewKey, 
								nil, 
								GetAnimCompModKey,
								GetAnimCompAnimName,
								GetMatAnimModKey,
								GetSoundNameAndIdx);

// End external component toolbox //////////////////////////////////////////////////////////////////

void plMaxBoneMap::AddBone(plMaxNodeBase *bone)
{
	char *dbgNodeName = bone->GetName();
	if (fBones.find(bone) == fBones.end())
		fBones[bone] = fNumBones++;
}

void plMaxBoneMap::FillBoneArray(plMaxNodeBase **boneArray)
{
	BoneMap::const_iterator boneIt = fBones.begin();
	for (; boneIt != fBones.end(); boneIt++)
		boneArray[(*boneIt).second] = (*boneIt).first;
}

UInt8 plMaxBoneMap::GetIndex(plMaxNodeBase *bone)
{
	hsAssert(fBones.find(bone) != fBones.end(), "Bone missing in remap!");
	return fBones[bone];
}

UInt32 plMaxBoneMap::GetBaseMatrixIndex(plDrawable *draw)
{
	if (fBaseMatrices.find(draw) == fBaseMatrices.end())
		return (UInt32)-1;

	return fBaseMatrices[draw];
}

void plMaxBoneMap::SetBaseMatrixIndex(plDrawable *draw, UInt32 idx)
{
	fBaseMatrices[draw] = idx;
}

// Don't call this after you've started assigning indices to spans, or
// you'll be hosed (duh).
void plMaxBoneMap::SortBones()
{
	plMaxNodeBase **tempBones = TRACKED_NEW plMaxNodeBase*[fNumBones];	
	FillBoneArray(tempBones);

	// Look ma! An n^2 bubble sort!
	// (It's a 1-time thing for an array of less than 100 items. Speed is not essential here)
	int i,j;
	for (i = 0; i < fNumBones; i++)
	{
		hsBool swap = false;		
		for (j = i + 1; j < fNumBones; j++)
		{
			if (strcmp(tempBones[i]->GetName(), tempBones[j]->GetName()) > 0)
			{
				plMaxNodeBase *temp = tempBones[i];
				tempBones[i] = tempBones[j];
				tempBones[j] = temp;
				swap = true;
			}
		}
		if (!swap)
			break;
	}

	for (i = 0; i < fNumBones; i++)
		fBones[tempBones[i]] = i;

	delete [] tempBones;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

plKey plMaxNode::AddModifier(plModifier *pMod, const char* name)
{
	plKey modKey = pMod->GetKey();
	if (!modKey)
		modKey = hsgResMgr::ResMgr()->NewKey(name, pMod, GetLocation());
	hsgResMgr::ResMgr()->AddViaNotify(modKey, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
	return modKey;
}

hsBool plMaxNode::DoRecur(PMaxNodeFunc pDoFunction,plErrorMsg *pErrMsg, plConvertSettings *settings,plExportProgressBar*bar)
{
#ifdef HS_DEBUGGING
	const char *tmpName = GetName();
#endif

	// If there is a progess bar, update it with the current node
	// If the user cancels during the update, set bogus so we'll exit
	if (bar && bar->Update(GetName()))
	{
		pErrMsg->Set(true);
		return false;
	}

	// If we can't convert (and we aren't the root node) stop recursing.
	if (!IsRootNode() && !CanConvert())
		return false;

	(this->*pDoFunction)(pErrMsg, settings);
	
	for (int i = 0; (!pErrMsg || !pErrMsg->IsBogus()) && i < NumberOfChildren(); i++)
	{
		plMaxNode *pChild = (plMaxNode *)GetChildNode(i);
		pChild->DoRecur(pDoFunction, pErrMsg, settings, bar);
	}
	return true;
}

// This is the same as DoRecur except that it ignores the canconvert field.  We
// need this for things like clearing the old data, where we need to ignore the old value.
hsBool plMaxNode::DoAllRecur(PMaxNodeFunc pDoFunction,plErrorMsg *pErrMsg, plConvertSettings *settings,plExportProgressBar*bar)
{
#ifdef HS_DEBUGGING
	const char *tmpName = GetName();
#endif

	// If there is a progess bar, update it with the current node
	// If the user cancels during the update, set bogus so we'll exit
	if (bar && bar->Update(GetName()))
	{
		pErrMsg->Set(true);
		return false;
	}

	(this->*pDoFunction)(pErrMsg, settings);
	
	for (int i = 0; (!pErrMsg || !pErrMsg->IsBogus()) && i < NumberOfChildren(); i++)
	{
		plMaxNode *pChild = (plMaxNode *)GetChildNode(i);
		pChild->DoAllRecur(pDoFunction, pErrMsg, settings, bar);
	}
	return true;
}

hsBool plMaxNode::ConvertValidate(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	TimeValue	t = hsConverterUtils::Instance().GetTime(GetInterface());
	Object *obj = EvalWorldState( t ).obj;
	const char* dbgName = GetName();

	// Always want to recalculate if this object can convert at this point.
	// In general there won't be any cached flag anyway, but in the SceneViewer
	// there can be if we're reconverting.
	hsBool canConvert = CanConvert(true);

	plMaxNodeData thisNodeData;		// Extra data stored for each node

	if (IsTarget())
	{
		plMaxNode* targetNode = ((plMaxNode*)GetLookatNode());
		if (targetNode)
		{
			Object* targObj = targetNode->EvalWorldState( 0 ).obj;
			
			if (targObj && targObj->SuperClassID() == CAMERA_CLASS_ID)
				canConvert = true;
			else
				canConvert = false;
		}
		else
			canConvert = false;	
	}	
	if (canConvert && obj->SuperClassID() == LIGHT_CLASS_ID)
	{
		thisNodeData.SetDrawable(false);
		thisNodeData.SetRunTimeLight(true);
		thisNodeData.SetForceLocal(true);
	}

	if (UserPropExists("Occluder"))
	{
//		thisNodeData.SetDrawable(false);
	}
	if( UserPropExists("PSRunTimeLight") )
		thisNodeData.SetRunTimeLight(true);

	if (GetParticleRelated())
		thisNodeData.SetForceLocal(true);
//	if (UserPropExists("cloth"))
//	{
//		thisNodeData.SetForceLocal(true);
//	}

	// If we want a physicals only world, set everything to not drawable by default.
	if (settings->fPhysicalsOnly)
		thisNodeData.SetDrawable(false);

	// Remember info in MaxNodeData block for later
	thisNodeData.SetCanConvert(canConvert);

	SetMaxNodeData(&thisNodeData);

#define MF_DISABLE_INSTANCING
#ifndef MF_DISABLE_INSTANCING
	// Send this node off to the instance list, to see if we're instanced
	if( CanMakeMesh( obj, pErrMsg, settings ) ) 
	{
		hsTArray<plMaxNode *> nodes;
		UInt32 numInstances = IBuildInstanceList( GetObjectRef(), t, nodes );
		if( numInstances > 1 )
		{
			/// INSTANCED. Make sure to force local on us
			SetForceLocal( true );
			SetInstanced( true );
		}
	}
#endif // MF_DISABLE_INSTANCING

	// If this is for the SceneViewer, turn off the dirty flags so we won't try
	// reconverting this node again.
	if (settings->fSceneViewer)
		SetDirty(kAllDirty, false);

	return canConvert;
}

hsBool plMaxNode::ClearMaxNodeData(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	// The right place to delete the boneMap is really in ~plMaxNodeData, but that class
	// is only allowed to know about stuff in nucleusLib.
	if (GetBoneMap() && GetBoneMap()->fOwner == this)
		delete GetBoneMap();

	if (GetSwappableGeom())
	{
		// Ref and unref it, so it goes away if no one kept it around (i.e. we just
		// looked at the mesh during export for reference, but don't want to keep it.)
		GetSwappableGeom()->GetKey()->RefObject();
		GetSwappableGeom()->GetKey()->UnRefObject();
	}

	SetMaxNodeData( nil );
	return true;
}

#include "plGetLocationDlg.h"

//
// Helper for setting synchedObject options, until we have a GUI
//
#include "../plResMgr/plKeyFinder.h"
#include "plMaxCFGFile.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "../plResMgr/plPageInfo.h"
#include "../pnNetCommon/plSDLTypes.h"

void plMaxNode::CheckSynchOptions(plSynchedObject* so)
{
	if (so)
	{
		//////////////////////////////////////////////////////////////////////////
		// TEMP - remove
		//
		TSTR sdata,sdataList[128];
		int i,num;

		//
		// check for LocalOnly or DontPersist props
		//
		if (gUserPropMgr.UserPropExists(this, "LocalOnly"))
			so->SetLocalOnly(true);	// disable net synching and persistence
		else
		if (gUserPropMgr.UserPropExists(this, "DontPersistAny"))	// disable all types of persistence
			so->SetSynchFlagsBit(plSynchedObject::kExcludeAllPersistentState);
		else
		{
			if (gUserPropMgr.GetUserPropStringList(this, "DontPersist", num, sdataList))
			{
				for(i=0;i<num;i++)
					so->AddToSDLExcludeList(sdataList[i]);	// disable a type of persistence
			}
		}

		//
		// Check for Volatile prop
		//
		if (gUserPropMgr.UserPropExists(this, "VolatileAll"))	// make all sdl types on this object Volatile
			so->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
		else
		{
			if (gUserPropMgr.GetUserPropStringList(this, "Volatile", num, sdataList))
			{
				for(i=0;i<num;i++)
					so->AddToSDLVolatileList(sdataList[i]);	// make volatile a type of persistence
			}
		}

		bool tempOldOverride = (gUserPropMgr.UserPropExists(this, "OverrideHighLevelSDL") != 0);

		//
		// TEMP - remove
		//////////////////////////////////////////////////////////////////////////

		// If this object isn't in a global room, turn off sync flags
		if ((!tempOldOverride && !GetOverrideHighLevelSDL()) && !GetLocation().IsReserved())
		{
			bool isDynSim = GetPhysicalProps()->GetGroup() == plSimDefs::kGroupDynamic;
			bool hasPFC = false;
			int count = NumAttachedComponents();
			for (UInt32 x = 0; x < count; x++)
			{
				plComponentBase *comp = GetAttachedComponent(x);
				if (comp->ClassID() == Class_ID(0x670d3629, 0x559e4f11))
				{	
					hasPFC = true;
					break;
				}
			}
			if (!isDynSim && !hasPFC)
			{	
				so->SetSynchFlagsBit(plSynchedObject::kExcludeAllPersistentState);
			}
			else
			{
				so->AddToSDLExcludeList(kSDLAGMaster);
				so->AddToSDLExcludeList(kSDLResponder);
				so->AddToSDLExcludeList(kSDLLayer);
				so->AddToSDLExcludeList(kSDLSound);
				so->AddToSDLExcludeList(kSDLXRegion);
			}
		}
	}
}

hsBool plMaxNode::MakeSceneObject(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	const char* dbgName = GetName();
	if (!CanConvert()) 
		return false;

	plLocation nodeLoc = GetLocation();//GetLocFromStrings();	// After this we can use GetLocation()
	if (!nodeLoc.IsValid())
	{
		// If we are reconverting, we don't want to bother the user about a room.
		// In most cases, if it doesn't have a room we are in the middle of creating
		// it.  We don't want to pop up a dialog at that point.
		if (settings->fReconvert)
		{
			SetCanConvert(false);
			return false;
		}

		if (!plGetLocationDlg::Instance().GetLocation(this, pErrMsg))
			return false;
		nodeLoc = GetLocation();
	}

	plSceneObject* pso;
	plKey objKey;

	// Handle this as a SceneObject
	pso = TRACKED_NEW plSceneObject;
	objKey = hsgResMgr::ResMgr()->NewKey(GetName(), pso, nodeLoc, GetLoadMask());

	// Remember info in MaxNodeData block for later
	plMaxNodeData *pDat = GetMaxNodeData();
	pDat->SetKey(objKey);
	pDat->SetSceneObject(pso);

	CheckSynchOptions(pso);
	
	return true;
}

hsBool plMaxNode::PrepareSkin(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	if( !IFindBones(pErrMsg, settings) )
		return false;

	if( !NumBones() )
		return true;

	int i;
	for( i = 0; i < NumBones(); i++ )
	{
		GetBone(i)->SetForceLocal(true);
		GetBone(i)->SetDrawable(false);
	}

	return true;
}

hsBool plMaxNode::IFindBones(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	if( !CanConvert() )
		return false;

	if (UserPropExists("Bone"))
	{
		AddBone(this);
		SetForceLocal(true);
	}

	ISkin* skin = FindSkinModifier();
	if( skin && skin->GetNumBones() )
	{
		char *dbgNodeName = GetName();

		// BoneUpdate
		//SetForceLocal(true);
		int i;
		for( i = 0; i < skin->GetNumBones(); i++ )
		{
			plMaxNode* bone = (plMaxNode*)skin->GetBone(i);
			if( bone )
			{
				if( !bone->CanConvert() || !bone->GetMaxNodeData() )
				{
					if( pErrMsg->Set(true, GetName(), "Trouble connecting to bone %s - skipping", bone->GetName()).CheckAndAsk() )
						SetDrawable(false);
				}
				else
				{
					AddBone(bone);
					bone->SetForceLocal(true);
				}
			}
			else
			{
				if( pErrMsg->Set(true, GetName(), "Trouble finding bone - skipping").CheckAndAsk() )
					SetDrawable(false);
			}
		}
	}

	return true;
}

#include "plMaxMeshExtractor.h"
#include "plPhysXCooking.h"
#include "../plPhysX/plPXStream.h"
#include "../plPhysX/plSimulationMgr.h"
#include "hsSTLStream.h"

hsBool plMaxNode::MakePhysical(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	const char* dbgNodeName = GetName();

	if( !CanConvert() )
		return false;

	if( !GetPhysical() )
		return true;

	plPhysicalProps *physProps = GetPhysicalProps();

	if (!physProps->IsUsed())
		return true;

	//hsStatusMessageF("Making phys for %s", dbgNodeName);

	plSimDefs::Group group = (plSimDefs::Group)physProps->GetGroup();
	plSimDefs::Bounds bounds = (plSimDefs::Bounds)physProps->GetBoundsType();
	float mass = physProps->GetMass();

	plMaxNode *proxyNode = physProps->GetProxyNode();
	if (!proxyNode)
		proxyNode = this;

	// We want to draw solid physicals only.  If it is something avatars bounce off,
	// set it to drawable.
	if (settings->fPhysicalsOnly)
	{
		if (group == plSimDefs::kGroupStatic ||
			group == plSimDefs::kGroupAvatarBlocker ||
			group == plSimDefs::kGroupDynamic)
			proxyNode->SetDrawable(true);
	}

	// If mass is zero and we're animated, set the mass to 1 so it will get a rigid
	// body.  Otherwise PhysX will make assumptions about the physical which will
	// fail when it gets moved.
	if (physProps->GetPhysAnim() && mass == 0.f)
		mass = 1.f;

	TSTR sdata;
	plMaxNode* baseNode = this;	
	while (!baseNode->GetParentNode()->IsRootNode())
		baseNode = (plMaxNode*)baseNode->GetParentNode();
	plKey roomKey = baseNode->GetRoomKey();
	if (!roomKey)
	{
		pErrMsg->Set(true, "Room Processing Error - Physics" "The Room that physics component %s is attached to should have already been\nprocessed.", GetName());
		return false;
	}

	plMaxNode* subworld = physProps->GetSubworld();

	PhysRecipe recipe;
	recipe.mass = mass;
	recipe.friction = physProps->GetFriction();
	recipe.restitution = physProps->GetRestitution();
	recipe.bounds = (plSimDefs::Bounds)physProps->GetBoundsType();
	recipe.group = group;
	recipe.reportsOn = physProps->GetReportGroup();
	recipe.objectKey = GetKey();
	recipe.sceneNode = roomKey;
	recipe.worldKey = subworld ? subworld->GetKey() : nil;

	plMaxMeshExtractor::NeutralMesh mesh;
	plMaxMeshExtractor::Extract(mesh, proxyNode, bounds == plSimDefs::kBoxBounds, this);

	if (subworld)
		recipe.l2s = subworld->GetWorldToLocal44() * mesh.fL2W;
	else
		recipe.l2s = mesh.fL2W;

	switch (bounds)
	{
	case plSimDefs::kBoxBounds:
		{
			hsPoint3 minV(FLT_MAX, FLT_MAX, FLT_MAX), maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (int i = 0; i < mesh.fNumVerts; i++)
			{
				minV.fX = hsMinimum(mesh.fVerts[i].fX, minV.fX);
				minV.fY = hsMinimum(mesh.fVerts[i].fY, minV.fY);
				minV.fZ = hsMinimum(mesh.fVerts[i].fZ, minV.fZ);
				maxV.fX = hsMaximum(mesh.fVerts[i].fX, maxV.fX);
				maxV.fY = hsMaximum(mesh.fVerts[i].fY, maxV.fY);
				maxV.fZ = hsMaximum(mesh.fVerts[i].fZ, maxV.fZ);
			}
			hsPoint3 width = maxV - minV;
			recipe.bDimensions = width / 2;
			recipe.bOffset = minV + (width / 2.f);
		}
		break;
	case plSimDefs::kProxyBounds:
	case plSimDefs::kExplicitBounds:
		{
			// if this is a detector then try to convert to a convex hull first... if that doesn't succeed then do it as an exact
			if ( group == plSimDefs::kGroupDetector )
			{
				// try converting to a convex hull mesh
				recipe.meshStream = plPhysXCooking::CookHull(mesh.fNumVerts, mesh.fVerts,false);
				if (recipe.meshStream)
				{
					plPXStream pxs(recipe.meshStream);
					recipe.convexMesh = plSimulationMgr::GetInstance()->GetSDK()->createConvexMesh(pxs);
					recipe.bounds = plSimDefs::kHullBounds;
					// then test to see if the original mesh was convex (unless they said to skip 'em)
#ifdef WARNINGS_ON_CONCAVE_PHYSX_WORKAROUND
					if ( !plPhysXCooking::fSkipErrors )
					{
						if ( !plPhysXCooking::TestIfConvex(recipe.convexMesh, mesh.fNumVerts, mesh.fVerts) )
						{
							int retStatus = pErrMsg->Set(true, "Physics Warning: PhysX workaround", "Detector region that is marked as exact and is concave but switching to convex hull for PhysX: %s", GetName()).CheckAskOrCancel();
							pErrMsg->Set();
							if ( retStatus == 1 )  // cancel?
								plPhysXCooking::fSkipErrors = true;
						}
					}
#endif  // WARNINGS_ON_CONCAVE_PHYSX_WORKAROUND
				}
				if (!recipe.meshStream)
				{
					if ( !pErrMsg->Set(true, "Physics Warning", "Detector region exact failed to be made a Hull, trying trimesh: %s", GetName()).Show() )
						pErrMsg->Set();
					recipe.meshStream = plPhysXCooking::CookTrimesh(mesh.fNumVerts, mesh.fVerts, mesh.fNumFaces, mesh.fFaces);
					if (!recipe.meshStream)
					{
						pErrMsg->Set(true, "Physics Error", "Trimesh creation failed for physical %s", GetName()).Show();
						return false;
					}
					plPXStream pxs(recipe.meshStream);
					recipe.triMesh = plSimulationMgr::GetInstance()->GetSDK()->createTriangleMesh(pxs);
				}
			}
			else
			{
				recipe.meshStream = plPhysXCooking::CookTrimesh(mesh.fNumVerts, mesh.fVerts, mesh.fNumFaces, mesh.fFaces);
				if (!recipe.meshStream)
				{
					pErrMsg->Set(true, "Physics Error", "Trimesh creation failed for physical %s", GetName()).Show();
					return false;
				}
				plPXStream pxs(recipe.meshStream);
				recipe.triMesh = plSimulationMgr::GetInstance()->GetSDK()->createTriangleMesh(pxs);
			}
		}
		break;
	case plSimDefs::kSphereBounds:
		{
			hsPoint3 minV(FLT_MAX, FLT_MAX, FLT_MAX), maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (int i = 0; i < mesh.fNumVerts; i++)
			{
				minV.fX = hsMinimum(mesh.fVerts[i].fX, minV.fX);
				minV.fY = hsMinimum(mesh.fVerts[i].fY, minV.fY);
				minV.fZ = hsMinimum(mesh.fVerts[i].fZ, minV.fZ);
				maxV.fX = hsMaximum(mesh.fVerts[i].fX, maxV.fX);
				maxV.fY = hsMaximum(mesh.fVerts[i].fY, maxV.fY);
				maxV.fZ = hsMaximum(mesh.fVerts[i].fZ, maxV.fZ);
			}
			hsPoint3 width = maxV - minV;
			recipe.radius = hsMaximum(width.fX, hsMaximum(width.fY, width.fZ));
			recipe.radius /= 2.f;
			recipe.offset = minV + (width / 2.f);
		}
		break;
	case plSimDefs::kHullBounds:
		{
			
			
			
			if ( group == plSimDefs::kGroupDynamic )
			{
				
							
				recipe.meshStream = plPhysXCooking::IMakePolytope(mesh);
				
				
				if (!recipe.meshStream)
				{
					pErrMsg->Set(true, "Physics Error", "polyTope-convexhull failed for physical %s", GetName()).Show();
					return false;
				}
			}
			else
			{
				recipe.meshStream = plPhysXCooking::CookHull(mesh.fNumVerts, mesh.fVerts,false);
				if(!recipe.meshStream)
				{
					pErrMsg->Set(true, "Physics Error", "Convex hull creation failed for physical %s", GetName()).Show();
					return false;
				}
			}
			plPXStream pxs(recipe.meshStream);
			recipe.convexMesh = plSimulationMgr::GetInstance()->GetSDK()->createConvexMesh(pxs);
		}
		break;
	}

	delete [] mesh.fFaces;
	delete [] mesh.fVerts;

	//
	// Create the physical
	//
	plPXPhysical* physical = TRACKED_NEW plPXPhysical;

	// add the object to the resource manager, keyed to the new name
	plLocation nodeLoc = GetKey()->GetUoid().GetLocation();
	const char *objName = GetKey()->GetName();
	plKey physKey = hsgResMgr::ResMgr()->NewKey(objName, physical, nodeLoc, GetLoadMask());

	if (!physical->Init(recipe))
	{
		pErrMsg->Set(true, "Physics Error", "Physical creation failed for object %s", GetName()).Show();
		physKey->RefObject();
		physKey->UnRefObject();
		return false;
	}

	physical->SetProperty(plSimulationInterface::kPinned, physProps->GetPinned());
	physical->SetProperty(plSimulationInterface::kPhysAnim, physProps->GetPhysAnim());
	physical->SetProperty(plSimulationInterface::kNoSynchronize, (physProps->GetNoSynchronize() != 0));
	physical->SetProperty(plSimulationInterface::kStartInactive, (physProps->GetStartInactive() != 0));
	physical->SetProperty(plSimulationInterface::kAvAnimPushable, (physProps->GetAvAnimPushable() != 0));

	if(physProps->GetLOSBlockCamera())
		physical->AddLOSDB(plSimDefs::kLOSDBCameraBlockers);
	if(physProps->GetLOSBlockUI())
		physical->AddLOSDB(plSimDefs::kLOSDBUIBlockers);
	if(physProps->GetLOSBlockCustom())
		physical->AddLOSDB(plSimDefs::kLOSDBCustom);
	if(physProps->GetLOSUIItem())
		physical->AddLOSDB(plSimDefs::kLOSDBUIItems);
	if(physProps->GetLOSShootable())
		physical->AddLOSDB(plSimDefs::kLOSDBShootableItems);
	if(physProps->GetLOSAvatarWalkable())
		physical->AddLOSDB(plSimDefs::kLOSDBAvatarWalkable);
	if(physProps->GetLOSSwimRegion())
		physical->AddLOSDB(plSimDefs::kLOSDBSwimRegion);
	
	plSimulationInterface* si = TRACKED_NEW plSimulationInterface;
	plKey pSiKey = hsgResMgr::ResMgr()->NewKey(objName, si, nodeLoc, GetLoadMask());

	// link the simulation interface to the scene object
	hsgResMgr::ResMgr()->AddViaNotify(pSiKey, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	// add the physical to the simulation interface
	hsgResMgr::ResMgr()->AddViaNotify(physKey , TRACKED_NEW plIntRefMsg(pSiKey, plRefMsg::kOnCreate, 0, plIntRefMsg::kPhysical), plRefFlags::kActiveRef);

	return true;
}

hsBool plMaxNode::MakeController(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	if (!CanConvert()) 
		return false;

	hsBool forceLocal = hsControlConverter::Instance().ForceLocal(this);
		// Rember the force Local setting
	hsBool CurrForceLocal = GetForceLocal();					// dont want to clobber it with false if componentPass made it true
	forceLocal = (CurrForceLocal || forceLocal) ? true : false;		// if it was set before, or is true now, make it true... 
	SetForceLocal(forceLocal);

	if( IsTMAnimated() && (!GetParentNode()->IsRootNode()) )
	{
		((plMaxNode*)GetParentNode())->SetForceLocal(true);
	}

	return true;
}

hsBool plMaxNode::MakeCoordinateInterface(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	const char* dbgNodeName = GetName();
	if (!CanConvert()) 
		return false;
	plCoordinateInterface* ci = nil;

	hsBool forceLocal = GetForceLocal();

	hsBool needCI = (!GetParentNode()->IsRootNode())
		|| NumberOfChildren()
		|| forceLocal;
	// If we have a transform, set up a coordinateinterface
	if( needCI )
	{
		hsMatrix44 loc2Par = GetLocalToParent44();
		hsMatrix44 par2Loc = GetParentToLocal44();
		if( GetFilterInherit() )
			ci = TRACKED_NEW plFilterCoordInterface;
		else
			ci = TRACKED_NEW plCoordinateInterface;
		//-------------------------
		// Get data from Node, then its key, then its name
		//-------------------------
		plKey pNodeKey = GetKey();
		hsAssert(pNodeKey, "Missing key for this Object");
		const char *pName =	pNodeKey->GetName();
		plLocation nodeLoc = GetLocation();

		plKey pCiKey = hsgResMgr::ResMgr()->NewKey(pName, ci,nodeLoc, GetLoadMask());
		ci->SetLocalToParent(loc2Par, par2Loc);

		hsBool usesPhysics = GetPhysicalProps()->IsUsed();
		ci->SetProperty(plCoordinateInterface::kCanEverDelayTransform, !usesPhysics);
		ci->SetProperty(plCoordinateInterface::kDelayedTransformEval, !usesPhysics);

		hsgResMgr::ResMgr()->AddViaNotify(pCiKey, TRACKED_NEW plObjRefMsg(pNodeKey, plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
	}
	return true;
}

hsBool plMaxNode::MakeModifiers(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	if (!CanConvert()) 
		return false;
	
	hsBool forceLocal = GetForceLocal();
	const char *dbgNodeName = GetName();

	hsBool addMods = (!GetParentNode()->IsRootNode())
		|| forceLocal;

	if (addMods)
	{
	// create / add modifiers

		// mf horse hack testing ViewFace which is already obsolete
		if ( UserPropExists("ViewFacing") )
		{
			plViewFaceModifier* pMod = TRACKED_NEW plViewFaceModifier;
			if( UserPropExists("VFPivotFavorY") )
				pMod->SetFlag(plViewFaceModifier::kPivotFavorY);
			else if( UserPropExists("VFPivotY") )
				pMod->SetFlag(plViewFaceModifier::kPivotY);
			else if( UserPropExists("VFPivotTumble") )
				pMod->SetFlag(plViewFaceModifier::kPivotTumble);
			else
				pMod->SetFlag(plViewFaceModifier::kPivotFace);
			if( UserPropExists("VFScale") )
			{
				pMod->SetFlag(plViewFaceModifier::kScale);
				TSTR sdata;
				GetUserPropString("VFScale",sdata);
				hsStringTokenizer toker;
				toker.Reset(sdata, hsConverterUtils::fTagSeps);
				int nGot = 0;
				char* token;
				hsVector3 scale;
				scale.Set(1.f,1.f,1.f);
				while( (nGot < 3) && (token = toker.next()) )
				{
					switch( nGot )
					{
					case 0:
						scale.fZ = hsScalar(atof(token));
						break;
					case 1:
						scale.fX = scale.fZ;
						scale.fY = hsScalar(atof(token));
						scale.fZ = 1.f;
						break;
					case 2:
						scale.fZ = hsScalar(atof(token));
						break;
					}
					nGot++;
				}
				pMod->SetScale(scale);
			}
			AddModifier(pMod, GetName());
		}
	}
	return true;

}

hsBool plMaxNode::MakeParentOrRoomConnection(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	if (!CanConvert()) 
		return false;

	char *dbgNodeName = GetName();
	plSceneObject *pso = GetSceneObject();
	if( !GetParentNode()->IsRootNode() )
	{
		plKey parKey = GetParentKey();
		plCoordinateInterface* ci = const_cast <plCoordinateInterface*> (pso->GetCoordinateInterface());
		hsAssert(ci,"Missing CI");


		plIntRefMsg* msg = TRACKED_NEW plIntRefMsg(parKey, plRefMsg::kOnCreate, -1, plIntRefMsg::kChildObject);
		msg->SetRef(pso);
		hsgResMgr::ResMgr()->AddViaNotify(msg, plRefFlags::kPassiveRef);
	}

	hsgResMgr::ResMgr()->AddViaNotify(pso->GetKey(), TRACKED_NEW plNodeRefMsg(GetRoomKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kObject), plRefFlags::kActiveRef);
	return true;
}

void plMaxNode::IWipeBranchDrawable(hsBool b)
{
	SetDrawable(b);
	for (int i = 0; i < NumberOfChildren(); i++)
	{
		plMaxNode *pChild = (plMaxNode *)GetChildNode(i);
		pChild->IWipeBranchDrawable(b);
	}
}

//// CanMakeMesh /////////////////////////////////////////////////////////////
//	Returns true if MakeMesh() on this node will result in spans being stored
//	on a drawable. Takes in the object pointer to avoid having to do redundant
//	work to get it.
//	9.25.2001 mcn - Made public so components can figure out if this node is
//	meshable.

hsBool	plMaxNode::CanMakeMesh( Object *obj, plErrorMsg *pErrMsg, plConvertSettings *settings )
{
	if( obj == nil )
		return false;

	if( UserPropExists( "Plasma2_Camera" ) )
		return false;

	if( !GetSwappableGeom() && !GetDrawable() )
		return false;

	if( GetParticleRelated() )
		return false;
	
	if( obj->CanConvertToType( triObjectClassID ) )
		return true;

	return false;
}

void ITestAdjacencyRecur(const hsTArray<int>* vertList, int iVert, hsBitVector& adjVerts)
{
	adjVerts.SetBit(iVert);

	int i;
	for( i = 0; i < vertList[iVert].GetCount(); i++ )
	{
		if( !adjVerts.IsBitSet(vertList[iVert][i]) )
		{
			ITestAdjacencyRecur(vertList, vertList[iVert][i], adjVerts);
		}
	}
}

hsBool ITestAdjacency(const hsTArray<int>* vertList, int numVerts)
{
	hsBitVector adjVerts;
	ITestAdjacencyRecur(vertList, 0, adjVerts);

	int i;
	for( i = 0; i < numVerts; i++ )
	{
		if( !adjVerts.IsBitSet(i) )
			return false;
	}
	return true;
}

int IsGeoSpanConvexExhaust(const plGeometrySpan* span)
{
	// Brute force, check every point against every face

	UInt16* idx = span->fIndexData;
	int numFaces = span->fNumIndices / 3;

	UInt32 stride = span->GetVertexSize(span->fFormat);

	UInt8* vertData = span->fVertexData;
	int numVerts = span->fNumVerts;

	hsBool someIn = false;
	hsBool someOut = false;

	int i;
	for( i = 0; i < numFaces; i++ )
	{
		// compute norm and dist for face
		hsPoint3* pos[3];
		pos[0] = (hsPoint3*)(vertData + idx[0] * stride);
		pos[1] = (hsPoint3*)(vertData + idx[1] * stride);
		pos[2] = (hsPoint3*)(vertData + idx[2] * stride);

		hsVector3 edge01(pos[1], pos[0]);
		hsVector3 edge02(pos[2], pos[0]);

		hsVector3 faceNorm = edge01 % edge02;
		hsFastMath::NormalizeAppr(faceNorm);
		hsScalar faceDist = faceNorm.InnerProduct(pos[0]);

		int j;
		for( j = 0; j < numVerts; j++ )
		{
			hsPoint3* p = (hsPoint3*)(vertData + idx[0] * stride);


			hsScalar dist = p->InnerProduct(faceNorm) - faceDist;

			const hsScalar kSmall = 1.e-3f;
			if( dist < -kSmall )
				someIn = true;
			else if( dist > kSmall )
				someOut = true;

			if( someIn && someOut )
				return false;
		}

		idx += 3;
	}
	return true;
}

int IsGeoSpanConvex(plMaxNode* node, const plGeometrySpan* span)
{
	static int skipTest = false;
	if( skipTest )
		return 0;

	// May not be now, but could become.
	if( span->fFormat & plGeometrySpan::kSkinWeightMask )
		return 0;

	// May not be now, but could become.
	if( node->GetConcave() || node->UserPropExists("XXXWaterColor") )
		return 0;

	if( span->fMaterial && span->fMaterial->GetLayer(0) && (span->fMaterial->GetLayer(0)->GetMiscFlags() & hsGMatState::kMiscTwoSided) )
		return 0;

	int numVerts = span->fNumVerts;
	if( !numVerts )
		return 0;

	int numFaces = span->fNumIndices / 3;
	if( !numFaces )
		return 0;

	const int kSmallNumFaces = 20;
	if( numFaces <= kSmallNumFaces )
		return IsGeoSpanConvexExhaust(span);

	hsTArray<int>*	vertList = TRACKED_NEW hsTArray<int> [numVerts];

	hsTArray<hsVector3>* normList = TRACKED_NEW hsTArray<hsVector3> [numVerts];
	hsTArray<hsScalar>* distList = TRACKED_NEW hsTArray<hsScalar> [numVerts];

	UInt16* idx = span->fIndexData;

	UInt32 stride = span->GetVertexSize(span->fFormat);

	UInt8* vertData = span->fVertexData;

	// For each face
	int iFace;
	for( iFace = 0; iFace < numFaces; iFace++ )
	{
		// compute norm and dist for face
		hsPoint3* pos[3];
		pos[0] = (hsPoint3*)(vertData + idx[0] * stride);
		pos[1] = (hsPoint3*)(vertData + idx[1] * stride);
		pos[2] = (hsPoint3*)(vertData + idx[2] * stride);

		hsVector3 edge01(pos[1], pos[0]);
		hsVector3 edge02(pos[2], pos[0]);

		hsVector3 faceNorm = edge01 % edge02;
		hsFastMath::NormalizeAppr(faceNorm);
		hsScalar faceDist = faceNorm.InnerProduct(pos[0]);


		// For each vert
		int iVtx;
		for( iVtx = 0; iVtx < 3; iVtx++ )
		{
			int jVtx;
			for( jVtx = 0; jVtx < 3; jVtx++ )
			{
				if( iVtx != jVtx )
				{
					// if idx[jVtx] not in list vertList[idx[iVtx]], add it
					if( vertList[idx[iVtx]].kMissingIndex == vertList[idx[iVtx]].Find(idx[jVtx]) )
						vertList[idx[iVtx]].Append(idx[jVtx]);
				}
			}
			normList[idx[iVtx]].Append(faceNorm);
			distList[idx[iVtx]].Append(faceDist);

		}
		idx += 3;
	}

	hsBool someIn = false;
	hsBool someOut = false;
	int i;
	for( i = 0; i < numVerts; i++ )
	{
		int k;
		for( k = 0; k < normList[i].GetCount(); k++ )
		{
			int j;
			for( j = 0; j < vertList[i].GetCount(); j++ )
			{
				hsPoint3* pos = (hsPoint3*)(vertData + vertList[i][j] * stride);
				hsScalar dist = pos->InnerProduct(normList[i][k]) - distList[i][k];

				const hsScalar kSmall = 1.e-3f;
				if( dist < -kSmall )
					someIn = true;
				else if( dist > kSmall )
					someOut = true;

				if( someIn && someOut )
					goto cleanUp;
			}
		}
	}
	
	if( !ITestAdjacency(vertList, numVerts) )
		someIn = someOut = true;

cleanUp:
	delete [] vertList;
	delete [] normList;
	delete [] distList;

	if( someIn && someOut )
		return 0;

	return someIn ? -1 : 1;
}

// Returns nil if there isn't a sceneobject and a drawinterface.
plDrawInterface* plMaxNode::GetDrawInterface()
{
	plDrawInterface* di = nil;
	plSceneObject* obj = GetSceneObject();
	if( obj )
	{
		di = obj->GetVolatileDrawInterface();
	}
	return di;
}

hsBool plMaxNode::MakeMesh(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	hsTArray<plGeometrySpan *>	spanArray;
	plDrawInterface				*newDI = nil;

	hsBool		gotMade = false;
	hsBool		haveAddedToSceneNode = false;
	hsGMesh		*myMesh = nil;
	UInt32		i, triMeshIndex = (UInt32)-1;
	const char	*dbgNodeName = GetName();
	TSTR sdata;
	hsStringTokenizer toker;
	plLocation nodeLoc = GetLocation();
	
	if (!GetSwappableGeom())
	{
		if (!CanConvert()) 
			return false;

		if( UserPropExists( "Plasma2_Camera" ) || !GetDrawable()  )
		{
			SetMesh( nil );
			return true;
		}
	}
	
	if( GetSwappableGeomTarget() != (UInt32)-1)
	{
		// This node has no geometry on export, but will have some added at runtime,
		// so it needs a special drawInterface

		plInstanceDrawInterface *newDI = TRACKED_NEW plInstanceDrawInterface;
		newDI->fTargetID = GetSwappableGeomTarget();
		plKey pDiKey = hsgResMgr::ResMgr()->NewKey( GetKey()->GetName(), newDI, nodeLoc, GetLoadMask() );
		hsgResMgr::ResMgr()->AddViaNotify(pDiKey, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

		plSwapSpansRefMsg *sMsg = TRACKED_NEW plSwapSpansRefMsg(pDiKey, plRefMsg::kOnCreate, -1, -1);
		plDrawableSpans *drawable = IGetSceneNodeSpans(IGetDrawableSceneNode(pErrMsg), true, true);
		hsgResMgr::ResMgr()->AddViaNotify(drawable->GetKey(), sMsg, plRefFlags::kActiveRef);

		return true;
	}

	if( GetInstanced() )
	{
		hsTArray<plMaxNode *>	nodes;
		TimeValue	t = hsConverterUtils::Instance().GetTime(GetInterface());
		UInt32		numInstances = IBuildInstanceList( GetObjectRef(), t, nodes, true );

		/// Instanced, find an iNode in the list that's been converted already
		for( i = 0; i < numInstances; i++ )
		{
			if( nodes[ i ]->GetSceneObject() && nodes[ i ]->GetSceneObject()->GetDrawInterface() )
			{
				/// Found it!
				if( !IMakeInstanceSpans( nodes[ i ], spanArray, pErrMsg, settings ) )
					return false;

				gotMade = true;
				break;
			}
		}
		/// If we didn't find anything, nothing's got converted yet, so convert the first one
		/// like normal
	}

	// This has the side effect of calling SetMovable(true) if it should be and
	// isn't already. So it needs to be before we make the mesh (and material).
	// (Really, whatever makes it movable should do so then, but that has the potential
	// to break other stuff, which I don't want to do 2 weeks before we ship).
	hsBool movable = IsMovable();

	if( !gotMade )
	{
		if( !plMeshConverter::Instance().CreateSpans( this, spanArray, !settings->fDoPreshade ) )
			return false;
	}
	if( !spanArray.GetCount() )
		return true;

	for( i = 0; i < spanArray.GetCount(); i++ )
		spanArray[i]->fMaxOwner = GetKey()->GetName();

	UInt32 shadeFlags = 0;
	if( GetNoPreShade() )
		shadeFlags |= plGeometrySpan::kPropNoPreShade;
	if( GetRunTimeLight() )
		shadeFlags |= plGeometrySpan::kPropRunTimeLight;
	if( GetNoShadow() )
		shadeFlags |= plGeometrySpan::kPropNoShadow;
	if( GetForceShadow() || GetAvatarSO() )
		shadeFlags |= plGeometrySpan::kPropForceShadow;
	if( GetReverseSort() )
		shadeFlags |= plGeometrySpan::kPropReverseSort;
	if( GetForceVisLOS() )
		shadeFlags |= plGeometrySpan::kVisLOS;
	if( shadeFlags )
	{
		for( i = 0; i < spanArray.GetCount(); i++ )
			spanArray[ i ]->fProps |= shadeFlags;
	}

	hsBool DecalMat = false;
	hsBool NonDecalMat = false;
		
	for (i = 0; i < spanArray.GetCount(); i++)
	{
		if (spanArray[i]->fMaterial->IsDecal())
			DecalMat = true;
		else
			NonDecalMat = true;					
	}
	if (!(DecalMat ^ NonDecalMat))
	{
		for( i = 0; i < spanArray.GetCount(); i++ )
			spanArray[ i ]->ClearBuffers();

		if (pErrMsg->Set((plConvert::Instance().fWarned & plConvert::kWarnedDecalAndNonDecal) == 0, GetName(), 
			"This node has both regular and decal materials, and thus will be ignored.").CheckAskOrCancel())
		{
			plConvert::Instance().fWarned |= plConvert::kWarnedDecalAndNonDecal;
		}
		pErrMsg->Set(false);

		return false;
	}

	hsBool isDecal = IsLegalDecal(false); // Don't complain about the parent

	/// Get some stuff
	hsBool forceLocal = GetForceLocal();

	hsMatrix44 l2w = GetLocalToWorld44();
	hsMatrix44 w2l = GetWorldToLocal44();

	/// 4.17.2001 mcn - TEMP HACK to test fog by adding a key to a bogus fogEnviron object to ALL spans
/*		plFogEnvironment	*myFog = nil;
	plKey				myFogKey = hsgResMgr::ResMgr()->FindExportAlias( "HACK_FOG", plFogEnvironment::Index() );	
	if( myFogKey != nil )
		myFog = plFogEnvironment::ConvertNoRef( myFogKey->GetObjectPtr() );
	else
	{
		hsColorRGBA		color;
		color.Set( 0.5, 0.5, 1, 1 );

		// Exp fog
		myFog = TRACKED_NEW plFogEnvironment( plFogEnvironment::kExpFog, 700.f, 1.f, color );
		myFogKey = hsgResMgr::ResMgr()->NewKey( "HACK_FOG", myFog, nodeLoc );
		hsgResMgr::ResMgr()->AddExportAlias( "HACK_FOG", plFogEnvironment::Index(), myFogKey );
	}

	for( int j = 0; j < spanArray.GetCount(); j++ )
	{
		spanArray[ j ].fFogEnviron = myFog;
	}
*/		/// 4.17.2001 mcn - TEMP HACK end


	plDrawable* drawable = nil;
	plSceneNode* tmpNode = nil;

	/// Find the ice to add it to

	if (GetSwappableGeom()) // We just want to make a geo span, not actually add it to a drawable(interface)
	{
		plMaxNode *drawableSource = (plMaxNode *)(GetParentNode()->IsRootNode() ? this : GetParentNode());
		plSceneNode *tmpNode = drawableSource->IGetDrawableSceneNode(pErrMsg);

		plDrawableSpans *drawable = IGetSceneNodeSpans(tmpNode, true, true);
		ISetupBones(drawable, spanArray, l2w, w2l, pErrMsg, settings);

		hsTArray<plGeometrySpan *> *swapSpans = &GetSwappableGeom()->fSpans;
		for (i = 0; i < spanArray.GetCount(); i++)
			swapSpans->Append(spanArray.Get(i));

		char tmpName[256];
		sprintf(tmpName, "%s_SMsh", GetName());
		hsgResMgr::ResMgr()->NewKey(tmpName, GetSwappableGeom(), GetLocation(), GetLoadMask());
				
		return true;
	}

	plMaxNode *nonDecalParent = this;
	if( GetRoomKey() )
	{
		tmpNode = plSceneNode::ConvertNoRef( GetRoomKey()->GetObjectPtr() );

		if (isDecal) // If we're a decal, we just want to use our parent's drawable
		{	
			plMaxNode *parent = (plMaxNode *)GetParentNode();
							
			SetDecalLevel(parent->GetDecalLevel() + 1);
			for( i = 0; i < spanArray.GetCount(); i++ )
				spanArray[ i ]->fDecalLevel = GetDecalLevel();
		}

		{
			/// Make a new drawInterface (will assign stuff to it later)
			newDI = TRACKED_NEW plDrawInterface;
			plKey pDiKey = hsgResMgr::ResMgr()->NewKey( GetKey()->GetName(), newDI, nodeLoc, GetLoadMask() );
			hsgResMgr::ResMgr()->AddViaNotify(pDiKey, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

			/// Attach the processed spans to the DI (through drawables)
			IAssignSpansToDrawables( spanArray, newDI, pErrMsg, settings );
		}
	}

	return true;
}

plSceneNode *plMaxNode::IGetDrawableSceneNode(plErrorMsg *pErrMsg)
{
	plSceneNode *sn = nil;

	sn = plSceneNode::ConvertNoRef( GetRoomKey()->GetObjectPtr() );

	return sn;
}

//// IAssignSpansToDrawables /////////////////////////////////////////////////
//	Given a span array, adds it to the node's drawables, creating them if
//	necessary. Then it takes the resulting indices and drawable pointers
//	and assigns them to the given drawInterface.

void	plMaxNode::IAssignSpansToDrawables( hsTArray<plGeometrySpan *> &spanArray, plDrawInterface *di,
											plErrorMsg *pErrMsg, plConvertSettings *settings )
{
	hsTArray<plGeometrySpan *>	opaqueArray, blendingArray, sortingArray;
	plDrawableSpans				*oSpans = nil, *bSpans = nil, *sSpans = nil;

	int			sCount, oCount, bCount, i;
	plSceneNode	*tmpNode = nil;
	hsMatrix44	l2w = GetLocalToWorld44();
	hsMatrix44	w2l = GetWorldToLocal44();
	UInt32		oIndex = (UInt32)-1, bIndex = (UInt32)-1, sIndex = UInt32(-1);

	tmpNode = IGetDrawableSceneNode(pErrMsg);
/*
	/// Get sceneNode. If we're itinerant and not the parent node, this won't just
	/// be GetRoomKey()->GetObjectPtr()....
	if( GetItinerant() && !GetParentNode()->IsRootNode() )
	{
		/// Step up to the top of the chain
		plMaxNode *baseNode = this;
		while( !baseNode->GetParentNode()->IsRootNode() )
			baseNode = (plMaxNode *)baseNode->GetParentNode();

		if( baseNode->GetItinerant() )
			tmpNode = plSceneNode::ConvertNoRef( baseNode->GetRoomKey()->GetObjectPtr() );
		else
		{
			tmpNode = plSceneNode::ConvertNoRef( GetRoomKey()->GetObjectPtr() );

			/// Warn, since we should only be itinerant if our parent is as well
			pErrMsg->Set( true, "Warning", "Itinerant flag in child '%s' of non-itinerant tree. This should never happen. You should inform a programmer...", GetName() ).Show();
		}		
	}
	else
		tmpNode = plSceneNode::ConvertNoRef( GetRoomKey()->GetObjectPtr() );
*/

	hsBitVector convexBits;
	/// Separate the array into two arrays, one opaque and one blending
	for( sCount = 0, oCount = 0, bCount = 0, i = 0; i < spanArray.GetCount(); i++ )
	{
		if( spanArray[ i ]->fProps & plGeometrySpan::kRequiresBlending )
		{
			hsBool needFaceSort = !GetNoFaceSort() && !IsGeoSpanConvex(this, spanArray[i]);
			if( needFaceSort )
			{
				sCount++;
			}
			else
			{
				convexBits.SetBit(i);
				bCount++;
			}
		}
		else
			oCount++;
	}

	// Done this way, since expanding an hsTArray has the nasty side effect of just copying data, which we don't
	// want when we have memory pointers...
	opaqueArray.SetCount( oCount );
	blendingArray.SetCount( bCount );
	sortingArray.SetCount( sCount );
	for( sCount = 0, oCount = 0, bCount = 0, i = 0; i < spanArray.GetCount(); i++ )
	{
		if( spanArray[ i ]->fProps & plGeometrySpan::kRequiresBlending )
		{
			if( convexBits.IsBitSet(i) )
				blendingArray[ bCount++ ] = spanArray[ i ];
			else
				sortingArray [ sCount++ ] = spanArray[ i ];
		}
		else
			opaqueArray[ oCount++ ] = spanArray[ i ];
	}

	/// Get some drawable pointers
	if( opaqueArray.GetCount() > 0 )
		oSpans = plDrawableSpans::ConvertNoRef( IGetSceneNodeSpans( tmpNode, false ) );
	if( blendingArray.GetCount() > 0 )
		bSpans = plDrawableSpans::ConvertNoRef( IGetSceneNodeSpans( tmpNode, true, false ) );
	if( sortingArray.GetCount() > 0 )
		sSpans = plDrawableSpans::ConvertNoRef( IGetSceneNodeSpans( tmpNode, true, true ) );

	if( oSpans != nil )
		IAssignSpan( oSpans, opaqueArray, oIndex, l2w, w2l, pErrMsg, settings );
	if( bSpans != nil )
		IAssignSpan( bSpans, blendingArray, bIndex, l2w, w2l, pErrMsg, settings );
	if( sSpans )
		IAssignSpan( sSpans, sortingArray, sIndex, l2w, w2l, pErrMsg, settings );

	/// Now assign to the interface
	if( oSpans )
	{
		UInt8 iDraw = di->GetNumDrawables();
		di->SetDrawable( iDraw, oSpans );
		di->SetDrawableMeshIndex( iDraw, oIndex );
	}

	if( bSpans )
	{
		UInt8 iDraw = di->GetNumDrawables();
		di->SetDrawable( iDraw, bSpans );
		di->SetDrawableMeshIndex( iDraw, bIndex );
	}

	if( sSpans )
	{
		UInt8 iDraw = di->GetNumDrawables();
		di->SetDrawable( iDraw, sSpans );
		di->SetDrawableMeshIndex( iDraw, sIndex );
	}

}

//// IAssignSpan /////////////////////////////////////////////////////////////
//	Small utility function for IAssignSpansToDrawables, just does some of
//	the low-down work that's identical for each drawable/spans/etc.

void	plMaxNode::IAssignSpan( plDrawableSpans *drawable, hsTArray<plGeometrySpan *> &spanArray, UInt32 &index,
								hsMatrix44 &l2w, hsMatrix44 &w2l,
								plErrorMsg *pErrMsg, plConvertSettings *settings )
{
	if( NumBones() )
		ISetupBones( drawable, spanArray, l2w, w2l, pErrMsg, settings );

	// Assign spans to the drawables, plus set the volatile flag on the 
	// drawables for the SceneViewer, just in case it hasn't been set yet
	if( settings->fSceneViewer )
	{
		drawable->SetNativeProperty( plDrawable::kPropVolatile, true );
		index = drawable->AppendDISpans( spanArray, index, false );
	}
	else
		index = drawable->AddDISpans( spanArray, index );

	if( GetItinerant() )
		drawable->SetNativeProperty(plDrawable::kPropCharacter, true);
}

// Tiny helper for the function below
void SetSpansBoneInfo(hsTArray<plGeometrySpan *> &spanArray, UInt32 baseMatrix, UInt32 numMatrices)
{
	int i;
	for( i = 0; i < spanArray.GetCount(); i++ )
	{
		spanArray[ i ]->fBaseMatrix = baseMatrix;
		spanArray[ i ]->fNumMatrices = numMatrices;
	}
}

//// ISetupBones /////////////////////////////////////////////////////////////
//	Adds the given bones to the given drawable, then sets up the given spans
//	with the right indices and sets the initial bone positions.
void	plMaxNode::ISetupBones(plDrawableSpans *drawable, hsTArray<plGeometrySpan *> &spanArray,
								hsMatrix44 &l2w, hsMatrix44 &w2l,
								plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	const char* dbgNodeName = GetName();

	if( !NumBones() )
		return;

	plMaxBoneMap *boneMap = GetBoneMap();
	if (boneMap && boneMap->GetBaseMatrixIndex(drawable) != (UInt32)-1)
	{
		SetSpansBoneInfo(spanArray, boneMap->GetBaseMatrixIndex(drawable), boneMap->fNumBones);
		return;
	}
	
	int	baseMatrix, i;

	UInt8 numBones = (boneMap ? boneMap->fNumBones : NumBones()) + 1;
	plMaxNodeBase **boneArray = TRACKED_NEW plMaxNodeBase*[numBones];

	if (boneMap)
		boneMap->FillBoneArray(boneArray);
	else
	{
		for (i = 0; i < NumBones(); i++)
		{
			boneArray[i] = GetBone(i);
		}
	}

	hsTArray<hsMatrix44>	initialB2W;
	hsTArray<hsMatrix44>	initialW2B;
	initialB2W.SetCount(numBones);
	initialW2B.SetCount(numBones);

	hsTArray<hsMatrix44>	initialL2B;
	hsTArray<hsMatrix44>	initialB2L;
	initialL2B.SetCount(numBones);
	initialB2L.SetCount(numBones);

	initialB2W[0].Reset();
	initialW2B[0].Reset();

	initialL2B[0].Reset();
	initialB2L[0].Reset();

	for( i = 1; i < numBones; i++ )
	{
		hsMatrix44 b2w;
		hsMatrix44 w2b;
		hsMatrix44 l2b;
		hsMatrix44 b2l;

		plMaxNodeBase *bone = boneArray[i-1];
		const char* dbgBoneName = bone->GetName();

		Matrix3 localTM = bone->GetNodeTM(TimeValue(0));

		b2w = Matrix3ToMatrix44(localTM);
		b2w.GetInverse(&w2b);

		l2b = w2b * l2w;
		b2l = w2l * b2w;

		initialB2W[i] = b2w;
		initialW2B[i] = w2b;

		initialL2B[i] = l2b;
		initialB2L[i] = b2l;
	}

	// First, see if the bones are already set up appropriately.
	// Appropriately means:
	// a) Associated with the correct drawable (maybe others too, we don't care).
	// b) InitialBone transforms match. If we (or another user of the same bone)
	//		are force localed, Our InitialBone won't match, because it also includes
	//		our transform as well as the bone's. If we've been flattened into world
	//		space, our transform is ident and we can share. This is the normal case
	//		in scene boning. So InitialBones have to match in count and matrix value.
	baseMatrix = drawable->FindBoneBaseMatrix(initialL2B, GetSwappableGeom() != nil);
	if( baseMatrix != UInt32(-1) )
	{
		SetSpansBoneInfo(spanArray, baseMatrix, numBones);
		delete [] boneArray;
		return;
	}
	
	baseMatrix = drawable->AppendDIMatrixSpans(numBones);
	SetSpansBoneInfo(spanArray, baseMatrix, numBones);
	if (boneMap)
		boneMap->SetBaseMatrixIndex(drawable, baseMatrix);

	for( i = 1; i < numBones; i++ )
	{
		plMaxNodeBase *bone = boneArray[i-1];
		plSceneObject* obj = bone->GetSceneObject();
		const char	*dbgBoneName = bone->GetName();

		// Pick which drawable to point the DI to
		UInt8 iDraw = 0;

		/// Now create the actual bone DI, or grab it if it's already created
		plDrawInterface *di = obj->GetVolatileDrawInterface();
		if( di )
		{
			for( iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++ )
			{
				if( di->GetDrawable(iDraw) == drawable )
					break;
			}
		}
		else
		{
			plLocation nodeLoc = bone->GetLocation();
			di = TRACKED_NEW plDrawInterface;
			plKey diKey = hsgResMgr::ResMgr()->NewKey(GetKey()->GetName(), di, nodeLoc, GetLoadMask());
			hsgResMgr::ResMgr()->AddViaNotify(diKey, TRACKED_NEW plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
		}

		if( di->GetNumDrawables() <= iDraw )
		{
			UInt32 diIndex = drawable->NewDIMatrixIndex();
			di->SetDrawableMeshIndex(iDraw, diIndex);

			di->SetDrawable(iDraw, drawable);
		}


		plDISpanIndex& skinIndices = drawable->GetDISpans(di->GetDrawableMeshIndex(iDraw));
		skinIndices.Append(baseMatrix + i);

		drawable->SetInitialBone(baseMatrix + i, initialL2B[i], initialB2L[i]);
		di->SetTransform(initialB2W[i], initialW2B[i]);
	}
	delete [] boneArray;
}

//// IMakeInstanceSpans //////////////////////////////////////////////////////
//	Given an instance node, instances the geoSpans that the node owns and
//	stores them in the given array.

hsBool	plMaxNode::IMakeInstanceSpans( plMaxNode *node, hsTArray<plGeometrySpan *> &spanArray,
									   plErrorMsg *pErrMsg, plConvertSettings *settings )
{
	UInt8	iDraw;
	int		index, i;

	
	plSceneObject *obj = node->GetSceneObject();
	if( !obj )
		return false;

	const plDrawInterface *di = obj->GetDrawInterface();
	if( !di )
		return false;

	hsBool setVisDists = false;
	hsScalar minDist, maxDist;
	if( hsMaterialConverter::HasVisDists(this, 0, minDist, maxDist) )
	{
		setVisDists = true;
	}

	index = 0;
	spanArray.Reset();
	for( iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
		if( !dr )
			continue;
		if( di->GetDrawableMeshIndex(iDraw) == (UInt32)-1 )
			continue;

		plDISpanIndex disi = dr->GetDISpans(di->GetDrawableMeshIndex(iDraw));

		spanArray.ExpandAndZero( spanArray.GetCount() + disi.fIndices.GetCount() );
		for( i = 0; i < disi.fIndices.GetCount(); i++ )
		{
			spanArray[ index ] = TRACKED_NEW plGeometrySpan;
			spanArray[ index ]->MakeInstanceOf( dr->GetGeometrySpan( disi.fIndices[ i ] ) );

			if( setVisDists )
			{
				spanArray[ index ]->fMinDist = (minDist);
				spanArray[ index ]->fMaxDist = (maxDist);
			}

			dr->GetGeometrySpan(disi.fIndices[i])->fProps |= plGeometrySpan::kInstanced;

			spanArray[ index++ ]->fProps |= plGeometrySpan::kInstanced;
		}
	}

	// Now that we have all of our instanced spans, we need to make sure we
	// have the right materials. Why? There are some isolated cases (such as when "force
	// material copy" is set) where we want to still instance the geometry but we want
	// separate materials. In this case, GetMaterialArray() will return the right array of
	// materials for our instanced node. However, since we've tossed everything except the
	// final plGeometrySpans from MakeMesh(), we have to do a reverse lookup to see what
	// materials get assigned to whom. GetMaterialArray() is guaranteed (according to Bob)
	// to return materials in the same order for instanced nodes, so what we do is call
	// GMA() for the old node and the new node (the old one should just be a lookup), then
	// for each geoSpan look its old material up in the old array, find the matching material
	// in the new array (i.e. same position) and assign that new material to the span.
#if 1		// Change this to 0 to just always use the same materials on instances (old, incorrect way)
	Mtl *newMtl = GetMtl(), *origMtl = node->GetMtl();
	if( newMtl != nil && newMtl == origMtl )	// newMtl should == origMtl, but check just in case
	{
		hsTArray<hsGMaterial *>	oldMaterials, newMaterials;

		if( hsMaterialConverter::IsMultiMat( newMtl ) )
		{
			for( i = 0; i < newMtl->NumSubMtls(); i++ )
			{
				hsMaterialConverter::Instance().GetMaterialArray( origMtl->GetSubMtl( i ), node, oldMaterials );
				hsMaterialConverter::Instance().GetMaterialArray( newMtl->GetSubMtl( i ), this, newMaterials );
			}
		}
		else
		{
			hsMaterialConverter::Instance().GetMaterialArray( origMtl, node, oldMaterials );
			hsMaterialConverter::Instance().GetMaterialArray( newMtl, this, newMaterials );
		}

		/// Now we have two arrays to let us map, so walk through our geoSpans and translate them!
		/// The good thing is that this is all done before the spans are added to the drawable,
		/// so we don't have to worry about reffing or unreffing or any of that messiness; all of
		/// that will be done for us as part of the normal AppendDISpans() process.
		for( i = 0; i < spanArray.GetCount(); i++ )
		{
			int		j;


			// Find the span's original material
			for( j = 0; j < oldMaterials.GetCount(); j++ )
			{
				if( spanArray[ i ]->fMaterial == oldMaterials[ j ] )
				{
					spanArray[ i ]->fMaterial = newMaterials[ j ];
					break;
				}
			}

		}
	}
#endif

	return true;
}

//// IBuildInstanceList //////////////////////////////////////////////////////
//	For the given object, builds a list of all the iNodes that have that
//	object as their object. Returns the total node count

UInt32	plMaxNode::IBuildInstanceList( Object *obj, TimeValue t, hsTArray<plMaxNode *> &nodes, hsBool beMoreAccurate )
{
	Object				*thisObj = EvalWorldState( t ).obj;
	DependentIterator	di( obj );
	ReferenceMaker		*rm;
	plMaxNode			*node;
	plKey				sceneNodeKey = GetRoomKey();


	/// Use the DependentIterator to loop through all the dependents of the object,
	/// looking for nodes that use it
	nodes.Reset();
	while( rm = di.Next() )
	{
		if( rm->SuperClassID() == BASENODE_CLASS_ID )
		{
			node = (plMaxNode *)rm;
			if( node->EvalWorldState( t ).obj == thisObj )
			{
				// Note: we CANNOT instance across pages (i.e. sceneNodes), so we need to make sure this
				// INode will be in the same page as our master object

				// Also note: RoomKeys will be nil until we've finished the first component pass, so when
				// we test this in ConvertValidate(), the keys will be nil and all objects will be "in the
				// same room", even though they're not. This is not too bad, though, since the worst that
				// could happen is the object gets forced local even when there ends up not being any other
				// instances of it in the same page. Ooooh.

				if( sceneNodeKey == node->GetRoomKey() )
				{
					// Make sure the materials generated for both of these nodes will be the same
					if( IMaterialsMatch( node, beMoreAccurate ) )
						nodes.Append( node );
				}
			}
		}
	}

	return nodes.GetCount();
}

//// IMaterialsMatch /////////////////////////////////////////////////////////
//	Given two nodes that are instances of each other, this function determines
//	whether the resulting exported materials for both will be the same or not.
//	If not, we need to not instance/share the geometry, since the UV channels
//	could (and most likely will) be different.
//	To test this, all we really need to do is check the return values of
//	AlphaHackLayersNeeded(), since all the other material parameters will be
//	identical due to these nodes being instances of each other.

hsBool	plMaxNode::IMaterialsMatch( plMaxNode *otherNode, hsBool beMoreAccurate )
{
	Mtl *mtl = GetMtl(), *otherMtl = otherNode->GetMtl();
	if( mtl != otherMtl )
		return false;	// The two objects have different materials, no way we
						// can try to instance them now
	if( mtl == nil )
		return true;	// Both nodes have no material, works for me

	// If we're not told to be accurate, then we just quit here. This is because
	// in the early passes, we *can't* be more accurate, since we won't have all
	// the info yet, so we don't bother checking it
	if( !beMoreAccurate )
		return true;

	if( hsMaterialConverter::IsMultiMat( mtl ) )
	{
		int		i;
		for( i = 0; i < mtl->NumSubMtls(); i++ )
		{
			if( AlphaHackLayersNeeded( i ) != otherNode->AlphaHackLayersNeeded( i ) )
				return false;
		}
	}
	else
	{
		if( AlphaHackLayersNeeded( -1 ) != otherNode->AlphaHackLayersNeeded( -1 ) )
			return false;
	}

	// They're close enough!
	return true;
}

hsBool plMaxNode::ShadeMesh(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	const char* dbgNodeName = GetName();

	hsTArray<plGeometrySpan *> spanArray;

	if( !(CanConvert() && GetDrawable()) ) 
		return true;

	plSceneObject* obj = GetSceneObject();
	if( !obj )
		return true;

	const plDrawInterface* di = obj->GetDrawInterface();
	if( !di )
		return true;

	UInt8 iDraw;
	for( iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++ )
	{
		plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
		if( !dr )
			continue;
		if( di->GetDrawableMeshIndex(iDraw) == (UInt32)-1 )
			continue;

		plDISpanIndex disi = dr->GetDISpans(di->GetDrawableMeshIndex(iDraw));

		int i;
		for( i = 0; i < disi.fIndices.GetCount(); i++ )
		{
			spanArray.Append( dr->GetGeometrySpan( disi.fIndices[ i ] ) );
		}

		hsMatrix44 l2w = GetLocalToWorld44();
		hsMatrix44 w2l = GetWorldToLocal44();

		/// Shade the spans now
		// Either do vertex shading or generate a light map.
		if( GetLightMapComponent() )
		{
			plLightMapGen::Instance().MakeMaps(this, l2w, w2l, spanArray, pErrMsg, nil);

			// Since they were already pointers to the geometry spans, we don't have
			// to re-stuff them. Horray!
		}
		else
		{
			hsVertexShader::Instance().ShadeNode(this, l2w, w2l, spanArray);
		}

		if (settings && settings->fSceneViewer)
			dr->RefreshDISpans(di->GetDrawableMeshIndex(iDraw));
	}
	return true;
}

hsBool plMaxNode::MakeOccluder(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	if( !UserPropExists("Occluder") )
		return true;

	hsBool twoSided = UserPropExists("OccTwoSided");
	hsBool isHole = UserPropExists("OccHole");

	return ConvertToOccluder(pErrMsg, twoSided, isHole);
}

static void IRemoveCollinearPoints(hsTArray<Point3>& facePts)
{
	int i;
	for( i = 0; i < facePts.GetCount(); )
	{
		int j = i + 1 >= facePts.GetCount() ? 0 : i + 1;
		int k = j + 1 >= facePts.GetCount() ? 0 : j + 1;
		Point3 ab = FNormalize(facePts[i] - facePts[j]);
		Point3 bc = FNormalize(facePts[j] - facePts[k]);

		const float kDotCutoff = 1.f - 1.e-3f;
		float dot = DotProd(ab, bc);
		if( (dot < kDotCutoff) && (dot > -kDotCutoff) )
		{
			i++;
		}
		else
		{
			facePts.Remove(j);
		}
	}
}

hsBool plMaxNode::ConvertToOccluder(plErrorMsg* pErrMsg, hsBool twoSided, hsBool isHole)
{
	if( !CanConvert() )
		return false;

	/// Get some stuff
	plLocation nodeLoc = GetLocation();

	hsBool moving = IsMovable();
	if( moving )
		moving++;

	Matrix3 tmp(true);

	Matrix3 maxL2V = GetLocalToVert(TimeValue(0));
	Matrix3 maxV2L = GetVertToLocal(TimeValue(0));

	hsTArray<plCullPoly> polys;

	UInt32 polyInitFlags = plCullPoly::kNone;
	if( isHole )
		polyInitFlags |= plCullPoly::kHole;
	else
	if( twoSided )
		polyInitFlags |= plCullPoly::kTwoSided;

	Object *obj = EvalWorldState(TimeValue(0)).obj;
	if( obj->CanConvertToType(triObjectClassID) )
	{
		TriObject	*meshObj = (TriObject *)obj->ConvertToType(TimeValue(0), triObjectClassID);
		if( meshObj )
		{

			Mesh mesh(meshObj->mesh);
			
			const float kNormThresh = hsScalarPI / 20.f;
			const float kEdgeThresh = hsScalarPI / 20.f;
			const float kBias = 0.1f;
			const float kMaxEdge = -1.f;
			const DWORD kOptFlags = OPTIMIZE_SAVESMOOTHBOUNDRIES; 

			mesh.Optimize(
				kNormThresh, // threshold of normal differences to preserve
				kEdgeThresh, // When the angle between adjacent surface normals is less than this value the auto edge is performed (if the OPTIMIZE_AUTOEDGE flag is set). This angle is specified in radians.
				kBias, // Increasing the bias parameter keeps triangles from becoming degenerate. range [0..1] (0 = no bias).
				kMaxEdge, // This will prevent the optimize function from creating edges longer than this value. If this parameter is <=0 no limit is placed on the length of the edges.
				kOptFlags, // Let them input using smoothing groups, but nothing else.
				NULL); // progress bar

			
			MNMesh mnMesh(mesh);

			mnMesh.EliminateCollinearVerts();
			mnMesh.EliminateCoincidentVerts(0.1f);

			// Documentation recommends MakeConvexPolyMesh over MakePolyMesh. Naturally, MakePolyMesh works better.
//			mnMesh.MakeConvexPolyMesh();
			mnMesh.MakePolyMesh();
			mnMesh.MakeConvex();
//			mnMesh.MakePlanar(1.f * hsScalarPI / 180.f); // Completely ineffective. Winding up with majorly non-planar polys.

			mnMesh.Transform(maxV2L);

			polys.SetCount(mesh.getNumFaces());
			polys.SetCount(0);

			// Unfortunate problem here. Max is assuming that eventually this will get rendered, and so
			// we need to avoid T-junctions. Fact is, T-junctions don't bother us at all, where-as colinear
			// verts within a poly do (just as added overhead).
			// So, to make this as painless (ha ha) as possible, we could detach each poly as we go to
			// its own mnMesh, then eliminate colinear verts on that single poly mesh. Except
			// EliminateCollinearVerts doesn't seem to actually do that. So we'll just have to
			// manually detect and skip collinear verts.
			hsTArray<Point3> facePts;
			int i;
			for( i = 0; i < mnMesh.numf; i++ )
			{
				MNFace& face = mnMesh.f[i];

				facePts.SetCount(0);
				int j;
				for( j = 0; j < face.deg; j++ )
				{
					facePts.Append(mnMesh.v[face.vtx[j]].p);
				}
				IRemoveCollinearPoints(facePts);

				if( facePts.GetCount() < 3 )
					continue;

				int lastAdded = 2;

				plCullPoly* poly = polys.Push();
				poly->fVerts.SetCount(0);

				Point3 p;
				hsPoint3 pt;

				p = facePts[0];
				pt.Set(p.x, p.y, p.z);
				poly->fVerts.Append(pt);

				p = facePts[1];
				pt.Set(p.x, p.y, p.z);
				poly->fVerts.Append(pt);

				p = facePts[2];
				pt.Set(p.x, p.y, p.z);
				poly->fVerts.Append(pt);

				for( j = lastAdded+1; j < facePts.GetCount(); j++ )
				{
					p = facePts[j];
					pt.Set(p.x, p.y, p.z);

					hsVector3 a = hsVector3(&pt, &poly->fVerts[0]);
					hsVector3 b = hsVector3(&poly->fVerts[lastAdded], &poly->fVerts[0]);
					hsVector3 c = hsVector3(&poly->fVerts[lastAdded-1], &poly->fVerts[0]);

					hsVector3 aXb = a % b;
					hsVector3 bXc = b % c;

					hsFastMath::Normalize(aXb);
					hsFastMath::Normalize(bXc);


					hsScalar dotSq = aXb.InnerProduct(bXc);
					dotSq *= dotSq;

					const hsScalar kMinLenSq = 1.e-8f;
					const hsScalar kMinDotFracSq = 0.998f * 0.998f;

					hsScalar lenSq = aXb.MagnitudeSquared() * bXc.MagnitudeSquared();
					if( lenSq < kMinLenSq )
						continue;

					// If not planar, move to new poly.
					if( dotSq < lenSq * kMinDotFracSq )
					{
						poly->InitFromVerts(polyInitFlags);

						poly = polys.Push();
						plCullPoly* lastPoly = &polys[polys.GetCount()-2];
						poly->fVerts.SetCount(0);
						poly->fVerts.Append(lastPoly->fVerts[0]);
						poly->fVerts.Append(lastPoly->fVerts[lastAdded]);
	
						lastAdded = 1;
					}

					poly->fVerts.Append(pt);
					lastAdded++;
				}

				poly->InitFromVerts(polyInitFlags);
			}
		}
	}

	if( polys.GetCount() )
	{
		plOccluder* occ = nil;
		plMobileOccluder* mob = nil;
		if( moving )
		{
			mob = TRACKED_NEW plMobileOccluder;
			occ = mob;
		}
		else
		{
			occ = TRACKED_NEW plOccluder;
		}

		occ->SetPolyList(polys);
		occ->ComputeFromPolys();

		// Register it.
		char tmpName[256];
		if( GetKey() && GetKey()->GetName() && *GetKey()->GetName() )
		{
			sprintf(tmpName, "%s_%s", GetKey()->GetName(), "Occluder");
		}
		else
		{
			static int numOcc = 0;
			sprintf(tmpName, "%s_%4.4d", "Occluder", numOcc);
		}
		plKey key = hsgResMgr::ResMgr()->NewKey( tmpName, occ, nodeLoc, GetLoadMask() );

		hsgResMgr::ResMgr()->AddViaNotify(occ->GetKey(), TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

	}
	return true;
}

hsBool plMaxNode::MakeLight(plErrorMsg *pErrMsg, plConvertSettings *settings)
{

	if (!CanConvert()) 
		return false;

	if (!GetRunTimeLight())
		return true;

	/// Get some stuff
	plLocation nodeLoc = GetLocation();
	hsBool forceLocal = GetForceLocal();

	hsMatrix44 l2w = GetLocalToWorld44();
	hsMatrix44 w2l = GetWorldToLocal44();

	hsMatrix44 lt2l = GetVertToLocal44();
	hsMatrix44 l2lt = GetLocalToVert44();


	plLightInfo* liInfo = nil;


	liInfo = IMakeLight(pErrMsg, settings);

	if( liInfo )
	{
		// 12.03.01 mcn - Um, we want RT lights to affect static objects if they're animated. So 
		// why wasn't this here a long time ago? :~
		if( IsMovable() || IsAnimatedLight() )
			liInfo->SetProperty(plLightInfo::kLPMovable, true);

		liInfo->SetTransform(l2w, w2l);
		liInfo->SetLocalToLight(l2lt, lt2l);

		plKey key = hsgResMgr::ResMgr()->NewKey( GetKey()->GetName(), liInfo, nodeLoc, GetLoadMask() );

		hsgResMgr::ResMgr()->AddViaNotify(liInfo->GetKey(), TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);


		// Only support projection for spots and dir lights for now.
		if( plLimitedDirLightInfo::ConvertNoRef(liInfo) || plSpotLightInfo::ConvertNoRef(liInfo) )
		{
			// Have to do this after the drawable gets a key.
			IGetProjection(liInfo, pErrMsg);

		}
	}

	return true;
}

plLightInfo* plMaxNode::IMakeLight(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
	plLightInfo* liInfo = nil;
	Object *obj = EvalWorldState(timeVal).obj;
	if( obj->ClassID() == Class_ID(OMNI_LIGHT_CLASS_ID, 0) )
		liInfo = IMakeOmni(pErrMsg, settings);
	else 
	if( (obj->ClassID() == Class_ID(SPOT_LIGHT_CLASS_ID, 0)) || (obj->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0)) )
		liInfo = IMakeSpot(pErrMsg, settings);
	else 
	if( (obj->ClassID() == Class_ID(DIR_LIGHT_CLASS_ID, 0)) || (obj->ClassID() == Class_ID(TDIR_LIGHT_CLASS_ID, 0)) )
		liInfo = IMakeDirectional(pErrMsg, settings);
	else
	if( obj->ClassID() == RTOMNI_LIGHT_CLASSID )
		liInfo = IMakeRTOmni(pErrMsg, settings);
	else
	if( (obj->ClassID() == RTSPOT_LIGHT_CLASSID) ) //|| (obj->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0)) )
		liInfo = IMakeRTSpot(pErrMsg, settings);
	else 
	if( (obj->ClassID() == RTDIR_LIGHT_CLASSID) ) //|| (obj->ClassID() == Class_ID(FSPOT_LIGHT_CLASS_ID, 0)) )
		liInfo = IMakeRTDirectional(pErrMsg, settings);
	else
	if( obj->ClassID() == RTPDIR_LIGHT_CLASSID )
		liInfo = IMakeRTProjDirectional( pErrMsg, settings );

	return liInfo;
}

void plMaxNode::IGetLightAttenuation(plOmniLightInfo* liInfo, LightObject* light, LightState& ls)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	hsScalar attenConst, attenLinear, attenQuadratic;

	float intens = ls.intens >= 0 ? ls.intens : -ls.intens;
	float attenEnd = ls.attenEnd;
	// Decay type 0:None, 1:Linear, 2:Squared
	if( ls.useAtten )
	{
		switch(((GenLight*)light)->GetDecayType())
		{
		case 0:
		case 1:
			attenConst = 1.f;
			attenLinear = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / attenEnd;
			attenQuadratic = 0;
			break;
		case 2:
			attenConst = 1.f;
			attenLinear = 0;
			attenQuadratic = (intens * plSillyLightKonstants::GetFarPowerKonst() -1.f) / (attenEnd * attenEnd);
			break;
		case 3:
			attenConst = intens;		
			attenLinear = 0.f;
			attenQuadratic = 0.f;
			liInfo->SetCutoffAttenuation( ( (GenLight *)light )->GetDecayRadius( timeVal ) );
			break;
		}
	}
	else
	{
		attenConst = 1.f;
		attenLinear = 0.f;
		attenQuadratic = 0.f;
	}

	liInfo->SetConstantAttenuation(attenConst);
	liInfo->SetLinearAttenuation(attenLinear);
	liInfo->SetQuadraticAttenuation(attenQuadratic);

}

hsBool plMaxNode::IGetRTLightAttenValues(IParamBlock2* ProperPB, hsScalar& attenConst, hsScalar& attenLinear, hsScalar& attenQuadratic, hsScalar &attenCutoff )
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	float intens = ProperPB->GetFloat(plRTLightBase::kIntensity, timeVal);
	if( intens < 0 )
		intens = -intens;
	float attenEnd;
	
	attenEnd = ProperPB->GetFloat(plRTLightBase::kAttenMaxFalloffEdit, timeVal);//ls.attenEnd;

	// Decay Type New == 0 for Linear and 1 for Squared.... OLD and OBSOLETE:Decay type 0:None, 1:Linear, 2:Squared
	// Oh, and now 2 = cutoff attenuation
	if( ProperPB->GetInt(plRTLightBase::kUseAttenuationBool, timeVal))
	{
		switch(ProperPB->GetInt(plRTLightBase::kAttenTypeRadio, timeVal))//((GenLight*)light)->GetDecayType())
		{
		case 0:
			attenConst = 1.f;
			attenLinear = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / attenEnd;
			if( attenLinear < 0 )
				attenLinear = 0;
			attenQuadratic = 0;
			attenCutoff = attenEnd;
			break;
		case 1:
			attenConst = 1.f;
			attenLinear = 0;
			attenQuadratic = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / (attenEnd * attenEnd);
			if( attenQuadratic < 0 )
				attenQuadratic = 0;
			attenCutoff = attenEnd;
			break;
		case 2:
			attenConst = intens;
			attenLinear = 0.f;
			attenQuadratic = 0.f;
			attenCutoff = attenEnd;
			break;
		}
		return true;
	}
	else
	{
		attenConst = 1.f;
		attenLinear = 0.f;
		attenQuadratic = 0.f;
		attenCutoff = 0.f;
		return true;
	}

	return false;
}

void plMaxNode::IGetRTLightAttenuation(plOmniLightInfo* liInfo, IParamBlock2* ProperPB)
{
	hsScalar attenConst, attenLinear, attenQuadratic, attenCutoff;

	if( IGetRTLightAttenValues(ProperPB, attenConst, attenLinear, attenQuadratic, attenCutoff) )
	{
		liInfo->SetConstantAttenuation(attenConst);
		liInfo->SetLinearAttenuation(attenLinear);
		liInfo->SetQuadraticAttenuation(attenQuadratic);
		liInfo->SetCutoffAttenuation( attenCutoff );
	}
}

void plMaxNode::IGetLightColors(plLightInfo* liInfo, LightObject* light, LightState& ls)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Point3 color = light->GetRGBColor(timeVal);
	float intensity = light->GetIntensity(timeVal);

	color *= intensity;

	liInfo->SetAmbient(hsColorRGBA().Set(0,0,0,1.f));
	if( ls.affectDiffuse )
		liInfo->SetDiffuse(hsColorRGBA().Set(color.x, color.y, color.z, intensity));
	else
		liInfo->SetDiffuse(hsColorRGBA().Set(0,0,0,intensity));
	if( ls.affectSpecular )
		liInfo->SetSpecular(hsColorRGBA().Set(color.x, color.y, color.z, intensity));
	else
		liInfo->SetSpecular(hsColorRGBA().Set(0,0,0,intensity));

}

void plMaxNode::IGetRTLightColors(plLightInfo* liInfo, IParamBlock2* ProperPB)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Point3 color = ProperPB->GetPoint3(plRTLightBase::kLightColor, timeVal);//light->GetRGBColor(timeVal);
	float intensity = ProperPB->GetFloat(plRTLightBase::kIntensity, timeVal); //light->GetIntensity(timeVal);

	color *= intensity;

	liInfo->SetAmbient(hsColorRGBA().Set(0,0,0,1.f));
	if( ProperPB->GetInt( plRTLightBase::kAffectDiffuse, timeVal ) )
		liInfo->SetDiffuse(hsColorRGBA().Set(color.x, color.y, color.z, intensity));
	else
		liInfo->SetDiffuse(hsColorRGBA().Set(0,0,0,intensity));
	if( ProperPB->GetInt(plRTLightBase::kSpec, timeVal)) //ls.affectSpecular )
	{
		Color spec = ProperPB->GetColor(plRTLightBase::kSpecularColorSwatch);
		liInfo->SetSpecular(hsColorRGBA().Set(spec.r, spec.g, spec.b, intensity));
	}
	else
		liInfo->SetSpecular(hsColorRGBA().Set(0,0,0,intensity));
}

void plMaxNode::IGetCone(plSpotLightInfo* liInfo, LightObject* light, LightState& ls)
{

	hsScalar inner = hsScalarDegToRad(ls.hotsize);
	hsScalar outer = hsScalarDegToRad(ls.fallsize);

	/// 4.26.2001 mcn - MAX gives us full angles, but we want to store half angles
	liInfo->SetSpotInner( inner / 2.0f );
	liInfo->SetSpotOuter( outer / 2.0f );
	liInfo->SetFalloff(1.f);
}

void plMaxNode::IGetRTCone(plSpotLightInfo* liInfo, IParamBlock2* ProperPB)
{

	//TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
	hsScalar inner, outer;

	inner = hsScalarDegToRad(ProperPB->GetFloat(plRTLightBase::kHotSpot, timeVal)); //ls.hotsize);
	outer = hsScalarDegToRad(ProperPB->GetFloat(plRTLightBase::kFallOff, timeVal)); //ls.fallsize);

	/// 4.26.2001 mcn - MAX gives us full angles, but we want to store half angles
	liInfo->SetSpotInner( inner / 2.0f );
	liInfo->SetSpotOuter( outer / 2.0f );
	liInfo->SetFalloff(1.f);

}

plLightInfo* plMaxNode::IMakeSpot(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
	Object *obj = EvalWorldState(timeVal).obj;
	LightObject *light = (LightObject*)obj->ConvertToType(timeVal, Class_ID(SPOT_LIGHT_CLASS_ID,0));
	if( !light )
		light = (LightObject*)obj->ConvertToType(timeVal, Class_ID(FSPOT_LIGHT_CLASS_ID,0)); 

	LightState ls;
	if (!(REF_SUCCEED == light->EvalLightState(timeVal, Interval(timeVal, timeVal), &ls)))
	{
		pErrMsg->Set(true, GetName(), "Trouble evaluating light").CheckAndAsk();
		return nil;
	}

	plSpotLightInfo* spot = TRACKED_NEW plSpotLightInfo;

	IGetLightColors(spot, light, ls);

	IGetLightAttenuation(spot, light, ls);

	IGetCone(spot, light, ls);

	if( obj != light )
		light->DeleteThis();

	return spot;
}

plLightInfo* plMaxNode::IMakeOmni(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Object *obj = EvalWorldState(timeVal).obj;
	LightObject *light = (LightObject*)obj->ConvertToType(timeVal, 
		Class_ID(OMNI_LIGHT_CLASS_ID,0));

	LightState ls;
	if (!(REF_SUCCEED == light->EvalLightState(timeVal, Interval(timeVal, timeVal), &ls)))
	{
		pErrMsg->Set(true, GetName(), "Trouble evaluating light").CheckAndAsk();
		return nil;
	}

	plOmniLightInfo* omni = TRACKED_NEW plOmniLightInfo;

	IGetLightAttenuation(omni, light, ls);

	IGetLightColors(omni, light, ls);

	if( obj != light )
		light->DeleteThis();

	return omni;
}

plLightInfo* plMaxNode::IMakeDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Object *obj = EvalWorldState(timeVal).obj;
	LightObject *light = (LightObject*)obj->ConvertToType(timeVal, Class_ID(DIR_LIGHT_CLASS_ID,0));
	if( !light )
		light = (LightObject*)obj->ConvertToType(timeVal, Class_ID(TDIR_LIGHT_CLASS_ID,0)); 

	LightState ls;
	if (!(REF_SUCCEED == light->EvalLightState(timeVal, Interval(timeVal, timeVal), &ls)))
	{
		pErrMsg->Set(true, GetName(), "Trouble evaluating light").CheckAndAsk();
		return nil;
	}

	plLightInfo* plasLight = nil;
	if( light->GetProjMap() )
	{
		plLimitedDirLightInfo* ldl = TRACKED_NEW plLimitedDirLightInfo;

		float sz = light->GetFallsize(timeVal, FOREVER);
		float depth = 1000.f;
		ldl->SetWidth(sz);
		ldl->SetHeight(sz);
		ldl->SetDepth(depth);

		plasLight = ldl;
	}
	else
	{

		plDirectionalLightInfo* direct = TRACKED_NEW plDirectionalLightInfo;
		plasLight = direct;
	}

	IGetLightColors(plasLight, light, ls);

	if( obj != light )
		light->DeleteThis();

	return plasLight;
}

plLightInfo* plMaxNode::IMakeRTSpot(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
	Object *obj = EvalWorldState(timeVal).obj;
	Object *ThisObj = ((INode*)this)->GetObjectRef();
	IParamBlock2* ThisObjPB = ThisObj->GetParamBlockByID(plRTLightBase::kBlkSpotLight);


	if(!obj->CanConvertToType(RTSPOT_LIGHT_CLASSID))
	{
		pErrMsg->Set(true, GetName(), "Trouble evaluating light, improper classID").CheckAndAsk();
		return nil;

	}

	plSpotLightInfo* spot = TRACKED_NEW plSpotLightInfo;

	if(!ThisObjPB->GetInt(plRTLightBase::kLightOn))
		spot->SetProperty(plLightInfo::kDisable, true); 

	IGetRTLightColors(spot,ThisObjPB);

	IGetRTLightAttenuation(spot,ThisObjPB);

	IGetRTCone(spot, ThisObjPB);

	//plSpotModifier* liMod = TRACKED_NEW plSpotModifier;

	//GetRTLightColAnim(ThisObjPB, liMod);
	//GetRTLightAttenAnim(ThisObjPB, liMod);
	//GetRTConeAnim(ThisObjPB, liMod);

	//IAttachRTLightModifier(liMod);

//	if( obj != light )
	//	light->DeleteThis();

	return spot;
}


plLightInfo* plMaxNode::IMakeRTOmni(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Object *obj = EvalWorldState(timeVal).obj;
	
	Object *ThisObj = ((INode*)this)->GetObjectRef();
	IParamBlock2* ThisObjPB = ThisObj->GetParamBlockByID(plRTLightBase::kBlkOmniLight);

	plOmniLightInfo* omni = TRACKED_NEW plOmniLightInfo;

	if(!ThisObjPB->GetInt(plRTLightBase::kLightOn))
		omni->SetProperty(plLightInfo::kDisable, true); 

	IGetRTLightAttenuation(omni, ThisObjPB);

	IGetRTLightColors(omni, ThisObjPB);

	//plOmniModifier* liMod = TRACKED_NEW plOmniModifier;

	//GetRTLightColAnim(ThisObjPB, liMod);
	//GetRTLightAttenAnim(ThisObjPB, liMod);

	//IAttachRTLightModifier(liMod);


//	if( obj != light )
//		light->DeleteThis();

	return omni;
}


plLightInfo* plMaxNode::IMakeRTDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Object *obj = EvalWorldState(timeVal).obj;
	Object *ThisObj = ((INode*)this)->GetObjectRef();
	IParamBlock2* ThisObjPB = ThisObj->GetParamBlockByID(plRTLightBase::kBlkTSpotLight);


	plDirectionalLightInfo* direct = TRACKED_NEW plDirectionalLightInfo;

	if(!ThisObjPB->GetInt(plRTLightBase::kLightOn))
		direct->SetProperty(plLightInfo::kDisable, true); 

	IGetRTLightColors(direct, ThisObjPB);

	//plLightModifier* liMod = TRACKED_NEW plLightModifier;

	//GetRTLightColAnim(ThisObjPB, liMod);

	//IAttachRTLightModifier(liMod);


//	if( obj != light )
//		light->DeleteThis();

	return direct;
}

//// IMakeRTProjDirectional //////////////////////////////////////////////////
//	Conversion function for RT Projected Directional lights

plLightInfo	*plMaxNode::IMakeRTProjDirectional( plErrorMsg *pErrMsg, plConvertSettings *settings )
{
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

	Object *obj = EvalWorldState(timeVal).obj;
	Object *ThisObj = ((INode*)this)->GetObjectRef();

	IParamBlock2	*mainPB = ThisObj->GetParamBlockByID( plRTLightBase::kBlkMain );
	IParamBlock2	*projPB = ThisObj->GetParamBlockByID( plRTProjDirLight::kBlkProj );

	plLimitedDirLightInfo *light = TRACKED_NEW plLimitedDirLightInfo;

	light->SetWidth( projPB->GetFloat( plRTProjDirLight::kWidth ) );
	light->SetHeight( projPB->GetFloat( plRTProjDirLight::kHeight ) );
	light->SetDepth( projPB->GetFloat( plRTProjDirLight::kRange ) );

	if( !mainPB->GetInt( plRTLightBase::kLightOn ) )
		light->SetProperty( plLightInfo::kDisable, true ); 

	IGetRTLightColors( light, mainPB );

	//plLightModifier *liMod = TRACKED_NEW plLightModifier;

	//GetRTLightColAnim( mainPB, liMod );

	//IAttachRTLightModifier(liMod);

	return light;
}

hsBool plMaxNode::IGetProjection(plLightInfo* li, plErrorMsg* pErrMsg)
{
	hsBool persp = false;
	TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
	Object *obj = EvalWorldState(timeVal).obj;
	LightObject *light = (LightObject*)obj->ConvertToType(timeVal, RTSPOT_LIGHT_CLASSID);

	if( light )
		persp = true;
	
	if( !light )
		light = (LightObject*)obj->ConvertToType(timeVal, RTPDIR_LIGHT_CLASSID);

	if( !light )
		return false;

	hsBool retVal = false;
	Texmap* projMap = light->GetProjMap();
	if( !projMap )
		return false;

	plConvert& convert = plConvert::Instance();
	if( projMap->ClassID() != LAYER_TEX_CLASS_ID )
	{
		if( pErrMsg->Set(!(convert.fWarned & plConvert::kWarnedWrongProj), GetName(),
					"Only Plasma Layers supported for projection").CheckAskOrCancel() )
		{
			convert.fWarned |= plConvert::kWarnedWrongProj;
		}
		pErrMsg->Set(false);
		return false;
	}

	IParamBlock2 *pb = nil;

	Class_ID cid = obj->ClassID();

	// Get the paramblock
	if (cid == RTSPOT_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkSpotLight);
	else if (cid == RTOMNI_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkOmniLight);
	else if (cid == RTDIR_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkTSpotLight);
	else if (cid == RTPDIR_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTProjDirLight::kBlkProj);

	// Have the layer converter process this layer directly
	plLayerConverter::Instance().MuteWarnings();
	plLayerInterface* proj = plLayerConverter::Instance().ConvertTexmap( projMap, this, 0, true, false );
	plLayerConverter::Instance().UnmuteWarnings();
	if( proj )
	{
		plLayer* projLay = plLayer::ConvertNoRef(proj->BottomOfStack());
		if( projLay && projLay->GetTexture() )
		{
			if( persp )
				projLay->SetMiscFlags(projLay->GetMiscFlags() | hsGMatState::kMiscPerspProjection);
			else
				projLay->SetMiscFlags(projLay->GetMiscFlags() | hsGMatState::kMiscOrthoProjection);
			projLay->SetUVWSrc(projLay->GetUVWSrc() | plLayerInterface::kUVWPosition);
			projLay->SetClampFlags(hsGMatState::kClampTexture);
			projLay->SetZFlags(hsGMatState::kZNoZWrite);

			switch( pb->GetInt(plRTLightBase::kProjTypeRadio) )
			{
			default:
			case plRTLightBase::kIlluminate:
				projLay->SetBlendFlags(hsGMatState::kBlendMult);
				li->SetProperty(plLightInfo::kLPOverAll, false);
			break;
			case plRTLightBase::kAdd:
				projLay->SetBlendFlags(hsGMatState::kBlendAdd);
				li->SetProperty(plLightInfo::kLPOverAll, true);
			break;
			case plRTLightBase::kMult:
				projLay->SetBlendFlags(hsGMatState::kBlendMult | hsGMatState::kBlendInvertColor | hsGMatState::kBlendInvertFinalColor);
				li->SetProperty(plLightInfo::kLPOverAll, true);
			break;
			case plRTLightBase::kMADD:
				projLay->SetBlendFlags(hsGMatState::kBlendMADD);
				li->SetProperty(plLightInfo::kLPOverAll, true);
			break;
			}

			hsgResMgr::ResMgr()->AddViaNotify(proj->GetKey(), TRACKED_NEW plGenRefMsg(li->GetKey(), plRefMsg::kOnCreate, 0, 0), plRefFlags::kActiveRef);

			li->SetShadowCaster(false);

			li->SetProperty(plLightInfo::kLPMovable, true);


			retVal = true;
		}
		else
		{
			char buff[256];
			if( projMap && projMap->GetName() && *projMap->GetName() )
				sprintf(buff, "Can't find projected bitmap - %s", projMap->GetName());
			else
				sprintf(buff, "Can't find projected bitmap - <unknown>");
			if( pErrMsg->Set(!(convert.fWarned & plConvert::kWarnedMissingProj), GetName(),
					buff).CheckAskOrCancel() )
				convert.fWarned |= plConvert::kWarnedMissingProj;
			pErrMsg->Set(false);
			retVal = false;
		}
	}
	else
	{
		if( pErrMsg->Set(!(convert.fWarned & plConvert::kWarnedWrongProj), GetName(),
					"Failure to convert projection map - check type.").CheckAskOrCancel() )
			convert.fWarned |= plConvert::kWarnedWrongProj;
		pErrMsg->Set(false);
		retVal = false;
	}

	if( light != obj )
		light->DeleteThis();

	return retVal;
}
/*
hsBool plMaxNode::IAttachRTLightModifier(plLightModifier* liMod)
{
	if( liMod->HasAnima() )
	{
		liMod->DefaultAnimation();
		CreateModifierKey(liMod, "_ANIMA");
		AddModifier(liMod);
	}
	else
	{
		delete liMod;
		return false;
	}
	return true;
}
*/
bool plMaxNode::IsAnimatedLight()
{
	Object *obj = GetObjectRef();
	if (!obj)
		return false;

	const char* dbgNodeName = GetName();

	Class_ID cid = obj->ClassID();

	if (!(cid == RTSPOT_LIGHT_CLASSID ||
		cid == RTOMNI_LIGHT_CLASSID ||
		cid == RTDIR_LIGHT_CLASSID ||
		cid == RTPDIR_LIGHT_CLASSID))
		return false;

	IParamBlock2 *pb = nil;

	// Get the paramblock
	if (cid == RTSPOT_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkSpotLight);
	else if (cid == RTOMNI_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkOmniLight);
	else if (cid == RTDIR_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkTSpotLight);
	else if (cid == RTPDIR_LIGHT_CLASSID)
		pb = obj->GetParamBlockByID(plRTLightBase::kBlkMain);

	hsControlConverter& cc = hsControlConverter::Instance();

	// Is the color animated?
	Control *colorCtl = pb->GetController( ParamID( plRTLightBase::kLightColor ) );
	if (colorCtl && cc.HasKeyTimes(colorCtl))
		return true;

	// Is the specularity animated?
	Control *specCtl = pb->GetController( ParamID( plRTLightBase::kSpecularColorSwatch ) );
	if (specCtl && cc.HasKeyTimes(specCtl))
		return true;

	// Is the attenuation animated?  (Spot and Omni lights only)
	if (cid == RTSPOT_LIGHT_CLASSID || cid == RTOMNI_LIGHT_CLASSID)
	{
		Control *falloffCtl = pb->GetController( ParamID( plRTLightBase::kAttenMaxFalloffEdit ) );
		if (falloffCtl && cc.HasKeyTimes(falloffCtl))
			return true;
	}

	// Is the cone animated? (Spot only)
	if (cid == RTSPOT_LIGHT_CLASSID)
	{
		Control *innerCtl = pb->GetController( ParamID( plRTLightBase::kHotSpot ) );
		if (innerCtl && cc.HasKeyTimes(innerCtl))
			return true;

		Control *outerCtl = pb->GetController( ParamID( plRTLightBase::kFallOff ) );
		if (outerCtl && cc.HasKeyTimes(outerCtl))
			return true;
	}

	return false;
}


void plMaxNode::GetRTLightAttenAnim(IParamBlock2* ProperPB, plAGAnim *anim)
{
	if( ProperPB->GetInt(plRTLightBase::kUseAttenuationBool, TimeValue(0)) )
	{
		Control* falloffCtl = ProperPB->GetController(ParamID(plRTLightBase::kAttenMaxFalloffEdit));
		if( falloffCtl )
		{
			plLeafController* subCtl;
			if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
				subCtl = hsControlConverter::Instance().MakeScalarController(falloffCtl, this);
			else
				subCtl = hsControlConverter::Instance().MakeScalarController(falloffCtl, this,
																			anim->GetStart(), anim->GetEnd());

			if( subCtl )
			{
				if( ProperPB->GetInt(plRTLightBase::kAttenTypeRadio, TimeValue(0)) == 2 )
				{
					// Animation of a cutoff attenuation, which only needs a scalar channel
					plOmniCutoffApplicator *app = TRACKED_NEW plOmniCutoffApplicator();
					app->SetChannelName(GetName());
					plScalarControllerChannel *chan = TRACKED_NEW plScalarControllerChannel(subCtl);
					app->SetChannel(chan);
					anim->AddApplicator(app);
					if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
						anim->ExtendToLength(subCtl->GetLength());
				}
				else
				{
					hsBool distSq = ProperPB->GetInt(plRTLightBase::kAttenTypeRadio, TimeValue(0));

					int i;
					for( i = 0; i < subCtl->GetNumKeys(); i++ )
					{
						hsScalarKey *key = subCtl->GetScalarKey(i);
						if (key)
						{
							hsScalar attenEnd = key->fValue;
							TimeValue tv = key->fFrame * MAX_TICKS_PER_FRAME;
							hsScalar intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
							hsScalar newVal = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / attenEnd;
							if( distSq )
								newVal /= attenEnd;

							key->fValue = newVal;
						}
						hsBezScalarKey *bezKey = subCtl->GetBezScalarKey(i);
						if (bezKey)
						{
							hsScalar attenEnd = bezKey->fValue;
							TimeValue tv = bezKey->fFrame * MAX_TICKS_PER_FRAME;
							hsScalar intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
							hsScalar newVal = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / attenEnd;
							if( distSq )
								newVal /= attenEnd;

							/// From the chain rule, fix our tangents.
							bezKey->fInTan *= -(intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / (attenEnd*attenEnd);
							if( distSq )
								bezKey->fInTan *= 2.f / attenEnd;

							bezKey->fOutTan *= -(intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / (attenEnd*attenEnd);
							if( distSq )
								bezKey->fOutTan *= 2.f / attenEnd;

							bezKey->fValue = newVal;
						}
					}
					plAGApplicator *app;
					if (distSq)
						app = TRACKED_NEW plOmniSqApplicator;
					else
						app = TRACKED_NEW plOmniApplicator;

					app->SetChannelName(GetName());
					plScalarControllerChannel *chan = TRACKED_NEW plScalarControllerChannel(subCtl);
					app->SetChannel(chan);
					anim->AddApplicator(app);
					if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
						anim->ExtendToLength(subCtl->GetLength());

					hsScalar attenConst, attenLinear, attenQuadratic, attenCutoff;
					IGetRTLightAttenValues(ProperPB, attenConst, attenLinear, attenQuadratic, attenCutoff);

					plOmniLightInfo *info = plOmniLightInfo::ConvertNoRef(GetSceneObject()->GetGenericInterface(plOmniLightInfo::Index()));
					if (info)
					{
						hsPoint3 initAtten(attenConst, attenLinear, attenQuadratic);
						info->SetConstantAttenuation(attenConst);
						info->SetLinearAttenuation(attenLinear);
						info->SetQuadraticAttenuation(attenQuadratic);
					}
					else
						hsAssert(false, "Failed to find light info");
				}
			}
		}
	}
}

void plMaxNode::IAdjustRTColorByIntensity(plController* ctl, IParamBlock2* ProperPB)
{
	plLeafController* simp = plLeafController::ConvertNoRef(ctl);
	plCompoundController* comp;
	if( simp )
	{
		int i;
		for( i = 0; i < simp->GetNumKeys(); i++ )
		{
			hsPoint3Key* key = simp->GetPoint3Key(i);
			if (key)
			{
				TimeValue tv = key->fFrame * MAX_TICKS_PER_FRAME;
				hsScalar intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
				key->fValue *= intens;
			}
			hsBezPoint3Key* bezKey = simp->GetBezPoint3Key(i);
			if (bezKey)
			{
				TimeValue tv = bezKey->fFrame * MAX_TICKS_PER_FRAME;
				hsScalar intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
				bezKey->fInTan *= intens;
				bezKey->fOutTan *= intens;
				bezKey->fValue *= intens;
			}
		}
	}
	else if( comp = plCompoundController::ConvertNoRef(ctl) )
	{
		int j;
		for( j = 0; j < 3; j++ )
		{
			IAdjustRTColorByIntensity(comp->GetController(j), ProperPB);
		}
	}
}

void plMaxNode::GetRTLightColAnim(IParamBlock2* ProperPB, plAGAnim *anim)
{
	Control* ambientCtl = nil; // Ambient not currently supported
	Control* colorCtl = ProperPB->GetController(ParamID(plRTLightBase::kLightColor));
	Control* specCtl = ProperPB->GetController(ParamID(plRTLightBase::kSpecularColorSwatch));
	plPointControllerChannel *chan;

	if( ambientCtl )
	{
		plController* ctl;
		if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
			ctl = hsControlConverter::Instance().MakeColorController(ambientCtl, this);
		else
			ctl = hsControlConverter::Instance().MakeColorController(ambientCtl, this, anim->GetStart(), anim->GetEnd());

		if( ctl )
		{
			plLightAmbientApplicator *app = TRACKED_NEW plLightAmbientApplicator();
			app->SetChannelName(GetName());
			chan = TRACKED_NEW plPointControllerChannel(ctl);
			app->SetChannel(chan);
			anim->AddApplicator(app);
			if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
				anim->ExtendToLength(ctl->GetLength());
		}
	}
	if( colorCtl )
	{
		plController* ctl;
		if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
			ctl = hsControlConverter::Instance().MakeColorController(colorCtl, this);
		else
			ctl = hsControlConverter::Instance().MakeColorController(colorCtl, this, anim->GetStart(), anim->GetEnd());
						
		if( ctl )
		{
			IAdjustRTColorByIntensity(ctl, ProperPB);
			plLightDiffuseApplicator *app = TRACKED_NEW plLightDiffuseApplicator();
			app->SetChannelName(GetName());
			chan = TRACKED_NEW plPointControllerChannel(ctl);
			app->SetChannel(chan);
			anim->AddApplicator(app);
			if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
				anim->ExtendToLength(ctl->GetLength());
		}
	}
	if( specCtl )
	{
		plController* ctl;
		if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
			ctl = hsControlConverter::Instance().MakeColorController(specCtl, this);
		else
			ctl = hsControlConverter::Instance().MakeColorController(specCtl, this, anim->GetStart(), anim->GetEnd());
		
		if( ctl )
		{
			plLightSpecularApplicator *app = TRACKED_NEW plLightSpecularApplicator();
			app->SetChannelName(GetName());
			chan = TRACKED_NEW plPointControllerChannel(ctl);
			app->SetChannel(chan);
			anim->AddApplicator(app);
			if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
				anim->ExtendToLength(ctl->GetLength());
		}
	}
}

void plMaxNode::GetRTConeAnim(IParamBlock2* ProperPB, plAGAnim *anim)
{
	Control* innerCtl = ProperPB->GetController(ParamID(plRTLightBase::kHotSpot));
	Control* outerCtl = ProperPB->GetController(ParamID(plRTLightBase::kFallOff));
	plScalarControllerChannel *chan;

	if( innerCtl )
	{
		plLeafController* ctl;
		if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
			ctl = hsControlConverter::Instance().MakeScalarController(innerCtl, this);
		else
			ctl = hsControlConverter::Instance().MakeScalarController(innerCtl, this, anim->GetStart(), anim->GetEnd());
			
		if( ctl )
		{
			plSpotInnerApplicator *app = TRACKED_NEW plSpotInnerApplicator();
			app->SetChannelName(GetName());
			chan = TRACKED_NEW plScalarControllerChannel(ctl);
			app->SetChannel(chan);
			anim->AddApplicator(app);
			if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
				anim->ExtendToLength(ctl->GetLength());
		}
	}
	if( outerCtl )
	{
		plController* ctl;
		if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
			ctl = hsControlConverter::Instance().MakeScalarController(outerCtl, this);
		else
			ctl = hsControlConverter::Instance().MakeScalarController(outerCtl, this, anim->GetStart(), anim->GetEnd());

		if( ctl )
		{
			plSpotOuterApplicator *app = TRACKED_NEW plSpotOuterApplicator();
			app->SetChannelName(GetName());
			chan = TRACKED_NEW plScalarControllerChannel(ctl);
			app->SetChannel(chan);
			anim->AddApplicator(app);
			if (!strcmp(anim->GetName(), ENTIRE_ANIMATION_NAME))
				anim->ExtendToLength(ctl->GetLength());
		}
	}
}

plXImposterComp* plMaxNode::GetXImposterComp()
{
	int count = NumAttachedComponents();
	int i;
	for( i = 0; i < count; i++ )
	{
		// See if any are a x-imposter component.
		plComponentBase *comp = GetAttachedComponent(i);
		if( comp && (comp->ClassID() == XIMPOSTER_COMP_CID) )
		{
			plXImposterComp* ximp = (plXImposterComp*)comp;
			return ximp;
		}
	}
	return nil;
}

Point3 plMaxNode::GetFlexibility()
{
	UInt32 count = NumAttachedComponents();

	// Go through all the components attached to this node
	for (UInt32 i = 0; i < count; i++)
	{
		// See if any are a flexibility component.
		plComponentBase *comp = GetAttachedComponent(i);
		if( comp && (comp->ClassID() == FLEXIBILITY_COMP_CID) )
		{
			plFlexibilityComponent* flex = (plFlexibilityComponent*)comp;
			return flex->GetFlexibility();
		}
	}
	return Point3(0.f, 0.f, 0.f);
}

plLightMapComponent* plMaxNode::GetLightMapComponent()
{
	UInt32 count = NumAttachedComponents();

	// Go through all the components attached to this node
	for (UInt32 i = 0; i < count; i++)
	{
		// See if any are a flexibility component.
		plComponentBase *comp = GetAttachedComponent(i);
		if( comp && (comp->ClassID() == LIGHTMAP_COMP_CID) )
		{
			plLightMapComponent* lmap = (plLightMapComponent*)comp;
			return lmap;
		}
	}
	return nil;
}

plDrawableCriteria plMaxNode::GetDrawableCriteria(hsBool needBlending, hsBool needSorting)
{
	plRenderLevel level = needBlending ? GetRenderLevel(needBlending) : plRenderLevel::OpaqueRenderLevel();

	if( GetSortAsOpaque() )
		level.Set(plRenderLevel::kOpaqueMajorLevel, level.Minor());

	UInt32 crit = 0;
	if( needBlending )
	{
		if( needSorting && !GetNoFaceSort() )
			crit |= plDrawable::kCritSortFaces;
		if( !GetNoSpanSort() )
			crit |= plDrawable::kCritSortSpans;
	}

	if( GetItinerant() )
		crit |= plDrawable::kCritCharacter;

	plDrawableCriteria retVal(crit, level, GetLoadMask());

	if( GetEnviron() )
		retVal.fType |= plDrawable::kEnviron;
	if( GetEnvironOnly() )
		retVal.fType &= ~plDrawable::kNormal;

	return retVal;
}

//// IGetSceneNodeSpans //////////////////////////////////////////////////////
//	Gets the required drawableSpans from a sceneNode. Creates a new one
//	if it can't find one.

plDrawableSpans	*plMaxNode::IGetSceneNodeSpans( plSceneNode *node, hsBool needBlending, hsBool needSorting )
{

	plDrawableSpans	*spans;
	char			tmpName[ 512 ];
	plLocation		nodeLoc = GetLocation();  
	
	if( !needBlending )
		needSorting = false;

	plDrawableCriteria			crit = GetDrawableCriteria(needBlending, needSorting);

	spans = plDrawableSpans::ConvertNoRef( node->GetMatchingDrawable( crit ) );

	if( spans != nil )
	{
		if( GetNoSpanReSort() )
		{
			spans->SetNativeProperty(plDrawable::kPropNoReSort, true);
		}
		return spans;
	}


	/// Couldn't find--create and return it
	spans = TRACKED_NEW plDrawableSpans;
	if( needBlending )
	{
		/// Blending (deferred) spans
		spans->SetCriteria( crit );
		sprintf( tmpName, "%s_%8.8x_%xBlendSpans", node->GetKeyName(), crit.fLevel.fLevel, crit.fCriteria);
	}
	else
	{
		/// Normal spans
		spans->SetCriteria( crit );
		sprintf( tmpName, "%s_%8.8x_%xSpans", node->GetKeyName(), crit.fLevel.fLevel, crit.fCriteria);
	}

	if (GetSwappableGeomTarget() != (UInt32)-1 || GetSwappableGeom()) // We intend to swap geometry with this node... flag the drawable as volatile
	{
		if( GetItinerant() )
			spans->SetNativeProperty(plDrawable::kPropCharacter, true);

		spans->SetNativeProperty( plDrawable::kPropVolatile, true );
	}

	// Add a key for the spans
	plKey key = hsgResMgr::ResMgr()->NewKey( tmpName, spans, nodeLoc, GetLoadMask() );

	spans->SetSceneNode(node->GetKey());

	/// Created! Return it now...
	if( GetNoSpanReSort() )
		spans->SetNativeProperty(plDrawable::kPropNoReSort, true);

	return spans;
}

hsBool plMaxNode::SetupPropertiesPass(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	// TEMP
	if (IsComponent())
		return false;
	// End TEMP

	hsBool ret = true;
	
		UInt32 count = NumAttachedComponents();

	// Go through all the components attached to this node
	for (UInt32 i = 0; i < count; i++)
	{
		// For each one, call the requested function.  If any of the attached components
		// return false this function will return false.
		plComponentBase *comp = GetAttachedComponent(i);

		if (comp->IsExternal())
		{
			if (!((plComponentExt*)comp)->SetupProperties(this, &gComponentTools, pErrMsg))
				ret = false;
		}
		else
		{
			if (!((plComponent*)comp)->SetupProperties(this, pErrMsg))
				ret = false;
		}
	}

	if( ret )
	{
		// Now loop through all the plPassMtlBase-derived materials that are applied to this node
		Mtl *mtl = GetMtl();
		if( mtl != nil && !GetParticleRelated() )
		{
			if( hsMaterialConverter::IsMultiMat( mtl ) || hsMaterialConverter::IsMultipassMat( mtl ) || hsMaterialConverter::IsCompositeMat( mtl ) )
			{
				int i;
				for (i = 0; i < mtl->NumSubMtls(); i++)
				{
					plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl->GetSubMtl( i ) );
					if( pass != nil )
					{
						if( !pass->SetupProperties( this, pErrMsg ) )
							ret = false;
					}
				}
			}
			else
			{
				plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl );
				if( pass != nil )
				{
					if( !pass->SetupProperties( this, pErrMsg ) )
						ret = false;
				}
			}
		}
	}
	if( ret )
	{
		plMaxNode* parent = (plMaxNode*)GetParentNode();
		if( parent && IsLegalDecal(false) )
		{
			AddRenderDependency(parent);
			SetNoSpanSort(true);
			SetNoFaceSort(true);
			SetNoDeferDraw(true);
		}
	}

	return ret;
}

hsBool plMaxNode::FirstComponentPass(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	// TEMP
	if (IsComponent())
		return false;
	// End TEMP

	hsBool ret = true;
	
	if (!CanConvert())
		return ret;
	UInt32 count = NumAttachedComponents();

	// Go through all the components attached to this node
	for (UInt32 i = 0; i < count; i++)
	{
		// For each one, call the requested function.  If any of the attached components
		// return false this function will return false.
		plComponentBase *comp = GetAttachedComponent(i);

		if (comp->IsExternal())
		{
			if (!((plComponentExt*)comp)->PreConvert(this, &gComponentTools, pErrMsg))
				ret = false;
		}
		else
		{
			if (!((plComponent*)comp)->PreConvert(this, pErrMsg))
				ret = false;
		}
	}

	return ret;
}

hsBool plMaxNode::ConvertComponents(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	// TEMP
	if (IsComponent())
		return false;
	// End TEMP

	hsBool ret = true;

	char *dbgNodeName = GetName();
	if (!CanConvert())
		return ret;

	UInt32 count = NumAttachedComponents();

	// Go through all the components attached to this node
	for (UInt32 i = 0; i < count; i++)
	{
		// For each one, call the requested function.  If any of the attached components
		// return false this function will return false.
		plComponentBase *comp = GetAttachedComponent(i);

		if (comp->IsExternal())
		{
			if (!((plComponentExt*)comp)->Convert(this, &gComponentTools, pErrMsg))
				ret = false;
		}
		else
		{
			if (!((plComponent*)comp)->Convert(this, pErrMsg))
				ret = false;
		}
	}

	return ret;
}

hsBool plMaxNode::DeInitComponents(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	// TEMP
	if (IsComponent())
		return false;
	// End TEMP

	hsBool ret = true;

	char *dbgNodeName = GetName();
	if (!CanConvert())
		return ret;

	UInt32 count = NumAttachedComponents();

	// Go through all the components attached to this node
	for (UInt32 i = 0; i < count; i++)
	{
		// For each one, call the requested function.  If any of the attached components
		// return false this function will return false.
		plComponentBase *comp = GetAttachedComponent(i);

		if (comp->IsExternal())
		{
			if (!((plComponentExt*)comp)->DeInit(this, &gComponentTools, pErrMsg))
				ret = false;
		}
		else
		{
			if (!((plComponent*)comp)->DeInit(this, pErrMsg))
				ret = false;
		}
	}

	if( ret )
	{
		// Now loop through all the plPassMtlBase-derived materials that are applied to this node
		// So we can call ConvertDeInit() on them
		Mtl *mtl = GetMtl();
		if( mtl != nil && !GetParticleRelated() )
		{
			if( hsMaterialConverter::IsMultiMat( mtl ) || hsMaterialConverter::IsMultipassMat( mtl ) || hsMaterialConverter::IsCompositeMat( mtl ) )
			{
				int i;
				for (i = 0; i < mtl->NumSubMtls(); i++)
				{
					plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl->GetSubMtl( i ) );
					if( pass != nil )
					{
						if( !pass->ConvertDeInit( this, pErrMsg ) )
							ret = false;
					}
				}
			}
			else
			{
				plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl );
				if( pass != nil )
				{
					if( !pass->ConvertDeInit( this, pErrMsg ) )
						ret = false;
				}
			}
		}
	}

	return ret;
}

hsBool plMaxNode::ClearData(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaAgeChunk);
	RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaDistChunk);
	RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaRoomChunk);

	RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaMaxNodeDataChunk);
//	RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaSceneViewerChunk);
	RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaLightChunk);

	return true;
}

// HASAGMOD
// Little special-purpose thing to see if a node has an animation graph modifier on it.
plAGModifier *plMaxNode::HasAGMod()
{
	char *name = GetName();
	if (CanConvert())
	{
		plSceneObject *SO = GetSceneObject();
		int numMods = SO->GetNumModifiers();

		for (int i = 0; i < numMods; i++)
		{
			const plModifier *mod = SO->GetModifier(i);

			if(plAGModifier::ConvertNoRef(mod)) {
				return (plAGModifier *)mod;
			}
		}
	}
	return nil;
}

plAGMasterMod *plMaxNode::GetAGMasterMod()
{
	char *name = GetName();
	if (CanConvert())
	{
		plSceneObject *SO = GetSceneObject();
		int numMods = SO->GetNumModifiers();

		for (int i = 0; i < numMods; i++)
		{
			const plModifier *mod = SO->GetModifier(i);

			if(plAGMasterMod::ConvertNoRef(mod)) {
				return (plAGMasterMod *)mod;
			}
		}
	}
	return nil;
}


// SETUPBONESALIASESRECUR
void plMaxNode::SetupBonesAliasesRecur(const char *rootName)
{
	if(CanConvert()) {
		if (!HasAGMod()) {
			const char *nameToUse;
			
			// parse UserPropsBuf for entire BoneName line
			char localName[256];
			TSTR propsBuf;
			GetUserPropBuffer(propsBuf);
			char* start=strstr(propsBuf, "BoneName=");
			if (!start)
				start=strstr(propsBuf, "bonename=");
			const int len = hsStrlen("BoneName=");
			if(start && UserPropExists("BoneName"))
			{
				start+=len;
				int i=0;
				while(*start != '\n' && *start != '\0' && *start)
				{
					hsAssert(i<256, "localName overflow");
					localName[i++]=*start++;
				}
				localName[i]=0;

				nameToUse = localName;

			}
			else
			{
				const char *nodeName = GetName();
		//		char str[256];
		//		sprintf(str, "Missing 'BoneName=foo' UserProp, on object %s, using node name", nodeName ? nodeName : "?");
		//		hsAssert(false, str);

				nameToUse = nodeName;
			}

		/*	char aliasName[256];
			sprintf(aliasName, "%s_%s", rootName, nameToUse);

			plUoid* uoid = hsgResMgr::ResMgr()->FindAlias(aliasName, plSceneObject::Index());
			if( !uoid )
			{
				plAliasModifier* pAlMod = TRACKED_NEW plAliasModifier;
				pAlMod->SetAlias(aliasName);
				AddModifier(pAlMod);
			}
		*/
			plAGModifier *mod = TRACKED_NEW plAGModifier(nameToUse);
			AddModifier(mod, GetName());
		}
	}

	int j = 0;
	for( j = 0; j < NumberOfChildren(); j++ )
		((plMaxNode*)GetChildNode(j))->SetupBonesAliasesRecur(rootName);
}

void plMaxNode::SetDISceneNodeSpans( plDrawInterface *di, hsBool needBlending )
{
	// This poorly named function is currently only used by ParticleComponent.
	// di->GetNumDrawables() will always be zero. In general, particles only
	// need a blending drawable, which can always be index zero since it's the only
	// one.
	di->SetDrawable( di->GetNumDrawables(),
						IGetSceneNodeSpans(plSceneNode::ConvertNoRef( GetRoomKey()->GetObjectPtr() ), 
									   needBlending));
}

plMaxNode* plMaxNode::GetBonesRoot()
{
	ISkin* skin = FindSkinModifier();
	if( !skin )
		return nil;

	INode* bone = skin->GetBone(0);

	if( !bone )
		return nil;

	while( !bone->GetParentNode()->IsRootNode() )
		bone = bone->GetParentNode();

	plMaxNode* boneRoot = (plMaxNode*)bone;

	if( !(boneRoot && boneRoot->CanConvert()) )
		return nil;

	return boneRoot;
}

void plMaxNode::GetBonesRootsRecur(hsTArray<plMaxNode*>& nodes)
{
	plMaxNode* bRoot = GetBonesRoot();
	if( bRoot )
	{
		int idx = nodes.Find(bRoot);
		if( idx == nodes.kMissingIndex )
			nodes.Append(bRoot);
	}

	int i;
	for( i = 0; i < NumberOfChildren(); i++ )
		((plMaxNode*)GetChildNode(i))->GetBonesRootsRecur(nodes);
}

plSceneObject* plMaxNode::MakeCharacterHierarchy(plErrorMsg *pErrMsg)
{
	plSceneObject* playerRoot = GetSceneObject();
	if( pErrMsg->Set(playerRoot->GetDrawInterface() != nil, GetName(), "Non-helper as player root").CheckAndAsk() )
		return nil;
	const char *playerRootName = GetName();

	hsTArray<plMaxNode*> bonesRoots;
	int i;
	for( i = 0; i < NumberOfChildren(); i++ )
		((plMaxNode*)GetChildNode(i))->GetBonesRootsRecur(bonesRoots);

	if( pErrMsg->Set(bonesRoots.GetCount() > 1, playerRootName, "Found multiple bones hierarchies").CheckAndAsk() )
		return nil;

	if( bonesRoots.GetCount() )
	{
		bonesRoots[0]->SetupBonesAliasesRecur(playerRootName);

		plSceneObject* boneRootObj = bonesRoots[0]->GetSceneObject();

		if( pErrMsg->Set(boneRootObj == nil, playerRootName, "No scene object for the bones root").CheckAndAsk() )
			return nil;

		if( boneRootObj != playerRoot )
			hsMessageBox("This avatar's bone hierarchy does not have the avatar root node linked as a parent. "
						 "This may cause the avatar draw incorrectly.", playerRootName, hsMessageBoxNormal);
	}

	return playerRoot;
}

// Takes all bones found on this node (and any descendents) and sets up a single palette
void plMaxNode::SetupBoneHierarchyPalette(plMaxBoneMap *bones /* = nil */)
{
	const char* dbgNodeName = GetName();

	if( !CanConvert() )
		return;

	if (GetBoneMap())
		return;
	
	if (bones == nil)
	{
		bones = TRACKED_NEW plMaxBoneMap();
		bones->fOwner = this;
	}
	
	if (UserPropExists("Bone"))
		bones->AddBone(this);

	int i;
	for (i = 0; i < NumBones(); i++)
		bones->AddBone(GetBone(i));

	SetBoneMap(bones);

	for (i = 0; i < NumberOfChildren(); i++)
		((plMaxNode*)GetChildNode(i))->SetupBoneHierarchyPalette(bones);

	// If we were the one to start this whole thing, then sort all the bones now that
	// they've been added.
	if (bones->fOwner == this)
		bones->SortBones();
}

hsBool plMaxNode::IsLegalDecal(hsBool checkParent /* = true */)
{
	Mtl *mtl = GetMtl();
	if (mtl == nil || GetParticleRelated())
		return false;
	if (hsMaterialConverter::IsMultiMat(mtl))
	{
		int i;
		for (i = 0; i < mtl->NumSubMtls(); i++)
		{
			if (!hsMaterialConverter::IsDecalMat(mtl->GetSubMtl(i)))
				return false;
		}
	}
	else if (!hsMaterialConverter::IsDecalMat(mtl))
		return false;

	return true;
}

int plMaxNode::NumUVWChannels()
{
	hsBool deleteIt;

	TriObject* triObj = GetTriObject(deleteIt);
	if( triObj )
	{
		Mesh* mesh = &(triObj->mesh);

		// note: There's been a bit of a change with UV accounting. There are two variables, numChannels
		// and numBlendChannels. The first represents texture UV channels MAX gives us, the latter is
		// the number of extra blending channels the current material uses to handle its effects.

		// numMaps includes map #0, which is the vertex colors. So subtract 1 to get the # of uv maps...
		int numChannels = mesh->getNumMaps() - 1;
		if( numChannels > plGeometrySpan::kMaxNumUVChannels )
			numChannels = plGeometrySpan::kMaxNumUVChannels;

		/// Check the mapping channels. See, MAX likes to tell us we have mapping channels
		/// but the actual channel pointer is nil. When MAX tries to render the scene, it says that the
		/// object in question has no UV channel. So apparently we have to check numChannels *and* 
		/// that each mapChannel is non-nil...
		int i;
		for( i = 0; i < numChannels; i++ )
		{
			// i + 1 is exactly what IGenerateUVs uses, so I'm not questioning it...
			if( mesh->mapFaces( i + 1 ) == nil )
			{
				numChannels = i;
				break;
			}
		}
		int numUsed = hsMaterialConverter::MaxUsedUVWSrc(this, GetMtl());
		plLightMapComponent* lmc = GetLightMapComponent();
		if( lmc )
		{
			if( (lmc->GetUVWSrc() < numChannels) && (lmc->GetUVWSrc() >= numUsed) )
				numUsed = lmc->GetUVWSrc() + 1;
		}
		if( numChannels > numUsed )
			numChannels = numUsed;

		if( GetWaterDecEnv() )
			numChannels = 3;

		if( deleteIt )
			triObj->DeleteThis();

		return numChannels;
	}

	return 0;
}

//// IGet/SetCachedAlphaHackValue ////////////////////////////////////////////
//	Pair of functions to handle accessing the TArray cache on plMaxNodeData.
//	See AlphaHackLayersNeeded() for details.

int	plMaxNode::IGetCachedAlphaHackValue( int iSubMtl )
{
	plMaxNodeData *pDat = GetMaxNodeData();
	if( pDat == nil )
		return -1;

	hsTArray<int>	*cache = pDat->GetAlphaHackLayersCache();
	if( cache == nil )
		return -1;

	iSubMtl++;
	if( iSubMtl >= cache->GetCount() )
		return -1;

	return (*cache)[ iSubMtl ];
}

void	plMaxNode::ISetCachedAlphaHackValue( int iSubMtl, int value )
{
	plMaxNodeData *pDat = GetMaxNodeData();
	if( pDat == nil )
		return;

	hsTArray<int>	*cache = pDat->GetAlphaHackLayersCache();
	if( cache == nil )
	{
		cache = TRACKED_NEW hsTArray<int>;
		pDat->SetAlphaHackLayersCache( cache );
	}

	iSubMtl++;

	if( iSubMtl >= cache->GetCount() )
	{
		int i = cache->GetCount();
		cache->ExpandAndZero( iSubMtl + 1 );
		for( ; i < cache->GetCount(); i++ )
			(*cache)[ i ] = -1;
	}

	(*cache)[ iSubMtl ] = value;
}

//// AlphaHackLayersNeeded ///////////////////////////////////////////////////
//	Updated 8.13.02 mcn - Turns out this function is actually very slow, and
//	it also happens to be used a lot in testing instanced objects and whether
//	they really can be instanced or not. Since the return value of this
//	function will be constant after the SetupProperties() pass (and undefined
//	before), we cache the value now after the first time we calculate it.
//	Note: mf said that putting long comments in are good so long as most of
//	them aren't obscenities, so I'm trying to keep the #*$&(*#$ obscenities
//	to a minimum here.

int plMaxNode::AlphaHackLayersNeeded(int iSubMtl)
{
	const char* dbgNodeName = GetName();

	int cached = IGetCachedAlphaHackValue( iSubMtl );
	if( cached != -1 )
		return cached;

	int numVtxOpacChanAvail = VtxAlphaNotAvailable() ? 0 : 1;

	int numVtxOpacChanNeeded = hsMaterialConverter::NumVertexOpacityChannelsRequired(this, iSubMtl);

	cached = numVtxOpacChanNeeded - numVtxOpacChanAvail;
	ISetCachedAlphaHackValue( iSubMtl, cached );
	return cached;
}

// Will our lighting pay attention to vertex alpha values?
hsBool plMaxNode::VtxAlphaNotAvailable()
{
	if( NonVtxPreshaded() || GetParticleRelated())
		return false;

	return true;
}

hsBool plMaxNode::NonVtxPreshaded()
{
	if( GetForceMatShade() )
		return false;

	if( GetAvatarSO() != nil ||
		hsMaterialConverter::Instance().HasMaterialDiffuseOrOpacityAnimation(this) )
		return false;

	if( GetRunTimeLight() && !hsMaterialConverter::Instance().HasEmissiveLayer(this) )
		return true;

	return( GetLightMapComponent() != nil );
}

TriObject* plMaxNode::GetTriObject(hsBool& deleteIt)
{
	// Get da object
	Object *obj = EvalWorldState(TimeValue(0)).obj;
	if( obj == nil )
		return nil;

	if( !obj->CanConvertToType(triObjectClassID) )
		return nil;

	// Convert to triMesh object
	TriObject	*meshObj = (TriObject *)obj->ConvertToType(TimeValue(0), triObjectClassID);
	if( meshObj == nil )
		return nil;

	deleteIt = meshObj != obj;

	return meshObj;
}

//// GetNextSoundIdx /////////////////////////////////////////////////////////
//	Starting at 0, returns an incrementing index for each maxNode. Useful for 
//	assigning indices to sound objects attached to the node.

UInt32	plMaxNode::GetNextSoundIdx( void )
{
	UInt32	idx = GetSoundIdxCounter();
	SetSoundIdxCounter( idx + 1 );
	return idx;
}

//// IsPhysical //////////////////////////////////////////////////////////////
//	Fun temp hack function to tell if a maxNode is physical. Useful after
//	preConvert (checks for a physical on the simInterface)

hsBool	plMaxNode::IsPhysical( void )
{
	if( GetSceneObject() && GetSceneObject()->GetSimulationInterface() && 
		GetSceneObject()->GetSimulationInterface()->GetPhysical() )
		return true;

	return false;
}


plPhysicalProps *plMaxNode::GetPhysicalProps()
{
	plMaxNodeData *pDat = GetMaxNodeData();
	if (pDat)
		return pDat->GetPhysicalProps();

	return nil;
}

//// FindPageKey /////////////////////////////////////////////////////////////
//	Little helper function. Calls FindKey() in the resManager using the location (page) of this node

plKey	plMaxNode::FindPageKey( UInt16 classIdx, const char *name )
{
	return hsgResMgr::ResMgr()->FindKey( plUoid( GetLocation(), classIdx, name ) );
}

char *plMaxNode::GetAgeName()
{
	int i;
	for (i = 0; i < NumAttachedComponents(); i++)
	{
		plComponentBase *comp = GetAttachedComponent(i);
		if (comp->ClassID() == PAGEINFO_CID)
			return ((plPageInfoComponent*)comp)->GetAgeName();
	}
	return nil;
}

// create a list of keys used by the run-time interface for things like
// determining cursor changes, what kind of object this is, etc.
// we're doing this here because multiple logic triggers can be attached to a 
// single object and tracking down all their run-time counterpart objects (who might
// need a message sent to them) is a huge pain and very ugly.  This will capture anything
// important in a single list.

hsBool plMaxNode::MakeIfaceReferences(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
	hsBool ret = true;

	char *dbgNodeName = GetName();
	if (!CanConvert())
		return ret;
	
	UInt32 count = GetSceneObject()->GetNumModifiers();
	hsTArray<plKey> keys;
	// Go through all the modifiers attached to this node's scene object
	// and grab keys for objects who we would need to send interface messages to
	for (UInt32 i = 0; i < count; i++)
	{
		const plModifier* pMod = GetSceneObject()->GetModifier(i);
		// right now all we care about are these, but I guarentee you we will
		// care about more as the interface gets more complex
		const plPickingDetector* pDet = plPickingDetector::ConvertNoRef(pMod);
		const plLogicModifier* pLog = plLogicModifier::ConvertNoRef(pMod);
		if( pDet )
		{
			for (int j = 0; j < pDet->GetNumReceivers(); j++)
				keys.Append(pDet->GetReceiver(j));
		}
		else
		if( pLog )
		{
			keys.Append(pLog->GetKey());
		}
	}
	// if there is anything there, create an 'interface object modifier' which simply stores 
	// the list in a handy form
	if (keys.Count())
	{
		plInterfaceInfoModifier* pMod = TRACKED_NEW plInterfaceInfoModifier;
		
		plKey modifierKey = hsgResMgr::ResMgr()->NewKey(GetName(), pMod, GetLocation(), GetLoadMask());
		hsgResMgr::ResMgr()->AddViaNotify(modifierKey, TRACKED_NEW plObjRefMsg(GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
		
		for(int i = 0; i < keys.Count(); i++)
			pMod->AddRefdKey(keys[i]);
	}

	return ret;
}
