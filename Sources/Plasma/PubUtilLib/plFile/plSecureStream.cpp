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
#include "plSecureStream.h"
#include "hsUtils.h"
#include "plFileUtils.h"
#include "hsSTLStream.h"

#include <time.h>

// our default encryption key
const UInt32 plSecureStream::kDefaultKey[4] = { 0x6c0a5452, 0x3827d0f, 0x3a170b92, 0x16db7fc2 };

static const int kEncryptChunkSize = 8;

static const char* kMagicString    = "notthedroids";
static const int kMagicStringLen = 12;

static const int kFileStartOffset = kMagicStringLen + sizeof(UInt32);

static const int kMaxBufferedFileSize = 10*1024;

plSecureStream::plSecureStream(hsBool deleteOnExit, UInt32* key) :
fRef(INVALID_HANDLE_VALUE),
fActualFileSize(0),
fBufferedStream(false),
fRAMStream(nil),
fWriteFileName(nil),
fOpenMode(kOpenFail),
fDeleteOnExit(deleteOnExit)
{
	if (key)
		memcpy(&fKey, key, sizeof(kDefaultKey));
	else
		memcpy(&fKey, &kDefaultKey, sizeof(kDefaultKey));
}

plSecureStream::~plSecureStream()
{
}

//
// XXTEA
// http://www-users.cs.york.ac.uk/~matthew/TEA/
//
// A potential weakness in this implementation is the fact that a known value
// (length of the original file) is written at the start of the encrypted file.  -Colin
//

#define MX (z>>5 ^ y<<2) + (y>>3 ^ z<<4) ^ (sum^y) + (fKey[p&3^e]^z)

void plSecureStream::IEncipher(UInt32* const v, UInt32 n)
{
	register unsigned long y=v[0], z=v[n-1], e, delta=0x9E3779B9;
	register unsigned long q = 6 + 52/n, p, sum = 0;

	while (q-- > 0)
	{
		sum += delta;
		e = (sum >> 2) & 3;
		for (p = 0; p < n - 1; p++)
		{
			y = v[p + 1];
			v[p] += MX;
			z = v[p];
		}
		y = v[0];
		v[n - 1] += MX;
		z = v[n - 1];
	}
}

void plSecureStream::IDecipher(UInt32* const v, UInt32 n)
{
	register unsigned long y=v[0], z=v[n-1], e, delta=0x9E3779B9;
	register unsigned long q = 6 + 52/n, p, sum = q * delta;

	while (sum > 0)
	{
		e = (sum >> 2) & 3;
		for (p = n - 1; p > 0; p--)
		{
			z = v[p - 1];
			v[p] -= MX;
			y = v[p];
		}
		z = v[n - 1];
		v[0] -= MX;
		y = v[0];
		sum -= delta;
	}
}

hsBool plSecureStream::Open(const char* name, const char* mode)
{
	wchar* wName = hsStringToWString(name);
	wchar* wMode = hsStringToWString(mode);
	hsBool ret = Open(wName, wMode);
	delete [] wName;
	delete [] wMode;
	return ret;
}

hsBool plSecureStream::Open(const wchar* name, const wchar* mode)
{
	if (wcscmp(mode, L"rb") == 0)
	{
		if (fDeleteOnExit)
		{
			fRef = CreateFileW(name,
								GENERIC_READ,	// open for reading
								0,				// no one can open the file until we're done
								NULL,			// default security
								OPEN_EXISTING,	// only open existing files (no creation)
								FILE_FLAG_DELETE_ON_CLOSE,	// delete the file from disk when we close the handle
								NULL);			// no template
		}
		else
		{
			fRef = CreateFileW(name,
								GENERIC_READ,	// open for reading
								0,				// no one can open the file until we're done
								NULL,			// default security
								OPEN_EXISTING,	// only open existing files (no creation)
								FILE_ATTRIBUTE_NORMAL,	// normal file attributes
								NULL);			// no template
		}

		fPosition = 0;

		if (fRef == INVALID_HANDLE_VALUE)
			return false;

		// Make sure our special magic string is there
		if (!ICheckMagicString(fRef))
		{
			CloseHandle(fRef);
			fRef = INVALID_HANDLE_VALUE;
			return false;
		}

		DWORD numBytesRead;
		ReadFile(fRef, &fActualFileSize, sizeof(UInt32), &numBytesRead, NULL);

		// The encrypted stream is inefficient if you do reads smaller than
		// 8 bytes.  Since we do a lot of those, any file under a size threshold
		// is buffered in memory
		if (fActualFileSize <= kMaxBufferedFileSize)
			IBufferFile();

		fOpenMode = kOpenRead;

		return true;
	}
	else if (wcscmp(mode, L"wb") == 0)
	{
		fRAMStream = TRACKED_NEW hsVectorStream;
		fWriteFileName = TRACKED_NEW wchar[wcslen(name) + 1];
		wcscpy(fWriteFileName, name);
		fPosition = 0;

		fOpenMode = kOpenWrite;
		fBufferedStream = true;

		return true;
	}
	else
	{
		hsAssert(0, "Unsupported open mode");
		fOpenMode = kOpenFail;
		return false;
	}
}

