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
//  plDrawableGenerator Class Functions                                     //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  5.15.2001 mcn - Created.                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plDrawableGenerator.h"
#include "plDrawableSpans.h"
#include "plGeoSpanDice.h"
#include "plGeometrySpan.h"
#include "plGBufferGroup.h"
#include <string_theory/format>
#include "hsFastMath.h"
#include "plRenderLevel.h"
#include "hsResMgr.h"
#include "pnKeyedObject/plUoid.h"

// Making light white and dark black by default, because this is really
// redundant. The handling of what color unlit and fully lit map to is
// encapsulated in the material used to draw the mesh. The caller
// wants illumination values, and can handle on screen contrast 
// through the material. mf
hsColorRGBA         plDrawableGenerator::fLiteColor = { 1, 1, 1, 1 };
hsColorRGBA         plDrawableGenerator::fDarkColor = { 0.0, 0.0, 0.0, 1 };


//// SetFauxLightColors //////////////////////////////////////////////////////
//  Set the colors for the foux lighting on generated drawables

void    plDrawableGenerator::SetFauxLightColors( hsColorRGBA &lite, hsColorRGBA &dark )
{
    fLiteColor = lite;
    fDarkColor = dark;
}

//// IQuickShadeVerts ////////////////////////////////////////////////////////
//  Quickly shades vertices based on a fake directional light. Good for doing
//  faux shadings on proxy objects.

void    plDrawableGenerator::IQuickShadeVerts( uint32_t count, hsVector3 *normals, hsColorRGBA *colors, hsColorRGBA* origColors, const hsColorRGBA* multColor )
{
    hsVector3       lightDir;
    float           scale;


    lightDir.Set( 1, 1, 1 );
    lightDir.Normalize();

    while( count-- )
    {
        scale = ( normals[ count ] * lightDir );
        // pretend there are two opposing directional lights, but the
        // one pointing downish is a little stronger.
        const float kReverseLight = -0.8f;
        if( scale < 0 )
            scale = kReverseLight * scale;
        colors[ count ] = fLiteColor * scale + fDarkColor * ( 1.f - scale );
        if( origColors )
            colors[ count ] *= origColors[ count ];
        if( multColor )
            colors[ count ] *= *multColor;
    }
}

void plDrawableGenerator::IFillSpan( uint32_t vertCount, hsPoint3 *positions, hsVector3 *normals, 
                                                            hsPoint3 *uvws, uint32_t uvwsPerVtx, 
                                                            hsColorRGBA *origColors, bool fauxShade, const hsColorRGBA* multColor,
                                                            uint32_t numIndices, uint16_t *indices, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                            plGeometrySpan* span )
{
    std::vector<hsVector3>      myNormals;


    /// Calculate normals if we don't have them
    if (normals == nullptr)
    {
        int         i;
        hsVector3   normal, v1, v2;

        myNormals.resize(vertCount);
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

        normals = myNormals.data();
    }

    if (uvws == nullptr)
        uvwsPerVtx = 0;
    span->BeginCreate( material, localToWorld, plGeometrySpan::UVCountToFormat( (uint8_t)uvwsPerVtx ) );

    if( !origColors && !fauxShade )
        span->AddVertexArray(vertCount, positions, normals, nullptr, uvws, uvwsPerVtx);
    else
    {
        std::vector<hsColorRGBA> colArray;

        hsColorRGBA* colors;
        if( fauxShade )
        {
            colArray.resize(vertCount);
            IQuickShadeVerts(vertCount, normals, colArray.data(), origColors, multColor);
            colors = colArray.data();
        }
        else // just use the origColors
        {
            colors = origColors;
        }


        std::vector<uint32_t> tempColors;
        int                 i;
        uint8_t               a, r, g, b;


        tempColors.resize(vertCount);
        for( i = 0; i < vertCount; i++ )
        {
            hsColorRGBA *color = &colors[ i ];
            a = (uint8_t)( color->a >= 1 ? 255 : color->a <= 0 ? 0 : color->a * 255.0 );
            r = (uint8_t)( color->r >= 1 ? 255 : color->r <= 0 ? 0 : color->r * 255.0 );
            g = (uint8_t)( color->g >= 1 ? 255 : color->g <= 0 ? 0 : color->g * 255.0 );
            b = (uint8_t)( color->b >= 1 ? 255 : color->b <= 0 ? 0 : color->b * 255.0 );
            
            tempColors[ i ] = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );
        }

        span->AddVertexArray(vertCount, positions, normals, tempColors.data(), uvws, uvwsPerVtx);
    }

    span->AddIndexArray( numIndices, indices );
    span->EndCreate();


}

