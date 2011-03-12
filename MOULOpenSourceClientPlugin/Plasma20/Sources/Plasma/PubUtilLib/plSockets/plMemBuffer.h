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
#ifndef plMemBuffer_h_inc
#define plMemBuffer_h_inc


class plMemBuffer
{
public:
	plMemBuffer();
	plMemBuffer(int len);
	plMemBuffer(char * data, int len);
	virtual ~plMemBuffer();
	void SetBuffer(char * data, int len);
	void CopyBuffer(char * data, int len);
	void GrowBuffer(int len);
	int GetBufferSize();
	char * GetBuffer();
	bool InBufferRange(char *);

protected:
	bool fBufferLocal;
	int fBufferLen;
	char * fBuffer;
	
	void ClearBuffer();
	void AllocBuffer(int len);
};



#endif //  plMemBuffer_h_inc
