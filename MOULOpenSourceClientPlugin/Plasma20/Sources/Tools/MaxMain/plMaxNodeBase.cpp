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
#include "plMaxNodeBase.h"
#include "plMaxNodeData.h"

// Max includes
#include "iparamm2.h"
#include "modstack.h"
#include "ISkin.h"
#include "dummy.h"

//To support the new Plasma Light Objs, the classes are included below
#include "../MaxPlasmaLights/plRealTimeLightBase.h"


#include <vector>
#include <algorithm>

#include "GlobalUtility.h"	// Only needed for PLASMA_MAX_CLASSID, fix?

#include "../pnKeyedObject/plUoid.h"
#include "../pnKeyedObject/plKey.h"

#include "../pnModifier/plModifier.h"

#include "../MaxPlasmaMtls/Materials/plDecalMtl.h"

#include "../MaxComponent/plComponentBase.h"

CoreExport void *__cdecl MAX_new(size_t size);
CoreExport void __cdecl MAX_delete(void* mem);

void plMaxNodeBase::SetMaxNodeData(plMaxNodeData * pdat)
{
	const char* dbgNodeName = GetName();

	// If object is a component, don't add node data
	if (IsComponent())
		return;

	AppDataChunk *adc = GetAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaMaxNodeDataChunk);

	// If pointer is nil, remove any data
	if (!pdat)
	{
		if( adc )
		{
			plMaxNodeData *pDataChunk = (plMaxNodeData*)adc->data;
			pDataChunk->DeInit();
		}
		RemoveAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaMaxNodeDataChunk);
		return;		
	}

	if (!adc)
	{
		// Does not exist, create a new one...
		int len = sizeof(plMaxNodeData);
		plMaxNodeData *pDataChunk = (plMaxNodeData *)MAX_new(len);
		memcpy(pDataChunk, pdat, sizeof(*pdat));
		pDataChunk->Init();
		*pDataChunk = *pdat;
		AddAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaMaxNodeDataChunk, len, pDataChunk);
	}
	else
	{
		plMaxNodeData *pDataChunk = (plMaxNodeData*)adc->data;
		// if someone does a GetMaxNodeData, they get a pointer to the data,
		// No need to set it, other wise set the Data chunk from the  MaxNodeData passed in
		if (pDataChunk != pdat)
			*pDataChunk = *pdat;
	}
}

plMaxNodeData *plMaxNodeBase::GetMaxNodeData()
{
	AppDataChunk *adc = GetAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaMaxNodeDataChunk);
	if (adc)
		return (plMaxNodeData*)adc->data;

	return nil;
}

#define GetMD	plMaxNodeData *pMD = GetMaxNodeData(); //hsAssert(pMD,"Missing MaxNodeData");	// Get MaxNode Data

