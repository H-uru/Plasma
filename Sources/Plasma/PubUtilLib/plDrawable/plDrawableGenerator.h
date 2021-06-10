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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plDrawableGenerator Header                                              //
//                                                                          //
//  Static helper class for creating various kind of drawable primitives.   //
//  Can be very useful for visualization stuff ;)                           //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  5.15.2001 mcn - Created.                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDrawableGenerator_h
#define _plDrawableGenerator_h

#include "HeadSpin.h"
#include <vector>

class hsBounds3Ext;
struct hsColorRGBA;
class hsGMaterial;
class plDrawableSpans;
class plGeometrySpan;
struct hsMatrix44;
struct hsPoint3;
struct hsVector3;

//// Class Definition ////////////////////////////////////////////////////////

class plDrawableGenerator
{
    public:

        // Set the colors for the faux lighting on generated drawables
        static void                 SetFauxLightColors( hsColorRGBA &lite, hsColorRGBA &dark );

        // Refills a drawable previously created with GenerateDrawable with the new data. New data
        // must match previous data in counts.
        bool                        RegenerateDrawable( uint32_t vertCount, hsPoint3 *positions, hsVector3 *normals, 
                                                            hsPoint3 *uvws, uint32_t uvwsPerVtx, 
                                                            hsColorRGBA *origColors, bool fauxShade, const hsColorRGBA* multColor,
                                                            uint32_t numIndices, uint16_t *indices, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                            uint32_t diIndex, plDrawableSpans *destDraw );

        // Generates a drawable based on the vertex/index data given
        // uvws is an array vertCount*uvwsPerVtx long in order [uvw(s) for vtx0, uvw(s) for vtx1, ...], or is nullptr
        static plDrawableSpans      *GenerateDrawable( uint32_t vertCount, hsPoint3 *positions, hsVector3 *normals, 
                                                        hsPoint3 *uvws, uint32_t uvwsPerVtx, 
                                                        hsColorRGBA *origColors, bool fauxShade, const hsColorRGBA* multColor,
                                                        uint32_t numIndices, uint16_t *indices, 
                                                        hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended = false,
                                                        std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

        // Generates a spherical drawable
        static plDrawableSpans      *GenerateSphericalDrawable( const hsPoint3& localPos, float radius, hsGMaterial *material, 
                                                                const hsMatrix44 &localToWorld, bool blended = false,
                                                                const hsColorRGBA* multColor = nullptr,
                                                                std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr,
                                                                float qualityScalar = 1.f );

        // Generates a capsule drawable
        static plDrawableSpans      *GenerateCapsuleDrawable( const hsPoint3& localPos, float radius, float halfHeight,
                                                              hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended = false,
                                                              const hsColorRGBA* multColor = nullptr,
                                                              std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr,
                                                              float qualityScalar = 1.f );

        // Generates a rectangular drawable
        static plDrawableSpans      *GenerateBoxDrawable( float width, float height, float depth, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

        // Generate a rectangular drawable based on a corner and three vectors
        static plDrawableSpans      *GenerateBoxDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec, const hsVector3 &zVec, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);
        // Generates a bounds-based drawable
        static plDrawableSpans      *GenerateBoundsDrawable( hsBounds3Ext *bounds,
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

        // Generates a conical drawable
        static plDrawableSpans      *GenerateConicalDrawable( float radius, float height, hsGMaterial *material, 
                                                            const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

        // Generates a general conical drawable based on a center and direction
        static plDrawableSpans      *GenerateConicalDrawable( hsPoint3 &apex, hsVector3 &direction, float radius, hsGMaterial *material, 
                                                            const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

        // Generates a drawable representing 3 axes
        static plDrawableSpans      *GenerateAxesDrawable( hsGMaterial *material, 
                                                            const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

        // Generate a planar drawable based on a corner and two vectors
        static plDrawableSpans      *GeneratePlanarDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended = false,
                                                            const hsColorRGBA* multColor = nullptr,
                                                            std::vector<uint32_t> *retIndex = nullptr, plDrawableSpans *toAddTo = nullptr);

    protected:
        
        // Shade the vertices given based on a quick fake directional light.
        // If origColors is non-nil, it must be an array of length count. Each outColor[i] *= origColor[i].
        // If multColor is non-nil, modulate the output by multColor.
        static void                 IQuickShadeVerts( uint32_t count, hsVector3 *normals, 
                                            hsColorRGBA *colors, 
                                            hsColorRGBA* origColors = nullptr,
                                            const hsColorRGBA* multColor = nullptr);

        // Take the vertex and connectivity info supplied and fill out a geometry span with it.
        // Output span is ready to be added to a Drawable, or refreshed in a Drawable if it's
        // already in the SourceSpans.
        static void                 IFillSpan( uint32_t vertCount, hsPoint3 *positions, hsVector3 *normals, 
                                                                    hsPoint3 *uvws, uint32_t uvwsPerVtx, 
                                                                    hsColorRGBA *origColors, bool fauxShade, const hsColorRGBA* multColor,
                                                                    uint32_t numIndices, uint16_t *indices, 
                                                                    hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                                    plGeometrySpan* span );

        static hsColorRGBA          fLiteColor;
        static hsColorRGBA          fDarkColor;

};

#endif // _plDrawableGenerator_h
