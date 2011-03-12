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
#ifndef plApplyAvatarCustomizationsMsg_h
#define plApplyAvatarCustomizationsMsg_h

#error

#include "../pnMessage/plMessage.h"
#include "../plNetCommon/plNetAvatarVault.h"
#include "hsResMgr.h"


class plApplyAvatarCustomizationsMsg : public plMessage
{
public:
	CLASSNAME_REGISTER( plApplyAvatarCustomizationsMsg );
	GETINTERFACE_ANY( plApplyAvatarCustomizationsMsg, plMessage );

	void	SetCustomizations(const plPlayerCustomizations * value) { fCustomizations=*value;}
	const plPlayerCustomizations * GetCustomizations() const { return &fCustomizations;}
	void	SetAvatarKey(plKey * key) { fAvatarKey=key;}
	const plKey * GetAvatarKey() const { return fAvatarKey;}

	void Read(hsStream * stream, hsResMgr * mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fCustomizations.Read(stream);
		fAvatarKey = mgr->ReadKey(stream);
	}

	void Write(hsStream * stream, hsResMgr * mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		fCustomizations.Write(stream);
		mgr->WriteKey(stream,fAvatarKey);
	}
private:
	plPlayerCustomizations	fCustomizations;
	plKey *					fAvatarKey;
};


#endif //plApplyAvatarCustomizationsMsg_h
