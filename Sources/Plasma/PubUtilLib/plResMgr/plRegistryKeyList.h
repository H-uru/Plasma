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
#ifndef plRegistryKeyList_h_inc
#define plRegistryKeyList_h_inc

#include "HeadSpin.h"
#include "pnKeyedObject/plKeyImp.h"
#include <vector>
#include <set>

class plRegistryKeyIterator;

class KeySorter
{
public:
    bool operator() (plKeyImp* k1, plKeyImp* k2) const
    {
        hsAssert(k1 && k2, "Should have valid keys here");
        return stricmp(k1->GetName(), k2->GetName()) < 0;
    }
};

//
//  List of keys for a single class type.
//
class plRegistryKeyList
{
protected:
    friend class plKeyFinder;

    uint16_t fClassType;
    // Lock counter for iterating. If this is >0, don't do any ops that
    // can change key positions in the array (instead, just leave holes)
    uint16_t fLocked;

    enum Flags { kStaticUnsorted = 0x1 };
    uint8_t fFlags;

    // Static keys are one's we read off disk.  These don't change and are
    // assumed to be already sorted when they're read in.
    typedef std::vector<plKeyImp*> StaticVec;
    StaticVec fStaticKeys;
    uint32_t fReffedStaticKeys;   // Number of static keys that are loaded

    // Dynamic keys are anything created at runtime.  They are put in the
    // correct sorted position when they are added
    typedef std::set<plKeyImp*, KeySorter> DynSet;
    DynSet fDynamicKeys;

    plRegistryKeyList() {}

    void ILock();
    void IUnlock();

    void IRepack();

public:
    plRegistryKeyList(uint16_t classType);
    ~plRegistryKeyList();

    uint16_t GetClassType() const { return fClassType; }

    // Find a key by name (case-insensitive)
    plKeyImp* FindKey(const char* keyName);
    // Find a key by uoid index.
    plKeyImp* FindKey(const plUoid& uoid);

    bool IterateKeys(plRegistryKeyIterator* iterator);

    // Changes in our load status that can be caused by loading or unloading a key
    enum LoadStatus
    {
        kNoChange,
        kDynLoaded,
        kDynUnloaded,
        kStaticUnloaded,
    };
    void AddKey(plKeyImp* key, LoadStatus& loadStatusChange);
    void SetKeyUsed(plKeyImp* key);
    bool SetKeyUnused(plKeyImp* key, LoadStatus& loadStatusChange);

    // Export time only.  Before we write to disk, assign all the static keys
    // object ID's that they can use to do fast lookups at load time.
    void PrepForWrite();

    void Read(hsStream* s);
    void Write(hsStream* s);
};

#endif // plRegistryKeyList_h_inc
