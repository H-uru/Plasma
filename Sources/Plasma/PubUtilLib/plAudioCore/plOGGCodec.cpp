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
//  plOGGCodec - Plasma codec support for the OGG/Vorbis file format.       //
//                                                                          //
//// Notes ///////////////////////////////////////////////////////////////////
//                                                                          //
//  2.7.2003 - Created by mcn. If only life were really this simple...      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include <cmath>
#include <string_theory/format>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "HeadSpin.h"
#include "plOGGCodec.h"

#include "hsTimer.h"

plOGGCodec::DecodeFormat    plOGGCodec::fDecodeFormat = plOGGCodec::k16bitSigned;
uint8_t                       plOGGCodec::fDecodeFlags = 0;

#define VORBIS_ERROR_CASE(constant, message) case constant: return ST_LITERAL("[" #constant "] " message)
static ST::string IFormatVorbisErrorMessage(int vorbisError) {
    // List of libvorbis error codes:
    // https://www.xiph.org/vorbis/doc/libvorbis/return.html
    switch (vorbisError) {
        VORBIS_ERROR_CASE(OV_FALSE, "Not true, or no data available");
        VORBIS_ERROR_CASE(OV_EOF, "End of file"); // not documented on website
        VORBIS_ERROR_CASE(OV_HOLE, "Missing or corrupt data in the bitstream");
        VORBIS_ERROR_CASE(OV_EREAD, "Read error while fetching compressed data for decode");
        VORBIS_ERROR_CASE(OV_EFAULT, "Internal inconsistency in encode or decode state");
        VORBIS_ERROR_CASE(OV_EIMPL, "Feature not implemented");
        VORBIS_ERROR_CASE(OV_EINVAL, "Either an invalid argument, or incompletely initialized argument passed to a call");
        VORBIS_ERROR_CASE(OV_ENOTVORBIS, "The given file/data was not recognized as Ogg Vorbis data");
        VORBIS_ERROR_CASE(OV_EBADHEADER, "The file/data is apparently an Ogg Vorbis stream, but contains a corrupted or undecipherable header");
        VORBIS_ERROR_CASE(OV_EVERSION, "The bitstream format revision of the given stream is not supported");
        VORBIS_ERROR_CASE(OV_ENOTAUDIO, "Not audio"); // not documented on website
        VORBIS_ERROR_CASE(OV_EBADPACKET, "Bad packet"); // not documented on website
        VORBIS_ERROR_CASE(OV_EBADLINK, "The given link exists in the Vorbis data stream, but is not decipherable due to garbage or corruption");
        VORBIS_ERROR_CASE(OV_ENOSEEK, "The given stream is not seekable");
        case 0: return ST_LITERAL("(no error)");
        default: return ST_LITERAL("(unknown Vorbis error code)");
    }
}
#undef VORBIS_ERROR_CASE

//// Constructor/Destructor //////////////////////////////////////////////////

plOGGCodec::plOGGCodec(const plFileName &path, plAudioCore::ChannelSelect whichChan)
    : fFileHandle(),
      fOggFile(),
      fCurHeaderPos(),
      fHeadBuf()
{
    IOpen( path, whichChan );
}

void    plOGGCodec::BuildActualWaveHeader()
{
    // Build an actual WAVE header for this ogg
    int fmtSize = 16;
    short fmt = 1;
    int factsize = 4;
    int factdata = 0;
    int size = fDataSize+48; // size of data with header except for first four bytes

    fHeadBuf = (uint8_t *)malloc(56);
    memcpy(fHeadBuf, "RIFF", 4);
    memcpy(fHeadBuf+4, &size, 4);
    memcpy(fHeadBuf+8, "WAVE", 4);
    memcpy(fHeadBuf+12, "fmt ", 4);
    memcpy(fHeadBuf+16, &fmtSize, 4);
    memcpy(fHeadBuf+20, &fmt, 2); /* format */
    memcpy(fHeadBuf+22, &fHeader.fNumChannels, 2);
    memcpy(fHeadBuf+24, &fHeader.fNumSamplesPerSec, 4);
    memcpy(fHeadBuf+28, &fHeader.fAvgBytesPerSec, 4);
    memcpy(fHeadBuf+32, &fHeader.fBlockAlign, 4);
    memcpy(fHeadBuf+34, &fHeader.fBitsPerSample, 2);
    memcpy(fHeadBuf+36, "fact", 4);
    memcpy(fHeadBuf+40, &factsize, 4);
    memcpy(fHeadBuf+44, &factdata, 4);
    memcpy(fHeadBuf+48, "data", 4);
    memcpy(fHeadBuf+52, &fDataSize, 4);
}

