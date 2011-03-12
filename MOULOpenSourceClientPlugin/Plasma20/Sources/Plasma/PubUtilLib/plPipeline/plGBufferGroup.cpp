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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plGBufferGroup Class Functions											 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	2.21.2001 mcn - Created.  												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsWindows.h"
#include "HeadSpin.h"
#include "plGBufferGroup.h"
#include "hsStream.h"

#include "../plSurface/hsGMaterial.h"
#include "../plDrawable/plGeometrySpan.h"
#include "plPipeline.h"
#include "hsGDeviceRef.h"
#include "plProfile.h"

#include "plVertCoder.h"

plProfile_CreateMemCounter("Buf Group Vertices", "Memory", MemBufGrpVertex);
plProfile_CreateMemCounter("Buf Group Indices", "Memory", MemBufGrpIndex);
plProfile_CreateTimer("Refill Vertex", "Draw", DrawRefillVertex);
plProfile_CreateTimer("Refill Index", "Draw", DrawRefillIndex);

const UInt32 plGBufferGroup::kMaxNumVertsPerBuffer = 32000;
const UInt32 plGBufferGroup::kMaxNumIndicesPerBuffer = 32000;


//// plGBufferTriangle Read and Write /////////////////////////////////////////

void	plGBufferTriangle::Read( hsStream *s )
{
	fIndex1 = s->ReadSwap16();
	fIndex2 = s->ReadSwap16();
	fIndex3 = s->ReadSwap16();
	fSpanIndex = s->ReadSwap16();
	fCenter.Read( s );
}

void	plGBufferTriangle::Write( hsStream *s )
{
	s->WriteSwap16( fIndex1 );
	s->WriteSwap16( fIndex2 );
	s->WriteSwap16( fIndex3 );
	s->WriteSwap16( fSpanIndex );
	fCenter.Write( s );
}

//// plGBufferCell Read/Write /////////////////////////////////////////////////
	
void	plGBufferCell::Read( hsStream *s )
{
	fVtxStart = s->ReadSwap32();
	fColorStart = s->ReadSwap32();
	fLength = s->ReadSwap32();
}

void	plGBufferCell::Write( hsStream *s )
{
	s->WriteSwap32( fVtxStart );
	s->WriteSwap32( fColorStart );
	s->WriteSwap32( fLength );
}

//// Constructor //////////////////////////////////////////////////////////////

plGBufferGroup::plGBufferGroup( UInt8 format, hsBool vertsVolatile, hsBool idxVolatile, int LOD ) 
{
	fVertBuffStorage.Reset();
	fIdxBuffStorage.Reset();
	fColorBuffStorage.Reset();
	fVertexBufferRefs.Reset();
	fIndexBufferRefs.Reset();
	fCells.Reset();
	fNumVerts = fNumIndices = 0;

	fFormat = format;
	fStride = ICalcVertexSize( fLiteStride );
	fVertsVolatile = vertsVolatile;
	fIdxVolatile = idxVolatile;
	fLOD = LOD;
}

//// Destructor ///////////////////////////////////////////////////////////////

plGBufferGroup::~plGBufferGroup()
{
	UInt32	 i;

	CleanUp();

	for( i = 0; i < fVertexBufferRefs.GetCount(); i++ )
		hsRefCnt_SafeUnRef( fVertexBufferRefs[ i ] );

	for( i = 0; i < fIndexBufferRefs.GetCount(); i++ )
		hsRefCnt_SafeUnRef( fIndexBufferRefs[ i ] );

	fVertexBufferRefs.Reset();
	fIndexBufferRefs.Reset();
}

void plGBufferGroup::DirtyVertexBuffer(int i)
{
	if( (i < fVertexBufferRefs.GetCount()) && fVertexBufferRefs[i] )
		fVertexBufferRefs[i]->SetDirty(true);
}

void plGBufferGroup::DirtyIndexBuffer(int i)
{
	if( (i < fIndexBufferRefs.GetCount()) && fIndexBufferRefs[i] )
		fIndexBufferRefs[i]->SetDirty(true);
}

//// TidyUp ///////////////////////////////////////////////////////////////////

void	plGBufferGroup::TidyUp( void )
{
/*	if( fVertBuffStorage.GetCount() == 0 && fNumVerts > 0 )
		return;		// Already tidy'd!

//	IConvertToStorage();
*/
}

void plGBufferGroup::PurgeVertBuffer(UInt32 idx)
{
	if( AreVertsVolatile() )
		return;

//#define MF_TOSSER
#ifdef MF_TOSSER
	plProfile_DelMem(MemBufGrpVertex, fVertBuffSizes[idx]);
	delete [] fVertBuffStorage[idx];
	fVertBuffStorage[idx] = nil;

	plProfile_DelMem(MemBufGrpVertex, fColorBuffCounts[idx] * sizeof(plGBufferColor));
	delete [] fColorBuffStorage[idx];
	fColorBuffStorage[idx] = nil;
	
	delete fCells[idx];
	fCells[idx] = nil;

#endif // MF_TOSSER
	return;
}

