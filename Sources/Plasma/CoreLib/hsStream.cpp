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

#include "hsStream.h"

#include "hsEndian.h"
#include "hsExceptions.h"

#include <cctype>
#if HS_BUILD_FOR_WIN32
#   include <io.h>
#endif
#include <algorithm>
#include <string_theory/format>

#if HS_BUILD_FOR_UNIX
#include <unistd.h>
#endif

uint32_t hsStream::GetPosition() const
{
    return fPosition;
}

void hsStream::SetPosition(uint32_t position)
{
    if (position == fPosition)
        return;
    Rewind();
    Skip(position);
}

uint32_t hsStream::GetSizeLeft()
{
    uint32_t ret = 0;
    if (GetPosition() > GetEOF())
    {
        hsThrow("Position is beyond EOF");
    }
    else
    {
        ret = GetEOF() - GetPosition();
    }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////////

uint32_t hsStream::WriteSafeString(const ST::string &string)
{
    size_t len = string.size();
    hsAssert(len < 0x1000, ST::format("string len of {} is too long for WriteSafeString {}",
        len, string).c_str() );

    WriteLE16(static_cast<uint16_t>(len | 0xf000));
    if (len > 0)
    {
        uint32_t i;
        const char *buffp = string.c_str();
        for (i = 0; i < len; i++)
        {
            WriteByte(static_cast<uint8_t>(~buffp[i]));
        }
        return i;
    }
    else
        return 0;
}

uint32_t hsStream::WriteSafeWString(const ST::string &string)
{
    ST::utf16_buffer wbuff = string.to_utf16();
    size_t len = wbuff.size();
    hsAssert(len < 0x1000, ST::format("string len of {} is too long for WriteSafeWString",
        len).c_str() );

    WriteLE16(static_cast<uint16_t>(len | 0xf000));
    if (len > 0)
    {
        const char16_t *buffp = wbuff.data();
        for (uint32_t i=0; i<len; i++)
        {
            WriteLE16(static_cast<uint16_t>(~buffp[i]));
        }
        WriteLE16(static_cast<uint16_t>(0));
    }
    return 0;
}

ST::string hsStream::ReadSafeString()
{
    ST::char_buffer name;
    uint16_t numChars = ReadLE16();

    hsAssert(numChars & 0xf000, "SafeString in old (pre-2003) format");

    numChars &= ~0xf000;
    hsAssert(numChars <= GetSizeLeft(), "Bad string");
    if (numChars > 0 && numChars <= GetSizeLeft())
    {
        name.allocate(numChars);
        Read(numChars, name.data());

        // if the high bit is set, flip the bits. Otherwise it's a normal string, do nothing.
        if (name[0] & 0x80)
        {
            int i;
            for (i = 0; i < numChars; i++)
                name[i] = ~name[i];
        }
    }

    return name;
}

ST::string hsStream::ReadSafeWString()
{
    ST::utf16_buffer retVal;
    uint32_t numChars = ReadLE16();
    
    numChars &= ~0xf000;
    hsAssert(numChars <= GetSizeLeft()/2, "Bad string");
    if (numChars > 0 && numChars <= (GetSizeLeft()/2)) // divide by two because each char is two bytes
    {
        retVal.allocate(numChars);
        for (int i=0; i<numChars; i++)
            retVal[i] = ReadLE16();
        (void)ReadLE16(); // we wrote the null out, read it back in

        for (int i=0; i<numChars; i++)
            retVal[i] = ~retVal[i];
    }

    return ST::string::from_utf16(retVal);
}

bool hsStream::ReadBOOL()
{
    uint32_t val;
    this->Read(sizeof(uint32_t), &val);
    return val != 0;
}

bool hsStream::ReadBool() // Virtual, faster version in sub classes
{
    return (this->ReadByte() != 0);
}

uint8_t hsStream::ReadByte()
{
    uint8_t   value;

    this->Read(sizeof(uint8_t), &value);
    return value;
}

bool hsStream::IsTokenSeparator(char c)
{
    return (isspace(static_cast<unsigned char>(c)) || c==',' || c=='=');
}

bool hsStream::GetToken(char *s, uint32_t maxLen, const char beginComment, const char endComment)
{
    char c;
    char endCom;
        endCom = endComment;

    while( true )
    {
        while( !AtEnd() && IsTokenSeparator(c = ReadByte()) )
            /* empty */;

        if( AtEnd() )
            return false;

        if( beginComment != c )
            break;

        // skip to end of comment
        while( !AtEnd() && (endCom != (c = ReadByte())) )
            /* empty */;
    }

    s[0] = c;
    uint32_t k = 1;
    while( !AtEnd() && !IsTokenSeparator(c = ReadByte()) )
    {
        if( k < maxLen )
            s[k++] = c;
    }
    s[k] = 0;

    return true;
}

bool hsStream::ReadLn(char *s, uint32_t maxLen, const char beginComment, const char endComment)
{
    char c;
    char endCom;
        endCom = endComment;

    while( true )
    {
        while( !AtEnd() && strchr("\r\n",c = ReadByte()) )
            /* empty */;

        if( AtEnd() )
            return false;

        if( beginComment != c )
            break;

        // skip to end of comment
        while( !AtEnd() && (endCom != (c = ReadByte())) )
            /* empty */;
    }

    s[0] = c;
    uint32_t k = 1;
    while( !AtEnd() && !strchr("\r\n",c = ReadByte()) )
    {
        if( k < maxLen )
            s[k++] = c;
    }
    s[k] = 0;

    return true;
}

bool hsStream::ReadLn(ST::string& s, const char beginComment, const char endComment)
{
    ST::string_stream ss;
    char c;
    char endCom = endComment;

    while (true) {
        while (!AtEnd() && strchr("\r\n", c = ReadByte()))
            /* empty */;

        if (AtEnd())
            return false;

        if (beginComment != c)
            break;

        // skip to end of comment
        while (!AtEnd() && (endCom != (c = ReadByte())))
            /* empty */;
    }

    ss.append_char(c);
    while (!AtEnd() && !strchr("\r\n", c = ReadByte()))
        ss.append_char(c);
    s = ss.to_string();

    return true;
}

uint16_t hsStream::ReadLE16()
{
    uint16_t  value;
    this->Read(sizeof(uint16_t), &value);
    value = hsToLE16(value);
    return value;
}

void hsStream::ReadLE16(size_t count, uint16_t values[])
{
    this->Read(count * sizeof(uint16_t), values);
    for (size_t i = 0; i < count; i++)
        values[i] = hsToLE16(values[i]);
}

uint32_t hsStream::ReadLE32()
{
    uint32_t  value;
    Read(sizeof(uint32_t), &value);
    value = hsToLE32(value);
    return value;
}

void hsStream::ReadLE32(size_t count, uint32_t values[])
{
    this->Read(count * sizeof(uint32_t), values);
    for (size_t i = 0; i < count; i++)
        values[i] = hsToLE32(values[i]);
}

uint64_t hsStream::ReadLE64()
{
    uint64_t  value;
    Read(sizeof(uint64_t), &value);
    value = hsToLE64(value);
    return value;
}

void hsStream::ReadLE64(size_t count, uint64_t values[])
{
    this->Read(count * sizeof(uint64_t), values);
    for (size_t i = 0; i < count; i++)
        values[i] = hsToLE64(values[i]);
}

double hsStream::ReadLEDouble()
{
    double  value;
    Read(sizeof(double), &value);
    value = hsToLEDouble(value);
    return value;
}

void hsStream::ReadLEDouble(size_t count, double values[])
{
    this->Read(count * sizeof(double), values);
    for (size_t i = 0; i < count; i++)
        values[i] = hsToLEDouble(values[i]);
}


float hsStream::ReadLEFloat()
{
    float   value;
    Read(sizeof(float), &value);
    value = hsToLEFloat(value);
    return value;
}

void hsStream::ReadLEFloat(size_t count, float values[])
{
    this->Read(count * sizeof(float), values);
    for (size_t i = 0; i < count; i++)
        values[i] = hsToLEFloat(values[i]);
}

void hsStream::WriteBOOL(bool value)
{
    uint32_t dst = value ? hsToLE32(1) : 0;
    this->Write(sizeof(uint32_t), &dst);
}

void hsStream::WriteBool(bool value)
{
    uint8_t dst = value ? 1 : 0;
    this->Write(sizeof(uint8_t), &dst);
}

void hsStream::WriteByte(uint8_t value)
{
    this->Write(sizeof(uint8_t), &value);
}

void  hsStream::WriteLE16(uint16_t value)
{
    value = hsToLE16(value);
    this->Write(sizeof(uint16_t), &value);
}

void  hsStream::WriteLE16(size_t count, const uint16_t values[])
{
    for (size_t i = 0; i < count; i++)
        this->WriteLE16(values[i]);
}

void  hsStream::WriteLE32(uint32_t value)
{
    value = hsToLE32(value);
    this->Write(sizeof(uint32_t), &value);
}

void  hsStream::WriteLE32(size_t count, const uint32_t values[])
{
    for (size_t i = 0; i < count; i++)
        this->WriteLE32(values[i]);
}

void hsStream::WriteLE64(uint64_t value)
{
    value = hsToLE64(value);
    this->Write(sizeof(uint64_t), &value);
}

void hsStream::WriteLE64(size_t count, const uint64_t values[])
{
    for (size_t i = 0; i < count; i++)
        this->WriteLE64(values[i]);
}

void hsStream::WriteLEDouble(double value)
{
    value = hsToLEDouble(value);
    this->Write(sizeof(double), &value);
}

void hsStream::WriteLEDouble(size_t count, const double values[])
{
    for (size_t i = 0; i < count; i++)
        this->WriteLEDouble(values[i]);
}

void hsStream::WriteLEFloat(float value)
{
    value = hsToLEFloat(value);
    this->Write(sizeof(float), &value);
}

void hsStream::WriteLEFloat(size_t count, const float values[])
{
    for (size_t i = 0; i < count; i++)
        this->WriteLEFloat(values[i]);
}


//////////////////////////////////////////////////////////////////////////////////////


hsUNIXStream::~hsUNIXStream()
{
    if (fRef) {
        fclose(fRef);
    }
}

bool hsUNIXStream::Open(const plFileName &name, const char *mode)
{
    fPosition = 0;
    fRef = plFileSystem::Open(name, mode);
    return (fRef) ? true : false;
}

uint32_t hsUNIXStream::Read(uint32_t bytes,  void* buffer)
{
    if (!fRef || !bytes)
        return 0;
    size_t numItems = ::fread(buffer, 1 /*size*/, bytes /*count*/, fRef);
    fPosition += numItems;
    if (numItems < bytes)
    {
        hsAssert(feof(fRef), ST::format("Error on UNIX Read (ferror = {})", ferror(fRef)).c_str());
    }
    return numItems;
}

bool  hsUNIXStream::AtEnd()
{
    if (!fRef)
        return 1;
    bool rVal;
    int x = getc(fRef);
    rVal = feof(fRef) != 0;
    ungetc(x, fRef);
    return rVal;
}

uint32_t hsUNIXStream::Write(uint32_t bytes, const void* buffer)
{
    if (!fRef)
        return 0;
    fPosition += bytes;
    return fwrite(buffer, bytes, 1, fRef);
}

void hsUNIXStream::SetPosition(uint32_t position)
{
    if (!fRef || (position == fPosition))
        return;
    fPosition = position;
    (void)::fseek(fRef, position, SEEK_SET);
}

void hsUNIXStream::Skip(uint32_t delta)
{
    if (!fRef)
        return;
    fPosition += delta;
    (void)::fseek(fRef, delta, SEEK_CUR);
}

void hsUNIXStream::Rewind()
{
    if (!fRef)
        return;
    fPosition = 0;
    (void)::fseek(fRef, 0, SEEK_SET);
}

void hsUNIXStream::FastFwd()
{
    if (!fRef)
        return;
    (void)::fseek(fRef, 0, SEEK_END);
    fPosition = ftell(fRef);
}

uint32_t  hsUNIXStream::GetEOF()
{
    if( !fRef )
        return 0;

    long oldPos = ftell( fRef );
    (void)::fseek( fRef, 0, SEEK_END );
    uint32_t end = (uint32_t)ftell( fRef );
    (void)::fseek( fRef, oldPos, SEEK_SET );

    return end;
}

void hsUNIXStream::Truncate()
{
    if (!fRef)
        return;
    int handle = fileno(fRef);
#if HS_BUILD_FOR_WIN32
    _chsize(handle, fPosition);
#else
    ftruncate(handle, fPosition);
#endif
}

void hsUNIXStream::Flush()
{
    if (!fRef)
        return;
    (void)::fflush(fRef);
}


//////////////////////////////////////////////////////////////////////////////////////

plReadOnlySubStream::plReadOnlySubStream(hsStream* base, uint32_t offset, uint32_t length)
    : fBase(base), fOffset(offset), fLength(length)
{
    fBase->SetPosition( fOffset );
    IFixPosition();
}

void    plReadOnlySubStream::IFixPosition()
{
    fPosition = fBase->GetPosition() - fOffset;
}

bool  plReadOnlySubStream::AtEnd()
{
    if( fPosition >= fLength )
        return true;
    return false;
}

uint32_t  plReadOnlySubStream::Read(uint32_t byteCount, void* buffer)
{
    if( byteCount > GetSizeLeft() )
    {
        hsThrow("Attempting to read past end of stream");
        byteCount = GetSizeLeft();
    }

    uint32_t read = fBase->Read( byteCount, buffer );
    IFixPosition();
    return read;
}

uint32_t  plReadOnlySubStream::Write(uint32_t byteCount, const void* buffer)
{
    hsAssert( false, "Write not allowed on an plReadOnlySubStream" );
    return 0;
}

void    plReadOnlySubStream::Skip(uint32_t deltaByteCount)
{
    fBase->Skip( deltaByteCount );
    IFixPosition();
}

void    plReadOnlySubStream::Rewind()
{
    fBase->SetPosition( fOffset );
    IFixPosition();
}

void    plReadOnlySubStream::FastFwd()
{
    fBase->SetPosition( fOffset + fLength );
    IFixPosition();
}

void    plReadOnlySubStream::Truncate()
{
    hsAssert( false, "Can't truncate a read-only stream" );
}

uint32_t  plReadOnlySubStream::GetEOF()
{
    return fLength;
}

//////////////////////////////////////////////////////////////////////////////////////

bool hsRAMStream::AtEnd()
{
    return (fPosition >= fVector.size());
}

uint32_t hsRAMStream::Read(uint32_t byteCount, void * buffer)
{
    if (fPosition + byteCount > fVector.size()) {
        byteCount = fVector.size() - fPosition;
    }
    
    memcpy(buffer, fVector.data() + fPosition, byteCount);

    fPosition += byteCount;

    return byteCount;
}

uint32_t hsRAMStream::Write(uint32_t byteCount, const void* buffer)
{
    size_t spaceUntilEof = fVector.size() - fPosition;
    if (byteCount <= spaceUntilEof) {
        hsAssert(fVector.data(), "Trying to write to a null RAM buffer");
        memcpy(fVector.data() + fPosition, buffer, byteCount);
    } else {
        if (spaceUntilEof) {
            hsAssert(fVector.data(), "Trying to write to a null RAM buffer");
            memcpy(fVector.data() + fPosition, buffer, spaceUntilEof);
        }
        auto buf = static_cast<const uint8_t*>(buffer);
        fVector.insert(fVector.end(), buf + spaceUntilEof, buf + byteCount);
    }

    fPosition += byteCount;

    return byteCount;
}

void hsRAMStream::Skip(uint32_t deltaByteCount)
{
    fPosition += deltaByteCount;
}

void hsRAMStream::Rewind()
{
    fPosition = 0;
}

void hsRAMStream::FastFwd()
{
    fPosition = fVector.size();
}

void hsRAMStream::Truncate()
{
    fVector.resize(fPosition);
}

uint32_t hsRAMStream::GetEOF()
{
    return fVector.size();
}

void hsRAMStream::CopyToMem(void* mem)
{
    memcpy(mem, fVector.data(), fVector.size());
}

void hsRAMStream::Erase(uint32_t bytes)
{
    hsAssert(fPosition+bytes <= fVector.size(), "Erasing past end of stream");

    fVector.erase(fVector.begin()+fPosition, fVector.begin()+fPosition+bytes);
}

void hsRAMStream::Reset()
{
    fPosition = 0;
    fVector.clear();
}

const void *hsRAMStream::GetData()
{
    return fVector.data();
}

//////////////////////////////////////////////////////////////////////

bool hsNullStream::AtEnd()
{
    return true;
}

uint32_t hsNullStream::Read(uint32_t byteCount, void * buffer)
{
    hsThrow("hsNullStream: Can't read from this stream!");
    return 0;
}

uint32_t hsNullStream::Write(uint32_t byteCount, const void* buffer)
{
    fPosition += byteCount;

    return byteCount;
}

void hsNullStream::Skip(uint32_t deltaByteCount)
{
    fPosition += deltaByteCount;
}

void hsNullStream::Rewind()
{
    fPosition = 0;
}

void hsNullStream::FastFwd()
{}

void hsNullStream::Truncate()
{
}

/////////////////////////////////////////////////////////////////////////////////

bool hsReadOnlyStream::AtEnd()
{
    return fData >= fStop;
}

uint32_t hsReadOnlyStream::Read(uint32_t byteCount, void* buffer)
{
    if (fData + byteCount > fStop)
    {
        hsThrow("Attempting to read past end of stream");
        byteCount = GetSizeLeft();
    }

    memmove(buffer, fData, byteCount);
    fData += byteCount;
    fPosition += byteCount;
    return byteCount;
}

uint32_t hsReadOnlyStream::Write(uint32_t byteCount, const void* buffer)
{
    hsThrow( "can't write to a readonly stream");
    return 0;
}

void hsReadOnlyStream::Skip(uint32_t deltaByteCount)
{
    fPosition += deltaByteCount;
    fData += deltaByteCount;
    if (fData > fStop)
        hsThrow( "Skip went past end of stream");
}

void hsReadOnlyStream::Rewind()
{
    fPosition = 0;
    fData = fStart;
}

void hsReadOnlyStream::FastFwd()
{
    fPosition = GetEOF();
    fData = fStop;
}

void hsReadOnlyStream::Truncate()
{
    hsThrow( "can't write to a readonly stream");
}

void hsReadOnlyStream::CopyToMem(void* mem)
{
    if (fData < fStop)
        memmove(mem, fData, fStop-fData);
}


////////////////////////////////////////////////////////////////////////////////////

bool hsWriteOnlyStream::AtEnd()
{
    return fData >= fStop;
}

uint32_t hsWriteOnlyStream::Read(uint32_t byteCount, void* buffer)
{
    hsThrow( "can't read to a writeonly stream");
    return 0;
}

uint32_t hsWriteOnlyStream::Write(uint32_t byteCount, const void* buffer)
{
    if (fData + byteCount > fStop)
        hsThrow("Write past end of stream");
    memmove(fData, buffer, byteCount);
    fData += byteCount;
    fPosition += byteCount;
    return byteCount;
}

void hsWriteOnlyStream::Skip(uint32_t deltaByteCount)
{
    fPosition += deltaByteCount;
    fData += deltaByteCount;
    if (fData > fStop) {
        hsThrow("Skip went past end of stream");
    }
}

void hsWriteOnlyStream::Rewind()
{
    fPosition = 0;
    fData = fStart;
}

void hsWriteOnlyStream::FastFwd()
{
    fPosition = GetEOF();
    fData = fStop;
}

void hsWriteOnlyStream::Truncate()
{
    hsThrow("can't change size of hsWriteOnlyStream");
}

void hsWriteOnlyStream::CopyToMem(void* mem)
{
    if (fData < fStop) {
        memmove(mem, fData, fStop - fData);
    }
}


///////////////////////////////////////////////////////////////////////////////////

hsQueueStream::hsQueueStream(int32_t size) :
    fReadCursor(), fWriteCursor(), fSize(size)
{
    fQueue = new char[fSize];
}

hsQueueStream::~hsQueueStream()
{
    delete [] fQueue;
}

uint32_t hsQueueStream::Read(uint32_t byteCount, void * buffer)
{
    hsAssert(fWriteCursor < fSize,"hsQueueStream: WriteCursor out of range.");
    hsAssert(fReadCursor < fSize,"hsQueueStream: ReadCursor out of range.");

    int32_t limit, length, total;
    
    limit = fWriteCursor >= fReadCursor ? fWriteCursor : fSize;
    length = std::min(limit-fReadCursor, byteCount);
    memmove(buffer, fQueue+fReadCursor, length);
    fReadCursor += length;
    fReadCursor %= fSize;
    total = length;
        
    if (length < byteCount && limit != fWriteCursor)
    {
        limit = fWriteCursor;
        length = std::min(limit, static_cast<int32_t>(byteCount)-length);
        memmove(static_cast<char*>(buffer)+total, fQueue, length);
        fReadCursor = length;
        total += length;
    }

    return total;
}

uint32_t hsQueueStream::Write(uint32_t byteCount, const void* buffer)
{
    hsAssert(fWriteCursor < fSize,"hsQueueStream: WriteCursor out of range.");
    hsAssert(fReadCursor < fSize,"hsQueueStream: ReadCursor out of range.");

    int32_t length;

    length = std::min(fSize-fWriteCursor, byteCount);
    memmove(fQueue+fWriteCursor, buffer, length);
    if (fReadCursor > fWriteCursor)
    {
#if 0
        if (fReadCursor < fWriteCursor+length+1)
            hsStatusMessage("ReadCursor wrapped");
#endif
        fReadCursor = std::min(fReadCursor, fWriteCursor+length+1);
        fReadCursor %= fSize;
    }
    fWriteCursor += length;
    fWriteCursor %= fSize;

    if (length < byteCount)
    {
        Write(byteCount - length,static_cast<const char*>(buffer)+length);
    }

    return byteCount;
}

void hsQueueStream::Skip(uint32_t deltaByteCount)
{
    int32_t limit, length;
    
    limit = fWriteCursor >= fReadCursor ? fWriteCursor : fSize;
    length = std::min(limit-fReadCursor, deltaByteCount);
    fReadCursor += length;

    if (length < deltaByteCount && limit != fWriteCursor)
    {
        limit = fWriteCursor;
        length = std::min(limit, static_cast<int32_t>(deltaByteCount)-length);
        fReadCursor = length;
    }
    else
    {
        fReadCursor %= fSize;
    }
}

void hsQueueStream::Rewind()
{
    fReadCursor = fWriteCursor+1;
    fReadCursor %= fSize;
}

void hsQueueStream::FastFwd()
{
    fReadCursor = fWriteCursor;
}

void hsQueueStream::Truncate()
{
    fWriteCursor = fReadCursor;
}

bool hsQueueStream::AtEnd()
{
    return fReadCursor == fWriteCursor;
}

///////////////////////////////////////////////////////////////////////////////
// hsBufferedStream
///////////////////////////////////////////////////////////////////////////////

//#define LOG_BUFFERED

hsBufferedStream::hsBufferedStream()
: fRef()
, fFileSize()
, fBufferLen()
, fWriteBufferUsed()
#ifdef HS_DEBUGGING
, fBufferHits()
, fBufferMisses()
, fBufferReadIn()
, fBufferReadOut()
, fReadDirect()
, fLastReadPos()
, fCloseReason()
#endif
{
}

hsBufferedStream::~hsBufferedStream()
{
    if (fRef) {
        fclose(fRef);
    }

#ifdef LOG_BUFFERED
    hsUNIXStream s;
    static bool firstClose = true;
    if (firstClose) {
        firstClose = false;
        s.Open("log\\BufferedStream.csv", "wt");
        s.WriteString("File,Hits,Misses,Read In,Read Out,Read Direct,% Wasted,Reason\n");
    } else {
        s.Open("log\\BufferedStream.csv", "at");
    }

    int wasted = 100;
    if (fBufferReadIn + fReadDirect > 0) {
        wasted -= int((float(fBufferReadOut+fReadDirect) / float(fBufferReadIn+fReadDirect)) * 100.f);
    }

    s.WriteString(ST::format("{},{},{},{},{},{},{},{}\n",
        fFilename, fBufferHits, fBufferMisses, fBufferReadIn, fBufferReadOut, fReadDirect,
        wasted,
        fCloseReason ? fCloseReason : "Unknown"));
#endif // LOG_BUFFERED
}

bool hsBufferedStream::Open(const plFileName& name, const char* mode)
{
    hsAssert(!fRef, "hsBufferedStream:Open Stream already opened");
    fRef = plFileSystem::Open(name, mode);
    if (!fRef)
        return false;

    SetFileRef(fRef);

#ifdef LOG_BUFFERED
    fBufferHits = fBufferMisses = 0;
    fBufferReadIn = fBufferReadOut = fReadDirect = fLastReadPos = 0;
    fFilename = name;
    fCloseReason = nullptr;
#endif // LOG_BUFFERED

    return true;
}

FILE* hsBufferedStream::GetFileRef()
{
    return fRef;
}

void hsBufferedStream::SetFileRef(FILE* ref)
{
    hsAssert(ref, "bad ref");
    fRef = ref;

    fseek(fRef, 0, SEEK_END);
    fFileSize = ftell(fRef);
    fseek(fRef, 0, SEEK_SET);

    fBufferLen = 0;
    fPosition = 0;
    fWriteBufferUsed = false;
}

uint32_t hsBufferedStream::Read(uint32_t bytes, void* buffer)
{
    hsAssert(fRef, "fRef uninitialized");
    if (!fRef || bytes == 0)
        return 0;

    uint32_t numReadBytes = 0;

    while (bytes > 0 && fPosition < fFileSize)
    {
        // First, see if we've got anything in the buffer
        if (fBufferLen > 0)
        {
            // Figure out how much we can copy out of the buffer
            uint32_t bufferPos = fPosition % kBufferSize;
            uint32_t bytesInBuffer = fBufferLen - bufferPos;
            uint32_t cachedReadSize = bytesInBuffer < bytes ? bytesInBuffer : bytes;

            memcpy(buffer, &fBuffer[bufferPos], cachedReadSize);

            fPosition += cachedReadSize;
            numReadBytes += cachedReadSize;
            bytes -= cachedReadSize;
            buffer = (void*)(((char*)buffer) + cachedReadSize);

            // If we read all the data out of the buffer, set it to empty
            if ((bufferPos + cachedReadSize) == fBufferLen)
                fBufferLen = 0;

#ifdef HS_DEBUGGING
            fLastReadPos = fPosition;
            fBufferHits++;
            fBufferReadOut += cachedReadSize;
#endif
        }

        // Now see if the remaining read (if any) is the size of the buffer or larger.
        // If it is, read as many complete blocks as possible directly into the output buffer.
        if (bytes >= kBufferSize && fPosition % kBufferSize == 0)
        {
            uint32_t directReadSize = bytes - (bytes % kBufferSize);
            hsAssert(ftell(fRef) % kBufferSize == 0 , "read buffer is not in alignment.");
            int amtRead = ::fread(buffer, 1, directReadSize, fRef);
            fPosition += amtRead;
            numReadBytes += amtRead;
            bytes -= amtRead;
            buffer = (void*)(((char*)buffer) + amtRead);
#ifdef HS_DEBUGGING
            fLastReadPos = fPosition;
            fReadDirect += directReadSize;
#endif
        }

        // If we've got bytes left to read and we didn't pass the end of the file, buffer a new block
        if (bytes > 0 && fPosition < fFileSize)
        {
            hsAssert(ftell(fRef) % kBufferSize == 0 , "read buffer is not in alignment.");
            fBufferLen = ::fread(fBuffer, 1, kBufferSize, fRef);

#ifdef HS_DEBUGGING
            // If our last read wasn't at the start of the new buffer, it's a miss.
            if (fLastReadPos != fPosition)
            {
                fBufferMisses++;
                fBufferHits--;
            }

            fBufferReadIn += fBufferLen;
#endif
        }
    }

    return numReadBytes;
}

uint32_t hsBufferedStream::Write(uint32_t bytes, const void* buffer)
{
    hsAssert(fRef, "fRef uninitialized");
    fWriteBufferUsed = true;
    int amtWritten = fwrite((void*)buffer, 1, bytes, fRef);
    fPosition += amtWritten;
    return amtWritten;
}

bool hsBufferedStream::AtEnd()
{
    if (fWriteBufferUsed)
    {
        if (!fRef)
            return true;
        bool rVal;
        int x = getc(fRef);
        rVal = feof(fRef) != 0;
        ungetc(x, fRef);
        return rVal;
    }
    else
    {
        // buffered read
        return fPosition >= fFileSize;
    }
}

void hsBufferedStream::Skip(uint32_t delta)
{
    if (fWriteBufferUsed)
    {
        // buffered write not implemented yet.
        fseek(fRef, delta, SEEK_CUR);
    }
    else
    {
        uint32_t blockStart = ((fPosition + delta) / kBufferSize) * kBufferSize;

        // We've got data in the buffer, see if we can just skip in that
        if (fBufferLen > 0)
        {
            int32_t newBufferPos = int32_t(fPosition % kBufferSize) + int32_t(delta);

            // If we skipped outside of our buffer, invalidate it
            if (newBufferPos < 0 || uint32_t(newBufferPos) >= fBufferLen)
            {
                fBufferLen = 0;
                fseek(fRef, blockStart, SEEK_SET);
            }
        }
        else
            fseek(fRef, blockStart, SEEK_SET);
    }

    fPosition += delta;
}

void hsBufferedStream::Rewind()
{
    if (fWriteBufferUsed)
    {
        // buffered write not implemented yet.
        fseek(fRef, 0, SEEK_SET);
    }
    // If the currently buffered block isn't the first one, invalidate our buffer
    else if (fPosition >= kBufferSize)
        fBufferLen = 0;

    fPosition = 0;
}

void hsBufferedStream::FastFwd()
{
    fseek(fRef, 0, SEEK_END);
    fBufferLen = 0;
    fPosition = ftell(fRef);
}

uint32_t hsBufferedStream::GetEOF()
{
    if (fWriteBufferUsed)
    {
        if (!fRef)
            return 0;

        long oldPos = ftell(fRef);
        fseek(fRef, 0, SEEK_END);
        uint32_t end = (uint32_t)ftell(fRef);
        fseek(fRef, oldPos, SEEK_SET);

        return end;
    }
    else
        return fFileSize;
}

void hsBufferedStream::Truncate()
{
    hsAssert(0, "hsBufferedStream::Truncate unimplemented");
}
