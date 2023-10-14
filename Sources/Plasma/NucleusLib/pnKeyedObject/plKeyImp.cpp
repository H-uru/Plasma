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
#include "plKeyImp.h"
#include "hsStream.h"
#include "hsKeyedObject.h"
#include "hsResMgr.h"
#include "HeadSpin.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plSelfDestructMsg.h"
#include "hsTimer.h"
#include "plProfile.h"
#include "plgDispatch.h"

#if defined(HS_DEBUGGING) || defined(LOG_ACTIVE_REFS)
#include "pnFactory/plFactory.h"
#endif

plProfile_CreateMemCounter("Keys", "Memory", KeyMem);

static uint32_t CalcKeySize(plKeyImp* key)
{
    uint32_t nameLen = 0;
    if (!key->GetUoid().GetObjectName().empty())
        nameLen = key->GetUoid().GetObjectName().size() + 1;
    return sizeof(plKeyImp) + nameLen;
}

//#define LOG_ACTIVE_REFS
#ifdef LOG_ACTIVE_REFS
#include "plCreatableIndex.h"
static const char* kObjName = "GUI_District_OptionsMenuGUI";
static uint16_t kClassType = CLASS_INDEX_SCOPED(plSceneNode);
static uint32_t kCloneID = 0;
bool IsTrackedKey(const plKeyImp* key)
{
    return (key->GetName() == kObjName) &&
            key->GetUoid().GetClassType() == kClassType &&
            key->GetUoid().GetCloneID() == kCloneID;
}
#endif

hsKeyedObject* plKeyImp::SafeGetObject(const plKeyImp* key) {
    return key ? key->fObjectPtr : nullptr;
}

plKeyImp::plKeyImp() :
    fObjectPtr(),
    fStartPos(-1),
    fDataLen(-1),
    fNumActiveRefs(),
    fPendingRefs(1)
{
#ifdef HS_DEBUGGING
    fClassType = nullptr;
#endif
}

plKeyImp::plKeyImp(plUoid u, uint32_t pos,uint32_t len):
    fUoid(std::move(u)),
    fObjectPtr(),
    fStartPos(pos),
    fDataLen(len),
    fNumActiveRefs(),
    fPendingRefs(1)
{
    plProfile_NewMem(KeyMem, CalcKeySize(this));

#ifdef HS_DEBUGGING
    fIDName = fUoid.GetObjectName();
    fClassType = plFactory::GetNameOfClass( fUoid.GetClassType() );
#endif
}

plKeyImp::~plKeyImp()
{
    plProfile_DelMem(KeyMem, CalcKeySize(this));

#if defined(HS_DEBUGGING) && 0
    // Colin debugging
    char buf[512];
    sprintf(buf, "0x%x %s %s\n", this, fIDName, fClassType);
    hsStatusMessage(buf);
#endif

    hsAssert(fObjectPtr == nullptr, "Deleting non-nil key!  Bad idea!");

    if (fCloneOwner != nullptr)
    {
        // Must be a clone, remove us from our parent list
        plKeyImp::GetFromKey(fCloneOwner)->RemoveClone(this);
    }

    for (plKeyImp* clone : fClones)
    {
        if (clone)
            clone->UnRegister();
    }
    fClones.clear();

    // This is normally empty by now, but if we never got loaded,
    // there will be unsent ref messages in the NotifyCreated list
    ClearNotifyCreated();
}

void plKeyImp::SetUoid(const plUoid& uoid)
{
    fUoid = uoid;
#ifdef HS_DEBUGGING
    fIDName = fUoid.GetObjectName();
    fClassType = plFactory::GetNameOfClass(fUoid.GetClassType());
#endif
}

ST::string plKeyImp::GetName() const
{
    return fUoid.GetObjectName();
}

hsKeyedObject* plKeyImp::GetObjectPtr()
{
    return ObjectIsLoaded();
}

hsKeyedObject* plKeyImp::ObjectIsLoaded() const
{
    return plKeyImp::SafeGetObject(this);
}