void plGBufferGroup::PurgeIndexBuffer(UInt32 idx)
{
	if( AreIdxVolatile() )
		return;

	return;
}

//// CleanUp //////////////////////////////////////////////////////////////////

void	plGBufferGroup::CleanUp( void )
{
	int		i;

	
	// Clean up the storage
	for( i = 0; i < fVertBuffStorage.GetCount(); i++ )
	{
		plProfile_DelMem(MemBufGrpVertex, fVertBuffSizes[i]);
		delete [] fVertBuffStorage[ i ];
	}
	for( i = 0; i < fIdxBuffStorage.GetCount(); i++ )
	{
		plProfile_DelMem(MemBufGrpIndex, fIdxBuffCounts[i] * sizeof(UInt16));
		delete [] fIdxBuffStorage[ i ];
	}
	for( i = 0; i < fColorBuffStorage.GetCount(); i++ )
	{
		plProfile_DelMem(MemBufGrpVertex, fColorBuffCounts[i] * sizeof(plGBufferColor));
		delete [] fColorBuffStorage[ i ];
	}

	for( i = 0; i < fCells.GetCount(); i++ )
		delete fCells[ i ];

	fVertBuffStorage.Reset();
	fVertBuffSizes.Reset();
	fVertBuffStarts.Reset();
	fVertBuffEnds.Reset();
	fIdxBuffStorage.Reset();
	fIdxBuffCounts.Reset();
	fIdxBuffStarts.Reset();
	fIdxBuffEnds.Reset();
	fColorBuffStorage.Reset();
	fColorBuffCounts.Reset();

	fCells.Reset();
}

//// SetVertexBufferRef ///////////////////////////////////////////////////////

void	plGBufferGroup::SetVertexBufferRef( UInt32 index, hsGDeviceRef *vb )
{
	hsAssert( index < fVertexBufferRefs.GetCount() + 1, "Vertex buffers must be assigned linearly!" );

	if( (int)index > (int)fVertexBufferRefs.GetCount() - 1 )
	{
		fVertexBufferRefs.Append( vb );
		hsRefCnt_SafeRef( vb );
	}
	else
	{
		hsRefCnt_SafeAssign( fVertexBufferRefs[ index ], vb );
	}

}

//// SetIndexBufferRef ////////////////////////////////////////////////////////

void	plGBufferGroup::SetIndexBufferRef( UInt32 index, hsGDeviceRef *ib )	
{
	hsAssert( index < fIndexBufferRefs.GetCount() + 1, "Index buffers must be assigned linearly!" );

	if( (int)index > (int)fIndexBufferRefs.GetCount() - 1 )
	{
		fIndexBufferRefs.Append( ib );
		hsRefCnt_SafeRef( ib );
	}
	else
	{
		hsRefCnt_SafeAssign( fIndexBufferRefs[ index ], ib );
	}

}

//// PrepForRendering /////////////////////////////////////////////////////////

void	plGBufferGroup::PrepForRendering( plPipeline *pipe, hsBool adjustForNvidiaLighting )
{
	ISendStorageToBuffers( pipe, adjustForNvidiaLighting );
	// The following line was taken out so we'd keep our data around, allowing
	// us to rebuild the buffer if necessary on the fly
//	CleanUp();

}

hsGDeviceRef* plGBufferGroup::GetVertexBufferRef(UInt32 i) 
{ 
	if( i >= fVertexBufferRefs.GetCount() )
		fVertexBufferRefs.ExpandAndZero(i+1);

	return fVertexBufferRefs[i]; 
}

hsGDeviceRef* plGBufferGroup::GetIndexBufferRef(UInt32 i) 
{ 
	if( i >= fIndexBufferRefs.GetCount() )
		fIndexBufferRefs.ExpandAndZero(i+1);

	return fIndexBufferRefs[i]; 
}

//// ISendStorageToBuffers ////////////////////////////////////////////////////

void	plGBufferGroup::ISendStorageToBuffers( plPipeline *pipe, hsBool adjustForNvidiaLighting )
{
	plProfile_BeginTiming(DrawRefillVertex);

	/// Creating or refreshing?
	int i;
	for( i = 0; i < fVertBuffStorage.GetCount(); i++ )
	{
		pipe->CheckVertexBufferRef(this, i);
	}
	plProfile_EndTiming(DrawRefillVertex);

	plProfile_BeginTiming(DrawRefillIndex);

	for( i = 0; i < fIdxBuffStorage.GetCount(); i++ )
	{
		pipe->CheckIndexBufferRef(this, i);
	}

	plProfile_EndTiming(DrawRefillIndex);
}


//// ICalcVertexSize //////////////////////////////////////////////////////////

UInt8	plGBufferGroup::ICalcVertexSize( UInt8 &liteStride )
{
	UInt8	size;


	size = sizeof( float ) * ( 3 + 3 );	// pos + normal
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
			size += sizeof( UInt32 );
	}

	liteStride = size;
	size += sizeof( DWORD ) * 2;			// diffuse + specular
	return size;
}

//// I/O Functions ////////////////////////////////////////////////////////////

