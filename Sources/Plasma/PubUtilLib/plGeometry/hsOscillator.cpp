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
#include "hsStream.h"
#include "hsOscillator.h"
#include "../plMath/hsFastMath.h"
#include "hsGTriMesh.h"
#include "hsTriangle3.h"
#include "../plPipeline/plPipeline.h"

#if defined(__MWERKS__) && !defined(HS_DEBUGGING)
#pragma optimization_level 0
#endif

static hsScalar rnd0_1()
{
	return hsScalar(rand()) / hsScalar(RAND_MAX);
}

void hsWave::Save(hsStream* s, hsScalar secs)
{
	fWorldCenter.Write(s);
	
	s->WriteSwapScalar(fWorldFrequency);

	s->WriteSwapScalar(fWorldAmplitude);

	s->WriteSwapScalar(fPhase);
	s->WriteSwapScalar(fRate);

	s->WriteSwapScalar(secs - fStartSecs);

	s->WriteSwapScalar(fSecsToLive);
}

void hsWave::Load(hsStream* s, hsScalar secs)
{
	fWorldCenter.Read(s);
	
	fWorldFrequency = s->ReadSwapScalar();

	fWorldAmplitude = s->ReadSwapScalar();

	fPhase = s->ReadSwapScalar();
	fRate = s->ReadSwapScalar();

	fStartSecs = s->ReadSwapScalar();
	fStartSecs = secs - fStartSecs;

	fSecsToLive = s->ReadSwapScalar();
}

void hsWave::Init(hsScalar secs, hsPoint3& center, hsScalar per, hsScalar amp, hsScalar rate, hsScalar life, hsBool32 attenOut)
{
	fStartSecs = secs;
	fWorldCenter = center;
	fWorldFrequency = hsScalarInvert(per);
	fWorldAmplitude = amp;
	fRate = rate;
	fSecsToLive = life;
	AttenuateOut(attenOut);
}

hsBool32 hsWave::IsSpent(hsScalar secs) const
{ 
	return secs - fStartSecs > fSecsToLive;
}

void hsWave::Accumulate(const hsPoint3& pos, const hsVector3& localZ, hsVector3& accum, hsVector3& accumNorm) const
{
	hsVector3 del(&pos, &fLocalCenter);
	hsScalar dot = del.InnerProduct(localZ);
	dot *= -2.f;
	del += localZ * dot;

	hsScalar dist = del.MagnitudeSquared();
	dist = hsFastMath::InvSqrtAppr(dist);
	del *= dist;
	dist = hsScalarInvert(dist);

	hsScalar ampl = fLocalAmplitude;
	if( fAttenuateOutScale > 0 )
	{
		if( dist > fInnerRadius )
		{
			if( dist > fOuterRadius )
				return;
			ampl *= fOuterRadius - dist;
			ampl *= fAttenuateOutScale;
		}
	}

	dist *= fLocalFrequency;
	dist += fPhase;

	hsScalar s, c;
	hsFastMath::SinCosAppr(dist, s, c);

	s *= ampl;
	s += ampl;
	c *= ampl * fLocalFrequency;

//	accum += s * localZ;
	accum.fZ += s / localZ.fZ;

	hsVector3 norm;
	norm = localZ;
	norm += del * -c;
	accumNorm += norm;

	return;
}

void hsWave::Update(hsScalar secs, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( l2w.fFlags & hsMatrix44::kIsIdent )
	{
		fLocalCenter = fWorldCenter;
		fLocalFrequency = fWorldFrequency;
		fLocalAmplitude = fWorldAmplitude;
	}
	else
	{
		hsVector3 ax;
		ax.Set(w2l.fMap[0][2], w2l.fMap[1][2], w2l.fMap[2][2]);
		hsScalar ooScale = ax.MagnitudeSquared();
		ooScale = hsFastMath::InvSqrtAppr(ooScale);

		fLocalCenter = w2l * fWorldCenter;
		fLocalFrequency = fWorldFrequency * ooScale;
		
		hsScalar scale = 1.f / ooScale;
		fLocalAmplitude = fWorldAmplitude * scale;
	}
	fLocalAmplitude *= AgeScale(secs);

	if( fAttenuateOutScale > 0 )
	{
		fInnerRadius = fRate * (secs - fStartSecs) * hsScalarPI * 2.f;
		fOuterRadius = fInnerRadius * (5.f/4.f);
		fAttenuateOutScale = hsScalarInvert(fOuterRadius - fInnerRadius);
	}

	fPhase = -(secs - fStartSecs) * fRate * hsScalarPI * 2.f;
}

