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
//	plDrawableGenerator Class Functions										//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	5.15.2001 mcn - Created.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plDrawableGenerator.h"
#include "plDrawableSpans.h"
#include "plGeometrySpan.h"
#include "hsFastMath.h"
#include "plRenderLevel.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"

// Making light white and dark black by default, because this is really
// redundant. The handling of what color unlit and fully lit map to is
// encapsulated in the material used to draw the mesh. The caller
// wants illumination values, and can handle on screen contrast 
// through the material. mf
hsColorRGBA			plDrawableGenerator::fLiteColor = { 1, 1, 1, 1 };
hsColorRGBA			plDrawableGenerator::fDarkColor = { 0.0, 0.0, 0.0, 1 };


//// SetFauxLightColors //////////////////////////////////////////////////////
//	Set the colors for the foux lighting on generated drawables

void	plDrawableGenerator::SetFauxLightColors( hsColorRGBA &lite, hsColorRGBA &dark )
{
	fLiteColor = lite;
	fDarkColor = dark;
}

//// IQuickShadeVerts ////////////////////////////////////////////////////////
//	Quickly shades vertices based on a fake directional light. Good for doing
//	faux shadings on proxy objects.

void	plDrawableGenerator::IQuickShadeVerts( UInt32 count, hsVector3 *normals, hsColorRGBA *colors, hsColorRGBA* origColors, const hsColorRGBA* multColor )
{
	hsVector3		lightDir;
	float			scale;


	lightDir.Set( 1, 1, 1 );
	lightDir.Normalize();

	while( count-- )
	{
		scale = ( normals[ count ] * lightDir );
		// pretend there are two opposing directional lights, but the
		// one pointing downish is a little stronger.
		const hsScalar kReverseLight = -0.8f;
		if( scale < 0 )
			scale = kReverseLight * scale;
		colors[ count ] = fLiteColor * scale + fDarkColor * ( 1.f - scale );
		if( origColors )
			colors[ count ] *= origColors[ count ];
		if( multColor )
			colors[ count ] *= *multColor;
	}
}

void plDrawableGenerator::IFillSpan( UInt32 vertCount, hsPoint3 *positions, hsVector3 *normals, 
															hsPoint3 *uvws, UInt32 uvwsPerVtx, 
															hsColorRGBA *origColors, hsBool fauxShade, const hsColorRGBA* multColor,
															UInt32 numIndices, UInt16 *indices, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
															plGeometrySpan* span )
{
	hsTArray<hsVector3>			myNormals;


	/// Calculate normals if we don't have them
	if( normals == nil )
	{
		int			i;
		hsVector3	normal, v1, v2;

		myNormals.SetCount( vertCount );
		for( i = 0; i < vertCount; i++ )
			myNormals[ i ].Set( 0, 0, 0 );

		for( i = 0; i < numIndices; i += 3 )
		{
			v1.Set( &positions[ indices[ i + 1 ] ], &positions[ indices[ i ] ] );
			v2.Set( &positions[ indices[ i + 2 ] ], &positions[ indices[ i ] ] );

			normal = v1 % v2;
			myNormals[ indices[ i ] ] += normal;
			myNormals[ indices[ i + 1 ] ] += normal;
			myNormals[ indices[ i + 2 ] ] += normal;
		}

		for( i = 0; i < vertCount; i++ )
			myNormals[ i ].Normalize();

		normals = myNormals.AcquireArray();
	}

	if( uvws == nil )
		uvwsPerVtx = 0;
	span->BeginCreate( material, localToWorld, plGeometrySpan::UVCountToFormat( (UInt8)uvwsPerVtx ) );

	if( !origColors && !fauxShade )
		span->AddVertexArray( vertCount, positions, normals, nil, uvws, uvwsPerVtx );
	else
	{
		hsTArray<hsColorRGBA> colArray;

		hsColorRGBA* colors;
		if( fauxShade )
		{
			colArray.SetCount(vertCount);
			IQuickShadeVerts( vertCount, normals, colArray.AcquireArray(), origColors, multColor );
			colors = colArray.AcquireArray();
		}
		else // just use the origColors
		{
			colors = origColors;
		}


		hsTArray<UInt32>	tempColors;
		int					i;
		UInt8				a, r, g, b;


		tempColors.SetCount( vertCount );
		for( i = 0; i < vertCount; i++ )
		{
			hsColorRGBA *color = &colors[ i ];
			a = (UInt8)( color->a >= 1 ? 255 : color->a <= 0 ? 0 : color->a * 255.0 );
			r = (UInt8)( color->r >= 1 ? 255 : color->r <= 0 ? 0 : color->r * 255.0 );
			g = (UInt8)( color->g >= 1 ? 255 : color->g <= 0 ? 0 : color->g * 255.0 );
			b = (UInt8)( color->b >= 1 ? 255 : color->b <= 0 ? 0 : color->b * 255.0 );
			
			tempColors[ i ] = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );
		}

		span->AddVertexArray( vertCount, positions, normals, tempColors.AcquireArray(), uvws, uvwsPerVtx );
	}

	span->AddIndexArray( numIndices, indices );
	span->EndCreate();


}

