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
#ifndef PL_RTLIGHT_BASE_H
#define PL_RTLIGHT_BASE_H

// Our generic headers
#include "HeadSpin.h"
#include "hsWindows.h"

// Max related headers
#include <max.h>
#include <iparamm2.h>
#include "MaxMain/MaxCompat.h"


extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define RTOMNI_LIGHT_CLASSID    Class_ID(0x57cf7089, 0x282e5b71)
#define RTSPOT_LIGHT_CLASSID    Class_ID(0x2b263fdd, 0x19c4351f)
#define RTTSPOT_LIGHT_CLASSID   Class_ID(0x48cb06ab, 0x3c142832)
#define RTDIR_LIGHT_CLASSID     Class_ID(0x5a6d278c, 0x780a78b1)
// Projected Directional Light
#define RTPDIR_LIGHT_CLASSID    Class_ID(0x2f611934, 0x3681ff0)



#define FOREVER Interval(TIME_NegInfinity, TIME_PosInfinity)

#define DEF_TDIST       240.0f // 160.0f

#define NUM_HALF_ARC    5
#define NUM_ARC_PTS     (2*NUM_HALF_ARC+1)
#define NUM_CIRC_PTS    28
#define SEG_INDEX       7

#define ATTEN_START     2  // far
#define ATTEN_END       3  // far


#define WM_SET_TYPE     WM_USER + 0x04002


inline float MaxF(float a, float b) { return a>b?a:b; }
inline float MinF(float a, float b) { return a<b?a:b; }

class plLayerTex;
class plMaxNode;
class plRTLightBase;


///////////////////////////////////////////////////////////////////////////////
//// Base LightDlgProc ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plBaseLightProc : public ParamMap2UserDlgProc
{
    protected:
        void            ILoadComboBox( HWND hComboBox, const char *names[] );
        void            IBuildLightMesh( plRTLightBase *base, float coneSize );

    public:
        INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};

///////////////////////////////////////////////////////////////////////////////
//// plLightTexPBAccessor /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plLightTexPBAccessor : public PBAccessor
{
    public:
        void Set(PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override;
        void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid) override;

        static plLightTexPBAccessor *Instance() { return &fAccessor; }

    protected:

        static plLightTexPBAccessor fAccessor;
};

