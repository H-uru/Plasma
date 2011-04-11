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

#include "hsTypes.h"
#include "plMorphDelta.h"

#include "hsStream.h"
#include "hsMemory.h"

#include "plAccessGeometry.h"
#include "plAccessSpan.h"
#include "plAccessVtxSpan.h"
#include "plGeometrySpan.h"

#include "plTweak.h"

static const hsScalar kMinWeight = 1.e-2f;

plMorphSpan::plMorphSpan()
:	fUVWs(nil),
	fNumUVWChans(0)
{
}

plMorphSpan::~plMorphSpan()
{
	delete [] fUVWs;
}

plMorphDelta::plMorphDelta()
:	fWeight(0)
{
}

plMorphDelta::~plMorphDelta()
{
}

plMorphDelta::plMorphDelta(const plMorphDelta& src)
{
	*this = src;
}

plMorphDelta& plMorphDelta::operator=(const plMorphDelta& src)
{
	SetNumSpans(src.GetNumSpans());
	int i;
	for( i = 0; i < fSpans.GetCount(); i++ )
	{
		SetDeltas(i, src.fSpans[i].fDeltas, src.fSpans[i].fNumUVWChans, src.fSpans[i].fUVWs);
	}
	return *this;
}

void plMorphDelta::Apply(hsTArray<plAccessSpan>& dst, hsScalar weight /* = -1.f */) const
{
	if( weight == -1.f)
		weight = fWeight; // None passed in, use our stored value

	if( weight <= kMinWeight )
		return;

	// Easy
	// For each span
	int iSpan;
	for( iSpan = 0; iSpan < fSpans.GetCount(); iSpan++ )
	{
		plAccessVtxSpan& vtxDst = dst[iSpan].AccessVtx();

		plMorphSpan& span = fSpans[iSpan];

		// For each vertDelta
		const hsPoint3* uvwDel = span.fUVWs;
		int iDelta;
		for( iDelta = 0; iDelta < span.fDeltas.GetCount(); iDelta++ )
		{
			const plVertDelta& delta = span.fDeltas[iDelta];
			// Add delPos * wgt to position
			// Add delNorm * wgt to normal
			vtxDst.Position(delta.fIdx) += delta.fPos * weight;
			vtxDst.Normal(delta.fIdx) += delta.fNorm * weight;

			// Leave skin weights and indices alone?

			// Skip color for now, since diffuse and specular are
			// ignored on the avatar?
			// // Add delDiff * wgt to diffuse
			// // Add delSpec * wgt to specular

			// For each UVW
			hsPoint3* uvws = vtxDst.UVWs(delta.fIdx);
			int iUVW;
			for( iUVW = 0; iUVW < span.fNumUVWChans; iUVW++ )
			{
				// Add delUVW * wgt to uvw
				*uvws += *uvwDel * weight;
				uvws++;
				uvwDel++;
			}
		}
	}
}

// MorphDelta - ComputeDeltas
void plMorphDelta::ComputeDeltas(const hsTArray<plAccessSpan>& base, const hsTArray<plAccessSpan>& moved)
{
	SetNumSpans(base.GetCount());

	// For each span
	{
		// for( i = 0; i < numVerts; i++ )
		{
			// NOTE: we want to discard zero deltas, but a
			// delta in any channel forces us to save the whole thing.
			// But we don't want to compare to zero (because we'll end
			// up with a lot of near zero deltas), but the epsilon we
			// compare to needs to be different for comparing something
			// like a normal delta and a position delta.
			//
			// For position, normal, color and all uvws
			// Calc del and delLenSq
			// If any delLenSq big enough, set nonZero to true
			// If nonZero
			{
				// Append to deltas (i, del's)
			}
		}
	}
}

