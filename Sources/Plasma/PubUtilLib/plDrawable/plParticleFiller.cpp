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

#include "hsTypes.h"

#include "plParticleFiller.h"

// Core background
#include "hsTemplates.h"
#include "hsFastMath.h"
#include "plPipeline.h"
#include "plViewTransform.h"

// Getting at the destination data
#include "../pnSceneObject/plDrawInterface.h"
#include "../plDrawable/plDrawableSpans.h"
#include "../plPipeline/plGBufferGroup.h"

// For shading
#include "../plGLight/plLightInfo.h"

// Getting at the source data
#include "../plParticleSystem/plParticleEmitter.h"
#include "../plParticleSystem/plParticle.h"

static hsScalar sInvDelSecs;

//// Local Static Stuff ///////////////////////////////////////////////////////

/// Macros for getting/setting data in a D3D vertex buffer
#define STUFF_POINT( ptr, point ) { float *fPtr = (float *)ptr; \
									fPtr[ 0 ] = point.fX; fPtr[ 1 ] = point.fY; fPtr[ 2 ] = point.fZ; \
									ptr += sizeof( float ) * 3; }

#define STUFF_UINT32( ptr, uint ) { UInt32 *dPtr = (UInt32 *)ptr; \
									dPtr[ 0 ] = uint; ptr += sizeof( UInt32 ); }

#define EXTRACT_POINT( ptr, pt ) { float *fPtr = (float *)ptr; \
									  pt.fX = fPtr[ 0 ]; pt.fY = fPtr[ 1 ]; pt.fZ = fPtr[ 2 ]; \
									  ptr += sizeof( float ) * 3; }
#define EXTRACT_FLOAT( ptr, f ) { float *fPtr = (float *)ptr; \
									  f = fPtr[ 0 ]; \
									  ptr += sizeof( float ); }

#define EXTRACT_UINT32( ptr, uint ) { UInt32 *dPtr = (UInt32 *)ptr; \
									  uint = dPtr[ 0 ]; ptr += sizeof( UInt32 ); }


static hsScalar sCurrMinWidth = 0;

///////////////////////////////////////////////////////////////////////////////
//// Particles ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Particle Processing Inlines //////////////////////////////////////////////
//	Thanks to the <cough> beauty of C++, the internal loop of 
//	IFillParticlePolys would be horrendous without these inlines (with them,
//	it's just slightly annoying). The goal is to make the code easier to
//	maintain without loosing speed (hence the inlines, which will *hopefully*
//	remove the function call overhead....)

void inline	IInlSetParticlePathFollow( const plParticleCore &particle, const hsMatrix44& viewToWorld, 
									  hsVector3 &xVec, hsVector3 &yVec, hsVector3 &zVec )
{

	/// Follow path specified by interpreting orientation as a velocity vector.

	hsVector3 viewDir(&particle.fPos, &viewToWorld.GetTranslate());
	hsFastMath::NormalizeAppr(viewDir);

	zVec = viewDir;

	const hsVector3& orientation = (const hsVector3)(particle.fOrientation);
	yVec = orientation - orientation.InnerProduct(viewDir) * viewDir;
	hsFastMath::NormalizeAppr(yVec);

	xVec = yVec % viewDir;
	hsFastMath::NormalizeAppr(xVec);
	xVec *= particle.fHSize;

	yVec *= particle.fVSize;
}

