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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtArray.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTARRAY_H
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTARRAY_H

#include "Pch.h"
#include "pnUtSort.h"

/****************************************************************************
*
*   Macros
*
***/

#define  ARRAY(type)      TArray< type, TArrayCopyBits< type > >
#define  ARRAYOBJ(type)   TArray< type, TArrayCopyObject< type > >
#define  FARRAY(type)     TFArray< type, TArrayCopyBits< type > >
#define  FARRAYOBJ(type)  TFArray< type, TArrayCopyObject< type > >

/****************************************************************************
*
*   CBuffer
*
***/

template<class T>
class TBuffer {

protected:
    T * m_data;

public:
    inline TBuffer ();
    inline TBuffer (unsigned count);
    inline TBuffer (const void * source, unsigned count);
    inline TBuffer (const TBuffer<T> & source);
    inline ~TBuffer ();
    inline TBuffer<T> & operator= (const TBuffer<T> & source);
    inline bool operator== (const TBuffer<T> & source) const;
    inline T & operator[] (unsigned index);
    inline T operator[] (unsigned index) const;
    inline void Attach (T * source, unsigned count);
    inline unsigned Bytes () const;
    inline void Clear ();
    inline unsigned Count () const;
    inline T * Detach ();
    inline void Fill (uint8_t value);
    inline T * Ptr ();
    inline const T * Ptr () const;
    inline void Set (const T * source, unsigned count);
    inline void SetBytes (unsigned bytes);
    inline void SetCount (unsigned count);
    inline void Zero ();

};

//===========================================================================
template<class T>
TBuffer<T>::TBuffer () {
    m_data = nil;
}

//===========================================================================
template<class T>
TBuffer<T>::TBuffer (unsigned count) {
    m_data = nil;
    SetCount(count);
}

//===========================================================================
template<class T>
TBuffer<T>::TBuffer (const void * source, unsigned count) {
    m_data = nil;
    SetCount(count);
    memcpy(m_data, source, count * sizeof(T));
}

//===========================================================================
template<class T>
TBuffer<T>::TBuffer (const TBuffer<T> & source) {
    m_data = nil;
    unsigned bytes = source.Bytes();
    SetBytes(bytes);
    if (bytes)
        memcpy(m_data, source.m_data, bytes);
}

//===========================================================================
template<class T>
TBuffer<T>::~TBuffer () {
    if (m_data)
        free(m_data);
}

//===========================================================================
template<class T>
TBuffer<T> & TBuffer<T>::operator= (const TBuffer<T> & source) {
    unsigned newBytes = source.Bytes();
    if (newBytes != Bytes())
        SetBytes(newBytes);
    if (&source != this)
        memcpy(m_data, source.m_data, newBytes);
    return *this;
}

//===========================================================================
template<class T>
bool TBuffer<T>::operator== (const TBuffer<T> & source) const {
    unsigned size = _m_size(m_data);
    return (size == _m_size(source.m_data)) && !memcmp(m_data, source.m_data, size);
}

//===========================================================================
template<class T>
T & TBuffer<T>::operator[] (unsigned index) {
    ASSERT(index < Count());
    return m_data[index];
}

//===========================================================================
template<class T>
T TBuffer<T>::operator[] (unsigned index) const {
    ASSERT(index < Count());
    return m_data[index];
}

//===========================================================================
template<class T>
void TBuffer<T>::Attach (T * source, unsigned count) {
    if (m_data)
        free(m_data);
    m_data = source;
    ASSERT(_m_size(source) >= count * sizeof(T));
}

//===========================================================================
template<class T>
unsigned TBuffer<T>::Bytes () const {
    return m_data ? _m_size(m_data) : 0;
}

//===========================================================================
template<class T>
void TBuffer<T>::Clear () {
    if (m_data) {
        free(m_data);
        m_data = nil;
    }
}

//===========================================================================
template<class T>
unsigned TBuffer<T>::Count () const {
    return m_data ? (_m_size(m_data) / sizeof(T)) : 0;
}

//===========================================================================
template<class T>
T * TBuffer<T>::Detach () {
    T * result = m_data;
    m_data = nil;
    return result;
}

