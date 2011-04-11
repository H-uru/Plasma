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
#include "plSeekPointMod.h"

// local
#include "plAvatarMgr.h"

// CTOR()
plSeekPointMod::plSeekPointMod()
: fName(nil), plMultiModifier()
{
	// this constructor is called from the loader. 
}

// CTOR(char *)
plSeekPointMod::plSeekPointMod(char * name)
: fName(name),  plMultiModifier()
{
	// this constructor is called from the converter. it adds the seek point to the
	// registry immediately because it has the name already
}

// DTOR()
plSeekPointMod::~plSeekPointMod()
{
	if(fName) {
		delete[] fName;
		fName = nil;
	}
}

// MSGRECEIVE
hsBool plSeekPointMod::MsgReceive(plMessage* msg)
{
	return plMultiModifier::MsgReceive(msg);
}

// ADDTARGET
// Here I am. Announce presence to the avatar registry.
void plSeekPointMod::AddTarget(plSceneObject* so)
{
	plMultiModifier::AddTarget(so);
//	plAvatarMgr::GetInstance()->AddSeekPoint(this);
}

void plSeekPointMod::Read(hsStream *stream, hsResMgr *mgr)
{
	plMultiModifier::Read(stream, mgr);

	// read in the name of the animation itself
	int length = stream->ReadSwap32();
	if(length > 0)
	{
		fName = TRACKED_NEW char[length + 1];
		stream->Read(length, fName);
		fName[length] = 0;
	}

}

void plSeekPointMod::Write(hsStream *stream, hsResMgr *mgr)
{
	plMultiModifier::Write(stream, mgr);

	int length = strlen(fName);
	stream->WriteSwap32(length);
	if (length > 0)
	{
		stream->Write(length, fName);
	}

}