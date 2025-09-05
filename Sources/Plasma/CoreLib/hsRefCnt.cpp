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

#include "HeadSpin.h"

#include "hsExceptions.h"
#include "hsRefCnt.h"

#define REFCOUNT_DBG_NONE   0
#define REFCOUNT_DBG_REFS   1
#define REFCOUNT_DBG_LEAKS  2
#define REFCOUNT_DBG_ALL    3
#define REFCOUNT_DEBUGGING  REFCOUNT_DBG_NONE

#if (REFCOUNT_DEBUGGING == REFCOUNT_DBG_LEAKS) || (REFCOUNT_DEBUGGING == REFCOUNT_DBG_ALL)
#include <unordered_set>
#include <mutex>
#include <string_theory/format>
#include "hsDebug.h"
#include "hsLockGuard.h"

struct _RefCountLeakCheck
{
    std::unordered_set<hsRefCnt *> m_refs;
    unsigned m_added, m_removed;
    std::mutex m_mutex;

    ~_RefCountLeakCheck()
    {
        hsLockGuard(m_mutex);

        hsDebugPrintToTerminal(ST::format("Refs tracked:  {} created, {} destroyed\n",
                                          m_added, m_removed));
        if (m_refs.empty())
            return;

        hsDebugPrintToTerminal(ST::format("    {} objects leaked...\n", m_refs.size()));
        for (hsRefCnt *ref : m_refs) {
            hsDebugPrintToTerminal(ST::format("    {#08x} {}: {} refs remain\n",
                                              (uintptr_t)ref, typeid(*ref).name(), ref->RefCnt()));
        }
    }

    static _RefCountLeakCheck *_instance()
    {
        static _RefCountLeakCheck s_instance;
        return &s_instance;
    }

    static void add(hsRefCnt *ref)
    {
        _RefCountLeakCheck *this_p = _instance();
        hsLockGuard(this_p->m_mutex);
        ++this_p->m_added;
        this_p->m_refs.insert(ref);
    }

    static void del(hsRefCnt *ref)
    {
        _RefCountLeakCheck *this_p = _instance();
        hsLockGuard(this_p->m_mutex);
        ++this_p->m_removed;
        this_p->m_refs.erase(ref);
    }
};
#endif

hsRefCnt::hsRefCnt(int initRefs)
    : fRefCnt(initRefs)
{
#if (REFCOUNT_DEBUGGING == REFCOUNT_DBG_LEAKS) || (REFCOUNT_DEBUGGING == REFCOUNT_DBG_ALL)
    _RefCountLeakCheck::add(this);
#endif
}

hsRefCnt::~hsRefCnt()
{
#ifdef HS_DEBUGGING
    hsThrowIfFalse(fRefCnt == 1);
#endif

#if (REFCOUNT_DEBUGGING == REFCOUNT_DBG_LEAKS) || (REFCOUNT_DEBUGGING == REFCOUNT_DBG_ALL)
    _RefCountLeakCheck::del(this);
#endif
}

void hsRefCnt::UnRef(const char* tag)
{
#ifdef HS_DEBUGGING
    hsThrowIfFalse(fRefCnt >= 1);
#endif

#if (REFCOUNT_DEBUGGING == REFCOUNT_DBG_REFS) || (REFCOUNT_DEBUGGING == REFCOUNT_DBG_ALL)
    if (tag)
        hsDebugPrintToTerminal(ST::format("Dec {#x} {}: {}", reinterpret_cast<uintptr_t>(this), tag, fRefCnt - 1));
    else
        hsDebugPrintToTerminal(ST::format("Dec {#x}: {}", reinterpret_cast<uintptr_t>(this), fRefCnt - 1));
#endif

    if (fRefCnt == 1)   // don't decrement if we call delete
        delete this;
    else
        --fRefCnt;
}

void hsRefCnt::Ref(const char* tag)
{
#if (REFCOUNT_DEBUGGING == REFCOUNT_DBG_REFS) || (REFCOUNT_DEBUGGING == REFCOUNT_DBG_ALL)
    if (tag)
        hsDebugPrintToTerminal(ST::format("Inc {#x} {}: {}", reinterpret_cast<uintptr_t>(this), tag, fRefCnt + 1));
    else
        hsDebugPrintToTerminal(ST::format("Inc {#x}: {}", reinterpret_cast<uintptr_t>(this), fRefCnt + 1));
#endif

    ++fRefCnt;
}

void hsRefCnt::TransferRef(const char* oldTag, const char* newTag)
{
#if (REFCOUNT_DEBUGGING == REFCOUNT_DBG_REFS) || (REFCOUNT_DEBUGGING == REFCOUNT_DBG_ALL)
    hsDebugPrintToTerminal(ST::format("Inc {#x} {}: (xfer)", reinterpret_cast<uintptr_t>(this), newTag));
    hsDebugPrintToTerminal(ST::format("Dec {#x} {}: (xfer)", reinterpret_cast<uintptr_t>(this), oldTag));
#endif
}
