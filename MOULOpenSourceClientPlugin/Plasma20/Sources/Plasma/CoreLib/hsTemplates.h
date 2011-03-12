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
#ifndef hsTemplatesDefined
#define hsTemplatesDefined

#include "hsExceptions.h"
#include "hsMemory.h"
#include "hsRefCnt.h"
#include "hsUtils.h"

#include <stdarg.h>


#ifdef HS_DEBUGGING
// #define	 HS_DEBUGTARRAY
#endif

#ifdef HS_DEBUGTARRAY

	// just a quickie d-link list class for debugging
class hsDlistNode
{
public:
	static hsDlistNode *fpFirst;
	static hsDlistNode *fpLast;
	static UInt32 		fcreated;
	static UInt32 		fdestroyed;

	void *fpThing;
	hsDlistNode *fpPrev;
	hsDlistNode *fpNext;
	hsDlistNode(void *tng): fpThing(tng), fpNext(0), fpPrev(0) { AddNode(); }
	void AddNode();
	void RemoveNode();
	hsDlistNode *GetNext() { return fpNext; }
};

#endif	

// Use this for a pointer to a single object of class T allocated with new
template <class T> class hsTempObject {
	T*	fObject;
public:
	hsTempObject(): fObject(nil){}
	hsTempObject(T* p) : fObject(p) {}
	hsTempObject(const hsTempObject & that)
	{*this=that;}
	~hsTempObject() { delete fObject; }
	hsTempObject & operator=(const hsTempObject & src)
	{
		if (fObject!=src.fObject)
		{
			delete fObject;
			fObject=src.fObject;
		}
		return *this;
	}
	hsTempObject & operator=(T * ptr)
	{
		if (fObject!=ptr)
		{
			delete fObject;
			fObject=ptr;
		}
		return *this;
	}
	operator T*() const { return fObject; }
	operator T*&() { return fObject; }
	operator const T&() const { return *fObject; }
	operator bool() const { return fObject!=nil;}
	T * operator->() const { return fObject; }
	T * operator *() const { return fObject; }
};

// Use this for subclasses of hsRefCnt, where UnRef should be called at the end
template <class T> class hsTempRef {
	T*	fObject;
public:
		hsTempRef(T* object = nil) : fObject(object) {}
		~hsTempRef() { if (fObject) fObject->UnRef(); }

		operator T*() const { return fObject; }
	T*	operator->() const { return fObject; }
	
	T*	operator=(T* src) { hsRefCnt_SafeUnRef(fObject); fObject = src; return fObject; }
};

// Use this for an array of objects of class T allocated with new[]
template <class T> class hsTempArray {
	T*		fArray;
	UInt32	fCount;
	hsTempArray<T>&	operator=(const hsTempArray<T>&);
public:
	hsTempArray(long count) : fArray(TRACKED_NEW T[count]), fCount(count)
	{
	}
	hsTempArray(long count, T initValue) : fArray(TRACKED_NEW T[count]), fCount(count)
	{
		for (int i = 0; i < count; i++)
			fArray[i] = initValue;
	}
	hsTempArray(T* p) : fArray(p), fCount(1)
	{
	}
	hsTempArray() : fArray(nil), fCount(0)
	{
	}
	~hsTempArray()
	{
		delete[] fArray;
	}

	operator T*() const { return fArray; }
	T* GetArray() const { return fArray; }
	void Accomodate(UInt32 count)
	{
		if (count > fCount)
		{	delete[] fArray;
			fCount = count;
			fArray = TRACKED_NEW T[count];
		}
	}
};