void	plGBufferGroup::Read( hsStream *s ) 
{
	UInt32			totalDynSize, i, count, temp = 0, j;
	UInt8			*vData;
	UInt16			*iData;
	plGBufferColor	*cData;


	s->ReadSwap( &fFormat );
	totalDynSize = s->ReadSwap32();
	fStride = ICalcVertexSize( fLiteStride );

	fVertBuffSizes.Reset();
	fVertBuffStarts.Reset();
	fVertBuffEnds.Reset();
	fColorBuffCounts.Reset();
	fIdxBuffCounts.Reset();
	fIdxBuffStarts.Reset();
	fIdxBuffEnds.Reset();
	fVertBuffStorage.Reset();
	fIdxBuffStorage.Reset();

	plVertCoder coder;
	/// Create buffers and read in as we go
	count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
	{
		if( fFormat & kEncoded )
		{
			const UInt16 numVerts = s->ReadSwap16();
			const UInt32 size = numVerts * fStride;

			fVertBuffSizes.Append(size);
			fVertBuffStarts.Append(0);
			fVertBuffEnds.Append(-1);

			vData = TRACKED_NEW UInt8[size];
			fVertBuffStorage.Append( vData );
			plProfile_NewMem(MemBufGrpVertex, temp);

			coder.Read(s, vData, fFormat, fStride, numVerts);

			fColorBuffCounts.Append(0);						
			fColorBuffStorage.Append(nil);

		}
		else
		{
			temp = s->ReadSwap32();
	
			fVertBuffSizes.Append( temp );
			fVertBuffStarts.Append(0);
			fVertBuffEnds.Append(-1);
			
			vData = TRACKED_NEW UInt8[ temp ];
			hsAssert( vData != nil, "Not enough memory to read in vertices" );
			s->Read( temp, (void *)vData );
			fVertBuffStorage.Append( vData );
			plProfile_NewMem(MemBufGrpVertex, temp);
			
			temp = s->ReadSwap32();
			fColorBuffCounts.Append( temp );
			
			if( temp > 0 )
			{
				cData = TRACKED_NEW plGBufferColor[ temp ];
				s->Read( temp * sizeof( plGBufferColor ), (void *)cData );
				plProfile_NewMem(MemBufGrpVertex, temp * sizeof(plGBufferColor));
			}
			else
				cData = nil;
			
			fColorBuffStorage.Append( cData );
		}
	}

	count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
	{
		temp = s->ReadSwap32();
		fIdxBuffCounts.Append( temp );
		fIdxBuffStarts.Append(0);
		fIdxBuffEnds.Append(-1);

		iData = TRACKED_NEW UInt16[ temp ];
		hsAssert( iData != nil, "Not enough memory to read in indices" );
		s->ReadSwap16( temp, (UInt16 *)iData );
		fIdxBuffStorage.Append( iData );
		plProfile_NewMem(MemBufGrpIndex, temp * sizeof(UInt16));
	}

	/// Read in cell arrays, one per vBuffer
	for( i = 0; i < fVertBuffStorage.GetCount(); i++ )
	{
		temp = s->ReadSwap32();

		fCells.Append( TRACKED_NEW hsTArray<plGBufferCell> );
		fCells[ i ]->SetCount( temp );

		for( j = 0; j < temp; j++ )
			(*fCells[ i ])[ j ].Read( s );
	}

}

//#define VERT_LOG

void	plGBufferGroup::Write( hsStream *s ) 
{
	UInt32		totalDynSize, i, j;

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
	for( i = 0; i < fVertBuffSizes.GetCount(); i++ )
		totalDynSize += fVertBuffSizes[ i ];
	for( i = 0; i < fIdxBuffCounts.GetCount(); i++ )
		totalDynSize += sizeof( UInt16 ) * fIdxBuffCounts[ i ];

	s->WriteSwap( fFormat );
	s->WriteSwap32( totalDynSize );

	plVertCoder coder;

	/// Write out dyanmic data
	s->WriteSwap32( (UInt32)fVertBuffStorage.GetCount() );
	for( i = 0; i < fVertBuffStorage.GetCount(); i++ )
	{
#ifdef MF_VERTCODE_ENABLED

		hsAssert(fCells[i]->GetCount() == 1, "Data must be interleaved for compression");
		UInt32 numVerts = fVertBuffSizes[i] / fStride;
		s->WriteSwap16((UInt16)numVerts);
		coder.Write(s, fVertBuffStorage[i], fFormat, fStride, (UInt16)numVerts);

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
		
		s->WriteSwap32( fVertBuffSizes[ i ] );
		s->Write( fVertBuffSizes[ i ], (void *)fVertBuffStorage[ i ] );
		

		s->WriteSwap32( fColorBuffCounts[ i ] );
		s->Write( fColorBuffCounts[ i ] * sizeof( plGBufferColor ), (void *)fColorBuffStorage[ i ] );

#endif // MF_VERTCODE_ENABLED
	}

	s->WriteSwap32( (UInt32)fIdxBuffCounts.GetCount() );
	for( i = 0; i < fIdxBuffStorage.GetCount(); i++ )
	{
		s->WriteSwap32( fIdxBuffCounts[ i ] );
		s->WriteSwap16( fIdxBuffCounts[ i ], fIdxBuffStorage[ i ] );
	}

	/// Write out cell arrays
	for( i = 0; i < fVertBuffStorage.GetCount(); i++ )
	{
		s->WriteSwap32( fCells[ i ]->GetCount() );
		for( j = 0; j < fCells[ i ]->GetCount(); j++ )
			(*fCells[ i ])[ j ].Write( s );
	}

#ifdef VERT_LOG
	log.Close();
#endif
	// All done!
}

