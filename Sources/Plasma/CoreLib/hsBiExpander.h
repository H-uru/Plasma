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

#ifndef hsBiExpander_inc
#define hsBiExpander_inc

#include "hsMemory.h"
#include "hsTemplates.h"

///////////////////////////////////////////////////////////////////////////////
////////////// Expander ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> class hsExpander {
private:
	Int32		fNumPost;
	Int32		fNumPostAlloc;
	T*			fArray;	

	Int32		fGrowBy; // default = 0, to double
	Int32		fMinSize; // default = 1, min == 1

	Int32		fCurrent;

	hsExpander(const hsExpander& x);					// make it passed as ref or pointer

	void		IExpand(int newSize);
public:
	enum { kMissingIndex = -1 };

				hsExpander(Int32 minSize = 1, Int32 growBy = 0);
	virtual		~hsExpander();

	hsExpander<T>& operator=(const hsExpander<T>&orig) { return Copy(orig); }
	hsExpander<T>& Copy(const hsExpander<T>& orig);

	void		SetCount(int cnt) { if( cnt >= fNumPostAlloc )IExpand(cnt); fNumPost = cnt; }
	Int32		GetCount() const { return fNumPost; }
	hsBool		Empty() const { return GetCount() == 0; }
	const T&	Get(Int32 index) const;
	Int32		Get(Int32 index, Int32 count, T data[]) const;
	Int32 		Find(const T&) const;	// returns kMissingIndex if not found

	void		SetArray(T* a, Int32 cnt);
	T*			GetArray() { return fArray; }
	T&			operator[]( Int32 index );
	Int32		Append(const T&); // returns t's index
	T*			Append();
	Int32		Push(const T& t) { return Append(t); }
	T*			Push() { return Append(); }
	T*			Top() { return fNumPost ? fArray + fNumPost-1 : nil; }
	Int32		Pop(T* t); // returns count of remaining
	Int32		Pop();
	void		Reset();	// clears out everything

	T&			Head() { return fArray[0]; }
	T&			Tail() { return fArray[fNumPost-1]; }
	T&			Current() { return fArray[fCurrent]; }
	void		First();
	void		Last();
	void		Plus() { ++fCurrent; }
	hsBool	More() { return (fCurrent < fNumPost); }
};

template <class T>
hsExpander<T>& hsExpander<T>::Copy(const hsExpander<T>& orig)
{
	SetCount(orig.GetCount());
	int i;
	for( i = 0; i < GetCount(); i++ )
		fArray[i] = orig.fArray[i];
	return *this;
}

template <class T>
void hsExpander<T>::SetArray(T* a, Int32 cnt)
{
	delete [] fArray;
	if( a )
		fArray = a;
	fNumPost = fNumPostAlloc = cnt;
}

template <class T>
void hsExpander<T>::IExpand(int newSize)
{
	Int32 newPostAlloc = fNumPostAlloc;
	if( !newPostAlloc )
		newPostAlloc++;
	while( newPostAlloc <= newSize )
		newPostAlloc = fGrowBy ? newPostAlloc + fGrowBy : newPostAlloc << 1;
	T* newArray = TRACKED_NEW T[newPostAlloc];
	int i;
	for( i = 0; i < fNumPost; i++ )
		newArray[i] = fArray[i];
	delete [] (fArray);
	fArray = newArray;
	fNumPostAlloc = newPostAlloc;
}

template <class T>
hsExpander<T>::hsExpander(Int32 minSize, Int32 growBy)
{
	hsThrowIfBadParam(minSize < 0);
	hsThrowIfBadParam(growBy < 0);

	fMinSize = minSize+1;
	fGrowBy = growBy;
	
	fArray	= TRACKED_NEW T[fMinSize];
	fNumPostAlloc = fMinSize;
	
	fNumPost = 0;
}

template <class T>
hsExpander<T>::~hsExpander()
{
	delete [] fArray;
}

template <class T> 
void hsExpander<T>::First()
{ 
	fCurrent = 0; 
}

template <class T> 
void hsExpander<T>::Last()
{ 
	fCurrent = fNumPost-1; 
}

template <class T> 
T& hsExpander<T>::operator[]( Int32 index )
{
	hsDebugCode(hsThrowIfBadParam((index < 0)||(index >= fNumPost));)

	return fArray[index];
}

template <class T> 
const T& hsExpander<T>::Get( Int32 index ) const
{
	hsDebugCode(hsThrowIfBadParam((index < 0)||(index >= fNumPost));)

	return fArray[index];
}

