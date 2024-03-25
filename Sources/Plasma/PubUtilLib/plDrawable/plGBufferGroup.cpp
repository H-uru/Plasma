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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plGBufferGroup Class Functions                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  2.21.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include "HeadSpin.h"
#include "plGBufferGroup.h"
#include "hsStream.h"

#include "plSurface/hsGMaterial.h"
#include "plGeometrySpan.h"
#include "plPipeline.h"
#include "hsGDeviceRef.h"
#include "plProfile.h"

#include "plVertCoder.h"

plProfile_CreateMemCounter("Buf Group Vertices", "Memory", MemBufGrpVertex);
plProfile_CreateMemCounter("Buf Group Indices", "Memory", MemBufGrpIndex);
plProfile_CreateTimer("Refill Vertex", "Draw", DrawRefillVertex);
plProfile_CreateTimer("Refill Index", "Draw", DrawRefillIndex);

const uint32_t plGBufferGroup::kMaxNumVertsPerBuffer = 32000;
const uint32_t plGBufferGroup::kMaxNumIndicesPerBuffer = 32000;


//// plGBufferTriangle Read and Write /////////////////////////////////////////

void    plGBufferTriangle::Read( hsStream *s )
{
    fIndex1 = s->ReadLE16();
    fIndex2 = s->ReadLE16();
    fIndex3 = s->ReadLE16();
    fSpanIndex = s->ReadLE16();
    fCenter.Read( s );
}

void    plGBufferTriangle::Write( hsStream *s )
{
    s->WriteLE16( fIndex1 );
    s->WriteLE16( fIndex2 );
    s->WriteLE16( fIndex3 );
    s->WriteLE16( fSpanIndex );
    fCenter.Write( s );
}

//// plGBufferCell Read/Write /////////////////////////////////////////////////
    
void    plGBufferCell::Read( hsStream *s )
{
    fVtxStart = s->ReadLE32();
    fColorStart = s->ReadLE32();
    fLength = s->ReadLE32();
}

void plGBufferCell::Write(hsStream *s) const
{
    s->WriteLE32( fVtxStart );
    s->WriteLE32( fColorStart );
    s->WriteLE32( fLength );
}

//// Constructor //////////////////////////////////////////////////////////////

plGBufferGroup::plGBufferGroup(uint8_t format, bool vertsVolatile, bool idxVolatile, int LOD)
    : fNumVerts(), fNumIndices(), fNumSkinWeights(), fFormat(format),
      fVertsVolatile(vertsVolatile), fIdxVolatile(idxVolatile), fLOD(LOD)
{
    fStride = ICalcVertexSize(fLiteStride);
}

//// Destructor ///////////////////////////////////////////////////////////////

plGBufferGroup::~plGBufferGroup()
{
    CleanUp();

    for (auto i : fVertexBufferRefs) {
        hsRefCnt_SafeUnRef(i);
    }
    for (auto i : fIndexBufferRefs) {
        hsRefCnt_SafeUnRef(i);
    }
}

void plGBufferGroup::DirtyVertexBuffer(size_t i)
{
    if( (i < fVertexBufferRefs.size()) && fVertexBufferRefs[i] )
        fVertexBufferRefs[i]->SetDirty(true);
}

void plGBufferGroup::DirtyIndexBuffer(size_t i)
{
    if( (i < fIndexBufferRefs.size()) && fIndexBufferRefs[i] )
        fIndexBufferRefs[i]->SetDirty(true);
}

//// TidyUp ///////////////////////////////////////////////////////////////////

void    plGBufferGroup::TidyUp()
{
/*  if( fVertBuffStorage.GetCount() == 0 && fNumVerts > 0 )
        return;     // Already tidy'd!

//  IConvertToStorage();
*/
}

void plGBufferGroup::PurgeVertBuffer(uint32_t idx)
{
    if( AreVertsVolatile() )
        return;

//#define MF_TOSSER
#ifdef MF_TOSSER
    plProfile_DelMem(MemBufGrpVertex, fVertBuffSizes[idx]);
    delete [] fVertBuffStorage[idx];
    fVertBuffStorage[idx] = nullptr;

    plProfile_DelMem(MemBufGrpVertex, fColorBuffCounts[idx] * sizeof(plGBufferColor));
    delete [] fColorBuffStorage[idx];
    fColorBuffStorage[idx] = nullptr;
    
    delete fCells[idx];
    fCells[idx] = nullptr;

#endif // MF_TOSSER
    return;
}

void plGBufferGroup::PurgeIndexBuffer(uint32_t idx)
{
    if( AreIdxVolatile() )
        return;

    return;
}

//// CleanUp //////////////////////////////////////////////////////////////////

void    plGBufferGroup::CleanUp()
{
    // Clean up the storage
    for (size_t i = 0; i < fVertBuffSizes.size(); ++i)
    {
        plProfile_DelMem(MemBufGrpVertex, fVertBuffSizes[i]);
        delete [] fVertBuffStorage[ i ];
    }
    for (size_t i = 0; i < fIdxBuffStorage.size(); i++)
    {
        plProfile_DelMem(MemBufGrpIndex, fIdxBuffCounts[i] * sizeof(uint16_t));
        delete [] fIdxBuffStorage[ i ];
    }
    for (size_t i = 0; i < fColorBuffStorage.size(); ++i)
    {
        plProfile_DelMem(MemBufGrpVertex, fColorBuffCounts[i] * sizeof(plGBufferColor));
        delete [] fColorBuffStorage[ i ];
    }

    fVertBuffStorage.clear();
    fVertBuffSizes.clear();
    fVertBuffStarts.clear();
    fVertBuffEnds.clear();
    fIdxBuffStorage.clear();
    fIdxBuffCounts.clear();
    fIdxBuffStarts.clear();
    fIdxBuffEnds.clear();
    fColorBuffStorage.clear();
    fColorBuffCounts.clear();

    fCells.clear();
}

