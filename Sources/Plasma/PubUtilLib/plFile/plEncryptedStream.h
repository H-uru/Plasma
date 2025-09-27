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
#ifndef plEncryptedStream_h_inc
#define plEncryptedStream_h_inc

#include "hsStream.h"

#include <memory>

//
// Encrypt a large file by running FileEncrypt on it.  Small files can be done
// in the usual way, but they will be in memory until Close is called.  Files
// will be decrypted on the fly during read.operations
//
class plEncryptedStream : public hsFileSystemStream
{
protected:
    FILE* fRef;
    uint32_t fKey[4];

    uint32_t fActualFileSize;

    bool fBufferedStream;

    std::unique_ptr<hsStream> fRAMStream;

    plFileName fWriteFileName;

    enum OpenMode { kOpenRead, kOpenWrite, kOpenFail };
    OpenMode fOpenMode;

    void IBufferFile();

    size_t IRead(size_t bytes, void* buffer);

    void IEncipher(uint32_t* const v);
    void IDecipher(uint32_t* const v);

    bool IWriteEncypted(hsStream* sourceStream, const plFileName& outputFile);

    static bool ICheckMagicString(FILE* fp);

public:
    // If you don't pass in a key (4 uint32_t's), the default one will be used
    plEncryptedStream(uint32_t* key=nullptr);
    plEncryptedStream(const plEncryptedStream& other) = delete;
    plEncryptedStream(plEncryptedStream&& other) = delete;
    ~plEncryptedStream();

    const plEncryptedStream& operator=(const plEncryptedStream& other) = delete;
    plEncryptedStream& operator=(plEncryptedStream&& other) = delete;

    bool    Open(const plFileName& name, const char* mode = "rb") override;

    size_t  Read(size_t byteCount, void* buffer) override;
    size_t  Write(size_t byteCount, const void* buffer) override;
    bool    AtEnd() override;
    void    Skip(uint32_t deltaByteCount) override;
    void    Rewind() override;
    void    FastFwd() override;
    void Truncate() override;
    uint32_t  GetEOF() override;

    uint32_t GetActualFileSize() const { return fActualFileSize;}

    static bool FileEncrypt(const plFileName& fileName);
    static bool FileDecrypt(const plFileName& fileName);

    static bool IsEncryptedFile(const plFileName& fileName);

    // Attempts to create a read-binary stream for the requested file.  If it's
    // encrypted, you'll get a plEncryptedStream, otherwise just a standard hsUNIXStream.
    static std::unique_ptr<hsStream> OpenEncryptedFile(const plFileName& fileName, uint32_t* cryptKey = nullptr);
    static std::unique_ptr<hsStream> OpenEncryptedFileWrite(const plFileName& fileName, uint32_t* cryptKey = nullptr);
};

#endif // plEncryptedStream_h_inc
