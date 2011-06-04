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
#ifndef plExcludeRegionMsg_inc
#define plExcludeRegionMsg_inc

#include "../pnMessage/plMessage.h"

class hsStream;

class plExcludeRegionMsg : public plMessage
{
public:
	enum CmdType
	{
		kClear,		// Moves all avatars from the affected region. Once they are gone, It sends a
					// callback message and turns the region into a solid object that avatars
					// cannot penetrate.
		kRelease	// Makes the xRegion not solid anymore
	};

protected:
	UInt8 fCmd;

public:
	plExcludeRegionMsg() : fCmd(kClear), fSynchFlags(0) {}
	plExcludeRegionMsg(const plKey &s, const plKey &r, const double* t) : fCmd(kClear), fSynchFlags(0) {}
	~plExcludeRegionMsg() {}

	CLASSNAME_REGISTER(plExcludeRegionMsg);
	GETINTERFACE_ANY(plExcludeRegionMsg, plMessage);
	
	void SetCmd(CmdType cmd) { fCmd = cmd; }
	UInt8 GetCmd() { return fCmd; }

	UInt32 fSynchFlags;
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fCmd = stream->ReadByte();
		fSynchFlags = stream->ReadSwap32();
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteByte(fCmd);
		stream->WriteSwap32(fSynchFlags);
	}
};

#endif // plExcludeRegionMsg_inc