//// RegenerateDrawable ////////////////////////////////////////////////////////
//  Static function that refills an existing drawable based on the vertex/index
//  data given. That data had better match the data the drawable was first filled
//  with (i.e. vertex/index count

bool plDrawableGenerator::RegenerateDrawable( uint32_t vertCount, hsPoint3 *positions, hsVector3 *normals, 
                                                            hsPoint3 *uvws, uint32_t uvwsPerVtx, 
                                                            hsColorRGBA *origColors, bool fauxShade, const hsColorRGBA* multColor,
                                                            uint32_t numIndices, uint16_t *indices, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                            uint32_t diIndex, plDrawableSpans *destDraw )
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
//  Static function that creates a new drawable based on the vertex/index
//  data given.

plDrawableSpans *plDrawableGenerator::GenerateDrawable( uint32_t vertCount, hsPoint3 *positions, hsVector3 *normals, 
                                                            hsPoint3 *uvws, uint32_t uvwsPerVtx, 
                                                            hsColorRGBA *origColors, bool fauxShade, const hsColorRGBA* multColor,
                                                            uint32_t numIndices, uint16_t *indices, 
                                                            hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                            std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    plDrawableSpans             *newDraw;

    // Set up props on the new drawable
    if (toAddTo != nullptr)
        newDraw = toAddTo;
    else
    {
        newDraw = new plDrawableSpans;
//      newDraw->SetNativeProperty( plDrawable::kPropVolatile, true );
        if( blended )
        {
            newDraw->SetRenderLevel(plRenderLevel(plRenderLevel::kBlendRendMajorLevel, plRenderLevel::kDefRendMinorLevel));
            newDraw->SetNativeProperty( plDrawable::kPropSortSpans | plDrawable::kPropSortFaces, true );
        }

        static int nameIdx = 0;
        ST::string buff = ST::format("GenDrawable_{}", nameIdx++);
        hsgResMgr::ResMgr()->NewKey( buff, newDraw, plLocation::kGlobalFixedLoc );
    }

    // Create a temp plGeometrySpan
    std::vector<plGeometrySpan *> spanArray { new plGeometrySpan };
    plGeometrySpan* span = spanArray[0];

    IFillSpan( vertCount, positions, normals, 
                                uvws, uvwsPerVtx, 
                                origColors, fauxShade, multColor,
                                numIndices, indices, 
                                material, localToWorld, blended,
                                span );

    // Ensure that we don't overflow the maximum permissible verts/indices
    if (spanArray[0]->fNumIndices > plGBufferGroup::kMaxNumIndicesPerBuffer ||
        spanArray[0]->fNumVerts > plGBufferGroup::kMaxNumVertsPerBuffer) {
        plGeoSpanDice dice;
        dice.SetMaxFaces(plGBufferGroup::kMaxNumIndicesPerBuffer / 3);
        bool weDiced = dice.Dice(spanArray);
        hsAssert(weDiced, "Uh oh, a generated drawable was too large and couldn't be diced.");
        hsAssert(
            std::any_of(
                spanArray.cbegin(), spanArray.cend(),
                [](const plGeometrySpan* span) -> bool {
                    return (
                        span->fNumIndices < plGBufferGroup::kMaxNumIndicesPerBuffer ||
                        span->fNumVerts < plGBufferGroup::kMaxNumVertsPerBuffer
                    );
                }
            ),
            "Uh oh, a generated drawable is too large even after dicing."
        );
    }


    /// Now add the span to the new drawable, clear up the span's buffers and return!
    uint32_t trash = uint32_t(-1);
    uint32_t idx = newDraw->AppendDISpans( spanArray, trash, false );
    if (retIndex != nullptr)
        retIndex->emplace_back(idx);

    return newDraw;
}

