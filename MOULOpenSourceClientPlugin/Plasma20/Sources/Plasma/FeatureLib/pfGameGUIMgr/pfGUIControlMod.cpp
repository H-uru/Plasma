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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfGUIControlMod Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIControlMod.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIListElement.h"	// Includes dropTargetProc

#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plAudioInterface.h"

#include "../plGImage/plDynamicTextMap.h"
#include "../plSurface/plLayer.h"
#include "../plMessage/plRenderMsg.h"
#include "../pnMessage/plSoundMsg.h"
#include "plPipeline.h"

#include "../plDrawable/plAccessGeometry.h"
#include "../plDrawable/plAccessSpan.h"
#include "../plDrawable/plAccessVtxSpan.h"

#include "pfGUIPopUpMenu.h"		// For skin, can we move that please? Thank you

#include "plgDispatch.h"
#include "hsResMgr.h"


//// pfGUIColorScheme Functions //////////////////////////////////////////////

void	pfGUIColorScheme::IReset( void )
{
	fForeColor.Set( 1, 1, 1, 1 );
	fBackColor.Set( 0, 0, 0, 1 );
	fSelForeColor.Set( 1, 1, 1, 1 );
	fSelBackColor.Set( 0, 0, 1, 1 );
	fTransparent = false;
	fFontFace = hsStrcpy( "Times New Roman" );
	fFontSize = 10;
	fFontFlags = 0;
}

pfGUIColorScheme::pfGUIColorScheme()
{
	IReset();
}

pfGUIColorScheme::~pfGUIColorScheme()
{
	delete [] fFontFace;
}

pfGUIColorScheme::pfGUIColorScheme( hsColorRGBA &foreColor, hsColorRGBA &backColor )
{
	IReset();
	fForeColor = foreColor;
	fBackColor = backColor;
}

pfGUIColorScheme::pfGUIColorScheme( const char *face, UInt8 size, UInt8 fontFlags )
{
	IReset();
	fFontFace = hsStrcpy( face );
	fFontSize = size;
	fFontFlags = fontFlags;
}

void	pfGUIColorScheme::SetFontFace( const char *face )
{
	delete [] fFontFace;
	fFontFace = hsStrcpy( face );
}

void	pfGUIColorScheme::Read( hsStream *s )
{
	fForeColor.Read( s );
	fBackColor.Read( s );
	fSelForeColor.Read( s );
	fSelBackColor.Read( s );
	s->ReadSwap( &fTransparent );

	delete [] fFontFace;
	fFontFace = s->ReadSafeString();
	s->ReadSwap( &fFontSize );
	s->ReadSwap( &fFontFlags );
}

void	pfGUIColorScheme::Write( hsStream *s )
{
	fForeColor.Write( s );
	fBackColor.Write( s );
	fSelForeColor.Write( s );
	fSelBackColor.Write( s );
	s->WriteSwap( fTransparent );

	s->WriteSafeString( fFontFace );
	s->WriteSwap( fFontSize );
	s->WriteSwap( fFontFlags );
}

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIControlMod::pfGUIControlMod()
{
	fEnabled = true;
	fDialog = nil;
	fBoundsValid = false;
	fCenterValid = false;
	fFocused = false;
	fInteresting = false;
	fVisible = true;
	fHandler = nil;
	fTagID = 0;
	fDropTargetHdlr = nil;
	fDynTextMap = nil;
	fProxy = nil;

	fColorScheme = nil;
	fSkin = nil;
	
	fNotifyOnInteresting = false;
}

pfGUIControlMod::~pfGUIControlMod()
{
	ISetHandler( nil );
	SetDropTargetHdlr( nil );
	SetColorScheme( nil );
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIControlMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
//	UpdateBounds();
	return false;
}

//// GetBounds ///////////////////////////////////////////////////////////////

const hsBounds3	&pfGUIControlMod::GetBounds( void )
{
	UpdateBounds();
	return fBounds; 
}

//// SetTransform ////////////////////////////////////////////////////////////
//	Override from plModifier so we can update our bounds

