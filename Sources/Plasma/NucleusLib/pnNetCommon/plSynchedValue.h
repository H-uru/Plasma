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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
#include "HeadSpin.h"
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
    enum Flags  // 16 bits
    {
        kValueIsDirty       = 0x1,      
        kValueSendOnlyOnce  = 0x2,      // perm flag
        kValueHasBeenSent   = 0x4,      // perm flag
        kRegistered         = 0x8,      // perm flag
        kDontDirty          = 0x10      // perm flag
    };

protected:
    int16_t   fSynchedObjectAddrOffset;   // this could represent uint32_ts instead of uint8_t offsets
    uint16_t  fFlags;     

    void IConstruct()   // too bad this can't be virtual (because it's called from ctor)
    { 
        // The synchMgr for the class that owns us is constructed first and the staticMgr
        // is set to his address so we can automatically get at it during construction.
        fFlags=0;
        int32_t off = (int32_t)plSynchedObject::GetStaticSynchedObject() - (int32_t)this; 
        if ( abs(off) <  (1<<(sizeof(fSynchedObjectAddrOffset)<<3)) )
            fSynchedObjectAddrOffset = (int16_t)off;
        else
            fSynchedObjectAddrOffset=-1;
    }

    bool32    IOKToDirty() 
    { 
        if (fFlags & (kDontDirty | kValueIsDirty))
            return false;
        return GetSynchedObject() ? GetSynchedObject()->IOKToDirty() : false; 
    }
    virtual void ISaveOrLoad(bool32 save, hsStream* stream, hsResMgr* mgr) = 0;

    // save/load methods for different types
    const plKey ISaveOrLoad(const plKey key, bool32 save, hsStream* stream, hsResMgr* mgr);
    hsKeyedObject* ISaveOrLoad(hsKeyedObject* obj, bool32 save, hsStream* stream, hsResMgr* mgr);
    plSceneNode* ISaveOrLoad(plSceneNode* obj, bool32 save, hsStream* stream, hsResMgr* mgr);
    plSceneObject* ISaveOrLoad(plSceneObject* obj, bool32 save, hsStream* stream, hsResMgr* mgr);
    int32_t ISaveOrLoad(int32_t v, bool32 save, hsStream* stream, hsResMgr* mgr);
    uint32_t ISaveOrLoad(uint32_t v, bool32 save, hsStream* stream, hsResMgr* mgr);
    bool ISaveOrLoad(bool v, bool32 save, hsStream* stream, hsResMgr* mgr);
    int ISaveOrLoad(int v, bool32 save, hsStream* stream, hsResMgr* mgr);     // or bool32
    float ISaveOrLoad(float v, bool32 save, hsStream* stream, hsResMgr* mgr);
    double ISaveOrLoad(double v, bool32 save, hsStream* stream, hsResMgr* mgr);
    hsBitVector ISaveOrLoad(hsBitVector& v, bool32 save, hsStream* stream, hsResMgr* mgr);
    plCoordinateInterface* ISaveOrLoad(const plCoordinateInterface* cInt, bool32 save, hsStream* stream, hsResMgr* mgr);