//// GenerateSphericalDrawable ///////////////////////////////////////////////

plDrawableSpans     *plDrawableGenerator::GenerateSphericalDrawable( const hsPoint3& pos, float radius, hsGMaterial *material, 
                                                                    const hsMatrix44 &localToWorld, bool blended,
                                                                    const hsColorRGBA* multColor,
                                                                    std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo, 
                                                                    float qualityScalar )
{
    std::vector<hsPoint3>   points;
    std::vector<hsVector3>  normals;
    std::vector<uint16_t>   indices;
    std::vector<hsColorRGBA> colors;

    int                 i, j, numDivisions, start;
    float               angle, z, x, y, internRad;
    plDrawableSpans     *drawable;


    numDivisions = (int)( radius * qualityScalar / 10.f );
    if( numDivisions < 5 )
        numDivisions = 5;
    else if( numDivisions > 30 )
        numDivisions = 30;


    /// Generate points
    for( i = 0; i <= numDivisions; i++ )
    {
        angle = float(i) * hsConstants::pi<float> / float(numDivisions);
        hsFastMath::SinCosInRange( angle, internRad, z );
        internRad *= radius;
                
        for( j = 0; j < numDivisions; j++ )
        {
            angle = float(j) * hsConstants::two_pi<float> / float(numDivisions);
            hsFastMath::SinCosInRange( angle, x, y );

            hsPoint3 point(pos.fX + x * internRad, pos.fY + y * internRad, pos.fZ + z * radius);
            hsVector3 normal(x * internRad, y * internRad, z * radius);
            normal.Normalize();

            points.emplace_back(point);
            normals.emplace_back(normal);
        }
    }

    /// Generate indices
    for( i = 0, start = 0; i < numDivisions; i++, start += numDivisions )
    {
        for( j = 0; j < numDivisions - 1; j++ )
        {
            indices.emplace_back(start + j);
            indices.emplace_back(start + j + 1);
            indices.emplace_back(start + j + numDivisions + 1);

            indices.emplace_back(start + j);
            indices.emplace_back(start + j + numDivisions + 1);
            indices.emplace_back(start + j + numDivisions);
        }

        indices.emplace_back(start + j);
        indices.emplace_back(start);
        indices.emplace_back(start + numDivisions);

        indices.emplace_back(start + j);
        indices.emplace_back(start + numDivisions);
        indices.emplace_back(start + j + numDivisions);
    }

    /// Create a drawable for it
    drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(), normals.data(),
                                                        nullptr, 0,
                                                        nullptr, true, multColor,
                                                        indices.size(), indices.data(),
                                                        material, localToWorld, blended, retIndex, toAddTo );

    return drawable;
}

//// GenerateCapsuleDrawable //////////////////////////////////////////////////

