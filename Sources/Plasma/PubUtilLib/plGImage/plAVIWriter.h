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
#ifndef plAVIWriter_h_inc
#define plAVIWriter_h_inc

#include "../pnKeyedObject/hsKeyedObject.h"

class plPipeline;

class plAVIWriter : public hsKeyedObject
{
protected:
	static bool fInitialized;

	virtual ~plAVIWriter();

public:
	static plAVIWriter& Instance();

	// If IsInitialized returns true, you need to call Shutdown before clearing
	// the registry (dang key).
	static bool IsInitialized() { return fInitialized; }
	virtual void Shutdown()=0;

	CLASSNAME_REGISTER(plAVIWriter);
	GETINTERFACE_ANY(plAVIWriter, hsKeyedObject);

	virtual bool Open(const char* fileName, plPipeline* pipeline)=0;
	virtual void Close()=0;
};

#endif // plAVIWriter_h_inc