// Copy the contents of p for cloning process
void plKeyImp::CopyForClone(const plKeyImp *p, uint32_t playerID, uint32_t cloneID)
{
    fObjectPtr = nullptr;           // the clone object start as nil
    fUoid = p->GetUoid();           // we will set the UOID the same to start

#ifdef HS_DEBUGGING
    fIDName = fUoid.GetObjectName();
    fClassType = plFactory::GetNameOfClass( fUoid.GetClassType() );
#endif

    fStartPos = p->GetStartPos();
    fDataLen = p->GetDataLen();
    fUoid.SetClone(playerID, cloneID);
}

hsKeyedObject* plKeyImp::VerifyLoaded()
{
    if (!fObjectPtr)
        hsgResMgr::ResMgr()->ReadObject(this);

    return fObjectPtr;
}

//// Read/Write //////////////////////////////////////////////////////////////
//  The actual key read/writes for the index file, the only time the whole
//  key is ever actually stored.

void plKeyImp::Read(hsStream* s)
{
    fUoid.Read(s);
    s->ReadLE32(&fStartPos);
    s->ReadLE32(&fDataLen);

    plProfile_NewMem(KeyMem, CalcKeySize(this));

#ifdef HS_DEBUGGING
    fIDName = fUoid.GetObjectName();
    fClassType = plFactory::GetNameOfClass(fUoid.GetClassType());
#endif
}

void plKeyImp::SkipRead(hsStream* s)
{
    plUoid tempUoid;
    tempUoid.Read(s);
    (void)s->ReadLE32();
    (void)s->ReadLE32();
}

void plKeyImp::Write(hsStream* s)
{
    fUoid.Write(s);
    s->WriteLE32(fStartPos);
    s->WriteLE32(fDataLen);
}

//// WriteObject /////////////////////////////////////////////////////////////
//  Writes the key's object to the already opened stream

void plKeyImp::WriteObject(hsStream* stream)
{
    hsKeyedObject* ko = ObjectIsLoaded();
    if (ko == nullptr)
    {
        // Mark the key as not written
        fStartPos = (uint32_t)-1;
        fDataLen = (uint32_t)-1;
        return;
    }

    fStartPos = stream->GetPosition();
    hsgResMgr::ResMgr()->WriteCreatable(stream, ko);
    fDataLen = stream->GetPosition() - fStartPos;
}

void plKeyImp::UnRegister()     // called from plRegistry
{
    plKey safeRefUntilWereDone = plKey::Make(this);

    hsKeyedObject* ko = ObjectIsLoaded();
    if (ko)
    {
        INotifyDestroyed();
        fObjectPtr = nullptr;
        fNumActiveRefs = 0;

        hsRefCnt_SafeUnRef(ko);
    }
    IClearRefs();
    ClearNotifyCreated();
};

hsKeyedObject* plKeyImp::RefObject(plRefFlags::Type flags)
{
    if ((flags == plRefFlags::kPassiveRef) && !ObjectIsLoaded())
        return nullptr;

#ifdef LOG_ACTIVE_REFS
    if (IsTrackedKey(this))
        hsStatusMessageF("@@@ RefObject adding active ref to %s (%d total)", kObjName, fNumActiveRefs+1);
#endif // LOG_ACTIVE_REFS

    IncActiveRefs();

    return VerifyLoaded();  // load object on demand
}

void plKeyImp::UnRefObject(plRefFlags::Type flags)
{
    // Rather than using hsRefCnt's, make Ref and
    // UnRef work with ActiveRef system
    if ( (flags == plRefFlags::kPassiveRef) && !ObjectIsLoaded())
        return;

#ifdef LOG_ACTIVE_REFS
    if (IsTrackedKey(this))
        hsStatusMessageF("@@@ UnRefObject releasing active ref to %s (%d total)", kObjName, fNumActiveRefs-1);
#endif // LOG_ACTIVE_REFS
    DecActiveRefs();

    if( !GetActiveRefs() )
    {
        INotifyDestroyed();

        IClearRefs();
        ClearNotifyCreated();

        plKey key=plKey::Make( this );  // for linux build
        plSelfDestructMsg* nuke = new plSelfDestructMsg( key );
        plgDispatch::Dispatch()->MsgSend(nuke);
    }
}