plDrawableSpans      *plDrawableGenerator::GenerateCapsuleDrawable( const hsPoint3& localPos, float radius, float halfHeight,
                                                                    hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                                    const hsColorRGBA* multColor,
                                                                    std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo,
                                                                    float qualityScalar )
{
    std::vector<hsPoint3>    points;
    std::vector<uint16_t>    indices;

    // The following has been adapted from the FOSS library libcinder https://libcinder.org/
    const unsigned numSegments = std::clamp((unsigned)(radius * hsConstants::two_pi<float> * qualityScalar * .1f),
                                            5U, 30U);
    const unsigned subsHeight = std::max((unsigned)(30U * qualityScalar * 1.f), 5U);
    const unsigned ringsBody = subsHeight + 1;
    const unsigned ringsTotal = subsHeight + ringsBody;

    points.reserve(numSegments * ringsTotal);
    indices.reserve((numSegments - 1) * (ringsTotal - 1) * 6);

    const auto generateRing = [numSegments, radius, halfHeight, &localPos, &points]
                              (float r, float z, float dz) -> void {
        float segIncr = 1.f / (numSegments - 1);
        for (unsigned i = 0; i < numSegments; ++i) {
            float x = std::cos(hsConstants::two_pi<float> * i * segIncr) * r;
            float y = std::sin(hsConstants::two_pi<float> * i * segIncr) * r;
            points.emplace_back(localPos.fX + (radius * x),
                                localPos.fY + (radius * y),
                                localPos.fZ + (radius * z + halfHeight * dz));
        }
    };

    const float bodyIncr = 1.f / (ringsBody - 1U);
    const float ringIncr = 1.f / (subsHeight - 1U);

    for (unsigned r = 0; r < subsHeight / 2; ++r) {
        generateRing(std::sin(hsConstants::pi<float> * r * ringIncr),
                     std::sin(hsConstants::pi<float> * (r * ringIncr - 0.5f)),
                     -0.5f);
    }
    for (unsigned r = 0; r < ringsBody; ++r) {
        generateRing(1.f, 0.f, r * bodyIncr - 0.5f);
    }
    for (unsigned r = subsHeight / 2; r < subsHeight; ++r) {
        generateRing(std::sin(hsConstants::pi<float> * r * ringIncr),
                     std::sin(hsConstants::pi<float> * (r * ringIncr - 0.5f)),
                     +0.5f);
    }

    for (unsigned r = 0; r < ringsTotal - 1; ++r) {
        for (unsigned s = 0; s < numSegments - 1; ++s) {
            indices.push_back(((r    ) * numSegments + (s + 1)));
            indices.push_back(((r    ) * numSegments + (s + 0)));
            indices.push_back(((r + 1) * numSegments + (s + 1)));

            indices.push_back(((r + 1) * numSegments + (s + 0)));
            indices.push_back(((r + 1) * numSegments + (s + 1)));
            indices.push_back(((r    ) * numSegments + (s    )));
        }
    }

    /// Create a drawable for it
    auto* drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(),
                                                           nullptr, nullptr, 0,
                                                           nullptr, true, multColor,
                                                           indices.size(), indices.data(),
                                                           material, localToWorld, blended, retIndex, toAddTo);
    return drawable;
}

//// GenerateBoxDrawable /////////////////////////////////////////////////////

plDrawableSpans     *plDrawableGenerator::GenerateBoxDrawable( float width, float height, float depth, 
                                                                hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                                const hsColorRGBA* multColor,
                                                                std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    hsVector3 xVec(width, 0.f, 0.f);
    hsVector3 yVec(0.f, height, 0.f);
    hsVector3 zVec(0.f, 0.f, depth);
    hsPoint3 pt(-width / 2.f, -height / 2.f, -depth / 2.f);

    return GenerateBoxDrawable( pt, xVec, yVec, zVec, material, localToWorld, blended, multColor, retIndex, toAddTo );
}

//// GenerateBoxDrawable /////////////////////////////////////////////////////
//  Version that takes a corner and three vectors, for x, y and z edges.

#define CALC_NORMAL(nA, xVec, yVec, zVec) { hsVector3 n = (xVec) + (yVec) + (zVec); n = -n; n.Normalize(); nA.emplace_back(n); }

