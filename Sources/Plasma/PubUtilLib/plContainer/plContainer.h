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
#ifndef plContainer_h_inc
#define plContainer_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"

template < class T >
class plDataContainerT
{
	typedef std::map<UInt32,T*>	Items;
	Items		fItems;
	UInt32		fNextKey;
public:
	plDataContainerT()
	: fNextKey( 1 )
	{}
	UInt32 Add( T* data )
	{
		UInt32 key = fNextKey;
		fItems[ key ] = data;
		fNextKey++;
		return key;
	}
	bool Get( UInt32 key, T*& outItem, bool remove=true )
	{
		outItem = nil;
		Items::iterator ii = fItems.find( key );
		if ( ii==fItems.end() )
			return false;
		outItem = ii->second;
		if ( remove )
			fItems.erase( ii );
		return true;
	}
};


#endif // plContainer_h_inc
