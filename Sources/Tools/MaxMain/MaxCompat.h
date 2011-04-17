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

/* AUTHOR:  Adam Johnson
 * DATE:    April 16, 2011
 * SUMMARY: Cool helpers that let us target MANY versions of 3ds Max
 */

#ifndef _PLASMA_MAXCOMPAT_H
#define _PLASMA_MAXCOMPAT_H

#include "maxversion.h"

#if MAX_VERSION_MAJOR <= 9
#define BMMCOLOR(x, y, z, w) \
    {x, y, x, w};

#define DEFAULTREMAP NoRemap()

#define ENUMDEPENDENTS(maxObject, proc) \
    maxObject->EnumDependents(proc);

typedef TCHAR MCHAR;
#else
#define BMMCOLOR(x, y, z, w) \
    BMM_Color_64(x, y, z, w);

#define DEFAULTREMAP DefaultRemapDir()

#define ENUMDEPENDENTS(maxObject, proc) \
    maxObject->DoEnumDependents(proc);
#endif //MAX_VERSION_MAJOR

#endif // _PLASMA_MAXCOMPAT_H