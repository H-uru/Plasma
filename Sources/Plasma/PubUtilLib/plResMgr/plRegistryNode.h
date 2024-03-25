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
#ifndef plRegistryNode_h_inc
#define plRegistryNode_h_inc

#include "HeadSpin.h"
#include "plFileSystem.h"
#include "plPageInfo.h"

#include <map>
#include <memory>

class hsStream;
class plRegistryKeyList;
class plKeyImp;
class plRegistryKeyIterator;

enum PageCond
{
    kPageOk,
    kPageOutOfDate,
    kPageTooNew,
    kPageCorrupt,
};

//
// Represents one entire (age,page) location and contains all keys in that
// location. Note: just because the node exists does not mean that the keys are loaded.
//
class plRegistryPageNode 
{
protected:
    friend class plKeyFinder;

    // Map from class type to a list of keys of that type
    typedef std::map<uint16_t, plRegistryKeyList*> KeyMap;
    KeyMap fKeyLists;
    uint32_t fLoadedTypes;      // The number of key types that have dynamic keys loaded

    PageCond    fValid;         // Condition of the page
    plFileName  fPath;          // Path to the page file
    plPageInfo  fPageInfo;      // Info about this page

    std::unique_ptr<hsStream> fStream; // Stream for reading/writing our page
    uint8_t fOpenRequests;        // How many handles there are to fStream (or
                                // zero if it's closed)
    bool fIsNewPage;          // True if this page is new (not read off disk)

    plRegistryPageNode();

    plRegistryKeyList* IGetKeyList(uint16_t classType) const;
    PageCond IVerify();

public:
    // For reading a page off disk
    plRegistryPageNode(const plFileName& path);

    // For creating a new page.
    plRegistryPageNode(const plLocation& location, const ST::string& age,
                       const ST::string& page, const plFileName& dataPath);
    ~plRegistryPageNode();

    bool IsValid() const { return fValid == kPageOk; }
    PageCond GetPageCondition() { return fValid; }

    // True if we have any static or dynamic keys loaded
    bool IsLoaded() const     { return fLoadedTypes > 0; }
    // True if all of our static keys are loaded
    bool IsFullyLoaded() const    { return (fLoadedTypes == fKeyLists.size() && !fKeyLists.empty()) || fIsNewPage; }

    // Export time only.  If we want to reuse a page, load the keys we want then
    // call SetNewPage, so it will be considered a new page from now on.  That
    // way we won't try to load it's keys again.
    bool IsNewPage() const    { return fIsNewPage; }
    void SetNewPage()       { fIsNewPage = true; }

    const plPageInfo& GetPageInfo() const { return fPageInfo; }

    void LoadKeys();    // Loads the keys off disk
    void UnloadKeys();  // Frees all our keys

    // Find a key by type and name
    plKeyImp* FindKey(uint16_t classType, const ST::string& name) const;
    // Find a key by direct uoid lookup (or fallback to name lookup if that doesn't work)
    plKeyImp* FindKey(const plUoid& uoid) const;
    
    void AddKey(plKeyImp* key);

    // Sets a key as used or unused, ie there aren't any refs to it anymore.
    // When all the static keys are unused we can free the memory associated with
    // them.  When a dynamic key is unused we just delete it right away.
    void SetKeyUsed(plKeyImp* key);
    bool SetKeyUnused(plKeyImp* key);

    bool IterateKeys(plRegistryKeyIterator* iterator) const;
    bool IterateKeys(plRegistryKeyIterator* iterator, uint16_t classToRestrictTo) const;

    // Call this to get a read stream for the page.  If a valid pointer is
    // returned, make sure to call CloseStream when you're done using it.
    hsStream*   OpenStream();
    void        CloseStream();

    // Export time only.  Before we write to disk, assign all the loaded keys
    // sequential object IDs that they can use to do fast lookups at load time.
    void PrepForWrite();

    // Takes care of everything involved in writing this page to disk
    void Write();
    void DeleteSource();

    const plFileName& GetPagePath() const { return fPath; }
};

#endif // plRegistryNode_h_inc
