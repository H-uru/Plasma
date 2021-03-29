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
/** \file plAvBrainDrive.h
    */
#ifndef AVBRAINDRIVE_INC
#define AVBRAINDRIVE_INC

// base class
#include "plAvBrain.h"

/** A simple brain used to steer the player around without regard
    for physics. Used in production to quickly fly through a scene
    without having to actually solve puzzles, jump, etc.
    The 'Drive Brain' uses the same input keys as the avatar, with
    a few secret additions for convenience. At the time of this
    writing, you invoke the drive brain by pressing shift-P, and
    then use the forward and back arrows to move and th e left and 
    right arrows to rotate. The 'u' and 'j' keys will move your avatar
    vertically.
    Gravity and collision are completely suspended for avatars in
    drive mode, but they *can* still trigger region detectors if
    you need to fire off triggers, puzzles, or whatnot.
    Note that the drive brain inherits from the user brain, which
    parses control input for us.
*/
class plAvBrainDrive : public plArmatureBrain
{
public:
    plAvBrainDrive();
    /** Canonical constructer. Use this one.
        \param maxVelocity The highest speed this avatar can fly at.
        \param turnRate The speed at which we will turn, in radians per second.
        */
    plAvBrainDrive(float maxVelocity, float turnRate);


    // BRAIN PROTOCOL
    /** Suspend physics and get in line to receive keyboard control messages. */
    void Activate(plArmatureModBase *avMod) override;

    /** Restore physical reality and stop handling input messages */
    void Deactivate() override;

    /** Look at the key states and figure out if and how we should move */
    bool Apply(double timeNow, float elapsed) override;     // main control loop. called by avatar eval()

    // the user brain base handles most of the details of control messages,
    // so this function just looks for the special command which gets us out
    // of drive mode. 
    bool MsgReceive(plMessage* pMsg) override; // handle control input from the user

    CLASSNAME_REGISTER( plAvBrainDrive );
    GETINTERFACE_ANY( plAvBrainDrive, plArmatureBrain );

protected:
    void IEnablePhysics(bool enable, plKey avKey);

    void IToggleCtrlCodes(bool on) const;

    float    fMaxVelocity;
    float    fTurnRate;
};

#endif // AVBRAINDRIVE_INC