plDrawableSpans     *plDrawableGenerator::GenerateBoxDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec, const hsVector3 &zVec, 
                                                                hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                                const hsColorRGBA* multColor,
                                                                std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    std::vector<hsPoint3>   points;
    std::vector<hsVector3>  normals;
    std::vector<uint16_t>   indices;
    std::vector<hsColorRGBA> colors;
    std::vector<hsPoint3>   uvws;
    hsPoint3                point;

    plDrawableSpans     *drawable;


    /// Generate points and normals
    points.reserve(8);
    normals.reserve(8);

    point = corner;                 points.emplace_back(point);
    point += xVec;                  points.emplace_back(point);
    point += yVec;                  points.emplace_back(point);
    point = corner + yVec;          points.emplace_back(point);
    point = corner + zVec;          points.emplace_back(point);
    point += xVec;                  points.emplace_back(point);
    point += yVec;                  points.emplace_back(point);
    point = corner + zVec + yVec;   points.emplace_back(point);

    CALC_NORMAL( normals, xVec, yVec, zVec );
    CALC_NORMAL( normals, -xVec, yVec, zVec );
    CALC_NORMAL( normals, -xVec, -yVec, zVec );
    CALC_NORMAL( normals, xVec, -yVec, zVec );
    CALC_NORMAL( normals, xVec, yVec, -zVec );
    CALC_NORMAL( normals, -xVec, yVec, -zVec );
    CALC_NORMAL( normals, -xVec, -yVec, -zVec );
    CALC_NORMAL( normals, xVec, -yVec, -zVec );

    uvws.reserve(8);
    uvws.emplace_back(0.f, 1.f, 0.f);
    uvws.emplace_back(1.f, 1.f, 0.f);
    uvws.emplace_back(1.f, 0.f, 0.f);
    uvws.emplace_back(0.f, 0.f, 0.f);
    uvws.emplace_back(1.f, 1.f, 0.f);
    uvws.emplace_back(1.f, 0.f, 0.f);
    uvws.emplace_back(0.f, 0.f, 0.f);
    uvws.emplace_back(0.f, 1.f, 0.f);

    /// Generate indices
    indices.reserve(36);
    indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
    indices.emplace_back(0); indices.emplace_back(2); indices.emplace_back(3);

    indices.emplace_back(1); indices.emplace_back(0); indices.emplace_back(4);
    indices.emplace_back(1); indices.emplace_back(4); indices.emplace_back(5);

    indices.emplace_back(2); indices.emplace_back(1); indices.emplace_back(5);
    indices.emplace_back(2); indices.emplace_back(5); indices.emplace_back(6);

    indices.emplace_back(3); indices.emplace_back(2); indices.emplace_back(6);
    indices.emplace_back(3); indices.emplace_back(6); indices.emplace_back(7);

    indices.emplace_back(0); indices.emplace_back(3); indices.emplace_back(7);
    indices.emplace_back(0); indices.emplace_back(7); indices.emplace_back(4);

    indices.emplace_back(7); indices.emplace_back(6); indices.emplace_back(5);
    indices.emplace_back(7); indices.emplace_back(5); indices.emplace_back(4);

    /// Create a drawable for it
    drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(), normals.data(),
                                                        uvws.data(), 1,
                                                        nullptr, true, multColor,
                                                        indices.size(), indices.data(),
                                                        material, localToWorld, blended, retIndex, toAddTo );

    return drawable;
}


//// GenerateBoundsDrawable //////////////////////////////////////////////////

