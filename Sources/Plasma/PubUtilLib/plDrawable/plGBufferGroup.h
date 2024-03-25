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
//  plGBufferGroup Class Header                                              //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  2.21.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plGBufferGroup_h
#define _plGBufferGroup_h

#include "hsGeometry3.h"
#include "hsColorRGBA.h"

//// plGBufferTriangle Struct Definition //////////////////////////////////////
//
//  Represents a single triangle inside a plGBufferGroup, which consists of
//  three indices (the indices of the three vertices) and a 3-D point
//  representing the center of the triangle.

class plGBufferTriangle
{
    public:
        uint16_t      fIndex1, fIndex2, fIndex3, fSpanIndex;
        hsPoint3    fCenter;

        void    Read( hsStream *s );
        void    Write( hsStream *s );
};

//// plGBufferCell and plGBufferColor Definitions /////////////////////////////

class plGBufferCell 
{
    public:
        uint32_t  fVtxStart;      // In bytes
        uint32_t  fColorStart;    // In bytes
        uint32_t  fLength;

        plGBufferCell( uint32_t vStart, uint32_t cStart, uint32_t len )
        {
            fVtxStart = vStart; fColorStart = cStart; fLength = len;
        }

        plGBufferCell() {}

        void    Read( hsStream *s );
        void    Write(hsStream *s) const;
};

class plGBufferColor
{
    public:
        uint32_t  fDiffuse, fSpecular;
};

//// plGBufferGroup Class Definition //////////////////////////////////////////
//
//  Represents a list of vertex and index buffers in a nice package.

class hsStream;
class hsResMgr;
class plPipeline;
class hsGDeviceRef;
class plGeometrySpan;

class plGBufferGroup
{
    protected:
        uint8_t   fFormat;
        uint8_t   fStride;
        uint8_t   fLiteStride;
        uint8_t   fNumSkinWeights;
        uint32_t  fNumVerts;
        uint32_t  fNumIndices;
        bool    fVertsVolatile;
        bool    fIdxVolatile;
        int     fLOD;

        std::vector<hsGDeviceRef*> fVertexBufferRefs;
        std::vector<hsGDeviceRef*> fIndexBufferRefs;

        std::vector<uint32_t>  fVertBuffSizes;
        std::vector<uint32_t>  fIdxBuffCounts;
        std::vector<uint32_t>  fColorBuffCounts;
        std::vector<uint8_t *> fVertBuffStorage;
        std::vector<uint16_t*> fIdxBuffStorage;

        std::vector<uint32_t> fVertBuffStarts;
        std::vector<int32_t>  fVertBuffEnds;
        std::vector<uint32_t> fIdxBuffStarts;
        std::vector<int32_t>  fIdxBuffEnds;

        std::vector<plGBufferColor*>  fColorBuffStorage;

        std::vector<std::vector<plGBufferCell>> fCells;

        virtual void    ISendStorageToBuffers( plPipeline *pipe, bool adjustForNvidiaLighting );

        uint8_t           ICalcVertexSize( uint8_t &liteStride );

        uint8_t* IVertBuffStorage(int iBuff, int iVtx) const { return fVertBuffStorage[iBuff] + iVtx*fStride; }

        uint32_t  IMakeCell( uint32_t vbIndex, uint8_t flags, uint32_t vStart, uint32_t cStart, uint32_t len, uint32_t *offset );
        void    IGetStartVtxPointer( uint32_t vbIndex, uint32_t cell, uint32_t offset, uint8_t *&tempPtr, plGBufferColor *&cPtr );

    public:

        static const uint32_t     kMaxNumVertsPerBuffer;
        static const uint32_t     kMaxNumIndicesPerBuffer;

        enum Formats
        {
            kUVCountMask    = 0x0f, // Problem is, we need enough bits to store the max #, which means
                                    // we really want ( max # << 1 ) - 1

            kSkinNoWeights  = 0x00, // 0000000
            kSkin1Weight    = 0x10, // 0010000
            kSkin2Weights   = 0x20, // 0100000
            kSkin3Weights   = 0x30, // 0110000
            kSkinWeightMask = 0x30, // 0110000

            kSkinIndices    = 0x40,  // 1000000

            kEncoded        = 0x80  
        };

