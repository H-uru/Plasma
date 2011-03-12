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
#include "plParticleSystem.h"
#include "plParticleGenerator.h"
#include "plParticleApplicator.h"
#include "../plAvatar/plScalarChannel.h"
#include "../plAvatar/plAGModifier.h"
#include "../plMessage/plParticleUpdateMsg.h"
#include "../pnSceneObject/plSceneObject.h"

#define PI 3.14159

plParticleGenerator *plParticleApplicator::IGetParticleGen(plSceneObject *so)
{
	UInt32 numMods = so->GetNumModifiers();
	int i;
	for (i = 0; i < numMods; i++)
	{
		const plParticleSystem *result = plParticleSystem::ConvertNoRef(so->GetModifier(i));
		if (result != nil)
			return result->GetExportedGenerator();
	}

	return nil;
}

void plParticleLifeMinApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamPartLifeMin,
													chan->Value(time));
}

void plParticleLifeMaxApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamPartLifeMax,
													chan->Value(time));
}

void plParticlePPSApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamParticlesPerSecond,
													chan->Value(time));
}

void plParticleAngleApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamInitPitchRange,
													(hsScalar)(chan->Value(time) * PI / 180.f));
}

void plParticleVelMinApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamVelMin,
													chan->Value(time));
}

void plParticleVelMaxApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamVelMax,
													chan->Value(time));
}

void plParticleScaleMinApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamScaleMin,
													chan->Value(time) / 100.f);
}

void plParticleScaleMaxApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamScaleMax,
													chan->Value(time) / 100.f);
}

void plParticleGravityApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
//	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamParticlesPerSecond,
//													chan->Value(time));
}

void plParticleDragApplicator::IApply(const plAGModifier *mod, double time)
{
	plScalarChannel *chan = plScalarChannel::ConvertNoRef(fChannel);
//	IGetParticleGen(mod->GetTarget(0))->UpdateParam(plParticleUpdateMsg::kParamParticlesPerSecond,
//													chan->Value(time));
}

