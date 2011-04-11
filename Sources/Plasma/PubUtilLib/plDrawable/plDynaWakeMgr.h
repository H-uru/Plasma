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

#ifndef plDynaWakeMgr_inc
#define plDynaWakeMgr_inc

#include "plDynaRippleMgr.h"

class plArmatureUpdateMsg;
class plAnimPath;

class plDynaWakeMgr : public plDynaRippleMgr
{
protected:
	hsVector3			fDefaultDir;
	plAnimPath*			fAnimPath;

	hsScalar			fAnimWgt;
	hsScalar			fVelWgt;

	virtual hsVector3	IGetDirection(const plDynaDecalInfo& info, const hsPoint3& pos) const;

	virtual hsBool		IRippleFromShape(const plPrintShape* shape, hsBool force=false);

	virtual int			INewDecal();
public:
	plDynaWakeMgr();
	virtual ~plDynaWakeMgr();

	CLASSNAME_REGISTER( plDynaWakeMgr );
	GETINTERFACE_ANY( plDynaWakeMgr, plDynaRippleMgr );

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void			SetAnimPath(plAnimPath* a);
	plAnimPath*		GetAnimPath() const { return fAnimPath; }

	void				SetDefaultDir(const hsVector3& v);
	const hsVector3&	GetDefaultDir() const { return fDefaultDir; }

	void				SetAnimWeight(hsScalar f) { fAnimWgt = f; }
	hsScalar			GetAnimWeight() const { return fAnimWgt; }

	void				SetVelocityWeight(hsScalar f) { fVelWgt = f; }
	hsScalar			GetVelocityWeight() const { return fVelWgt; }
};


#endif // plDynaWakeMgr_inc
