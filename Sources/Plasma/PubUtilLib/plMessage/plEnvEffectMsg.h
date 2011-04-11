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

#ifndef plEnvEffectMsg_inc
#define plEnvEffectMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsStream.h"

class hsResMgr;


class plEnvEffectMsg : public plMessage
{

	hsBool fEnable;

public:
	plEnvEffectMsg(){ SetBCastFlag(plMessage::kPropagateToModifiers); }
	
	plEnvEffectMsg(const plKey* s, 
					const plKey* r, 
					const double* t){SetBCastFlag(plMessage::kPropagateToModifiers);}

	~plEnvEffectMsg(){;}

	CLASSNAME_REGISTER( plEnvEffectMsg );
	GETINTERFACE_ANY( plEnvEffectMsg, plMessage );
	
	hsBool Enabled() { return fEnable; }
	void Enable(hsBool b) { fEnable = b; }
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		stream->ReadSwap(&fEnable);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwap(fEnable);
	}
};


class plEnvAudioEffectMsg : public plEnvEffectMsg
{
	UInt32 fPreset;

public:
	plEnvAudioEffectMsg(){SetBCastFlag(plMessage::kPropagateToModifiers);}
	plEnvAudioEffectMsg(const plKey* s, 
					const plKey* r, 
					const double* t){SetBCastFlag(plMessage::kPropagateToModifiers);}
	
	~plEnvAudioEffectMsg(){;}

	CLASSNAME_REGISTER( plEnvAudioEffectMsg );
	GETINTERFACE_ANY( plEnvAudioEffectMsg, plEnvEffectMsg );
	
	UInt32 GetEffect() { return fPreset; }
	void SetEffect(UInt32 i) { fPreset = i; }

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plEnvEffectMsg::Read(stream, mgr);
		stream->ReadSwap(&fPreset);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plEnvEffectMsg::Write(stream, mgr);
		stream->WriteSwap(fPreset);
	}
};

#endif // plEnvEffectMsg_inc