template <class T>
Int32 hsExpander<T>::Get(Int32 index, Int32 count, T data[]) const
{
	if( count > 0 )
	{	hsThrowIfNilParam(data);
		hsThrowIfBadParam((index < 0)||(index >= fNumPost));

		if (index + count > fNumPost)
			count = fNumPost - index;
		for (int i = 0; i < count; i++)
			data[i] = fArray[i + index];
	}
	return count;
}

template <class T>
Int32 hsExpander<T>::Find(const T& obj) const
{
	for (int i = 0; i < fNumPost; i++)
		if (fArray[i] == obj)
			return i;
	return kMissingIndex;
}

template <class T>
Int32 hsExpander<T>::Append(const T& obj)
{
	hsAssert(!(fNumPost >= fNumPostAlloc), "Must be less");
	if( fNumPost == fNumPostAlloc-1 )
		IExpand(fNumPostAlloc);
	fArray[fNumPost] = obj;
	return fNumPost++;
}

template <class T>
T* hsExpander<T>::Append()
{
	hsAssert(!(fNumPost >= fNumPostAlloc), "Must be less");
	if( fNumPost == fNumPostAlloc-1 )
		IExpand(fNumPostAlloc);
	return fArray + fNumPost++;
}

template <class T>
Int32 hsExpander<T>::Pop(T*t)
{
	hsThrowIfBadParam(Empty());
	--fNumPost;
	if( t )
		*t = fArray[fNumPost];
	return GetCount();
}

template <class T>
Int32 hsExpander<T>::Pop()
{
	hsThrowIfBadParam(Empty());
	--fNumPost;
	return GetCount();
}

template <class T>
void hsExpander<T>::Reset()
{
	fNumPost = 0;
}

///////////////////////////////////////////////////////////////////////////////
////////////// BiExpander /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> class hsBiExpander {
private:
	Int32		fNumPre;
	Int32		fNumPost;
	Int32		fNumPreAlloc;
	Int32		fNumPostAlloc;
	T*			fArray;	

	Int32		fGrowBy; // default = 0, to double
	Int32		fMinSize; // default = 1, min == 1

	Int32		fCurrent;

	hsBiExpander<T>& operator=(const hsBiExpander<T>&);		// don't allow assignment
	hsBiExpander(const hsBiExpander<T>&);					// make it passed as ref or pointer

	void		IExpand(int newSize, hsBool towardEnd = true);
public:
	enum { kMissingIndex = -1 };

				hsBiExpander(Int32 minSize = 1, Int32 growBy = 0);
	virtual		~hsBiExpander();

	Int32		GetFirst() const { return -fNumPre; }
	Int32		GetCount() const { return fNumPre + fNumPost; }
	hsBool		Empty() const { return GetCount() == 0; }
	const T&	Get(Int32 index) const;
	Int32		Get(Int32 index, Int32 count, T data[]) const;
	Int32 		Find(const T&) const;	// returns kMissingIndex if not found

	void		SetArray(T* a, Int32 cnt, Int32 numPre=0);
	T**			GetArray() { return fArray - fNumPre; }
	T&			operator[]( Int32 index );
	T*			Append(); // returns t's index
	T*			Push(); // returns t's index
	Int32		Append(const T&); // returns t's index
	Int32		Push(const T&); // returns t's index
	Int32		Pop(T*t = nil) { return PopHead(t); } // returns count of remaining
	Int32		PopHead(T*t = nil); // returns count of remaining
	Int32		PopTail(T*t = nil); // returns count of remaining
	void		Reset();	// clears out everything

	T&			Head() { return fArray[-fNumPre]; }
	T&			Tail() { return fArray[fNumPost-1]; }
	T&			Current() { return fArray[fCurrent]; }
	void		First();
	void		Last();
	void		Plus() { ++fCurrent; }
	void		Minus() { --fCurrent; }
	hsBool		More() { return (fCurrent < fNumPost)&&(fCurrent >= -fNumPre); }
};

template <class T>
void hsBiExpander<T>::SetArray(T* a, Int32 cnt, Int32 numPre)
{
	if( !numPre )
		Reset();
	else
	{
		fNumPreAlloc = fNumPre = numPre;
		fNumPostAlloc = fNumPost = cnt - numPre;
		fArray = a + numPre;
	}
}

