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

#ifndef plClassIndexMacros_inc
#define plClassIndexMacros_inc


// Macros for deriving the class index enum from the class name.
// These should always be used in place of 'plFooIDX'.
// Place before includes to avoid recursive include prob.
//  --For use within plCreatableIndex
#define CLASS_INDEX(plClassName) plClassName##IDX
//  --For use within plExternalCreatableIndex
#define EXTERN_CLASS_INDEX(plClassName) plClassName##IDX
//  --For use outside of plCreatableIndex
#define CLASS_INDEX_SCOPED(plClassName) plCreatableIndex::CLASS_INDEX(plClassName)
//  --For use outside of plExternalCreatableIndex
#define EXTERN_CLASS_INDEX_SCOPED(plClassName) plExternalCreatableIndex::EXTERN_CLASS_INDEX(plClassName)

static constexpr int EXTERNAL_KEYED_DLL_BEGIN      = 256;
static constexpr int EXTERNAL_KEYED_DLL_END        = 511;
static constexpr int KEYED_OBJ_DELINEATOR          = 512;
static constexpr int EXTERNAL_NONKEYED_DLL_BEGIN   = 1436;
static constexpr int EXTERNAL_NONKEYED_DLL_END     = 1536;

// Macros for the start and end of the class index list
#define CLASS_INDEX_LIST_START  class plCreatableIndex { public: enum {
#define CLASS_INDEX_LIST_END    plNumClassIndices = EXTERNAL_NONKEYED_DLL_END + 1, }; };
// Macro to mark which class index is the start of the nonkeyed object section
#define CLASS_INDEX_NONKEYED_OBJ_START      kKeyedObjDelineator = KEYED_OBJ_DELINEATOR-1,

#define CLASS_INDEX_DATABASE_STRUCT_INDEXES_START   kDatabaseStructIndexesStart,
#define CLASS_INDEX_DATABASE_STRUCT_INDEXES_END     kDatabaseStructIndexesEnd=kDatabaseStructIndexesStart+100,


#endif // plClassIndexMacros_inc
