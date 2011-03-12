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
#include "plRealTimeLightBase.h"
#include "iparamm2.h"
#include "resource.h"
#include "decomp.h"
#include "hsv.h"
#include "target.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"


static int GetTargetPoint(TimeValue t, INode *inode, Point3& p) 
{
	Matrix3 tmat;
	if (inode->GetTargetTM(t,tmat)) 
	{
		p = tmat.GetTrans();
		return 1;
	}
	else 
		return 0;
}

///////////////////////////////////////////////////////////////////////////////
//// plBaseLightProc Functions ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//	This class provides the base functionality for all the dialog procs for
//	the ParamBlock rollouts for each light

void	plBaseLightProc::ILoadComboBox( HWND hComboBox, const char *names[] )
{
	SendMessage(hComboBox, CB_RESETCONTENT, 0, 0);
	for (int i = 0; names[i]; i++)
		SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM)names[i]);
	SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
}

void	plBaseLightProc::IBuildLightMesh( plRTLightBase *base, float coneSize )
{
	base->BuildSpotMesh( coneSize );
	base->fMesh = base->spotMesh;
}

BOOL	plBaseLightProc::DlgProc( TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	IParamBlock2	*pb = map->GetParamBlock();
	plRTLightBase	*gl = (plRTLightBase *) pb->GetOwner();


	switch( msg )
	{
		case WM_INITDIALOG:
			{
				HWND DecayTypeCombo = GetDlgItem(hWnd, IDC_LIGHT_DECAY);
				HWND ShadowStateCombo	= GetDlgItem(hWnd, IDC_SHADOW_TYPE);
				
				map->Invalidate(plRTLightBase::kProjMapTexButton);
			}
			break;

		case WM_COMMAND:
			break;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//// plLightTexPBAccessor Class Functions /////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plLightTexPBAccessor	plLightTexPBAccessor::fAccessor;

void	plLightTexPBAccessor::Set( PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
{
	plRTLightBase* Obj = (plRTLightBase*)owner;
	IParamBlock2 *pb = Obj->fLightPB;
	
	switch (id)
	{
		case plRTLightBase::kProjMapTexButton:
			if (1) //(val.bm)->bm)
			{
				if (pb->GetMap())
					pb->GetMap()->Invalidate(plRTLightBase::kProjMapTexButton);
				//pb->SetValue(plRTLightBase::kProjMapTexButton, Obj->fIP->GetTime(), val.bm);
				PBBitmap* ThisMap = val.bm;
				Obj->SetProjMap(&ThisMap->bi);
			break;
			}
		default:
			break;

	}
}

void	plLightTexPBAccessor::Get( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid )
{
}


////////////////////////////////////////////////////////////
//
// Mouse creation callback class for this plug-in

class RTLightMouseCallBack : public CreateMouseCallBack
{
	plRTLightBase *ob;
public:
	int proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3 &mat);
	void SetObj(plRTLightBase *obj) { ob = obj; }
};
static RTLightMouseCallBack gRTMouseCallback;


int RTLightMouseCallBack::proc(ViewExp *vpt, int msg, int point, int flags, IPoint2 m, Matrix3 &mat)
{
	switch (msg)
	{
	case MOUSE_POINT:
	case MOUSE_MOVE:
		switch (point)
		{
		case 0:
			mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			break;

		case 1:
			mat.SetTrans(vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			if (msg == MOUSE_POINT)
				return 0;
			break;			
		}
		break;

	case MOUSE_ABORT:
		return CREATE_ABORT;
	}

	return CREATE_CONTINUE;
}

static RTLightMouseCallBack sRTBaseLgtCreateCB;

CreateMouseCallBack* plRTLightBase::GetCreateMouseCallBack() 
{
	sRTBaseLgtCreateCB.SetObj(this);

	return &sRTBaseLgtCreateCB;
}
/////////////////////////////////////////////////////////////////////////////

#if 0
plRTLightBase::plRTLightBase() : fIP(nil), fClassDesc(nil), fLightPB(nil)
{
	fColor = Color(0.5f, 0.5f, 1.f);
}
#endif



void plRTLightBase::SetHSVColor(TimeValue t, Point3 &hsv)
{
	DWORD rgb = HSVtoRGB ((int)(hsv[0]*255.0f), (int)(hsv[1]*255.0f), (int)(hsv[2]*255.0f));
	Point3 temp = fLightPB->GetPoint3(kLightColor, t);
	Point3 rgbf;
	rgbf[0] = temp.x / 255.0f;
	rgbf[1] = temp.y / 255.0f;
	rgbf[2] = temp.z / 255.0f;
	SetRGBColor(t, rgbf);
}


Point3 plRTLightBase::GetHSVColor(TimeValue t, Interval& b)
{
	int h, s, v;
	Point3 rgbf = GetRGBColor(t, b);
	DWORD rgb = RGB((int)(rgbf[0]*255.0f), (int)(rgbf[1]*255.0f), (int)(rgbf[2]*255.0f));
	RGBtoHSV (rgb, &h, &s, &v);
	return Point3(h/255.0f, s/255.0f, v/255.0f);


}

#define FZ (0.0f)

#define SET_QUAD(face, v0, v1, v2, v3) \
	staticMesh[RT_OMNI +1].faces[face].setVerts(v0, v1, v2); \
	staticMesh[RT_OMNI +1].faces[face].setEdgeVisFlags(1,1,0); \
	staticMesh[RT_OMNI +1].faces[face+1].setVerts(v0, v2, v3); \
	staticMesh[RT_OMNI +1].faces[face+1].setEdgeVisFlags(0,1,1);

void plRTLightBase::BuildStaticMeshes()
{
	if(!meshBuilt) {
		int nverts = 6;
		int nfaces = 8;
		// Build a leetle octahedron
		staticMesh[RT_OMNI].setNumVerts(nverts);
		staticMesh[RT_OMNI].setNumFaces(nfaces);
		float s = 8.0f;
		staticMesh[RT_OMNI].setVert(0, Point3( FZ,FZ, -s));
		staticMesh[RT_OMNI].setVert(1, Point3( s, FZ, FZ));
		staticMesh[RT_OMNI].setVert(2, Point3( FZ, s, FZ));
		staticMesh[RT_OMNI].setVert(3, Point3(-s, FZ, FZ));
		staticMesh[RT_OMNI].setVert(4, Point3( FZ,-s, FZ));
		staticMesh[RT_OMNI].setVert(5, Point3( FZ,FZ,  s));
		staticMesh[RT_OMNI].faces[0].setVerts(0,1,4);
		staticMesh[RT_OMNI].faces[1].setVerts(0,4,3);
		staticMesh[RT_OMNI].faces[2].setVerts(0,3,2);
		staticMesh[RT_OMNI].faces[3].setVerts(0,2,1);
		staticMesh[RT_OMNI].faces[4].setVerts(5,1,2);
		staticMesh[RT_OMNI].faces[5].setVerts(5,2,3);
		staticMesh[RT_OMNI].faces[6].setVerts(5,3,4);
		staticMesh[RT_OMNI].faces[7].setVerts(5,4,1);
		for (int i=0; i<nfaces; i++) {
			staticMesh[RT_OMNI].faces[i].setSmGroup(i);
			staticMesh[RT_OMNI].faces[i].setEdgeVisFlags(1,1,1);
			}
		staticMesh[RT_OMNI].buildNormals();
		staticMesh[RT_OMNI].EnableEdgeList(1);

		// Build an "arrow"
		nverts = 13;
		nfaces = 16;
		staticMesh[RT_OMNI+1].setNumVerts(nverts);
		staticMesh[RT_OMNI+1].setNumFaces(nfaces);
		s = 4.0f;
		float s4 = 16.0f;
		staticMesh[RT_OMNI+1].setVert( 0, Point3( -s,-s, FZ));
		staticMesh[RT_OMNI+1].setVert( 1, Point3(  s,-s, FZ));
		staticMesh[RT_OMNI+1].setVert( 2, Point3(  s, s, FZ));
		staticMesh[RT_OMNI+1].setVert( 3, Point3( -s, s, FZ));
		staticMesh[RT_OMNI+1].setVert( 4, Point3( -s,-s, -s4));
		staticMesh[RT_OMNI+1].setVert( 5, Point3(  s,-s, -s4));
		staticMesh[RT_OMNI+1].setVert( 6, Point3(  s, s, -s4));
		staticMesh[RT_OMNI+1].setVert( 7, Point3( -s, s, -s4));
		s *= (float)2.0;
		staticMesh[RT_OMNI+1].setVert( 8, Point3( -s,-s, -s4));
		staticMesh[RT_OMNI+1].setVert( 9, Point3(  s,-s, -s4));
		staticMesh[RT_OMNI+1].setVert(10, Point3(  s, s, -s4));
		staticMesh[RT_OMNI+1].setVert(11, Point3( -s, s, -s4));
		staticMesh[RT_OMNI+1].setVert(12, Point3( FZ,FZ, -s4-s));
		SET_QUAD( 0, 1, 0, 4, 5);
		SET_QUAD( 2, 0, 3, 7, 4);
		SET_QUAD( 4, 3, 2, 6, 7);
		SET_QUAD( 6, 2, 1, 5, 6);
		SET_QUAD( 8, 0, 1, 2, 3);
		SET_QUAD(10, 8, 9, 10, 11);
		staticMesh[RT_OMNI+1].faces[12].setVerts(8,12,9);
		staticMesh[RT_OMNI+1].faces[12].setEdgeVisFlags(1,1,1);
		staticMesh[RT_OMNI+1].faces[13].setVerts(9,12,10);
		staticMesh[RT_OMNI+1].faces[13].setEdgeVisFlags(1,1,1);
		staticMesh[RT_OMNI+1].faces[14].setVerts(10,12,11);
		staticMesh[RT_OMNI+1].faces[14].setEdgeVisFlags(1,1,1);
		staticMesh[RT_OMNI+1].faces[15].setVerts(11,12,8);
		staticMesh[RT_OMNI+1].faces[15].setEdgeVisFlags(1,1,1);
		for (int i=0; i<nfaces; i++)
			staticMesh[RT_OMNI+1].faces[i].setSmGroup(i);
		staticMesh[RT_OMNI+1].buildNormals();
		staticMesh[RT_OMNI+1].EnableEdgeList(1);

		meshBuilt = 1;
	}
}


void plRTLightBase::BuildSpotMesh(float coneSize)
{
	// build a cone
	if(coneSize < 0.0f)
		return;
	int nverts = 9;
	int nfaces = 8;
	spotMesh.setNumVerts(nverts);
	spotMesh.setNumFaces(nfaces);
	double radang = 3.1415926 * coneSize / 360.0;
	float h = 20.0f;					// hypotenuse
	float d = -h * (float)cos(radang);	// dist from origin to cone circle
	float r = h * (float)sin(radang);	// radius of cone circle
	float s = 0.70711f * r;  			// sin(45) * r
	spotMesh.setVert(0, Point3( FZ, FZ, FZ));
	spotMesh.setVert(1, Point3( -r, FZ, d));
	spotMesh.setVert(2, Point3( -s, -s, d));
	spotMesh.setVert(3, Point3( FZ, -r, d));
	spotMesh.setVert(4, Point3(  s, -s, d));
	spotMesh.setVert(5, Point3(  r, FZ, d));
	spotMesh.setVert(6, Point3(  s,  s, d));
	spotMesh.setVert(7, Point3( FZ,  r, d));
	spotMesh.setVert(8, Point3( -s,  s, d));
	spotMesh.faces[0].setVerts(0,1,2);
	spotMesh.faces[1].setVerts(0,2,3);
	spotMesh.faces[2].setVerts(0,3,4);
	spotMesh.faces[3].setVerts(0,4,5);
	spotMesh.faces[4].setVerts(0,5,6);
	spotMesh.faces[5].setVerts(0,6,7);
	spotMesh.faces[6].setVerts(0,7,8);
	spotMesh.faces[7].setVerts(0,8,1);
	for (int i=0; i<nfaces; i++) {
		spotMesh.faces[i].setSmGroup(i);
		spotMesh.faces[i].setEdgeVisFlags(1,1,1);
	}
	spotMesh.buildNormals();
	spotMesh.EnableEdgeList(1);
	//fMesh = spotMesh;
}

plRTLightBase::~plRTLightBase()
{
	DeleteAllRefsFromMe();
	fLightPB = NULL;
	if( fTex )
		delete fTex;
}

IParamBlock2* plRTLightBase::GetParamBlock2()
{
	return fLightPB; 
}

IParamBlock2* plRTLightBase::GetParamBlockByID( short id )
{
	if( id == fLightPB->ID() )
		return fLightPB; 
	else 
		return nil;
}

IParamBlock2	*plRTLightBase::GetParamBlock( int i )
{
	switch( i )
	{
		case 0: return fLightPB;
		default: return nil;
	}
}


// So our animatables will show up in the trackview
int plRTLightBase::NumSubs()
{
	return 1;
}
Animatable *plRTLightBase::SubAnim(int i)
{
	return (Animatable *)fLightPB;

	switch(i)
	{			
		/*	kRefProjMap,
		kRefShadowProjMap,
		kRefShadowType,
		kRefOmniLight,
		kRefSpotLight,
		kRefTSpotLight,
		kRefDirLight,
		kRefTDirLight
		*/
		case kRefOmniLight:
		case kRefSpotLight:
		case kRefTSpotLight:
		case kRefDirLight:
		case kRefTDirLight:
		case kRefProjDirLight:
			return (Animatable*)fLightPB;
		case kRefProjMap: 
			Texmap* MyMap;
			return (Animatable*) fLightPB->GetValue(kProjMapTexButton, 0, MyMap, FOREVER);			
		case kRefShadowType: 
			return NULL;
		default: return NULL;

	}

	//return (Animatable*) fLightPB;
}

TSTR plRTLightBase::SubAnimName(int i) 
{ 
	return fLightPB->GetLocalName();
		switch(i) 
		{	
		
		case kRefOmniLight:return _T("");
		case kRefSpotLight: return TSTR(GetString(IDS_DB_FSPOT));
		case kRefTSpotLight:return _T("");
		case kRefDirLight:return _T("");
		case kRefTDirLight:return _T("");
		case kRefProjMap: return TSTR(GetString(IDS_DS_PROJMAP));			
		case kRefShadowType: return _T("");
			default: return _T("");
	
/*			case PBLOCK_REF: return TSTR(GetString(IDS_RB_PARAMETERS));
			case PROJMAP_REF: return TSTR(GetString(IDS_DS_PROJMAP));
			case SHADPROJMAP_REF: return TSTR(GetString(IDS_DS_SHADPROJMAP));
			case SHADTYPE_REF: return TSTR(GetString(IDS_DS_SHAD_GEN));
			case EMITTER_REF: 
				if ( IsCompatibleRenderer ())
					return TSTR(GetString(IDS_EMITTER));
				else
					return _T("");
			default: return _T("");
*/
		}
}

#if 0 
RefTargetHandle plRTSpotLight::Clone(RemapDir &remap)
{
	plRTLightBase *obj = TRACKED_NEW plRTSpotLight;

	obj->GetParamBlock2->SetValue(kLightOn, t, fLightPB->GetInt(kLightOn, t));
//	obj->fLightPB->SetValue(kLightType, t, fLightPB->GetInt(kLightType, t));
	obj->fLightPB->SetValue(kLightColor, t, fLightPB->GetInt(kLightOn, t));
	obj->fLightPB->SetValue(kCastShadows, t, fLightPB->GetInt(kLightOn, t));
	//obj->fLightPB->SetValue(kContrast, t, fLightPB->GetInt(kLightOn, t));
	//obj->fLightPB->SetValue(kDiffSoft, t, fLightPB->GetInt(kLightOn, t));
	//obj->fLightPB->SetValue(kDiffOn, t, fLightPB->GetInt(kLightOn, t));
	obj->fLightPB->SetValue(kSpec, t, fLightPB->GetInt(kLightOn, t));	
	obj->fLightPB->SetValue(kSpecularColorSwatch, t, fLightPB->GetInt(kLightOn, t));
	obj->fLightPB->SetValue(kIntensity, t, fLightPB->GetInt(kLightOn, t));
	
	if( IHasAttenuation() )
	{
		obj->fLightPB->SetValue(kUseAttenuationBool, t, fLightPB->GetInt(kLightType, t));
		obj->fLightPB->SetValue(kAttenMaxFalloffEdit, t, fLightPB->GetInt(kLightOn, t));
		obj->fLightPB->SetValue(kAttenTypeRadio, t, fLightPB->GetInt(kLightOn, t));
		obj->fLightPB->SetValue(kShowConeBool, t, fLightPB->GetInt(kLightOn, t));
		obj->fLightPB->SetValue(kHotSpot, t, fLightPB->GetInt(kLightOn, t));
		obj->fLightPB->SetValue(kAttenMaxFalloffEdit, fLightPB->GetInt(kLightType, t));
		obj->fLightPB->SetValue(kFallOff, t, fLightPB->GetInt(kLightOn, t));

	obj->fLightPB->SetValue(kUseProjectorBool, t, fLightPB->GetInt(kLightOn, t));
	obj->fLightPB->SetValue(kProjMapTexButton, t, fLightPB->GetInt(kLightOn, t));
	}
	obj->ReplaceReference(kRefSpotLight,fLightPB->Clone(remap));
	/*
		GeneralLight* newob = TRACKED_NEW GeneralLight(type);
	newob->enable = enable;
	newob->coneDisplay = coneDisplay;
	newob->useLight = useLight;
	newob->attenDisplay = attenDisplay;
	newob->useAtten = useAtten;
	newob->useAttenNear = useAttenNear;
	newob->attenNearDisplay = attenNearDisplay;
	newob->decayDisplay = decayDisplay;
	newob->shape = shape;
	newob->shadow = shadow;
	newob->shadowType = shadowType;
	newob->overshoot = overshoot;
	newob->projector = projector;
	newob->absMapBias = absMapBias;
	newob->exclList = exclList;
	newob->softenDiffuse = softenDiffuse;
	newob->affectDiffuse = affectDiffuse;
	newob->affectSpecular = affectSpecular;
	newob->ambientOnly = ambientOnly;
	newob->decayType = decayType;
	newob->atmosShadows = atmosShadows;
	newob->atmosOpacity = atmosOpacity;
	newob->atmosColAmt = atmosColAmt;
	newob->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	if (projMap)     newob->ReplaceReference(PROJMAP_REF,projMap->Clone(remap));
	if (shadProjMap) newob->ReplaceReference(SHADPROJMAP_REF,shadProjMap->Clone(remap));
	if (shadType)    newob->ReplaceReference(SHADTYPE_REF,shadType->Clone(remap));
	if (emitter)     newob->ReplaceReference(EMITTER_REF ,emitter->Clone(remap));
	BaseClone(this, newob, remap);
	return(newob);
	*/

	//plRTLightBase *obj = (plRTLightBase*) fClassDesc->Create(false);
	//obj->ReplaceReference(kRefComp, fLightPB->Clone(remap));
	return obj;
}
#endif


void plRTLightBase::FreeCaches()
{
//	fMesh.FreeAll();
}



void plRTLightBase::BoxCircle(TimeValue t, float r, float d, Box3& box, int extraPt, Matrix3 *tm) 
{
	Point3 q[3*NUM_CIRC_PTS];
	int npts;
	float asp;
	if ( 1 /*Circle Object*/) { 	npts =  NUM_CIRC_PTS+extraPt; 	asp = -1.0f; }
	else { npts = 4+extraPt;  asp = -1.0; } 
 	GetConePoints(t, asp , r, d, q);
 	box.IncludePoints(q,npts,tm);
}

void plRTLightBase::BoxDirPoints(TimeValue t, float angle, float dist, Box3 &box, Matrix3 *tm)
 {
	int npts;
	Point3 q[3*NUM_CIRC_PTS];
	npts = 1 /*Circle Object*/? GetCirXPoints(t,angle,dist,q): GetRectXPoints(t,angle,dist,q);
	box.IncludePoints(q,npts,tm);
}


void plRTLightBase::BoxPoints(TimeValue t, float angle, float dist, Box3 &box, Matrix3 *tm) 
{
	if (IsDir())
		BoxCircle(t, angle, dist, box, 0,tm);
	else 
		BoxDirPoints(t, angle, dist, box, tm);
}


void plRTLightBase::BoxLight(TimeValue t, INode *inode, Box3& box, Matrix3 *tm) 
{
	Point3 pt;
	float d;
	if (GetTargetPoint(t, inode, pt)) 
	{
		Point3 loc = inode->GetObjectTM(t).GetTrans();
		d = FLength(loc - pt) / FLength(inode->GetObjectTM(t).GetRow(2));
		box += tm? (*tm)*Point3(0.0f, 0.0f, -d): Point3(0.0f, 0.0f, -d);
	}
	else 
	{
		d = GetTDist(t);
	//	if(fLightPB->GetInt(kLightType) == RT_FREE_DIR)
	//		d = GetFallsize(t);
		if(IsSpot())
				box += tm? (*tm)*Point3(0.0f, 0.0f, -d): Point3(0.0f, 0.0f, -d);
		if(IsDir())
		{
			d = GetFallsize(t);
			box += tm? (*tm)*Point3(0.0f, 0.0f, -d): Point3(0.0f, 0.0f, -d);

		}
	}
	if( this->ClassID() == RTSPOT_LIGHT_CLASSID )
//	if	(fLightPB->GetInt(kLightType) == RT_FREE_SPOT || fLightPB->GetInt(kLightType) == RT_TARGET_SPOT)
		if((fLightPB->GetInt(kShowConeBool,t)) || (extDispFlags & EXT_DISP_ONLY_SELECTED)) 
		{
			float rad = MaxF(GetHotspot(t), GetFallsize(t));
			if (IsDir()) 
				BoxCircle(t,rad,0.0f,box,1,tm);
			BoxCircle(t,rad,d,box,1,tm);
		}
	if( this->ClassID() == RTDIR_LIGHT_CLASSID 
		|| this->ClassID() == RTPDIR_LIGHT_CLASSID )
//	if	(fLightPB->GetInt(kLightType) == RT_FREE_DIR || fLightPB->GetInt(kLightType) == RT_TARGET_DIR)
		if((extDispFlags & EXT_DISP_ONLY_SELECTED)) 
		{
			float rad = MaxF(GetHotspot(t), GetFallsize(t));
			if (IsDir()) 
				BoxCircle(t,rad,0.0f,box,1,tm);
			BoxCircle(t,rad,2.82841*GetFallsize(t),box,1,tm);	//hack, hack.  Do 2root2 at corners...
		}
	BOOL dispAtten = false;
	BOOL dispAttenNear = false;
	BOOL dispDecay = false;
	if( this->ClassID() == RTOMNI_LIGHT_CLASSID || this->ClassID() == RTSPOT_LIGHT_CLASSID )
	{
		dispAtten = fLightPB->GetInt(kUseAttenuationBool,t);
		dispAttenNear = 0; //attenNearDisplay;

		dispDecay = (GetDecayType()&&(extDispFlags & EXT_DISP_ONLY_SELECTED));
	}
	if( dispAtten || dispDecay) 
	{
		if( this->ClassID() == RTOMNI_LIGHT_CLASSID )
		{ 
			Point3 q[3*NUM_CIRC_PTS];
			float rad = 0;
			if (dispAtten) 
				rad = MaxF(GetAtten(t, ATTEN_START), GetAtten(t, ATTEN_END));
			if (dispDecay) rad = MaxF(rad,0.0/*GetDecayRadius(t)*/);
			GetAttenPoints(t, rad, q);
			box.IncludePoints(q,3*NUM_CIRC_PTS,tm);
		}
		if( this->ClassID() == RTSPOT_LIGHT_CLASSID )
		{
			if (dispAtten) 
			{
				BoxPoints(t, GetFallsize(t), GetAtten(t,ATTEN_END), box, tm);
				BoxPoints(t, GetFallsize(t), GetAtten(t,ATTEN_START), box, tm);
			}
			if (dispDecay) 
				BoxPoints(t, GetFallsize(t), 0.0/*GetDecayRadius(t)*/, box, tm);
		}
	}

}

int plRTLightBase::GetRectXPoints(TimeValue t, float angle, float dist, Point3 *q) 
{
	int i;
	if(dist==0.0f) dist = .00001f;
	float ang = DegToRad(angle)/2.0f;
	float da,sn,cs,x,y,z,a;
	float aspect = GetAspect(t);
	float w = dist * (float)tan(ang) * (float)sqrt((double)aspect);
	float h = w/aspect;
	float wang = (float)atan(w/dist);
	float hang = (float)atan(h/dist);
	float aw = float(atan(w/dist)*cos(hang));  // half-angle of top and bottom arcs
	float ah = float(atan(h/dist)*cos(wang));  // half-angle of left and right arcs
	int j = 0;

	// draw horizontal and vertical center lines
	da = wang/float(NUM_HALF_ARC);
	for(i = -NUM_HALF_ARC, a = -wang; i<= NUM_HALF_ARC; i++, a+=da) 
		q[j++] = Point3(dist*(float)sin(a), 0.0f, -dist*(float)cos(a));
	da = hang/float(NUM_HALF_ARC);
	for(i = -NUM_HALF_ARC, a = -hang; i<= NUM_HALF_ARC; i++, a+=da) 
		q[j++] = Point3(0.0f, dist*(float)sin(a), -dist*(float)cos(a));


	// draw top and bottom arcs
	da = aw/float(NUM_HALF_ARC);
	sn = (float)sin(hang);
	cs = (float)cos(hang);
	for (i = -NUM_HALF_ARC, a = -aw; i<= NUM_HALF_ARC; i++, a+=da) 
	{
		x =  dist*(float)sin(a); 
		z = -dist*(float)cos(a);
		q[j]             = Point3(x, z*sn, z*cs);  				
		q[j+NUM_ARC_PTS] = Point3(x,-z*sn, z*cs);  				
		j++;
	}
	
	j+= NUM_ARC_PTS;

	// draw left and right arcs
	da = ah/float(NUM_HALF_ARC);
	sn = (float)sin(wang);
	cs = (float)cos(wang);
	for (i = -NUM_HALF_ARC, a = -ah; i<= NUM_HALF_ARC; i++, a+=da)
	{
		y =  dist*(float)sin(a); 
		z = -dist*(float)cos(a);
		q[j]             = Point3( z*sn, y, z*cs);  				
		q[j+NUM_ARC_PTS] = Point3(-z*sn, y, z*cs);  				
		j++;
	}

	return 6*NUM_ARC_PTS;
}

int plRTLightBase::GetCirXPoints(TimeValue t, float angle, float dist, Point3 *q) 
{
	int i;
	float ang = DegToRad(angle)/2.0f;
	float da = ang/float(NUM_HALF_ARC);
	// first draw circle:
	float d = dist*(float)cos(ang);
	GetConePoints(t, -1.0f, angle, d, q);
	int j=NUM_CIRC_PTS;
	// then draw Arc X
	float a = -ang;
	for(i = -NUM_HALF_ARC; i<= NUM_HALF_ARC; i++, a+=da) 
		q[j++] = Point3(0.0f, dist*(float)sin(a), -dist*(float)cos(a));
	a = -ang;	
	for(i = -NUM_HALF_ARC; i<= NUM_HALF_ARC; i++, a+=da) 
		q[j++] = Point3(dist*(float)sin(a), 0.0f, -dist*(float)cos(a));
	return NUM_CIRC_PTS + 2*NUM_ARC_PTS;
}

void plRTLightBase::DrawX(TimeValue t, float asp, int npts, float dist, GraphicsWindow *gw, int indx) 
{
	Point3 q[3*NUM_CIRC_PTS+1];
	Point3 u[2];
	GetConePoints(t, asp, GetFallsize(t), dist, q);
	gw->polyline(npts, q,NULL, NULL, TRUE, NULL);
	u[0] = q[0]; u[1] = q[2*indx];
	gw->polyline(2, u,NULL, NULL, FALSE, NULL);
	u[0] = q[indx]; u[1] = q[3*indx];
	gw->polyline(2, u,NULL, NULL, FALSE, NULL);
}



void plRTLightBase::GetAttenPoints(TimeValue t, float rad, Point3 *q)
{
	double a;
	float sn, cs;
	for(int i = 0; i < NUM_CIRC_PTS; i++) 
	{
		a = (double)i * 2.0 * 3.1415926 / (double)NUM_CIRC_PTS;
		sn = rad * (float)sin(a);
		cs = rad * (float)cos(a);
		q[i+0*NUM_CIRC_PTS] = Point3(sn, cs, 0.0f);
		q[i+1*NUM_CIRC_PTS] = Point3(sn, 0.0f, cs);
		q[i+2*NUM_CIRC_PTS] = Point3(0.0f, sn, cs);
	}
}


// Draw warped rectangle
void plRTLightBase::DrawWarpRect(TimeValue t, GraphicsWindow *gw, float angle, float dist, Point3 *q) 
{
 	GetRectXPoints(t, angle,dist,q);
	for (int i=0; i<6; i++)
		gw->polyline(NUM_ARC_PTS, q+i*NUM_ARC_PTS,NULL, NULL, FALSE, NULL);  
}

void plRTLightBase::DrawCircleX(TimeValue t, GraphicsWindow *gw, float angle, float dist, Point3 *q) 
{
 	GetCirXPoints(t, angle,dist,q);
	gw->polyline(NUM_CIRC_PTS, q,NULL, NULL, TRUE, NULL);  // circle 
	gw->polyline(NUM_ARC_PTS, q+NUM_CIRC_PTS,NULL, NULL, FALSE, NULL); // vert arc
	gw->polyline(NUM_ARC_PTS, q+NUM_CIRC_PTS+NUM_ARC_PTS,NULL, NULL, FALSE, NULL);  // horiz arc
}

void plRTLightBase::DrawSphereArcs(TimeValue t, GraphicsWindow *gw, float r, Point3 *q) 
{
	GetAttenPoints(t, r, q);
	gw->polyline(NUM_CIRC_PTS, q,				NULL, NULL, TRUE, NULL);
	gw->polyline(NUM_CIRC_PTS, q+NUM_CIRC_PTS,	NULL, NULL, TRUE, NULL);
	gw->polyline(NUM_CIRC_PTS, q+2*NUM_CIRC_PTS,NULL, NULL, TRUE, NULL);
}

//

void plRTLightBase::DrawAttenCirOrRect(TimeValue t, GraphicsWindow *gw, float dist, BOOL froze, int uicol) 
{
	if (!froze) gw->setColor( LINE_COLOR, GetUIColor(uicol));
	if (IsDir()) 
	{
		int npts,indx;
		float asp;
		npts = NUM_CIRC_PTS; 	asp  = -1.0f; 	indx = SEG_INDEX;
		DrawX(t, asp, npts, dist, gw, indx);
	}
	else 
	{
		Point3 q[3*NUM_CIRC_PTS+1];
		if( this->ClassID() == RTOMNI_LIGHT_CLASSID )
			DrawSphereArcs(t, gw, dist, q);
		else 	
			DrawCircleX(t, gw, GetFallsize(t),dist,q);
		
	}
}


int plRTLightBase::DrawAtten(TimeValue t, INode *inode, GraphicsWindow *gw)
{
	BOOL dispAtten = false;
	BOOL dispDecay = false;
	if( this->ClassID() == RTOMNI_LIGHT_CLASSID || this->ClassID() == RTSPOT_LIGHT_CLASSID )
	{
		dispAtten = (fLightPB->GetInt(kUseAttenuationBool,t) && (extDispFlags & EXT_DISP_ONLY_SELECTED));
		//BOOL dispAttenNear = (fLightPB->GetInt(kUseNearAtten) && (extDispFlags & EXT_DISP_ONLY_SELECTED))?TRUE:fLightPB->GetInt(kShowFarAttenRanges);
		dispDecay = (GetDecayType() && (extDispFlags & EXT_DISP_ONLY_SELECTED));
	}
	if (dispAtten || dispDecay) 
	{
		Matrix3 tm = inode->GetObjectTM(t);
		gw->setTransform(tm);
		BOOL froze = inode->IsFrozen() && !inode->Dependent();
	 	if (dispAtten) 
		{
	 		DrawAttenCirOrRect(t, gw, GetAtten(t,ATTEN_START), froze, COLOR_START_RANGE);
	 		DrawAttenCirOrRect(t, gw, GetAtten(t,ATTEN_END), froze, COLOR_END_RANGE);
	 	}
		
		if (dispDecay) 
		{
			DrawAttenCirOrRect(t, gw, 0.0 /*DecayRadius() Stuff here */, froze, COLOR_DECAY_RADIUS);
		}
	}
	return 0;
}

void plRTLightBase::GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box)
{
	//BuildMeshes(t);
	//box = fMesh.getBoundingBox();
//	int nv;
//	Matrix3 tm;
//	GetMat(t, node,vpt,tm);
//	Point3 loc = tm.GetTrans();
//	nv = fMesh.getNumVerts();
//	box.Init();
//	if(!(extDispFlags & EXT_DISP_ZOOM_EXT)) 
//		box.IncludePoints(fMesh.verts,nv,&tm);
//	else
//		box += loc;
//	tm = node->GetObjectTM(t);
//	BoxLight(t, node, box, &tm);
	Point3 loc = node->GetObjectTM(t).GetTrans();
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(loc) / 360.0f;
	box = fMesh.getBoundingBox();
	box.Scale(scaleFactor);
	BoxLight(t, node, box, NULL);

}



/*
RefTargetHandle Clone(RemapDir &remap = NoRemap())
{

	plRTLightBase* newOb = TRACKED_NEW plRTLightBase;
	newOb->fLightPB->SetValue(kLightOn, 0, this->fLightPB->GetInt(kLightOn, 0));
	newOb->fLightPB->SetValue(kLightColor, 0, this->fLightPB->GetValue(kLightColor, 0));
	newOb->fLightPB->SetValue(kLightExclude, 0, this->fLightPB->GetValue(kLightExclude, 0));
	newOb->fLightPB->SetValue(kRed, 0, this->fLightPB->GetInt(kRed, 0));
	newOb->fLightPB->SetValue(kGreen, 0, this->fLightPB->GetInt(kGreen, 0));
	newOb->fLightPB->SetValue(kBlue, 0, this->fLightPB->GetInt(kBlue, 0));
	newOb->fLightPB->SetValue(kHue, 0, this->fLightPB->GetInt(kHue, 0));
	newOb->fLightPB->SetValue(kSat, 0, this->fLightPB->GetInt(kSat, 0));
	newOb->fLightPB->SetValue(kVal, 0, this->fLightPB->GetInt(kVal, 0));
	newOb->fLightPB->SetValue(kIntensity, 0, this->fLightPB->GetFloat(kIntensity, 0));
	newOb->fLightPB->SetValue(kContrast, 0, this->fLightPB->GetFloat(kContrast, 0));
	newOb->fLightPB->SetValue(kDiffSoft, 0, this->fLightPB->GetFloat(kDiffSoft, 0));
	newOb->fLightPB->SetValue(kDiffOn, 0, this->fLightPB->GetInt(kDiffOn, 0));
	newOb->fLightPB->SetValue(kStartAttenNear, 0, this->fLightPB->GetFloat(kStartAttenNear, 0));
	newOb->fLightPB->SetValue(kAmbiOnly, 0, this->fLightPB->GetInt(kAmbiOnly, 0));
	newOb->fLightPB->SetValue(kEndAttenNear, 0, this->fLightPB->GetFloat(kEndAttenNear, 0));
	newOb->fLightPB->SetValue(kUseNearAtten, 0, this->fLightPB->GetInt(kUseNearAtten, 0));
	newOb->fLightPB->SetValue(kShowNearAttenRanges, 0, this->fLightPB->GetInt(kShowNearAttenRanges, 0));
	newOb->fLightPB->SetValue(kStartAttenFar, 0, this->fLightPB->GetFloat(kStartAttenFar, 0));
	newOb->fLightPB->SetValue(kEndAttenFar, 0, this->fLightPB->GetFloat(kEndAttenFar, 0));
	
	newOb->fLightPB->SetValue(kUseFarAtten, 0, this->fLightPB->GetInt(kUseFarAtten, 0));
	newOb->fLightPB->SetValue(kShowFarAttenRanges, 0, this->fLightPB->GetInt(kShowFarAttenRanges, 0));
	newOb->fLightPB->SetValue(kLightDecay, 0, this->fLightPB->GetInt(kLightDecay, 0));
	newOb->fLightPB->SetValue(kRed, 0, this->fLightPB->GetInt(kRed, 0));
	newOb->fLightPB->SetValue(kGreen, 0, this->fLightPB->GetInt(kGreen, 0));
	newOb->fLightPB->SetValue(kBlue, 0, this->fLightPB->GetInt(kBlue, 0));
	newOb->fLightPB->SetValue(kHue, 0, this->fLightPB->GetInt(kHue, 0));
	newOb->fLightPB->SetValue(kSat, 0, this->fLightPB->GetInt(kSat, 0));
	newOb->fLightPB->SetValue(kVal, 0, this->fLightPB->GetInt(kVal, 0));
	newOb->fLightPB->SetValue(kIntensity, 0, this->fLightPB->GetFloat(kIntensity, 0));
	newOb->fLightPB->SetValue(kContrast, 0, this->fLightPB->GetFloat(kContrast, 0));
	newOb->fLightPB->SetValue(kDiffSoft, 0, this->fLightPB->GetFloat(kDiffSoft, 0));
	newOb->fLightPB->SetValue(kDiffOn, 0, this->fLightPB->GetInt(kDiffOn, 0));
	newOb->fLightPB->SetValue(kStartAttenNear, 0, this->fLightPB->GetFloat(kStartAttenNear, 0));
	newOb->fLightPB->SetValue(kAmbiOnly, 0, this->fLightPB->GetInt(kAmbiOnly, 0));
	newOb->fLightPB->SetValue(kEndAttenNear, 0, this->fLightPB->GetFloat(kEndAttenNear, 0));
	newOb->fLightPB->SetValue(kUseNearAtten, 0, this->fLightPB->GetInt(kUseNearAtten, 0));
	newOb->fLightPB->SetValue(kShowNearAttenRanges, 0, this->fLightPB->GetInt(kShowNearAttenRanges, 0));
	newOb->fLightPB->SetValue(kStartAttenFar, 0, this->fLightPB->GetFloat(kStartAttenFar, 0));
	newOb->fLightPB->SetValue(kEndAttenFar, 0, this->fLightPB->GetFloat(kEndAttenFar, 0));

*/

/*


  	kLightType,			//Inserted in v1
		kLightOn,			//Inserted in v1
		kLightColor,		//Inserted in v1
		kLightExclude,		//Inserted in v1
		kRed,				//Inserted in v1
		kGreen,				//Inserted in v1
		kBlue,				//Inserted in v1
		kHue,				//Inserted in v1
		kSat,				//Inserted in v1
		kVal,				//Inserted in v1
		kIntensity,			//Inserted in v1
		kContrast,			//Inserted in v1
		kDiffSoft,			//Inserted in v1
		kDiffOn,			//Inserted in v1
		kSpec,				//Inserted in v1
		kAmbiOnly,			//Inserted in v1
		kStartAttenNear,	 //Inserted in v1
		kEndAttenNear,		 //Inserted in v1
		kUseNearAtten,		 //Inserted in v1
		kShowNearAttenRanges,//Inserted in v1
		kStartAttenFar,		 //Inserted in v1
		kEndAttenFar,		 //Inserted in v1
		kUseFarAtten,		 //Inserted in v1
		kShowFarAttenRanges, //Inserted in v1
		kLightDecay,		 //Inserted in v1
		kDecayEdit,			 //Inserted in v1
		kShowDecay,			 //Inserted in v1
		kUseProjectBool,	//Inserted in v1
		kProjMapTexButton,	//Inserted in v1
		kShowConeBool, 		//Inserted in v1
		kOvershootBool,		//Inserted in v1
		kHotSpot,			//Inserted in v1
		kFallOff,			//Inserted in v1
		kLightShapeRadio,	//Inserted in v1
		kAspect,			//Inserted in v1
		kUseProjectorBool,	//Inserted in v1
		kProjMapTexButton2,	//Inserted in v1
		kTargetDist			//Inserted in v1
	GeneralLight* newob = TRACKED_NEW GeneralLight(type);
	newob->enable = enable;
	newob->coneDisplay = coneDisplay;
	newob->useLight = useLight;
	newob->attenDisplay = attenDisplay;
	newob->useAtten = useAtten;
	newob->useAttenNear = useAttenNear;
	newob->attenNearDisplay = attenNearDisplay;
	newob->decayDisplay = decayDisplay;
	newob->shape = shape;
	newob->shadow = shadow;
	newob->shadowType = shadowType;
	newob->overshoot = overshoot;
	newob->projector = projector;
	newob->absMapBias = absMapBias;
	newob->exclList = exclList;
	newob->softenDiffuse = softenDiffuse;
	newob->affectDiffuse = affectDiffuse;
	newob->affectSpecular = affectSpecular;
	newob->ambientOnly = ambientOnly;
	newob->decayType = decayType;
	newob->atmosShadows = atmosShadows;
	newob->atmosOpacity = atmosOpacity;
	newob->atmosColAmt = atmosColAmt;
	newob->ReplaceReference(PBLOCK_REF,pblock->Clone(remap));
	if (projMap)     newob->ReplaceReference(PROJMAP_REF,projMap->Clone(remap));
	if (shadProjMap) newob->ReplaceReference(SHADPROJMAP_REF,shadProjMap->Clone(remap));
	if (shadType)    newob->ReplaceReference(SHADTYPE_REF,shadType->Clone(remap));
	if (emitter)     newob->ReplaceReference(EMITTER_REF ,emitter->Clone(remap));
	BaseClone(this, newob, remap);
	return(newob);
	}
*/



//}





void plRTLightBase::GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box)
{
	GetLocalBoundBox( t, node, vpt, box );
	box = box * node->GetObjectTM( t );
}

void plRTLightBase::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	box = fMesh.getBoundingBox(tm);
}



