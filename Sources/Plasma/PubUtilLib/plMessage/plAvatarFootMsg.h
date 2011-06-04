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

#ifndef plAvatarFootMsg_inc
#define plAvatarFootMsg_inc

#include "../pnMessage/plEventCallbackMsg.h"

class hsStream;
class hsResMgr;
class plArmatureMod;
class plAvBrain;

class plAvatarFootMsg : public plEventCallbackMsg
{
protected:
	hsBool			fIsLeft;
	plArmatureMod*	fArmature;
public:
	plAvatarFootMsg() 
	{
		fEvent = kTime;
		SetBCastFlag(plMessage::kBCastByExactType);
	}
	plAvatarFootMsg(const plKey& s, plArmatureMod *armature, hsBool isLeft) : plEventCallbackMsg(s, nil, nil), fArmature(armature), fIsLeft(isLeft) 
	{
		fEvent = kTime;
		SetBCastFlag(plMessage::kBCastByExactType);
	}

	CLASSNAME_REGISTER( plAvatarFootMsg );
	GETINTERFACE_ANY( plAvatarFootMsg, plEventCallbackMsg );

	virtual void Read(hsStream *stream, hsResMgr *mgr)
	{
		hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
	}
	virtual void Write(hsStream *stream, hsResMgr *mgr)
	{
		hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
	}

	hsBool IsLeft() const { return fIsLeft; }
	void SetIsLeft(hsBool on) { fIsLeft = (0 != on); }

	plArmatureMod* GetArmature() const { return fArmature; }
	void SetArmature(plArmatureMod* a) { fArmature = a; }

};

#endif // plAvatarFootMsg_inc
