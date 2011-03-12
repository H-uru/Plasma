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

#ifndef plMaxNodeData_inc
#define plMaxNodeData_inc

#include "hsTypes.h"
#include "hsTemplates.h"
#include "hsColorRGBA.h"
#include "plRenderLevel.h"
#include "plPhysicalProps.h"
#include "hsBitVector.h"
#include "../pnKeyedObject/plKey.h"
#include "plLoadMask.h"

class plMaxNodeBase;
class plMaxNode;
class plLocation;
class plSceneObject;
class hsGMesh;
class plGeometrySpan;
class plMaxBoneMap;
class plSharedMesh;

class plMaxNodeBaseTab : public Tab<plMaxNodeBase*>
{
};


//
//		Created hoping to trim some fat so to speak for all the bools that
//			we are tossing around.
//

class DataBF
{
public:
	enum DatBit
	{
		kCanConvert		= 0x0,
		kForceLocal,
		kDrawable,			// Absence of this prop forbids making this drawable
		kBlendToFB,
		kItinerant,
		kRunTimeLight,
		kMovable,
		kIsBarney,
		kForceMatCopy,
		kNoPreShade,
		kIsInstanced,
		kParticleRelated,
		kNoSpanReSort,
		kHasFade,
		kNoFaceSort,
		kNoSpanSort,
		kNoDeferDraw,
		kBLevelSet,
		kOLevelSet,
		kDup2Sided,
		kRadiateNorms,
		kGeoDice,
		kForcePreShade,
		kIsGUI,
		kSwappableGeomTarget,
		kNoShadow,
		kForceShadow,
		kFilterInherit,
		kReverseSort,
		kVS,
		kConcave,
		kCalcEdgeLens,
		kEnviron,
		kEnvironOnly,
		kPhysical,			// Absence of this prop forbids making this physical
		kSmoothAll,
		kForceSortable,
		kOverrideHighLevelSDL,
		kDisableNormal,
		kHasLoadMask,
		kWaterDecEnv,
		kWaterHeight,
		kAlphaTestHigh,
		kForceMatShade,
		kUnBounded,
		kForceVisLOS,
		kSortAsOpaque,
	};

	hsBitVector*	fBitVector;

	DataBF() 											
	{ 
		fBitVector = TRACKED_NEW hsBitVector;	
		fBitVector->SetBit(kDrawable); 
		fBitVector->SetBit(kPhysical); 
	}
	virtual ~DataBF()									{ delete fBitVector; }

	DataBF& operator=(const DataBF& ot)
	{
		*fBitVector = *ot.fBitVector;
		return *this;
	}
	DataBF(const DataBF& ot)
	{
		fBitVector = TRACKED_NEW hsBitVector;
		*fBitVector = *ot.fBitVector;
	}

	void Init()											
	{ 
		fBitVector = TRACKED_NEW hsBitVector; 
		fBitVector->SetBit(kDrawable); 
		fBitVector->SetBit(kPhysical); 
	}
	void DeInit()										{ delete fBitVector; fBitVector = nil; }

	hsBool	CanBF(DatBit bitChoice)					{ return fBitVector->IsBitSet(bitChoice);	}
	void	SetBF(hsBool b, DatBit bitChoice)		{ fBitVector->SetBit(bitChoice, b); }
};


class plMaxNodeData
{
public:
	plMaxNodeData() : 
		fpKey(nil), 
		fpSO(nil) , 
		fDecalLevel(0),
		fpMesh(nil), 
		fpRoomKey(nil), 
		fSoundIdxCounter( 0 ), 
		fAvatarSO(nil), 
		fFade(Point3(0,0,0), Point3(0,0,0)),
		fNormalChan(0),
		fWaterHeight(0),
		fGDMaxFaces(0), fGDMaxSize(0), fGDMinFaces(0),
		fSwapMesh(nil),
		fSwapTargetID((UInt32)-1),
		fCachedAlphaHackLayerCounts(nil),
		fBoneMap(nil),
		fAnimCompression(1), // Should be plAnimCompressComp::kCompressionLow,
							 // but I don't want to include the entire header.
	    fKeyReduceThreshold(0.0002)
	{ }
	~plMaxNodeData() 
	{ 
		fpKey = nil; 
		fpRoomKey = nil;
		fpSO = 0; 
		fAvatarSO = nil; 
		delete fCachedAlphaHackLayerCounts; 
		MaxDatBF.DeInit();
	}