hsScalar hsWave::ScaledAmplitude(hsScalar secs) const
{
	return fWorldAmplitude * AgeScale(secs);
}

hsScalar hsWave::AgeScale(hsScalar secs) const
{
	hsScalar age = secs - fStartSecs;
	extern int dbgCurrentTest;
	if( dbgCurrentTest )
	{
		age *= 4.f;
		age -= 2.f * fSecsToLive;
		if( age < 0 )
			age = -age;
		age -= fSecsToLive;
	}
	else
	{
		age *= 2.f;
		age -= fSecsToLive;
		if( age < 0 )
			age = -age;
	}
	hsScalar ageScale = 1.f - age / fSecsToLive;
	if( ageScale < 0 )
		ageScale = 0;
	else if( ageScale > 1.f )
		ageScale = 1.f;
	return ageScale;
}

hsOscillator::hsOscillator()
{
}

hsOscillator::~hsOscillator()
{
}

hsWave& hsOscillator::GetWeakestWave(hsScalar secs)
{
	hsAssert(!GetDisabled(), "Shouldn't be messing with disabled oscillator system");
	int weakest = 0;
	hsScalar amp = fWaves[0].ScaledAmplitude(secs);
	int i;
	for( i = 0; i < fWaves.GetCount(); i++ )
	{
		hsScalar tAmp = fWaves[i].ScaledAmplitude(secs);
		if( tAmp < amp )
		{
			weakest = i;
			amp = tAmp;
		}
	}
	return fWaves[weakest];
}

hsWave& hsOscillator::GetTempWave(hsScalar secs)
{
	int i;
	for( i = 0; i < fTempWaves.GetCount(); i++ )
	{
		if( fTempWaves[i].IsSpent(secs) )
			return fTempWaves[i];
	}
	fTempWaves.Push();
	return fTempWaves[fTempWaves.GetCount()-1];
}

void hsOscillator::ISpawnWave(hsScalar secs, int i)
{
	hsPoint3 corner;
	fWorldCenterBounds.GetCorner(&corner);
	hsVector3 ax[3];
	fWorldCenterBounds.GetAxes(ax+0, ax+1, ax+2);
	hsScalar r;
	r = rnd0_1();
	ax[0] *= r;
	corner += ax[0];
	r = rnd0_1();
	ax[1] *= r;
	corner += ax[1];
	r = rnd0_1();
	ax[2] *= r;
	corner += ax[2];

	hsScalar per = fMinPeriod;
	r = rnd0_1();
	hsScalar rr = r;
	r *= fMaxPeriod - fMinPeriod;
	per += r;

	hsScalar amp = fMinAmplitude;
	r = rr * rnd0_1();
	r *= fMaxAmplitude - fMinAmplitude;
	amp += r;

	hsScalar life = fMinLife;
	r = rnd0_1();
	r *= fMaxLife - fMinLife;
	life += r;

	hsScalar rate = fMinRate;
	r = rnd0_1();
	r *= fMaxRate - fMinRate;
	rate += r;

	fWaves[i].Init(secs, corner, per, amp, rate, life);

}