//// SetVertexBufferRef ///////////////////////////////////////////////////////

void    plGBufferGroup::SetVertexBufferRef( uint32_t index, hsGDeviceRef *vb )
{
    hsAssert( index < fVertexBufferRefs.size() + 1, "Vertex buffers must be assigned linearly!" );

    if (index > fVertexBufferRefs.size() - 1)
    {
        fVertexBufferRefs.push_back( vb );
        hsRefCnt_SafeRef( vb );
    }
    else
    {
        hsRefCnt_SafeAssign( fVertexBufferRefs[ index ], vb );
    }

}

//// SetIndexBufferRef ////////////////////////////////////////////////////////

void    plGBufferGroup::SetIndexBufferRef( uint32_t index, hsGDeviceRef *ib ) 
{
    hsAssert( index < fIndexBufferRefs.size() + 1, "Index buffers must be assigned linearly!" );

    if(index > fIndexBufferRefs.size() - 1)
    {
        fIndexBufferRefs.push_back( ib );
        hsRefCnt_SafeRef( ib );
    }
    else
    {
        hsRefCnt_SafeAssign( fIndexBufferRefs[ index ], ib );
    }

}

//// PrepForRendering /////////////////////////////////////////////////////////

void    plGBufferGroup::PrepForRendering( plPipeline *pipe, bool adjustForNvidiaLighting )
{
    ISendStorageToBuffers( pipe, adjustForNvidiaLighting );
    // The following line was taken out so we'd keep our data around, allowing
    // us to rebuild the buffer if necessary on the fly
//  CleanUp();

}

hsGDeviceRef* plGBufferGroup::GetVertexBufferRef(uint32_t i) 
{ 
    if( i >= fVertexBufferRefs.size() )
        fVertexBufferRefs.resize(i+1);

    return fVertexBufferRefs[i]; 
}

hsGDeviceRef* plGBufferGroup::GetIndexBufferRef(uint32_t i) 
{ 
    if( i >= fIndexBufferRefs.size() )
        fIndexBufferRefs.resize(i+1);

    return fIndexBufferRefs[i]; 
}

//// ISendStorageToBuffers ////////////////////////////////////////////////////

void    plGBufferGroup::ISendStorageToBuffers( plPipeline *pipe, bool adjustForNvidiaLighting )
{
    plProfile_BeginTiming(DrawRefillVertex);

    /// Creating or refreshing?
    for (size_t i = 0; i < fVertBuffStorage.size(); i++)
        pipe->CheckVertexBufferRef(this, i);
    plProfile_EndTiming(DrawRefillVertex);

    plProfile_BeginTiming(DrawRefillIndex);

    for (size_t i = 0; i < fIdxBuffStorage.size(); i++)
        pipe->CheckIndexBufferRef(this, i);

    plProfile_EndTiming(DrawRefillIndex);
}


//// ICalcVertexSize //////////////////////////////////////////////////////////

uint8_t   plGBufferGroup::ICalcVertexSize( uint8_t &liteStride )
{
    uint8_t   size;


    size = sizeof( float ) * ( 3 + 3 ); // pos + normal
    size += sizeof( float ) * 3 * GetNumUVs();

    switch( fFormat & kSkinWeightMask )
    {
        case kSkinNoWeights: fNumSkinWeights = 0; break;
        case kSkin1Weight:   fNumSkinWeights = 1; break;
        case kSkin2Weights:  fNumSkinWeights = 2; break;
        case kSkin3Weights:  fNumSkinWeights = 3; break;
        default: hsAssert( false, "Bad weight count in ICalcVertexSize()" );
    }
    if( fNumSkinWeights )
    {
        size += sizeof( float ) * fNumSkinWeights;
        if( fFormat & kSkinIndices )
            size += sizeof( uint32_t );
    }

    liteStride = size;
    size += sizeof( uint32_t ) * 2;            // diffuse + specular
    return size;
}

//// I/O Functions ////////////////////////////////////////////////////////////

