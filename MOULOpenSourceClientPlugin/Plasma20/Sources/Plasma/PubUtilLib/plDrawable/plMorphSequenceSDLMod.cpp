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
#include "plMorphSequenceSDLMod.h"
#include "plMorphSequence.h"
#include "plSharedMesh.h"
#include "hsResMgr.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../plSDL/plSDL.h"

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
	
	const plMorphSequence *kMorphMod = nil;
	int i, j;
	kMorphMod = plMorphSequence::ConvertNoRef(sobj->GetModifierByType(plMorphSequence::Index()));
	if (!kMorphMod)
	{
		hsAssert(false, "Couldn't find a morph sequence.");
		return;
	}
	plMorphSequence *morphMod = const_cast<plMorphSequence*>(kMorphMod);
	
	//dstState->FindVar(kStrTarget)->Set(morphMod->GetKey());
	
	plSDStateVariable *morphSD = dstState->FindSDVar(kStrMorphs);
	int numMorphs = morphMod->fSharedMeshes.GetCount() + 1; // 1 for the non-sharedMesh morph
	hsTArray<plKey> keys;
	for (i = 0; i < numMorphs; i++)
	{
		if (i == morphMod->fSharedMeshes.GetCount())
		{
			// the non-sharedMesh morph
			if (morphMod->GetNumLayers(nil) != 0)
				keys.Append(nil);
		}
		else
		{
			if (!(morphMod->fSharedMeshes[i].fMesh->fFlags & plSharedMesh::kDontSaveMorphState))
				keys.Append(morphMod->fSharedMeshes[i].fMesh->GetKey());
		}
	}
	if (morphSD->GetCount() != keys.GetCount())
		morphSD->Alloc(keys.GetCount());
	for (i = 0; i < keys.GetCount(); i++)
	{
		plKey meshKey = keys[i];
		morphSD->GetStateDataRecord(i)->FindVar(kStrMesh)->Set(meshKey);
		
		plSimpleStateVariable *weights = morphSD->GetStateDataRecord(i)->FindVar(kStrWeights);
		int numLayers = morphMod->GetNumLayers(meshKey);
		if (weights->GetCount() != numLayers)
			weights->Alloc(numLayers);
		
		for (j = 0; j < numLayers; j++)
		{
			int numDeltas = morphMod->GetNumDeltas(j, meshKey);
			if (numDeltas != 2)
				continue; // plMorphSequenceSDLMod assumes 2 deltas (pos/neg) per layer, so that we can
						  // store both in a single byte

			// Translate the range [-1.0, 1.0] into a 0-255 byte
			UInt8 weight = (UInt8)((1.f + morphMod->GetWeight(j, 0, meshKey) - morphMod->GetWeight(j, 1, meshKey)) * 255 / 2);

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

	if (strcmp(srcState->GetDescriptor()->GetName(), kSDLMorphSequence))
	{
		hsAssert(false, "Wrong type of state data record passed into plMorphSequenceSDLMod.");
		return;
	}	

	int i, j;	
	const plMorphSequence *kMorphMod = plMorphSequence::ConvertNoRef(sobj->GetModifierByType(plMorphSequence::Index()));
	if (!kMorphMod)
	{
		hsAssert(false, "Couldn't find a morph sequence.");
		return;
	}
	plMorphSequence *morphMod = const_cast<plMorphSequence*>(kMorphMod);
	
	plSDStateVariable *morphSD = srcState->FindSDVar(kStrMorphs);
	for (i = 0; i < morphSD->GetCount(); i++)
	{
		plKey meshKey;
		morphSD->GetStateDataRecord(i)->FindVar(kStrMesh)->Get(&meshKey);
		if (meshKey && !meshKey->GetUoid().GetClassType() == plSharedMesh::Index())
			continue;
		
		// meshKey will be nil when dealing with non-sharedMesh data
		if (meshKey)
			hsgResMgr::ResMgr()->AddViaNotify(meshKey, TRACKED_NEW plGenRefMsg(morphMod->GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kPassiveRef);
		
		plSimpleStateVariable *weights = morphSD->GetStateDataRecord(i)->FindVar(kStrWeights);

		// Count down so that we do the high index first and the pending state struct
		// of plMorphSequence only has to resize the array once.
		for (j = weights->GetCount() - 1; j >= 0; j--)
		{			
			UInt8 weight;
			weights->Get(&weight, j);
			hsScalar posWeight = weight * 2.f / 255.f - 1.f;
			hsScalar negWeight = 0;

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