void	pfGUIControlMod::SetTransform( const hsMatrix44 &l2w, const hsMatrix44 &w2l )
{
	fBoundsValid = false;
}


//// GetVectorAngle //////////////////////////////////////////////////////////

static hsScalar	GetVectorAngle( const hsPoint3 &basePt, const hsPoint3 &pointA, const hsPoint3 &pointB )
{
	hsVector3	vectorA( &pointA, &basePt ), vectorB( &pointB, &basePt );

	hsScalar dot = vectorA * vectorB;
	hsVector3 cross = vectorA % vectorB;
	hsScalar crossLen = cross.fZ;

	return atan2( crossLen, dot );
}

//// CreateConvexHull ////////////////////////////////////////////////////////
//	Algorithm is Graham's scan algorithm:
//			R.L. Graham, "An efficient algorithm for determining the convex hull of a finite 
//						planar set", Info. Proc. Lett. 1, 132-133 (1972). 
//	Note: THIS WILL DESTROY YOUR INPOINTS ARRAY.

static hsBool	CreateConvexHull( hsPoint3 *inPoints, int &numPoints )
{
	int			i, j, pointA, pointB, pointC;
	hsScalar	*angles;

	if( numPoints < 3 )
		return false;

	// Step 1: Find a point interior to our hull. Easiest is average of all our input points...
	// (plus: set the Zs of all the points to the Z of the first point, since we want to be
	//  working in 2D)
	hsPoint3	avgPoint = inPoints[ 0 ];
	for( i = 1; i < numPoints; i++ )
	{
		avgPoint += inPoints[ i ];
		inPoints[ i ].fZ = inPoints[ 0 ].fZ;
	}
	avgPoint.fX /= numPoints;
	avgPoint.fY /= numPoints;
	avgPoint.fZ /= numPoints;

	// Step 2: Sort all the in points by the angle to the X axis (vector <1,0>).
	//   Step A: Calculate all the angles

	angles = TRACKED_NEW hsScalar[ numPoints ];
	hsPoint3	xAxisPoint( avgPoint.fX + 1, avgPoint.fY, avgPoint.fZ );
	for( i = 0; i < numPoints; i++ )
		angles[ i ] = GetVectorAngle( avgPoint, inPoints[ i ], xAxisPoint );

	//   Step B: Bubble sort by the angles
	for( i = 0; i < numPoints - 1; i++ )
	{
		for( j = i + 1; j < numPoints; j++ )
		{
			if( angles[ j ] < angles[ i ] )
			{
				hsScalar tempAngle = angles[ j ];
				angles[ j ] = angles[ i ];
				angles[ i ] = tempAngle;

				hsPoint3 tempPt = inPoints[ j ];
				inPoints[ j ] = inPoints[ i ];
				inPoints[ i ] = tempPt;
			}
		}
	}

	// Step 3: Eliminate non-convex points to form the hull
	for( pointA = 0, pointB = 1, pointC = 2; pointA < numPoints && numPoints > 3; )
	{
		// Two cases of wrap-around...
		if( pointC >= numPoints )
			pointC -= numPoints;
		else if( pointC < 0 )
			pointC += numPoints;
		if( pointB >= numPoints )
			pointB -= numPoints;
		else if( pointB < 0 )
			pointB += numPoints;

		// For points A, B, and C, find the interior angle between them
		hsScalar angle = GetVectorAngle( inPoints[ pointB ], inPoints[ pointA ], inPoints[ pointC ] );
		
		// If the angle is < 180, then it's a good angle and we can advance all our points by 1...
		// Note: we have a tolerance so that we don't get points that form edges that are pretty darned close...
		const hsScalar tolerance = hsScalarPI / 90.f;
		if( angle > tolerance && angle < hsScalarPI - tolerance )
		{
			pointA++;
			pointB++;
			pointC++;
		}
		else
		{
			// Angle is > 180 degrees, this is bad. This means our middle point doesn't belong,
			// so we need to remove it
			for( i = pointB; i < numPoints - 1; i++ )
				inPoints[ i ] = inPoints[ i + 1 ];
			numPoints--;
			if( pointC > pointB )
				pointC--;
			// There's one case where point B and C could've wrapped around and so deleting that point
			// actually moves point A down by 1...
			if( pointA > pointB )
				pointA--;

			// Back up the points by 1 if possible (so we can keep checking to make sure we're still convex).
			// If not, just increment C up
			if( pointA > 0 )
			{
				pointA--;
				pointB--;
			}
			else
				pointC++;
		}
	}

	delete [] angles;

	return true;
}

