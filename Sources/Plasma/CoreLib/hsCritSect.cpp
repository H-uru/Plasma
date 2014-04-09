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
#pragma hdrstop

#include "hsCritSect.h"
#include "hsWindows.h"
#include <mutex>

// Sure, go ahead and uncomment...  YOU try and figure this insanity out...
//#define CRITSECT_WTF

/****************************************************************************
*
*   Critical section implementation
*
***/

//===========================================================================
CCritSect::CCritSect () {
#ifdef CRITSECT_WTF
    m_handle = new std::recursive_mutex;
#else
    m_handle = new CRITICAL_SECTION;
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
#endif
}

//===========================================================================
CCritSect::~CCritSect () {
#ifdef CRITSECT_WTF
    if (!reinterpret_cast<std::recursive_mutex*>(m_handle)->try_lock())
        hsAssert(0, "CCritSect destroyed while still locked!");
    else
        reinterpret_cast<std::recursive_mutex*>(m_handle)->unlock();

    delete reinterpret_cast<std::recursive_mutex*>(m_handle);
#else
    DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
    delete reinterpret_cast<CRITICAL_SECTION*>(m_handle);
#endif
}

//===========================================================================
void CCritSect::Enter () {
#ifdef CRITSECT_WTF
    reinterpret_cast<std::recursive_mutex*>(m_handle)->lock();
#else
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
#endif
}

//===========================================================================
void CCritSect::Leave () {
#ifdef CRITSECT_WTF
    reinterpret_cast<std::recursive_mutex*>(m_handle)->unlock();
#else
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_handle));
#endif
}
