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
#include <ctype.h>
#include "hsStream.h"
#include "hsMemory.h"
#include "hsUtils.h"

#include "hsTemplates.h"
#include "hsStlUtils.h"

#if HS_BUILD_FOR_UNIX
#include <unistd.h>
#endif

#if HS_BUILD_FOR_MAC
	#include <Files.h>
	#include <stdio.h>
	#include <unistd.h>
#endif

#if HS_BUILD_FOR_PS2
#include <eekernel.h>
#include <sifdev.h>
#endif

#include "hsWindows.h"
#if HS_BUILD_FOR_WIN32
#include <io.h>
#endif
#include "hsStlUtils.h"

//////////////////////////////////////////////////////////////////////////////////

#if HS_CPU_BENDIAN
	static void swapIt(Int32 *swap)
	{
		Byte*	c = (Byte*)swap;
		Byte		t = c[0];

		c[0] = c[3];
		c[3] = t;
		t = c[1];
		c[1] = c[2];
		c[2] = t;
	}
	static void swapIt(int *swap)
	{
		swapIt((Int32*)swap);
	}
	static void swapIt(float *swap)
	{
		swapIt((Int32*)swap);
	}

	static void swapIt(double *swap)
	{
		float* a = (float*)&swap;
		float* b = (float*)(((Byte*)&swap)+4);
		swapIt(a);
		swapIt(b);
	}

	static void swapIt(Int16 *swap)
	{
		Byte *c = (Byte*)swap;
		Byte t;
		t = c[0];
		c[0] = c[1];
		c[1] = t;
	}
	#define unswapIt(value)
#else
	#define swapIt(value)
	static void unswapIt(Int32 *swap)
	{
		Byte*	c = (Byte*)swap;
		Byte		t = c[0];

		c[0] = c[3];
		c[3] = t;
		t = c[1];
		c[1] = c[2];
		c[2] = t;
	}
	static void unswapIt(int *swap)
	{
		unswapIt((Int32*)swap);
	}
	static void unswapIt(float *swap)
	{
		unswapIt((Int32*)swap);
	}

	static void unswapIt(double *swap)
	{
		float* a = (float*)&swap;
		float* b = (float*)(((Byte*)&swap)+4);
		swapIt(a);
		swapIt(b);
	}

	static void unswapIt(Int16 *swap)
	{
		Byte *c = (Byte*)swap;
		Byte t;
		t = c[0];
		c[0] = c[1];
		c[1] = t;
	}
#endif

//////////////////////////////////////////////////////////////////////////////////

void hsStream::FastFwd()
{
	hsThrow("FastFwd unimplemented by subclass of stream");
}

UInt32 hsStream::GetPosition() const
{
	return fPosition;
}

void hsStream::SetPosition(UInt32 position)
{
	if (position == fPosition)
		return;
    Rewind();
    Skip(position);
}

void hsStream::Truncate()
{
	hsThrow("Truncate unimplemented by subclass of stream");
}

