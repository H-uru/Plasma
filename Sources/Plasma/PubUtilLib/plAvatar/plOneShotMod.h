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
#ifndef plOneShotMod_INC
#define plOneShotMod_INC

#include "pnModifier/plMultiModifier.h"

// PLONESHOTMOD
// A one shot
// - suspends user input
// - moves the avatar to a specific position
// - plays a specific animation
// - returns control to the user from the final position of the animation
// This modifier holds the information that the avatar needs to do that.
class plOneShotMod : public plMultiModifier
{
protected:
    bool IEval(double secs, float del, uint32_t dirty) override { return true; }
    ST::string fAnimName;   // the name of the animation associated with this one-shot
    bool fDrivable;       // whether the user can control the position of the animation
    bool fReversable;     // whether the user can back up the animation (fDrivable must be true as well)
    float fSeekDuration;    // how long to take to get to the seek point (??? should this be speed instead?)
    float fSmartSeek;       // use smart seek to walk to the seek point?
    bool fNoSeek;
public:
    plOneShotMod();
    plOneShotMod(const ST::string &animName, bool drivable, bool reversable,
                 float seekDuration, bool smartSeek, bool noSeek = false);

    void Init(const ST::string &animName, bool drivable, bool reversable,
              float seekDuration, bool smartSeek, bool noSeek = false);

    CLASSNAME_REGISTER( plOneShotMod );
    GETINTERFACE_ANY( plOneShotMod, plMultiModifier );
    
    void AddTarget(plSceneObject* so) override;
    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};

#endif
