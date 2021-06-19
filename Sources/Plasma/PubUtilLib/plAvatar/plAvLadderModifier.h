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
#ifndef plAvLadderMod_INC
#define plAvLadderMod_INC

#include "pnModifier/plSingleModifier.h"
#include "hsGeometry3.h"

// has a detector region. when a local avatar enters that region,
// creates a ladder brain and applies it to the avatar with the relevant fields
class plAvLadderMod :  public plSingleModifier
{
public:
    plAvLadderMod()
        : plSingleModifier(),
          fGoingUp(true),
          fType(kBig),
          fLoops(),
          fEnabled(true),
          fAvatarInBox(),
          fAvatarMounting()
    { }

    plAvLadderMod(bool goingUp, int type, int loops, bool enabled, const hsVector3& ladderView)
        : plSingleModifier(),
          fGoingUp(goingUp),
          fType(type),
          fLoops(loops),
          fEnabled(enabled),
          fAvatarInBox(),
          fLadderView(ladderView)
    { }

    void EmitCommand(const plKey& receiver);

    CLASSNAME_REGISTER( plAvLadderMod );
    GETINTERFACE_ANY( plAvLadderMod, plSingleModifier );
    
    // virtual void AddTarget(plSceneObject* so) {  SetTarget(so);  }
    bool MsgReceive(plMessage* msg) override;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
    
    bool GetGoingUp() const;
    void SetGoingUp(bool v);

    int GetLoops() const;
    void SetLoops(int v);

    int GetType() const;
    void SetType(int v);

    void SetEnabled(bool enabled) { fEnabled = enabled; }

protected:
    bool IEval(double secs, float del, uint32_t dirty) override { return true; }
    bool IIsReadyToClimb();
    void ITriggerSelf(const plKey& avKey);

    enum fTypeField
    {
        kBig,               // big ladders are built from a mount, N traverse loops, and a dismount
        kFourFeet,          // four-foot ladders are a one-shot
        kTwoFeet,           // two-foot (step)ladders are a one-shot
        kNumOfTypeFields,
    };

    bool fGoingUp;      // true means heading up; false means down      
    int fType;          // what type of ladder are we?
    int fLoops;         // if we're a big ladder, how many traverse loops do we need?
    hsVector3 fLadderView;
    bool fEnabled;
    bool fAvatarInBox;
    bool fAvatarMounting;   // True if the avatar is in the process of mounting the ladder.
                            // Don't try to trigger during this time.
};

#endif //plAvLadderMod_INC
