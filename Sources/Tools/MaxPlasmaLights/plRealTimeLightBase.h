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
#ifndef PL_RTLIGHT_BASE_H
#define PL_RTLIGHT_BASE_H

// Max related headers
#include "max.h"
#include "iparamb2.h"
#include "iparamm2.h"

// Our generic headers
#include "hsTypes.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"


extern TCHAR *GetString(int id);



extern HINSTANCE hInstance;

#define RTOMNI_LIGHT_CLASSID	Class_ID(0x57cf7089, 0x282e5b71)
#define RTSPOT_LIGHT_CLASSID	Class_ID(0x2b263fdd, 0x19c4351f)
#define RTTSPOT_LIGHT_CLASSID	Class_ID(0x48cb06ab, 0x3c142832)
#define RTDIR_LIGHT_CLASSID		Class_ID(0x5a6d278c, 0x780a78b1)
// Projected Directional Light
#define RTPDIR_LIGHT_CLASSID	Class_ID(0x2f611934, 0x3681ff0)



#define FOREVER Interval(TIME_NegInfinity, TIME_PosInfinity)

#define DEF_TDIST		240.0f // 160.0f

#define NUM_HALF_ARC	5
#define NUM_ARC_PTS	    (2*NUM_HALF_ARC+1)
#define NUM_CIRC_PTS	28
#define SEG_INDEX		7

#define ATTEN_START		2  // far
#define ATTEN_END		3  // far


#define WM_SET_TYPE		WM_USER + 0x04002


inline float MaxF(float a, float b) { return a>b?a:b; }
inline float MinF(float a, float b) { return a<b?a:b; }

class plMaxNode;
class ClassDesc2;


///////////////////////////////////////////////////////////////////////////////
//// Base LightDlgProc ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTLightBase;

class plBaseLightProc : public ParamMap2UserDlgProc
{
	protected:
		void			ILoadComboBox( HWND hComboBox, const char *names[] );
		void			IBuildLightMesh( plRTLightBase *base, float coneSize );

	public:
		virtual BOOL	DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
};

///////////////////////////////////////////////////////////////////////////////
//// plLightTexPBAccessor /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plLightTexPBAccessor : public PBAccessor
{
	public:
		void Set( PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t );
		void Get( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid );

		static plLightTexPBAccessor	*Instance( void ) { return &fAccessor; }

	protected:

		static plLightTexPBAccessor	fAccessor;
};

