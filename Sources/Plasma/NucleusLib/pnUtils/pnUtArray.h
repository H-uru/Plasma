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
*   TArrayCopy
*
***/

class TArrayCopy {
public:
    template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
    inline static void Assign(T* dest, const T source[], unsigned count)
    {
        memmove(dest, source, count * sizeof(T));
    }

    template <class T, typename std::enable_if_t<!std::is_trivially_copyable<T>::value, int> = 0>
    inline static void Assign(T* dest, const T source[], unsigned count)
    {
        if (dest > source) {
            for (unsigned loop = count; loop--; )
                dest[loop] = source[loop];
        } else if (dest < source) {
            for (unsigned loop = 0; loop < count; ++loop)
                dest[loop] = source[loop];
        }
    }

    template <class T, typename std::enable_if_t<std::is_trivial<T>::value, int> = 0>
    inline static void Construct(T* dest) { }

    template <class T, typename std::enable_if_t<!std::is_trivial<T>::value, int> = 0>
    inline static void Construct(T* dest)
    {
        new(dest) T;
    }

    template <class T, typename std::enable_if_t<std::is_trivial<T>::value, int> = 0>
    inline static void Construct(T* dest, unsigned count) { }

    template <class T, typename std::enable_if_t<!std::is_trivial<T>::value, int> = 0>
    inline static void Construct(T* dest, unsigned count)
    {
        for (unsigned loop = 0; loop < count; ++loop)
            new(&dest[loop]) T;
    }

    template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
    inline static void CopyConstruct(T* dest, const T& source)
    {
        *dest = source;
    }

    template <class T, typename std::enable_if_t<!std::is_trivially_copyable<T>::value, int> = 0>
    inline static void CopyConstruct(T* dest, const T& source)
    {
        new(dest) T(source);
    }

    template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
    inline static void CopyConstruct(T* dest, const T source[], unsigned count)
    {
        ASSERT((dest + count <= source) || (source + count <= dest));
        memcpy(dest, source, count * sizeof(T));
    }

    template <class T, typename std::enable_if_t<!std::is_trivially_copyable<T>::value, int> = 0>
    inline static void CopyConstruct(T* dest, const T source[], unsigned count)
    {
        ASSERT((dest + count <= source) || (source + count <= dest));
        for (unsigned loop = 0; loop < count; ++loop)
            new(&dest[loop]) T(source[loop]);
    }

    template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
    inline static void Destruct(T* dest) { }

    template <class T, typename std::enable_if_t<!std::is_trivially_copyable<T>::value, int> = 0>
    inline static void Destruct(T* dest)
    {
        dest->~T();
    }

    template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
    inline static void Destruct(T* dest, unsigned count) { }

    template <class T, typename std::enable_if_t<!std::is_trivially_copyable<T>::value, int> = 0>
    inline static void Destruct(T* dest, unsigned count)
    {
        for (unsigned loop = count; loop--; )
            dest[loop].~T();
    }
};


/****************************************************************************
*
*   TArray
*
***/

template<class T>
class TArray : protected CBaseArray {

private:
    T *      m_data;
    unsigned m_alloc;
    unsigned m_count;
    unsigned m_chunkSize;

    inline void AdjustSize (unsigned newAlloc, unsigned newCount);
    inline void AdjustSizeChunked (unsigned newAlloc, unsigned newCount);

public:
    inline TArray ();
    inline TArray (const char file[], int line) = delete;
    inline TArray (unsigned count);
    inline TArray (const T * source, unsigned count);
    inline TArray (const TArray<T> & source);
    inline ~TArray ();
    inline TArray<T> & operator= (const TArray<T> & source);
    inline bool operator== (const TArray<T> & source) const;
    inline T & operator[] (unsigned index);
    inline const T & operator[] (unsigned index) const;
    inline unsigned Add (const T & source);
    inline unsigned Add (const T * source, unsigned count);
    inline unsigned AddArray (const TArray<T> & source);
    inline void Attach (T * source, unsigned count);
    inline void AttachTemp (T * source, unsigned count);
    inline unsigned Bytes () const;
    inline void Clear ();
    inline void Copy (unsigned destIndex, unsigned sourceIndex, unsigned count);
    inline unsigned Count () const;
    inline void DeleteOrdered (unsigned index);
    inline void DeleteUnordered (unsigned index);
    inline T * Detach ();
    inline void Fill (uint8_t value);
    inline void GrowToCount (unsigned count, bool zero);
    inline void GrowToFit (unsigned index, bool zero);
    inline void ShrinkBy (unsigned count);
    inline void Move (unsigned destIndex, unsigned sourceIndex, unsigned count);
    inline T * New ();
    inline T * New (unsigned count);
    inline T * Ptr ();
    inline const T * Ptr () const;
    inline void Push (const T & source);
    inline T Pop ();
    inline void Reserve (unsigned additionalCount);
    inline void Set (const T * source, unsigned count);
    inline void SetArray (const TArray<T> & source);
    inline void SetChunkSize (unsigned chunkSize);
    inline void SetCount (unsigned count);
    inline void SetCountFewer (unsigned count);
    inline T * Term ();
    inline const T * Term () const;
    inline T * Top ();
    inline const T * Top () const;
    inline void Trim ();
    inline void Zero ();
    inline void ZeroCount ();
    inline void ZeroRange (unsigned index, unsigned count);

};