//// GetObjectPoints /////////////////////////////////////////////////////////
//	Retrieves ALL of the points of a sceneObject's meshes. And I mean ALL of 
//  'em...

static void	GetObjectPoints( plSceneObject *so, hsTArray<hsPoint3> &outPoints )
{
	const plDrawInterface* di = so->GetDrawInterface();
	if( !di )
		return;

	// The following uses mf's spiffy plAccessGeometry/Spans stuff, which, in 
	// one word, kicksAss.
	hsTArray<plAccessSpan> spans;
	plAccessGeometry::Instance()->OpenRO( di, spans );

	int i;
	outPoints.Reset();
	for( i = 0; i < spans.GetCount(); i++ )
	{
		plAccessVtxSpan& vtxSrc = spans[ i ].AccessVtx();
		plAccPositionIterator iterSrc( &vtxSrc );

		for( iterSrc.Begin(); iterSrc.More(); iterSrc.Advance() )
			outPoints.Append( *iterSrc.Position() );
	}
	
	if (plAccessGeometry::Instance())
		plAccessGeometry::Instance()->Close( spans );
}
	
//// PointsOnSameSide ////////////////////////////////////////////////////////
//	Given two ends of a line segment and two points, tells you whether the
//	two points are on the same side of the line. Used in PointInTriangle().

static hsBool	PointsOnSameSide( const hsPoint3 &line1, const hsPoint3 &line2, const hsPoint3 &pointA, const hsPoint3 &pointB )
{
	hsVector3 baseVec( &line2, &line1 );
	hsVector3	cp1 = hsVector3( &pointA, &line1 ) % baseVec;
	hsVector3	cp2 = hsVector3( &pointB, &line1 ) % baseVec;
	return ( cp1.fZ * cp2.fZ > 0 ) ? true : false;
}

//// PointInTriangle /////////////////////////////////////////////////////////
//	Given three points that define a triangle and a fourth point, tells you
//	whether the fourth point is inside the triangle.

static hsBool	PointInTriangle( hsPoint3 tri1, hsPoint3 tri2, hsPoint3 tri3, const hsPoint3 &testPoint )
{
	tri1.fZ = tri2.fZ = tri3.fZ = testPoint.fZ;
	if( PointsOnSameSide( tri1, tri2, testPoint, tri3 ) &&
		PointsOnSameSide( tri2, tri3, testPoint, tri1 ) &&
		PointsOnSameSide( tri3, tri1, testPoint, tri2 ) )
		return true;
	return false;
}

//// PointInBounds ///////////////////////////////////////////////////////////
//	Tells you whether said point is in the control's bounds.

hsBool	pfGUIControlMod::PointInBounds( const hsPoint3 &point )
{
	UpdateBounds();

	if( fBounds.GetType() != kBoundsEmpty && fBounds.GetType() != kBoundsUninitialized && fBounds.IsInside( &point ) )
	{
		if( fBoundsPoints.GetCount() > 0 )
		{
			// We have a more-accurate bounds set, so use it
			int		i;


			for( i = 1; i < fBoundsPoints.GetCount() - 1; i++ )
			{
				// Test the triangle (0,i,i+1)
				if( PointInTriangle( fBoundsPoints[ 0 ], fBoundsPoints[ i ], fBoundsPoints[ i + 1 ], point ) )
					return true;
			}
			return false;
		}
		else
			return true;
	}
	return false;
}

//// CalcInitialBounds ///////////////////////////////////////////////////////
//	Called by the dialog once as soon as the dialog adds the control, so that
//	initial bounds for the control can be calced. This is used for initing
//	any dynmaic text maps, since we want to use the initial bounds to do so
//	instead of any currently animated state of the bounds.