//// RegenerateDrawable ////////////////////////////////////////////////////////
//	Static function that refills an existing drawable based on the vertex/index
//	data given. That data had better match the data the drawable was first filled
//	with (i.e. vertex/index count

hsBool plDrawableGenerator::RegenerateDrawable( UInt32 vertCount, hsPoint3 *positions, hsVector3 *normals, 
															hsPoint3 *uvws, UInt32 uvwsPerVtx, 
															hsColorRGBA *origColors, hsBool fauxShade, const hsColorRGBA* multColor,
															UInt32 numIndices, UInt16 *indices, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
															UInt32 diIndex, plDrawableSpans *destDraw )
{
	plDISpanIndex spanList = destDraw->GetDISpans( diIndex );
	if( spanList.GetCount() != 1 )
	{
		hsAssert(false, "Don't know how to distribute this geometry over multiple spans");
		return false;
	}
	plGeometrySpan* span = destDraw->GetGeometrySpan(spanList[0]);

	if( (span->fNumVerts != vertCount)
		||(span->fNumIndices != numIndices) )
	{
		hsAssert(false, "Mismatched data coming in for a refill");
		return false;
	}
	IFillSpan( vertCount, positions, normals, 
								uvws, uvwsPerVtx, 
								origColors, fauxShade, multColor,
								numIndices, indices, 
								material, localToWorld, blended,
								span );

	destDraw->RefreshDISpans( diIndex );

	return true;
}


//// GenerateDrawable ////////////////////////////////////////////////////////
//	Static function that creates a new drawable based on the vertex/index
//	data given.

plDrawableSpans	*plDrawableGenerator::GenerateDrawable( UInt32 vertCount, hsPoint3 *positions, hsVector3 *normals, 
															hsPoint3 *uvws, UInt32 uvwsPerVtx, 
															hsColorRGBA *origColors, hsBool fauxShade, const hsColorRGBA* multColor,
															UInt32 numIndices, UInt16 *indices, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
															hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	plDrawableSpans				*newDraw;
	hsTArray<plGeometrySpan *>	spanArray;
	plGeometrySpan				*span;

	// Set up props on the new drawable
	if( toAddTo != nil )
		newDraw = toAddTo;
	else
	{
		newDraw = TRACKED_NEW plDrawableSpans;
//		newDraw->SetNativeProperty( plDrawable::kPropVolatile, true );
		if( blended )
		{
			newDraw->SetRenderLevel(plRenderLevel(plRenderLevel::kBlendRendMajorLevel, plRenderLevel::kDefRendMinorLevel));
			newDraw->SetNativeProperty( plDrawable::kPropSortSpans | plDrawable::kPropSortFaces, true );
		}

		static int nameIdx = 0;
		char buff[256];
		sprintf(buff, "%s_%d", "GenDrawable", nameIdx++);
		hsgResMgr::ResMgr()->NewKey( buff, newDraw, plLocation::kGlobalFixedLoc );
	}

	// Create a temp plGeometrySpan
	spanArray.SetCount( 1 );
	span = spanArray[ 0 ] = TRACKED_NEW plGeometrySpan;

	IFillSpan( vertCount, positions, normals, 
								uvws, uvwsPerVtx, 
								origColors, fauxShade, multColor,
								numIndices, indices, 
								material, localToWorld, blended,
								span );

	/// Now add the span to the new drawable, clear up the span's buffers and return!
	UInt32 trash = UInt32(-1);
	UInt32 idx = newDraw->AppendDISpans( spanArray, trash, false );
	if( retIndex != nil )
		retIndex->Append(idx);

	return newDraw;
}