plKey			plMaxNodeBase::GetKey()				{ GetMD; return (pMD) ? pMD->GetKey() : nil;		}
plSceneObject*	plMaxNodeBase::GetSceneObject()		{ GetMD; return (pMD) ? pMD->GetSceneObject() : nil;}
hsBool			plMaxNodeBase::GetForceLocal()			{ GetMD; return (pMD) ? pMD->GetForceLocal() : nil;	}
hsBool			plMaxNodeBase::GetReverseSort()			{ GetMD; return (pMD) ? pMD->GetReverseSort() : nil;}
hsBool			plMaxNodeBase::GetSortAsOpaque()		{ GetMD; return (pMD) ? pMD->GetSortAsOpaque() : nil;}
hsBool			plMaxNodeBase::GetVS()					{ GetMD; return (pMD) ? pMD->GetVS() : nil;}
hsBool			plMaxNodeBase::GetHasWaterHeight()		{ GetMD; return (pMD) ? pMD->GetHasWaterHeight() : nil; }
hsScalar		plMaxNodeBase::GetWaterHeight()			{ GetMD; return (pMD) ? pMD->GetWaterHeight() : nil; }
hsBool			plMaxNodeBase::GetSmoothAll()			{ GetMD; return (pMD) ? pMD->GetSmoothAll() : nil;}
hsBool			plMaxNodeBase::GetForceSortable()		{ GetMD; return (pMD) ? pMD->GetForceSortable() : nil;}
hsBool			plMaxNodeBase::GetConcave()				{ GetMD; return (pMD) ? pMD->GetConcave() : nil;}
hsBool			plMaxNodeBase::GetCalcEdgeLens()		{ GetMD; return (pMD) ? pMD->GetCalcEdgeLens() : nil;}
hsBool			plMaxNodeBase::GetRunTimeLight()		{ GetMD; return (pMD) ? pMD->GetRunTimeLight() : nil;}
hsBool			plMaxNodeBase::GetForceMatShade()		{ GetMD; return (pMD) ? pMD->GetForceMatShade() : nil;}
hsBool			plMaxNodeBase::GetForceVisLOS()			{ GetMD; return (pMD) ? pMD->GetForceVisLOS() : nil;}
hsBool			plMaxNodeBase::GetEnviron()		{ GetMD; return (pMD) ? pMD->GetEnviron() : nil;}
hsBool			plMaxNodeBase::GetEnvironOnly()		{ GetMD; return (pMD) ? pMD->GetEnvironOnly() : nil;}
hsBool			plMaxNodeBase::GetWaterDecEnv()			{ GetMD; return (pMD) ? pMD->GetWaterDecEnv() : nil; }
hsBool			plMaxNodeBase::GetNoPreShade()			{ GetMD; return (pMD) ? pMD->GetNoPreShade() && !pMD->GetForcePreShade() : nil;}
hsBool			plMaxNodeBase::GetForcePreShade()		{ GetMD; return (pMD) ? pMD->GetForcePreShade() : nil;}
plKey			plMaxNodeBase::GetRoomKey()			{ GetMD; return (pMD) ? pMD->GetRoomKey() : nil;	}
hsBool			plMaxNodeBase::GetDrawable()			{ GetMD; return (pMD) ? pMD->GetDrawable() : nil;	}
hsBool			plMaxNodeBase::GetPhysical()			{ GetMD; return (pMD) ? pMD->GetPhysical() : nil;	}
hsBool			plMaxNodeBase::GetItinerant()			{ GetMD; return (pMD) ? pMD->GetItinerant() : nil;	}
hsBool			plMaxNodeBase::GetUnBounded()			{ GetMD; return (pMD) ? pMD->GetUnBounded() : nil;	}
hsBool			plMaxNodeBase::GetDisableNormal()		{ GetMD; return (pMD) ? pMD->GetDisableNormal() : nil;	}
UInt32			plMaxNodeBase::GetDecalLevel()			{ GetMD; return (pMD) ? pMD->GetDecalLevel() : nil;	}
hsBool			plMaxNodeBase::GetMovable()			{ GetMD; return (pMD) ? pMD->GetMovable() : nil;	}
hsBool			plMaxNodeBase::GetIsBarney()			{ GetMD; return (pMD) ? pMD->GetIsBarney() : nil;	}
hsBool			plMaxNodeBase::GetForceShadow()			{ GetMD; return (pMD) ? pMD->GetForceShadow() : nil;	}
hsBool			plMaxNodeBase::GetAlphaTestHigh()		{ GetMD; return (pMD) ? pMD->GetAlphaTestHigh() : nil;	}
hsBool			plMaxNodeBase::GetFilterInherit()			{ GetMD; return (pMD) ? pMD->GetFilterInherit() : nil;	}
hsBool			plMaxNodeBase::GetNoShadow()			{ GetMD; return (pMD) ? pMD->GetNoShadow() : nil;	}
hsBool			plMaxNodeBase::GetNoSpanSort()			{ GetMD; return (pMD) ? pMD->GetNoSpanSort() : nil; }
hsBool			plMaxNodeBase::GetNoSpanReSort()		{ GetMD; return (pMD) ? pMD->GetNoSpanReSort() : nil; }
hsBool			plMaxNodeBase::GetNoFaceSort()			{ GetMD; return (pMD) ? pMD->GetNoFaceSort() : nil; }
hsBool			plMaxNodeBase::GetNoDeferDraw()		{ GetMD; return (pMD) ? pMD->GetNoDeferDraw() : nil; }
hsBool			plMaxNodeBase::GetBlendToFB()		{ GetMD; return (pMD) ? pMD->GetBlendToFB() : nil; }
hsBool			plMaxNodeBase::GetForceMaterialCopy()	{ GetMD; return (pMD) ? pMD->GetForceMaterialCopy() : nil; }
hsBool			plMaxNodeBase::GetInstanced()			{ GetMD; return (pMD) ? pMD->GetInstanced() : nil; }
hsBool			plMaxNodeBase::GetParticleRelated()	{ GetMD; return (pMD) ? pMD->GetParticleRelated() : nil; }
UInt32			plMaxNodeBase::GetSoundIdxCounter()	{ GetMD; return (pMD) ? pMD->GetSoundIdxCounter() : 0; }
plSceneObject*	plMaxNodeBase::GetAvatarSO()			{ GetMD; return (pMD) ? pMD->GetAvatarSO() : nil; }
BOOL			plMaxNodeBase::HasFade()				{ GetMD; return (pMD) ? pMD->HasFade() : false; }
Box3			plMaxNodeBase::GetFade()				{ GetMD; return (pMD) ? pMD->GetFade() : Box3(Point3(0,0,0), Point3(0,0,0)); }
hsBool			plMaxNodeBase::GetDup2Sided()			{ GetMD; return (pMD) ? pMD->GetDup2Sided() : false;}
hsBool			plMaxNodeBase::GetRadiateNorms()		{ GetMD; return (pMD) ? pMD->GetRadiateNorms() : false;}
BOOL			plMaxNodeBase::HasNormalChan()			{ GetMD; return (pMD) ? pMD->HasNormalChan() : false; }
int				plMaxNodeBase::GetNormalChan()			{ GetMD; return (pMD) ? pMD->GetNormalChan() : 0; }
hsBool			plMaxNodeBase::GetIsGUI()				{ GetMD; return (pMD) ? pMD->GetIsGUI() : false; }
plSharedMesh*	plMaxNodeBase::GetSwappableGeom()		{ GetMD; return (pMD) ? pMD->GetSwappableGeom() : nil; }
UInt32			plMaxNodeBase::GetSwappableGeomTarget()		{ GetMD; return (pMD) ? pMD->GetSwappableGeomTarget() : -1; }
plMaxBoneMap*	plMaxNodeBase::GetBoneMap()					{ GetMD; return (pMD) ? pMD->GetBoneMap() : nil; }
hsBool			plMaxNodeBase::GetOverrideHighLevelSDL()	{ GetMD; return (pMD) ? pMD->GetOverrideHighLevelSDL() : false; }
UInt8			plMaxNodeBase::GetAnimCompress()					{ GetMD; return (pMD) ? pMD->GetAnimCompress() : false; }
hsScalar		plMaxNodeBase::GetKeyReduceThreshold()				{ GetMD; return (pMD) ? pMD->GetKeyReduceThreshold() : 0; }
int				plMaxNodeBase::NumRenderDependencies()				{ GetMD; return (pMD) ? pMD->NumRenderDependencies() : 0; }
plMaxNodeBase*	plMaxNodeBase::GetRenderDependency(int i)			{ GetMD; return (pMD) ? pMD->GetRenderDependency(i) : nil; }

int				plMaxNodeBase::NumBones()							{ GetMD; return (pMD) ? pMD->NumBones() : 0;	}
plMaxNodeBase*	plMaxNodeBase::GetBone(int i)						{ GetMD; return (pMD) ? pMD->GetBone(i) : nil;	}