void    plGBufferGroup::Read( hsStream *s ) 
{
    uint32_t          i, count, temp = 0, j;
    uint8_t           *vData;
    uint16_t          *iData;
    plGBufferColor  *cData;


    s->ReadByte(&fFormat);
    (void)s->ReadLE32();    // totalDynSize
    fStride = ICalcVertexSize( fLiteStride );

    fVertBuffSizes.clear();
    fVertBuffStarts.clear();
    fVertBuffEnds.clear();
    fColorBuffCounts.clear();
    fIdxBuffCounts.clear();
    fIdxBuffStarts.clear();
    fIdxBuffEnds.clear();
    fVertBuffStorage.clear();
    fIdxBuffStorage.clear();

    plVertCoder coder;

    /// Create buffers and read in as we go
    count = s->ReadLE32();
    fVertBuffSizes.reserve(count);
    fVertBuffStarts.reserve(count);
    fVertBuffEnds.reserve(count);
    fVertBuffStorage.reserve(count);
    fColorBuffCounts.reserve(count);
    fColorBuffStorage.reserve(count);
    for( i = 0; i < count; i++ )
    {
        if( fFormat & kEncoded )
        {
            const uint16_t numVerts = s->ReadLE16();
            const uint32_t size = numVerts * fStride;

            fVertBuffSizes.push_back(size);
            fVertBuffStarts.push_back(0);
            fVertBuffEnds.push_back(-1);

            vData = new uint8_t[size];
            fVertBuffStorage.push_back( vData );
            plProfile_NewMem(MemBufGrpVertex, size);

            coder.Read(s, vData, fFormat, fStride, numVerts);

            fColorBuffCounts.push_back(0);
            fColorBuffStorage.push_back(nullptr);

        }
        else
        {
            temp = s->ReadLE32();
    
            fVertBuffSizes.push_back( temp );
            fVertBuffStarts.push_back(0);
            fVertBuffEnds.push_back(-1);
            
            vData = new uint8_t[ temp ];
            hsAssert(vData, "Not enough memory to read in vertices");
            s->Read( temp, (void *)vData );
            fVertBuffStorage.push_back( vData );
            plProfile_NewMem(MemBufGrpVertex, temp);
            
            temp = s->ReadLE32();
            fColorBuffCounts.push_back( temp );
            
            if( temp > 0 )
            {
                cData = new plGBufferColor[ temp ];
                s->Read( temp * sizeof( plGBufferColor ), (void *)cData );
                plProfile_NewMem(MemBufGrpVertex, temp * sizeof(plGBufferColor));
            }
            else
                cData = nullptr;
            
            fColorBuffStorage.push_back( cData );
        }
    }

    count = s->ReadLE32();
    fIdxBuffCounts.reserve(count);
    fIdxBuffStarts.reserve(count);
    fIdxBuffEnds.reserve(count);
    fIdxBuffStorage.reserve(count);
    for( i = 0; i < count; i++ )
    {
        temp = s->ReadLE32();
        fIdxBuffCounts.push_back(temp);
        fIdxBuffStarts.push_back(0);
        fIdxBuffEnds.push_back(-1);

        iData = new uint16_t[ temp ];
        hsAssert(iData != nullptr, "Not enough memory to read in indices");
        s->ReadLE16( temp, (uint16_t *)iData );
        fIdxBuffStorage.push_back( iData );
        plProfile_NewMem(MemBufGrpIndex, temp * sizeof(uint16_t));
    }

    /// Read in cell arrays, one per vBuffer
    fCells.resize(fVertBuffStorage.size());
    for( i = 0; i < fVertBuffStorage.size(); i++ )
    {
        temp = s->ReadLE32();

        fCells[ i ].resize( temp );

        for( j = 0; j < temp; j++ )
            fCells[ i ][ j ].Read( s );
    }

}

//#define VERT_LOG

void    plGBufferGroup::Write( hsStream *s ) 
{
    uint32_t      totalDynSize;

#define MF_VERTCODE_ENABLED
#ifdef MF_VERTCODE_ENABLED
    fFormat |= kEncoded;
#endif // MF_VERTCODE_ENABLED

#ifdef VERT_LOG
    hsUNIXStream log;
    log.Open("log\\GBuf.log", "ab");
#endif

    /// Calc total dynamic data size, for fun
    totalDynSize = 0;
    for (auto it : fVertBuffSizes)
        totalDynSize += it;
    for (auto it : fIdxBuffCounts)
        totalDynSize += sizeof( uint16_t ) * it;

    s->WriteByte(fFormat);
    s->WriteLE32( totalDynSize );

    plVertCoder coder;

    /// Write out dyanmic data
    s->WriteLE32( (uint32_t)fVertBuffStorage.size() );
    for (size_t i = 0; i < fVertBuffStorage.size(); ++i)
    {
#ifdef MF_VERTCODE_ENABLED

        hsAssert(fCells[i].size() == 1, "Data must be interleaved for compression");
        uint32_t numVerts = fVertBuffSizes[i] / fStride;
        s->WriteLE16((uint16_t)numVerts);
        coder.Write(s, fVertBuffStorage[i], fFormat, fStride, (uint16_t)numVerts);

#ifdef VERT_LOG
        char buf[256];
        sprintf(buf, "Vert Buff: %u bytes, idx=%u\r\n", fVertBuffSizes[i], i);
        log.WriteString(buf);
        for (int xx = 0; xx < fVertBuffSizes[i] / 4; xx++)
        {
            float* buff32 = (float*)fVertBuffStorage[i];
            buff32 += xx;
            sprintf(buf, "[%d]%f\r\n", xx*4, *buff32);
            log.WriteString(buf);
        }
#endif

#else // MF_VERTCODE_ENABLED
        
        s->WriteLE32( fVertBuffSizes[ i ] );
        s->Write( fVertBuffSizes[ i ], (void *)fVertBuffStorage[ i ] );
        

        s->WriteLE32( fColorBuffCounts[ i ] );
        s->Write( fColorBuffCounts[ i ] * sizeof( plGBufferColor ), (void *)fColorBuffStorage[ i ] );

#endif // MF_VERTCODE_ENABLED
    }

    s->WriteLE32( (uint32_t)fIdxBuffCounts.size() );
    for (size_t i = 0; i < fIdxBuffStorage.size(); i++)
    {
        s->WriteLE32( fIdxBuffCounts[ i ] );
        s->WriteLE16( fIdxBuffCounts[ i ], fIdxBuffStorage[ i ] );
    }

    /// Write out cell arrays
    for (size_t i = 0; i < fVertBuffStorage.size(); i++)
    {
        s->WriteLE32((uint32_t)fCells[i].size());
        for (const plGBufferCell& cell : fCells[i])
            cell.Write(s);
    }
}

///////////////////////////////////////////////////////////////////////////////
//// Editing Functions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// DeleteVertsFromStorage ///////////////////////////////////////////////////
//  Deletes a span of verts from the vertex storage. Remember to Prep this
//  group after doing this!
//  Note: does NOT adjust index storage, since we don't know inside here
//  which indices to adjust. Have to call that separately.
//  Note 2: for simplicity sake, we only do this for groups with ONE interleaved
//  cell. Doing this for multiple separated cells would be, literally, hell.

