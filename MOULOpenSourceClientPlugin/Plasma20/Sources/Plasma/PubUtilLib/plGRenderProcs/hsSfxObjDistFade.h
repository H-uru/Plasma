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

#ifndef hsSfxObjDistFade_inc
#define hsSfxObjDistFade_inc

#include "hsBiExpander.h"
#include "hsGRenderProcs.h"
#include "../plPipeline/hsGMatState.h"

class hsSfxObjDistFade : public hsGRenderProcs {
public:
	enum {
		kCullsBefore		= 0x10000,
		kCullsBeyond		= 0x20000,

		kDistFromView		= 0x40000,
		kDistFromTarget		= 0x80000,
		kDistAlongX			= 0x100000,
		kZOff				= 0x200000,
		kNoZTrans			= 0x400000,
		kByBoundsCenter		= 0x800000,

		kPostInterp			= 0x1000000,

		kIdleBefore			= 0x2000000,
		kIdleBeyond			= 0x4000000,

		kZWasOff			= 0x8000000
	};

	struct hsSfxDfTableEntry {
		hsScalar			fDistDel;
		hsScalar			fDistNorm;
		hsScalar			fOpacity;
	};
protected:

	hsScalar							fMinDist;
	hsScalar							fMaxDist;

	hsScalar							fMinIdle;
	hsScalar							fMaxIdle;

	Int32								fTreeCnt;

	hsExpander<hsSfxDfTableEntry>		fTable;

	hsGMatState							fRestoreOver;

	hsBool32 ISetOpac(plDrawable* refObj);
	hsScalar IOpacFromDist(hsScalar dist);
public:
	hsSfxObjDistFade();
	virtual ~hsSfxObjDistFade();

	virtual hsBool32 BeginTree(plPipeline* pipe, plDrawable* root);
	virtual hsBool32 BeginObject(plPipeline* pipe, plDrawable* obj);

	virtual void EndObject();
	virtual void EndTree();

	void MakeTable(float* distList, float* opacList, int num); // lists sorted from lowest cosine to highest

	virtual void Read(hsStream* s);
	virtual void Write(hsStream* s);

	virtual const char* GetLabel() const { return "hsSfxObjDistFade"; }

	virtual ProcType GetType() const { return kTypeObjDistFade; }

	CLASSNAME_REGISTER( hsSfxObjDistFade );
	GETINTERFACE_ANY( hsSfxObjDistFade, hsGRenderProcs );

};

#endif // hsSfxObjDistFade_inc
