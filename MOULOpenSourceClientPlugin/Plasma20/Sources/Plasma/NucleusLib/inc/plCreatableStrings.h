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
//////////////////////////////////////////////////////////////////////////////
//
//	plCreatableStrings.h - Handy header file that declares a class with two
//						   static public string arrays representing the names
//						   of the classes before and after keyedObjDelineator
//						   in plCreatableIndex.h. Note the cunning (and humble)
//						   use of macros to avoid having to change this file 
//						   whenever plCreatableIndex.h changes.
//
//						   Should only be included once, probably in plFactory.cpp
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plCreatableStrings_h
#define _plCreatableStrings_h

// Step 0: Undefine plCreatableIndex_inc, since we're doing something sneaky with it

#undef plCreatableIndex_inc


// Step 1: Define plCreator_inc, so the plCreator.h include in plCI.h won't do anything

#define plCreator_inc


// Step 2: Define the CLASS_INDEX macros so we generate our class. The class name
//		   is plCreatableStrings and the arrays are fKeyedStrings and fNonKeyedStrings

#undef CLASS_INDEX_LIST_START
#undef CLASS_INDEX
#undef CLASS_INDEX_NONKEYED_OBJ_START
#undef CLASS_INDEX_LIST_END

#define CLASS_INDEX_LIST_START class plCreatableStrings { public:\
	static char	*fKeyedStrings[]; static char *fNonKeyedStrings[]; static char *fNonKeyedPostDBStrings[];\
	}; \
	char *plCreatableStrings::fKeyedStrings[] = {
#define CLASS_INDEX(ci) #ci
#define CLASS_INDEX_NONKEYED_OBJ_START		}; char *plCreatableStrings::fNonKeyedStrings[] = { 
#define CLASS_INDEX_LIST_END	};

#undef CLASS_INDEX_DATABASE_STRUCT_INDEXES_START
#undef CLASS_INDEX_DATABASE_STRUCT_INDEXES_END
#define CLASS_INDEX_DATABASE_STRUCT_INDEXES_START
#define CLASS_INDEX_DATABASE_STRUCT_INDEXES_END		}; char *plCreatableStrings::fNonKeyedPostDBStrings[] = { 

// Step 3: Include plCI.h

#include "plCreatableIndex.h"


#endif // _plCreatableStrings_h