void hsOscillator::IUpdate(hsScalar secs, plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l)
{
	if( GetDisabled() )
		return;

	fWorldCenter = pipe->GetViewPositionWorld();
	fWorldCenter.fZ = (fWorldCenterBounds.GetMins().fZ + fWorldCenterBounds.GetMaxs().fZ) * 0.5f;
	fLocalCenter = w2l * fWorldCenter;

	fLocalToWorld = l2w;
	fWorldToLocal = w2l;

	fLocalX.Set(w2l.fMap[0][0],w2l.fMap[1][0],w2l.fMap[2][0]);
	fLocalX.Normalize();
	fLocalY.Set(w2l.fMap[0][1],w2l.fMap[1][1],w2l.fMap[2][1]);
	fLocalY.Normalize();
	fLocalZ.Set(w2l.fMap[0][2],w2l.fMap[1][2],w2l.fMap[2][2]);
	fLocalZ.Normalize();

	hsVector3 ax;
	hsScalar ooScale;
	ax.Set(w2l.fMap[0][0], w2l.fMap[1][0], w2l.fMap[2][0]);
	ooScale = ax.MagnitudeSquared();
	ooScale = hsFastMath::InvSqrtAppr(ooScale);
	fLocalAttenScale.fX = fWorldAttenScale.fX * ooScale;

	ax.Set(w2l.fMap[0][1], w2l.fMap[1][1], w2l.fMap[2][1]);
	ooScale = ax.MagnitudeSquared();
	ooScale = hsFastMath::InvSqrtAppr(ooScale);
	fLocalAttenScale.fY = fWorldAttenScale.fY * ooScale;

	fLocalAttenScale.fZ = 0;

	int i;
	for( i = 0; i < fWaves.GetCount(); i++ )
	{
		if( fWaves[i].IsSpent(secs) )
			ISpawnWave(secs, i);
		fWaves[i].Update(secs, l2w, w2l);
	}
	for( i = 0; i < fTempWaves.GetCount(); i++ )
	{
		while( (i < fTempWaves.GetCount()) && fTempWaves[i].IsSpent(secs) )
			fTempWaves.Remove(i, 1);
		if( i < fTempWaves.GetCount() )
			fTempWaves[i].Update(secs, l2w, w2l);
	}
}

hsScalar hsOscillator::IAttenuate(const hsPoint3& in) const 
{
	const hsPoint3& cen = fLocalCenter;
	hsVector3 del(&in, &cen);

	hsScalar atX = del.InnerProduct(fLocalX);
	atX *= fLocalAttenScale.fX;
	if( atX > 0 )
		atX = -atX;
	atX += 1.f;
	if( atX < 0 )
		atX = 0;

	hsScalar atY = del.InnerProduct(fLocalY);
	atY *= fLocalAttenScale.fY;
	if( atY > 0 )
		atY = -atY;
	atY += 1.f;
	if( atY < 0 )
		atY = 0;

	hsScalar at = atX * atY;
	return at;
}

void hsOscillator::AdjustWorldBounds(const hsMatrix44& l2w, const hsMatrix44& w2l, hsBounds3Ext& bnd) const
{
	if( GetDisabled() )
		return;

	hsVector3 adj;
	adj.Set(0,1.f/fLocalZ.fZ,0);
	adj = l2w * adj;
	adj *= fMaxAmplitude * fWaves.GetCount();

	bnd.Union(&adj);
	adj = -adj;
	bnd.Union(&adj);
}

void hsOscillator::IPerterb(const hsPoint3& in, hsGVertex3& out) const
{
	hsPoint3 pos = in;
	hsVector3 del(&pos, &fLocalCenter);
	hsScalar dot = del.InnerProduct(fLocalZ);
	pos += fLocalZ * -dot;

	hsVector3 accum;
	hsVector3 accumNorm;
	accum.Set(0,0,0);
	accumNorm.Set(0,0,0);
	int i;
	for( i = 0; i < fWaves.GetCount(); i++ )
	{
		fWaves[i].Accumulate(pos, fLocalZ, accum, accumNorm);
	}
	for( i = 0; i < fTempWaves.GetCount(); i++ )
	{
		fTempWaves[i].Accumulate(pos, fLocalZ, accum, accumNorm);
	}
	hsScalar atten = IAttenuate(pos);
	static int attenuating = 1;
	if( attenuating ) // nuke me
		accum *= atten;
	out.fLocalPos = in + accum;

	hsScalar invNorm = hsFastMath::InvSqrtAppr(accumNorm.MagnitudeSquared());
	accumNorm *= invNorm;
	out.fNormal = accumNorm;
}

void hsOscillator::Read(hsStream* s)
{
	int n = s->ReadSwap32();
	SetNumWaves(n);

	fWorldAttenScale.Read(s);
	fWorldCenterBounds.Read(s);

	fMinPeriod = s->ReadSwapScalar();
	fMaxPeriod = s->ReadSwapScalar();

	fMinAmplitude = s->ReadSwapScalar();
	fMaxAmplitude = s->ReadSwapScalar();

	fMinRate = s->ReadSwapScalar();
	fMaxRate = s->ReadSwapScalar();

	fMinLife = s->ReadSwapScalar();
	fMaxLife = s->ReadSwapScalar();

	int i;
	for( i = 0; i < fWaves.GetCount(); i++ )
		fWaves[i].Kill();

	fTempWaves.Reset();
}

