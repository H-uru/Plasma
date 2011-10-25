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

#include <stdarg.h> // Included for GCC 3.2.2+

#include "hsTypes.h"
#include "hsMemory.h"

namespace hsPackFileSys {
struct FileEntry;
}

// Define this for use of Streams with Logging (commonly used w/ a packet sniffer)
// These streams log their reads to an event list
//#define STREAM_LOGGER

#ifndef STREAM_LOGGER
#define hsReadOnlyLoggingStream hsReadOnlyStream
#define LogRead(byteCount, buffer, desc) Read(byteCount, buffer)
#define LogReadSafeString() ReadSafeString();
#define LogReadSafeStringLong() ReadSafeStringLong();
#define LogSkip(deltaByteCount, desc) Skip(deltaByteCount)
#define LogReadSwap(value, desc) ReadSwap(value)
#define LogReadSwapArray(count, values, desc) ReadSwap(count, values)
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
enum  VDB_Type  {// Virtual Database type
    kVDB_GroupObject,
    kVDB_Mesh
    };
protected:
    UInt32      fBytesRead;
    UInt32      fPosition;

    hsBool      IsTokenSeparator(char c);
public:
                hsStream() : fBytesRead(0), fPosition(0) {}
    virtual     ~hsStream();

    virtual hsBool  Open(const char *, const char * = "rb")=0;
    virtual hsBool  Open(const wchar *, const wchar * = L"rb")=0;
    virtual hsBool  Close()=0;
    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void * buffer) = 0;
    virtual UInt32  Write(UInt32 byteCount, const void* buffer) = 0;
    virtual void    Skip(UInt32 deltaByteCount) = 0;
    virtual void    Rewind() = 0;
    virtual void    FastFwd();
    virtual UInt32  GetPosition() const;
    virtual void    SetPosition(UInt32 position);
    virtual void    Truncate();
    virtual void    Flush() {}

#ifdef STREAM_LOGGER
    // Logging Reads & Skips
    virtual UInt32  LogRead(UInt32 byteCount, void * buffer, const char* desc) { return Read(byteCount,buffer); }
    virtual char*   LogReadSafeString() { return ReadSafeString(); }
    virtual char*   LogReadSafeStringLong() { return ReadSafeStringLong(); }
    virtual void    LogSkip(UInt32 deltaByteCount, const char* desc) { Skip(deltaByteCount); }

    // Stream Notes for Logging 
    virtual void LogStringString(const char* s) { }
    virtual void LogSubStreamStart(const char* desc) { }
    virtual void LogSubStreamEnd() { }
    virtual void LogSubStreamPushDesc(const char* desc) { }
