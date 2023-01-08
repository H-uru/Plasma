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

#ifndef hsBiExpander_inc
#define hsBiExpander_inc

#include "hsExceptions.h"

///////////////////////////////////////////////////////////////////////////////
////////////// Expander ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <class T> class hsExpander {
private:
    int32_t       fNumPost;
    int32_t       fNumPostAlloc;
    T*            fArray; 

    int32_t       fGrowBy; // default = 0, to double
    int32_t       fMinSize; // default = 1, min == 1

    int32_t       fCurrent;

    hsExpander(const hsExpander& x);                    // make it passed as ref or pointer

    void        IExpand(int newSize);
public:
    enum { kMissingIndex = -1 };

                hsExpander(int32_t minSize = 1, int32_t growBy = 0);
    virtual     ~hsExpander();

    hsExpander<T>& operator=(const hsExpander<T>&orig) { return Copy(orig); }
    hsExpander<T>& Copy(const hsExpander<T>& orig);

    void      SetCount(int cnt) { if( cnt >= fNumPostAlloc )IExpand(cnt); fNumPost = cnt; }
    int32_t   GetCount() const { return fNumPost; }
    bool      Empty() const { return GetCount() == 0; }
    const T&  Get(int32_t index) const;
    int32_t   Get(int32_t index, int32_t count, T data[]) const;
    int32_t   Find(const T&) const;   // returns kMissingIndex if not found

    void        SetArray(T* a, int32_t cnt);
    T*          GetArray() { return fArray; }
    T&          operator[]( int32_t index );
    int32_t     Append(const T&); // returns t's index
    T*          Append();
    int32_t     Push(const T& t) { return Append(t); }
    T*          Push() { return Append(); }
    T*          Top() { return fNumPost ? fArray + fNumPost-1 : nullptr; }
    int32_t     Pop(T* t); // returns count of remaining
    int32_t     Pop();
    void        Reset();    // clears out everything

    T&          Head() { return fArray[0]; }
    T&          Tail() { return fArray[fNumPost-1]; }
    T&          Current() { return fArray[fCurrent]; }
    void        First();
    void        Last();
    void        Plus() { ++fCurrent; }
    bool        More() { return (fCurrent < fNumPost); }
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
void hsExpander<T>::SetArray(T* a, int32_t cnt)
{
    delete [] fArray;
    if( a )
        fArray = a;
    fNumPost = fNumPostAlloc = cnt;
}

template <class T>
void hsExpander<T>::IExpand(int newSize)
{
    int32_t newPostAlloc = fNumPostAlloc;
    if( !newPostAlloc )
        newPostAlloc++;
    while( newPostAlloc <= newSize )
        newPostAlloc = fGrowBy ? newPostAlloc + fGrowBy : newPostAlloc << 1;
    T* newArray = new T[newPostAlloc];
    int i;
    for( i = 0; i < fNumPost; i++ )
        newArray[i] = fArray[i];
    delete [] (fArray);
    fArray = newArray;
    fNumPostAlloc = newPostAlloc;
}

template <class T>
hsExpander<T>::hsExpander(int32_t minSize, int32_t growBy)
    : fCurrent(), fNumPost()
{
    hsThrowIfBadParam(minSize < 0);
    hsThrowIfBadParam(growBy < 0);

    fMinSize = minSize+1;
    fGrowBy = growBy;
    
    fArray  = new T[fMinSize];
    fNumPostAlloc = fMinSize;
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
T& hsExpander<T>::operator[]( int32_t index )
{
    hsDebugCode(hsThrowIfBadParam((index < 0)||(index >= fNumPost));)

    return fArray[index];
}

template <class T> 
const T& hsExpander<T>::Get( int32_t index ) const
{
    hsDebugCode(hsThrowIfBadParam((index < 0)||(index >= fNumPost));)

    return fArray[index];
}

template <class T>
int32_t hsExpander<T>::Get(int32_t index, int32_t count, T data[]) const
{
    if( count > 0 )
    {   hsThrowIfNilParam(data);
        hsThrowIfBadParam((index < 0)||(index >= fNumPost));

        if (index + count > fNumPost)
            count = fNumPost - index;
        for (int i = 0; i < count; i++)
            data[i] = fArray[i + index];
    }
    return count;
}

template <class T>
int32_t hsExpander<T>::Find(const T& obj) const
{
    for (int i = 0; i < fNumPost; i++)
        if (fArray[i] == obj)
            return i;
    return kMissingIndex;
}

template <class T>
int32_t hsExpander<T>::Append(const T& obj)
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
int32_t hsExpander<T>::Pop(T*t)
{
    hsThrowIfBadParam(Empty());
    --fNumPost;
    if( t )
        *t = fArray[fNumPost];
    return GetCount();
}

template <class T>
int32_t hsExpander<T>::Pop()
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
    int32_t       fNumPre;
    int32_t       fNumPost;
    int32_t       fNumPreAlloc;
    int32_t       fNumPostAlloc;
    T*            fArray; 

    int32_t       fGrowBy; // default = 0, to double
    int32_t       fMinSize; // default = 1, min == 1

    int32_t       fCurrent;

    hsBiExpander<T>& operator=(const hsBiExpander<T>&);     // don't allow assignment
    hsBiExpander(const hsBiExpander<T>&);                   // make it passed as ref or pointer

    void        IExpand(int newSize, bool towardEnd = true);
public:
    enum { kMissingIndex = -1 };

                hsBiExpander(int32_t minSize = 1, int32_t growBy = 0);
    virtual     ~hsBiExpander();

    int32_t       GetFirst() const { return -fNumPre; }
    int32_t       GetCount() const { return fNumPre + fNumPost; }
    bool          Empty() const { return GetCount() == 0; }
    const T&      Get(int32_t index) const;
    int32_t       Get(int32_t index, int32_t count, T data[]) const;
    int32_t       Find(const T&) const;   // returns kMissingIndex if not found

    void          SetArray(T* a, int32_t cnt, int32_t numPre=0);
    T**           GetArray() { return fArray - fNumPre; }
    T&            operator[]( int32_t index );
    T*            Append(); // returns t's index
    T*            Push(); // returns t's index
    int32_t       Append(const T&); // returns t's index
    int32_t       Push(const T&); // returns t's index
    int32_t       Pop(T* t = nullptr) { return PopHead(t); } // returns count of remaining
    int32_t       PopHead(T* t = nullptr); // returns count of remaining
    int32_t       PopTail(T* t = nullptr); // returns count of remaining
    void          Reset();    // clears out everything

    T&          Head() { return fArray[-fNumPre]; }
    T&          Tail() { return fArray[fNumPost-1]; }
    T&          Current() { return fArray[fCurrent]; }
    void        First();
    void        Last();
    void        Plus() { ++fCurrent; }
    void        Minus() { --fCurrent; }
    bool        More() { return (fCurrent < fNumPost)&&(fCurrent >= -fNumPre); }
};