///////////////////////////////////////////////////////////////////////////////
//// Editing Functions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// DeleteVertsFromStorage ///////////////////////////////////////////////////
//	Deletes a span of verts from the vertex storage. Remember to Prep this
//	group after doing this!
//	Note: does NOT adjust index storage, since we don't know inside here
//	which indices to adjust. Have to call that separately.
//	Note 2: for simplicity sake, we only do this for groups with ONE interleaved
//	cell. Doing this for multiple separated cells would be, literally, hell.

void	plGBufferGroup::DeleteVertsFromStorage( UInt32 which, UInt32 start, UInt32 length )
{
	UInt8		*dstPtr, *srcPtr;
	UInt32		amount;


	hsAssert( fCells[ which ]->GetCount() == 1, "Cannot delete verts on a mixed buffer group" );

	// Adjust cell 0
	(*fCells[ which ])[ 0 ].fLength -= length;

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

	if( fVertexBufferRefs.GetCount() > which && fVertexBufferRefs[ which ] != nil )
	{
		hsRefCnt_SafeUnRef(fVertexBufferRefs[which]);
		fVertexBufferRefs[which] = nil;
	}

}

//// AdjustIndicesInStorage ///////////////////////////////////////////////////
//	Adjusts indices >= a given threshold by a given delta. Use it to adjust
//	indices after vertex deletion. Affects only the given buffer.

void	plGBufferGroup::AdjustIndicesInStorage( UInt32 which, UInt16 threshhold, Int16 delta )
{
	int			i;


	for( i = 0; i < fIdxBuffCounts[ which ]; i++ )
	{
		if( fIdxBuffStorage[ which ][ i ] >= threshhold )
			fIdxBuffStorage[ which ][ i ] += delta;
	}

	if( fIndexBufferRefs.GetCount() > which && fIndexBufferRefs[ which ] != nil )
		fIndexBufferRefs[ which ]->SetDirty( true );

}

//// DeleteIndicesFromStorage /////////////////////////////////////////////////
//	Deletes a span of indices from the index storage. Remember to Prep this
//	group after doing this!

void	plGBufferGroup::DeleteIndicesFromStorage( UInt32 which, UInt32 start, UInt32 length )
{
	UInt16		*dstPtr, *srcPtr;
	UInt32		amount;


	hsAssert( start + length <= fIdxBuffCounts[ which ], "Illegal range to DeleteIndicesFromStorage()" );

	if( start + length < fIdxBuffCounts[ which ] )
	{
		dstPtr = &( fIdxBuffStorage[ which ][ start ] );
		srcPtr = &( fIdxBuffStorage[ which ][ start + length ] );

		amount = fIdxBuffCounts[ which ] - ( start + length );

		memmove( dstPtr, srcPtr, amount * sizeof( UInt16 ) );
	}

	fIdxBuffCounts[ which ] -= length;
	plProfile_DelMem(MemBufGrpIndex, length * sizeof(UInt16));

	if( fIndexBufferRefs.GetCount() > which && fIndexBufferRefs[ which ] != nil )
	{
		hsRefCnt_SafeUnRef(fIndexBufferRefs[which]);
		fIndexBufferRefs[which] = nil;
	}

}

//// GetNumPrimaryVertsLeft ///////////////////////////////////////////////////
//	Base on the cells, so we can take instanced cells into account

UInt32	plGBufferGroup::GetNumPrimaryVertsLeft( void ) const
{
	return GetNumVertsLeft( 0 );
}

//// GetNumVertsLeft //////////////////////////////////////////////////////////
//	Base on the cells, so we can take instanced cells into account

UInt32	plGBufferGroup::GetNumVertsLeft( UInt32 idx ) const
{
	if( idx >= fCells.GetCount() )
		return kMaxNumVertsPerBuffer;

	UInt32		i, total = kMaxNumVertsPerBuffer;


	for( i = 0; i < fCells[ idx ]->GetCount(); i++ )
		total -= (*fCells[ idx ])[ i ].fLength;

	return total;
}

//// IMakeCell ////////////////////////////////////////////////////////////////
//	Get a cell from the given cell array.