void hsOscillator::Load(hsStream* s, hsScalar secs)
{
	Read(s);

	int i;
	for( i = 0; i < fWaves.GetCount(); i++ )
		fWaves[i].Load(s, secs);

	fTempWaves.Reset();
}

void hsOscillator::Write(hsStream* s)
{
	s->WriteSwap32(fWaves.GetCount());

	fWorldAttenScale.Write(s);
	fWorldCenterBounds.Write(s);

	s->WriteSwapScalar(fMinPeriod);
	s->WriteSwapScalar(fMaxPeriod);

	s->WriteSwapScalar(fMinAmplitude);
	s->WriteSwapScalar(fMaxAmplitude);

	s->WriteSwapScalar(fMinRate);
	s->WriteSwapScalar(fMaxRate);

	s->WriteSwapScalar(fMinLife);
	s->WriteSwapScalar(fMaxLife);

}

void hsOscillator::Save(hsStream* s, hsScalar secs)
{
	Write(s);

	int i;
	for( i = 0; i < fWaves.GetCount(); i++ )
		fWaves[i].Save(s, secs);
}

void hsOscillator::SetNumWaves(int n)
{
	fWaves.SetCount(n);
	int i;
	for( i = 0; i < n; i++ )
		fWaves[i].Kill();
}

void hsOscillator::Init(Int32 nParams, hsScalar* params)
{
// NumWaves				= 1
// AttenScale			= 2
// WorldCenterBounds	= 6
// Period				= 2
// Amp					= 2
// Rate					= 2
// Life					= 2

	hsAssert(17 == nParams, "Parameter input mismatch");

	SetNumWaves(int(*params++));

	fWorldAttenScale.fX = *params++;
	fWorldAttenScale.fY = *params++;
	fWorldAttenScale.fZ = 0;

	hsPoint3 pt;
	hsBounds3Ext bnd;
	pt.fX = *params++;
	pt.fY = *params++;
	pt.fZ = *params++;
	bnd.Reset(&pt);
	pt.fX = *params++;
	pt.fY = *params++;
	pt.fZ = *params++;
	bnd.Union(&pt);
	SetWorldCenterBounds(bnd);

	SetPeriodRange(params[0], params[1]);
	params += 2;

	SetAmplitudeRange(params[0], params[1]);
	params += 2;

	SetRateRange(params[0], params[1]);
	params += 2;

	SetLifeRange(params[0], params[1]);

	fTempWaves.Reset();
}


