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
#ifndef plMtlCollector_h_inc
#define plMtlCollector_h_inc

#include "hsTypes.h"
#include <set>
#include "hsSTLSortUtils.h"

class Mtl;
class Texmap;
class plPlasmaMAXLayer;
class PBBitmap;

typedef std::set<Mtl*> MtlSet;
typedef std::set<Texmap*> TexSet;
typedef std::set<plPlasmaMAXLayer*> LayerSet;
typedef std::set<PBBitmap*> PBSet;
typedef std::set<const char*, stringISorter> TexNameSet;

class plMtlCollector
{
public:
	enum
	{
		kUsedOnly = 0x1,
		kPlasmaOnly = 0x2,
		kNoMultiMtl = 0x4,
		kClothingMtlOnly = 0x8,
		kNoSubMtls = 0x10,
	};

	static void GetMtls(MtlSet* mtls, TexSet* texmaps, UInt32 flags=0);

	static void GetMtlLayers(Mtl *mtl, LayerSet& layers);

	// Warning: These pointers are only valid until you return control to Max
	// (where a bitmap could be modified).  Don't hang on to them!
	static void GetAllTextures(TexNameSet& texNames);
};

#endif // plMtlCollector_h_inc