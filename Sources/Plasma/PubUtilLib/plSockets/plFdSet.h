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
#ifndef plFdSet_h_inc
#define plFdSet_h_inc

#include "plNet.h"

////////////////////////////////////////////////////

class plSocket;

////////////////////////////////////////////////////

class plFdSet
{
public:
	enum { kInfinite = 0xffffffff };

public:
	plFdSet();
	void SetForSocket(plSocket & in);
	void ClearForSocket(plSocket & in);
	bool IsSetFor(plSocket & in);
	bool IsErrFor(plSocket & in);
	int WaitForRead(bool shouldZeroFds, unsigned long timeoutMillis=kInfinite);
	int WaitForWrite(bool shouldZeroFds, unsigned long timeoutMillis=kInfinite);
	int WaitForError(bool shouldZeroFds, unsigned long timeoutMillis=0);
	void ZeroFds();

private:
	unsigned	fMaxFd;
	fd_set		fFds;
	fd_set		fErrFds;
};




#endif //plFdSet_h_inc