void	pfGUIControlMod::CalcInitialBounds( void )
{
	UpdateBounds( nil, true );
	fInitialBounds = fBounds;
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUIControlMod::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	hsMatrix44	xformMatrix, projMatrix;
	hsPoint3	corners[ 8 ];
	int			i;


	if( ( !fBoundsValid || force ) && fDialog && GetTarget() )
	{
		plDrawInterface *DI = IGetTargetDrawInterface( 0 );
		if( DI == nil )
			return;

		if( HasFlag( kBetterHitTesting ) )
		{
			hsTArray<hsPoint3>	scrnPoints;

			// Create a list of points to make a 2D convex hull from
			GetObjectPoints( GetTarget(), scrnPoints );
			hsMatrix44 l2w = GetTarget()->GetLocalToWorld();
			for( i = 0; i < scrnPoints.GetCount(); i++ )
			{
				scrnPoints[ i ] = l2w * scrnPoints[ i ];
				scrnPoints[ i ] = fDialog->WorldToScreenPoint( scrnPoints[ i ] );
			}

			// Now create a convex hull from them, assuming the Zs are all the same
			int numPoints = scrnPoints.GetCount();
			if( !CreateConvexHull( scrnPoints.AcquireArray(), numPoints ) )
				return;

			// Copy & store. Also recalc our bounding box just for fun
			fBounds.MakeEmpty();
			fBoundsPoints.SetCount( numPoints );
			for( i = 0; i < numPoints; i++ )
			{
				fBoundsPoints[ i ] = scrnPoints[ i ];
				fBounds.Union( &fBoundsPoints[ i ] );
			}
		}
		else
		{
			fBounds.MakeEmpty();

			hsBounds3Ext worldBounds = DI->GetLocalBounds();
			hsMatrix44 l2w = GetTarget()->GetLocalToWorld();
			worldBounds.Transform( &l2w );

			worldBounds.GetCorners( corners );
			for( i = 0; i < 8; i++ )
			{
				hsPoint3 scrnPt = fDialog->WorldToScreenPoint( corners[ i ] );
				fBounds.Union( &scrnPt );
			}
		}

		// Calc center Z
//		if( !fCenterValid )
		{
#if 0
			corners[ 1 ] = GetTarget()->GetLocalToWorld().GetTranslate();
			float w = corners[ 1 ].fX * fXformMatrix.fMap[3][0]
					+ corners[ 1 ].fY * fXformMatrix.fMap[3][1]
					+ corners[ 1 ].fZ * fXformMatrix.fMap[3][2]
					+ 1.f * fXformMatrix.fMap[3][3];
			corners[ 1 ] = fXformMatrix * corners[ 1 ];

			corners[ 1 ].fX = ( ( corners[ 1 ].fX / corners[ 1 ].fZ ) + 1.f ) / 2.f;
			corners[ 1 ].fY = ( ( corners[ 1 ].fY / corners[ 1 ].fZ ) + 1.f ) / 2.f;
			fScreenCenter = corners[ 1 ];

//			fScreenCenter.fZ = w;


			corners[ 1 ] = GetTarget()->GetLocalToWorld().GetTranslate();
			fDialog->WorldToScreenPoint( corners[ 1 ].fX, corners[ 1 ].fY, corners[ 1 ].fZ, fScreenCenter );
			fCenterValid = true;
#else
			corners[ 1 ] = GetTarget()->GetLocalToWorld().GetTranslate();
			fScreenCenter = fDialog->WorldToScreenPoint( corners[ 1 ] );
			corners[ 1 ] = fScreenCenter;
			fCenterValid = true;
#endif
		}

		fScreenMinZ = fBounds.GetMins().fZ;

		// Manually change the bounds so we know the z ranges from at least -1 to 1, suitable for us testing against for clicks
		corners[ 0 ] = fBounds.GetCenter();
		corners[ 0 ].fZ = -1.f;
		fBounds.Union( &corners[ 0 ] );
		corners[ 0 ].fZ = 1.f;
		fBounds.Union( &corners[ 0 ] );

		fBoundsValid = true;
	}
}