void inline	IInlSetParticlePathStretch( const plParticleCore &particle, const hsMatrix44& viewToWorld, 
									  hsVector3 &xVec, hsVector3 &yVec, hsVector3 &zVec )
{

	/// Follow path specified by interpreting orientation as a velocity vector.

	// Well, that's one way to do it, but it has a bit of a flaw. If the length of the
	// particle doesn't match the distance it's moved each frame, you get a nasty strobing
	// effect, particularly as particles move faster. One of those things they don't teach
	// you in the math class you slept through.
	// So what we want is for the tail of the particle this frame to be where the head of
	// the particle was last frame.
	// First thing changed was the orientation passed in is now the change in position for 
	// this frame (was the last frame's velocity).
	// zVec will still be the normalized vector from the eye to the particle (in world space).
	// Does zVec ever get used? Hmm, only gets used for the normal facing the camera.
	// This thing gets a lot faster and cleaner when we just use the view axes to compute.
	// So zVec is just -viewDir.
	// yVec = orientation - orientation.InnerProduct(zVec) * zVec
	// xVec = yVec % zVec
	// To be really correct, you can normalize xVec, because the particle doesn't get fatter
	// as it goes faster, just longer. Costs a little more, but...
	// And it also raises the question of what to do with the user supplied sizes. Again,
	// for correctness, we want to add the size to the length of the displacement.
	// We could afford that (maybe) by doing a fast normalize on yVec, then multiplying
	// by orientation.InnerProduct(yVec) + fVSize.
	// So the new stuff we need here are:
	//	The particle (to get the size out of).
	//	The viewToWorld transform (we can pull the eyePt and viewDir out of that.
	// Note that we could probably slim away a normalize or two, but the actual number
	// of normalizes we're doing hasn't gone up, I've just moved them up from IInlSetParticlePoints().

	hsVector3 viewDir(&particle.fPos, &viewToWorld.GetTranslate());
	hsScalar invD = hsFastMath::InvSqrtAppr(viewDir.MagnitudeSquared());
	viewDir *= invD;

	zVec = viewDir;

	const hsVector3& orientation = (const hsVector3)(particle.fOrientation);
	// We don't want the projection of orientation orthogonal to viewDir here,
	// it's okay (and looks better) if yVec isn't in the image plane, we just
	// want to make sure that xVec is in the image plane. Might want to make
	// the same change to IInlSetParticlePathFollow(), but I haven't checked
	// that yet. Artifact to look for is particles starting to look goofy
	// as their orientation gets close in direction to viewDir. mf
	yVec = orientation;
	hsFastMath::NormalizeAppr(yVec);

	xVec = yVec % viewDir; // cross product of two orthonormal vectors, no need to normalize.

	hsScalar xLen = particle.fHSize;
	if( xLen * invD < sCurrMinWidth )
		xLen = sCurrMinWidth / invD;
	xVec *= xLen;

	hsScalar len = yVec.InnerProduct(orientation);
	// Might want to give it a little boost to overlap itself (and compensate for the massive
	// transparent border the artists love). But they can do that themselves with the VSize.
//	len *= 1.5f; 
	len += particle.fVSize;
	yVec *= len * -1.f;
}

void inline	IInlSetParticlePathFlow( const plParticleCore &particle, const hsMatrix44& viewToWorld, 
									  hsVector3 &xVec, hsVector3 &yVec, hsVector3 &zVec )
{

	// Okay, all the notes for SetParticlePathStretch apply here too. The only
	// difference is that we're going to keep the area of the particle constant,
	// so the longer it stretches, the narrower it gets orthogonal to the velocity.

	hsVector3 viewDir(&particle.fPos, &viewToWorld.GetTranslate());
	hsScalar invD = hsFastMath::InvSqrtAppr(viewDir.MagnitudeSquared());
	viewDir *= invD;

	zVec = viewDir;

	const hsVector3& orientation = (const hsVector3)(particle.fOrientation);
	// We don't want the projection of orientation orthogonal to viewDir here,
	// it's okay (and looks better) if yVec isn't in the image plane, we just
	// want to make sure that xVec is in the image plane. Might want to make
	// the same change to IInlSetParticlePathFollow(), but I haven't checked
	// that yet. Artifact to look for is particles starting to look goofy
	// as their orientation gets close in direction to viewDir. mf
	yVec = orientation;
	hsFastMath::NormalizeAppr(yVec);

	xVec = yVec % viewDir; // cross product of two orthonormal vectors, no need to normalize.

	hsScalar len = yVec.InnerProduct(orientation);

	hsScalar xLen = particle.fHSize * hsFastMath::InvSqrtAppr(1.f + len * sInvDelSecs);
	if( xLen * invD < sCurrMinWidth )
		xLen = sCurrMinWidth / invD;
	xVec *= xLen;

	// Might want to give it a little boost to overlap itself (and compensate for the massive
	// transparent border the artists love). But they can do that themselves with the VSize.
	len += particle.fVSize;
	yVec *= len * -2.f;

}