void    plGBufferGroup::DeleteVertsFromStorage( uint32_t which, uint32_t start, uint32_t length )
{
    uint8_t       *dstPtr, *srcPtr;
    uint32_t      amount;


    hsAssert( fCells[ which ].size() == 1, "Cannot delete verts on a mixed buffer group" );

    // Adjust cell 0
    fCells[ which ][ 0 ].fLength -= length;

    start *= fStride;
    length *= fStride;

    if( start + length < fVertBuffSizes[ which ] )
    {
        dstPtr = &( fVertBuffStorage[ which ][ start ] );
        srcPtr = &( fVertBuffStorage[ which ][ start + length ] );

        amount = ( fVertBuffSizes[ which ] ) - ( start + length );

        memmove( dstPtr, srcPtr, amount );
    }

    fVertBuffSizes[ which ] -= length;
    plProfile_DelMem(MemBufGrpVertex, length);

    if (fVertexBufferRefs.size() > which && fVertexBufferRefs[which])
    {
        hsRefCnt_SafeUnRef(fVertexBufferRefs[which]);
        fVertexBufferRefs[which] = nullptr;
    }

}

//// AdjustIndicesInStorage ///////////////////////////////////////////////////
//  Adjusts indices >= a given threshold by a given delta. Use it to adjust
//  indices after vertex deletion. Affects only the given buffer.

void    plGBufferGroup::AdjustIndicesInStorage( uint32_t which, uint16_t threshhold, int16_t delta )
{
    int         i;


    for( i = 0; i < fIdxBuffCounts[ which ]; i++ )
    {
        if( fIdxBuffStorage[ which ][ i ] >= threshhold )
            fIdxBuffStorage[ which ][ i ] += delta;
    }

    if (fIndexBufferRefs.size() > which && fIndexBufferRefs[which])
        fIndexBufferRefs[which]->SetDirty( true );

}

//// DeleteIndicesFromStorage /////////////////////////////////////////////////
//  Deletes a span of indices from the index storage. Remember to Prep this
//  group after doing this!

void    plGBufferGroup::DeleteIndicesFromStorage( uint32_t which, uint32_t start, uint32_t length )
{
    uint16_t      *dstPtr, *srcPtr;
    uint32_t      amount;


    hsAssert( start + length <= fIdxBuffCounts[ which ], "Illegal range to DeleteIndicesFromStorage()" );

    if( start + length < fIdxBuffCounts[ which ] )
    {
        dstPtr = &( fIdxBuffStorage[ which ][ start ] );
        srcPtr = &( fIdxBuffStorage[ which ][ start + length ] );

        amount = fIdxBuffCounts[ which ] - ( start + length );

        memmove( dstPtr, srcPtr, amount * sizeof( uint16_t ) );
    }

    fIdxBuffCounts[ which ] -= length;
    plProfile_DelMem(MemBufGrpIndex, length * sizeof(uint16_t));

    if (fIndexBufferRefs.size() > which && fIndexBufferRefs[which])
    {
        hsRefCnt_SafeUnRef(fIndexBufferRefs[which]);
        fIndexBufferRefs[which] = nullptr;
    }

}

//// GetNumPrimaryVertsLeft ///////////////////////////////////////////////////
//  Base on the cells, so we can take instanced cells into account

uint32_t  plGBufferGroup::GetNumPrimaryVertsLeft() const
{
    return GetNumVertsLeft( 0 );
}

//// GetNumVertsLeft //////////////////////////////////////////////////////////
//  Base on the cells, so we can take instanced cells into account

uint32_t  plGBufferGroup::GetNumVertsLeft( uint32_t idx ) const
{
    if( idx >= fCells.size() )
        return kMaxNumVertsPerBuffer;

    uint32_t total = kMaxNumVertsPerBuffer;
    for( const auto& i : fCells[ idx ] )
        total -= i.fLength;

    return total;
}

//// IMakeCell ////////////////////////////////////////////////////////////////
//  Get a cell from the given cell array.

uint32_t  plGBufferGroup::IMakeCell( uint32_t vbIndex, uint8_t flags, uint32_t vStart, uint32_t cStart, uint32_t len, uint32_t *offset )
{
    std::vector<plGBufferCell>& cells = fCells[ vbIndex ];


    if( !(flags & kReserveInterleaved) )
    {
        /// Note that there are THREE types of cells: interleaved (colorStart == -1),
        /// first of an instance group (colorStart != -1 && vStart != -1) and 
        /// an instance (colorStart != -1 && vStart == -1 ). To simplify things,
        /// we never merge any separated cells

        if( flags & kReserveSeparated )
            cells.emplace_back(vStart, cStart, len);
        else
            cells.emplace_back((uint32_t)-1, cStart, len);
        *offset = 0;
    }
    else
    {
        /// Merge if the last cell was an interleaved cell
        if( !cells.empty() && cells.back().fColorStart == (uint32_t)-1 )
        {
            *offset = cells.back().fLength;
            cells.back().fLength += len;
        }
        else
        {
            cells.emplace_back(vStart, (uint32_t)-1, len);
            *offset = 0;
        }
    }

    return cells.size() - 1;
}

//// ReserveVertStorage ///////////////////////////////////////////////////////
//  Takes a length to reserve in a vertex buffer and returns the buffer index
//  and starting position. Basically does what AppendToVertStorage() used to
//  do except it doesn't actually copy any data into the space reserved.

