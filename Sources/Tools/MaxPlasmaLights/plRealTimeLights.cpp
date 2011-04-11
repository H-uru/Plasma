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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plRealTimeLights.cpp - Implementation for some MAX RT lights			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.2.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plRealTimeLights.h"
#include "iparamm2.h"
#include "../MaxPlasmaMtls/Layers/plLayerTex.h"
#include "../MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"
#include "../MaxComponent/plMaxAnimUtils.h"
#include "plRTLightBaseAnimDlgProc.h"
#include "../plGLight/plLightKonstants.h"

void DummyLightCodeIncludeFunc() {}


//// Static ClassDesc2 Get Functions //////////////////////////////////////////

plRTOmniLightDesc	plRTOmniLightDesc::fStaticDesc;
plRTSpotLightDesc	plRTSpotLightDesc::fStaticDesc;
plRTDirLightDesc	plRTDirLightDesc::fStaticDesc;


//// ParamBlock2 DlgProc Functions ////////////////////////////////////////////

const char* DecayLevel[] = {
							"None",
							"Inverse",
							"Inverse Square",
							NULL
							};

const char* ShadowState[] = {
							"Shadow Map",
							NULL
							};


class LightDlgProc : public plBaseLightProc
{
protected:
	void	IValidateSpinners( TimeValue t, WPARAM wParam, IParamBlock2 *pb, IParamMap2 *map )
	{
		/// Make sure falloff is >= hotspot (adjust the one we're not editing)
		float	hotspot, falloff;
		hotspot = pb->GetFloat( plRTLightBase::kHotSpot, t );
		falloff = pb->GetFloat( plRTLightBase::kFallOff, t );

		if( falloff < hotspot )
		{
			if( LOWORD( wParam ) == IDC_LHOTSIZESPINNER )
				pb->SetValue( plRTLightBase::kFallOff, t, hotspot );
			else
				pb->SetValue( plRTLightBase::kHotSpot, t, falloff );

			map->Invalidate( plRTLightBase::kHotSpot );
			map->Invalidate( plRTLightBase::kFallOff );
		}
		IBuildLightMesh( (plRTLightBase *)pb->GetOwner(), falloff );
	}

public:
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		IParamBlock2 *pb = map->GetParamBlock();
		plRTLightBase *gl = (plRTLightBase *) pb->GetOwner();

		switch (msg)
		{
			case WM_COMMAND:
				if(LOWORD(wParam) == IDC_PROJ_MAPNAME )
				{
					if( gl->ClassID() != RTOMNI_LIGHT_CLASSID )
//					if(pb->GetInt(plRTLightBase::kLightType) != plRTLightBase::RT_OMNI)
					
					//	map->SetValue(plRTSpotLight::kProjMapTexButton, t, 
						//gl->SetProjMap(
						map->Invalidate(plRTSpotLight::kProjMapTexButton);
					return false;
				}
				else if( LOWORD( wParam ) == IDC_LHOTSIZE || LOWORD( wParam ) == IDC_LFALLOFF )
				{
					if( HIWORD( wParam ) == EN_CHANGE )
						IValidateSpinners( t, wParam, pb, map );
				}
				break;

			case CC_SPINNER_CHANGE:		 
				if( LOWORD( wParam ) == IDC_LHOTSIZESPINNER || LOWORD( wParam ) == IDC_LFALLOFFSPINNER )
					IValidateSpinners( t, wParam, pb, map );
				break;
		}

		return plBaseLightProc::DlgProc( t, map, hWnd, msg, wParam, lParam );;
	}
	void DeleteThis() {};
};
static LightDlgProc gLiteDlgProc;


#include "plRealTimeLightsPBDec.h"


#include "plRTObjLightDesc.h"


///////////////////////////////////////////////////////////////////////
//
//	Light Descriptors declared below for the different plRTLights...
//
//
//
//

//--- Base Light Class derived from the ObjLightDesc 

#define COS_45 0.7071067f
#define COS_45_2X 1.4142136f

static float stepFactor[] = {50.0f,80.0f,140.0f};
#define MAXSTEPS 1000


BaseObjLight::BaseObjLight(INode *n) : ObjLightDesc(n) 
{
	ObjectState os = n->EvalWorldState(TimeValue(0));
	assert(os.obj->SuperClassID()==LIGHT_CLASS_ID);
	gl = (os.obj->GetInterface(I_MAXSCRIPTPLUGIN) != NULL) ? (plRTLightBase*)os.obj->GetReference(0) : (plRTLightBase*)os.obj;  // JBW 4/7/99
}	