//// SetObjectCenter /////////////////////////////////////////////////////////
//	Given the x/y coordinates in 0..1 space, recalcs the sceneObject position
//	and moves the object to match, retaining the stored fCenterZ coordinate

void	pfGUIControlMod::SetObjectCenter( hsScalar x, hsScalar y )
{
	hsMatrix44	xformMatrix, l2p, p2l;
	hsPoint3	center, corners[ 8 ];


	if( x > 1.f )
		x = 1.f;
	else if( x < 0.f )
		x = 0.f;
	if( y > 1.f )
		y = 1.f;
	else if( y < 0.f )
		y = 0.f;
	
	if( fDialog && GetTarget() )
	{
		plCoordinateInterface *CI = IGetTargetCoordinateInterface( 0 );
		if( CI == nil )
			return;

//		if( !fInvXformValid )
//			UpdateBounds();

		l2p = GetTarget()->GetLocalToWorld();
hsPoint3 oldPt = l2p.GetTranslate();

hsPoint3 oldScrnPt = fDialog->WorldToScreenPoint( oldPt );
hsPoint3 oldPtRedux;
fDialog->ScreenToWorldPoint( oldScrnPt.fX, oldScrnPt.fY, oldScrnPt.fZ, oldPtRedux );

		fDialog->ScreenToWorldPoint( x, y, fScreenCenter.fZ, center );

		l2p.SetTranslate( &center );
		l2p.GetInverse( &p2l );

		GetTarget()->SetTransform( l2p, p2l );

		fScreenCenter.fX = x;
		fScreenCenter.fY = y;
	}
}

void	pfGUIControlMod::SetTarget( plSceneObject *object )
{
	plSingleModifier::SetTarget( object );

	UpdateBounds();
}

//// MsgReceive //////////////////////////////////////////////////////////////

#include "plProfile.h"
plProfile_CreateTimer("Gui", "RenderSetup", GUITime);

hsBool	pfGUIControlMod::MsgReceive( plMessage *msg )
{
	plRenderMsg* rend = plRenderMsg::ConvertNoRef( msg );

	if( rend )
	{
		plProfile_BeginLap(GUITime, this->GetKey()->GetUoid().GetObjectName());
		// Only need it once
		if( ISetUpDynTextMap( rend->Pipeline() ) )
			plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), GetKey() );
		plProfile_EndLap(GUITime, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}

	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
	{
		if( refMsg->fType == kRefDynTextMap )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{
				fDynTextMap = plDynamicTextMap::ConvertNoRef( refMsg->GetRef() );
				// Register for a render msg so we can leech the material when we finally
				// have a pipeline to work with
				plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), GetKey() );
			}
			else
				fDynTextMap = nil;
			return true;
		}
		else if( refMsg->fType == kRefDynTextLayer )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fDynTextLayer = plLayerInterface::ConvertNoRef( refMsg->GetRef() );
			else
				fDynTextLayer = nil;
			return true;
		}
		else if( refMsg->fType == kRefProxy )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fProxy = plSceneObject::ConvertNoRef( refMsg->GetRef() );
			else
				fProxy = nil;
			return true;
		}
		else if( refMsg->fType == kRefSkin )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fSkin = pfGUISkin::ConvertNoRef( refMsg->GetRef() );
			else
				fSkin = nil;

			return true;
		}
	}

	return plSingleModifier::MsgReceive( msg );
}

//// ISetUpDynTextMap ////////////////////////////////////////////////////////
//	Given a pointer to a dynamic text map, regurgitates it so it matches our
//	screen res and fun stuff like that. Also sets the layer transform to give
//	us a 1:1 textel-pixel ratio, which we like.