public:     
    plSynchedValueBase()  { IConstruct();  }
    virtual ~plSynchedValueBase() {}

    // getters
    virtual plSynchedObject* GetSynchedObject() 
    { 
        hsAssert(fSynchedObjectAddrOffset!=-1, "invalid synchedObject address offset");
        plSynchedObject* so = fSynchedObjectAddrOffset == -1 ? nil : (plSynchedObject*)((int32_t)this+fSynchedObjectAddrOffset); 
        if (!(fFlags & kRegistered) && so)
        {
            so->RegisterSynchedValue(this);
            fFlags |= kRegistered;
        }
        return so;
    }
    uint16_t GetFlags() { return fFlags; }
    
    // setters
    void SetFlags(uint16_t f) { fFlags=f; }
    
    void MakeDirty() { SetFlags(GetFlags() | kValueIsDirty); }
    void MakeClean() { SetFlags(GetFlags() & ~kValueIsDirty); }

    void DirtyIfNecessary() 
    { 
        if (IOKToDirty())   
        {
            MakeDirty();                        // dirty value
            if (GetSynchedObject())
                GetSynchedObject()->DirtySynchState(plString::Null, 0);    // dirty owner
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

    void ISaveOrLoad(bool32 save, hsStream* stream, hsResMgr* mgr) 
    { fValue=(T)plSynchedValueBase::ISaveOrLoad(fValue, save, stream, mgr); }       // default method

public: 

    plSynchedValue() {}
    plSynchedValue(const T& v) : plSynchedValueBase() { fValue=v; }
    plSynchedValue(const plSynchedValue<T>& pRHS) : plSynchedValueBase() { fValue=pRHS.GetValue(); }    // copy ctor

    // conversion operators
    operator T() const { return fValue; }
    T* operator &() const { return &fValue; }
    T* operator &() { return &fValue; }

    // equality
    bool32 operator==(const T& other) const   { return fValue==(T)other; }
    bool32 operator!=(const T& other) const   { return !(*this == other); }

    // other operators
    T operator++()      { DirtyIfNecessary(); return ++fValue; }
    T operator++(int)   { DirtyIfNecessary(); return fValue++; }    // postfix
    T operator--()      { DirtyIfNecessary(); return --fValue; }
    T operator--(int)   { DirtyIfNecessary(); return fValue--; }    // postfix
    T operator+=(const T& other) { return SetValue(fValue+other); }
    T operator*=(const T& other) { return SetValue(fValue*other);  }
    T operator/=(const T& other) { return SetValue(fValue/other);  }    

    // these return reference in the event they are bitvector types
    T& operator&=(const T& other) { return SetValue(fValue&other); }
    T& operator|=(const T& other) { return SetValue(fValue|other); }
    T& operator^=(const T& other) { return SetValue(fValue^other); }
    T& operator-=(const T& other) { return SetValue(fValue-other); }

    const T& operator=(const T& v){ return SetValue(v); }

#if HS_BUILD_FOR_WIN32
#pragma warning( push )
#pragma warning( disable : 4284 )   // disable annoying warnings in release build for non pointer types
#endif
    // for pointer types, which are allowed to change the object pointed to
    T operator->(void) { return fValue; }
#if HS_BUILD_FOR_WIN32
#pragma warning( pop ) 
#endif    
    // asignment, can use instead of setvalue
    const T& operator=(const plSynchedValue<T>& pRHS )  { return SetValue(pRHS.GetValue()); }

    // setters
    T&  SetValue(const T& v)    // return true if changed value
    {
        if (v != fValue)    // dont dirty unless value changes
        {
            fValue=v;
            DirtyIfNecessary();
        }
        return fValue;
    }

    // getters
    const T&    GetValue() const { return fValue; }

    // for hsBitVector
    bool32 IsBitSet(uint32_t which) const { return fValue.IsBitSet(which); }
    bool32 SetBit(uint32_t which, bool32 on = true)
    {   bool32 bitSet = IsBitSet(which);
        if ( (on && !bitSet) || (!on && bitSet) )
            DirtyIfNecessary();
        return fValue.SetBit(which, on); 
    }
    void Read(hsStream* s) { fValue.Read(s); }
    void Write(hsStream* s) const { fValue.Write(s); }
    void Clear()    { DirtyIfNecessary();   fValue.Clear(); }
    bool32 ClearBit(uint32_t which) {   if (fValue.IsBitSet(which)) DirtyIfNecessary(); return fValue.ClearBit(which); }
    void Reset() { if (fValue.GetSize()!=0) DirtyIfNecessary(); fValue.Reset(); }
    bool32 ToggleBit(uint32_t which) {  DirtyIfNecessary(); return fValue.ToggleBit(which); }
    uint32_t GetSize() { return fValue.GetSize(); }
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

    void ISaveOrLoad(bool32 save, hsStream* stream, hsResMgr* mgr); 
public:
    enum {  kMissingIndex = hsTArray<T>::kMissingIndex  };
    plSynchedTArray() {}
    ~plSynchedTArray() {}

    // conversion operators
    operator T() const { return fValueList; }

    // common methods
    const T& operator[](int i) const { return Get(i); }

    const T& Get(int i) const { return fValueList.Get(i); }
    void    Set(int i, const T& item)   {   if (fValueList[i] != item) DirtyIfNecessary(); fValueList[i]=item;  }
    void    Append(const T& item)   {   fValueList.Append(item);    DirtyIfNecessary(); }
    T*      Insert(int index)   {   fValueList.Insert(index);   DirtyIfNecessary(); }
    void    Remove(int index)   {   fValueList.Remove(index);   DirtyIfNecessary(); }
    int     Count() const { return fValueList.Count(); }
    int     GetCount() const { return Count(); }
    void    Reset() {   if (fValueList.GetCount() != 0) DirtyIfNecessary(); fValueList.Reset();     }
    void    SetCountAndZero(int count)  {   if (count || GetCount()) DirtyIfNecessary(); fValueList.SetCountAndZero(count); }
    void    SetCount(int count)         {   if (count || GetCount()) DirtyIfNecessary(); fValueList.SetCount(count);    }
    void    ExpandAndZero(int count)    {   if (count || GetCount()) DirtyIfNecessary(); fValueList.ExpandAndZero(count);   }
    int     Find(const T& item) const   {   return fValueList.Find(item);   }
    T*      Push()  {   DirtyIfNecessary(); return fValueList.Push();   }
    void    Push(const T& item) {   DirtyIfNecessary(); return fValueList.Push(item);   }
    T   Pop()   {   DirtyIfNecessary(); return fValueList.Pop();    }
    const T&    Peek() const { return fValue.Peek(); }
    T*      DetachArray()   {   DirtyIfNecessary(); return fValueList.DetachArray();    }
    T*      AcquireArray()  {   DirtyIfNecessary(); return fValueList.AcquireArray();   }
};

//
// inlines
//
template <class T> inline
void plSynchedTArray<T>::ISaveOrLoad(bool32 save, hsStream* stream, hsResMgr* mgr)
{
    if (save)
    {
        // write out size of array
        int32_t i, num = fValueList.GetCount();
        stream->WriteLE(num);
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
        int32_t i, num;
        stream->ReadLE(&num);

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
#endif  // USE_SYNCHED_VALUES

#endif  // PL_SYNCHEDVALUE_inc