static Color blackCol(0,0,0);

int BaseObjLight::Update(TimeValue t, const RendContext& rc, RenderGlobalContext * rgc, BOOL shadows, BOOL shadowGeomChanged) {
	ObjLightDesc::Update(t,rc,rgc,shadows,shadowGeomChanged);
	intensCol = ls.intens*ls.color*rc.GlobalLightLevel();
	ObjectState os = inode->EvalWorldState(t);
    plRTLightBase* lob = (plRTLightBase *)os.obj;		
	contrast = 0; //lob->GetParamBlock2()->GetFloat(plRTLightBase::kContrast, t);
	diffSoft = 0; //lob->GetDiffuseSoft(t)/100.0f;

	float a = .5; //contrast/204.0f + 0.5f; // so "a" varies from .5 to .99
	kA = (2.0f-1.0f/a);
	kB = 1.0f-kA;
	diffSoften = false; //lob->GetParamBlock2()->GetInt(plRTLightBase::kDiffOn, t);		//GetSoftenDiffuse();

	return 1;
}






//--- Omni Light ------------------------------------------------


OmniLight::OmniLight(INode *inode, BOOL forceShadowBuf ) : BaseObjLight(inode){

	//projector = /*doShadows =  shadowRay =*/  FALSE; 
	//projMap = NULL;
	needMultiple = FALSE;

}

OmniLight::~OmniLight() 
{

}




int OmniLight::UpdateViewDepParams(const Matrix3& worldToCam) {
	BaseObjLight::UpdateViewDepParams(worldToCam);
	return 1;
	}

static Point3 MapToDir(Point3 p, int k) {
	switch(k) {
		case 0: return Point3(  p.z, p.y, -p.x); // +X
		case 1: return Point3( -p.z, p.y,  p.x); // -X
		case 2: return Point3(  p.x, p.z, -p.y); // +Y 
		case 3:	return Point3(  p.x,-p.z,  p.y); // -Y
		case 4: return Point3( -p.x, p.y, -p.z); // +Z
		case 5: return p;                        // -Z
		}
 	return p;
	}

static void GetMatrixForDir(Matrix3 &origm, Matrix3 &tm, int k ) {
	tm = origm;
	switch(k) {
		case 0:	tm.PreRotateY(-HALFPI); break;	// Map 0: +X axis	
		case 1: tm.PreRotateY( HALFPI); break;	// Map 1: -X axis	
		case 2:	tm.PreRotateX( HALFPI); break;	// Map 2: +Y axis	
		case 3:	tm.PreRotateX(-HALFPI); break; 	// Map 3: -Y axis	
		case 4:	tm.PreRotateY(   PI  );	break; 	// Map 4: +Z axis	
		case 5: 						break; 	// Map 5: -Z axis	
		}
	}

static int WhichDir(Point3 &p) {
	int j = MaxComponent(p);  // the component with the maximum abs value
	return  (p[j]<0.0f) ? 2*j+1 : 2*j;
	}

int OmniLight::Update(TimeValue t, const RendContext & rc,
		RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged)
{
	BaseObjLight::Update(t,rc,rgc,shadows,shadowGeomChanged);	

	ObjectState os = inode->EvalWorldState(t);
	LightObject* lob = (LightObject *)os.obj;		
	assert(os.obj->SuperClassID()==LIGHT_CLASS_ID);
	plRTOmniLight* gl = (lob->GetInterface(I_MAXSCRIPTPLUGIN) != NULL) ? (plRTOmniLight*)lob->GetReference(0) : (plRTOmniLight*)lob;  // JBW 4/7/99

	decayType = gl->GetDecayType();	
	decayRadius = gl->GetDecayRadius(t);

	fov = HALFPI; // 90 degree fov
	int res=1;
	if(gl->GetTex())
		gl->GetTex()->Update(t, FOREVER);
	//projector =  gl->GetProjector();
	//if (projector){
	//	projMap = gl->GetProjMap();
	//	if( projMap ) projMap->Update(t,FOREVER);
	//}

	return res;
}



////------------------------------------------------------------------
//		
//
//			SpotLight descriptors.....
//
//
//
//
//