//------------------------------
// Set Data from MaxNodeData
//------------------------------
void			plMaxNodeBase::SetCanConvert(hsBool b)				{ GetMD; pMD->SetCanConvert(b);			}
void			plMaxNodeBase::SetMesh(hsGMesh *p)					{ GetMD; pMD->SetMesh(p);				}		
void			plMaxNodeBase::SetRoomKey(plKey p)					{ GetMD; pMD->SetRoomKey(p);			}
void			plMaxNodeBase::SetDrawable(hsBool b)				{ GetMD; pMD->SetDrawable(b);			}
void			plMaxNodeBase::SetPhysical(hsBool b)				{ GetMD; pMD->SetPhysical(b);			}
//void			plMaxNodeBase::SetItinerant(hsBool b);
void			plMaxNodeBase::SetUnBounded(hsBool b)				{ GetMD; pMD->SetUnBounded(b);		}
void			plMaxNodeBase::SetDisableNormal(hsBool b)			{ GetMD; pMD->SetDisableNormal(b);		}
void			plMaxNodeBase::SetDecalLevel(UInt32 i)				{ GetMD; pMD->SetDecalLevel(i);			}
void			plMaxNodeBase::SetMovable(hsBool b)					{ GetMD; pMD->SetMovable(b); pMD->SetRunTimeLight(b); pMD->SetNoPreShade(b);			}
void			plMaxNodeBase::SetReverseSort(hsBool b)				{ GetMD; pMD->SetReverseSort(b); }
void			plMaxNodeBase::SetSortAsOpaque(hsBool b)			{ GetMD; pMD->SetSortAsOpaque(b); }
void			plMaxNodeBase::SetVS(hsBool b)						{ GetMD; pMD->SetVS(b); }
void			plMaxNodeBase::SetHasWaterHeight(hsBool b)			{ GetMD; pMD->SetHasWaterHeight(b); }
void			plMaxNodeBase::SetWaterHeight(hsScalar h)			{ GetMD; pMD->SetWaterHeight(h); }
void			plMaxNodeBase::SetSmoothAll(hsBool b)				{ GetMD; pMD->SetSmoothAll(b); }
void			plMaxNodeBase::SetForceSortable(hsBool b)			{ GetMD; pMD->SetForceSortable(b); }
void			plMaxNodeBase::SetConcave(hsBool b)					{ GetMD; pMD->SetConcave(b); }
void			plMaxNodeBase::SetCalcEdgeLens(hsBool b)			{ GetMD; pMD->SetCalcEdgeLens(b); }
void			plMaxNodeBase::SetRunTimeLight(hsBool b)			{ GetMD; pMD->SetRunTimeLight(b); }
void			plMaxNodeBase::SetForceMatShade(hsBool b)			{ GetMD; pMD->SetForceMatShade(b); }
void			plMaxNodeBase::SetForceVisLOS(hsBool b)				{ GetMD; pMD->SetForceVisLOS(b); }
void			plMaxNodeBase::SetEnviron(hsBool b)					{ GetMD; pMD->SetEnviron(b); }
void			plMaxNodeBase::SetEnvironOnly(hsBool b)				{ GetMD; pMD->SetEnvironOnly(b); }
void			plMaxNodeBase::SetWaterDecEnv(hsBool b)				{ GetMD; pMD->SetWaterDecEnv(b); }
void			plMaxNodeBase::SetNoPreShade(hsBool b)				{ GetMD; pMD->SetNoPreShade(b);			}
void			plMaxNodeBase::SetForcePreShade(hsBool b)			{ GetMD; pMD->SetForcePreShade(b);		}
void			plMaxNodeBase::SetForceLocal(hsBool b)				{ GetMD; pMD->SetForceLocal(b);			}
void			plMaxNodeBase::SetIsBarney(hsBool b)				{ GetMD; pMD->SetIsBarney(b);			}
void			plMaxNodeBase::SetForceShadow(hsBool b)				{ GetMD; pMD->SetForceShadow(b);		}
void			plMaxNodeBase::SetAlphaTestHigh(hsBool b)			{ GetMD; pMD->SetAlphaTestHigh(b);		}
void			plMaxNodeBase::SetFilterInherit(hsBool b)			{ GetMD; pMD->SetFilterInherit(b);		}
void			plMaxNodeBase::SetNoShadow(hsBool b)				{ GetMD; pMD->SetNoShadow(b);			}
void			plMaxNodeBase::SetNoSpanSort(hsBool b)				{ GetMD; pMD->SetNoSpanSort(b);			}
void			plMaxNodeBase::SetNoSpanReSort(hsBool b)			{ GetMD; pMD->SetNoSpanReSort(b);		}
void			plMaxNodeBase::SetNoFaceSort(hsBool b)				{ GetMD; pMD->SetNoFaceSort(b);			}
void			plMaxNodeBase::SetNoDeferDraw(hsBool b)				{ GetMD; pMD->SetNoDeferDraw(b);		}
void			plMaxNodeBase::SetBlendToFB(hsBool b)				{ GetMD; pMD->SetBlendToFB(b);		}
void			plMaxNodeBase::SetForceMaterialCopy(hsBool b)		{ GetMD; pMD->SetForceMaterialCopy(b);	}
void			plMaxNodeBase::SetInstanced(hsBool b)				{ GetMD; pMD->SetInstanced(b);			}
void			plMaxNodeBase::SetParticleRelated(hsBool b)			{ GetMD; pMD->SetParticleRelated(b);	}
void			plMaxNodeBase::SetSoundIdxCounter(UInt32 ctr)		{ GetMD; pMD->SetSoundIdxCounter(ctr);	}
void			plMaxNodeBase::SetAvatarSO(plSceneObject *so)		{ GetMD; pMD->SetAvatarSO(so);			}
void			plMaxNodeBase::SetFade(const Box3& b)				{ GetMD; pMD->SetFade(b);				}
void			plMaxNodeBase::SetDup2Sided(hsBool b)				{ GetMD; pMD->SetDup2Sided(b);			}
void			plMaxNodeBase::SetRadiateNorms(hsBool b)			{ GetMD; pMD->SetRadiateNorms(b);		}
void			plMaxNodeBase::SetNormalChan(int n)					{ GetMD; pMD->SetNormalChan(n);			}
void			plMaxNodeBase::SetIsGUI(hsBool b)					{ GetMD; pMD->SetIsGUI(b);			}
void			plMaxNodeBase::SetSwappableGeom(plSharedMesh *sm)	{ GetMD; pMD->SetSwappableGeom(sm);	}
void			plMaxNodeBase::SetSwappableGeomTarget(UInt32 id)	{ GetMD; pMD->SetSwappableGeomTarget(id);	}
void			plMaxNodeBase::SetBoneMap(plMaxBoneMap *bones)		{ GetMD; pMD->SetBoneMap(bones);	}
void			plMaxNodeBase::SetOverrideHighLevelSDL(hsBool b)	{ GetMD; pMD->SetOverrideHighLevelSDL(b); }
void			plMaxNodeBase::SetAnimCompress(UInt8 v)				{ GetMD; pMD->SetAnimCompress(v); }
void			plMaxNodeBase::SetKeyReduceThreshold(hsScalar v)	{ GetMD; pMD->SetKeyReduceThreshold(v); }
void			plMaxNodeBase::ClearRenderDependencies()			{ GetMD; pMD->ClearRenderDependencies(); }

