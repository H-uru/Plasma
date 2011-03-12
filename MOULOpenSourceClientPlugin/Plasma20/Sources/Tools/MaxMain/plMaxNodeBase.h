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
#ifndef plMaxNodeBase_inc
#define plMaxNodeBase_inc

#include "hsTypes.h"
#include "hsTemplates.h"
#include "max.h"
#include "hsMatrix44.h"
#include "hsColorRGBA.h"
#include "../pnKeyedObject/plKey.h"
#include "plLoadMask.h"

class plLocation;
class plSceneObject;
class plModifier;
class plComponentBase;
class ISkin;
class plMaxNodeData;
class hsGMesh;
class plRenderLevel;
class plGeometrySpan;
class plSharedMesh;
class plMaxBoneMap;

//-------------------------------------------
// plMaxNodeBase
//-------------------------------------------
// CAREFUL! This class is different, it is derived from Max's INode (as you can see)
// But we can only add (NON Virtual) functions to plMaxNodeBase directly
// If you want some new Data members, you can add them by adding to the class
// plMaxNodeData  This data is stored in each INode through some mechanisms supplied
// It would be nice of you to add GetFunctions for each new data member you add (see below)
//--------------------------------------------
// NOTE: an INode can be cast to a plMaxNodeBase, but currently it is the MakeSceneObject Pass which
// Adds the plMaxNodeData to the Node
//
// This class should only reference classes that are in the nucleus.  Anything
// that needs more should go into plMaxNode
class plMaxNodeBase : public INode
{
public:
	plMaxNodeData *GetMaxNodeData();	// perhaps with full getters and Setters, we can make this protected
	void SetMaxNodeData(plMaxNodeData * pDat);

	//------------------------------
	// Get Data from MaxNodeData
	//------------------------------
	// If recalculate is true the cached value is ignored. (Useful in the SceneViewer)
	hsBool			CanConvert(bool recalculate=false);
	plLocation		GetLocation();
	plKey			GetKey();
	plSceneObject*	GetSceneObject();
	hsBool			GetForceLocal();
	hsBool			GetReverseSort();
	hsBool			GetSortAsOpaque();
	hsBool			GetRunTimeLight();
	hsBool			GetForceMatShade();
	hsBool			GetForceVisLOS();
	hsBool			GetEnviron();
	hsBool			GetEnvironOnly();
	hsBool			GetWaterDecEnv();
	hsBool			GetVS();
	hsBool			GetHasWaterHeight();
	hsScalar		GetWaterHeight();
	hsBool			GetSmoothAll();
	hsBool			GetForceSortable();
	hsBool			GetConcave();
	hsBool			GetCalcEdgeLens();
	hsBool			GetNoPreShade();
	hsBool			GetForcePreShade();
	plKey			GetRoomKey();
	hsBool			GetDrawable();
	hsBool			GetPhysical();
	hsBool			GetItinerant();
	hsBool			GetUnBounded();
	hsBool			GetDisableNormal();
	UInt32			GetDecalLevel();
	hsBool			GetMovable();
	hsBool			GetNoShadow();
	hsBool			GetForceShadow();
	hsBool			GetAlphaTestHigh();
	hsBool			GetFilterInherit();
	hsBool			GetIsBarney();
	hsBool			GetNoSpanSort();
	hsBool			GetNoSpanReSort();
	hsBool			GetNoFaceSort();
	hsBool			GetNoDeferDraw();
	hsBool			GetBlendToFB();
	hsBool			GetForceMaterialCopy();
	hsBool			GetInstanced();
	hsBool			GetParticleRelated();
	UInt32			GetSoundIdxCounter();
	plSceneObject*	GetAvatarSO();
	BOOL			HasFade();
	Box3			GetFade();
	hsBool			GetDup2Sided();
	hsBool			GetRadiateNorms();
	BOOL			HasNormalChan();
	int				GetNormalChan();
	BOOL			GetGeoDice(int& maxFaces, float& maxSize, int& minFaces);
	hsBool			GetIsGUI();
	plSharedMesh*	GetSwappableGeom();
	UInt32			GetSwappableGeomTarget();
	plMaxBoneMap*	GetBoneMap();
	hsBool			GetOverrideHighLevelSDL();
	UInt8			GetAnimCompress();
	hsScalar		GetKeyReduceThreshold();
	int				NumRenderDependencies();
	plMaxNodeBase*	GetRenderDependency(int i);

