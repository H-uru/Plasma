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
#ifndef _pyCritterBrain_h_
#define _pyCritterBrain_h_

#include <python.h>
#include "pyGlueHelpers.h"

#include "pyKey.h"

#include <string>
#include "hsGeometry3.h"

class plAvBrainCritter;

// simply here so we can pass our message types on to python
class pyAIMsg
{
public:
	static void	AddPlasmaConstantsClasses(PyObject *m);
};

// python glue class for the critter brain
class pyCritterBrain
{
private:
	plAvBrainCritter* fBrain;

protected:
	pyCritterBrain();

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptCritterBrain);
	static PyObject* New(plAvBrainCritter* brain);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyCritterBrain object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyCritterBrain); // converts a PyObject to a pyCritterBrain (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	hsBool operator==(const pyCritterBrain& other) const;
	hsBool operator!=(const pyCritterBrain& other) const {return !(*this == other);}

	void AddReceiver(pyKey& newReceiver);
	void RemoveReceiver(pyKey& oldReceiver);

	void LocallyControlled(bool local);
	bool LocallyControlled() const;

	void AddBehavior(const std::string& animationName, const std::string& behaviorName, bool loop = true, bool randomStartPos = true,
		float fadeInLen = 2.f, float fadeOutLen = 2.f);
	void StartBehavior(const std::string& behaviorName, bool fade = true);
	bool RunningBehavior(const std::string& behaviorName) const;

	std::string BehaviorName(int behavior) const;
	std::string AnimationName(int behavior) const;
	int CurBehavior() const;
	int NextBehavior() const;

	std::string IdleBehaviorName() const;
	std::string RunBehaviorName() const;

	void GoToGoal(hsPoint3 newGoal, bool avoidingAvatars = false);
	PyObject* CurrentGoal() const; // returns ptPoint3
	bool AvoidingAvatars() const;
	bool AtGoal() const;

	void StopDistance(hsScalar stopDistance);
	hsScalar StopDistance() const;

	void SightCone(hsScalar coneRad);
	hsScalar SightCone() const;
	void SightDistance(hsScalar sightDis);
	hsScalar SightDistance() const;
	void HearingDistance(hsScalar hearDis);
	hsScalar HearingDistance() const;

	bool CanSeeAvatar(unsigned long id) const;
	bool CanHearAvatar(unsigned long id) const;

	PyObject* PlayersICanSee() const; // returns list of unsigned longs
	PyObject* PlayersICanHear() const; // returns list of unsigned longs

	PyObject* VectorToPlayer(unsigned long id) const; // returns ptVector3
};

#endif // _pyCritterBrain_h_