//// GenerateSphericalDrawable ///////////////////////////////////////////////

plDrawableSpans		*plDrawableGenerator::GenerateSphericalDrawable( const hsPoint3& pos, hsScalar radius, hsGMaterial *material, 
																	const hsMatrix44 &localToWorld, hsBool blended,
																	const hsColorRGBA* multColor,
																	hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo, 
																	hsScalar qualityScalar )
{
	hsTArray<hsPoint3>		points;
	hsTArray<hsVector3>		normals;
	hsTArray<UInt16>		indices;
	hsTArray<hsColorRGBA>	colors;
	hsPoint3				point;
	hsVector3				normal;

	int					i, j, numDivisions, start;
	float				angle, z, x, y, internRad;
	plDrawableSpans		*drawable;


	numDivisions = (int)( radius * qualityScalar / 10.f );
	if( numDivisions < 5 )
		numDivisions = 5;
	else if( numDivisions > 30 )
		numDivisions = 30;


	/// Generate points
	for( i = 0; i <= numDivisions; i++ )
	{
		angle = (float)i * ( hsScalarPI ) / (float)numDivisions;
		hsFastMath::SinCosInRange( angle, internRad, z );
		internRad *= radius;
				
		for( j = 0; j < numDivisions; j++ )
		{
			angle = (float)j * ( 2 * hsScalarPI ) / (float)numDivisions;
			hsFastMath::SinCosInRange( angle, x, y );

			point.Set( pos.fX + x * internRad, pos.fY + y * internRad, pos.fZ + z * radius );
			normal.Set( x * internRad, y * internRad, z * radius );
			normal.Normalize();

			points.Append( point );
			normals.Append( normal );
		}
	}

	/// Generate indices
	for( i = 0, start = 0; i < numDivisions; i++, start += numDivisions )
	{
		for( j = 0; j < numDivisions - 1; j++ )
		{
			indices.Append( start + j );
			indices.Append( start + j + 1 );
			indices.Append( start + j + numDivisions + 1 );

			indices.Append( start + j );
			indices.Append( start + j + numDivisions + 1 );
			indices.Append( start + j + numDivisions );
		}

		indices.Append( start + j );
		indices.Append( start );
		indices.Append( start + numDivisions );

		indices.Append( start + j );
		indices.Append( start + numDivisions );
		indices.Append( start + j + numDivisions );
	}

	/// Create a drawable for it
	drawable = plDrawableGenerator::GenerateDrawable( points.GetCount(), points.AcquireArray(), normals.AcquireArray(),
														nil, 0,
														nil, true, multColor,
														indices.GetCount(), indices.AcquireArray(),
														material, localToWorld, blended, retIndex, toAddTo );

	return drawable;
}

//// GenerateBoxDrawable /////////////////////////////////////////////////////

