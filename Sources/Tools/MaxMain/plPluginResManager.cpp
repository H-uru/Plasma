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
#include "HeadSpin.h"
#include "plgDispatch.h"
#include "pnKeyedObject/plKey.h"

// For our common object libs
#include "plCommonObjLib.h"
#include "MaxComponent/plMiscComponents.h"

#include "plPluginResManager.h"
#include "plResMgr/plRegistryNode.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plVersion.h"
#include "plResMgr/plResMgrSettings.h"
#include "plScene/plSceneNode.h"
#include "pnKeyedObject/plKeyImp.h"
#include "plAgeDescription/plAgeDescription.h"

plKey plPluginResManager::NameToLoc(const ST::string& age, const ST::string& page, int32_t sequenceNumber, bool itinerant)
{
    // Get or create our page
    plRegistryPageNode* pageNode = INameToPage(age, page, sequenceNumber, itinerant);
    hsAssert(pageNode != nullptr, "No page returned from INameToPage(), shouldn't be possible");

    // Go find the sceneNode now, since we know the page exists (go through our normal channels, though)
    ST::string keyName = ST::format("{}_{}", age, page);

    plUoid nodeUoid(pageNode->GetPageInfo().GetLocation(), plSceneNode::Index(), keyName);

    plKey snKey = FindKey(nodeUoid);
    if (snKey == nullptr)
    {
        // Not found, create a new one
        plSceneNode *newSceneNode = new plSceneNode;
        snKey = NewKey(keyName, newSceneNode, pageNode->GetPageInfo().GetLocation());

        // Call init after it gets a key
        newSceneNode->Init();

        // Add to our list of exported nodes
        fExportedNodes.emplace_back(newSceneNode);
        newSceneNode->GetKey()->RefObject();
    }
    else
    {
        hsAssert(snKey->ObjectIsLoaded() != nullptr, "Somehow we still have the key for a sceneNode that hasn't been loaded.");

        // Force load, or attempt to at least
        plSceneNode* node = plSceneNode::ConvertNoRef(snKey->VerifyLoaded());

        // Add to our list if necessary
        if (std::find(fExportedNodes.cbegin(), fExportedNodes.cend(), node) == fExportedNodes.cend())
        {
            fExportedNodes.emplace_back(node);
            node->GetKey()->RefObject();
        }
    }

    // Return the key
    return snKey;
}

//// INameToPage /////////////////////////////////////////////////////////////
//  Given the age/chapter/page combo we all know and love, plus an optional
//  seqNumber, returns the page for that combo (either by preloading it or
//  by creating it).

plRegistryPageNode* plPluginResManager::INameToPage(const ST::string& age, const ST::string& page, int32_t sequenceNumber, bool itinerant)
{
    // Find the location first, to see if it already exists
    plRegistryPageNode* pageNode = FindPage(age, page);
    if (pageNode == nullptr)
    {
        // This page does not yet exist, so create a new page
        if (sequenceNumber != uint32_t(-1))
        {
            const plLocation& newLoc = ICreateLocation(age, page, sequenceNumber, itinerant);
            pageNode = CreatePage(newLoc, age.c_str(), page.c_str());
        }
        else
        {
            const plLocation& newLoc = ICreateLocation(age, page, itinerant);
            pageNode = CreatePage(newLoc, age.c_str(), page.c_str());
        }
        // Still preload textures on this guy. This should be a no-op for this page since it's new, but won't be
        // for the shared textures page
        IPreLoadTextures(pageNode, sequenceNumber);
    }
    else if (!pageNode->IsNewPage())
    {
        // Node's already in our registry (i.e. already stored somewhere), so make sure it loads so
        // we can update from that
        LoadPageKeys(pageNode);

        // Now clear out all the unwanted keys.
        IPreLoadTextures(pageNode, sequenceNumber);
    }

    return pageNode;
}

//// plCommonKeyDistributor //////////////////////////////////////////////////
//  Iterator that takes the keys in a common page and distributes them to
//  whichever commonObjectLibs are interested in them.

class plCommonKeyDistributor : public plRegistryKeyIterator
{
    plPluginResManager* fMgr;

public:
    plCommonKeyDistributor(plPluginResManager* mgr) : fMgr(mgr) {}

