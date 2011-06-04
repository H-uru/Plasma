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

#ifndef plRemoteAvatarInfoMsg_inc
#define plRemoteAvatarInfoMsg_inc

//
// this message is to fake out a gadget to see if it would potentially trigger...
//
#include "../pnMessage/plMessage.h"

class hsStream;
class hsResMgr;
class plKey;

class plRemoteAvatarInfoMsg : public plMessage
{
protected:

	plKey fAvatar;
public:
	plRemoteAvatarInfoMsg() : fAvatar(nil){SetBCastFlag(plMessage::kBCastByExactType);}
	plRemoteAvatarInfoMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : fAvatar(nil){SetBCastFlag(plMessage::kBCastByExactType);}
	
	CLASSNAME_REGISTER( plRemoteAvatarInfoMsg );
	GETINTERFACE_ANY( plRemoteAvatarInfoMsg, plMessage );
	
	void SetAvatarKey(plKey p) { fAvatar = p; }
	plKey GetAvatarKey() { return fAvatar; }
		
	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fAvatar = mgr->ReadKey(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		mgr->WriteKey(stream, fAvatar);
	}
};

#endif // plRemoteAvatarInfoMsg_inc
