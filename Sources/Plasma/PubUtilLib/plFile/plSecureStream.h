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

#include "hsStream.h"

#if HS_BUILD_FOR_WIN32
#    include <windows.h>
#    define hsFD HANDLE
#else
#    define hsFD FILE*
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
    UInt32 fKey[4];

    UInt32 fActualFileSize;

    bool fBufferedStream;

    hsStream* fRAMStream;
    
    wchar* fWriteFileName;

    enum OpenMode {kOpenRead, kOpenWrite, kOpenFail};
    OpenMode fOpenMode;

    hsBool fDeleteOnExit;

    void IBufferFile();

    UInt32 IRead(UInt32 bytes, void* buffer);

    void IEncipher(UInt32* const v, UInt32 n);
    void IDecipher(UInt32* const v, UInt32 n);

    bool IWriteEncrypted(hsStream* sourceStream, const wchar* outputFile);

    static bool ICheckMagicString(hsFD fp);
    static bool ICheckMagicString(hsStream* s);

public:
    plSecureStream(hsBool deleteOnExit = false, UInt32* key = nil); // uses default key if you don't pass one in
    plSecureStream(hsStream* base, UInt32* key = nil);
    ~plSecureStream();

    virtual hsBool Open(const char* name, const char* mode = "rb");
    virtual hsBool Open(const wchar* name, const wchar* mode = L"rb");
    hsBool         Open(hsStream* stream);
    virtual hsBool Close();

    virtual UInt32 Read(UInt32 byteCount, void* buffer);
    virtual UInt32 Write(UInt32 byteCount, const void* buffer);
    virtual hsBool AtEnd();
    virtual void Skip(UInt32 deltaByteCount);
    virtual void Rewind();
    virtual void FastFwd();
    virtual UInt32 GetEOF();

    UInt32 GetActualFileSize() const {return fActualFileSize;}

    static bool FileEncrypt(const char* fileName, UInt32* key = nil);
    static bool FileEncrypt(const wchar* fileName, UInt32* key = nil);
    static bool FileDecrypt(const char* fileName, UInt32* key = nil);
    static bool FileDecrypt(const wchar* fileName, UInt32* key = nil);

    enum OpenSecureFileFlags
    {
        kRequireEncryption = 0x01,
        kDeleteOnExit = 0x02,
    };

    static bool IsSecureFile(const char* fileName);
    static bool IsSecureFile(const wchar* fileName);

    // Attempts to create a read-binary stream for the requested file (delete the stream
    // when you are done with it!)
    static hsStream* OpenSecureFile(const char* fileName, const UInt32 flags = kRequireEncryption, UInt32* key = nil);
    static hsStream* OpenSecureFile(const wchar* fileName, const UInt32 flags = kRequireEncryption, UInt32* key = nil);
    // Attempts to create a write-binary stream for the requested file (delete the stream
    // when you are done with it!)
    static hsStream* OpenSecureFileWrite(const char* fileName, UInt32* key = nil);
    static hsStream* OpenSecureFileWrite(const wchar* fileName, UInt32* key = nil);

    static const UInt32 kDefaultKey[4]; // our default encryption key
};

#endif // plSecureStream_h_inc
