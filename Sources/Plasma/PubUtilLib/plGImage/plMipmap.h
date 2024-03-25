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
//  plMipmap Class Header                                                    //
//  Derived bitmap class representing a single mipmap.                       //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plMipmap_h
#define _plMipmap_h

#include "plBitmap.h"

#ifdef HS_DEBUGGING
    #define ASSERT_PIXELSIZE(bitmap, pixelsize)     hsAssert((bitmap)->fPixelSize == (pixelsize), "pixelSize mismatch")
    #define ASSERT_XY(bitmap, x, y)                 hsAssert(x < (bitmap)->fWidth && y < (bitmap)->fHeight, "bad XY")
    #define ASSERT_UNCOMPRESSED()                   hsAssert(fCompressionType!=kDirectXCompression, "Can't operate on compressed map.")

    // Define the following konstant to enable mipmap leak checking. This is because our normal
    // memory manager sucks when trying to track down these problems
    #define MEMORY_LEAK_TRACER
#else
    #define ASSERT_PIXELSIZE(bitmap, pixelsize)
    #define ASSERT_XY(bitmap, x, y)
    #define ASSERT_UNCOMPRESSED()               
#endif

#ifdef MEMORY_LEAK_TRACER
    #include <string_theory/string>
#endif

//// Class Definition /////////////////////////////////////////////////////////

class plBitmapCreator;

class plMipmap : public plBitmap
{
    friend class plBitmapCreator;

    public:
        //// Public Flags ////


        //// Public Data /////
        
        
        //// Public Members ////


        plMipmap();
        plMipmap( uint32_t width, uint32_t height, unsigned config, uint8_t numLevels = 0, uint8_t compType = kUncompressed, uint8_t format = UncompressedInfo::kRGB8888 );
        plMipmap( plMipmap *bm, float sig, uint32_t createFlags, 
                            float detailDropoffStart, float detailDropoffStop, 
                            float detailMax, float detailMin );
        virtual ~plMipmap();

        CLASSNAME_REGISTER( plMipmap );
        GETINTERFACE_ANY( plMipmap, plBitmap );


        void            Create( uint32_t width, uint32_t height, unsigned config, uint8_t numLevels, uint8_t compType = kUncompressed, uint8_t format = UncompressedInfo::kRGB8888 );

        virtual void    Reset();

        // Get the total size in bytes
        uint32_t  GetTotalSize() const override;

        void    Read(hsStream *s, hsResMgr *mgr) override { hsKeyedObject::Read(s, mgr); this->Read(s); }
        void    Write(hsStream *s, hsResMgr *mgr) override { hsKeyedObject::Write(s, mgr); this->Write(s); }

        virtual uint8_t   GetNumLevels() const { return fNumLevels; }
        virtual uint32_t  GetLevelSize( uint8_t level );        // 0 is the largest

        virtual void        Colorize();
        virtual plMipmap    *Clone() const;
        virtual void        CopyFrom( const plMipmap *source );

        inline uint32_t   GetWidth() const { return fWidth; }
        inline uint32_t   GetHeight() const { return fHeight; }
        inline uint32_t   GetRowBytes() const { return fRowBytes; }

        void            *GetImage() const { return fImage; }
        void            SetImagePtr( void *ptr ) { fImage = ptr; }
        uint8_t           *GetLevelPtr(uint8_t level, uint32_t *width = nullptr, uint32_t *height = nullptr, uint32_t *rowBytes = nullptr);

        // Sets the current level pointer for use with GetAddr*
        virtual void    SetCurrLevel(uint8_t level);
        void            *GetCurrLevelPtr() const { return fCurrLevelPtr; }
        uint32_t          GetCurrWidth() const { return fCurrLevelWidth; }
        uint32_t          GetCurrHeight() const { return fCurrLevelHeight; }
        uint32_t          GetCurrLevelSize() const { return fLevelSizes[ fCurrLevel ]; }
        uint32_t          GetCurrLevel() const { return fCurrLevel; }

        //  These methods return the address of the pixel specified by x and y
        //  They are meant to be fast, therefore they are inlined and do not check
        //  the fPixelSize field at runtime (except when debugging)

