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
#ifndef PL_SYNCHEDVALUE_inc
#define PL_SYNCHEDVALUE_inc

#include "plSynchedObject.h"

#ifdef USE_SYNCHED_VALUES
#include "hsTypes.h"
#include "hsStream.h"

#include "hsBitVector.h"
#include "hsMemory.h"
//
// Defines a class for variable types which need to automatically be synchronized
// (replicated) over the network.
// current size is 8 bytes
//

//
// -----------------------------------------------------
// SYNCHED VALUE BASE CLASS
// -----------------------------------------------------
//
class hsResMgr;
class plSceneNode;
class plSceneObject;
class plCoordinateInterface;
class plSynchedValueBase
{
public:
	enum Flags	// 16 bits
	{
		kValueIsDirty		= 0x1,		
		kValueSendOnlyOnce	= 0x2,		// perm flag
		kValueHasBeenSent	= 0x4,		// perm flag
		kRegistered			= 0x8,		// perm flag
		kDontDirty			= 0x10		// perm flag
	};

protected:
	Int16	fSynchedObjectAddrOffset;	// this could represent dwords instead of byte offsets
	UInt16	fFlags;		

	void IConstruct()	// too bad this can't be virtual (because it's called from ctor)
	{ 
		// The synchMgr for the class that owns us is constructed first and the staticMgr
		// is set to his address so we can automatically get at it during construction.
		fFlags=0;
		Int32 off = (Int32)plSynchedObject::GetStaticSynchedObject() - (Int32)this;	
		if ( hsABS(off) <  (1<<(sizeof(fSynchedObjectAddrOffset)<<3)) )
			fSynchedObjectAddrOffset = (Int16)off;
		else
			fSynchedObjectAddrOffset=-1;
	}

	hsBool32	IOKToDirty() 
	{ 
		if (fFlags & (kDontDirty | kValueIsDirty))
			return false;
		return GetSynchedObject() ? GetSynchedObject()->IOKToDirty() : false; 
	}
	virtual void ISaveOrLoad(hsBool32 save, hsStream* stream, hsResMgr* mgr) = 0;

