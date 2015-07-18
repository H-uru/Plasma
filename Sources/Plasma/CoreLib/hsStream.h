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


// Define this for use of Streams with Logging (commonly used w/ a packet sniffer)
// These streams log their reads to an event list
//#define STREAM_LOGGER

#ifndef STREAM_LOGGER
#define hsReadOnlyLoggingStream hsReadOnlyStream
#define LogRead(byteCount, buffer, desc) Read(byteCount, buffer)
#define LogReadSafeString() ReadSafeString()
#define LogReadSafeStringLong() ReadSafeStringLong();
#define LogSkip(deltaByteCount, desc) Skip(deltaByteCount)
#define LogReadLE(value, desc) ReadLE(value)
#define LogReadLEArray(count, values, desc) ReadLE(count, values)
#define LogSubStreamStart(desc) LogVoidFunc()
#define LogSubStreamPushDesc(desc) LogVoidFunc()
#define LogSubStreamEnd() LogVoidFunc()
#define LogStringString(s) LogVoidFunc()
#endif

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

#ifdef STREAM_LOGGER
    // Logging Reads & Skips
    virtual uint32_t  LogRead(uint32_t byteCount, void * buffer, const char* desc) { return Read(byteCount,buffer); }
    virtual char*   LogReadSafeString() { return ReadSafeString(); }
    virtual char*   LogReadSafeStringLong() { return ReadSafeStringLong(); }
    virtual void    LogSkip(uint32_t deltaByteCount, const char* desc) { Skip(deltaByteCount); }

    // Stream Notes for Logging 
    virtual void    LogStringString(const char* s) { }
    virtual void    LogSubStreamStart(const char* desc) { }
    virtual void    LogSubStreamEnd() { }
    virtual void    LogSubStreamPushDesc(const char* desc) { }