bool    plGBufferGroup::ReserveVertStorage( uint32_t numVerts, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset, uint8_t flags )
{
    uint8_t                   *storagePtr = nullptr;
    uint32_t                  cStartIdx = 0, vStartIdx = 0;
    plGBufferColor          *cStoragePtr = nullptr;
    int                     i;


    if( numVerts >= kMaxNumVertsPerBuffer )
    {
        hsAssert( false, "Egad, why on earth are you adding that many verts???" );
        return false;
    }

    /// Find a spot
    if( !(flags & kReserveIsolate) )
    {
        for( i = 0; i < fVertBuffStorage.size(); i++ )
        {
            if( GetNumVertsLeft( i ) >= numVerts )
                break;
        }
    }
    else
    {
        i = fVertBuffStorage.size();
    }
    if( i == fVertBuffStorage.size() )
    {
        if( (flags & kReserveInterleaved) || (flags & kReserveSeparated) )
        {
            fVertBuffStorage.push_back(nullptr);
            fVertBuffSizes.push_back(0);
        }
        fVertBuffStarts.push_back(0);
        fVertBuffEnds.push_back(-1);

        fColorBuffStorage.push_back(nullptr);
        fColorBuffCounts.push_back(0);

        fCells.emplace_back();
    }

    *vbIndex = i;

    if( !(flags & kReserveInterleaved) )
    {
        // Splitting the data into vertex and color storage
        if( flags & kReserveSeparated )
        {
            /// Increase the storage size
            vStartIdx = fVertBuffSizes[ i ];
            storagePtr = new uint8_t[ fVertBuffSizes[ i ] + numVerts * fLiteStride ];
            if( fVertBuffSizes[ i ] > 0 )
                memcpy( storagePtr, fVertBuffStorage[ i ], fVertBuffSizes[ i ] );
            fVertBuffSizes[ i ] += numVerts * fLiteStride;
            plProfile_NewMem(MemBufGrpVertex, numVerts * fLiteStride);
        }

        /// Color too
        cStartIdx = fColorBuffCounts[ i ];
        cStoragePtr = new plGBufferColor[ fColorBuffCounts[ i ] + numVerts ];
        if( fColorBuffCounts[ i ] > 0 )
            memcpy( cStoragePtr, fColorBuffStorage[ i ], fColorBuffCounts[ i ] * sizeof( plGBufferColor ) );
    }
    else
    {
        // Interleaved

        /// Increase the storage size
        vStartIdx = fVertBuffSizes[ i ];
        storagePtr = new uint8_t[ fVertBuffSizes[ i ] + numVerts * fStride ];
        if( fVertBuffSizes[ i ] > 0 )
            memcpy( storagePtr, fVertBuffStorage[ i ], fVertBuffSizes[ i ] );
        fVertBuffSizes[ i ] += numVerts * fStride;
        plProfile_NewMem(MemBufGrpVertex, numVerts * fStride);
    }

    /// Switch over
    if (storagePtr)
    {
        if (fVertBuffStorage[i] != nullptr)
            delete [] fVertBuffStorage[ i ];
        fVertBuffStorage[ i ] = storagePtr;
    }
    if (cStoragePtr)
    {
        if (fColorBuffStorage[ i ])
            delete [] fColorBuffStorage[ i ];
        fColorBuffStorage[ i ] = cStoragePtr;
        fColorBuffCounts[ i ] += numVerts;
        plProfile_NewMem(MemBufGrpVertex, numVerts * sizeof(plGBufferColor));
    }

    if (fVertexBufferRefs.size() > i && fVertexBufferRefs[i])
    {
        hsRefCnt_SafeUnRef(fVertexBufferRefs[i]);
        fVertexBufferRefs[i] = nullptr;
    }

    /// Append a cell entry
    *cell = IMakeCell( i, flags, vStartIdx, cStartIdx, numVerts, offset );

    /// All done!
    return true;
}

//// AppendToVertStorage //////////////////////////////////////////////////////
//  Given an opaque array of vertex data, puts it into the first available spot 
//  in fVertStorage. If there is no array on fVertStorage that can hold them,
//  we create a new one. Returns the index of the storage array and the
//  starting index into the array. Note that we basically stick it wherever
//  we can, instead of at the end.
//  Updated 4.30.2001 mcn to take in a plGeometrySpan as a source rather than
//  raw data, since the plGeometrySpan no longer stores data in exactly the
//  same format.
//  Updated 6.15.2001 mcn to use ReserveVertStorage().

void    plGBufferGroup::AppendToVertStorage( plGeometrySpan *srcSpan, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset )
{
    if( !ReserveVertStorage( srcSpan->fNumVerts, vbIndex, cell, offset, kReserveInterleaved ) )
        return;

    StuffToVertStorage( srcSpan, *vbIndex, *cell, *offset, kReserveInterleaved );
}

//// AppendToVertAndColorStorage //////////////////////////////////////////////
//  Same as AppendToVertStorage(), but writes only the verts to the vertex
//  storage and the colors to the separate color storage.

void    plGBufferGroup::AppendToVertAndColorStorage( plGeometrySpan *srcSpan, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset )
{
    if( !ReserveVertStorage( srcSpan->fNumVerts, vbIndex, cell, offset, kReserveSeparated ) )
        return;

    StuffToVertStorage( srcSpan, *vbIndex, *cell, *offset, kReserveSeparated );
}

//// AppendToColorStorage /////////////////////////////////////////////////////
//  Same as AppendToVertStorage(), but writes JUST to color storage.

void    plGBufferGroup::AppendToColorStorage( plGeometrySpan *srcSpan, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset, uint32_t origCell )
{
    if( !ReserveVertStorage( srcSpan->fNumVerts, vbIndex, cell, offset, kReserveColors ) )
        return;

    fCells[ *vbIndex ][ *cell ].fVtxStart = fCells[ *vbIndex ][ origCell ].fVtxStart;

    StuffToVertStorage( srcSpan, *vbIndex, *cell, *offset, kReserveColors );
}

