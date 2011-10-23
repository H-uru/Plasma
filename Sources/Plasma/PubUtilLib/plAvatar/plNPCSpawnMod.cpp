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
#include "plNPCSpawnMod.h"

// local
#include "plAvatarMgr.h"

// global
#include "hsMatrix44.h"

// other
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
//#include "pnMessage/plWarpMsg.h"
#include "pnMessage/plNotifyMsg.h"


// plNPCSpawnMod ctor
plNPCSpawnMod::plNPCSpawnMod()
: fModelName(nil),
  fAccountName(nil),
  fAutoSpawn(false),
  fNotify(nil)
{
}

// plNPCSpawnMod ctor modelName accountName
plNPCSpawnMod::plNPCSpawnMod(const char * modelName, const char * accountName, bool autoSpawn)
: fAutoSpawn(autoSpawn), fNotify(nil)
{
    fModelName = hsStrcpy(modelName);
    fAccountName = hsStrcpy(accountName);
}

// plNPCSpawnMod dtor
plNPCSpawnMod::~plNPCSpawnMod()
{
    if(fModelName)
    {
        delete[] fModelName;
        fModelName = nil;
    }
    if(fAccountName)
    {
        delete[] fAccountName;
        fAccountName = nil;
    }
    if (fNotify)
        fNotify->UnRef();
}

void plNPCSpawnMod::AddTarget(plSceneObject* so)
{
    plSingleModifier::AddTarget(so);

    if(fAutoSpawn)
        Trigger();
}

void plNPCSpawnMod::RemoveTarget(plSceneObject *so)
{
    plSingleModifier::RemoveTarget(so);

    if(fSpawnedKey)
    {
        plAvatarMgr::GetInstance()->UnLoadAvatar(fSpawnedKey, false);
    }
}

// TRIGGER
bool plNPCSpawnMod::Trigger()
{
    bool result = false;

    // you can ONLY spawn if you are local. the spawn message
    // will netpropagate
    if(this->IsLocallyOwned())
    {
        if(fModelName)
        {
            // spawn the NPC
            plKey spawnPoint = GetTarget(0)->GetKey();

            fSpawnedKey = plAvatarMgr::GetInstance()->LoadAvatar(fModelName, fAccountName, false, spawnPoint, nil);

            ISendNotify(fSpawnedKey);
        }
    }

    return result;
}

// SETNOTIFY
void plNPCSpawnMod::SetNotify(plNotifyMsg *notify)
{
    fNotify = notify;
}

// GETNOTIFY
plNotifyMsg * plNPCSpawnMod::GetNotify()
{
    return fNotify;
}

// READ
void plNPCSpawnMod::Read(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Read(stream, mgr);

    fModelName = stream->ReadSafeString();
    fAccountName = stream->ReadSafeString();
    fAutoSpawn = stream->Readbool();
    if(stream->Readbool())
        fNotify = plNotifyMsg::ConvertNoRef(mgr->ReadCreatable(stream));
}

// WRITE
void plNPCSpawnMod::Write(hsStream *stream, hsResMgr *mgr)
{
    plSingleModifier::Write(stream, mgr);
    
    stream->WriteSafeString(fModelName);
    stream->WriteSafeString(fAccountName);
    stream->Writebool(fAutoSpawn);
    if(fNotify)
    {
        stream->Writebool(true);
        mgr->WriteCreatable(stream, fNotify);
    } else {
        stream->Writebool(false);
    }
}

// IEVAL
// attack of the bogons
hsBool plNPCSpawnMod::IEval(double secs, hsScalar del, UInt32 dirty)
{
    return true;
}

// ISENDNOTIFY
void plNPCSpawnMod::ISendNotify(plKey &avatarKey)
{
    if(fNotify)
    {
        proSpawnedEventData * event = TRACKED_NEW proSpawnedEventData;
        event->fSpawner = GetKey();
        event->fSpawnee = avatarKey;
        fNotify->ClearEvents();
        fNotify->AddEvent(event);
        delete event; // fNotify->AddEvent makes a copy
        int i = fNotify->GetEventCount();
        fNotify->Ref();     // so we still hold onto it after it is delivered
        fNotify->Send();
    } else {
        hsStatusMessage("NPC Spawner is spawning but there is no notify message to send.");
    }
}