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
#ifndef plMaxNodeBase_inc
#define plMaxNodeBase_inc

#include "plLoadMask.h"
#include "pnKeyedObject/plKey.h"
#include "hsMatrix44.h"
#include "plRenderLevel.h"
#include "hsWindows.h"

#include <max.h>

class plComponentBase;
class hsGMesh;
class plGeometrySpan;
class plLocation;
class plMaxBoneMap;
class plMaxNodeData;
class plModifier;
class plSceneObject;
class plSharedMesh;
class ISkin;

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
    plMaxNodeData *GetMaxNodeData();    // perhaps with full getters and Setters, we can make this protected
    void SetMaxNodeData(plMaxNodeData * pDat);

    //------------------------------
    // Get Data from MaxNodeData
    //------------------------------
    // If recalculate is true the cached value is ignored. (Useful in the SceneViewer)
    bool            CanConvert(bool recalculate=false);
    plLocation      GetLocation();
    plKey           GetKey();
    plSceneObject*  GetSceneObject();
    bool            GetForceLocal();
    bool            GetReverseSort();
    bool            GetSortAsOpaque();
    bool            GetRunTimeLight();
    bool            GetForceMatShade();
    bool            GetForceVisLOS();
    bool            GetEnviron();
    bool            GetEnvironOnly();
    bool            GetWaterDecEnv();
    bool            GetVS();
    bool            GetHasWaterHeight();
    float           GetWaterHeight();
    bool            GetSmoothAll();
    bool            GetForceSortable();
    bool            GetConcave();
    bool            GetCalcEdgeLens();
    bool            GetNoPreShade();
    bool            GetForcePreShade();
    plKey           GetRoomKey();
    bool            GetDrawable();
    bool            GetPhysical();
    bool            GetItinerant();
    bool            GetUnBounded();
    bool            GetDisableNormal();
    uint32_t        GetDecalLevel();
    bool            GetMovable();
    bool            GetNoShadow();
    bool            GetForceShadow();
    bool            GetAlphaTestHigh();
    bool            GetFilterInherit();
    bool            GetIsBarney();
    bool            GetNoSpanSort();
    bool            GetNoSpanReSort();
    bool            GetNoFaceSort();
    bool            GetNoDeferDraw();
    bool            GetBlendToFB();
    bool            GetForceMaterialCopy();
    bool            GetInstanced();
    bool            GetParticleRelated();
    uint32_t        GetSoundIdxCounter();
    plSceneObject*  GetAvatarSO();
    BOOL            HasFade();
    Box3            GetFade();
    bool            GetDup2Sided();
    bool            GetRadiateNorms();
    BOOL            HasNormalChan();
    int             GetNormalChan();
    BOOL            GetGeoDice(int& maxFaces, float& maxSize, int& minFaces);
    bool            GetIsGUI();
    plSharedMesh*   GetSwappableGeom();
    uint32_t        GetSwappableGeomTarget();
    plMaxBoneMap*   GetBoneMap();
    bool            GetOverrideHighLevelSDL();
    uint8_t         GetAnimCompress();
    float           GetKeyReduceThreshold();
    int             NumRenderDependencies();
    plMaxNodeBase*  GetRenderDependency(int i);

    int             NumBones();
    plMaxNodeBase*  GetBone(int i);
    
    //------------------------------
    // Set Data from MaxNodeData
    //------------------------------
    void            SetCanConvert(bool b);
    void            SetMesh(hsGMesh *p);
    void            SetRoomKey(plKey p);
    void            SetDrawable(bool b);
    void            SetPhysical(bool b);
    void            SetItinerant(bool b);
    void            SetUnBounded(bool b);
    void            SetDisableNormal(bool b);
    void            SetDecalLevel(uint32_t i);
    void            SetMovable(bool b);
    void            SetNoPreShade(bool b);
    void            SetForcePreShade(bool b);
    void            SetReverseSort(bool b);
    void            SetSortAsOpaque(bool b);
    void            SetVS(bool b);
    void            SetHasWaterHeight(bool b);
    void            SetWaterHeight(float h);
    void            SetSmoothAll(bool b);
    void            SetForceSortable(bool b);
    void            SetConcave(bool b);
    void            SetCalcEdgeLens(bool b);
    void            SetRunTimeLight(bool b);
    void            SetForceMatShade(bool b);
    void            SetForceVisLOS(bool b);
    void            SetEnviron(bool b);
    void            SetEnvironOnly(bool b);
    void            SetWaterDecEnv(bool b);
    void            SetForceLocal(bool b);
    void            SetIsBarney(bool b);
    void            SetForceShadow(bool b);
    void            SetAlphaTestHigh(bool b);
    void            SetFilterInherit(bool b);
    void            SetNoShadow(bool b);
    void            SetNoSpanSort(bool b);
    void            SetNoSpanReSort(bool b);
    void            SetNoFaceSort(bool b);
    void            SetNoDeferDraw(bool b);
    void            SetBlendToFB(bool b);
    void            SetForceMaterialCopy(bool b);
    void            SetInstanced(bool b);
    void            SetParticleRelated(bool b);
    void            SetSoundIdxCounter(uint32_t ctr);
    void            SetAvatarSO(plSceneObject *so);
    void            SetFade(const Box3& b);
    void            SetDup2Sided(bool b);
    void            SetRadiateNorms(bool b);
    void            SetNormalChan(int n);
    void            SetGeoDice(BOOL on, int maxFaces, float maxSize, int minFaces);
    void            SetIsGUI(bool b);
    void            SetSwappableGeom(plSharedMesh *sm);
    void            SetSwappableGeomTarget(uint32_t id);
    void            SetBoneMap(plMaxBoneMap *bones);
    void            SetOverrideHighLevelSDL(bool b);
    void            SetAnimCompress(uint8_t v);
    void            SetKeyReduceThreshold(float v);
    bool            AddRenderDependency(plMaxNodeBase* m);
    bool            RenderDependsOn(plMaxNodeBase* m);
    void            ClearRenderDependencies();

    void            AddBone(plMaxNodeBase* m);
    void            ClearBones();

    // Dirty flags for SceneWatcher use
    enum { kGeomDirty = 0x1, kMatDirty = 0x2, kAllDirty = 0xFF };
    bool    GetDirty(uint8_t i);
    void    SetDirty(uint8_t i, bool b);

    plKey GetParentKey() { plMaxNodeBase *pPar = (plMaxNodeBase*)GetParentNode(); hsAssert(pPar, "No Parent"); return pPar->GetKey(); }

    ISkin* FindSkinModifier(); // Returns the object's skin modifier if it has one, else nil
    const plRenderLevel& GetRenderLevel(bool forBlend);

    bool HasLoadMask();
    plLoadMask GetLoadMask();
    void AddLoadMask(const plLoadMask& m);

    bool IsTMAnimated();
    bool IsTMAnimatedRecur();
    bool IsMovable(); // Checks to see whether this node will ever move in the scene
    bool IsXRef();      // Returns true if object is an XRef or part of an XRef'd scene

    //----------
    // Component
    //----------
    bool IsComponent(Object *obj=nullptr);        // Object pointer is only necessary for internal use,
    bool IsExternComponent(Object *obj=nullptr);
    plComponentBase *ConvertToComponent();      // Returns nil if node is not a component


    // Normally you will only want the components that are attached to you
    // because you are in their targets list.  However, in some cases a
    // component will want to know what other components are attached to it. In
    // that case, set all to true, so that the attached components won't be
    // verified to be in your target list.
    uint32_t NumAttachedComponents(bool all=false);
    plComponentBase *GetAttachedComponent(uint32_t i, bool all=false);

    bool        Contains(const Point3& worldPt); // is the world space point inside my (CONVEX) geometry or dummy box?
    bool        Contains(const Box3& bnd, const Matrix3& l2w); // is the box contained entirely inside my (CONVEX) geometry or dummy box?
    float       BoxVolume(const Box3& bnd, const Matrix3& l2w);
    float       RegionPriority();   // returns a dominance factor. If a point is in more than one environmental
                                    // region, the region with highest priority wins.

    Interface *GetInterface() { return ::GetCOREInterface(); }

    static hsMatrix44   Matrix3ToMatrix44(const Matrix3& m3);
    static Matrix3      Matrix44ToMatrix3(const hsMatrix44& m44);

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
        kPlasmaAgeChunk,    // No longer in use, but cleared from old files
        kPlasmaDistChunk,   // No longer in use, but cleared from old files
        kPlasmaRoomChunk,   // No longer in use, but cleared from old files
        kPlasmaMaxNodeDataChunk,
        kPlasmaSceneViewerChunk,
        kPlasmaLightChunk,  // No longer in use, but cleared from old files
    };

    uint8_t *IGetSceneViewerChunk();
    // Attempts to convert a RefMaker pointer to a component.  Returns nil if it is not a component.
    plComponentBase *IRefMakerToComponent(ReferenceMaker *maker, bool all);

    uint32_t          IGetMajorRenderLevel(bool forBlend);
    uint32_t          IGetMinorRenderLevel(bool forBlend);

    bool            IRenderLevelSet(bool forBlend);
    void            ISetRenderLevel(const plRenderLevel& l, bool forBlend);
    const plRenderLevel& IGetRenderLevel(bool forBlend);
    plRenderLevel   ICalcRenderLevel(bool forBlend);
};

#endif //plMaxNodeBase_inc