	// Call init on MaxNodeData whose constructor was never called, e.g. if it was malloc'd.
	plMaxNodeData&	Init() 
	{
		memset( &fpKey, 0, sizeof( plKey ) ); 
		memset( &fpRoomKey, 0, sizeof( plKey ) ); 
		fRenderDependencies.Init(); 
		fBones.Init(); 
		fCachedAlphaHackLayerCounts = nil; 
		MaxDatBF.Init();
		return *this; 
	}
	// Ditto
	void			DeInit( void ) 
	{ 
		fpKey = nil; 
		fpRoomKey = nil;
		fpSO = 0; fAvatarSO = nil; 
		delete fCachedAlphaHackLayerCounts; 
		fCachedAlphaHackLayerCounts = nil; 
		MaxDatBF.DeInit();
	}

	plKey 			GetKey()							{ return fpKey;		}
	void			SetKey(plKey p )					{ fpKey = p;		}
	plSceneObject * GetSceneObject()					{ return fpSO;		}
	void			SetSceneObject(plSceneObject *p)	{ fpSO = p;			}

	hsBool			CanConvert()						{ return MaxDatBF.CanBF(MaxDatBF.kCanConvert);	}
	void			SetCanConvert(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kCanConvert);		}

	hsBool			GetForceLocal()						{ return MaxDatBF.CanBF(MaxDatBF.kForceLocal);	}
	void			SetForceLocal(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kForceLocal);		}

	void *			GetMesh()							{ return fpMesh;	}	// void pointer for header simplicity
	void			SetMesh(hsGMesh *p)					{ fpMesh = p;		}		

	hsBool			GetDrawable()						{return MaxDatBF.CanBF(MaxDatBF.kDrawable);		}
	void			SetDrawable(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kDrawable);		}

	hsBool			GetPhysical()						{return MaxDatBF.CanBF(MaxDatBF.kPhysical);		}
	void			SetPhysical(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kPhysical);		}

	hsBool			GetRunTimeLight()					{return MaxDatBF.CanBF(MaxDatBF.kRunTimeLight);	}
	void			SetRunTimeLight(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kRunTimeLight);	}

	hsBool			GetForceMatShade()					{return MaxDatBF.CanBF(MaxDatBF.kForceMatShade);	}
	void			SetForceMatShade(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kForceMatShade);	}

	hsBool			GetForceVisLOS()					{return MaxDatBF.CanBF(MaxDatBF.kForceVisLOS);	}
	void			SetForceVisLOS(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kForceVisLOS);	}

	hsBool			GetEnviron()						{return MaxDatBF.CanBF(MaxDatBF.kEnviron);	}
	void			SetEnviron(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kEnviron);	}

	hsBool			GetEnvironOnly()					{return MaxDatBF.CanBF(MaxDatBF.kEnvironOnly);	}
	void			SetEnvironOnly(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kEnvironOnly);	}

	hsBool			GetWaterDecEnv()					{ return MaxDatBF.CanBF(MaxDatBF.kWaterDecEnv); }
	void			SetWaterDecEnv(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kWaterDecEnv);		}

	hsBool			GetItinerant()						{return MaxDatBF.CanBF(MaxDatBF.kItinerant);	}
	void			SetItinerant(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kItinerant);		}

	hsBool			GetUnBounded()						{return MaxDatBF.CanBF(MaxDatBF.kUnBounded);	}
	void			SetUnBounded(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kUnBounded);		}

	hsBool			GetDisableNormal()					{return MaxDatBF.CanBF(MaxDatBF.kDisableNormal);	}
	void			SetDisableNormal(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kDisableNormal);		}

	hsBool			GetMovable()						{return MaxDatBF.CanBF(MaxDatBF.kMovable);		}
	void			SetMovable(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kMovable);			}

	hsBool			GetNoPreShade()						{return MaxDatBF.CanBF(MaxDatBF.kNoPreShade);	}
	void			SetNoPreShade(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kNoPreShade);		}

	hsBool			GetForcePreShade()					{return MaxDatBF.CanBF(MaxDatBF.kForcePreShade);	}
	void			SetForcePreShade(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kForcePreShade);		}

	hsBool			GetNoShadow()						{return MaxDatBF.CanBF(MaxDatBF.kNoShadow);		}
	void			SetNoShadow(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kNoShadow);		}

	hsBool			GetForceShadow()					{return MaxDatBF.CanBF(MaxDatBF.kForceShadow);	}
	void			SetForceShadow(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kForceShadow);		}

	hsBool			GetAlphaTestHigh()					{return MaxDatBF.CanBF(MaxDatBF.kAlphaTestHigh);	}
	void			SetAlphaTestHigh(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kAlphaTestHigh);		}

	hsBool			GetFilterInherit()					{return MaxDatBF.CanBF(MaxDatBF.kFilterInherit);	}
	void			SetFilterInherit(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kFilterInherit);		}

	hsBool			GetIsBarney()						{return MaxDatBF.CanBF(MaxDatBF.kIsBarney);		}
	void			SetIsBarney(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kIsBarney);		}

	hsBool			GetNoSpanSort()						{return MaxDatBF.CanBF(MaxDatBF.kNoSpanSort);	}
	void			SetNoSpanSort(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kNoSpanSort);		}

	hsBool			GetNoSpanReSort()					{return MaxDatBF.CanBF(MaxDatBF.kNoSpanReSort);	}
	void			SetNoSpanReSort(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kNoSpanReSort);	}

	hsBool			GetNoFaceSort()						{return MaxDatBF.CanBF(MaxDatBF.kNoFaceSort);	}
	void			SetNoFaceSort(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kNoFaceSort);		}

	hsBool			GetReverseSort()					{return MaxDatBF.CanBF(MaxDatBF.kReverseSort);	}
	void			SetReverseSort(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kReverseSort);		}

	hsBool			GetSortAsOpaque()					{return MaxDatBF.CanBF(MaxDatBF.kSortAsOpaque);	}
	void			SetSortAsOpaque(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kSortAsOpaque);		}

	hsBool			GetVS()								{return MaxDatBF.CanBF(MaxDatBF.kVS);	}
	void			SetVS(hsBool b)						{ MaxDatBF.SetBF(b, MaxDatBF.kVS);		}

	hsBool			GetHasWaterHeight()					{ return MaxDatBF.CanBF(MaxDatBF.kWaterHeight); }
	void			SetHasWaterHeight(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kWaterHeight); }
	hsScalar		GetWaterHeight()					{ return fWaterHeight; }
	void			SetWaterHeight(hsScalar f)			{ SetHasWaterHeight(true); fWaterHeight = f; }

	hsBool			GetSmoothAll()						{return MaxDatBF.CanBF(MaxDatBF.kSmoothAll);	}
	void			SetSmoothAll(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kSmoothAll);		}

	hsBool			GetForceSortable()					{return MaxDatBF.CanBF(MaxDatBF.kForceSortable);	}
	void			SetForceSortable(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kForceSortable);		}

	hsBool			GetConcave()						{return MaxDatBF.CanBF(MaxDatBF.kConcave);	}
	void			SetConcave(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kConcave);		}

	hsBool			GetCalcEdgeLens()					{return MaxDatBF.CanBF(MaxDatBF.kCalcEdgeLens);	}
	void			SetCalcEdgeLens(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kCalcEdgeLens);	}

	hsBool			GetNoDeferDraw()					{return MaxDatBF.CanBF(MaxDatBF.kNoDeferDraw);	}
	void			SetNoDeferDraw(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kNoDeferDraw);		}

	hsBool			GetBlendToFB()						{return MaxDatBF.CanBF(MaxDatBF.kBlendToFB);	}
	void			SetBlendToFB(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kBlendToFB);		}

	hsBool			GetForceMaterialCopy()				{return MaxDatBF.CanBF(MaxDatBF.kForceMatCopy);	}
	void			SetForceMaterialCopy(hsBool b)		{ MaxDatBF.SetBF(b, MaxDatBF.kForceMatCopy);	}

	hsBool			GetInstanced()						{return MaxDatBF.CanBF(MaxDatBF.kIsInstanced);	}
	void			SetInstanced(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kIsInstanced);		}

	hsBool			GetParticleRelated()				{return MaxDatBF.CanBF(MaxDatBF.kParticleRelated);	}
	void			SetParticleRelated(hsBool b)		{ MaxDatBF.SetBF(b, MaxDatBF.kParticleRelated);		}

	hsBool			GetDup2Sided()						{return MaxDatBF.CanBF(MaxDatBF.kDup2Sided);	}
	void			SetDup2Sided(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kDup2Sided);		}

	hsBool			GetRadiateNorms()					{return MaxDatBF.CanBF(MaxDatBF.kRadiateNorms);	}
	void			SetRadiateNorms(hsBool b)			{ MaxDatBF.SetBF(b, MaxDatBF.kRadiateNorms);	}

	hsBool			GetGeoDice()						{return MaxDatBF.CanBF(MaxDatBF.kGeoDice);		}
	void			SetGeoDice(hsBool b)				{ MaxDatBF.SetBF(b, MaxDatBF.kGeoDice);			}

	hsBool			GetIsGUI()							{return MaxDatBF.CanBF(MaxDatBF.kIsGUI);		}
	void			SetIsGUI(hsBool b)					{ MaxDatBF.SetBF(b, MaxDatBF.kIsGUI);			}

	plSharedMesh*	GetSwappableGeom()					{ return fSwapMesh;}
	void			SetSwappableGeom(plSharedMesh *sm)	{ fSwapMesh = sm;	}

	UInt32			GetSwappableGeomTarget()			{ return fSwapTargetID;}
	void			SetSwappableGeomTarget(UInt32 id)	{ fSwapTargetID = id;}

	plMaxBoneMap*	GetBoneMap()						{ return fBoneMap;}
	void			SetBoneMap(plMaxBoneMap *bones)		{ fBoneMap = bones;}

	int				GetGeoDiceMaxFaces()				{ return fGDMaxFaces; }
	void			SetGeoDiceMaxFaces(int f)			{ fGDMaxFaces = f; }

	float			GetGeoDiceMaxSize()					{ return fGDMaxSize; }
	void			SetGeoDiceMaxSize(float f)			{ fGDMaxSize = f; }

	int				GetGeoDiceMinFaces()				{ return fGDMinFaces; }
	void			SetGeoDiceMinFaces(int f)			{ fGDMinFaces = f; }

	plKey 			GetRoomKey()						{ return fpRoomKey;	}
	void			SetRoomKey(plKey p)					{ fpRoomKey = p; }
	
	UInt32			GetDecalLevel()						{ return fDecalLevel; }
	void			SetDecalLevel(UInt32 i)				{ fDecalLevel = i; }

	UInt32			GetSoundIdxCounter()				{ return fSoundIdxCounter; }
	void			SetSoundIdxCounter( UInt32 i )		{ fSoundIdxCounter = i; }

	plSceneObject * GetAvatarSO()						{ return fAvatarSO; }
	void			SetAvatarSO(plSceneObject *so)		{ fAvatarSO = so; }
	
	int				NumRenderDependencies()				{ return fRenderDependencies.Count(); }
	plMaxNodeBase*	GetRenderDependency(int i)			{ return fRenderDependencies[i]; }
	void			AddRenderDependency(plMaxNodeBase* m)	{ fRenderDependencies.Append(1, &m); }
	void			ClearRenderDependencies()			{ fRenderDependencies.ZeroCount(); }

	int				NumBones()							{ return fBones.Count(); }
	plMaxNodeBase*	GetBone(int i)						{ return fBones[i]; }
	void			AddBone(plMaxNodeBase* m)			{ fBones.Append(1, &m); }
	void			ClearBones()						{ fBones.ZeroCount(); }

	hsBool			HasFade()							{ return MaxDatBF.CanBF(MaxDatBF.kHasFade); }
	void			SetFade(const Box3& b)				{ MaxDatBF.SetBF((b.Min()[2] != 0)||(b.Max()[2] != 0), MaxDatBF.kHasFade); fFade = b; }
	Box3			GetFade()							{ return fFade; }

	hsBool			HasLoadMask()						{ return MaxDatBF.CanBF(MaxDatBF.kHasLoadMask); }
	plLoadMask		GetLoadMask()						{ return HasLoadMask() ? fLoadMask : plLoadMask::kAlways; }
	void			AddLoadMask(const plLoadMask& m)	{ if( !HasLoadMask() ) { fLoadMask = m; MaxDatBF.SetBF(true, MaxDatBF.kHasLoadMask); }else{ fLoadMask |= m; } }

	hsBool			HasNormalChan()						{ return fNormalChan > 0; }
	void			SetNormalChan(int n)				{ fNormalChan = n; }
	int				GetNormalChan()						{ return fNormalChan; }

	hsBool			BlendLevelSet()						{ return MaxDatBF.CanBF(MaxDatBF.kBLevelSet); }
	void			SetBlendLevel(const plRenderLevel& l) { fBlendLevel = l; MaxDatBF.SetBF(true, MaxDatBF.kBLevelSet); }
	const plRenderLevel&	GetBlendLevel()				{ return fBlendLevel; }
	hsBool			OpaqueLevelSet()					{ return MaxDatBF.CanBF(MaxDatBF.kOLevelSet); }
	void			SetOpaqueLevel(const plRenderLevel& l) { fOpaqueLevel = l; MaxDatBF.SetBF(true, MaxDatBF.kOLevelSet); }
	const plRenderLevel&	GetOpaqueLevel()			{ return fOpaqueLevel; }

	plPhysicalProps* GetPhysicalProps()					{ return &fPhysicalProps; }

	hsTArray<int>	*GetAlphaHackLayersCache( void )						{ return fCachedAlphaHackLayerCounts; }
	void			SetAlphaHackLayersCache( hsTArray<int> *cache )			{ fCachedAlphaHackLayerCounts = cache; }
	hsBool			GetOverrideHighLevelSDL()			{ return MaxDatBF.CanBF(MaxDatBF.kOverrideHighLevelSDL); }
	void			SetOverrideHighLevelSDL(hsBool b)	{ MaxDatBF.SetBF(b, MaxDatBF.kOverrideHighLevelSDL); }
	UInt8			GetAnimCompress()					{ return fAnimCompression; }
	void			SetAnimCompress(UInt8 v)			{ fAnimCompression = v; }
	hsScalar		GetKeyReduceThreshold()				{ return fKeyReduceThreshold; }
	void			SetKeyReduceThreshold(hsScalar v)	{ fKeyReduceThreshold = v; }

protected:
	plKey 			fpKey;
	plSceneObject *	fpSO;
	// hacking this in here temporarily because everything's about to change in the mesh world...
	hsGMesh*		fpMesh;
	plKey 			fpRoomKey;
	UInt32			fDecalLevel;
	Int32			fNormalChan;
	UInt32			fSoundIdxCounter;
	plSceneObject *	fAvatarSO;
	plMaxNodeBaseTab	fRenderDependencies;
	plMaxNodeBaseTab	fBones;
	Box3			fFade;
	int				fGDMaxFaces;
	float			fGDMaxSize;
	int				fGDMinFaces;
	plRenderLevel	fBlendLevel;
	plRenderLevel	fOpaqueLevel;
	plPhysicalProps fPhysicalProps;
	UInt32			fSwapTargetID;
	hsTArray<int>	*fCachedAlphaHackLayerCounts;
	plSharedMesh    *fSwapMesh;
	plMaxBoneMap	*fBoneMap;
	plLoadMask		fLoadMask;
	hsScalar		fWaterHeight;
	UInt8			fAnimCompression;
	hsScalar		fKeyReduceThreshold;
	DataBF	MaxDatBF;
};


#endif // plSimpleConvert_inc