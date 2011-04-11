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

#ifndef hsTempPointer_inc
#define hsTempPointer_inc

#include "hsMemory.h"
#include "hsExceptions.h"

template <class T> class hsTempPointer {
private:
	T**			fArray;

	UInt32		fCurrBlock;
	UInt32		fNumBlockAlloc;
	
	UInt32		fCurrElem;
	UInt32		fNumElemAlloc;

	UInt32		fGrowBy; // def = 0, to double
	UInt32		fMinSize; // def = 1

	hsTempPointer<T>& operator=(const hsTempPointer<T>&);

	void		IConsolidate();
	void		IGrow();

public:
	hsTempPointer(UInt32 minSize = 1, UInt32 growBy = 0);
	~hsTempPointer();

	void		Reset();

	T*			Next();
	T*			Array(int n);
};

template <class T>
hsTempPointer<T>::~hsTempPointer()
{
	int i;
	for( i = 0; i <= fCurrBlock; i++ )
		delete [] fArray[i];
	delete [] fArray;
}

template <class T>
hsTempPointer<T>::hsTempPointer(UInt32 minSize, UInt32 growBy)
{
	fGrowBy = growBy;
	fMinSize = minSize;

	fArray = TRACKED_NEW T*[2];
	fNumBlockAlloc = 2;
	fCurrBlock = 0;

	fArray[fCurrBlock] = TRACKED_NEW T[fMinSize];
	fNumElemAlloc = minSize;

	fCurrElem = 0;
}


template <class T>
void hsTempPointer<T>::IConsolidate()
{
	hsAssert(fCurrBlock > 0, "Shouldn't consolidate when nothing to do");

	UInt32 numUsed = fCurrBlock * fNumElemAlloc + fCurrElem;

	UInt32 newSize = fNumElemAlloc;
	if( !fGrowBy )
	{
		while( newSize <= numUsed )
			newSize <<= 1;
	}
	else
	{
		while( newSize <= numUsed )
			newSize += fGrowBy;
	}
	int i;
	for( i = 0; i <= fCurrBlock; i++ )
		delete [] fArray[i];

	fArray[0] = TRACKED_NEW T[newSize];
	fNumElemAlloc = newSize;
	fCurrElem = 0;
	fCurrBlock = 0;
}

template <class T>
void hsTempPointer<T>::IGrow()
{
	if( ++fCurrBlock >= fNumBlockAlloc )
	{
		T** newBlockArray = TRACKED_NEW T*[fNumBlockAlloc <<= 1];
		HSMemory::BlockMove(fArray, newBlockArray, fCurrBlock * sizeof(*fArray));
		delete [] fArray;
		fArray = newBlockArray;
	}
	fArray[fCurrBlock] = TRACKED_NEW T[fNumElemAlloc];
	fCurrElem = 0;

}

template <class T>
T* hsTempPointer<T>::Next()
{
	if( fCurrElem >= fNumElemAlloc )
		IGrow();
	return fArray[fCurrBlock] + fCurrElem++;
}

template <class T>
T* hsTempPointer<T>::Array(int n)
{
	// minSize (on constructor) should be greater than max n
	hsDebugCode(hsThrowIfBadParam((UInt32)n > (UInt32)fNumElemAlloc);)
	if( fCurrElem + n >= fNumElemAlloc )
		IGrow();
	int idx = fCurrElem;
	fCurrElem += n;
	return fArray[fCurrBlock] + idx;
}

template <class T>
void hsTempPointer<T>::Reset()
{
	if( fCurrBlock > 0 )
		IConsolidate();
	fCurrElem = 0;
}

#endif // hsTempPointer_inc
