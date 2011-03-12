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

#ifndef plDynaRippleMgr_inc
#define plDynaRippleMgr_inc

#include "plDynaDecalMgr.h"

class plArmatureUpdateMsg;

class plDynaRippleMgr : public plDynaDecalMgr
{
protected:
	hsVector3					fInitUVW;
	hsVector3					fFinalUVW;

	virtual hsBool		IRippleFromShape(const plPrintShape* shape, hsBool force=false);

	virtual int			INewDecal();
public:
	plDynaRippleMgr();
	virtual ~plDynaRippleMgr();

	CLASSNAME_REGISTER( plDynaRippleMgr );
	GETINTERFACE_ANY( plDynaRippleMgr, plDynaDecalMgr );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	void SetUVWAnim(const hsVector3& init, const hsVector3& final) { fInitUVW = init; fFinalUVW = final; }
	const hsVector3& GetInitUVW() const { return fInitUVW; }
	const hsVector3& GetFinalUVW() const { return fFinalUVW; }
};


#endif // plDynaRippleMgr_inc
