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

#ifndef plDynaRippleVSMgr_inc
#define plDynaRippleVSMgr_inc

#include "plDynaRippleMgr.h"

class plWaveSetBase;

class plDynaRippleVSMgr : public plDynaRippleMgr
{
public:
	enum {
		kRefWaveSetBase		= kRefNextAvailable
	};
protected:

	plWaveSetBase*		fWaveSetBase;

	virtual hsBool		IRippleFromShape(const plPrintShape* shape, hsBool force=false);

	virtual int			INewDecal();

	virtual hsBool		ICheckRTMat();

public:
	plDynaRippleVSMgr();
	virtual ~plDynaRippleVSMgr();

	CLASSNAME_REGISTER( plDynaRippleVSMgr );
	GETINTERFACE_ANY( plDynaRippleVSMgr, plDynaRippleMgr );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);
};

#endif // plDynaRippleVSMgr_inc
