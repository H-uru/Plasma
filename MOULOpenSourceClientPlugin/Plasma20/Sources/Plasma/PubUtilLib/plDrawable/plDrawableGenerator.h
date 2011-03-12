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
//	plDrawableGenerator Header												//
//																			//
//	Static helper class for creating various kind of drawable primitives.	//
//	Can be very useful for visualization stuff ;)							//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	5.15.2001 mcn - Created.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDrawableGenerator_h
#define _plDrawableGenerator_h

#include "hsTemplates.h"
#include "hsBounds.h"
#include "hsMatrix44.h"

class hsGMaterial;
class plDrawableSpans;
class plGeometrySpan;
struct hsColorRGBA;


//// Class Definition ////////////////////////////////////////////////////////

class plDrawableGenerator
{
	public:

		// Set the colors for the faux lighting on generated drawables
		static void					SetFauxLightColors( hsColorRGBA &lite, hsColorRGBA &dark );

		// Refills a drawable previously created with GenerateDrawable with the new data. New data
		// must match previous data in counts.
		hsBool						RegenerateDrawable( UInt32 vertCount, hsPoint3 *positions, hsVector3 *normals, 
															hsPoint3 *uvws, UInt32 uvwsPerVtx, 
															hsColorRGBA *origColors, hsBool fauxShade, const hsColorRGBA* multColor,
															UInt32 numIndices, UInt16 *indices, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
															UInt32 diIndex, plDrawableSpans *destDraw );

		// Generates a drawable based on the vertex/index data given
		// uvws is an array vertCount*uvwsPerVtx long in order [uvw(s) for vtx0, uvw(s) for vtx1, ...], or is nil
		static plDrawableSpans		*GenerateDrawable( UInt32 vertCount, hsPoint3 *positions, hsVector3 *normals, 
														hsPoint3 *uvws, UInt32 uvwsPerVtx, 
														hsColorRGBA *origColors, hsBool fauxShade, const hsColorRGBA* multColor,
														UInt32 numIndices, UInt16 *indices, 
														hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended = false,
														hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

		// Generates a spherical drawable
		static plDrawableSpans		*GenerateSphericalDrawable( const hsPoint3& localPos, hsScalar radius, hsGMaterial *material, 
																const hsMatrix44 &localToWorld, hsBool blended = false,
																const hsColorRGBA* multColor = nil,
																hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil,
																hsScalar qualityScalar = 1.f );

		// Generates a rectangular drawable
		static plDrawableSpans		*GenerateBoxDrawable( hsScalar width, hsScalar height, hsScalar depth, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

		// Generate a rectangular drawable based on a corner and three vectors
		static plDrawableSpans		*GenerateBoxDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec, const hsVector3 &zVec, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );
		// Generates a bounds-based drawable
		static plDrawableSpans		*GenerateBoundsDrawable( hsBounds3Ext *bounds,
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

		// Generates a conical drawable
		static plDrawableSpans		*GenerateConicalDrawable( hsScalar radius, hsScalar height, hsGMaterial *material, 
															const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

		// Generates a general conical drawable based on a center and direction
		static plDrawableSpans		*GenerateConicalDrawable( hsPoint3 &apex, hsVector3 &direction, hsScalar radius, hsGMaterial *material, 
															const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

		// Generates a drawable representing 3 axes
		static plDrawableSpans		*GenerateAxesDrawable( hsGMaterial *material, 
															const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

		// Generate a planar drawable based on a corner and two vectors
		static plDrawableSpans		*GeneratePlanarDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec, 
															hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended = false,
															const hsColorRGBA* multColor = nil,
															hsTArray<UInt32> *retIndex = nil, plDrawableSpans *toAddTo = nil );

	protected:
		
		// Shade the vertices given based on a quick fake directional light.
		// If origColors is non-nil, it must be an array of length count. Each outColor[i] *= origColor[i].
		// If multColor is non-nil, modulate the output by multColor.
		static void					IQuickShadeVerts( UInt32 count, hsVector3 *normals, 
											hsColorRGBA *colors, 
											hsColorRGBA* origColors = nil, 
											const hsColorRGBA* multColor = nil );

		// Take the vertex and connectivity info supplied and fill out a geometry span with it.
		// Output span is ready to be added to a Drawable, or refreshed in a Drawable if it's
		// already in the SourceSpans.
		static void					IFillSpan( UInt32 vertCount, hsPoint3 *positions, hsVector3 *normals, 
																	hsPoint3 *uvws, UInt32 uvwsPerVtx, 
																	hsColorRGBA *origColors, hsBool fauxShade, const hsColorRGBA* multColor,
																	UInt32 numIndices, UInt16 *indices, 
																	hsGMaterial *material, const hsMatrix44 &localToWorld, hsBool blended,
																	plGeometrySpan* span );

		static hsColorRGBA			fLiteColor;
		static hsColorRGBA			fDarkColor;

};

#endif // _plDrawableGenerator_h