SpotLight::SpotLight(INode *inode, BOOL forceShadowBuf ):BaseObjLight(inode) 
{
	projMap = NULL;
}

int SpotLight::Update(TimeValue t, const RendContext &rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged)
{
	int res = 1;
	BaseObjLight::Update(t,rc,rgc,shadows, shadowGeomChanged);

	float hs = DegToRad(ls.hotsize);
	float fs = DegToRad(ls.fallsize);
	fall_tan = (float)tan(fs/2.0f);
	hot_cos = (float)cos(hs/2.0f);
	fall_cos =(float)cos(fs/2.0f);
	fall_sin = (float)sin(fs/2.0f);
	hotpct = ls.hotsize/ls.fallsize;
	ihotpct = 1.0f - hotpct;		

	ObjectState os = inode->EvalWorldState(t);
	LightObject* lob = (LightObject *)os.obj;		
	assert(os.obj->SuperClassID()==LIGHT_CLASS_ID);
	plRTLightBase* gl = (lob->GetInterface(I_MAXSCRIPTPLUGIN) != NULL) ? (plRTLightBase*)lob->GetReference(0) : (plRTLightBase*)lob;  // JBW 4/7/99

	decayType = gl->GetDecayType();	
	decayRadius = gl->GetDecayRadius(t);

	projector =  gl->GetProjector();
	fov = hsMaximum(fs,hs);

	float aspect = 1.0f;
	 
	fov = 2.0f* (float)atan(tan(fov*0.5f)*sqrt(aspect));
	zfac = -sz2 /(float)tan(0.5*(double)fov);
	xscale = zfac;								
	yscale = -zfac*aspect;
	curve =(float)fabs(1.0f/xscale); 

	rectv0.y = fall_sin * (float)sqrt(aspect);
	rectv1.y = fall_sin / (float)sqrt(aspect);

	rectv0.x = rectv1.x = fall_cos;
	rectv0 = Normalize(rectv0);
	rectv1 = Normalize(rectv1);

	Interval v;
	if (projector){
		projMap = gl->GetProjMap();
		if( projMap ) projMap->Update(t,v);
	}

	return res;
}

int  SpotLight::UpdateViewDepParams(const Matrix3& worldToCam) 
{
	BaseObjLight::UpdateViewDepParams(worldToCam);
	lightDir = -FNormalize(lightToCam.GetRow(2));
	return 1;
}

BOOL SpotLight::IsFacingLight(Point3 &dir)
{
	return dir.z>0.0f;
}



//--- Directional Light ------------------------------------------------

DirLight::DirLight(INode *inode, BOOL forceShadowBuf ) : BaseObjLight(inode)
{
	projMap = NULL;
}

int DirLight::Update(TimeValue t, const RendContext &rc, 
		RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged)
{
	int res = 1;
	BaseObjLight::Update(t,rc,rgc,shadows,shadowGeomChanged);
	hotsz = ls.hotsize;
	fallsz = ls.fallsize;
	fallsq = fallsz*fallsz;
	hotpct = ls.hotsize/ls.fallsize;
	ihotpct = 1.0f - hotpct;

	ObjectState os = inode->EvalWorldState(t);
	LightObject* lob = (LightObject *)os.obj;		
	assert(os.obj->SuperClassID()==LIGHT_CLASS_ID);
	plRTDirLight* gl = (lob->GetInterface(I_MAXSCRIPTPLUGIN) != NULL) ? (plRTDirLight*)lob->GetReference(0) : (plRTDirLight*)lob;  // JBW 4/7/99

	//projector =  gl->GetProjector();

	aspect = 1.0f;

	//if (projector){
	//		projMap = gl->GetProjMap();
	//		if( projMap ) projMap->Update(t,FOREVER);
	//	}
	return res;
};

int DirLight::UpdateViewDepParams(const Matrix3& worldToCam) {
	BaseObjLight::UpdateViewDepParams(worldToCam);
	lightDir = FNormalize(lightToCam.GetRow(2));

	return 1;
	}







////////////////////////////////////////////////////////////////////////////////////
//
//	plRTOmni Stuff below
//
//
//
//
//

