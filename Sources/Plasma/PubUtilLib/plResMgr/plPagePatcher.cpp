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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "HeadSpin.h"
#include "plPagePatcher.h"

#include "plBSDiffBuffer.h"
#include "plDiffBuffer.h"
#include "plFileSystem.h"
#include "pnEncryption/plChecksum.h"
#include "pnKeyedObject/plKeyImp.h"
#include "plRegistryHelpers.h"
#include "plRegistryNode.h"
#include "hsStream.h"
#include "plStatusLog/plStatusLog.h"

#define KeyToKeyImpPublic(x) static_cast<plKeyImpPublic*>(static_cast<plKeyImp*>(x));

#ifdef HS_DEBUGGING
#   define PatcherLogDebug PatcherLog
#else
#   define PatcherLogDebug (void)
#endif

class plKeyImpPublic : public plKeyImp
{
public:
    plKeyImpPublic(const plKeyImpPublic& copy)
        : plKeyImp()
    {
        fStartPos = -1;
        fDataLen = copy.GetDataLen();
        fUoid = copy.GetUoid();

        // We don't own the other key's buffer, so don't point to it.
        fObjectPtr = nullptr;
    }

    void CreatePatch(plKeyImpPublic* const oldKey, plKeyImpPublic* const newKey)
    {
        hsAssert(newKey, "newKey cannot be nullptr");
        void* objptr = reinterpret_cast<void*>(fObjectPtr);

        PatcherLogDebug(kStatus, "\tDiffing: %s", GetUoid().StringIze().c_str());

        if (oldKey)
        {
            // Use a BSPATCH for changed objects
            plBSDiffBuffer bsdiff(newKey->GetDataLen(), oldKey->GetDataLen());
            bsdiff.Diff(oldKey->GetDataLen(), reinterpret_cast<void*>(oldKey->GetObjectPtr()),
                        newKey->GetDataLen(), reinterpret_cast<void*>(newKey->GetObjectPtr()));
            bsdiff.GetBuffer(fDataLen, objptr);
        }
        else
        {
            // A simple add buffer will suffice here
            plDiffBuffer diff(newKey->GetDataLen());
            diff.Add(newKey->GetDataLen(), reinterpret_cast<void*>(newKey->GetObjectPtr()));
            diff.GetBuffer(fDataLen, objptr);
        }
        SetObjectPtrDirect(objptr);
    }

    void KillBuffer()
    {
        delete[] reinterpret_cast<void*>(fObjectPtr);
        fObjectPtr = nullptr;
    }

    void MarkUnchanged()
    {
        PatcherLogDebug(kStatus, "\tUnchanged: %s", GetUoid().StringIze().c_str());
        if (fObjectPtr)
            KillBuffer();
        fStartPos = -1;
    }

    void MoveFrom(plKeyImpPublic* const src)
    {
        fObjectPtr = src->GetObjectPtr();
        fStartPos = -1;
        fDataLen = src->GetDataLen();
        fUoid = src->GetUoid();

        src->SetObjectPtrDirect(nullptr);
    }

    void SetObjectPtrDirect(void* pointer)
    {
        fObjectPtr = reinterpret_cast<hsKeyedObject*>(pointer);
    }

    void WriteBuffer(hsStream* const stream)
    {
        if (fObjectPtr)
        {
            fStartPos = stream->GetPosition();
            stream->Write(fDataLen, fObjectPtr);
        }
        else
            fStartPos = -1;
    }

    void PatchUsingOldKey(plKeyImpPublic* const oldPKey)
    {
        PatcherLogDebug(kStatus, "\tPatching: %s", GetUoid().StringIze().c_str());
        void* newData = nullptr;
        uint32_t newSize = 0;

        plDiffBuffer diffs(reinterpret_cast<void*>(fObjectPtr), fDataLen);
        diffs.Apply(oldPKey->fDataLen, reinterpret_cast<void*>(oldPKey->fObjectPtr), newSize, newData);
        oldPKey->KillBuffer();

        SetObjectPtrDirect(newData);
        fDataLen = newSize;
    }
};