void			plMaxNodeBase::AddBone(plMaxNodeBase* m)			{ GetMD; if(pMD) pMD->AddBone(m);		}
void			plMaxNodeBase::ClearBones()							{ GetMD; if(pMD) pMD->ClearBones();		}

plLocation plMaxNodeBase::GetLocation()
{
	plKey rmKey= GetRoomKey();
	plLocation loc;
	loc.Invalidate();
	if (rmKey)
		loc = rmKey->GetUoid().GetLocation();
	return loc;
}

hsBool plMaxNodeBase::GetDirty(UInt8 i)
{
	UInt8 *dirty = IGetSceneViewerChunk();
	return *dirty & i;
}

void plMaxNodeBase::SetDirty(UInt8 i, hsBool b)
{
	UInt8 *dirty = IGetSceneViewerChunk();

	if (b)
		*dirty |= i;
	else
		*dirty &= ~i;
}

hsBool plMaxNodeBase::HasLoadMask()
{
	GetMD;
	return pMD->HasLoadMask();
}

plLoadMask plMaxNodeBase::GetLoadMask()
{
	GetMD;
	return pMD->GetLoadMask();
}

void plMaxNodeBase::AddLoadMask(const plLoadMask& m)
{
	GetMD;
	pMD->AddLoadMask(m);
}

hsBool plMaxNodeBase::RenderDependsOn(plMaxNodeBase* m)
{
	if( m == this )
		return true;

	int i;
	for( i = 0; i < NumRenderDependencies(); i++ )
	{
		if( GetRenderDependency(i)->RenderDependsOn(m) )
			return true;
	}
	return false;
}

hsBool plMaxNodeBase::AddRenderDependency(plMaxNodeBase* m)	
{ 
	if( m->RenderDependsOn(this) )
		return false;
	GetMD; 
	pMD->AddRenderDependency(m); 
	return true;
}

UInt8 *plMaxNodeBase::IGetSceneViewerChunk()
{
	UInt8 *SVChunk = nil;

	AppDataChunk *adc = GetAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaSceneViewerChunk);
	if (adc)
		SVChunk = (UInt8*)adc->data;
	else
	{
		// Does not exist, create a new one...
		SVChunk = (UInt8*)MAX_new(1);
		*SVChunk = 0;
		AddAppDataChunk(PLASMA_MAX_CLASSID, GUP_CLASS_ID, kPlasmaSceneViewerChunk, 1, SVChunk);
	}

	hsAssert(SVChunk, "SceneViewer chunk not found or created");
	return SVChunk;
}

hsBool plMaxNodeBase::CanConvert(bool recalculate)
{
	// Try and find a cached return value
	plMaxNodeData *md = GetMaxNodeData();
	if (md && !recalculate)
		return md->CanConvert();

	if (UserPropExists("IGNORE"))
		return false;

	Object *obj = EvalWorldState(0/*hsConverterUtils::Instance().GetTime(GetInterface())*/).obj;
	if (obj)
	{
		if 	(  obj->CanConvertToType(triObjectClassID)		// MeshObjs are accepted here
			|| obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0)	// Dummy boxes are accepted here
			|| obj->SuperClassID() == CAMERA_CLASS_ID		// All Camera types are accepted here
			|| obj->ClassID() == Class_ID(UTILITY_CLASS_ID, 0)		// All Camera targets are accepted here
			|| (  obj->ClassID() ==  RTOMNI_LIGHT_CLASSID
				|| obj->ClassID() == RTSPOT_LIGHT_CLASSID
				|| obj->ClassID() == RTDIR_LIGHT_CLASSID 
				|| obj->ClassID() == RTPDIR_LIGHT_CLASSID )
			|| (  obj->SuperClassID() == LIGHT_CLASS_ID		// All run time lights are accepted here
			   && UserPropExists("RunTimeLight"))

			|| IsGroupMember()								// Group objects are accepted here
			)
		return true;
	}
	return false;
}

hsBool plMaxNodeBase::IsTMAnimated()
{
	Control* tmControl = GetTMController();
	return (tmControl && tmControl->IsAnimated());
}

//// IsTMAnimatedRecur  - test recursively up the chain ///////////////////////////////////////////////////////////////
hsBool plMaxNodeBase::IsTMAnimatedRecur()
{
	const char* dbgNodeName = GetName();
	hsBool		shouldBe = false;

	if( !CanConvert() )
		return false;
	if( IsTMAnimated() )
		return true;

	return ((plMaxNodeBase*)GetParentNode())->IsTMAnimatedRecur();
}