plRTOmniLight::plRTOmniLight()
{
	fIP = NULL; 
	fLightPB = NULL; 
	fClassDesc = plRTOmniLightDesc::GetDesc();
	fClassDesc->MakeAutoParamBlocks(this);

	fLightPB->SetValue(kLightColor, 0,  Color(255,255,255));
	SetHSVColor(0, Point3(255, 255, 255));
	
	fTex = NULL;

	meshBuilt = 0; 
	
	IBuildMeshes(true);
}

ObjLightDesc *plRTOmniLight::CreateLightDesc(INode *n, BOOL forceShadowBuf)
{
	return TRACKED_NEW OmniLight( n, forceShadowBuf );
}


RefTargetHandle plRTOmniLight::Clone(RemapDir &remap)
{

	plRTOmniLight *obj = TRACKED_NEW plRTOmniLight;//(plRTLightBase*) fClassDesc->Create(false);
	obj->ReplaceReference(kRefSpotLight, fLightPB->Clone(remap));
	BaseClone(this, obj, remap);

	return obj;
}

void plRTOmniLight::IBuildMeshes( BOOL isnew ) 
{
	BuildStaticMeshes();
	fMesh = staticMesh[ plRTLightBase::RT_OMNI ];
}

//// DrawConeAndLine /////////////////////////////////////////////////////////

int		plRTOmniLight::DrawConeAndLine( TimeValue t, INode* inode, GraphicsWindow *gw, int drawing ) 
{
	float	atOneHalfDist;
	Matrix3	tm = inode->GetObjectTM( t );


	gw->setTransform( tm );
	gw->clearHitCode();

	if( ( extDispFlags & EXT_DISP_ONLY_SELECTED ) )
	{
		if( GetUseAtten() )
		{
			// Draw hotspot as the point at which light is 1/2 intensity (just to help the visual)
			gw->setColor( LINE_COLOR, GetUIColor( COLOR_HOTSPOT ) );

			if( fLightPB->GetInt( kAttenTypeRadio, t ) == 0 )
				atOneHalfDist = GetAtten( t, ATTEN_END ) / ( fLightPB->GetFloat( kIntensity, t ) * plSillyLightKonstants::GetFarPowerKonst() - 1.f );
			else
				atOneHalfDist = sqrt( GetAtten( t, ATTEN_END ) * GetAtten( t, ATTEN_END ) / ( fLightPB->GetFloat( kIntensity, t ) * plSillyLightKonstants::GetFarPowerKonst() - 1.f ) );

			if( atOneHalfDist > 0.0f )
				DrawCone( t, gw, atOneHalfDist );

			gw->setColor( LINE_COLOR, GetUIColor( COLOR_FALLOFF ) );
			DrawCone( t, gw, GetAtten( t, ATTEN_END ) );
		}
		else
			DrawArrows( t, gw, 50 );
	}

	return gw->checkHitCode();
}

//// DrawCone ////////////////////////////////////////////////////////////////
//	Function called by MAX to render the cone shape in the viewport for this
//	light. Note that this is the cone, not the actual object itself!

void	plRTOmniLight::DrawCone( TimeValue t, GraphicsWindow *gw, float dist ) 
{
	Point3	pts[ NUM_CIRC_PTS * 3 + 1 ];


	/// Draw sphere-thingy
	GetAttenPoints( t, dist, pts );

	gw->polyline( NUM_CIRC_PTS, pts,					nil, nil, true, nil );
	gw->polyline( NUM_CIRC_PTS, pts + NUM_CIRC_PTS,		nil, nil, true, nil );
	gw->polyline( NUM_CIRC_PTS, pts + 2 * NUM_CIRC_PTS,	nil, nil, true, nil );
}


//// DrawArrows //////////////////////////////////////////////////////////////
//	Renders some arrows in all directions, to show a radiating, attenuation-less
//	omni light.