	int				NumBones();
	plMaxNodeBase*	GetBone(int i);
	
	//------------------------------
	// Set Data from MaxNodeData
	//------------------------------
	void			SetCanConvert(hsBool b);
	void			SetMesh(hsGMesh *p);
	void			SetRoomKey(plKey p);
	void			SetDrawable(hsBool b);
	void			SetPhysical(hsBool b);
	void			SetItinerant(hsBool b);
	void			SetUnBounded(hsBool b);
	void			SetDisableNormal(hsBool b);
	void			SetDecalLevel(UInt32 i);
	void			SetMovable(hsBool b);
	void			SetNoPreShade(hsBool b);
	void			SetForcePreShade(hsBool b);
	void			SetReverseSort(hsBool b);
	void			SetSortAsOpaque(hsBool b);
	void			SetVS(hsBool b);
	void			SetHasWaterHeight(hsBool b);
	void			SetWaterHeight(hsScalar h);
	void			SetSmoothAll(hsBool b);
	void			SetForceSortable(hsBool b);
	void			SetConcave(hsBool b);
	void			SetCalcEdgeLens(hsBool b);
	void			SetRunTimeLight(hsBool b);
	void			SetForceMatShade(hsBool b);
	void			SetForceVisLOS(hsBool b);
	void			SetEnviron(hsBool b);
	void			SetEnvironOnly(hsBool b);
	void			SetWaterDecEnv(hsBool b);
	void			SetForceLocal(hsBool b);
	void			SetIsBarney(hsBool b);
	void			SetForceShadow(hsBool b);
	void			SetAlphaTestHigh(hsBool b);
	void			SetFilterInherit(hsBool b);
	void			SetNoShadow(hsBool b);
	void			SetNoSpanSort(hsBool b);
	void			SetNoSpanReSort(hsBool b);
	void			SetNoFaceSort(hsBool b);
	void			SetNoDeferDraw(hsBool b);
	void			SetBlendToFB(hsBool b);
	void			SetForceMaterialCopy(hsBool b);
	void			SetInstanced(hsBool b);
	void			SetParticleRelated(hsBool b);
	void			SetSoundIdxCounter(UInt32 ctr);
	void			SetAvatarSO(plSceneObject *so);
	void			SetFade(const Box3& b);
	void			SetDup2Sided(hsBool b);
	void			SetRadiateNorms(hsBool b);
	void			SetNormalChan(int n);
	void			SetGeoDice(BOOL on, int maxFaces, float maxSize, int minFaces);
	void			SetIsGUI(hsBool b);
	void			SetSwappableGeom(plSharedMesh *sm);
	void			SetSwappableGeomTarget(UInt32 id);
	void			SetBoneMap(plMaxBoneMap *bones);
	void			SetOverrideHighLevelSDL(hsBool b);
	void			SetAnimCompress(UInt8 v);
	void			SetKeyReduceThreshold(hsScalar v);
	hsBool			AddRenderDependency(plMaxNodeBase* m);
	hsBool			RenderDependsOn(plMaxNodeBase* m);
	void			ClearRenderDependencies();

	void			AddBone(plMaxNodeBase* m);
	void			ClearBones();

	// Dirty flags for SceneWatcher use
	enum { kGeomDirty = 0x1, kMatDirty = 0x2, kAllDirty = 0xFF };
	hsBool	GetDirty(UInt8 i);
	void	SetDirty(UInt8 i, hsBool b);

	plKey GetParentKey() { plMaxNodeBase *pPar = (plMaxNodeBase*)GetParentNode(); hsAssert(pPar, "No Parent"); return pPar->GetKey(); }

	ISkin* FindSkinModifier(); // Returns the object's skin modifier if it has one, else nil
	const plRenderLevel& GetRenderLevel(hsBool forBlend);

	hsBool HasLoadMask();
	plLoadMask GetLoadMask();
	void AddLoadMask(const plLoadMask& m);

	hsBool IsTMAnimated();
	hsBool IsTMAnimatedRecur();
	hsBool IsMovable();	// Checks to see whether this node will ever move in the scene
	bool IsXRef();		// Returns true if object is an XRef or part of an XRef'd scene

