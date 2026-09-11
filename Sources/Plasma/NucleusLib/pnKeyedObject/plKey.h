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
#ifndef plKey_h_inc
#define plKey_h_inc

#include "HeadSpin.h"
#include "plRefFlags.h"

class hsKeyedObject;
class plRefMsg;
class plUoid;
class hsBitVector;

//// plKey ///////////////////////////////////////////////////////////////////
//  Pointer to a plKeyData struct, which is a handle to a keyedObject

class plKeyData;

class plKey 
{
public:
    // Constructors and destructors and copy stuff
    plKey() : fKeyData(nullptr) { }
    plKey(std::nullptr_t) : fKeyData(nullptr) { }
    plKey(const plKey& rhs);
    plKey(plKey&& rhs) noexcept : fKeyData(rhs.fKeyData) { rhs.fKeyData = nullptr; }
    ~plKey();
    plKey& operator=(const plKey& rhs);
    plKey& operator=(std::nullptr_t);

    plKey& operator=(plKey&& rhs)
    {
        std::swap(fKeyData, rhs.fKeyData);
        return *this;
    }

    bool operator==(const plKey& rhs) const { return fKeyData == rhs.fKeyData; }
    bool operator==(const plKeyData* rhs) const { return fKeyData == rhs; }
    bool operator!=(const plKey& rhs) const { return !(*this == rhs); }
    bool operator!=(const plKeyData* rhs) const { return !(*this == rhs); }
    // Ordering operators for stdlib containers, etc. that rely on < (std::less) by default
    bool operator<(const plKey& rhs) const { return fKeyData < rhs.fKeyData; }
    bool operator>(const plKey& rhs) const { return fKeyData > rhs.fKeyData; }
    bool operator<=(const plKey& rhs) const { return fKeyData <= rhs.fKeyData; }
    bool operator>=(const plKey& rhs) const { return fKeyData >= rhs.fKeyData; }

    plKeyData* operator->() const;
    plKeyData& operator*() const;

    operator bool() const { return fKeyData != nullptr; }

    static plKey Make(plKeyData* data) { return plKey(data); }

protected:
    // Pointer to our real key
    plKeyData* fKeyData;
    void IIncRef();
    void IDecRef();

    // Internal constructor
    plKey(plKeyData* data);
};

//// plKeyData ///////////////////////////////////////////////////////////////
//  Base virtual class that provides the essential functionality you would
//  want from a plKey-ish thing.

class plKeyData
{
public:
    virtual const plUoid&   GetUoid() const=0;
    virtual ST::string      GetName() const=0;

    virtual hsKeyedObject*  GetObjectPtr()=0;
    virtual hsKeyedObject*  ObjectIsLoaded() const=0;
    virtual hsKeyedObject*  VerifyLoaded() = 0;

    //----------------------
    // Allow a keyed object to behave as if it has an active ref when in fact the object
    // should only be active ref'ed by a non-keyed parent.  Essentially just bumps/decs
    // the active ref count to facilitate normal object creation/destruction
    //----------------------
    virtual hsKeyedObject*  RefObject(plRefFlags::Type flags = plRefFlags::kActiveRef)=0;
    virtual void            UnRefObject(plRefFlags::Type flags = plRefFlags::kActiveRef)=0;

    //----------------------
    // Release has two behaviors, depending on whether the ref is active or passive:
    // Active - Release decs the ActiveRefCnt. When it gets to zero, the object will be deleted.
    // Passive - Unregisters my interest in when the object is created or destroyed.
    //----------------------
    virtual void Release(plKey targetKey)=0;

    virtual uint16_t      GetActiveRefs() const = 0;

    virtual size_t      GetNumNotifyCreated() const = 0;
    virtual plRefMsg*   GetNotifyCreated(size_t i) const = 0;
    virtual const hsBitVector& GetActiveBits() const = 0;

protected:
    // Protected so only the registry can create it
    plKeyData();
    virtual ~plKeyData();

    //// RefCount Stuff //////////////////////////////////////////////////////////
    //  The refcounts on plKeyData/plKeyImps are zero-based. When you first create
    //  a new plKeyImp (which should ONLY ever be done inside the resMgr), it gets
    //  a refcount of zero. Assigning a new plKey to represent it bumps it to 1,
    //  and when that key goes away, the refcount drops to zero and the ResManager
    //  is notified of the fact and may delete the keyImp.
    //  So the only refcounts on keys outside of the resMgr should be one or more;
    //  only inside the resMgr should there EVER exist keys with a refcount of 0.
    //
    //  Using our own refCount system instead of hsRefCnt allows us to make it all
    //  protected, so that only plKey can actually ref/unref, which is as it should
    //  be.

    // Only one class should ever touch this...
    friend class plKey;

    // Refcount--the number of plKeys that have pointers to us.
    uint16_t fRefCount;
};

#endif // plKey_h_inc
