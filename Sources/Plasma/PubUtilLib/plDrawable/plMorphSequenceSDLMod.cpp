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

#include <vector>

#include "plMorphSequenceSDLMod.h"
#include "plMorphSequence.h"
#include "plSharedMesh.h"


#include "hsResMgr.h"

#include "pnMessage/plRefMsg.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plSDL/plSDL.h"

// static vars
char plMorphSequenceSDLMod::kStrMorphArrayDescName[]="MorphArray";
char plMorphSequenceSDLMod::kStrWeights[]="weights";

char plMorphSequenceSDLMod::kStrMorphSetDescName[]="MorphSet";
char plMorphSequenceSDLMod::kStrMesh[]="mesh";
char plMorphSequenceSDLMod::kStrArrays[]="arrays";

char plMorphSequenceSDLMod::kStrTargetID[]="targetID";
char plMorphSequenceSDLMod::kStrMorphs[]="morphs";

void plMorphSequenceSDLMod::PutCurrentStateIn(plStateDataRecord* dstState)
{
    IPutCurrentStateIn(dstState);
}

void plMorphSequenceSDLMod::IPutCurrentStateIn(plStateDataRecord* dstState)
{
    plSceneObject* sobj=GetTarget();
    hsAssert(sobj, "plMorphSequenceSDLMod, nil target");
    
    const plMorphSequence *kMorphMod = nullptr;
    kMorphMod = plMorphSequence::ConvertNoRef(sobj->GetModifierByType(plMorphSequence::Index()));
    if (!kMorphMod)
    {
        hsAssert(false, "Couldn't find a morph sequence.");
        return;
    }
    plMorphSequence *morphMod = const_cast<plMorphSequence*>(kMorphMod);
    
    //dstState->FindVar(kStrTarget)->Set(morphMod->GetKey());
    
    plSDStateVariable *morphSD = dstState->FindSDVar(kStrMorphs);
    size_t numMorphs = morphMod->fSharedMeshes.size() + 1; // 1 for the non-sharedMesh morph
    std::vector<plKey> keys;
    for (size_t i = 0; i < numMorphs; i++)
    {
        if (i == morphMod->fSharedMeshes.size())
        {
            // the non-sharedMesh morph
            if (morphMod->GetNumLayers(nullptr) != 0)
                keys.emplace_back(nullptr);
        }
        else
        {
            if (!(morphMod->fSharedMeshes[i].fMesh->fFlags & plSharedMesh::kDontSaveMorphState))
                keys.emplace_back(morphMod->fSharedMeshes[i].fMesh->GetKey());
        }
    }
    if (morphSD->GetCount() != keys.size())
        morphSD->Alloc(keys.size());
    for (size_t i = 0; i < keys.size(); i++)
    {
        const plKey &meshKey = keys[i];
        morphSD->GetStateDataRecord(i)->FindVar(kStrMesh)->Set(meshKey);
        
        plSimpleStateVariable *weights = morphSD->GetStateDataRecord(i)->FindVar(kStrWeights);
        size_t numLayers = morphMod->GetNumLayers(meshKey);
        if (weights->GetCount() != numLayers)
            weights->Alloc(numLayers);
        
        for (size_t j = 0; j < numLayers; j++)
        {
            size_t numDeltas = morphMod->GetNumDeltas(j, meshKey);
            if (numDeltas != 2)
                continue; // plMorphSequenceSDLMod assumes 2 deltas (pos/neg) per layer, so that we can
                          // store both in a single byte

            // Translate the range [-1.0, 1.0] into a 0-255 byte
            uint8_t weight = (uint8_t)((1.f + morphMod->GetWeight(j, 0, meshKey) - morphMod->GetWeight(j, 1, meshKey)) * 255 / 2);

            weights->Set(&weight, j);
        }
    }
}

void plMorphSequenceSDLMod::SetCurrentStateFrom(const plStateDataRecord* srcState)
{
    ISetCurrentStateFrom(srcState);
}       

void plMorphSequenceSDLMod::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
    plSceneObject* sobj=GetTarget();
    hsAssert(sobj, "plMorphSequenceSDLMod, nil target");

    if (srcState->GetDescriptor()->GetName() != kSDLMorphSequence)
    {
        hsAssert(false, "Wrong type of state data record passed into plMorphSequenceSDLMod.");
        return;
    }

    const plMorphSequence *kMorphMod = plMorphSequence::ConvertNoRef(sobj->GetModifierByType(plMorphSequence::Index()));
    if (!kMorphMod)
    {
        hsAssert(false, "Couldn't find a morph sequence.");
        return;
    }
    plMorphSequence *morphMod = const_cast<plMorphSequence*>(kMorphMod);
    
    plSDStateVariable *morphSD = srcState->FindSDVar(kStrMorphs);
    for (int i = 0; i < morphSD->GetCount(); i++)
    {
        plKey meshKey;
        morphSD->GetStateDataRecord(i)->FindVar(kStrMesh)->Get(&meshKey);
        if (meshKey && meshKey->GetUoid().GetClassType() != plSharedMesh::Index())
            continue;
        
        // meshKey will be nil when dealing with non-sharedMesh data
        if (meshKey)
            hsgResMgr::ResMgr()->AddViaNotify(meshKey, new plGenRefMsg(morphMod->GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kPassiveRef);
        
        plSimpleStateVariable *weights = morphSD->GetStateDataRecord(i)->FindVar(kStrWeights);

        // Count down so that we do the high index first and the pending state struct
        // of plMorphSequence only has to resize the array once.
        for (int j = weights->GetCount() - 1; j >= 0; j--)
        {
            uint8_t weight;
            weights->Get(&weight, j);
            float posWeight = weight * 2.f / 255.f - 1.f;
            float negWeight = 0;

            if (posWeight < 0)
            {
                negWeight = -posWeight;
                posWeight = 0;
            }
            morphMod->SetWeight(j, 1, negWeight, meshKey);          
            morphMod->SetWeight(j, 0, posWeight, meshKey);
        }
    }
}