    bool EatKey(const plKey& key) override
    {
        size_t count = plCommonObjLib::GetNumLibs();

        for (size_t i = 0; i < count; i++)
        {
            plCommonObjLib* lib = plCommonObjLib::GetLib(i);
            if (lib->IsInteresting(key))
            {
                // Lib wants this guy, so load the object and add it
                // Note: we want to switch to passive mode here, so any keys read in
                // will NOT force a load on whichever page those keys belong to. Otherwise,
                // we'd have to load that entire page and ref all the objects all over
                // again and I just really don't want to do that...
                plResMgrSettings::Get().SetPassiveKeyRead(true);
                hsKeyedObject* object = key->VerifyLoaded();
                plResMgrSettings::Get().SetPassiveKeyRead(false);

                lib->AddObject(object);
                break;
            }
        }

        return true;
    }
};

//// IPreLoadTextures ////////////////////////////////////////////////////////
//  Given a page of loaded keys, culls through them and make sure all our
//  registered commonObjectLibs get them as if we had just converted them.
//  Note: Broken out in a separate function 5.31.2002 mcn to facilitate
//  pre-loading textures exported to our special textures page.

void plPluginResManager::IPreLoadTextures(plRegistryPageNode* pageNode, int32_t origSeqNumber)
{
    // For common pages, we want to kinda-maybe-load all the objects so they don't get wiped when we
    // re-export them. However, we don't have a good way of telling whether a page is a common page,
    // which is where this hack comes in
    bool common = false;
    for (int i = 0; i < plAgeDescription::kNumCommonPages; i++)
    {
        if (pageNode->GetPageInfo().GetPage().compare_i(plAgeDescription::GetCommonPage(i)) == 0)
        {
            common = true;
            break;
        }
    }

    if (common)
    {
        // Iterate through all the keys in our page, scattering them to various objectLibs if they're
        // interested. If nobody likes them, they get unreffed and disappear.
        plCommonKeyDistributor distributor(this);
        pageNode->IterateKeys(&distributor);
    }

    // Clear out all the unwanted keys in the page we just loaded (by implication; they won't clear if we already
    // stored the keys via our objectLibs above)
    {
        class plEmptyIterator : public plRegistryKeyIterator
        {
        public:
            bool EatKey(const plKey& key) override { return true; }
        } empty;

        pageNode->IterateKeys(&empty);
    }

    // We've loaded anything we needed from this page now, so set it as new so
    // that we won't try loading again
    pageNode->SetNewPage();

    // Get our texture page now, if we're not a texture page
    if (!common)
    {
        // Make sure it's not a global page we're handling either
        if (!pageNode->GetPageInfo().GetLocation().IsReserved())
        {
            int32_t texSeqNum = -1;
            if (origSeqNumber != -1)
                texSeqNum = plPageInfoUtils::GetCommonSeqNumFromNormal(origSeqNumber, plAgeDescription::kTextures);

            // Note: INameToPage will turn around and call us again, so no need to do the call twice
            plRegistryPageNode* texturePage = INameToPage(pageNode->GetPageInfo().GetAge(), 
                                                            plAgeDescription::GetCommonPage(plAgeDescription::kTextures), texSeqNum);
            hsAssert(texturePage != nullptr, "Unable to get or create the shared textures page? Shouldn't be possible.");

            // Do the other one
            int32_t commonSeqNum = -1;
            if (origSeqNumber != -1)
                commonSeqNum = plPageInfoUtils::GetCommonSeqNumFromNormal(origSeqNumber, plAgeDescription::kGlobal);

            // Note: INameToPage will turn around and call us again, so no need to do the call twice
            plRegistryPageNode* commonPage = INameToPage(pageNode->GetPageInfo().GetAge(), 
                                                            plAgeDescription::GetCommonPage(plAgeDescription::kGlobal),
                                                            commonSeqNum);
            hsAssert(commonPage != nullptr, "Unable to get or create the shared built-in page? Shouldn't be possible.");
        }
    }
}

//// GetCommonPage ///////////////////////////////////////////////////////////
//  Given a plLocation, finds the texture page that's in the same age and
//  returns its location.

const plLocation& plPluginResManager::GetCommonPage(const plLocation &sisterPage, int whichPage)
{
    if (sisterPage.IsReserved())
        return sisterPage;          // Reserved pages have no common pages

    plRegistryPageNode* page = FindPage(sisterPage);
    if (page == nullptr)
    {
        hsAssert(false, "Trying to find the sister common page to a page that doesn't exist!");
        return sisterPage;
    }

    // Find the common page in the same age as this one
    plRegistryPageNode* commonPage = FindPage(page->GetPageInfo().GetAge(), 
                                                plAgeDescription::GetCommonPage(whichPage));
    if (commonPage == nullptr)
    {
        hsAssert(false, "Unable to find sister common page to this page");
        return sisterPage;
    }

    return commonPage->GetPageInfo().GetLocation();
}

