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
#include "plZlibStream.h"
#include <zlib.h>

voidpf ZlibAlloc(voidpf opaque, uInt items, uInt size)
{
    return malloc(items*size);
}

void ZlibFree(voidpf opaque, voidpf address)
{
    free(address);
}

plZlibStream::~plZlibStream()
{
    hsAssert(!fOutput && !fZStream, "plZlibStream not closed");
}

bool plZlibStream::Open(const plFileName& filename, const char* mode)
{
    fFilename = filename;
    fMode = mode;

    fOutput = new hsUNIXStream;
    return fOutput->Open(filename, "wb");
}

bool plZlibStream::Close()
{
    if (fOutput)
    {
        fOutput->Close();
        delete fOutput;
        fOutput = nullptr;
    }
    if (fZStream)
    {
        z_streamp zstream = (z_streamp)fZStream;
        inflateEnd(zstream);
        delete zstream;
        fZStream = nullptr;
    }

    return true;
}

uint32_t plZlibStream::Write(uint32_t byteCount, const void* buffer)
{
    uint8_t* byteBuf = (uint8_t*)buffer;

    // Check if we've read in the full gzip header yet
    if (fHeader == kNeedMoreData)
    {
        int cutAmt = IValidateGzHeader(byteCount, buffer);

        // Once we've read in the whole header, cut out whatever part of it is
        // in this buffer, leaving raw compressed data
        if (cutAmt != 0)
        {
            byteCount -= cutAmt;
            byteBuf += cutAmt;
        }
    }

    if (fHeader == kInvalidHeader)
        return 0;

    if (fHeader == kValidHeader)
    {
        ASSERT(fOutput);
        ASSERT(fZStream);
        z_streamp zstream = (z_streamp)fZStream;
        zstream->avail_in = byteCount;
        zstream->next_in = byteBuf;

        char outBuf[2048];
        while (zstream->avail_in != 0)
        {
            zstream->avail_out = sizeof(outBuf);
            zstream->next_out = (uint8_t*)outBuf;

            uint32_t amtWritten = zstream->total_out;

            int ret = inflate(zstream, Z_NO_FLUSH);

            bool inflateErr = (ret == Z_NEED_DICT || ret == Z_DATA_ERROR ||
                                ret == Z_STREAM_ERROR || ret == Z_MEM_ERROR || ret == Z_BUF_ERROR);
            // If we have a decompression error, just fail
            if (inflateErr)
            {
                hsAssert(!inflateErr, "Error in inflate");
                fHeader = kInvalidHeader;
                break;
            }

            amtWritten = zstream->total_out - amtWritten;
            fOutput->Write(amtWritten, outBuf);

            // If zlib says we hit the end of the stream, ignore avail_in
            if (ret == Z_STREAM_END)
            {
                fDecompressedOk = true;
                break;
            }
        }
    }

    return byteCount;
}

int plZlibStream::IValidateGzHeader(uint32_t byteCount, const void* buffer)
{
    // Ripped from gzio.cpp
    #define HEAD_CRC     0x02
    #define EXTRA_FIELD  0x04
    #define ORIG_NAME    0x08
    #define COMMENT      0x10
    #define RESERVED     0xE0
    static int gz_magic[2] = {0x1f, 0x8b}; // gzip magic header
    
    #define CheckForEnd() if (s.AtEnd()) { fHeader = kNeedMoreData; return 0; }

    int i;

    int initCacheSize = fHeaderCache.size();
    for (i = 0; i < byteCount; i++)
        fHeaderCache.push_back(((uint8_t*)buffer)[i]);
    hsReadOnlyStream s(fHeaderCache.size(), &fHeaderCache[0]);
    
    // Check the gzip magic header
    for (i = 0; i < 2; i++)
    {
        uint8_t c = s.ReadByte();
        if (c != gz_magic[i])
        {
            CheckForEnd();
            fHeader = kInvalidHeader;
            return 0;
        }
    }
    
    int method = s.ReadByte();
    int flags = s.ReadByte();
    if (method != Z_DEFLATED || (flags & RESERVED) != 0)
    {
        CheckForEnd();
        fHeader = kInvalidHeader;
        return 0;
    }
    
    // Discard time, xflags and OS code:
    for (i = 0; i < 6; i++)
        (void)s.ReadByte();
    CheckForEnd();
    
    if ((flags & EXTRA_FIELD) != 0)
    {
        // skip the extra field
        uint16_t len = s.ReadLE16();
        while (len-- != 0 && s.ReadByte())
        {
            CheckForEnd();
        }
    }
    
    int c;

    // skip the original file name
    if ((flags & ORIG_NAME) != 0)
    {
        while ((c = s.ReadByte()) != 0)
        {
            CheckForEnd();
        }
    }
    
    // skip the .gz file comment
    if ((flags & COMMENT) != 0)
    {
        while ((c = s.ReadByte()) != 0)
        {
            CheckForEnd();
        }
    }

    // skip the header crc
    if ((flags & HEAD_CRC) != 0)
    {
        (void)s.ReadLE16();
        CheckForEnd();
    }
    
    CheckForEnd();
    
    uint32_t headerSize = s.GetPosition();
    uint32_t clipBuffer = headerSize - initCacheSize;
    
    // Initialize the zlib stream
    z_streamp zstream = new z_stream_s;
    memset(zstream, 0, sizeof(z_stream_s));
    zstream->zalloc = ZlibAlloc;
    zstream->zfree = ZlibFree;
    zstream->opaque = nullptr;
    zstream->avail_in = byteCount - clipBuffer;
    zstream->next_in = (uint8_t*)&fHeaderCache[clipBuffer];
    // Gotta use inflateInit2, because there's no header for zlib to look at.
    // The -15 tells it to not try and find a header
    bool initOk = (inflateInit2(zstream, -15) == Z_OK);
    fZStream = zstream;

    fHeaderCache.clear();

    if (!initOk)
    {
        hsAssert(0, "Zip init failed");
        fHeader = kInvalidHeader;
        return 0;
    }
    ASSERT(fZStream);

    fHeader = kValidHeader;
    return clipBuffer;
}

bool plZlibStream::AtEnd()
{
    hsAssert(0, "AtEnd not supported");
    return true;
}

uint32_t plZlibStream::Read(uint32_t byteCount, void* buffer)
{
    hsAssert(0, "Read not supported");
    return 0;
}

void plZlibStream::Skip(uint32_t deltaByteCount)
{
    hsAssert(0, "Skip not supported");
}

void plZlibStream::Rewind()
{
    // hack so rewind will work (someone thought it would be funny to not implement base class functions)
    Close();
    Open(fFilename, fMode);
    fHeader = kNeedMoreData;
    fDecompressedOk = false;
}

void plZlibStream::FastFwd()
{
    hsAssert(0, "FastFwd not supported");
}

uint32_t plZlibStream::GetEOF()
{
    hsAssert(0, "GetEOF not supported");
    return 0;
}
