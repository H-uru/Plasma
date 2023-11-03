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
#include <string>
#include <ctime>

#include "plSecureStream.h"
#include "hsWindows.h"

#if !HS_BUILD_FOR_WIN32
#include <errno.h>
#define INVALID_HANDLE_VALUE nullptr
#endif

// our default encryption key
const uint32_t plSecureStream::kDefaultKey[4] = { 0x6c0a5452, 0x3827d0f, 0x3a170b92, 0x16db7fc2 };

static const int kEncryptChunkSize = 8;

static const char* kMagicString    = "notthedroids";
static const int kMagicStringLen = 12;

static const int kFileStartOffset = kMagicStringLen + sizeof(uint32_t);

static const int kMaxBufferedFileSize = 10*1024;

const char plSecureStream::kKeyFilename[] = "encryption.key";

plSecureStream::plSecureStream(bool deleteOnExit, uint32_t* key) :
fRef(),
fActualFileSize(),
fBufferedStream(),
fRAMStream(),
fOpenMode(kOpenFail),
fDeleteOnExit(deleteOnExit)
{
    if (key)
        memcpy(&fKey, key, sizeof(kDefaultKey));
    else
        memcpy(&fKey, &kDefaultKey, sizeof(kDefaultKey));
}

plSecureStream::plSecureStream(hsStream* base, uint32_t* key) :
fRef(),
fActualFileSize(),
fBufferedStream(),
fRAMStream(),
fOpenMode(kOpenFail),
fDeleteOnExit(false)
{
    if (key)
        memcpy(&fKey, key, sizeof(kDefaultKey));
    else
        memcpy(&fKey, &kDefaultKey, sizeof(kDefaultKey));
    Open(base);
}

plSecureStream::~plSecureStream()
{
    if (fOpenMode == kOpenWrite) {
        fRAMStream->Rewind();
        IWriteEncrypted(fRAMStream.get(), fWriteFileName);
    }

    if (fRef != INVALID_HANDLE_VALUE) {
#if defined(HS_BUILD_FOR_WIN32)
        CloseHandle(fRef);
#elif defined(HS_BUILD_FOR_UNIX)
        fclose(fRef);
#endif
    }
}

//
// XXTEA
// http://www-users.cs.york.ac.uk/~matthew/TEA/
//
// A potential weakness in this implementation is the fact that a known value
// (length of the original file) is written at the start of the encrypted file.  -Colin
//

#define MX (z>>5 ^ y<<2) + (y>>3 ^ z<<4) ^ (sum^y) + (fKey[p&3^e]^z)

void plSecureStream::IEncipher(uint32_t* const v, uint32_t n)
{
    unsigned long y=v[0], z=v[n-1], e, delta=0x9E3779B9;
    unsigned long q = 6 + 52/n, p, sum = 0;

    while (q-- > 0)
    {
        sum += delta;
        e = (sum >> 2) & 3;
        for (p = 0; p < n - 1; p++)
        {
            y = v[p + 1];
            v[p] += MX;
            z = v[p];
        }
        y = v[0];
        v[n - 1] += MX;
        z = v[n - 1];
    }
}

void plSecureStream::IDecipher(uint32_t* const v, uint32_t n)
{
    unsigned long y=v[0], z=v[n-1], e, delta=0x9E3779B9;
    unsigned long q = 6 + 52/n, p, sum = q * delta;

    while (sum > 0)
    {
        e = (sum >> 2) & 3;
        for (p = n - 1; p > 0; p--)
        {
            z = v[p - 1];
            v[p] -= MX;
            y = v[p];
        }
        z = v[n - 1];
        v[0] -= MX;
        y = v[0];
        sum -= delta;
    }
}