int plRTLightBase::Display(TimeValue t, INode *node, ViewExp *vpt, int flags)
{

	
	Matrix3 m;
//	if (!enable) 
//		return 0;
	GraphicsWindow *gw = vpt->getGW();
	GetMat(t,node,vpt,m);
	gw->setTransform(m);
	DWORD rlim = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|GW_BACKCULL|(gw->getRndMode() & GW_Z_BUFFER));
	if (node->Selected())
		gw->setColor( LINE_COLOR, GetSelColor());
	else if(!node->IsFrozen() && !node->Dependent())	
	{
		if(fLightPB->GetInt(kLightOn))
			gw->setColor( LINE_COLOR, GetUIColor(COLOR_LIGHT_OBJ));
		// I un-commented this line DS 6/11/99
		else
			gw->setColor( LINE_COLOR, 0.0f, 0.0f, 0.0f);
	}
	
	fMesh.render( gw, gw->getMaterial(),
		(flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL);	
	
	DrawConeAndLine(t, node, gw, 1);
//	DrawAtten(t, node, gw);
	gw->setRndLimits(rlim);
	return 0;
	
}



static void RemoveScaling(Matrix3 &tm) 
{
	AffineParts ap;
	decomp_affine(tm, &ap);
	tm.IdentityMatrix();
	tm.SetRotate(ap.q);
	tm.SetTrans(ap.t);
}