hsKeyedObject* plKeyImp::SetObjectPtr(hsKeyedObject* p) 
{
    hsKeyedObject* retVal = nullptr;

    // If our object is the only one with a ref to us, this function will crash, so we 
    // make sure we have an extra ref, just like in UnRegister().
    plKey safeRefUntilWereDone = plKey::Make(this);

    if (p)
    {
#ifdef HS_DEBUGGING
        if (fClassType)
        {
            char str[2048];
            sprintf(str, "Mismatch of class (we are a %s, given a %s)", fClassType, p->ClassName());
            hsAssert(fClassType == p->ClassName() || strcmp(fClassType, p->ClassName()) == 0, str); // points to static
        }
        else
            fClassType = p->ClassName();
#endif

        hsAssert(!fObjectPtr, "Setting an ObjectPtr thats already Set!");

        retVal = fObjectPtr = p;
    }
    else
    {
        if (fObjectPtr)
            UnRegister();

        fObjectPtr = nullptr;
        retVal = nullptr;
    }

    return retVal;
}

void plKeyImp::ClearNotifyCreated()
{
    for (plRefMsg* msg : fNotifyCreated)
        hsRefCnt_SafeUnRef(msg);
    fNotifyCreated.clear();
    fNotified.Reset();
    fActiveRefs.Reset();
}

void plKeyImp::AddNotifyCreated(plRefMsg* msg, plRefFlags::Type flags)
{
    if (!(flags == plRefFlags::kPassiveRef))
    {
#ifdef LOG_ACTIVE_REFS
        if (IsTrackedKey(this))
        {
            hsStatusMessageF("@@@ %s(%s) adding active ref to %s (%d total)", msg->GetReceiver(0)->GetName().c_str(),
                plFactory::GetNameOfClass(msg->GetReceiver(0)->GetUoid().GetClassType()), kObjName, fNumActiveRefs+1);
        }
#endif // LOG_ACTIVE_REFS

        IncActiveRefs();
        SetActiveRef(GetNumNotifyCreated());
    }

    hsRefCnt_SafeRef(msg);
    fNotifyCreated.emplace_back(msg);
}

void plKeyImp::RemoveNotifyCreated(size_t i)
{
    hsRefCnt_SafeUnRef(fNotifyCreated[i]);
    fNotifyCreated.erase(fNotifyCreated.begin() + i);

    fNotified.RemoveBit(i);
    fActiveRefs.RemoveBit(i);
}

void plKeyImp::AddRef(plKeyImp* key) const
{
    fPendingRefs++;
    fRefs.emplace_back(key);
}


void plKeyImp::RemoveRef(plKeyImp* key) const
{
    auto idx = std::find(fRefs.cbegin(), fRefs.cend(), key);
    if (idx != fRefs.cend())
        fRefs.erase(idx);
}

void plKeyImp::AddClone(plKeyImp* key)
{
    hsAssert(!GetClone(key->GetUoid().GetClonePlayerID(), key->GetUoid().GetCloneID()),
                "Adding a clone which is already there?");

    key->fCloneOwner = plKey::Make(this);
    fClones.emplace_back(key);
}

void plKeyImp::RemoveClone(plKeyImp* key) const
{
    if (key->GetUoid().IsClone())
    {
        auto idx = std::find(fClones.cbegin(), fClones.cend(), key);
        if (idx != fClones.cend())
        {
            fClones.erase(idx);
            key->fCloneOwner = nullptr;
        }
    }
}

plKey plKeyImp::GetClone(uint32_t playerID, uint32_t cloneID) const
{
    for (plKeyImp* cloneKey : fClones)
    {
        if (cloneKey
            && cloneKey->GetUoid().GetCloneID() == cloneID
            && cloneKey->GetUoid().GetClonePlayerID() == playerID)
            return plKey::Make(cloneKey);
    }

    return plKey();
}

size_t plKeyImp::GetNumClones()
{
    return fClones.size();
}

plKey plKeyImp::GetCloneByIdx(size_t idx)
{
    if (idx < fClones.size())
        return plKey::Make(fClones[idx]);

    return nullptr;
}

void plKeyImp::SatisfyPending(plRefMsg* msg) const
{
    for (int i = 0; i < msg->GetNumReceivers(); i++)
        plKeyImp::GetFromKey(msg->GetReceiver(i))->SatisfyPending();
}

