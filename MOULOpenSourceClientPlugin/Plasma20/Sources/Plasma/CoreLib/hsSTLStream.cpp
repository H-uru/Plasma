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
#include "hsSTLStream.h"

hsVectorStream::hsVectorStream() : fEnd(0)
{
}

hsVectorStream::hsVectorStream(UInt32 chunkSize)
{	
	fVector.reserve(chunkSize);
}

hsVectorStream::~hsVectorStream()
{
}

hsBool hsVectorStream::AtEnd()
{
	return (fBytesRead >= fEnd);
}

UInt32 hsVectorStream::Read(UInt32 byteCount, void *buffer)
{
	if (fBytesRead + byteCount > fEnd)
	{
//		hsStatusMessageF("Reading past end of hsVectorStream (read %u of %u requested bytes)", fEnd-fBytesRead, byteCount);
		byteCount = fEnd - fBytesRead;
	}
	
	memcpy(buffer, &fVector[fBytesRead], byteCount);

	fBytesRead += byteCount;
    fPosition += byteCount;

	return byteCount;
}

UInt32 hsVectorStream::Write(UInt32 byteCount, const void* buffer)
{
	// If we are at the end of the vector, we can just do a block insert of the data
	if (fPosition == fVector.size())
		fVector.insert(fVector.end(), (Byte*)buffer, (Byte*)buffer+byteCount);
	// If we are in the middle, I don't know how to just overwrite a block of the vector.
	// So, we make sure there is enough space and copy the elements one by one
	else
	{
		fVector.reserve(fPosition+byteCount);
		for (UInt32 i = 0; i < byteCount; i++)
			fVector[fPosition+i] = ((Byte*)buffer)[i];
	}

    fPosition += byteCount;

	if (fPosition > fEnd)
		fEnd = fPosition;

	return byteCount;
}

void hsVectorStream::Skip(UInt32 deltaByteCount)
{
	fBytesRead += deltaByteCount;
    fPosition += deltaByteCount;
}

void hsVectorStream::Rewind()
{
	fBytesRead = 0;
	fPosition = 0;
}

void hsVectorStream::FastFwd()
{
	fBytesRead = fPosition = fEnd;
}

void hsVectorStream::Truncate()
{
	fVector.erase(fVector.begin()+fPosition, fVector.end());
	fEnd = fPosition-1;
}

UInt32 hsVectorStream::GetEOF()
{
	return fEnd;
}

void hsVectorStream::CopyToMem(void* mem)
{
	memcpy(mem, &fVector[0], fEnd);
}

void hsVectorStream::Erase(UInt32 bytes)
{
	hsAssert(fPosition+bytes <= fEnd, "Erasing past end of stream");

	fVector.erase(fVector.begin()+fPosition, fVector.begin()+fPosition+bytes);
	fEnd -= bytes;
}

void hsVectorStream::Reset()
{
	fBytesRead = 0;
    fPosition = 0;
	fEnd = 0;
	fVector.clear();
}

const void *hsVectorStream::GetData()
{
	if (fVector.size() > 0)
		return &fVector[0];
	else
		return nil;
}

/////////////////////////////////////////////////////////////////////////////////////////

#ifdef HS_BUILD_FOR_WIN32