plDrawableSpans		*plDrawableGenerator::GenerateBoxDrawable( hsScalar width, hsScalar height, hsScalar depth, 
																hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
																const hsColorRGBA* multColor,
																hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsVector3		xVec, yVec, zVec;
	hsPoint3		pt;


	xVec.Set( width, 0, 0 );
	yVec.Set( 0, height, 0 );
	zVec.Set( 0, 0, depth );

	pt.Set( -width / 2.f, -height / 2.f, -depth / 2.f );

	return GenerateBoxDrawable( pt, xVec, yVec, zVec, material, localToWorld, blended, multColor, retIndex, toAddTo );
}

//// GenerateBoxDrawable /////////////////////////////////////////////////////
//	Version that takes a corner and three vectors, for x, y and z edges.

#define CALC_NORMAL( nA, xVec, yVec, zVec ) { hsVector3 n = (xVec) + (yVec) + (zVec); n = -n; n.Normalize(); nA.Append( n ); }

plDrawableSpans		*plDrawableGenerator::GenerateBoxDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec, const hsVector3 &zVec, 
																hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
																const hsColorRGBA* multColor,
																hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsTArray<hsPoint3>		points;
	hsTArray<hsVector3>		normals;
	hsTArray<UInt16>		indices;
	hsTArray<hsColorRGBA>	colors;
	hsTArray<hsPoint3>		uvws;
	hsPoint3				point;

	plDrawableSpans		*drawable;
	float				mults[ 8 ][ 3 ] = { { -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
											{ -1, -1,  1 }, { 1, -1,  1 }, { 1, 1,  1 }, { -1, 1,  1 } };



	/// Generate points and normals
	points.Expand( 8 );
	normals.Expand( 8 );

	point = corner;					points.Append( point );
	point += xVec;					points.Append( point );
	point += yVec;					points.Append( point );
	point = corner + yVec;			points.Append( point );
	point = corner + zVec;			points.Append( point );
	point += xVec;					points.Append( point );
	point += yVec;					points.Append( point );
	point = corner + zVec + yVec;	points.Append( point );

	CALC_NORMAL( normals, xVec, yVec, zVec );
	CALC_NORMAL( normals, -xVec, yVec, zVec );
	CALC_NORMAL( normals, -xVec, -yVec, zVec );
	CALC_NORMAL( normals, xVec, -yVec, zVec );
	CALC_NORMAL( normals, xVec, yVec, -zVec );
	CALC_NORMAL( normals, -xVec, yVec, -zVec );
	CALC_NORMAL( normals, -xVec, -yVec, -zVec );
	CALC_NORMAL( normals, xVec, -yVec, -zVec );

	uvws.Expand( 8 );
	uvws.Append( hsPoint3( 0.f, 1.f, 0.f ) );
	uvws.Append( hsPoint3( 1.f, 1.f, 0.f ) );
	uvws.Append( hsPoint3( 1.f, 0.f, 0.f ) );
	uvws.Append( hsPoint3( 0.f, 0.f, 0.f ) );
	uvws.Append( hsPoint3( 1.f, 1.f, 0.f ) );
	uvws.Append( hsPoint3( 1.f, 0.f, 0.f ) );
	uvws.Append( hsPoint3( 0.f, 0.f, 0.f ) );
	uvws.Append( hsPoint3( 0.f, 1.f, 0.f ) );

	/// Generate indices
	indices.Expand( 36 );
	indices.Append( 0 ); indices.Append( 1 ); indices.Append( 2 );
	indices.Append( 0 ); indices.Append( 2 ); indices.Append( 3 );

	indices.Append( 1 ); indices.Append( 0 ); indices.Append( 4 );
	indices.Append( 1 ); indices.Append( 4 ); indices.Append( 5 );

	indices.Append( 2 ); indices.Append( 1 ); indices.Append( 5 );
	indices.Append( 2 ); indices.Append( 5 ); indices.Append( 6 );

	indices.Append( 3 ); indices.Append( 2 ); indices.Append( 6 );
	indices.Append( 3 ); indices.Append( 6 ); indices.Append( 7 );

	indices.Append( 0 ); indices.Append( 3 ); indices.Append( 7 );
	indices.Append( 0 ); indices.Append( 7 ); indices.Append( 4 );

	indices.Append( 7 ); indices.Append( 6 ); indices.Append( 5 );
	indices.Append( 7 ); indices.Append( 5 ); indices.Append( 4 );

	/// Create a drawable for it
	drawable = plDrawableGenerator::GenerateDrawable( points.GetCount(), points.AcquireArray(), normals.AcquireArray(),
														uvws.AcquireArray(), 1,
														nil, true, multColor,
														indices.GetCount(), indices.AcquireArray(),
														material, localToWorld, blended, retIndex, toAddTo );

	return drawable;
}


