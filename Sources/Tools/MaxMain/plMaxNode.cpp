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
#include "MaxAPI.h"

#include "plgDispatch.h"
#include "hsFastMath.h"
#include "pnKeyedObject/plKey.h"
#include "plRenderLevel.h"
#include "hsSTLStream.h"
#include "hsStringTokenizer.h"

#include "plMaxNode.h"
#include "plMaxNodeData.h"
#include "MaxComponent/plComponent.h"

#include "GlobalUtility.h"
#include "plPluginResManager.h"

#include "MaxConvert/plConvert.h"
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/hsControlConverter.h"
#include "MaxConvert/plMeshConverter.h"
#include "MaxConvert/hsMaterialConverter.h"
#include "MaxConvert/plLayerConverter.h"
#include "MaxConvert/UserPropMgr.h"
#include "MaxExport/plErrorMsg.h"
#include "MaxConvert/hsVertexShader.h"
#include "MaxConvert/plLightMapGen.h"
#include "plMaxMeshExtractor.h"
#include "MaxPlasmaMtls/Layers/plLayerTex.h"

#include "pnSceneObject/plSceneObject.h"
#include "plScene/plSceneNode.h"
#include "plPhysX/plPXCooking.h"
#include "plPhysX/plPXPhysical.h"
#include "plDrawable/plInstanceDrawInterface.h"
#include "plDrawable/plSharedMesh.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plAudioInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pfAnimation/plFilterCoordInterface.h"
#include "plParticleSystem/plBoundInterface.h"
#include "plPhysical/plPickingDetector.h"
#include "plModifier/plLogicModifier.h"
#include "plModifier/plResponderModifier.h"
#include "plModifier/plInterfaceInfoModifier.h"
#include "pfAnimation/plLightModifier.h"
#include "plAnimation/plAGModifier.h"
#include "plAnimation/plAGAnim.h"
#include "plAnimation/plPointChannel.h"
#include "plAnimation/plScalarChannel.h"
#include "plAnimation/plAGMasterMod.h"
#include "plMessage/plReplaceGeometryMsg.h"
#include "plGImage/plMipmap.h"
#include "plModifier/plSpawnModifier.h"
#include "plInterp/plController.h"
#include "plInterp/hsInterp.h"
#include "pnMessage/plTimeMsg.h"
#include "pfAnimation/plViewFaceModifier.h" // mf horse temp hack testing to be thrown away

#include "plScene/plCullPoly.h"
#include "plScene/plOccluder.h"

#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGeometrySpan.h"
#include "plPipeline/plFogEnvironment.h"

#include "plGLight/plLightInfo.h"
#include "plGLight/plLightKonstants.h"
#include "plSurface/plLayerInterface.h"
#include "plSurface/plLayer.h"
#include "plSurface/hsGMaterial.h"

#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plIntRefMsg.h"

#include "MaxExport/plExportProgressBar.h"

#include "MaxPlasmaMtls/Materials/plDecalMtl.h"

#include "MaxComponent/plAnimComponent.h"
#include "MaxComponent/plComponentTools.h"
#include "MaxComponent/plComponentExt.h"
#include "MaxComponent/plFlexibilityComponent.h"
#include "MaxComponent/plLightMapComponent.h"
#include "MaxComponent/plXImposter.h"
#include "MaxComponent/plMiscComponents.h"

#include "plParticleSystem/plParticleSystem.h"
#include "plParticleSystem/plParticleEmitter.h"
#include "plParticleSystem/plParticleEffect.h"
#include "plParticleSystem/plParticleGenerator.h"
#include "plParticleSystem/plConvexVolume.h"

#include "MaxPlasmaLights/plRealTimeLightBase.h"
#include "MaxPlasmaLights/plRTProjDirLight.h"

#include "plGetLocationDlg.h"

#include "plResMgr/plKeyFinder.h"
#include "plMaxCFGFile.h"
#include "plAgeDescription/plAgeDescription.h"
#include "plResMgr/plPageInfo.h"
#include "pnNetCommon/plSDLTypes.h"

#include "plMaxMeshExtractor.h"
#include "plPhysX/plSimulationMgr.h"

extern UserPropMgr gUserPropMgr;

bool ThreePlaneIntersect(const hsVector3& norm0, const hsPoint3& point0, 
                         const hsVector3& norm1, const hsPoint3& point1, 
                         const hsVector3& norm2, const hsPoint3& point2, hsPoint3& loc);

// Begin external component toolbox ///////////////////////////////////////////////////////////////
static plKey ExternAddModifier(plMaxNodeBase *node, plModifier *mod)
{
    return nullptr;//((plMaxNode*)node)->AddModifier(mod);
}

static plKey ExternGetNewKey(const ST::string &name, plModifier *mod, plLocation loc)
{
    return nullptr;//hsgResMgr::ResMgr()->NewKey(name, mod, loc);
}

// In plResponderComponent (for no apparent reason).
int GetMatAnimModKey(Mtl* mtl, plMaxNodeBase* node, const ST::string &segName, std::vector<plKey>& keys);
// In plAudioComponents
int GetSoundNameAndIdx(plComponentBase *comp, plMaxNodeBase *node, const char*& name);

static ST::string GetAnimCompAnimName(plComponentBase *comp)
{
    if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
        return ((plAnimComponentBase*)comp)->GetAnimName();
    return ST::string();
}

static plKey GetAnimCompModKey(plComponentBase *comp, plMaxNodeBase *node)
{
    if (comp->ClassID() == ANIM_COMP_CID || comp->ClassID() == ANIM_GROUP_COMP_CID)
        return ((plAnimComponentBase*)comp)->GetModKey((plMaxNode*)node);
    return nullptr;
}

plComponentTools gComponentTools(ExternAddModifier, 
                                ExternGetNewKey, 
                                nullptr,
                                GetAnimCompModKey,
                                GetAnimCompAnimName,
                                GetMatAnimModKey,
                                GetSoundNameAndIdx);

// End external component toolbox //////////////////////////////////////////////////////////////////

void plMaxBoneMap::AddBone(plMaxNodeBase *bone)
{
    auto dbgNodeName = bone->GetName();
    if (fBones.find(bone) == fBones.end())
        fBones[bone] = fNumBones++;
}

void plMaxBoneMap::FillBoneArray(plMaxNodeBase **boneArray)
{
    BoneMap::const_iterator boneIt = fBones.begin();
    for (; boneIt != fBones.end(); boneIt++)
        boneArray[(*boneIt).second] = (*boneIt).first;
}

uint8_t plMaxBoneMap::GetIndex(plMaxNodeBase *bone)
{
    hsAssert(fBones.find(bone) != fBones.end(), "Bone missing in remap!");
    return fBones[bone];
}

uint32_t plMaxBoneMap::GetBaseMatrixIndex(plDrawable *draw)
{
    if (fBaseMatrices.find(draw) == fBaseMatrices.end())
        return (uint32_t)-1;

    return fBaseMatrices[draw];
}

void plMaxBoneMap::SetBaseMatrixIndex(plDrawable *draw, uint32_t idx)
{
    fBaseMatrices[draw] = idx;
}

