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
// hsHashTable.h

#ifndef _hsHashTable_Included_
#define _hsHashTable_Included_

#include "hsTemplates.h"

template <class T>
class hsHashTableIterator
{
public:
    hsHashTableIterator() : fList(nil), fIndex(-1)  { }
    explicit hsHashTableIterator(hsTArray<T>* list, uint32_t idx) : fList(list), fIndex(idx) { }
    
    T* operator->() const   { return &((*fList)[fIndex]); }
    T& operator*() const    { return (*fList)[fIndex]; }
    
    hsHashTableIterator<T>& operator++()            { fIndex--; return *this; }
    const hsHashTableIterator<T>& operator++(int)   { hsHashTableIterator<T> temp(*this); --(*this); return temp; }
    
    hsHashTableIterator<T>& operator--()            { fIndex++; return *this; }
    const hsHashTableIterator<T>& operator--(int)   { hsHashTableIterator<T> temp(*this); ++(*this); return temp; }
    
    bool operator==(const hsHashTableIterator<T>& other) const    { return fList==other.fList && fIndex==other.fIndex; }
    bool operator!=(const hsHashTableIterator<T>& other) const    { return !(*this == other); }
    
private:
    hsTArray<T>* fList;
    uint32_t fIndex;
};


template <class T> 
class hsHashTable
{
public:
    hsHashTable(uint32_t size=150001, uint32_t step=1);
    ~hsHashTable();

    typedef hsHashTableIterator<T> iterator;

    iterator begin()    { return iterator(&fItemList,fItemList.Count()-1); }
    iterator end()      { return iterator(&fItemList,0); }
    void clear();

    uint32_t count()          { return fItemList.Count()-1; }
    uint32_t size()           { return fSize; }

    uint32_t CollisionCount() { return fCollisionCount; }

    inline void insert(T& item);
    inline void erase(T& item);
    inline iterator find(const T& item);

    iterator GetItem(uint32_t i);
    
private:
    hsTArray<T>         fItemList;
    hsTArray<uint32_t>    fClearList;

    uint32_t*     fHashTable;
    uint32_t      fSize;
    uint32_t      fCollisionStep;
    uint32_t      fCollisionCount;

    // No copy or assignment
    hsHashTable(const hsHashTable&);
    hsHashTable &operator=(const hsHashTable&);
};

template <class T>
hsHashTable<T>::hsHashTable(uint32_t size, uint32_t step) :
fSize(size),
fCollisionStep(step),
fCollisionCount(0)
{
    fItemList.SetCount(1);
    fHashTable = new uint32_t[fSize];
    memset(fHashTable,0,fSize*sizeof(uint32_t));
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
    for (int32_t i=0; i<fClearList.Count(); i++)
        fHashTable[ fClearList[i] ] = 0;
    fClearList.Reset();
}

template <class T>
void hsHashTable<T>::insert(T& item)
{
    hsAssert(fClearList.Count() < fSize,"Hash table overflow!  Increase the table size.");
    uint32_t h = item.GetHash();
    h %= fSize;
    while (uint32_t it = fHashTable[h])
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
    uint32_t h = item.GetHash();
    h %= fSize;
    while (uint32_t it = fHashTable[h])
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
    uint32_t h = item.GetHash();
    h %= fSize;
    while (uint32_t it = fHashTable[h])
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
hsHashTableIterator<T> hsHashTable<T>::GetItem(uint32_t i)
{
    return iterator(&fItemList,i+1);
}

#endif // _hsHashTable_Included_
