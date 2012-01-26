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
//  plGeometrySpan Class Functions                                          //
//                                                                          //
//// Version History /////////////////////////////////////////////////////////
//                                                                          //
//  Created 3.8.2001 mcn                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include "HeadSpin.h"
#include "plGeometrySpan.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"
#include "hsBitVector.h"


//// Static Data /////////////////////////////////////////////////////////////

hsBitVector     plGeometrySpan::fInstanceGroupIDFlags;
uint32_t          plGeometrySpan::fHighestReadInstanceGroup = 0;

hsTArray<hsTArray<plGeometrySpan *> *>  plGeometrySpan::fInstanceGroups;


//// Constructor and Destructor //////////////////////////////////////////////

plGeometrySpan::plGeometrySpan()
{
    IClearMembers();
}

plGeometrySpan::plGeometrySpan( const plGeometrySpan *instance )
{
    IClearMembers();
    MakeInstanceOf( instance ); 
}

plGeometrySpan::~plGeometrySpan()
{
    ClearBuffers();
}

void    plGeometrySpan::IClearMembers( void )
{
    fVertexData = nil; 
    fIndexData = nil; 
    fMaterial = nil; 
    fNumVerts = fNumIndices = 0; 
    fBaseMatrix = fNumMatrices = 0;
    fLocalUVWChans = 0;
    fMaxBoneIdx = 0;
    fPenBoneIdx = 0;
    fCreating = false;
    fFogEnviron = nil;
    fProps = 0;

    fMinDist = fMaxDist = -1.f;

    fWaterHeight = 0;

    fMultColor = nil;
    fAddColor = nil;

    fDiffuseRGBA = nil;
    fSpecularRGBA = nil;
    fInstanceRefs = nil;
    fInstanceGroupID = kNoGroupID;
    fSpanRefIndex = (uint32_t)-1;

    fLocalToOBB.Reset();
    fOBBToLocal.Reset();

    fDecalLevel = 0;

    fMaxOwner = nil;
}

//// ClearBuffers ////////////////////////////////////////////////////////////

void    plGeometrySpan::ClearBuffers( void )
{
    // If UserOwned, the actual buffer data belongs to someone else (like a BufferGroup).
    // Just erase our knowledge of it and move on.
    if( fProps & kUserOwned )
    {
        IClearMembers();
        return;
    }

    bool    removeData = true;


    // If we have an instanceRefs array, remove ourselves from
    // the array. If we are the last in the array, remove the array itself
    if( fInstanceRefs != nil )
    {
        if( fInstanceRefs->GetCount() == 1 )
        {
            delete fInstanceRefs;

            // Remove the group ID flag as well
            IClearGroupID( fInstanceGroupID );
        }
        else
        {
            int idx = fInstanceRefs->Find( this );
            hsAssert( idx != fInstanceRefs->kMissingIndex, "Invalid instance ref data in plGeometrySpan::ClearBuffers()" );

            fInstanceRefs->Remove( idx );
            removeData = false; // Don't remove data until we're the last one
        }
        fInstanceRefs = nil;
        fInstanceGroupID = kNoGroupID;
    }

    if( removeData )
    {
        delete [] fVertexData;
        fVertexData = nil;

        delete [] fMultColor;
        delete [] fAddColor;
        fMultColor = nil;
        fAddColor = nil;
    }

    delete [] fIndexData;
    delete [] fDiffuseRGBA;
    delete [] fSpecularRGBA;
    fIndexData = nil;
    fDiffuseRGBA = fSpecularRGBA = nil;
}

//// MakeInstanceOf //////////////////////////////////////////////////////////
//  Note: instancing copies the index buffer but just copies the POINTERS
//  for the vertex buffers and source colors. This is because the indices
//  will eventually change but the vertices won't.

void    plGeometrySpan::MakeInstanceOf( const plGeometrySpan *instance )
{
    hsTArray<plGeometrySpan *> *array;


    ClearBuffers();

    /// Adjust the instanceRefs array
    array = instance->fInstanceRefs;
    if( array == nil )
    {
        // Go find a new groupID
        instance->fInstanceGroupID = IAllocateNewGroupID();

        instance->fInstanceRefs = array = new hsTArray<plGeometrySpan *>;
        // Go figure, it won't append the const version to the array...this is a cheat,
        // but then, so is making fInstanceRefs mutable :)
        array->Append( (plGeometrySpan *)instance );
    }

    fInstanceGroupID = instance->fInstanceGroupID;
    array->Append( this );
    fInstanceRefs = array;

    /// Copy over the data
    IDuplicateUniqueData( instance );
    fVertexData = instance->fVertexData;
    fMultColor = instance->fMultColor;
    fAddColor = instance->fAddColor;

    /// All done!
}

