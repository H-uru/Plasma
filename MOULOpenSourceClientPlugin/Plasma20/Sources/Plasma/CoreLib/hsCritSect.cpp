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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/CoreLib/hsCritSect.cpp
*   
*
*   By Eric Anderson (10/23/2005)
*   Copyright 2005 Cyan Worlds, Inc.
*
***/

#include "HeadSpin.h"
#include "hsCritSect.h"
#pragma hdrstop


/****************************************************************************
*
*   Critical section implementation
*
***/

#ifdef HS_BUILD_FOR_WIN32
//===========================================================================
CCritSect::CCritSect () {
    InitializeCriticalSection(&m_handle);
}

//===========================================================================
CCritSect::~CCritSect () {
    DeleteCriticalSection(&m_handle);
}

//===========================================================================
void CCritSect::Enter () {
    EnterCriticalSection(&m_handle);
}

//===========================================================================
void CCritSect::Leave () {
    LeaveCriticalSection(&m_handle);
}
#endif
