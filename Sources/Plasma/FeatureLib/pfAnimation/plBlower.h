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

#ifndef plBlower_inc
#define plBlower_inc

#include "../pnModifier/plSingleModifier.h"
#include "hsGeometry3.h"
#include "../plMath/plRandom.h"
#include "hsTemplates.h"

class plSceneObject;
class hsStream;
class hsResMgr;

class plBlower : public plSingleModifier
{
protected:
	class Oscillator
	{
	public:
		hsScalar	fFrequency;
		hsScalar	fPhase;
		hsScalar	fPower;
	};
	static plRandom	fRandom;

	// Parameters
	hsScalar		fMasterPower;
	hsScalar		fMasterFrequency;
	hsScalar		fDirectRate;
	hsScalar		fImpulseRate;
	hsScalar		fSpringKonst;
	hsScalar		fBias;

	hsScalar					fAccumTime;
	hsTArray<Oscillator>		fOscillators;

	// CurrentState
	hsVector3		fDirection;
	hsPoint3		fRestPos;
	hsPoint3		fLocalRestPos;
	hsVector3		fCurrDel;
	hsScalar		fMaxOffsetDist;

	void	IInitOscillators();
	void	ISetTargetTransform();
	void	IBlow(double secs, hsScalar delSecs);
	
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
public:
	~plBlower();
	plBlower();

	CLASSNAME_REGISTER( plBlower );
	GETINTERFACE_ANY( plBlower, plSingleModifier );
	
	virtual void SetTarget(plSceneObject* so);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void SetMasterPower(hsScalar f) { fMasterPower = f; }
	void SetMasterFrequency(hsScalar f) { fMasterFrequency = f; }
	void SetDirectRate(hsScalar f) { fDirectRate = f; }
	void SetImpulseRate(hsScalar f) { fImpulseRate = f; }
	void SetSpringKonst(hsScalar f) { fSpringKonst = f; }
	void SetConstancy(hsScalar f);

	hsScalar GetMasterPower() const { return fMasterPower; }
	hsScalar GetMasterFrequency() const { return fMasterFrequency; }
	hsScalar GetDirectRate() const { return fDirectRate; }
	hsScalar GetImpulseRate() const { return fImpulseRate; }
	hsScalar GetSpringKonst() const { return fSpringKonst; }
	hsScalar GetConstancy() const;
};

#endif // plBlower_inc
