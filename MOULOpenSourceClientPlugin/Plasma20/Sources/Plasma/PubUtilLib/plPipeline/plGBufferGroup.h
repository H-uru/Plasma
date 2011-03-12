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
//	plGBufferGroup Class Header	    										 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	2.21.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plGBufferGroup_h
#define _plGBufferGroup_h

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "hsColorRGBA.h"

//// plGBufferTriangle Struct Definition //////////////////////////////////////
//
//	Represents a single triangle inside a plGBufferGroup, which consists of
//	three indices (the indices of the three vertices) and a 3-D point
//	representing the center of the triangle.

class plGBufferTriangle
{
	public:
		UInt16		fIndex1, fIndex2, fIndex3, fSpanIndex;
		hsPoint3	fCenter;

		void	Read( hsStream *s );
		void	Write( hsStream *s );
};

//// plGBufferCell and plGBufferColor Definitions /////////////////////////////

class plGBufferCell 
{
	public:
		UInt32	fVtxStart;		// In bytes
		UInt32	fColorStart;	// In bytes
		UInt32	fLength;

		plGBufferCell( UInt32 vStart, UInt32 cStart, UInt32 len )
		{
			fVtxStart = vStart; fColorStart = cStart; fLength = len;
		}

		plGBufferCell() {}

		void	Read( hsStream *s );
		void	Write( hsStream *s );
};

class plGBufferColor
{
	public:
		UInt32	fDiffuse, fSpecular;
};

//// plGBufferGroup Class Definition //////////////////////////////////////////
//
//	Represents a list of vertex and index buffers in a nice package.

class hsStream;
class hsResMgr;
class plPipeline;
class hsGDeviceRef;
class plGeometrySpan;

class plGBufferGroup
{
	protected:
		UInt8	fFormat;
		UInt8	fStride;
		UInt8	fLiteStride;
		UInt8	fNumSkinWeights;
		UInt32	fNumVerts;
		UInt32	fNumIndices;
		hsBool	fVertsVolatile;
		hsBool	fIdxVolatile;
		int		fLOD;
		
		hsTArray<hsGDeviceRef *>		fVertexBufferRefs;
		hsTArray<hsGDeviceRef *>		fIndexBufferRefs;

		hsTArray<UInt32>	fVertBuffSizes;
		hsTArray<UInt32>	fIdxBuffCounts;
		hsTArray<UInt32>	fColorBuffCounts;
		hsTArray<UInt8 *>	fVertBuffStorage;
		hsTArray<UInt16 *>	fIdxBuffStorage;

		hsTArray<UInt32>	fVertBuffStarts;
		hsTArray<Int32>		fVertBuffEnds;
		hsTArray<UInt32>	fIdxBuffStarts;
		hsTArray<Int32>		fIdxBuffEnds;

		hsTArray<plGBufferColor *>	fColorBuffStorage;

		hsTArray<hsTArray<plGBufferCell> *>	fCells;

		virtual void	ISendStorageToBuffers( plPipeline *pipe, hsBool adjustForNvidiaLighting );

		UInt8			ICalcVertexSize( UInt8 &liteStride );

		UInt8* IVertBuffStorage(int iBuff, int iVtx) const { return fVertBuffStorage[iBuff] + iVtx*fStride; }

		UInt32	IMakeCell( UInt32 vbIndex, UInt8 flags, UInt32 vStart, UInt32 cStart, UInt32 len, UInt32 *offset );
		void	IGetStartVtxPointer( UInt32 vbIndex, UInt32 cell, UInt32 offset, UInt8 *&tempPtr, plGBufferColor *&cPtr );

	public:

		static const UInt32		kMaxNumVertsPerBuffer;
		static const UInt32		kMaxNumIndicesPerBuffer;

		enum Formats
		{
			kUVCountMask	= 0x0f,	// Problem is, we need enough bits to store the max #, which means
									// we really want ( max # << 1 ) - 1

			kSkinNoWeights  = 0x00,	// 0000000
			kSkin1Weight	= 0x10,	// 0010000
			kSkin2Weights	= 0x20,	// 0100000
			kSkin3Weights	= 0x30,	// 0110000
			kSkinWeightMask	= 0x30,	// 0110000

			kSkinIndices	= 0x40,  // 1000000

			kEncoded		= 0x80  
		};

