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

#ifndef hsSfxDistShade_inc
#define hsSfxDistShade_inc

#include "hsBiExpander.h"
#include "hsGRenderProcs.h"

class hsSfxDistShade : public hsGRenderProcs {
public:
	enum {
		kShadeConstant		= 0x10000
	};

	struct hsSfxDfTableEntry {
		hsScalar			fDistDel;
		hsScalar			fDistNorm;
		hsScalar			fShade;
	};
protected:

	hsScalar							fMinDist;
	hsScalar							fMaxDist;

	hsScalar							fConstShade;

	hsScalar							fMinIdle;
	hsScalar							fMaxIdle;

	hsExpander<hsSfxDfTableEntry>		fTable;

	void		IConstShadeVerts(hsExpander<hsGShadeVertex*>& vList);
	void		ICalcShadeVerts(hsExpander<hsGShadeVertex*>& vList);
	hsScalar	IShadeFromDist(hsScalar dist);
public:
	hsSfxDistShade();
	virtual ~hsSfxDistShade();

	virtual hsBool32 BeginObject(plPipeline* pipe, plDrawable* obj);

	virtual void ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList);

	void MakeTable(float* distList, float* shadeList, int num); // lists sorted from lowest cosine to highest

	virtual void Read(hsStream* s);
	virtual void Write(hsStream* s);

	virtual const char* GetLabel() const { return "hsSfxDistShade"; }

	virtual ProcType GetType() const { return kTypeDistShade; }

	CLASSNAME_REGISTER( hsSfxDistShade );
	GETINTERFACE_ANY( hsSfxDistShade, hsGRenderProcs );

};

#endif // hsSfxDistShade_inc
