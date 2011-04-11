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
//	plRealTimeLights - Header for the derived MAX RT light type plug-ins	 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.2.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef PL_RTLIGHT_H
#define PL_RTLIGHT_H

#include "plRealTimeLightBase.h"
#include "iparamm2.h"
#include "resource.h"

class plMaxNode;



///////////////////////////////////////////////////////////////////////////////
//// Omni Light ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTOmniLight : public plRTLightBase
{
	public:

		Class_ID ClassID() { return RTOMNI_LIGHT_CLASSID; }		
		SClass_ID SuperClassID() { return LIGHT_CLASS_ID; }

		int CanConvertToType(Class_ID obtype) { return (obtype ==  RTOMNI_LIGHT_CLASSID ) ? 1 : 0; }
		plRTOmniLight();
			
		ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuf= FALSE);
		GenLight* NewLight(int type) {return TRACKED_NEW plRTOmniLight();} 
		RefTargetHandle Clone(RemapDir &remap);

		virtual void			InitNodeName( TSTR &s ) { s = _T( "RTOmniLight" ); }

		virtual int				DrawConeAndLine( TimeValue t, INode* inode, GraphicsWindow *gw, int drawing );
		virtual void			DrawCone( TimeValue t, GraphicsWindow *gw, float dist );
		virtual void			DrawArrows( TimeValue t, GraphicsWindow *gw, float dist );
		virtual void			GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box );

	protected:
		virtual void	IBuildMeshes( BOOL isNew );
		virtual hsBool	IHasAttenuation( void ) { return true; }
};

class plRTOmniLightDesc : public ClassDesc2 
{
	public:
	int 			IsPublic()						{ return TRUE; }
	void*			Create(BOOL loading)			{ return TRACKED_NEW plRTOmniLight; }
	const TCHAR*	ClassName()						{ return GetString(IDS_DB_OMNI); }
	SClass_ID		SuperClassID()					{ return LIGHT_CLASS_ID; }
	Class_ID		ClassID()						{ return RTOMNI_LIGHT_CLASSID; }
	const TCHAR* 	Category()						{ return _T("Plasma RunTime");}
	const TCHAR*	InternalName()					{ return _T("plRTOmni"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance()						{ return hInstance; }


	static plRTOmniLightDesc	fStaticDesc;

	static ClassDesc2	*GetDesc( void )		{ return &fStaticDesc; }
};


///////////////////////////////////////////////////////////////////////////////
//// Spotlight ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTSpotLight : public plRTLightBase
{
	public:
		
		plRTSpotLight(BOOL loading);

		plRTSpotLight();

		Class_ID ClassID() { return RTSPOT_LIGHT_CLASSID; }		
		SClass_ID SuperClassID() { return LIGHT_CLASS_ID; }

		int CanConvertToType(Class_ID obtype) { return (obtype ==  RTSPOT_LIGHT_CLASSID ) ? 1 : 0; }
		ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuf= FALSE);
		GenLight* NewLight(int type) {return TRACKED_NEW plRTSpotLight();}
		RefTargetHandle Clone(RemapDir &remap);
		
		virtual Texmap	*GetProjMap();

		virtual BOOL	IsSpot( void )	{ return TRUE; }
		virtual int		GetProjector() { return fLightPB->GetInt( kUseProjectorBool, 0 ); }

		virtual void			InitNodeName( TSTR &s ) { s = _T( "RTSpotLight" ); }

		virtual int		DrawConeAndLine( TimeValue t, INode* inode, GraphicsWindow *gw, int drawing );
		virtual void	DrawCone( TimeValue t, GraphicsWindow *gw, float dist );
		virtual void	GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box );

	protected:
		virtual void	IBuildMeshes( BOOL isNew );
		virtual hsBool	IHasAttenuation( void ) { return true; }
};

class plRTSpotLightDesc : public ClassDesc2 
{
	public:
	int 			IsPublic()						{ return TRUE; }
	void*			Create(BOOL loading = FALSE)	{ return TRACKED_NEW plRTSpotLight; }
	const TCHAR*	ClassName()						{ return GetString(IDS_DB_FREE_SPOT); }
	SClass_ID		SuperClassID()					{ return LIGHT_CLASS_ID; }
	Class_ID		ClassID()						{ return RTSPOT_LIGHT_CLASSID; }
	const TCHAR* 	Category()						{ return _T("Plasma RunTime"); }
	const TCHAR*	InternalName()					{ return _T("RTSpot"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance()						{ return hInstance; }

	static plRTSpotLightDesc	fStaticDesc;

	static ClassDesc2	*GetDesc( void )		{ return &fStaticDesc; }

};


///////////////////////////////////////////////////////////////////////////////
//// Directional Light ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTDirLight : public plRTLightBase
{
	public:

		plRTDirLight();

		Class_ID ClassID() { return RTDIR_LIGHT_CLASSID; }		
		SClass_ID SuperClassID() { return LIGHT_CLASS_ID; }

		ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuf= FALSE);
		GenLight* NewLight(int type) {return TRACKED_NEW plRTDirLight();}
		RefTargetHandle Clone(RemapDir &remap);

		int CanConvertToType(Class_ID obtype) { return (obtype ==  RTDIR_LIGHT_CLASSID ) ? 1 : 0; }

		virtual void	DrawCone(TimeValue t, GraphicsWindow *gw, float dist);
		virtual void	GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box );

		virtual BOOL IsDir( void )	{ return TRUE; }

		virtual void			InitNodeName( TSTR &s ) { s = _T( "RTDirLight" ); }

	protected:
		virtual void	IBuildMeshes( BOOL isNew );

		void	IBuildZArrow( float x, float y, float zDist, float arrowSize, Point3 *pts );
};

class plRTDirLightDesc : public ClassDesc2 
{
	public:
	int 			IsPublic()						{ return TRUE; }
	void*			Create(BOOL loading)			{ return TRACKED_NEW plRTDirLight; }
	const TCHAR*	ClassName()						{ return GetString(IDS_DB_DIRECTIONAL); }
	SClass_ID		SuperClassID()					{ return LIGHT_CLASS_ID; }
	Class_ID		ClassID()						{ return RTDIR_LIGHT_CLASSID; }
	const TCHAR* 	Category()						{ return _T("Plasma RunTime");}
	const TCHAR*	InternalName()					{ return _T("RTDir"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance()						{ return hInstance; }

	static plRTDirLightDesc		fStaticDesc;

	static ClassDesc2	*GetDesc( void )		{ return &fStaticDesc; }

};


#endif