hsBool	pfGUIControlMod::ISetUpDynTextMap( plPipeline *pipe )
{
	if( fDynTextMap == nil )
	{
		hsAssert( false, "Trying to set up a nil dynamicTextMap in a GUI control" );
		return true;
	}
	if( fDynTextLayer == nil || fInitialBounds.GetType() == kBoundsUninitialized )//|| fDialog == nil )
		return false;

	UInt32 scrnWidth, scrnHeight;
	if( !HasFlag( kScaleTextWithResolution ) )
	{
		// Scale so that there is a 1:1 pixel:textel ratio
		scrnWidth = pipe->Width();
		scrnHeight = pipe->Height();
	}
	else
	{
		// Scale with the resolution so that we take up the same % of screen space no matter what resolution
		// Assume a base "resolution" of 1024xX, where X is such that the ratio "1024/X = scrnWidth/scrnHt" holds
		const int kBaseScaleRes = 1024;
		const int kBaseScaleHeightRes = 768;
		scrnWidth = kBaseScaleRes;
		scrnHeight = kBaseScaleHeightRes;
		// we are going to just force things to be in 4 by 3 ratio...
		// ...cause it seems to work better.
///////		scrnHeight = ( pipe->Height() * kBaseScaleRes ) / pipe->Width();
	}

	const hsBounds3 &bounds = fInitialBounds;//GetBounds();
	UInt16 width = (UInt16)(( bounds.GetMaxs().fX - bounds.GetMins().fX ) * scrnWidth);
	UInt16 height = (UInt16)(( bounds.GetMaxs().fY - bounds.GetMins().fY ) * scrnHeight);

	// Allow derived controls to allocate some extra scratch space if desired
	// (Do it this way so we can pass in our current calculated dimensions for them to play with)
	UInt16 extraW = width, extraH = height;
	IGrowDTMDimsToDesiredSize( extraW, extraH );
	extraW -= width;
	extraH -= height;

	fDynTextMap->Reset();
	fDynTextMap->Create( width, height, HasFlag( kXparentBgnd ), extraW, extraH );

	fDynTextMap->SetFont( GetColorScheme()->fFontFace, GetColorScheme()->fFontSize, GetColorScheme()->fFontFlags,
							HasFlag( kXparentBgnd ) ? false : true );
	fDynTextMap->SetTextColor( GetColorScheme()->fForeColor, 
							( HasFlag( kXparentBgnd ) && GetColorScheme()->fBackColor.a == 0.f ) ? true : false );

	// Now we gotta set the texture transform on the layer so our texture comes
	// out with 1:1 mapping from textel to pixel
	plLayer	*layer = (plLayer *)fDynTextLayer;
	layer->SetTransform( fDynTextMap->GetLayerTransform() );

	// Let the derived classes do their things
	IPostSetUpDynTextMap();

	// Do our first update
	IUpdate();

	return true;
}

//// Get/SetColorScheme //////////////////////////////////////////////////////

pfGUIColorScheme	*pfGUIControlMod::GetColorScheme( void ) const
{
	if( fColorScheme == nil )
		return fDialog->GetColorScheme();

	return fColorScheme;
}

void	pfGUIControlMod::SetColorScheme( pfGUIColorScheme *newScheme )
{
	if( fColorScheme != nil )
	{
		hsRefCnt_SafeUnRef( fColorScheme );
		fColorScheme = nil;
	}

	fColorScheme = newScheme;
	if( fColorScheme != nil )
		hsRefCnt_SafeRef( fColorScheme );
}

//// SetDynTextMap ///////////////////////////////////////////////////////////
//	EXPORT ONLY

void	pfGUIControlMod::SetDynTextMap( plLayerInterface *layer, plDynamicTextMap *dynText )
{
	hsgResMgr::ResMgr()->AddViaNotify( layer->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, pfGUIControlMod::kRefDynTextLayer ), plRefFlags::kActiveRef );
	hsgResMgr::ResMgr()->AddViaNotify( dynText->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, pfGUIControlMod::kRefDynTextMap ), plRefFlags::kActiveRef );
}

//// SetEnabled //////////////////////////////////////////////////////////////

void	pfGUIControlMod::SetEnabled( hsBool e )
{
	if( e == fEnabled )
		return;

	fEnabled = e;
	IUpdate();
}

//// SetFocused //////////////////////////////////////////////////////////////

void	pfGUIControlMod::SetFocused( hsBool e )
{
	if( e == fFocused )
		return;

	fFocused = e;
	IUpdate();
}

