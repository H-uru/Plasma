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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plParticleApplicator.h"

#include "plParticleSystem.h"
#include "plParticleGenerator.h"

#include "pnSceneObject/plSceneObject.h"

#include "plAnimation/plScalarChannel.h"
#include "plAnimation/plAGModifier.h"
#include "plMessage/plParticleUpdateMsg.h"


plParticleGenerator *plParticleApplicator::IGetParticleGen(plSceneObject *so)
{
    size_t numMods = so->GetNumModifiers();
    for (size_t i = 0; i < numMods; i++)
    {
        const plParticleSystem *result = plParticleSystem::ConvertNoRef(so->GetModifier(i));
        if (result != nullptr)
            return result->GetExportedGenerator();
    }

    return nullptr;
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
                                                    hsDegreesToRadians(chan->Value(time)));
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
