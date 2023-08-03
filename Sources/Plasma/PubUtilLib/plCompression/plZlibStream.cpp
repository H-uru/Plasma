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

    if (fErrorOccurred) {
        return 0;
    }

    if (fZStream == nullptr) {
        // Initialize the zlib stream
        z_streamp zstream = new z_stream_s;
        memset(zstream, 0, sizeof(z_stream_s));
        zstream->zalloc = ZlibAlloc;
        zstream->zfree = ZlibFree;
        zstream->opaque = nullptr;
        zstream->avail_in = byteCount;
        zstream->next_in = byteBuf;
        // windowBits = 31 means require a gzip header and window size 15.
        bool initOk = (inflateInit2(zstream, 31) == Z_OK);
        fZStream = zstream;

        if (!initOk) {
            hsAssert(0, "Zip init failed");
            fErrorOccurred = true;
            return 0;
        }
    }

    ASSERT(fOutput);
    ASSERT(fZStream);
    z_streamp zstream = (z_streamp)fZStream;
    zstream->avail_in = byteCount;
    zstream->next_in = byteBuf;

    char outBuf[2048];
    while (zstream->avail_in != 0) {
        zstream->avail_out = sizeof(outBuf);
        zstream->next_out = (uint8_t*)outBuf;

        uint32_t amtWritten = zstream->total_out;

        int ret = inflate(zstream, Z_NO_FLUSH);

        bool inflateErr = (ret == Z_NEED_DICT || ret == Z_DATA_ERROR ||
                            ret == Z_STREAM_ERROR || ret == Z_MEM_ERROR || ret == Z_BUF_ERROR);
        // If we have a decompression error, just fail
        if (inflateErr) {
            hsAssert(!inflateErr, "Error in inflate");
            fErrorOccurred = true;
            break;
        }

        amtWritten = zstream->total_out - amtWritten;
        fOutput->Write(amtWritten, outBuf);

        // If zlib says we hit the end of the stream, ignore avail_in
        if (ret == Z_STREAM_END) {
            fDecompressedOk = true;
            break;
        }
    }

    return byteCount;
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
    fErrorOccurred = false;
    fDecompressedOk = false;
}

void plZlibStream::FastFwd()
{
    hsAssert(0, "FastFwd not supported");
}

void plZlibStream::Truncate()
{
    hsAssert(false, "Truncate not supported");
}

uint32_t plZlibStream::GetEOF()
{
    hsAssert(0, "GetEOF not supported");
    return 0;
}