#endif
    void LogVoidFunc() { }

    // Optimization for small Reads
    virtual uint8_t ReadByte();
    virtual bool    Read4Bytes(void *buffer);   // Reads 4 bytes,  return true if success 
    virtual bool    Read8Bytes(void *buffer);   // Reads 8 bytes,  return true if success 
    virtual bool    Read12Bytes(void *buffer);  // Reads 12 bytes, return true if success

    virtual uint32_t  GetEOF();
    uint32_t          GetSizeLeft();
    virtual void      CopyToMem(void* mem);
    virtual bool      IsCompressed() { return false; }

    uint32_t        WriteString(const plString & string) { return Write(string.GetSize(), string.c_str()); }
    hsDeprecated("hsStream::WriteFmt is deprecated -- use plFormat instead")
    uint32_t        WriteFmt(const char * fmt, ...);
    uint32_t        WriteFmtV(const char * fmt, va_list av);

    uint32_t        WriteSafeStringLong(const plString &string);    // uses 4 bytes for length
    uint32_t        WriteSafeWStringLong(const plString &string);
    plString        ReadSafeStringLong();
    plString        ReadSafeWStringLong();

    uint32_t        WriteSafeString(const plString &string);        // uses 2 bytes for length
    uint32_t        WriteSafeWString(const plString &string);
    plString        ReadSafeString();
    plString        ReadSafeWString();

    bool            GetToken(char *s, uint32_t maxLen=uint32_t(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    bool            ReadLn(char* s, uint32_t maxLen=uint32_t(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    
    // Reads a 4-byte BOOLean
    bool            ReadBOOL();
    // Reads a 1-byte boolean
    bool            ReadBool();
    void            ReadBool(int count, bool values[]);
    uint16_t        ReadLE16();
    void            ReadLE16(int count, uint16_t values[]);
    uint32_t        ReadLE32();
    void            ReadLE32(int count, uint32_t values[]);
    uint32_t        ReadBE32();

    void            WriteBOOL(bool value);
    void            WriteBool(bool value);
    void            WriteBool(int count, const bool values[]);
    void            WriteByte(uint8_t value);
    void            WriteLE16(uint16_t value);
    void            WriteLE16(int count, const uint16_t values[]);
    void            WriteLE32(uint32_t value);
    void            WriteLE32(int count, const  uint32_t values[]);
    void            WriteBE32(uint32_t value);


    /* Overloaded  Begin (8 & 16 & 32 int)*/
    /* yes, swapping an 8 bit value does nothing, just useful*/
    void            ReadLE(bool* value) { *value = this->ReadByte() ? true : false; }
    void            ReadLE(uint8_t* value) { *value = this->ReadByte(); }
    void            ReadLE(int count, uint8_t values[]) { this->Read(count, values); }
    void            ReadLE(uint16_t* value) { *value = this->ReadLE16(); }
    void            ReadLE(int count, uint16_t values[]) { this->ReadLE16(count, values); }
    void            ReadLE(uint32_t* value) { *value = this->ReadLE32(); }
    void            ReadLE(int count, uint32_t values[]) { this->ReadLE32(count, values); }
#ifdef STREAM_LOGGER
                // Begin LogReadLEs
    virtual void    LogReadLE(bool* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLE(uint8_t* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, uint8_t values[], const char* desc) { this->ReadLE(count, values); }
    virtual void    LogReadLE(uint16_t* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, uint16_t values[], const char* desc) { this->ReadLE(count, values); }
    virtual void    LogReadLE(uint32_t* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, uint32_t values[], const char* desc) { this->ReadLE(count, values); }
                // End LogReadLEs
#endif
    void            WriteLE(bool value) { this->Write(1,&value); }
    void            WriteLE(uint8_t value) { this->Write(1,&value); }
    void            WriteLE(int count, const uint8_t values[]) { this->Write(count, values); }
    void            WriteLE(uint16_t value) { this->WriteLE16(value); }
    void            WriteLE(int count, const uint16_t values[]) { this->WriteLE16(count, values); }
    void            WriteLE(uint32_t value) { this->WriteLE32(value); }
    void            WriteLE(int count, const  uint32_t values[]) { this->WriteLE32(count, values); }
    void            ReadLE(int8_t* value) { *value = this->ReadByte(); }
    void            ReadLE(int count, int8_t values[]) { this->Read(count, values); }
    void            ReadLE(char* value) { *value = (char)this->ReadByte(); }
    void            ReadLE(int count, char values[]) { this->Read(count, values); }
    void            ReadLE(int16_t* value) { *value = (int16_t)this->ReadLE16(); }
    void            ReadLE(int count, int16_t values[]) { this->ReadLE16(count, (uint16_t*)values); }
    void            ReadLE(int32_t* value) { *value = (int32_t)this->ReadLE32(); }
    void            ReadLE(int count, int32_t values[]) { this->ReadLE32(count, (uint32_t*)values); }
#ifdef STREAM_LOGGER
                // Begin LogReadLEs
    virtual void    LogReadLE(int8_t* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, int8_t values[], const char* desc) { this->ReadLE(count, values); }
    virtual void    LogReadLE(char* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, char values[], const char* desc) { this->ReadLE(count, values); }
    virtual void    LogReadLE(int16_t* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, int16_t values[], const char* desc) { this->ReadLE(count, (uint16_t*)values); }
    virtual void    LogReadLE(int32_t* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, int32_t values[], const char* desc) { this->ReadLE(count, (uint32_t*)values); }
    virtual void    LogReadLE(int* value, const char* desc) { this->ReadLE(value); }
    virtual void    LogReadLEArray(int count, int values[], const char* desc) { this->ReadLE(count, (uint32_t*)values); }
                // End LogReadLEs
#endif
    void            WriteLE(int8_t value) { this->Write(1,&value); }
    void            WriteLE(int count, const int8_t values[]) { this->Write(count, values); }
    void            WriteLE(char value) { this->Write(1,(uint8_t*)&value); }
    void            WriteLE(int count, const char values[]) { this->Write(count, (uint8_t*)values); }
    void            WriteLE(int16_t value) { this->WriteLE16((uint16_t)value); }
    void            WriteLE(int count, const int16_t values[]) { this->WriteLE16(count, (uint16_t*)values); }
    void            WriteLE(int32_t value) { this->WriteLE32((uint32_t)value); }
    void            WriteLE(int count, const  int32_t values[]) { this->WriteLE32(count, (uint32_t*)values); }
    /* Overloaded  End */


    float           ReadLEFloat();
    void            ReadLEFloat(int count, float values[]);
    double          ReadLEDouble();
    void            ReadLEDouble(int count, double values[]);
    float           ReadBEFloat();
    void            WriteLEFloat(float value);
    void            WriteLEFloat(int count, const float values[]);
    void            WriteLEDouble(double value);
    void            WriteLEDouble(int count, const double values[]);
    void            WriteBEFloat(float value);


    /* Overloaded  Begin (Float)*/
    void            ReadLE(float* value) { *value = ReadLEFloat(); }
    void            ReadLE(int count, float values[]) { ReadLEFloat(count, values); }
    void            ReadLE(double* value) { *value = ReadLEDouble(); }
    void            ReadLE(int count, double values[]) { ReadLEDouble(count, values); }
#ifdef STREAM_LOGGER
                    // Begin LogReadLEs
    virtual void    LogReadLE(float* value, const char* desc) { ReadLE(value); }
    virtual void    LogReadLEArray(int count, float values[], const char* desc) { ReadLE(count, values); }
    virtual void    LogReadLE(double* value, const char* desc) { ReadLE(value); }
    virtual void    LogReadLEArray(int count, double values[], const char* desc) { ReadLE(count, values); }
                    // End LogReadLEs
#endif
    void            WriteLE(float value) { WriteLEFloat(value); }
    void            WriteLE(int count, const float values[]) { WriteLEFloat(count, values); }
    void            WriteLE(double value) { WriteLEDouble(value); }
    void            WriteLE(int count, const double values[]) { WriteLEDouble(count, values); }
    /* Overloaded End */

    float           ReadLEScalar() { return (float)this->ReadLEFloat(); }
    void            ReadLEScalar(int count, float values[])
                    {
                        this->ReadLEFloat(count, (float*)values);
                    }
    float           ReadBEScalar() { return (float)this->ReadBEFloat(); }
    void            WriteLEScalar(float value) { this->WriteLEFloat(value); }
    void            WriteLEScalar(int count, const float values[])
                    {
                        this->WriteLEFloat(count, (float*)values);
                    }
    void            WriteBEScalar(float value) { this->WriteBEFloat(value); }

    void            WriteLEAtom(uint32_t tag, uint32_t size);
    uint32_t          ReadLEAtom(uint32_t* size);


    /* Overloaded  Begin (Atom)*/
    void            WriteLE(uint32_t* tag, uint32_t size) { WriteLEAtom(*tag, size); }
    void            ReadLE(uint32_t* tag, uint32_t *size) { *tag = ReadLEAtom(size); }
    /* Overloaded  End */
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
    hsUNIXStream(): fRef(0), fBuff(nil) {}
    ~hsUNIXStream();
    virtual bool  Open(const plFileName& name, const char* mode = "rb");
    virtual bool  Close();

    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void* buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      SetPosition(uint32_t position);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      FastFwd();
    virtual void      Truncate();
    virtual void      Flush();

    FILE*           GetFILE() { return fRef; }
    void            SetFILE(FILE* file) { fRef = file; }

    virtual uint32_t  GetEOF();
};

// Small substream class: give it a base stream, an offset and a length, and it'll
// treat all ops as if you had a chunk from the base stream as a separate, vanilla 
// stream of the given length.

class plReadOnlySubStream: public hsStream
{   
    hsStream    *fBase;
    uint32_t      fOffset, fLength;

    void    IFixPosition( void );

public:
    plReadOnlySubStream(): fBase( nil ), fOffset( 0 ), fLength( 0 ) {}
    ~plReadOnlySubStream();

    virtual bool      Open(const plFileName &, const char *) { hsAssert(0, "plReadOnlySubStream::Open  NotImplemented"); return false; }
    void              Open( hsStream *base, uint32_t offset, uint32_t length );
    virtual bool      Close() { fBase = nil; fOffset = 0; fLength = 0; return true; }
    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void* buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      FastFwd();
    virtual void      Truncate();

    virtual uint32_t  GetEOF();
};

class hsRAMStream : public hsStream {
    hsAppender          fAppender;
    hsAppenderIterator  fIter;
public:
                hsRAMStream();
                hsRAMStream(uint32_t chunkSize);
    virtual     ~hsRAMStream();

    virtual bool  Open(const plFileName &, const char *) { hsAssert(0, "hsRAMStream::Open  NotImplemented"); return false; }
    virtual bool  Close() { return false; }

    
    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void * buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      Truncate();

    virtual uint32_t  GetEOF();
    virtual void    CopyToMem(void* mem);

    void            Reset();        // clears the buffers
};

class hsNullStream : public hsStream {
public:

    virtual bool      Open(const plFileName &, const char *) { return true; }
    virtual bool      Close()             { return true; }

    virtual uint32_t  Read(uint32_t byteCount, void * buffer);  // throw's exception
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      Truncate();

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
    hsReadOnlyStream() {}

    virtual void      Init(int size, const void* data) { fStart=((char*)data); fData=((char*)data); fStop=((char*)data + size); }
    virtual bool      Open(const plFileName &, const char *) { hsAssert(0, "hsReadOnlyStream::Open  NotImplemented"); return false; }
    virtual bool      Close() { hsAssert(0, "hsReadOnlyStream::Close  NotImplemented"); return false; }
    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void * buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);    // throws exception
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      Truncate();
    virtual uint32_t  GetBytesRead() const { return fBytesRead; }
    virtual uint32_t  GetEOF() { return (uint32_t)(fStop-fStart); }
    virtual void      CopyToMem(void* mem);
};

// write only mem stream
class hsWriteOnlyStream : public hsReadOnlyStream {
public:
    hsWriteOnlyStream(int size, const void* data) : hsReadOnlyStream(size, data) {}
    hsWriteOnlyStream() {}

    virtual bool      Open(const plFileName &, const char *) { hsAssert(0, "hsWriteOnlyStream::Open  NotImplemented"); return false; }
    virtual bool      Close() { hsAssert(0, "hsWriteOnlyStream::Close  NotImplemented"); return false; }
    virtual uint32_t  Read(uint32_t byteCount, void * buffer);  // throws exception
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);    
    virtual uint32_t  GetBytesRead() const { return 0; }
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

    virtual bool  Open(const plFileName &, const char *) { hsAssert(0, "hsQueueStream::Open  NotImplemented"); return false; }
    virtual bool  Close() { hsAssert(0, "hsQueueStream::Close  NotImplemented"); return false; }

    virtual uint32_t  Read(uint32_t byteCount, void * buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      FastFwd();
    virtual bool      AtEnd();

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

    virtual bool  Open(const plFileName& name, const char* mode = "rb");
    virtual bool  Close();

    virtual bool      AtEnd();
    virtual uint32_t  Read(uint32_t byteCount, void* buffer);
    virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
    virtual void      Skip(uint32_t deltaByteCount);
    virtual void      Rewind();
    virtual void      Truncate();
    virtual uint32_t  GetEOF();

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
