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

#include <type_traits>

#include <strbasic.h>
#include <maxversion.h>

#include <string_theory/string>

#if MAX_VERSION_MAJOR <= 9
#   define BMMCOLOR(x, y, z, w) \
        {x, y, x, w};

#   define DEFAULTREMAP NoRemap()

#   define ENUMDEPENDENTS(maxObject, proc) \
        maxObject->EnumDependents(proc);

#   define INIT_CUSTOM_CONTROLS(instance) InitCustomControls(instance); InitCommonControls();

#   define DisableThreadLibraryCalls()

#   define MCHAR TCHAR
#   define MSTR TSTR
#else
#   define BMMCOLOR(x, y, z, w) \
        BMM_Color_64(x, y, z, w);

#   define DEFAULTREMAP DefaultRemapDir()

#   define ENUMDEPENDENTS(maxObject, proc) \
        maxObject->DoEnumDependents(proc);

#   define INIT_CUSTOM_CONTROLS(instance)
#endif // Max 9

#if MAX_VERSION_MAJOR <= 10 // Max 2008
#   define MAX10_CONST
#else
#   define MAX10_CONST const
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
#   define MAX14_CONST
#   define GETDLGTEXT_RETURN_TYPE TCHAR*
#   define READ_VOID_BUFFER(s) s->Read
#   define WRITE_VOID_BUFFER(s) s->Write
#   define BITMAP_LOAD_CONFIGURE_DATASIZE
#   define BMNAME_VALUE_TYPE TCHAR*
#else
#   define MAX14_CONST const
#   define GETDLGTEXT_RETURN_TYPE const MCHAR*
#   define READ_VOID_BUFFER(s) s->ReadVoid
#   define WRITE_VOID_BUFFER(s) s->WriteVoid
#   define BITMAP_LOAD_CONFIGURE_DATASIZE , DWORD piDataSize
#   define BMNAME_VALUE_TYPE const MCHAR*
#endif // MAX_VERSION_MAJOR

#if MAX_VERSION_MAJOR <= 15 // Max 2013
#   define USE_LANGUAGE_PACK_LOCALE(...)
#else
#   define USE_LANGUAGE_PACK_LOCALE(...) MaxSDK::Util::UseLanguagePackLocale(__VA_ARGS__)
#endif

#if MAX_VERSION_MAJOR <= 17 // Max 2015
#   define MAX_REF_INTERVAL Interval
#   define MAX_REF_PROPAGATE
#   define MAX_REF_PROPAGATE_VALUE TRUE
#else
#   define MAX_REF_INTERVAL const Interval&
#   define MAX_REF_PROPAGATE , BOOL propagate
#   define MAX_REF_PROPAGATE_VALUE propagate
#endif

#if MAX_VERSION_MAJOR < 24 // Max 2022
#   define MAX_NAME_LOCALIZED1
#   define MAX_NAME_LOCALIZED2
#   define MAX_NAME_LOCALIZED_DEFAULT
#   define MAX_NAME_LOCALIZED_VALUE
#   define MAX24_CONST

#   define plClassDesc2 ClassDesc2
#   define plClassDesc ClassDesc

#   define GetSystemUnitInfo GetMasterUnitInfo
#else
#   define MAX_NAME_LOCALIZED1 bool localized
#   define MAX_NAME_LOCALIZED2 , bool localized
#   define MAX_NAME_LOCALIZED_DEFAULT = true
#   define MAX_NAME_LOCALIZED_VALUE , localized
#   define MAX24_CONST const

#   include <iparamb2.h> // Dammit

class plClassDesc2 : public ClassDesc2
{
public:
    const MCHAR* NonLocalizedClassName() override { return ClassName(); }
};

class plClassDesc : public ClassDesc
{
public:
    const MCHAR* NonLocalizedClassName() override { return ClassName(); }
};
#endif

// Old versions of Max define this as an integer, not a Class_ID
#define XREFOBJ_COMPAT_CLASS_ID Class_ID(0x92aab38c, 0)

// Special 3ds Max message box support added in 2021 for HiDPI
#if MAX_VERSION_MAJOR >= 23
#   define plMaxMessageBox  MaxSDK::MaxMessageBox
#else
#   define plMaxMessageBox MessageBox
#endif

// Limitation: MCHAR and TCHAR must always be the same type.
static_assert(std::is_same_v<MCHAR, TCHAR>, "MCHAR and TCHAR must have the same underlying type");

#ifndef _M
#   define _M(x) _T(x)
#endif
#ifndef M_STD_STRING
#   define M_STD_STRING std::string
#endif

// Analagous to Max's M2T, etc. except doesn't require ATL
#ifdef MCHAR_IS_WCHAR
#   define ST2M(x) x.to_wchar().data()
#   define M2ST(x) (x ? ST::string::from_wchar(x) : ST::string())
#else
#   define ST2M(x) x.to_latin_1().data()
#   define M2ST(x) (x ? ST::string::from_latin_1(x) : ST::string())
#endif

#ifdef UNICODE
#   define ST2T(x) x.to_wchar().data()
#   define T2ST(x) (x ? ST::string::from_wchar(x) : ST::string())
#else
#   define ST2T(x) x.to_latin_1().data()
#   define T2ST(x) (x ? ST::string::from_latin_1(x) : ST::string())
#endif

// Nonstandard, but useful
#ifndef M_STD_STRINGSTREAM
#   if defined(UNICODE) || defined(MCHAR_IS_WCHAR)
#       define M_STD_STRINGSTREAM std::wstringstream
#   else
#       define M_STD_STRINGSTREAM std::stringstream
#   endif
#endif

#endif // _PLASMA_MAXCOMPAT_H
