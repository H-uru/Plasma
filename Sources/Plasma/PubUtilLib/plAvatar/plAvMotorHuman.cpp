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
#pragma once

#ifndef PLAVMOTORHUMAN_INC

/*** \file plAvMotorHuman.h
*/

#include <hkdynamics\action\action.h>

class plAvBrainHuman;

/** \class plAvMotorHuman
    Applies avatar at simulation frequency rather than frame frequency.
*/
class plAvMotorHuman : public Havok::Action
{
public:
    plAvMotorHuman();

    /** Called from havok during integration; converts time to plasma format
        and asks the brain to move. Brain will sample animation at current time
        and apply velocity or forces to the body.
        */
    virtual void apply(Havok::Subspace &subspace, Havok::hkTime time);

    /** Sets the motor to control a different human brain.
        Returns the previously controlled brain, or nil.
        */
    plAvBrainHuman * SetBrain(plAvBrainHuman *brain);

    /** Which brain is this motor currently controlling?
        */
    plAvBrainHuman * GetBrain();

private:
    /** The brain we get our movement data from */
    plAvBrainHuman *fBrain;
    double fLastTime;
};



/*** \file plAvMotorHuman.cpp
*/


#include "plAvBrainHuman.h"

plAvMotorHuman::plAvMotorHuman()
: fBrain(0),
  fLastTime(0.0f)
{
}

//plAvMotorHuman::plAvMotorHuman(plAvBrainHuman *brain)
//: fBrain(brain),
//  fLastTime(0.0f);

void plAvMotorHuman::apply(Havok::Subspace &subspace, Havok::hkTime time)
{
    double timeNow = time.asDouble();
    double elapsedD = timeNow - fLastTime;
    float elapsed = elapsedD;

    if(elapsed > 1.0f)
    {
        elapsed = 0.1f;
        hsStatusMessageF("Clamping plAvMotorHuman::apply interval to <%f>", elapsed);

        fLastTime = timeNow - elapsed;
    }

//  fBrain->MoveViaAnimation(timeNow, elapsed);
}

plAvBrainHuman * plAvMotorHuman::SetBrain(plAvBrainHuman *brain)
{
    plAvBrainHuman * oldBrain = fBrain;

    fBrain = brain;

    return oldBrain;
}

plAvBrainHuman * plAvMotorHuman::GetBrain()
{
    return fBrain;
}

#endif