#endif
    void LogVoidFunc() { }

    // Optimization for small Reads
    virtual UInt8   ReadByte();
    virtual hsBool  Read4Bytes(void *buffer);   // Reads 4 bytes,  return true if success 
    virtual hsBool  Read8Bytes(void *buffer);   // Reads 8 bytes,  return true if success 
    virtual hsBool  Read12Bytes(void *buffer);  // Reads 12 bytes, return true if success

    virtual UInt32  GetEOF();
    UInt32          GetSizeLeft();
    virtual void    CopyToMem(void* mem);
    virtual hsBool  IsCompressed() { return false; }

    UInt32          WriteString(const char cstring[]);
    UInt32          WriteFmt(const char * fmt, ...);
    UInt32          WriteFmtV(const char * fmt, va_list av);

    UInt32      WriteSafeStringLong(const char *string);    // uses 4 bytes for length
    UInt32      WriteSafeWStringLong(const wchar_t *string);
    char *      ReadSafeStringLong();
    wchar_t *   ReadSafeWStringLong();

    UInt32      WriteSafeString(const char *string);        // uses 2 bytes for length
    UInt32      WriteSafeWString(const wchar_t *string);
    char *      ReadSafeString();
    wchar_t *   ReadSafeWString();

    hsBool      GetToken(char *s, UInt32 maxLen=UInt32(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    hsBool      ReadLn(char* s, UInt32 maxLen=UInt32(-1), const char beginComment=kComment, const char endComment=kEolnCode);
    
    bool        Readbool();
    hsBool      ReadBool();
    void        ReadBool(int count, hsBool values[]);
    UInt16      ReadSwap16();
    void        ReadSwap16(int count, UInt16 values[]);
    UInt32      ReadSwap32();
    void        ReadSwap32(int count, UInt32 values[]);
    UInt32      ReadUnswap32();

    void            Writebool(bool value);
    void            WriteBool(hsBool value);
    void            WriteBool(int count, const hsBool values[]);
    void            WriteByte(UInt8 value);
    void            WriteSwap16(UInt16 value);
    void            WriteSwap16(int count, const UInt16 values[]);
    void            WriteSwap32(UInt32 value);
    void            WriteSwap32(int count, const  UInt32 values[]);
    void            WriteUnswap32(UInt32 value);


    /* Overloaded  Begin (8 & 16 & 32 int)*/
    /* yes, swapping an 8 bit value does nothing, just useful*/
    void        ReadSwap(bool* value) { *value = this->ReadByte() ? true : false; }
    void        ReadSwap(UInt8* value) { *value = this->ReadByte(); }
    void        ReadSwap(int count, UInt8 values[]) { this->Read(count, values); }
    void        ReadSwap(UInt16* value) { *value = this->ReadSwap16(); }
    void        ReadSwap(int count, UInt16 values[]) { this->ReadSwap16(count, values); }
    void        ReadSwap(UInt32* value) { *value = this->ReadSwap32(); }
    void        ReadSwap(int count, UInt32 values[]) { this->ReadSwap32(count, values); }
#ifdef STREAM_LOGGER
                // Begin LogReadSwaps
    virtual void    LogReadSwap(bool* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwap(UInt8* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, UInt8 values[], const char* desc) { this->ReadSwap(count, values); }
    virtual void    LogReadSwap(UInt16* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, UInt16 values[], const char* desc) { this->ReadSwap(count, values); }
    virtual void    LogReadSwap(UInt32* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, UInt32 values[], const char* desc) { this->ReadSwap(count, values); }
                // End LogReadSwaps
#endif
    void            WriteSwap(bool value) { this->Write(1,&value); }
    void            WriteSwap(UInt8 value) { this->Write(1,&value); }
    void            WriteSwap(int count, const UInt8 values[]) { this->Write(count, values); }
    void            WriteSwap(UInt16 value) { this->WriteSwap16(value); }
    void            WriteSwap(int count, const UInt16 values[]) { this->WriteSwap16(count, values); }
    void            WriteSwap(UInt32 value) { this->WriteSwap32(value); }
    void            WriteSwap(int count, const  UInt32 values[]) { this->WriteSwap32(count, values); }
    void        ReadSwap(Int8* value) { *value = this->ReadByte(); }
    void        ReadSwap(int count, Int8 values[]) { this->Read(count, values); }
    void        ReadSwap(char* value) { *value = (char)this->ReadByte(); }
    void        ReadSwap(int count, char values[]) { this->Read(count, values); }
    void        ReadSwap(Int16* value) { *value = (Int16)this->ReadSwap16(); }
    void        ReadSwap(int count, Int16 values[]) { this->ReadSwap16(count, (UInt16*)values); }
    void        ReadSwap(Int32* value) { *value = (Int32)this->ReadSwap32(); }
    void        ReadSwap(int count, Int32 values[]) { this->ReadSwap32(count, (UInt32*)values); }
#ifdef STREAM_LOGGER
                // Begin LogReadSwaps
    virtual void    LogReadSwap(Int8* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, Int8 values[], const char* desc) { this->ReadSwap(count, values); }
    virtual void    LogReadSwap(char* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, char values[], const char* desc) { this->ReadSwap(count, values); }
    virtual void    LogReadSwap(Int16* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, Int16 values[], const char* desc) { this->ReadSwap(count, (UInt16*)values); }
    virtual void    LogReadSwap(Int32* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, Int32 values[], const char* desc) { this->ReadSwap(count, (UInt32*)values); }
    virtual void    LogReadSwap(int* value, const char* desc) { this->ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, int values[], const char* desc) { this->ReadSwap(count, (UInt32*)values); }
                // End LogReadSwaps
#endif
    void            WriteSwap(Int8 value) { this->Write(1,&value); }
    void            WriteSwap(int count, const Int8 values[]) { this->Write(count, values); }
    void            WriteSwap(char value) { this->Write(1,(UInt8*)&value); }
    void            WriteSwap(int count, const char values[]) { this->Write(count, (UInt8*)values); }
    void            WriteSwap(Int16 value) { this->WriteSwap16((UInt16)value); }
    void            WriteSwap(int count, const Int16 values[]) { this->WriteSwap16(count, (UInt16*)values); }
    void            WriteSwap(Int32 value) { this->WriteSwap32((UInt32)value); }
    void            WriteSwap(int count, const  Int32 values[]) { this->WriteSwap32(count, (UInt32*)values); }
    /* Overloaded  End */


#if HS_CAN_USE_FLOAT
    float           ReadSwapFloat();
    void            ReadSwapFloat(int count, float values[]);
    double          ReadSwapDouble();
    void            ReadSwapDouble(int count, double values[]);
    float           ReadUnswapFloat();
    void            WriteSwapFloat(float value);
    void            WriteSwapFloat(int count, const float values[]);
    void            WriteSwapDouble(double value);
    void            WriteSwapDouble(int count, const double values[]);
    void            WriteUnswapFloat(float value);


    /* Overloaded  Begin (Float)*/
    void            ReadSwap(float* value) { *value = ReadSwapFloat(); }
    void            ReadSwap(int count, float values[]) { ReadSwapFloat(count, values); }
    void            ReadSwap(double* value) { *value = ReadSwapDouble(); }
    void            ReadSwap(int count, double values[]) { ReadSwapDouble(count, values); }
#ifdef STREAM_LOGGER
                    // Begin LogReadSwaps
    virtual void    LogReadSwap(float* value, const char* desc) { ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, float values[], const char* desc) { ReadSwap(count, values); }
    virtual void    LogReadSwap(double* value, const char* desc) { ReadSwap(value); }
    virtual void    LogReadSwapArray(int count, double values[], const char* desc) { ReadSwap(count, values); }
                    // End LogReadSwaps
#endif
    void            WriteSwap(float value) { WriteSwapFloat(value); }
    void            WriteSwap(int count, const float values[]) { WriteSwapFloat(count, values); }
    void            WriteSwap(double value) { WriteSwapDouble(value); }
    void            WriteSwap(int count, const double values[]) { WriteSwapDouble(count, values); }
    /* Overloaded End */
#endif

#if HS_SCALAR_IS_FIXED
    hsFixed     ReadSwapScalar() { return (hsFixed)this->ReadSwap32(); }
    void            ReadSwapScalar(int count, hsFixed values[])
                {
                    this->ReadSwap32(count, (UInt32*)values);
                }
    hsFixed     ReadUnswapScalar() { return (hsFixed)this->ReadUnswap32(); }


    void            WriteSwapScalar(hsFixed value) { this->WriteSwap32(value); }
    void            WriteSwapScalar(int count, const hsFixed values[])
                {
                    this->WriteSwap32(count, (UInt32*)values);
                }
    void            WriteUnswapScalar(hsFixed value) { this->WriteUnswap32(value); }


    /* Overloaded Begin (Scalar) */
    void        ReadSwap(hsFixed* value) { this->ReadSwap((UInt32*)value); }
    void            ReadSwap(int count, hsFixed values[]) { this->ReadSwap(count, (UInt32*)values); }
    void            WriteSwap(hsFixed value) { this->WriteSwap((UInt32)value); }
    void            WriteSwap(int count, const hsFixed values[]) { this->WriteSwap(count, (UInt32*)values); }
    /* Overloaded End */

#else
    float           ReadSwapScalar() { return (float)this->ReadSwapFloat(); }
    void            ReadSwapScalar(int count, float values[])
                {
                    this->ReadSwapFloat(count, (float*)values);
                }
    float           ReadUnswapScalar() { return (float)this->ReadUnswapFloat(); }
    void            WriteSwapScalar(float value) { this->WriteSwapFloat(value); }
    void            WriteSwapScalar(int count, const float values[])
                {
                    this->WriteSwapFloat(count, (float*)values);
                }
    void            WriteUnswapScalar(float value) { this->WriteUnswapFloat(value); }
#endif

    void            WriteSwapAtom(UInt32 tag, UInt32 size);
    UInt32      ReadSwapAtom(UInt32* size);


    /* Overloaded  Begin (Atom)*/
    void            WriteSwap(UInt32* tag, UInt32 size) { WriteSwapAtom(*tag, size); }
    void            ReadSwap(UInt32* tag, UInt32 *size) { *tag = ReadSwapAtom(size); }
    /* Overloaded  End */
    virtual void    VirtualSetPosition(UInt32 pos, VDB_Type ){ SetPosition(pos); };
    virtual hsPackFileSys::FileEntry *GetFileEntry() { return nil; }  // Streams from Packfiles can return a FileEntry

};

class hsStreamable {
public:
    virtual void    Read(hsStream* stream) = 0;
    virtual void    Write(hsStream* stream) = 0;
    virtual UInt32  GetStreamSize() = 0;
};

class hsFileStream: public hsStream
{   
    UInt32      fRef;
#if HS_BUILD_FOR_PS2
    enum {
        kBufferSize = 2*1024
    };
    UInt32      fFileSize;
    char        fBuffer[kBufferSize];
    Int32       fVirtualFilePointer;
    Int32       fBufferBase;            // offset to top of fBuffer
    hsBool      fBufferIsEmpty;
    hsBool      fWriteBufferUsed;       // In write mode. fBuffer must be flush, when file was closed.
#endif

public:
                hsFileStream();
    virtual     ~hsFileStream();
    virtual hsBool  Open(const char *name, const char *mode = "rb");
    virtual hsBool  Open(const wchar *name, const wchar *mode = L"rb");
    virtual hsBool  Close();

    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void* buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    Truncate();

    virtual UInt32  GetFileRef();
    virtual void    SetFileRef(UInt32 refNum);
};

class hsUNIXStream: public hsStream
{   
    FILE*       fRef;
    char*       fBuff;

public:
    hsUNIXStream(): fRef(0), fBuff(nil) {}
    ~hsUNIXStream();
    virtual hsBool  Open(const char* name, const char* mode = "rb");
    virtual hsBool  Open(const wchar *name, const wchar *mode = L"rb");
    virtual hsBool  Close();

    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void* buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    SetPosition(UInt32 position);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    FastFwd();
    virtual void    Truncate();
    virtual void    Flush();

    FILE*           GetFILE() { return fRef; }
    void            SetFILE(FILE* file) { fRef = file; }

    virtual UInt32  GetEOF();
};

// Small substream class: give it a base stream, an offset and a length, and it'll
// treat all ops as if you had a chunk from the base stream as a separate, vanilla 
// stream of the given length.

class plReadOnlySubStream: public hsStream
{   
    hsStream    *fBase;
    UInt32      fOffset, fLength;

    void    IFixPosition( void );

public:
    plReadOnlySubStream(): fBase( nil ), fOffset( 0 ), fLength( 0 ) {}
    ~plReadOnlySubStream();

    virtual hsBool  Open(const char *, const char *)    { hsAssert(0, "plReadOnlySubStream::Open  NotImplemented"); return false; }
    virtual hsBool  Open(const wchar *, const wchar *)  { hsAssert(0, "plReadOnlySubStream::Open  NotImplemented"); return false; }
    void    Open( hsStream *base, UInt32 offset, UInt32 length );
    virtual hsBool  Close() { fBase = nil; fOffset = 0; fLength = 0; return true; }
    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void* buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    FastFwd();
    virtual void    Truncate();

    virtual UInt32  GetEOF();
};

class hsRAMStream : public hsStream {
    hsAppender          fAppender;
    hsAppenderIterator  fIter;
public:
                hsRAMStream();
                hsRAMStream(UInt32 chunkSize);
    virtual     ~hsRAMStream();

    virtual hsBool  Open(const char *, const char *)    { hsAssert(0, "hsRAMStream::Open  NotImplemented"); return false; }
    virtual hsBool  Open(const wchar *, const wchar *)  { hsAssert(0, "hsRAMStream::Open  NotImplemented"); return false; }
    virtual hsBool  Close()             { hsAssert(0, "hsRAMStream::Close  NotImplemented"); return false; }

    
    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void * buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    Truncate();

    virtual UInt32  GetEOF();
    virtual void    CopyToMem(void* mem);

    void            Reset();        // clears the buffers
};

class hsNullStream : public hsStream {
public:

    virtual hsBool  Open(const char *, const char *)    { return true; }
    virtual hsBool  Open(const wchar *, const wchar *)  { return true; }
    virtual hsBool  Close()             { return true; }

    virtual UInt32  Read(UInt32 byteCount, void * buffer);  // throw's exception
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    Truncate();

    UInt32      GetBytesWritten() const { return fBytesRead; }
    void            Reset( ) { fBytesRead = 0;   }
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

    virtual void    Init(int size, const void* data) { fStart=((char*)data); fData=((char*)data); fStop=((char*)data + size); }
    virtual hsBool  Open(const char *, const char *)    { hsAssert(0, "hsReadOnlyStream::Open  NotImplemented"); return false; }
    virtual hsBool  Open(const wchar *, const wchar *)  { hsAssert(0, "hsReadOnlyStream::Open  NotImplemented"); return false; }
    virtual hsBool  Close()             { hsAssert(0, "hsReadOnlyStream::Close  NotImplemented"); return false; }
    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void * buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);    // throws exception
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    Truncate();
    virtual UInt32  GetBytesRead() const { return fBytesRead; }
    virtual UInt32  GetEOF() { return (UInt32)(fStop-fStart); }
    virtual void    CopyToMem(void* mem);
};

// write only mem stream
class hsWriteOnlyStream : public hsReadOnlyStream {
public:
    hsWriteOnlyStream(int size, const void* data) : hsReadOnlyStream(size, data) {}
    hsWriteOnlyStream() {}

    virtual hsBool  Open(const char *, const char *)    { hsAssert(0, "hsWriteOnlyStream::Open  NotImplemented"); return false; }
    virtual hsBool  Open(const wchar *, const wchar *)  { hsAssert(0, "hsWriteOnlyStream::Open  NotImplemented"); return false; }
    virtual hsBool  Close()             { hsAssert(0, "hsWriteOnlyStream::Close  NotImplemented"); return false; }
    virtual UInt32  Read(UInt32 byteCount, void * buffer);  // throws exception
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);    
    virtual UInt32  GetBytesRead() const { return 0; }
    virtual UInt32  GetBytesWritten() const { return fBytesRead; }
};