void plRTLightBase::GetMat(TimeValue t, INode* inode, ViewExp *vpt, Matrix3& tm) 
{
	tm = inode->GetObjectTM(t);
//	tm.NoScale();
	RemoveScaling(tm);
	float scaleFactor = vpt->NonScalingObjectSize()*vpt->GetVPWorldWidth(tm.GetTrans())/(float)360.0;
	tm.Scale(Point3(scaleFactor,scaleFactor,scaleFactor));
}


void plRTLightBase::GetConePoints(TimeValue t, float aspect, float angle, float dist, Point3 *q) 
{
	float ta = (float)tan(0.5*DegToRad(angle));   
	if(1 /*fLightPB->GetFloat(kAspect, t) <= 0.0f*/) 
	{ 
		// CIRCULAR
		float rad = dist * ta;
		double a;
		if(IsDir())
			rad = angle;
		int i;
		for(i = 0; i < NUM_CIRC_PTS; i++) {
			a = (double)i * 2.0 * 3.1415926 / (double)NUM_CIRC_PTS;
			q[i] = Point3(rad*(float)sin(a), rad*(float)cos(a), -dist);
			}
		q[i] = q[0] + Point3(0.0f, 15.0f, 0.0f);
	}
	else 
	{		 
		// RECTANGULAR
		float w = IsDir()? angle : dist * ta * (float)sqrt((double)aspect);
		float h = w / aspect;
		q[0] = Point3( w, h,-dist);				
		q[1] = Point3(-w, h,-dist);				
		q[2] = Point3(-w,-h,-dist);				
		q[3] = Point3( w,-h,-dist);
		q[4] = Point3( 0.0f, h+15.0f, -dist);
		q[5] = Point3( 0.0f, h, -dist);
	}
}