        enum
        {
            kReserveInterleaved = 0x01,
            kReserveVerts       = 0x02,
            kReserveColors      = 0x04,
            kReserveSeparated   = 0x08,
            kReserveIsolate     = 0x10
        };

        plGBufferGroup(uint8_t format, bool vertsVolatile, bool idxVolatile, int LOD = 0);
        virtual ~plGBufferGroup();

        uint8_t   GetNumUVs() const { return ( fFormat & kUVCountMask ); }
        uint8_t   GetNumWeights() const { return (fFormat & kSkinWeightMask) >> 4; }

        static uint8_t    CalcNumUVs( uint8_t format ) { return ( format & kUVCountMask ); }
        static uint8_t    UVCountToFormat( uint8_t numUVs ) { return numUVs & kUVCountMask; }

        void    DirtyVertexBuffer(size_t i);
        void    DirtyIndexBuffer(size_t i);
        bool    VertexReady(size_t i) const { return (i < fVertexBufferRefs.size()) && fVertexBufferRefs[i]; }
        bool    IndexReady(size_t i) const { return  (i < fIndexBufferRefs.size()) && fIndexBufferRefs[i]; }
        uint8_t   GetVertexSize() const { return fStride; }
        uint8_t   GetVertexLiteStride() const { return fLiteStride; }
        uint8_t   GetVertexFormat() const { return fFormat; }
        uint32_t  GetMemUsage() const { return ( fNumVerts * GetVertexSize() ) + ( fNumIndices * sizeof( uint16_t ) ); }
        uint32_t  GetNumVerts() const { return fNumVerts; }
        uint32_t  GetNumIndices() const { return fNumIndices; }
        uint32_t  GetNumPrimaryVertsLeft() const;
        uint32_t  GetNumVertsLeft( uint32_t idx ) const;

        uint32_t  GetVertBufferSize(uint32_t idx) const { return fVertBuffSizes[idx]; }
        uint32_t  GetVertBufferCount(uint32_t idx) const;
        uint32_t  GetIndexBufferCount(uint32_t idx) const { return fIdxBuffCounts[idx]; }
        uint32_t  GetVertStartFromCell(uint32_t idx, uint32_t cell, uint32_t offset) const;

        // These should only be called by the pipeline, because only it knows when it's safe.
        // If the data is volatile, these are no-ops
        void PurgeVertBuffer(uint32_t idx);
        void PurgeIndexBuffer(uint32_t idx);

        ///////////////////////////////////////////////////////////////////////////////
        // The following group of functions is an advanced optimization, and a pretty 
        // specialized one at that. It just limits the amount of data that will get
        // uploaded to video. If you don't know you are limited by bandwidth to the
        // board, or you just don't know what your are doing, don't mess with them.
        // If you never touch them, everything will work. If you set them correcly,
        // things may work faster. If you set them incorrectly, be sure to save
        // all files before running.
        // All of these are indices, not bytes. from the beginning of the buffer. 
        uint32_t  GetVertBufferStart(uint32_t idx) const { return fVertBuffStarts[idx]; }
        uint32_t  GetVertBufferEnd(uint32_t idx) const { return fVertBuffEnds[idx] >= 0 ? uint32_t(fVertBuffEnds[idx]) : GetVertBufferCount(idx); }
        uint32_t  GetIndexBufferStart(uint32_t idx) const { return fIdxBuffStarts[idx]; }
        uint32_t  GetIndexBufferEnd(uint32_t idx) const { return fIdxBuffEnds[idx] >= 0 ? uint32_t(fIdxBuffEnds[idx]) : GetIndexBufferCount(idx); }

        void    SetVertBufferStart(uint32_t idx, uint32_t s) { fVertBuffStarts[idx] = s; }
        void    SetVertBufferEnd(uint32_t idx, uint32_t e) { fVertBuffEnds[idx] = e; }
        void    SetIndexBufferStart(uint32_t idx, uint32_t s) { fIdxBuffStarts[idx] = s; }
        void    SetIndexBufferEnd(uint32_t idx, uint32_t e) { fIdxBuffEnds[idx] = e; }
        ///////////////////////////////////////////////////////////////////////////////

