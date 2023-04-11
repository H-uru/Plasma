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

#ifndef hsPoolVector_inc
#define hsPoolVector_inc

#include <vector>

// This helper class was borne out of the pl*ShadowMaster classes' abuse
// of the implementation details of hsTArray<T>. This pool type allows a
// user to track a subset of "in-use" items separately from the full set
// of allocated items in a collection. TODO: Eliminate the need for this
// ugly hack altogether.
template <class T>
class hsPoolVector
{
public:
    hsPoolVector() : fUsed() { }

    // Access the underlying pool
    std::vector<T>& pool() { return fPool; }
    const std::vector<T>& pool() const { return fPool; }

    size_t size() const noexcept { return fUsed; }
    bool empty() const noexcept { return fUsed == 0; }

    T& front() { return fPool.front(); }
    const T& front() const { return fPool.front(); }
    T& back() { return fPool[fUsed - 1]; }
    const T& back() const { return fPool[fUsed - 1]; }

    T& operator[](size_t pos) { return fPool[pos]; }
    const T& operator[](size_t pos) const { return fPool[pos]; }

    /**
     * Return an existing item after the last used, or add a new
     * element with the specified createItem() callback if we're
     * already at capacity in the underlying storage.
     */
    template <class CreateItem>
    T& next(CreateItem createItem)
    {
        if (fUsed == fPool.size()) {
            fUsed++;
            return fPool.emplace_back(createItem());
        }
        return fPool[fUsed++];
    }

    void pop_back() noexcept
    {
        fUsed--;
    }

    /**
     * Clear the in-use count, so the next element is picked from the
     * beginning of the pool.
     */
    void clear() noexcept
    {
        fUsed = 0;
    }

    /**
     * Reset both the in-use count and the underlying storage back to empty,
     * so the next element is guaranteed to be freshly created.
     */
    void release_and_clear() noexcept
    {
        fPool.clear();
        fUsed = 0;
    }

private:
    std::vector<T>  fPool;
    size_t          fUsed;
};

#endif // hsPoolVector_inc
