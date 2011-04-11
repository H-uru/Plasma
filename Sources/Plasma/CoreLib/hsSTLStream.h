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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsStream.h"
#include "hsStlUtils.h"

//
// In-memory only
// Erase function lets you cut a chunk out of the middle of the stream
//
class hsVectorStream : public hsStream
{
protected:
	std::vector<Byte> fVector;
	UInt32 fEnd;	// End of file (one past the last byte)

public:
	hsVectorStream();
	hsVectorStream(UInt32 chunkSize);
	virtual ~hsVectorStream();

	virtual hsBool	Open(const char *, const char *)	{ hsAssert(0, "hsVectorStream::Open Not Implemented"); return false; }
	virtual hsBool	Open(const wchar *, const wchar *)	{ hsAssert(0, "hsVectorStream::Open Not Implemented"); return false; }
	virtual hsBool	Close()				{ hsAssert(0, "hsVectorStream::Close Not Implemented"); return false; }
	
	virtual hsBool	AtEnd();
	virtual UInt32	Read(UInt32 byteCount, void * buffer);
	virtual UInt32	Write(UInt32 byteCount, const void* buffer);
	virtual void	Skip(UInt32 deltaByteCount);
	virtual void	Rewind();
	virtual void	FastFwd();
    virtual void    Truncate();

	virtual UInt32	GetEOF();
	virtual void	CopyToMem(void* mem);

	virtual void	Reset();		// clears the buffers

	// Erase number of bytes at the current position
	virtual void	Erase(UInt32 bytes);
	// A pointer to the beginning of the data in the stream.  This is only valid
	// until someone modifies the stream.
	const void		*GetData();
	// In case you want to try and be efficient with your memory allocations
	void Reserve(UInt32 bytes) { fVector.reserve(bytes); }
};

#ifdef HS_BUILD_FOR_WIN32

#include "hsWindows.h"

class hsNamedPipeStream : public hsStream
{
protected:
	HANDLE		fPipe;
	OVERLAPPED	fOverlap;
	hsBool		fReadMode;	// True for read, false for write
	UInt8		fFlags;
	UInt32		fTimeout;

	hsBool ICheckOverlappedResult(BOOL result, UInt32 &numTransferred);
	hsBool IRead(UInt32 byteCount, void *buffer, UInt32 &numRead);
	hsBool IWrite(UInt32 byteCount, const void *buffer, UInt32 &numWritten);

public:
	enum { kThrowOnError = 1 };	// Throws if a read or write operation fails

	hsNamedPipeStream(UInt8 flags=0, UInt32 timeout=INFINITE);
	virtual ~hsNamedPipeStream();

	// The server (writer) and client (reader) need to open the same file.
	// The format is "\\.\pipe\pipeName".  The '.' can be replaced with a
	// computer name to do it over the network.  'pipeName' is whatever you
	// want.
	virtual hsBool	Open(const char *name, const char *mode);
	virtual hsBool	Open(const wchar *name, const wchar *mode);
	virtual hsBool	Close();
	
	virtual UInt32	Read(UInt32 byteCount, void *buffer);
	virtual UInt32	Write(UInt32 byteCount, const void *buffer);
	virtual void	Skip(UInt32 deltaByteCount);
	virtual void	Rewind();

	// - For the server (writer) only -
	// After calling open, signal your client to start reading and call this function.
	// If a client connects, this will return true and you can start writing.  If it
	// returns false, close the pipe, it ain't happening.
	hsBool WaitForClientConnect();
};

#endif // HS_BUILD_FOR_WIN32