//===========================================================================
template<class T>
void TBuffer<T>::Fill (uint8_t value) {
    if (m_data)
        memset(m_data, value, Bytes());
}

//===========================================================================
template<class T>
T * TBuffer<T>::Ptr () {
    return m_data;
}

//===========================================================================
template<class T>
const T * TBuffer<T>::Ptr () const {
    return m_data;
}

//===========================================================================
template<class T>
void TBuffer<T>::Set (const T * source, unsigned count) {
    SetCount(count);
    memcpy(m_data, source, count * sizeof(T));
}

//===========================================================================
template<class T>
void TBuffer<T>::SetBytes (unsigned bytes) {
    if (bytes)
        m_data = (T *)realloc(m_data, bytes);
    else if (m_data) {
        free(m_data);
        m_data = nil;
    }
}

//===========================================================================
template<class T>
void TBuffer<T>::SetCount (unsigned count) {
    SetBytes(count * sizeof(T));
}

//===========================================================================
template<class T>
void TBuffer<T>::Zero () {
    if (m_data)
        memset(m_data, 0, Bytes());
}

typedef TBuffer<uint8_t> CBuffer;


/****************************************************************************
*
*   CBaseArray
*
***/

class CBaseArray {
protected:
    unsigned CalcAllocGrowth (unsigned newAlloc, unsigned oldAlloc, unsigned * chunkSize);
    void * ReallocPtr (void * ptr, unsigned bytes);
};


/****************************************************************************
*
*   TArrayCopyBits
*
***/

template<class T>
class TArrayCopyBits {
public:
    inline static void Assign (T * dest, const T source[], unsigned count);
    inline static void Construct (T * dest) { }
    inline static void Construct (T * dest, unsigned count) { }
    inline static void CopyConstruct (T * dest, const T & source);
    inline static void CopyConstruct (T * dest, const T source[], unsigned count);
    inline static void Destruct (T * dest) { }
    inline static void Destruct (T * dest, unsigned count) { }
};

//===========================================================================
template<class T>
void TArrayCopyBits<T>::Assign (T * dest, const T source[], unsigned count) {
    memmove(dest, source, count * sizeof(T));
}

//===========================================================================
template<class T>
void TArrayCopyBits<T>::CopyConstruct (T * dest, const T & source) {
    *dest = source;
}

//===========================================================================
template<class T>
void TArrayCopyBits<T>::CopyConstruct (T * dest, const T source[], unsigned count) {
    ASSERT((dest + count <= source) || (source + count <= dest));
    memcpy(dest, source, count * sizeof(T));
}


/****************************************************************************
*
*   TArrayCopyObject
*
***/

template<class T>
class TArrayCopyObject {
public:
    inline static void Assign (T * dest, const T source[], unsigned count);
    inline static void Construct (T * dest);
    inline static void Construct (T * dest, unsigned count);
    inline static void CopyConstruct (T * dest, const T & source);
    inline static void CopyConstruct (T * dest, const T source[], unsigned count);
    inline static void Destruct (T * dest);
    inline static void Destruct (T * dest, unsigned count);
};

//===========================================================================
template<class T>
void TArrayCopyObject<T>::Assign (T * dest, const T source[], unsigned count) {
    if (dest > source)
        for (unsigned loop = count; loop--; )
            dest[loop] = source[loop];
    else if (dest < source)
        for (unsigned loop = 0; loop < count; ++loop)
            dest[loop] = source[loop];
}

//===========================================================================
template<class T>
void TArrayCopyObject<T>::Construct (T * dest) {
    new(dest) T;
}

//===========================================================================
template<class T>
void TArrayCopyObject<T>::Construct (T * dest, unsigned count) {
    for (unsigned loop = 0; loop < count; ++loop)
        new(&dest[loop]) T;
}

//===========================================================================
template<class T>
void TArrayCopyObject<T>::CopyConstruct (T * dest, const T & source) {
    new(dest) T(source);
}

