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
#ifndef plTimerCallbackManager_Defined
#define plTimerCallbackManager_Defined

#include "hsScalar.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsTemplates.h"

class plMessage;

class plTimerCallback 
{
public:
	
	plTimerCallback(double time, plMessage* pMsg);
	~plTimerCallback();
	
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	double		fTime;
	plMessage*	fMsg;
};

class plTimerCallbackManager : public hsKeyedObject
{
public:
	
	plTimerCallbackManager();
	~plTimerCallbackManager();

	CLASSNAME_REGISTER( plTimerCallbackManager );
	GETINTERFACE_ANY( plTimerCallbackManager, hsKeyedObject );

	virtual plTimerCallback* NewTimer(hsScalar time, plMessage* pMsg);
	hsBool CancelCallback(plTimerCallback* pTimer);
	hsBool CancelCallbacksToKey(const plKey& key);


	virtual hsBool MsgReceive(plMessage* msg);
		
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

private:
	
	hsTArray<plTimerCallback*>  fCallbacks;
};

class plgTimerCallbackMgr
{
private:
	static plTimerCallbackManager*		fMgr;
public:

	static void Init();
	static void Shutdown();
	static plTimerCallbackManager* Mgr() { return fMgr; }

	// External modifier use only
	static void SetTheTimerCallbackMgr(plTimerCallbackManager *mgr) { fMgr = mgr; }

	static plTimerCallback* NewTimer(hsScalar time, plMessage* pMsg) { return (fMgr->NewTimer(time, pMsg)); }
	static hsBool CancelCallback(plTimerCallback* pTimer);
	static hsBool CancelCallbacksToKey(const plKey& key);
};


#endif