void plGeometrySpan::IUnShareData()
{
    if( fVertexData )
    {
        uint8_t* oldVtxData = fVertexData;

        uint32_t size = GetVertexSize( fFormat );

        fVertexData = new uint8_t[ size * fNumVerts ];
        memcpy( fVertexData, oldVtxData, size * fNumVerts );
    }

    if( fMultColor )
    {
        hsColorRGBA* oldMult = fMultColor;

        fMultColor = new hsColorRGBA[ fNumVerts ];
        memcpy( fMultColor, oldMult, sizeof(hsColorRGBA) * fNumVerts );
    }

    if( fAddColor )
    {
        hsColorRGBA* oldAdd = fAddColor;

        fAddColor = new hsColorRGBA[ fNumVerts ];
        memcpy( fAddColor, oldAdd, sizeof(hsColorRGBA) * fNumVerts );
    }
}

void plGeometrySpan::BreakInstance()
{
    hsAssert(fInstanceRefs, "Breaking instancing when I'm not instanced");
    int idx = fInstanceRefs->Find(this);
    hsAssert(idx != fInstanceRefs->kMissingIndex, "I'm not in my own instance refs list");
    fInstanceRefs->Remove(idx);
    hsAssert(fInstanceRefs->GetCount(), "Don't BreakInstance if I'm the last one, use UnInstance instead");

    fInstanceGroupID = IAllocateNewGroupID();
    fInstanceRefs = new hsTArray<plGeometrySpan*>;
    fInstanceRefs->Append(this);

    IUnShareData();

    fProps |= plGeometrySpan::kInstanced;
}

void plGeometrySpan::ChangeInstance(plGeometrySpan* newInstance)
{
    hsAssert(fInstanceRefs, "Changing instancing when I'm not instanced");
    int idx = fInstanceRefs->Find(this);
    hsAssert(idx != fInstanceRefs->kMissingIndex, "I'm not in my own instance refs list");
    fInstanceRefs->Remove(idx);

    fInstanceGroupID = newInstance->fInstanceGroupID;
    fInstanceRefs = newInstance->fInstanceRefs;
    fInstanceRefs->Append(this);

    fVertexData = newInstance->fVertexData;
    fMultColor = newInstance->fMultColor;
    fAddColor = newInstance->fAddColor;

    fProps |= plGeometrySpan::kInstanced;
}

void plGeometrySpan::UnInstance()
{
    hsAssert(fInstanceRefs, "UnInstancing a non-instance");

    int idx = fInstanceRefs->Find(this);
    hsAssert(idx != fInstanceRefs->kMissingIndex, "I'm not in my own instance refs list");
    fInstanceRefs->Remove(idx);

    // If we're the last one, we just take ownership of the shared data,
    // else we make our own copy of it.
    if( !fInstanceRefs->GetCount() )
    {
        delete fInstanceRefs;
    }
    else
    {
        IUnShareData();
    }
    fInstanceRefs = nil;
    fInstanceGroupID = kNoGroupID;
    fProps &= ~plGeometrySpan::kInstanced;
}

//// IAllocateNewGroupID /////////////////////////////////////////////////////
//  Static function that allocates a new groupID by finding an empty slot in
//  the bitVector, then marking it as used and returning that bit #

uint32_t  plGeometrySpan::IAllocateNewGroupID( void )
{
    uint32_t          id;


    for( id = 0; id < fInstanceGroupIDFlags.GetSize(); id++ )
    {
        if( !fInstanceGroupIDFlags.IsBitSet( id ) )
            break;
    }

    fInstanceGroupIDFlags.SetBit( id, true );
    return id + 1;
}

//// IClearGroupID ///////////////////////////////////////////////////////////
//  Done with this groupID, so clear the entry in the bit table.

void    plGeometrySpan::IClearGroupID( uint32_t groupID )
{
    fInstanceGroupIDFlags.ClearBit( groupID - 1 );
}

//// IGetInstanceGroup ///////////////////////////////////////////////////////
//  This does the whole hash table thing lookup during read. If:
//  - The group ID does not yet exist:
//      - Allocates a new hsTArray, puts it into the hash table if expectedCount > 1,
//        sets the groupID flag, and returns a pointer to the array.
//  - The group ID does exist:
//      - If the array count is less than expectedCount - 1, returns the array
//      - else sets the hash entry to nil and returns the array. 
//  Since we want to clear the hash table as soon as possible, but don't want
//  to search the entire hash table every time to make sure its empty, we
//  keep an ID of the highest element in the hash table that's set; every time
//  we remove an entry, we decrement this ID until we hit a used pointer again;
//  if we don't find one, we reset the array.