///////////////////////////////////////////////////////////////////////////////
//// plRTLightBase Class //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTLightBase : public GenLight
{

protected:
    ClassDesc2   *fClassDesc;   // Must set in derived classes constructor
    Color        fColor;        // Color of mesh
    Mesh         fMesh;         // Mesh to draw
    IParamBlock2 *fLightPB;     // The derived component's paramblock (optional)
    plLayerTex*  fTex;


    virtual bool    IHasAttenuation() { return false; }
    virtual void    IBuildMeshes( BOOL isNew ) {}

    void    BuildStaticMeshes();
    void    BuildSpotMesh(float coneSize);
    int     meshBuilt;
    int     extDispFlags;

    
    Mesh    staticMesh[2];
    Mesh    spotMesh;


public:
    friend class plBaseLightProc;
    friend class plLightTexPBAccessor;
    friend class SetTypeRest;

    ExclList exclList;
    IObjParam    *fIP;



    enum LightType
    {
        RT_OMNI,
        RT_TARGET_SPOT,
        RT_FREE_SPOT,
        RT_TARGET_DIR,
        RT_FREE_DIR
    };


    //// NOTE /////////////////////////////////////////////////////////////////////
    //  I would've overhauled this entire system to be far more sane, but doing
    //  so would break every scene with RT lights that the artists have (as in
    //  they would crash and burn, or at best just not read in). As a result,
    //  as much as it annoys me to death, we can't do a damn thing about it.
    //  Hopefully, in the future we will be able to clean all of this out somehow
    //  (a conversion utility might come in handy, dunno). 
    //
    //  For reference, the setup SHOULD have separate paramBlocks for each rollout,
    //  using the P_USE_PARAMS to duplicate the shared ones, and all the rollouts
    //  should have *IDENTICAL* block and ref #s, so the shared code actually looks
    //  sane. The final block/ref #s needed would be kBlkMain, kBlkAnim, kBlkProj 
    //  (for spots and proj dir) and kBlkAttenuation (projMaps don't need ref #s, 
    //  since they're part of the paramBlocks).
    ///////////////////////////////////////////////////////////////////////////////

    // Blk Numbers
    enum BlkNumber
    {
        // Old numbers. Phase out when possible
        kBlkGeneralLightProp,
        kBlkAttenLightProp,
        kBlkOmniLightProp,
        kBlkOtherLightProp,
        kBlkOmniLight,
        kBlkSpotLight,
        kBlkTSpotLight,
        kBlkDirLight,
        kBlkTDirLight,

        // New ones
        kBlkMain = 0,
        kBlkAnim = 9,

        kBlkDerivedStart = 20
    };

    // Ref numbers
    enum    RefNumber
    {
        kRefGeneralLightProp,
        kRefProjMap,
        kRefShadowProjMap,
        kRefShadowType,
        kRefOmniLight,
        kRefSpotLight,
        kRefTSpotLight,
        kRefDirLight,
        kRefTDirLight,
        kRefProjDirLight,
        kRefAnimParams_DEAD,

        kNumRefs,

        kRefDerivedStart = 30
    };

    //Multimap Support?
    enum    MapChoice
    {
        kLightMap1,
        kLightMap2,
        kLightMap3,
        kLightMap4,
        kLightMap5,
        kLightMap6
    };

    enum    ParamVals
    {
        kLightType,         //Inserted in v1, Removed in v4
        kAffectDiffuse,     //Inserted in v1
        kLightColor,        //Inserted in v1
        kLightExclude,      //Inserted in v1
        kCastShadows,       //Inserted in v2, OBSOLETE
        kIntensity,         //Inserted in v1
        kContrast,          //Inserted in v1
        kDiffSoft,          //Inserted in v1
        kDiffOn,            //Inserted in v1
        kSpec,              //Inserted in v1
        kAmbiOnly,          //Inserted in v1
        kStartAttenNear,     //Inserted in v1, Removed in v3
        kEndAttenNear,       //Inserted in v1, Removed in v3
        kUseNearAtten,       //Inserted in v1, Removed in v3
        kShowNearAttenRanges,//Inserted in v1, Removed in v3
        kStartAttenFar,      //Inserted in v1, Removed in v3
        kEndAttenFar,        //Inserted in v1, Removed in v3
        kUseFarAtten,        //Inserted in v1, Removed in v3
        kShowFarAttenRanges, //Inserted in v1, Removed in v3
        kLightDecay,         //Inserted in v1, Removed in v3
        kDecayEdit,          //Inserted in v1, Removed in v3
        kShowDecay,          //Inserted in v1, Removed in v3
        kUseProjectBool,    //Inserted in v1
        kProjMapTexButton,  //Inserted in v1
        kShowConeBool,      //Inserted in v1
        kOvershootBool,     //Inserted in v1, Removed in v2
        kHotSpot,           //Inserted in v1
        kFallOff,           //Inserted in v1
        kLightShapeRadio,   //Inserted in v1, Removed in v2
        kAspect,            //Inserted in v1, Removed in v2
        kUseProjectorBool,  //Inserted in v1
        kProjMapTexButton2, //Inserted in v1
        kTargetDist,        //Inserted in v1
        
        kShadowOn,          //Inserted in v2
        kShadowChoice,      //Inserted in v2
        kUseShadGlobal,     //Inserted in v2
        kShadowColor,       //Inserted in v2
        kShadowDensity,     //Inserted in v2
        kUseShadMapBool,    //Inserted in v2
        kShadMapButton,     //Inserted in v2
        kLightColorEffects, //Inserted in v2
        kAtmosShadowsBool,  //Inserted in v2
        kAtmosShadOpacity,  //Inserted in v2
        kAtmosShadColor,    //Inserted in v2
        kShadMapBias,       //Inserted in v2
        kShadMapSize,       //Inserted in v2
        kShadSampleRange,   //Inserted in v2
        kAbsoluteShadBias,  //Inserted in v2

        kUseAttenuationBool,//Inserted in v3
        kAttenMaxFalloffEdit, //Inserted in v3
        kAttenTypeRadio,    //Inserted in v3
        kSpecularColorSwatch,//Inserted in v3

        kAttenSlider,
        kLightOn,
        kAmbientOnlyStub,

        kProjTypeRadio,
        kProjNoCompress,
        kProjNoMip
    };

    // Projection types.
    enum
    {
        kIlluminate, 
        kAdd, 
        kMult, 
        kMADD
    };
    
    // Animation rollout parameters
    enum
    {
        kAnimName,
        kAnimAutoStart,
        kAnimLoop,
        kAnimLoopName
    };
    
    plRTLightBase() { }//meshBuilt = 0; fClassDesc = nullptr;  fLightPB = nullptr; fIP = nullptr; BuildMeshes(true); }
    virtual ~plRTLightBase();
    void DeleteThis() override { delete this; }

    static ParamBlockDesc2  *GetAnimPBDesc();

    MAX14_CONST MCHAR* GetObjectName() override { return (MAX14_CONST MCHAR*)fClassDesc->ClassName(); }
    void GetClassName(TSTR& s) override { s = fClassDesc->ClassName(); }

    IParamBlock2 *GetParamBlock(int i) override;
    virtual IParamBlock2* GetParamBlock2();
    IParamBlock2* GetParamBlockByID(short id) override;
    plLayerTex*   GetTex() { return fTex; }
    // So our animatables will show up in the trackview
    int NumParamBlocks() override { return 1; }
    int NumSubs() override;
    Animatable* SubAnim(int i) override;
    TSTR SubAnimName(int i) override;

    // plug-in mouse creation callback
    CreateMouseCallBack* GetCreateMouseCallBack() override;
    RefTargetHandle Clone(RemapDir &remap = DEFAULTREMAP) override { plRTLightBase* thisObj = new plRTLightBase(); BaseClone(this, thisObj, remap); return thisObj;}
    
    void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) override;
    void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) override;
    
    // main function that will build our mesh
    void FreeCaches() override;
    
    // retreives bounding box in object space/world space
    void GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;
    void GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;
    
    // main display function for this object
    int Display(TimeValue t, INode *node, ViewExp *vpt, int flags) override;
    virtual int DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing );
    void GetConePoints(TimeValue t, float aspect, float angle, float dist, Point3 *q);
    virtual void DrawCone(TimeValue t, GraphicsWindow *gw, float dist);
    int GetSpotShape() override { return 0; }
    void SetExtendedDisplay(int flags) override { extDispFlags = flags; }
    void BoxCircle(TimeValue t, float r, float d, Box3& box, int extraPt, Matrix3 *tm);
    void BoxDirPoints(TimeValue t, float angle, float dist, Box3 &box, Matrix3 *tm);
    void BoxPoints(TimeValue t, float angle, float dist, Box3 &box, Matrix3 *tm);

    void    DrawArrow( TimeValue t, GraphicsWindow *gw, Point3 &direction, float dist );
    
    void GetAttenPoints(TimeValue t, float rad, Point3 *q);
    int GetRectXPoints(TimeValue t, float angle, float dist, Point3 *q);
    int GetCirXPoints(TimeValue t, float angle, float dist, Point3 *q);
    void DrawSphereArcs(TimeValue t, GraphicsWindow *gw, float r, Point3 *q);
    
