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

#ifndef _pfStackWalker_h_
#define _pfStackWalker_h_

#include "HeadSpin.h"
#include "plFileSystem.h"

#include <iterator>
#include <optional>
#include <memory>
#include <string_theory/string>

struct plStackFrame
{
    virtual ~plStackFrame() = default;
};

struct plStackEntry
{
    ST::string fModuleName;
    ST::string fFunctionName;
    uint64_t   fOffset{};
    plFileName fFileName;
    std::optional<uint32_t> fLine;
};

/**
 * An STL-like container representing a stack trace.
 * Objects of this class can be used to walk an arbitrary stack trace using
 * range based for loops or with a classic STL forward iterator.
 * \note The implementations are platform specific, but they generally assume
 * that memory can still be allocated from the heap and that there will only
 * be one thread using the class at a time.
 */
class plStackWalker
{
public:
#ifdef HS_BUILD_FOR_WIN32
    using Process = HANDLE;
    using Thread = HANDLE;
#else
    using Process = int;
    using Thread = int;
#endif

private:
    Process fProcess;
    Thread  fThread;
    void*   fContext;
    size_t  fNumLevels;

public:
    class iterator
    {
        const plStackWalker* fThis;
        std::shared_ptr<plStackFrame> fFrame;
        size_t fLevelIdx;

        friend class plStackWalker;

    public:
        using difference_type = size_t; // standards compliance only, don't do this.
        using reference = plStackEntry&;
        using value_type = plStackEntry;

    protected:
        iterator()
            : fThis(), fLevelIdx((size_t)-1)
        { }
        iterator(const plStackWalker* sw, std::shared_ptr<plStackFrame> frame, size_t level)
            : fThis(sw), fFrame(std::move(frame)), fLevelIdx(level)
        { }

        value_type get() const;
        bool next();

    public:
        iterator(const iterator&) = default;
        iterator(iterator&&) = default;
        ~iterator() = default;

        iterator& operator =(const iterator& rhs) = default;
        iterator& operator =(iterator&& rhs) = default;

        iterator& operator ++()
        {
            if (next())
                fLevelIdx++;
            else
                *this = fThis->cend();
            return *this;
        }

        value_type operator *() const { return get(); };
        value_type operator ->() const { return get(); };

        bool operator <(const iterator& rhs) const
        {
            hsAssert(fThis == rhs.fThis, "mismatched iterators");
            return fLevelIdx < rhs.fLevelIdx;
        }

        bool operator ==(const iterator& rhs) const
        {
            hsAssert(fThis == rhs.fThis, "mismatched iterators");
            return fLevelIdx == rhs.fLevelIdx;
        }

        bool operator !=(const iterator& rhs) const
        {
            hsAssert(fThis == rhs.fThis, "mismatched iterators");
            return fLevelIdx != rhs.fLevelIdx;
        }
    };

public:
    plStackWalker() = delete;

    /**
     * Constructs a stack trace for the specified process and thread.
     * \param process[in] OS specific handle to the process being traced.
     * \param thread[in] OS specific handle to the thread being traced.
     * \param context[in] OS specific thread execution context data.
     * \param levels[in] Maximum number of stack levels to iterate.
     * \remarks This class is primarily designed for performing out of
     * process traces for maximum reliability and safety. However, you
     * must take care that the OS specific context information is available
     * in the calling process's address space such that processes that use
     * this functionality will not crash with an access violation or
     * segmentation fault. On Windows, this can be accomplished using
     * `ReadProcessMemory()` to copy the context information. This must
     * be done before constructing any plStackWalker objects.
     */
    plStackWalker(Process process, Thread thread, void* context, size_t levels = (size_t)-1);
    virtual ~plStackWalker();

    iterator begin() { return cbegin(); };
    iterator cbegin() const;

    iterator end() { return cend(); };
    iterator cend() const
    {
        return iterator(this, nullptr, fNumLevels);
    }
};

// For usage in STL algorithms, such as `std::copy()`.
namespace std
{
    template<>
    class iterator_traits<::plStackWalker::iterator>
    {
        public:
        using difference_type = ::plStackWalker::iterator::difference_type;
        using iterator_category = forward_iterator_tag;
        using reference = ::plStackWalker::iterator::reference;
        using value_type = ::plStackWalker::iterator::value_type;
    };
};

#endif