		enum
		{
			kReserveInterleaved = 0x01,
			kReserveVerts		= 0x02,
			kReserveColors		= 0x04,
			kReserveSeparated	= 0x08,
			kReserveIsolate		= 0x10
		};

		plGBufferGroup(UInt8 format, hsBool vertsVolatile, hsBool idxVolatile, int LOD = 0);
		~plGBufferGroup();

		UInt8	GetNumUVs( void ) const { return ( fFormat & kUVCountMask ); }
		UInt8	GetNumWeights() const { return (fFormat & kSkinWeightMask) >> 4; }

		static UInt8	CalcNumUVs( UInt8 format ) { return ( format & kUVCountMask ); }
		static UInt8	UVCountToFormat( UInt8 numUVs ) { return numUVs & kUVCountMask; }

		void	DirtyVertexBuffer(int i);
		void	DirtyIndexBuffer(int i);
		hsBool	VertexReady(int i) const { return (i < fVertexBufferRefs.GetCount()) && fVertexBufferRefs[i]; }
		hsBool	IndexReady(int i) const { return  (i < fIndexBufferRefs.GetCount()) && fIndexBufferRefs[i]; }
		UInt8	GetVertexSize( void ) const { return fStride; }
		UInt8	GetVertexLiteStride( void ) const { return fLiteStride; }
		UInt8	GetVertexFormat( void ) const { return fFormat; }
		UInt32	GetMemUsage( void ) const { return ( fNumVerts * GetVertexSize() ) + ( fNumIndices * sizeof( UInt16 ) ); }
		UInt32	GetNumVerts( void ) const { return fNumVerts; }
		UInt32	GetNumIndices( void ) const { return fNumIndices; }
		UInt32	GetNumPrimaryVertsLeft( void ) const;
		UInt32	GetNumVertsLeft( UInt32 idx ) const;

		UInt32	GetVertBufferSize(UInt32 idx) const { return fVertBuffSizes[idx]; }
		UInt32	GetVertBufferCount(UInt32 idx) const;
		UInt32	GetIndexBufferCount(UInt32 idx) const { return fIdxBuffCounts[idx]; }
		UInt32	GetVertStartFromCell(UInt32 idx, UInt32 cell, UInt32 offset) const;

		// These should only be called by the pipeline, because only it knows when it's safe.
		// If the data is volatile, these are no-ops
		void PurgeVertBuffer(UInt32 idx);
		void PurgeIndexBuffer(UInt32 idx);

		///////////////////////////////////////////////////////////////////////////////
		// The following group of functions is an advanced optimization, and a pretty 
		// specialized one at that. It just limits the amount of data that will get
		// uploaded to video. If you don't know you are limited by bandwidth to the
		// board, or you just don't know what your are doing, don't mess with them.
		// If you never touch them, everything will work. If you set them correcly,
		// things may work faster. If you set them incorrectly, be sure to save
		// all files before running.
		// All of these are indices, not bytes. from the beginning of the buffer. 
		UInt32	GetVertBufferStart(UInt32 idx) const { return fVertBuffStarts[idx]; }
		UInt32	GetVertBufferEnd(UInt32 idx) const { return fVertBuffEnds[idx] >= 0 ? UInt32(fVertBuffEnds[idx]) : GetVertBufferCount(idx); }
		UInt32	GetIndexBufferStart(UInt32 idx) const { return fIdxBuffStarts[idx]; }
		UInt32	GetIndexBufferEnd(UInt32 idx) const { return fIdxBuffEnds[idx] >= 0 ? UInt32(fIdxBuffEnds[idx]) : GetIndexBufferCount(idx); }

		void	SetVertBufferStart(UInt32 idx, UInt32 s) { fVertBuffStarts[idx] = s; }
		void	SetVertBufferEnd(UInt32 idx, UInt32 e) { fVertBuffEnds[idx] = e; }
		void	SetIndexBufferStart(UInt32 idx, UInt32 s) { fIdxBuffStarts[idx] = s; }
		void	SetIndexBufferEnd(UInt32 idx, UInt32 e) { fIdxBuffEnds[idx] = e; }
		///////////////////////////////////////////////////////////////////////////////

		UInt32	GetNumVertexBuffers( void ) const { return fVertBuffStorage.GetCount(); }
		UInt32	GetNumIndexBuffers( void ) const { return fIdxBuffStorage.GetCount(); }