plDrawableSpans     *plDrawableGenerator::GenerateBoundsDrawable( hsBounds3Ext *bounds,
                                                                    hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                                    const hsColorRGBA* multColor,
                                                                    std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    std::vector<hsPoint3>   points;
    std::vector<hsVector3>  normals;
    std::vector<uint16_t>   indices;
    std::vector<hsColorRGBA> colors;

    int                 i;
    plDrawableSpans     *drawable;
    float               mults[ 8 ][ 3 ] = { { -1, -1, -1 }, { 1, -1, -1 }, { 1, 1, -1 }, { -1, 1, -1 },
                                            { -1, -1,  1 }, { 1, -1,  1 }, { 1, 1,  1 }, { -1, 1,  1 } };


    /// Generate points and normals
    points.reserve(8);
    normals.reserve(8);
    hsPoint3 min = bounds->GetMins();
    hsPoint3 max = bounds->GetMaxs();

    for( i = 0; i < 8; i++ )
    {
        points.emplace_back(mults[i][0] > 0 ? max.fX : min.fX,
                            mults[i][1] > 0 ? max.fY : min.fY,
                            mults[i][2] > 0 ? max.fZ : min.fZ);
        normals.emplace_back(mults[i][0], mults[i][1], mults[i][2]);
    }

    /// Generate indices
    indices.reserve(36);
    indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
    indices.emplace_back(0); indices.emplace_back(2); indices.emplace_back(3);

    indices.emplace_back(1); indices.emplace_back(0); indices.emplace_back(4);
    indices.emplace_back(1); indices.emplace_back(4); indices.emplace_back(5);

    indices.emplace_back(2); indices.emplace_back(1); indices.emplace_back(5);
    indices.emplace_back(2); indices.emplace_back(5); indices.emplace_back(6);

    indices.emplace_back(3); indices.emplace_back(2); indices.emplace_back(6);
    indices.emplace_back(3); indices.emplace_back(6); indices.emplace_back(7);

    indices.emplace_back(0); indices.emplace_back(3); indices.emplace_back(7);
    indices.emplace_back(0); indices.emplace_back(7); indices.emplace_back(4);

    indices.emplace_back(7); indices.emplace_back(6); indices.emplace_back(5);
    indices.emplace_back(7); indices.emplace_back(5); indices.emplace_back(4);

    /// Create a drawable for it
    drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(), normals.data(),
                                                        nullptr, 0,
                                                        nullptr, true, multColor,
                                                        indices.size(), indices.data(),
                                                        material, localToWorld, blended, retIndex, toAddTo );

    return drawable;
}

//// GenerateConicalDrawable /////////////////////////////////////////////////

plDrawableSpans     *plDrawableGenerator::GenerateConicalDrawable( float radius, float height, hsGMaterial *material, 
                                                                    const hsMatrix44 &localToWorld, bool blended,
                                                                    const hsColorRGBA* multColor,
                                                                    std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    hsVector3 direction(0.f, 0.f, height);
    hsPoint3 zero;
    return GenerateConicalDrawable(zero, direction, radius, material, localToWorld, blended,
                                   multColor, retIndex, toAddTo);
}


//// GenerateConicalDrawable /////////////////////////////////////////////////

plDrawableSpans     *plDrawableGenerator::GenerateConicalDrawable( hsPoint3 &apex, hsVector3 &direction, float radius, hsGMaterial *material, 
                                                                    const hsMatrix44 &localToWorld, bool blended,
                                                                    const hsColorRGBA* multColor,
                                                                    std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    std::vector<hsPoint3>   points;
    std::vector<hsVector3>  normals;
    std::vector<uint16_t>   indices;
    std::vector<hsColorRGBA> colors;

    int                 i, numDivisions;
    float               angle, x, y;
    plDrawableSpans     *drawable;


    numDivisions = (int)( radius / 4.f );
    if( numDivisions < 6 )
        numDivisions = 6;
    else if( numDivisions > 20 )
        numDivisions = 20;


    /// First, we need a few more vectors--specifically, the x and y vectors for the cone's base
    hsPoint3    baseCenter = apex + direction;
    hsVector3   xVec, yVec;

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
    points.reserve(numDivisions + 2);
    normals.reserve(numDivisions + 2);

    points.emplace_back(apex);
    normals.emplace_back(-direction);
    for( i = 0; i < numDivisions; i++ )
    {
        angle = float(i) * hsConstants::two_pi<float> / float(numDivisions);
        hsFastMath::SinCosInRange( angle, x, y );

        points.emplace_back(baseCenter + (xVec * x * radius) + (yVec * y * radius));
        normals.emplace_back((xVec * x) + (yVec * y));
    }

    /// Generate indices
    indices.reserve((numDivisions + 1 + numDivisions - 2) * 3);
    for( i = 0; i < numDivisions - 1; i++ )
    {
        indices.emplace_back(0);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 1);
    }
    indices.emplace_back(0);
    indices.emplace_back(1);
    indices.emplace_back(numDivisions);
    // Bottom cap
    for( i = 3; i < numDivisions + 1; i++ )
    {
        indices.emplace_back(i - 1);
        indices.emplace_back(i);
        indices.emplace_back(1);
    }

    /// Create a drawable for it
    drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(), normals.data(),
                                                        nullptr, 0,
                                                        nullptr, true, multColor,
                                                        indices.size(), indices.data(),
                                                        material, localToWorld, blended, retIndex, toAddTo );

    return drawable;
}