//===========================================================================
template<class T>
void TArrayCopyObject<T>::CopyConstruct (T * dest, const T source[], unsigned count) {
    ASSERT((dest + count <= source) || (source + count <= dest));
    for (unsigned loop = 0; loop < count; ++loop)
        new(&dest[loop]) T(source[loop]);
}

//===========================================================================
template<class T>
void TArrayCopyObject<T>::Destruct (T * dest) {
    dest->~T();
}

//===========================================================================
template<class T>
void TArrayCopyObject<T>::Destruct (T * dest, unsigned count) {
    for (unsigned loop = count; loop--; )
        dest[loop].~T();
}


/****************************************************************************
*
*   TFArray
*
***/

template<class T, class C>
class TFArray : protected CBaseArray {
protected:
    T *      m_data;
    unsigned m_alloc;
    unsigned m_count;

    inline void AdjustSize (unsigned newAlloc, unsigned newCount);

public:
    inline TFArray ();
    inline TFArray (unsigned count);
    inline TFArray (const T * source, unsigned count);
    inline TFArray (const TFArray<T,C> & source);
    inline ~TFArray ();
    inline TFArray<T,C> & operator= (const TFArray<T,C> & source);
    inline bool operator== (const TFArray<T,C> & source) const;
    inline T & operator[] (unsigned index);
    inline const T & operator[] (unsigned index) const;
    inline void Attach (T * source, unsigned count);
    inline void AttachTemp (T * source, unsigned count);
    inline unsigned Bytes () const;
    inline void Clear ();
    inline unsigned Count () const;
    inline T * Detach ();
    inline void Fill (uint8_t value);
    inline T * Ptr ();
    inline const T * Ptr () const;
    inline void Set (const T * source, unsigned count);
    inline void SetArray (const TFArray<T,C> & source);
    inline void SetCount (unsigned count);
    inline T * Term ();
    inline const T * Term () const;
    inline T * Top ();
    inline const T * Top () const;
    inline void Zero ();
    inline void ZeroCount ();
    inline void ZeroRange (unsigned index, unsigned count);

};

//===========================================================================
template<class T, class C>
TFArray<T,C>::TFArray () {
    m_alloc = 0;
    m_count = 0;
    m_data  = nil;
}

//===========================================================================
template<class T, class C>
TFArray<T,C>::TFArray (unsigned count) {
    m_alloc = m_count = count;
    if (count) {
        m_data = (T *)malloc(count * sizeof(T));
        C::Construct(m_data, count);
    }
    else
        m_data = nil;
}

//===========================================================================
template<class T, class C>
TFArray<T,C>::TFArray (const T * source, unsigned count) {
    m_alloc = m_count = count;
    if (count) {
        m_data = (T *)malloc(count * sizeof(T));
        C::CopyConstruct(m_data, source, count);
    }
    else
        m_data = nil;
}

//===========================================================================
template<class T, class C>
TFArray<T,C>::TFArray (const TFArray<T,C> & source) {
    m_alloc = m_count = source.m_count;
    if (m_count) {
        m_data = (T *)malloc(m_count * sizeof(T));
        C::CopyConstruct(m_data, source.m_data, m_count);
    }
    else
        m_data = nil;
}

//===========================================================================
template<class T, class C>
TFArray<T,C>::~TFArray () {
    Clear();
}

//===========================================================================
template<class T, class C>
TFArray<T,C> & TFArray<T,C>::operator= (const TFArray<T,C> & source) {
    if (&source == this)
        return *this;
    AdjustSize(source.m_count, 0);
    C::CopyConstruct(m_data, source.m_data, source.m_count);
    m_count = source.m_count;
    return *this;
}

//===========================================================================
template<class T, class C>
inline bool TFArray<T,C>::operator== (const TFArray<T,C> & source) const {
    if (m_count != source.m_count)
        return false;
    for (unsigned index = 0; index < m_count; ++index)
        if (!((*this)[index] == source[index]))
            return false;
    return true;
}

//===========================================================================
template<class T, class C>
T & TFArray<T,C>::operator[] (unsigned index) {
    ASSERT(index < m_count);
    return m_data[index];
}