// MorphDelta - ComputeDeltas
void plMorphDelta::ComputeDeltas(const hsTArray<plGeometrySpan*>& base, const hsTArray<plGeometrySpan*>& moved, const hsMatrix44& d2b, const hsMatrix44& d2bTInv)
{
	SetNumSpans(base.GetCount());

	hsPoint3 delUVWs[8];

	// For each span
	int iSpan;
	for( iSpan = 0; iSpan < base.GetCount(); iSpan++ )
	{
		plAccessSpan baseAcc;
		plAccessGeometry::Instance()->AccessSpanFromGeometrySpan(baseAcc, base[iSpan]);
		plAccessSpan movedAcc;
		plAccessGeometry::Instance()->AccessSpanFromGeometrySpan(movedAcc, moved[iSpan]);

		plAccPosNormUVWIterator baseIter(&baseAcc.AccessVtx());
		plAccPosNormUVWIterator movedIter(&movedAcc.AccessVtx());


		plMorphSpan& dst = fSpans[iSpan];

		const UInt16 numUVWs = baseAcc.AccessVtx().NumUVWs();

		hsTArray<plVertDelta> deltas;
		hsTArray<hsPoint3> uvws;
		deltas.SetCount(0);
		uvws.SetCount(0);


		int iVert = 0;;
		for( baseIter.Begin(), movedIter.Begin(); baseIter.More(); baseIter.Advance(), movedIter.Advance() )
		{
			// NOTE: we want to discard zero deltas, but a
			// delta in any channel forces us to save the whole thing.
			// But we don't want to compare to zero (because we'll end
			// up with a lot of near zero deltas), but the epsilon we
			// compare to needs to be different for comparing something
			// like a normal delta and a position delta.
			//
			// For position, normal, color and all uvws
			// Calc del and delLenSq
			// If any delLenSq big enough, set nonZero to true
			hsBool nonZero = false;

			// These are actually min del SQUARED.
			plConst(hsScalar) kMinDelPos(1.e-4f); // From Budtpueller's Handbook of Constants
			plConst(hsScalar) kMinDelNorm(3.e-2f); // About 10 degrees
			plConst(hsScalar) kMinDelUVW(1.e-4f); // From BHC
			hsPoint3 mPos = d2b * *movedIter.Position();
			hsVector3 delPos( &mPos, baseIter.Position());
			hsScalar delPosSq = delPos.MagnitudeSquared();
			if( delPosSq > kMinDelPos )
				nonZero = true;
			else
				delPos.Set(0,0,0);


			hsVector3 delNorm = (d2bTInv * *movedIter.Normal()) - *baseIter.Normal();
			hsScalar delNormSq = delNorm.MagnitudeSquared();
			if( delNormSq > kMinDelNorm )
				nonZero = true;
			else
				delNorm.Set(0,0,0);

			int i;
			for( i = 0; i < numUVWs; i++ )
			{
				delUVWs[i] = *movedIter.UVW(i) - *baseIter.UVW(i);
				hsScalar delUVWSq = delUVWs[i].MagnitudeSquared();
				if( delUVWSq > kMinDelUVW )
					nonZero = true;
				else
					delUVWs[i].Set(0,0,0);
			}

			if( nonZero )
			{
				// Append to deltas (i, del's)
				plVertDelta del;
				del.fIdx = iVert;
				del.fPos = delPos;
				del.fNorm = delNorm;
				deltas.Append(del);

				for( i = 0; i < numUVWs; i++ )
					uvws.Append(delUVWs[i]);
			}
			else
			{
				nonZero = false; // Breakpoint.
			}

			iVert++;
		}
		SetDeltas(iSpan, deltas, numUVWs, uvws.AcquireArray());
	}
}

void plMorphDelta::SetNumSpans(int n)
{
	fSpans.Reset();
	fSpans.SetCount(n);
}


void plMorphDelta::AllocDeltas(int iSpan, int nDel, int nUVW)
{
	fSpans[iSpan].fDeltas.SetCount(nDel);
	fSpans[iSpan].fNumUVWChans = nUVW;

	delete [] fSpans[iSpan].fUVWs;

	int uvwCnt = nDel * nUVW;
	if( uvwCnt )
		fSpans[iSpan].fUVWs = TRACKED_NEW hsPoint3[uvwCnt];
	else
		fSpans[iSpan].fUVWs = nil;
}

void plMorphDelta::SetDeltas(int iSpan, const hsTArray<plVertDelta>& deltas, int numUVWChans, const hsPoint3* uvws)
{
	AllocDeltas(iSpan, deltas.GetCount(), numUVWChans);
	if( deltas.GetCount() )
	{
		HSMemory::BlockMove(&deltas[0], fSpans[iSpan].fDeltas.AcquireArray(), deltas.GetCount() * sizeof(plVertDelta));

		if( numUVWChans )
			HSMemory::BlockMove(uvws, fSpans[iSpan].fUVWs, deltas.GetCount() * numUVWChans * sizeof(*uvws));
	}
}

void plMorphDelta::Read(hsStream* s, hsResMgr* mgr)
{
	fWeight = s->ReadSwapScalar();

	int n = s->ReadSwap32();
	SetNumSpans(n);
	int iSpan;
	for( iSpan = 0; iSpan < n; iSpan++ )
	{
		int nDel = s->ReadSwap32();
		int nUVW = s->ReadSwap32();
		AllocDeltas(iSpan, nDel, nUVW);
		if( nDel )
		{
			s->Read(nDel * sizeof(plVertDelta), fSpans[iSpan].fDeltas.AcquireArray());
			if( nUVW )
				s->Read(nDel * nUVW * sizeof(hsPoint3), fSpans[iSpan].fUVWs);
		}
	}

}

void plMorphDelta::Write(hsStream* s, hsResMgr* mgr)
{
	s->WriteSwapScalar(fWeight);

	s->WriteSwap32(fSpans.GetCount());

	int iSpan;
	for( iSpan = 0; iSpan < fSpans.GetCount(); iSpan++ )
	{
		int nDel = fSpans[iSpan].fDeltas.GetCount();
		int nUVW = fSpans[iSpan].fNumUVWChans;
		s->WriteSwap32(nDel);
		s->WriteSwap32(nUVW);

		if( nDel )
		{
			// Initialize our padding here, so we don't write random data
			for (int i = 0; i < nDel; i++)
			{
				plVertDelta& delta = fSpans[iSpan].fDeltas[i];
				delta.fPadding = 0;
			}

			s->Write(nDel * sizeof(plVertDelta), fSpans[iSpan].fDeltas.AcquireArray());

			if( nUVW )
				s->Write(nDel * nUVW * sizeof(hsPoint3), fSpans[iSpan].fUVWs);
		}
	}
}