//// SetInteresting //////////////////////////////////////////////////////////

void	pfGUIControlMod::SetInteresting( hsBool i )
{
	if( i == fInteresting )
		return;

	fInteresting = i;
	IUpdate();

	if ( fNotifyOnInteresting && fDialog && fDialog->GetHandler() )
		fDialog->GetHandler()->OnInterestingEvent(this);

}

//// SetVisible //////////////////////////////////////////////////////////////

void	pfGUIControlMod::SetVisible( hsBool vis )
{
	if( vis == fVisible )
		return;

	fVisible = vis;
	if (fTarget)
	{
		plEnableMsg	*msg = TRACKED_NEW plEnableMsg();
		msg->SetCmd( fVisible ? plEnableMsg::kEnable : plEnableMsg::kDisable );
		msg->SetCmd( plEnableMsg::kDrawable );
		msg->AddReceiver( fTarget->GetKey() );
		plgDispatch::MsgSend( msg );
	}

	if( !fVisible && fFocused )
		fDialog->SetFocus( nil );
}

void	pfGUIControlMod::Refresh( void )
{
	IUpdate();
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIControlMod::Read( hsStream *s, hsResMgr *mgr )
{
	plSingleModifier::Read(s, mgr);
	s->ReadSwap( &fTagID );
	fVisible = s->ReadBool();

	// Read the handler in
	ISetHandler( pfGUICtrlProcWriteableObject::Read( s ) );

	// Read in the dynTextMap if there is one
	if( s->ReadBool() )
	{
		mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDynTextLayer ), plRefFlags::kActiveRef );
		mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDynTextMap ), plRefFlags::kActiveRef );
	}
	else
	{
		fDynTextLayer = nil;
		fDynTextMap = nil;
	}

	if( s->ReadBool() )
	{
		SetColorScheme( nil );
		fColorScheme = TRACKED_NEW pfGUIColorScheme();
		fColorScheme->Read( s );
	}

	// Read in our sound indices
	UInt8 i, count = s->ReadByte();
	if( count == 0 )
		fSoundIndices.Reset();
	else
	{
		fSoundIndices.SetCountAndZero( count );
		for( i = 0; i < count; i++ )
			fSoundIndices[ i ] = (int)s->ReadSwap32();
	}

	if( HasFlag( kHasProxy ) )
		mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefProxy ), plRefFlags::kActiveRef );

	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );
}

void	pfGUIControlMod::Write( hsStream *s, hsResMgr *mgr )
{
	if( HasFlag( kHasProxy ) && !fProxy )
		ClearFlag( kHasProxy );

	plSingleModifier::Write( s, mgr );
	s->WriteSwap( fTagID );
	s->WriteBool( fVisible );

	// Write the handler out (if it's not a writeable, damn you)
	pfGUICtrlProcWriteableObject::Write( (pfGUICtrlProcWriteableObject *)fHandler, s );

	// Write out the dynTextMap
	if( fDynTextMap != nil )
	{
		s->WriteBool( true );
		mgr->WriteKey( s, fDynTextLayer->GetKey() );
		mgr->WriteKey( s, fDynTextMap->GetKey() );
	}
	else
		s->WriteBool( false );

	if( fColorScheme != nil )
	{
		s->WriteBool( true );
		fColorScheme->Write( s );
	}
	else
		s->WriteBool( false );

	// Write out our sound indices
	s->WriteByte( fSoundIndices.GetCount() );
	UInt8 i;
	for( i = 0; i < fSoundIndices.GetCount(); i++ )
		s->WriteSwap32( fSoundIndices[ i ] );

	if( HasFlag( kHasProxy ) )
		mgr->WriteKey( s, fProxy->GetKey() );

	mgr->WriteKey( s, fSkin );
}

//// HandleKeyPress/Event ////////////////////////////////////////////////////

hsBool	pfGUIControlMod::HandleKeyPress( char key, UInt8 modifiers ) 
{
	return false; 
}

hsBool	pfGUIControlMod::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers ) 
{
	return false; 
}

