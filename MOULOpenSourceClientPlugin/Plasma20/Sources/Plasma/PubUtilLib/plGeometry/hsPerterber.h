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

#ifndef hsPerterber_inc
#define hsPerterber_inc

#include "../plResMgr/hsKeyedObject.h"
#include "hsGeometry3.h"

class hsStream;
class hsGShape3;
class hsBounds3Ext;
class hsGMesh;
class plPipeline;
struct hsMatrix44;
struct hsGVertex3;

class hsPerterber : public hsKeyedObject
{
public:
	enum {
		kTypeUndefined		= 0x0,
		kTypeOscillator		= 0x1
	};
protected:
	static		hsBool32	fDisabled;

	virtual void IUpdate(hsScalar secs, plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual void IPerterb(const hsPoint3& in, hsGVertex3& out) const = 0;

	hsGMesh* IGetMesh(hsGShape3* shape);
public:
	hsPerterber();
	virtual ~hsPerterber();

	static void SetDisabled(hsBool32 on) { fDisabled = on; }
	static void ToggleDisabled() { fDisabled = !fDisabled; }
	static hsBool32 GetDisabled() { return fDisabled; }

	virtual void Perterb(hsScalar secs, plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l, hsGShape3* shape);

	virtual void AdjustWorldBounds(const hsMatrix44& l2w, const hsMatrix44& w2l, hsBounds3Ext& bnd) const = 0;

	virtual UInt32 GetType() const = 0;

	virtual void Write(hsStream* s) = 0;
	virtual void Read(hsStream* s) = 0;

	virtual void Save(hsStream* s, hsScalar secs) = 0;
	virtual void Load(hsStream* s, hsScalar secs) = 0;

	void TimeStampAndSave(hsStream* s);
	void TimeStampAndLoad(hsStream* s);

	void LabelAndWrite(hsStream* s);
	static hsPerterber* CreateAndRead(hsStream* s);

	virtual void Init(Int32 nParams, hsScalar* params) = 0;

#if 0	// Used Registry...need to change paulg
	static void InitSystem(plResMgr* reg);

	static void Shutdown(plResMgr* reg);
#endif
};

#endif hsPerterber_inc
