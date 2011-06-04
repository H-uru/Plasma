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
#ifndef plMultistageBehMod_h_inc
#define plMultistageBehMod_h_inc

#include "../pnModifier/plSingleModifier.h"
#include "hsStlUtils.h"

class plAnimStageVec;

class plMultistageBehMod : public plSingleModifier
{
protected:
	plAnimStageVec* fStages;
	bool fFreezePhys;
	bool fSmartSeek;
	bool fReverseFBControlsOnRelease;

	bool fNetProp;
	bool fNetForce;

	std::vector<plKey> fReceivers;

	void IDeleteStageVec();
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return true; }

public:
	plMultistageBehMod();
	plMultistageBehMod(plAnimStageVec* stages, bool freezePhys, bool smartSeek, bool reverseFBControlsOnRelease, std::vector<plKey>* receivers);
	virtual ~plMultistageBehMod();
	
	CLASSNAME_REGISTER( plMultistageBehMod );
	GETINTERFACE_ANY( plMultistageBehMod, plSingleModifier );

	hsBool NetProp() { return fNetProp; }
	hsBool NetForce() { return fNetForce; }

	void SetNetProp(hsBool netProp) { fNetProp = netProp; }
	void SetNetForce(hsBool netForce) { fNetForce = netForce; }
	
	hsBool MsgReceive(plMessage* msg);

	virtual void Init(plAnimStageVec *stages, bool freezePhys, bool smartSeek, bool reverseFBControlsOnRelease, std::vector<plKey>* receivers);

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};

#endif // plMultistageBehMod_h_inc
