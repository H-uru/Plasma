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
//	plRTProjDirLight.h - Header for the derived MAX RT projected directional //
//						 light												 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	8.2.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRTProjDirLight_h
#define _plRTProjDirLight_h

#include "plRealTimeLightBase.h"
#include "iparamm2.h"


///////////////////////////////////////////////////////////////////////////////
//// plRTProjPBAccessor ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTProjPBAccessor : public PBAccessor
{
	public:
		void Set( PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t );
		void Get( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid );

		static plRTProjPBAccessor	*Instance( void ) { return &fAccessor; }

	protected:

		static plRTProjPBAccessor	fAccessor;
};

///////////////////////////////////////////////////////////////////////////////
//// Projected Directional Light //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class plRTProjDirLight : public plRTLightBase
{
	public:

		friend class plRTProjPBAccessor;

		enum Blocks
		{
			kBlkProj = kBlkDerivedStart
		};

		enum References
		{
			kRefMainRollout = kRefDerivedStart,
			kRefProjRollout,

			kNumRefs
		};

		enum ProjRollout
		{
			kWidth,
			kHeight,
			kRange,
			kShowCone,
			kProjMap,
			kTexmap,

			// Someone goofed and used this index from the base class in our param block.
			// luckily it worked because there was no overlap, but we can't change it to
			// clean it up without breaking everything that uses it. So hopefully this
			// will at least clarify what's going on and prevent someone from stepping on
			// the index later.
			kProjTypeRadio = plRTLightBase::kProjTypeRadio,
		};

		plRTProjDirLight();

		/// Class ID stuff
		Class_ID	ClassID( void ) { return RTPDIR_LIGHT_CLASSID; }		
		SClass_ID	SuperClassID( void ) { return LIGHT_CLASS_ID; }

		ObjLightDesc	*CreateLightDesc( INode *n, BOOL forceShadowBuf = FALSE );
		GenLight		*NewLight( int type ) { return TRACKED_NEW plRTProjDirLight(); }
		RefTargetHandle	Clone( RemapDir &remap );

		int				CanConvertToType( Class_ID obtype ) { return ( obtype ==  RTPDIR_LIGHT_CLASSID ) ? 1 : 0; }

		virtual void	GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box );
		virtual int		DrawConeAndLine( TimeValue t, INode* inode, GraphicsWindow *gw, int drawing );
		virtual void	DrawCone( TimeValue t, GraphicsWindow *gw, float dist );

		virtual BOOL			IsDir( void )	{ return TRUE; }
		virtual RefTargetHandle GetReference( int i );
		virtual void			SetReference( int ref, RefTargetHandle rtarg );
		virtual int				NumRefs() { return kNumRefs; }

		virtual int				NumSubs() { return 2; }
		virtual TSTR			SubAnimName( int i );
		virtual Animatable		*SubAnim( int i );

		virtual int				NumParamBlocks();
		virtual IParamBlock2	*GetParamBlock( int i );
		virtual IParamBlock2	*GetParamBlock2() { return fLightPB; }
		virtual IParamBlock2	*GetParamBlockByID( BlockID id );

		virtual Texmap			*GetProjMap();
		
		virtual void			InitNodeName( TSTR &s ) { s = _T( "RTProjDirLight" ); }

		// To get using-light-as-camera-viewport to work
		virtual int				GetSpotShape( void ) { return RECT_LIGHT; }
		virtual float			GetAspect( TimeValue t, Interval &valid = Interval(0,0) );
		virtual float			GetFallsize( TimeValue t, Interval &valid = Interval(0,0) );
		virtual int				Type() { return DIR_LIGHT; }
		virtual float			GetTDist( TimeValue t, Interval &valid = Interval(0,0) );
		virtual void			SetFallsize( TimeValue time, float f ); 

		RefResult				EvalLightState(TimeValue t, Interval& valid, LightState *ls);

	protected:

		IParamBlock2	*fProjPB;

		virtual void	IBuildMeshes( BOOL isNew );
		
		void			IBuildRectangle( float width, float height, float z, Point3 *pts );
};

#endif	// _plRTProjDirLight_h