//
    void DrawX(TimeValue t, float asp, int npts, float dist, GraphicsWindow *gw, int indx); 
    void DrawCircleX(TimeValue t, GraphicsWindow *gw, float angle, float dist, Point3 *q);
    void DrawWarpRect(TimeValue t, GraphicsWindow *gw, float angle, float dist, Point3 *q);
    void DrawAttenCirOrRect(TimeValue t, GraphicsWindow *gw, float dist, BOOL froze, int uicol);
    int DrawAtten(TimeValue t, INode *inode, GraphicsWindow *gw);
    
    
    
    
    //void SetType(int tp);

    

    // hit testing of this object
    int HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) override;
    
    void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) override;
    void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) override;


    //Internal routines
    void BoxLight(TimeValue t, INode *inode, Box3& box, Matrix3 *tm);
    void GetMat(TimeValue t, INode* inode, ViewExp *vpt, Matrix3& tm);

    RefTargetHandle GetReference(int i) override;
    void SetReference(int ref, RefTargetHandle rtarg) override;
    int NumRefs() override { return kNumRefs;}
    
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) override;

    // Called to retreive the state of this object at the specified time.
    ObjectState Eval(TimeValue t) override { return ObjectState(this); }

    const MCHAR* GetCategory() { return fClassDesc->Category(); }

    //
    //  LightObject Specific Stuff below
    //
    //

    BOOL IsSpot() override { return FALSE; }
    BOOL IsDir() override  { return FALSE; }

    RefResult EvalLightState(TimeValue time, Interval& valid, LightState *ls) override;
    ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuffer=FALSE) override;
    void SetUseLight(int onOff) override { fLightPB->SetValue(kLightOn, 0, onOff); NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE); }
    BOOL GetUseLight() override { BOOL v; fLightPB->GetValue(kLightOn, 0, v, FOREVER); return v; }
    void SetHotspot(TimeValue time, float f) override;
    float GetHotspot(TimeValue t, Interval& valid = Interval(0,0)) override;
    void SetFallsize(TimeValue time, float f) override;
    float GetFallsize(TimeValue t, Interval& valid = Interval(0,0)) override;
    void SetAtten(TimeValue time, int which, float f) override;
    float GetAtten(TimeValue t, int which, Interval& valid = Interval(0,0)) override;
    
    // TDist funcs needs implementation  as of 31/5/01
    void SetTDist(TimeValue time, float f) override;
    float GetTDist(TimeValue t, Interval& valid = Interval(0,0)) override;

    void SetConeDisplay(int s, int notify=TRUE) override;
    BOOL GetConeDisplay() override;

    void SetRGBColor(TimeValue t, Point3& rgb) override; //fLightPB->SetValue(kRGB, t, rgb); NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
    Point3 GetRGBColor(TimeValue t, Interval &valid = Interval(0,0)) override; //return fLightPB->GetPoint3(kRGB, t); }        
    void SetIntensity(TimeValue t, float f) override { fLightPB->SetValue(kIntensity, t, f); NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); }
    float GetIntensity(TimeValue t, Interval& valid = Interval(0,0)) override { return fLightPB->GetFloat(kIntensity, t); }
    void SetAspect(TimeValue t, float f) override { }
    float GetAspect(TimeValue t, Interval& valid = Interval(0,0)) override { return 0.0; }
    void SetUseAtten(int s) override { fLightPB->SetValue(kUseAttenuationBool, 0, s); NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE); }
    BOOL GetUseAtten() override { return fLightPB->GetInt(kUseAttenuationBool, 0); }
    void SetUseAttenNear(int s) override { }
    BOOL GetUseAttenNear() override { return false; }
    void SetAttenDisplay(int s) override { }
    BOOL GetAttenDisplay() override { return fLightPB->GetInt(kUseAttenuationBool, 0); }
    void SetAttenDisplayNear(int s) { }
    BOOL GetAttenDisplayNear() { return false; }
    void Enable(int enab) override { fLightPB->SetValue(kLightOn, 0, enab); }
    void SetMapBias(TimeValue t, float f) override { }
    float GetMapBias(TimeValue t, Interval& valid = Interval(0,0)) override { return 0.0f; }
    void SetMapRange(TimeValue t, float f) override { }
    float GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) override { return 0.0f; }
    void SetMapSize(TimeValue t, int f) override { }
    int GetMapSize(TimeValue t, Interval& valid = Interval(0,0)) override { return 0; }
    void SetRayBias(TimeValue t, float f) override { }
    float GetRayBias(TimeValue t, Interval& valid = Interval(0,0)) override { return 0.0; } //{return 0.0f;}
    int GetAbsMapBias() override { return 0; }
    void SetAbsMapBias(int a) override { }
    int GetOvershoot() override { return 0; }
    void SetOvershoot(int a) override { }
    int GetProjector() override { return 0; }
    void SetProjector(int a) override { fLightPB->SetValue(kUseProjectorBool, 0, a); }
    ExclList* GetExclList() override { return nullptr; }
    BOOL Include() override { return false; }
    Texmap* GetProjMap() override; //{ Interval valid = Interval(0,0); Texmap* MyMap; fLightPB->GetValue(kProjMapTexButton, 0, MyMap, valid); return MyMap; }
    void SetProjMap(BitmapInfo* pmap);
    void UpdateTargDistance(TimeValue t, INode* inode) override;

    // GenLight Specific Stuff below

    BOOL GetUseGlobal() override { return false; }   //Global Shadow param
    void SetUseGlobal(int a) override { }
    BOOL GetShadow() override;
    void SetShadow(int a) override;
    BOOL GetShadowType() override { return -1; } //No Shadows generated ....
    void SetShadowType(int a) override { }       //Until implemented....
    GenLight* NewLight(int type) override { return nullptr; }
    int Type() override { return -1; }
    void SetSpotShape(int a) override { }
    void SetHSVColor(TimeValue t, class Point3 &b) override;
    Point3 GetHSVColor(TimeValue t, Interval& b) override;
    void SetContrast(int a, float other) override { } // fLightPB->SetValue(kContrast, a, other); NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
    float GetContrast(int a, Interval &valid) override { return 0.0; } //float f; f = fLightPB->GetFloat(kContrast, a); return f;}
    void SetAttenNearDisplay(int a) override { }
    BOOL GetAttenNearDisplay() override { return false; }
    ExclList& GetExclusionList() override { return exclList; }
    void SetExclusionList(ExclList & a) override { }
    BOOL SetHotSpotControl(Control* a) override { SetParamBlock2Controller(fLightPB, ParamID(kHotSpot),0, a); return true; }
    BOOL SetFalloffControl(Control* a) override { SetParamBlock2Controller(fLightPB, ParamID(kFallOff),0, a); return true; }
    Control* GetHotSpotControl() override { return GetParamBlock2Controller(fLightPB, ParamID(kHotSpot)); }
    Control* GetFalloffControl() override { return GetParamBlock2Controller(fLightPB, ParamID(kFallOff)); }
    BOOL SetColorControl(Control * a) override { SetParamBlock2Controller(fLightPB, ParamID(kLightColor),0, a); return true; }
    Control* GetColorControl() override { return GetParamBlock2Controller(fLightPB, ParamID(kLightColor)); }

    BOOL GetDecayType() override { return fLightPB->GetInt(kAttenTypeRadio, 0) + 1; } //Offset for the radio.
    void SetDecayType(BOOL onOff) override { if (!onOff) return; else { fLightPB->SetValue(kAttenTypeRadio, 0, ((int) onOff - 1)); return; } }
    void SetDecayRadius(TimeValue time, float f) override { fLightPB->SetValue(kAttenMaxFalloffEdit, time, f); }
    float GetDecayRadius(TimeValue t, Interval& valid = Interval(0,0)) override { return fLightPB->GetFloat(kAttenMaxFalloffEdit, t); }

    void SetDiffuseSoft(TimeValue time, float f) override { }//fLightPB->SetValue(kDiffSoft, time, f);}
    float GetDiffuseSoft(TimeValue t, Interval& valid = Interval(0,0)) override { return 0.0; } //return fLightPB->GetFloat(kDiffSoft, t);}
    void SetAffectDiffuse(BOOL onOff) override { }//fLightPB->SetValue(kDiffOn, 0, onOff); }
    BOOL GetAffectDiffuse() override { return false; } //fLightPB->GetInt(kDiffOn, 0); }

    LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
            WPARAM wParam,   LPARAM lParam ) override { return(0); }
};

#endif