UInt32 hsStream::GetSizeLeft()
{
	UInt32 ret = 0;
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

UInt32 hsStream::GetEOF()
{
	hsThrow( "GetEOF() unimplemented by subclass of stream");
	return 0;
}

void hsStream::CopyToMem(void* mem)
{
	hsThrow( "CopyToMem unimplemented by subclass of stream");
}

//////////////////////////////////////////////////////////////////////////////////

hsStream::~hsStream()
{
}

UInt32 hsStream::WriteString(const char cstring[])
{
	return Write(hsStrlen(cstring), cstring);
}

UInt32 hsStream::WriteFmt(const char * fmt, ...)
{
	va_list av;
	va_start( av, fmt );
	UInt32 n = WriteFmtV( fmt, av );
	va_end( av );
	return n;
}

UInt32 hsStream::WriteFmtV(const char * fmt, va_list av)
{
	std::string buf;
	xtl::formatv( buf, fmt, av );
	return Write( buf.length(), buf.data() );
}

UInt32 hsStream::WriteSafeStringLong(const char *string)
{
	UInt32 len = hsStrlen(string);		
	WriteSwap32(len);
	if (len > 0)
	{	
		char *buff = TRACKED_NEW char[len+1];
		int i;
		for (i = 0; i < len; i++)
		{
			buff[i] = ~string[i];
		}
		buff[len] = '\0';
		UInt32 result = Write(len, buff);
		delete [] buff;
		return result;
	}
	else
		return 0;
}

UInt32 hsStream::WriteSafeWStringLong(const wchar_t *string)
{
	UInt32 len = wcslen(string);
	WriteSwap32(len);
	if (len > 0)
	{
		int i;
		for (i=0; i<len; i++)
		{
			wchar_t buff = ~string[i];
			WriteSwap16((UInt16)buff);
		}
		WriteSwap16((UInt16)L'\0');
	}
	return 0;
}

char *hsStream::ReadSafeStringLong()
{
	char *name = nil;
	UInt32 numChars = ReadSwap32();
	if (numChars > 0 && numChars <= GetSizeLeft())
	{
		name = TRACKED_NEW char[numChars+1];
		Read(numChars, name);
		name[numChars] = '\0';

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

wchar_t *hsStream::ReadSafeWStringLong()
{
	wchar_t *retVal = nil;
	UInt32 numChars = ReadSwap32();
	if (numChars > 0 && numChars <= (GetSizeLeft()/2)) // divide by two because each char is two bytes
	{
		retVal = TRACKED_NEW wchar_t[numChars+1];
		int i;
		for (i=0; i<numChars; i++)
			retVal[i] = (wchar_t)ReadSwap16();
		retVal[numChars] = (wchar_t)ReadSwap16(); // we wrote the null out, read it back in

		if (retVal[0]* 0x80)
		{
			int i;
			for (i=0; i<numChars; i++)
				retVal[i] = ~retVal[i];
		}
	}

	return retVal;
}

UInt32 hsStream::WriteSafeString(const char *string)
{
	int len = hsStrlen(string);
	hsAssert(len<0xf000, xtl::format("string len of %d is too long for WriteSafeString %s, use WriteSafeStringLong", 
		string, len).c_str() );

	WriteSwap16(len | 0xf000);
	if (len > 0)
	{
		char *buff = TRACKED_NEW char[len+1];
		int i;
		for (i = 0; i < len; i++)
		{
			buff[i] = ~string[i];
		}
		buff[len] = '\0';
		UInt32 result = Write(len, buff);
		delete [] buff;
		return result;
	}
	else
		return 0;
}

UInt32 hsStream::WriteSafeWString(const wchar_t *string)
{
	int len = wcslen(string);
	hsAssert(len<0xf000, xtl::format("string len of %d is too long for WriteSafeWString, use WriteSafeWStringLong",
		len).c_str() );

	WriteSwap16(len | 0xf000);
	if (len > 0)
	{
		int i;
		for (i=0; i<len; i++)
		{
			wchar_t buff = ~string[i];
			WriteSwap16((UInt16)buff);
		}
		WriteSwap16((UInt16)L'\0');
	}
	return 0;
}

char *hsStream::ReadSafeString()
{
	char *name = nil;
	UInt16 numChars = ReadSwap16();

#ifndef REMOVE_ME_SOON
	// Backward compat hack - remove in a week or so (from 6/30/03)
	hsBool oldFormat = !(numChars & 0xf000);
	if (oldFormat)
		ReadSwap16();
#endif

	numChars &= ~0xf000;
	hsAssert(numChars <= GetSizeLeft(), "Bad string");
	if (numChars > 0 && numChars <= GetSizeLeft())
	{
		name = TRACKED_NEW char[numChars+1];
		Read(numChars, name);
		name[numChars] = '\0';	

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

wchar_t *hsStream::ReadSafeWString()
{
	wchar_t *retVal = nil;
	UInt32 numChars = ReadSwap16();
	
	numChars &= ~0xf000;
	hsAssert(numChars <= GetSizeLeft()/2, "Bad string");
	if (numChars > 0 && numChars <= (GetSizeLeft()/2)) // divide by two because each char is two bytes
	{
		retVal = TRACKED_NEW wchar_t[numChars+1];
		int i;
		for (i=0; i<numChars; i++)
			retVal[i] = (wchar_t)ReadSwap16();
		retVal[numChars] = (wchar_t)ReadSwap16(); // we wrote the null out, read it back in

		if (retVal[0]* 0x80)
		{
			int i;
			for (i=0; i<numChars; i++)
				retVal[i] = ~retVal[i];
		}
	}

	return retVal;
}

hsBool	hsStream::Read4Bytes(void *pv)	// Virtual, faster version in sub classes
{
	int knt = this->Read(sizeof(UInt32), pv);
	if (knt != 4)
		return false;
	return true;
}

hsBool  hsStream::Read12Bytes(void *buffer)	// Reads 12 bytes, return true if success
{
	int knt = this->Read(12,buffer);
	if (knt != 12)
		return false;
	return true;
}

hsBool  hsStream::Read8Bytes(void *buffer)	// Reads 12 bytes, return true if success
{
	int knt = this->Read(8,buffer);
	if (knt !=8)
		return false;
	return true;
}

hsBool hsStream::ReadBool() // Virtual, faster version in sub classes
{
	return this->ReadByte();
}

bool hsStream::Readbool() // Virtual, faster version in sub classes
{
	return this->ReadByte() ? true : false;
}

void hsStream::ReadBool(int count, hsBool values[])
{
	this->Read(count, values);

	if (sizeof(hsBool) > 1)
	{	const UInt8* src = (UInt8*)values;

		//	go backwards so we don't overwrite ourselves
		for (int i = count - 1; i >= 0; --i)
			values[i] = src[i];
	}
}

UInt8 hsStream::ReadByte()
{
	UInt8	value;

	this->Read(sizeof(UInt8), &value);
	return value;
}

hsBool hsStream::AtEnd()
{
	hsAssert(0,"No hsStream::AtEnd() implemented for this stream class");
	return false;
}

hsBool hsStream::IsTokenSeparator(char c)
{
	return (isspace(c) || c==',' || c=='=');
}

hsBool hsStream::GetToken(char *s, UInt32 maxLen, const char beginComment, const char endComment)
{
	char c;
	char endCom;
        endCom = endComment;

	while( true )
	{
		while( !AtEnd() && IsTokenSeparator(c = ReadByte()) )
			c = c;
			;
		if( AtEnd() )
			return false;

		if( beginComment != c )
			break;

		// skip to end of comment
		while( !AtEnd() && (endCom != (c = ReadByte())) )
			c= c;
			;
	}

	s[0] = c;
	UInt32 k = 1;
	while( !AtEnd() && !IsTokenSeparator(c = ReadByte()) )
	{
		if( k < maxLen )
			s[k++] = c;
	}
	s[k] = 0;


	if( (k > 0)&&!_stricmp(s, "skip") )
	{
		int depth = 1;
		while( depth && GetToken(s, maxLen, beginComment, endCom) )
		{
			if( !_stricmp(s, "skip") )
				depth++;
			else
			if( !_stricmp(s, "piks") )
				depth--;
		}
		return GetToken(s, maxLen, beginComment, endCom);
	}

	return true;
}

hsBool hsStream::ReadLn(char *s, UInt32 maxLen, const char beginComment, const char endComment)
{
	char c;
	char endCom;
        endCom = endComment;

	while( true )
	{
		while( !AtEnd() && strchr("\r\n",c = ReadByte()) )
			c = c;
			;
		if( AtEnd() )
			return false;

		if( beginComment != c )
			break;

		// skip to end of comment
		while( !AtEnd() && (endCom != (c = ReadByte())) )
			c= c;
			;
	}

	s[0] = c;
	UInt32 k = 1;
	while( !AtEnd() && !strchr("\r\n",c = ReadByte()) )
	{
		if( k < maxLen )
			s[k++] = c;
	}
	s[k] = 0;


	if( (k > 0)&&!_stricmp(s, "skip") )
	{
		int depth = 1;
		while( depth && ReadLn(s, maxLen, beginComment, endCom) )
		{
			if( !_stricmp(s, "skip") )
				depth++;
			else
			if( !_stricmp(s, "piks") )
				depth--;
		}
		return ReadLn(s, maxLen, beginComment, endCom);
	}

	return true;
}

UInt16 hsStream::ReadSwap16()
{
	UInt16	value;
	this->Read(sizeof(UInt16), &value);
	swapIt((Int16*)&value);
	return value;
}

void hsStream::ReadSwap16(int count, UInt16 values[])
{
	this->Read(count * sizeof(UInt16), values);
#if HS_CPU_BENDIAN
	for (int i = 0; i < count; i++)
		swapIt((Int16*)&values[i]);
#endif
}

UInt32 hsStream::ReadSwap32()
{
	UInt32	value;
	Read4Bytes(&value);
	swapIt((Int32*)&value);
	return value;
}

void hsStream::ReadSwap32(int count, UInt32 values[])
{
	this->Read(count * sizeof(UInt32), values);
#if HS_CPU_BENDIAN
	for (int i = 0; i < count; i++)
		swapIt((Int32*)&values[i]);
#endif
}

UInt32 hsStream::ReadUnswap32()
{
	UInt32	value;
	Read4Bytes(&value);
	unswapIt((Int32*)&value);
	return value;
}

#if HS_CAN_USE_FLOAT
	double hsStream::ReadSwapDouble()
	{
		double  ival;
		Read8Bytes(&ival);
		double *pval = (double *)&ival;		// all in the name of speed, 
		swapIt(pval);
		return *pval;
	}

	void hsStream::ReadSwapDouble(int count, double values[])
	{
		this->Read(count * sizeof(double), values);
#if HS_CPU_BENDIAN
                for (int i = 0; i < count; i++)
                        swapIt(&values[i]);
#endif
        }


	float hsStream::ReadSwapFloat()
	{
		UInt32  ival;
		Read4Bytes(&ival);
		float *pval = (float *)&ival;		// all in the name of speed, 
		swapIt(pval);
		return *pval;
	}

	void hsStream::ReadSwapFloat(int count, float values[])
	{
		this->Read(count * sizeof(float), values);
#if HS_CPU_BENDIAN
		for (int i = 0; i < count; i++)
			swapIt(&values[i]);
#endif
	}

	float hsStream::ReadUnswapFloat()
	{
		float value;
		this->Read(sizeof(float), &value);
		unswapIt(&value);
		return value;
	}
#endif


void hsStream::WriteBool(hsBool value)
{
	UInt8 dst = (value != 0);

	this->Write(sizeof(UInt8), &dst);
}

void hsStream::Writebool(bool value)
{
	UInt8 dst = (value != 0);

	this->Write(sizeof(UInt8), &dst);
}

void hsStream::WriteBool(int count, const hsBool values[])
{
	if (sizeof(hsBool) > 1)
	{	hsTempArray<UInt8> storage(count);
		UInt8*			 dst = (UInt8*)values;
	
		for (int i = 0; i < count; i++)
			dst[i] = (values[i] != 0);
		this->Write(count, dst);
	}
	else
		this->Write(count, values);
}

void hsStream::WriteByte(UInt8 value)
{
	this->Write(sizeof(UInt8), &value);
}

void  hsStream::WriteSwap16(UInt16 value)
{
	swapIt((Int16*)&value);
	this->Write(sizeof(Int16), &value);
}

void  hsStream::WriteSwap16(int count, const UInt16 values[])
{
	for (int i = 0; i < count; i++)
		this->WriteSwap16(values[i]);
}

void  hsStream::WriteSwap32(UInt32 value)
{
	swapIt((Int32*)&value);
	this->Write(sizeof(Int32), &value);
}

void  hsStream::WriteSwap32(int count, const UInt32 values[])
{
	for (int i = 0; i < count; i++)
		this->WriteSwap32(values[i]);
}

void hsStream::WriteUnswap32(UInt32 value)
{
	unswapIt((Int32*)&value);
	this->Write(sizeof(Int32), &value);
}

#if HS_CAN_USE_FLOAT
	void hsStream::WriteSwapDouble(double value)
	{
		swapIt(&value);
		this->Write(sizeof(double), &value);
	}

	void hsStream::WriteSwapDouble(int count, const double values[])
	{
		for (int i = 0; i < count; i++)
			this->WriteSwapDouble(values[i]);
	}

	void hsStream::WriteSwapFloat(float value)
	{
		swapIt(&value);
		this->Write(sizeof(float), &value);
	}

	void hsStream::WriteSwapFloat(int count, const float values[])
	{
		for (int i = 0; i < count; i++)
			this->WriteSwapFloat(values[i]);
	}

	void hsStream::WriteUnswapFloat(float value)
	{
		unswapIt(&value);
		this->Write(sizeof(float), &value);
	}
#endif

void hsStream::WriteSwapAtom(UInt32 tag, UInt32 size)
{
	this->WriteSwap32(tag);
	this->WriteSwap32(size);
}

UInt32 hsStream::ReadSwapAtom(UInt32* sizePtr)
{
	UInt32	tag = this->ReadSwap32();
	UInt32	size = this->ReadSwap32();

	if (sizePtr)
		*sizePtr = size;
	return tag;
}

//////////////////////////////////////////////////////////////////////////////////

#define kFileStream_Uninitialized		~0

hsBool hsFileStream::Open(const char *name, const char *mode)
{
#ifdef HS_BUILD_FOR_PS2
	hsAssert(fRef == kFileStream_Uninitialized, "hsFileStream:Open  Stream already opened");

	Int32 ref = hsPS2Open(name, mode);
	if (ref == -1)
		return false;

	fRef = (UInt32) ref;
	fFileSize = sceLseek(fRef, 0, SCE_SEEK_END);
	sceLseek(fRef, 0, SCE_SEEK_SET);
	fBufferIsEmpty = true;
	fWriteBufferUsed = false;
	fVirtualFilePointer = 0;
	fBufferBase = 0;

	return true;
#else
	hsAssert(0, "hsFileStream::Open  NotImplemented");
	return false;
#endif
}

hsBool hsFileStream::Open(const wchar *name, const wchar *mode)
{
	hsAssert(0, "hsFileStream::Open  NotImplemented");
	return false;
}

hsBool hsFileStream::Close ()
{
#ifdef HS_BUILD_FOR_PS2
	if (fRef != kFileStream_Uninitialized)
	{
		hsPS2Close(fRef);
		fRef = kFileStream_Uninitialized;
	}
	return true;
#else
	hsAssert(0, "hsFileStream::Close  NotImplemented");
	return false;
#endif
}

UInt32 hsFileStream::GetFileRef()
{
	return fRef;
}

void hsFileStream::SetFileRef(UInt32 ref)
{
	hsAssert(ref != kFileStream_Uninitialized, "bad ref");
	fRef = ref;
#if HS_BUILD_FOR_PS2
	fFileSize = sceLseek(fRef, 0, SCE_SEEK_END);
	sceLseek(fRef, 0, SCE_SEEK_SET);
	fBufferIsEmpty= true;
	fWriteBufferUsed= false;
	fVirtualFilePointer= 0;
	fBufferBase= 0;
#endif
}

hsFileStream::hsFileStream()
{
	fRef = kFileStream_Uninitialized;
#if HS_BUILD_FOR_PS2
	fBufferIsEmpty= true;
	fWriteBufferUsed= false;
#endif
}

hsFileStream::~hsFileStream()
{
}

UInt32 hsFileStream::Read(UInt32 bytes,  void* buffer)
{
	hsAssert(fRef != kFileStream_Uninitialized, "fRef uninitialized");

	fBytesRead += bytes;
    fPosition += bytes;

#if HS_BUILD_FOR_MAC
	Int16	err;

	err = FSRead(fRef, (long*)&bytes, buffer);
	if (err == noErr)
		return bytes;
	else
		return 0;
#elif HS_BUILD_FOR_PS2
	Int32 ret;
	Int32 nReadBytes= 0;
	while(bytes){
		if( !fBufferIsEmpty ){	// read at already chatched.
			Int32 DataBytesInBuffer= fBufferBase + kBufferSize - fVirtualFilePointer;
			Int32 ChatchedReadSize= DataBytesInBuffer < bytes ? DataBytesInBuffer : bytes;
			memcpy( buffer, &fBuffer[fVirtualFilePointer-fBufferBase], ChatchedReadSize );
			nReadBytes += ChatchedReadSize;
			buffer= (void *)(((char*)buffer) + ChatchedReadSize);
			fVirtualFilePointer += ChatchedReadSize;
			bytes -= ChatchedReadSize;
			fBufferIsEmpty= (fBufferBase + kBufferSize <= fVirtualFilePointer);
		}
		if( kBufferSize <= bytes ){	// read directry, for Large block read.
			hsAssert( fBufferIsEmpty, "read buffer was not used.");
			Int32 DirectReadSize= bytes - bytes % kBufferSize;
			ret= sceRead(fRef, buffer, DirectReadSize);
			if( ret == -1 ){
				return 0;
			}
			hsAssert( ret == DirectReadSize, "require read size != return size");
			nReadBytes += DirectReadSize;
			buffer= (void *)(((char*)buffer) + DirectReadSize);
			fVirtualFilePointer += DirectReadSize;
			bytes -= DirectReadSize;
		}
		if( 0 < bytes && fBufferIsEmpty ){	// fill buffer
			hsAssert( fVirtualFilePointer % kBufferSize == 0 , "read buffer is not alignment.");
			ret= sceRead(fRef, fBuffer, kBufferSize );
			if( ret == -1 ){
				return 0;
			}
			fBufferBase= fVirtualFilePointer;
			fBufferIsEmpty= false;
		}
	}
	return nReadBytes;

#elif HS_BUILD_FOR_WIN32
	UInt32 rBytes;
	ReadFile((HANDLE)fRef, buffer, bytes, &rBytes, nil);
	if(bytes == rBytes)
		return bytes;
	else
		return 0;
#else
	return 0;
#endif
}

UInt32 hsFileStream::Write(UInt32 bytes, const void* buffer)
{
	hsAssert(fRef != kFileStream_Uninitialized, "fRef uninitialized");

    fBytesRead += bytes;
    fPosition += bytes;

#if HS_BUILD_FOR_MAC
	Int16	err;

	err = FSWrite(fRef, (long*)&bytes, buffer);
	if (err == noErr)
		return bytes;
	else
	{	
		hsDebugMessage("hsFileStream::Write failed", err);
		return 0;
	}
#elif HS_BUILD_FOR_PS2
       Int32 ret;
	
	fWriteBufferUsed =true;	// buffered write was not implement, not yet.
	
	ret = sceWrite(fRef, (void*)buffer ,bytes);
	if(ret != -1)
	  return ret;
	else
	  return 0;
#elif HS_BUILD_FOR_WIN32
	UInt32 wBytes;
	WriteFile((HANDLE)fRef, buffer, bytes, &wBytes, nil);
	if(bytes == wBytes)
		return bytes;
	else
	{
		char str[128];
		sprintf(str, "hsFileStream::Write failed.  err %d", GetLastError());
		hsAssert(false, str);
		return 0;
	}
#else
	return 0;
#endif
}


hsBool hsFileStream::AtEnd()
{
#if HS_BUILD_FOR_MAC
	Int32 eof;
	Int32 pos;
	::GetEOF(fRef, &eof);
	::GetFPos(fRef, &pos);
	return pos >= eof;
#elif HS_BUILD_FOR_PS2
	Int32 rVal = 0;
	if( fWriteBufferUsed || fVirtualFilePointer == 0 ){
		// bufferd write was not implement, yiet.
		rVal = sceLseek(fRef, 0, SCE_SEEK_CUR);
		return rVal >= fFileSize;
	}
	else{	// bufferd read
		return fVirtualFilePointer >= fFileSize;
	}
	
#elif HS_BUILD_FOR_WIN32
	UInt32 bytes;
	PeekNamedPipe((void*)fRef, nil, 0, nil, &bytes, nil);
	return bytes>0;
#else
	hsAssert(0,"No hsStream::AtEnd() implemented for this stream class");
	return false;
#endif
}

void hsFileStream::Skip(UInt32 delta)
{
	fBytesRead += delta;
    fPosition += delta;

#if HS_BUILD_FOR_MAC
	short err = SetFPos(fRef, fsFromMark, delta);
	hsAssert(err == noErr, "SetFPos failed");
#elif HS_BUILD_FOR_PS2
	const Int32 NewPointer= fVirtualFilePointer+delta;
	if( fWriteBufferUsed || fVirtualFilePointer == 0 ){
		// bufferd write was not implement, yiet.
		sceLseek(fRef, delta, SCE_SEEK_CUR);
	}
	else{	// bufferd read.
		if( !fBufferIsEmpty ){
			Int32 CurBlock= fVirtualFilePointer / kBufferSize;
			Int32 NewBlock= NewPointer / kBufferSize;
			if( CurBlock == NewBlock ){
				fVirtualFilePointer += delta;
				return;
			}
			fBufferIsEmpty= false;
		}
		Int32 NewBaseMod= NewPointer % kBufferSize;
		Int32 NewBase= NewPointer - NewBaseMod;
		if( NewBaseMod ){
			sceLseek( fRef, NewBase, SCE_SEEK_SET );
			sceRead( fRef, fBuffer, kBufferSize );
			fVirtualFilePointer= NewPointer;
			fBufferBase= NewBase;
			fBufferIsEmpty= false;
		}
		else{
			// just block border.
			fVirtualFilePointer= NewPointer;
			fBufferBase= NewBase;
		}
	}
#elif HS_BUILD_FOR_WIN32
	hsDebugMessage("hsFileStream::Skip unimplemented", 0);
#endif
}

void hsFileStream::Rewind()
{
	fBytesRead = 0;
    fPosition = 0;

#if HS_BUILD_FOR_MAC
	short err = SetFPos(fRef, fsFromStart, 0);
	hsAssert(err == noErr, "SetFPos failed");
#elif HS_BUILD_FOR_PS2
	if( fWriteBufferUsed || fVirtualFilePointer == 0 ){
		// bufferd write was not implement, yiet.
		sceLseek(fRef,0,SCE_SEEK_SET);
	}
	else{	// bufferd read.
		sceLseek(fRef, 0, SCE_SEEK_SET);
		fBufferIsEmpty= true;
		fVirtualFilePointer= 0;
		fBufferBase= 0;
	}
#elif HS_BUILD_FOR_WIN32
	hsDebugMessage("hsFileStream::Rewind unimplemented", 0);
#endif
}

void hsFileStream::Truncate()
{
	hsDebugMessage("hsFileStream::Truncate unimplemented", 0);
}

//////////////////////////////////////////////////////////////////////////////////////

#if !HS_BUILD_FOR_PS2
#if !(HS_BUILD_FOR_REFERENCE)

hsUNIXStream::~hsUNIXStream()
{
	// Don't Close here, because Sub classes Don't always want that behaviour!
}

hsBool hsUNIXStream::Open(const char *name, const char *mode)
{
	fPosition = 0;
	fRef = hsFopen(name, mode);
	return (fRef) ? true : false;
}

hsBool hsUNIXStream::Open(const wchar *name, const wchar *mode)
{
	fPosition = 0;
	fRef = _wfopen(name, mode);
	return (fRef) ? true : false;
}

hsBool hsUNIXStream::Close()
{
	int rtn = true;
	if (fRef)
		rtn = fclose(fRef);
	fRef = nil;
	delete [] fBuff;
	fBuff = nil;

	return !rtn;
}

UInt32 hsUNIXStream::Read(UInt32 bytes,  void* buffer)
{
	if (!fRef || !bytes)
		return 0;
	int numItems = ::fread(buffer, 1 /*size*/, bytes /*count*/, fRef);
	fBytesRead += numItems;
    fPosition += numItems;
	if ((unsigned)numItems < bytes) {
		if (feof(fRef)) {
			// EOF ocurred
			char str[128];
			sprintf(str, "Hit EOF on UNIX Read, only read %d out of requested %d bytes\n", numItems, bytes);
			hsDebugMessage(str, 0);
		}
		else {
			hsDebugMessage("Error on UNIX Read", ferror(fRef));
		}
	}
	return numItems;
}

hsBool  hsUNIXStream::AtEnd()
{
	if (!fRef)
		return 1;
	hsBool rVal;
	int x = getc(fRef);
	rVal = feof(fRef) != 0;
	ungetc(x, fRef);
	return rVal;
}

UInt32 hsUNIXStream::Write(UInt32 bytes, const void* buffer)
{
	if (!fRef)
		return 0;
    fPosition += bytes;
	return fwrite(buffer, bytes, 1, fRef);
}

void hsUNIXStream::SetPosition(UInt32 position)
{
	if (!fRef || (position == fPosition))
		return;
	fBytesRead = position;
	fPosition = position;
	(void)::fseek(fRef, position, SEEK_SET);
}

void hsUNIXStream::Skip(UInt32 delta)
{
	if (!fRef)
		return;
	fBytesRead += delta;
    fPosition += delta;
	(void)::fseek(fRef, delta, SEEK_CUR);
}

void hsUNIXStream::Rewind()
{
	if (!fRef)
		return;
	fBytesRead = 0;
    fPosition = 0;
	(void)::fseek(fRef, 0, SEEK_SET);
}

void hsUNIXStream::FastFwd()
{
	if (!fRef)
		return;
	(void)::fseek(fRef, 0, SEEK_END);
	fBytesRead = fPosition = ftell(fRef);
}

UInt32	hsUNIXStream::GetEOF()
{
	if( !fRef )
		return 0;

	long oldPos = ftell( fRef );
	(void)::fseek( fRef, 0, SEEK_END );
	UInt32 end = (UInt32)ftell( fRef );
	(void)::fseek( fRef, oldPos, SEEK_SET );

	return end;
}

void hsUNIXStream::Truncate()
{
	if (!fRef)
		return;
#if! __MWERKS__
    int handle = _fileno(fRef);
#if !HS_BUILD_FOR_UNIX
    _chsize(handle, fPosition);
#else
    ftruncate(handle, fPosition);
#endif
#else
#if 1
	UInt32 handle = (UInt32)fRef->handle;
	OSErr err = ::SetEOF(handle, fPosition); 
	if(err != noErr)
	{
		hsThrow("Truncate error!");
	}
#endif
#endif
}

void hsUNIXStream::Flush()
{
	if (!fRef)
		return;
	(void)::fflush(fRef);
}

#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////

plReadOnlySubStream::~plReadOnlySubStream()
{
}

void	plReadOnlySubStream::Open( hsStream *base, UInt32 offset, UInt32 length )
{
	fBase = base;
	fOffset = offset;
	fLength = length;

	fBase->SetPosition( fOffset );
	IFixPosition();
}

void	plReadOnlySubStream::IFixPosition( void )
{
	fPosition = fBase->GetPosition() - fOffset;
}

hsBool	plReadOnlySubStream::AtEnd()
{
	if( fPosition >= fLength )
		return true;
	return false;
}

UInt32	plReadOnlySubStream::Read(UInt32 byteCount, void* buffer)
{
	if( byteCount > GetSizeLeft() )
	{
		hsThrow("Attempting to read past end of stream");
		byteCount = GetSizeLeft();
	}

	UInt32 read = fBase->Read( byteCount, buffer );
	IFixPosition();
	return read;
}

UInt32	plReadOnlySubStream::Write(UInt32 byteCount, const void* buffer)
{
	hsAssert( false, "Write not allowed on an plReadOnlySubStream" );
	return 0;
}

void	plReadOnlySubStream::Skip(UInt32 deltaByteCount)
{
	fBase->Skip( deltaByteCount );
	IFixPosition();
}

void	plReadOnlySubStream::Rewind()
{
	fBase->SetPosition( fOffset );
	IFixPosition();
}

void	plReadOnlySubStream::FastFwd()
{
	fBase->SetPosition( fOffset + fLength );
	IFixPosition();
}

void    plReadOnlySubStream::Truncate()
{
	hsAssert( false, "Can't truncate a read-only stream" );
}

UInt32	plReadOnlySubStream::GetEOF()
{
	return fLength;
}

//////////////////////////////////////////////////////////////////////////////////////

#define kRAMStreamChunkSize		1024

hsRAMStream::hsRAMStream() : fAppender(1, kRAMStreamChunkSize)
{
	fIter.ResetToHead(&fAppender);
}

hsRAMStream::hsRAMStream(UInt32 chunkSize) : fAppender(1, chunkSize)
{
	fIter.ResetToHead(&fAppender);
}

hsRAMStream::~hsRAMStream()
{
}

void hsRAMStream::Reset()
{
	fBytesRead = 0;
    fPosition = 0;

	fAppender.Reset();
	fIter.ResetToHead(&fAppender);
}

hsBool hsRAMStream::AtEnd()
{
	return (fBytesRead >= fAppender.Count() * fAppender.ElemSize());
}

UInt32 hsRAMStream::Read(UInt32 byteCount, void * buffer)
{
	if (fBytesRead + byteCount > fAppender.Count() * fAppender.ElemSize())
	{
		hsThrow("Attempting to read past end of stream");
		byteCount = (fAppender.Count() * fAppender.ElemSize()) - fBytesRead;
	}

	fBytesRead += byteCount;
    fPosition += byteCount;

	fIter.Next(byteCount, buffer);

	return byteCount;
}

UInt32 hsRAMStream::Write(UInt32 byteCount, const void* buffer)
{
    fPosition += byteCount;

	fAppender.PushTail(byteCount, buffer);

	return byteCount;
}

void hsRAMStream::Skip(UInt32 deltaByteCount)
{
    fPosition += deltaByteCount;
	fIter.Next(deltaByteCount, nil);
}

void hsRAMStream::Rewind()
{
	fBytesRead = 0;
    fPosition = 0;
	fIter.ResetToHead(&fAppender);
}

void hsRAMStream::Truncate()
{
	Reset();
}

UInt32 hsRAMStream::GetEOF()
{
	return fAppender.Count() * fAppender.ElemSize();
}

void hsRAMStream::CopyToMem(void* mem)
{
	(void)fAppender.CopyInto(mem);
}

//////////////////////////////////////////////////////////////////////

UInt32 hsNullStream::Read(UInt32 byteCount, void * buffer)
{
	hsThrow("hsNullStream: Can't read from this stream!");
	return 0;
}

UInt32 hsNullStream::Write(UInt32 byteCount, const void* buffer)
{
	fBytesRead += byteCount;
    fPosition += byteCount;

	return byteCount;
}

void hsNullStream::Skip(UInt32 deltaByteCount)
{
	fBytesRead += deltaByteCount;
    fPosition += deltaByteCount;
}

void hsNullStream::Rewind()
{
	fBytesRead = 0;
    fPosition = 0;
}

void hsNullStream::Truncate()
{
}

/////////////////////////////////////////////////////////////////////////////////

hsBool hsReadOnlyStream::AtEnd()
{
	return fData >= fStop;
}

UInt32 hsReadOnlyStream::Read(UInt32 byteCount, void* buffer)
{
	if (fData + byteCount > fStop)
	{
		hsThrow("Attempting to read past end of stream");
		byteCount = GetSizeLeft();
	}

	HSMemory::BlockMove(fData, buffer, byteCount);
	fData += byteCount;
	fBytesRead += byteCount;
    fPosition += byteCount;
	return byteCount;
}

UInt32 hsReadOnlyStream::Write(UInt32 byteCount, const void* buffer)
{
	hsThrow( "can't write to a readonly stream");
	return 0;
}

void hsReadOnlyStream::Skip(UInt32 deltaByteCount)
{
	fBytesRead += deltaByteCount;
    fPosition += deltaByteCount;
	fData += deltaByteCount;
	if (fData > fStop)
		hsThrow( "Skip went past end of stream");
}

void hsReadOnlyStream::Rewind()
{
	fBytesRead = 0;
    fPosition = 0;
	fData = fStart;
}

void hsReadOnlyStream::Truncate()
{
	hsThrow( "can't write to a readonly stream");
}

void hsReadOnlyStream::CopyToMem(void* mem)
{
	if (fData < fStop)
		HSMemory::BlockMove(fData, mem, fStop-fData);
}


////////////////////////////////////////////////////////////////////////////////////
UInt32 hsWriteOnlyStream::Read(UInt32 byteCount, void* buffer)
{
	hsThrow( "can't read to a writeonly stream");
	return 0;
}

UInt32 hsWriteOnlyStream::Write(UInt32 byteCount, const void* buffer)
{
	if (fData + byteCount > fStop)
		hsThrow("Write past end of stream");
	HSMemory::BlockMove(buffer, fData, byteCount);
	fData += byteCount;
	fBytesRead += byteCount;
    fPosition += byteCount;
	return byteCount;
}


///////////////////////////////////////////////////////////////////////////////////

hsQueueStream::hsQueueStream(Int32 size) :
	fSize(size),
	fReadCursor(0),
	fWriteCursor(0)
{
	fQueue = TRACKED_NEW char[fSize];
}

hsQueueStream::~hsQueueStream()
{
	delete [] fQueue;
}

UInt32 hsQueueStream::Read(UInt32 byteCount, void * buffer)
{
	hsAssert(fWriteCursor >= 0 && fWriteCursor < fSize,"hsQueueStream: WriteCursor out of range.");
	hsAssert(fReadCursor >= 0 && fReadCursor < fSize,"hsQueueStream: ReadCursor out of range.");

	Int32 limit, length, total;
	
	limit = fWriteCursor >= fReadCursor ? fWriteCursor : fSize;
	length = hsMinimum(limit-fReadCursor,byteCount);
	HSMemory::BlockMove(fQueue+fReadCursor,buffer,length);
	fReadCursor += length;
	fReadCursor %= fSize;
	total = length;
		
	if (length < byteCount && limit != fWriteCursor)
	{
		limit = fWriteCursor;
		length = hsMinimum(limit,byteCount-length);
		HSMemory::BlockMove(fQueue,static_cast<char*>(buffer)+total,length);
		fReadCursor = length;
		total += length;
	}

	return total;
}

UInt32 hsQueueStream::Write(UInt32 byteCount, const void* buffer)
{
	hsAssert(fWriteCursor >= 0 && fWriteCursor < fSize,"hsQueueStream: WriteCursor out of range.");
	hsAssert(fReadCursor >= 0 && fReadCursor < fSize,"hsQueueStream: ReadCursor out of range.");

	Int32 length;

	length = hsMinimum(fSize-fWriteCursor,byteCount);
	HSMemory::BlockMove(buffer,fQueue+fWriteCursor,length);
	if (fReadCursor > fWriteCursor)
	{
#if 0
		if (fReadCursor < fWriteCursor+length+1)
			hsStatusMessage("ReadCursor wrapped\n");
#endif
		fReadCursor = hsMaximum(fReadCursor,fWriteCursor+length+1);
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

void hsQueueStream::Skip(UInt32 deltaByteCount)
{
	Int32 limit, length;
	
	limit = fWriteCursor >= fReadCursor ? fWriteCursor : fSize;
	length = hsMinimum(limit-fReadCursor,deltaByteCount);
	fReadCursor += length;

	if (length < deltaByteCount && limit != fWriteCursor)
	{
		limit = fWriteCursor;
		length = hsMinimum(limit,deltaByteCount-length);
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

hsBool hsQueueStream::AtEnd()
{
	return fReadCursor == fWriteCursor;
}

///////////////////////////////////////////////////////////////////////////////
// hsBufferedStream
///////////////////////////////////////////////////////////////////////////////

inline void FastByteCopy(void* dest, const void* src, UInt32 bytes)
{
	// Don't use memcpy if the read is 4 bytes or less, it's faster to just do a
	// direct copy
	switch (bytes)
	{
	case 4:
		*((UInt32*)dest) = *((const UInt32*)src);
		break;
	case 2:
		*((UInt16*)dest) = *((const UInt16*)src);
		break;
	case 1:
		*((UInt8*)dest) = *((const UInt8*)src);
		break;
	default:
		memcpy(dest, src, bytes);
	}
}

//#define LOG_BUFFERED

hsBufferedStream::hsBufferedStream()
: fRef(nil)
, fFileSize(0)
, fBufferLen(0)
, fWriteBufferUsed(false)
#ifdef HS_DEBUGGING
, fBufferHits(0)
, fBufferMisses(0)
, fBufferReadIn(0)
, fBufferReadOut(0)
, fReadDirect(0)
, fLastReadPos(0)
, fFilename(nil)
, fCloseReason(nil)
#endif
{
}

hsBufferedStream::~hsBufferedStream()
{
#ifdef LOG_BUFFERED
	delete [] fFilename;
#endif // LOG_BUFFERED
}

hsBool hsBufferedStream::Open(const char* name, const char* mode)
{
	hsAssert(!fRef, "hsBufferedStream:Open Stream already opened");
	fRef = hsFopen(name, mode);
	if (!fRef)
		return false;

	SetFileRef(fRef);

#ifdef LOG_BUFFERED
	fBufferHits = fBufferMisses = 0;
	fBufferReadIn = fBufferReadOut = fReadDirect = fLastReadPos = 0;
	delete [] fFilename;
	fFilename = hsStrdup(name);
	fCloseReason = nil;
#endif // LOG_BUFFERED

	return true;
}

hsBool hsBufferedStream::Open(const wchar *name, const wchar *mode)
{
	hsAssert(0, "hsFileStream::Open  NotImplemented for wchar");
	return false;
}

hsBool hsBufferedStream::Close()
{
	int rtn = true;
	if (fRef)
		rtn = fclose(fRef);
	fRef = nil;

#ifdef LOG_BUFFERED
	hsUNIXStream s;
	static bool firstClose = true;
	if (firstClose)
	{
		firstClose = false;
		s.Open("log\\BufferedStream.csv", "wt");
		s.WriteString("File,Hits,Misses,Read In,Read Out,Read Direct,% Wasted,Reason\n");
	}
	else
		s.Open("log\\BufferedStream.csv", "at");

	int wasted = 100;
	if (fBufferReadIn + fReadDirect > 0)
		wasted -= int((float(fBufferReadOut+fReadDirect) / float(fBufferReadIn+fReadDirect)) * 100.f);

	s.WriteFmt("%s,%d,%d,%u,%u,%u,%d,%s\n",
		fFilename, fBufferHits, fBufferMisses, fBufferReadIn, fBufferReadOut, fReadDirect,
		wasted,
		fCloseReason ? fCloseReason : "Unknown");

	s.Close();
#endif // LOG_BUFFERED

	return !rtn;
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

UInt32 hsBufferedStream::Read(UInt32 bytes, void* buffer)
{
	hsAssert(fRef, "fRef uninitialized");
	if (!fRef || bytes == 0)
		return 0;

	UInt32 numReadBytes = 0;

	while (bytes > 0 && fPosition < fFileSize)
	{
		// First, see if we've got anything in the buffer
		if (fBufferLen > 0)
		{
			// Figure out how much we can copy out of the buffer
			UInt32 bufferPos = fPosition % kBufferSize;
			UInt32 bytesInBuffer = fBufferLen - bufferPos;
			UInt32 cachedReadSize = bytesInBuffer < bytes ? bytesInBuffer : bytes;

			FastByteCopy(buffer, &fBuffer[bufferPos], cachedReadSize);

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
			UInt32 directReadSize = bytes - (bytes % kBufferSize);
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

UInt32 hsBufferedStream::Write(UInt32 bytes, const void* buffer)
{
	hsAssert(fRef, "fRef uninitialized");
	fWriteBufferUsed = true;
	int amtWritten = fwrite((void*)buffer, 1, bytes, fRef);
	fPosition += amtWritten;
	return amtWritten;
}

hsBool hsBufferedStream::AtEnd()
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

void hsBufferedStream::Skip(UInt32 delta)
{
	if (fWriteBufferUsed)
	{
		// buffered write not implemented yet.
		fseek(fRef, delta, SEEK_CUR);
	}
	else
	{
		UInt32 blockStart = ((fPosition + delta) / kBufferSize) * kBufferSize;

		// We've got data in the buffer, see if we can just skip in that
		if (fBufferLen > 0)
		{
			Int32 newBufferPos = Int32(fPosition % kBufferSize) + Int32(delta);

			// If we skipped outside of our buffer, invalidate it
			if (newBufferPos < 0 || UInt32(newBufferPos) >= fBufferLen)
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

UInt32 hsBufferedStream::GetEOF()
{
	if (fWriteBufferUsed)
	{
		if (!fRef)
			return 0;

		long oldPos = ftell(fRef);
		fseek(fRef, 0, SEEK_END);
		UInt32 end = (UInt32)ftell(fRef);
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
