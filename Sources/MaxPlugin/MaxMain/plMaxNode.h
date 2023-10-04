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

#ifndef plMaxNode_inc
#define plMaxNode_inc

#include <map>
#include <vector>

#include "plMaxNodeBase.h"
#include <iparamb2.h>

class plAGAnim;
class plAGMasterMod;
class plAGModifier;
class plController;
class plConvertSettings;
class plDrawInterface;
class plDrawable;
class plDrawableCriteria;
class plDrawableSpans;
class plErrorMsg;
class plExportProgressBar;
class plGeometrySpan;
class plLightInfo;
class plLightMapComponent;
class plLightModifier;
class plLtdDirModifier;
class plMaxBoneMap;
class plMaxNode;
class plOmniLightInfo;
class plOmniModifier;
class plPageInfoComponent;
class plPhysicalProps;
class plSceneNode;
class plSceneObject;
class plSpotLightInfo;
class plSpotModifier;
class plSynchedObject;
class plXImposterComp;

typedef bool (plMaxNode:: *PMaxNodeFunc) (plErrorMsg *, plConvertSettings *); // Function pointer to a plMaxNode member funtion

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
    bool DoRecur(PMaxNodeFunc p,plErrorMsg *, plConvertSettings *, plExportProgressBar*);
    bool DoAllRecur(PMaxNodeFunc p,plErrorMsg *, plConvertSettings *, plExportProgressBar*);

    // DoRecur takes one of the following functions
    bool ConvertValidate              (plErrorMsg *, plConvertSettings *);
    bool SetupPropertiesPass          (plErrorMsg *, plConvertSettings *);
    bool MakeSceneObject              (plErrorMsg *, plConvertSettings *);
    bool PrepareSkin                  (plErrorMsg *, plConvertSettings *);
    bool MakePhysical                 (plErrorMsg *, plConvertSettings *);
    bool FirstComponentPass           (plErrorMsg *, plConvertSettings *);
    bool MakeController               (plErrorMsg *, plConvertSettings *);
    bool MakeCoordinateInterface      (plErrorMsg *, plConvertSettings *);
    bool MakeModifiers                (plErrorMsg *, plConvertSettings *);
    bool MakeParentOrRoomConnection   (plErrorMsg *, plConvertSettings *);
    bool MakeMesh                     (plErrorMsg *, plConvertSettings *);
    bool MakeLight                    (plErrorMsg *, plConvertSettings *);
    bool MakeOccluder                 (plErrorMsg *, plConvertSettings *);
    bool ConvertComponents            (plErrorMsg *, plConvertSettings *);
    bool ClearData                    (plErrorMsg *, plConvertSettings *);
    bool ShadeMesh                    (plErrorMsg *, plConvertSettings *);
    bool MakeIfaceReferences          (plErrorMsg *, plConvertSettings *);
    bool ClearMaxNodeData             (plErrorMsg *, plConvertSettings *);
    bool DeInitComponents             (plErrorMsg *, plConvertSettings *);

    // Does specified function for all components attached to this node
    enum { kSetupProperties, kPreConvert, kConvert };
    bool DoComponents(int convertType, plErrorMsg *, plConvertSettings *);

    plKey AddModifier(plModifier *pMod, const ST::string& name);

    bool ConvertToOccluder            (plErrorMsg* pErrMsg, bool twoSided, bool isHole);

    plDrawableCriteria          GetDrawableCriteria(bool needBlending, bool needSorting);
    Point3                      GetFlexibility(); // returns Point3(flexibility, interRand, intraRand).
    plXImposterComp*            GetXImposterComp();

    int             AlphaHackLayersNeeded(int iSubMtl);
    int             NumUVWChannels();
    bool            VtxAlphaNotAvailable();
    bool            NonVtxPreshaded();
    TriObject*      GetTriObject(bool& deleteIt);

    plAGModifier*   HasAGMod();
    plAGMasterMod*  GetAGMasterMod();
    plMaxNode*      GetBonesRoot(); // Returns the root of my bones hierarchy, if I have any bones, else nil.
    void            GetBonesRootsRecur(std::vector<plMaxNode*>& list);
    plSceneObject*  MakeCharacterHierarchy(plErrorMsg *pErrMsg);
    void            SetupBonesAliasesRecur(const ST::string& rootName);
    void            SetupBoneHierarchyPalette(plMaxBoneMap *bones = nullptr);

    void SetDISceneNodeSpans( plDrawInterface *di, bool needBlending );

    bool IsLegalDecal(bool checkParent = true);
    
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
    uint32_t  GetNextSoundIdx();

    bool    IsPhysical();

    bool    CanMakeMesh( Object *obj, plErrorMsg *pErrMsg, plConvertSettings *settings );
    plDrawInterface* GetDrawInterface(); // Returns nil if there isn't a sceneobject and a drawinterface.

    // Only call during convert
    plPhysicalProps *GetPhysicalProps();

    // Little helper function. Calls FindKey() in the resManager using the location (page) of this node
    plKey   FindPageKey( uint16_t classIdx, const ST::string &name );
    const MCHAR* GetAgeName();

    void CheckSynchOptions(plSynchedObject* so);