#define HOTCONE		0
#define FALLCONE	1

void plRTLightBase::DrawCone(TimeValue t, GraphicsWindow *gw, float dist) 
{
	Point3 q[NUM_CIRC_PTS+1], u[3];
	int dirLight = IsDir();
	int i;
	BOOL dispAtten = false; 
	BOOL dispDecay = false;
	if( this->ClassID() == RTOMNI_LIGHT_CLASSID || this->ClassID() == RTSPOT_LIGHT_CLASSID )
	{
		dispAtten = (fLightPB->GetInt(kUseAttenuationBool, t) && (extDispFlags & EXT_DISP_ONLY_SELECTED));//attenDisplay;
		dispDecay = (/*fLightPB->GetInt(kAttenTypeRadio, t)*/GetDecayType()  && (extDispFlags & EXT_DISP_ONLY_SELECTED));
	}
	if(!IsDir())
		GetConePoints(t, -1.0f, GetHotspot(t), dist, q);
	else
		GetConePoints(t, -1.0f, 20.0, dist, q);

	gw->setColor( LINE_COLOR, GetUIColor(COLOR_HOTSPOT));
	if(1 /*Circular Hack*/) {  
		// CIRCULAR
		if(GetHotspot(t) >= GetFallsize(t)) 
		{
			// draw (far) hotspot circle
			u[0] = q[0];
			u[1] = q[NUM_CIRC_PTS];
			gw->polyline( 2, u, NULL, NULL, FALSE, NULL);
		}
		gw->polyline(NUM_CIRC_PTS, q, NULL, NULL, TRUE, NULL);
		if (dirLight) 
		{
			// draw 4 axial hotspot lines
			for (i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX) 
			{
				u[0] =  q[i]; 	u[1] =  q[i]; u[1].z += dist;
				gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
			}
			GetConePoints(t, -1.0f, 20/*GetHotspot(t)*/, 0.0f, q);
			// draw (near) hotspot circle
			gw->polyline(NUM_CIRC_PTS, q, NULL, NULL, TRUE, NULL);
		}
		else  
		{
			// draw 4 axial lines
			u[0] = Point3(0,0,0);
			for (i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX) 
			{
				u[1] =  q[i];
				gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
			}
		}
		if(!IsDir())
			GetConePoints(t, -1.0f, GetFallsize(t), dist, q);
		else
			GetConePoints(t, -1.0f, 200.0, dist, q);
		gw->setColor( LINE_COLOR, GetUIColor(COLOR_FALLOFF));
		if(GetHotspot(t) < GetFallsize(t)) 
		{
			// draw (far) fallsize circle
			u[0] = q[0];	u[1] = q[NUM_CIRC_PTS];
			gw->polyline( 2, u, NULL, NULL, FALSE, NULL);
			u[0] = Point3(0,0,0);
		}
		gw->polyline(NUM_CIRC_PTS, q, NULL, NULL, TRUE, NULL);
		if (dirLight)
		{
			float dfar = q[0].z;
			float dnear = 0.0f;
			if (dispAtten)
			{
				dfar  = MinF(-GetAtten(t,ATTEN_END),dfar);
			///	dnear = MaxF(-GetAtten(t,ATTEN_START),dnear);
			}
			if (dispDecay) {
				dfar  = MinF(/*-GetDecayRadius(t)*/ 0.0,dfar);
			}

			// draw axial fallsize lines
			for (i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX) 
			{
				u[0] =  q[i];  u[0].z = dfar;	u[1] =  q[i]; u[1].z = dnear;
				gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
			}

			GetConePoints(t, -1.0f, 10000.0, 0.0f, q);
			// draw (near) fallsize circle
			gw->polyline(NUM_CIRC_PTS, q, NULL, NULL, TRUE, NULL);
			
		}
		else 
		{
			float cs = (float)cos(DegToRad(GetFallsize(t)*0.5f));
			float dfar = q[0].z;
			if (dispAtten)
				dfar  = MinF(-cs*GetAtten(t,ATTEN_END),dfar);
			if (dispDecay) 
				dfar  = MinF(/*-cs*GetDecayRadius(t)*/0.0,dfar);

			for (i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX) 
			{
				u[1] =  -q[i]*dfar/dist;	
				gw->polyline( 2, u, NULL, NULL, FALSE, NULL );	
			}
		}
	}

}


