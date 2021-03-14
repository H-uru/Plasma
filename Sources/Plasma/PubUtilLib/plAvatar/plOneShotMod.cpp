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

// singular
#include "plOneShotMod.h"

// local
#include "plArmatureMod.h"
#include "plAvatarMgr.h"

// global
#include "plgDispatch.h"

// other
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plAvatarMsg.h"
#include "plMessage/plOneShotCallbacks.h"
#include "plMessage/plOneShotMsg.h"

// CTOR()
plOneShotMod::plOneShotMod()
: fDrivable(false),
  fReversable(false),
  fSeekDuration(1.0f),
  fSmartSeek(false),
  fNoSeek(false)
{
    // this constructor is called from the loader. 
}

// CTOR(char *)
plOneShotMod::plOneShotMod(const ST::string &animName,
                           bool drivable,
                           bool reversable,
                           float seekDuration,
                           bool smartSeek,
                           bool noSeek)
: fAnimName(animName),
  fDrivable(drivable),
  fReversable(reversable),
  fSeekDuration(seekDuration),
  fSmartSeek((float)smartSeek),
  fNoSeek(noSeek)
{
}

// INIT
void plOneShotMod::Init(const ST::string &animName,
                        bool drivable,
                        bool reversable,
                        float seekDuration,
                        bool smartSeek,
                        bool noSeek)
{
    fAnimName = animName;
    fDrivable = drivable;
    fReversable = reversable;
    fSeekDuration = seekDuration;
    fSmartSeek = (float)smartSeek;
    fNoSeek = noSeek;
}


// MSGRECEIVE
bool plOneShotMod::MsgReceive(plMessage* msg)
{
    plOneShotMsg *oneShotMsg = plOneShotMsg::ConvertNoRef(msg);
    if (oneShotMsg)
    {
        // Send a one shot task request to the given target, which darn well better have an avatar modifier on it.
        plKey myKey = GetKey();
        plKey objKey = GetTarget(0)->GetKey();
        plKey avKey = oneShotMsg->fPlayerKey;
        hsAssert(avKey,"The avatar key is missing in the one shot!");

        if ( avKey )
        {
            plSceneObject *avObj = (plSceneObject *)avKey->ObjectIsLoaded();
            if(avObj)
            {
                const plArmatureMod *avMod = (plArmatureMod*)avObj->GetModifierByType(plArmatureMod::Index());

                if(avMod)
                {
                    ST::string animName = avMod->MakeAnimationName(fAnimName);

                    plAvOneShotMsg *avOSmsg = new plAvOneShotMsg(myKey, oneShotMsg->fPlayerKey, objKey,
                                                                 fSeekDuration, (bool)fSmartSeek, animName, fDrivable,
                                                                 fReversable);

                    avOSmsg->fNoSeek = fNoSeek;
                    avOSmsg->SetBCastFlag(plMessage::kPropagateToModifiers);
                    hsRefCnt_SafeRef(oneShotMsg->fCallbacks);
                    avOSmsg->fCallbacks = oneShotMsg->fCallbacks;
                    plgDispatch::MsgSend(avOSmsg);
                }
            }

        }
        return true;
    }
    return plMultiModifier::MsgReceive(msg);
}

// ADDTARGET
// Here I am. Announce presence to the avatar registry.
void plOneShotMod::AddTarget(plSceneObject* so)
{
    plMultiModifier::AddTarget(so);
    
    plAvatarMgr::GetInstance()->AddOneShot(this);
}

void plOneShotMod::Read(hsStream *stream, hsResMgr *mgr)
{
    plMultiModifier::Read(stream, mgr);

    // read in the name of the animation itself
    fAnimName = stream->ReadSafeString();
    fSeekDuration = stream->ReadLEFloat();
    fDrivable = stream->ReadBool();
    fReversable = stream->ReadBool();
    fSmartSeek = (float)stream->ReadBool();
    fNoSeek = stream->ReadBool();
}

void plOneShotMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plMultiModifier::Write(stream, mgr);

    stream->WriteSafeString(fAnimName);
    stream->WriteLEFloat(fSeekDuration);
    stream->WriteBool(fDrivable);
    stream->WriteBool(fReversable);
    stream->WriteBool((bool)fSmartSeek);
    stream->WriteBool(fNoSeek);
}