////////////////////////////////////////////////////////////////////////////////
//
//// Like hsTempArray, but more useful when working with char * type arrays.
//enum KStringFormatConstructor	{kFmtCtor};
//enum KStringFormatVConstructor	{kFmtVCtor};
//class hsTempString
//{
//public:
//	char * fStr;
//	hsTempString(): fStr(nil){}
//	hsTempString(char * p) : fStr(p) {}
//	hsTempString(const char * p) { fStr=hsStrcpy(p); }
//	hsTempString(KStringFormatConstructor, char * fmt, ...);
//	hsTempString(KStringFormatVConstructor, char * fmt, va_list args);
//	hsTempString(const hsTempString & other):fStr(hsStrcpy(other.fStr)){}
//	virtual ~hsTempString() { delete [] fStr; }
//	hsTempString & operator=(char * ptr)
//	{
//		if (fStr!=ptr)
//		{
//			delete [] fStr;
//			fStr=ptr;
//		}
//		return *this;
//	}
//	hsTempString & operator=(const hsTempString & other)
//	{
//		delete [] fStr;
//		fStr=hsStrcpy(other.fStr);
//		return *this;
//	}
//	operator char *() const { return fStr; }
//	operator char *&() { return fStr; }
//	operator const char *() const { return fStr; }
//	operator bool() const { return fStr!=nil;}
//	char * operator *() const { return fStr; }
//	const char* c_str() const { return fStr; }
//	char* c_str() { return fStr; }
//};
//
//// shorthand
//typedef hsTempString tmpstr_t;
//
//class hsTempStringF : public hsTempString
//{
//public:
//	hsTempStringF(char * fmt, ...);
//	void Format(char * fmt, ...);
//
//	hsTempString & operator=(char * ptr) { return hsTempString::operator=(ptr);	}
//	hsTempString & operator=(const hsTempString & other) { return hsTempString::operator=(other);	}
//	hsTempString & operator=(const hsTempStringF & other) { return hsTempString::operator=(other);	}
//	operator char *() const { return fStr; }
//	operator char *&() { return fStr; }
//	operator const char *() const { return fStr; }
//	operator bool() const { return fStr!=nil;}
//	char * operator *() const { return fStr; }	
//};

//////////////////////////////////////////////////////////////////////////////

template <class T> class hsDynamicArray {
private:
	Int32		fCount;
	T*			fArray;	

	hsDynamicArray<T>&	operator=(const hsDynamicArray<T>&);		// don't allow assignment
public:
	enum { kMissingIndex = -1 };

				hsDynamicArray(Int32 count = 0);
	virtual		~hsDynamicArray();

	Int32		GetCount() const { return fCount; }
	hsBool		IsEmpty() const { return fCount == 0; }
	const T&		Get(Int32 index) const;
	Int32		Get(Int32 index, Int32 count, T data[]) const;
	Int32 		Find(const T&) const;	// returns kMissingIndex if not found

	void			SetCount(Int32 count);
	T&			operator[]( Int32 index );
	Int32		Append(const T&);
	Int32		InsertAtIndex(UInt32 index, const T& obj);
	Int32		Push(const T&);
	Int32		Pop(T*);
	void			Remove(Int32);
	void			Reset();	// clears out everything

	T*			AcquireArray() { return fArray; }
	T*			DetachArray() { T* t = fArray; fCount = 0; fArray = nil; return t;  }
	void			ReleaseArray(T*) {}
	hsDynamicArray<T>*	Copy(hsDynamicArray<T>* dst = nil) const;

	T*			ForEach(Boolean (*proc)(T&));
	T*			ForEach(Boolean (*proc)(T&, void* p1), void* p1);
	T*			ForEach(Boolean (*proc)(T&, void* p1, void* p2), void* p1, void* p2);
};

// Use this for block of memory allocated with HSMemory::New()
template <class T> class hsDynamicArrayAccess {
	T*	fArray;
	hsDynamicArray<T> *fArrayObj;
	hsDynamicArrayAccess<T>& operator=(const hsDynamicArrayAccess<T>&);
public:
	hsDynamicArrayAccess(hsDynamicArray<T> *array) : fArrayObj(array) { fArray = array->AcquireArray();}
	~hsDynamicArrayAccess() { fArrayObj->ReleaseArray(fArray); }
		
		operator T*() const { return fArray; }
	T*	operator->() const { return fArray; }
};


template <class T>
	hsDynamicArray<T>::hsDynamicArray(Int32 count)
{
	fCount	= count;
	fArray	= nil;
	if (count)
		fArray = TRACKED_NEW T[ count ];
}

template <class T>
hsDynamicArray<T>::~hsDynamicArray()
{
	this->Reset();
}

template <class T>
void hsDynamicArray<T>::SetCount(Int32 count)
{
	if (fCount != count)
	{	if (count == 0)
			this->Reset();
		else
		{	T*	newArray = TRACKED_NEW T[count];
			
			if (fArray)
			{	int	copyCount = hsMinimum(count, fCount);

				for (int i = 0; i < copyCount; i++)
					newArray[i] = fArray[i];
				delete[] fArray;
			}
			fCount = count;
			fArray = newArray;
		}
	}
}

template <class T> T& hsDynamicArray<T>::operator[]( Int32 index )
{
	hsDebugCode(hsThrowIfBadParam((UInt32)index >= (UInt32)fCount);)

	return fArray[index];
}

template <class T> const T& hsDynamicArray<T>::Get( Int32 index ) const
{
	hsDebugCode(hsThrowIfBadParam((UInt32)index >= (UInt32)fCount);)

	return fArray[index];
}

