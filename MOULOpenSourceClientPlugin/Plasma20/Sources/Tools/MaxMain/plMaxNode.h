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

#ifndef plMaxNode_inc
#define plMaxNode_inc

#include "plMaxNodeBase.h"

#include "hsTemplates.h"
#include "hsMatrix44.h"
#include "iparamb2.h"
#include "../pnKeyedObject/plKey.h"
#include <map>

class plMaxNode;
class plErrorMsg;
class plConvertSettings;
class plExportProgressBar;
class plSceneNode;
class plDrawable;
class plDrawInterface;
class plDrawableSpans;
class plLightInfo;
class plSpotLightInfo;
class plOmniLightInfo;
class plGeometrySpan;
class ISkin;
class plSpotModifier;
class plOmniModifier;
class plLtdDirModifier;
class plLightModifier;
class plController;
class plAGModifier;
class plAGMasterMod;
class plAGAnim;
class plRenderLevel;
class plDrawableCriteria;
class plXImposterComp;
class plPhysicalProps;
class plLightMapComponent;
class plPageInfoComponent;
class plMaxBoneMap;
class plSynchedObject;

typedef hsBool (plMaxNode:: *PMaxNodeFunc) (plErrorMsg *, plConvertSettings *);	// Function pointer to a plMaxNode member funtion

class plMaxNodeTab : public Tab<plMaxNode*>
{
};

//-------------------------------------------
// plMaxNode
//-------------------------------------------
// CAREFUL! This class is different, it is derived from Max's INode (as you can see)
// But we can only add (NON Virtual) functions to plMaxNode directly
// If you want some new Data members, you can add them by adding to the class
// plMaxNodeData  This data is stored in each INode through some mechanisms supplied
// It would be nice of you to add GetFunctions for each new data member you add (see below)
//--------------------------------------------
// NOTE: an INode can be cast to a plMaxNode, but currently it is the MakeSceneObject Pass which
// Adds the plMaxNodeData to the Node

class plMaxNode : public plMaxNodeBase
{
public:
	hsBool DoRecur(PMaxNodeFunc p,plErrorMsg *, plConvertSettings *, plExportProgressBar*);
	hsBool DoAllRecur(PMaxNodeFunc p,plErrorMsg *, plConvertSettings *, plExportProgressBar*);

	// DoRecur takes one of the following functions
	hsBool ConvertValidate				(plErrorMsg *, plConvertSettings *);
	hsBool SetupPropertiesPass			(plErrorMsg *, plConvertSettings *);
	hsBool MakeSceneObject				(plErrorMsg *, plConvertSettings *);
	hsBool PrepareSkin					(plErrorMsg *, plConvertSettings *);
	hsBool MakePhysical					(plErrorMsg *, plConvertSettings *);
	hsBool FirstComponentPass			(plErrorMsg *, plConvertSettings *);
	hsBool MakeController				(plErrorMsg *, plConvertSettings *);
	hsBool MakeCoordinateInterface		(plErrorMsg *, plConvertSettings *);
	hsBool MakeModifiers				(plErrorMsg *, plConvertSettings *);
	hsBool MakeParentOrRoomConnection	(plErrorMsg *, plConvertSettings *);
	hsBool MakeMesh						(plErrorMsg *, plConvertSettings *);
	hsBool MakeLight					(plErrorMsg *, plConvertSettings *);
	hsBool MakeOccluder					(plErrorMsg *, plConvertSettings *);
	hsBool ConvertComponents			(plErrorMsg *, plConvertSettings *);
	hsBool ClearData					(plErrorMsg *, plConvertSettings *);
	hsBool ShadeMesh					(plErrorMsg *, plConvertSettings *);
	hsBool MakeIfaceReferences			(plErrorMsg *, plConvertSettings *);
	hsBool ClearMaxNodeData			    (plErrorMsg *, plConvertSettings *);
	hsBool DeInitComponents			    (plErrorMsg *, plConvertSettings *);

	// Does specified function for all components attached to this node
	enum { kSetupProperties, kPreConvert, kConvert };
	hsBool DoComponents(int convertType, plErrorMsg *, plConvertSettings *);

	plKey AddModifier(plModifier *pMod, const char* name);

	hsBool ConvertToOccluder			(plErrorMsg* pErrMsg, hsBool twoSided, hsBool isHole);

	plDrawableCriteria			GetDrawableCriteria(hsBool needBlending, hsBool needSorting);
	Point3						GetFlexibility(); // returns Point3(flexibility, interRand, intraRand).
	plXImposterComp*			GetXImposterComp();

	int				AlphaHackLayersNeeded(int iSubMtl);
	int				NumUVWChannels();
	hsBool			VtxAlphaNotAvailable();
	hsBool			NonVtxPreshaded();
	TriObject*		GetTriObject(hsBool& deleteIt);

	plAGModifier*	HasAGMod();
	plAGMasterMod*	GetAGMasterMod();
	plMaxNode*		GetBonesRoot(); // Returns the root of my bones hierarchy, if I have any bones, else nil.
	void			GetBonesRootsRecur(hsTArray<plMaxNode*>& list);
	plSceneObject*	MakeCharacterHierarchy(plErrorMsg *pErrMsg);
	void			SetupBonesAliasesRecur(const char *rootName);
	void			SetupBoneHierarchyPalette(plMaxBoneMap *bones = nil);

	void SetDISceneNodeSpans( plDrawInterface *di, hsBool needBlending );

	hsBool IsLegalDecal(hsBool checkParent = true);
	
	bool IsAnimatedLight();

	// These are public so the animation component can use them, no one else should need them
	void GetRTLightColAnim(IParamBlock2* ProperPB, plAGAnim* anim);
	void GetRTConeAnim(IParamBlock2* ProperPB, plAGAnim* anim);
	void GetRTLightAttenAnim(IParamBlock2* ProperPB, plAGAnim* anim);

