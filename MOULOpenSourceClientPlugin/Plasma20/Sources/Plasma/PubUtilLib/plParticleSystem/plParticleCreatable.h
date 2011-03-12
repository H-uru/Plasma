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

#ifndef plParticleCreatable_inc
#define plParticleCreatable_inc

#include "../pnFactory/plCreator.h"

#include "plParticleSystem.h"
#include "plParticleEffect.h"
#include "plParticleEmitter.h"
#include "plParticleGenerator.h"
#include "plParticleSystem.h"
#include "plParticleApplicator.h"
#include "plParticleSDLMod.h"
#include "plConvexVolume.h"
#include "plBoundInterface.h"

REGISTER_CREATABLE( plParticleSystem );
REGISTER_NONCREATABLE( plParticleEffect );
REGISTER_NONCREATABLE( plParticleCollisionEffect );
REGISTER_CREATABLE( plParticleCollisionEffectBeat );
REGISTER_CREATABLE( plParticleCollisionEffectDie );
REGISTER_CREATABLE( plParticleCollisionEffectBounce );
REGISTER_CREATABLE( plParticleFadeVolumeEffect );
REGISTER_NONCREATABLE( plParticleGenerator );
REGISTER_CREATABLE( plSimpleParticleGenerator );
REGISTER_CREATABLE( plOneTimeParticleGenerator );
REGISTER_CREATABLE( plParticleEmitter );
REGISTER_CREATABLE( plConvexVolume );
REGISTER_CREATABLE( plBoundInterface );
REGISTER_NONCREATABLE( plParticleApplicator );
REGISTER_CREATABLE( plParticleLifeMinApplicator );
REGISTER_CREATABLE( plParticleLifeMaxApplicator );
REGISTER_CREATABLE( plParticlePPSApplicator );
REGISTER_CREATABLE( plParticleAngleApplicator );
REGISTER_CREATABLE( plParticleVelMinApplicator );
REGISTER_CREATABLE( plParticleVelMaxApplicator );
REGISTER_CREATABLE( plParticleScaleMinApplicator );
REGISTER_CREATABLE( plParticleScaleMaxApplicator );
//REGISTER_CREATABLE( plParticleGravityApplicator );
//REGISTER_CREATABLE( plParticleDragApplicator );
REGISTER_NONCREATABLE( plParticleWindEffect );
REGISTER_CREATABLE( plParticleLocalWind );
REGISTER_CREATABLE( plParticleUniformWind );
REGISTER_CREATABLE( plParticleFlockEffect );
REGISTER_CREATABLE( plParticleFollowSystemEffect );
REGISTER_CREATABLE( plParticleSDLMod );

#endif // plParticleCreatable_inc
