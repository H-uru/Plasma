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
#ifndef PLASMA_MAX_H
#define PLASMA_MAX_H

class BitmapManager;
class Texmap;

#include "max.h"
#include "guplib.h"
#include "hsTypes.h"
#include <string>

#define	PLASMA_MAX_CLASSID Class_ID(0x3d494269, 0x103c5c5f)

extern ClassDesc* GetGUPDesc();

typedef void (*TextureSetFunc)(Texmap* texmap, int iBmp, UInt64 assetId);

struct TexInfo
{
	Texmap* texmap;
	int iBmp;
	std::string texName;
};

class PlasmaMax : public GUP
{
public:
	PlasmaMax();
	~PlasmaMax() {}

	// GUP Methods
	DWORD Start();
	void Stop();

	enum ControlVals
	{
		// Pass this to Control and get back a jvArray<TexInfo>* of all the textures in the scene
		kGetTextures,
		// Pass this to Control and get back a pointer to the TextureSetFunc
		kGetTextureSetFunc,
	};
	DWORD Control(DWORD parameter);
};

#endif