//// IsMovable ///////////////////////////////////////////////////////////////
//	Returns whether this node is "animated" (i.e. could move at runtime)
hsBool plMaxNodeBase::IsMovable()
{
	const char* dbgNodeName = GetName();
	hsBool		shouldBe = false;


	if( !CanConvert() )
		return false;

	if( GetMovable() )
		return true;

	if( GetItinerant() )
		shouldBe = true;
	else if( FindSkinModifier() )
		shouldBe = true;

	// Moved this to plAnimComponent (so GetMovable() will reveal it)
	/*
	else if( IsTMAnimated() )
		shouldBe = true;
	*/
	if( shouldBe )
	{
		SetMovable( true );
		return true;
	}

	return ((plMaxNodeBase*)GetParentNode())->IsMovable();
}

// Recursively set so we don't have to recursively check.
void plMaxNodeBase::SetItinerant(hsBool b)
{ 
	const char* dbgNodeName = GetName();

	if( !CanConvert() )
		return;

	GetMD; 
	pMD->SetItinerant(b);			

	int i;
	for( i = 0; i < NumChildren(); i++ )
	{
		((plMaxNodeBase*)GetChildNode(i))->SetItinerant(b);
	}
}

//// FindSkinModifier ///////////////////////////////////////////////////////
//	Given an INode, gets the ISkin object of that node, or nil if there is
//	none. Taken from the Max4 SDK, ISkin.h
ISkin* plMaxNodeBase::FindSkinModifier()
{
	int	modStackIndex;

	// Get object from node. Abort if no object.
	Object *pObj = GetObjectRef();
	if( pObj == nil )
		return nil;

	// Is derived object ?
	while( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID )
	{
		IDerivedObject *pDerObj = (IDerivedObject *)pObj;

		// Iterate over all entries of the modifier stack.
		for( modStackIndex = 0; modStackIndex < pDerObj->NumModifiers(); modStackIndex++ )
		{
			// Get current modifier.
			Modifier *mod = pDerObj->GetModifier( modStackIndex );

			// Is this Skin ?
			if( mod->ClassID() == SKIN_CLASSID )
			{
				ISkin* skin = (ISkin*)mod->GetInterface(I_SKIN);
				if( skin->GetNumBones() > 0 )
					return skin;
			}
		}
		pObj = pDerObj->GetObjRef();
	}

	// Not found.
	return nil;
}

bool plMaxNodeBase::IsXRef()
{
	// Is this an XRef'd object?
	Object *obj = GetObjectRef();
	if (obj->SuperClassID() == SYSTEM_CLASS_ID && obj->ClassID() == Class_ID(XREFOBJ_CLASS_ID,0))
		return true;

	//
	// Is this part of an XRef'd scene?
	//
	// Walk up to our root node
	INode *root = GetParentNode();
	while (!root->IsRootNode())
		root = root->GetParentNode();
	// If our root isn't the main root, we're in an XRef'd scene.
	if (root != GetCOREInterface()->GetRootNode())
		return true;

	return false;
}

hsBool plMaxNodeBase::IsComponent(Object *obj)
{
	if (!obj)
		obj = GetObjectRef();

	if (obj && obj->CanConvertToType(COMPONENT_CLASSID))
		return true;

	return false;
}

hsBool plMaxNodeBase::IsExternComponent(Object *obj)
{
	if (!obj)
		obj = GetObjectRef();

	if (obj && obj->CanConvertToType(EXT_COMPONENT_CLASSID))
		return true;

	return false;
}

plComponentBase *plMaxNodeBase::ConvertToComponent()
{
	if (IsComponent())
		return (plComponentBase*)GetObjectRef();

	return nil;
}

// There isn't an easy way to determine if there are any components attached to a node.
// This method goes through the reflist of a node and backtracks up each ref (using
// IRefMakerToComponent) to determine if it is a component.  There are cases though
// where a component can show up as two or more refs to a node, when it has a legitimate
// target ref and some other ref like a proxy object.  To catch this case we maintain a
// list of the components found so far and ensure there aren't any duplicates.  Maybe it
// would be easier to just go through every component in the scene and check their target
// lists... -Colin
UInt32 plMaxNodeBase::NumAttachedComponents(bool all)
{
	UInt32 numComponents = 0;
	std::vector<plComponentBase*> comps;

	// Go through this item's reflist, looking for components
	RefList &refList = GetRefList();
	RefListItem *item = refList.FirstItem();
	while (item)
	{
		plComponentBase *comp = IRefMakerToComponent(item->maker, all);
		if (comp && std::find(comps.begin(), comps.end(), comp) == comps.end())
		{
			comps.push_back(comp);
			numComponents++;
		}

		item = item->next;
	}

	return numComponents;
}

plComponentBase *plMaxNodeBase::GetAttachedComponent(UInt32 i, bool all)
{
	UInt32 numComponents = 0;
	std::vector<plComponentBase*> comps;

	// Go through this item's reflist, looking for components
	RefList &refList = GetRefList();
	RefListItem *item = refList.FirstItem();
	while (item)
	{
		plComponentBase *comp = IRefMakerToComponent(item->maker, all);
		if (comp && std::find(comps.begin(), comps.end(), comp) == comps.end())
		{
			if (numComponents == i)
				return comp;

			comps.push_back(comp);
			numComponents++;
		}

		item = item->next;
	}

	return nil;
}

plComponentBase *plMaxNodeBase::IRefMakerToComponent(ReferenceMaker *maker, bool all)
{
	if (!maker)
		return nil;

	// Is the refmaker a paramblock?  If so, it may be the
	// targets block of a component
	if (maker->SuperClassID() == PARAMETER_BLOCK2_CLASS_ID)
	{
		IParamBlock2 *pb = (IParamBlock2*)maker;
		ReferenceMaker *pbowner = pb->GetOwner();

		// Is the owner of the paramblock a helper object (component superclass)?
		if (pbowner && pbowner->SuperClassID() == HELPER_CLASS_ID)
		{
			Object *obj = (Object*)pbowner;
			// Is the owner of the paramblock a component?
			if (IsComponent(obj))
			{
				plComponentBase *comp = (plComponentBase*)obj;

				if (!all)
				{
					// Does this component actually ref us? (A component can have other
					// refs to a node, like a proxy object, so we want to make sure this
					// node is actually in the target list.)
					for (UInt32 i = 0; i < comp->NumTargets(); i++)
					{
						if (comp->GetTarget(i) == this)
							return comp;
					}
				}
				else
					return comp;
			}
		}
	}

	return nil;
}