int plRTLightBase::DrawConeAndLine(TimeValue t, INode* inode, GraphicsWindow *gw, int drawing ) 
	{
	if(!IsSpot() && !IsDir())
		return 0;
	Matrix3 tm = inode->GetObjectTM(t);
	gw->setTransform(tm);
	gw->clearHitCode();
	if( 0 )
	{
		Point3 pt,v[3];
		if (GetTargetPoint(t, inode, pt)) 
		{
			float den = FLength(tm.GetRow(2));
			float dist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;
			fLightPB->SetValue(kAttenMaxFalloffEdit, t, dist);

			//fLightPB->SetValue(kTargetDist, t, dist);
			//if (hSpotLight&&(currentEditLight==this)) {
			//	TCHAR buf[40];
			//	_stprintf(buf,_T("%0.3f"),targDist);
			//	SetWindowText(GetDlgItem(hSpotLight,IDC_TARG_DISTANCE),buf);
			//	}

			if ((drawing != -1) && (fLightPB->GetInt(kShowConeBool, t) || (extDispFlags & EXT_DISP_ONLY_SELECTED)))
				DrawCone(t, gw, dist);
			if(!inode->IsFrozen() && !inode->Dependent())
				gw->setColor( LINE_COLOR, GetUIColor(COLOR_TARGET_LINE));
			v[0] = Point3(0,0,0);
			v[1] = Point3(0.0f, 0.0f, (drawing == -1)? (-0.9f * dist): -dist);
			gw->polyline( 2, v, NULL, NULL, FALSE, NULL );	
		}
		
	}
	else if( this->ClassID() == RTSPOT_LIGHT_CLASSID )
	{
		if ((drawing != -1) && (fLightPB->GetInt(kShowConeBool, t) || (extDispFlags & EXT_DISP_ONLY_SELECTED)))
			DrawCone(t, gw, fLightPB->GetFloat( kAttenMaxFalloffEdit, t ) );
	}
	else if( this->ClassID() == RTDIR_LIGHT_CLASSID )
	{
		if ((extDispFlags & EXT_DISP_ONLY_SELECTED))
			DrawCone(t, gw, 500/*GetTDist(t)*/);
	}
	return gw->checkHitCode();
}


