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
#ifndef PLPARTICLEAPPLICATOR_INC
#define PLPARTICLEAPPLICATOR_INC

#include "../plAvatar/plAGChannel.h"
#include "../plAvatar/plAGApplicator.h"

class plParticleSystem;

class plParticleApplicator : public plAGApplicator
{
protected:
	plParticleGenerator *IGetParticleGen(plSceneObject *so);
	virtual void IApply(const plAGModifier *mod, double time) = 0;

public:
	CLASSNAME_REGISTER( plParticleApplicator );
	GETINTERFACE_ANY( plParticleApplicator, plAGApplicator );
};

class plParticleLifeMinApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleLifeMinApplicator );
	GETINTERFACE_ANY( plParticleLifeMinApplicator, plAGApplicator );
};

class plParticleLifeMaxApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleLifeMaxApplicator );
	GETINTERFACE_ANY( plParticleLifeMaxApplicator, plAGApplicator );
};

class plParticlePPSApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticlePPSApplicator );
	GETINTERFACE_ANY( plParticlePPSApplicator, plAGApplicator );
};

class plParticleAngleApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleAngleApplicator );
	GETINTERFACE_ANY( plParticleAngleApplicator, plAGApplicator );
};

class plParticleVelMinApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleVelMinApplicator );
	GETINTERFACE_ANY( plParticleVelMinApplicator, plAGApplicator );
};

class plParticleVelMaxApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleVelMaxApplicator );
	GETINTERFACE_ANY( plParticleVelMaxApplicator, plAGApplicator );
};

class plParticleScaleMinApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleScaleMinApplicator );
	GETINTERFACE_ANY( plParticleScaleMinApplicator, plAGApplicator );
};

class plParticleScaleMaxApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleScaleMaxApplicator );
	GETINTERFACE_ANY( plParticleScaleMaxApplicator, plAGApplicator );
};

class plParticleGravityApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleGravityApplicator );
	GETINTERFACE_ANY( plParticleGravityApplicator, plAGApplicator );
};

class plParticleDragApplicator : public plParticleApplicator
{
protected:
	virtual void IApply(const plAGModifier *mod, double time);

public:
	CLASSNAME_REGISTER( plParticleDragApplicator );
	GETINTERFACE_ANY( plParticleDragApplicator, plAGApplicator );
};

#endif