bool plSecureStream::Open(const plFileName& name, const char* mode)
{
    if (strcmp(mode, "rb") == 0)
    {
#if HS_BUILD_FOR_WIN32
        if (fDeleteOnExit)
        {
            fRef = CreateFileW(name.WideString().data(),
                                GENERIC_READ,   // open for reading
                                0,              // no one can open the file until we're done
                                nullptr,        // default security
                                OPEN_EXISTING,  // only open existing files (no creation)
                                FILE_FLAG_DELETE_ON_CLOSE,  // delete the file from disk when we close the handle
                                nullptr);       // no template
        }
        else
        {
            fRef = CreateFileW(name.WideString().data(),
                                GENERIC_READ,   // open for reading
                                0,              // no one can open the file until we're done
                                nullptr,        // default security
                                OPEN_EXISTING,  // only open existing files (no creation)
                                FILE_ATTRIBUTE_NORMAL,  // normal file attributes
                                nullptr);       // no template
        }

        fPosition = 0;

        if (fRef == INVALID_HANDLE_VALUE)
            return false;

        // Make sure our special magic string is there
        if (!ICheckMagicString(fRef))
        {
            CloseHandle(fRef);
            fRef = INVALID_HANDLE_VALUE;
            return false;
        }

        DWORD numBytesRead;
        ReadFile(fRef, &fActualFileSize, sizeof(uint32_t), &numBytesRead, nullptr);
#elif HS_BUILD_FOR_UNIX
        fRef = plFileSystem::Open(name, "rb");
        fPosition = 0;

        if (fRef == INVALID_HANDLE_VALUE)
            return false;

        if (!ICheckMagicString(fRef))
        {
            fclose(fRef);
            fRef = INVALID_HANDLE_VALUE;
            return false;
        }
#endif

        // The encrypted stream is inefficient if you do reads smaller than
        // 8 bytes.  Since we do a lot of those, any file under a size threshold
        // is buffered in memory
        if (fActualFileSize <= kMaxBufferedFileSize)
            IBufferFile();

        fOpenMode = kOpenRead;

        return true;
    }
    else if (strcmp(mode, "wb") == 0)
    {
        fRAMStream = std::make_unique<hsRAMStream>();
        fWriteFileName = name;
        fPosition = 0;

        fOpenMode = kOpenWrite;
        fBufferedStream = true;

        return true;
    }
    else
    {
        hsAssert(0, "Unsupported open mode");
        fOpenMode = kOpenFail;
        return false;
    }
}

bool plSecureStream::Open(hsStream* stream)
{
    uint32_t pos = stream->GetPosition();
    stream->Rewind();
    if (!ICheckMagicString(stream))
        return false;

    fActualFileSize = stream->ReadLE32();
    uint32_t trimSize = kMagicStringLen + sizeof(uint32_t) + fActualFileSize;
    fRAMStream = std::make_unique<hsRAMStream>();
    while (!stream->AtEnd())
    {
        // Don't write out any garbage
        uint32_t size;
        if ((trimSize - stream->GetPosition()) < kEncryptChunkSize)
            size = (trimSize - stream->GetPosition());
        else
            size = kEncryptChunkSize;

        uint8_t buf[kEncryptChunkSize];
        stream->Read(kEncryptChunkSize, &buf);
        IDecipher((uint32_t*)&buf, kEncryptChunkSize / sizeof(uint32_t));
        fRAMStream->Write(size, &buf);
    }

    stream->SetPosition(pos);
    fRAMStream->Rewind();
    fPosition = 0;
    fBufferedStream = true;
    fOpenMode = kOpenRead;
    return true;
}

uint32_t plSecureStream::IRead(uint32_t bytes, void* buffer)
{
    if (fRef == INVALID_HANDLE_VALUE)
        return 0;
    uint32_t numItems = 0;
#if HS_BUILD_FOR_WIN32
    bool success = (ReadFile(fRef, buffer, bytes, (LPDWORD)&numItems, nullptr) != 0);
#elif HS_BUILD_FOR_UNIX
    numItems = fread(buffer, bytes, 1, fRef);
    bool success = numItems != 0;
#endif
    fPosition += numItems;
    if ((unsigned)numItems < bytes)
    {
        if (success)
        {
            // EOF ocurred
            char str[128];
            sprintf(str, "Hit EOF on Windows read, only read %d out of requested %d bytes\n", numItems, bytes);
            hsDebugMessage(str, 0);
        }
        else
        {
#if HS_BUILD_FOR_WIN32
            hsDebugMessage("Error on Windows read", GetLastError());
#else
            hsDebugMessage("Error on POSIX read", errno);
#endif
        }
    }
    return numItems;
}

