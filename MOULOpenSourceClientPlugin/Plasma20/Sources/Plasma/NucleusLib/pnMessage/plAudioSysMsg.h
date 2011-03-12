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

#ifndef plAudioSysMsg_inc
#define plAudioSysMsg_inc

#include "plMessage.h"
#include "hsStream.h"
#include "hsResMgr.h"

class plKey;

class plAudioSysMsg : public plMessage
{
	int	fAudFlag;	
	plKey pObj;
	hsBool	fBoolFlag;
public:
	enum
	{
		kActivate	= 0,
		kDeActivate,
		kDestroy,
		kInitialize,
		kPing,
		kSetVol,
		kMuteAll,
		kUnmuteAll,
		kRequestMuteState,
		kChannelVolChanged,
		kRegisteringSound,
		kForceListUpdate
	};


	plAudioSysMsg() : pObj(nil){SetBCastFlag(plMessage::kBCastByExactType);}
	plAudioSysMsg(const plKey &s) : pObj(nil){SetBCastFlag(plMessage::kBCastByExactType);SetSender(s);}  
	plAudioSysMsg(int i) : pObj(nil){fAudFlag = i; SetBCastFlag(plMessage::kBCastByExactType );}  
	plAudioSysMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : pObj(nil){SetBCastFlag(plMessage::kBCastByExactType);}
	~plAudioSysMsg(){;}

	CLASSNAME_REGISTER( plAudioSysMsg );
	GETINTERFACE_ANY( plAudioSysMsg, plMessage );
	
	int GetAudFlag() { return fAudFlag; }
	plKey GetSceneObject() { return pObj; }
	void SetSceneObject(plKey &k) { pObj = k; }

	hsBool	GetBoolFlag() { return fBoolFlag; }
	void	SetBoolFlag( hsBool b ) { fBoolFlag = b; }

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		stream->WriteSwap(fAudFlag);
		mgr->WriteKey(stream, pObj);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->ReadSwap(&fAudFlag);
		pObj = mgr->ReadKey(stream);
	}
};

#endif // plAudioSysMsg