hsNamedPipeStream::hsNamedPipeStream(UInt8 flags, UInt32 timeout) :
	fFlags(flags),
	fPipe(INVALID_HANDLE_VALUE),
	fReadMode(false),
	fTimeout(timeout)
{
	memset(&fOverlap, 0, sizeof(OVERLAPPED));
	fOverlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

hsNamedPipeStream::~hsNamedPipeStream()
{
	CloseHandle(fOverlap.hEvent);
	fOverlap.hEvent = INVALID_HANDLE_VALUE;
}

hsBool hsNamedPipeStream::WaitForClientConnect()
{
	// Look for a client connect (this should return zero since it's overlapped)
	BOOL ret = ConnectNamedPipe(fPipe, &fOverlap);
	if (ret)
		return true;
	else
	{
		switch (GetLastError())
		{
		// Waiting for client to connect
		case ERROR_IO_PENDING:
			if (WaitForSingleObject(fOverlap.hEvent, fTimeout) == WAIT_OBJECT_0)
				return true;
			break;

		// Client is already connected
		case ERROR_PIPE_CONNECTED:
//			if (SetEvent(fOverlap.hEvent))
				return true;
			break;
		}
	}

	return false;
}

hsBool hsNamedPipeStream::Open(const char *name, const char *mode)
{
	wchar* wName = hsStringToWString(name);
	wchar* wMode = hsStringToWString(mode);
	hsBool ret = Open(wName, wMode);
	delete [] wName;
	delete [] wMode;
	return ret;
}

hsBool hsNamedPipeStream::Open(const wchar *name, const wchar *mode)
{
	if (wcschr(mode, L'w'))
	{
		fReadMode = false;

		// Try to create the pipe
		fPipe = CreateNamedPipeW(name,
								PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
								PIPE_TYPE_BYTE,
								1,
								1024,
								1024,
								fTimeout,
								NULL);

		if (fPipe != INVALID_HANDLE_VALUE)
			return true;
	}
	else if (wcschr(mode, L'r'))
	{
		fReadMode = true;

		fPipe = CreateFileW(name,
							GENERIC_READ, 
							0,              // no sharing 
							NULL,           // no security attributes
							OPEN_EXISTING,  // opens existing pipe 
							FILE_FLAG_OVERLAPPED,              // default attributes 
							NULL);          // no template file

		if (fPipe != INVALID_HANDLE_VALUE)
			return true;
	}

	return false;
}

hsBool hsNamedPipeStream::Close()
{
	if (fPipe == INVALID_HANDLE_VALUE)
		return false;

	if (fReadMode)
	{
		CloseHandle(fPipe);			// Close our end of the pipe
		fPipe = INVALID_HANDLE_VALUE;
	}
	else
	{
		FlushFileBuffers(fPipe);	// Make sure the client is done reading
		DisconnectNamedPipe(fPipe);	// Disconnect the pipe from the client
		CloseHandle(fPipe);			// Close our end of the pipe
		fPipe = INVALID_HANDLE_VALUE;
	}

	return true;
}

hsBool hsNamedPipeStream::ICheckOverlappedResult(BOOL result, UInt32 &numTransferred)
{
	// Read/Write succeeded, return now
	if (result)
		return true;
	// Read failed because the operation is taking a while.  Wait for it
	else if (GetLastError() == ERROR_IO_PENDING)
	{
		if (WaitForSingleObject(fOverlap.hEvent, fTimeout) == WAIT_OBJECT_0)
		{
			BOOL oResult = GetOverlappedResult(fPipe, &fOverlap, &numTransferred, FALSE);
			if (oResult)
				return true;
			hsAssert(oResult, "GetOverlappedResult failed");
		}
		else
			hsAssert(0, "Wait failed");
	}
	else
		hsAssert(0, "Read/Write failed");

	return false;
}

hsBool hsNamedPipeStream::IRead(UInt32 byteCount, void *buffer, UInt32 &numRead)
{
	numRead = 0;

	if (fPipe != INVALID_HANDLE_VALUE && fReadMode)
	{
		BOOL result = ReadFile(fPipe, buffer, byteCount, &numRead, &fOverlap);
		if (ICheckOverlappedResult(result, numRead))
			return true;
	}

	// If we got here, the pipe is probably broken.  Throw if it is enabled.
	if (fFlags & kThrowOnError)
		throw this;

	return false;
}

hsBool hsNamedPipeStream::IWrite(UInt32 byteCount, const void *buffer, UInt32 &numWritten)
{
	numWritten = 0;

	if (fPipe != INVALID_HANDLE_VALUE && !fReadMode)
	{
		BOOL result = WriteFile(fPipe, buffer, byteCount, &numWritten, &fOverlap);
		if (ICheckOverlappedResult(result, numWritten))
			return true;
	}

	// If we got here, the pipe is probably broken.  Throw if it is enabled.
	if (fFlags & kThrowOnError)
		throw this;

	return false;
}

UInt32 hsNamedPipeStream::Read(UInt32 byteCount, void *buffer)
{
	UInt32 totalRead = 0;

	// Read until we get all our data or an error
	UInt32 numRead = 0;
	while (IRead(byteCount-totalRead, (void*)((UInt32)buffer+totalRead), numRead))
	{
		totalRead += numRead;

		if (totalRead >= byteCount)
			return totalRead;
	}

	return totalRead;
}

UInt32 hsNamedPipeStream::Write(UInt32 byteCount, const void *buffer)
{
	UInt32 totalWritten = 0;

	// Write until we get all our data or an error
	UInt32 numWritten = 0;
	while (IWrite(byteCount-totalWritten, (const void*)((UInt32)buffer+totalWritten), numWritten))
	{
		totalWritten += numWritten;

		if (totalWritten >= byteCount)
			return totalWritten;
	}

	return totalWritten;
}

#ifdef __SGI_STL_PORT
using std::min;
#endif

void hsNamedPipeStream::Skip(UInt32 deltaByteCount)
{
	char buf[256];

	// Read until we get all our data or an error
	UInt32 totalRead = 0;
	UInt32 numRead = 0;
	while (IRead(min((UInt32)256L, deltaByteCount-totalRead), buf, numRead))
	{
		totalRead += numRead;

		if (totalRead >= deltaByteCount)
			return;
	}

}

void hsNamedPipeStream::Rewind()
{
	hsAssert(0, "Rewind not allowed on a pipe");
}

#endif // HS_BUILD_FOR_WIN32