#if 1
hsGTriMesh* hsOscillator::MakeWaveMesh(int nSpokes, const hsPoint3& center, hsScalar minRad, hsScalar maxRad, hsScalar uRange, hsScalar vRange, hsScalar attenStartFrac, hsBool32 stitch)
{
	hsGTriMesh* triMesh = new hsGTriMesh;

	hsTArray<hsScalar> radii;
	hsScalar cRad = 0;
	while( cRad < maxRad )
	{
		// OOPS - for the half circle, this should be PI*R/n, not 2PI. Don't fix until we've corrected the callers. Or we might want to leave it like
		// this anyway, since we're looking obliquely at these faces anyway, and this error stretches the side that perspective compresses. May
		// want to make the unstitched version wrong in the same way.
		hsScalar tRad = 2.f * hsScalarPI * cRad / nSpokes;
		if( tRad < minRad )
			tRad = minRad;
		cRad += tRad;
		radii.Append(cRad);
	}

	int nShell = radii.GetCount();

	int nTris = stitch
		? 2 * nSpokes * (nShell-1) + nSpokes
		: 2 * (nSpokes-1) * (nShell-1) + (nSpokes-1);
	int nVerts = nSpokes * nShell + 1;
	triMesh->AllocatePointers(nTris, nVerts, nVerts, nVerts);
	triMesh->SetNumTriVertex(nVerts);
	triMesh->SetNumPoints(nVerts);
	triMesh->SetNumUvs(nVerts);
	triMesh->SetHasColors(true);

	*triMesh->GetPoint(0) = center;
	triMesh->GetNormal(0)->Set(0,1.f,0);
	triMesh->GetColor(0)->Set(0,0,0,1.f);
	triMesh->GetUvs(0)->fX = triMesh->GetUvs(0)->fY = triMesh->GetUvs(0)->fZ = 0;

	hsScalar iToRadians = stitch 
		? 2.f * hsScalarPI / nSpokes
		: hsScalarPI / nSpokes;
	hsScalar attenStart = maxRad * attenStartFrac;
	hsScalar attenEnd = maxRad;
	hsScalar attenScale = hsScalarInvert(attenEnd - attenStart);
	int i, j;
	for( i = 0; i < nSpokes; i++ )
	{
		hsScalar s = hsSine(i * iToRadians);
		hsScalar c = hsCosine(i * iToRadians);
		for( j = 0; j < nShell; j++ )
		{
			hsAssert(1 + i*nShell + j < nVerts, "Going out of range on verts");
			hsGVertex3* vtx = triMesh->GetVertex(1 + i*nShell + j);
			hsColorRGBA* col = triMesh->GetColor(1 + i*nShell + j);
			hsGUv* uv = triMesh->GetUvs(1 + i*nShell + j);

			hsScalar x = c * radii[j];
			hsScalar y = s * radii[j];

			hsScalar u = x / uRange;
			hsScalar v = y / vRange;

			vtx->fLocalPos.fX = center.fX + x;
			vtx->fLocalPos.fY = center.fY + y;
			vtx->fLocalPos.fZ = 0.f;

			vtx->fNormal.Set(0,0,1.f);

			uv->fX = u;
			uv->fY = v;
			uv->fZ = 0.f;

			if( radii[j] > attenStart )
			{
				hsScalar a = (attenEnd - radii[j]) * attenScale;
				if( a < 0 )
					a = 0;
				else if( a > 1.f )
					a = 1.f;
				col->Set(0,0,0,a);
			}
			else
				col->Set(0,0,0,1.f);
		}
	}

	int spokeEnd = stitch ? nSpokes : nSpokes-1;
	int nextTri = 0;
	for( i = 0; i < spokeEnd; i++ )
	{
		hsTriangle3* tri = triMesh->GetTriFromPool(nextTri);
		tri->Zero();
		tri->fOrigTri = tri;
		triMesh->SetTriangle(nextTri++, tri);

		tri->fVert[0] = triMesh->GetTriVertex(0);
		tri->fVert[0]->fVtx = triMesh->GetVertex(0);
		tri->fVert[0]->SetNumUvChannels(1);
		tri->fVert[0]->fUvChan[0] = triMesh->GetUvs(0);
		tri->fVert[0]->fVtxColor = triMesh->GetColor(0);

		int iv0 = 1 + i * nShell;
		int iv1 = i < nSpokes - 1 ? 1 + (i+1)*nShell : 1;
		hsAssert((iv0 < nVerts)&&(iv1 < nVerts), "Out of range on triverts");
		
		tri->fVert[1] = triMesh->GetTriVertex(iv0);
		tri->fVert[1]->fVtx = triMesh->GetVertex(iv0);
		tri->fVert[1]->SetNumUvChannels(1);
		tri->fVert[1]->fUvChan[0] = triMesh->GetUvs(iv0);
		tri->fVert[1]->fVtxColor = triMesh->GetColor(iv0);

		tri->fVert[2] = triMesh->GetTriVertex(iv1);
		tri->fVert[2]->fVtx = triMesh->GetVertex(iv1);
		tri->fVert[2]->SetNumUvChannels(1);
		tri->fVert[2]->fUvChan[0] = triMesh->GetUvs(iv1);
		tri->fVert[2]->fVtxColor = triMesh->GetColor(iv1);

		tri->fVert[0]->fFlags = hsGTriVertex::kHasPointers
			| hsGTriVertex::kHasVertexUvs
			| hsGTriVertex::kHasVertexColors;
		tri->fVert[1]->fFlags = hsGTriVertex::kHasPointers
			| hsGTriVertex::kHasVertexUvs
			| hsGTriVertex::kHasVertexColors;
		tri->fVert[2]->fFlags = hsGTriVertex::kHasPointers
			| hsGTriVertex::kHasVertexUvs
			| hsGTriVertex::kHasVertexColors;

		tri->fFlags |= hsTriangle3::kHasVertexPosNorms 
			| hsTriangle3::kHasVertexUvs
			| hsTriangle3::kHasVertexColors
			| hsTriangle3::kHasPointers;

		int iv2 = iv0 + 1;
		int iv3 = iv1 + 1;
		hsAssert((iv1 < nVerts)&&(iv2 < nVerts), "Out of range on triverts");
		for( j = 0; j < nShell-1; j++ )
		{
			tri = triMesh->GetTriFromPool(nextTri);
			tri->Zero();
			tri->fOrigTri = tri;
			triMesh->SetTriangle(nextTri++, tri);

			tri->fVert[0] = triMesh->GetTriVertex(iv0);
			tri->fVert[0]->fVtx = triMesh->GetVertex(iv0);
			tri->fVert[0]->SetNumUvChannels(1);
			tri->fVert[0]->fUvChan[0] = triMesh->GetUvs(iv0);
			tri->fVert[0]->fVtxColor = triMesh->GetColor(iv0);

			tri->fVert[1] = triMesh->GetTriVertex(iv2);
			tri->fVert[1]->fVtx = triMesh->GetVertex(iv2);
			tri->fVert[1]->SetNumUvChannels(1);
			tri->fVert[1]->fUvChan[1] = triMesh->GetUvs(iv2);
			tri->fVert[1]->fVtxColor = triMesh->GetColor(iv2);

			tri->fVert[2] = triMesh->GetTriVertex(iv3);
			tri->fVert[2]->fVtx = triMesh->GetVertex(iv3);
			tri->fVert[2]->SetNumUvChannels(1);
			tri->fVert[2]->fUvChan[0] = triMesh->GetUvs(iv3);
			tri->fVert[2]->fVtxColor = triMesh->GetColor(iv3);

			tri->fVert[0]->fFlags = hsGTriVertex::kHasPointers
				| hsGTriVertex::kHasVertexUvs
				| hsGTriVertex::kHasVertexColors;
			tri->fVert[1]->fFlags = hsGTriVertex::kHasPointers
				| hsGTriVertex::kHasVertexUvs
				| hsGTriVertex::kHasVertexColors;
			tri->fVert[2]->fFlags = hsGTriVertex::kHasPointers
				| hsGTriVertex::kHasVertexUvs
				| hsGTriVertex::kHasVertexColors;

			tri->fFlags |= hsTriangle3::kHasVertexPosNorms 
				| hsTriangle3::kHasVertexUvs
				| hsTriangle3::kHasVertexColors
				| hsTriangle3::kHasPointers;

			tri = triMesh->GetTriFromPool(nextTri);
			tri->Zero();
			tri->fOrigTri = tri;
			triMesh->SetTriangle(nextTri++, tri);

			tri->fVert[0] = triMesh->GetTriVertex(iv0);
			tri->fVert[0]->fVtx = triMesh->GetVertex(iv0);
			tri->fVert[0]->SetNumUvChannels(1);
			tri->fVert[0]->fUvChan[0] = triMesh->GetUvs(iv0);
			tri->fVert[0]->fVtxColor = triMesh->GetColor(iv0);

			tri->fVert[1] = triMesh->GetTriVertex(iv3);
			tri->fVert[1]->fVtx = triMesh->GetVertex(iv3);
			tri->fVert[1]->SetNumUvChannels(1);
			tri->fVert[1]->fUvChan[0] = triMesh->GetUvs(iv3);
			tri->fVert[1]->fVtxColor = triMesh->GetColor(iv3);

			tri->fVert[2] = triMesh->GetTriVertex(iv1);
			tri->fVert[2]->fVtx = triMesh->GetVertex(iv1);
			tri->fVert[2]->SetNumUvChannels(1);
			tri->fVert[2]->fUvChan[0] = triMesh->GetUvs(iv1);
			tri->fVert[2]->fVtxColor = triMesh->GetColor(iv1);

			tri->fVert[0]->fFlags = hsGTriVertex::kHasPointers
				| hsGTriVertex::kHasVertexUvs
				| hsGTriVertex::kHasVertexColors;
			tri->fVert[1]->fFlags = hsGTriVertex::kHasPointers
				| hsGTriVertex::kHasVertexUvs
				| hsGTriVertex::kHasVertexColors;
			tri->fVert[2]->fFlags = hsGTriVertex::kHasPointers
				| hsGTriVertex::kHasVertexUvs
				| hsGTriVertex::kHasVertexColors;

			tri->fFlags |= hsTriangle3::kHasVertexPosNorms 
				| hsTriangle3::kHasVertexUvs
				| hsTriangle3::kHasVertexColors
				| hsTriangle3::kHasPointers;

			iv0++;
			iv1++;
			iv2++;
			iv3++;
		}
	}
	hsAssert(nextTri <= nTris, "Out of range on tris");

	triMesh->StoreOrigPoints();

	return triMesh;
}
#endif