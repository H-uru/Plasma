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
// hsHashTable.h

#ifndef _hsHashTable_Included_
#define _hsHashTable_Included_

#include "hsTemplates.h"

template <class T>
class hsHashTableIterator
{
public:
	hsHashTableIterator() : fList(nil), fIndex(-1)	{ }
	explicit hsHashTableIterator(hsTArray<T>* list, UInt32 idx)	: fList(list), fIndex(idx) { }
	
	T* operator->() const	{ return &((*fList)[fIndex]); }
	T& operator*() const	{ return (*fList)[fIndex]; }
	
	hsHashTableIterator<T>& operator++()			{ fIndex--; return *this; }
	const hsHashTableIterator<T>& operator++(int)	{ hsHashTableIterator<T> temp(*this); --(*this); return temp; }
	
	hsHashTableIterator<T>& operator--()			{ fIndex++; return *this; }
	const hsHashTableIterator<T>& operator--(int)	{ hsHashTableIterator<T> temp(*this); ++(*this); return temp; }
	
	hsBool operator==(const hsHashTableIterator<T>& other) const	{ return fList==other.fList && fIndex==other.fIndex; }
	hsBool operator!=(const hsHashTableIterator<T>& other) const	{ return !(*this == other); }
	
private:
	hsTArray<T>* fList;
	UInt32 fIndex;
};


template <class T> 
class hsHashTable
{
public:
	hsHashTable(UInt32 size=150001, UInt32 step=1);
	~hsHashTable();

	typedef hsHashTableIterator<T> iterator;

	iterator begin()	{ return iterator(&fItemList,fItemList.Count()-1); }
	iterator end()		{ return iterator(&fItemList,0); }
	void clear();

	UInt32 count()			{ return fItemList.Count()-1; }
	UInt32 size()			{ return fSize; }

	UInt32 CollisionCount() { return fCollisionCount; }

	inline void insert(T& item);
	inline void erase(T& item);
	inline iterator find(const T& item);

	iterator GetItem(UInt32 i);
	
private:
	hsTArray<T>			fItemList;
	hsTArray<UInt32>	fClearList;

	UInt32*		fHashTable;
	UInt32		fSize;
	UInt32		fCollisionStep;
	UInt32		fCollisionCount;

    // No copy or assignment
	hsHashTable(const hsHashTable&);
    hsHashTable &operator=(const hsHashTable&);
};

template <class T>
hsHashTable<T>::hsHashTable(UInt32 size, UInt32 step) :
fSize(size),
fCollisionStep(step),
fCollisionCount(0)
{
	fItemList.SetCount(1);
	fHashTable = TRACKED_NEW UInt32[fSize];
	memset(fHashTable,0,fSize*sizeof(UInt32));
}

template <class T>
hsHashTable<T>::~hsHashTable()
{
	delete [] fHashTable;
}

template <class T>
void hsHashTable<T>::clear()
{
	fItemList.SetCount(1);
	for (Int32 i=0; i<fClearList.Count(); i++)
		fHashTable[ fClearList[i] ] = 0;
	fClearList.Reset();
}

template <class T>
void hsHashTable<T>::insert(T& item)
{
	hsAssert(fClearList.Count() < fSize,"Hash table overflow!  Increase the table size.");
	UInt32 h = item.GetHash();
	h %= fSize;
	while (UInt32 it = fHashTable[h])
	{
		if ( fItemList[it] == item)
		{
			fItemList[it] = item;
			return;
		}
		h += fCollisionStep;
		h %= fSize;
		fCollisionCount++;
	}
	fHashTable[h] = fItemList.Count();
	fItemList.Append(item);
	fClearList.Append(h);
}

template <class T>
void hsHashTable<T>::erase(T& item)
{
	UInt32 h = item.GetHash();
	h %= fSize;
	while (UInt32 it = fHashTable[h])
	{
		if ( fItemList[it] == item )
		{
			fHashTable[h] = 0;
			return;
		}
	}
}

template <class T>
hsHashTableIterator<T> hsHashTable<T>::find(const T& item)
{
	UInt32 h = item.GetHash();
	h %= fSize;
	while (UInt32 it = fHashTable[h])
	{
		if ( fItemList[it] == item )
		{
			return iterator(&fItemList,it);
		}
		h += fCollisionStep;
		h %= fSize;
		fCollisionCount++;
	}
	return end();
}

template <class T>
hsHashTableIterator<T> hsHashTable<T>::GetItem(UInt32 i)
{
	return iterator(&fItemList,i+1);
}

#endif // _hsHashTable_Included_