hsTArray<plGeometrySpan *>  *plGeometrySpan::IGetInstanceGroup( uint32_t groupID, uint32_t expectedCount )
{
    hsTArray<plGeometrySpan *>  *array;


    groupID--;      // Make it our array index

    if( fInstanceGroups.GetCount() <= groupID || fInstanceGroups[ groupID ] == nil )
    {
        // Not yet in the list--make a new hsTArray
        array = new hsTArray<plGeometrySpan *>;
        fInstanceGroupIDFlags.SetBit( groupID, true );
        
        if( expectedCount > 1 )
        {
            if( fInstanceGroups.GetCount() <= groupID )
                fInstanceGroups.ExpandAndZero( groupID + 1 );

            fInstanceGroups[ groupID ] = array;
            if( fHighestReadInstanceGroup < groupID + 1 )
                fHighestReadInstanceGroup = groupID + 1;
        }

        return array;
    }
    else 
    {
        // In the list...get it, but are we done with it?
        array = fInstanceGroups[ groupID ];
        if( expectedCount == array->GetCount() + 1 )    // I.E. next Append() will make it ==
        {
            // Done with it, remove from hash table
            fInstanceGroups[ groupID ] = nil;
            
            // Find new fHighestReadInstanceGroup
            for( ; fHighestReadInstanceGroup > 0 && fInstanceGroups[ fHighestReadInstanceGroup - 1 ] == nil; 
                   fHighestReadInstanceGroup-- );

            if( fHighestReadInstanceGroup == 0 )
                fInstanceGroups.Reset();
        }

        // Either way, return the array
        return array;
    }
}

//// IDuplicateUniqueData ////////////////////////////////////////////////////
//  Does the copy on all unique data--i.e. data copied for both clones and
//  instances.

void    plGeometrySpan::IDuplicateUniqueData( const plGeometrySpan *source )
{
    fCreating = source->fCreating;
    fVertAccum = source->fVertAccum;
    fIndexAccum = source->fIndexAccum;
    
    fMaterial = source->fMaterial;
    fLocalToWorld = source->fLocalToWorld;
    fWorldToLocal = source->fWorldToLocal;
    fLocalBounds = source->fLocalBounds;
    fWorldBounds = source->fWorldBounds;
    fFogEnviron = source->fFogEnviron;

    fBaseMatrix = source->fBaseMatrix;
    fNumMatrices = source->fNumMatrices;
    fLocalUVWChans = source->fLocalUVWChans;

    fFormat = source->fFormat;
    fProps = source->fProps;
    fNumVerts = source->fNumVerts;
    fNumIndices = source->fNumIndices;
    fDecalLevel = source->fDecalLevel;

    if( source->fIndexData != nil )
    {
        fIndexData = new uint16_t[ fNumIndices ];
        memcpy( fIndexData, source->fIndexData, sizeof( uint16_t ) * fNumIndices );
    }
    else
        fIndexData = nil;

    if( source->fDiffuseRGBA )
    {
        fDiffuseRGBA = new uint32_t[ fNumVerts ];
        memcpy( fDiffuseRGBA, source->fDiffuseRGBA, sizeof( uint32_t ) * fNumVerts );
    }
    else 
        fDiffuseRGBA = nil;

    if( source->fSpecularRGBA )
    {
        fSpecularRGBA = new uint32_t[ fNumVerts ];
        memcpy( fSpecularRGBA, source->fSpecularRGBA, sizeof( uint32_t ) * fNumVerts );
    }
    else 
        fSpecularRGBA = nil;

    fLocalToOBB = source->fLocalToOBB;
    fOBBToLocal = source->fOBBToLocal;
}

//// CopyFrom ////////////////////////////////////////////////////////////////
//  Duplicate this span from a given span.

void    plGeometrySpan::CopyFrom( const plGeometrySpan *source )
{
    uint32_t      size;


    // Just to make sure
    ClearBuffers();

    IDuplicateUniqueData( source );

    fInstanceRefs = nil;
    fInstanceGroupID = kNoGroupID;

    if( source->fVertexData != nil )
    {
        size = GetVertexSize( fFormat );

        fVertexData = new uint8_t[ size * fNumVerts ];
        memcpy( fVertexData, source->fVertexData, size * fNumVerts );
    }
    else
        fVertexData = nil;

    if( source->fMultColor )
    {
        fMultColor = new hsColorRGBA[ fNumVerts ];
        memcpy( fMultColor, source->fMultColor, sizeof(hsColorRGBA) * fNumVerts );
    }
    else
    {
        fMultColor = nil;
    }

    if( source->fAddColor )
    {
        fAddColor = new hsColorRGBA[ fNumVerts ];
        memcpy( fAddColor, source->fAddColor, sizeof(hsColorRGBA) * fNumVerts );
    }
    else
    {
        fAddColor = nil;
    }
}

//// Read ////////////////////////////////////////////////////////////////////