//// IGetStartVtxPointer //////////////////////////////////////////////////////
//  Get the start vertex and color buffer pointers for a given cell and offset

void    plGBufferGroup::IGetStartVtxPointer( uint32_t vbIndex, uint32_t cell, uint32_t offset, uint8_t *&tempPtr, plGBufferColor *&cPtr )
{
    hsAssert( vbIndex < fVertBuffStorage.size(), "Invalid vbIndex in StuffToVertStorage()" );
    hsAssert( cell < fCells[ vbIndex ].size(), "Invalid cell in StuffToVertStorage()" );

    tempPtr = fVertBuffStorage[ vbIndex ];
    cPtr = fColorBuffStorage[ vbIndex ];

    tempPtr += fCells[ vbIndex ][ cell ].fVtxStart;
    cPtr += fCells[ vbIndex ][ cell ].fColorStart;

    if( offset > 0 )
    {
        tempPtr += offset * ( ( fCells[ vbIndex ][ cell ].fColorStart == (uint32_t)-1 ) ? fStride : fLiteStride );
        cPtr += offset;
    }
}

//// GetVertBufferCount ///////////////////////////////////////////////////////

uint32_t  plGBufferGroup::GetVertBufferCount( uint32_t idx ) const
{
    return GetVertStartFromCell( idx, fCells[ idx ].size(), 0 );
}

//// GetVertStartFromCell /////////////////////////////////////////////////////

uint32_t  plGBufferGroup::GetVertStartFromCell( uint32_t vbIndex, uint32_t cell, uint32_t offset ) const
{
    uint32_t  i, numVerts;


    hsAssert( vbIndex < fVertBuffStorage.size(), "Invalid vbIndex in StuffToVertStorage()" );
    hsAssert( cell <= fCells[ vbIndex ].size(), "Invalid cell in StuffToVertStorage()" );

    numVerts = 0;
    for( i = 0; i < cell; i++ )
        numVerts += fCells[ vbIndex ][ i ].fLength;

    numVerts += offset;

    return numVerts;
}

//// StuffToVertStorage ///////////////////////////////////////////////////////
//  Stuffs data from a plGeometrySpan into the specified location in the 
//  specified vertex buffer.

void    plGBufferGroup::StuffToVertStorage( plGeometrySpan *srcSpan, uint32_t vbIndex, uint32_t cell, uint32_t offset, uint8_t flags )
{
    uint8_t           *tempPtr, stride;
    plGBufferColor  *cPtr;
    int             i, j, numVerts;
    plGBufferCell   *cellPtr;


    hsAssert( vbIndex < fVertBuffStorage.size(), "Invalid vbIndex in StuffToVertStorage()" );
    hsAssert( cell < fCells[ vbIndex ].size(), "Invalid cell in StuffToVertStorage()" );

    IGetStartVtxPointer( vbIndex, cell, offset, tempPtr, cPtr );
    cellPtr = &fCells[ vbIndex ][ cell ];
    stride = ( cellPtr->fColorStart != (uint32_t)-1 ) ? fLiteStride : fStride;

    numVerts = srcSpan->fNumVerts;

    /// Copy the data over
    for( i = 0; i < numVerts; i++ )
    {
        hsPoint3    pos;
        float       weights[ 3 ];
        uint32_t      weightIndices;
        hsVector3   norm;
        uint32_t      color, specColor;
        hsPoint3    uvs[ plGeometrySpan::kMaxNumUVChannels ];
        float       *fPtr;
        uint32_t      *dPtr;


        // Gotta swap the data around, since plGeometrySpans store the data slightly differently
        if( flags & kReserveColors )
        {
            /// Just do colors
            srcSpan->ExtractVertex( i, &pos, &norm, &color, &specColor );

            cPtr->fDiffuse = color;
            cPtr->fSpecular = specColor;
        }
        else
        {
            /// Do verts, possibly colors as well
            srcSpan->ExtractVertex( i, &pos, &norm, &color, &specColor );
            if( ( fFormat & kSkinWeightMask ) != kSkinNoWeights )
                srcSpan->ExtractWeights( i, weights, &weightIndices );
            for( j = 0; j < GetNumUVs(); j++ )
                srcSpan->ExtractUv( i, j, &uvs[ j ] );

            // Stuff it in now
            fPtr = (float *)tempPtr;
            fPtr[ 0 ] = pos.fX;
            fPtr[ 1 ] = pos.fY;
            fPtr[ 2 ] = pos.fZ;
            fPtr += 3;

            if( fNumSkinWeights > 0 )
            {
                for( j = 0; j < fNumSkinWeights; j++ )
                {
                    *fPtr = weights[ j ];
                    fPtr++;
                }

                if( fNumSkinWeights > 1 )
                {
                    dPtr = (uint32_t *)fPtr;
                    *dPtr = weightIndices;

                    dPtr++;
                    fPtr = (float *)dPtr;
                }
            }
            
            fPtr[ 0 ] = norm.fX;
            fPtr[ 1 ] = norm.fY;
            fPtr[ 2 ] = norm.fZ;
            fPtr += 3;

            if( flags & kReserveInterleaved )
            {
                dPtr = (uint32_t *)fPtr;
                dPtr[ 0 ] = color;
                dPtr[ 1 ] = specColor;
                dPtr += 2;
                fPtr = (float *)dPtr;
            }
            else 
            {
                cPtr->fDiffuse = color;
                cPtr->fSpecular = specColor;
            }

            for( j = 0; j < GetNumUVs(); j++ )
            {
                fPtr[ 0 ] = uvs[ j ].fX;
                fPtr[ 1 ] = uvs[ j ].fY;
                fPtr[ 2 ] = uvs[ j ].fZ;
                fPtr += 3;
            }
        }

        tempPtr += stride;
        cPtr++;
    }

    if( ( vbIndex < fVertexBufferRefs.size() ) && fVertexBufferRefs[ vbIndex ] )
        fVertexBufferRefs[ vbIndex ]->SetDirty( true );
}


