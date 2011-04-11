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

#ifndef plLightMapComponent_inc
#define plLightMapComponent_inc

#include "plComponent.h"

#include "hsColorRGBA.h"

class plMipmap;

const Class_ID LIGHTMAP_COMP_CID(0x1b1d0317, 0x3b3821db);

class plLightMapComponent : public plComponent
{
protected:
	plKey		fLightMapKey;

public:
	plLightMapComponent();

	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	void SetLightMapKey(const plKey& key) { fLightMapKey = key; }
	plKey GetLightMapKey() const { return fLightMapKey; }

	float GetScale() const;
	UInt32 GetUVWSrc() const;

	hsBool GetCompress() const;
	hsBool GetShared() const;

	hsColorRGBA GetInitColor() const;
};

#endif // plLightMapComponent_inc

