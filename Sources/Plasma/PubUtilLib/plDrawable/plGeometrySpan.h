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
//  plGeometrySpan Header                                                   //
//                                                                          //
//  plGeometrySpans are abstract reprentations of Plasma 2.0 geometry data. //
//  They consist of a material, a transform, bounds and an abstract vertex  //
//  and index buffer pair. plGeometrySpans is what is fed to plDrawableIce  //
//  to convert into its own internal data structures; the format for the    //
//  vertex and index data is (or should be) identical. More or less, they   //
//  are identical to Ice's plIcicle, but this is more abstract (read: not   //
//  internal to Ice).                                                       //
//                                                                          //
//  Also included is a temporary hacked triMesh-to-geometrySpan[] converter //
//  for everyone's convenience until triMeshes disappear.                   //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  Created 3.8.2001 mcn                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plGeometrySpan_h
#define _plGeometrySpan_h

#include <vector>

#include "hsBounds.h"
#include "hsMatrix44.h"
#include "hsColorRGBA.h"
#include "hsBitVector.h"

class hsGMaterial;
class plFogEnvironment;


//// plGeometrySpan Class Definition /////////////////////////////////////////

class plGeometrySpan
{
    public:
        enum 
        {
            kMaxNumUVChannels   = 8
        };

        /// Duplication of the formats from plGBufferGroup; theoretically, they
        /// could be different, but they're identical for now
        enum Formats
        {
            kUVCountMask    = 0x0f, // Problem is, we need enough bits to store the max #, which means
                                    // we really want ( max # << 1 ) - 1

            kSkinNoWeights  = 0x00, // 0000000
            kSkin1Weight    = 0x10, // 0010000
            kSkin2Weights   = 0x20, // 0100000
            kSkin3Weights   = 0x30, // 0110000
            kSkinWeightMask = 0x30, // 0110000

            kSkinIndices    = 0x40, // 1000000
        };

        enum Properties
        {
            kPropRunTimeLight       = 0x01,
            kPropNoPreShade         = 0x02,
            kLiteMaterial           = 0x00,
            kLiteVtxPreshaded       = 0x04,
            kLiteVtxNonPreshaded    = 0x08,
            kLiteMask               = 0x0c,
            kRequiresBlending       = 0x10,
            kInstanced              = 0x20,
            kUserOwned              = 0x40,
            kPropNoShadow           = 0x80,
            kPropForceShadow        = 0x100,
            kDiffuseFoldedIn        = 0x200,        // Sometimes we want to fold the diffuse color of the material into the vertex color (but only once).
            kPropReverseSort        = 0x400,
            kWaterHeight            = 0x800,
            kFirstInstance          = 0x1000,
            kPartialSort            = 0x2000,
            kVisLOS                 = 0x4000,
            kPropNoShadowCast       = 0x8000
        };

        enum
        {
            kNoGroupID = 0
        };

        // Note: these are public because this is really just a glorified
        // struct; no data hiding here
        hsGMaterial         *fMaterial;
        hsMatrix44          fLocalToWorld;
        hsMatrix44          fWorldToLocal;
        hsBounds3Ext        fLocalBounds;
        hsBounds3Ext        fWorldBounds;
        plFogEnvironment    *fFogEnviron;

        uint32_t        fBaseMatrix;
        uint8_t         fNumMatrices;
        uint16_t        fLocalUVWChans;
        uint16_t        fMaxBoneIdx;
        uint32_t        fPenBoneIdx;

        float           fMinDist;
        float           fMaxDist;

        float           fWaterHeight;

        uint8_t         fFormat;
        uint32_t        fProps;
        uint32_t        fNumVerts, fNumIndices;

        /// Current vertex format:
        ///     float   position[ 3 ];
        ///     float   normal[ 3 ];
        ///     float   uvCoords[ ][ 3 ];
        ///     float   weights[];              // 0-3 blending weights
        ///     uint32_t  weightIndices;          // Only if there are >= 1 blending weights