	// This is used in the shading pass, where the lightmap component can
	// serve as a cache for some shading info. Returns nil if there is no LightMapComponent on this.
	plLightMapComponent* GetLightMapComponent();
	// Starting at 0, returns an incrementing index for each maxNode. Useful for assigning
	// indices to sound objects attached to the node
	UInt32	GetNextSoundIdx( void );

	hsBool	IsPhysical( void );

	hsBool	CanMakeMesh( Object *obj, plErrorMsg *pErrMsg, plConvertSettings *settings );
	plDrawInterface* GetDrawInterface(); // Returns nil if there isn't a sceneobject and a drawinterface.

	// Only call during convert
	plPhysicalProps *GetPhysicalProps();

	// Little helper function. Calls FindKey() in the resManager using the location (page) of this node
	plKey	FindPageKey( UInt16 classIdx, const char *name );
	char *GetAgeName();

	void CheckSynchOptions(plSynchedObject* so);

protected:
	INode *GetRootNode()	{ return GetInterface()->GetRootNode(); }

	plDrawableSpans	*IGetSceneNodeSpans( plSceneNode *node, hsBool needBlending, hsBool needSorting=true );

	plLightInfo*	IMakeDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings);
	plLightInfo*	IMakeOmni(plErrorMsg* pErrMsg, plConvertSettings* settings);
	plLightInfo*	IMakeSpot(plErrorMsg* pErrMsg, plConvertSettings* settings);
	hsBool			IGetProjection(plLightInfo* li, plErrorMsg* pErrMsg);
	plLightInfo*	IMakeRTDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings);
	plLightInfo*	IMakeRTOmni(plErrorMsg* pErrMsg, plConvertSettings* settings);
	plLightInfo*	IMakeRTSpot(plErrorMsg* pErrMsg, plConvertSettings* settings);
	plLightInfo*	IMakeRTProjDirectional( plErrorMsg *pErrMsg, plConvertSettings *settings );

	void			IGetCone(plSpotLightInfo* liInfo, LightObject* light, LightState& ls);
	void			IGetLightColors(plLightInfo* liInfo, LightObject* light, LightState& ls);
	void			IGetLightAttenuation(plOmniLightInfo* liInfo, LightObject* light, LightState& ls);
	// RunTime Lights versions
	void			IGetRTCone(plSpotLightInfo* liInfo, IParamBlock2* ProperPB);
	void			IGetRTLightColors(plLightInfo* liInfo, IParamBlock2* ProperPB);
	void			IGetRTLightAttenuation(plOmniLightInfo* liInfo, IParamBlock2* ProperPB);
	// RunTime Light animation builders
	hsBool			IGetRTLightAttenValues(IParamBlock2* ProperPB, hsScalar& attenConst, hsScalar& attenLinear, hsScalar& attenQuadratic,hsScalar &attenCutoff);
	void			IAdjustRTColorByIntensity(plController* ctl, IParamBlock2* ProperPB);
	hsBool			IAttachRTLightModifier(plLightModifier* liMod);

	plLightInfo*	IMakeLight(plErrorMsg *pErrMsg, plConvertSettings *settings);

	plSceneNode*	IGetDrawableSceneNode(plErrorMsg *pErrMsg);
	void			IAssignSpansToDrawables( hsTArray<plGeometrySpan *> &spanArray, plDrawInterface *di,
											plErrorMsg *pErrMsg, plConvertSettings *settings );
	void			IAssignSpan( plDrawableSpans *drawable, hsTArray<plGeometrySpan *> &spanArray, UInt32 &index,
								 hsMatrix44 &l2w, hsMatrix44 &w2l,
								 plErrorMsg *pErrMsg, plConvertSettings *settings );
	void			ISetupBones( plDrawableSpans *drawable, hsTArray<plGeometrySpan *> &spanArray,
								 hsMatrix44 &l2w, hsMatrix44 &w2l,
								 plErrorMsg *pErrMsg, plConvertSettings *settings );
	hsBool			IFindBones(plErrorMsg *pErrMsg, plConvertSettings *settings);

	void			IWipeBranchDrawable(hsBool b);

	UInt32			IBuildInstanceList( Object *obj, TimeValue t, hsTArray<plMaxNode *> &nodes, hsBool beMoreAccurate = false );
	hsBool			IMakeInstanceSpans( plMaxNode *node, hsTArray<plGeometrySpan *> &spanArray,
									   plErrorMsg *pErrMsg, plConvertSettings *settings );
	hsBool			IMaterialsMatch( plMaxNode *otherNode, hsBool beMoreAccurate );

	int				IGetCachedAlphaHackValue( int iSubMtl );
	void			ISetCachedAlphaHackValue( int iSubMtl, int value );
	
friend class plLocationDlg;
};

class plMaxBoneMap
{
protected:
	typedef std::map<plMaxNodeBase*, UInt32> BoneMap;
	BoneMap fBones;
	typedef std::map<plDrawable*, UInt32> DrawableMap;
	DrawableMap fBaseMatrices;
	
public:
	UInt8 fNumBones;
	plMaxNodeBase *fOwner; // Make note of which node created us, so they can delete us.

	plMaxBoneMap() : fNumBones(0), fOwner(nil) {}

	void AddBone(plMaxNodeBase *bone);
	UInt8 GetIndex(plMaxNodeBase *bone);
	void FillBoneArray(plMaxNodeBase **boneArray);
	UInt32 GetBaseMatrixIndex(plDrawable *draw);
	void SetBaseMatrixIndex(plDrawable *draw, UInt32 idx);
	void SortBones();
};

#endif 