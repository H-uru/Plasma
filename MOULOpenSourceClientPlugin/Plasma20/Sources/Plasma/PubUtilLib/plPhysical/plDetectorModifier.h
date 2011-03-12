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

#ifndef plDetectorModifier_inc
#define plDetectorModifier_inc

#include "../pnModifier/plSingleModifier.h"
#include "../pnMessage/plObjRefMsg.h"
#include "hsStream.h"
#include "hsResMgr.h"

class plDetectorModifier : public plSingleModifier
{
protected:
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty){ return true; }

	hsTArray<plKey>		fReceivers;
	plModifier*			fRemoteMod;
	plKey				fProxyKey;

public:
	plDetectorModifier() : fRemoteMod(nil),fProxyKey(nil){;}
	virtual ~plDetectorModifier(){;}
	
//	virtual hsBool MsgReceive(plMessage* msg) = 0;

	CLASSNAME_REGISTER( plDetectorModifier );
	GETINTERFACE_ANY( plDetectorModifier, plSingleModifier );
	void AddLogicObj(plKey pKey) { fReceivers.Append(pKey); }
	void SetRemote(plModifier* p) { fRemoteMod = p; }
	plModifier* RemoteMod() { return fRemoteMod; }
	virtual void SetType(Int8 i) {;}
	int GetNumReceivers() const { return fReceivers.Count(); }
	plKey GetReceiver(int i) const { return fReceivers[i]; }
	void SetProxyKey(const plKey &k) { fProxyKey = k; }
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plSingleModifier::Read(stream, mgr);
		int n = stream->ReadSwap32();
		fReceivers.Reset();
		for(int i = 0; i < n; i++ )
		{	
			fReceivers.Append(mgr->ReadKey(stream));
		}
		mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier), plRefFlags::kActiveRef);
		fProxyKey = mgr->ReadKey(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plSingleModifier::Write(stream, mgr);
		stream->WriteSwap32(fReceivers.GetCount());
		for( int i = 0; i < fReceivers.GetCount(); i++ )
			mgr->WriteKey(stream, fReceivers[i]);
		
		mgr->WriteKey(stream, fRemoteMod);
		mgr->WriteKey(stream, fProxyKey);
	}
};


#endif plDetectorModifier_inc
