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
#ifndef plClientMsg_inc
#define plClientMsg_inc

#include "../pnMessage/plMessage.h"
#include "../pnMessage/plRefMsg.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "hsUtils.h"
#include "hsStlUtils.h"
#include "../pnKeyedObject/plUoid.h"

//
// Handles various types of client (app) msgs, relating 
// to loading rooms, players, camera, and progress bars
//
class plClientMsg : public plMessage
{
	int fMsgFlag;
	char* fAgeName;
	std::vector<plLocation> fRoomLocs;

	void IReset();

	class GraphicsSettings
	{
	public:
		GraphicsSettings() : fWidth (800), fHeight(600), fColorDepth(32), fWindowed(false), fNumAASamples(0),
							 fMaxAnisoSamples(0), fVSync(false) {}
		int fWidth;
		int fHeight;
		int fColorDepth;
		hsBool fWindowed;
		int fNumAASamples;
		int fMaxAnisoSamples;
		hsBool fVSync;
	};


public:
	enum
	{
		kLoadRoom,
		kLoadRoomHold,
		kUnloadRoom,
		kLoadNextRoom,	// For internal client use only

		kLoadAgeKeys,
		kReleaseAgeKeys,

		kQuit,				// exit the app
		kInitComplete,
		kDisableRenderScene,
		kEnableRenderScene,
		kResetGraphicsDevice,
		kSetGraphicsDefaults,
	};

	// graphics settings fields
	GraphicsSettings fGraphicsSettings;


	plClientMsg() { IReset();}
	plClientMsg(const plKey &s) { IReset();}  
	plClientMsg(int i) { IReset(); fMsgFlag = i; }  
	plClientMsg(const plKey &s, const plKey &r, const double* t) { IReset(); }
	~plClientMsg() { delete [] fAgeName; }

	CLASSNAME_REGISTER(plClientMsg);
	GETINTERFACE_ANY(plClientMsg, plMessage);

	int GetClientMsgFlag() const { return fMsgFlag; }

	void AddRoomLoc(plLocation loc);

	// Used for kLoadAgeKeys, kLetGoOfAgeKeys only
	const char*	GetAgeName() const { return fAgeName; }
	void		SetAgeName(const char* age) { delete [] fAgeName; fAgeName = hsStrcpy(age); }

	int GetNumRoomLocs() { return fRoomLocs.size(); }
	const plLocation& GetRoomLoc(int i) const { return fRoomLocs[i]; }
	const std::vector<plLocation>& GetRoomLocs() { return fRoomLocs; }

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

class plClientRefMsg : public plRefMsg
{

public:
	enum 
	{
		kLoadRoom	= 0,
		kLoadRoomHold,
		kManualRoom,
	};

	plClientRefMsg(): fType(-1), fWhich(-1) {};

	plClientRefMsg(const plKey &r, UInt8 refMsgFlags, Int8 which , Int8 type)
		: plRefMsg(r, refMsgFlags), fType(type), fWhich(which) {}


	CLASSNAME_REGISTER( plClientRefMsg );
	GETINTERFACE_ANY( plClientRefMsg, plRefMsg );

	Int8					fType;
	Int8					fWhich;

	// IO - not really applicable to ref msgs, but anyway
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plRefMsg::Read(stream, mgr);
		stream->ReadSwap(&fType);
		stream->ReadSwap(&fWhich);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plRefMsg::Write(stream, mgr);
		stream->WriteSwap(fType);
		stream->WriteSwap(fWhich);
	}
};


#endif // plClientMsg