//// GenerateAxesDrawable ////////////////////////////////////////////////////

plDrawableSpans     *plDrawableGenerator::GenerateAxesDrawable( hsGMaterial *material, 
                                                                const hsMatrix44 &localToWorld, bool blended,
                                                                const hsColorRGBA* multColor,
                                                                std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    std::vector<hsPoint3>   points(6 * 3);
    std::vector<hsVector3>  normals(6 * 3);
    std::vector<hsColorRGBA> colors(6 * 3);
    std::vector<uint16_t>   indices(6 * 3);

    int                 i;
    float               size = 15;
    plDrawableSpans     *drawable;


    /// Generate points
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
    drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(), normals.data(),
                                                        nullptr, 0,
                                                        nullptr, true, multColor,
                                                        indices.size(), indices.data(),
                                                        material, localToWorld, blended, retIndex, toAddTo );

    return drawable;
}

//// GeneratePlanarDrawable //////////////////////////////////////////////////
//  Version that takes a corner and two vectors, for x and y edges.

#define CALC_PNORMAL(nA, xVec, yVec) { hsVector3 n = (xVec) % (yVec); n.Normalize(); nA.emplace_back(n); }

plDrawableSpans     *plDrawableGenerator::GeneratePlanarDrawable( const hsPoint3 &corner, const hsVector3 &xVec, const hsVector3 &yVec,
                                                                hsGMaterial *material, const hsMatrix44 &localToWorld, bool blended,
                                                                const hsColorRGBA* multColor,
                                                                std::vector<uint32_t> *retIndex, plDrawableSpans *toAddTo )
{
    std::vector<hsPoint3>   points;
    std::vector<hsVector3>  normals;
    std::vector<uint16_t>   indices;
    std::vector<hsColorRGBA> colors;
    std::vector<hsPoint3>   uvws;

    plDrawableSpans     *drawable;


    /// Generate points and normals
    points.reserve(4);
    normals.reserve(4);

    points.emplace_back(corner);
    points.emplace_back(corner + xVec);
    points.emplace_back(corner + xVec + yVec);
    points.emplace_back(corner + yVec);

    CALC_PNORMAL( normals, xVec, yVec );
    CALC_PNORMAL( normals, xVec, yVec );
    CALC_PNORMAL( normals, xVec, yVec );
    CALC_PNORMAL( normals, xVec, yVec );

    uvws.reserve(4);
    uvws.emplace_back(0.f, 1.f, 0.f);
    uvws.emplace_back(1.f, 1.f, 0.f);
    uvws.emplace_back(1.f, 0.f, 0.f);
    uvws.emplace_back(0.f, 0.f, 0.f);

    /// Generate indices
    indices.reserve(6);
    indices.emplace_back(0); indices.emplace_back(1); indices.emplace_back(2);
    indices.emplace_back(0); indices.emplace_back(2); indices.emplace_back(3);

    /// Create a drawable for it
    drawable = plDrawableGenerator::GenerateDrawable(points.size(), points.data(), normals.data(),
                                                        uvws.data(), 1,
                                                        nullptr, true, multColor,
                                                        indices.size(), indices.data(),
                                                        material, localToWorld, blended, retIndex, toAddTo );

    return drawable;
}


