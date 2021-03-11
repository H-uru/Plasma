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
#include "hsMemory.h"
#include "plFileSystem.h"
#include <string_theory/format>

class hsStream {
public:
enum {
    kEolnCode = '\n',
    kComment = '#'
    };
protected:
    uint32_t      fBytesRead;
    uint32_t      fPosition;

    bool      IsTokenSeparator(char c);
public:
                hsStream() : fBytesRead(0), fPosition(0) {}
    virtual     ~hsStream() { }

    virtual bool      Open(const plFileName &, const char * = "rb") = 0;
    virtual bool      Close()=0;
    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void * buffer) = 0;
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer) = 0;
    virtual void      Skip(uint32_t deltaByteCount) = 0;
    virtual void      Rewind() = 0;
    virtual void      FastFwd();
    virtual uint32_t  GetPosition() const;
    virtual void      SetPosition(uint32_t position);
    virtual void      Truncate();
    virtual void      Flush() {}

    virtual uint32_t  GetEOF();
    uint32_t          GetSizeLeft();
    virtual void      CopyToMem(void* mem);
    virtual bool      IsCompressed() { return false; }

    uint32_t        WriteString(const ST::string & string) { return Write(string.size(), string.c_str()); }

    template        <typename... _Args>
    uint32_t        WriteFmt(const char * fmt, _Args ... args) { return WriteString(ST::format(fmt, args...)); }

    uint32_t        WriteSafeStringLong(const ST::string &string);  // uses 4 bytes for length
    uint32_t        WriteSafeWStringLong(const ST::string &string);
    ST::string      ReadSafeStringLong();
    ST::string      ReadSafeWStringLong();

    uint32_t        WriteSafeString(const ST::string &string);      // uses 2 bytes for length
    uint32_t        WriteSafeWString(const ST::string &string);
    ST::string      ReadSafeString();
    ST::string      ReadSafeWString();

    bool            GetToken(char *s, uint32_t maxLen=uint32_t(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    bool            ReadLn(char* s, uint32_t maxLen=uint32_t(-1), const char beginComment=kComment, const char endComment=kEolnCode);

    bool            ReadBOOL(); // Reads a 4-byte BOOLean
    bool            ReadBool(); // Reads a 1-byte boolean

    uint8_t         ReadByte();
    uint16_t        ReadLE16();
    void            ReadLE16(int count, uint16_t values[]);
    uint32_t        ReadLE32();
    void            ReadLE32(int count, uint32_t values[]);

    void            WriteBOOL(bool value);
    void            WriteBool(bool value);

    void            WriteByte(uint8_t value);
    void            WriteLE16(uint16_t value);
    void            WriteLE16(int count, const uint16_t values[]);
    void            WriteLE32(uint32_t value);
    void            WriteLE32(int count, const  uint32_t values[]);


    /* Overloaded  Begin (8 & 16 & 32 int)*/
    /* yes, swapping an 8 bit value does nothing, just useful*/
    void            ReadLE(uint8_t* value) { *value = this->ReadByte(); }
    void            ReadLE(uint16_t* value) { *value = this->ReadLE16(); }
    void            ReadLE(uint32_t* value) { *value = this->ReadLE32(); }
    void            WriteLE(uint8_t value) { this->Write(1,&value); }
    void            WriteLE(uint16_t value) { this->WriteLE16(value); }
    void            WriteLE(uint32_t value) { this->WriteLE32(value); }
    void            ReadLE(int8_t* value) { *value = this->ReadByte(); }
    void            ReadLE(char* value) { *value = (char)this->ReadByte(); }
    void            ReadLE(int16_t* value) { *value = (int16_t)this->ReadLE16(); }
    void            ReadLE(int32_t* value) { *value = (int32_t)this->ReadLE32(); }
    void            WriteLE(int8_t value) { this->Write(1,&value); }
    void            WriteLE(char value) { this->Write(1,(uint8_t*)&value); }
    void            WriteLE(int16_t value) { this->WriteLE16((uint16_t)value); }
    void            WriteLE(int32_t value) { this->WriteLE32((uint32_t)value); }
    /* Overloaded  End */


    float           ReadLEFloat();
    void            ReadLEFloat(int count, float values[]);
    double          ReadLEDouble();
    void            ReadLEDouble(int count, double values[]);
    void            WriteLEFloat(float value);
    void            WriteLEFloat(int count, const float values[]);
    void            WriteLEDouble(double value);
    void            WriteLEDouble(int count, const double values[]);


    /* Overloaded  Begin (Float)*/
    void            ReadLE(float* value) { *value = ReadLEFloat(); }
    void            ReadLE(double* value) { *value = ReadLEDouble(); }
    void            WriteLE(float value) { WriteLEFloat(value); }
    void            WriteLE(double value) { WriteLEDouble(value); }
    /* Overloaded End */
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
    char*       fBuff;

public:
    hsUNIXStream(): fRef(), fBuff() { }
    ~hsUNIXStream();
    bool  Open(const plFileName& name, const char* mode = "rb") override;
    bool  Close() override;

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
    ~plReadOnlySubStream();

    bool      Open(const plFileName &, const char *) override { hsAssert(0, "plReadOnlySubStream::Open  NotImplemented"); return false; }
    void      Open( hsStream *base, uint32_t offset, uint32_t length );
    bool      Close() override { fBase = nullptr; fOffset = 0; fLength = 0; return true; }
    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void* buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      FastFwd() override;
    void      Truncate() override;

    uint32_t  GetEOF() override;
};

class hsRAMStream : public hsStream {
    hsAppender          fAppender;
    hsAppenderIterator  fIter;
public:
                hsRAMStream();
                hsRAMStream(uint32_t chunkSize);
    virtual     ~hsRAMStream();

    bool  Open(const plFileName &, const char *) override { hsAssert(0, "hsRAMStream::Open  NotImplemented"); return false; }
    bool  Close() override { return false; }

    
    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void * buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      Truncate() override;

    uint32_t  GetEOF() override;
    void    CopyToMem(void* mem) override;

    void            Reset();        // clears the buffers
};

class hsNullStream : public hsStream {
public:

    bool      Open(const plFileName &, const char *) override { return true; }
    bool      Close() override { return true; }

    uint32_t  Read(uint32_t byteCount, void * buffer) override;  // throws exception
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      Truncate() override;

    uint32_t          GetBytesWritten() const { return fBytesRead; }
    void              Reset( ) { fBytesRead = 0;   }
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
    bool      Close() override { hsAssert(0, "hsReadOnlyStream::Close  NotImplemented"); return false; }
    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void * buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;    // throws exception
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      Truncate() override;
    virtual uint32_t  GetBytesRead() const { return fBytesRead; }
    uint32_t  GetEOF() override { return (uint32_t)(fStop-fStart); }
    void      CopyToMem(void* mem) override;
};

// write only mem stream
class hsWriteOnlyStream : public hsReadOnlyStream {
public:
    hsWriteOnlyStream(int size, const void* data) : hsReadOnlyStream(size, data) {}
    hsWriteOnlyStream() {}

    bool      Open(const plFileName &, const char *) override { hsAssert(0, "hsWriteOnlyStream::Open  NotImplemented"); return false; }
    bool      Close() override { hsAssert(0, "hsWriteOnlyStream::Close  NotImplemented"); return false; }
    uint32_t  Read(uint32_t byteCount, void * buffer) override;  // throws exception
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    uint32_t  GetBytesRead() const override { return 0; }
    virtual uint32_t  GetBytesWritten() const { return fBytesRead; }
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
    ~hsQueueStream();

    bool  Open(const plFileName &, const char *) override { hsAssert(0, "hsQueueStream::Open  NotImplemented"); return false; }
    bool  Close() override { hsAssert(0, "hsQueueStream::Close  NotImplemented"); return false; }

    uint32_t  Read(uint32_t byteCount, void * buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
    void      FastFwd() override;
    bool      AtEnd() override;

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
    virtual ~hsBufferedStream() { }

    bool  Open(const plFileName& name, const char* mode = "rb") override;
    bool  Close() override;

    bool      AtEnd() override;
    uint32_t  Read(uint32_t byteCount, void* buffer) override;
    uint32_t  Write(uint32_t byteCount, const void* buffer) override;
    void      Skip(uint32_t deltaByteCount) override;
    void      Rewind() override;
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
