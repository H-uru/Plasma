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
#ifndef _pyCritterBrain_h_
#define _pyCritterBrain_h_

#include "pyGlueDefinitions.h"

struct hsPoint3;
class plAvBrainCritter;
class pyKey;
namespace ST { class string; }

// simply here so we can pass our message types on to python
class pyAIMsg
{
public:
    static void AddPlasmaConstantsClasses(PyObject *m);
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

    bool operator==(const pyCritterBrain& other) const;
    bool operator!=(const pyCritterBrain& other) const {return !(*this == other);}

    void AddReceiver(pyKey& newReceiver);
    void RemoveReceiver(pyKey& oldReceiver);

    PyObject* GetSceneObject();

    void AddBehavior(const ST::string& animationName, const ST::string& behaviorName, bool loop = true, bool randomStartPos = true,
        float fadeInLen = 2.f, float fadeOutLen = 2.f);
    void StartBehavior(const ST::string& behaviorName, bool fade = true);
    bool RunningBehavior(const ST::string& behaviorName) const;

    ST::string BehaviorName(int behavior) const;
    ST::string AnimationName(int behavior) const;
    int CurBehavior() const;
    int NextBehavior() const;

    ST::string IdleBehaviorName() const;
    ST::string RunBehaviorName() const;

    void GoToGoal(hsPoint3 newGoal, bool avoidingAvatars = false);
    PyObject* CurrentGoal() const; // returns ptPoint3
    bool AvoidingAvatars() const;
    bool AtGoal() const;

    void StopDistance(float stopDistance);
    float StopDistance() const;

    void SightCone(float coneRad);
    float SightCone() const;
    void SightDistance(float sightDis);
    float SightDistance() const;
    void HearingDistance(float hearDis);
    float HearingDistance() const;

    bool CanSeeAvatar(unsigned long id) const;
    bool CanHearAvatar(unsigned long id) const;

    PyObject* PlayersICanSee() const; // returns list of unsigned longs
    PyObject* PlayersICanHear() const; // returns list of unsigned longs

    PyObject* VectorToPlayer(unsigned long id) const; // returns ptVector3
};

#endif // _pyCritterBrain_h_