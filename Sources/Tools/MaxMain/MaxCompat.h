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

/* AUTHOR:  Adam Johnson
 * DATE:    April 16, 2011
 * SUMMARY: Cool helpers that let us target MANY versions of 3ds Max
 */

#ifndef _PLASMA_MAXCOMPAT_H
#define _PLASMA_MAXCOMPAT_H

#include <maxversion.h>

#if MAX_VERSION_MAJOR <= 9
#   define BMMCOLOR(x, y, z, w) \
        {x, y, x, w};

#   define DEFAULTREMAP NoRemap()

#   define ENUMDEPENDENTS(maxObject, proc) \
        maxObject->EnumDependents(proc);

    typedef TCHAR MCHAR;
#else
#   define BMMCOLOR(x, y, z, w) \
        BMM_Color_64(x, y, z, w);

#   define DEFAULTREMAP DefaultRemapDir()

#   define ENUMDEPENDENTS(maxObject, proc) \
        maxObject->DoEnumDependents(proc);
#endif //MAX_VERSION_MAJOR

#if MAX_VERSION_MAJOR <= 10 // Max 2008
#   define GETNAME_RETURN_TYPE TCHAR*
#   define SETTEXT_VALUE_TYPE MCHAR*
#else
#   define GETNAME_RETURN_TYPE const TCHAR*
#   define SETTEXT_VALUE_TYPE const MCHAR*
#endif

#if MAX_VERSION_MAJOR <= 11 // max 2009. Just a guess, really. 2010 doesn't need this function.
#   define INIT_CUSTOM_CONTROLS(instance) InitCustomControls(instance)
#else
#   define INIT_CUSTOM_CONTROLS(instance)
#endif

#if MAX_VERSION_MAJOR <= 13
#   define GetParamBlock2Controller(pb, id) pb->GetController(id)
#   define SetParamBlock2Controller(pb, id, tab, ctl) pb->SetController(id, tab, ctl)
#else
#   define GetParamBlock2Controller(pb, id) pb->GetControllerByID(id)
#   define SetParamBlock2Controller(pb, id, tab, ctl) pb->SetControllerByID(id, tab, ctl)
#endif // MAX_VERSION_MAJOR

#if MAX_VERSION_MAJOR <= 14 // Max 2012
#   define p_end end
#endif // MAX_VERSION_MAJOR

// Old versions of Max define this as an integer, not a Class_ID
#define XREFOBJ_COMPAT_CLASS_ID Class_ID(0x92aab38c, 0)

#endif // _PLASMA_MAXCOMPAT_H