hsBool plSecureStream::Close()
{
	int rtn = false;

	if (fOpenMode == kOpenWrite)
	{
		fRAMStream->Rewind();
		rtn = IWriteEncrypted(fRAMStream, fWriteFileName);
	}
	if (fRef != INVALID_HANDLE_VALUE)
	{
		rtn = CloseHandle(fRef);
		fRef = INVALID_HANDLE_VALUE;
	}

	if (fRAMStream)
	{
		delete fRAMStream;
		fRAMStream = nil;
	}

	if (fWriteFileName)
	{
		delete [] fWriteFileName;
		fWriteFileName = nil;
	}

	fActualFileSize = 0;
	fBufferedStream = false;
	fOpenMode = kOpenFail;

	return rtn;
}

UInt32 plSecureStream::IRead(UInt32 bytes, void* buffer)
{
	if (fRef == INVALID_HANDLE_VALUE)
		return 0;
	DWORD numItems;
	bool success = (ReadFile(fRef, buffer, bytes, &numItems, NULL) != 0);
	fBytesRead += numItems;
	fPosition += numItems;
	if ((unsigned)numItems < bytes)
	{
		if (success)
		{
			// EOF ocurred
			char str[128];
			sprintf(str, "Hit EOF on Windows read, only read %d out of requested %d bytes\n", numItems, bytes);
			hsDebugMessage(str, 0);
		}
		else
		{
			hsDebugMessage("Error on Windows read", GetLastError());
		}
	}
	return numItems;
}

void plSecureStream::IBufferFile()
{
	fRAMStream = TRACKED_NEW hsVectorStream;
	char buf[1024];
	while (!AtEnd())
	{
		UInt32 numRead = Read(1024, buf);
		fRAMStream->Write(numRead, buf);
	}
	fRAMStream->Rewind();

	fBufferedStream = true;
	CloseHandle(fRef);
	fRef = INVALID_HANDLE_VALUE;
	fPosition = 0;
}

hsBool plSecureStream::AtEnd()
{
	if (fBufferedStream)
		return fRAMStream->AtEnd();
	else
		return (GetPosition() == fActualFileSize);
}

void plSecureStream::Skip(UInt32 delta)
{
	if (fBufferedStream)
	{
		fRAMStream->Skip(delta);
		fPosition = fRAMStream->GetPosition();
	}
	else if (fRef != INVALID_HANDLE_VALUE)
	{
		fBytesRead += delta;
		fPosition += delta;
		SetFilePointer(fRef, delta, 0, FILE_CURRENT);
	}
}

void plSecureStream::Rewind()
{
	if (fBufferedStream)
	{
		fRAMStream->Rewind();
		fPosition = fRAMStream->GetPosition();
	}
	else if (fRef != INVALID_HANDLE_VALUE)
	{
		fBytesRead = 0;
		fPosition = 0;
		SetFilePointer(fRef, kFileStartOffset, 0, FILE_BEGIN);
	}
}

void plSecureStream::FastFwd()
{
	if (fBufferedStream)
	{
		fRAMStream->FastFwd();
		fPosition = fRAMStream->GetPosition();
	}
	else if (fRef != INVALID_HANDLE_VALUE)
	{
		fBytesRead = fPosition = SetFilePointer(fRef, kFileStartOffset + fActualFileSize, 0, FILE_BEGIN);
	}
}

UInt32 plSecureStream::GetEOF()
{
	return fActualFileSize;
}