bool plOGGCodec::ReadFromHeader(int numBytes, void *data)
{
    if(fCurHeaderPos < 56)
    {
        memcpy(data, fHeadBuf+fCurHeaderPos, numBytes);
        fCurHeaderPos += numBytes;
        return true;
    }
    return false;
}

void    plOGGCodec::IOpen( const plFileName &path, plAudioCore::ChannelSelect whichChan )
{
    hsAssert( path.IsValid(), "Invalid path specified in plOGGCodec reader" );

    fFilename = path;
    fWhichChannel = whichChan;

    /// Open the file as a plain binary stream
    fFileHandle = plFileSystem::Open(path, "rb");
    if (fFileHandle != nullptr)
    {
        /// Create the OGG data struct
        fOggFile = new OggVorbis_File;

        /// Open the OGG decompressor
        int openRes = ov_open(fFileHandle, fOggFile, nullptr, 0);
        if (openRes < 0) {
            IError(openRes, ST::format("Unable to open OGG source file {}", path));
            return;
        }

        /// Construct some header info from the ogg info
        vorbis_info *vInfo = ov_info( fOggFile, -1 );

        fHeader.fFormatTag = 1;
        fHeader.fNumChannels = vInfo->channels;
        fHeader.fNumSamplesPerSec = vInfo->rate;
    
        // Funny thing about the bits per sample: we get to CHOOSE. Go figure! 
        fHeader.fBitsPerSample = ( fDecodeFormat == k8bitUnsigned ) ? 8 : 16;

        // Why WAV files hold this info when it can be calculated is beyond me...
        fHeader.fBlockAlign = ( fHeader.fBitsPerSample * fHeader.fNumChannels ) >> 3;
        fHeader.fAvgBytesPerSec = fHeader.fNumSamplesPerSec * fHeader.fBlockAlign;

        
        /// The size in bytes of our PCM data stream
        /// Note: OGG sometimes seems to be off by 1 sample, which causes our reads to suck
        /// because we end up waiting for 1 more sample than we actually have. So, on the
        /// assumption that OGG is just slightly wrong sometimes, we just subtract 1 sample
        /// from what it tells us. As Brice put it, who's going to miss 1/40,000'ths of a second?
        fDataSize = (uint32_t)(( ov_pcm_total( fOggFile, -1 ) - 1 ) * fHeader.fBlockAlign);

        /// Channel select
        if( fWhichChannel != plAudioCore::kAll )
        {
            fChannelAdjust = 2;
            fChannelOffset = ( fWhichChannel == plAudioCore::kLeft ) ? 0 : 1;
        }
        else
        {
            fChannelAdjust = 1;
            fChannelOffset = 0;
        }
        

        /// Construct our fake header for channel adjustment
        fFakeHeader = fHeader;
        fFakeHeader.fAvgBytesPerSec /= fChannelAdjust;
        fFakeHeader.fNumChannels /= (uint16_t)fChannelAdjust;
        fFakeHeader.fBlockAlign /= (uint16_t)fChannelAdjust;

        SetPosition( 0 );
    }
}

plOGGCodec::~plOGGCodec()
{
    Close();
}

void    plOGGCodec::Close()
{
    free(fHeadBuf);
    fHeadBuf = nullptr;
    if (fOggFile != nullptr)
    {
        ov_clear( fOggFile );
        delete fOggFile;
        fOggFile = nullptr;
        fFileHandle = nullptr; // ov_clear closes this
    }

    if (fFileHandle != nullptr)
    {
        fclose( fFileHandle );
        fFileHandle = nullptr;
    }
}

void plOGGCodec::IError(int vorbisError, const ST::string& message)
{
    ST::string fullMessage;
    if (vorbisError == 0) {
        fullMessage = message;
    } else {
        fullMessage = ST::format("{}: libvorbis error {}: {}", message, vorbisError, IFormatVorbisErrorMessage(vorbisError));
    }
    hsAssert(false, fullMessage.c_str());
    Close();
}