        uint8_t*  GetAddr8(unsigned x, unsigned y) const
                {
                    ASSERT_PIXELSIZE(this, 8);
                    ASSERT_XY(this, x, y);
                    ASSERT_UNCOMPRESSED();
                    return (uint8_t*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + x);
                }
        uint16_t* GetAddr16(unsigned x, unsigned y) const
                {
                    ASSERT_PIXELSIZE(this, 16);
                    ASSERT_XY(this, x, y);
                    ASSERT_UNCOMPRESSED();
                    return (uint16_t*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + (x << 1));
                }
        uint32_t* GetAddr32(unsigned x, unsigned y) const
                {
                    ASSERT_PIXELSIZE(this, 32);
                    ASSERT_XY(this, x, y);
                    ASSERT_UNCOMPRESSED();
                    return (uint32_t*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + (x << 2));
                }
        void*   GetAddr64(unsigned x, unsigned y) const
                {
                    ASSERT_PIXELSIZE(this, 64);
                    ASSERT_XY(this, x, y);
                    ASSERT_UNCOMPRESSED();
                    return (void*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + (x << 3));
                }

        //  This sets fPixelSize, fSpace, fFlags, for you
        //  All you need to set is
        //      fWidth, fHeight, fRowBytes, fImage and fColorTable
        enum {
            kColor8Config = 0,
            kGray44Config = 1,
            kGray4Config = 2,
            kGray8Config = 8,       // So we can use bit depths instead
            kRGB16Config = 16,
            kRGB32Config = 24,
            kARGB32Config = 32,
        };

        void    SetConfig( unsigned config );


        //// Really complex creation stuff ////

        enum {
            kCreateDetailAlpha      = 0x1,
            kCreateDetailAdd        = 0x2,
            kCreateDetailMult       = 0x4,
            kCreateDetailMask       = kCreateDetailAlpha | kCreateDetailAdd | kCreateDetailMult,
            kCreateCarryAlpha       = 0x8,
            kCreateCarryWhite       = 0x10,
            kCreateCarryBlack       = 0x20,
            kCreateCarryMask        = kCreateCarryAlpha | kCreateCarryWhite | kCreateCarryBlack
        };
        enum hsGPixelType {
            kPixelARGB4444,
            kPixelARGB1555,
            kPixelAI88,
            kPixelI8
        };
        enum hsGCopyOptions {
            kCopyLODMask,
        };

        enum {
            kColorDataRLE   = 0x1,
            kAlphaDataRLE   = 0x2
        };

        void    SetBitmapAsLevel(uint8_t iDst, plMipmap *bm, float sig, uint32_t createFlags, 
                                              float detailDropoffStart, float detailDropoffStop, 
                                              float detailMax, float detailMin);
        void    ICreateLevelNoDetail(uint8_t iDst, const plFilterMask& mask);
        void    IBlendLevelDetailAlpha(uint8_t iDst, const plFilterMask& mask, 
                                          float detailDropoffStart, float detailDropoffStop, 
                                          float detailMax, float detailMin);
        void    IBlendLevelDetailAdd(uint8_t iDst, const plFilterMask& mask, 
                                          float detailDropoffStart, float detailDropoffStop, 
                                          float detailMax, float detailMin);
        void    IBlendLevelDetailMult(uint8_t iDst, const plFilterMask& mask, 
                                          float detailDropoffStart, float detailDropoffStop, 
                                          float detailMax, float detailMin);
        void    Filter(float sig);
        uint32_t  CopyOutPixels(uint32_t destXSize, uint32_t destYSize, uint32_t dstFormat, void *destPixels, uint32_t copyOptions);

        void    ClipToMaxSize( uint32_t maxDimension );
        void    RemoveMipping();

        void    EnsureKonstantBorder( bool clampU, bool clampV );

        enum CompositeFlags
        {
            kForceOpaque        = 0x0001,       // Copy src pixels raw, force dest alphas to opaque
            kCopySrcAlpha       = 0x0002,       // Copy the src pixels raw, including alphas, overwrite dest
            kBlendSrcAlpha      = 0x0004,       // Blend src pixels onto dest using src alpha, dest alpha = src alpha
            kMaskSrcAlpha       = 0x0008,       // Same as copySrcAlpha, but dest is untouched when src alpha = 0
            kBlendWriteAlpha    = 0x0010,       // Like default (0), but writes dest alpha values

            kDestPremultiplied  = 0x0020,       // Dest has color premultiplied by alpha
                                                // (src always assumed nonpremultiplied for now)
        };

        class CompositeOptions
        {
            // Helper class for specifying options to Composite()
            public:
                uint16_t      fFlags;
                uint8_t       fSrcLevelsToSkip;
                uint8_t       fOpacity;
                float    fRedTint, fGreenTint, fBlueTint;
                uint16_t      fSrcClipX, fSrcClipY;           // Clipping is applied AFTER levelSkip
                uint16_t      fSrcClipWidth, fSrcClipHeight;  // 0 means max width/height

                CompositeOptions() { fFlags = 0; fSrcLevelsToSkip = 0; fRedTint = fGreenTint = fBlueTint = 1.f;
                                    fSrcClipX = fSrcClipY = fSrcClipWidth = fSrcClipHeight = 0; fOpacity = 255;}

