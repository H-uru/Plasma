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
	UInt32 fKey[4];

	UInt32 fActualFileSize;

	bool fBufferedStream;

	hsStream* fRAMStream;

	wchar* fWriteFileName;

	enum OpenMode { kOpenRead, kOpenWrite, kOpenFail };
	OpenMode fOpenMode;

	void IBufferFile();

	UInt32 IRead(UInt32 bytes, void* buffer);

	void IEncipher(UInt32* const v);
	void IDecipher(UInt32* const v);

	bool IWriteEncypted(hsStream* sourceStream, const wchar* outputFile);

	static bool ICheckMagicString(FILE* fp);

public:
	// If you don't pass in a key (4 UInt32's), the default one will be used
	plEncryptedStream(UInt32* key=nil);
	~plEncryptedStream();

	virtual hsBool	Open(const char* name, const char* mode = "rb");
	virtual hsBool	Open(const wchar* name, const wchar* mode = L"rb");
	virtual hsBool	Close();

	virtual UInt32	Read(UInt32 byteCount, void* buffer);
	virtual UInt32	Write(UInt32 byteCount, const void* buffer);
	virtual hsBool	AtEnd();
	virtual void	Skip(UInt32 deltaByteCount);
	virtual void	Rewind();
	virtual void	FastFwd();
	virtual UInt32	GetEOF();

	UInt32 GetActualFileSize() const { return fActualFileSize;}

	static bool FileEncrypt(const char* fileName);
	static bool FileEncrypt(const wchar* fileName);
	static bool FileDecrypt(const char* fileName);
	static bool FileDecrypt(const wchar* fileName);

	static bool IsEncryptedFile(const char* fileName);
	static bool IsEncryptedFile(const wchar* fileName);

	// Attempts to create a read-binary stream for the requested file.  If it's
	// encrypted, you'll get a plEncryptedStream, otherwise just a standard
	// hsUNIXStream.  Remember to delete the stream when you're done with it.
	static hsStream* OpenEncryptedFile(const char* fileName, bool requireEncrypted = true, UInt32* cryptKey = nil);
	static hsStream* OpenEncryptedFile(const wchar* fileName, bool requireEncrypted = true, UInt32* cryptKey = nil);
	static hsStream* OpenEncryptedFileWrite(const char* fileName, UInt32* cryptKey = nil);
	static hsStream* OpenEncryptedFileWrite(const wchar* fileName, UInt32* cryptKey = nil);
};

#endif // plEncryptedStream_h_inc
