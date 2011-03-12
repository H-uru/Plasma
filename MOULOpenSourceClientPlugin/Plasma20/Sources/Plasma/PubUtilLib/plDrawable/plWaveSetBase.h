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

#ifndef plWaveSetBase_inc
#define plWaveSetBase_inc

#include "../pnModifier/plMultiModifier.h"
#include "hsGeometry3.h"

class hsGMaterial;
class plRipVSConsts;

class plWaveSetBase : public plMultiModifier
{
	virtual int IShoreRef() const = 0;
	virtual int IDecalRef() const = 0;

public:
	plWaveSetBase();
	virtual ~plWaveSetBase();

	CLASSNAME_REGISTER( plWaveSetBase );
	GETINTERFACE_ANY( plWaveSetBase, plMultiModifier );

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

	Int32		GetNumProperties() const { return 0; }

	virtual hsBool		SetupRippleMat(hsGMaterial* mat, const plRipVSConsts& ripConsts) = 0;
	virtual hsScalar	GetHeight() const = 0;
	virtual hsVector3	GetWindDir() const = 0;


	void			AddShore(plKey soKey);
	void			AddDecal(plKey key);

};

#endif // plWaveSetBase_inc
