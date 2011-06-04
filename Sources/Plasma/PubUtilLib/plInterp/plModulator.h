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

#ifndef plModulator_inc
#define plModulator_inc

#include "../pnFactory/plCreatable.h"

struct hsMatrix44;
struct hsPoint3;
class plVolumeIsect;
class hsBounds3Ext;

class plModulator : public plCreatable
{
protected:
	plVolumeIsect*			fVolume;
	hsScalar				fSoftDist;

public:
	plModulator();
	virtual ~plModulator();

	CLASSNAME_REGISTER( plModulator );
	GETINTERFACE_ANY( plModulator, plCreatable );

	const plVolumeIsect*	GetVolume() const { return fVolume; }
	void					SetVolume(plVolumeIsect* vol); // Takes ownership, so don't delete after handing it in.

	hsScalar				Modulation(const hsPoint3& pos) const;
	hsScalar				Modulation(const hsBounds3Ext& bnd) const;

	void					SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	hsScalar				GetSoftDist() const { return fSoftDist; }
	void					SetSoftDist(hsScalar s) { fSoftDist = s; }

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);
};

#endif // plModulator_inc