void    plGeometrySpan::Read( hsStream *stream )
{
    uint32_t      size, i;


    hsAssert( !fCreating, "Cannot read geometry span while creating" );

    // Just to make sure
    ClearBuffers();

    fCreating = false;

    // WARNING: can't read in the material here, so hopefully the owner will set it for us...
    // Same with the fog environ

    fLocalToWorld.Read( stream );
    fWorldToLocal.Read( stream );
    fLocalBounds.Read( stream );
    fWorldBounds = fLocalBounds;
    fWorldBounds.Transform(&fLocalToWorld);

    fOBBToLocal.Read(stream);
    fLocalToOBB.Read(stream);

    fBaseMatrix = stream->ReadLE32();
    fNumMatrices = stream->ReadByte();
    fLocalUVWChans = stream->ReadLE16();
    fMaxBoneIdx = stream->ReadLE16();
    fPenBoneIdx = stream->ReadLE16();

    fMinDist = stream->ReadLEScalar();
    fMaxDist = stream->ReadLEScalar();

    fFormat = stream->ReadByte();
    fProps = stream->ReadLE32();
    fNumVerts = stream->ReadLE32();
    fNumIndices = stream->ReadLE32();

    // FIXME MAJOR VERSION
    // remove these two lines. No more patches.
    stream->ReadLE32();
    stream->ReadByte();

    fDecalLevel = stream->ReadLE32();

    if( fProps & kWaterHeight )
        fWaterHeight = stream->ReadLEScalar();

    if( fNumVerts > 0 )
    {
        size = GetVertexSize( fFormat );

        fVertexData = new uint8_t[ size * fNumVerts ];
        stream->Read( size * fNumVerts, fVertexData );

        fMultColor = new hsColorRGBA[ fNumVerts ];
        fAddColor = new hsColorRGBA[ fNumVerts ];
        for( i = 0; i < fNumVerts; i++ )
        {
            fMultColor[ i ].Read( stream );
            fAddColor[ i ].Read( stream );
        }

        fDiffuseRGBA = new uint32_t[ fNumVerts ];
        fSpecularRGBA = new uint32_t[ fNumVerts ];
        stream->ReadLE32( fNumVerts, fDiffuseRGBA );
        stream->ReadLE32( fNumVerts, fSpecularRGBA );
    }
    else
    {
        fVertexData = nil;
        fMultColor = nil;
        fAddColor = nil;
        fDiffuseRGBA = nil;
        fSpecularRGBA = nil;
    }

    if( fNumIndices > 0 )
    {
        fIndexData = new uint16_t[ fNumIndices ];
        stream->ReadLE16( fNumIndices, fIndexData );
    }
    else
        fIndexData = nil;

    // Read the group ID, then look up our instanceRef array from it
    fInstanceGroupID = stream->ReadLE32();
    if( fInstanceGroupID != kNoGroupID )
    {
        uint32_t  count = stream->ReadLE32();

        fInstanceRefs = IGetInstanceGroup( fInstanceGroupID, count );
        fInstanceRefs->Append( this );
        hsAssert( fInstanceRefs != nil, "Cannot locate fInstanceRefs on plGeometrySpan::Read()" );
    }
}

//// Write ///////////////////////////////////////////////////////////////////

void    plGeometrySpan::Write( hsStream *stream )
{
    uint32_t      size, i;


    hsAssert( !fCreating, "Cannot write geometry span while creating" );

    // WARNING: can't write out the material here, so hopefully the owner will do it for us...
    // Same with the fog environ

    fLocalToWorld.Write( stream );
    fWorldToLocal.Write( stream );
    fLocalBounds.Write( stream );

    fOBBToLocal.Write(stream);
    fLocalToOBB.Write(stream);

    stream->WriteLE32( fBaseMatrix );
    stream->WriteByte( fNumMatrices );
    stream->WriteLE16(fLocalUVWChans);
    stream->WriteLE16(fMaxBoneIdx);
    stream->WriteLE16((uint16_t)fPenBoneIdx);

    stream->WriteLEScalar(fMinDist);
    stream->WriteLEScalar(fMaxDist);

    stream->WriteByte( fFormat );
    stream->WriteLE32( fProps );
    stream->WriteLE32( fNumVerts );
    stream->WriteLE32( fNumIndices );

    // FIXME MAJOR VERSION
    // Remove these two lines.
    stream->WriteLE32(0);
    stream->WriteByte(0);

    stream->WriteLE32( fDecalLevel );

    if( fProps & kWaterHeight )
        stream->WriteLEScalar(fWaterHeight);

    if( fNumVerts > 0 )
    {
        size = GetVertexSize( fFormat );

        stream->Write( size * fNumVerts, fVertexData );

        for( i = 0; i < fNumVerts; i++ )
        {
            fMultColor[ i ].Write( stream );
            fAddColor[ i ].Write( stream );
        }
        stream->WriteLE32( fNumVerts, fDiffuseRGBA );
        stream->WriteLE32( fNumVerts, fSpecularRGBA );
    }

    if( fNumIndices > 0 )
    {
        stream->WriteLE16( fNumIndices, fIndexData );
    }

    // Write the groupID as well as the count for instanceRefs. This way
    stream->WriteLE32( fInstanceGroupID );
    if( fInstanceGroupID != kNoGroupID )
        stream->WriteLE32( fInstanceRefs->GetCount() );
    else
    {
        hsAssert( fInstanceRefs == nil, "Nil instanceRefs array but no group ID, non sequitur" );
    }
}

//// GetVertexSize ///////////////////////////////////////////////////////////