template <class T>
Int32 hsDynamicArray<T>::Get(Int32 index, Int32 count, T data[]) const
{
	if (count > 0)
	{	hsThrowIfNilParam(data);
		hsThrowIfBadParam((UInt32)index >= fCount);

		if (index + count > fCount)
			count = fCount - index;
		for (int i = 0; i < count; i++)
			data[i] = fArray[i + index];
	}
	return count;
}

template <class T>
Int32 hsDynamicArray<T>::Find(const T& obj) const
{
	for (int i = 0; i < fCount; i++)
		if (fArray[i] == obj)
			return i;
	return kMissingIndex;
}

template <class T>
void hsDynamicArray<T>::Remove(Int32 index)
{
	hsThrowIfBadParam((UInt32)index >= (UInt32)fCount);
	
	T rVal = fArray[index];

	if (--fCount > 0)
	{	
		int i;
		T* newList = TRACKED_NEW T[fCount];
		for(i = 0 ; i < index;i++)
			newList[i] = fArray[i];
		for (i = index; i < fCount; i++)
			newList[i] = fArray[i + 1];
		delete [] fArray;
		fArray = newList;
	}
	else
	{	delete[] fArray;
		fArray = nil;
	}
}

template <class T>
Int32 hsDynamicArray<T>::Pop(T *obj)
{
	hsThrowIfBadParam(this->IsEmpty());

	*obj = fArray[0];
	Remove(0);
	return fCount;
}


template <class T>
Int32 hsDynamicArray<T>::Push(const T& obj)
{
	if (fArray)
	{
		T* newList = TRACKED_NEW T[fCount+1];
		for(int i = 0 ; i < fCount; i++)
			newList[i+1] = fArray[i];
		newList[0] = obj;
		delete [] fArray;
		fArray = newList;
	}
	else
	{	hsAssert(fCount == 0, "mismatch");
		fArray = TRACKED_NEW T[1];
		fArray[0] = obj;
	}
	return ++fCount;
}

template <class T>
Int32 hsDynamicArray<T>::Append(const T& obj)
{
	if (fArray)
	{	T* newList = TRACKED_NEW T[fCount + 1];

		for (int i = 0; i < fCount; i++)
			newList[i] = fArray[i];
		newList[fCount] = obj;
		delete [] fArray;
		fArray = newList;
	}
	else
	{	hsAssert(fCount == 0, "mismatch");
		fArray = TRACKED_NEW T[1];
		fArray[0] = obj;
	}
	return ++fCount;
}


template <class T>
Int32 hsDynamicArray<T>::InsertAtIndex(UInt32 index, const T& obj)
{
	if (fArray)
	{	
		hsAssert(UInt32(fCount) >= index, "Index too large for array");
		T* newList = TRACKED_NEW T[fCount + 1];
		unsigned i;
		for ( i = 0; i < index; i++)
			newList[i] = fArray[i];
		newList[index] = obj;
		for ( i = index; i < UInt32(fCount); i++)
			newList[i+1] = fArray[i];
	
		delete [] fArray;
		fArray = newList;
	}
	else
	{	
		hsAssert(fCount == 0, "mismatch");
		hsAssert(index ==0,"Can't insert at non zero index in empty array");
		fArray = TRACKED_NEW T[1];
		fArray[0] = obj;
	}
	return ++fCount;
}

template <class T> void hsDynamicArray<T>::Reset()
{
	if (fArray)
	{	delete[] fArray;
		fArray = nil;
		fCount = 0;
	}
}

template <class T>
hsDynamicArray<T>* hsDynamicArray<T>::Copy(hsDynamicArray<T>* dst) const
{
	if (dst == nil)
		dst = TRACKED_NEW hsDynamicArray<T>;
	else
		dst->Reset();

	dst->SetCount(this->fCount);
	for (int i = 0; i < this->fCount; i++)
		dst->fArray[i] = this->fArray[i];

	return dst;
}

template <class T> T* hsDynamicArray<T>::ForEach(Boolean (*proc)(T&))
{
	for (int i = 0; i < fCount; i++)
		if (proc(fArray[i]))
			return &fArray[i];
	return nil;
}

template <class T> T* hsDynamicArray<T>::ForEach(Boolean (*proc)(T&, void* p1), void * p1)
{
	for (int i = 0; i < fCount; i++)
		if (proc(fArray[i], p1))
			return &fArray[i];
	return nil;
}

template <class T> T* hsDynamicArray<T>::ForEach(Boolean (*proc)(T&, void* p1, void* p2), void *p1, void *p2)
{
	for (int i = 0; i < fCount; i++)
		if (proc(fArray[i], p1, p2))
			return &fArray[i];
	return nil;
}

////////////////////////////////////////////////////////////////////////////////

