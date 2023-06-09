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
#ifndef hsStream_Defined
#define hsStream_Defined

#include "HeadSpin.h"
#include "plFileSystem.h"

#include <string_theory/string>
#include <vector>

class hsStream {
public:
enum {
    kEolnCode = '\n',
    kComment = '#'
    };
protected:
    uint32_t      fPosition;

    bool      IsTokenSeparator(char c);
public:
    hsStream() : fPosition(0) {}
    virtual     ~hsStream() { }

    virtual bool      Open(const plFileName &, const char * = "rb") = 0;
    virtual bool      AtEnd() = 0;
    virtual uint32_t  Read(uint32_t byteCount, void * buffer) = 0;
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer) = 0;
    virtual void      Skip(uint32_t deltaByteCount) = 0;
    virtual void      Rewind() = 0;
    virtual void      FastFwd() = 0;
    virtual uint32_t  GetPosition() const;
    virtual void      SetPosition(uint32_t position);
    virtual void      Truncate() = 0;
    virtual void      Flush() {}

    virtual uint32_t  GetEOF() = 0;
    uint32_t          GetSizeLeft();

    uint32_t        WriteString(const ST::string & string) { return Write((uint32_t)string.size(), string.c_str()); }

    uint32_t        WriteSafeString(const ST::string &string);
    uint32_t        WriteSafeWString(const ST::string &string);
    ST::string      ReadSafeString();
    ST::string      ReadSafeWString();

    bool            GetToken(char *s, uint32_t maxLen=uint32_t(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    bool            ReadLn(char* s, uint32_t maxLen=uint32_t(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    bool            ReadLn(ST::string& s, const char beginComment=kComment, const char endComment=kEolnCode);

    bool            ReadBOOL(); // Reads a 4-byte BOOLean
    bool            ReadBool(); // Reads a 1-byte boolean

    uint8_t         ReadByte();
    uint16_t        ReadLE16();
    void            ReadLE16(size_t count, uint16_t values[]);
    uint32_t        ReadLE32();
    void            ReadLE32(size_t count, uint32_t values[]);

    void            WriteBOOL(bool value);
    void            WriteBool(bool value);

    void            WriteByte(uint8_t value);
    void            WriteLE16(uint16_t value);
    void            WriteLE16(size_t count, const uint16_t values[]);
    void            WriteLE32(uint32_t value);
    void            WriteLE32(size_t count, const uint32_t values[]);

    float           ReadLEFloat();
    void            ReadLEFloat(size_t count, float values[]);
    double          ReadLEDouble();
    void            ReadLEDouble(size_t count, double values[]);

    void            WriteLEFloat(float value);
    void            WriteLEFloat(size_t count, const float values[]);
    void            WriteLEDouble(double value);
    void            WriteLEDouble(size_t count, const double values[]);

    // Type-safe placement readers
    template <typename T> inline void ReadByte(T*) = delete;
    void ReadByte(uint8_t* v) { *v = ReadByte(); }
    void ReadByte(int8_t* v) { *v = (int8_t)ReadByte(); }
    template <typename T> inline void ReadLE16(T*) = delete;
    void ReadLE16(uint16_t* v) { *v = ReadLE16(); }
    void ReadLE16(int16_t* v) { *v = (int16_t)ReadLE16(); }
    template <typename T> inline void ReadLE32(T*) = delete;
    void ReadLE32(uint32_t* v) { *v = ReadLE32(); }
    void ReadLE32(int32_t* v) { *v = (int32_t)ReadLE32(); }
    template <typename T> inline void ReadLEFloat(T*) = delete;
    void ReadLEFloat(float* v) { *v = ReadLEFloat(); }
    template <typename T> inline void ReadLEDouble(T*) = delete;
    void ReadLEDouble(double* v) { *v = ReadLEDouble(); }

    // Type-safety checks for writers
    template <typename T> void WriteByte(T) = delete;
    void WriteByte(int8_t v) { WriteByte((uint8_t)v); }
    template <typename T> void WriteLE16(T) = delete;
    void WriteLE16(int16_t v) { WriteLE16((uint16_t)v); }
    template <typename T> void WriteLE32(T) = delete;
    void WriteLE32(int32_t v) { WriteLE32((uint32_t)v); }
    template <typename T> void WriteLEFloat(T) = delete;
    template <typename T> void WriteLEDouble(T) = delete;
};

class hsStreamable {
public:
    virtual void    Read(hsStream* stream) = 0;
    virtual void    Write(hsStream* stream) = 0;
    virtual uint32_t  GetStreamSize() = 0;
};

class hsUNIXStream: public hsStream
{   
    FILE*       fRef;

public:
    hsUNIXStream() : fRef() {}
    hsUNIXStream(const hsUNIXStream& other) = delete;
    hsUNIXStream(hsUNIXStream&& other) = delete;
    ~hsUNIXStream();

    const hsUNIXStream& operator=(const hsUNIXStream& other) = delete;
    hsUNIXStream& operator=(hsUNIXStream&& other) = delete;

    bool  Open(const plFileName& name, const char* mode = "rb") override;

    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void* buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      SetPosition(uint32_t position) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      FastFwd() override;
    void      Truncate() override;
    void      Flush() override;

    FILE*           GetFILE() { return fRef; }
    void            SetFILE(FILE* file) { fRef = file; }

    uint32_t  GetEOF() override;
};

// Small substream class: give it a base stream, an offset and a length, and it'll
// treat all ops as if you had a chunk from the base stream as a separate, vanilla 
// stream of the given length.

class plReadOnlySubStream: public hsStream
{   
    hsStream    *fBase;
    uint32_t      fOffset, fLength;

    void    IFixPosition();

public:
    plReadOnlySubStream() : fBase(), fOffset(), fLength() { }

    bool      Open(const plFileName &, const char *) override { hsAssert(0, "plReadOnlySubStream::Open  NotImplemented"); return false; }
    void      Open( hsStream *base, uint32_t offset, uint32_t length );
    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void* buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      FastFwd() override;
    void      Truncate() override;

    uint32_t  GetEOF() override;
};

//
// In-memory only
// Erase function lets you cut a chunk out of the middle of the stream
//
class hsRAMStream : public hsStream
{
    std::vector<uint8_t> fVector;

public:
    hsRAMStream() {}
    hsRAMStream(uint32_t chunkSize) { fVector.reserve(chunkSize); }

    bool  Open(const plFileName &, const char *) override { hsAssert(0, "hsRAMStream::Open  NotImplemented"); return false; }

    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void * buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void FastFwd() override;
    void      Truncate() override;

    uint32_t  GetEOF() override;
    void CopyToMem(void* mem);

    void            Reset();        // clears the buffers

    // Erase number of bytes at the current position
    void Erase(uint32_t bytes);
    // A pointer to the beginning of the data in the stream.  This is only valid
    // until someone modifies the stream.
    const void* GetData();
    // In case you want to try and be efficient with your memory allocations
    void Reserve(uint32_t bytes) { fVector.reserve(bytes); }
};

class hsNullStream : public hsStream {
public:

    bool      Open(const plFileName &, const char *) override { return true; }

    bool AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void * buffer) override;  // throws exception
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void FastFwd() override;
    void      Truncate() override;

    uint32_t GetEOF() override { return fPosition; }
    void Reset() { fPosition = 0; }
};

// read only mem stream
class hsReadOnlyStream : public hsStream {
protected:
    char*   fStart;
    char*   fData;
    char*   fStop;
public:
    hsReadOnlyStream(int size, const void* data) { Init(size, data); }
    hsReadOnlyStream() : fStart(), fData(), fStop() { }

    virtual void      Init(int size, const void* data) { fStart=((char*)data); fData=((char*)data); fStop=((char*)data + size); }
    bool      Open(const plFileName &, const char *) override { hsAssert(0, "hsReadOnlyStream::Open  NotImplemented"); return false; }
    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void * buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;    // throws exception
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void FastFwd() override;
    void      Truncate() override;
    uint32_t  GetEOF() override { return (uint32_t)(fStop-fStart); }
    void CopyToMem(void* mem);
};

// write only mem stream
class hsWriteOnlyStream : public hsReadOnlyStream {
public:
    hsWriteOnlyStream(int size, const void* data) : hsReadOnlyStream(size, data) {}
    hsWriteOnlyStream() {}
    hsWriteOnlyStream(const hsWriteOnlyStream& other) = delete;
    hsWriteOnlyStream(hsWriteOnlyStream&& other) = delete;

    const hsWriteOnlyStream& operator=(const hsWriteOnlyStream& other) = delete;
    hsWriteOnlyStream& operator=(hsWriteOnlyStream&& other) = delete;

    bool      Open(const plFileName &, const char *) override { hsAssert(0, "hsWriteOnlyStream::Open  NotImplemented"); return false; }
    uint32_t  Read(uint32_t byteCount, void * buffer) override;  // throws exception
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
};

// circular queue stream
class hsQueueStream : public hsStream {
private:
    char* fQueue;
    uint32_t fReadCursor;
    uint32_t fWriteCursor;
    uint32_t fSize;
    
public:
    hsQueueStream(int32_t size);
    hsQueueStream(const hsQueueStream& other) = delete;
    hsQueueStream(hsQueueStream&& other) = delete;
    ~hsQueueStream();

    const hsQueueStream& operator=(const hsQueueStream& other) = delete;
    hsQueueStream& operator=(hsQueueStream&& other) = delete;

    bool  Open(const plFileName &, const char *) override { hsAssert(0, "hsQueueStream::Open  NotImplemented"); return false; }

    uint32_t  Read(uint32_t byteCount, void * buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      FastFwd() override;
    void Truncate() override;
    bool      AtEnd() override;

    uint32_t GetEOF() override { return fWriteCursor - fReadCursor; }
    uint32_t GetSize() { return fSize; }
    const char* GetQueue() { return fQueue; }
    uint32_t GetReadCursor() { return fReadCursor; }
    uint32_t GetWriteCursor() { return fWriteCursor; }
};

class hsBufferedStream : public hsStream
{
    FILE* fRef;
    uint32_t fFileSize;

    enum { kBufferSize = 2*1024 };
    char fBuffer[kBufferSize];
    // If the buffer is empty, this is zero.  Otherwise it is the size of the
    // buffer (if we read a full block), or something less than that if we read
    // a partial block at the end of the file.
    uint32_t fBufferLen;

    bool fWriteBufferUsed;

#ifdef HS_DEBUGGING
    // For doing statistics on how efficient we are
    int fBufferHits, fBufferMisses;
    uint32_t fBufferReadIn, fBufferReadOut, fReadDirect, fLastReadPos;
    plFileName fFilename;
    const char* fCloseReason;
#endif

public:
    hsBufferedStream();
    hsBufferedStream(const hsBufferedStream& other) = delete;
    hsBufferedStream(hsBufferedStream&& other) = delete;
    ~hsBufferedStream();

    const hsBufferedStream& operator=(const hsBufferedStream& other) = delete;
    hsBufferedStream& operator=(hsBufferedStream&& other) = delete;

    bool  Open(const plFileName& name, const char* mode = "rb") override;

    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void* buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void FastFwd() override;
    void      Truncate() override;
    uint32_t  GetEOF() override;

    FILE*   GetFileRef();
    void    SetFileRef(FILE* file);

    // Something optional for when we're doing stats.  Will log the reason why
    // the file was closed.  Really just for plRegistryPageNode.
    void SetCloseReason(const char* reason)
    {
#ifdef HS_DEBUGGING
        fCloseReason = reason;
#endif
    }
};

#endif