        uint32_t  GetNumVertexBuffers() const { return fVertBuffStorage.size(); }
        uint32_t  GetNumIndexBuffers() const { return fIdxBuffStorage.size(); }

        uint8_t           *GetVertBufferData( uint32_t idx ) { return fVertBuffStorage[ idx ]; }
        uint16_t          *GetIndexBufferData( uint32_t idx ) { return fIdxBuffStorage[ idx ]; }
        plGBufferColor  *GetColorBufferData( size_t idx ) { return fColorBuffStorage[ idx ]; }

        hsGDeviceRef    *GetVertexBufferRef( uint32_t i );
        hsGDeviceRef    *GetIndexBufferRef( uint32_t i );

        size_t          GetNumCells( size_t idx ) const { return fCells[ idx ].size(); }
        plGBufferCell   *GetCell( size_t idx, size_t cell ) { return &(fCells[ idx ][ cell ]); }

        void    SetVertexBufferRef( uint32_t index, hsGDeviceRef *vb );
        void    SetIndexBufferRef( uint32_t index, hsGDeviceRef *ib );

        virtual void Read( hsStream* s );
        virtual void Write( hsStream* s );

        // Accessor functions
        hsPoint3    &Position( int iBuff, uint32_t cell, int iVtx );
        hsVector3   &Normal( int iBuff, uint32_t cell, int iVtx );
        uint32_t      &Color( int iBuff, uint32_t cell, int iVtx );
        uint32_t      &Specular( int iBuff, uint32_t cell, int iVtx );
        hsPoint3    &UV( int iBuff, uint32_t cell, int iVtx, int channel );
        uint32_t      Format() const { return fFormat; }

        // Take temp accumulators and actually build buffer data from them
        void    TidyUp();

        // Delete the buffer data storage
        void    CleanUp();

        // Take buffer data and convert it to device-specific buffers
        void    PrepForRendering( plPipeline *pipe, bool adjustForNvidiaLighting );

        // Reserves space in a vertex buffer
        bool    ReserveVertStorage( uint32_t numVerts, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset, uint8_t flags );

        // Append vertex data to the first available storage buffer
        void    AppendToVertStorage( plGeometrySpan *srcSpan, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset );
        void    AppendToVertAndColorStorage( plGeometrySpan *srcSpan, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset );
        void    AppendToColorStorage( plGeometrySpan *srcSpan, uint32_t *vbIndex, uint32_t *cell, uint32_t *offset, uint32_t origCell );

        // Reserves space in an index buffer
        bool    ReserveIndexStorage(uint32_t numIndices, uint32_t *ibIndex, uint32_t *ibStart, uint16_t **dataPtr = nullptr);

        // Append index data to the first available storage buffer
        void    AppendToIndexStorage( uint32_t numIndices, uint16_t *data, uint32_t addToAll, uint32_t *ibIndex, uint32_t *ibStart );


        /// Dynamic functions (addition/deletion of raw data)
        void    DeleteVertsFromStorage( uint32_t which, uint32_t start, uint32_t length );
        void    AdjustIndicesInStorage( uint32_t which, uint16_t threshhold, int16_t delta );
        void    DeleteIndicesFromStorage( uint32_t which, uint32_t start, uint32_t length );

        // Returns an array of plGBufferTriangles representing the span of indices specified
        plGBufferTriangle   *ConvertToTriList( int16_t spanIndex, uint32_t whichIdx, uint32_t whichVtx, uint32_t whichCell, uint32_t start, uint32_t numTriangles );

        // Stuffs the indices from an array of plGBufferTriangles into the index storage
        void    StuffFromTriList( uint32_t which, uint32_t start, uint32_t numTriangles, uint16_t *data );
        void    StuffTri( uint32_t iBuff, uint32_t iTri, uint16_t idx0, uint16_t idx1, uint16_t idx2 );

        // Stuff the data from a geometry span into vertex storage
        void    StuffToVertStorage( plGeometrySpan *srcSpan, uint32_t vbIndex, uint32_t cell, uint32_t offset, uint8_t flags );

        // Are our verts volatile?
        bool    AreVertsVolatile() const { return fVertsVolatile; }
        bool    AreIdxVolatile() const { return fIdxVolatile; }

        int GetLOD() const { return fLOD; }
};

#endif // _plGBufferGroup_h