//////////////////////////////////////////////////////////////////////////////

class plDiffKeyIterator : public plRegistryKeyIterator
{
    const plRegistryPageNode* const fOldPage;
    plRegistryPageNode* const fPatchPage;

public:
    plDiffKeyIterator(const plRegistryPageNode* const oldnode, plRegistryPageNode* const patchnode)
        : fOldPage(oldnode), fPatchPage(patchnode)
    { }

    bool EatKey(const plKey& key)
    {
        plKeyImpPublic* newKeyImp = KeyToKeyImpPublic(key);
        plKeyImpPublic* diffKey = new plKeyImpPublic(*newKeyImp);

        // First, attempt to find the object in the old page.
        plKeyImp* oldKey = fOldPage->FindKey(key->GetUoid());
        if (oldKey)
        {
            plKeyImpPublic* oldKeyImp = static_cast<plKeyImpPublic*>(oldKey);

            // Found the key, so first chance to be knocked out is by the data length.
            if (newKeyImp->GetDataLen() == oldKeyImp->GetDataLen())
            {
                // Maybe do something more optimized at some point?
                // We have a checksum library, but I'm lazy right now.
                int result = memcmp((void*)newKeyImp->GetObjectPtr(),
                                    (void*)oldKeyImp->GetObjectPtr(),
                                    newKeyImp->GetDataLen());
                if (result == 0)
                    diffKey->MarkUnchanged();
                else
                    diffKey->CreatePatch(oldKeyImp, newKeyImp);
            }
            else
            {
                // size different -- need to patch
                diffKey->CreatePatch(oldKeyImp, newKeyImp);
            }
        }
        else
        {
            // No old key, so we obviously want this one.
            diffKey->CreatePatch(nullptr, newKeyImp);
        }

        newKeyImp->KillBuffer();
        fPatchPage->AddKey(diffKey);
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

class plKillBufferIterator : public plRegistryKeyIterator
{
public:
    bool EatKey(const plKey& key)
    {
        plKeyImpPublic* imp = KeyToKeyImpPublic(key);
        imp->KillBuffer();
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

class plReadObjectIterator : public plRegistryKeyIterator
{
    hsStream* fStream;

    bool IReadObjectBuffer(plKeyImpPublic* const pKey)
    {
        if (pKey->GetStartPos() == -1 || pKey->GetDataLen() == -1)
            return false;

        uint32_t oldPos = fStream->GetPosition();
        fStream->SetPosition(pKey->GetStartPos());

        uint8_t* buffer = new uint8_t[pKey->GetDataLen()];
        fStream->Read(pKey->GetDataLen(), buffer);
        pKey->SetObjectPtrDirect(buffer);

        fStream->SetPosition(oldPos);
        return true;
    }

public:
    plReadObjectIterator(hsStream* const stream)
        : fStream(stream)
    { }

    bool EatKey(const plKey& key)
    {
        if (!key->ObjectIsLoaded())
        {
            plKeyImpPublic* pKey = KeyToKeyImpPublic(key);
            IReadObjectBuffer(pKey);
        }
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

class plPatchKeyIterator : public plRegistryKeyIterator
{
    plRegistryPageNode* fNewPage;
    plRegistryPageNode* fOldPage;
    bool fIsPartial;

public:
    enum Errors { kNoError, kOldKeyNotFound, kOldKeyLengthInvalid };

    Errors fError;
    plUoid fErrorUoid;

    plPatchKeyIterator(plRegistryPageNode* const newpage, plRegistryPageNode* const oldpage, bool isPartial)
            : fNewPage(newpage), fOldPage(oldpage), fIsPartial(isPartial), fError(kNoError)
    {
        fErrorUoid.Invalidate();
    }

    bool EatKey(const plKey& patchKey)
    {
        plKeyImpPublic* oldKey = nullptr;
        plKeyImpPublic* pKey = KeyToKeyImpPublic(patchKey);
        fNewPage->AddKey(pKey);

        if (pKey->GetStartPos() == -1)
        {
            plUoid uoid = pKey->GetUoid();
            oldKey = static_cast<plKeyImpPublic*>(fOldPage->FindKey(uoid));

            if (!oldKey)
            {
                fError = kOldKeyNotFound;
                fErrorUoid = uoid;
                return false;
            }
            else if (oldKey->GetDataLen() != pKey->GetDataLen())
            {
                fError = kOldKeyLengthInvalid;
                fErrorUoid = uoid;
                return false;
            }

            pKey->MoveFrom(oldKey);
        }
        else
        {
            if (pKey->ObjectIsLoaded())
            {
                plUoid uoid = pKey->GetUoid();
                oldKey = static_cast<plKeyImpPublic*>(fOldPage->FindKey(uoid));
                if (oldKey && fIsPartial)
                    pKey->PatchUsingOldKey(oldKey);
            }
        }
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

class plValidatePatchIterator : public plRegistryKeyIterator
{
    plRegistryPageNode* fPage;

public:
    plValidatePatchIterator(plRegistryPageNode* const patched)
        : fPage(patched)
    { }

    bool EatKey(const plKey& key)
    {
        plKeyImpPublic* srcKey = KeyToKeyImpPublic(key);
        plKeyImpPublic* patchedKey = static_cast<plKeyImpPublic*>(fPage->FindKey(srcKey->GetUoid()));
        PatcherLog(kMajorStatus, "\t%s", srcKey->GetUoid().StringIze().c_str());

        plMD5Checksum srcMD5(srcKey->GetDataLen(), reinterpret_cast<uint8_t*>(srcKey->GetObjectPtr()));
        PatcherLog(kStatus, "\t\tSource: %s", srcMD5.GetAsHexString());
        plMD5Checksum patchedMD5;
        if (patchedKey)
        {
            patchedMD5.Start();
            patchedMD5.AddTo(patchedKey->GetDataLen(), reinterpret_cast<uint8_t*>(patchedKey->GetObjectPtr()));
            patchedMD5.Finish();
            PatcherLog(kStatus, "\t\tPatched: %s", patchedMD5.GetAsHexString());
        }
        else
            PatcherLog(kStatus, "\t\tPatched: NULL KEY");

        if (srcMD5 == patchedMD5)
            PatcherLog(kStatus, "\t\tValidation Success!");
        else
            PatcherLog(kStatus, "\t\tValidation FAILED!");

        // Should be sufficient to unload all the buffers. If not, your data really sucks.
        srcKey->KillBuffer();
        if (patchedKey)
            patchedKey->KillBuffer();
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

class plWriteAndClearIterator : public plWriteIterator
{
public:
    bool EatKey(const plKey& key)
    {
        plKeyImpPublic* pKey = KeyToKeyImpPublic(key);
        pKey->WriteBuffer(fStream);
        pKey->KillBuffer();
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////

static bool ICheckPages(const plRegistryPageNode* const p1, const plRegistryPageNode* const p2)
{
    const plLocation& l1 = p1->GetPageInfo().GetLocation();
    const plLocation& l2 = p2->GetPageInfo().GetLocation();
    if (l1 != l2)
    {
        PatcherLog(kError, "\tPages have different locations: %s != %s",
                   l1.StringIze().c_str(), l2.StringIze().c_str());
        return false;
    }
    return true;
}

static bool ILoadObjectBuffers(plRegistryPageNode* const page)
{
    plReadObjectIterator iter(page->OpenStream());
    bool retval = page->IterateKeys(&iter);
    page->CloseStream();
    return retval;
}

static bool IOpenPage(plRegistryPageNode& node, const plFileName& page)
{
    node = plRegistryPageNode(page);
    if (!node.IsValid())
    {
        const char* reason = 0;
        switch (node.GetPageCondition())
        {
        case kNodeNotReady:
            reason = "RPN not properly created";
            break;
        case kPageOutOfDate:
            reason = "too old";
            break;
        case kPageTooNew:
            reason = "too new";
            break;
        case kPageCorrupt:
            reason = "corrupt";
            break;
        default:
            reason = "???";
            break;
        }
        PatcherLog(kError, "Page `%s` is invalid (%s)", page.AsString().c_str(), reason);
        return false;
    }
    return true;
}

plRegistryPageNode* plPagePatcher::GeneratePatch(const plFileName& oldpage, const plFileName& newpage)
{
    // Step 1: Open up the base files (old and new pages)
    plRegistryPageNode oldnode;
    if (!IOpenPage(oldnode, oldpage))
        return nullptr;
    plRegistryPageNode newnode;
    if (!IOpenPage(newnode, newpage))
        return nullptr;
    PatcherLog(kHeader, "--- Begin Diff for %s ---", newnode.GetPageInfo().StringIze().c_str());

    // Step 1.1: Sanity
    if (!ICheckPages(&oldnode, &newnode))
        return nullptr;

    // Step 1.5: Setup a new patch RPN -- note you must set the write path (SetPagePathDirect)
    plRegistryPageNode* patchnode = new plRegistryPageNode(newnode.GetPageInfo());
    const_cast<plPageInfo&>(patchnode->GetPageInfo()).SetFlag(plPageInfo::kPartialPatchFile, true);

    // Step 2: Load keys from both pages
    oldnode.LoadKeys();
    newnode.LoadKeys();

    // Step 2.1: Iterate through the pages and load the objects into KeyImpPublics
    ILoadObjectBuffers(&oldnode);
    ILoadObjectBuffers(&newnode);

    // Step 3: Iterate through the ***new*** page with the diff iterator.
    //         End result: keys deleted from the new page not in the patch.
    plDiffKeyIterator iter(&oldnode, patchnode);
    newnode.IterateKeys(&iter);

    // Step 3.1: Iterate through pages and kill their object buffers.
    //           We do this because plKeyImp's dtor and the entire resmgr expects for fObjectPtr to be nullptr
    plKillBufferIterator kill;
    oldnode.IterateKeys(&kill);
    newnode.IterateKeys(&kill);

    // Done!
    PatcherLog(kHeader, "--- End Diff for %s ---", newnode.GetPageInfo().StringIze().c_str());
    return patchnode;
}

plRegistryPageNode* plPagePatcher::PatchPage(const plFileName& oldpage, const plFileName& patchpage)
{
    // Step 1: Open up the base files (old and new pages)
    plRegistryPageNode oldnode;
    if (!IOpenPage(oldnode, oldpage))
        return nullptr;
    plRegistryPageNode patchnode;
    if (!IOpenPage(patchnode, patchpage))
        return nullptr;
    PatcherLog(kHeader, "--- Begin Patch for %s ---", patchnode.GetPageInfo().StringIze().c_str());

    // Step 1.1: Sanity
    if (!ICheckPages(&oldnode, &patchnode))
        return nullptr;

    // Step 1.5: Setup the new registry page node we're going to flush to
    plRegistryPageNode* newnode = new plRegistryPageNode(patchnode.GetPageInfo());
    const_cast<plPageInfo&>(newnode->GetPageInfo()).SetFlag(plPageInfo::kPatchFlags, false);

    // Step 2: Load the keys from the source page and the patch
    oldnode.LoadKeys();
    patchnode.LoadKeys();

    // Step 2.1: Iterate through the pages and load the buffers into KeyImpPublics
    ILoadObjectBuffers(&oldnode);
    ILoadObjectBuffers(&patchnode);

    // Step 3: Iterate through the patch keys and apply the diffs where needed
    plPatchKeyIterator iter(newnode, &oldnode, patchnode.GetPageInfo().HasFlag(plPageInfo::kPartialPatchFile));
    if (!patchnode.IterateKeys(&iter))
    {
        char* errormsg = nullptr;
        switch (iter.fError)
        {
        case plPatchKeyIterator::kOldKeyLengthInvalid:
            errormsg = "KeyData lengths do not match";
            break;
        case plPatchKeyIterator::kOldKeyNotFound:
            errormsg = "Key omitted in patch not found in dataset";
            break;
        };
        PatcherLog(kHeader, "\tERROR: %s\n\tKey: %s\n--- Patch FAILED for %s ---",
                   errormsg, iter.fErrorUoid.StringIze().c_str(),
                   patchnode.GetPageInfo().StringIze().c_str());
        return nullptr;
    }

    // Step 3.1: Iterate through the old page and kill its object buffers.
    //           We do this because plKeyImp's dtor and the entire resmgr expects for fObjectPtr to be nullptr
    plKillBufferIterator kill;
    oldnode.IterateKeys(&kill);

    // Done!
    PatcherLog(kHeader, "--- Patch Successful for %s ---", patchnode.GetPageInfo().StringIze().c_str());
    return newnode;
}

bool plPagePatcher::ValidatePatch(const plFileName& sourcePage, const plFileName& patchedPage)
{
    // Step 1: Basic MD5 of both files
    plMD5Checksum sourceMD5(sourcePage.AsString().c_str());
    plMD5Checksum patchedMD5(patchedPage.AsString().c_str());

    PatcherLog(kMajorStatus, "Source Page: %s\nPatched Page: %s",
                sourceMD5.GetAsHexString(),
                patchedMD5.GetAsHexString());

    // Step 2: Load pages for detailed validation
    plRegistryPageNode source;
    if (!IOpenPage(source, sourcePage))
        return false;
    plRegistryPageNode patched;
    if (!IOpenPage(patched, patchedPage))
        return false;
    PatcherLog(kHeader, "--- Begin Validation for %s ---", source.GetPageInfo().StringIze().c_str());

    // Step 3: Load all keys
    source.LoadKeys();
    patched.LoadKeys();

    // Step 3.1: Load Object Buffers
    ILoadObjectBuffers(&source);
    ILoadObjectBuffers(&patched);

    // Step 4: Iterate through the keys in the source file and compare with those in the patched file
    //         Unlike the other steps like this one, this one does totally kill off object buffers
    plValidatePatchIterator iter(&patched);
    source.IterateKeys(&iter);

    PatcherLog(kHeader, "--- End Validation for %s ---", source.GetPageInfo().StringIze().c_str());
    return false; // file checksums are different so it failed anyway
}

void plPagePatcher::WriteAndClear(plRegistryPageNode* const page, const plFileName& path)
{
    page->SetPagePathDirect(path);
    plWriteAndClearIterator iter;
    page->Write(&iter);
}


char* g_PatcherError = nullptr;

void PatcherLog(PatcherLogType type, const char* format, ...)
{
    uint32_t color = 0;
    switch (type)
    {
    case kHeader:       color = plStatusLog::kWhite;    break;
    case kInfo:         color = plStatusLog::kBlue;     break;
    case kMajorStatus:  color = plStatusLog::kYellow;   break;
    case kStatus:       color = plStatusLog::kGreen;    break;
    case kError:        color = plStatusLog::kRed;      break;
    }

    va_list args;
    va_start(args, format);

    plStatusLog* log = plStatusLogMgr::GetInstance().FindLog("patcher.log", true);
    if (type == kError)
    {
        g_PatcherError = new char[1024]; // Deleted by Finish(false)
        vsprintf(g_PatcherError, format, args);
        log->AddLine(g_PatcherError, color);
    } else
        log->AddLineV(color, format, args);

    va_end(args);
}