///////////////////////////////////////////////////////////////////////////////
//// plRTLightBase Class //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTLightBase : public GenLight
{

protected:
	ClassDesc2	 *fClassDesc;	// Must set in derived classes constructor
	Color		 fColor;		// Color of mesh
	Mesh		 fMesh;			// Mesh to draw
	IParamBlock2 *fLightPB;		// The derived component's paramblock (optional)
	plLayerTex*  fTex;


	virtual hsBool	IHasAttenuation( void ) { return false; }
	virtual void	IBuildMeshes( BOOL isNew ) {}

	void	BuildStaticMeshes();
	void	BuildSpotMesh(float coneSize);
	int		meshBuilt;
	int		extDispFlags;

	
	Mesh	staticMesh[2];
	Mesh	spotMesh;


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
	//	I would've overhauled this entire system to be far more sane, but doing
	//	so would break every scene with RT lights that the artists have (as in
	//	they would crash and burn, or at best just not read in). As a result,
	//	as much as it annoys me to death, we can't do a damn thing about it.
	//	Hopefully, in the future we will be able to clean all of this out somehow
	//	(a conversion utility might come in handy, dunno). 
	//
	//	For reference, the setup SHOULD have separate paramBlocks for each rollout,
	//	using the P_USE_PARAMS to duplicate the shared ones, and all the rollouts
	//	should have *IDENTICAL* block and ref #s, so the shared code actually looks
	//	sane. The final block/ref #s needed would be kBlkMain, kBlkAnim, kBlkProj 
	//	(for spots and proj dir) and kBlkAttenuation (projMaps don't need ref #s, 
	//	since they're part of the paramBlocks).
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
	enum	RefNumber
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
	enum	MapChoice
	{
		kLightMap1,
		kLightMap2,
		kLightMap3,
		kLightMap4,
		kLightMap5,
		kLightMap6
	};

	enum	ParamVals
	{
		kLightType,			//Inserted in v1, Removed in v4
		kAffectDiffuse,		//Inserted in v1
		kLightColor,		//Inserted in v1
		kLightExclude,		//Inserted in v1
		kCastShadows,		//Inserted in v2, OBSOLETE
		kIntensity,			//Inserted in v1
		kContrast,			//Inserted in v1
		kDiffSoft,			//Inserted in v1
		kDiffOn,			//Inserted in v1
		kSpec,				//Inserted in v1
		kAmbiOnly,			//Inserted in v1
		kStartAttenNear,	 //Inserted in v1, Removed in v3
		kEndAttenNear,		 //Inserted in v1, Removed in v3
		kUseNearAtten,		 //Inserted in v1, Removed in v3
		kShowNearAttenRanges,//Inserted in v1, Removed in v3
		kStartAttenFar,		 //Inserted in v1, Removed in v3
		kEndAttenFar,		 //Inserted in v1, Removed in v3
		kUseFarAtten,		 //Inserted in v1, Removed in v3
		kShowFarAttenRanges, //Inserted in v1, Removed in v3
		kLightDecay,		 //Inserted in v1, Removed in v3
		kDecayEdit,			 //Inserted in v1, Removed in v3
		kShowDecay,			 //Inserted in v1, Removed in v3
		kUseProjectBool,	//Inserted in v1
		kProjMapTexButton,	//Inserted in v1
		kShowConeBool, 		//Inserted in v1
		kOvershootBool,		//Inserted in v1, Removed in v2
		kHotSpot,			//Inserted in v1
		kFallOff,			//Inserted in v1
		kLightShapeRadio,	//Inserted in v1, Removed in v2
		kAspect,			//Inserted in v1, Removed in v2
		kUseProjectorBool,	//Inserted in v1
		kProjMapTexButton2,	//Inserted in v1
		kTargetDist,		//Inserted in v1
		
		kShadowOn,			//Inserted in v2
		kShadowChoice,		//Inserted in v2
		kUseShadGlobal,		//Inserted in v2
		kShadowColor,		//Inserted in v2
		kShadowDensity,		//Inserted in v2
		kUseShadMapBool,	//Inserted in v2
		kShadMapButton,		//Inserted in v2
		kLightColorEffects,	//Inserted in v2
		kAtmosShadowsBool,	//Inserted in v2
		kAtmosShadOpacity,	//Inserted in v2
		kAtmosShadColor,	//Inserted in v2
		kShadMapBias,		//Inserted in v2
		kShadMapSize,		//Inserted in v2
		kShadSampleRange,	//Inserted in v2
		kAbsoluteShadBias,	//Inserted in v2

		kUseAttenuationBool,//Inserted in v3
		kAttenMaxFalloffEdit, //Inserted in v3
		kAttenTypeRadio,	//Inserted in v3
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
	
	plRTLightBase() { }//meshBuilt = 0; fClassDesc = NULL;  fLightPB = NULL; fIP = NULL; BuildMeshes(true); }
	virtual ~plRTLightBase();
	void DeleteThis() { delete this; }

	static ParamBlockDesc2	*GetAnimPBDesc( void );

	TCHAR* GetObjectName()		{ return (TCHAR*)fClassDesc->ClassName(); }
	void GetClassName(TSTR& s)	{ s = fClassDesc->ClassName(); }

	virtual IParamBlock2 *GetParamBlock( int i );
	virtual IParamBlock2* GetParamBlock2();
	virtual IParamBlock2* GetParamBlockByID(short id);
	plLayerTex*	  GetTex() { return fTex; }
	// So our animatables will show up in the trackview
	virtual int	NumParamBlocks() { return 1; }
	virtual int NumSubs();
	virtual Animatable* SubAnim(int i);
	virtual TSTR SubAnimName(int i);

	// plug-in mouse creation callback
	CreateMouseCallBack* GetCreateMouseCallBack();
	RefTargetHandle Clone(RemapDir &remap = NoRemap()){ plRTLightBase* thisObj = TRACKED_NEW plRTLightBase(); BaseClone(this, thisObj, remap); return thisObj;}
	
	virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
	virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
	
	// main function that will build our mesh
	void FreeCaches();
	
	// retreives bounding box in object space/world space
	void GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box);
	void GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box);
	
	// main display function for this object
	int Display(TimeValue t, INode *node, ViewExp *vpt, int flags);
	virtual int DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing );
	void GetConePoints(TimeValue t, float aspect, float angle, float dist, Point3 *q);
	virtual void DrawCone(TimeValue t, GraphicsWindow *gw, float dist);
	int GetSpotShape(void){ return 0; }
	void SetExtendedDisplay(int flags){ extDispFlags = flags; }
	void BoxCircle(TimeValue t, float r, float d, Box3& box, int extraPt, Matrix3 *tm);
	void BoxDirPoints(TimeValue t, float angle, float dist, Box3 &box, Matrix3 *tm);
	void BoxPoints(TimeValue t, float angle, float dist, Box3 &box, Matrix3 *tm);

	void	DrawArrow( TimeValue t, GraphicsWindow *gw, Point3 &direction, float dist );
	
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
	int HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	
	void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );


	//Internal routines
	void BoxLight(TimeValue t, INode *inode, Box3& box, Matrix3 *tm);
	void GetMat(TimeValue t, INode* inode, ViewExp *vpt, Matrix3& tm);

	virtual RefTargetHandle GetReference(int i);
	virtual void SetReference(int ref, RefTargetHandle rtarg);
	virtual int NumRefs() { return kNumRefs;}
	
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

	// Called to retreive the state of this object at the specified time.
	ObjectState Eval(TimeValue t) { return ObjectState(this); }

	const char *GetCategory(){ return fClassDesc->Category(); }

	//
	//	LightObject	Specific Stuff below
	//
	//

	virtual BOOL IsSpot( void )	{ return FALSE; }
	virtual BOOL IsDir( void )	{ return FALSE; }

	RefResult EvalLightState(TimeValue time, Interval& valid, LightState *ls);
	ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuffer=FALSE);
	void SetUseLight(int onOff) { fLightPB->SetValue(kLightOn, 0, onOff); NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE); }
	BOOL GetUseLight(void) { BOOL v; fLightPB->GetValue(kLightOn, 0, v, FOREVER); return v; }
	void SetHotspot(TimeValue time, float f); 
	float GetHotspot(TimeValue t, Interval& valid = Interval(0,0));
	void SetFallsize(TimeValue time, float f); 
	float GetFallsize(TimeValue t, Interval& valid = Interval(0,0));
	void SetAtten(TimeValue time, int which, float f);
	float GetAtten(TimeValue t, int which, Interval& valid = Interval(0,0));
	
	// TDist funcs needs implementation  as of 31/5/01
	void SetTDist(TimeValue time, float f); 
	float GetTDist(TimeValue t, Interval& valid = Interval(0,0));

	void SetConeDisplay(int s, int notify=TRUE);
	BOOL GetConeDisplay(void);

	void SetRGBColor(TimeValue t, Point3& rgb); //fLightPB->SetValue(kRGB, t, rgb); NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
	Point3 GetRGBColor(TimeValue t, Interval &valid = Interval(0,0)); //return fLightPB->GetPoint3(kRGB, t); }        
	void SetIntensity(TimeValue t, float f) { fLightPB->SetValue(kIntensity, t, f); NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
	float GetIntensity(TimeValue t, Interval& valid = Interval(0,0)) {return fLightPB->GetFloat(kIntensity, t); }
	void SetAspect(TimeValue t, float f) {}
	float GetAspect(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0; }    
	void SetUseAtten(int s){ fLightPB->SetValue(kUseAttenuationBool, 0, s); NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE); }
	BOOL GetUseAtten(void) {return fLightPB->GetInt(kUseAttenuationBool, 0);}
	void SetUseAttenNear(int s) {  }
	BOOL GetUseAttenNear(void) {return false;}
	void SetAttenDisplay(int s){  }
	BOOL GetAttenDisplay(void) {return fLightPB->GetInt(kUseAttenuationBool, 0);}      
	void SetAttenDisplayNear(int s){ }
	BOOL GetAttenDisplayNear(void){return false;}      
	void Enable(int enab) { fLightPB->SetValue(kLightOn, 0, enab); }
	void SetMapBias(TimeValue t, float f) {}
	float GetMapBias(TimeValue t, Interval& valid = Interval(0,0)){return 0.0f;}
	void SetMapRange(TimeValue t, float f) {}
	float GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	void SetMapSize(TimeValue t, int f) {}
	int GetMapSize(TimeValue t, Interval& valid = Interval(0,0)){return 0;}
	void SetRayBias(TimeValue t, float f) {}
	float GetRayBias(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0;} //{return 0.0f;}
	int GetAbsMapBias() {return 0;}
	void SetAbsMapBias(int a) {}
	int GetOvershoot(){return 0;}
	void SetOvershoot(int a){}
	virtual int GetProjector() { return 0; }
	void SetProjector(int a){fLightPB->SetValue(kUseProjectorBool, 0, a); }
	ExclList* GetExclList(){return NULL;}
	BOOL Include() {return false;}
	virtual Texmap* GetProjMap(); //{ Interval valid = Interval(0,0); Texmap* MyMap; fLightPB->GetValue(kProjMapTexButton, 0, MyMap, valid); return MyMap; }
	void SetProjMap(BitmapInfo* pmap);
	void UpdateTargDistance(TimeValue t, INode* inode);

	// GenLight Specific Stuff below

	BOOL GetUseGlobal() { return false; }	//Global Shadow param
	void SetUseGlobal(int a) {}
	BOOL GetShadow();
	void SetShadow(int a);
	BOOL GetShadowType() { return -1; }	//No Shadows generated ....
	void SetShadowType(int a) {}		//Until implemented....
	GenLight* NewLight(int type) { return NULL;} 
	int Type()	{return -1;}
	void SetSpotShape(int a) {}
	void SetHSVColor(TimeValue t, class Point3 &b);
	Point3 GetHSVColor(TimeValue t, Interval& b);
	void SetContrast(int a, float other) {} // fLightPB->SetValue(kContrast, a, other); NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);}
	float GetContrast(int a, Interval &valid) {return 0.0;} //float f; f = fLightPB->GetFloat(kContrast, a); return f;}
	void SetAttenNearDisplay(int a) {}
	BOOL GetAttenNearDisplay() {return false;}
	ExclList& GetExclusionList() {return exclList;}
	void SetExclusionList(ExclList & a) {}
	BOOL SetHotSpotControl(Control* a) {fLightPB->SetController(ParamID(kHotSpot),0, a); return true;}
	BOOL SetFalloffControl(Control* a) {fLightPB->SetController(ParamID(kFallOff),0, a); return true;}
	Control* GetHotSpotControl() {return fLightPB->GetController(ParamID(kHotSpot));}
	Control* GetFalloffControl() {return fLightPB->GetController(ParamID(kFallOff));}
	BOOL SetColorControl(Control * a) {fLightPB->SetController(ParamID(kLightColor),0, a); return true;}
	Control* GetColorControl() {return fLightPB->GetController(ParamID(kLightColor)); }

	BOOL GetDecayType() { return fLightPB->GetInt(kAttenTypeRadio, 0) + 1;} //Offset for the radio.
	void SetDecayType(BOOL onOff) {if (!onOff) return; else {fLightPB->SetValue(kAttenTypeRadio, 0, ((int) onOff - 1)); return;} } 
	void SetDecayRadius(TimeValue time, float f) { fLightPB->SetValue(kAttenMaxFalloffEdit, time, f); }
	float GetDecayRadius(TimeValue t, Interval& valid = Interval(0,0)) {return fLightPB->GetFloat(kAttenMaxFalloffEdit, t); }

	void SetDiffuseSoft(TimeValue time, float f) {}//fLightPB->SetValue(kDiffSoft, time, f);}
	float GetDiffuseSoft(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0;} //return fLightPB->GetFloat(kDiffSoft, t);}
	void SetAffectDiffuse(BOOL onOff) {}//fLightPB->SetValue(kDiffOn, 0, onOff); }
	BOOL GetAffectDiffuse() {return false; } //fLightPB->GetInt(kDiffOn, 0); }

	LRESULT CALLBACK TrackViewWinProc( HWND hwnd,  UINT message, 
            WPARAM wParam,   LPARAM lParam ){return(0);}
};

#endif