UInt32 plSecureStream::Read(UInt32 bytes, void* buffer)
{
	if (fBufferedStream)
	{
		UInt32 numRead = fRAMStream->Read(bytes, buffer);
		fPosition = fRAMStream->GetPosition();
		return numRead;
	}

	UInt32 startPos = fPosition;

	// Offset into the first buffer (0 if we are aligned on a chunk, which means no extra block read)
	UInt32 startChunkPos = startPos % kEncryptChunkSize;
	// Amount of data in the partial first chunk (0 if we're aligned)
	UInt32 startAmt = (startChunkPos != 0) ? hsMinimum(kEncryptChunkSize - startChunkPos, bytes) : 0;

	UInt32 totalNumRead = IRead(bytes, buffer);

	UInt32 numMidChunks = (totalNumRead - startAmt) / kEncryptChunkSize;
	UInt32 endAmt = (totalNumRead - startAmt) % kEncryptChunkSize;

	// If the start position is in the middle of a chunk we need to rewind and
	// read that whole chunk in and decrypt it.
	if (startChunkPos != 0)
	{
		// Move to the start of this chunk
		SetPosition(startPos-startChunkPos);

		// Read in the chunk and decrypt it
		char buf[kEncryptChunkSize];
		UInt32 numRead = IRead(kEncryptChunkSize, &buf);
		IDecipher((UInt32*)&buf, kEncryptChunkSize / sizeof(UInt32));

		// Copy the relevant portion to the output buffer
		memcpy(buffer, &buf[startChunkPos], startAmt);

		SetPosition(startPos+totalNumRead);
	}

	if (numMidChunks != 0)
	{
		UInt32* bufferPos = (UInt32*)(((char*)buffer)+startAmt);
		for (int i = 0; i < numMidChunks; i++)
		{
			// Decrypt chunk
			IDecipher(bufferPos, kEncryptChunkSize / sizeof(UInt32));
			bufferPos += (kEncryptChunkSize / sizeof(UInt32));
		}
	}

	if (endAmt != 0)
	{
		// Read in the final chunk and decrypt it
		char buf[kEncryptChunkSize];
		SetPosition(startPos + startAmt + numMidChunks*kEncryptChunkSize);
		UInt32 numRead = IRead(kEncryptChunkSize, &buf);
		IDecipher((UInt32*)&buf, kEncryptChunkSize / sizeof(UInt32));

		memcpy(((char*)buffer)+totalNumRead-endAmt, &buf, endAmt);

		SetPosition(startPos+totalNumRead);
	}

	// If we read into the padding at the end, update the total read to not include that
	if (totalNumRead > 0 && startPos + totalNumRead > fActualFileSize)
	{
		totalNumRead -= (startPos + totalNumRead) - fActualFileSize;
		SetPosition(fActualFileSize);
	}

	return totalNumRead;
}

UInt32 plSecureStream::Write(UInt32 bytes, const void* buffer)
{
	if (fOpenMode != kOpenWrite)
	{
		hsAssert(0, "Trying to write to a read stream");
		return 0;
	}

	return fRAMStream->Write(bytes, buffer);
}

bool plSecureStream::IWriteEncrypted(hsStream* sourceStream, const wchar* outputFile)
{
	hsUNIXStream outputStream;

	if (!outputStream.Open(outputFile, L"wb"))
		return false;

	outputStream.Write(kMagicStringLen, kMagicString);

	// Save some space to write the file size at the end
	outputStream.WriteSwap32(0);

	// Write out all the full size encrypted blocks we can
	char buf[kEncryptChunkSize];
	UInt32 amtRead;
	while ((amtRead = sourceStream->Read(kEncryptChunkSize, &buf)) == kEncryptChunkSize)
	{
		IEncipher((UInt32*)&buf, kEncryptChunkSize / sizeof(UInt32));
		outputStream.Write(kEncryptChunkSize, &buf);
	}

	// Pad with random data and write out the final partial block, if there is one
	if (amtRead > 0)
	{
		static bool seededRand = false;
		if (!seededRand)
		{
			seededRand = true;
			srand((unsigned int)time(nil));
		}

		for (int i = amtRead; i < kEncryptChunkSize; i++)
			buf[i] = rand();

		IEncipher((UInt32*)&buf, kEncryptChunkSize / sizeof(UInt32));

		outputStream.Write(kEncryptChunkSize, &buf);
	}

	// Write the original file size at the start
	UInt32 actualSize = sourceStream->GetPosition();
	outputStream.Rewind();
	outputStream.Skip(kMagicStringLen);
	outputStream.WriteSwap32(actualSize);

	outputStream.Close();

	return true;
}

bool plSecureStream::FileEncrypt(const char* fileName, UInt32* key /* = nil */)
{
	wchar* wFilename = hsStringToWString(fileName);
	bool ret = FileEncrypt(wFilename, key);
	delete [] wFilename;
	return ret;
}

