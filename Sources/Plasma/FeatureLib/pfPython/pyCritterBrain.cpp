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
#include "pyCritterBrain.h"
#include "pyGeometry3.h"

#include "..\plAvatar\plAvBrainCritter.h"

pyCritterBrain::pyCritterBrain(): fBrain(nil) {}

hsBool pyCritterBrain::operator==(const pyCritterBrain& other) const
{
	// pointer compare
	return fBrain == other.fBrain;
}

void pyCritterBrain::AddReceiver(pyKey& newReceiver)
{
	if (!fBrain)
		return;
	fBrain->AddReceiver(newReceiver.getKey());
}

void pyCritterBrain::RemoveReceiver(pyKey& oldReceiver)
{
	if (!fBrain)
		return;
	fBrain->RemoveReceiver(oldReceiver.getKey());
}

void pyCritterBrain::LocallyControlled(bool local)
{
	if (!fBrain)
		return;
	fBrain->LocallyControlled(local);
}

bool pyCritterBrain::LocallyControlled() const
{
	if (!fBrain)
		return false;
	return fBrain->LocallyControlled();
}

void pyCritterBrain::AddBehavior(const std::string& animationName, const std::string& behaviorName, bool loop /* = true */,
	bool randomStartPos /* = true */, float fadeInLen /* = 2.f */, float fadeOutLen /* = 2.f */)
{
	if (!fBrain)
		return;
	fBrain->AddBehavior(animationName, behaviorName, loop, randomStartPos, fadeInLen, fadeOutLen);
}

void pyCritterBrain::StartBehavior(const std::string& behaviorName, bool fade /* = true */)
{
	if (!fBrain)
		return;
	fBrain->StartBehavior(behaviorName, fade);
}

bool pyCritterBrain::RunningBehavior(const std::string& behaviorName) const
{
	if (!fBrain)
		return false;
	return fBrain->RunningBehavior(behaviorName);
}

std::string pyCritterBrain::BehaviorName(int behavior) const
{
	if (!fBrain)
		return false;
	return fBrain->BehaviorName(behavior);
}

std::string pyCritterBrain::AnimationName(int behavior) const
{
	if (!fBrain)
		return false;
	return fBrain->AnimationName(behavior);
}

int pyCritterBrain::CurBehavior() const
{
	if (!fBrain)
		return false;
	return fBrain->CurBehavior();
}

int pyCritterBrain::NextBehavior() const
{
	if (!fBrain)
		return false;
	return fBrain->NextBehavior();
}

std::string pyCritterBrain::IdleBehaviorName() const
{
	if (!fBrain)
		return false;
	return fBrain->IdleBehaviorName();
}

std::string pyCritterBrain::RunBehaviorName() const
{
	if (!fBrain)
		return false;
	return fBrain->RunBehaviorName();
}

void pyCritterBrain::GoToGoal(hsPoint3 newGoal, bool avoidingAvatars /* = false */)
{
	if (!fBrain)
		return;
	fBrain->GoToGoal(newGoal, avoidingAvatars);
}

PyObject* pyCritterBrain::CurrentGoal() const
{
	if (!fBrain)
		PYTHON_RETURN_NONE;
	return pyPoint3::New(fBrain->CurrentGoal());
}

bool pyCritterBrain::AvoidingAvatars() const
{
	if (!fBrain)
		return false;
	return fBrain->AvoidingAvatars();
}

bool pyCritterBrain::AtGoal() const
{
	if (!fBrain)
		return false;
	return fBrain->AtGoal();
}

void pyCritterBrain::StopDistance(hsScalar stopDistance)
{
	if (!fBrain)
		return;
	fBrain->StopDistance(stopDistance);
}

hsScalar pyCritterBrain::StopDistance() const
{
	if (!fBrain)
		return 0;
	return fBrain->StopDistance();
}

void pyCritterBrain::SightCone(hsScalar coneRad)
{
	if (!fBrain)
		return;
	fBrain->SightCone(coneRad);
}

hsScalar pyCritterBrain::SightCone() const
{
	if (!fBrain)
		return 0;
	return fBrain->SightCone();
}

void pyCritterBrain::SightDistance(hsScalar sightDis)
{
	if (!fBrain)
		return;
	fBrain->SightDistance(sightDis);
}

hsScalar pyCritterBrain::SightDistance() const
{
	if (!fBrain)
		return 0;
	return fBrain->SightDistance();
}

void pyCritterBrain::HearingDistance(hsScalar hearDis)
{
	if (!fBrain)
		return;
	fBrain->HearingDistance(hearDis);
}

hsScalar pyCritterBrain::HearingDistance() const
{
	if (!fBrain)
		return 0;
	return fBrain->HearingDistance();
}

bool pyCritterBrain::CanSeeAvatar(unsigned long id) const
{
	if (!fBrain)
		return false;
	return fBrain->CanSeeAvatar(id);
}

bool pyCritterBrain::CanHearAvatar(unsigned long id) const
{
	if (!fBrain)
		return false;
	return fBrain->CanHearAvatar(id);
}

PyObject* pyCritterBrain::PlayersICanSee() const
{
	if (!fBrain)
		PYTHON_RETURN_NONE;
	std::vector<unsigned long> players = fBrain->PlayersICanSee();
	PyObject* retVal = PyList_New(players.size());
	for (unsigned i = 0; i < players.size(); ++i)
		PyList_SetItem(retVal, i, PyLong_FromUnsignedLong(players[i]));
	return retVal;
}

PyObject* pyCritterBrain::PlayersICanHear() const
{
	if (!fBrain)
		PYTHON_RETURN_NONE;
	std::vector<unsigned long> players = fBrain->PlayersICanHear();
	PyObject* retVal = PyList_New(players.size());
	for (unsigned i = 0; i < players.size(); ++i)
		PyList_SetItem(retVal, i, PyLong_FromUnsignedLong(players[i]));
	return retVal;
}

PyObject* pyCritterBrain::VectorToPlayer(unsigned long id) const
{
	if (!fBrain)
		PYTHON_RETURN_NONE;
	return pyVector3::New(fBrain->VectorToPlayer(id));
}