void plKeyImp::SatisfyPending() const
{
    hsAssert(fPendingRefs > 0, "Have more requests satisfied than we made");
    if (!--fPendingRefs)
    {
#ifdef PL_SEND_SATISFIED
        plSatisfiedMsg* msg = new plSatisfiedMsg(this);
        plgDispatch::MsgSend(msg);
#endif // PL_SEND_SATISFIED
    }
}

void plKeyImp::ISetupNotify(plRefMsg* msg, plRefFlags::Type flags)
{
    msg->SetSender(nullptr);

    AddNotifyCreated(msg, flags);

    hsAssert(msg->GetNumReceivers(), "nil object getting a reference");
    for (int i = 0; i < msg->GetNumReceivers(); i++)
        plKeyImp::GetFromKey(msg->GetReceiver(i))->AddRef(plKeyImp::GetFromKey(plKey::Make(this)));
}

void plKeyImp::SetupNotify(plRefMsg* msg, plRefFlags::Type flags)
{
    hsKeyedObject* ko = ObjectIsLoaded();

    ISetupNotify(msg, flags);

    // the KeyedObject is already Loaded, so we better go ahead and send the notify message just added.
    if (ko)
    {
        hsRefCnt_SafeRef(ko);
        msg->SetRef(ko);
        msg->SetTimeStamp(hsTimer::GetSysSeconds());

        // Add ref, since Dispatch will unref this msg but we want to keep using it.
        hsRefCnt_SafeRef(msg);

        SetNotified(GetNumNotifyCreated()-1);
        SatisfyPending(msg);

#ifdef LOAD_IN_THREAD       // test for resLoader
        plgDispatch::Dispatch()->MsgQueue(msg);
#else
        plgDispatch::Dispatch()->MsgSend(msg);
#endif

        hsRefCnt_SafeUnRef(ko);
    }
    hsRefCnt_SafeUnRef(msg);
}

// We could just call NotifyCreated() on all our fRefs, and then fix
// up fNotified to only get set when the message actually was delivered (i.e.
// refMsg->GetReceiver(0)->GetObjectPtr() != nullptr. But that only really works
// if we guarantee the refMsg->GetNumReceivers() == 1.
// This looks like it'll take forever to run, but this is only called right
// when our object has just been loaded, at which time normally fRefs.GetCount() == 0.
void plKeyImp::INotifySelf(hsKeyedObject* ko)
{
    for (plKeyImp* ref : fRefs)
    {
        hsKeyedObject* rcv = ref->GetObjectPtr();
        if (rcv)
        {
            for (size_t j = 0; j < ref->fNotifyCreated.size(); j++)
            {
                plRefMsg* refMsg = ref->fNotifyCreated[j];
                if (refMsg && refMsg->GetRef() && !ref->IsNotified(j))
                {
                    hsAssert(refMsg->GetRef() == rcv, "Ref message out of sync with its ref");

                    // GetNumReceivers() should always be 1 for a refMsg.
                    for (int k = 0; k < refMsg->GetNumReceivers(); k++)
                    {
                        if (refMsg->GetReceiver(k) == this)
                        {
                            ref->SetNotified(j);
                            ref->SatisfyPending(refMsg);

                            hsRefCnt_SafeRef(refMsg);
                            plgDispatch::MsgSend(refMsg);
                            break;
                        }
                    }
                }
            }
        }
    }
}

void plKeyImp::NotifyCreated()
{
    hsKeyedObject* ko = GetObjectPtr();
    hsRefCnt_SafeRef(ko);
    hsAssert(ko, "Notifying of created before on nil object");

    INotifySelf(ko);

    for (size_t i = 0; i < GetNumNotifyCreated(); i++)
    {
        if (!IsNotified(i) && GetNotifyCreated(i)->GetReceiver(0)->GetObjectPtr())
        {
            plRefMsg* msg = GetNotifyCreated(i);
            msg->SetRef(ko);
            msg->SetTimeStamp(hsTimer::GetSysSeconds());
            msg->SetContext(plRefMsg::kOnCreate);
            hsRefCnt_SafeRef(msg);

            SetNotified(i);
            SatisfyPending(msg);

            plgDispatch::MsgSend(msg);
        }
    }
    hsRefCnt_SafeUnRef(ko);
}

