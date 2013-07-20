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

#ifndef _HS_ALIGNED_ALLOCATOR_H
#define _HS_ALIGNED_ALLOCATOR_H

#include "HeadSpin.h"
#include <stdexcept>

template<class T, size_t ALIGNMENT=16>

/**
 * An aligned allocator for storing SIMD ready values in STL containers
 * \remarks Based on https://gist.github.com/donny-dont/1471329
 */
class hsAlignedAllocator
{
    hsAlignedAllocator& operator=(const hsAlignedAllocator&) { }

public:
    template <typename U, size_t ALIGN=16>
    struct rebind
    {
        typedef hsAlignedAllocator<U, ALIGN> other;
    };

    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    hsAlignedAllocator() { }
    hsAlignedAllocator(const hsAlignedAllocator&) { }
    template <typename U> hsAlignedAllocator(const hsAlignedAllocator<U, ALIGNMENT>&) { }
    ~hsAlignedAllocator() { }

    pointer address(reference r) const { return &r; }
    const_pointer address(const_reference r) const { return &r; }

    pointer allocate(size_type size, const_pointer hint=nullptr)
    {
        if (size == 0)
            return nullptr;
        if (size > max_size())
            throw std::length_error("integer overflow");

#ifdef HS_BUILD_FOR_WIN32
        void* ptr = _aligned_malloc(size * sizeof(value_type), ALIGNMENT);
#else
        void* ptr = nullptr;
        posix_memalign(&ptr, ALIGNMENT, size * sizeof(value_type));
#endif // HS_BUILD_FOR_WIN32

        if (!ptr)
            throw std::bad_alloc();
        return static_cast<pointer>(ptr);
    }

    void construct(T* const p, const_reference t) const
    {
        void * const pv = static_cast<void *>(p);
        new (pv) value_type(t);
    }


    void deallocate(pointer ptr, size_type size)
    {
#ifdef HS_BUILD_FOR_WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif // HS_BUILD_FOR_WIN32
    }

    void destroy(T* const p) const
    {
        p->~T();
    }

    size_type max_size() const
    {
        return static_cast<size_t>(-1) / sizeof(value_type);
    }

    bool operator==(const hsAlignedAllocator& other) const { return true; }
};

#endif // _HS_ALIGNED_ALLOCATOR_H
