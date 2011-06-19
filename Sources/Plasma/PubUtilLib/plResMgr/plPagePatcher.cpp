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
#include "plDiffBuffer.h"
#include "plRegistryNode.h"
#include "hsStream.h"

#define KeyToKeyImpPublic(x) static_cast<plKeyImpPublic*>(static_cast<plKeyImp*>(x));

void plKeyImpPublic::SetObjectPtrDirect(void* pointer)
{
    fObjectPtr = (hsKeyedObject*)pointer;
}

void plKeyImpPublic::KillBuffer()
{
    delete[] fObjectPtr;
    fObjectPtr = nullptr;
}

void plKeyImpPublic::MoveFrom(plKeyImp* src)
{
    fObjectPtr = src->GetObjectPtr();
    fStartPos = -1;
    fDataLen = src->GetDataLen();
    fUoid = src->GetUoid();

    src->SetObjectPtr(nullptr);
}

void plKeyImpPublic::WriteBuffer(hsStream* stream)
{
    if (fObjectPtr)
    {
        fStartPos = stream->GetPosition();
        stream->Write(fDataLen, fObjectPtr);
    }
    else
    {
        fStartPos = -1;
        fDataLen = -1;
    }
}

void plKeyImpPublic::PatchUsingOldKey(plKeyImpPublic* oldPKey)
{
    uint32_t newLen = 0;
    void* newData = nullptr;
    plDiffBuffer diffs(fObjectPtr, fDataLen);

    diffs.Apply(oldPKey->fDataLen, oldPKey->fObjectPtr, newLen, newData);
    oldPKey->KillBuffer();

    fDataLen = newLen;
    fObjectPtr = (hsKeyedObject*)newData;
}

//////////////////////////////////////////////////////////////////////////////

plReadObjectIterator::plReadObjectIterator(hsStream* stream)
    : fStream(stream)
{ }

bool plReadObjectIterator::EatKey(const plKey& key)
{
    if (!key->ObjectIsLoaded())
    {
        plKeyImpPublic* pKey = KeyToKeyImpPublic(key);
        IReadObjectBuffer(pKey);
    }
    return true;
}

bool plReadObjectIterator::IReadObjectBuffer(plKeyImpPublic* pKey)
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

//////////////////////////////////////////////////////////////////////////////

plWriteAndClearIterator::plWriteAndClearIterator(hsStream* s)
    : fStream(s)
{ }

bool plWriteAndClearIterator::EatKey(const plKey& key)
{
    plKeyImpPublic* pKey = KeyToKeyImpPublic(key);
    pKey->WriteBuffer(fStream);
    pKey->KillBuffer();
    return true;
}

//////////////////////////////////////////////////////////////////////////////

plPatchKeyIterator::plPatchKeyIterator(hsTArray<plRegistryPageNode*>& pages, bool isPartial)
        : fPages(pages), fIsPartial(isPartial)
{
    fErrorUoid.Invalidate();
    fError = kNoError;
}

bool plPatchKeyIterator::EatKey(const plKey& patchKey)
{
    plKeyImp* oldKey = nullptr;
    plKeyImpPublic* pKey = KeyToKeyImpPublic(patchKey);

    if (pKey->GetStartPos() == -1)
    {
        plUoid uoid = pKey->GetUoid();
        oldKey = IFindOldKey(uoid);

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
            oldKey = IFindOldKey(uoid);
            if (oldKey && fIsPartial)
                pKey->PatchUsingOldKey(static_cast<plKeyImpPublic*>(oldKey));
        }
    }
    return true;
}

plKeyImp* plPatchKeyIterator::IFindOldKey(plUoid& uoid)
{
    for (int i = 0; i < fPages.GetCount(); i++)
    {
        if (uoid.GetLocation() == fPages[i]->GetPageInfo().GetLocation())
            return fPages[i]->FindKey(uoid);
    }
    return nullptr;
}