protected:
    INode *GetRootNode()    { return GetInterface()->GetRootNode(); }

    plDrawableSpans *IGetSceneNodeSpans( plSceneNode *node, bool needBlending, bool needSorting=true );

    plLightInfo*    IMakeDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings);
    plLightInfo*    IMakeOmni(plErrorMsg* pErrMsg, plConvertSettings* settings);
    plLightInfo*    IMakeSpot(plErrorMsg* pErrMsg, plConvertSettings* settings);
    bool            IGetProjection(plLightInfo* li, plErrorMsg* pErrMsg);
    plLightInfo*    IMakeRTDirectional(plErrorMsg* pErrMsg, plConvertSettings* settings);
    plLightInfo*    IMakeRTOmni(plErrorMsg* pErrMsg, plConvertSettings* settings);
    plLightInfo*    IMakeRTSpot(plErrorMsg* pErrMsg, plConvertSettings* settings);
    plLightInfo*    IMakeRTProjDirectional( plErrorMsg *pErrMsg, plConvertSettings *settings );

    void            IGetCone(plSpotLightInfo* liInfo, LightObject* light, LightState& ls);
    void            IGetLightColors(plLightInfo* liInfo, LightObject* light, LightState& ls);
    void            IGetLightAttenuation(plOmniLightInfo* liInfo, LightObject* light, LightState& ls);
    // RunTime Lights versions
    void            IGetRTCone(plSpotLightInfo* liInfo, IParamBlock2* ProperPB);
    void            IGetRTLightColors(plLightInfo* liInfo, IParamBlock2* ProperPB);
    void            IGetRTLightAttenuation(plOmniLightInfo* liInfo, IParamBlock2* ProperPB);
    // RunTime Light animation builders
    bool            IGetRTLightAttenValues(IParamBlock2* ProperPB, float& attenConst, float& attenLinear, float& attenQuadratic,float &attenCutoff);
    void            IAdjustRTColorByIntensity(plController* ctl, IParamBlock2* ProperPB);
    bool            IAttachRTLightModifier(plLightModifier* liMod);

    plLightInfo*    IMakeLight(plErrorMsg *pErrMsg, plConvertSettings *settings);

    plSceneNode*    IGetDrawableSceneNode(plErrorMsg *pErrMsg);
    void            IAssignSpansToDrawables(std::vector<plGeometrySpan *> &spanArray, plDrawInterface *di,
                                            plErrorMsg *pErrMsg, plConvertSettings *settings );
    void            IAssignSpan(plDrawableSpans *drawable, std::vector<plGeometrySpan *> &spanArray, uint32_t &index,
                                hsMatrix44 &l2w, hsMatrix44 &w2l,
                                plErrorMsg *pErrMsg, plConvertSettings *settings);
    void            ISetupBones(plDrawableSpans *drawable, std::vector<plGeometrySpan *> &spanArray,
                                hsMatrix44 &l2w, hsMatrix44 &w2l,
                                plErrorMsg *pErrMsg, plConvertSettings *settings);
    bool            IFindBones(plErrorMsg *pErrMsg, plConvertSettings *settings);

    void            IWipeBranchDrawable(bool b);

    size_t          IBuildInstanceList(Object *obj, TimeValue t, std::vector<plMaxNode *> &nodes, bool beMoreAccurate = false);
    bool            IMakeInstanceSpans(plMaxNode *node, std::vector<plGeometrySpan *> &spanArray,
                                       plErrorMsg *pErrMsg, plConvertSettings *settings );
    bool            IMaterialsMatch( plMaxNode *otherNode, bool beMoreAccurate );

    int             IGetCachedAlphaHackValue( int iSubMtl );
    void            ISetCachedAlphaHackValue( int iSubMtl, int value );
    
friend class plLocationDlg;
};

class plMaxBoneMap
{
protected:
    typedef std::map<plMaxNodeBase*, uint32_t> BoneMap;
    BoneMap fBones;
    typedef std::map<plDrawable*, uint32_t> DrawableMap;
    DrawableMap fBaseMatrices;
    
public:
    uint8_t fNumBones;
    plMaxNodeBase *fOwner; // Make note of which node created us, so they can delete us.

    plMaxBoneMap() : fNumBones(), fOwner() { }

    void AddBone(plMaxNodeBase *bone);
    uint8_t GetIndex(plMaxNodeBase *bone);
    void FillBoneArray(plMaxNodeBase **boneArray);
    uint32_t GetBaseMatrixIndex(plDrawable *draw);
    void SetBaseMatrixIndex(plDrawable *draw, uint32_t idx);
    void SortBones();
};

#endif 