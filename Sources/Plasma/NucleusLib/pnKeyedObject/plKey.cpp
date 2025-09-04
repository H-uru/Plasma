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
//////////////////////////////////////////////////////////////////////////////
//
//  plKey - An opaque pointer to actual key data, so that we can keep track
//          of how many people have pointers (plKeys) to key data (plKeyImp)
//          and destroy the key data when it's been fully unreffed.
//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plKey.h"
#include "plUoid.h"
#include "hsResMgr.h"

#define TRACK_REFS 0 // MEMLEAKFISH

#if TRACK_REFS
#include <string_theory/format>

#include "plCreatableIndex.h"
#include "plClassIndexMacros.h"
#include "plTweak.h"

int mlfTrack = 1;

static const char* keyNameToLookFor = "AgeSDLHook";
static const uint16_t CLASS_TO_TRACK = CLASS_INDEX_SCOPED(plSceneObject);
static const int kCloneID = 0;
static const int kClonePlayerID = 0;
static const int kLocSeq = -1;

class keyDataFriend : public plKeyData
{
public:
    uint16_t RefCount() const { return fRefCount; }
};

static int IsTracked(const plKeyData* keyData)
{
    if( mlfTrack && keyData )
    {
        if( !keyData->GetUoid().GetObjectName().compare_i(keyNameToLookFor)
            && (keyData->GetUoid().GetClassType() == CLASS_TO_TRACK) )
        {
            if( (kCloneID < 0)
                ||(kCloneID == keyData->GetUoid().GetCloneID()) )
            {
                if( (kLocSeq < 0)
                    ||(kLocSeq == keyData->GetUoid().GetLocation().GetSequenceNumber()) )
                {
                    plConst(uint16_t) kMinRefCount(0);
                    const keyDataFriend* kdf = (keyDataFriend*)keyData;
                    if( kdf->RefCount() > kMinRefCount )
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static ST::string CloneString(const plKeyData* keyData)
{
    if (keyData) {
        return ST::format("CID:{}, CPID:{} LOC:{}",
            keyData->GetUoid().GetCloneID(), 
            keyData->GetUoid().GetClonePlayerID(), 
            keyData->GetUoid().GetLocation().GetSequenceNumber());
    } else {
        return ST_LITERAL("nil");
    }
}
#endif


plKey::plKey(const plKey& rhs) : fKeyData(rhs.fKeyData)
{
#if TRACK_REFS  // FOR DEBUGGING ONLY
    if( IsTracked(fKeyData) )
    {
        ST::string msg = ST::format("C: Key {} {} is being constructed using the plKey(plKey&) constructor", keyNameToLookFor, CloneString(fKeyData));
        //hsAssert(false, msg.c_str());
        hsStatusMessage(msg);
    }
#endif
    IIncRef();
}

plKey::plKey(plKeyData* data) : fKeyData(data)
{
#if TRACK_REFS  // FOR DEBUGGING ONLY
    if( IsTracked(fKeyData) )
    {
        ST::string msg = ST::format("C: Key {} {} is being constructed using the plKey(plKeyData*) constructor", keyNameToLookFor, CloneString(fKeyData));
        //hsAssert(false, msg.c_str());
        hsStatusMessage(msg);
    }
#endif
    IIncRef();
}

plKey::~plKey()
{
#if TRACK_REFS  // FOR DEBUGGING ONLY
    if( IsTracked(fKeyData) )
    {
        ST::string msg = ST::format("D: Key {} {} is being destructed", keyNameToLookFor, CloneString(fKeyData));
        //hsAssert(false, msg.c_str());
        hsStatusMessage(msg);
    }
#endif
    IDecRef();
}

plKey &plKey::operator=( const plKey &rhs )
{
#if TRACK_REFS  // FOR DEBUGGING ONLY
    if( fKeyData != rhs.fKeyData )
    {
        if( IsTracked(rhs.fKeyData) )
        {
            ST::string msg;
            if (fKeyData == nullptr)
                msg = ST::format("=: Key {} {} is being assigned to a nil key",
                    keyNameToLookFor, CloneString(rhs.fKeyData));
            else
                msg = ST::format("=: Key {} {} is being assigned to {}",
                    keyNameToLookFor, CloneString(rhs.fKeyData),
                    fKeyData->GetUoid().GetObjectName());
            //hsAssert(false, msg.c_str());
            hsStatusMessage(msg);
        }
        if( IsTracked(fKeyData) )
        {
            ST::string msg;
            if (fKeyData == nullptr)
                msg = ST::format("=: Nil key is being assigned to {} {}",
                    keyNameToLookFor, CloneString(fKeyData));
            else
                msg = ST::format("=: Key {} {} is being assigned to {}",
                    fKeyData->GetUoid().GetObjectName(),
                    CloneString(fKeyData), keyNameToLookFor);
            //hsAssert(false, msg.c_str());
            hsStatusMessage(msg);
        }
    }
#endif
    if( fKeyData != rhs.fKeyData )
    {
        IDecRef();
        fKeyData = rhs.fKeyData;
        IIncRef();
    }

    return *this;
}

plKey &plKey::operator=(std::nullptr_t)
{
#if TRACK_REFS  // FOR DEBUGGING ONLY
    if (IsTracked(fKeyData))
    {
        ST::string msg = ST::format("D: Key {} {} is being nilified", keyNameToLookFor, CloneString(fKeyData));
        //hsAssert(false, msg.c_str());
        hsStatusMessage(msg);
    }
#endif

    IDecRef();
    fKeyData = nullptr;

    return *this;
}

plKeyData   *plKey::operator->() const
{
    return fKeyData;
}

plKeyData   &plKey::operator*() const
{
    return *fKeyData;
}

void plKey::IIncRef()
{
    if (!fKeyData)
        return;

    hsAssert(fKeyData->fRefCount < 0xffff, "Too many refs to plKeyImp");
    fKeyData->fRefCount++;

#if TRACK_REFS  // FOR DEBUGGING ONLY
    if( IsTracked(fKeyData) )
    {
        ST::string msg = ST::format("+: Key {} {} is being reffed! Refcount: {}", keyNameToLookFor, CloneString(fKeyData), fKeyData->fRefCount);
        //hsAssert(false, msg.c_str());
        hsStatusMessage(msg);
    }
#endif

    if (fKeyData->fRefCount == 1)
        hsgResMgr::ResMgr()->IKeyReffed((plKeyImp*)fKeyData);
}

void plKey::IDecRef()
{
    if (!fKeyData)
        return;

    hsAssert(fKeyData->fRefCount, "Dec'ing ref on unreffed key");
    fKeyData->fRefCount--;

#if TRACK_REFS  // FOR DEBUGGING ONLY
    if( IsTracked(fKeyData) )
    {
        ST::string msg = ST::format("-: Key {} {} is being de-reffed! Refcount: {}", keyNameToLookFor, CloneString(fKeyData), fKeyData->fRefCount);
        //hsAssert(false, msg.c_str());
        hsStatusMessage(msg);
    }
#endif

    if (fKeyData->fRefCount == 0)
        hsgResMgr::ResMgr()->IKeyUnreffed((plKeyImp*)fKeyData);
}

//// plKeyData ///////////////////////////////////////////////////////////////
//  Our base class of key data

plKeyData::plKeyData()
{
    fRefCount = 0;
}

plKeyData::~plKeyData()
{
    //  hsAssert(fRefCount == 0, "Deleting key data when someone still has a ref to it");
}


