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
//																			//
//	plCommonObjLib - Base class for a library of objects that are stored	//
//					 in common pages (for now, that means Textures or		//
//					 BuiltIn pages).										//
//																			//
//// How To Use //////////////////////////////////////////////////////////////
//																			//
//	Derive from this class and override the IsInteresting function to		//
//	filter out which objects you're interested in. Then, create an object	//
//	of this type (static is fine) before the export process begins.			//
//																			//
//	Then, when you are going to create an object in a common page, call		//
//	this lib to see if the object is already created. If so, it'll already	//
//	be loaded and ready to go, and you shouldn't create another one. Of		//
//	course, you always have the option of deleting it and creating another.	//
//																			//
//	The single biggest limitation is that objects in common pages can NOT	//
//	have ANY refs to ANY other objects in ANY other page. Otherwise, their	//
//	refs will get completely screwed up at export time. So don't do it!!!	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plCommonObjLib_h
#define _plCommonObjLib_h

#include "hsTemplates.h"


//// Class Definition /////////////////////////////////////////////////////////

class plCommonObjLibList;
class hsKeyedObject;
class plKey;
class plCommonObjLib
{
	protected:

		hsTArray<hsKeyedObject *>	fObjects;

	public:

		plCommonObjLib();
		virtual ~plCommonObjLib();

		/// Base utility functions

		void			AddObject( hsKeyedObject *object );
		hsBool			RemoveObjectAndKey( plKey &key );
		hsKeyedObject	*FindObject( const char *name, UInt16 classType = (UInt16)-1 );
		void			ClearObjectList( void );

		/// THIS IS YOUR VIRTUAL HERE. Override this to define which objects you collect
		virtual hsBool	IsInteresting( const plKey &objectKey ) { return false; }


		/// Static functions for use only by the export resManager
		static UInt32			GetNumLibs( void );
		static plCommonObjLib	*GetLib( UInt32 idx );


	private:

		static plCommonObjLibList	*fLibList;
};

#endif //_plCommonObjLib_h
