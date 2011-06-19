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
#ifndef plPagePatcher_h_inc
#define plPagePatcher_h_inc

#include "hsTemplates.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plKeyImp.h"
#include "plRegistryHelpers.h"

class plRegistryPageNode;

//// plKeyImpPublic //////////////////////////////////////////////////////////
//  Key class that stores objects as buffers for patching
class plKeyImpPublic : public plKeyImp
{
public:
    void SetObjectPtrDirect(void* pointer);
    void KillBuffer();
    void MoveFrom(plKeyImp* src);
    void WriteBuffer(hsStream* stream);
    void PatchUsingOldKey(plKeyImpPublic* oldPKey);
};


//// plReadObjectIterator ////////////////////////////////////////////////////
//  Helper key iterator that reads objects as buffers
class plReadObjectIterator : public plRegistryKeyIterator
{
protected:
    hsStream* fStream;

public:
    plReadObjectIterator(hsStream* stream);
    virtual bool EatKey(const plKey& key);

private:
    bool IReadObjectBuffer(plKeyImpPublic* pKey);
};


//// plWriteAndClearIterator /////////////////////////////////////////////////
//  Helper key iterator that writes object buffers to a stream
class plWriteAndClearIterator : public plRegistryKeyIterator
{
protected:
    hsStream* fStream;

public:
    plWriteAndClearIterator(hsStream* s);
    virtual bool EatKey(const plKey& key);
};


//// plPatchKeyIterator //////////////////////////////////////////////////////
//  Helper key iterator that applies patches to objects
class plPatchKeyIterator : public plRegistryKeyIterator
{
public:
    enum Errors { kNoError, kOldKeyNotFound, kOldKeyLengthInvalid };

    Errors fError;
    plUoid fErrorUoid;

protected:
    hsTArray<plRegistryPageNode*>& fPages;
    bool fIsPartial;

public:
    plPatchKeyIterator(hsTArray<plRegistryPageNode*>& pages, bool isPartial);
    virtual bool EatKey(const plKey& key);

private:
    plKeyImp* IFindOldKey(plUoid& uoid);
};

#endif
