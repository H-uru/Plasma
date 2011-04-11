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

#ifndef hsSfxAngleFade_inc
#define hsSfxAngleFade_inc

#include "hsBiExpander.h"
#include "hsGRenderProcs.h"
#include "hsBitVector.h"

class hsSfxAngleFade : public hsGRenderProcs {
public:
	enum {
		kDirectional		= 0x10000,
		kTargetRelative		= 0x20000,
		kTwoSided			= 0x40000,
		kFaceNormals		= 0x80000
	};
	struct hsSfxAfTableEntry {
		hsScalar			fCosineDel;
		hsScalar			fCosineNorm;
		hsScalar			fOpacity;
	};
protected:

	hsBitVector							fSetVector;
	hsExpander<hsSfxAfTableEntry>		fTable;

	hsScalar IOpacFromDot(hsScalar dot);
public:
	hsSfxAngleFade();
	virtual ~hsSfxAngleFade();

	virtual void ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList);
	virtual void ProcessPreInterpTris(hsExpander<hsTriangle3*>& tList, hsExpander<hsGTriVertex*>& vList);

	void MakeTable(float* cosList, float* opacList, int num); // lists sorted from lowest cosine to highest

	virtual void Read(hsStream* s);
	virtual void Write(hsStream* s);

	virtual const char* GetLabel() const { return "hsSfxAngleFade"; }

	virtual ProcType GetType() const { return kTypeAngleFade; }

	CLASSNAME_REGISTER( hsSfxAngleFade );
	GETINTERFACE_ANY( hsSfxAngleFade, hsGRenderProcs );

};

#endif // hsSfxAngleFade_inc