	// save/load methods for different types
	const plKey ISaveOrLoad(const plKey key, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	hsKeyedObject* ISaveOrLoad(hsKeyedObject* obj, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	plSceneNode* ISaveOrLoad(plSceneNode* obj, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	plSceneObject* ISaveOrLoad(plSceneObject* obj, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	Int32 ISaveOrLoad(Int32 v, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	UInt32 ISaveOrLoad(UInt32 v, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	bool ISaveOrLoad(bool v, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	int ISaveOrLoad(int v, hsBool32 save, hsStream* stream, hsResMgr* mgr);		// or hsBool32
	hsScalar ISaveOrLoad(hsScalar v, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	double ISaveOrLoad(double v, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	hsBitVector ISaveOrLoad(hsBitVector& v, hsBool32 save, hsStream* stream, hsResMgr* mgr);
	plCoordinateInterface* ISaveOrLoad(const plCoordinateInterface* cInt, hsBool32 save, hsStream* stream, hsResMgr* mgr);
public:		
	plSynchedValueBase()  { IConstruct();  }
	virtual ~plSynchedValueBase() {}

	// getters
	virtual plSynchedObject* GetSynchedObject() 
	{ 
		hsAssert(fSynchedObjectAddrOffset!=-1, "invalid synchedObject address offset");
		plSynchedObject* so = fSynchedObjectAddrOffset == -1 ? nil : (plSynchedObject*)((Int32)this+fSynchedObjectAddrOffset); 
		if (!(fFlags & kRegistered) && so)
		{
			so->RegisterSynchedValue(this);
			fFlags |= kRegistered;
		}
		return so;
	}
	UInt16 GetFlags() { return fFlags; }
	
	// setters
	void SetFlags(UInt16 f) { fFlags=f; }
	
	void MakeDirty() { SetFlags(GetFlags() | kValueIsDirty); }
	void MakeClean() { SetFlags(GetFlags() & ~kValueIsDirty); }

	void DirtyIfNecessary() 
	{ 
		if (IOKToDirty()) 	
		{
			MakeDirty();						// dirty value
			if (GetSynchedObject())
				GetSynchedObject()->DirtySynchState(nil, 0);	// dirty owner
		}
	}

	// save/load
	static void Save(plSynchedValueBase& obj, hsStream* stream, hsResMgr* mgr) { obj.ISaveOrLoad(true, stream, mgr); }
	static void Load(plSynchedValueBase& obj, hsStream* stream, hsResMgr* mgr) { obj.ISaveOrLoad(false, stream, mgr); }
};

//
// -----------------------------------
// SYNCHED VALUE TEMPLATE
// -----------------------------------
//
template <class T> 
class plSynchedValue : public plSynchedValueBase
{
protected:
	T fValue;

	void ISaveOrLoad(hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	{ fValue=(T)plSynchedValueBase::ISaveOrLoad(fValue, save, stream, mgr); }		// default method

public:	

	plSynchedValue() {}
	plSynchedValue(const T& v) : plSynchedValueBase() { fValue=v; }
	plSynchedValue(const plSynchedValue<T>& pRHS) : plSynchedValueBase() { fValue=pRHS.GetValue(); }	// copy ctor

	// conversion operators
    operator T() const { return fValue; }
	T* operator &() const { return &fValue; }
	T* operator &() { return &fValue; }

	// equality
	hsBool32 operator==(const T& other) const	{ return fValue==(T)other; }
	hsBool32 operator!=(const T& other) const	{ return !(*this == other); }

	// other operators
	T operator++()		{ DirtyIfNecessary(); return ++fValue; }
	T operator++(int)	{ DirtyIfNecessary(); return fValue++; }	// postfix
	T operator--()		{ DirtyIfNecessary(); return --fValue; }
	T operator--(int)	{ DirtyIfNecessary(); return fValue--; }	// postfix
	T operator+=(const T& other) { return SetValue(fValue+other); }
	T operator*=(const T& other) { return SetValue(fValue*other);  }
	T operator/=(const T& other) { return SetValue(fValue/other);  }	

	// these return reference in the event they are bitvector types
	T& operator&=(const T& other) { return SetValue(fValue&other); }
	T& operator|=(const T& other) { return SetValue(fValue|other); }
	T& operator^=(const T& other) { return SetValue(fValue^other); }
	T& operator-=(const T& other) { return SetValue(fValue-other); }

	const T& operator=(const T& v){	return SetValue(v); }

#if HS_BUILD_FOR_WIN32
#pragma warning( push )
#pragma warning( disable : 4284 )	// disable annoying warnings in release build for non pointer types
#endif
	// for pointer types, which are allowed to change the object pointed to
    T operator->(void) { return fValue; }
#if HS_BUILD_FOR_WIN32
#pragma warning( pop ) 
#endif    
	// asignment, can use instead of setvalue
	const T& operator=(const plSynchedValue<T>& pRHS ) 	{ return SetValue(pRHS.GetValue()); }

	// setters
	T&	SetValue(const T& v)	// return true if changed value
	{
		if (v != fValue)	// dont dirty unless value changes
		{
			fValue=v;
			DirtyIfNecessary();
		}
		return fValue;
	}

	// getters
	const T&	GetValue() const { return fValue; }

	// for hsBitVector
	hsBool32 IsBitSet(UInt32 which) const {	return fValue.IsBitSet(which); }
	hsBool32 SetBit(UInt32 which, hsBool32 on = true)
	{	hsBool32 bitSet = IsBitSet(which);
		if ( (on && !bitSet) || (!on && bitSet) )
			DirtyIfNecessary();
		return fValue.SetBit(which, on); 
	}
	void Read(hsStream* s) { fValue.Read(s); }
	void Write(hsStream* s) const { fValue.Write(s); }
	void Clear()	{ DirtyIfNecessary();	fValue.Clear(); }
	hsBool32 ClearBit(UInt32 which) { 	if (fValue.IsBitSet(which)) DirtyIfNecessary();	return fValue.ClearBit(which); }
	void Reset() { if (fValue.GetSize()!=0) DirtyIfNecessary();	fValue.Reset(); }
	hsBool32 ToggleBit(UInt32 which) { 	DirtyIfNecessary();	return fValue.ToggleBit(which); }
	UInt32 GetSize() { return fValue.GetSize();	}
};

//////////////////////////////////////
// Synched Value Friend - allows a synched value to be contained
// in an object which is not a synchedObject.  Uses a pointer instead
// of an computer the addr offset of it's associated synchedObject.
// This one is 4 bytes bigger than regular synched values.
//////////////////////////////////////
template <class T> 
class plSynchedValueFriend : public plSynchedValue<T>
{
protected:
	plSynchedObject** fSynchedObject;
public:
	plSynchedValueFriend() : fSynchedObject(nil) { }
	// this is explicit so it won't be invoked instead of operator()=
	explicit plSynchedValueFriend(const T& v) : plSynchedValue<T>(v),fSynchedObject(nil)  { }
	plSynchedValueFriend(const plSynchedValueFriend<T>& pRHS) : plSynchedValue<T>(pRHS) 
	{ fSynchedObject = pRHS.fSynchedObject; }
	~plSynchedValueFriend() 
	{
		if (GetSynchedObject())
			GetSynchedObject()->RemoveSynchedValue(this);
	}
	
	// this isn't inherited for some reason
	const T& operator=(const T& v) { return SetValue(v); }

	plSynchedObject* GetSynchedObject() 
	{ 
		hsAssert(fSynchedObject, "nil synched object, need to SetSynchedObjectPtrAddr?"); 

		if (*fSynchedObject && !(fFlags & kRegistered))
		{
			(*fSynchedObject)->RegisterSynchedValueFriend(this);
			fFlags |= kRegistered;
		}
		return *fSynchedObject; 
	}

	void SetSynchedObjectPtrAddr(plSynchedObject** so) 
	{ 
		hsAssert(!(fFlags & kRegistered), "SynchedValueFriend already registered?");
		fSynchedObject=so; 
	}
};

/////////////////////////////////////
// Synched TArray Template
/////////////////////////////////////
#include "hsTemplates.h"
template <class T> 
class plSynchedTArray : public plSynchedValueBase
{
private:
	hsTArray<T> fValueList;

	void ISaveOrLoad(hsBool32 save, hsStream* stream, hsResMgr* mgr); 
public:
	enum {	kMissingIndex = hsTArray<T>::kMissingIndex	};
	plSynchedTArray() {}
	~plSynchedTArray() {}

	// conversion operators
    operator T() const { return fValueList; }

	// common methods
    const T& operator[](int i) const { return Get(i); }

    const T& Get(int i) const { return fValueList.Get(i); }
	void	Set(int i, const T& item)	{	if (fValueList[i] != item) DirtyIfNecessary(); fValueList[i]=item;	}
	void	Append(const T& item)	{	fValueList.Append(item);	DirtyIfNecessary();	}
	T*		Insert(int index)	{	fValueList.Insert(index);	DirtyIfNecessary();	}
	void	Remove(int index)	{	fValueList.Remove(index);	DirtyIfNecessary();	}
	int		Count() const { return fValueList.Count(); }
	int		GetCount() const { return Count(); }
	void	Reset()	{	if (fValueList.GetCount() != 0) DirtyIfNecessary(); fValueList.Reset();		}
	void	SetCountAndZero(int count)	{	if (count || GetCount()) DirtyIfNecessary(); fValueList.SetCountAndZero(count);	}
	void	SetCount(int count)			{	if (count || GetCount()) DirtyIfNecessary(); fValueList.SetCount(count);	}
	void	ExpandAndZero(int count)	{	if (count || GetCount()) DirtyIfNecessary(); fValueList.ExpandAndZero(count);	}
	int	 	Find(const T& item) const	{	return fValueList.Find(item);	}
	T*		Push()	{	DirtyIfNecessary();	return fValueList.Push();	}
	void	Push(const T& item)	{	DirtyIfNecessary();	return fValueList.Push(item);	}
	T	Pop()	{	DirtyIfNecessary();	return fValueList.Pop();	}
	const T&	Peek() const { return fValue.Peek(); }
	T*		DetachArray()	{	DirtyIfNecessary();	return fValueList.DetachArray();	}
	T*		AcquireArray() 	{	DirtyIfNecessary();	return fValueList.AcquireArray();	}
};

//
// inlines
//
template <class T> inline
void plSynchedTArray<T>::ISaveOrLoad(hsBool32 save, hsStream* stream, hsResMgr* mgr)
{
	if (save)
	{
		// write out size of array
		Int32 i, num = fValueList.GetCount();
		stream->WriteSwap(num);
		for(i=0;i<num;i++)
		{
			plSynchedValueBase::ISaveOrLoad(fValueList[i], save, stream, mgr);
		}
	}
	else
	{
		// clear array
		fValueList.Reset();
		// read in size of array
		Int32 i, num;
		stream->ReadSwap(&num);

		for(i=0;i<num;i++)
		{
			T v;
			HSMemory::ClearMemory(&v, sizeof(v));
			v=(T)plSynchedValueBase::ISaveOrLoad(v, save, stream, mgr);
			fValueList.Append(v);
		}
	}
}


//////////////////////////////////////
// Variation on synchedTArray.  See plSynchedValueFriend above for more info
//////////////////////////////////////
template <class T> 
class plSynchedTArrayFriend : public plSynchedTArray<T>
{
protected:
	plSynchedObject** fSynchedObject;
public:
	plSynchedTArrayFriend() : fSynchedObject(nil) { }

	plSynchedObject* GetSynchedObject() 
	{ 
		hsAssert(fSynchedObject, "nil synched object, need to SetSynchedObjectPtrAddr?"); 

		if (*fSynchedObject && !(fFlags & kRegistered))
		{
			(*fSynchedObject)->RegisterSynchedValueFriend(this);
			fFlags |= kRegistered;
		}
		return *fSynchedObject; 
	}

	void SetSynchedObjectPtrAddr(plSynchedObject** so) 
	{ 
		hsAssert(!(fFlags & kRegistered), "SynchedValueTArrayFriend already registered?");
		fSynchedObject=so; 
	}


#if 0
	//
	// redefine operators since they are not inherited
	//

	// conversion operators
    operator T() const { return fValueList; }

	// common methods
    const T& operator[](int i) const { return Get(i); }
#endif
};
#endif	// USE_SYNCHED_VALUES

#endif	// PL_SYNCHEDVALUE_inc