//// ReserveIndexStorage //////////////////////////////////////////////////////
//  Same as ReserveVertStorage(), only for indices :)

bool    plGBufferGroup::ReserveIndexStorage( uint32_t numIndices, uint32_t *ibIndex, uint32_t *ibStart, uint16_t **dataPtr )
{
    uint16_t          *storagePtr;
    int             i;


    if( numIndices >= kMaxNumIndicesPerBuffer )
    {
        hsAssert( false, "Egad, why on earth are you adding that many indices???" );
        return false;
    }

    /// Find a spot
    for( i = 0; i < fIdxBuffStorage.size(); i++ )
    {

        if( fIdxBuffCounts[ i ] + numIndices < kMaxNumIndicesPerBuffer )
            break;
    }
    if( i == fIdxBuffStorage.size() )
    {
        fIdxBuffStorage.push_back(nullptr);
        fIdxBuffCounts.push_back(0);

        fIdxBuffStarts.push_back(0);
        fIdxBuffEnds.push_back(-1);
    }

    *ibIndex = i;
    *ibStart = fIdxBuffCounts[ i ];

    /// Increase the storage size
    storagePtr = new uint16_t[ fIdxBuffCounts[ i ] + numIndices ];
    if( fIdxBuffCounts[ i ] > 0 )
        memcpy( storagePtr, fIdxBuffStorage[ i ], fIdxBuffCounts[ i ] * sizeof( uint16_t ) );

    if (dataPtr != nullptr)
        *dataPtr = storagePtr + fIdxBuffCounts[ i ];

    /// Switch over
    i = *ibIndex;
    if (fIdxBuffStorage[i] != nullptr)
        delete [] fIdxBuffStorage[ i ];
    fIdxBuffStorage[ i ] = storagePtr;
    fIdxBuffCounts[ i ] += numIndices;
    plProfile_NewMem(MemBufGrpIndex, numIndices * sizeof(uint16_t));

    /// All done!
    if ( fIndexBufferRefs.size() > i && fIndexBufferRefs[i])
    {
        hsRefCnt_SafeUnRef(fIndexBufferRefs[i]);
        fIndexBufferRefs[i] = nullptr;
    }

    return true;
}

//// AppendToIndexStorage /////////////////////////////////////////////////////
//  Same as AppendToVertStorage, only for the index buffers and indices. Duh :)

void    plGBufferGroup::AppendToIndexStorage( uint32_t numIndices, uint16_t *data, uint32_t addToAll, 
                                              uint32_t *ibIndex, uint32_t *ibStart )
{
    uint16_t          *tempPtr;
    int             i;


    if( !ReserveIndexStorage( numIndices, ibIndex, ibStart, &tempPtr ) )
        return;

    /// Copy new indices over, offsetting them as we were told to
    for( i = 0; i < numIndices; i++ )
        tempPtr[ i ] = data[ i ] + (uint16_t)addToAll;

    /// All done!
}

//// ConvertToTriList /////////////////////////////////////////////////////////
//  Returns an array of plGBufferTriangles representing the span of indices 
//  specified. 

plGBufferTriangle   *plGBufferGroup::ConvertToTriList( int16_t spanIndex, uint32_t whichIdx, uint32_t whichVtx, uint32_t whichCell, uint32_t start, uint32_t numTriangles )
{
    plGBufferTriangle   *array;
    uint16_t              *storagePtr;
    uint8_t               *vertStgPtr, stride;
    float               *vertPtr;
    int                 i, j;
    hsPoint3            center;
    uint32_t              offsetBy;
    plGBufferColor      *wastePtr;


    /// Sanity checks
    hsAssert( whichIdx < fIdxBuffStorage.size(), "Invalid index buffer ID to ConvertToTriList()" );
    hsAssert( whichVtx < fVertBuffStorage.size(), "Invalid vertex buffer ID to ConvertToTriList()" );
    hsAssert( start < fIdxBuffCounts[ whichIdx ], "Invalid start index to ConvertToTriList()" );
    hsAssert( start + numTriangles * 3 <= fIdxBuffCounts[ whichIdx ], "Invalid count to ConvertToTriList()" );
    hsAssert( whichCell < fCells[ whichVtx ].size(), "Invalid cell to ConvertToTriList()" );

    /// Create the array and fill it
    array = new plGBufferTriangle[ numTriangles ];
    hsAssert(array != nullptr, "Not enough memory to create triangle data in ConvertToTriList()");

    storagePtr = fIdxBuffStorage[ whichIdx ];
    IGetStartVtxPointer( whichVtx, whichCell, 0, vertStgPtr, wastePtr );
    offsetBy = GetVertStartFromCell( whichVtx, whichCell, 0 );
    stride = ( fCells[ whichVtx ][ whichCell ].fColorStart == (uint32_t)-1 ) ? fStride : fLiteStride;
    
    for( i = 0, j = 0; i < numTriangles; i++, j += 3 )
    {
        center.fX = center.fY = center.fZ = 0;

        vertPtr = (float *)( vertStgPtr + stride * ( storagePtr[ start + j + 0 ] - offsetBy ) );

        center.fX += vertPtr[ 0 ];
        center.fY += vertPtr[ 1 ];
        center.fZ += vertPtr[ 2 ];

        vertPtr = (float *)( vertStgPtr + stride * ( storagePtr[ start + j + 1 ] - offsetBy ) );

        center.fX += vertPtr[ 0 ];
        center.fY += vertPtr[ 1 ];
        center.fZ += vertPtr[ 2 ];

        vertPtr = (float *)( vertStgPtr + stride * ( storagePtr[ start + j + 2 ] - offsetBy ) );

        center.fX += vertPtr[ 0 ];
        center.fY += vertPtr[ 1 ];
        center.fZ += vertPtr[ 2 ];
        
        center.fX /= 3.0f;
        center.fY /= 3.0f;
        center.fZ /= 3.0f;

        array[ i ].fSpanIndex = spanIndex;
        array[ i ].fIndex1 = storagePtr[ start + j + 0 ];
        array[ i ].fIndex2 = storagePtr[ start + j + 1 ];
        array[ i ].fIndex3 = storagePtr[ start + j + 2 ];
        array[ i ].fCenter = center;
    }

    /// All done!
    return array;
}