                CompositeOptions( uint16_t flags, uint8_t srcLevelsToSkip = 0, float red = 1.f, float green = 1.f,
                                    float blue = 1.f, uint16_t srcClipX = 0, uint16_t srcClipY = 0, 
                                    uint16_t srcClipWidth = 0, uint16_t srcClipHeight = 0, uint8_t opacity = 255 ) 
                {
                    fFlags = flags;
                    fSrcLevelsToSkip = srcLevelsToSkip;
                    fRedTint = red;
                    fGreenTint = green;
                    fBlueTint = blue;
                    fSrcClipX = srcClipX;
                    fSrcClipY = srcClipY;
                    fSrcClipWidth = srcClipWidth;
                    fSrcClipHeight = srcClipHeight;
                    fOpacity = opacity;
                }
        };

        // Compositing function. Take a (smaller) mipmap and composite it onto this one at the given location. Nil options means use default
        virtual void    Composite(plMipmap *source, uint16_t x, uint16_t y, CompositeOptions *options = nullptr);

        // Scaling function
        enum ScaleFilter
        {
            kBoxFilter,
            kDefaultFilter = kBoxFilter
        };

        virtual void    ScaleNicely( uint32_t *destPtr, uint16_t destWidth, uint16_t destHeight,
                                uint16_t destStride, plMipmap::ScaleFilter filter ) const;

        virtual bool    ResizeNicely( uint16_t newWidth, uint16_t newHeight, plMipmap::ScaleFilter filter );

    protected:

        //// Protected Members ////

        void        *fImage;
        uint32_t    fWidth, fHeight, fRowBytes, fTotalSize;
        uint8_t     fNumLevels;
        uint32_t    *fLevelSizes;

        void        *fCurrLevelPtr;
        uint8_t     fCurrLevel;
        uint32_t    fCurrLevelWidth, fCurrLevelHeight, fCurrLevelRowBytes;

        void    IReadRawImage( hsStream *stream );
        void    IWriteRawImage( hsStream *stream );
        plMipmap *ISplitAlpha();
        void    IRecombineAlpha( plMipmap *alphaChannel );
        plMipmap *IReadRLEImage( hsStream *stream );
        void    IWriteRLEImage( hsStream *stream, plMipmap *mipmap );
        void    IReadJPEGImage( hsStream *stream );
        void    IWriteJPEGImage( hsStream *stream );
        void    IReadPNGImage( hsStream *stream );
        void    IWritePNGImage( hsStream *stream );
        void    IBuildLevelSizes();

        void    IColorLevel( uint8_t level, const uint8_t *colorMask );

        float    IGetDetailLevelAlpha( uint8_t level, float dropStart, float dropStop, float min, float max );

        void        ICarryZeroAlpha(uint8_t iDst);
        void        ICarryColor(uint8_t iDst, uint32_t col);

        bool        IGrabBorderColor( bool grabVNotU, uint32_t *color );
        void        ISetCurrLevelUBorder( uint32_t color );
        void        ISetCurrLevelVBorder( uint32_t color );

        uint32_t  Read(hsStream *s) override;
        uint32_t  Write(hsStream *s) override;

        friend class plCubicEnvironmap;

#ifdef MEMORY_LEAK_TRACER

    protected:
    
        class plRecord
        {
            public:
                plRecord    *fNext;
                plRecord    **fBackPtr;

                ST::string  fKeyName;
                void        *fImage;
                uint32_t    fWidth, fHeight, fRowBytes;
                uint8_t     fNumLevels;
                uint8_t     fCompressionType;
                union 
                {
                    DirectXInfo         fDirectXInfo;
                    UncompressedInfo    fUncompressedInfo;
                };
                enum Method
                {
                    kViaCreate,
                    kViaRead,
                    kViaClipToMaxSize,
                    kViaDetailMapConstructor,
                    kViaCopyFrom,
                    kViaResize
                } fCreationMethod;

                void    Link( plRecord **backPtr )
                {
                    fBackPtr = backPtr;
                    fNext = *backPtr;
                    if (fNext != nullptr)
                        fNext->fBackPtr = &fNext;
                    *backPtr = this;
                }

                void    Unlink()
                {
                    *fBackPtr = fNext;
                    if (fNext != nullptr)
                        fNext->fBackPtr = fBackPtr;
                }
        };

        static plRecord *fRecords;
        static uint32_t   fNumMipmaps;

        static void IAddToMemRecord( plMipmap *mip, plRecord::Method method );
        static void IRemoveFromMemRecord( uint8_t *image );
        static void IReportLeaks();

#endif
};


#endif // _plMipmap_h
