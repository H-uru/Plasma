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

// The override-layer and piggyback stacks.
//
// Piggybacks are layers appended to every pass of every material -- projective
// lighting and light maps use them. If a material draws in two passes,
//      pass0: layer0+layer1
//      pass1: layer2
// then with a piggyback active it draws
//      pass0: layer0+layer1+piggyback
//      pass1: layer2+piggyback
//
// Override layers are different: they wrap an existing layer rather than sitting
// beside it, so the wrapped layer's state is what the pass sees.
//
// Ported from plMetalPipeline.cpp:2629-2782.

#include "plVulkanPipeline.h"

#include "plVulkanMaterialShaderRef.h"

#include "hsGMatState.h"
#include "plPipeDebugFlags.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include <algorithm>

plLayerInterface* plVulkanPipeline::IPushOverBaseLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    fOverLayerStack.push_back(li);

    if (!fOverBaseLayer)
        return fOverBaseLayer = li;

    fOverBaseLayer = fOverBaseLayer->Attach(li);
    fOverBaseLayer->Eval(fTime, fFrame, 0);
    return fOverBaseLayer;
}

plLayerInterface* plVulkanPipeline::IPopOverBaseLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    plLayerInterface* pop = fOverLayerStack.back();
    fOverLayerStack.pop_back();
    fOverBaseLayer = fOverBaseLayer->Detach(pop);

    return pop;
}

plLayerInterface* plVulkanPipeline::IPushOverAllLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    fOverLayerStack.push_back(li);

    if (!fOverAllLayer) {
        fOverAllLayer = li;
        fOverAllLayer->Eval(fTime, fFrame, 0);
        return fOverAllLayer;
    }

    fOverAllLayer = fOverAllLayer->Attach(li);
    fOverAllLayer->Eval(fTime, fFrame, 0);

    return fOverAllLayer;
}

plLayerInterface* plVulkanPipeline::IPopOverAllLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    plLayerInterface* pop = fOverLayerStack.back();
    fOverLayerStack.pop_back();
    fOverAllLayer = fOverAllLayer->Detach(pop);

    return pop;
}

void plVulkanPipeline::IPushPiggyBacks(hsGMaterial* mat)
{
    hsAssert(!fMatPiggyBacks, "Push/Pop Piggy mismatch");

    if (fView.fRenderState & plPipeline::kRenderNoPiggyBacks)
        return;

    for (size_t i = 0; i < mat->GetNumPiggyBacks(); i++) {
        if (!mat->GetPiggyBack(i))
            continue;

        if ((mat->GetPiggyBack(i)->GetMiscFlags() & hsGMatState::kMiscLightMap) &&
            IsDebugFlagSet(plPipeDbg::kFlagNoLightmaps)) {
            continue;
        }

        fPiggyBackStack.push_back(mat->GetPiggyBack(i));
        fMatPiggyBacks++;
    }

    ISetNumActivePiggyBacks();
}

void plVulkanPipeline::IPopPiggyBacks()
{
    if (fView.fRenderState & plPipeline::kRenderNoPiggyBacks)
        return;

    fPiggyBackStack.resize(fPiggyBackStack.size() - fMatPiggyBacks);
    fMatPiggyBacks = 0;

    ISetNumActivePiggyBacks();
}

size_t plVulkanPipeline::ISetNumActivePiggyBacks()
{
    return fActivePiggyBacks = std::min(fMaxPiggyBacks, uint32_t(fPiggyBackStack.size()));
}
