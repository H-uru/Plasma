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

#ifndef hsSearchVersion_inc
#define hsSearchVersion_inc

#include "HeadSpin.h"

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
    T                                       fData;
    int32_t                                   fIndex;
    hsVersionNode<T>*       fNext;
public:
    hsVersionNode(const uint32_t idx, const T &data) : fIndex(idx), fNext(nil) { fData = data; }
    ~hsVersionNode() { delete fNext; }

    hsVersionNode<T>*   Next() const { return fNext; }

    int32_t               Index() const { return fIndex; }

    inline void Append(hsVersionNode<T>* next);
    inline int operator==(const T& o) const;
    
    int operator!=(const T& o) const { return !(this->operator==(o)); }

    T&          GetData() { return fData; }
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
    uint32_t              fLength;
    hsVersionNode<T>**  fArray;
    uint32_t              fNextIndex;
    uint32_t              fNumIndex;
    uint32_t              fIncIndex;
    T**                 fBackArray;

    void                ICheckBackArray();
public:
    hsSearchVersion(uint32_t len, uint32_t inc = 0);
    ~hsSearchVersion();

    T&          operator[]( int32_t index );

    int32_t               Find(int where, const T& what, bool forceUnique=false);

    uint32_t              GetCount() const { return fNextIndex; }
};

template <class T> T& hsSearchVersion<T>::operator[]( int32_t index )
{
    hsDebugCode(hsThrowIfBadParam((uint32_t)index >= (uint32_t)fNextIndex);)

    return *fBackArray[index];
}

template <class T> hsSearchVersion<T>::hsSearchVersion(uint32_t len, uint32_t inc)
    : fNextIndex(0)
{ 
    fIncIndex = inc ? inc : len;
    fLength = len; 
    fArray = new hsVersionNode<T>*[fLength]; 
    HSMemory::Clear(fArray, fLength*sizeof(*fArray)); 
    fBackArray = new T*[fNumIndex = fLength];
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
        T** newBackArray = new T*[fNumIndex + fIncIndex];
        HSMemory::BlockMove(fBackArray, newBackArray, fNextIndex*sizeof(T*));
        delete [] fBackArray;
        fBackArray = newBackArray;
        fNumIndex += fIncIndex;
    }
}

template <class T> int32_t hsSearchVersion<T>::Find(int where, const T&what, bool forceUnique)
{
    hsVersionNode<T>* curr = fArray[where];

    ICheckBackArray();

    if( !curr )
    {
        hsVersionNode<T>* next = new hsVersionNode<T>(fNextIndex, what);
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

    hsVersionNode<T>* next = new hsVersionNode<T>(fNextIndex, what);
    curr->Append(next);
    fBackArray[fNextIndex] = &next->GetData();
    return fNextIndex++;
}

#endif // hsSearchVersion_inc