//// IShutdown ///////////////////////////////////////////////////////////////

void plPluginResManager::IShutdown()
{
    if (!fInited)
        return;

    // Loop through all the commonObjLibs and clear their object lists, just
    // as a safety measure (the creators of the various libs should really
    // be doing it)
    for (size_t i = 0; i < plCommonObjLib::GetNumLibs(); i++)
        plCommonObjLib::GetLib(i)->ClearObjectList();

    plResManager::IShutdown();
}

// Little finder class to, erm, find unused location sequence numbers
class plSeqNumberFinder : public plRegistryPageIterator
{
protected:
    int32_t&  fSeqNum;
    bool    fWillBeReserved;

public:
    plSeqNumberFinder(int32_t& seqNum, bool willBeReserved) : fSeqNum(seqNum), fWillBeReserved(willBeReserved) {}

    bool EatPage(plRegistryPageNode* page) override
    {
        if (fSeqNum <= page->GetPageInfo().GetLocation().GetSequenceNumber() &&
            fWillBeReserved == page->GetPageInfo().GetLocation().IsReserved())
            fSeqNum = page->GetPageInfo().GetLocation().GetSequenceNumber() + 1;
        return true;
    }
};


plLocation plPluginResManager::ICreateLocation(const ST::string& age, const ST::string& page, bool itinerant)
{
    int32_t seqNum = VerifySeqNumber(0, age, page);
    return ICreateLocation(age, page, seqNum, itinerant);
}

plLocation plPluginResManager::ICreateLocation(const ST::string& age, const ST::string& page, int32_t seqNum, bool itinerant)
{
    bool willBeReserved = age.compare_i("global") == 0;

    int32_t oldNum = seqNum;
    seqNum = VerifySeqNumber(seqNum, age, page);
    if (seqNum != oldNum)
    {
        hsAssert(false, "Conflicting page sequence number. Somebody called NameToLoc without verifying their seq# first!"); 
    }

    if (seqNum < 0)
    {
        willBeReserved = true;
        seqNum = -seqNum;
    }

    plLocation newLoc;
    if (willBeReserved)
        newLoc = plLocation::MakeReserved(seqNum);
    else
        newLoc = plLocation::MakeNormal(seqNum);

    // Flag common pages
    for (int i = 0; i < plAgeDescription::kNumCommonPages; i++)
    {
        if (page.compare(plAgeDescription::GetCommonPage(i)) == 0)
        {
            newLoc.SetFlags(plLocation::kBuiltIn);
            break;
        }
    }

    // If we have an age description file for the age we're creating a location
    // for, grab some extra flags from it
    plAgeDescription* ageDesc = plPageInfoUtils::GetAgeDesc(age);
    const plAgePage* agePage = ageDesc ? ageDesc->FindPage(page) : nullptr;
    if (agePage)
    {
        if (agePage->GetFlags() & plAgePage::kIsLocalOnly)
            newLoc.SetFlags(plLocation::kLocalOnly);

        if (agePage->GetFlags() & plAgePage::kIsVolatile)
            newLoc.SetFlags(plLocation::kVolatile);
    }
    if (itinerant)
        newLoc.SetFlags(plLocation::kItinerant);

    delete ageDesc;
    return newLoc;
}

class plObjectIDSortingPageIterator : public plRegistryPageIterator
{
public:
    plObjectIDSortingPageIterator() {}
    bool EatPage(plRegistryPageNode *page) override
    {
        if (page->GetPageInfo().GetLocation() != plLocation::kGlobalFixedLoc)
            page->PrepForWrite();
        return true;
    }
};

class plWritePageIterator : public plRegistryPageIterator
{
public:
    plWritePageIterator() {}
    bool EatPage(plRegistryPageNode *page) override
    {
        if (page->GetPageInfo().GetLocation() != plLocation::kGlobalFixedLoc)
            page->Write();
        return true;
    }
};

void plPluginResManager::WriteAllPages()
{
    plObjectIDSortingPageIterator idSort;
    IteratePages(&idSort);

    plWritePageIterator iter;
    IteratePages(&iter);
}

//// EndExport ///////////////////////////////////////////////////////////////
//  Called after export is done and pages are all written out. Cleans up
//  by paging out all the sceneNodes we just created.
void plPluginResManager::EndExport()
{
    for (plSceneNode* node : fExportedNodes)
    {
        if (node != nullptr)
            node->GetKey()->UnRefObject();
    }
    fExportedNodes.clear();

    for (const plKey& key : fLooseEnds)
    {
        if (key)
            key->UnRefObject();
    }
    fLooseEnds.clear();
    // Flush the message queue, so all the messages for paging out stuff actually get delivered
    plgDispatch::Dispatch()->MsgQueueProcess();
}