//// DrawArrow ///////////////////////////////////////////////////////////////

void	plRTLightBase::DrawArrow( TimeValue t, GraphicsWindow *gw, Point3 &direction, float dist ) 
{
	Point3	pts[ 5 ];

	
	pts[ 0 ] = Point3( 0, 0, 0 );
	pts[ 1 ] = direction * dist;
	pts[ 3 ] = pts[ 1 ] - direction * 10.f;
	pts[ 2 ] = pts[ 3 ] + Point3( direction.y, direction.z, direction.x ) * 5.f;

	gw->polyline( 4, pts, nil, nil, true, nil );
}

int plRTLightBase::HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	DWORD savedLimits;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hitRegion;
	int res;
	Matrix3 m;
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	Material *mtl = gw->getMaterial();
	
	gw->setRndLimits( ((savedLimits = gw->getRndLimits()) | GW_PICK) & ~(GW_ILLUM|GW_BACKCULL));
	GetMat(t,node,vpt,m);
	
	//BuildMeshes(t);
	gw->setTransform(m);
	res = fMesh.select( gw, mtl, &hitRegion, flags & HIT_ABORTONHIT);
	// if not, check the target line, and set the pair flag if it's hit
	if( !res )	
	{
		// this special case only works with point selection of targeted lights
		if((type != HITTYPE_POINT) || !node->GetTarget())
			return 0;
		// don't let line be active if only looking at selected stuff and target isn't selected
		if((flags & HIT_SELONLY) && !node->GetTarget()->Selected() )
			return 0;
		gw->clearHitCode();
		if(res = DrawConeAndLine(t, node, gw, -1))
			node->SetTargetNodePair(1);
	}
	gw->setRndLimits(savedLimits);
	return res;

	//return fMesh.select(gw,node->Mtls(),&hitRegion,flags & HIT_ABORTONHIT,node->NumMtls());
}
static void GenericSnap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
{
	// Make sure the vertex priority is active and at least as important as the best snap so far
	if(snap->vertPriority > 0 && snap->vertPriority <= snap->priority) 
	{
		Matrix3 tm = inode->GetObjectTM(t);	
		GraphicsWindow *gw = vpt->getGW();	
   	
		gw->setTransform(tm);

		Point2 fp = Point2((float)p->x, (float)p->y);
		IPoint3 screen3;
		Point2 screen2;
		Point3 vert(0.0f,0.0f,0.0f);

		gw->wTransPoint(&vert,&screen3);

		screen2.x = (float)screen3.x;
		screen2.y = (float)screen3.y;

		// Are we within the snap radius?
		int len = (int)Length(screen2 - fp);
		if(len <= snap->strength) 
		{
			// Is this priority better than the best so far?
			if(snap->vertPriority < snap->priority) 
			{
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
			// Closer than the best of this priority?
			else if(len < snap->bestDist) 
			{
				snap->priority = snap->vertPriority;
				snap->bestWorld = vert * tm;
				snap->bestScreen = screen2;
				snap->bestDist = len;
			}
		}
	}
}

void plRTLightBase::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) 
{
	GenericSnap(t,inode,snap,p,vpt);
}
// This generic snap routine is used to snap to the 0,0,0 point of the given node.  For lights,
// this works to snap all types.



RefTargetHandle plRTLightBase::GetReference(int i)
{
	/*
		kRefProjMap,
		kRefShadowProjMap,
		kRefShadowType,
  *///Texmap*	MyMap;
	switch(i)
		{
		case kRefGeneralLightProp:
			return NULL;
		case kRefProjMap:
			//if(fLightPB->GetTexmap(kProjMapTexButton, 0) != NULL)
			//{
			//	MyMap = fLightPB->GetTexmap(kProjMapTexButton, 0);			
			//return (RefTargetHandle) MyMap; 
			//}else
				return NULL;
		case kRefShadowProjMap:
			return NULL;
		case kRefShadowType:
			return NULL;
		case kRefOmniLight:
		case kRefSpotLight:
		case kRefTSpotLight:
		case kRefDirLight:
		case kRefTDirLight:
		case kRefProjDirLight:
			return (RefTargetHandle)fLightPB;
		default:
			return NULL;
		}

}		

void plRTLightBase::SetReference(int ref, RefTargetHandle rtarg)
{
		//Texmap* MyMap;
		switch(ref)
		{
		case kRefGeneralLightProp:
			return;
		case kRefProjMap:
				
				//MyMap = (Texmap *)rtarg;
				//fLightPB->SetValue(kProjMapTexButton, 0, MyMap);
				
				//if (projMapName) 
				//	projMapName->SetText(projMap?projMap->GetFullName().data():GetString(IDS_DB_NONE));
				return;
			
		case kRefShadowProjMap:
			return;
		case kRefShadowType:
			return;
		case kRefOmniLight:
		case kRefSpotLight:
		case kRefTSpotLight:
		case kRefDirLight:
		case kRefTDirLight:
		case kRefProjDirLight:
			fLightPB = (IParamBlock2*)rtarg; break;
		}	
		//fLightPB = (IParamBlock2*)rtarg;
}

RefResult plRTLightBase::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{			
	if( fLightPB )
	{
		ParamID		param = fLightPB->LastNotifyParamID();

		if( param == kAmbientOnlyStub )
		{
			fLightPB->EnableNotifications( false );
			fLightPB->SetValue( kAmbientOnlyStub, TimeValue( 0 ), false );
			// Assume this was true, since, well, we're IN NotifyRefChanged....
			fLightPB->EnableNotifications( true );

			return REF_SUCCEED;
		}
	}

	return REF_SUCCEED;
}

void plRTLightBase::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
	fIP = ip;
	fClassDesc->BeginEditParams(ip, this, flags, prev);
}

void plRTLightBase::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{	
	GenLight::EndEditParams( ip, flags, next );
	fIP = NULL;
	fClassDesc->EndEditParams(ip, this, flags, next);
}

RefResult plRTLightBase::EvalLightState(TimeValue t, Interval& valid, LightState *ls)
{
		//t uselight;
//#if 0	//fLightPB->GetInt(kLightOn, t);
	if(fLightPB->GetInt(kLightOn, t) == true)
		ls->color = GetRGBColor(t,valid);
	else
		ls->color = Color(0,0,0);
	ls->on = fLightPB->GetInt(kLightOn, t);
	ls->intens = GetIntensity(t, valid);
	if( this->ClassID() == RTSPOT_LIGHT_CLASSID || this->ClassID() == RTOMNI_LIGHT_CLASSID )
	{
		ls->hotsize = GetHotspot(t, valid);
		ls->fallsize = GetFallsize(t, valid);
		ls->useNearAtten = GetUseAttenNear();
		ls->useAtten = GetUseAtten();
		ls->attenStart = GetAtten(t, ATTEN_START, valid);
		ls->attenEnd = GetAtten(t, ATTEN_END, valid);
	}else
	{
		ls->hotsize = 20;
		ls->fallsize = 10000;
		ls->useNearAtten = false; //GetUseAttenNear();
		ls->useAtten = false; //GetUseAtten();
		//ls->attenStart = GetAtten(t, ATTEN_START, valid);
		//ls->attenEnd = GetAtten(t, ATTEN_END, valid);


	}
	ls->shape = GW_SHAPE_CIRCULAR;	//fLightPB->GetInt(kLightShapeRadio);

	ls->aspect = -1.0;//GetAspect(t, valid);
	ls->overshoot = false;	//GetOvershoot();
	ls->shadow = GetShadow();
	ls->ambientOnly = false; //fLightPB->GetValue(	kAmbiOnly,t, ls->ambientOnly, valid);	//ls->ambientOnly = fLightP.AmbiOnly;	
	ls->affectDiffuse = true;   //fLightPB->GetInt(kDiffOn,t);	//ls->affectDiffusey = fLightP.AmbiOnly;	
	ls->affectSpecular = fLightPB->GetInt(kSpec,t);	//ls- = fLightP.DiffOn;;

	//ls->

	if( this->ClassID() == RTOMNI_LIGHT_CLASSID )
		ls->type = OMNI_LGT;
	else if( this->ClassID() == RTDIR_LIGHT_CLASSID || this->ClassID() == RTPDIR_LIGHT_CLASSID )
		ls->type = DIRECT_LGT;
	else
		ls->type = SPOT_LGT;

	return REF_SUCCEED;
}