class hsTArrayBase 
{
protected:
	UInt16		fUseCount;
	UInt16		fTotalCount;

	void GrowArraySize(UInt16 nSize);

#ifdef HS_DEBUGTARRAY
	hsTArrayBase();
	virtual char	*GetTypeName();
	virtual int		 GetSizeOf();
	hsDlistNode *self;
	friend void TArrayStats();
	virtual ~hsTArrayBase();
#else
	hsTArrayBase():fUseCount(0), fTotalCount(0){}
#endif

public:
	UInt16 GetNumAlloc() const { return fTotalCount; }
};


template <class T> class hsTArray :	public hsTArrayBase 
{
	T*		fArray;
	
	inline void	IncCount(int index, int count);
	inline void	DecCount(int index, int count);

#ifdef HS_DEBUGGING
	#define	hsTArray_ValidateCount(count)		hsAssert(((count) >= 0)&&((count) <= 0xffffL), "bad count")
	#define	hsTArray_ValidateIndex(index)		hsAssert(unsigned(index) < fUseCount, "bad index")
	#define	hsTArray_ValidateInsertIndex(index)	hsAssert(unsigned(index) <= fUseCount, "bad index")
	#define	hsTArray_Validate(condition)		hsAssert(condition, "oops")

	#ifdef HS_DEBUGTARRAY
		virtual int      GetSizeOf() { return sizeof(T); }
	#endif
#else
	#define	hsTArray_ValidateCount(count)
	#define	hsTArray_ValidateIndex(index)
	#define	hsTArray_ValidateInsertIndex(index)
	#define	hsTArray_Validate(condition)
#endif
public:
	hsTArray() : fArray(nil) {}
	inline	hsTArray(int count);
	inline	hsTArray(const hsTArray<T>& src);
			~hsTArray() { if (fArray) delete[] fArray; 
			}
	inline void Expand(int NewTotal);

	inline hsTArray<T>&	operator=(const hsTArray<T>& src);
	bool operator==(const hsTArray<T>& src) const;	// checks sizes and contents

	// Swaps the internal data (including the fArray POINTER) with the data from the array given
	void	Swap( hsTArray<T>& src );

	void	Set(int index, const T& item)	{ hsTArray_ValidateIndex(index); fArray[index]=item; }
	const T&	Get(int index) const { hsTArray_ValidateIndex(index); return fArray[index]; }
	T&		operator[](int index) const { hsTArray_ValidateIndex(index); return fArray[index]; }

	T*		FirstIter() { return &fArray[0]; }
	T*		StopIter() { return &fArray[fUseCount]; }

	int		Count() const { return fUseCount; }
	int		GetCount() const { return fUseCount; }
	inline void	SetCount(int count);
	
	inline void SetCountAndZero(int count); // does block clear, don't use for types with vtbl
	inline void ExpandAndZero(int count);   // Same as set count and zero except won't decrease
											// usecount
	inline void	Reset();

	T*		Insert(int index)
			{
				hsTArray_ValidateInsertIndex(index);
				this->IncCount(index, 1);
				return &fArray[index];
			}
	void		Insert(int index, const T& item)
			{
				hsTArray_ValidateInsertIndex(index);
				this->IncCount(index, 1);
				fArray[index] = item;
			}
	void		Insert(int index, int count, T item[])
			{
				hsTArray_ValidateCount(count);
				if (count > 0)
				{	hsTArray_ValidateInsertIndex(index);
					this->IncCount(index, count);
					hsTArray_CopyForward(item, &fArray[index], count);
				}
			}
	// This guy is a duplicate for compatibility with the older hsDynamicArray<>
	void		InsertAtIndex(int index, const T& item) { this->Insert(index, item); }

	void		Remove(int index)
			{
				hsTArray_ValidateIndex(index);
				this->DecCount(index, 1);
			}
	void		Remove(int index, int count)
			{
				hsTArray_ValidateCount(count);
				hsTArray_ValidateIndex(index);
				hsTArray_ValidateIndex(index + count - 1);
				this->DecCount(index, count);
			}
	hsBool	RemoveItem(const T& item);

	T*		Push()
			{
				this->IncCount(fUseCount, 1);
				return &fArray[fUseCount - 1];
			}
	void		Push(const T& item)
			{
				this->IncCount(fUseCount, 1);
				fArray[fUseCount - 1] = item;
			}
	void		Append(const T& item)
			{
				this->IncCount(fUseCount, 1);
				fArray[fUseCount - 1] = item;
			}
	inline T	Pop();
	inline const T&	Peek() const;

	enum {
		kMissingIndex	 = -1
	};
	int	 	Find(const T& item) const;	// returns kMissingIndex if not found
	inline T*	ForEach(hsBool (*proc)(T&));
	inline T*	ForEach(hsBool (*proc)(T&, void* p1), void* p1);
	inline T*	ForEach(hsBool (*proc)(T&, void* p1, void* p2), void* p1, void* p2);

	T*		DetachArray()
			{
				T* array = fArray;
				fUseCount = fTotalCount = 0;
				fArray = nil;
				return array;
			}
	T*		AcquireArray() { return fArray; }
};