void plSecureStream::IBufferFile()
{
    fRAMStream = std::make_unique<hsRAMStream>();
    char buf[1024];
    while (!AtEnd())
    {
        uint32_t numRead = Read(1024, buf);
        fRAMStream->Write(numRead, buf);
    }
    fRAMStream->Rewind();

    fBufferedStream = true;
#if HS_BUILD_FOR_WIN32
    CloseHandle(fRef);
#elif HS_BUILD_FOR_UNIX
    fclose(fRef);
#endif
    fRef = INVALID_HANDLE_VALUE;
    fPosition = 0;
}

bool plSecureStream::AtEnd()
{
    if (fBufferedStream)
        return fRAMStream->AtEnd();
    else
        return (GetPosition() == fActualFileSize);
}

void plSecureStream::Skip(uint32_t delta)
{
    if (fBufferedStream)
    {
        fRAMStream->Skip(delta);
        fPosition = fRAMStream->GetPosition();
    }
    else if (fRef != INVALID_HANDLE_VALUE)
    {
        fPosition += delta;
#if HS_BUILD_FOR_WIN32
        SetFilePointer(fRef, delta, nullptr, FILE_CURRENT);
#elif HS_BUILD_FOR_UNIX
        fseek(fRef, delta, SEEK_CUR);
#endif
    }
}

void plSecureStream::Rewind()
{
    if (fBufferedStream)
    {
        fRAMStream->Rewind();
        fPosition = fRAMStream->GetPosition();
    }
    else if (fRef != INVALID_HANDLE_VALUE)
    {
        fPosition = 0;
#if HS_BUILD_FOR_WIN32
        SetFilePointer(fRef, kFileStartOffset, nullptr, FILE_BEGIN);
#elif HS_BUILD_FOR_UNIX
        fseek(fRef, kFileStartOffset, SEEK_SET);
#endif
    }
}

void plSecureStream::FastFwd()
{
    if (fBufferedStream)
    {
        fRAMStream->FastFwd();
        fPosition = fRAMStream->GetPosition();
    }
    else if (fRef != INVALID_HANDLE_VALUE)
    {
#if HS_BUILD_FOR_WIN32
        fPosition = SetFilePointer(fRef, kFileStartOffset + fActualFileSize, nullptr, FILE_BEGIN);
#elif HS_BUILD_FOR_UNIX
        fPosition = fseek(fRef, 0, SEEK_END);
#endif
    }
}

void plSecureStream::Truncate()
{
    if (fOpenMode != kOpenWrite) {
        hsAssert(false, "Trying to write to a read stream");
        return;
    }

    return fRAMStream->Truncate();
}

uint32_t plSecureStream::GetEOF()
{
    return fActualFileSize;
}

uint32_t plSecureStream::Read(uint32_t bytes, void* buffer)
{
    if (fBufferedStream)
    {
        uint32_t numRead = fRAMStream->Read(bytes, buffer);
        fPosition = fRAMStream->GetPosition();
        return numRead;
    }

    uint32_t startPos = fPosition;

    // Offset into the first buffer (0 if we are aligned on a chunk, which means no extra block read)
    uint32_t startChunkPos = startPos % kEncryptChunkSize;
    // Amount of data in the partial first chunk (0 if we're aligned)
    uint32_t startAmt = (startChunkPos != 0) ? std::min(kEncryptChunkSize - startChunkPos, bytes) : 0;

    uint32_t totalNumRead = IRead(bytes, buffer);

    uint32_t numMidChunks = (totalNumRead - startAmt) / kEncryptChunkSize;
    uint32_t endAmt = (totalNumRead - startAmt) % kEncryptChunkSize;

    // If the start position is in the middle of a chunk we need to rewind and
    // read that whole chunk in and decrypt it.
    if (startChunkPos != 0)
    {
        // Move to the start of this chunk
        SetPosition(startPos-startChunkPos);

        // Read in the chunk and decrypt it
        char buf[kEncryptChunkSize];
        (void)IRead(kEncryptChunkSize, &buf);   // numRead
        IDecipher((uint32_t*)&buf, kEncryptChunkSize / sizeof(uint32_t));

        // Copy the relevant portion to the output buffer
        memcpy(buffer, &buf[startChunkPos], startAmt);

        SetPosition(startPos+totalNumRead);
    }

    if (numMidChunks != 0)
    {
        uint32_t* bufferPos = (uint32_t*)(((char*)buffer)+startAmt);
        for (int i = 0; i < numMidChunks; i++)
        {
            // Decrypt chunk
            IDecipher(bufferPos, kEncryptChunkSize / sizeof(uint32_t));
            bufferPos += (kEncryptChunkSize / sizeof(uint32_t));
        }
    }

    if (endAmt != 0)
    {
        // Read in the final chunk and decrypt it
        char buf[kEncryptChunkSize];
        SetPosition(startPos + startAmt + numMidChunks*kEncryptChunkSize);
        (void)IRead(kEncryptChunkSize, &buf);   // numRead
        IDecipher((uint32_t*)&buf, kEncryptChunkSize / sizeof(uint32_t));

        memcpy(((char*)buffer)+totalNumRead-endAmt, &buf, endAmt);

        SetPosition(startPos+totalNumRead);
    }

    // If we read into the padding at the end, update the total read to not include that
    if (totalNumRead > 0 && startPos + totalNumRead > fActualFileSize)
    {
        totalNumRead -= (startPos + totalNumRead) - fActualFileSize;
        SetPosition(fActualFileSize);
    }

    return totalNumRead;
}