//===========================================================================
template<class T>
TArray<T>::TArray () {
    m_alloc = 0;
    m_count = 0;
    m_data  = nil;
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T>
TArray<T>::TArray (unsigned count) {
    m_alloc = m_count = count;
    if (count) {
        m_data = (T *)malloc(count * sizeof(T));
        TArrayCopy::Construct(m_data, count);
    }
    else
        m_data = nil;
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T>
TArray<T>::TArray (const T * source, unsigned count) {
    m_alloc = m_count = count;
    if (count) {
        m_data = (T *)malloc(count * sizeof(T));
        TArrayCopy::CopyConstruct(m_data, source, count);
    }
    else
        m_data = nil;
    m_chunkSize = std::max(size_t(1), 256 / sizeof(T));
}

//===========================================================================
template<class T>
TArray<T>::TArray (const TArray<T> & source) {
    m_alloc = m_count = source.m_count;
    if (m_count) {
        m_data = (T *)malloc(m_count * sizeof(T));
        TArrayCopy::CopyConstruct(m_data, source.m_data, m_count);
    }
    else
        m_data = nil;
    m_chunkSize = source.m_chunkSize;
}

//===========================================================================
template<class T>
TArray<T>::~TArray () {
    Clear();
}

//===========================================================================
template<class T>
TArray<T> & TArray<T>::operator= (const TArray<T> & source) {
    if (&source == this)
        return *this;
    m_chunkSize = source.m_chunkSize;
    AdjustSize(std::max(m_alloc, source.m_count), 0);
    TArrayCopy::CopyConstruct(this->m_data, source.m_data, source.m_count);
    this->m_count = source.m_count;
    return *this;
}

//===========================================================================
template<class T>
inline bool TArray<T>::operator== (const TArray<T> & source) const {
    if (m_count != source.m_count)
        return false;
    for (unsigned index = 0; index < m_count; ++index)
        if (!((*this)[index] == source[index]))
            return false;
    return true;
}

//===========================================================================
template<class T>
T & TArray<T>::operator[] (unsigned index) {
    ASSERT(index < m_count);
    return m_data[index];
}

//===========================================================================
template<class T>
const T & TArray<T>::operator[] (unsigned index) const {
    ASSERT(index < m_count);
    return m_data[index];
}

//===========================================================================
template<class T>
unsigned TArray<T>::Add (const T & source) {
    unsigned index = this->m_count;
    Push(source);
    return index;
}

//===========================================================================
template<class T>
unsigned TArray<T>::Add (const T * source, unsigned count) {
    unsigned index = this->m_count;
    AdjustSizeChunked(this->m_count + count, this->m_count);
    TArrayCopy::CopyConstruct(&this->m_data[this->m_count], source, count);
    this->m_count += count;
    return index;
}

//===========================================================================
template<class T>
unsigned TArray<T>::AddArray (const TArray<T> & source) {
    unsigned index = this->m_count;
    AdjustSizeChunked(this->m_count + source.m_count, this->m_count);
    TArrayCopy::CopyConstruct(&this->m_data[this->m_count], source.m_data, source.m_count);
    this->m_count += source.m_count;
    return index;
}

//===========================================================================
template<class T>
void TArray<T>::AdjustSize (unsigned newAlloc, unsigned newCount) {

    // Destruct elements if the array is shrinking
    if (m_count > newCount) {
        TArrayCopy::Destruct(m_data + newCount, m_count - newCount);
        m_count = newCount;
    }

    // Change the memory allocation size if necessary
    if (m_alloc != newAlloc) {
        T * newData = (T *)ReallocPtr(m_data, newAlloc * sizeof(T));
        if (newData != m_data) {
            TArrayCopy::CopyConstruct(newData, m_data, m_count);
            TArrayCopy::Destruct(m_data, m_count);
            if (m_data)
                free(m_data);
        }
        m_alloc = newAlloc;
        m_data  = newData;
    }

    // Construct elements if the array is growing
    if (m_count < newCount) {
        TArrayCopy::Construct(m_data + m_count, newCount - m_count);
        m_count = newCount;
    }

}

//===========================================================================
template<class T>
void TArray<T>::AdjustSizeChunked (unsigned newAlloc, unsigned newCount) {

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
template<class T>
void TArray<T>::Attach (T * source, unsigned count) {
    TArrayCopy::Destruct(m_data, m_count);
    if (m_data)
        free(m_data);
    m_data  = source;
    m_alloc = _m_size(source) / sizeof(T);
    m_count = count;
    ASSERT(m_alloc >= m_count);
}

//===========================================================================
template<class T>
void TArray<T>::AttachTemp (T * source, unsigned count) {
    TArrayCopy::Destruct(m_data, m_count);
    if (m_data)
        free(m_data);
    m_data  = source;
    m_alloc = count;
    m_count = count;
}

//===========================================================================
template<class T>
unsigned TArray<T>::Bytes () const {
    return m_count * sizeof(T);
}

//===========================================================================
template<class T>
void TArray<T>::Clear () {
    TArrayCopy::Destruct(m_data, m_count);
    if (m_data)
        free(m_data);
    m_data = nil;
    m_alloc = m_count = 0;
}

//===========================================================================
template<class T>
void TArray<T>::Copy (unsigned destIndex, unsigned sourceIndex, unsigned count) {

    // Copy the data to the destination
    ASSERT(destIndex +   count <= this->m_count);
    ASSERT(sourceIndex + count <= this->m_count);
    TArrayCopy::Assign(this->m_data + destIndex, this->m_data + sourceIndex, count);

}

//===========================================================================
template<class T>
unsigned TArray<T>::Count () const {
    return m_count;
}

//===========================================================================
template<class T>
void TArray<T>::DeleteOrdered (unsigned index) {
    ASSERT(index < this->m_count);
    if (index + 1 < this->m_count)
        TArrayCopy::Assign(&this->m_data[index], &this->m_data[index + 1], this->m_count - index - 1);
    TArrayCopy::Destruct(&this->m_data[--this->m_count]);
}

//===========================================================================
template<class T>
void TArray<T>::DeleteUnordered (unsigned index) {
    ASSERT(index < this->m_count);
    if (index + 1 < this->m_count)
        TArrayCopy::Assign(&this->m_data[index], &this->m_data[this->m_count - 1], 1);
    TArrayCopy::Destruct(&this->m_data[--this->m_count]);
}

//===========================================================================
template<class T>
T * TArray<T>::Detach () {
    T * result = m_data;
    m_data  = nil;
    m_alloc = 0;
    m_count = 0;
    return result;
}

//===========================================================================
template<class T>
void TArray<T>::Fill (uint8_t value) {
    TArrayCopy::Destruct(m_data, m_count);
    memset(m_data, value, m_count * sizeof(T));
    TArrayCopy::Construct(m_data, m_count);
}

//===========================================================================
template<class T>
void TArray<T>::GrowToCount (unsigned count, bool zero) {
    if (count <= this->m_count)
        return;
    AdjustSizeChunked(count, this->m_count);
    if (zero)
        memset(this->m_data + this->m_count, 0, (count - this->m_count) * sizeof(T));
    TArrayCopy::Construct(this->m_data + this->m_count, count - this->m_count);
    this->m_count = count;
}

//===========================================================================
template<class T>
void TArray<T>::GrowToFit (unsigned index, bool zero) {
    GrowToCount(index + 1, zero);
}

//===========================================================================
template<class T>
void TArray<T>::ShrinkBy (unsigned count) {
    ASSERT(count <= this->m_count);
    TArrayCopy::Destruct(this->m_data + this->m_count - count, count);
    this->m_count -= count;
}

//===========================================================================
template<class T>
void TArray<T>::Move (unsigned destIndex, unsigned sourceIndex, unsigned count) {

    // Copy the data to the destination
    ASSERT(destIndex +   count <= this->m_count);
    ASSERT(sourceIndex + count <= this->m_count);
    TArrayCopy::Assign(this->m_data + destIndex, this->m_data + sourceIndex, count);

    // Remove it from the source
    if (destIndex >= sourceIndex) {
        TArrayCopy::Destruct(this->m_data + sourceIndex, std::min(count, destIndex - sourceIndex));
        TArrayCopy::Construct(this->m_data + sourceIndex, std::min(count, destIndex - sourceIndex));
    }
    else {
        unsigned overlap = (destIndex + count > sourceIndex) ? (destIndex + count - sourceIndex) : 0;
        ASSERT(overlap <= count);
        TArrayCopy::Destruct(this->m_data + sourceIndex + overlap, count - overlap);
        TArrayCopy::Construct(this->m_data + sourceIndex + overlap, count - overlap);
    }

}

//===========================================================================
template<class T>
T * TArray<T>::New () {
    AdjustSizeChunked(this->m_count + 1, this->m_count + 1);
    return &this->m_data[this->m_count - 1];
}

//===========================================================================
template<class T>
T * TArray<T>::New (unsigned count) {
    AdjustSizeChunked(this->m_count + count, this->m_count + count);
    return &this->m_data[this->m_count - count];
}

//===========================================================================
template<class T>
T * TArray<T>::Ptr () {
    return m_data;
}

//===========================================================================
template<class T>
const T * TArray<T>::Ptr () const {
    return m_data;
}

//===========================================================================
template<class T>
void TArray<T>::Push (const T & source) {
    AdjustSizeChunked(this->m_count + 1, this->m_count);
    TArrayCopy::CopyConstruct(&this->m_data[this->m_count], source);
    ++this->m_count;
}

//===========================================================================
template<class T>
T TArray<T>::Pop () {
    ASSERT(this->m_count);
    T result = this->m_data[--this->m_count];
    TArrayCopy::Destruct(this->m_data + this->m_count);
    return result;
}

//===========================================================================
template<class T>
void TArray<T>::Reserve (unsigned additionalCount) {
    AdjustSizeChunked(std::max(this->m_alloc, this->m_count + additionalCount), this->m_count);
}

//===========================================================================
template<class T>
void TArray<T>::Set (const T * source, unsigned count) {
    AdjustSizeChunked(count, 0);
    TArrayCopy::CopyConstruct(this->m_data, source, count);
    this->m_count = count;
}

//===========================================================================
template<class T>
void TArray<T>::SetArray (const TArray<T> & source) {
    AdjustSize(source.m_count, 0);
    TArrayCopy::CopyConstruct(m_data, source.m_data, source.m_count);
    m_count = source.m_count;
}

//===========================================================================
template<class T>
void TArray<T>::SetChunkSize (unsigned chunkSize) {
    this->m_chunkSize = chunkSize;
}

//===========================================================================
template<class T>
void TArray<T>::SetCount (unsigned count) {
    AdjustSizeChunked(std::max(this->m_alloc, count), count);
}

//===========================================================================
template<class T>
void TArray<T>::SetCountFewer (unsigned count) {
    ASSERT(count <= this->m_count);
    TArrayCopy::Destruct(this->m_data + count, this->m_count - count);
    this->m_count = count;
}

//===========================================================================
template<class T>
T * TArray<T>::Term () {
    return m_data + m_count;
}

//===========================================================================
template<class T>
const T * TArray<T>::Term () const {
    return m_data + m_count;
}

//===========================================================================
template<class T>
T * TArray<T>::Top () {
    ASSERT(m_count);
    return m_data + m_count - 1;
}

//===========================================================================
template<class T>
const T * TArray<T>::Top () const {
    ASSERT(m_count);
    return m_data + m_count - 1;
}

//===========================================================================
template<class T>
void TArray<T>::Trim () {
    this->AdjustSize(this->m_count, this->m_count);
}

//===========================================================================
template<class T>
void TArray<T>::Zero () {
    TArrayCopy::Destruct(m_data, m_count);
    memset(m_data, 0, m_count * sizeof(T));
    TArrayCopy::Construct(m_data, m_count);
}

//===========================================================================
template<class T>
void TArray<T>::ZeroCount () {
    TArrayCopy::Destruct(m_data, m_count);
    m_count = 0;
}

//===========================================================================
template<class T>
void TArray<T>::ZeroRange (unsigned index, unsigned count) {
    ASSERT(index + count <= m_count);
    TArrayCopy::Destruct(m_data + index, count);
    memset(m_data + index, 0, count * sizeof(T));
    TArrayCopy::Construct(m_data + index, count);
}

#endif