//////////////	Public hsTArray methods

template <class T> hsTArray<T>::hsTArray(int count) : fArray(nil)
{
	hsTArray_ValidateCount(count);
	fUseCount = fTotalCount = count;
	if (count > 0)
		fArray = TRACKED_NEW T[count];
}

template <class T> hsTArray<T>::hsTArray(const hsTArray<T>& src) : fArray(nil)
{
	int	count = src.Count();
	fUseCount = fTotalCount = count;

	if (count > 0)
	{	
		fArray = TRACKED_NEW T[count];
		hsTArray_CopyForward(src.fArray, fArray, count);
	}
}

template <class T> hsTArray<T>& hsTArray<T>::operator=(const hsTArray<T>& src)
{
	if (this->Count() != src.Count())
		this->SetCount(src.Count());
	hsTArray_CopyForward(src.fArray, fArray, src.Count());
	return *this;
}

// checks sizes and contents
template <class T> 
bool hsTArray<T>::operator==(const hsTArray<T>& src) const
{
	if (&src==this)
		return true;	// it's me

	if (GetCount() != src.GetCount())
		return false;	// different sizes

	int i;
	for(i=0;i<GetCount();i++)
		if (Get(i) != src[i])
			return false;	// different contents

	return true;		// the same
}


//// Swap ////////////////////////////////////////////////////////////////////
//	Added 5.2.2001 mcn - Given another hsTArray of the same type, "swaps" the
//	data stored in both. Basically we're literally swapping the fArray pointers
//	around, plus the use counts and such.

template <class T> void hsTArray<T>::Swap( hsTArray<T>& src )
{
	UInt16		use, tot;
	T			*array;


	use = fUseCount;
	tot = fTotalCount;
	array = fArray;

	fUseCount = src.fUseCount;
	fTotalCount = src.fTotalCount;
	fArray = src.fArray;

	src.fUseCount = use;
	src.fTotalCount = tot;
	src.fArray = array;
}

template <class T> void hsTArray<T>::SetCountAndZero(int count)
{
	if( fTotalCount <= count )
	{
		int n = fTotalCount;
		Expand(count);
	}
	int i;
	for( i = 0; i < fTotalCount; i++ )
		fArray[i] = nil;
	fUseCount = count;
}

template <class T> void hsTArray<T>::ExpandAndZero(int count)
{
	if( fTotalCount <= count )
	{
		int n = fTotalCount;
		Expand(count);
		int i;
		for( i = n; i < count; i++ )
			fArray[i] = nil;
	}
	if( fUseCount < count )
		fUseCount = count;
}

template <class T> void hsTArray<T>::SetCount(int count)
{
	hsTArray_ValidateCount(count);
	if (count > fTotalCount)
	{	
		if (fArray)
			delete[] fArray;
		fArray = TRACKED_NEW T[count];
		fUseCount = fTotalCount = count;
	}
	fUseCount = count;
}

template <class T> void hsTArray<T>::Expand(int NewCount) // New Count is Absolute not additional
{
	hsTArray_ValidateCount(NewCount);
	if (NewCount > fTotalCount)			// This is Expand not Shrink
	{	
		T*	newArray = TRACKED_NEW T[NewCount];

		if (fArray != nil)
		{	hsTArray_CopyForward(fArray, newArray, fUseCount);
//			hsTArray_CopyForward(&fArray[index], &newArray[index + count], fUseCount - index);
			delete[] fArray;
		}
		fArray = newArray;
		fTotalCount = NewCount;
	}
}

template <class T> void hsTArray<T>::Reset()
{
	if (fArray)
	{	
		delete[] fArray;
		fArray = nil;
		fUseCount = fTotalCount = 0;
	}
}

template <class T> T hsTArray<T>::Pop()
{
	hsTArray_Validate(fUseCount > 0);
	fUseCount -= 1;
	return fArray[fUseCount];
}

template <class T> const T& hsTArray<T>::Peek() const
{
	hsTArray_Validate(fUseCount > 0);
	return fArray[fUseCount-1];
}

template <class T> int hsTArray<T>::Find(const T& item) const
{
	for (int i = 0; i < fUseCount; i++)
		if (fArray[i] == item)
			return i;
	return kMissingIndex;
}