hsBool plMaxNodeBase::IRenderLevelSet(hsBool forBlend)
{
	plMaxNodeData* md = GetMaxNodeData();
	if( md )
		return forBlend ? md->BlendLevelSet() : md->OpaqueLevelSet();
	return false;
}

void plMaxNodeBase::ISetRenderLevel(const plRenderLevel& l, hsBool forBlend)
{
	plMaxNodeData* md = GetMaxNodeData();
	if( md )
	{
		if( forBlend )
			md->SetBlendLevel(l);
		else
			md->SetOpaqueLevel(l);
	}
}

const plRenderLevel& plMaxNodeBase::IGetRenderLevel(hsBool forBlend)
{
	plMaxNodeData* md = GetMaxNodeData();
	if( !md )
	{
		static plRenderLevel defRenderLevel;
		return defRenderLevel;
	}
	return forBlend ? md->GetBlendLevel() : md->GetOpaqueLevel();
}

UInt32 plMaxNodeBase::IGetMajorRenderLevel(hsBool forBlend)
{
	if( GetBlendToFB() )
		return plRenderLevel::kFBMajorLevel;

	if( GetNoDeferDraw() || GetAvatarSO() )
		forBlend = false;

	int numDep = NumRenderDependencies();
	if( !numDep )
		return forBlend ? plRenderLevel::kBlendRendMajorLevel : plRenderLevel::kDefRendMajorLevel;

	int iMaxDep = 0;
	const char* dbgNodeName = GetName();

	int i;
	for( i = 0; i < numDep; i++ )
	{
		plMaxNodeBase* dep = GetRenderDependency(i);
		if( dep )
		{
			const char* depNodeName = dep->GetName();
			int iDep = GetRenderDependency(i)->GetRenderLevel(forBlend).Major();
			if( iDep > iMaxDep )
				iMaxDep = iDep;
		}
	}

	return iMaxDep;
}

UInt32 plMaxNodeBase::IGetMinorRenderLevel(hsBool forBlend)
{
	if( GetAvatarSO() )
		return plRenderLevel::kAvatarRendMinorLevel;

	int numDep = NumRenderDependencies();
	if( !numDep )
		return plRenderLevel::kDefRendMinorLevel;

	int iMaxDep = 0;

	const char* dbgNodeName = GetName();

	int i;
	for( i = 0; i < numDep; i++ )
	{
		plMaxNodeBase* dep = GetRenderDependency(i);
		if( dep )
		{
			const char* depNodeName = dep->GetName();
			int iDep = GetRenderDependency(i)->GetRenderLevel(forBlend).Minor();
			if( iDep > iMaxDep )
				iMaxDep = iDep;
		}
	}

	return iMaxDep + 4;
}

plRenderLevel plMaxNodeBase::ICalcRenderLevel(hsBool forBlend)
{
	if( GetBlendToFB() )
		return plRenderLevel::kFBMajorLevel;

	if( GetAvatarSO() )
		return plRenderLevel(plRenderLevel::kOpaqueMajorLevel, plRenderLevel::kAvatarRendMinorLevel);

	if( GetNoDeferDraw() )
		forBlend = false;

	int numDep = NumRenderDependencies();
	if( !numDep )
		return forBlend 
			? plRenderLevel(plRenderLevel::kBlendRendMajorLevel, plRenderLevel::kDefRendMinorLevel)
			: plRenderLevel(plRenderLevel::kDefRendMajorLevel, plRenderLevel::kDefRendMinorLevel);

	plRenderLevel maxLevel(plRenderLevel::kFBMajorLevel, plRenderLevel::kDefRendMinorLevel);

	const char* dbgNodeName = GetName();

	int i;
	for( i = 0; i < numDep; i++ )
	{
		plMaxNodeBase* dep = GetRenderDependency(i);
		if( dep )
		{
			const char* depNodeName = dep->GetName();

			plRenderLevel depLev = GetRenderDependency(i)->GetRenderLevel(forBlend);
			if( depLev > maxLevel )
				maxLevel = depLev;
		}
	}
	maxLevel.Set(maxLevel.Level() + 4);

	return maxLevel;
}

const plRenderLevel& plMaxNodeBase::GetRenderLevel(hsBool forBlend)
{
	if( !CanConvert() )
	{
		static plRenderLevel retVal;
		return retVal;
	}

	if( !IRenderLevelSet(forBlend) )
	{
#if 0

		UInt32 major = IGetMajorRenderLevel(forBlend);
		UInt32 minor = IGetMinorRenderLevel(forBlend);

		ISetRenderLevel(plRenderLevel(major, minor), forBlend);
#else
		ISetRenderLevel(ICalcRenderLevel(forBlend), forBlend);
#endif
	}

	return IGetRenderLevel(forBlend);
}

BOOL plMaxNodeBase::GetGeoDice(int& maxFaces, float& maxSize, int& minFaces)
{
	plMaxNodeData* md = GetMaxNodeData();
	if( md && md->GetGeoDice() )
	{
		maxFaces = md->GetGeoDiceMaxFaces();
		maxSize = md->GetGeoDiceMaxSize();
		minFaces = md->GetGeoDiceMinFaces();
		return true;
	}
	maxFaces = 0;
	maxSize = 0;
	minFaces = 0;
	return false;
}

void plMaxNodeBase::SetGeoDice(BOOL on, int maxFaces, float maxSize, int minFaces)
{
	plMaxNodeData* md = GetMaxNodeData();
	if( md )
	{
		md->SetGeoDice(on);
		md->SetGeoDiceMaxFaces(maxFaces);
		md->SetGeoDiceMaxSize(maxSize);
		md->SetGeoDiceMinFaces(minFaces);
	}
}