uint32_t  plGeometrySpan::GetVertexSize( uint8_t format )
{
    uint32_t  size;


    size = sizeof( float ) * ( 3 + 3 ); // pos + normal
    // Taken out 8.6.2001 mcn - Diffuse and specular are now separate arrays
//  size += sizeof( DWORD ) * 2;            // diffuse + specular

    size += sizeof( float ) * 3 * CalcNumUVs( format );

    switch( format & kSkinWeightMask )
    {
        case kSkinNoWeights: break;
        case kSkin1Weight:  size += sizeof( float ) * 1; break;
        case kSkin2Weights: size += sizeof( float ) * 2; break;
        case kSkin3Weights: size += sizeof( float ) * 3; break;
        default: hsAssert( false, "Bad weight count in GetVertexSize()" );
    }
    if( format & kSkinIndices )
        size += sizeof(uint32_t);

    return size;
}

//// BeginCreate //////////////////////////////////////////////////////////////

void    plGeometrySpan::BeginCreate( hsGMaterial *material, const hsMatrix44 &l2wMatrix, uint8_t format )
{
    fCreating = true;

    fMaterial = material;
    fLocalToWorld = l2wMatrix;
    fLocalToWorld.GetInverse( &fWorldToLocal );
    fFormat = format;
}

//// AddVertex ////////////////////////////////////////////////////////////////
//  Note: uvPtrArray is an array of pointers to hsPoint3s. If a pointer is nil,
//  that UV channel (and all above them) are not used. The array of pointers 
//  MUST be of size kMaxNumUVChannels.

uint16_t  plGeometrySpan::AddVertex( hsPoint3 *position, hsPoint3 *normal, hsColorRGBA& multColor, hsColorRGBA& addColor, 
                                    hsPoint3 **uvPtrArray, float weight1, float weight2, float weight3, uint32_t indices )
{
    AddVertex( position, normal, 0, 0, uvPtrArray, weight1, weight2, weight3, indices );

    int idx = fVertAccum.GetCount() - 1;

    TempVertex& vert = fVertAccum[idx];
    vert.fMultColor = multColor;
    vert.fAddColor = addColor;
    

    return idx;
}

uint16_t  plGeometrySpan::AddVertex( hsPoint3 *position, hsPoint3 *normal, uint32_t hexColor, uint32_t specularColor, 
                                    hsPoint3 **uvPtrArray, float weight1, float weight2, float weight3, uint32_t indices )
{
    TempVertex      vert;
    int             i, numWeights;


    hsAssert( fCreating, "Calling AddVertex() on a non-creating plGeometrySpan!" );

    // UV channels
    if( uvPtrArray != nil )
    {
        for( i = 0; i < kMaxNumUVChannels; i++ )
        {
            if( uvPtrArray[ i ] == nil )
                break;
            vert.fUVs[ i ] = *(uvPtrArray[ i ]);
        }
    }
    else
        i = 0;
    hsAssert( GetNumUVs() == i, "Incorrect number of UVs passed to AddVertex()" );

    switch( fFormat & kSkinWeightMask )
    {
    case kSkin3Weights:
        numWeights = 3;
        vert.fWeights[ 0 ] = weight1;
        vert.fWeights[ 1 ] = weight2;
        vert.fWeights[ 2 ] = weight3;
        vert.fIndices = indices;
        break;
    case kSkin2Weights:
        numWeights = 2;
        vert.fWeights[ 0 ] = weight1;
        vert.fWeights[ 1 ] = weight2;
        vert.fIndices = indices;
        break;
    case kSkin1Weight:
        numWeights = 1;
        vert.fWeights[ 0 ] = weight1;
        vert.fIndices = indices;
        break;
    default:
    case kSkinNoWeights:
        numWeights = 0;
        break;
    }

    vert.fPosition = *position;
    vert.fNormal = *normal;
    vert.fColor = hexColor;
    vert.fSpecularColor = specularColor;
    vert.fMultColor.Set( 1.f, 1.f, 1.f, 1.f );
    vert.fAddColor.Set( 0, 0, 0, 1.f );

    fVertAccum.Append( vert );
    return fVertAccum.GetCount() - 1;
}

//// AddIndex Variations //////////////////////////////////////////////////////

void    plGeometrySpan::AddIndex( uint16_t index )
{
    hsAssert( fCreating, "Calling AddIndex() on a non-creating plGeometrySpan!" );

    fIndexAccum.Append( index );
}

void    plGeometrySpan::AddTriIndices( uint16_t index1, uint16_t index2, uint16_t index3 )
{
    hsAssert( fCreating, "Calling AddTriIndices() on a non-creating plGeometrySpan!" );

    fIndexAccum.Append( index1 );
    fIndexAccum.Append( index2 );
    fIndexAccum.Append( index3 );
}

//// AddTriangle //////////////////////////////////////////////////////////////

