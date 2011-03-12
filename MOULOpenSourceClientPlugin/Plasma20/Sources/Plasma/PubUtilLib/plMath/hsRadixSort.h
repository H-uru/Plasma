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

#ifndef hsRadixSort_inc
#define hsRadixSort_inc

class hsRadixSortElem 
{
public:
	union {
		float			fFloat;
		long			fLong;
		unsigned long	fULong;
	}							fKey;

	void*						fBody;

	hsRadixSortElem*			fNext;
};

class hsRadixSort {
public:
	enum {
		kFloat		= 0x0,
		kSignedInt	= 0x1,
		kUnsigned	= 0x2,
		kReverse	= 0x4
	};
	typedef hsRadixSortElem Elem;

protected:
	Elem*			fList;
	Elem*			fHeads[256];
	Elem*			fTails[256];

	inline void ILink(Elem*& head, Elem*& tail, int i); // inline?
	inline void ISlot(Elem* in, int i); // inline?

	void ICollapse();
	void IUnPackSignedInt();
	void IUnPackFloat();
	void IReverse();

public:

	hsRadixSort();

	Elem*	Sort(Elem* inList, UInt32 flags = 0);

};

#endif // hsRadixSort_inc