		UInt8			*GetVertBufferData( UInt32 idx ) { return fVertBuffStorage[ idx ]; }
		UInt16			*GetIndexBufferData( UInt32 idx ) { return fIdxBuffStorage[ idx ]; }
		plGBufferColor	*GetColorBufferData( UInt32 idx ) { return fColorBuffStorage[ idx ]; }

		hsGDeviceRef	*GetVertexBufferRef( UInt32 i );
		hsGDeviceRef	*GetIndexBufferRef( UInt32 i );

		UInt32			GetNumCells( UInt32 idx ) const { return fCells[ idx ]->GetCount(); }
		plGBufferCell	*GetCell( UInt32 idx, UInt32 cell ) { return &( (*fCells[ idx ])[ cell ] ); }

		void	SetVertexBufferRef( UInt32 index, hsGDeviceRef *vb );
		void	SetIndexBufferRef( UInt32 index, hsGDeviceRef *ib );

		virtual void Read( hsStream* s );
		virtual void Write( hsStream* s );

		// Accessor functions
		hsPoint3	&Position( int iBuff, UInt32 cell, int iVtx );
		hsVector3	&Normal( int iBuff, UInt32 cell, int iVtx );
		UInt32		&Color( int iBuff, UInt32 cell, int iVtx );
		UInt32		&Specular( int iBuff, UInt32 cell, int iVtx );
		hsPoint3	&UV( int iBuff, UInt32 cell, int iVtx, int channel );
		UInt32		Format() const { return fFormat; }

		// Take temp accumulators and actually build buffer data from them
		void	TidyUp( void );

		// Delete the buffer data storage
		void	CleanUp( void );

		// Take buffer data and convert it to device-specific buffers
		void	PrepForRendering( plPipeline *pipe, hsBool adjustForNvidiaLighting );

		// Reserves space in a vertex buffer
		hsBool	ReserveVertStorage( UInt32 numVerts, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset, UInt8 flags );

		// Append vertex data to the first available storage buffer
		void	AppendToVertStorage( plGeometrySpan *srcSpan, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset );
		void	AppendToVertAndColorStorage( plGeometrySpan *srcSpan, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset );
		void	AppendToColorStorage( plGeometrySpan *srcSpan, UInt32 *vbIndex, UInt32 *cell, UInt32 *offset, UInt32 origCell );

		// Reserves space in an index buffer
		hsBool	ReserveIndexStorage( UInt32 numIndices, UInt32 *ibIndex, UInt32 *ibStart, UInt16 **dataPtr = nil );

		// Append index data to the first available storage buffer
		void	AppendToIndexStorage( UInt32 numIndices, UInt16 *data, UInt32 addToAll, UInt32 *ibIndex, UInt32 *ibStart );


		/// Dynamic functions (addition/deletion of raw data)
		void	DeleteVertsFromStorage( UInt32 which, UInt32 start, UInt32 length );
		void	AdjustIndicesInStorage( UInt32 which, UInt16 threshhold, Int16 delta );
		void	DeleteIndicesFromStorage( UInt32 which, UInt32 start, UInt32 length );

		// Returns an array of plGBufferTriangles representing the span of indices specified
		plGBufferTriangle	*ConvertToTriList( Int16 spanIndex, UInt32 whichIdx, UInt32 whichVtx, UInt32 whichCell, UInt32 start, UInt32 numTriangles );

		// Stuffs the indices from an array of plGBufferTriangles into the index storage
		void	StuffFromTriList( UInt32 which, UInt32 start, UInt32 numTriangles, UInt16 *data );
		void	StuffTri( UInt32 iBuff, UInt32 iTri, UInt16 idx0, UInt16 idx1, UInt16 idx2 );

		// Stuff the data from a geometry span into vertex storage
		void	StuffToVertStorage( plGeometrySpan *srcSpan, UInt32 vbIndex, UInt32 cell, UInt32 offset, UInt8 flags );

		// Are our verts volatile?
		hsBool	AreVertsVolatile() const { return fVertsVolatile; }
		hsBool	AreIdxVolatile() const { return fIdxVolatile; }

		int GetLOD() const { return fLOD; }
};

#endif // _plGBufferGroup_h

