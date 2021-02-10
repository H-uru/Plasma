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
#ifndef hsTemplatesDefined
#define hsTemplatesDefined

#include "hsMemory.h"

#include <type_traits>


#ifdef HS_DEBUGGING
// #define   HS_DEBUGTARRAY
#endif

#ifdef HS_DEBUGTARRAY

    // just a quickie d-link list class for debugging
class hsDlistNode
{
public:
    static hsDlistNode *fpFirst;
    static hsDlistNode *fpLast;
    static uint32_t       fcreated;
    static uint32_t       fdestroyed;

    void *fpThing;
    hsDlistNode *fpPrev;
    hsDlistNode *fpNext;
    hsDlistNode(void *tng): fpThing(tng), fpNext(0), fpPrev(0) { AddNode(); }
    void AddNode();
    void RemoveNode();
    hsDlistNode *GetNext() { return fpNext; }
};

#endif  

////////////////////////////////////////////////////////////////////////////////

class hsTArrayBase 
{
protected:
    uint32_t      fUseCount;
    uint32_t      fTotalCount;

    void GrowArraySize(uint32_t nSize);

#ifdef HS_DEBUGTARRAY
    hsTArrayBase();
    virtual char    *GetTypeName();
    virtual int      GetSizeOf();
    hsDlistNode *self;
    friend void TArrayStats();
    virtual ~hsTArrayBase();
#else
    hsTArrayBase() : fUseCount(), fTotalCount() { }
#endif

public:
    uint32_t GetNumAlloc() const { return fTotalCount; }
};

template <class T> void hsTArray_CopyForward(const T src[], T dst[], int count);
template <class T> void hsTArray_CopyBackward(const T src[], T dst[], int count);

template <class T> class hsTArray : public hsTArrayBase 
{
    T*      fArray;
    
    inline void IncCount(int index, int count);
    inline void DecCount(int index, int count);

#ifdef HS_DEBUGGING
    #define hsTArray_ValidateCount(count)       hsAssert((count) >= 0, "bad count")
    #define hsTArray_ValidateIndex(index)       hsAssert(unsigned(index) < fUseCount, "bad index")
    #define hsTArray_ValidateInsertIndex(index) hsAssert(unsigned(index) <= fUseCount, "bad index")
    #define hsTArray_Validate(condition)        hsAssert(condition, "oops")

    #ifdef HS_DEBUGTARRAY
        int      GetSizeOf() override { return sizeof(T); }
    #endif
#else
    #define hsTArray_ValidateCount(count)
    #define hsTArray_ValidateIndex(index)
    #define hsTArray_ValidateInsertIndex(index)
    #define hsTArray_Validate(condition)
#endif
public:
    hsTArray() : fArray(nil) {}
    inline  hsTArray(int count);
    inline  hsTArray(const hsTArray<T>& src);
            ~hsTArray() { if (fArray) delete[] fArray; 
            }
    inline void Expand(int NewTotal);

    inline hsTArray<T>& operator=(const hsTArray<T>& src);
    bool operator==(const hsTArray<T>& src) const;  // checks sizes and contents

    // Swaps the internal data (including the fArray POINTER) with the data from the array given
    void    Swap( hsTArray<T>& src );

    void    Set(int index, const T& item)   { hsTArray_ValidateIndex(index); fArray[index]=item; }
    const T&    Get(int index) const { hsTArray_ValidateIndex(index); return fArray[index]; }
    T&       operator[](int index) { hsTArray_ValidateIndex(index); return fArray[index]; }
    const T& operator[](int index) const { hsTArray_ValidateIndex(index); return fArray[index]; }

    int     Count() const { return fUseCount; }
    int     GetCount() const { return fUseCount; }
    inline void SetCount(int count);

    /** WARNING: By design (sigh), the new elements are not (re)initialized... */
    void Resize(int count)
    {
        Expand(count);
        SetCount(count);
    }
    
    inline void SetCountAndZero(int count); // does block clear, don't use for types with vtbl
    inline void ExpandAndZero(int count);   // Same as set count and zero except won't decrease
                                            // usecount
    inline void Reset();

    T*      Insert(int index)
            {
                hsTArray_ValidateInsertIndex(index);
                this->IncCount(index, 1);
                return &fArray[index];
            }
    void        Insert(int index, const T& item)
            {
                hsTArray_ValidateInsertIndex(index);
                this->IncCount(index, 1);
                fArray[index] = item;
            }
    void        Insert(int index, int count, T item[])
            {
                hsTArray_ValidateCount(count);
                if (count > 0)
                {   hsTArray_ValidateInsertIndex(index);
                    this->IncCount(index, count);
                    hsTArray_CopyForward(item, &fArray[index], count);
                }
            }

    void        Remove(int index)
            {
                hsTArray_ValidateIndex(index);
                this->DecCount(index, 1);
            }
    void        Remove(int index, int count)
            {
                hsTArray_ValidateCount(count);
                hsTArray_ValidateIndex(index);
                hsTArray_ValidateIndex(index + count - 1);
                this->DecCount(index, count);
            }
    bool        RemoveItem(const T& item);

    T*      Push()
            {
                this->IncCount(fUseCount, 1);
                return &fArray[fUseCount - 1];
            }
    void        Push(const T& item)
            {
                this->IncCount(fUseCount, 1);
                fArray[fUseCount - 1] = item;
            }
    void        Append(const T& item)
            {
                this->IncCount(fUseCount, 1);
                fArray[fUseCount - 1] = item;
            }
    inline T    Pop();
    inline const T& Peek() const;

    enum {
        kMissingIndex    = -1
    };
    int     Find(const T& item) const;  // returns kMissingIndex if not found

    T*      AcquireArray() { return fArray; }
};