#if 0
#define PLASMAOBJ_DATA_CHUNK 1

IOResult plRTLightBase::Save(ISave* isave)
{
	return IO_OK;
}

IOResult plRTLightBase::Load(ILoad* iload)
{
	ULONG nb;
	IOResult res;
	int numrefs = 0;

	while (IO_OK == (res = iload->OpenChunk()))
	{
		if (iload->CurChunkID() == PLASMAOBJ_DATA_CHUNK)
			res = iload->Read(&numrefs, sizeof(int), &nb);
		iload->CloseChunk();

		if (res != IO_OK)
			return res;
	}

	return IO_OK;
}

#endif

	
ObjLightDesc *plRTLightBase::CreateLightDesc(INode *n, BOOL forceShadowBuf)
{

	return NULL;

}


void plRTLightBase::SetHotspot(TimeValue t, float f)
{
	if(!IsSpot())
		return;
	if(f < 0.5f)
		f = 0.5f;
	if(!IsDir() && (f > 179.5f))
		f = 179.5f;
	//pblock->SetValue( PB_HOTSIZE, t, f );
	fLightPB->SetValue(kHotSpot, t, f);

	//fLightP.HotSpot = f;
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

float plRTLightBase::GetHotspot(TimeValue t, Interval& valid)
{
	Interval iv;

	if(!IsSpot() && !IsDir())
		return -1.0f;
	float f;
	//pblock->GetValue( PB_HOTSIZE, t, f, valid );
	if(IsSpot())
		fLightPB->GetValue(kHotSpot, t, f, FOREVER);
	else
		f = 20.0;

	if(GetFallsize(t, iv) < f )
		return GetFallsize(t, iv) - 2.0;
	return f;
}

void plRTLightBase::SetRGBColor(TimeValue t, Point3& rgb) 
{
	//fLightPB->SetValue(plRTLightBase::kRed, t, rgb.x);
	//fLightPB->SetValue(plRTLightBase::kGreen, t, rgb.y);
	//fLightPB->SetValue(plRTLightBase::kBlue, t, rgb.z);
	fLightPB->SetValue(kLightColor, t, rgb );
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);


}

Point3 plRTLightBase::GetRGBColor(TimeValue t, Interval &valid)
{	
	//Point3 Foo = Point3(fLightPB->GetInt(plRTLightBase::kRed, t), fLightPB->GetInt(plRTLightBase::kGreen, t), fLightPB->GetInt(plRTLightBase::kBlue, t));
	Point3 rgb;
	fLightPB->GetValue( kLightColor, t, rgb, valid );
	return rgb;
}



void plRTLightBase::SetFallsize(TimeValue t, float f)
{
	if(!IsSpot() )
		return;
	if(f < 0.5f)
		f = 0.5f;
	if(!IsDir() && (f > 179.5f))
		f = 179.5f;
	//pblock->SetValue( PB_FALLSIZE, t, f );
	fLightPB->SetValue(kFallOff, t, f);

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}

float plRTLightBase::GetFallsize(TimeValue t, Interval& valid)
{
	if(!IsSpot() && !IsDir())
		return -1.0f;
	float f;
	
	//pblock->GetValue( PB_FALLSIZE, t, f, valid );
	if(IsSpot())
		fLightPB->GetValue(kFallOff, t, f, valid);
	else
		f = 200.0;
	return f;
}


void plRTLightBase::SetAtten(TimeValue t, int which, float f)
{

	if(which != ATTEN_START)
		fLightPB->SetValue(kAttenMaxFalloffEdit, t, f);

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}	


float plRTLightBase::GetAtten(TimeValue t, int which, Interval& valid)
{	
	float f = 0.0;
	//if(fLightPB->GetInt(kEndAttenFar, t) == 1)
	//if(LyteType == RT_OMNI)
	//	fLightPB->GetValue(kStartAttenFar, t, f, FOREVER); 
	//else
	//	fLightPB->GetValue(kStartAttenNear, t, f, FOREVER); 
	if(which == LIGHT_ATTEN_START)
		return f;
	else
		return( fLightPB->GetFloat(kAttenMaxFalloffEdit, t));

	//return f;
}

	
void plRTLightBase::SetTDist(TimeValue t, float f) 
{
//	int i;
//	fLightPB->GetInt(kLightType, t, i);
	
	//pblock->SetValue( PB_TDIST, t, f );
	//To be implemented.
	if( IHasAttenuation() )
		fLightPB->SetValue(kAttenMaxFalloffEdit, t, f);
	//fLightPB->SetValue(kTargetDist, t, f);

	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
}
	
float plRTLightBase::GetTDist(TimeValue t, Interval& valid)
{
	if(!IsSpot())
		return -1.0f;
//	int i;
	//fLightPB->GetValue(kLightType, t, i, valid);
	float f = -1.0;

	fLightPB->GetFloat( kAttenMaxFalloffEdit, t );

	//pblock->GetValue( PB_TDIST, t, f, valid );
	//fLightPB->GetValue(kTargetDist, t, f, valid);
	return f;
}

void plRTLightBase::SetConeDisplay(int s, int notify) 
{
	if(!IsDir())
		fLightPB->SetValue(kShowConeBool, 0, s);
	if(notify && IsSpot())
		NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
}

BOOL plRTLightBase::GetConeDisplay(void)
{
	if(!IsDir())
		return fLightPB->GetInt(kShowConeBool);
	return
		false;
}


void plRTLightBase::SetProjMap(BitmapInfo* pmap)
{
	//plLayerTex* MyMap = TRACKED_NEW plLayerTex;
	if(!fTex)
		fTex = TRACKED_NEW plLayerTex;
	fTex->SetBitmap(pmap);
	ReplaceReference(kRefProjMap,fTex);

	IParamBlock2 *bitmapPB = fTex->GetParamBlockByID(plLayerTex::kBlkBitmap);
	bitmapPB->SetValue(kBmpUseBitmap, 0, 1);

	// This is set in the call to fTex->SetBitmap(pmap)
	//PBBitmap pbb(*pmap);
	//bitmapPB->SetValue(kBmpBitmap, 0, &pbb);

	//Texmap* MyMap;
	//fLightPB->GetValue(kProjMapTexButton, 0, MyMap, FOREVER);
	if (fTex) fLightPB->SetValue(kUseProjectorBool, 0, true);	
	NotifyDependents(FOREVER,0,REFMSG_SUBANIM_STRUCTURE_CHANGED);
	if( fLightPB->GetMap() )
	{
		fLightPB->GetMap()->Invalidate(kProjMapTexButton);
		fLightPB->GetMap()->Invalidate(kUseProjectorBool);
	}

}


BOOL plRTLightBase::GetShadow()
{
	return fLightPB->GetInt(kCastShadows);
}

void plRTLightBase::SetShadow(int a)
{
	fLightPB->SetValue(kCastShadows, 0, 1);
}

Texmap* plRTLightBase::GetProjMap() 
{ 
	if( !fLightPB->GetInt(kUseProjectorBool) )
		return NULL;

	if( GetTex() )
	{
		const char* dbgTexName = GetTex()->GetName();

		IParamBlock2 *bitmapPB = fTex->GetParamBlockByID(plLayerTex::kBlkBitmap);
		hsAssert(bitmapPB, "LayerTex with no param block");

		int noCompress = fLightPB->GetInt(kProjNoCompress);
		int noMip = fLightPB->GetInt(kProjNoMip);
		bitmapPB->SetValue(kBmpNonCompressed, TimeValue(0), noCompress);
		bitmapPB->SetValue(kBmpNoFilter, TimeValue(0), noMip);
	}

	return (Texmap*) GetTex();
}
	
void plRTLightBase::UpdateTargDistance(TimeValue t, INode* inode)
{
	if( this->ClassID() == RTSPOT_LIGHT_CLASSID )
	{
		Point3 pt,v[3];
		if (GetTargetPoint(t, inode, pt))
		{
			Matrix3 tm = inode->GetObjectTM(t);
			float den = FLength(tm.GetRow(2));
			float dist = (den!=0) ? FLength(tm.GetTrans()-pt) / den : 0.0f;
			fLightPB->SetValue(kAttenMaxFalloffEdit, t, dist);

	//		fLightPB->SetValue(kTargetDist, t, dist);
			//TCHAR buf[40];
			//_stprintf(buf,_T("%0.3f"),targDist);
			//SetWindowText(GetDlgItem(hSpotLight,IDC_TARG_DISTANCE),buf);
		}
	}

}




///  
//
//
//	Target Creation for Targeted Lights...
//
//
//


#if 0
class TSpotCreationManager : public MouseCallBack, ReferenceMaker 
{
private:
	CreateMouseCallBack *createCB;	
	INode *lgtNode,*targNode;
	plRTLightBase *lgtObject;
	TargetObject *targObject;
	int attachedToNode;
	IObjCreate *createInterface;
	ClassDesc *cDesc;
	Matrix3 mat;  // the nodes TM relative to the CP
	IPoint2 pt0;
	int ignoreSelectionChange;
	int lastPutCount;

	void CreateNewObject();	

	int NumRefs() { return 1; }
	RefTargetHandle GetReference(int i) { return (RefTargetHandle)lgtNode; } 
	void SetReference(int i, RefTargetHandle rtarg) { lgtNode = (INode *)rtarg; }

	// StdNotifyRefChanged calls this, which can change the partID to new value 
	// If it doesnt depend on the particular message& partID, it should return
	// REF_DONTCARE
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	    	PartID& partID,  RefMessage message);

