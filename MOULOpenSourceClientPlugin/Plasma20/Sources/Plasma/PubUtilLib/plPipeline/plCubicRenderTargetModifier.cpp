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
//	plCubicRenderTargetModifier Class Functions								 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	7.20.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plCubicRenderTargetModifier.h"
#include "plCubicRenderTarget.h"
#include "plgDispatch.h"
#include "plPipeline.h"
#include "plDrawable.h"
#include "hsBounds.h"

#include "../plScene/plRenderRequest.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnMessage/plTimeMsg.h"
#include "../plMessage/plRenderRequestMsg.h"
#include "hsResMgr.h"
#include "hsTimer.h"


//// Constructor & Destructor /////////////////////////////////////////////////

plCubicRenderTargetModifier::plCubicRenderTargetModifier()
{
	fTarget = nil;
	fCubic = nil;

	fRequests[ 0 ] = fRequests[ 1 ] = fRequests[ 2 ] = fRequests[ 3 ] = fRequests[ 4 ] = fRequests[ 5 ] = nil;
}

plCubicRenderTargetModifier::~plCubicRenderTargetModifier()
{
	int		i;


	for( i = 0; i < 6; i++ )
		delete fRequests[ i ];
}

//// ICreateRenderRequest /////////////////////////////////////////////////////
//	Creates a blank renderRequest to use for fun stuff.

void	plCubicRenderTargetModifier::ICreateRenderRequest( int face )
{
	plRenderRequest	*rr = fRequests[ face ];
	hsColorRGBA		c;
	
	
	if( rr == nil )
		rr = fRequests[ face ] = TRACKED_NEW plRenderRequest;

	UInt32 renderState 
		= plPipeline::kRenderNormal
		| plPipeline::kRenderClearColor
		| plPipeline::kRenderClearDepth;
	rr->SetRenderState( renderState );

	rr->SetDrawableMask( plDrawable::kNormal );
	rr->SetSubDrawableMask( plDrawable::kSubAllTypes );

	rr->SetHither(0.3f); // MF_HORSE ????
	rr->SetYon(1000.f); // MF_HORSE ????

	rr->SetFovX( 90 );
	rr->SetFovY( 90 );

	c.Set( 0, 0, 0, 1 );
	rr->SetClearColor( c );
	rr->SetClearDepth( 1.f );

	rr->SetClearDrawable( nil );
	rr->SetRenderTarget( fCubic->GetFace( face ) );
}

//// IEval ////////////////////////////////////////////////////////////////////

hsBool	plCubicRenderTargetModifier::IEval( double secs, hsScalar del, UInt32 dirty )
{
	hsPoint3	center;
	hsMatrix44	mtx, invMtx;
	int			i;

	plRenderRequestMsg	*msg;


	if( fCubic == nil || fTarget == nil )
		return true;

	/// Get center point for RT
	plCoordinateInterface	*ci = IGetTargetCoordinateInterface( 0 );
	if( ci == nil )
	{
		plDrawInterface	*di = IGetTargetDrawInterface( 0 );
		center = di->GetWorldBounds().GetCenter();
	}
	else
		center = ci->GetLocalToWorld().GetTranslate();

	/// Set camera position of RT to this center
	fCubic->SetCameraMatrix(center);

	/// Submit render requests!
	for( i = 0; i < 6; i++ )
	{
		if( fRequests[ i ] != nil )
		{
			fRequests[ i ]->SetCameraTransform(fCubic->GetWorldToCamera(i), fCubic->GetCameraToWorld(i));

			msg = TRACKED_NEW plRenderRequestMsg( nil, fRequests[ i ] );
			plgDispatch::MsgSend( msg );
		}
	}

	/// Done!
	return true;
}

//// MsgReceive ///////////////////////////////////////////////////////////////

hsBool	plCubicRenderTargetModifier::MsgReceive( plMessage* msg )
{
	plSceneObject		*scene;
	plCubicRenderTarget	*cubic;
	int					i;


	plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
	if( eval )
	{
		const double secs = eval->DSeconds();
		const hsScalar del = eval->DelSeconds();
		IEval( secs, del, 0 );
		return true;
	}
	plRefMsg			*refMsg = plRefMsg::ConvertNoRef( msg );
	if( refMsg )
	{
		if( scene = plSceneObject::ConvertNoRef( refMsg->GetRef() ) )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				AddTarget( scene );
			else
				RemoveTarget( scene );
		}
		if( cubic = plCubicRenderTarget::ConvertNoRef( refMsg->GetRef() ) )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{
				fCubic = cubic;
				for( i = 0; i < 6; i++ )
					ICreateRenderRequest( i );
			}
			else
			{
				fCubic = nil;
				for( i = 0; i < 6; i++ )
				{
					delete fRequests[ i ];
					fRequests[ i ] = nil;
				}
			}
		}
		return true;
	}

	return plModifier::MsgReceive( msg );
}

//// AddTarget ////////////////////////////////////////////////////////////////

void	plCubicRenderTargetModifier::AddTarget( plSceneObject *so )
{
	if( fTarget != nil )
		RemoveTarget( fTarget );

	fTarget = so;
	plgDispatch::Dispatch()->RegisterForExactType( plEvalMsg::Index(), GetKey() );
}

//// RemoveTarget /////////////////////////////////////////////////////////////

void	plCubicRenderTargetModifier::RemoveTarget( plSceneObject *so )
{
	fTarget = nil;
}

//// Read /////////////////////////////////////////////////////////////////////

void	plCubicRenderTargetModifier::Read( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Read( s, mgr );

	plGenRefMsg* msg;
	msg = TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, 0 ); // SceneObject
	mgr->ReadKeyNotifyMe( s, msg, plRefFlags::kActiveRef );

	msg = TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, 0 ); // cubicRT
	mgr->ReadKeyNotifyMe( s, msg, plRefFlags::kActiveRef );
}

//// Write ////////////////////////////////////////////////////////////////////

void	plCubicRenderTargetModifier::Write( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Write(s, mgr);

	mgr->WriteKey( s, fTarget ); // Write the SceneNode
	mgr->WriteKey( s, fCubic );	// Write the cubicRT
}