//// GenerateBoundsDrawable //////////////////////////////////////////////////

plDrawableSpans		*plDrawableGenerator::GenerateBoundsDrawable( hsBounds3Ext *bounds,
																	hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
																	const hsColorRGBA* multColor,
																	hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsTArray<hsPoint3>		points;
	hsTArray<hsVector3>		normals;
	hsTArray<UInt16>		indices;
	hsTArray<hsColorRGBA>	colors;
	hsPoint3				point;
	hsVector3				normal;

	int					i;
	plDrawableSpans		*drawable;
	float				mults[ 8 ][ 3 ] = { { -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
											{ -1, -1,  1 }, { 1, -1,  1 }, { 1, 1,  1 }, { -1, 1,  1 } };


	/// Generate points and normals
	points.Expand( 8 );
	normals.Expand( 8 );
	hsPoint3 min = bounds->GetMins();
	hsPoint3 max = bounds->GetMaxs();

	for( i = 0; i < 8; i++ )
	{
		points.Append( hsPoint3( mults[ i ][ 0 ] > 0 ? max.fX : min.fX,
								 mults[ i ][ 1 ] > 0 ? max.fY : min.fY,
								 mults[ i ][ 2 ] > 0 ? max.fZ : min.fZ ) );
		normals.Append( hsVector3( mults[ i ][ 0 ], mults[ i ][ 1 ], mults[ i ][ 2 ] ) );
	}

	/// Generate indices
	indices.Expand( 36 );
	indices.Append( 0 ); indices.Append( 1 ); indices.Append( 2 );
	indices.Append( 0 ); indices.Append( 2 ); indices.Append( 3 );

	indices.Append( 1 ); indices.Append( 0 ); indices.Append( 4 );
	indices.Append( 1 ); indices.Append( 4 ); indices.Append( 5 );

	indices.Append( 2 ); indices.Append( 1 ); indices.Append( 5 );
	indices.Append( 2 ); indices.Append( 5 ); indices.Append( 6 );

	indices.Append( 3 ); indices.Append( 2 ); indices.Append( 6 );
	indices.Append( 3 ); indices.Append( 6 ); indices.Append( 7 );

	indices.Append( 0 ); indices.Append( 3 ); indices.Append( 7 );
	indices.Append( 0 ); indices.Append( 7 ); indices.Append( 4 );

	indices.Append( 7 ); indices.Append( 6 ); indices.Append( 5 );
	indices.Append( 7 ); indices.Append( 5 ); indices.Append( 4 );

	/// Create a drawable for it
	drawable = plDrawableGenerator::GenerateDrawable( points.GetCount(), points.AcquireArray(), normals.AcquireArray(),
														nil, 0,
														nil, true, multColor,
														indices.GetCount(), indices.AcquireArray(),
														material, localToWorld, blended, retIndex, toAddTo );

	return drawable;
}

//// GenerateConicalDrawable /////////////////////////////////////////////////

plDrawableSpans		*plDrawableGenerator::GenerateConicalDrawable( hsScalar radius, hsScalar height, hsGMaterial *material, 
																	const hsMatrix44 &localToWorld, hsBool blended,
																	const hsColorRGBA* multColor,
																	hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsVector3	direction;


	direction.Set( 0, 0, height );

	return GenerateConicalDrawable( hsPoint3( 0, 0, 0 ), direction, radius, material, localToWorld, blended,
									multColor, retIndex, toAddTo );
}


//// GenerateConicalDrawable /////////////////////////////////////////////////

plDrawableSpans		*plDrawableGenerator::GenerateConicalDrawable( hsPoint3 &apex, hsVector3 &direction, hsScalar radius, hsGMaterial *material, 
																	const hsMatrix44 &localToWorld, hsBool blended,
																	const hsColorRGBA* multColor,
																	hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsTArray<hsPoint3>		points;
	hsTArray<hsVector3>		normals;
	hsTArray<UInt16>		indices;
	hsTArray<hsColorRGBA>	colors;
	hsPoint3				point;
	hsVector3				normal;

	int					i, numDivisions;
	float				angle, x, y;
	plDrawableSpans		*drawable;


	numDivisions = (int)( radius / 4.f );
	if( numDivisions < 6 )
		numDivisions = 6;
	else if( numDivisions > 20 )
		numDivisions = 20;


	/// First, we need a few more vectors--specifically, the x and y vectors for the cone's base
	hsPoint3	baseCenter = apex + direction;
	hsVector3	xVec, yVec;

	xVec.Set( 1, 0, 0 );
	yVec = xVec % direction;
	if( yVec.MagnitudeSquared() == 0 )
	{
		xVec.Set( 0, 1, 0 );
		yVec = xVec % direction;
		hsAssert( yVec.MagnitudeSquared() != 0, "Weird funkiness when doing this!!!" );
	}

	xVec = yVec % direction;
	xVec.Normalize();
	yVec.Normalize();

	/// Now generate points based on those
	points.Expand( numDivisions + 2 );
	normals.Expand( numDivisions + 2 );

	points.Append( apex );
	normals.Append( -direction );
	for( i = 0; i < numDivisions; i++ )
	{
		angle = (float)i * ( hsScalarPI * 2.f ) / (float)numDivisions;
		hsFastMath::SinCosInRange( angle, x, y );

		points.Append( baseCenter + ( xVec * x * radius ) + ( yVec * y * radius ) );
		normals.Append( ( xVec * x ) + ( yVec * y ) );
	}

	/// Generate indices
	indices.Expand( ( numDivisions + 1 + numDivisions - 2 ) * 3 );
	for( i = 0; i < numDivisions - 1; i++ )
	{
		indices.Append( 0 );
		indices.Append( i + 2 );
		indices.Append( i + 1 );
	}
	indices.Append( 0 );
	indices.Append( 1 );
	indices.Append( numDivisions );
	// Bottom cap
	for( i = 3; i < numDivisions + 1; i++ )
	{
		indices.Append( i - 1 );
		indices.Append( i );
		indices.Append( 1 );
	}

	/// Create a drawable for it
	drawable = plDrawableGenerator::GenerateDrawable( points.GetCount(), points.AcquireArray(), normals.AcquireArray(),
														nil, 0,
														nil, true, multColor,
														indices.GetCount(), indices.AcquireArray(),
														material, localToWorld, blended, retIndex, toAddTo );

	return drawable;
}

//// GenerateAxesDrawable ////////////////////////////////////////////////////

plDrawableSpans		*plDrawableGenerator::GenerateAxesDrawable( hsGMaterial *material, 
																const hsMatrix44 &localToWorld, hsBool blended,
																const hsColorRGBA* multColor,
																hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsTArray<hsPoint3>		points;
	hsTArray<hsVector3>		normals;
	hsTArray<hsColorRGBA>	colors;
	hsTArray<UInt16>		indices;

	int					i;
	float				size = 15;
	plDrawableSpans		*drawable;


	/// Generate points
	points.SetCount( 6 * 3 );
	normals.SetCount( 6 * 3 );
	colors.SetCount( 6 * 3 );

	points[ 0 ].Set( 0, 0, 0 );
	points[ 1 ].Set( size, -size * 0.1f, 0 );
	points[ 2 ].Set( size, -size * 0.3f, 0 );
	points[ 3 ].Set( size * 1.3f, 0, 0 );
	points[ 4 ].Set( size, size * 0.3f, 0 );
	points[ 5 ].Set( size, size * 0.1f, 0 );
	for( i = 0; i < 6; i++ )
	{
		points[ i + 6 ].fX = - points[ i ].fY;
		points[ i + 6 ].fY = points[ i ].fX;
		points[ i + 6 ].fZ = 0;

		points[ i + 12 ].fX = points[ i ].fY;
		points[ i + 12 ].fZ = points[ i ].fX;
		points[ i + 12 ].fY = 0;

		colors[ i ].Set( 1, 0, 0, 1 );
		colors[ i + 6 ].Set( 0, 1, 0, 1 );
		colors[ i + 12 ].Set( 0, 0, 1, 1 );

		if( multColor )
			colors[ i ] *= *multColor;
	}

	/// Generate indices
	indices.SetCount( 6 * 3 );
	for( i = 0; i < 18; i += 6 )
	{
		indices[ i ] = i + 0;
		indices[ i + 1 ] = i + 1;
		indices[ i + 2 ] = i + 5;
		indices[ i + 3 ] = i + 2;
		indices[ i + 4 ] = i + 3;
		indices[ i + 5 ] = i + 4;
	}

	/// Create a drawable for it
	drawable = plDrawableGenerator::GenerateDrawable( points.GetCount(), points.AcquireArray(), normals.AcquireArray(),
														nil, 0,
														nil, true, multColor,
														indices.GetCount(), indices.AcquireArray(),
														material, localToWorld, blended, retIndex, toAddTo );

	return drawable;
}

//// GeneratePlanarDrawable //////////////////////////////////////////////////
//	Version that takes a corner and two vectors, for x and y edges.

#define CALC_PNORMAL( nA, xVec, yVec ) { hsVector3 n = (xVec) % (yVec); n.Normalize(); nA.Append( n ); }

plDrawableSpans		*plDrawableGenerator::GeneratePlanarDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec,
																hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
																const hsColorRGBA* multColor,
																hsTArray<UInt32> *retIndex, plDrawableSpans *toAddTo )
{
	hsTArray<hsPoint3>		points;
	hsTArray<hsVector3>		normals;
	hsTArray<UInt16>		indices;
	hsTArray<hsColorRGBA>	colors;
	hsTArray<hsPoint3>		uvws;
	hsPoint3				point;

	plDrawableSpans		*drawable;


	/// Generate points and normals
	points.Expand( 4 );
	normals.Expand( 4 );

	point = corner;					points.Append( point );
	point += xVec;					points.Append( point );
	point += yVec;					points.Append( point );
	point = corner + yVec;			points.Append( point );

	CALC_PNORMAL( normals, xVec, yVec );
	CALC_PNORMAL( normals, xVec, yVec );
	CALC_PNORMAL( normals, xVec, yVec );
	CALC_PNORMAL( normals, xVec, yVec );

	uvws.Expand( 4 );
	uvws.Append( hsPoint3( 0.f, 1.f, 0.f ) );
	uvws.Append( hsPoint3( 1.f, 1.f, 0.f ) );
	uvws.Append( hsPoint3( 1.f, 0.f, 0.f ) );
	uvws.Append( hsPoint3( 0.f, 0.f, 0.f ) );

	/// Generate indices
	indices.Expand( 6 );
	indices.Append( 0 ); indices.Append( 1 ); indices.Append( 2 );
	indices.Append( 0 ); indices.Append( 2 ); indices.Append( 3 );

	/// Create a drawable for it
	drawable = plDrawableGenerator::GenerateDrawable( points.GetCount(), points.AcquireArray(), normals.AcquireArray(),
														uvws.AcquireArray(), 1,
														nil, true, multColor,
														indices.GetCount(), indices.AcquireArray(),
														material, localToWorld, blended, retIndex, toAddTo );

	return drawable;
}