public:
	void Begin( IObjCreate *ioc, ClassDesc *desc );
	void End();
		
	TSpotCreationManager()	{ ignoreSelectionChange = FALSE; }
	int proc( HWND hwnd, int msg, int point, int flag, IPoint2 m );
	BOOL SupportAutoGrid(){return TRUE;}
};


#define CID_TSPOTCREATE	CID_USER + 3

class TSpotCreateMode : public CommandMode {
	TSpotCreationManager proc;
public:
	void Begin( IObjCreate *ioc, ClassDesc *desc ) { proc.Begin( ioc, desc ); }
	void End() { proc.End(); }

	int Class() { return CREATE_COMMAND; }
	int ID() { return CID_TSPOTCREATE; }
	MouseCallBack *MouseProc(int *numPoints) { *numPoints = 1000000; return &proc; }
	ChangeForegroundCallback *ChangeFGProc() { return CHANGE_FG_SELECTED; }
	BOOL ChangeFG( CommandMode *oldMode ) { return (oldMode->ChangeFGProc() != CHANGE_FG_SELECTED); }
	void EnterMode() {
		}
	void ExitMode() {
		}
	BOOL IsSticky() { return FALSE; }
};

static TSpotCreateMode theTSpotCreateMode;

#endif

#if 0
void TSpotCreationManager::Begin( IObjCreate *ioc, ClassDesc *desc )
{
	createInterface = ioc;
	cDesc           = desc;
	attachedToNode  = FALSE;
	createCB        = NULL;
	lgtNode         = NULL;
	targNode        = NULL;
	lgtObject       = NULL;
	targObject      = NULL;
	CreateNewObject();
}

void TSpotCreationManager::End()
{
	if ( lgtObject ) {
		lgtObject->EndEditParams( (IObjParam*)createInterface, END_EDIT_REMOVEUI, NULL);
		if ( !attachedToNode ) {
			// RB 4-9-96: Normally the hold isn't holding when this 
			// happens, but it can be in certain situations (like a track view paste)
			// Things get confused if it ends up with undo...
			theHold.Suspend(); 
			//delete lgtObject;
			lgtObject->DeleteThis();  // JBW 11.1.99, this allows scripted plugin lights to delete cleanly
			lgtObject = NULL;
			theHold.Resume();
			// RB 7/28/97: If something has been put on the undo stack since this object was created, we have to flush the undo stack.
			if (theHold.GetGlobalPutCount()!=lastPutCount) {
				GetSystemSetting(SYSSET_CLEAR_UNDO);
				}
			macroRec->Cancel();  // JBW 4/23/99
		} 
		else if ( lgtNode ) {
			 // Get rid of the reference.
			DeleteReference(0);  // sets lgtNode = NULL
		}
	}	
}

RefResult TSpotCreationManager::NotifyRefChanged(
	Interval changeInt, 
	RefTargetHandle hTarget, 
	PartID& partID,  
	RefMessage message) 
{
	switch (message) {
		
	case REFMSG_PRENOTIFY_PASTE:
	case REFMSG_TARGET_SELECTIONCHANGE:
	 	if ( ignoreSelectionChange ) {
			break;
		}
	 	if (lgtObject && lgtNode==hTarget) {
			// this will set camNode== NULL;
			DeleteReference(0);
			goto endEdit;
		}
		// fall through

	case REFMSG_TARGET_DELETED:		
		if ( lgtObject && lgtNode==hTarget ) {
			endEdit:
			lgtObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
			lgtObject  = NULL;				
			lgtNode    = NULL;
			CreateNewObject();	
			attachedToNode = FALSE;
		}
		else if (targNode==hTarget) {
			targNode = NULL;
			targObject = NULL;
		}
		break;		
	}
	return REF_SUCCEED;
}


void TSpotCreationManager::CreateNewObject()
{
	lgtObject = (plRTLightBase *)cDesc->Create();
	lastPutCount = theHold.GetGlobalPutCount();

    macroRec->BeginCreate(cDesc);  // JBW 4/23/99
	
	// Start the edit params process
	if ( lgtObject ) {
		lgtObject->BeginEditParams( (IObjParam*)createInterface, BEGIN_EDIT_CREATE, NULL );
	}	
}

static void whoa(){};

static BOOL needToss;
			
int TSpotCreationManager::proc( 
				HWND hwnd,
				int msg,
				int point,
				int flag,
				IPoint2 m )
{	
	int res;
	TSTR targName;	
	ViewExp *vpx = createInterface->GetViewport(hwnd); 
	assert( vpx );

	switch ( msg ) {
	case MOUSE_POINT:
		switch ( point ) {
		case 0:
			pt0 = m;
			assert( lgtObject );					
			vpx->CommitImplicitGrid(m, flag); //KENNY MERGE
			if ( createInterface->SetActiveViewport(hwnd) ) {
				return FALSE;
			}

			if (createInterface->IsCPEdgeOnInView()) { 
				res = FALSE;
				goto done;
			}

			// if lights were hidden by category, re-display them
			GetCOREInterface()->SetHideByCategoryFlags(
					GetCOREInterface()->GetHideByCategoryFlags() & ~HIDE_LIGHTS);

			if ( attachedToNode ) {
		   		// send this one on its way
		   		lgtObject->EndEditParams( (IObjParam*)createInterface, 0, NULL);
				macroRec->EmitScript();  // JBW 4/23/99
					
				// Get rid of the reference.
				if (lgtNode)
					DeleteReference(0);

				// new object
				CreateNewObject();   // creates lgtObject
			}

			needToss = theHold.GetGlobalPutCount()!=lastPutCount;

		   	theHold.Begin();	 // begin hold for undo
			mat.IdentityMatrix();

			// link it up
			lgtNode = createInterface->CreateObjectNode( lgtObject);
			attachedToNode = TRUE;
			assert( lgtNode );					
			createCB = lgtObject->GetCreateMouseCallBack();					
			createInterface->SelectNode( lgtNode );
					
			// Create target object and node
			targObject = TRACKED_NEW TargetObject;
			assert(targObject);
			targNode = createInterface->CreateObjectNode( targObject);
			assert(targNode);
			targName = lgtNode->GetName();
			targName += GetString(IDS_DB_DOT_TARGET);
			targNode->SetName(targName);
				
			// hook up camera to target using lookat controller.
			createInterface->BindToTarget(lgtNode,targNode);

			// Reference the new node so we'll get notifications.
			MakeRefByID( FOREVER, 0, lgtNode);
			
			// Position camera and target at first point then drag.
			mat.IdentityMatrix();
			//mat[3] = vpx->GetPointOnCP(m);
			#ifdef _3D_CREATE
				mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
			#else
				mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			#endif
			createInterface->SetNodeTMRelConstPlane(lgtNode, mat);
			createInterface->SetNodeTMRelConstPlane(targNode, mat);
			lgtObject->Enable(1);

		   	ignoreSelectionChange = TRUE;
		   	createInterface->SelectNode( targNode,0);
		   	ignoreSelectionChange = FALSE;
			res = TRUE;
			break;
					
		case 1:
			if (Length(m-pt0)<2)
				goto abort;
			//mat[3] = vpx->GetPointOnCP(m);
			#ifdef _3D_CREATE
				mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
			#else
				mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
			#endif
			macroRec->Disable();   // JBW 4/23/99
			createInterface->SetNodeTMRelConstPlane(targNode, mat);
			macroRec->Enable();

			ignoreSelectionChange = TRUE;
		   	createInterface->SelectNode( lgtNode);
		   	ignoreSelectionChange = FALSE;
					
		    theHold.Accept(IDS_DS_CREATE);	 

			createInterface->AddLightToScene(lgtNode); 
			createInterface->RedrawViews(createInterface->GetTime());  

			res = FALSE;	// We're done
			break;
		}			
		break;

	case MOUSE_MOVE:
		//mat[3] = vpx->GetPointOnCP(m);
		#ifdef _3D_CREATE
			mat.SetTrans( vpx->SnapPoint(m,m,NULL,SNAP_IN_3D) );
		#else
			mat.SetTrans(vpx->SnapPoint(m,m,NULL,SNAP_IN_PLANE));
		#endif
		macroRec->Disable();   // JBW 4/23/99
		createInterface->SetNodeTMRelConstPlane(targNode, mat);
		macroRec->Enable();
		createInterface->RedrawViews(createInterface->GetTime());

		macroRec->SetProperty(lgtObject, _T("target"),   // JBW 4/23/99
			mr_create, Class_ID(TARGET_CLASS_ID, 0), GEOMOBJECT_CLASS_ID, 1, _T("transform"), mr_matrix3, &mat);

		res = TRUE;
		break;

	case MOUSE_FREEMOVE:
		SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CROSS_HAIR)));
		#ifdef _OSNAP
			//Snap Preview
			#ifdef _3D_CREATE
				vpx->SnapPreview(m,m,NULL, SNAP_IN_3D);
			#else
				vpx->SnapPreview(m,m,NULL, SNAP_IN_PLANE);
			#endif
		#endif
		vpx->TrackImplicitGrid(m); //KENNY MERGE
		break;

    case MOUSE_PROPCLICK:
		// right click while between creations
		createInterface->RemoveMode(NULL);
		break;
		
	case MOUSE_ABORT:
		abort:
		assert( lgtObject );
		lgtObject->EndEditParams( (IObjParam*)createInterface,0,NULL);
		// Toss the undo stack if param changes have been made
		macroRec->Cancel();  // JBW 4/23/99
		theHold.Cancel();	 // deletes both the camera and target.
		if (needToss) 
			GetSystemSetting(SYSSET_CLEAR_UNDO);
		lgtNode = NULL;			
		targNode = NULL;	 	
		createInterface->RedrawViews(createInterface->GetTime()); 
		CreateNewObject();	
		attachedToNode = FALSE;
		res = FALSE;						
	}
	
done:
	//KENNY MERGE
	if ((res == CREATE_STOP)||(res==CREATE_ABORT))
		vpx->ReleaseImplicitGrid();
	createInterface->ReleaseViewport(vpx); 
	return res;
}


#endif