void    plGeometrySpan::AddTriangle( hsPoint3 *vert1, hsPoint3 *vert2, hsPoint3 *vert3, uint32_t color )
{
    hsVector3       twoTo1, twoTo3, normal;
    hsPoint3        normalPt;


    hsAssert( fCreating, "Calling AddTriangle() on a non-creating plGeometrySpan!" );

    twoTo1.Set( vert1, vert2 );
    twoTo3.Set( vert3, vert2 );

    normal = twoTo1 % twoTo3;
    normalPt.fX = normal.fX;
    normalPt.fY = normal.fY;
    normalPt.fZ = normal.fZ;

    AddIndex( AddVertex( vert1, &normalPt, color, 0 ) );
    AddIndex( AddVertex( vert2, &normalPt, color, 0 ) );
    AddIndex( AddVertex( vert3, &normalPt, color, 0 ) );
}

//// AddVertexArray ///////////////////////////////////////////////////////////

void    plGeometrySpan::AddVertexArray( uint32_t count, hsPoint3 *positions, hsVector3 *normals, uint32_t *colors, hsPoint3* uvws, int uvwsPerVtx )
{
    hsAssert( fCreating, "Calling AddTriIndices() on a non-creating plGeometrySpan!" );


    uint32_t      i, dest;

    // This test actually does work, even if it's bad form...
    hsAssert( GetNumUVs() == uvwsPerVtx, "Calling wrong AddVertex() for plGeometrySpan format" );


    dest = fVertAccum.GetCount();
    fVertAccum.SetCount( dest + count );
    for( i = 0; i < count; i++, dest++ )
    {
        fVertAccum[ dest ].fPosition = positions[ i ];
        if( normals != nil )
        {
            // Stupid hsPoint3...I COULD change it, but why?
            fVertAccum[ dest ].fNormal.fX = normals[ i ].fX;
            fVertAccum[ dest ].fNormal.fY = normals[ i ].fY;
            fVertAccum[ dest ].fNormal.fZ = normals[ i ].fZ;
        }
        else
            fVertAccum[ dest ].fNormal.Set( 0, 0, 0 );

        if( colors != nil )
            fVertAccum[ dest ].fColor = colors[ i ];
        else
            fVertAccum[ dest ].fColor = 0xffffffff;

        if( uvws && uvwsPerVtx )
        {
            int j;
            for( j = 0; j < uvwsPerVtx; j++ )
                fVertAccum[ dest ].fUVs[ j ] = uvws[j];
            uvws += uvwsPerVtx;
        }
    }
}

//// AddIndexArray ////////////////////////////////////////////////////////////

void    plGeometrySpan::AddIndexArray( uint32_t count, uint16_t *indices )
{
    hsAssert( fCreating, "Calling AddTriIndices() on a non-creating plGeometrySpan!" );


    uint32_t      i, dest;


    dest = fIndexAccum.GetCount();
    fIndexAccum.SetCount( dest + count );
    for( i = 0; i < count; i++, dest++ )
        fIndexAccum[ dest ] = indices[ i ];
}

//// EndCreate ////////////////////////////////////////////////////////////////

void    plGeometrySpan::EndCreate( void )
{
    hsBounds3Ext    bounds;
    uint32_t          i, size;
    uint8_t           *tempPtr;


    hsAssert( fCreating, "Calling EndCreate() on a non-creating plGeometrySpan!" );

    /// If we're empty, just clean up and return
    if( fVertAccum.GetCount() <= 0 )
    {
        delete [] fVertexData;
        fVertexData = nil;
        fNumVerts = 0;

        delete [] fIndexData;
        fIndexData = nil;
        fNumIndices = 0;
        fVertAccum.Reset();
        fIndexAccum.Reset();

        fCreating = false;
        return;
    }

    /// Convert vertices
    bounds.MakeEmpty();
    size = GetVertexSize( fFormat );

    if( fVertexData == nil || fNumVerts < fVertAccum.GetCount() )
    {
        if( fVertexData != nil )
            delete [] fVertexData;

        fNumVerts = fVertAccum.GetCount();
        fVertexData = new uint8_t[ size * fNumVerts ];

        delete [] fMultColor;
        fMultColor = new hsColorRGBA[ fNumVerts ];

        delete [] fAddColor;
        fAddColor = new hsColorRGBA[ fNumVerts ];

        delete [] fDiffuseRGBA;
        delete [] fSpecularRGBA;
        fDiffuseRGBA = new uint32_t[ fNumVerts ];
        fSpecularRGBA = new uint32_t[ fNumVerts ];
    }
    else
        fNumVerts = fVertAccum.GetCount();

    for( i = 0, tempPtr = fVertexData; i < fNumVerts; i++ )
    {
        // Get position, normal, color
        memcpy( tempPtr, &fVertAccum[ i ], 6 * sizeof(float) );
        tempPtr += 6 * sizeof(float);

        // Get Uvs
        int numUvs = GetNumUVs();
        if( numUvs > 0 )
        {
            memcpy( tempPtr, &fVertAccum[ i ].fUVs[ 0 ], numUvs * 3 * sizeof(float) );
            tempPtr += numUvs * 3 * sizeof(float);
        }

        // Get Weights
        if( fFormat & kSkinWeightMask )
        {
            int numWeights = 0;
            switch( fFormat & kSkinWeightMask )
            {
            case kSkin1Weight: numWeights = 1; break;
            case kSkin2Weights: numWeights = 2; break;
            case kSkin3Weights: numWeights = 3; break;
            default: hsAssert(false, "Garbage for weight format"); break;
            }
            memcpy( tempPtr, &fVertAccum[ i ].fWeights[ 0 ], numWeights * sizeof(float) );
            tempPtr += numWeights * sizeof(float);
            if( fFormat & kSkinIndices )
            {
                memcpy( tempPtr, &fVertAccum[ i ].fIndices, sizeof(uint32_t) );
                tempPtr += sizeof(uint32_t);
            }
        }

        fMultColor[i] = fVertAccum[i].fMultColor;
        fAddColor[i] = fVertAccum[i].fAddColor;
        fDiffuseRGBA[ i ] = fVertAccum[ i ].fColor;
        fSpecularRGBA[ i ] = fVertAccum[ i ].fSpecularColor;

        hsPoint3 pBnd = fLocalToOBB * fVertAccum[i].fPosition;
        bounds.Union(&pBnd);
    }
    bounds.Transform(&fOBBToLocal);
    
    if( fProps & kWaterHeight )
    {
        AdjustBounds(bounds);
    }
    
    fLocalBounds = bounds;
    fWorldBounds = fLocalBounds;
    fWorldBounds.Transform(&fLocalToWorld);

    /// Convert indices
    if( fIndexAccum.GetCount() == 0 )       // Allowed for patches
    {
        delete [] fIndexData;
        fIndexData = nil;
        fNumIndices = 0;
    }
    else if( fIndexData == nil || fNumIndices < fIndexAccum.GetCount() )
    {
        if( fIndexData != nil )
            delete [] fIndexData;

        fNumIndices = fIndexAccum.GetCount();
        fIndexData = new uint16_t[ fNumIndices ];
    }
    else
        fNumIndices = fIndexAccum.GetCount();

    for( i = 0; i < fNumIndices; i++ )
        fIndexData[ i ] = fIndexAccum[ i ];

    /// Cleanup
    fVertAccum.Reset();
    fIndexAccum.Reset();
    fCreating = false;
}