UInt32	plGBufferGroup::IMakeCell( UInt32 vbIndex, UInt8 flags, UInt32 vStart, UInt32 cStart, UInt32 len, UInt32 *offset )
{
	hsTArray<plGBufferCell>	*cells = fCells[ vbIndex ];


	if( !(flags & kReserveInterleaved) )
	{
		/// Note that there are THREE types of cells: interleaved (colorStart == -1),
		/// first of an instance group (colorStart != -1 && vStart != -1) and 
		/// an instance (colorStart != -1 && vStart == -1 ). To simplify things,
		/// we never merge any separated cells

		if( flags & kReserveSeparated )
			cells->Append( plGBufferCell( vStart, cStart, len ) );
		else
			cells->Append( plGBufferCell( (UInt32)-1, cStart, len ) );
		*offset = 0;
	}
	else
	{
		/// Merge if the last cell was an interleaved cell
		if( cells->GetCount() > 0 && (*cells)[ cells->GetCount() - 1 ].fColorStart == (UInt32)-1 )
		{
			*offset = (*cells)[ cells->GetCount() - 1 ].fLength;
			(*cells)[ cells->GetCount() - 1 ].fLength += len;
		}
		else
		{
			cells->Append( plGBufferCell( vStart, (UInt32)-1, len ) );
			*offset = 0;
		}
	}

	return cells->GetCount() - 1;
}

//// ReserveVertStorage ///////////////////////////////////////////////////////
//	Takes a length to reserve in a vertex buffer and returns the buffer index
//	and starting position. Basically does what AppendToVertStorage() used to
//	do except it doesn't actually copy any data into the space reserved.

hsBool	plGBufferGroup::ReserveVertStorage( UInt32 numVerts, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset, UInt8 flags )
{
	UInt8					*storagePtr = nil;
	UInt32					cStartIdx = 0, vStartIdx = 0;
	plGBufferColor			*cStoragePtr = nil;
	int						i;


	if( numVerts >= kMaxNumVertsPerBuffer )
	{
		hsAssert( false, "Egad, why on earth are you adding that many verts???" );
		return false;
	}

	/// Find a spot
	if( !(flags & kReserveIsolate) )
	{
		for( i = 0; i < fVertBuffStorage.GetCount(); i++ )
		{
			if( GetNumVertsLeft( i ) >= numVerts )
				break;
		}
	}
	else
	{
		i = fVertBuffStorage.GetCount();
	}
	if( i == fVertBuffStorage.GetCount() )
	{
		if( (flags & kReserveInterleaved) || (flags & kReserveSeparated) )
		{
			fVertBuffStorage.Append( nil );
			fVertBuffSizes.Append( 0 );
		}
		fVertBuffStarts.Append(0);
		fVertBuffEnds.Append(-1);

		fColorBuffStorage.Append( nil );
		fColorBuffCounts.Append( 0 );

		fCells.Append( TRACKED_NEW hsTArray<plGBufferCell> );
	}

	*vbIndex = i;

	if( !(flags & kReserveInterleaved) )
	{
		// Splitting the data into vertex and color storage
		if( flags & kReserveSeparated )
		{
			/// Increase the storage size
			vStartIdx = fVertBuffSizes[ i ];
			storagePtr = TRACKED_NEW UInt8[ fVertBuffSizes[ i ] + numVerts * fLiteStride ];
			if( fVertBuffSizes[ i ] > 0 )
				memcpy( storagePtr, fVertBuffStorage[ i ], fVertBuffSizes[ i ] );
			fVertBuffSizes[ i ] += numVerts * fLiteStride;
			plProfile_NewMem(MemBufGrpVertex, numVerts * fLiteStride);
		}

		/// Color too
		cStartIdx = fColorBuffCounts[ i ];
		cStoragePtr = TRACKED_NEW plGBufferColor[ fColorBuffCounts[ i ] + numVerts ];
		if( fColorBuffCounts[ i ] > 0 )
			memcpy( cStoragePtr, fColorBuffStorage[ i ], fColorBuffCounts[ i ] * sizeof( plGBufferColor ) );
	}
	else
	{
		// Interleaved

		/// Increase the storage size
		vStartIdx = fVertBuffSizes[ i ];
		storagePtr = TRACKED_NEW UInt8[ fVertBuffSizes[ i ] + numVerts * fStride ];
		if( fVertBuffSizes[ i ] > 0 )
			memcpy( storagePtr, fVertBuffStorage[ i ], fVertBuffSizes[ i ] );
		fVertBuffSizes[ i ] += numVerts * fStride;
		plProfile_NewMem(MemBufGrpVertex, numVerts * fStride);
	}

	/// Switch over
	if( storagePtr != nil )
	{
		if( fVertBuffStorage[ i ] != nil )
			delete [] fVertBuffStorage[ i ];
		fVertBuffStorage[ i ] = storagePtr;
	}
	if( cStoragePtr != nil )
	{
		if( fColorBuffStorage[ i ] != nil )
			delete [] fColorBuffStorage[ i ];
		fColorBuffStorage[ i ] = cStoragePtr;
		fColorBuffCounts[ i ] += numVerts;
		plProfile_NewMem(MemBufGrpVertex, numVerts * sizeof(plGBufferColor));
	}

	if( fVertexBufferRefs.GetCount() > i && fVertexBufferRefs[ i ] != nil )
	{
		hsRefCnt_SafeUnRef(fVertexBufferRefs[i]);
		fVertexBufferRefs[i] = nil;
	}

	/// Append a cell entry
	*cell = IMakeCell( i, flags, vStartIdx, cStartIdx, numVerts, offset );

	/// All done!
	return true;
}