template <class T>
void hsBiExpander<T>::SetArray(T* a, int32_t cnt, int32_t numPre)
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
void hsBiExpander<T>::IExpand(int newSize, bool towardEnd)
{
    int32_t newPreAlloc = fNumPreAlloc;
    int32_t newPostAlloc = fNumPostAlloc;
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
    T* newArray = new T[newPreAlloc + newPostAlloc];
    newArray += newPreAlloc;
    int i;
    for( i = -fNumPre; i < fNumPost; i++ )
        newArray[i] = fArray[i];
//  memmove(newArray-fNumPre, fArray-fNumPre,
//      (fNumPre+fNumPost)*sizeof(*fArray));
    delete [] (fArray-fNumPreAlloc);
    fArray = newArray;
    fNumPreAlloc = newPreAlloc;
    fNumPostAlloc = newPostAlloc;
}

template <class T>
hsBiExpander<T>::hsBiExpander(int32_t minSize, int32_t growBy)
{
    hsThrowIfBadParam(minSize < 0);
    hsThrowIfBadParam(growBy < 0);

    fMinSize = minSize+1;
    fGrowBy = growBy;
    
    fArray  = new T[fMinSize << 1];
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
T& hsBiExpander<T>::operator[]( int32_t index )
{
    hsDebugCode(hsThrowIfBadParam((index < -fNumPre)||(index >= fNumPost));)

    return fArray[index];
}

template <class T> 
const T& hsBiExpander<T>::Get( int32_t index ) const
{
    hsDebugCode(hsThrowIfBadParam((index < -fNumPre)||(index >= fNumPost));)

    return fArray[index];
}

template <class T>
int32_t hsBiExpander<T>::Get(int32_t index, int32_t count, T data[]) const
{
    if( count > 0 )
    {   hsThrowIfNilParam(data);
        hsThrowIfBadParam((index < -fNumPre)||(index >= fNumPost));

        if (index + count > fNumPost)
            count = fNumPost - index;
        for (int i = 0; i < count; i++)
            data[i] = fArray[i + index];
    }
    return count;
}

template <class T>
int32_t hsBiExpander<T>::Find(const T& obj) const
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
int32_t hsBiExpander<T>::Append(const T& obj)
{
    hsAssert(!(fNumPost >= fNumPostAlloc), "Must be less");
    if( fNumPost == fNumPostAlloc-1 )
        IExpand(fNumPostAlloc, true);
    fArray[fNumPost] = obj;
    return fNumPost++;
}

template <class T>
int32_t hsBiExpander<T>::Push(const T& obj)
{
    hsAssert(!(fNumPre >= fNumPreAlloc), "Must be less");
    if( ++fNumPre == fNumPreAlloc )
        IExpand(fNumPreAlloc, false);
    fArray[-fNumPre] = obj;
    return -fNumPre;
}

template <class T>
int32_t hsBiExpander<T>::PopHead(T*t)
{
    hsThrowIfBadParam(Empty());
    if( t )
        *t = fArray[-fNumPre];
    --fNumPre;
    return GetCount();
}

template <class T>
int32_t hsBiExpander<T>::PopTail(T*t)
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