        uint8_t*        fVertexData;
        uint16_t*       fIndexData;
        uint32_t        fDecalLevel;

        hsColorRGBA*    fMultColor;
        hsColorRGBA*    fAddColor;

        uint32_t*       fDiffuseRGBA;
        uint32_t*       fSpecularRGBA;

        mutable std::vector<plGeometrySpan *>* fInstanceRefs;
        mutable uint32_t                    fInstanceGroupID;       // For writing out/reading in instance refs

        // The following is only used for logging during export. It is never set
        // at runtime. Don't even think about using it for anything.
        ST::string                          fMaxOwner;

        // The following is ONLY used during pack; it's so we can do a reverse lookup
        // from the instanceRefs list to the correct span in the drawable
        uint32_t      fSpanRefIndex;

        // These two matrices are inverses of each other (duh). They are only used on computing the local
        // bounds. fLocalBounds is always the bounds in the space defined by fWorldToLocal, but the bounds
        // are an OBB, and the orientation of the OBB isn't necessarily the same as fLocalToWorld's axes.
        // For now, it is the orientation of the pivot point in max (but might be further optimized).
        hsMatrix44      fLocalToOBB;
        hsMatrix44      fOBBToLocal;

        plGeometrySpan();
        plGeometrySpan( const plGeometrySpan *instance );
        ~plGeometrySpan();

        /// UV stuff
        uint8_t   GetNumUVs() const { return ( fFormat & kUVCountMask ); }
        void    SetNumUVs( uint8_t numUVs ) 
        {
            hsAssert( numUVs < kMaxNumUVChannels, "Invalid UV count to plGeometrySpan" );
            fFormat = ( fFormat & ~kUVCountMask ) | numUVs; 
        }

        static uint8_t CalcNumUVs( uint8_t format ) { return ( format & kUVCountMask ); }
        static uint8_t UVCountToFormat( uint8_t numUVs ) { return numUVs & kUVCountMask; }
 
        /// Creation functions
        void    BeginCreate( hsGMaterial *material, const hsMatrix44 &l2wMatrix, uint8_t format );

        // Phasing these in...
        // Note: uvArray should be a fixed array with enough pointers for the max # of uv channels.
        // Any unused UVs should be nil
        uint16_t  AddVertex( hsPoint3 *position, hsPoint3 *normal, hsColorRGBA& multColor, hsColorRGBA& addColor,
                            hsPoint3 **uvPtrArray, float weight1 = -1.0f, float weight2 = -1.0f, float weight3 = -1.0f, uint32_t weightIndices = 0 );
        uint16_t  AddVertex( hsPoint3 *position, hsPoint3 *normal, uint32_t hexColor, uint32_t specularColor = 0,
                            hsPoint3 **uvPtrArray = nullptr, float weight1 = -1.0f, float weight2 = -1.0f, float weight3 = -1.0f, uint32_t weightIndices = 0);

        void    AddIndex( uint16_t index );
        void    AddTriIndices( uint16_t index1, uint16_t index2, uint16_t index3 );
        void    AddTriangle( hsPoint3 *vert1, hsPoint3 *vert2, hsPoint3 *vert3, uint32_t color );

        // uvws is an array count*uvwsPerVtx long in order [uvw(s) for vtx0, uvw(s) for vtx1, ...], or is nullptr
        void    AddVertexArray(uint32_t count, hsPoint3 *positions, hsVector3 *normals, uint32_t *colors, hsPoint3 *uvws=nullptr, int uvwsPerVtx=0);
        void    AddIndexArray( uint32_t count, uint16_t *indices );

        void    EndCreate();