//// AppendToVertStorage //////////////////////////////////////////////////////
//	Given an opaque array of vertex data, puts it into the first available spot 
//	in fVertStorage. If there is no array on fVertStorage that can hold them,
//	we create a new one. Returns the index of the storage array and the
//	starting index into the array. Note that we basically stick it wherever
//	we can, instead of at the end.
//	Updated 4.30.2001 mcn to take in a plGeometrySpan as a source rather than
//	raw data, since the plGeometrySpan no longer stores data in exactly the
//	same format.
//	Updated 6.15.2001 mcn to use ReserveVertStorage().

void	plGBufferGroup::AppendToVertStorage( plGeometrySpan *srcSpan, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset )
{
	if( !ReserveVertStorage( srcSpan->fNumVerts, vbIndex, cell, offset, kReserveInterleaved ) )
		return;

	StuffToVertStorage( srcSpan, *vbIndex, *cell, *offset, kReserveInterleaved );
}

//// AppendToVertAndColorStorage //////////////////////////////////////////////
//	Same as AppendToVertStorage(), but writes only the verts to the vertex
//	storage and the colors to the separate color storage.

void	plGBufferGroup::AppendToVertAndColorStorage( plGeometrySpan *srcSpan, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset )
{
	if( !ReserveVertStorage( srcSpan->fNumVerts, vbIndex, cell, offset, kReserveSeparated ) )
		return;

	StuffToVertStorage( srcSpan, *vbIndex, *cell, *offset, kReserveSeparated );
}

//// AppendToColorStorage /////////////////////////////////////////////////////
//	Same as AppendToVertStorage(), but writes JUST to color storage.

void	plGBufferGroup::AppendToColorStorage( plGeometrySpan *srcSpan, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset, UInt32 origCell )
{
	if( !ReserveVertStorage( srcSpan->fNumVerts, vbIndex, cell, offset, kReserveColors ) )
		return;

	(*fCells[ *vbIndex ])[ *cell ].fVtxStart = (*fCells[ *vbIndex ])[ origCell ].fVtxStart;

	StuffToVertStorage( srcSpan, *vbIndex, *cell, *offset, kReserveColors );
}

//// IGetStartVtxPointer //////////////////////////////////////////////////////
//	Get the start vertex and color buffer pointers for a given cell and offset

void	plGBufferGroup::IGetStartVtxPointer( UInt32 vbIndex, UInt32 cell, UInt32 offset, UInt8 *&tempPtr, plGBufferColor *&cPtr )
{
	hsAssert( vbIndex < fVertBuffStorage.GetCount(), "Invalid vbIndex in StuffToVertStorage()" );
	hsAssert( cell < fCells[ vbIndex ]->GetCount(), "Invalid cell in StuffToVertStorage()" );

	tempPtr = fVertBuffStorage[ vbIndex ];
	cPtr = fColorBuffStorage[ vbIndex ];

	tempPtr += (*fCells[ vbIndex ])[ cell ].fVtxStart;
	cPtr += (*fCells[ vbIndex ])[ cell ].fColorStart;

	if( offset > 0 )
	{
		tempPtr += offset * ( ( (*fCells[ vbIndex ])[ cell ].fColorStart == (UInt32)-1 ) ? fStride : fLiteStride );
		cPtr += offset;
	}
}

//// GetVertBufferCount ///////////////////////////////////////////////////////

UInt32	plGBufferGroup::GetVertBufferCount( UInt32 idx ) const
{
	return GetVertStartFromCell( idx, fCells[ idx ]->GetCount(), 0 );
}

//// GetVertStartFromCell /////////////////////////////////////////////////////

UInt32	plGBufferGroup::GetVertStartFromCell( UInt32 vbIndex, UInt32 cell, UInt32 offset ) const
{
	UInt32	i, numVerts;


	hsAssert( vbIndex < fVertBuffStorage.GetCount(), "Invalid vbIndex in StuffToVertStorage()" );
	hsAssert( cell <= fCells[ vbIndex ]->GetCount(), "Invalid cell in StuffToVertStorage()" );

	numVerts = 0;
	for( i = 0; i < cell; i++ )
		numVerts += (*fCells[ vbIndex ])[ i ].fLength;

	numVerts += offset;

	return numVerts;
}

//// StuffToVertStorage ///////////////////////////////////////////////////////
//	Stuffs data from a plGeometrySpan into the specified location in the 
//	specified vertex buffer.