template <class T> hsBool hsTArray<T>::RemoveItem(const T& item)
{
	for (int i = 0; i < fUseCount; i++)
		if (fArray[i] == item)
		{	this->DecCount(i, 1);
			return true;
		}
	return false;
}

//////////	These are the private methods for hsTArray

template <class T> void hsTArray_CopyForward(const T src[], T dst[], int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i];
}

template <class T> void hsTArray_CopyBackward(const T src[], T dst[], int count)
{
	for (int i = count - 1; i >= 0; --i)
		dst[i] = src[i];
}

template <class T> void hsTArray<T>::IncCount(int index, int count)
{
	int	newCount = fUseCount + count;

	if (newCount > fTotalCount)
	{	if (fTotalCount == 0)
			fTotalCount = newCount;

		GrowArraySize(newCount);	// Sets new fTotalCount
		T*	newArray = TRACKED_NEW T[fTotalCount];

		if (fArray != nil)
		{	hsTArray_CopyForward(fArray, newArray, index);
			hsTArray_CopyForward(&fArray[index], &newArray[index + count], fUseCount - index);
			delete[] fArray;
		}
		fArray = newArray;
	}
	else
		hsTArray_CopyBackward(&fArray[index], &fArray[index + count], fUseCount - index);
	fUseCount = newCount;
}

template <class T> void hsTArray<T>::DecCount(int index, int count)
{
	if (fUseCount == count)
		this->Reset();
	else
	{	hsTArray_CopyForward(&fArray[index + count], &fArray[index], fUseCount - index - count);
		fUseCount -= count;
	}
}

template <class T> T* hsTArray<T>::ForEach(hsBool (*proc)(T&))
{
	for (int i = 0; i < fUseCount; i++)
		if (proc(fArray[i]))
			return &fArray[i];
	return nil;
}

template <class T> T* hsTArray<T>::ForEach(hsBool (*proc)(T&, void* p1), void* p1)
{
	for (int i = 0; i < fUseCount; i++)
		if (proc(fArray[i], p1))
			return &fArray[i];
	return nil;
}