void	plRTOmniLight::DrawArrows( TimeValue t, GraphicsWindow *gw, float dist ) 
{
	Point3	directions[] = { Point3( 1, 0, 0 ), Point3( -1, 0, 0 ), Point3( 0, 1, 0 ), Point3( 0, -1, 0 ),
							 Point3( 0, 0, 1 ), Point3( 0, 0, -1 ), 
							 Point3( 2, 2, 0 ), Point3( 2, -2, 0 ), Point3( 2, 0, 2 ), Point3( 2, 0, -2 ),
							 Point3( -2, 2, 0 ), Point3( -2, -2, 0 ), Point3( -2, 0, 2 ), Point3( -2, 0, -2 ),
							 Point3( 0, 2, 2 ), Point3( 0, 2, -2 ), Point3( 0, -2, 2 ), Point3( 0, -2, -2 ),
							 Point3( 0, 0, 0 ) };
	Point3	empty( 0, 0, 0 );
	int		i;
	Point3	pts[ 5 ];


	/// Adjust directions
	for( i = 0; directions[ i ] != empty; i++ )
	{
		if( directions[ i ].x == 2.f )
			directions[ i ].x = 0.7f;
		else if( directions[ i ].x == -2.f )
			directions[ i ].x = -0.7f;

		if( directions[ i ].y == 2.f )
			directions[ i ].y = 0.7f;
		else if( directions[ i ].y == -2.f )
			directions[ i ].y = -0.7f;

		if( directions[ i ].z == 2.f )
			directions[ i ].z = 0.7f;
		else if( directions[ i ].z == -2.f )
			directions[ i ].z = -0.7f;
	}

	/// Draw da arrows
	gw->setColor( LINE_COLOR, GetUIColor( COLOR_HOTSPOT ) );
	for( i = 0; directions[ i ] != empty; i++ )
		DrawArrow( t, gw, directions[ i ], dist );
}

//// GetLocalBoundBox ////////////////////////////////////////////////////////

