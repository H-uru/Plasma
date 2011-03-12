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

// Macros for the start and end of the class index list
#define CLASS_INDEX_LIST_START		const int KEYED_OBJ_DELINEATOR	= 512; \
									const int EXTERNAL_KEYED_DLL_BEGIN	= 256; \
									const int EXTERNAL_KEYED_DLL_END	= 511; \
									const int EXTERNAL_NONKEYED_DLL_BEGIN	= 1436; \
									const int EXTERNAL_NONKEYED_DLL_END	= 1536; \
									class plCreatableIndex { public: enum {
#define CLASS_INDEX_LIST_END	plNumClassIndices = EXTERNAL_NONKEYED_DLL_END + 1, }; };
// Macro to mark which class index is the start of the nonkeyed object section
#define CLASS_INDEX_NONKEYED_OBJ_START		kKeyedObjDelineator = KEYED_OBJ_DELINEATOR-1,

#define CLASS_INDEX_DATABASE_STRUCT_INDEXES_START	kDatabaseStructIndexesStart,
#define CLASS_INDEX_DATABASE_STRUCT_INDEXES_END		kDatabaseStructIndexesEnd=kDatabaseStructIndexesStart+100,


#endif // plClassIndexMacros_inc