void plKeyImp::INotifyDestroyed()
{
    hsKeyedObject* ko = GetObjectPtr();
    hsAssert(ko, "Notifying of destroy on already destroyed");
    for (size_t i = 0; i < GetNumNotifyCreated(); i++)
    {
        hsAssert(ko, "Notifying of destroy on already destroyed");
        plRefMsg* msg = GetNotifyCreated(i);
        msg->SetRef(ko);
        msg->SetTimeStamp(hsTimer::GetSysSeconds());
        msg->SetContext(plRefMsg::kOnDestroy);
        hsRefCnt_SafeRef(msg);
        msg->Send();
    }
    fNotified.Clear();
}

void plKeyImp::IClearRefs()
{
    while (GetNumRefs())
        IRelease(GetRef(0));
    fRefs.clear();

    for (size_t i = 0; i < GetNumNotifyCreated(); i++)
    {
        plRefMsg* msg = GetNotifyCreated(i);
        for (size_t j = 0; j < msg->GetNumReceivers(); j++)
            plKeyImp::GetFromKey(msg->GetReceiver(j))->RemoveRef(this);
    }
}

void plKeyImp::Release(plKey targetKey)
{
    IRelease(plKeyImp::GetFromKey(targetKey));
}

void plKeyImp::IRelease(plKeyImp* iTargetKey)
{
    // Remove the target key from my ref list
    RemoveRef(iTargetKey);

    // Inspect the target key to find whether it is supposed to send a message
    // to me on destruction, and to find out if I have an active of passive 
    // ref on this key.  Not sure why I don't track my own active/passive ref states
    bool isActive = false;
    hsSsize_t iTarg = -1;
    for (size_t i = 0; (iTarg < 0) && (i < iTargetKey->GetNumNotifyCreated()); i++)
    {
        plMessage* rcvMsg = iTargetKey->GetNotifyCreated(i);
        for (int j = 0; j < rcvMsg->GetNumReceivers(); j++)
        {
            if (rcvMsg->GetReceiver(j) == this)
            {
                isActive = iTargetKey->IsActiveRef(iTarg = (hsSsize_t)i);
                break;
            }
        }
    }

    if (iTarg < 0)
    {
        // If it doesn't send me a message on destruction,  I am assuming I don't have an
        // active ref on the key (seems to be always true, but should we depend on it?)
        // Since it doesn't send me a message, and won't be destroyed by the release, no 
        // need to do anything more here
        return;
    }

    // If releasing this target causes its destruction, we'll notify
    // everyone that target is dead. Otherwise, we'll remove
    // releaser from target's notify list and notify releaser
    // it's been removed. Object doesn't actually get deleted until
    // it receives the SelfDestruct msg, by which time everyone referencing
    // it has been notified it is going away.
#ifdef LOG_ACTIVE_REFS
    if (isActive && IsTrackedKey(iTargetKey))
        hsStatusMessageF("@@@ %s(%s) releasing active ref on %s (%d total)", GetName().c_str(),
                         plFactory::GetNameOfClass(GetUoid().GetClassType()), kObjName, iTargetKey->fNumActiveRefs-1);
#endif // LOG_ACTIVE_REFS

    if (isActive && iTargetKey->GetActiveRefs() && !iTargetKey->DecActiveRefs())
    {
        iTargetKey->INotifyDestroyed();

        iTargetKey->IClearRefs();
        iTargetKey->ClearNotifyCreated();

        plKey key = plKey::Make(iTargetKey);
        plSelfDestructMsg* nuke = new plSelfDestructMsg(key);
        plgDispatch::Dispatch()->MsgSend(nuke);
    }
    else
    {
        plRefMsg* refMsg = iTargetKey->GetNotifyCreated(iTarg);
        hsRefCnt_SafeRef(refMsg);
        iTargetKey->RemoveNotifyCreated(iTarg);
        if (refMsg)
        {
            refMsg->SetRef(iTargetKey->ObjectIsLoaded());
            refMsg->SetTimeStamp(hsTimer::GetSysSeconds());
            refMsg->SetContext(plRefMsg::kOnRemove);
            hsRefCnt_SafeRef(refMsg);
            plgDispatch::MsgSend(refMsg);
        }
        hsRefCnt_SafeUnRef(refMsg);
    }
}
