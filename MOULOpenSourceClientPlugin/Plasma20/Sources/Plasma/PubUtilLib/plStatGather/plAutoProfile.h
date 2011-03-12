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
#ifndef plAutoProfile_h_inc
#define plAutoProfile_h_inc

#include "../pnKeyedObject/hsKeyedObject.h"

class plAutoProfile : public hsKeyedObject
{
public:
	CLASSNAME_REGISTER(plAutoProfile);
	GETINTERFACE_ANY(plAutoProfile, hsKeyedObject);

	static plAutoProfile* Instance();

	// If ageName is nil, do all ages
	virtual void StartProfile(const char* ageName = nil)=0;

	// For when we just want to link to each age, for other reasons (profiling load times)
	virtual void LinkToAllAges()=0;
};

#endif // plAutoProfile_h_inc