void inline	IInlSetParticleExplicit( const hsMatrix44 &viewToWorld, const plParticleCore &particle,
									  hsVector3 &xVec, hsVector3 &yVec, hsVector3 &zVec )
{
	const hsVector3& orientation = (const hsVector3)(particle.fOrientation);
#if 0 // See notes below - mf
	zVec.Set( 0, 0, -1 );
	yVec.Set( &orientation );
	zVec = viewToWorld * zVec;
	yVec = viewToWorld * yVec;
	xVec = yVec % zVec;
#else // See notes below - mf
	// The above has a bit of a problem with wide field of view. All of the
	// particles are facing the same direction with respect to lighting,
	// even though some are to the left and some are to the right of the camera
	// (when facing the center of the system). We'll also start seeing them side on
	// as they get to the edge of the screen. Fortunately, it's actually faster
	// to calculate the vector from camera to particle and normalize it than
	// to transform the vector (0,0,-1) (though not as fast as pulling the
	// camera direction directly from the viewToWorld).
	hsVector3 del(&particle.fPos, &viewToWorld.GetTranslate());
	hsFastMath::NormalizeAppr(del);
	zVec = del;
	yVec.Set(&(viewToWorld * orientation));
	xVec = yVec % zVec;
#endif // See notes below - mf

	xVec = hsFastMath::NormalizeAppr( xVec ) * particle.fHSize;
	yVec = hsFastMath::NormalizeAppr( yVec ) * particle.fVSize;
}

void inline	IInlSetParticlePoints( const hsVector3 &xVec, const hsVector3 &yVec, const plParticleCore &particle,
								   hsPoint3 *partPts, UInt32 &partColor )
{
	/// Do the 4 verts for this particle
	partPts[ 0 ] = partPts[ 1 ] = partPts[ 2 ] = partPts[ 3 ] = particle.fPos;
	partPts[ 0 ] +=  xVec - yVec;
	partPts[ 1 ] += -xVec - yVec;
	partPts[ 2 ] += -xVec + yVec;
	partPts[ 3 ] +=  xVec + yVec;

	partColor = particle.fColor;
}

void inline	IInlSetParticlePointsStretch( const hsVector3 &xVec, const hsVector3 &yVec, const plParticleCore &particle,
								   hsPoint3 *partPts, UInt32 &partColor )
{
	/// Do the 4 verts for this particle
	partPts[ 0 ] = partPts[ 1 ] = partPts[ 2 ] = partPts[ 3 ] = particle.fPos;
	partPts[ 0 ] +=  xVec + yVec;
	partPts[ 1 ] += -xVec + yVec;
	partPts[ 2 ] += -xVec;
	partPts[ 3 ] +=  xVec;

	partColor = particle.fColor;
}

void inline	IInlStuffParticle1UV( UInt8 *&destPtr, const hsPoint3 *partPts, const hsVector3 &partNorm,
								  const UInt32 &partColor, const plParticleCore &particle )
{
	UInt8		j;


	for( j = 0; j < 4; j++ )
	{
		STUFF_POINT( destPtr, partPts[ j ] );
		STUFF_POINT( destPtr, partNorm );
		STUFF_UINT32( destPtr, partColor );
		STUFF_UINT32( destPtr, 0 );
		STUFF_POINT( destPtr, particle.fUVCoords[ j ] );
	}			
}

void inline	IInlStuffParticleNoUVs( UInt8 *&destPtr, const hsPoint3 *partPts, const hsVector3 &partNorm,
								  const UInt32 &partColor )
{
	UInt8		j;


	for( j = 0; j < 4; j++ )
	{
		STUFF_POINT( destPtr, partPts[ j ] );
		STUFF_POINT( destPtr, partNorm );
		STUFF_UINT32( destPtr, partColor );
		STUFF_UINT32( destPtr, 0 );
	}			
}

void inline	IInlSetNormalViewFace( hsVector3 &partNorm, const hsVector3 &zVec )
{
	partNorm = -zVec;
}

void inline	IInlSetNormalStrongestLight( hsVector3 &partNorm, const plParticleCore &particle, 
										const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight, const hsVector3 &zVec )
{
	if( omniLight != nil )
	{
		partNorm.Set( &particle.fPos, &omniLight->GetWorldPosition() );
		partNorm = -partNorm;
	}
	else if( directionLight != nil )
	{
		partNorm = -directionLight->GetWorldDirection();
	}
	else
		partNorm = -zVec;
	partNorm = hsFastMath::NormalizeAppr( partNorm );
}

void inline	IInlSetNormalExplicit( hsVector3 &partNorm, const plParticleCore &particle )
{
	partNorm = particle.fNormal;
}