uint32_t plSecureStream::Write(uint32_t bytes, const void* buffer)
{
    if (fOpenMode != kOpenWrite)
    {
        hsAssert(0, "Trying to write to a read stream");
        return 0;
    }

    return fRAMStream->Write(bytes, buffer);
}

bool plSecureStream::IWriteEncrypted(hsStream* sourceStream, const plFileName& outputFile)
{
    hsUNIXStream outputStream;

    if (!outputStream.Open(outputFile, "wb"))
        return false;

    outputStream.Write(kMagicStringLen, kMagicString);

    // Save some space to write the file size at the end
    outputStream.WriteLE32(0);

    // Write out all the full size encrypted blocks we can
    char buf[kEncryptChunkSize];
    uint32_t amtRead;
    while ((amtRead = sourceStream->Read(kEncryptChunkSize, &buf)) == kEncryptChunkSize)
    {
        IEncipher((uint32_t*)&buf, kEncryptChunkSize / sizeof(uint32_t));
        outputStream.Write(kEncryptChunkSize, &buf);
    }

    // Pad with random data and write out the final partial block, if there is one
    if (amtRead > 0)
    {
        static bool seededRand = false;
        if (!seededRand)
        {
            seededRand = true;
            srand((unsigned int)time(nullptr));
        }

        for (int i = amtRead; i < kEncryptChunkSize; i++)
            buf[i] = rand();

        IEncipher((uint32_t*)&buf, kEncryptChunkSize / sizeof(uint32_t));

        outputStream.Write(kEncryptChunkSize, &buf);
    }

    // Write the original file size at the start
    uint32_t actualSize = sourceStream->GetPosition();
    outputStream.Rewind();
    outputStream.Skip(kMagicStringLen);
    outputStream.WriteLE32(actualSize);

    return true;
}

bool plSecureStream::FileEncrypt(const plFileName& fileName, uint32_t* key /* = nullptr */)
{
    bool wroteEncrypted;
    {
        hsUNIXStream sIn;
        if (!sIn.Open(fileName))
            return false;

        // Don't double encrypt any files
        if (ICheckMagicString(sIn.GetFILE()))
        {
            return true;
        }
        sIn.Rewind();

        plSecureStream sOut(false, key);
        wroteEncrypted = sOut.IWriteEncrypted(&sIn, "crypt.dat");
    }

    if (wroteEncrypted)
    {
        plFileSystem::Unlink(fileName);
        plFileSystem::Move("crypt.dat", fileName);
    }

    return true;
}

bool plSecureStream::FileDecrypt(const plFileName& fileName, uint32_t* key /* = nullptr */)
{
    {
        plSecureStream sIn(false, key);
        if (!sIn.Open(fileName))
            return false;

        hsUNIXStream sOut;
        if (!sOut.Open("crypt.dat", "wb"))
        {
            return false;
        }

        char buf[1024];

        while (!sIn.AtEnd())
        {
            uint32_t numRead = sIn.Read(sizeof(buf), buf);
            sOut.Write(numRead, buf);
        }
    }

    plFileSystem::Unlink(fileName);
    plFileSystem::Move("crypt.dat", fileName);

    return true;
}

