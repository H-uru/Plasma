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

#ifndef plMorphDelta_inc
#define plMorphDelta_inc

#include "hsTemplates.h"
#include "hsGeometry3.h"
#include "../pnFactory/plCreatable.h"

#include "plAccessSpan.h"

class plVertDelta
{
public:
	UInt16		fIdx;
	UInt16		fPadding;
	hsVector3	fPos;
	hsVector3	fNorm;
};

class plMorphSpan
{
public:
	plMorphSpan();
	virtual ~plMorphSpan();

	hsTArray<plVertDelta>	fDeltas;

	UInt16					fNumUVWChans;
	hsPoint3*				fUVWs; // Length is fUVWChans*fDeltas.GetCount() (*sizeof(hsPoint3) in bytes).
};

class plMorphDelta : public plCreatable
{
protected:
	hsTArray<plMorphSpan>	fSpans;

	hsScalar				fWeight;
public:
	plMorphDelta();
	virtual ~plMorphDelta();

	plMorphDelta(const plMorphDelta& src);
	plMorphDelta& operator=(const plMorphDelta& src);

	CLASSNAME_REGISTER( plMorphDelta );
	GETINTERFACE_ANY( plMorphDelta, plCreatable );

	void		SetWeight(hsScalar w) { fWeight = w; }
	hsScalar	GetWeight() const { return fWeight; }

	void		Apply(hsTArray<plAccessSpan>& dst, hsScalar weight = -1.f) const;

	void		ComputeDeltas(const hsTArray<plAccessSpan>& base, const hsTArray<plAccessSpan>& moved);
	void		ComputeDeltas(const hsTArray<plGeometrySpan*>& base, const hsTArray<plGeometrySpan*>& moved, const hsMatrix44& d2b, const hsMatrix44& d2bTInv);

	UInt32		GetNumSpans() const { return fSpans.GetCount(); }
	void		SetNumSpans(int n);
	void		SetDeltas(int iSpan, const hsTArray<plVertDelta>& deltas, int numUVWChans, const hsPoint3* uvws); // len uvws is deltas.GetCount() * numUVWChans

	void		AllocDeltas(int iSpan, int nDel, int nUVW);

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);	

};

#endif // plMorphDelta_inc
