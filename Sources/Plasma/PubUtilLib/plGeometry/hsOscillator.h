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

#ifndef hsOscillator_inc
#define hsOscillator_inc

#include "hsPerterber.h"
#include "hsTemplates.h"

#include "hsGeometry3.h"
#include "../plIntersect/hsBounds.h"

class hsStream;
class plPipeline;

class hsWave
{
protected:
	hsPoint3			fWorldCenter;
	hsPoint3			fLocalCenter;

	hsScalar			fWorldFrequency; // 1.0 / Period
	hsScalar			fLocalFrequency;

	hsScalar			fWorldAmplitude;
	hsScalar			fLocalAmplitude;

	hsScalar			fPhase;
	hsScalar			fRate; // how long a crest takes to reach next crest

	hsScalar			fStartSecs;
	hsScalar			fSecsToLive;

	hsScalar			fInnerRadius;
	hsScalar			fOuterRadius;
	hsScalar			fAttenuateOutScale;

	hsScalar			AgeScale(hsScalar secs) const;

public:
	void				Accumulate(const hsPoint3& pos, const hsVector3& localZ, hsVector3& accum, hsVector3& accumNorm) const;
	hsScalar			ScaledAmplitude(hsScalar secs) const;

	void				Init(hsScalar secs, hsPoint3& center, hsScalar per, hsScalar amp, hsScalar rate, hsScalar life, hsBool32 attenOut=false);
	
	void				Update(hsScalar secs, const hsMatrix44& l2w, const hsMatrix44& w2l);
	hsBool32			IsSpent(hsScalar secs) const;
	void				Kill() { fStartSecs = fSecsToLive = 0; }
	void				AttenuateOut(hsBool32 on) { fAttenuateOutScale = (on ? 1.f : 0); }
	hsBool32			GetAttenuateOut() { return fAttenuateOutScale > 0; }

	void				Save(hsStream* s, hsScalar secs);
	void				Load(hsStream* s, hsScalar secs);
};

class hsOscillator : public hsPerterber
{
protected:
	hsTArray<hsWave>	fWaves;
	hsTArray<hsWave>	fTempWaves;

	hsMatrix44		fLocalToWorld;
	hsMatrix44		fWorldToLocal;

	hsPoint3		fWorldCenter;
	hsPoint3		fLocalCenter;

	hsVector3		fWorldAttenScale;
	hsVector3		fLocalAttenScale;
	
	hsBounds3Ext	fWorldCenterBounds;

	hsScalar		fMinPeriod;
	hsScalar		fMaxPeriod;

	hsScalar		fMinAmplitude;
	hsScalar		fMaxAmplitude;

	hsScalar		fMinRate;
	hsScalar		fMaxRate;

	hsScalar		fMinLife;
	hsScalar		fMaxLife;

	hsVector3		fLocalX;
	hsVector3		fLocalY;
	hsVector3		fLocalZ;

	hsScalar			IAttenuate(const hsPoint3& in) const;
	void			ISpawnWave(hsScalar secs, int i);

	virtual void IUpdate(hsScalar secs, plPipeline* pipe, const hsMatrix44& l2w, const hsMatrix44& w2l);

	virtual void IPerterb(const hsPoint3& in, hsGVertex3& out) const;
public:
	hsOscillator();
	virtual ~hsOscillator();

	virtual void AdjustWorldBounds(const hsMatrix44& l2w, const hsMatrix44& w2l, hsBounds3Ext& bnd) const;

	virtual UInt32 GetType() const { return kTypeOscillator; }

	// Don't call these, use base class LabelAndWrite() and CreateAndRead()
	virtual void Read(hsStream* s);
	virtual void Write(hsStream* s);

	virtual void Load(hsStream* s, hsScalar secs);
	virtual void Save(hsStream* s, hsScalar secs);

	void SetPeriodRange(hsScalar lo, hsScalar hi) { fMinPeriod = lo; fMaxPeriod = hi; }
	void SetAmplitudeRange(hsScalar lo, hsScalar hi) { fMinAmplitude = lo; fMaxAmplitude = hi; }
	void SetRateRange(hsScalar lo, hsScalar hi) { fMinRate = lo; fMaxRate = hi; }
	void SetLifeRange(hsScalar lo, hsScalar hi) { fMinLife = lo; fMaxLife = hi; }

	hsScalar GetMinPeriod() const { return fMinPeriod; }
	hsScalar GetMaxPeriod() const { return fMaxPeriod; }
	hsScalar GetMinAmplitude() const { return fMinAmplitude; }
	hsScalar GetMaxAmplitude() const { return fMaxAmplitude; }
	hsScalar GetMinRate() const { return fMinRate; }
	hsScalar GetMaxRate() const { return fMaxRate; }
	hsScalar GetMinLife() const { return fMinLife; }
	hsScalar GetMaxLife() const { return fMaxLife; }

	void SetWorldAttenScale(const hsVector3& s) { fWorldAttenScale = s; }
	void SetWorldCenterBounds(const hsBounds3Ext& bnd) { fWorldCenterBounds = bnd; }

	const hsVector3& GetWorldAttenScale() const { return fWorldAttenScale; }
	const hsBounds3Ext& GetWorldCenterBounds() const { return fWorldCenterBounds; }

	void	SetNumWaves(int n);
	UInt32	GetNumWaves() const { return fWaves.GetCount(); }
	hsWave& GetWeakestWave(hsScalar secs);
	hsWave& GetTempWave(hsScalar secs);

	virtual void Init(Int32 nParams, hsScalar* params);

	static hsGTriMesh* MakeWaveMesh(int nSpokes, const hsPoint3& center, hsScalar minRad, hsScalar maxRad, hsScalar uRange, hsScalar vRange, hsScalar attenStartFrac, hsBool32 stitch);

};

#endif // hsOscillator_inc