void plGeometrySpan::AdjustBounds(hsBounds3Ext& bnd) const
{
    hsBounds3Ext wBnd = bnd;
    wBnd.Transform(&fLocalToWorld);

    const float kMaxWaveHeight(5.f);
    hsBounds3Ext rebound;
    rebound.MakeEmpty();
    hsPoint3 pos = wBnd.GetMins();
    pos.fZ = fWaterHeight - kMaxWaveHeight;
    rebound.Union(&pos);
    pos = wBnd.GetMaxs();
    pos.fZ = fWaterHeight + kMaxWaveHeight;
    rebound.Union(&pos);
    bnd = rebound;
    bnd.Transform(&fWorldToLocal);
}

//// ExtractVertex ////////////////////////////////////////////////////////////
void    plGeometrySpan::ExtractInitColor( uint32_t index, hsColorRGBA *multColor, hsColorRGBA *addColor) const
{
    if( multColor )
        *multColor = fMultColor[index];
    if( addColor )
        *addColor = fAddColor[index];
}

//  Extracts the given vertex out of the vertex buffer and into the pointers
//  given.

void    plGeometrySpan::ExtractVertex( uint32_t index, hsPoint3 *pos, hsVector3 *normal, hsColorRGBA *color, hsColorRGBA *specColor )
{
    uint32_t      hex, spec;


    ExtractVertex( index, pos, normal, &hex, &spec );
    color->a = ( ( hex >> 24 ) & 0xff ) / 255.0f;
    color->r = ( ( hex >> 16 ) & 0xff ) / 255.0f;
    color->g = ( ( hex >>  8 ) & 0xff ) / 255.0f;
    color->b = ( ( hex >>  0 ) & 0xff ) / 255.0f;

    if( specColor != nil )
    {
        specColor->a = ( ( spec >> 24 ) & 0xff ) / 255.0f;
        specColor->r = ( ( spec >> 16 ) & 0xff ) / 255.0f;
        specColor->g = ( ( spec >>  8 ) & 0xff ) / 255.0f;
        specColor->b = ( ( spec >>  0 ) & 0xff ) / 255.0f;
    }
}

//// ExtractVertex ////////////////////////////////////////////////////////////
//  Hex-color version of ExtractVertex.

void    plGeometrySpan::ExtractVertex( uint32_t index, hsPoint3 *pos, hsVector3 *normal, uint32_t *color, uint32_t *specColor )
{
    uint8_t       *basePtr;
    float       *fPtr;


    /// Where?
    hsAssert( index < fNumVerts, "Invalid index passed to ExtractVertex()" );
    basePtr = fVertexData + index * GetVertexSize( fFormat );

    /// Copy over point and normal
    fPtr = (float *)basePtr;
    pos->fX = fPtr[ 0 ];
    pos->fY = fPtr[ 1 ];
    pos->fZ = fPtr[ 2 ];
    normal->fX = fPtr[ 3 ];
    normal->fY = fPtr[ 4 ];
    normal->fZ = fPtr[ 5 ];
    fPtr += 6;

    /// Diffuse color
    *color = fDiffuseRGBA[ index ];
    if( specColor != nil )
        *specColor = fSpecularRGBA[ index ];
}