//////////////  Public hsTArray methods

template <class T> hsTArray<T>::hsTArray(int count) : fArray(nil)
{
    hsTArray_ValidateCount(count);
    fUseCount = fTotalCount = count;
    if (count > 0)
        fArray = new T[count];
}

template <class T> hsTArray<T>::hsTArray(const hsTArray<T>& src) : fArray(nil)
{
    int count = src.Count();
    fUseCount = fTotalCount = count;

    if (count > 0)
    {   
        fArray = new T[count];
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
        return true;    // it's me

    if (GetCount() != src.GetCount())
        return false;   // different sizes

    int i;
    for(i=0;i<GetCount();i++)
        if (Get(i) != src[i])
            return false;   // different contents

    return true;        // the same
}


//// Swap ////////////////////////////////////////////////////////////////////
//  Added 5.2.2001 mcn - Given another hsTArray of the same type, "swaps" the
//  data stored in both. Basically we're literally swapping the fArray pointers
//  around, plus the use counts and such.

template <class T> void hsTArray<T>::Swap( hsTArray<T>& src )
{
    uint32_t    use, tot;
    T           *array;


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
    static_assert(std::is_trivially_copyable<T>::value,
                  "Cannot use SetCountAndZero on non-trivially copyable types");

    if (fTotalCount <= count)
        Expand(count);
    HSMemory::Clear(fArray, fTotalCount * sizeof(T));
    fUseCount = count;
}

template <class T> void hsTArray<T>::ExpandAndZero(int count)
{
    static_assert(std::is_trivially_copyable<T>::value,
                  "Cannot use ExpandAndZero on non-trivially copyable types");

    if( fTotalCount <= count )
    {
        int n = fTotalCount;
        Expand(count);
        HSMemory::Clear(fArray + n, (count - n) * sizeof(T));
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
        fArray = new T[count];
        fUseCount = fTotalCount = count;
    }
    fUseCount = count;
}

template <class T> void hsTArray<T>::Expand(int NewCount) // New Count is Absolute not additional
{
    hsTArray_ValidateCount(NewCount);
    if (NewCount > fTotalCount)         // This is Expand not Shrink
    {   
        T*  newArray = new T[NewCount];

        if (fArray != nil)
        {   hsTArray_CopyForward(fArray, newArray, fUseCount);
//          hsTArray_CopyForward(&fArray[index], &newArray[index + count], fUseCount - index);
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

template <class T> bool hsTArray<T>::RemoveItem(const T& item)
{
    for (int i = 0; i < fUseCount; i++)
        if (fArray[i] == item)
        {   this->DecCount(i, 1);
            return true;
        }
    return false;
}

//////////  These are the private methods for hsTArray

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
    int newCount = fUseCount + count;

    if (newCount > fTotalCount)
    {   if (fTotalCount == 0)
            fTotalCount = newCount;

        GrowArraySize(newCount);    // Sets new fTotalCount
        T*  newArray = new T[fTotalCount];

        if (fArray != nil)
        {   hsTArray_CopyForward(fArray, newArray, index);
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
    {   hsTArray_CopyForward(&fArray[index + count], &fArray[index], fUseCount - index - count);
        fUseCount -= count;
    }
}

#endif

