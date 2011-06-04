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
#ifndef hsRefCnt_Defiend
#define hsRefCnt_Defiend

class hsRefCnt {
private:
	int			fRefCnt;
public:
				hsRefCnt() : fRefCnt(1) {}
	virtual		~hsRefCnt();

	virtual int		RefCnt() const { return fRefCnt; }
	virtual void	UnRef();
	virtual void	Ref();
};

#define hsRefCnt_SafeRef(obj)		do { if (obj) (obj)->Ref(); } while (0)
#define hsRefCnt_SafeUnRef(obj)	do { if (obj) (obj)->UnRef(); } while (0)

#define hsRefCnt_SafeAssign(dst, src)		\
		do {							\
			hsRefCnt_SafeRef(src);		\
			hsRefCnt_SafeUnRef(dst);		\
			dst = src;					\
		} while (0)

#endif