//// IScreenToLocalPt ////////////////////////////////////////////////////////

void	pfGUIControlMod::IScreenToLocalPt( hsPoint3 &pt )
{
	const hsBounds3 &bnds = GetBounds();

	pt.fX -= bnds.GetMins().fX;
	pt.fY -= bnds.GetMins().fY;
	pt.fX /= bnds.GetMaxs().fX - bnds.GetMins().fX;
	pt.fY /= bnds.GetMaxs().fY - bnds.GetMins().fY;
}

//// ISetHandler /////////////////////////////////////////////////////////////

void	pfGUIControlMod::ISetHandler( pfGUICtrlProcObject *h, hsBool clearInheritFlag )
{
	if( fHandler && fHandler->DecRef() )
		delete fHandler;

	fHandler = h; 
	if( fHandler )
		fHandler->IncRef();

	if( clearInheritFlag )
		ClearFlag( kInheritProcFromDlg );
}

//// DoSomething /////////////////////////////////////////////////////////////

void	pfGUIControlMod::DoSomething( void )
{
	if( fEnabled && fHandler != nil )
		fHandler->DoSomething( this );
}

//// HandleExtendedEvent /////////////////////////////////////////////////////

void	pfGUIControlMod::HandleExtendedEvent( UInt32 event )
{
	if( fEnabled && fHandler != nil )
		fHandler->HandleExtendedEvent( this, event );
}

//// SetDropTargetHdlr ///////////////////////////////////////////////////////

void	pfGUIControlMod::SetDropTargetHdlr( pfGUIDropTargetProc *h )
{
	if( fDropTargetHdlr && fDropTargetHdlr->DecRef() )
		delete fDropTargetHdlr;

	fDropTargetHdlr = h; 
	if( fDropTargetHdlr )
		fDropTargetHdlr->IncRef();
}

//// SetSoundIndex ///////////////////////////////////////////////////////////
//	Associates the given GUI event with an index of a sound on the target SO's
//	audioInterface. The guiCtrlEvent is specific to each type of control.

void	pfGUIControlMod::SetSoundIndex( UInt8 guiCtrlEvent, int soundIndex )
{
	if( fSoundIndices.GetCount() < guiCtrlEvent + 1 )
		fSoundIndices.ExpandAndZero( guiCtrlEvent + 1 );

	fSoundIndices[ guiCtrlEvent ] = soundIndex + 1;	// We +1, since 0 means no sound
}

//// IPlaySound //////////////////////////////////////////////////////////////
//	Sends a sound play message with the soundIndex associated with the given
//	event.

void	pfGUIControlMod::IPlaySound( UInt8 guiCtrlEvent, hsBool loop /* = false */ )
{
	if( guiCtrlEvent >= fSoundIndices.GetCount() || fSoundIndices[ guiCtrlEvent ] == 0 )
		return;

	if( GetTarget() == nil || GetTarget()->GetAudioInterface() == nil )
		return;

	plSoundMsg	*msg = TRACKED_NEW plSoundMsg;
	msg->fIndex = fSoundIndices[ guiCtrlEvent ] - 1;
	msg->SetCmd( plSoundMsg::kGoToTime );
	msg->fTime = 0.f;
	msg->SetCmd( plSoundMsg::kPlay );
	if (loop)
	{
		msg->fLoop = true;
		msg->SetCmd( plSoundMsg::kSetLooping );
	}
	msg->Send( GetTarget()->GetAudioInterface()->GetKey() );
}

void	pfGUIControlMod::IStopSound(UInt8 guiCtrlEvent)
{
	if (guiCtrlEvent >= fSoundIndices.GetCount() || fSoundIndices[guiCtrlEvent] == 0)
		return;

	if (GetTarget() == nil || GetTarget()->GetAudioInterface() == nil )
		return;

	plSoundMsg *msg = TRACKED_NEW plSoundMsg;
	msg->fIndex = fSoundIndices[guiCtrlEvent] - 1;
	msg->SetCmd(plSoundMsg::kStop);
	msg->Send(GetTarget()->GetAudioInterface()->GetKey());
}