	//----------
	// Component
	//----------
	hsBool IsComponent(Object *obj=nil);		// Object pointer is only necessary for internal use,
	hsBool IsExternComponent(Object *obj=nil);
	plComponentBase *ConvertToComponent();		// Returns nil if node is not a component


	// Normally you will only want the components that are attached to you
	// because you are in their targets list.  However, in some cases a
	// component will want to know what other components are attached to it. In
	// that case, set all to true, so that the attached components won't be
	// verified to be in your target list.
	UInt32 NumAttachedComponents(bool all=false);
	plComponentBase *GetAttachedComponent(UInt32 i, bool all=false);

	hsBool		Contains(const Point3& worldPt); // is the world space point inside my (CONVEX) geometry or dummy box?
	hsBool		Contains(const Box3& bnd, const Matrix3& l2w); // is the box contained entirely inside my (CONVEX) geometry or dummy box?
	hsScalar	BoxVolume(const Box3& bnd, const Matrix3& l2w);
	hsScalar	RegionPriority();	// returns a dominance factor. If a point is in more than one environmental
									// region, the region with highest priority wins.

	Interface *GetInterface() { return ::GetCOREInterface(); }

	static hsMatrix44	Matrix3ToMatrix44(const Matrix3& m3);
	static Matrix3		Matrix44ToMatrix3(const hsMatrix44& m44);

	// Don't use these two functions, they probably don't return what
	// you think they do. See code comments.
	Matrix3 GetWorldToParent(TimeValue t);
	Matrix3 GetParentToWorld(TimeValue t);

	hsMatrix44 GetLocalToParent44(TimeValue t = TimeValue(0));
	hsMatrix44 GetParentToLocal44(TimeValue t = TimeValue(0));
	hsMatrix44 GetLocalToWorld44(TimeValue t = TimeValue(0));
	hsMatrix44 GetWorldToLocal44(TimeValue t = TimeValue(0));
	hsMatrix44 GetOTM44(TimeValue t = TimeValue(0));
	hsMatrix44 GetVertToLocal44(TimeValue t = TimeValue(0));
	hsMatrix44 GetLocalToVert44(TimeValue t = TimeValue(0));
	hsMatrix44 GetOBBToLocal44(TimeValue t = TimeValue(0));
	hsMatrix44 GetLocalToOBB44(TimeValue t = TimeValue(0));

	Matrix3 GetLocalToParent(TimeValue t = TimeValue(0));
	Matrix3 GetParentToLocal(TimeValue t = TimeValue(0));
	Matrix3 GetLocalToWorld(TimeValue t = TimeValue(0));
	Matrix3 GetWorldToLocal(TimeValue t = TimeValue(0));
	Matrix3 GetOTM(TimeValue t = TimeValue(0));
	Matrix3 GetVertToLocal(TimeValue t = TimeValue(0));
	Matrix3 GetLocalToVert(TimeValue t = TimeValue(0));
	Matrix3 GetOBBToLocal(TimeValue t = TimeValue(0));
	Matrix3 GetLocalToOBB(TimeValue t = TimeValue(0));

protected:
	// AppDataChunk sub-chunk id's
	enum
	{
		kPlasmaAgeChunk,	// No longer in use, but cleared from old files
		kPlasmaDistChunk,	// No longer in use, but cleared from old files
		kPlasmaRoomChunk,	// No longer in use, but cleared from old files
		kPlasmaMaxNodeDataChunk,
		kPlasmaSceneViewerChunk,
		kPlasmaLightChunk,	// No longer in use, but cleared from old files
	};

	UInt8 *IGetSceneViewerChunk();
	// Attempts to convert a RefMaker pointer to a component.  Returns nil if it is not a component.
	plComponentBase *IRefMakerToComponent(ReferenceMaker *maker, bool all);

	UInt32			IGetMajorRenderLevel(hsBool forBlend);
	UInt32			IGetMinorRenderLevel(hsBool forBlend);

	hsBool			IRenderLevelSet(hsBool forBlend);
	void			ISetRenderLevel(const plRenderLevel& l, hsBool forBlend);
	const plRenderLevel& IGetRenderLevel(hsBool forBlend);
	plRenderLevel	ICalcRenderLevel(hsBool forBlend);
};

#endif //plMaxNodeBase_inc
