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

#ifndef plEnableMsg_inc
#define plEnableMsg_inc

#include "plMessage.h"
#include "hsBitVector.h"

class hsStream;

class plEnableMsg : public plMessage
{

public:
	enum 
	{
		kDisable		= 0,
		kEnable,
		kDrawable,
		kPhysical,
		kAudible,
		kAll,
		kByType
	};
	
	hsBitVector		fCmd;
	hsBitVector		fTypes;

	hsBool Cmd(int n) const { return fCmd.IsBitSet(n); }
	void SetCmd(int n) { fCmd.SetBit(n); }
	void ClearCmd() { fCmd.Clear(); }
	
	void AddType(UInt16 t) { fTypes.SetBit(t); }
	void RemoveType(UInt16 t) { fTypes.ClearBit(t); }
	hsBool Type(UInt16 t) const { return fTypes.IsBitSet(t); }
	const hsBitVector& Types() const { return fTypes; }

	plEnableMsg() { }
	plEnableMsg(const plKey &s, int which , int type) : plMessage() 
	{ SetCmd(which); SetCmd(type); }

	CLASSNAME_REGISTER( plEnableMsg );
	GETINTERFACE_ANY( plEnableMsg, plMessage );

	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fCmd.Read(stream);
		fTypes.Read(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		fCmd.Write(stream);
		fTypes.Write(stream);
	}

	enum MsgContentFlags
	{
		kCmd,
		kTypes,
	};

	void ReadVersion(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgReadVersion(stream, mgr);
		hsBitVector contentFlags;
		contentFlags.Read(stream);

		if (contentFlags.IsBitSet(kCmd))
			fCmd.Read(stream);
		if (contentFlags.IsBitSet(kTypes))
			fTypes.Read(stream);
	}

	void WriteVersion(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWriteVersion(stream, mgr);
		hsBitVector contentFlags;
		contentFlags.SetBit(kCmd);
		contentFlags.SetBit(kTypes);
		contentFlags.Write(stream);
		
		fCmd.Write(stream);
		fTypes.Write(stream);
	}
};

#endif // plEnableMsg_inc