//// IIPL Functions (Internal-Inline-Particle-Loop) ///////////////////////////
//	Function names go as follows:
//		IIPL_u_o_n()
//		where u is 1UV or 0UV (for 1 UV channel or no UV channels),
//		o is OVel (for orientation-follows-velocity) or OExp (for orientation-is-explicit)
//		and n is NViewFace (for normals facing view), NLite (for facing strongest light)
//				or NExp (for explicit)

#define IIPL_PROLOG			\
	UInt32		i, partColor; \
	hsVector3	xVec, yVec, zVec, partNorm; \
	hsPoint3	partPts[ 4 ]; \
	for( i = 0; i < numParticles; i++ )

void inline	IIPL_1UV_OVel_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFollow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OVel_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFollow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OVel_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFollow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OStr_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathStretch( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OStr_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathStretch( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OStr_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathStretch( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OFlo_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFlow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OFlo_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFlow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OFlo_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFlow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OExp_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticleExplicit( viewToWorld, particles[ i ], xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OExp_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticleExplicit( viewToWorld, particles[ i ], xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_1UV_OExp_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticleExplicit( viewToWorld, particles[ i ], xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticle1UV( destPtr, partPts, partNorm, partColor, particles[ i ] );
	}
}

void inline	IIPL_0UV_OVel_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFollow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OVel_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFollow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OVel_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFollow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OStr_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathStretch( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OStr_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathStretch( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OStr_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathStretch( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OFlo_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFlow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OFlo_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFlow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OFlo_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticlePathFlow( particles[ i ], viewToWorld, xVec, yVec, zVec );
		IInlSetParticlePointsStretch( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OExp_NViewFace( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticleExplicit( viewToWorld, particles[ i ], xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalViewFace( partNorm, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OExp_NLite( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr,
								const plOmniLightInfo *omniLight, const plDirectionalLightInfo *directionLight )
{
	IIPL_PROLOG
	{
		IInlSetParticleExplicit( viewToWorld, particles[ i ], xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalStrongestLight( partNorm, particles[ i ], omniLight, directionLight, zVec );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

void inline	IIPL_0UV_OExp_NExp( const UInt32 &numParticles, const plParticleCore *particles, const hsMatrix44& viewToWorld, UInt8 *&destPtr )
{
	IIPL_PROLOG
	{
		IInlSetParticleExplicit( viewToWorld, particles[ i ], xVec, yVec, zVec );
		IInlSetParticlePoints( xVec, yVec, particles[ i ], partPts, partColor );
		IInlSetNormalExplicit( partNorm, particles[ i ] );
		IInlStuffParticleNoUVs( destPtr, partPts, partNorm, partColor );
	}
}

//// IFillParticlePolys ///////////////////////////////////////////////////////
//	Takes a list of particles and makes the polys for them.
#include "hsTimer.h"
#include "plProfile.h"
plProfile_CreateTimer("Fill Polys", "Particles", ParticleFillPoly);

void plParticleFiller::FillParticles(plPipeline* pipe, plDrawableSpans* drawable, plParticleSpan* span)
{
	if (!span->fSource || span->fNumParticles <= 0)
		return;

	plProfile_BeginTiming(ParticleFillPoly);

	sInvDelSecs = hsTimer::GetDelSysSeconds();
	if( sInvDelSecs > 0 )
		sInvDelSecs = 1.f / sInvDelSecs;

	const plParticleCore* particles = span->fSource->GetParticleArray();
	const UInt32 numParticles = span->fNumParticles;

	plGBufferGroup* group = drawable->GetBufferGroup(span->fGroupIdx);

	UInt8* destPtr = group->GetVertBufferData(span->fVBufferIdx);

	destPtr += span->fVStartIdx * group->GetVertexSize();

	/// Get the z vector (pointing away from the camera) in worldspace
	hsMatrix44 viewToWorld = pipe->GetCameraToWorld();

	plOmniLightInfo* omniLight = nil;
	plDirectionalLightInfo* directionLight = nil;

	/// Get strongest light, if there is one, for normal generation
	if( span->GetNumLights( false ) > 0 )
	{
		omniLight = plOmniLightInfo::ConvertNoRef( span->GetLight( 0, false ) );
		directionLight = plDirectionalLightInfo::ConvertNoRef( span->GetLight( 0, false ) );
	}

	/// Fill with 1 UV channel
	if( group->GetNumUVs() == 1 )
	{
		/// Switch on orientation
		if( span->fSource->fMiscFlags & plParticleEmitter::kOrientationVelocityBased )
		{
			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_1UV_OVel_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_1UV_OVel_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_1UV_OVel_NExp( numParticles, particles, viewToWorld, destPtr );
		}
		else if( span->fSource->fMiscFlags & plParticleEmitter::kOrientationVelocityStretch )
		{
			sCurrMinWidth = pipe->GetViewTransform().GetOrthoWidth() / pipe->GetViewTransform().GetScreenWidth() * 0.75f;

			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_1UV_OStr_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_1UV_OStr_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_1UV_OStr_NExp( numParticles, particles, viewToWorld, destPtr );
		}
		else if( span->fSource->fMiscFlags & plParticleEmitter::kOrientationVelocityFlow )
		{
			sCurrMinWidth = pipe->GetViewTransform().GetOrthoWidth() / pipe->GetViewTransform().GetScreenWidth() * 0.75f;

			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_1UV_OFlo_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_1UV_OFlo_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_1UV_OFlo_NExp( numParticles, particles, viewToWorld, destPtr );
		}
		else	// Orientation explicit
		{
			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_1UV_OExp_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_1UV_OExp_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_1UV_OExp_NExp( numParticles, particles, viewToWorld, destPtr );
		}
	}
	else
	/// Fill with no UV channels
	{
		/// Switch on orientation
		if( span->fSource->fMiscFlags & plParticleEmitter::kOrientationVelocityBased )
		{
			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_0UV_OVel_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_0UV_OVel_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_0UV_OVel_NExp( numParticles, particles, viewToWorld, destPtr );
		}
		else if( span->fSource->fMiscFlags & plParticleEmitter::kOrientationVelocityStretch )
		{
			sCurrMinWidth = pipe->GetViewTransform().GetOrthoWidth() / pipe->GetViewTransform().GetScreenWidth() * 0.75f;

			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_0UV_OStr_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_0UV_OStr_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_0UV_OStr_NExp( numParticles, particles, viewToWorld, destPtr );
		}
		else if( span->fSource->fMiscFlags & plParticleEmitter::kOrientationVelocityFlow )
		{
			sCurrMinWidth = pipe->GetViewTransform().GetOrthoWidth() / pipe->GetViewTransform().GetScreenWidth() * 0.75f;

			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_0UV_OFlo_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_0UV_OFlo_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_0UV_OFlo_NExp( numParticles, particles, viewToWorld, destPtr );
		}
		else	// Orientation explicit
		{
			/// Switch on normal generation
			if( span->fSource->fMiscFlags & plParticleEmitter::kNormalViewFacing )
				IIPL_0UV_OExp_NViewFace( numParticles, particles, viewToWorld, destPtr );

			else if( span->fSource->fMiscFlags & plParticleEmitter::kNormalNearestLight )
				IIPL_0UV_OExp_NLite( numParticles, particles, viewToWorld, destPtr, omniLight, directionLight );

			else
				IIPL_0UV_OExp_NExp( numParticles, particles, viewToWorld, destPtr );
		}
	}

	/// All done!
	plProfile_EndTiming(ParticleFillPoly);
}

void plParticleFiller::FillParticlePolys(plPipeline* pipe, plDrawInterface* di)
{
	if( !di )
		return; // should probably be an asserted error.

	plDrawableSpans* drawable = plDrawableSpans::ConvertNoRef(di->GetDrawable(0));
	if( !drawable )
		return;

	// Currently, the di always points to exactly 1 drawable with 1 span index. If
	// that changes, this would just become a loop.
	UInt32 diIndex = di->GetDrawableMeshIndex(0);
	hsAssert(diIndex >= 0, "Bogus input to fill particles");

	const plDISpanIndex& diSpans = drawable->GetDISpans(diIndex);
	int i;
	for( i = 0; i < diSpans.GetCount(); i++ )
	{
		UInt32 spanIdx = diSpans[i];
		hsAssert(drawable->GetSpan(spanIdx), "Bogus input to fill particles");
		hsAssert(drawable->GetSpan(spanIdx)->fTypeMask & plSpan::kParticleSpan, "Bogus input to fill particles");

		// Safe cast, since we just checked the type mask.
		plParticleSpan* span = (plParticleSpan*)drawable->GetSpan(spanIdx);

		if( !span->fSource )
			return; // Nothing to do, it's idle.

		FillParticles(pipe, drawable, span);
	}

}