plWAVHeader &plOGGCodec::GetHeader()
{
    hsAssert( IsValid(), "GetHeader() called on an invalid OGG file" );

    return fFakeHeader;
}

float   plOGGCodec::GetLengthInSecs()
{
    hsAssert( IsValid(), "GetLengthInSecs() called on an invalid OGG file" );

    // Just query ogg directly...starting to see how cool ogg is yet?
    return (float)ov_time_total( fOggFile, -1 );
}

bool    plOGGCodec::SetPosition( uint32_t numBytes )
{
    hsAssert( IsValid(), "GetHeader() called on an invalid OGG file" );
    

    if( !ov_seekable( fOggFile ) )
    {
        hsAssert( false, "Trying to set position on an unseekable OGG stream!" );
        return false;
    }

    // The numBytes position is in uncompressed space and should be sample-aligned anyway,
    // so this should be just fine here.
    ogg_int64_t newSample = ( numBytes / (fFakeHeader.fBlockAlign * fChannelAdjust) );

    // Now please note how freaking easy it is here to do accurate or fast seeking...
    // Also note that if we're doing our channel extraction, we MUST do it the accurate way
    int seekRes;
    if( ( fDecodeFlags & kFastSeeking ) && fChannelAdjust == 1 )
    {
        seekRes = ov_pcm_seek_page(fOggFile, newSample);
    }
    else
    {
        seekRes = ov_pcm_seek(fOggFile, newSample);
    }
    if (seekRes != 0) {
        IError(seekRes, ST_LITERAL("Unable to seek OGG stream"));
        return false;
    }
    return true;
}

bool    plOGGCodec::Read( uint32_t numBytes, void *buffer )
{
    hsAssert( IsValid(), "GetHeader() called on an invalid OGG file" );

    int bytesPerSample = ( fDecodeFormat == k16bitSigned ) ? 2 : 1;
    int isSigned = ( fDecodeFormat == k16bitSigned ) ? 1 : 0;
    int currSection;
    
    if( fWhichChannel == plAudioCore::kAll )
    {
        // Easy, just a straight read
        char    *uBuffer = (char *)buffer;

        while( numBytes > 0 )
        {
            // Supposedly we should pay attention to currSection in case of bitrate changes,
            // but hopefully we'll never have those....

            long bytesRead = ov_read( fOggFile, uBuffer, numBytes, 0, bytesPerSample, isSigned, &currSection );
            
            // Since our job is so simple, do some extra error checking
            if (bytesRead == 0) {
                IError(0, ST_LITERAL("Unable to finish reading from OGG file: end of stream"));
                return false;
            } else if (bytesRead < 0) {
                IError(bytesRead, ST_LITERAL("Unable to read from OGG file"));
                return false;
            }

            numBytes -= bytesRead;
            uBuffer += bytesRead;
        }
    }
    else
    {
        /// Read in 4k chunks and extract
        static char     trashBuffer[ 4096 ];

        long    toRead, i, thisRead, sampleSize = fFakeHeader.fBlockAlign;

        for( ; numBytes > 0; )
        {
            /// Read 4k worth of samples
            toRead = ( sizeof( trashBuffer ) < numBytes * fChannelAdjust ) ? sizeof( trashBuffer ) : numBytes * fChannelAdjust;


            thisRead = ov_read( fOggFile, (char *)trashBuffer, toRead, 0, bytesPerSample, isSigned, &currSection );
            if( thisRead < 0 )
                return false;

            /// Copy every other sample out
            int sampleOffset = (fChannelOffset == 1) ? sampleSize : 0;
            for (i = 0; i < thisRead; i += sampleSize * 2)
            {
                memcpy(buffer, &trashBuffer[i + sampleOffset], sampleSize);
                buffer = (void*)((uint8_t*)buffer + sampleSize);

                numBytes -= sampleSize;
            }
        }
    }

    return true;
}

uint32_t  plOGGCodec::NumBytesLeft()
{
    if(!IsValid())
    {
        hsAssert( false, "GetHeader() called on an invalid OGG file" );
        return 0;
    }

    return (uint32_t)(( fDataSize - ( ov_pcm_tell( fOggFile ) * fHeader.fBlockAlign ) ) / fChannelAdjust);
}
