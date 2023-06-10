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
#ifndef plSecureStream_h_inc
#define plSecureStream_h_inc

#include "HeadSpin.h"
#include "hsStream.h"

#include <memory>

#if HS_BUILD_FOR_WIN32
    typedef void* HANDLE;
#   define hsFD HANDLE
#else
#   define hsFD FILE*
#endif

// A slightly more secure stream then plEncryptedStream in that it uses windows file functions
// to prevent other processes from accessing the file it is working with. It also can be set
// to download the file from a server into a temporary directory (with a mangled name) and
// delete that file on close, thereby minimizing the chance of having that file examined or
// edited.
class plSecureStream: public hsStream
{
protected:
    hsFD fRef;
    uint32_t fKey[4];

    uint32_t fActualFileSize;

    bool fBufferedStream;

    std::unique_ptr<hsStream> fRAMStream;

    plFileName fWriteFileName;

    enum OpenMode {kOpenRead, kOpenWrite, kOpenFail};
    OpenMode fOpenMode;

    bool fDeleteOnExit;

    void IBufferFile();

    uint32_t IRead(uint32_t bytes, void* buffer);

    void IEncipher(uint32_t* const v, uint32_t n);
    void IDecipher(uint32_t* const v, uint32_t n);

    bool IWriteEncrypted(hsStream* sourceStream, const plFileName& outputFile);

    static bool ICheckMagicString(hsFD fp);
    static bool ICheckMagicString(hsStream* s);

public:
    plSecureStream(bool deleteOnExit = false, uint32_t* key = nullptr); // uses default key if you don't pass one in
    plSecureStream(hsStream* base, uint32_t* key = nullptr);
    plSecureStream(const plSecureStream& other) = delete;
    plSecureStream(plSecureStream&& other) = delete;
    ~plSecureStream();

    const plSecureStream& operator=(const plSecureStream& other) = delete;
    plSecureStream& operator=(plSecureStream&& other) = delete;

    bool Open(const plFileName& name, const char* mode = "rb") override;
    bool Open(hsStream* stream);

    uint32_t Read(uint32_t byteCount, void* buffer) override;
    uint32_t Write(uint32_t byteCount, const void* buffer) override;
    bool AtEnd() override;
    void Skip(uint32_t deltaByteCount) override;
    void Rewind() override;
    void FastFwd() override;
    void Truncate() override;
    uint32_t GetEOF() override;

    uint32_t GetActualFileSize() const {return fActualFileSize;}

    static bool FileEncrypt(const plFileName& fileName, uint32_t* key = nullptr);
    static bool FileDecrypt(const plFileName& fileName, uint32_t* key = nullptr);

    enum OpenSecureFileFlags
    {
        kRequireEncryption = 0x01,
        kDeleteOnExit = 0x02,
    };

    static bool IsSecureFile(const plFileName& fileName);

    // Attempts to create a read-binary stream for the requested file
    static std::unique_ptr<hsStream> OpenSecureFile(const plFileName& fileName, const uint32_t flags = kRequireEncryption, uint32_t* key = nullptr);
    // Attempts to create a write-binary stream for the requested file
    static std::unique_ptr<hsStream> OpenSecureFileWrite(const plFileName& fileName, uint32_t* key = nullptr);

    static const uint32_t kDefaultKey[4]; // our default encryption key

    // searches the parent directory of filename for the encryption key file, and reads it
    // into the key passed in. Returns false if the key file didn't exist (and sets key to
    // the default key)
    static bool GetSecureEncryptionKey(const plFileName& filename, uint32_t* key, unsigned length);

    static const char kKeyFilename[];
};

#endif // plSecureStream_h_inc