template <class T> T* hsTArray<T>::ForEach(hsBool (*proc)(T&, void* p1, void* p2), void* p1, void* p2)
{
	for (int i = 0; i < fUseCount; i++)
		if (proc(fArray[i], p1, p2))
			return &fArray[i];
	return nil;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// hsTArray's big brother. Only to be used when expecting more than 64K of elements. The
// only difference between hsTArray and hsLargeArray is LargeArray uses 32 bit counters,
// vs 16 bit counters for hsTArray.

class hsLargeArrayBase 
{
protected:
	UInt32		fUseCount;
	UInt32		fTotalCount;

	void GrowArraySize(UInt32 nSize);

#ifdef HS_DEBUGTARRAY
	hsLargeArrayBase();
	virtual char	*GetTypeName();
	virtual int		 GetSizeOf();
	hsDlistNode *self;
	friend void LargeArrayStats();
	virtual ~hsLargeArrayBase();
#else
	hsLargeArrayBase():fUseCount(0), fTotalCount(0){}
#endif

public:
	UInt32 GetNumAlloc() const { return fTotalCount; }
};


template <class T> class hsLargeArray :	public hsLargeArrayBase 
{
	T*		fArray;
	
	inline void	IncCount(int index, int count);
	inline void	DecCount(int index, int count);

#ifdef HS_DEBUGGING
	#define	hsLargeArray_ValidateCount(count)		hsAssert((count) >= 0, "bad count")
	#define	hsLargeArray_ValidateIndex(index)		hsAssert(unsigned(index) < fUseCount, "bad index")
	#define	hsLargeArray_ValidateInsertIndex(index)	hsAssert(unsigned(index) <= fUseCount, "bad index")
	#define	hsLargeArray_Validate(condition)		hsAssert(condition, "oops")

	#ifdef HS_DEBUGTARRAY
		virtual int      GetSizeOf() { return sizeof(T); }
	#endif
#else
	#define	hsLargeArray_ValidateCount(count)
	#define	hsLargeArray_ValidateIndex(index)
	#define	hsLargeArray_ValidateInsertIndex(index)
	#define	hsLargeArray_Validate(condition)
#endif
public:
	hsLargeArray() : fArray(nil) {}
	inline	hsLargeArray(int count);
	inline	hsLargeArray(const hsLargeArray<T>& src);
			~hsLargeArray() { if (fArray) delete[] fArray; 
			}
	inline void Expand(int NewTotal);

	inline hsLargeArray<T>&	operator=(const hsLargeArray<T>& src);

	// Swaps the internal data (including the fArray POINTER) with the data from the array given
	void	Swap( hsLargeArray<T>& src );

	void	Set(int index, const T& item)	{ hsLargeArray_ValidateIndex(index); fArray[index]=item; }
	const T&	Get(int index) const { hsLargeArray_ValidateIndex(index); return fArray[index]; }
	T&		operator[](int index) const { hsLargeArray_ValidateIndex(index); return fArray[index]; }

	T*		FirstIter() { return &fArray[0]; }
	T*		StopIter() { return &fArray[fUseCount]; }

	int		Count() const { return fUseCount; }
	int		GetCount() const { return fUseCount; }
	inline void	SetCount(int count);
	
	inline void SetCountAndZero(int count); // does block clear, don't use for types with vtbl
	inline void ExpandAndZero(int count);   // Same as set count and zero except won't decrease
											// usecount
	inline void	Reset();

	T*		Insert(int index)
			{
				hsLargeArray_ValidateInsertIndex(index);
				this->IncCount(index, 1);
				return &fArray[index];
			}
	void		Insert(int index, const T& item)
			{
				hsLargeArray_ValidateInsertIndex(index);
				this->IncCount(index, 1);
				fArray[index] = item;
			}
	void		Insert(int index, int count, T item[])
			{
				hsLargeArray_ValidateCount(count);
				if (count > 0)
				{	hsLargeArray_ValidateInsertIndex(index);
					this->IncCount(index, count);
					hsLargeArray_CopyForward(item, &fArray[index], count);
				}
			}
	// This guy is a duplicate for compatibility with the older hsDynamicArray<>
	void		InsertAtIndex(int index, const T& item) { this->Insert(index, item); }

	void		Remove(int index)
			{
				hsLargeArray_ValidateIndex(index);
				this->DecCount(index, 1);
			}
	void		Remove(int index, int count)
			{
				hsLargeArray_ValidateCount(count);
				hsLargeArray_ValidateIndex(index);
				hsLargeArray_ValidateIndex(index + count - 1);
				this->DecCount(index, count);
			}
	hsBool	RemoveItem(const T& item);

	T*		Push()
			{
				this->IncCount(fUseCount, 1);
				return &fArray[fUseCount - 1];
			}
	void		Push(const T& item)
			{
				this->IncCount(fUseCount, 1);
				fArray[fUseCount - 1] = item;
			}
	void		Append(const T& item)
			{
				this->IncCount(fUseCount, 1);
				fArray[fUseCount - 1] = item;
			}
	inline T	Pop();
	inline const T&	Peek() const;

	enum {
		kMissingIndex	 = -1
	};
	int	 	Find(const T& item) const;	// returns kMissingIndex if not found
	inline T*	ForEach(hsBool (*proc)(T&));
	inline T*	ForEach(hsBool (*proc)(T&, void* p1), void* p1);
	inline T*	ForEach(hsBool (*proc)(T&, void* p1, void* p2), void* p1, void* p2);

	T*		DetachArray()
			{
				T* array = fArray;
				fUseCount = fTotalCount = 0;
				fArray = nil;
				return array;
			}
	T*		AcquireArray() { return fArray; }
};

//////////////	Public hsLargeArray methods

template <class T> hsLargeArray<T>::hsLargeArray(int count) : fArray(nil)
{
	hsLargeArray_ValidateCount(count);
	fUseCount = fTotalCount = count;
	if (count > 0)
		fArray = TRACKED_NEW T[count];
}

template <class T> hsLargeArray<T>::hsLargeArray(const hsLargeArray<T>& src) : fArray(nil)
{
	int	count = src.Count();
	fUseCount = fTotalCount = count;

	if (count > 0)
	{	
		fArray = TRACKED_NEW T[count];
		hsLargeArray_CopyForward(src.fArray, fArray, count);
	}
}

template <class T> hsLargeArray<T>& hsLargeArray<T>::operator=(const hsLargeArray<T>& src)
{
	if (this->Count() != src.Count())
		this->SetCount(src.Count());
	hsLargeArray_CopyForward(src.fArray, fArray, src.Count());
	return *this;
}

template <class T> void hsLargeArray<T>::Swap( hsLargeArray<T>& src )
{
	UInt32		use, tot;
	T			*array;


	use = fUseCount;
	tot = fTotalCount;
	array = fArray;

	fUseCount = src.fUseCount;
	fTotalCount = src.fTotalCount;
	fArray = src.fArray;

	src.fUseCount = use;
	src.fTotalCount = tot;
	src.fArray = array;
}

template <class T> void hsLargeArray<T>::SetCountAndZero(int count)
{
	if( fTotalCount <= count )
	{
		int n = fTotalCount;
		Expand(count);
	}
	HSMemory::Clear(fArray, fTotalCount * sizeof( T ));
	fUseCount = count;
}

template <class T> void hsLargeArray<T>::ExpandAndZero(int count)
{
	if( fTotalCount <= count )
	{
		int n = fTotalCount;
		Expand(count);
		HSMemory::Clear(fArray+n, (count - n) * sizeof( T ));
	}
	if( fUseCount < count )
		fUseCount = count;
}

template <class T> void hsLargeArray<T>::SetCount(int count)
{
	hsLargeArray_ValidateCount(count);
	if (count > fTotalCount)
	{	
		if (fArray)
			delete[] fArray;
		fArray = TRACKED_NEW T[count];
		fUseCount = fTotalCount = count;
	}
	fUseCount = count;
}

template <class T> void hsLargeArray<T>::Expand(int NewCount) // New Count is Absolute not additional
{
	hsLargeArray_ValidateCount(NewCount);
	if (NewCount > fTotalCount)			// This is Expand not Shrink
	{	
		T*	newArray = TRACKED_NEW T[NewCount];

		if (fArray != nil)
		{	hsLargeArray_CopyForward(fArray, newArray, fUseCount);
//			hsLargeArray_CopyForward(&fArray[index], &newArray[index + count], fUseCount - index);
			delete[] fArray;
		}
		fArray = newArray;
		fTotalCount = NewCount;
	}
}

template <class T> void hsLargeArray<T>::Reset()
{
	if (fArray)
	{	
		delete[] fArray;
		fArray = nil;
		fUseCount = fTotalCount = 0;
	}
}

template <class T> T hsLargeArray<T>::Pop()
{
	hsLargeArray_Validate(fUseCount > 0);
	fUseCount -= 1;
	return fArray[fUseCount];
}

template <class T> const T& hsLargeArray<T>::Peek() const
{
	hsLargeArray_Validate(fUseCount > 0);
	return fArray[fUseCount-1];
}

template <class T> int hsLargeArray<T>::Find(const T& item) const
{
	for (int i = 0; i < fUseCount; i++)
		if (fArray[i] == item)
			return i;
	return kMissingIndex;
}

template <class T> hsBool hsLargeArray<T>::RemoveItem(const T& item)
{
	for (int i = 0; i < fUseCount; i++)
		if (fArray[i] == item)
		{	this->DecCount(i, 1);
			return true;
		}
	return false;
}

//////////	These are the private methods for hsLargeArray

template <class T> void hsLargeArray_CopyForward(const T src[], T dst[], int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = src[i];
}

template <class T> void hsLargeArray_CopyBackward(const T src[], T dst[], int count)
{
	for (int i = count - 1; i >= 0; --i)
		dst[i] = src[i];
}

template <class T> void hsLargeArray<T>::IncCount(int index, int count)
{
	int	newCount = fUseCount + count;

	if (newCount > fTotalCount)
	{	if (fTotalCount == 0)
			fTotalCount = newCount;

		GrowArraySize(newCount);	// Sets new fTotalCount
		T*	newArray = TRACKED_NEW T[fTotalCount];

		if (fArray != nil)
		{	hsLargeArray_CopyForward(fArray, newArray, index);
			hsLargeArray_CopyForward(&fArray[index], &newArray[index + count], fUseCount - index);
			delete[] fArray;
		}
		fArray = newArray;
	}
	else
		hsLargeArray_CopyBackward(&fArray[index], &fArray[index + count], fUseCount - index);
	fUseCount = newCount;
}

template <class T> void hsLargeArray<T>::DecCount(int index, int count)
{
	if (fUseCount == count)
		this->Reset();
	else
	{	hsLargeArray_CopyForward(&fArray[index + count], &fArray[index], fUseCount - index - count);
		fUseCount -= count;
	}
}

template <class T> T* hsLargeArray<T>::ForEach(hsBool (*proc)(T&))
{
	for (int i = 0; i < fUseCount; i++)
		if (proc(fArray[i]))
			return &fArray[i];
	return nil;
}

template <class T> T* hsLargeArray<T>::ForEach(hsBool (*proc)(T&, void* p1), void* p1)
{
	for (int i = 0; i < fUseCount; i++)
		if (proc(fArray[i], p1))
			return &fArray[i];
	return nil;
}

template <class T> T* hsLargeArray<T>::ForEach(hsBool (*proc)(T&, void* p1, void* p2), void* p1, void* p2)
{
	for (int i = 0; i < fUseCount; i++)
		if (proc(fArray[i], p1, p2))
			return &fArray[i];
	return nil;
}



#endif