//// StuffFromTriList /////////////////////////////////////////////////////////
//  Stuffs the indices from an array of plGBufferTriangles into the index 
//  storage.

void    plGBufferGroup::StuffFromTriList( uint32_t which, uint32_t start, uint32_t numTriangles, uint16_t *data )
{
    uint16_t          *storagePtr;


    /// Sanity checks
    hsAssert( which < fIdxBuffStorage.size(), "Invalid index buffer ID to StuffFromTriList()" );
    hsAssert( start < fIdxBuffCounts[ which ], "Invalid start index to StuffFromTriList()" );
    hsAssert( start + numTriangles * 3 <= fIdxBuffCounts[ which ], "Invalid count to StuffFromTriList()" );


    /// This is easy--just stuff!
    storagePtr = fIdxBuffStorage[ which ];
#define MF_SPEED_THIS_UP
#ifndef MF_SPEED_THIS_UP
    int             i, j;
    for( i = 0, j = 0; i < numTriangles; i++, j += 3 )
    {
        storagePtr[ start + j     ] = data[ i ].fIndex1;
        storagePtr[ start + j + 1 ] = data[ i ].fIndex2;
        storagePtr[ start + j + 2 ] = data[ i ].fIndex3;
    }
#else // MF_SPEED_THIS_UP
    memcpy( storagePtr + start, data, numTriangles * 3 * sizeof(*data) );
#endif // MF_SPEED_THIS_UP

    /// All done! Just make sure we refresh before we render...
    if (fIndexBufferRefs.size() > which && fIndexBufferRefs[which])
        fIndexBufferRefs[which]->SetDirty(true);

}

//// StuffTri /////////////////////////////////////////////////////////////////

void    plGBufferGroup::StuffTri( uint32_t iBuff, uint32_t iTri, uint16_t idx0, uint16_t idx1, uint16_t idx2 )
{
    /// Sanity checks
    hsAssert( iBuff < fIdxBuffStorage.size(), "Invalid index buffer ID to StuffFromTriList()" );
    hsAssert( iTri < fIdxBuffCounts[ iBuff ], "Invalid start index to StuffFromTriList()" );

    fIdxBuffStorage[ iBuff ][ iTri + 0 ] = idx0;
    fIdxBuffStorage[ iBuff ][ iTri + 1 ] = idx1;
    fIdxBuffStorage[ iBuff ][ iTri + 2 ] = idx2;

}

//// Accessor Functions ///////////////////////////////////////////////////////

hsPoint3    &plGBufferGroup::Position( int iBuff, uint32_t cell, int iVtx )
{
    uint8_t           *vertStgPtr;
    plGBufferColor  *cPtr;

    IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

    return *(hsPoint3 *)( vertStgPtr + 0 ); 
}

hsVector3   &plGBufferGroup::Normal( int iBuff, uint32_t cell, int iVtx )
{
    uint8_t           *vertStgPtr;
    plGBufferColor  *cPtr;

    IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

    return *(hsVector3 *)( vertStgPtr + sizeof( hsPoint3 ) ); 
}

uint32_t  &plGBufferGroup::Color( int iBuff, uint32_t cell, int iVtx ) 
{
    uint8_t           *vertStgPtr;
    plGBufferColor  *cPtr;

    IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

    if( fCells[ iBuff ][ cell ].fColorStart != (uint32_t)-1 )
        return *(uint32_t *)( &cPtr->fDiffuse );
    else
        return *(uint32_t *)( vertStgPtr + 2 * sizeof( hsPoint3 ) );
}

uint32_t  &plGBufferGroup::Specular( int iBuff, uint32_t cell, int iVtx ) 
{
    uint8_t           *vertStgPtr;
    plGBufferColor  *cPtr;

    IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

    if( fCells[ iBuff ][ cell ].fColorStart != (uint32_t)-1 )
        return *(uint32_t *)( &cPtr->fSpecular );
    else
        return *(uint32_t *)( vertStgPtr + 2 * sizeof( hsPoint3 ) );
}

hsPoint3    &plGBufferGroup::UV( int iBuff, uint32_t cell, int iVtx, int channel )
{
    uint8_t           *vertStgPtr;
    plGBufferColor  *cPtr;

    IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

    vertStgPtr += 2 * sizeof( hsPoint3 ) + channel * sizeof( hsPoint3 );

    if( fCells[ iBuff ][ cell ].fColorStart != (uint32_t)-1 )
        return *(hsPoint3 *)( vertStgPtr  );
    else
        return *(hsPoint3 *)( vertStgPtr + 2 * sizeof( uint32_t ) );
}
