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

#include <vector>

class plKeyImp;
class plRegistryKeyIterator;
class hsStream;
class plUoid;

//
//  List of keys for a single class type.
//
class plRegistryKeyList
{
protected:
    friend class plKeyFinder;

    uint16_t fClassType;
    uint32_t fLocked;
    uint32_t fReffedKeys;

    std::vector<plKeyImp*> fKeys;

    plRegistryKeyList() {}

    void IRepack();
    void ILock() { ++fLocked; }
    void IUnlock() { --fLocked; }

public:
    enum LoadStatus
    {
        kNoChange,
        kTypeLoaded,
        kTypeUnloaded
    };

    plRegistryKeyList(uint16_t classType)
        : fClassType(classType), fReffedKeys(0), fLocked(0)
    { }
    ~plRegistryKeyList();

    uint16_t GetClassType() const { return fClassType; }

    plKeyImp* FindKey(const ST::string& keyName) const;
    plKeyImp* FindKey(const plUoid& uoid) const;

    bool IterateKeys(plRegistryKeyIterator* iterator);

    void AddKey(plKeyImp* key, LoadStatus& loadStatusChange);
    void SetKeyUsed(plKeyImp* key) { ++fReffedKeys; }
    bool SetKeyUnused(plKeyImp* key, LoadStatus& loadStatusChange);

    // Export time only.  Before we write to disk, assign all the loaded keys
    // sequential object IDs that they can use to do fast lookups at load time.
    void PrepForWrite();

    void Read(hsStream* s);
    void Write(hsStream* s);
};

#endif // plRegistryKeyList_h_inc