hsBool plMaxNodeBase::Contains(const Point3& worldPt)
{
	TimeValue currTime = 0;//hsConverterUtils::Instance().GetTime(GetInterface());
	Object *obj = EvalWorldState(currTime).obj;
	if( !obj )
		return false;

	Matrix3 l2w = GetObjectTM(currTime);
	Matrix3 w2l = Inverse(l2w);
	Point3 pt = w2l * worldPt;

	if( obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
	{
		DummyObject* dummy = (DummyObject*)obj;
		Box3 bnd = dummy->GetBox();
		return bnd.Contains(pt);
	}
	if( obj->CanConvertToType(triObjectClassID) )
	{
		TriObject	*meshObj = (TriObject *)obj->ConvertToType(currTime, triObjectClassID);
		if( !meshObj )
			return false;

		Mesh& mesh = meshObj->mesh;
		Box3 bnd = mesh.getBoundingBox();
		if( !bnd.Contains(pt) )
		{
			if( meshObj != obj )
				meshObj->DeleteThis();
			return false;
		}

		hsBool retVal = true;
		int i;
		for( i = 0; i < mesh.getNumFaces(); i++ )
		{
			Face& face = mesh.faces[i];

			Point3 p0 = mesh.verts[face.v[0]];
			Point3 p1 = mesh.verts[face.v[1]];
			Point3 p2 = mesh.verts[face.v[2]];

			Point3 n = CrossProd(p1 - p0, p2 - p0);

			if( DotProd(pt, n) > DotProd(p0, n) )
			{
				retVal = false;
				break;
			}
		}

		if( meshObj != obj )
			meshObj->DeleteThis();

		return retVal;
	}

	// If we can't figure out what it is, the point isn't inside it.
	return false;
}

hsBool plMaxNodeBase::Contains(const Box3& bnd, const Matrix3& l2w)
{
	int i;
	for( i = 0; i < 8; i++ )
	{
		if( !Contains(l2w * bnd[i]) )
			return false;
	}
	return true;
}

hsScalar plMaxNodeBase::BoxVolume(const Box3& bnd, const Matrix3& l2w)
{
	Point3 corner = l2w * bnd[0]; // min, min, min
	float len[3];

	len[0] = Length((l2w * bnd[1]) - corner); // max, min, min
	len[1] = Length((l2w * bnd[2]) - corner); // min, max, min
	len[2] = Length((l2w * bnd[4]) - corner); // min, min, max

	return len[0] * len[1] * len[2];
}

hsScalar plMaxNodeBase::RegionPriority()
{
	TimeValue currTime = 0;//hsConverterUtils::Instance().GetTime(GetInterface());
	Object *obj = EvalWorldState(currTime).obj;
	if( !obj )
		return 0;

	Matrix3 l2w = GetObjectTM(currTime);

	if( obj->ClassID() == Class_ID(DUMMY_CLASS_ID,0) )
	{
		DummyObject* dummy = (DummyObject*)obj;
		Box3 bnd = dummy->GetBox();

		return BoxVolume(bnd, l2w);
	}

	if( obj->CanConvertToType(triObjectClassID) )
	{
		TriObject	*meshObj = (TriObject *)obj->ConvertToType(currTime, triObjectClassID);
		if( !meshObj )
			return 0;

		Mesh& mesh = meshObj->mesh;
		Box3 bnd = mesh.getBoundingBox();
		
		if( meshObj != obj )
			meshObj->DeleteThis();

		return BoxVolume(bnd, l2w);
	}

	// Don't know how to interpret other, it's not contained.
	return 0;
}

hsMatrix44 plMaxNodeBase::GetLocalToParent44(TimeValue t)
{
	Matrix3 m3 = GetLocalToParent(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetParentToLocal44(TimeValue t)
{
	Matrix3 m3 = GetParentToLocal(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetLocalToWorld44(TimeValue t)
{
	Matrix3 m3 = GetLocalToWorld(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetWorldToLocal44(TimeValue t)
{
	Matrix3 m3 = GetWorldToLocal(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetOTM44(TimeValue t)
{
	Matrix3 m3 = GetOTM(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetVertToLocal44(TimeValue t)
{
	Matrix3 m3 = GetVertToLocal(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetLocalToVert44(TimeValue t)
{
	Matrix3 m3 = GetLocalToVert(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetLocalToOBB44(TimeValue t)
{
	Matrix3 m3 = GetLocalToOBB(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

hsMatrix44 plMaxNodeBase::GetOBBToLocal44(TimeValue t)
{
	Matrix3 m3 = GetOBBToLocal(t);
	hsMatrix44 m44 = Matrix3ToMatrix44(m3);
	return m44;
}

Matrix3 plMaxNodeBase::GetParentToWorld(TimeValue t)
{
	return Inverse(GetWorldToParent(t));
}

Matrix3 plMaxNodeBase::GetWorldToParent(TimeValue t)
{
	// This may look back-ass-ward, but that's only because it
	// is. If we've got inheritance filtering on, then our localtoworld
	// is no longer parentl2w * l2p, because we'll be ignoring
	// some of our parent's transform. More precisely, we'll be
	// clobbering parts of the product of our parent's current transform
	// and our current local to parent. So we're going to calculate
	// a parent to world transform here that would get us to the
	// right point and orientation in space, even though it has
	// little or nothing to do with our parent's real transform.
	// Note that we only go through this charade if we've got
	// filtering of inheritance active for this node.
	plMaxNodeBase* parent = (plMaxNodeBase*)GetParentNode();
	if( !GetFilterInherit() )
		return parent->GetWorldToLocal(t);

	// l2w = l2p * parentL2W
	// l2w * parentW2L = l2p
	// parentW2L = w2l * l2p
	Point3 pos;
	float rot[4];
	ScaleValue scl;
	Interval posInv;
	Interval rotInv;
	Interval sclInv;

	Matrix3Indirect parentMatrix(parent->GetNodeTM(t));

	TMComponentsArg cmpts(&pos, &posInv, rot, &rotInv, &scl, &sclInv);
	GetTMController()->GetLocalTMComponents(t, cmpts, parentMatrix);

	Quat q;
	if( cmpts.rotRep == TMComponentsArg::RotationRep::kQuat )
		q = Quat(rot);
	else
		EulerToQuat(rot, q, cmpts.rotRep);

	Matrix3 l2p(true);
	l2p.PreTranslate(pos);
	PreRotateMatrix(l2p, q);
	l2p.PreScale(scl.s);
	PreRotateMatrix(l2p, scl.q);

	Matrix3 w2l = GetWorldToLocal(t);

	return w2l * l2p;
}

Matrix3 plMaxNodeBase::GetLocalToParent(TimeValue t)
{
	// l2w = l2p * parentL2W
	// l2w * Inverse(parentL2W) = l2p
	// l2w * parentW2L = l2p
	Matrix3 l2w = GetLocalToWorld(t);
	Matrix3 w2p(true);
	if( !GetParentNode()->IsRootNode() )
		w2p = GetWorldToParent(t);

	Matrix3 l2p = l2w * w2p;
	return l2p;
}

Matrix3 plMaxNodeBase::GetParentToLocal(TimeValue t)
{
	Matrix3 loc2Par = GetLocalToParent(t);
	Matrix3 par2Loc = Inverse(loc2Par);
	return par2Loc;
}

Matrix3 plMaxNodeBase::GetLocalToWorld(TimeValue t)
{
	if( GetForceLocal() )
	{
		// v2l * l2w = objectTM
		// l2w = Inverse(v2l) * objectTM;
		// l2w = l2v * objectTM
		Matrix3 objectTM = GetObjectTM(t);
		Matrix3 l2w = GetLocalToVert(t) * objectTM ;
		return l2w;
	}
	if( !GetParentNode()->IsRootNode() )
		return GetParentToWorld(t);

	return Matrix3(true);
}

Matrix3 plMaxNodeBase::GetWorldToLocal(TimeValue t)
{
	Matrix3 l2w = GetLocalToWorld(t);
	Matrix3 w2l = Inverse(l2w);
	return w2l;
}

Matrix3 plMaxNodeBase::GetOTM(TimeValue t)
{
	// objectTM = otm * nodeTM
	// otm = objectTM * Inverse(nodeTM)
	Matrix3 objectTM = GetObjectTM(t);
	Matrix3 nodeTM = GetNodeTM(t);
	Matrix3 otm = objectTM * Inverse(nodeTM);
	otm.ValidateFlags();
	return otm;
}

Matrix3 plMaxNodeBase::GetVertToLocal(TimeValue t)
{
	Matrix3		objectTM;

	//
	// If animated or we want forced into local space
	// still return OTM to fold into vertices
	//
	if( GetForceLocal() )
	{
		return GetOTM(t);
	}

	// otherwise flatten our local to parent into the verts
	// note that our parent may have flattened their l2p into
	// their v2l as well.
	// so
	// objectTM = v2l * l2p * parentL2W
	// objectTM * Inverse(parentL2W) = v2l * l2p
	// objectTM * parentW2L = v2l (w/ l2p folded in)
	// 
	// Objects transformation ObjectTM = OTM * nodeTM 
	objectTM = GetObjectTM(t);

	Matrix3 w2p(true);
	if( !GetParentNode()->IsRootNode() )
		w2p = GetWorldToParent(t);

	Matrix3 v2l = objectTM * w2p;

	return v2l;
}

Matrix3 plMaxNodeBase::GetLocalToVert(TimeValue t)
{
	Matrix3 v2l = GetVertToLocal(t);
	Matrix3 l2v = Inverse(v2l);
	return l2v;
}

Matrix3 plMaxNodeBase::GetLocalToOBB(TimeValue t)
{
	return GetLocalToVert(t) * GetOTM(t);
}

Matrix3 plMaxNodeBase::GetOBBToLocal(TimeValue t)
{
	return Inverse(GetOTM(t)) * GetVertToLocal(t);
}

hsMatrix44 plMaxNodeBase::Matrix3ToMatrix44(const Matrix3& m3)
{
    const MRow* m = m3.GetAddr();

    hsMatrix44 m44;
    m44.Reset();
    m44.fMap[0][0] = m[0][0];
    m44.fMap[0][1] = m[1][0];
    m44.fMap[0][2] = m[2][0];
    m44.fMap[0][3] = m[3][0];
    
    m44.fMap[1][0] = m[0][1];
    m44.fMap[1][1] = m[1][1];
    m44.fMap[1][2] = m[2][1];
    m44.fMap[1][3] = m[3][1];
    
    m44.fMap[2][0] = m[0][2];
    m44.fMap[2][1] = m[1][2];
    m44.fMap[2][2] = m[2][2];
    m44.fMap[2][3] = m[3][2];
    
	m44.IsIdentity();

	return m44;
}

Matrix3 plMaxNodeBase::Matrix44ToMatrix3(const hsMatrix44& m44)
{
	Matrix3 m3;

    MRow* m = m3.GetAddr();

	m[0][0] = m44.fMap[0][0];
	m[1][0] = m44.fMap[0][1];
	m[2][0] = m44.fMap[0][2];
	m[3][0] = m44.fMap[0][3];

	m[0][1] = m44.fMap[1][0];
	m[1][1] = m44.fMap[1][1];
	m[2][1] = m44.fMap[1][2];
	m[3][1] = m44.fMap[1][3];

	m[0][2] = m44.fMap[2][0];
	m[1][2] = m44.fMap[2][1];
	m[2][2] = m44.fMap[2][2];
	m[3][2] = m44.fMap[2][3];

	m3.ValidateFlags();

	return m3;
}