void	plRTOmniLight::GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box )
{
	Point3	loc = node->GetObjectTM( t ).GetTrans();
	float	scaleFactor = vpt->NonScalingObjectSize() * vpt->GetVPWorldWidth( loc ) / 360.0f;
	float	width;

	box = fMesh.getBoundingBox();
	// Because we want to scale about the origin, not the box center, we have to do this funky offset
	Point3	boxCenter = box.Center();
	box.Translate( -boxCenter );
	box.Scale( scaleFactor );
	boxCenter *= scaleFactor;
	box.Translate( boxCenter );

	// Include points for the spotlight. That means either the attenuated cone or 
	// our unattenuated cone display
	if( ( extDispFlags & EXT_DISP_ONLY_SELECTED ) )
	{
		if( GetUseAtten() ) 
			width = GetAtten( t, ATTEN_END );
		else
			width = 50.f;		// Include our arrows

		box += Point3( -width, -width, -width );
		box += Point3( width, width, width );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//  
//
//			SpotLight Stuff
//
//


//// Local Copy of shared Anim PB /////////////////////////////////////////////

plRTSpotLight::plRTSpotLight()
{
	fIP = NULL; 
	fLightPB = NULL; 
	fClassDesc = plRTSpotLightDesc::GetDesc();
	fClassDesc->MakeAutoParamBlocks(this);

	fLightPB->SetValue(kLightColor, 0,  Color(255,255,255));
	SetHSVColor(0, Point3(255, 255, 255));
	
	fTex = NULL;
	meshBuilt = 0; 
	
	IBuildMeshes(true);
}

ObjLightDesc *plRTSpotLight::CreateLightDesc(INode *n, BOOL forceShadowBuf)
{
	return TRACKED_NEW SpotLight( n, forceShadowBuf );
}


RefTargetHandle plRTSpotLight::Clone(RemapDir &remap)
{

	plRTSpotLight *obj = TRACKED_NEW plRTSpotLight;//(plRTLightBase*) fClassDesc->Create(false);
	obj->ReplaceReference(kRefSpotLight, fLightPB->Clone(remap));
	BaseClone(this, obj, remap);
	return obj;
}

Texmap	*plRTSpotLight::GetProjMap()
{
	if( !fLightPB->GetInt( kUseProjectorBool ) )
		return nil;

	Interval valid = Interval(0,0); 
	if( !GetTex() )
	{
		if( fLightPB->GetInt( kUseProjectorBool ) )
		{
			PBBitmap* bitmap = fLightPB->GetBitmap( kProjMapTexButton, 0 );			
			SetProjMap( &bitmap->bi );
		}
	}

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

	return (Texmap *)GetTex();
}

void	plRTSpotLight::IBuildMeshes( BOOL isnew ) 
{
	float val = fLightPB->GetFloat( kHotSpot, TimeValue(0) );		//Init val of HotSpot
	if( isnew ) 
	{
		val = fLightPB->GetFloat(kHotSpot, TimeValue(0));
		SetHotspot(  TimeValue(0), val);
		//val = 45.0;
		val = fLightPB->GetFloat(kFallOff, TimeValue(0));
		SetFallsize(  TimeValue(0), val);
		val = fLightPB->GetFloat(kAttenMaxFalloffEdit, TimeValue(0)); //fLightPB->GetFloat(kTargetDist, TimeValue(0));
		if(val < 1.0f)
			SetTDist(  TimeValue(0), DEF_TDIST);
		else
			SetTDist(  TimeValue(0), val);

		val = fLightPB->GetFloat(kHotSpot, TimeValue(0));
	
	}

	BuildSpotMesh( val );

	fMesh = spotMesh;
}

//// DrawConeAndLine /////////////////////////////////////////////////////////

int		plRTSpotLight::DrawConeAndLine( TimeValue t, INode* inode, GraphicsWindow *gw, int drawing ) 
{
	Matrix3	tm = inode->GetObjectTM( t );


	gw->setTransform( tm );
	gw->clearHitCode();

	if( extDispFlags & EXT_DISP_ONLY_SELECTED )
		DrawCone( t, gw, GetAtten( t, ATTEN_END ) );

	return gw->checkHitCode();
}

//// DrawCone ////////////////////////////////////////////////////////////////
//	Function called by MAX to render the cone shape in the viewport for this
//	light. Note that this is the cone, not the actual object itself!

void	plRTSpotLight::DrawCone( TimeValue t, GraphicsWindow *gw, float dist ) 
{
	int		i;
	Point3	pts[ NUM_CIRC_PTS + 1 ], u[ 3 ];


	if( !GetUseAtten() )
	{
		/// Don't use atten, but still want a cone, so draw the cone w/ a dist of 100
		/// and the lines extending past it (thus indicating that it keeps going)
		dist = 100;
	}

	/// Draw hotspot cone
	gw->setColor( LINE_COLOR, GetUIColor( COLOR_HOTSPOT ) );
	GetConePoints( t, -1.0f, GetHotspot( t ), dist, pts );
	gw->polyline( NUM_CIRC_PTS, pts, nil, nil, true, nil );

	if( GetUseAtten() )
	{
		u[ 0 ] = Point3( 0, 0, 0 );
		for( i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX )
		{
			u[ 1 ] = pts[ i ];
			gw->polyline( 2, u, nil, nil, true, nil );
		}
	}
	else
	{
		for( i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX )
		{
			pts[ i ] = pts[ i ].Normalize();
			DrawArrow( t, gw, pts[ i ], dist + 50.f );
		}
	}

	/// Draw falloff cone if necessary
	if( GetHotspot( t ) < GetFallsize( t ) )
	{
		gw->setColor( LINE_COLOR, GetUIColor( COLOR_FALLOFF ) );
		GetConePoints( t, -1.0f, GetFallsize( t ), dist, pts );
		gw->polyline( NUM_CIRC_PTS, pts, nil, nil, true, nil );

		if( GetUseAtten() )
		{
			u[ 0 ] = Point3( 0, 0, 0 );
			for( i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX )
			{
				u[ 1 ] = pts[ i ];
				gw->polyline( 2, u, nil, nil, true, nil );
			}
		}
		else
		{
			for( i = 0; i < NUM_CIRC_PTS; i += SEG_INDEX )
			{
				pts[ i ] = pts[ i ].Normalize();
				DrawArrow( t, gw, pts[ i ], dist + 50.f );
			}
		}
	}
}

//// GetLocalBoundBox ////////////////////////////////////////////////////////

void	plRTSpotLight::GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box )
{
	Point3	loc = node->GetObjectTM( t ).GetTrans();
	float	scaleFactor = vpt->NonScalingObjectSize() * vpt->GetVPWorldWidth( loc ) / 360.0f;
	float	width, depth;

	box = fMesh.getBoundingBox();
	// Because we want to scale about the origin, not the box center, we have to do this funky offset
	Point3	boxCenter = box.Center();
	box.Translate( -boxCenter );
	box.Scale( scaleFactor );
	boxCenter *= scaleFactor;
	box.Translate( boxCenter );

	// Include points for the spotlight. That means either the attenuated cone or 
	// our unattenuated cone display
	if( ( extDispFlags & EXT_DISP_ONLY_SELECTED ) )
	{
		if( GetUseAtten() ) 
			depth = GetAtten( t, ATTEN_END );
		else
			depth = 100.f + 50.f;	// Include arrows

		width = depth * tan( DegToRad( GetFallsize( t ) / 2.f ) );

		box += Point3( -width, -width, 0.f );
		box += Point3( width, width, -depth );
	}
}

///////////////////////////////////////////////////////////////////////////////
//// Directional Light ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plRTDirLight::plRTDirLight()
{
	fIP = NULL; 
	fLightPB = NULL; 
	fClassDesc = plRTDirLightDesc::GetDesc();
	fClassDesc->MakeAutoParamBlocks(this);

	fLightPB->SetValue(kLightColor, 0,  Color(255,255,255));
	SetHSVColor(0, Point3(255, 255, 255));
	
	fTex = NULL;
	meshBuilt = 0; 
	
	IBuildMeshes(true);
}

ObjLightDesc *plRTDirLight::CreateLightDesc(INode *n, BOOL forceShadowBuf)
{
	return TRACKED_NEW DirLight( n, forceShadowBuf );
}


RefTargetHandle plRTDirLight::Clone(RemapDir &remap)
{

	plRTDirLight *obj = TRACKED_NEW plRTDirLight;//(plRTLightBase*) fClassDesc->Create(false);
	obj->ReplaceReference(kRefDirLight, fLightPB->Clone(remap));
	BaseClone(this, obj, remap);
	return obj;
}

//// IBuildMeshes ////////////////////////////////////////////////////////////

void	plRTDirLight::IBuildMeshes( BOOL isnew ) 
{
	BuildStaticMeshes();

	fMesh = staticMesh[ plRTLightBase::RT_OMNI + 1 ];
}

//// DrawCone ////////////////////////////////////////////////////////////////
//	Function called by MAX to render the cone shape in the viewport for this
//	light. Note that this is the cone, not the actual object itself!

void	plRTDirLight::DrawCone( TimeValue t, GraphicsWindow *gw, float dist ) 
{
	Point3	arrow[ 7 ];
	int		i, j, r;
	float	d;
	const float	spacing = 20.f;


	// Draw some funky arrows to represent our direction
	dist = 100.f;
	gw->setColor( LINE_COLOR, GetUIColor( COLOR_HOTSPOT ) );

	for( i = -2; i <= 2; i++ )
	{
		for( j = -2; j <= 2; j++ )
		{
			r = ( i * i ) + ( j * j );
			if( r <= 4 )
			{
				d = dist * ( 5 - r ) / 5;
				IBuildZArrow( i * spacing, j * spacing, -d, -10.f, arrow );
				gw->polyline( 6, arrow, nil, nil, true, nil );
			}
		}
	}
}

//// IBuildZArrow ////////////////////////////////////////////////////////////

void	plRTDirLight::IBuildZArrow( float x, float y, float zDist, float arrowSize, Point3 *pts )
{
	pts[ 0 ] = Point3( x, y, 0.f );
	pts[ 1 ] = Point3( x, y, zDist );
	pts[ 2 ] = Point3( x + arrowSize / 2.f, y, zDist - arrowSize );
	pts[ 3 ] = Point3( x, y, zDist - arrowSize );
	pts[ 4 ] = Point3( x, y + arrowSize / 2.f, zDist - arrowSize );
	pts[ 5 ] = Point3( x, y, zDist );
}

//// GetLocalBoundBox ////////////////////////////////////////////////////////

void	plRTDirLight::GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box )
{
	Point3	loc = node->GetObjectTM( t ).GetTrans();
	float	scaleFactor = vpt->NonScalingObjectSize() * vpt->GetVPWorldWidth( loc ) / 360.0f;
	float	width, height, depth;

	box = fMesh.getBoundingBox();
	// Because we want to scale about the origin, not the box center, we have to do this funky offset
	Point3	boxCenter = box.Center();
	box.Translate( -boxCenter );
	box.Scale( scaleFactor );
	boxCenter *= scaleFactor;
	box.Translate( boxCenter );

	if( ( extDispFlags & EXT_DISP_ONLY_SELECTED ) )
	{
		width = 2 * 20.f + ( 10.f / 2.f );	// Add in half arrow size
		height = 2 * 20.f + ( 10.f / 2.f );
		depth = 100.f;

		box += Point3( -width, -height, 0.f );
		box += Point3( width, height, -depth );
	}
}