//===========================================================================
template<class T, class C>
const T & TFArray<T,C>::operator[] (unsigned index) const {
    ASSERT(index < m_count);
    return m_data[index];
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::AdjustSize (unsigned newAlloc, unsigned newCount) {

    // Destruct elements if the array is shrinking
    if (m_count > newCount) {
        C::Destruct(m_data + newCount, m_count - newCount);
        m_count = newCount;
    }

    // Change the memory allocation size if necessary
    if (m_alloc != newAlloc) {
        T * newData = (T *)ReallocPtr(m_data, newAlloc * sizeof(T));
        if (newData != m_data) {
            C::CopyConstruct(newData, m_data, m_count);
            C::Destruct(m_data, m_count);
            if (m_data)
                free(m_data);
        }
        m_alloc = newAlloc;
        m_data  = newData;
    }

    // Construct elements if the array is growing
    if (m_count < newCount) {
        C::Construct(m_data + m_count, newCount - m_count);
        m_count = newCount;
    }

}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::Attach (T * source, unsigned count) {
    C::Destruct(m_data, m_count);
    if (m_data)
        free(m_data);
    m_data  = source;
    m_alloc = _m_size(source) / sizeof(T);
    m_count = count;
    ASSERT(m_alloc >= m_count);
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::AttachTemp (T * source, unsigned count) {
    C::Destruct(m_data, m_count);
    if (m_data)
        free(m_data);
    m_data  = source;
    m_alloc = count;
    m_count = count;
}

//===========================================================================
template<class T, class C>
unsigned TFArray<T,C>::Bytes () const {
    return m_count * sizeof(T);
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::Clear () {
    C::Destruct(m_data, m_count);
    if (m_data)
        free(m_data);
    m_data = nil;
    m_alloc = m_count = 0;
}

//===========================================================================
template<class T, class C>
unsigned TFArray<T,C>::Count () const {
    return m_count;
}

//===========================================================================
template<class T, class C>
T * TFArray<T,C>::Detach () {
    T * result = m_data;
    m_data  = nil;
    m_alloc = 0;
    m_count = 0;
    return result;
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::Fill (uint8_t value) {
    C::Destruct(m_data, m_count);
    memset(m_data, value, m_count * sizeof(T));
    C::Construct(m_data, m_count);
}

//===========================================================================
template<class T, class C>
T * TFArray<T,C>::Ptr () {
    return m_data;
}

//===========================================================================
template<class T, class C>
const T * TFArray<T,C>::Ptr () const {
    return m_data;
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::Set (const T * source, unsigned count) {
    AdjustSize(count, 0);
    C::CopyConstruct(m_data, source, count);
    m_count = count;
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::SetArray (const TFArray<T,C> & source) {
    AdjustSize(source.m_count, 0);
    C::CopyConstruct(m_data, source.m_data, source.m_count);
    m_count = source.m_count;
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::SetCount (unsigned count) {
    AdjustSize(count, count);
}

//===========================================================================
template<class T, class C>
T * TFArray<T,C>::Term () {
    return m_data + m_count;
}

//===========================================================================
template<class T, class C>
const T * TFArray<T,C>::Term () const {
    return m_data + m_count;
}

//===========================================================================
template<class T, class C>
T * TFArray<T,C>::Top () {
    ASSERT(m_count);
    return m_data + m_count - 1;
}

//===========================================================================
template<class T, class C>
const T * TFArray<T,C>::Top () const {
    ASSERT(m_count);
    return m_data + m_count - 1;
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::Zero () {
    C::Destruct(m_data, m_count);
    memset(m_data, 0, m_count * sizeof(T));
    C::Construct(m_data, m_count);
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::ZeroCount () {
    C::Destruct(m_data, m_count);
    m_count = 0;
}

//===========================================================================
template<class T, class C>
void TFArray<T,C>::ZeroRange (unsigned index, unsigned count) {
    ASSERT(index + count <= m_count);
    C::Destruct(m_data + index, count);
    memset(m_data + index, 0, count * sizeof(T));
    C::Construct(m_data + index, count);
}


/****************************************************************************
*
*   TArray
*
***/

template<class T, class C>
class TArray : public TFArray<T,C> {

private:
    unsigned m_chunkSize;

    inline void AdjustSizeChunked (unsigned newAlloc, unsigned newCount);

public:
    inline TArray ();
    inline TArray (const char file[], int line);
    inline TArray (unsigned count);
    inline TArray (const T * source, unsigned count);
    inline TArray (const TArray<T,C> & source);
    inline TArray<T,C> & operator= (const TArray<T,C> & source);
    inline unsigned Add (const T & source);
    inline unsigned Add (const T * source, unsigned count);
    inline unsigned AddArray (const TArray<T,C> & source);
    inline void Copy (unsigned destIndex, unsigned sourceIndex, unsigned count);
    inline void DeleteOrdered (unsigned index);
    inline void DeleteUnordered (unsigned index);
    inline void GrowToCount (unsigned count, bool zero);
    inline void GrowToFit (unsigned index, bool zero);
    inline void ShrinkBy (unsigned count);
    inline void Move (unsigned destIndex, unsigned sourceIndex, unsigned count);
    inline T * New ();
    inline T * New (unsigned count);
    inline void Push (const T & source);
    inline T Pop ();
    inline void Reserve (unsigned additionalCount);
    inline void Set (const T * source, unsigned count);
    inline void SetChunkSize (unsigned chunkSize);
    inline void SetCount (unsigned count);
    inline void SetCountFewer (unsigned count);
    inline void Trim ();

};

//===========================================================================
template<class T, class C>
TArray<T,C>::TArray () : TFArray<T,C>() {
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T, class C>
TArray<T,C>::TArray (const char file[], int line) : TFArray<T,C>(file, line) {
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T, class C>
TArray<T,C>::TArray (unsigned count) : TFArray<T,C>(count) {
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T, class C>
TArray<T,C>::TArray (const T * source, unsigned count) : TFArray<T,C>(source, count) {
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T, class C>
TArray<T,C>::TArray (const TArray & source) : TFArray<T,C>(source) {
    m_chunkSize = source.m_chunkSize;
}

//===========================================================================
template<class T, class C>
TArray<T,C> & TArray<T,C>::operator= (const TArray<T,C> & source) {
    if (&source == this)
        return *this;
    m_chunkSize = source.m_chunkSize;
    AdjustSize(max(this->m_alloc, source.m_count), 0);
    C::CopyConstruct(this->m_data, source.m_data, source.m_count);
    this->m_count = source.m_count;
    return *this;
}

//===========================================================================
template<class T, class C>
unsigned TArray<T,C>::Add (const T & source) {
    unsigned index = this->m_count;
    Push(source);
    return index;
}

//===========================================================================
template<class T, class C>
unsigned TArray<T,C>::Add (const T * source, unsigned count) {
    unsigned index = this->m_count;
    AdjustSizeChunked(this->m_count + count, this->m_count);
    C::CopyConstruct(&this->m_data[this->m_count], source, count);
    this->m_count += count;
    return index;
}

//===========================================================================
template<class T, class C>
unsigned TArray<T,C>::AddArray (const TArray<T,C> & source) {
    unsigned index = this->m_count;
    AdjustSizeChunked(this->m_count + source.m_count, this->m_count);
    C::CopyConstruct(&this->m_data[this->m_count], source.m_data, source.m_count);
    this->m_count += source.m_count;
    return index;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::AdjustSizeChunked (unsigned newAlloc, unsigned newCount) {

    // Disallow shrinking the allocation
    if (newAlloc <= this->m_alloc)
        newAlloc = this->m_alloc;

    // Process growing the allocation
    else
        newAlloc = this->CalcAllocGrowth(newAlloc, this->m_alloc, &this->m_chunkSize);

    // Perform the allocation
    this->AdjustSize(newAlloc, newCount);

}

//===========================================================================
template<class T, class C>
void TArray<T,C>::Copy (unsigned destIndex, unsigned sourceIndex, unsigned count) {

    // Copy the data to the destination
    ASSERT(destIndex +   count <= this->m_count);
    ASSERT(sourceIndex + count <= this->m_count);
    C::Assign(this->m_data + destIndex, this->m_data + sourceIndex, count);

}

//===========================================================================
template<class T, class C>
void TArray<T,C>::DeleteOrdered (unsigned index) {
    ASSERT(index < this->m_count);
    if (index + 1 < this->m_count)
        C::Assign(&this->m_data[index], &this->m_data[index + 1], this->m_count - index - 1);
    C::Destruct(&this->m_data[--this->m_count]);
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::DeleteUnordered (unsigned index) {
    ASSERT(index < this->m_count);
    if (index + 1 < this->m_count)
        C::Assign(&this->m_data[index], &this->m_data[this->m_count - 1], 1);
    C::Destruct(&this->m_data[--this->m_count]);
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::GrowToCount (unsigned count, bool zero) {
    if (count <= this->m_count)
        return;
    AdjustSizeChunked(count, this->m_count);
    if (zero)
        memset(this->m_data + this->m_count, 0, (count - this->m_count) * sizeof(T));
    C::Construct(this->m_data + this->m_count, count - this->m_count);
    this->m_count = count;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::GrowToFit (unsigned index, bool zero) {
    GrowToCount(index + 1, zero);
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::ShrinkBy (unsigned count) {
    ASSERT(count <= this->m_count);
    C::Destruct(this->m_data + this->m_count - count, count);
    this->m_count -= count;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::Move (unsigned destIndex, unsigned sourceIndex, unsigned count) {

    // Copy the data to the destination
    ASSERT(destIndex +   count <= this->m_count);
    ASSERT(sourceIndex + count <= this->m_count);
    C::Assign(this->m_data + destIndex, this->m_data + sourceIndex, count);

    // Remove it from the source
    if (destIndex >= sourceIndex) {
        C::Destruct(this->m_data + sourceIndex, std::min(count, destIndex - sourceIndex));
        C::Construct(this->m_data + sourceIndex, std::min(count, destIndex - sourceIndex));
    }
    else {
        unsigned overlap = (destIndex + count > sourceIndex) ? (destIndex + count - sourceIndex) : 0;
        ASSERT(overlap <= count);
        C::Destruct(this->m_data + sourceIndex + overlap, count - overlap);
        C::Construct(this->m_data + sourceIndex + overlap, count - overlap);
    }

}

//===========================================================================
template<class T, class C>
T * TArray<T,C>::New () {
    AdjustSizeChunked(this->m_count + 1, this->m_count + 1);
    return &this->m_data[this->m_count - 1];
}

//===========================================================================
template<class T, class C>
T * TArray<T,C>::New (unsigned count) {
    AdjustSizeChunked(this->m_count + count, this->m_count + count);
    return &this->m_data[this->m_count - count];
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::Push (const T & source) {
    AdjustSizeChunked(this->m_count + 1, this->m_count);
    C::CopyConstruct(&this->m_data[this->m_count], source);
    ++this->m_count;
}

//===========================================================================
template<class T, class C>
T TArray<T,C>::Pop () {
    ASSERT(this->m_count);
    T result = this->m_data[--this->m_count];
    C::Destruct(this->m_data + this->m_count);
    return result;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::Reserve (unsigned additionalCount) {
    AdjustSizeChunked(std::max(this->m_alloc, this->m_count + additionalCount), this->m_count);
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::Set (const T * source, unsigned count) {
    AdjustSizeChunked(count, 0);
    C::CopyConstruct(this->m_data, source, count);
    this->m_count = count;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::SetChunkSize (unsigned chunkSize) {
    this->m_chunkSize = chunkSize;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::SetCount (unsigned count) {
    AdjustSizeChunked(std::max(this->m_alloc, count), count);
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::SetCountFewer (unsigned count) {
    ASSERT(count <= this->m_count);
    C::Destruct(this->m_data + count, this->m_count - count);
    this->m_count = count;
}

//===========================================================================
template<class T, class C>
void TArray<T,C>::Trim () {
    this->AdjustSize(this->m_count, this->m_count);
}
#endif
