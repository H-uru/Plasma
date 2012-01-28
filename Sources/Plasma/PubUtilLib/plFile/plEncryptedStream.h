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

//
// Encrypt a large file by running FileEncrypt on it.  Small files can be done
// in the usual way, but they will be in memory until Close is called.  Files
// will be decrypted on the fly during read.operations
//
class plEncryptedStream : public hsStream
{
protected:
    FILE* fRef;
    uint32_t fKey[4];

    uint32_t fActualFileSize;

    bool fBufferedStream;

    hsStream* fRAMStream;

    wchar_t* fWriteFileName;

    enum OpenMode { kOpenRead, kOpenWrite, kOpenFail };
    OpenMode fOpenMode;

    void IBufferFile();

    uint32_t IRead(uint32_t bytes, void* buffer);

    void IEncipher(uint32_t* const v);
    void IDecipher(uint32_t* const v);

    bool IWriteEncypted(hsStream* sourceStream, const wchar_t* outputFile);

    static bool ICheckMagicString(FILE* fp);

public:
    // If you don't pass in a key (4 uint32_t's), the default one will be used
    plEncryptedStream(uint32_t* key=nil);
    ~plEncryptedStream();

    virtual hsBool  Open(const char* name, const char* mode = "rb");
    virtual hsBool  Open(const wchar_t* name, const wchar_t* mode = L"rb");
    virtual hsBool  Close();

    virtual uint32_t  Read(uint32_t byteCount, void* buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual hsBool  AtEnd();
    virtual void    Skip(uint32_t deltaByteCount);
    virtual void    Rewind();
    virtual void    FastFwd();
    virtual uint32_t  GetEOF();

    uint32_t GetActualFileSize() const { return fActualFileSize;}

    static bool FileEncrypt(const char* fileName);
    static bool FileEncrypt(const wchar_t* fileName);
    static bool FileDecrypt(const char* fileName);
    static bool FileDecrypt(const wchar_t* fileName);

    static bool IsEncryptedFile(const char* fileName);
    static bool IsEncryptedFile(const wchar_t* fileName);

    // Attempts to create a read-binary stream for the requested file.  If it's
    // encrypted, you'll get a plEncryptedStream, otherwise just a standard
    // hsUNIXStream.  Remember to delete the stream when you're done with it.
    static hsStream* OpenEncryptedFile(const char* fileName, bool requireEncrypted = true, uint32_t* cryptKey = nil);
    static hsStream* OpenEncryptedFile(const wchar_t* fileName, bool requireEncrypted = true, uint32_t* cryptKey = nil);
    static hsStream* OpenEncryptedFileWrite(const char* fileName, uint32_t* cryptKey = nil);
    static hsStream* OpenEncryptedFileWrite(const wchar_t* fileName, uint32_t* cryptKey = nil);
};

#endif // plEncryptedStream_h_inc