// Don't call this after you've started assigning indices to spans, or
// you'll be hosed (duh).
void plMaxBoneMap::SortBones()
{
    plMaxNodeBase **tempBones = new plMaxNodeBase*[fNumBones];  
    FillBoneArray(tempBones);

    // Look ma! An n^2 bubble sort!
    // (It's a 1-time thing for an array of less than 100 items. Speed is not essential here)
    int i,j;
    for (i = 0; i < fNumBones; i++)
    {
        bool swap = false;        
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

plKey plMaxNode::AddModifier(plModifier *pMod, const ST::string& name)
{
    plKey modKey = pMod->GetKey();
    if (!modKey)
        modKey = hsgResMgr::ResMgr()->NewKey(name, pMod, GetLocation());
    hsgResMgr::ResMgr()->AddViaNotify(modKey, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
    return modKey;
}

bool plMaxNode::DoRecur(PMaxNodeFunc pDoFunction,plErrorMsg *pErrMsg, plConvertSettings *settings,plExportProgressBar*bar)
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
bool plMaxNode::DoAllRecur(PMaxNodeFunc pDoFunction,plErrorMsg *pErrMsg, plConvertSettings *settings,plExportProgressBar*bar)
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

bool plMaxNode::ConvertValidate(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    TimeValue   t = hsConverterUtils::Instance().GetTime(GetInterface());
    Object *obj = EvalWorldState( t ).obj;
    const char* dbgName = GetName();

    // Always want to recalculate if this object can convert at this point.
    // In general there won't be any cached flag anyway, but in the SceneViewer
    // there can be if we're reconverting.
    bool canConvert = CanConvert(true);

    plMaxNodeData thisNodeData;     // Extra data stored for each node

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
//      thisNodeData.SetDrawable(false);
    }
    if( UserPropExists("PSRunTimeLight") )
        thisNodeData.SetRunTimeLight(true);

    if (GetParticleRelated())
        thisNodeData.SetForceLocal(true);
//  if (UserPropExists("cloth"))
//  {
//      thisNodeData.SetForceLocal(true);
//  }

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
        std::vector<plMaxNode *> nodes;
        uint32_t numInstances = IBuildInstanceList( GetObjectRef(), t, nodes );
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

bool plMaxNode::ClearMaxNodeData(plErrorMsg *pErrMsg, plConvertSettings *settings)
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

    SetMaxNodeData(nullptr);
    return true;
}



//
// Helper for setting synchedObject options, until we have a GUI
//

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
            so->SetLocalOnly(true); // disable net synching and persistence
        else
        if (gUserPropMgr.UserPropExists(this, "DontPersistAny"))    // disable all types of persistence
            so->SetSynchFlagsBit(plSynchedObject::kExcludeAllPersistentState);
        else
        {
            if (gUserPropMgr.GetUserPropStringList(this, "DontPersist", num, sdataList))
            {
                for(i=0;i<num;i++)
                    so->AddToSDLExcludeList((const char *)sdataList[i]);  // disable a type of persistence
            }
        }

        //
        // Check for Volatile prop
        //
        if (gUserPropMgr.UserPropExists(this, "VolatileAll"))   // make all sdl types on this object Volatile
            so->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
        else
        {
            if (gUserPropMgr.GetUserPropStringList(this, "Volatile", num, sdataList))
            {
                for(i=0;i<num;i++)
                    so->AddToSDLVolatileList((const char *)sdataList[i]); // make volatile a type of persistence
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
            for (uint32_t x = 0; x < count; x++)
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

bool plMaxNode::MakeSceneObject(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    const char* dbgName = GetName();
    if (!CanConvert()) 
        return false;

    plLocation nodeLoc = GetLocation();//GetLocFromStrings();   // After this we can use GetLocation()
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
    pso = new plSceneObject;
    objKey = hsgResMgr::ResMgr()->NewKey(ST::string::from_utf8(GetName()), pso, nodeLoc, GetLoadMask());

    // Remember info in MaxNodeData block for later
    plMaxNodeData *pDat = GetMaxNodeData();
    pDat->SetKey(objKey);
    pDat->SetSceneObject(pso);

    CheckSynchOptions(pso);
    
    return true;
}

bool plMaxNode::PrepareSkin(plErrorMsg *pErrMsg, plConvertSettings *settings)
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

bool plMaxNode::IFindBones(plErrorMsg *pErrMsg, plConvertSettings *settings)
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
        auto dbgNodeName = GetName();

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

bool plMaxNode::MakePhysical(plErrorMsg *pErrMsg, plConvertSettings *settings)
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

    plPXPhysical* physical = new plPXPhysical();
    PhysRecipe& recipe = physical->GetRecipe();
    recipe.mass = mass;
    recipe.friction = physProps->GetFriction();
    recipe.restitution = physProps->GetRestitution();
    recipe.bounds = (plSimDefs::Bounds)physProps->GetBoundsType();
    recipe.group = group;
    recipe.reportsOn = physProps->GetReportGroup();
    recipe.objectKey = GetKey();
    recipe.sceneNode = roomKey;
    recipe.worldKey = subworld ? subworld->GetKey() : nullptr;

    plMaxMeshExtractor::NeutralMesh mesh;
    plMaxMeshExtractor::Extract(mesh, proxyNode, bounds == plSimDefs::kBoxBounds, this);

    hsMatrix44 l2s = subworld ? (subworld->GetWorldToLocal44() * mesh.fL2W) : mesh.fL2W;
    l2s.DecompRigid(recipe.l2sP, recipe.l2sQ);

    switch (bounds)
    {
    case plSimDefs::kBoxBounds:
        {
            hsPoint3 minV(FLT_MAX, FLT_MAX, FLT_MAX), maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
            for (int i = 0; i < mesh.fNumVerts; i++)
            {
                minV.fX = std::min(mesh.fVerts[i].fX, minV.fX);
                minV.fY = std::min(mesh.fVerts[i].fY, minV.fY);
                minV.fZ = std::min(mesh.fVerts[i].fZ, minV.fZ);
                maxV.fX = std::max(mesh.fVerts[i].fX, maxV.fX);
                maxV.fY = std::max(mesh.fVerts[i].fY, maxV.fY);
                maxV.fZ = std::max(mesh.fVerts[i].fZ, maxV.fZ);
            }
            hsPoint3 width = maxV - minV;
            recipe.bDimensions = width / 2;
            recipe.bOffset = minV + (width / 2.f);
        }
        break;
    case plSimDefs::kProxyBounds:
    case plSimDefs::kExplicitBounds:
        {
            recipe.meshStream = std::make_unique<hsVectorStream>();
            plPXCooking::WriteTriMesh(recipe.meshStream.get(), mesh.fNumFaces, mesh.fFaces,
                                      mesh.fNumVerts, mesh.fVerts);
            recipe.meshStream->Rewind();

            // Attempt to cook the physical
            if (!(recipe.triMesh = physical->ICookTriMesh(recipe.meshStream.get()))) {
                pErrMsg->Set("Physics Error", "Failed to cook triangle mesh %s", GetName()).Show();
                return false;
            }
            recipe.meshStream->Rewind();
        }
        break;
    case plSimDefs::kSphereBounds:
        {
            hsPoint3 minV(FLT_MAX, FLT_MAX, FLT_MAX), maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);
            for (int i = 0; i < mesh.fNumVerts; i++)
            {
                minV.fX = std::min(mesh.fVerts[i].fX, minV.fX);
                minV.fY = std::min(mesh.fVerts[i].fY, minV.fY);
                minV.fZ = std::min(mesh.fVerts[i].fZ, minV.fZ);
                maxV.fX = std::max(mesh.fVerts[i].fX, maxV.fX);
                maxV.fY = std::max(mesh.fVerts[i].fY, maxV.fY);
                maxV.fZ = std::max(mesh.fVerts[i].fZ, maxV.fZ);
            }
            hsPoint3 width = maxV - minV;
            recipe.radius = std::max({ width.fX, width.fY, width.fZ });
            recipe.radius /= 2.f;
            recipe.offset = minV + (width / 2.f);
        }
        break;
    case plSimDefs::kHullBounds:
        {
            recipe.meshStream = std::make_unique<hsVectorStream>();
            plPXCooking::WriteConvexHull(recipe.meshStream.get(), mesh.fNumVerts, mesh.fVerts);
            recipe.meshStream->Rewind();

            // Attempt to cook the physical
            if (!(recipe.convexMesh = physical->ICookHull(recipe.meshStream.get()))) {
                pErrMsg->Set("Physics Error", "Failed to cook convex hull %s", GetName()).Show();
                return false;
            }
            recipe.meshStream->Rewind();
        }
        break;
    }

    delete [] mesh.fFaces;
    delete [] mesh.fVerts;

    // add the object to the resource manager, keyed to the new name
    plLocation nodeLoc = GetKey()->GetUoid().GetLocation();
    ST::string objName = GetKey()->GetName();
    plKey physKey = hsgResMgr::ResMgr()->NewKey(objName, physical, nodeLoc, GetLoadMask());

    // Sanity check creating the physical actor
    if (!physical->InitActor())
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
    
    plSimulationInterface* si = new plSimulationInterface;
    plKey pSiKey = hsgResMgr::ResMgr()->NewKey(objName, si, nodeLoc, GetLoadMask());

    // link the simulation interface to the scene object
    hsgResMgr::ResMgr()->AddViaNotify(pSiKey, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    // add the physical to the simulation interface
    hsgResMgr::ResMgr()->AddViaNotify(physKey , new plIntRefMsg(pSiKey, plRefMsg::kOnCreate, 0, plIntRefMsg::kPhysical), plRefFlags::kActiveRef);

    return true;
}

bool plMaxNode::MakeController(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    if (!CanConvert()) 
        return false;

    bool forceLocal = hsControlConverter::Instance().ForceLocal(this);
        // Rember the force Local setting
    bool CurrForceLocal = GetForceLocal();                    // dont want to clobber it with false if componentPass made it true
    forceLocal = (CurrForceLocal || forceLocal) ? true : false;     // if it was set before, or is true now, make it true... 
    SetForceLocal(forceLocal);

    if( IsTMAnimated() && (!GetParentNode()->IsRootNode()) )
    {
        ((plMaxNode*)GetParentNode())->SetForceLocal(true);
    }

    return true;
}

bool plMaxNode::MakeCoordinateInterface(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    const char* dbgNodeName = GetName();
    if (!CanConvert()) 
        return false;
    plCoordinateInterface* ci = nullptr;

    bool forceLocal = GetForceLocal();

    bool needCI = (!GetParentNode()->IsRootNode())
        || NumberOfChildren()
        || forceLocal;
    // If we have a transform, set up a coordinateinterface
    if( needCI )
    {
        hsMatrix44 loc2Par = GetLocalToParent44();
        hsMatrix44 par2Loc = GetParentToLocal44();
        if( GetFilterInherit() )
            ci = new plFilterCoordInterface;
        else
            ci = new plCoordinateInterface;
        //-------------------------
        // Get data from Node, then its key, then its name
        //-------------------------
        plKey pNodeKey = GetKey();
        hsAssert(pNodeKey, "Missing key for this Object");
        ST::string pName = pNodeKey->GetName();
        plLocation nodeLoc = GetLocation();

        plKey pCiKey = hsgResMgr::ResMgr()->NewKey(pName, ci,nodeLoc, GetLoadMask());
        ci->SetLocalToParent(loc2Par, par2Loc);

        bool usesPhysics = GetPhysicalProps()->IsUsed();
        ci->SetProperty(plCoordinateInterface::kCanEverDelayTransform, !usesPhysics);
        ci->SetProperty(plCoordinateInterface::kDelayedTransformEval, !usesPhysics);

        hsgResMgr::ResMgr()->AddViaNotify(pCiKey, new plObjRefMsg(pNodeKey, plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
    }
    return true;
}

bool plMaxNode::MakeModifiers(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    if (!CanConvert()) 
        return false;
    
    bool forceLocal = GetForceLocal();
    const char *dbgNodeName = GetName();

    bool addMods = (!GetParentNode()->IsRootNode())
        || forceLocal;

    if (addMods)
    {
    // create / add modifiers

        // mf horse hack testing ViewFace which is already obsolete
        if ( UserPropExists("ViewFacing") )
        {
            plViewFaceModifier* pMod = new plViewFaceModifier;
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
                hsVector3 scale(1.f, 1.f, 1.f);
                while( (nGot < 3) && (token = toker.next()) )
                {
                    switch( nGot )
                    {
                    case 0:
                        scale.fZ = float(atof(token));
                        break;
                    case 1:
                        scale.fX = scale.fZ;
                        scale.fY = float(atof(token));
                        scale.fZ = 1.f;
                        break;
                    case 2:
                        scale.fZ = float(atof(token));
                        break;
                    }
                    nGot++;
                }
                pMod->SetScale(scale);
            }
            AddModifier(pMod, ST::string::from_utf8(GetName()));
        }
    }
    return true;

}

bool plMaxNode::MakeParentOrRoomConnection(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    if (!CanConvert()) 
        return false;

    auto dbgNodeName = GetName();
    plSceneObject *pso = GetSceneObject();
    if( !GetParentNode()->IsRootNode() )
    {
        plKey parKey = GetParentKey();
        plCoordinateInterface* ci = const_cast <plCoordinateInterface*> (pso->GetCoordinateInterface());
        hsAssert(ci,"Missing CI");


        plIntRefMsg* msg = new plIntRefMsg(parKey, plRefMsg::kOnCreate, -1, plIntRefMsg::kChildObject);
        msg->SetRef(pso);
        hsgResMgr::ResMgr()->AddViaNotify(msg, plRefFlags::kPassiveRef);
    }

    hsgResMgr::ResMgr()->AddViaNotify(pso->GetKey(), new plNodeRefMsg(GetRoomKey(), plRefMsg::kOnCreate, -1, plNodeRefMsg::kObject), plRefFlags::kActiveRef);
    return true;
}

void plMaxNode::IWipeBranchDrawable(bool b)
{
    SetDrawable(b);
    for (int i = 0; i < NumberOfChildren(); i++)
    {
        plMaxNode *pChild = (plMaxNode *)GetChildNode(i);
        pChild->IWipeBranchDrawable(b);
    }
}

//// CanMakeMesh /////////////////////////////////////////////////////////////
//  Returns true if MakeMesh() on this node will result in spans being stored
//  on a drawable. Takes in the object pointer to avoid having to do redundant
//  work to get it.
//  9.25.2001 mcn - Made public so components can figure out if this node is
//  meshable.

bool    plMaxNode::CanMakeMesh( Object *obj, plErrorMsg *pErrMsg, plConvertSettings *settings )
{
    if (obj == nullptr)
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

void ITestAdjacencyRecur(const std::vector<int>* vertList, int iVert, hsBitVector& adjVerts)
{
    adjVerts.SetBit(iVert);

    for (int vert : vertList[iVert])
    {
        if (!adjVerts.IsBitSet(vert))
        {
            ITestAdjacencyRecur(vertList, vert, adjVerts);
        }
    }
}

bool ITestAdjacency(const std::vector<int>* vertList, int numVerts)
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

    uint16_t* idx = span->fIndexData;
    int numFaces = span->fNumIndices / 3;

    uint32_t stride = span->GetVertexSize(span->fFormat);

    uint8_t* vertData = span->fVertexData;
    int numVerts = span->fNumVerts;

    bool someIn = false;
    bool someOut = false;

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
        float faceDist = faceNorm.InnerProduct(pos[0]);

        int j;
        for( j = 0; j < numVerts; j++ )
        {
            hsPoint3* p = (hsPoint3*)(vertData + idx[0] * stride);


            float dist = p->InnerProduct(faceNorm) - faceDist;

            const float kSmall = 1.e-3f;
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

    std::vector<int>* vertList = new std::vector<int>[numVerts];

    std::vector<hsVector3>* normList = new std::vector<hsVector3>[numVerts];
    std::vector<float>* distList = new std::vector<float>[numVerts];

    uint16_t* idx = span->fIndexData;

    uint32_t stride = span->GetVertexSize(span->fFormat);

    uint8_t* vertData = span->fVertexData;

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
        float faceDist = faceNorm.InnerProduct(pos[0]);


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
                    std::vector<int>& verts = vertList[idx[iVtx]];
                    if (std::find(verts.cbegin(), verts.cend(), idx[jVtx]) == verts.cend())
                        verts.emplace_back(idx[jVtx]);
                }
            }
            normList[idx[iVtx]].emplace_back(faceNorm);
            distList[idx[iVtx]].emplace_back(faceDist);

        }
        idx += 3;
    }

    bool someIn = false;
    bool someOut = false;
    for (int i = 0; i < numVerts; i++)
    {
        for (size_t k = 0; k < normList[i].size(); k++)
        {
            for (size_t j = 0; j < vertList[i].size(); j++)
            {
                hsPoint3* pos = (hsPoint3*)(vertData + vertList[i][j] * stride);
                float dist = pos->InnerProduct(normList[i][k]) - distList[i][k];

                const float kSmall = 1.e-3f;
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

// Returns nullptr if there isn't a sceneobject and a drawinterface.
plDrawInterface* plMaxNode::GetDrawInterface()
{
    plDrawInterface* di = nullptr;
    plSceneObject* obj = GetSceneObject();
    if( obj )
    {
        di = obj->GetVolatileDrawInterface();
    }
    return di;
}

bool plMaxNode::MakeMesh(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    std::vector<plGeometrySpan *> spanArray;
    plDrawInterface             *newDI = nullptr;

    bool        gotMade = false;
    bool        haveAddedToSceneNode = false;
    hsGMesh     *myMesh = nullptr;
    uint32_t    triMeshIndex = (uint32_t)-1;
    const char  *dbgNodeName = GetName();
    TSTR sdata;
    hsStringTokenizer toker;
    plLocation nodeLoc = GetLocation();
    
    if (!GetSwappableGeom())
    {
        if (!CanConvert()) 
            return false;

        if( UserPropExists( "Plasma2_Camera" ) || !GetDrawable()  )
        {
            SetMesh(nullptr);
            return true;
        }
    }
    
    if( GetSwappableGeomTarget() != (uint32_t)-1)
    {
        // This node has no geometry on export, but will have some added at runtime,
        // so it needs a special drawInterface

        plInstanceDrawInterface *newDI = new plInstanceDrawInterface;
        newDI->fTargetID = GetSwappableGeomTarget();
        plKey pDiKey = hsgResMgr::ResMgr()->NewKey( GetKey()->GetName(), newDI, nodeLoc, GetLoadMask() );
        hsgResMgr::ResMgr()->AddViaNotify(pDiKey, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

        plSwapSpansRefMsg *sMsg = new plSwapSpansRefMsg(pDiKey, plRefMsg::kOnCreate, -1, -1);
        plDrawableSpans *drawable = IGetSceneNodeSpans(IGetDrawableSceneNode(pErrMsg), true, true);
        hsgResMgr::ResMgr()->AddViaNotify(drawable->GetKey(), sMsg, plRefFlags::kActiveRef);

        return true;
    }

    if( GetInstanced() )
    {
        std::vector<plMaxNode *> nodes;
        TimeValue   t = hsConverterUtils::Instance().GetTime(GetInterface());
        size_t      numInstances = IBuildInstanceList(GetObjectRef(), t, nodes, true);

        /// Instanced, find an iNode in the list that's been converted already
        for (size_t i = 0; i < numInstances; i++)
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
    bool movable = IsMovable();

    if( !gotMade )
    {
        if( !plMeshConverter::Instance().CreateSpans( this, spanArray, !settings->fDoPreshade ) )
            return false;
    }
    if (spanArray.empty())
        return true;

    for (plGeometrySpan* span : spanArray)
        span->fMaxOwner = GetKey()->GetName();

    uint32_t shadeFlags = 0;
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
        for (plGeometrySpan* span : spanArray)
            span->fProps |= shadeFlags;
    }

    bool DecalMat = false;
    bool NonDecalMat = false;

    for (plGeometrySpan* span : spanArray)
    {
        if (span->fMaterial->IsDecal())
            DecalMat = true;
        else
            NonDecalMat = true;
    }
    if (!(DecalMat ^ NonDecalMat))
    {
        for (plGeometrySpan* span : spanArray)
            span->ClearBuffers();

        if (pErrMsg->Set((plConvert::Instance().fWarned & plConvert::kWarnedDecalAndNonDecal) == 0, GetName(), 
            "This node has both regular and decal materials, and thus will be ignored.").CheckAskOrCancel())
        {
            plConvert::Instance().fWarned |= plConvert::kWarnedDecalAndNonDecal;
        }
        pErrMsg->Set(false);

        return false;
    }

    bool isDecal = IsLegalDecal(false); // Don't complain about the parent

    /// Get some stuff
    bool forceLocal = GetForceLocal();

    hsMatrix44 l2w = GetLocalToWorld44();
    hsMatrix44 w2l = GetWorldToLocal44();

    /// 4.17.2001 mcn - TEMP HACK to test fog by adding a key to a bogus fogEnviron object to ALL spans
/*      plFogEnvironment    *myFog = nullptr;
    plKey               myFogKey = hsgResMgr::ResMgr()->FindExportAlias( "HACK_FOG", plFogEnvironment::Index() );   
    if (myFogKey != nullptr)
        myFog = plFogEnvironment::ConvertNoRef( myFogKey->GetObjectPtr() );
    else
    {
        hsColorRGBA     color;
        color.Set( 0.5, 0.5, 1, 1 );

        // Exp fog
        myFog = new plFogEnvironment( plFogEnvironment::kExpFog, 700.f, 1.f, color );
        myFogKey = hsgResMgr::ResMgr()->NewKey( "HACK_FOG", myFog, nodeLoc );
        hsgResMgr::ResMgr()->AddExportAlias( "HACK_FOG", plFogEnvironment::Index(), myFogKey );
    }

    for( int j = 0; j < spanArray.GetCount(); j++ )
    {
        spanArray[ j ].fFogEnviron = myFog;
    }
*/      /// 4.17.2001 mcn - TEMP HACK end


    plDrawable* drawable = nullptr;
    plSceneNode* tmpNode = nullptr;

    /// Find the ice to add it to

    if (GetSwappableGeom()) // We just want to make a geo span, not actually add it to a drawable(interface)
    {
        plMaxNode *drawableSource = (plMaxNode *)(GetParentNode()->IsRootNode() ? this : GetParentNode());
        plSceneNode *tmpNode = drawableSource->IGetDrawableSceneNode(pErrMsg);

        plDrawableSpans *drawable = IGetSceneNodeSpans(tmpNode, true, true);
        ISetupBones(drawable, spanArray, l2w, w2l, pErrMsg, settings);

        std::vector<plGeometrySpan *> *swapSpans = &GetSwappableGeom()->fSpans;
        swapSpans->insert(swapSpans->end(), spanArray.begin(), spanArray.end());

        ST::string tmpName = ST::format("{}_SMsh", GetName());
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
            for (plGeometrySpan* span : spanArray)
                span->fDecalLevel = GetDecalLevel();
        }

        {
            /// Make a new drawInterface (will assign stuff to it later)
            newDI = new plDrawInterface;
            plKey pDiKey = hsgResMgr::ResMgr()->NewKey( GetKey()->GetName(), newDI, nodeLoc, GetLoadMask() );
            hsgResMgr::ResMgr()->AddViaNotify(pDiKey, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

            /// Attach the processed spans to the DI (through drawables)
            IAssignSpansToDrawables( spanArray, newDI, pErrMsg, settings );
        }
    }

    return true;
}

plSceneNode *plMaxNode::IGetDrawableSceneNode(plErrorMsg *pErrMsg)
{
    plSceneNode *sn = nullptr;

    sn = plSceneNode::ConvertNoRef( GetRoomKey()->GetObjectPtr() );

    return sn;
}

//// IAssignSpansToDrawables /////////////////////////////////////////////////
//  Given a span array, adds it to the node's drawables, creating them if
//  necessary. Then it takes the resulting indices and drawable pointers
//  and assigns them to the given drawInterface.

void plMaxNode::IAssignSpansToDrawables(std::vector<plGeometrySpan *> &spanArray, plDrawInterface *di,
                                        plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    std::vector<plGeometrySpan *> opaqueArray, blendingArray, sortingArray;
    plDrawableSpans             *oSpans = nullptr, *bSpans = nullptr, *sSpans = nullptr;

    size_t      sCount = 0, oCount = 0, bCount = 0;
    plSceneNode *tmpNode = nullptr;
    hsMatrix44  l2w = GetLocalToWorld44();
    hsMatrix44  w2l = GetWorldToLocal44();
    uint32_t      oIndex = (uint32_t)-1, bIndex = (uint32_t)-1, sIndex = uint32_t(-1);

    tmpNode = IGetDrawableSceneNode(pErrMsg);

    hsBitVector convexBits;
    /// Separate the array into two arrays, one opaque and one blending
    for (size_t i = 0; i < spanArray.size(); i++)
    {
        if( spanArray[ i ]->fProps & plGeometrySpan::kRequiresBlending )
        {
            bool needFaceSort = !GetNoFaceSort() && !IsGeoSpanConvex(this, spanArray[i]);
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

    opaqueArray.resize(oCount);
    blendingArray.resize(bCount);
    sortingArray.resize(sCount);
    sCount = oCount = bCount = 0;
    for (size_t i = 0; i < spanArray.size(); i++)
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
    if (!opaqueArray.empty())
        oSpans = plDrawableSpans::ConvertNoRef( IGetSceneNodeSpans( tmpNode, false ) );
    if (!blendingArray.empty())
        bSpans = plDrawableSpans::ConvertNoRef( IGetSceneNodeSpans( tmpNode, true, false ) );
    if (!sortingArray.empty())
        sSpans = plDrawableSpans::ConvertNoRef( IGetSceneNodeSpans( tmpNode, true, true ) );

    if (oSpans != nullptr)
        IAssignSpan( oSpans, opaqueArray, oIndex, l2w, w2l, pErrMsg, settings );
    if (bSpans != nullptr)
        IAssignSpan( bSpans, blendingArray, bIndex, l2w, w2l, pErrMsg, settings );
    if( sSpans )
        IAssignSpan( sSpans, sortingArray, sIndex, l2w, w2l, pErrMsg, settings );

    /// Now assign to the interface
    if( oSpans )
    {
        size_t iDraw = di->GetNumDrawables();
        di->SetDrawable( iDraw, oSpans );
        di->SetDrawableMeshIndex( iDraw, oIndex );
    }

    if( bSpans )
    {
        size_t iDraw = di->GetNumDrawables();
        di->SetDrawable( iDraw, bSpans );
        di->SetDrawableMeshIndex( iDraw, bIndex );
    }

    if( sSpans )
    {
        size_t iDraw = di->GetNumDrawables();
        di->SetDrawable( iDraw, sSpans );
        di->SetDrawableMeshIndex( iDraw, sIndex );
    }

}

//// IAssignSpan /////////////////////////////////////////////////////////////
//  Small utility function for IAssignSpansToDrawables, just does some of
//  the low-down work that's identical for each drawable/spans/etc.

void plMaxNode::IAssignSpan(plDrawableSpans *drawable, std::vector<plGeometrySpan *> &spanArray, uint32_t &index,
                            hsMatrix44 &l2w, hsMatrix44 &w2l,
                            plErrorMsg *pErrMsg, plConvertSettings *settings)
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
static void SetSpansBoneInfo(std::vector<plGeometrySpan *> &spanArray, uint32_t baseMatrix, uint32_t numMatrices)
{
    for (plGeometrySpan* span : spanArray)
    {
        span->fBaseMatrix = baseMatrix;
        span->fNumMatrices = numMatrices;
    }
}

//// ISetupBones /////////////////////////////////////////////////////////////
//  Adds the given bones to the given drawable, then sets up the given spans
//  with the right indices and sets the initial bone positions.
void plMaxNode::ISetupBones(plDrawableSpans *drawable, std::vector<plGeometrySpan *> &spanArray,
                            hsMatrix44 &l2w, hsMatrix44 &w2l,
                            plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    const char* dbgNodeName = GetName();

    if( !NumBones() )
        return;

    plMaxBoneMap *boneMap = GetBoneMap();
    if (boneMap && boneMap->GetBaseMatrixIndex(drawable) != (uint32_t)-1)
    {
        SetSpansBoneInfo(spanArray, boneMap->GetBaseMatrixIndex(drawable), boneMap->fNumBones);
        return;
    }

    int numBones = (boneMap ? boneMap->fNumBones : NumBones()) + 1;
    plMaxNodeBase **boneArray = new plMaxNodeBase*[numBones];

    if (boneMap)
        boneMap->FillBoneArray(boneArray);
    else
    {
        for (int i = 0; i < NumBones(); i++)
        {
            boneArray[i] = GetBone(i);
        }
    }

    std::vector<hsMatrix44> initialB2W(numBones);
    std::vector<hsMatrix44> initialW2B(numBones);

    std::vector<hsMatrix44> initialL2B(numBones);
    std::vector<hsMatrix44> initialB2L(numBones);

    initialB2W[0].Reset();
    initialW2B[0].Reset();

    initialL2B[0].Reset();
    initialB2L[0].Reset();

    for (int i = 1; i < numBones; i++)
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
    //      are force localed, Our InitialBone won't match, because it also includes
    //      our transform as well as the bone's. If we've been flattened into world
    //      space, our transform is ident and we can share. This is the normal case
    //      in scene boning. So InitialBones have to match in count and matrix value.
    uint32_t baseMatrix = drawable->FindBoneBaseMatrix(initialL2B, GetSwappableGeom() != nullptr);
    if( baseMatrix != uint32_t(-1) )
    {
        SetSpansBoneInfo(spanArray, baseMatrix, numBones);
        delete [] boneArray;
        return;
    }
    
    baseMatrix = drawable->AppendDIMatrixSpans(numBones);
    SetSpansBoneInfo(spanArray, baseMatrix, numBones);
    if (boneMap)
        boneMap->SetBaseMatrixIndex(drawable, baseMatrix);

    for (int i = 1; i < numBones; i++)
    {
        plMaxNodeBase *bone = boneArray[i-1];
        plSceneObject* obj = bone->GetSceneObject();
        const char  *dbgBoneName = bone->GetName();

        // Pick which drawable to point the DI to
        size_t iDraw = 0;

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
            di = new plDrawInterface;
            plKey diKey = hsgResMgr::ResMgr()->NewKey(GetKey()->GetName(), di, nodeLoc, GetLoadMask());
            hsgResMgr::ResMgr()->AddViaNotify(diKey, new plObjRefMsg(obj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
        }

        if( di->GetNumDrawables() <= iDraw )
        {
            size_t diIndex = drawable->NewDIMatrixIndex();
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
//  Given an instance node, instances the geoSpans that the node owns and
//  stores them in the given array.

bool plMaxNode::IMakeInstanceSpans(plMaxNode *node, std::vector<plGeometrySpan *> &spanArray,
                                   plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    int     index, i;

    
    plSceneObject *obj = node->GetSceneObject();
    if( !obj )
        return false;

    const plDrawInterface *di = obj->GetDrawInterface();
    if( !di )
        return false;

    bool setVisDists = false;
    float minDist, maxDist;
    if( hsMaterialConverter::HasVisDists(this, 0, minDist, maxDist) )
    {
        setVisDists = true;
    }

    index = 0;
    spanArray.clear();
    for (size_t iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
        if( !dr )
            continue;
        if( di->GetDrawableMeshIndex(iDraw) == (uint32_t)-1 )
            continue;

        plDISpanIndex disi = dr->GetDISpans(di->GetDrawableMeshIndex(iDraw));

        spanArray.resize(spanArray.size() + disi.GetCount());
        for (size_t i = 0; i < disi.GetCount(); i++)
        {
            spanArray[index] = new plGeometrySpan;
            spanArray[index]->MakeInstanceOf(dr->GetGeometrySpan(disi[i]));

            if( setVisDists )
            {
                spanArray[ index ]->fMinDist = (minDist);
                spanArray[ index ]->fMaxDist = (maxDist);
            }

            dr->GetGeometrySpan(disi[i])->fProps |= plGeometrySpan::kInstanced;

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
#if 1       // Change this to 0 to just always use the same materials on instances (old, incorrect way)
    Mtl *newMtl = GetMtl(), *origMtl = node->GetMtl();
    if (newMtl != nullptr && newMtl == origMtl)    // newMtl should == origMtl, but check just in case
    {
        std::vector<hsGMaterial *> oldMaterials, newMaterials;

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
        for (plGeometrySpan* span : spanArray)
        {
            // Find the span's original material
            for (size_t j = 0; j < oldMaterials.size(); j++)
            {
                if (span->fMaterial == oldMaterials[j])
                {
                    span->fMaterial = newMaterials[j];
                    break;
                }
            }

        }
    }
#endif

    return true;
}

//// IBuildInstanceList //////////////////////////////////////////////////////
//  For the given object, builds a list of all the iNodes that have that
//  object as their object. Returns the total node count

size_t plMaxNode::IBuildInstanceList(Object *obj, TimeValue t, std::vector<plMaxNode *> &nodes, bool beMoreAccurate)
{
    Object              *thisObj = EvalWorldState( t ).obj;
    DependentIterator   di( obj );
    ReferenceMaker      *rm;
    plMaxNode           *node;
    plKey               sceneNodeKey = GetRoomKey();


    /// Use the DependentIterator to loop through all the dependents of the object,
    /// looking for nodes that use it
    nodes.clear();
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
                        nodes.emplace_back(node);
                }
            }
        }
    }

    return nodes.size();
}

//// IMaterialsMatch /////////////////////////////////////////////////////////
//  Given two nodes that are instances of each other, this function determines
//  whether the resulting exported materials for both will be the same or not.
//  If not, we need to not instance/share the geometry, since the UV channels
//  could (and most likely will) be different.
//  To test this, all we really need to do is check the return values of
//  AlphaHackLayersNeeded(), since all the other material parameters will be
//  identical due to these nodes being instances of each other.

bool    plMaxNode::IMaterialsMatch( plMaxNode *otherNode, bool beMoreAccurate )
{
    Mtl *mtl = GetMtl(), *otherMtl = otherNode->GetMtl();
    if( mtl != otherMtl )
        return false;   // The two objects have different materials, no way we
                        // can try to instance them now
    if (mtl == nullptr)
        return true;    // Both nodes have no material, works for me

    // If we're not told to be accurate, then we just quit here. This is because
    // in the early passes, we *can't* be more accurate, since we won't have all
    // the info yet, so we don't bother checking it
    if( !beMoreAccurate )
        return true;

    if( hsMaterialConverter::IsMultiMat( mtl ) )
    {
        int     i;
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

bool plMaxNode::ShadeMesh(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    const char* dbgNodeName = GetName();

    std::vector<plGeometrySpan *> spanArray;

    if( !(CanConvert() && GetDrawable()) ) 
        return true;

    plSceneObject* obj = GetSceneObject();
    if( !obj )
        return true;

    const plDrawInterface* di = obj->GetDrawInterface();
    if( !di )
        return true;

    for (size_t iDraw = 0; iDraw < di->GetNumDrawables(); iDraw++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(iDraw));
        if( !dr )
            continue;
        if( di->GetDrawableMeshIndex(iDraw) == (uint32_t)-1 )
            continue;

        plDISpanIndex disi = dr->GetDISpans(di->GetDrawableMeshIndex(iDraw));

        for (size_t i = 0; i < disi.GetCount(); i++)
        {
            spanArray.emplace_back(dr->GetGeometrySpan(disi[i]));
        }

        hsMatrix44 l2w = GetLocalToWorld44();
        hsMatrix44 w2l = GetWorldToLocal44();

        /// Shade the spans now
        // Either do vertex shading or generate a light map.
        if( GetLightMapComponent() )
        {
            plLightMapGen::Instance().MakeMaps(this, l2w, w2l, spanArray, pErrMsg, nullptr);

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

bool plMaxNode::MakeOccluder(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    if( !UserPropExists("Occluder") )
        return true;

    bool twoSided = UserPropExists("OccTwoSided");
    bool isHole = UserPropExists("OccHole");

    return ConvertToOccluder(pErrMsg, twoSided, isHole);
}

static void IRemoveCollinearPoints(std::vector<Point3>& facePts)
{
    for (size_t i = 0; i < facePts.size(); )
    {
        size_t j = i + 1 >= facePts.size() ? 0 : i + 1;
        size_t k = j + 1 >= facePts.size() ? 0 : j + 1;
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
            facePts.erase(facePts.begin() + j);
        }
    }
}

bool plMaxNode::ConvertToOccluder(plErrorMsg* pErrMsg, bool twoSided, bool isHole)
{
    if( !CanConvert() )
        return false;

    /// Get some stuff
    plLocation nodeLoc = GetLocation();

    bool moving = IsMovable();

    Matrix3 tmp(true);

    Matrix3 maxL2V = GetLocalToVert(TimeValue(0));
    Matrix3 maxV2L = GetVertToLocal(TimeValue(0));

    std::vector<plCullPoly> polys;

    uint32_t polyInitFlags = plCullPoly::kNone;
    if( isHole )
        polyInitFlags |= plCullPoly::kHole;
    else
    if( twoSided )
        polyInitFlags |= plCullPoly::kTwoSided;

    Object *obj = EvalWorldState(TimeValue(0)).obj;
    if( obj->CanConvertToType(triObjectClassID) )
    {
        TriObject   *meshObj = (TriObject *)obj->ConvertToType(TimeValue(0), triObjectClassID);
        if( meshObj )
        {

            Mesh mesh(meshObj->mesh);
            
            constexpr float kNormThresh = hsConstants::pi<float> / 20.f;
            constexpr float kEdgeThresh = hsConstants::pi<float> / 20.f;
            constexpr float kBias = 0.1f;
            constexpr float kMaxEdge = -1.f;
            constexpr DWORD kOptFlags = OPTIMIZE_SAVESMOOTHBOUNDRIES;

            mesh.Optimize(
                kNormThresh, // threshold of normal differences to preserve
                kEdgeThresh, // When the angle between adjacent surface normals is less than this value the auto edge is performed (if the OPTIMIZE_AUTOEDGE flag is set). This angle is specified in radians.
                kBias, // Increasing the bias parameter keeps triangles from becoming degenerate. range [0..1] (0 = no bias).
                kMaxEdge, // This will prevent the optimize function from creating edges longer than this value. If this parameter is <=0 no limit is placed on the length of the edges.
                kOptFlags, // Let them input using smoothing groups, but nothing else.
                nullptr); // progress bar

            
            MNMesh mnMesh(mesh);

            mnMesh.EliminateCollinearVerts();
            mnMesh.EliminateCoincidentVerts(0.1f);

            // Documentation recommends MakeConvexPolyMesh over MakePolyMesh. Naturally, MakePolyMesh works better.
//          mnMesh.MakeConvexPolyMesh();
            mnMesh.MakePolyMesh();
            mnMesh.MakeConvex();
//          mnMesh.MakePlanar(hsDegreesToRadians(1.f)); // Completely ineffective. Winding up with majorly non-planar polys.

            mnMesh.Transform(maxV2L);

            polys.clear();
            polys.reserve(mesh.getNumFaces());

            // Unfortunate problem here. Max is assuming that eventually this will get rendered, and so
            // we need to avoid T-junctions. Fact is, T-junctions don't bother us at all, where-as colinear
            // verts within a poly do (just as added overhead).
            // So, to make this as painless (ha ha) as possible, we could detach each poly as we go to
            // its own mnMesh, then eliminate colinear verts on that single poly mesh. Except
            // EliminateCollinearVerts doesn't seem to actually do that. So we'll just have to
            // manually detect and skip collinear verts.
            std::vector<Point3> facePts;
            for (int i = 0; i < mnMesh.numf; i++)
            {
                MNFace& face = mnMesh.f[i];

                facePts.clear();
                for (int j = 0; j < face.deg; j++)
                {
                    facePts.emplace_back(mnMesh.v[face.vtx[j]].p);
                }
                IRemoveCollinearPoints(facePts);

                if (facePts.size() < 3)
                    continue;

                size_t lastAdded = 2;

                plCullPoly* poly = &polys.emplace_back();
                poly->fVerts.clear();

                Point3 p;
                hsPoint3 pt;

                p = facePts[0];
                pt.Set(p.x, p.y, p.z);
                poly->fVerts.emplace_back(pt);

                p = facePts[1];
                pt.Set(p.x, p.y, p.z);
                poly->fVerts.emplace_back(pt);

                p = facePts[2];
                pt.Set(p.x, p.y, p.z);
                poly->fVerts.emplace_back(pt);

                for (size_t j = lastAdded + 1; j < facePts.size(); j++)
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


                    float dotSq = aXb.InnerProduct(bXc);
                    dotSq *= dotSq;

                    const float kMinLenSq = 1.e-8f;
                    const float kMinDotFracSq = 0.998f * 0.998f;

                    float lenSq = aXb.MagnitudeSquared() * bXc.MagnitudeSquared();
                    if( lenSq < kMinLenSq )
                        continue;

                    // If not planar, move to new poly.
                    if( dotSq < lenSq * kMinDotFracSq )
                    {
                        poly->InitFromVerts(polyInitFlags);

                        plCullPoly* lastPoly = &polys.back();
                        poly = &polys.emplace_back();
                        poly->fVerts.clear();
                        poly->fVerts.emplace_back(lastPoly->fVerts[0]);
                        poly->fVerts.emplace_back(lastPoly->fVerts[lastAdded]);
    
                        lastAdded = 1;
                    }

                    poly->fVerts.emplace_back(pt);
                    lastAdded++;
                }

                poly->InitFromVerts(polyInitFlags);
            }
        }
    }

    if (!polys.empty())
    {
        plOccluder* occ = nullptr;
        plMobileOccluder* mob = nullptr;
        if( moving )
        {
            mob = new plMobileOccluder;
            occ = mob;
        }
        else
        {
            occ = new plOccluder;
        }

        occ->SetPolyList(polys);
        occ->ComputeFromPolys();

        // Register it.
        ST::string tmpName;
        if( GetKey() && !GetKey()->GetName().empty() )
        {
            tmpName = ST::format("{}_Occluder", GetKey()->GetName());
        }
        else
        {
            static int numOcc = 0;
            tmpName = ST::format("Occluder_{04}", numOcc);
        }
        plKey key = hsgResMgr::ResMgr()->NewKey( tmpName, occ, nodeLoc, GetLoadMask() );

        hsgResMgr::ResMgr()->AddViaNotify(occ->GetKey(), new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    }
    return true;
}

bool plMaxNode::MakeLight(plErrorMsg *pErrMsg, plConvertSettings *settings)
{

    if (!CanConvert()) 
        return false;

    if (!GetRunTimeLight())
        return true;

    /// Get some stuff
    plLocation nodeLoc = GetLocation();
    bool forceLocal = GetForceLocal();

    hsMatrix44 l2w = GetLocalToWorld44();
    hsMatrix44 w2l = GetWorldToLocal44();

    hsMatrix44 lt2l = GetVertToLocal44();
    hsMatrix44 l2lt = GetLocalToVert44();


    plLightInfo* liInfo = nullptr;


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

        hsgResMgr::ResMgr()->AddViaNotify(liInfo->GetKey(), new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface), plRefFlags::kActiveRef);


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
    plLightInfo* liInfo = nullptr;
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

    float attenConst, attenLinear, attenQuadratic;

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

bool plMaxNode::IGetRTLightAttenValues(IParamBlock2* ProperPB, float& attenConst, float& attenLinear, float& attenQuadratic, float &attenCutoff )
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
    float attenConst, attenLinear, attenQuadratic, attenCutoff;

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

    float inner = hsDegreesToRadians(ls.hotsize);
    float outer = hsDegreesToRadians(ls.fallsize);

    /// 4.26.2001 mcn - MAX gives us full angles, but we want to store half angles
    liInfo->SetSpotInner( inner / 2.0f );
    liInfo->SetSpotOuter( outer / 2.0f );
    liInfo->SetFalloff(1.f);
}

void plMaxNode::IGetRTCone(plSpotLightInfo* liInfo, IParamBlock2* ProperPB)
{

    //TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
    TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
    float inner, outer;

    inner = hsDegreesToRadians(ProperPB->GetFloat(plRTLightBase::kHotSpot, timeVal)); //ls.hotsize);
    outer = hsDegreesToRadians(ProperPB->GetFloat(plRTLightBase::kFallOff, timeVal)); //ls.fallsize);

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
        return nullptr;
    }

    plSpotLightInfo* spot = new plSpotLightInfo;

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
        return nullptr;
    }

    plOmniLightInfo* omni = new plOmniLightInfo;

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
        return nullptr;
    }

    plLightInfo* plasLight = nullptr;
    if( light->GetProjMap() )
    {
        plLimitedDirLightInfo* ldl = new plLimitedDirLightInfo;

        float sz = light->GetFallsize(timeVal, FOREVER);
        float depth = 1000.f;
        ldl->SetWidth(sz);
        ldl->SetHeight(sz);
        ldl->SetDepth(depth);

        plasLight = ldl;
    }
    else
    {

        plDirectionalLightInfo* direct = new plDirectionalLightInfo;
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
        return nullptr;

    }

    plSpotLightInfo* spot = new plSpotLightInfo;

    if(!ThisObjPB->GetInt(plRTLightBase::kLightOn))
        spot->SetProperty(plLightInfo::kDisable, true); 

    IGetRTLightColors(spot,ThisObjPB);

    IGetRTLightAttenuation(spot,ThisObjPB);

    IGetRTCone(spot, ThisObjPB);

    //plSpotModifier* liMod = new plSpotModifier;

    //GetRTLightColAnim(ThisObjPB, liMod);
    //GetRTLightAttenAnim(ThisObjPB, liMod);
    //GetRTConeAnim(ThisObjPB, liMod);

    //IAttachRTLightModifier(liMod);

//  if( obj != light )
    //  light->DeleteThis();

    return spot;
}


plLightInfo* plMaxNode::IMakeRTOmni(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
    TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

    Object *obj = EvalWorldState(timeVal).obj;
    
    Object *ThisObj = ((INode*)this)->GetObjectRef();
    IParamBlock2* ThisObjPB = ThisObj->GetParamBlockByID(plRTLightBase::kBlkOmniLight);

    plOmniLightInfo* omni = new plOmniLightInfo;

    if(!ThisObjPB->GetInt(plRTLightBase::kLightOn))
        omni->SetProperty(plLightInfo::kDisable, true); 

    IGetRTLightAttenuation(omni, ThisObjPB);

    IGetRTLightColors(omni, ThisObjPB);

    //plOmniModifier* liMod = new plOmniModifier;

    //GetRTLightColAnim(ThisObjPB, liMod);
    //GetRTLightAttenAnim(ThisObjPB, liMod);

    //IAttachRTLightModifier(liMod);


//  if( obj != light )
//      light->DeleteThis();

    return omni;
}


plLightInfo* plMaxNode::IMakeRTDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings)
{
    TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

    Object *obj = EvalWorldState(timeVal).obj;
    Object *ThisObj = ((INode*)this)->GetObjectRef();
    IParamBlock2* ThisObjPB = ThisObj->GetParamBlockByID(plRTLightBase::kBlkTSpotLight);


    plDirectionalLightInfo* direct = new plDirectionalLightInfo;

    if(!ThisObjPB->GetInt(plRTLightBase::kLightOn))
        direct->SetProperty(plLightInfo::kDisable, true); 

    IGetRTLightColors(direct, ThisObjPB);

    //plLightModifier* liMod = new plLightModifier;

    //GetRTLightColAnim(ThisObjPB, liMod);

    //IAttachRTLightModifier(liMod);


//  if( obj != light )
//      light->DeleteThis();

    return direct;
}

//// IMakeRTProjDirectional //////////////////////////////////////////////////
//  Conversion function for RT Projected Directional lights

plLightInfo *plMaxNode::IMakeRTProjDirectional( plErrorMsg *pErrMsg, plConvertSettings *settings )
{
    TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());

    Object *obj = EvalWorldState(timeVal).obj;
    Object *ThisObj = ((INode*)this)->GetObjectRef();

    IParamBlock2    *mainPB = ThisObj->GetParamBlockByID( plRTLightBase::kBlkMain );
    IParamBlock2    *projPB = ThisObj->GetParamBlockByID( plRTProjDirLight::kBlkProj );

    plLimitedDirLightInfo *light = new plLimitedDirLightInfo;

    light->SetWidth( projPB->GetFloat( plRTProjDirLight::kWidth ) );
    light->SetHeight( projPB->GetFloat( plRTProjDirLight::kHeight ) );
    light->SetDepth( projPB->GetFloat( plRTProjDirLight::kRange ) );

    if( !mainPB->GetInt( plRTLightBase::kLightOn ) )
        light->SetProperty( plLightInfo::kDisable, true ); 

    IGetRTLightColors( light, mainPB );

    //plLightModifier *liMod = new plLightModifier;

    //GetRTLightColAnim( mainPB, liMod );

    //IAttachRTLightModifier(liMod);

    return light;
}

bool plMaxNode::IGetProjection(plLightInfo* li, plErrorMsg* pErrMsg)
{
    bool persp = false;
    TimeValue timeVal = hsConverterUtils::Instance().GetTime(GetInterface());
    Object *obj = EvalWorldState(timeVal).obj;
    LightObject *light = (LightObject*)obj->ConvertToType(timeVal, RTSPOT_LIGHT_CLASSID);

    if( light )
        persp = true;
    
    if( !light )
        light = (LightObject*)obj->ConvertToType(timeVal, RTPDIR_LIGHT_CLASSID);

    if( !light )
        return false;

    bool retVal = false;
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

    IParamBlock2 *pb = nullptr;

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

            hsgResMgr::ResMgr()->AddViaNotify(proj->GetKey(), new plGenRefMsg(li->GetKey(), plRefMsg::kOnCreate, 0, 0), plRefFlags::kActiveRef);

            li->SetShadowCaster(false);

            li->SetProperty(plLightInfo::kLPMovable, true);


            retVal = true;
        }
        else
        {
            char buff[256];
            if( projMap && projMap->GetName() && *projMap->GetName() )
                sprintf(buff, "Can't find projected bitmap - %s", (const char *)projMap->GetName());
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
bool plMaxNode::IAttachRTLightModifier(plLightModifier* liMod)
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

    IParamBlock2 *pb = nullptr;

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
    Control *colorCtl = GetParamBlock2Controller(pb,  ParamID( plRTLightBase::kLightColor ) );
    if (colorCtl && cc.HasKeyTimes(colorCtl))
        return true;

    // Is the specularity animated?
    Control *specCtl = GetParamBlock2Controller(pb,  ParamID( plRTLightBase::kSpecularColorSwatch ) );
    if (specCtl && cc.HasKeyTimes(specCtl))
        return true;

    // Is the attenuation animated?  (Spot and Omni lights only)
    if (cid == RTSPOT_LIGHT_CLASSID || cid == RTOMNI_LIGHT_CLASSID)
    {
        Control *falloffCtl = GetParamBlock2Controller(pb,  ParamID( plRTLightBase::kAttenMaxFalloffEdit ) );
        if (falloffCtl && cc.HasKeyTimes(falloffCtl))
            return true;
    }

    // Is the cone animated? (Spot only)
    if (cid == RTSPOT_LIGHT_CLASSID)
    {
        Control *innerCtl = GetParamBlock2Controller(pb,  ParamID( plRTLightBase::kHotSpot ) );
        if (innerCtl && cc.HasKeyTimes(innerCtl))
            return true;

        Control *outerCtl = GetParamBlock2Controller(pb,  ParamID( plRTLightBase::kFallOff ) );
        if (outerCtl && cc.HasKeyTimes(outerCtl))
            return true;
    }

    return false;
}


void plMaxNode::GetRTLightAttenAnim(IParamBlock2* ProperPB, plAGAnim *anim)
{
    if( ProperPB->GetInt(plRTLightBase::kUseAttenuationBool, TimeValue(0)) )
    {
        Control* falloffCtl = GetParamBlock2Controller(ProperPB, ParamID(plRTLightBase::kAttenMaxFalloffEdit));
        if( falloffCtl )
        {
            plLeafController* subCtl;
            if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                subCtl = hsControlConverter::Instance().MakeScalarController(falloffCtl, this);
            else
                subCtl = hsControlConverter::Instance().MakeScalarController(falloffCtl, this,
                                                                            anim->GetStart(), anim->GetEnd());

            if( subCtl )
            {
                if( ProperPB->GetInt(plRTLightBase::kAttenTypeRadio, TimeValue(0)) == 2 )
                {
                    // Animation of a cutoff attenuation, which only needs a scalar channel
                    plOmniCutoffApplicator *app = new plOmniCutoffApplicator();
                    app->SetChannelName(GetName());
                    plScalarControllerChannel *chan = new plScalarControllerChannel(subCtl);
                    app->SetChannel(chan);
                    anim->AddApplicator(app);
                    if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                        anim->ExtendToLength(subCtl->GetLength());
                }
                else
                {
                    bool distSq = ProperPB->GetInt(plRTLightBase::kAttenTypeRadio, TimeValue(0));

                    int i;
                    for( i = 0; i < subCtl->GetNumKeys(); i++ )
                    {
                        hsScalarKey *key = subCtl->GetScalarKey(i);
                        if (key)
                        {
                            float attenEnd = key->fValue;
                            TimeValue tv = (TimeValue)(key->fFrame * MAX_TICKS_PER_FRAME);
                            float intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
                            float newVal = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / attenEnd;
                            if( distSq )
                                newVal /= attenEnd;

                            key->fValue = newVal;
                        }
                        hsBezScalarKey *bezKey = subCtl->GetBezScalarKey(i);
                        if (bezKey)
                        {
                            float attenEnd = bezKey->fValue;
                            TimeValue tv = (TimeValue)(bezKey->fFrame * MAX_TICKS_PER_FRAME);
                            float intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
                            float newVal = (intens * plSillyLightKonstants::GetFarPowerKonst() - 1.f) / attenEnd;
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
                        app = new plOmniSqApplicator;
                    else
                        app = new plOmniApplicator;

                    app->SetChannelName(GetName());
                    plScalarControllerChannel *chan = new plScalarControllerChannel(subCtl);
                    app->SetChannel(chan);
                    anim->AddApplicator(app);
                    if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                        anim->ExtendToLength(subCtl->GetLength());

                    float attenConst, attenLinear, attenQuadratic, attenCutoff;
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
                TimeValue tv = (TimeValue)(key->fFrame * MAX_TICKS_PER_FRAME);
                float intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
                key->fValue *= intens;
            }
            hsBezPoint3Key* bezKey = simp->GetBezPoint3Key(i);
            if (bezKey)
            {
                TimeValue tv = (TimeValue)(bezKey->fFrame * MAX_TICKS_PER_FRAME);
                float intens = ProperPB->GetFloat(plRTLightBase::kIntensity, tv);
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
    Control* ambientCtl = nullptr; // Ambient not currently supported
    Control* colorCtl = GetParamBlock2Controller(ProperPB, ParamID(plRTLightBase::kLightColor));
    Control* specCtl = GetParamBlock2Controller(ProperPB, ParamID(plRTLightBase::kSpecularColorSwatch));
    plPointControllerChannel *chan;

    if( ambientCtl )
    {
        plController* ctl;
        if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
            ctl = hsControlConverter::Instance().MakeColorController(ambientCtl, this);
        else
            ctl = hsControlConverter::Instance().MakeColorController(ambientCtl, this, anim->GetStart(), anim->GetEnd());

        if( ctl )
        {
            plLightAmbientApplicator *app = new plLightAmbientApplicator();
            app->SetChannelName(GetName());
            chan = new plPointControllerChannel(ctl);
            app->SetChannel(chan);
            anim->AddApplicator(app);
            if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                anim->ExtendToLength(ctl->GetLength());
        }
    }
    if( colorCtl )
    {
        plController* ctl;
        if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
            ctl = hsControlConverter::Instance().MakeColorController(colorCtl, this);
        else
            ctl = hsControlConverter::Instance().MakeColorController(colorCtl, this, anim->GetStart(), anim->GetEnd());
                        
        if( ctl )
        {
            IAdjustRTColorByIntensity(ctl, ProperPB);
            plLightDiffuseApplicator *app = new plLightDiffuseApplicator();
            app->SetChannelName(GetName());
            chan = new plPointControllerChannel(ctl);
            app->SetChannel(chan);
            anim->AddApplicator(app);
            if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                anim->ExtendToLength(ctl->GetLength());
        }
    }
    if( specCtl )
    {
        plController* ctl;
        if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
            ctl = hsControlConverter::Instance().MakeColorController(specCtl, this);
        else
            ctl = hsControlConverter::Instance().MakeColorController(specCtl, this, anim->GetStart(), anim->GetEnd());
        
        if( ctl )
        {
            plLightSpecularApplicator *app = new plLightSpecularApplicator();
            app->SetChannelName(GetName());
            chan = new plPointControllerChannel(ctl);
            app->SetChannel(chan);
            anim->AddApplicator(app);
            if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                anim->ExtendToLength(ctl->GetLength());
        }
    }
}

void plMaxNode::GetRTConeAnim(IParamBlock2* ProperPB, plAGAnim *anim)
{
    Control* innerCtl = GetParamBlock2Controller(ProperPB, ParamID(plRTLightBase::kHotSpot));
    Control* outerCtl = GetParamBlock2Controller(ProperPB, ParamID(plRTLightBase::kFallOff));
    plScalarControllerChannel *chan;

    if( innerCtl )
    {
        plLeafController* ctl;
        if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
            ctl = hsControlConverter::Instance().MakeScalarController(innerCtl, this);
        else
            ctl = hsControlConverter::Instance().MakeScalarController(innerCtl, this, anim->GetStart(), anim->GetEnd());
            
        if( ctl )
        {
            plSpotInnerApplicator *app = new plSpotInnerApplicator();
            app->SetChannelName(GetName());
            chan = new plScalarControllerChannel(ctl);
            app->SetChannel(chan);
            anim->AddApplicator(app);
            if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
                anim->ExtendToLength(ctl->GetLength());
        }
    }
    if( outerCtl )
    {
        plController* ctl;
        if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
            ctl = hsControlConverter::Instance().MakeScalarController(outerCtl, this);
        else
            ctl = hsControlConverter::Instance().MakeScalarController(outerCtl, this, anim->GetStart(), anim->GetEnd());

        if( ctl )
        {
            plSpotOuterApplicator *app = new plSpotOuterApplicator();
            app->SetChannelName(GetName());
            chan = new plScalarControllerChannel(ctl);
            app->SetChannel(chan);
            anim->AddApplicator(app);
            if (!anim->GetName().compare(ENTIRE_ANIMATION_NAME))
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
    return nullptr;
}

Point3 plMaxNode::GetFlexibility()
{
    uint32_t count = NumAttachedComponents();

    // Go through all the components attached to this node
    for (uint32_t i = 0; i < count; i++)
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
    uint32_t count = NumAttachedComponents();

    // Go through all the components attached to this node
    for (uint32_t i = 0; i < count; i++)
    {
        // See if any are a flexibility component.
        plComponentBase *comp = GetAttachedComponent(i);
        if( comp && (comp->ClassID() == LIGHTMAP_COMP_CID) )
        {
            plLightMapComponent* lmap = (plLightMapComponent*)comp;
            return lmap;
        }
    }
    return nullptr;
}

plDrawableCriteria plMaxNode::GetDrawableCriteria(bool needBlending, bool needSorting)
{
    plRenderLevel level = needBlending ? GetRenderLevel(needBlending) : plRenderLevel::OpaqueRenderLevel();

    if( GetSortAsOpaque() )
        level.Set(plRenderLevel::kOpaqueMajorLevel, level.Minor());

    uint32_t crit = 0;
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
//  Gets the required drawableSpans from a sceneNode. Creates a new one
//  if it can't find one.

plDrawableSpans *plMaxNode::IGetSceneNodeSpans( plSceneNode *node, bool needBlending, bool needSorting )
{

    plDrawableSpans *spans;
    ST::string      tmpName;
    plLocation      nodeLoc = GetLocation();  
    
    if( !needBlending )
        needSorting = false;

    plDrawableCriteria          crit = GetDrawableCriteria(needBlending, needSorting);

    spans = plDrawableSpans::ConvertNoRef( node->GetMatchingDrawable( crit ) );

    if (spans != nullptr)
    {
        if( GetNoSpanReSort() )
        {
            spans->SetNativeProperty(plDrawable::kPropNoReSort, true);
        }
        return spans;
    }


    /// Couldn't find--create and return it
    spans = new plDrawableSpans;
    if( needBlending )
    {
        /// Blending (deferred) spans
        spans->SetCriteria( crit );
        tmpName = ST::format("{}_{08x}_{x}BlendSpans", node->GetKeyName(), crit.fLevel.fLevel, crit.fCriteria);
    }
    else
    {
        /// Normal spans
        spans->SetCriteria( crit );
        tmpName = ST::format("{}_{08x}_{x}Spans", node->GetKeyName(), crit.fLevel.fLevel, crit.fCriteria);
    }

    if (GetSwappableGeomTarget() != (uint32_t)-1 || GetSwappableGeom()) // We intend to swap geometry with this node... flag the drawable as volatile
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

bool plMaxNode::SetupPropertiesPass(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    // TEMP
    if (IsComponent())
        return false;
    // End TEMP

    bool ret = true;
    
        uint32_t count = NumAttachedComponents();

    // Go through all the components attached to this node
    for (uint32_t i = 0; i < count; i++)
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
        if (mtl != nullptr && !GetParticleRelated())
        {
            if( hsMaterialConverter::IsMultiMat( mtl ) || hsMaterialConverter::IsMultipassMat( mtl ) || hsMaterialConverter::IsCompositeMat( mtl ) )
            {
                int i;
                for (i = 0; i < mtl->NumSubMtls(); i++)
                {
                    plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl->GetSubMtl( i ) );
                    if (pass != nullptr)
                    {
                        if( !pass->SetupProperties( this, pErrMsg ) )
                            ret = false;
                    }
                }
            }
            else
            {
                plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl );
                if (pass != nullptr)
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

bool plMaxNode::FirstComponentPass(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    // TEMP
    if (IsComponent())
        return false;
    // End TEMP

    bool ret = true;
    
    if (!CanConvert())
        return ret;
    uint32_t count = NumAttachedComponents();

    // Go through all the components attached to this node
    for (uint32_t i = 0; i < count; i++)
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

bool plMaxNode::ConvertComponents(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    // TEMP
    if (IsComponent())
        return false;
    // End TEMP

    bool ret = true;

    auto dbgNodeName = GetName();
    if (!CanConvert())
        return ret;

    uint32_t count = NumAttachedComponents();

    // Go through all the components attached to this node
    for (uint32_t i = 0; i < count; i++)
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

bool plMaxNode::DeInitComponents(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    // TEMP
    if (IsComponent())
        return false;
    // End TEMP

    bool ret = true;

    auto dbgNodeName = GetName();
    if (!CanConvert())
        return ret;

    uint32_t count = NumAttachedComponents();

    // Go through all the components attached to this node
    for (uint32_t i = 0; i < count; i++)
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
        if (mtl != nullptr && !GetParticleRelated())
        {
            if( hsMaterialConverter::IsMultiMat( mtl ) || hsMaterialConverter::IsMultipassMat( mtl ) || hsMaterialConverter::IsCompositeMat( mtl ) )
            {
                int i;
                for (i = 0; i < mtl->NumSubMtls(); i++)
                {
                    plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl->GetSubMtl( i ) );
                    if (pass != nullptr)
                    {
                        if( !pass->ConvertDeInit( this, pErrMsg ) )
                            ret = false;
                    }
                }
            }
            else
            {
                plPassMtlBase *pass = plPassMtlBase::ConvertToPassMtl( mtl );
                if (pass != nullptr)
                {
                    if( !pass->ConvertDeInit( this, pErrMsg ) )
                        ret = false;
                }
            }
        }
    }

    return ret;
}

bool plMaxNode::ClearData(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaAgeChunk);
    RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaDistChunk);
    RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaRoomChunk);

    RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaMaxNodeDataChunk);
//  RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaSceneViewerChunk);
    RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaLightChunk);

    return true;
}

// HASAGMOD
// Little special-purpose thing to see if a node has an animation graph modifier on it.
plAGModifier *plMaxNode::HasAGMod()
{
    auto name = GetName();
    if (CanConvert())
    {
        plSceneObject *SO = GetSceneObject();
        size_t numMods = SO->GetNumModifiers();

        for (size_t i = 0; i < numMods; i++)
        {
            const plModifier *mod = SO->GetModifier(i);

            if(plAGModifier::ConvertNoRef(mod)) {
                return (plAGModifier *)mod;
            }
        }
    }
    return nullptr;
}

plAGMasterMod *plMaxNode::GetAGMasterMod()
{
    auto name = GetName();
    if (CanConvert())
    {
        plSceneObject *SO = GetSceneObject();
        size_t numMods = SO->GetNumModifiers();

        for (size_t i = 0; i < numMods; i++)
        {
            const plModifier *mod = SO->GetModifier(i);

            if(plAGMasterMod::ConvertNoRef(mod)) {
                return (plAGMasterMod *)mod;
            }
        }
    }
    return nullptr;
}


// SETUPBONESALIASESRECUR
void plMaxNode::SetupBonesAliasesRecur(const char *rootName)
{
    if(CanConvert()) {
        if (!HasAGMod()) {
            ST::string nameToUse;
            
            // parse UserPropsBuf for entire BoneName line
            char localName[256];
            TSTR propsBuf;
            GetUserPropBuffer(propsBuf);
            auto start=strstr(propsBuf, "BoneName=");
            if (!start)
                start=strstr(propsBuf, "bonename=");
            const int len = strlen("BoneName=");
            if(start && UserPropExists("BoneName"))
            {
                start+=len;
                int i=0;
                while(*start != '\n' && *start)
                {
                    hsAssert(i<256, "localName overflow");
                    localName[i++]=*start++;
                }
                localName[i]=0;

                nameToUse = ST::string::from_utf8(localName);

            }
            else
            {
                ST::string nodeName = ST::string::from_utf8(GetName());
        //      char str[256];
        //      sprintf(str, "Missing 'BoneName=foo' UserProp, on object %s, using node name", nodeName ? nodeName : "?");
        //      hsAssert(false, str);

                nameToUse = nodeName;
            }

        /*  char aliasName[256];
            sprintf(aliasName, "%s_%s", rootName, nameToUse);

            plUoid* uoid = hsgResMgr::ResMgr()->FindAlias(aliasName, plSceneObject::Index());
            if( !uoid )
            {
                plAliasModifier* pAlMod = new plAliasModifier;
                pAlMod->SetAlias(aliasName);
                AddModifier(pAlMod);
            }
        */
            plAGModifier *mod = new plAGModifier(nameToUse);
            AddModifier(mod, ST::string::from_utf8(GetName()));
        }
    }

    int j = 0;
    for( j = 0; j < NumberOfChildren(); j++ )
        ((plMaxNode*)GetChildNode(j))->SetupBonesAliasesRecur(rootName);
}

void plMaxNode::SetDISceneNodeSpans( plDrawInterface *di, bool needBlending )
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
        return nullptr;

    INode* bone = skin->GetBone(0);

    if( !bone )
        return nullptr;

    while( !bone->GetParentNode()->IsRootNode() )
        bone = bone->GetParentNode();

    plMaxNode* boneRoot = (plMaxNode*)bone;

    if( !(boneRoot && boneRoot->CanConvert()) )
        return nullptr;

    return boneRoot;
}

void plMaxNode::GetBonesRootsRecur(std::vector<plMaxNode*>& nodes)
{
    plMaxNode* bRoot = GetBonesRoot();
    if( bRoot )
    {
        auto iter = std::find(nodes.cbegin(), nodes.cend(), bRoot);
        if (iter == nodes.cend())
            nodes.emplace_back(bRoot);
    }

    int i;
    for( i = 0; i < NumberOfChildren(); i++ )
        ((plMaxNode*)GetChildNode(i))->GetBonesRootsRecur(nodes);
}

plSceneObject* plMaxNode::MakeCharacterHierarchy(plErrorMsg *pErrMsg)
{
    plSceneObject* playerRoot = GetSceneObject();
    if (pErrMsg->Set(playerRoot->GetDrawInterface() != nullptr, GetName(), "Non-helper as player root").CheckAndAsk())
        return nullptr;
    const char *playerRootName = GetName();

    std::vector<plMaxNode*> bonesRoots;
    for (int i = 0; i < NumberOfChildren(); i++)
        ((plMaxNode*)GetChildNode(i))->GetBonesRootsRecur(bonesRoots);

    if (pErrMsg->Set(bonesRoots.size() > 1, playerRootName, "Found multiple bones hierarchies").CheckAndAsk())
        return nullptr;

    if (!bonesRoots.empty())
    {
        bonesRoots[0]->SetupBonesAliasesRecur(playerRootName);

        plSceneObject* boneRootObj = bonesRoots[0]->GetSceneObject();

        if (pErrMsg->Set(boneRootObj == nullptr, playerRootName, "No scene object for the bones root").CheckAndAsk())
            return nullptr;

        if( boneRootObj != playerRoot )
            hsMessageBox("This avatar's bone hierarchy does not have the avatar root node linked as a parent. "
                         "This may cause the avatar draw incorrectly.", playerRootName, hsMessageBoxNormal);
    }

    return playerRoot;
}

// Takes all bones found on this node (and any descendents) and sets up a single palette
void plMaxNode::SetupBoneHierarchyPalette(plMaxBoneMap *bones /* = nullptr */)
{
    const char* dbgNodeName = GetName();

    if( !CanConvert() )
        return;

    if (GetBoneMap())
        return;
    
    if (bones == nullptr)
    {
        bones = new plMaxBoneMap();
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

bool plMaxNode::IsLegalDecal(bool checkParent /* = true */)
{
    Mtl *mtl = GetMtl();
    if (mtl == nullptr || GetParticleRelated())
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
    bool deleteIt;

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
            if (mesh->mapFaces(i + 1) == nullptr)
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
//  Pair of functions to handle accessing the TArray cache on plMaxNodeData.
//  See AlphaHackLayersNeeded() for details.

int plMaxNode::IGetCachedAlphaHackValue( int iSubMtl )
{
    plMaxNodeData *pDat = GetMaxNodeData();
    if (pDat == nullptr)
        return -1;

    std::vector<int>* cache = pDat->GetAlphaHackLayersCache();
    if (cache == nullptr)
        return -1;

    iSubMtl++;
    if (iSubMtl >= (int)cache->size())
        return -1;

    return (*cache)[ iSubMtl ];
}

void    plMaxNode::ISetCachedAlphaHackValue( int iSubMtl, int value )
{
    plMaxNodeData *pDat = GetMaxNodeData();
    if (pDat == nullptr)
        return;

    std::vector<int>* cache = pDat->GetAlphaHackLayersCache();
    if (cache == nullptr)
    {
        cache = new std::vector<int>;
        pDat->SetAlphaHackLayersCache( cache );
    }

    iSubMtl++;

    if (iSubMtl >= (int)cache->size())
        cache->resize(iSubMtl + 1, -1);

    (*cache)[ iSubMtl ] = value;
}

//// AlphaHackLayersNeeded ///////////////////////////////////////////////////
//  Updated 8.13.02 mcn - Turns out this function is actually very slow, and
//  it also happens to be used a lot in testing instanced objects and whether
//  they really can be instanced or not. Since the return value of this
//  function will be constant after the SetupProperties() pass (and undefined
//  before), we cache the value now after the first time we calculate it.
//  Note: mf said that putting long comments in are good so long as most of
//  them aren't obscenities, so I'm trying to keep the #*$&(*#$ obscenities
//  to a minimum here.

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
bool plMaxNode::VtxAlphaNotAvailable()
{
    if( NonVtxPreshaded() || GetParticleRelated())
        return false;

    return true;
}

bool plMaxNode::NonVtxPreshaded()
{
    if( GetForceMatShade() )
        return false;

    if (GetAvatarSO() != nullptr ||
        hsMaterialConverter::Instance().HasMaterialDiffuseOrOpacityAnimation(this) )
        return false;

    if( GetRunTimeLight() && !hsMaterialConverter::Instance().HasEmissiveLayer(this) )
        return true;

    return (GetLightMapComponent() != nullptr);
}

TriObject* plMaxNode::GetTriObject(bool& deleteIt)
{
    // Get da object
    Object *obj = EvalWorldState(TimeValue(0)).obj;
    if (obj == nullptr)
        return nullptr;

    if( !obj->CanConvertToType(triObjectClassID) )
        return nullptr;

    // Convert to triMesh object
    TriObject   *meshObj = (TriObject *)obj->ConvertToType(TimeValue(0), triObjectClassID);
    if (meshObj == nullptr)
        return nullptr;

    deleteIt = meshObj != obj;

    return meshObj;
}

//// GetNextSoundIdx /////////////////////////////////////////////////////////
//  Starting at 0, returns an incrementing index for each maxNode. Useful for 
//  assigning indices to sound objects attached to the node.

uint32_t  plMaxNode::GetNextSoundIdx()
{
    uint32_t  idx = GetSoundIdxCounter();
    SetSoundIdxCounter( idx + 1 );
    return idx;
}

//// IsPhysical //////////////////////////////////////////////////////////////
//  Fun temp hack function to tell if a maxNode is physical. Useful after
//  preConvert (checks for a physical on the simInterface)

bool    plMaxNode::IsPhysical()
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

    return nullptr;
}

//// FindPageKey /////////////////////////////////////////////////////////////
//  Little helper function. Calls FindKey() in the resManager using the location (page) of this node

plKey   plMaxNode::FindPageKey( uint16_t classIdx, const ST::string &name )
{
    return hsgResMgr::ResMgr()->FindKey( plUoid( GetLocation(), classIdx, name ) );
}

const char *plMaxNode::GetAgeName()
{
    int i;
    for (i = 0; i < NumAttachedComponents(); i++)
    {
        plComponentBase *comp = GetAttachedComponent(i);
        if (comp->ClassID() == PAGEINFO_CID)
            return ((plPageInfoComponent*)comp)->GetAgeName();
    }
    return nullptr;
}

// create a list of keys used by the run-time interface for things like
// determining cursor changes, what kind of object this is, etc.
// we're doing this here because multiple logic triggers can be attached to a 
// single object and tracking down all their run-time counterpart objects (who might
// need a message sent to them) is a huge pain and very ugly.  This will capture anything
// important in a single list.

bool plMaxNode::MakeIfaceReferences(plErrorMsg *pErrMsg, plConvertSettings *settings)
{
    bool ret = true;

    auto dbgNodeName = GetName();
    if (!CanConvert())
        return ret;
    
    size_t count = GetSceneObject()->GetNumModifiers();
    std::vector<plKey> keys;
    // Go through all the modifiers attached to this node's scene object
    // and grab keys for objects who we would need to send interface messages to
    for (size_t i = 0; i < count; i++)
    {
        const plModifier* pMod = GetSceneObject()->GetModifier(i);
        // right now all we care about are these, but I guarentee you we will
        // care about more as the interface gets more complex
        const plPickingDetector* pDet = plPickingDetector::ConvertNoRef(pMod);
        const plLogicModifier* pLog = plLogicModifier::ConvertNoRef(pMod);
        if( pDet )
        {
            for (size_t j = 0; j < pDet->GetNumReceivers(); j++)
                keys.emplace_back(pDet->GetReceiver(j));
        }
        else
        if( pLog )
        {
            keys.emplace_back(pLog->GetKey());
        }
    }
    // if there is anything there, create an 'interface object modifier' which simply stores 
    // the list in a handy form
    if (!keys.empty())
    {
        plInterfaceInfoModifier* pMod = new plInterfaceInfoModifier;
        
        plKey modifierKey = hsgResMgr::ResMgr()->NewKey(ST::string::from_utf8(GetName()), pMod, GetLocation(), GetLoadMask());
        hsgResMgr::ResMgr()->AddViaNotify(modifierKey, new plObjRefMsg(GetSceneObject()->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
        
        for (const plKey& key : keys)
            pMod->AddRefdKey(key);
    }

    return ret;
}