void plPluginResManager::AddLooseEnd(plKey key)
{
    if( key )
    {
        key->RefObject();
        fLooseEnds.emplace_back(std::move(key));
    }
}
// Verifies that the given sequence number belongs to the given string combo and ONLY that combo. Returns a new, unique sequenceNumber if not
int32_t plPluginResManager::VerifySeqNumber(int32_t sequenceNumber, const ST::string& age, const ST::string& page)
{
    bool negated = false, willBeReserved = age.compare_i("global") == 0;
    if (sequenceNumber < 0)
    {
        sequenceNumber = -sequenceNumber;
        willBeReserved = negated = true;
    }

    fLastVerifyError = kNoVerifyError;
    fLastVerifyPage = nullptr;

    plLocation toCompareTo;
    if (willBeReserved)
        plLocation::MakeReserved(sequenceNumber);
    else
        plLocation::MakeNormal(sequenceNumber);

    // Does the page already exist?
    plRegistryPageNode* pageNode = FindPage(age, page);
    if (pageNode != nullptr)
    {
        if (pageNode->GetPageInfo().GetLocation() == toCompareTo)
            // Right page, right sequence #. Assume we're smart enough to already have it right
            return negated ? -sequenceNumber : sequenceNumber;

        // Right page, wrong seq #...tag our last error field so we can know this later on
        fLastVerifyError = kErrRightPageWrongSeq;
    }

    // Page doesn't yet exist, check to make sure the seq # isn't used yet
    if (sequenceNumber > 0)
    {
        pageNode = FindPage(toCompareTo);
        if (pageNode == nullptr)
            // Safe to use
            return negated ? -sequenceNumber : sequenceNumber;
        else
        {
            // If there is no error yet, set the error to "already taken"
            if (fLastVerifyError == kNoVerifyError)
                fLastVerifyError = kErrSeqAlreadyTaken;

            fLastVerifyPage = &pageNode->GetPageInfo();
        }
    }

    // Gotta find a good sequence number to use, so keep searching until we find one
    // (but start at a good high number so we won't hopefully ever run into anybody else)
    const int kTemporarySequenceStartPrefix = 100; // can't be larger then 0xFE, so well start out at 100 for kicks
    sequenceNumber = plPageInfoUtils::CombineSeqNum(kTemporarySequenceStartPrefix, 0);

    int32_t upperLimit = 0xFEFFFF; // largest legal sequence number is a prefix of FE and a suffix of FFFF
    for(; sequenceNumber < upperLimit; sequenceNumber++)
    {
        if (willBeReserved)
            toCompareTo = plLocation::MakeReserved(sequenceNumber);
        else
            toCompareTo = plLocation::MakeNormal(sequenceNumber);

        pageNode = FindPage(toCompareTo);
        if (pageNode == nullptr)
            return negated ? -sequenceNumber : sequenceNumber;
    }

    hsAssert(false, "Unable to find a valid sequence number to use");
    fLastVerifyError = kErrCantFindValid;
    return 0;
}

//// NukeKeyAndObject ////////////////////////////////////////////////////////
//  Given a key, will ref and unref the associated object so it goes away, 
//  then nukes the key and sets it to nil. The key's UOID should be safe to 
//  reuse at that point, unless someone else still has a ref, in which case
//  this function returns false (returns true if successful).

bool plPluginResManager::NukeKeyAndObject(plKey& objectKey)
{
    class plPublicRefKey : public plKeyImp
    {
    public:
        uint16_t GetRefCount() const { return fRefCount; }
    };

    plKeyImp* keyData = plKeyImp::GetFromKey(objectKey);

    // Check the ref count on the object. Nobody should have a ref to it
    // except the key
    hsKeyedObject* object = objectKey->ObjectIsLoaded();
    if (object != nullptr)
    {
        if (objectKey->GetActiveRefs())
            // Somebody still has a ref to this object, so we can't nuke it
            return false;
    }

    // Nobody has a ref to the object, so we're clear to nuke
    keyData->SetObjectPtr(nullptr);

    // Check the key. The refcount should be 1 at this point, for the copy
    // we're holding in this function. Nobody else should be holding the key
    // now that the object is gone.
    if (((plPublicRefKey*)keyData)->GetRefCount() > 1)
        return false;

    // Nuke out the key as well
    objectKey = nullptr;

    // All done!
    return true;
}