        /// Manipulation--currently only used for applying static lighting, which of course needs individual vertices
        // Wrong. Also used for the interleaving of the multiple vertex data streams here into single vertex
        //      stream within the plGBufferGroups. mf.
        void    ExtractInitColor( uint32_t index, hsColorRGBA *multColor, hsColorRGBA *addColor) const;
        void    ExtractVertex(uint32_t index, hsPoint3 *pos, hsVector3 *normal, hsColorRGBA *color, hsColorRGBA *specColor = nullptr);
        void    ExtractVertex(uint32_t index, hsPoint3 *pos, hsVector3 *normal, uint32_t *color, uint32_t *specColor = nullptr);
        void    ExtractUv( uint32_t vIdx, uint8_t uvIdx, hsPoint3* uv );
        void    ExtractWeights( uint32_t vIdx, float *weightArray, uint32_t *indices );
        void    StuffVertex(uint32_t index, hsPoint3 *pos, hsPoint3 *normal, hsColorRGBA *color, hsColorRGBA *specColor = nullptr);
        void    StuffVertex(uint32_t index, hsColorRGBA *color, hsColorRGBA *specColor = nullptr);

        // Clear out the buffers
        void            ClearBuffers();

        // Duplicate this span from a given span
        void            CopyFrom( const plGeometrySpan *source );

        // Make this span an instance of the given span. Handles the instance ref array as well as copying over pointers
        void            MakeInstanceOf( const plGeometrySpan *instance );

        // Get the size of one vertex in a span, based on a format
        static uint32_t   GetVertexSize( uint8_t format );

        void    Read( hsStream *stream );
        void    Write( hsStream *stream );

        static uint32_t   AllocateNewGroupID() { return IAllocateNewGroupID(); }

        void        BreakInstance();
        void        ChangeInstance(plGeometrySpan* newInstance);
        void        UnInstance();

        void        AdjustBounds(hsBounds3Ext& bnd) const;

    protected:

        struct TempVertex 
        {
            hsPoint3    fPosition;
            hsPoint3    fNormal;
            uint32_t      fColor, fSpecularColor;
            hsColorRGBA fMultColor, fAddColor;
            hsPoint3    fUVs[ kMaxNumUVChannels ];
            float       fWeights[ 3 ];
            uint32_t      fIndices;
        };

        bool                    fCreating;
        std::vector<TempVertex> fVertAccum;
        std::vector<uint16_t>   fIndexAccum;

        void        IUnShareData();
        void        IDuplicateUniqueData( const plGeometrySpan *source );
        void        IClearMembers();

        // Please don't yell at me. We can't write out the instanceRef pointers, and we can't write
        // out keys because we're not keyed objects, and we can't be keyed objects because we need
        // to be deleted eventually. So instead, we assign each geoSpan a instanceGroupID, unique
        // for each instance group but identical among all geoSpans in a given group (i.e. all
        // members of the instanceRef list). We write these IDs out, then on read, we rebuild the
        // instanceRef arrays by using a hash table to find insert new vectors at the given groupID,
        // and looking up in that hash table to get pointers for each geoSpan's instanceRef array.
        // THIS is because we need a way of assigning unique, unused groupIDs to each geoSpan instance
        // group, and since we only need to know if the ID has been used yet, we can just use a bitVector.
        // NOTE: Group IDs start at 1, not 0, because 0 is reserved for "no instance group". So subtract
        // 1 from the group ID when accessing this array...
        // ....Please don't yell at me :(
        static hsBitVector      fInstanceGroupIDFlags;

        // The following is for rebuilding the said groups on read. The sad thing is that we also
        // have to write out the instanceRef array count for each geoSpan, so that when we read in
        // to do the lookup here, we know that we've read everything and can dump the entry in this
        // table.
        static std::vector<std::vector<plGeometrySpan *> *> fInstanceGroups;

        // THIS is so we can clear fInstanceGroups as early and as efficiently as possible; see
        // the notes on IGetInstanceGroup().
        static uint32_t   fHighestReadInstanceGroup;

        static uint32_t   IAllocateNewGroupID();
        static void     IClearGroupID( uint32_t groupID );

        static std::vector<plGeometrySpan *> *IGetInstanceGroup(uint32_t groupID, uint32_t expectedCount);
};


#endif // _plGeometrySpan_h