void	plGBufferGroup::StuffToVertStorage( plGeometrySpan *srcSpan, UInt32 vbIndex, UInt32 cell, UInt32 offset, UInt8 flags )
{
	UInt8			*tempPtr, stride;
	plGBufferColor	*cPtr;
	int				i, j, numVerts;
	plGBufferCell	*cellPtr;


	hsAssert( vbIndex < fVertBuffStorage.GetCount(), "Invalid vbIndex in StuffToVertStorage()" );
	hsAssert( cell < fCells[ vbIndex ]->GetCount(), "Invalid cell in StuffToVertStorage()" );

	IGetStartVtxPointer( vbIndex, cell, offset, tempPtr, cPtr );
	cellPtr = &(*fCells[ vbIndex ])[ cell ];
	stride = ( cellPtr->fColorStart != (UInt32)-1 ) ? fLiteStride : fStride;

	numVerts = srcSpan->fNumVerts;

	/// Copy the data over
	for( i = 0; i < numVerts; i++ )
	{
		hsPoint3	pos;
		float		weights[ 3 ];
		UInt32		weightIndices;
		hsVector3	norm;
		UInt32		color, specColor;
		hsPoint3	uvs[ plGeometrySpan::kMaxNumUVChannels ];
		float		*fPtr;
		UInt32		*dPtr;


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
					dPtr = (UInt32 *)fPtr;
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
				dPtr = (UInt32 *)fPtr;
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

	if( ( vbIndex < fVertexBufferRefs.GetCount() ) && fVertexBufferRefs[ vbIndex ] )
		fVertexBufferRefs[ vbIndex ]->SetDirty( true );
}


//// ReserveIndexStorage //////////////////////////////////////////////////////
//	Same as ReserveVertStorage(), only for indices :)

hsBool	plGBufferGroup::ReserveIndexStorage( UInt32 numIndices, UInt32 *ibIndex, UInt32 *ibStart, UInt16 **dataPtr )
{
	UInt16			*storagePtr;
	int				i;


	if( numIndices >= kMaxNumIndicesPerBuffer )
	{
		hsAssert( false, "Egad, why on earth are you adding that many indices???" );
		return false;
	}

	/// Find a spot
	for( i = 0; i < fIdxBuffStorage.GetCount(); i++ )
	{

		if( fIdxBuffCounts[ i ] + numIndices < kMaxNumIndicesPerBuffer )
			break;
	}
	if( i == fIdxBuffStorage.GetCount() )
	{
		fIdxBuffStorage.Append( nil );
		fIdxBuffCounts.Append( 0 );

		fIdxBuffStarts.Append(0);
		fIdxBuffEnds.Append(-1);
	}

	*ibIndex = i;
	*ibStart = fIdxBuffCounts[ i ];

	/// Increase the storage size
	storagePtr = TRACKED_NEW UInt16[ fIdxBuffCounts[ i ] + numIndices ];
	if( fIdxBuffCounts[ i ] > 0 )
		memcpy( storagePtr, fIdxBuffStorage[ i ], fIdxBuffCounts[ i ] * sizeof( UInt16 ) );

	if( dataPtr != nil )
		*dataPtr = storagePtr + fIdxBuffCounts[ i ];

	/// Switch over
	i = *ibIndex;
	if( fIdxBuffStorage[ i ] != nil )
		delete [] fIdxBuffStorage[ i ];
	fIdxBuffStorage[ i ] = storagePtr;
	fIdxBuffCounts[ i ] += numIndices;
	plProfile_NewMem(MemBufGrpIndex, numIndices * sizeof(UInt16));

	/// All done!
	if( fIndexBufferRefs.GetCount() > i && fIndexBufferRefs[ i ] != nil )
	{
		hsRefCnt_SafeUnRef(fIndexBufferRefs[i]);
		fIndexBufferRefs[i] = nil;
	}

	return true;
}

//// AppendToIndexStorage /////////////////////////////////////////////////////
//	Same as AppendToVertStorage, only for the index buffers and indices. Duh :)

void	plGBufferGroup::AppendToIndexStorage( UInt32 numIndices, UInt16 *data, UInt32 addToAll, 
											  UInt32 *ibIndex, UInt32 *ibStart )
{
	UInt16			*tempPtr;
	int				i;


	if( !ReserveIndexStorage( numIndices, ibIndex, ibStart, &tempPtr ) )
		return;

	/// Copy new indices over, offsetting them as we were told to
	for( i = 0; i < numIndices; i++ )
		tempPtr[ i ] = data[ i ] + (UInt16)addToAll;

	/// All done!
}

//// ConvertToTriList /////////////////////////////////////////////////////////
//	Returns an array of plGBufferTriangles representing the span of indices 
//	specified. 

plGBufferTriangle	*plGBufferGroup::ConvertToTriList( Int16 spanIndex, UInt32 whichIdx, UInt32 whichVtx, UInt32 whichCell, UInt32 start, UInt32 numTriangles )
{
	plGBufferTriangle	*array;
	UInt16				*storagePtr;
	UInt8				*vertStgPtr, stride;
	float				*vertPtr;
	int					i, j;
	hsPoint3			center;
	UInt32				offsetBy;
	plGBufferColor		*wastePtr;


	/// Sanity checks
	hsAssert( whichIdx < fIdxBuffStorage.GetCount(), "Invalid index buffer ID to ConvertToTriList()" );
	hsAssert( whichVtx < fVertBuffStorage.GetCount(), "Invalid vertex buffer ID to ConvertToTriList()" );
	hsAssert( start < fIdxBuffCounts[ whichIdx ], "Invalid start index to ConvertToTriList()" );
	hsAssert( start + numTriangles * 3 <= fIdxBuffCounts[ whichIdx ], "Invalid count to ConvertToTriList()" );
	hsAssert( whichCell < fCells[ whichVtx ]->GetCount(), "Invalid cell to ConvertToTriList()" );

	/// Create the array and fill it
	array = TRACKED_NEW plGBufferTriangle[ numTriangles ];
	hsAssert( array != nil, "Not enough memory to create triangle data in ConvertToTriList()" );

	storagePtr = fIdxBuffStorage[ whichIdx ];
	IGetStartVtxPointer( whichVtx, whichCell, 0, vertStgPtr, wastePtr );
	offsetBy = GetVertStartFromCell( whichVtx, whichCell, 0 );
	stride = ( (*fCells[ whichVtx ])[ whichCell ].fColorStart == (UInt32)-1 ) ? fStride : fLiteStride;
	
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
//	Stuffs the indices from an array of plGBufferTriangles into the index 
//	storage.

void	plGBufferGroup::StuffFromTriList( UInt32 which, UInt32 start, UInt32 numTriangles, UInt16 *data )
{
	UInt16			*storagePtr;


	/// Sanity checks
	hsAssert( which < fIdxBuffStorage.GetCount(), "Invalid index buffer ID to StuffFromTriList()" );
	hsAssert( start < fIdxBuffCounts[ which ], "Invalid start index to StuffFromTriList()" );
	hsAssert( start + numTriangles * 3 <= fIdxBuffCounts[ which ], "Invalid count to StuffFromTriList()" );


	/// This is easy--just stuff!
	storagePtr = fIdxBuffStorage[ which ];
#define MF_SPEED_THIS_UP
#ifndef MF_SPEED_THIS_UP
	int				i, j;
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
	if( fIndexBufferRefs.GetCount() > which && fIndexBufferRefs[ which ] != nil )
		fIndexBufferRefs[ which ]->SetDirty( true );

}

//// StuffTri /////////////////////////////////////////////////////////////////

void	plGBufferGroup::StuffTri( UInt32 iBuff, UInt32 iTri, UInt16 idx0, UInt16 idx1, UInt16 idx2 )
{
	/// Sanity checks
	hsAssert( iBuff < fIdxBuffStorage.GetCount(), "Invalid index buffer ID to StuffFromTriList()" );
	hsAssert( iTri < fIdxBuffCounts[ iBuff ], "Invalid start index to StuffFromTriList()" );

	fIdxBuffStorage[ iBuff ][ iTri + 0 ] = idx0;
	fIdxBuffStorage[ iBuff ][ iTri + 1 ] = idx1;
	fIdxBuffStorage[ iBuff ][ iTri + 2 ] = idx2;

}

//// Accessor Functions ///////////////////////////////////////////////////////

hsPoint3	&plGBufferGroup::Position( int iBuff, UInt32 cell, int iVtx )
{
	UInt8			*vertStgPtr;
	plGBufferColor	*cPtr;

	IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

	return *(hsPoint3 *)( vertStgPtr + 0 ); 
}

hsVector3	&plGBufferGroup::Normal( int iBuff, UInt32 cell, int iVtx )
{
	UInt8			*vertStgPtr;
	plGBufferColor	*cPtr;

	IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

	return *(hsVector3 *)( vertStgPtr + sizeof( hsPoint3 ) ); 
}

UInt32	&plGBufferGroup::Color( int iBuff, UInt32 cell, int iVtx ) 
{
	UInt8			*vertStgPtr;
	plGBufferColor	*cPtr;

	IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

	if( (*fCells[ iBuff ])[ cell ].fColorStart != (UInt32)-1 )
		return *(UInt32 *)( &cPtr->fDiffuse );
	else
		return *(UInt32 *)( vertStgPtr + 2 * sizeof( hsPoint3 ) );
}

UInt32	&plGBufferGroup::Specular( int iBuff, UInt32 cell, int iVtx ) 
{
	UInt8			*vertStgPtr;
	plGBufferColor	*cPtr;

	IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

	if( (*fCells[ iBuff ])[ cell ].fColorStart != (UInt32)-1 )
		return *(UInt32 *)( &cPtr->fSpecular );
	else
		return *(UInt32 *)( vertStgPtr + 2 * sizeof( hsPoint3 ) );
}

hsPoint3	&plGBufferGroup::UV( int iBuff, UInt32 cell, int iVtx, int channel )
{
	UInt8			*vertStgPtr;
	plGBufferColor	*cPtr;

	IGetStartVtxPointer( iBuff, cell, iVtx, vertStgPtr, cPtr );

	vertStgPtr += 2 * sizeof( hsPoint3 ) + channel * sizeof( hsPoint3 );

	if( (*fCells[ iBuff ])[ cell ].fColorStart != (UInt32)-1 )
		return *(hsPoint3 *)( vertStgPtr  );
	else
		return *(hsPoint3 *)( vertStgPtr + 2 * sizeof( UInt32 ) );
}