bool plSecureStream::FileEncrypt(const wchar* fileName, UInt32* key /* = nil */)
{
	hsUNIXStream sIn;
	if (!sIn.Open(fileName))
		return false;

	// Don't double encrypt any files
	if (ICheckMagicString(sIn.GetFILE()))
	{
		sIn.Close();
		return true;
	}
	sIn.Rewind();

	plSecureStream sOut(false, key);
	bool wroteEncrypted = sOut.IWriteEncrypted(&sIn, L"crypt.dat");

	sIn.Close();
	sOut.Close();

	if (wroteEncrypted)
	{
		plFileUtils::RemoveFile(fileName);
		plFileUtils::FileMove(L"crypt.dat", fileName);
	}

	return true;
}

bool plSecureStream::FileDecrypt(const char* fileName, UInt32* key /* = nil */)
{
	wchar* wFilename = hsStringToWString(fileName);
	bool ret = FileDecrypt(wFilename, key);
	delete [] wFilename;
	return ret;
}

bool plSecureStream::FileDecrypt(const wchar* fileName, UInt32* key /* = nil */)
{
	plSecureStream sIn(false, key);
	if (!sIn.Open(fileName))
		return false;

	hsUNIXStream sOut;
	if (!sOut.Open(L"crypt.dat", L"wb"))
	{
		sIn.Close();
		return false;
	}

	char buf[1024];

	while (!sIn.AtEnd())
	{
		UInt32 numRead = sIn.Read(sizeof(buf), buf);
		sOut.Write(numRead, buf);
	}

	sIn.Close();
	sOut.Close();

	plFileUtils::RemoveFile(fileName);
	plFileUtils::FileMove(L"crypt.dat", fileName);

	return true;
}

bool plSecureStream::ICheckMagicString(HANDLE fp)
{
	char magicString[kMagicStringLen+1];
	DWORD numBytesRead;
	ReadFile(fp, &magicString, kMagicStringLen, &numBytesRead, NULL);
	magicString[kMagicStringLen] = '\0';
	return (hsStrEQ(magicString, kMagicString) != 0);
}

bool plSecureStream::IsSecureFile(const char* fileName)
{
	wchar* wFilename = hsStringToWString(fileName);
	bool ret = IsSecureFile(wFilename);
	delete [] wFilename;
	return ret;
}

bool plSecureStream::IsSecureFile(const wchar* fileName)
{
	HANDLE fp = INVALID_HANDLE_VALUE;
	fp = CreateFileW(fileName,
		GENERIC_READ,	// open for reading
		0,				// no one can open the file until we're done
		NULL,			// default security
		OPEN_EXISTING,	// only open existing files (no creation)
		FILE_ATTRIBUTE_NORMAL,	// normal file attributes
		NULL);			// no template

	if (fp == INVALID_HANDLE_VALUE)
		return false;

	bool isEncrypted = ICheckMagicString(fp);

	CloseHandle(fp);

	return isEncrypted;
}

hsStream* plSecureStream::OpenSecureFile(const char* fileName, const UInt32 flags /* = kRequireEncryption */, UInt32* key /* = nil */)
{
	wchar* wFilename = hsStringToWString(fileName);
	hsStream* ret = OpenSecureFile(wFilename, flags, key);
	delete [] wFilename;
	return ret;
}

hsStream* plSecureStream::OpenSecureFile(const wchar* fileName, const UInt32 flags /* = kRequireEncryption */, UInt32* key /* = nil */)
{
	bool requireEncryption = flags & kRequireEncryption;
#ifndef PLASMA_EXTERNAL_RELEASE
	requireEncryption = false;
#endif

	hsBool deleteOnExit = flags & kDeleteOnExit;
	bool isEncrypted = IsSecureFile(fileName);

	hsStream* s = nil;
	if (isEncrypted)
		s = TRACKED_NEW plSecureStream(deleteOnExit, key);
	else if (!requireEncryption) // If this isn't an external release, let them use unencrypted data
		s = TRACKED_NEW hsUNIXStream;

	if (s)
		s->Open(fileName, L"rb");
	return s;
}

hsStream* plSecureStream::OpenSecureFileWrite(const char* fileName, UInt32* key /* = nil */)
{
	wchar* wFilename = hsStringToWString(fileName);
	hsStream* ret = OpenSecureFileWrite(wFilename, key);
	delete [] wFilename;
	return ret;
}

hsStream* plSecureStream::OpenSecureFileWrite(const wchar* fileName, UInt32* key /* = nil */)
{
	hsStream* s = nil;
#ifdef PLASMA_EXTERNAL_RELEASE
	s = TRACKED_NEW plSecureStream(false, key);
#else
	s = TRACKED_NEW hsUNIXStream;
#endif

	s->Open(fileName, L"wb");
	return s;
}