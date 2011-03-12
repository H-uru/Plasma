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


#ifndef plCluster_inc
#define plCluster_inc

#include "hsTemplates.h"

#include "plClusterGroup.h"
#include "plSpanInstance.h"

class hsGMaterial;
class hsStream;
class plSpanInstance;
class hsKeyedObject;
class plLightInfo;
class plSpanTemplate;
class plVisRegion;
class hsBounds3Ext;

class plCluster
{
public:
	enum
	{
		kNoIdx = UInt8(-1)
	};
protected:
	plClusterGroup*				fGroup;

	hsTArray<plSpanInstance*>	fInsts;

	plSpanEncoding				fEncoding;

	friend class plClusterUtil;
	plSpanInstance*				IGetInst(int i) const { return fInsts[i]; }
	void						IAddInst(plSpanInstance* inst) { fInsts.Append(inst); }
public:

	plCluster();
	~plCluster();

	void Read(hsStream* s, plClusterGroup* grp);
	void Write(hsStream* s) const;

	UInt32 NumInsts() const { return fInsts.GetCount(); }
	const plSpanInstance& GetInst(int i) const { return *fInsts[i]; }

	void UnPack(UInt8* vDst, UInt16* iDst, int idxOffset, hsBounds3Ext& wBnd) const;

	// Getters and setters, mostly for export construction.
	const plSpanTemplate* GetTemplate() const { return fGroup->GetTemplate(); }

	void SetEncoding(const plSpanEncoding& c) { fEncoding = c; }
	plSpanEncoding GetEncoding() const { return fEncoding; }

	plClusterGroup*	GetGroup() const { return fGroup; }
	void SetGroup(plClusterGroup* g) { fGroup = g; }

	hsGMaterial* GetMaterial() const { return fGroup->GetMaterial(); }

	const hsBitVector& GetVisSet() const { return fGroup->GetVisSet(); }
	const hsBitVector& GetVisNot() const { return fGroup->GetVisNot(); }

	const hsTArray<plLightInfo*>& GetLights() const { fGroup->GetLights(); }

	const plLODDist& GetLOD() const { return fGroup->GetLOD(); }
};

#endif // plCluster_inc
