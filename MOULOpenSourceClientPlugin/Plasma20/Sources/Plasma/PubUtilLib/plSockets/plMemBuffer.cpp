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
#include "hsUtils.h"
#include "plMemBuffer.h"

/////////////////////////////////////////////////

plMemBuffer::plMemBuffer()
{
	fBuffer = nil;
	fBufferLocal = false;
	fBufferLen = 0;
}


plMemBuffer::plMemBuffer(int len)
{
	AllocBuffer(len);
}


plMemBuffer::plMemBuffer(char * data, int len)
{
	fBufferLocal = false;
	fBufferLen = len;
	fBuffer = data;
}


plMemBuffer::~plMemBuffer()
{
	ClearBuffer();
}


void plMemBuffer::SetBuffer(char * data, int len)
{
	ClearBuffer();
	fBufferLocal = false;
	fBufferLen = len;
	fBuffer = data;        
}


void plMemBuffer::CopyBuffer(char * data, int len)
{
	char * tmp =  TRACKED_NEW char[len];
	memcpy(tmp,data,len);
	ClearBuffer();
	AllocBuffer(len);
	memcpy(fBuffer,tmp,len);
	delete [] tmp;
}


void plMemBuffer::GrowBuffer(int newLen)
{
	const int kThrashSize	= 25;
	if(newLen >= fBufferLen)
	{
		int len = newLen + kThrashSize;
		len = len+len;
		
		char * tmp =  TRACKED_NEW char[len];
		
		if(fBuffer != nil)
			memcpy(tmp,fBuffer,fBufferLen);
		
		ClearBuffer();
		
		fBuffer = tmp;
		fBufferLocal = true;
		fBufferLen = len;
	}
}


int plMemBuffer::GetBufferSize()  
{
	return  fBufferLen; 
};


char * plMemBuffer::GetBuffer() 
{
	return  fBuffer; 
};


bool plMemBuffer::InBufferRange(char * inpos)
{
	return (inpos >= fBuffer && inpos <= (fBuffer + fBufferLen));
}

void plMemBuffer::ClearBuffer()
{
	if(fBufferLocal == true)
	{
		if(fBuffer != nil)
			delete [] fBuffer;
		fBuffer = nil;
	}        
}

void plMemBuffer::AllocBuffer(int len)
{        
	fBuffer = TRACKED_NEW char[len];
	fBufferLocal = true;
	fBufferLen = len;
}


