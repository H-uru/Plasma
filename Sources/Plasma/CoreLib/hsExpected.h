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

#ifndef hsExpected_inc
#define hsExpected_inc

#include "HeadSpin.h"

#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

/**
 * Replacement for std::monostate, because that's only guaranteed to be in <utility> since C++26,
 * and we don't want to include <variant> just for this trivial type.
 */
struct hsMonostate {};

template<typename E>
class hsBadExpectedAccess : public std::exception
{
    E fValue;

public:
    hsBadExpectedAccess() = delete;
    hsBadExpectedAccess(const hsBadExpectedAccess<E>&) = default;
    hsBadExpectedAccess(hsBadExpectedAccess<E>&&) = delete;
    explicit hsBadExpectedAccess(E unex)
        : fValue(std::move(unex))
    {
    }

    [[nodiscard]]
    E& Value() &
    {
        return fValue;
    }

    [[nodiscard]]
    const E& Value() const&
    {
        return fValue;
    }

    [[nodiscard]]
    E&& Value() &&
    {
        return std::move(fValue);
    }

    [[nodiscard]]
    const E&& Value() const&&
    {
        return std::move(fValue);
    }
};

template<typename E>
class hsUnexpected
{
    E fValue;

public:
    hsUnexpected() = delete;
    hsUnexpected(const hsUnexpected<E>&) = delete;
    hsUnexpected(hsUnexpected<E>&&) = default;
    hsUnexpected(E unex)
        : fValue(std::move(unex)) {}

    [[nodiscard]]
    E& Value() &
    {
        return fValue;
    }

    [[nodiscard]]
    const E& Value() const&
    {
        return fValue;
    }

    [[nodiscard]]
    E&& Value() &&
    {
        return std::move(fValue);
    }

    [[nodiscard]]
    const E&& Value() const&&
    {
        return std::move(fValue);
    }
};

template<typename T, typename E>
class hsExpected
{
    static_assert(!(std::is_void_v<T> || std::is_void_v<E>), "hsExpected cannot use void");
    static_assert(!(std::is_reference_v<T> || std::is_reference_v<E>), "hsExpected cannot use reference types");

    union
    {
        T fExpected;
        E fUnexpected;
    };
    bool fHasValue;

    inline void IThrowIfNoValue() const
    {
        if (!fHasValue)
            throw hsBadExpectedAccess(hsMonostate());
    }

    inline void IThrowIfNoValue()
    {
        if (!fHasValue)
            throw hsBadExpectedAccess<E>(std::move(fUnexpected));
    }

    inline void IThrowIfValue() const
    {
        if (fHasValue)
            throw hsBadExpectedAccess(hsMonostate());
    }

    inline void IThrowIfValue()
    {
        if (fHasValue)
            throw hsBadExpectedAccess<T>(std::move(fExpected));
    }

public:
    hsExpected() = delete;
    hsExpected(const hsExpected<T, E>&) = delete;
    hsExpected(hsExpected<T, E>&& move)
        : fHasValue(move.fHasValue)
    {
        if (fHasValue)
            fExpected = std::move(move.fExpected);
        else
            fUnexpected = std::move(move.fUnexpected);
    }

    hsExpected(T value)
        : fExpected(std::move(value)), fHasValue(true)
    {}

    hsExpected(hsUnexpected<E>&& unex)
        : fUnexpected(std::move(unex.Value())), fHasValue()
    {}

    ~hsExpected()
    {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            if (fHasValue)
                std::destroy_at(&fExpected);
        }
        if constexpr (!std::is_trivially_destructible_v<E>) {
            if (!fHasValue)
                std::destroy_at(&fUnexpected);
        }
    }

    [[nodiscard]]
    bool HasValue() const { return fHasValue; }

    [[nodiscard]]
    operator bool() const { return fHasValue; }

    [[nodiscard]]
    T& Value() &
    {
        IThrowIfNoValue();
        return fExpected;
    }

    [[nodiscard]]
    const T& Value() const &
    {
        IThrowIfNoValue();
        return fExpected;
    }

    [[nodiscard]]
    T&& Value() &&
    {
        IThrowIfNoValue();
        return std::move(fExpected);
    }

    [[nodiscard]]
    const T&& Value() const&&
    {
        IThrowIfNoValue();
        return std::move(fExpected);
    }

    [[nodiscard]]
    T* operator->()
    {
        hsAssert(fHasValue, "Dereferencing an hsExpected without the expected value");
        return &fExpected;
    }

    [[nodiscard]]
    const T* operator->() const
    {
        hsAssert(fHasValue, "Dereferencing an hsExpected without the expected value");
        return &fExpected;
    }

    [[nodiscard]]
    T& operator*() &
    {
        hsAssert(fHasValue, "Dereferencing an hsExpected without the expected value");
        return fExpected;
    }

    [[nodiscard]]
    const T& operator*() const &
    {
        hsAssert(fHasValue, "Dereferencing an hsExpected without the expected value");
        return fExpected;
    }

    [[nodiscard]]
    T&& operator*() &&
    {
        hsAssert(fHasValue, "Dereferencing an hsExpected without the expected value");
        return std::move(fExpected);
    }

    [[nodiscard]]
    const T&& operator*() const &&
    {
        hsAssert(fHasValue, "Dereferencing an hsExpected without the expected value");
        return std::move(fExpected);
    }

    [[nodiscard]]
    E& Error() &
    {
        IThrowIfValue();
        return fUnexpected;
    }

    [[nodiscard]]
    const E& Error() const &
    {
        IThrowIfValue();
        return fUnexpected;
    }

    [[nodiscard]]
    E&& Error() &&
    {
        IThrowIfValue();
        return std::move(fUnexpected);
    }

    [[nodiscard]]
    const E&& Error() const&&
    {
        IThrowIfValue();
        return std::move(fUnexpected);
    }
};

#endif // hsExpected_inc