template <class T>
void hsBiExpander<T>::IExpand(int newSize, hsBool towardEnd)
{
	Int32 newPreAlloc = fNumPreAlloc;
	Int32 newPostAlloc = fNumPostAlloc;
	if( towardEnd )
	{
		if( !newPostAlloc )
			newPostAlloc++;
		while( newPostAlloc <= newSize )
			newPostAlloc = fGrowBy ? newPostAlloc + fGrowBy : newPostAlloc << 1;
	}
	else
	{
		if( !newPreAlloc )
			newPreAlloc++;
		while( newPreAlloc <= newSize )
			newPreAlloc = fGrowBy ? newPreAlloc + fGrowBy : newPreAlloc << 1;
	}
	T* newArray = TRACKED_NEW T[newPreAlloc + newPostAlloc];
	newArray += newPreAlloc;
	int i;
	for( i = -fNumPre; i < fNumPost; i++ )
		newArray[i] = fArray[i];
//	HSMemory::BlockMove(fArray-fNumPre, newArray-fNumPre, 
//		(fNumPre+fNumPost)*sizeof(*fArray));
	delete [] (fArray-fNumPreAlloc);
	fArray = newArray;
	fNumPreAlloc = newPreAlloc;
	fNumPostAlloc = newPostAlloc;
}

template <class T>
hsBiExpander<T>::hsBiExpander(Int32 minSize, Int32 growBy)
{
	hsThrowIfBadParam(minSize < 0);
	hsThrowIfBadParam(growBy < 0);

	fMinSize = minSize+1;
	fGrowBy = growBy;
	
	fArray	= TRACKED_NEW T[fMinSize << 1];
	fNumPreAlloc = fNumPostAlloc = fMinSize;
	fArray += fNumPreAlloc;
	
	fNumPre = fNumPost = 0;
}

template <class T>
hsBiExpander<T>::~hsBiExpander()
{
	delete [] (fArray - fNumPreAlloc);
}

template <class T> 
void hsBiExpander<T>::First()
{ 
	fCurrent = -fNumPre; 
}

template <class T> 
void hsBiExpander<T>::Last()
{ 
	fCurrent = fNumPost-1; 
}

template <class T> 
T& hsBiExpander<T>::operator[]( Int32 index )
{
	hsDebugCode(hsThrowIfBadParam((index < -fNumPre)||(index >= fNumPost));)

	return fArray[index];
}

template <class T> 
const T& hsBiExpander<T>::Get( Int32 index ) const
{
	hsDebugCode(hsThrowIfBadParam((index < -fNumPre)||(index >= fNumPost));)

	return fArray[index];
}

template <class T>
Int32 hsBiExpander<T>::Get(Int32 index, Int32 count, T data[]) const
{
	if( count > 0 )
	{	hsThrowIfNilParam(data);
		hsThrowIfBadParam((index < -fNumPre)||(index >= fNumPost));

		if (index + count > fNumPost)
			count = fNumPost - index;
		for (int i = 0; i < count; i++)
			data[i] = fArray[i + index];
	}
	return count;
}

template <class T>
Int32 hsBiExpander<T>::Find(const T& obj) const
{
	for (int i = -fNumPre; i < fNumPost; i++)
		if (fArray[i] == obj)
			return i;
	return kMissingIndex;
}

template <class T>
T* hsBiExpander<T>::Append()
{
	hsAssert(!(fNumPost >= fNumPostAlloc), "Must be less");
	if( fNumPost == fNumPostAlloc-1 )
		IExpand(fNumPostAlloc, true);
	return fArray + fNumPost++;
}

template <class T>
T* hsBiExpander<T>::Push()
{
	hsAssert(!(fNumPre >= fNumPreAlloc), "Must be less");
	if( ++fNumPre == fNumPreAlloc )
		IExpand(fNumPreAlloc, false);
	return fArray - fNumPre;
}

template <class T>
Int32 hsBiExpander<T>::Append(const T& obj)
{
	hsAssert(!(fNumPost >= fNumPostAlloc), "Must be less");
	if( fNumPost == fNumPostAlloc-1 )
		IExpand(fNumPostAlloc, true);
	fArray[fNumPost] = obj;
	return fNumPost++;
}

template <class T>
Int32 hsBiExpander<T>::Push(const T& obj)
{
	hsAssert(!(fNumPre >= fNumPreAlloc), "Must be less");
	if( ++fNumPre == fNumPreAlloc )
		IExpand(fNumPreAlloc, false);
	fArray[-fNumPre] = obj;
	return -fNumPre;
}

template <class T>
Int32 hsBiExpander<T>::PopHead(T*t)
{
	hsThrowIfBadParam(Empty());
	if( t )
		*t = fArray[-fNumPre];
	--fNumPre;
	return GetCount();
}

template <class T>
Int32 hsBiExpander<T>::PopTail(T*t)
{
	hsThrowIfBadParam(Empty());
	--fNumPost;
	if( t )
		*t = fArray[fNumPost];
	return GetCount();
}

template <class T>
void hsBiExpander<T>::Reset()
{
	fNumPre = fNumPost = 0;
}

#endif // hsBiExpander_inc
