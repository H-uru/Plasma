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
//	plGeometrySpan Header													//
//																			//
//	plGeometrySpans are abstract reprentations of Plasma 2.0 geometry data.	//
//	They consist of a material, a transform, bounds and an abstract vertex	//
//	and index buffer pair. plGeometrySpans is what is fed to plDrawableIce	//
//	to convert into its own internal data structures; the format for the	//
//	vertex and index data is (or should be) identical. More or less, they   //
//	are identical to Ice's plIcicle, but this is more abstract (read: not	//
//	internal to Ice).														//
//																			//
//	Also included is a temporary hacked triMesh-to-geometrySpan[] converter //
//	for everyone's convenience until triMeshes disappear.					//
//																			//
//// Version History /////////////////////////////////////////////////////////
//																			//
//	Created 3.8.2001 mcn													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plGeometrySpan_h
#define _plGeometrySpan_h


#include "hsTemplates.h"
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
			kMaxNumUVChannels	= 8
		};

		/// Duplication of the formats from plGBufferGroup; theoretically, they
		/// could be different, but they're identical for now
		enum Formats
		{
			kUVCountMask	= 0x0f,	// Problem is, we need enough bits to store the max #, which means
									// we really want ( max # << 1 ) - 1

			kSkinNoWeights  = 0x00,	// 0000000
			kSkin1Weight	= 0x10,	// 0010000
			kSkin2Weights	= 0x20,	// 0100000
			kSkin3Weights	= 0x30,	// 0110000
			kSkinWeightMask	= 0x30,	// 0110000

			kSkinIndices	= 0x40, // 1000000
		};

		enum Properties
		{
			kPropRunTimeLight		= 0x01,
			kPropNoPreShade			= 0x02,
			kLiteMaterial			= 0x00,
			kLiteVtxPreshaded		= 0x04,
			kLiteVtxNonPreshaded	= 0x08,
			kLiteMask				= 0x0c,
			kRequiresBlending		= 0x10,
			kInstanced				= 0x20,
			kUserOwned				= 0x40,
			kPropNoShadow			= 0x80,
			kPropForceShadow		= 0x100,
			kDiffuseFoldedIn		= 0x200,		// Sometimes we want to fold the diffuse color of the material into the vertex color (but only once).
			kPropReverseSort		= 0x400,
			kWaterHeight			= 0x800,
			kFirstInstance			= 0x1000,
			kPartialSort			= 0x2000,
			kVisLOS					= 0x4000,
			kPropNoShadowCast		= 0x8000
		};

		enum
		{
			kNoGroupID = 0
		};

		// Note: these are public because this is really just a glorified
		// struct; no data hiding here
		hsGMaterial			*fMaterial;
		hsMatrix44			fLocalToWorld;
		hsMatrix44			fWorldToLocal;
		hsBounds3Ext		fLocalBounds;
		hsBounds3Ext		fWorldBounds;
		plFogEnvironment	*fFogEnviron;

		UInt32			fBaseMatrix;
		UInt8			fNumMatrices;
		UInt16			fLocalUVWChans;
		UInt16			fMaxBoneIdx;
		UInt32			fPenBoneIdx;

		hsScalar		fMinDist;
		hsScalar		fMaxDist;

		hsScalar		fWaterHeight;

		UInt8			fFormat;
		UInt32			fProps;
		UInt32			fNumVerts, fNumIndices;

		/// Current vertex format:
		///		float	position[ 3 ];
		///		float	normal[ 3 ];
		///		float	uvCoords[ ][ 3 ];
		///		float	weights[];				// 0-3 blending weights
		///		UInt32	weightIndices;			// Only if there are >= 1 blending weights

		UInt8*			fVertexData;
		UInt16*			fIndexData;
		UInt32			fDecalLevel;

		hsColorRGBA*	fMultColor;
		hsColorRGBA*	fAddColor;

		UInt32*			fDiffuseRGBA;
		UInt32*			fSpecularRGBA;

		mutable hsTArray<plGeometrySpan *>*	fInstanceRefs;
		mutable UInt32						fInstanceGroupID;		// For writing out/reading in instance refs

		// The following is only used for logging during export. It is never set
		// at runtime. Don't even think about using it for anything.
		const char*							fMaxOwner; 

		// The following is ONLY used during pack; it's so we can do a reverse lookup
		// from the instanceRefs list to the correct span in the drawable
		UInt32		fSpanRefIndex;

		// These two matrices are inverses of each other (duh). They are only used on computing the local
		// bounds. fLocalBounds is always the bounds in the space defined by fWorldToLocal, but the bounds
		// are an OBB, and the orientation of the OBB isn't necessarily the same as fLocalToWorld's axes.
		// For now, it is the orientation of the pivot point in max (but might be further optimized).
		hsMatrix44		fLocalToOBB;
		hsMatrix44		fOBBToLocal;

		plGeometrySpan();
		plGeometrySpan( const plGeometrySpan *instance );
		~plGeometrySpan();

		/// UV stuff
		UInt8	GetNumUVs( void ) const { return ( fFormat & kUVCountMask ); }
		void	SetNumUVs( UInt8 numUVs ) 
		{
			hsAssert( numUVs < kMaxNumUVChannels, "Invalid UV count to plGeometrySpan" );
			fFormat = ( fFormat & ~kUVCountMask ) | numUVs; 
		}

		static UInt8 CalcNumUVs( UInt8 format ) { return ( format & kUVCountMask ); }
		static UInt8 UVCountToFormat( UInt8 numUVs ) { return numUVs & kUVCountMask; }
 
 		/// Creation functions
		void	BeginCreate( hsGMaterial *material, const hsMatrix44 &l2wMatrix, UInt8 format );

		// Phasing these in...
		// Note: uvArray should be a fixed array with enough pointers for the max # of uv channels.
		// Any unused UVs should be nil
		UInt16	AddVertex( hsPoint3 *position, hsPoint3 *normal, hsColorRGBA& multColor, hsColorRGBA& addColor,
							hsPoint3 **uvPtrArray, float weight1 = -1.0f, float weight2 = -1.0f, float weight3 = -1.0f, UInt32 weightIndices = 0 );
		UInt16	AddVertex( hsPoint3 *position, hsPoint3 *normal, UInt32 hexColor, UInt32 specularColor = 0,
							hsPoint3 **uvPtrArray = nil, float weight1 = -1.0f, float weight2 = -1.0f, float weight3 = -1.0f, UInt32 weightIndices = 0 );

		void	AddIndex( UInt16 index );
		void	AddTriIndices( UInt16 index1, UInt16 index2, UInt16 index3 );
		void	AddTriangle( hsPoint3 *vert1, hsPoint3 *vert2, hsPoint3 *vert3, UInt32 color );

		// uvws is an array count*uvwsPerVtx long in order [uvw(s) for vtx0, uvw(s) for vtx1, ...], or is nil
		void	AddVertexArray( UInt32 count, hsPoint3 *positions, hsVector3 *normals, UInt32 *colors, hsPoint3 *uvws=nil, int uvwsPerVtx=0 );
		void	AddIndexArray( UInt32 count, UInt16 *indices );

		void	EndCreate( void );


		/// Manipulation--currently only used for applying static lighting, which of course needs individual vertices
		// Wrong. Also used for the interleaving of the multiple vertex data streams here into single vertex
		//		stream within the plGBufferGroups. mf.
		void	ExtractInitColor( UInt32 index, hsColorRGBA *multColor, hsColorRGBA *addColor) const;
		void	ExtractVertex( UInt32 index, hsPoint3 *pos, hsVector3 *normal, hsColorRGBA *color, hsColorRGBA *specColor = nil );
		void	ExtractVertex( UInt32 index, hsPoint3 *pos, hsVector3 *normal, UInt32 *color, UInt32 *specColor = nil );
		void	ExtractUv( UInt32 vIdx, UInt8 uvIdx, hsPoint3* uv );
		void	ExtractWeights( UInt32 vIdx, float *weightArray, UInt32 *indices );
		void	StuffVertex( UInt32 index, hsPoint3 *pos, hsPoint3 *normal, hsColorRGBA *color, hsColorRGBA *specColor = nil );
		void	StuffVertex( UInt32 index, hsColorRGBA *color, hsColorRGBA *specColor = nil );

		// Clear out the buffers
		void			ClearBuffers( void );

		// Duplicate this span from a given span
		void			CopyFrom( const plGeometrySpan *source );

		// Make this span an instance of the given span. Handles the instance ref array as well as copying over pointers
		void			MakeInstanceOf( const plGeometrySpan *instance );

		// Get the size of one vertex in a span, based on a format
		static UInt32	GetVertexSize( UInt8 format );

		void	Read( hsStream *stream );
		void	Write( hsStream *stream );

		static UInt32	AllocateNewGroupID() { return IAllocateNewGroupID(); }

		void		BreakInstance();
		void		ChangeInstance(plGeometrySpan* newInstance);
		void		UnInstance();

		void		AdjustBounds(hsBounds3Ext& bnd) const;

	protected:

		struct TempVertex 
		{
			hsPoint3	fPosition;
			hsPoint3	fNormal;
			UInt32		fColor, fSpecularColor;
			hsColorRGBA	fMultColor, fAddColor;
			hsPoint3	fUVs[ kMaxNumUVChannels ];
			float		fWeights[ 3 ];
			UInt32		fIndices;
		};

		hsBool					fCreating;
		hsTArray<TempVertex>	fVertAccum;
		hsTArray<UInt16>		fIndexAccum;

		void		IUnShareData();
		void		IDuplicateUniqueData( const plGeometrySpan *source );
		void		IClearMembers( void );

		// Please don't yell at me. We can't write out the instanceRef pointers, and we can't write
		// out keys because we're not keyed objects, and we can't be keyed objects because we need
		// to be deleted eventually. So instead, we assign each geoSpan a instanceGroupID, unique
		// for each instance group but identical among all geoSpans in a given group (i.e. all
		// members of the instanceRef list). We write these IDs out, then on read, we rebuild the
		// instanceRef arrays by using a hash table to find insert new hsTArrays at the given groupID,
		// and looking up in that hash table to get pointers for each geoSpan's instanceRef array.
		// THIS is because we need a way of assigning unique, unused groupIDs to each geoSpan instance
		// group, and since we only need to know if the ID has been used yet, we can just use a bitVector.
		// NOTE: Group IDs start at 1, not 0, because 0 is reserved for "no instance group". So subtract
		// 1 from the group ID when accessing this array...
		// ....Please don't yell at me :(
		static hsBitVector		fInstanceGroupIDFlags;

		// The following is for rebuilding the said groups on read. The sad thing is that we also
		// have to write out the instanceRef array count for each geoSpan, so that when we read in
		// to do the lookup here, we know that we've read everything and can dump the entry in this
		// table.
		static hsTArray<hsTArray<plGeometrySpan *> *>	fInstanceGroups;

		// THIS is so we can clear fInstanceGroups as early and as efficiently as possible; see
		// the notes on IGetInstanceGroup().
		static UInt32	fHighestReadInstanceGroup;

		static UInt32	IAllocateNewGroupID( void );
		static void		IClearGroupID( UInt32 groupID );

		static hsTArray<plGeometrySpan *>	*IGetInstanceGroup( UInt32 groupID, UInt32 expectedCount );
};


#endif // _plGeometrySpan_h