//// ExtractUv ////////////////////////////////////////////////////////////////

void    plGeometrySpan::ExtractUv( uint32_t vIdx, uint8_t uvIdx, hsPoint3 *uv )
{
    uint8_t       *basePtr;
    float       *fPtr;


    /// Where?
    hsAssert( vIdx < fNumVerts, "Invalid index passed to ExtractVertex()" );
    hsAssert( uvIdx < GetNumUVs(), "Invalid UV index passed to ExtractVertex()" );
    basePtr = fVertexData + vIdx * GetVertexSize( fFormat );

    /// Skip over point, normal, and color and specular color
    basePtr += 12 + 12;

    fPtr = (float *)basePtr;
    fPtr += 3 * uvIdx;
    uv->fX = *fPtr++;
    uv->fY = *fPtr++;
    uv->fZ = *fPtr;
}

//// ExtractWeights ///////////////////////////////////////////////////////////
//  Gets the weights out of the vertex data.

void    plGeometrySpan::ExtractWeights( uint32_t vIdx, float *weightArray, uint32_t *indices )
{
    uint8_t       *basePtr;
    float       *fPtr;
    uint32_t      *dPtr;
    int         numWeights;


    /// Where?
    hsAssert( vIdx < fNumVerts, "Invalid index passed to ExtractVertex()" );
    basePtr = fVertexData + vIdx * GetVertexSize( fFormat );

    /// Copy over weights
    basePtr += sizeof( float ) * ( 6 + 3 * GetNumUVs() );
    fPtr = (float *)basePtr;
    switch( fFormat & kSkinWeightMask )
    {
        case kSkinNoWeights: hsAssert( false, "ExtractWeights() called on a span with no weights" ); break;
        case kSkin1Weight: numWeights = 1; break;
        case kSkin2Weights: numWeights = 2; break;
        case kSkin3Weights: numWeights = 3; break;
        default: hsAssert( false, "Bad number of weights in ExtractWeights()" );
    }

    memcpy( weightArray, fPtr, sizeof( float ) * numWeights );

    if( fFormat & kSkinIndices )
    {
        fPtr += numWeights;

        dPtr = (uint32_t *)fPtr;
        *indices = *dPtr;
    }
}

//// StuffVertex //////////////////////////////////////////////////////////////
//  Stuffs the given vertex data into the vertex buffer. Vertex must already
//  exist!

void    plGeometrySpan::StuffVertex( uint32_t index, hsPoint3 *pos, hsPoint3 *normal, hsColorRGBA *color, hsColorRGBA *specColor )
{
    uint8_t       *basePtr;
    float       *fPtr;


    /// Where?
    hsAssert( index < fNumVerts, "Invalid index passed to StuffVertex()" );
    basePtr = fVertexData + index * GetVertexSize( fFormat );

    /// Copy over point and normal
    fPtr = (float *)basePtr;
    fPtr[ 0 ] = pos->fX;
    fPtr[ 1 ] = pos->fY;
    fPtr[ 2 ] = pos->fZ;

    fPtr[ 3 ] = normal->fX;
    fPtr[ 4 ] = normal->fY;
    fPtr[ 5 ] = normal->fZ;
    fPtr += 6;

    /// Diffuse color
    StuffVertex( index, color, specColor );
}

void    plGeometrySpan::StuffVertex( uint32_t index, hsColorRGBA *color, hsColorRGBA *specColor )
{
    uint8_t       r, g, b, a;


    /// Where?
    hsAssert( index < fNumVerts, "Invalid index passed to StuffVertex()" );

    a = (uint8_t)( color->a >= 1 ? 255 : color->a <= 0 ? 0 : color->a * 255.0 );
    r = (uint8_t)( color->r >= 1 ? 255 : color->r <= 0 ? 0 : color->r * 255.0 );
    g = (uint8_t)( color->g >= 1 ? 255 : color->g <= 0 ? 0 : color->g * 255.0 );
    b = (uint8_t)( color->b >= 1 ? 255 : color->b <= 0 ? 0 : color->b * 255.0 );
    
    fDiffuseRGBA[ index ] = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );

    if( specColor != nil )
    {
        a = (uint8_t)( specColor->a >= 1 ? 255 : specColor->a <= 0 ? 0 : specColor->a * 255.0 );
        r = (uint8_t)( specColor->r >= 1 ? 255 : specColor->r <= 0 ? 0 : specColor->r * 255.0 );
        g = (uint8_t)( specColor->g >= 1 ? 255 : specColor->g <= 0 ? 0 : specColor->g * 255.0 );
        b = (uint8_t)( specColor->b >= 1 ? 255 : specColor->b <= 0 ? 0 : specColor->b * 255.0 );
        
        fSpecularRGBA[ index ] = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | ( b );
    }
}