// circular queue stream
class hsQueueStream : public hsStream {
private:
    char* fQueue;
    UInt32 fReadCursor;
    UInt32 fWriteCursor;
    UInt32 fSize;
    
public:
    hsQueueStream(Int32 size);
    ~hsQueueStream();

    virtual hsBool  Open(const char *, const char *)    { hsAssert(0, "hsQueueStream::Open  NotImplemented"); return false; }
    virtual hsBool  Open(const wchar *, const wchar *)  { hsAssert(0, "hsQueueStream::Open  NotImplemented"); return false; }
    virtual hsBool  Close()             { hsAssert(0, "hsQueueStream::Close  NotImplemented"); return false; }

    virtual UInt32  Read(UInt32 byteCount, void * buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    FastFwd();
    virtual hsBool  AtEnd();

    UInt32 GetSize() { return fSize; }
    const char* GetQueue() { return fQueue; }
    UInt32 GetReadCursor() { return fReadCursor; }
    UInt32 GetWriteCursor() { return fWriteCursor; }
};

class hsBufferedStream : public hsStream
{
    FILE* fRef;
    UInt32 fFileSize;

    enum { kBufferSize = 2*1024 };
    char fBuffer[kBufferSize];
    // If the buffer is empty, this is zero.  Otherwise it is the size of the
    // buffer (if we read a full block), or something less than that if we read
    // a partial block at the end of the file.
    UInt32 fBufferLen;

    hsBool fWriteBufferUsed;

#ifdef HS_DEBUGGING
    // For doing statistics on how efficient we are
    int fBufferHits, fBufferMisses;
    UInt32 fBufferReadIn, fBufferReadOut, fReadDirect, fLastReadPos;
    char* fFilename;
    const char* fCloseReason;
#endif

public:
    hsBufferedStream();
    virtual ~hsBufferedStream();

    virtual hsBool  Open(const char* name, const char* mode = "rb");
    virtual hsBool  Open(const wchar* name, const wchar* mode = L"rb");
    virtual hsBool  Close();

    virtual hsBool  AtEnd();
    virtual UInt32  Read(UInt32 byteCount, void* buffer);
    virtual UInt32  Write(UInt32 byteCount, const void* buffer);
    virtual void    Skip(UInt32 deltaByteCount);
    virtual void    Rewind();
    virtual void    Truncate();
    virtual UInt32  GetEOF();

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
