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
#ifndef plSittingModifier_inc
#define plSittingModifier_inc

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////

#include "pnModifier/plSingleModifier.h"    // base class
#include "pnKeyedObject/plKey.h"            // for the notification keys
#include <vector>                           // for the vector they're kept in

/////////////////////////////////////////////////////////////////////////////////////////
//
// DECLARATIONS
//
/////////////////////////////////////////////////////////////////////////////////////////
class plNotifyMsg;
class plAvBrainGeneric;
class plArmatureMod;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

class plSittingModifier : public plSingleModifier
{
public: 
    enum
    {
        kApproachFront  = 0x01,
        kApproachLeft   = 0x02,
        kApproachRight  = 0x04,
        kApproachRear   = 0x08,
        kApproachMask   = kApproachFront | kApproachLeft | kApproachRight | kApproachRear,
        kDisableForward = 0x10,
    };
    
    uint8_t fMiscFlags;   

    plSittingModifier();
    plSittingModifier(bool hasFront, bool hasLeft, bool hasRight);
    virtual ~plSittingModifier();

    CLASSNAME_REGISTER( plSittingModifier );
    GETINTERFACE_ANY( plSittingModifier, plSingleModifier );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    bool MsgReceive(plMessage *msg) override;

    void AddNotifyKey(plKey key) { fNotifyKeys.emplace_back(std::move(key)); }

    virtual void Trigger(const plArmatureMod *avMod, plNotifyMsg *enterNotify, plNotifyMsg *exitNotify);
    virtual void UnTrigger();

protected:
    /** We've been triggered: go ahead and send the seek and brain tasks to the 
        triggering avatar. */
    bool IEmitCommand(plKey playerKey, plMessage *enterCallback, plMessage *exitCallback);

    /** Set up generic notification messages which were passed in by the responder / 
        max authoring stuff. */
    void ISetupNotify(plNotifyMsg *notifyMsg, plNotifyMsg *originalNotify);

    /** Figure out which approach we should use to the sit target, and add the relevant
        stages to the brain. */
    plAvBrainGeneric * IBuildSitBrain(const plKey& avModKey, const plKey& seekKey, const char **pAnimName, plNotifyMsg *enterNotify, plNotifyMsg *exitNotify);

    /** Unused. */
    bool IEval(double secs, float del, uint32_t dirty) override { return true; }

    /** A vector of keys to objects that are interested in receiving our sit messages. */
    std::vector<plKey> fNotifyKeys;

    /** The chair in question is in use. It will untrigger when the avatar leaves it. */
    //bool              fTriggered;
    plKey           fTriggeredAvatarKey;
};



#endif //plSittingModifier_inc
