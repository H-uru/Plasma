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

#ifndef hsSearchVersion_inc
#define hsSearchVersion_inc

#include "hsTypes.h"

/*
do a template of lists to search for a matching entry. basic idea is that
you start off with an array of buckets and you know that you will get at
least one, possibly several, in each bucket. when you go to search, you already
know which bucket it will be in if it's there. even as the array is being filled,
each filled entry in each bucket has a valid forever index, as well as it's key
value. so array is fixed length, index into array has no bearing on forever index,
elements of array can grow, and at all times the used forever indices form a contiguous
set from 0 to max forever index.
*/

template <class T> class hsVersionNode {
protected:
	T										fData;
	Int32									fIndex;
	hsVersionNode<T>*		fNext;
public:
	hsVersionNode(const UInt32 idx, const T &data) : fIndex(idx), fNext(nil) { fData = data; }
	~hsVersionNode() { delete fNext; }

	hsVersionNode<T>*	Next() const { return fNext; }

	Int32				Index() const { return fIndex; }

	inline void Append(hsVersionNode<T>* next);
	inline int operator==(const T& o) const;
	
	int operator!=(const T& o) const { return !(this->operator==(o)); }

	T&			GetData() { return fData; }
};

template <class T> int hsVersionNode<T>::operator==(const T& data) const
{
	return fData == data;
}

template <class T> void hsVersionNode<T>::Append(hsVersionNode<T>* next)
{
	if( fNext )
		fNext->Append(next);
	else
		fNext = next;
}

template <class T> class hsSearchVersion {
protected:
	UInt32				fLength;
	hsVersionNode<T>**	fArray;
	UInt32				fNextIndex;
	UInt32				fNumIndex;
	UInt32				fIncIndex;
	T**					fBackArray;

	void				ICheckBackArray();
public:
	hsSearchVersion(UInt32 len, UInt32 inc = 0);
	~hsSearchVersion();

	T&			operator[]( Int32 index );

	Int32				Find(int where, const T& what, hsBool forceUnique=false);

	UInt32				GetCount() const { return fNextIndex; }
};

template <class T> T& hsSearchVersion<T>::operator[]( Int32 index )
{
	hsDebugCode(hsThrowIfBadParam((UInt32)index >= (UInt32)fNextIndex);)

	return *fBackArray[index];
}

template <class T> hsSearchVersion<T>::hsSearchVersion(UInt32 len, UInt32 inc)
	: fNextIndex(0)
{ 
	fIncIndex = inc ? inc : len;
	fLength = len; 
	fArray = TRACKED_NEW hsVersionNode<T>*[fLength]; 
	HSMemory::Clear(fArray, fLength*sizeof(*fArray)); 
	fBackArray = TRACKED_NEW T*[fNumIndex = fLength];
}

template <class T> hsSearchVersion<T>::~hsSearchVersion()
{
	int i;
	for( i = 0; i < fLength; i++ )
		delete fArray[i];
	delete [] fArray;
	delete [] fBackArray;
}

template <class T> void hsSearchVersion<T>::ICheckBackArray()
{
	if( fNextIndex >= fNumIndex )
	{
		T**	newBackArray = TRACKED_NEW T*[fNumIndex + fIncIndex];
		HSMemory::BlockMove(fBackArray, newBackArray, fNextIndex*sizeof(T*));
		delete [] fBackArray;
		fBackArray = newBackArray;
		fNumIndex += fIncIndex;
	}
}

template <class T> Int32 hsSearchVersion<T>::Find(int where, const T&what, hsBool forceUnique)
{
	hsVersionNode<T>* curr = fArray[where];

	ICheckBackArray();

	if( !curr )
	{
		hsVersionNode<T>* next = TRACKED_NEW hsVersionNode<T>(fNextIndex, what);
		fArray[where] = next;
		fBackArray[fNextIndex] = &next->GetData();
		return fNextIndex++;
	}
	if( *curr == what )
		return curr->Index();

	while( curr->Next() 
		&& (forceUnique || (*curr->Next() != what)) )
		curr = curr->Next();

	if( curr->Next() )
		return curr->Next()->Index();

	hsVersionNode<T>* next = TRACKED_NEW hsVersionNode<T>(fNextIndex, what);
	curr->Append(next);
	fBackArray[fNextIndex] = &next->GetData();
	return fNextIndex++;
}

#endif // hsSearchVersion_inc