bool plSecureStream::ICheckMagicString(hsStream* s)
{
    char magicString[kMagicStringLen+1];
    s->Read(kMagicStringLen, &magicString);
    magicString[kMagicStringLen] = '\0';
    return (strcmp(magicString, kMagicString) == 0);
}

bool plSecureStream::ICheckMagicString(hsFD fp)
{
    char magicString[kMagicStringLen+1];
#ifdef HS_BUILD_FOR_WIN32
    DWORD numread;
    ReadFile(fp, &magicString, kMagicStringLen, &numread, nullptr);
#elif HS_BUILD_FOR_UNIX
    fread(&magicString, kMagicStringLen, 1, fp);
#endif
    magicString[kMagicStringLen] = '\0';
    return (strcmp(magicString, kMagicString) == 0);
}

bool plSecureStream::IsSecureFile(const plFileName& fileName)
{
    hsFD fp = INVALID_HANDLE_VALUE;

#if HS_BUILD_FOR_WIN32
    fp = CreateFileW(fileName.WideString().data(),
        GENERIC_READ,   // open for reading
        0,              // no one can open the file until we're done
        nullptr,        // default security
        OPEN_EXISTING,  // only open existing files (no creation)
        FILE_ATTRIBUTE_NORMAL,  // normal file attributes
        nullptr);       // no template
#elif HS_BUILD_FOR_UNIX
    fp = plFileSystem::Open(fileName, "rb");
#endif

    if (fp == INVALID_HANDLE_VALUE)
        return false;

    bool isEncrypted = ICheckMagicString(fp);

#if HS_BUILD_FOR_WIN32
    CloseHandle(fp);
#elif HS_BUILD_FOR_UNIX
    fclose(fp);
#endif

    return isEncrypted;
}

std::unique_ptr<hsStream> plSecureStream::OpenSecureFile(const plFileName& fileName, const uint32_t flags /* = kRequireEncryption */, uint32_t* key /* = nullptr */)
{
    bool requireEncryption = flags & kRequireEncryption;
    bool deleteOnExit = flags & kDeleteOnExit;
    bool isEncrypted = IsSecureFile(fileName);

    std::unique_ptr<hsFileSystemStream> s;
    if (isEncrypted)
        s = std::make_unique<plSecureStream>(deleteOnExit, key);
    else if (!requireEncryption)
        s = std::make_unique<hsUNIXStream>();

    if (s)
        s->Open(fileName, "rb");
    return s;
}

std::unique_ptr<hsStream> plSecureStream::OpenSecureFileWrite(const plFileName& fileName, uint32_t* key /* = nullptr */)
{
    std::unique_ptr<hsFileSystemStream> s;
#ifdef PLASMA_EXTERNAL_RELEASE
    s = std::make_unique<plSecureStream>(false, key);
#else
    s = std::make_unique<hsUNIXStream>();
#endif

    s->Open(fileName, "wb");
    return s;
}

//// GetSecureEncryptionKey //////////////////////////////////////////////////

bool plSecureStream::GetSecureEncryptionKey(const plFileName& filename, uint32_t* key, unsigned length)
{
    // looks for an encryption key file in the same directory, and reads it
    plFileName keyFile = plFileName::Join(filename.StripFileName(), kKeyFilename);

    if (plFileInfo(keyFile).Exists())
    {
        // file exists, read from it
        hsUNIXStream file;
        file.Open(keyFile, "rb");

        unsigned bytesToRead = length * sizeof(uint32_t);
        uint8_t* buffer = (uint8_t*)malloc(bytesToRead);
        unsigned bytesRead = file.Read(bytesToRead, buffer);

        unsigned memSize = std::min(bytesToRead, bytesRead);
        memcpy(key, buffer, memSize);
        free(buffer);

        return true;
    }

    // file doesn't exist, use default key
    unsigned memSize = std::min(size_t(length), std::size(plSecureStream::kDefaultKey));
    memSize *= sizeof(uint32_t);
    memcpy(key, plSecureStream::kDefaultKey, memSize);

    return false;
}
