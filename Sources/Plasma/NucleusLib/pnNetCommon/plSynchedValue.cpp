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
#include "plSynchedValue.h"

#ifdef USE_SYNCHED_VALUES
#include "pnKeyedObject/plKey.h"
#include "hsBitVector.h"
#include "pnSceneObject/plSceneObject.h"
#include "hsResMgr.h"
#include "pnKeyedObject/plUoid.h"
#include "pnSceneObject/plCoordinateInterface.h"

#define ISaveOrLoadSimpleType() \
{ \
    if (save) \
        stream->WriteLE(v); \
    else \
        stream->ReadLE(&v); \
    return v; \
}

//
// baseclass save/load methods for various types
//
hsBitVector plSynchedValueBase::ISaveOrLoad(hsBitVector& v, bool32 save, hsStream* stream, hsResMgr* mgr) 
{
    if (save)
        v.Write(stream);
    else
        v.Read(stream);
    return v;
}

float plSynchedValueBase::ISaveOrLoad(float v, bool32 save, hsStream* stream, hsResMgr* mgr) 
    ISaveOrLoadSimpleType();

double plSynchedValueBase::ISaveOrLoad(double v, bool32 save, hsStream* stream, hsResMgr* mgr) 
    ISaveOrLoadSimpleType();

int32_t plSynchedValueBase::ISaveOrLoad(int32_t v, bool32 save, hsStream* stream, hsResMgr* mgr) 
    ISaveOrLoadSimpleType();

uint32_t plSynchedValueBase::ISaveOrLoad(uint32_t v, bool32 save, hsStream* stream, hsResMgr* mgr) 
    ISaveOrLoadSimpleType();

int plSynchedValueBase::ISaveOrLoad(int v, bool32 save, hsStream* stream, hsResMgr* mgr) 
    ISaveOrLoadSimpleType();

bool plSynchedValueBase::ISaveOrLoad(bool v, bool32 save, hsStream* stream, hsResMgr* mgr) 
    ISaveOrLoadSimpleType();

//
// save or load a key.  return the key.
//
const plKey plSynchedValueBase::ISaveOrLoad(const plKey key, bool32 save, hsStream* stream, hsResMgr* mgr)
{
    if (save)
    {   
        if (key)
        {
            stream->WriteByte(1);
            // I need to write a key to MY stream...
#if 0       // DEBUG
            plStringBuffer<char> buf = key->GetName()->ToIso8859_1();
            stream->WriteLE32(buf.GetSize());
            stream->Write(buf.GetSize(), buf.GetData());
#endif
            key->GetUoid().Write(stream);
        }
        else
        {
            stream->WriteByte(0);
        }
        return key;
    }
    else
    {   
        int32_t has=stream->ReadByte();
        if (has)
        {
            // read a key from MY stream
#if 0       // DEBUG
            int32_t len = stream->ReadLE32();
            char tmp[256];
            hsAssert(len<256, "key name overflow");
            stream->Read(len, tmp);
#endif
            plUoid uoid;
            uoid.Read(stream);
            return mgr->FindKey(uoid);
        }
        else
            return (nil);
    }
    return nil;
}

hsKeyedObject* plSynchedValueBase::ISaveOrLoad(hsKeyedObject* obj, bool32 save, hsStream* stream, hsResMgr* mgr) 
{
    plKey key = obj ? obj->GetKey() : nil;
    key = ISaveOrLoad(key, save, stream, mgr);
    return key ? key->ObjectIsLoaded() : nil;
}

plSceneNode* plSynchedValueBase::ISaveOrLoad(plSceneNode* obj, bool32 save, hsStream* stream, hsResMgr* mgr)
{ 
    // return plSceneNode::ConvertNoRef(ISaveOrLoad(hsKeyedObject::ConvertNoRef(obj), save, stream, mgr)); 
    hsAssert(false, "SceneNode synchedValues currently not implemented");
    return nil;
}

plSceneObject* plSynchedValueBase::ISaveOrLoad(plSceneObject* obj, bool32 save, hsStream* stream, hsResMgr* mgr)
{ return plSceneObject::ConvertNoRef(ISaveOrLoad(hsKeyedObject::ConvertNoRef(obj), save, stream, mgr)); }

plCoordinateInterface* plSynchedValueBase::ISaveOrLoad(const plCoordinateInterface* cInt, bool32 save, hsStream* stream, hsResMgr* mgr)
{
    plSceneObject* obj = ISaveOrLoad(cInt ? cInt->fOwner : nil, save, stream, mgr);
    return obj ? obj->fCoordinateInterface : nil;
}

#else

// dummy function to prevent a linker warning complaining about no public symbols if the
// contents of the file get compiled out via pre-processor
void _preventLNK4221WarningStub()
{
}

#endif  // USE_SYNCHED_VALUES

