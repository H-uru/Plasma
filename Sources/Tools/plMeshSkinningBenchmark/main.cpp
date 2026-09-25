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

#include <chrono>
#include <string_theory/stdio>

#include "hsMain.inl"
#include "plCreatableIndex.h"

#include "pnKeyedObject/plKeyImp.h"
#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGBufferGroup.h"
#include "plPipeline/plSoftwareMeshSkinner.h"
#include "plResMgr/plKeyFinder.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"
#include "plResMgr/plResManager.h"

using ClockT = std::chrono::steady_clock;

class plPageIt : public plRegistryPageIterator 
{
public:
    plLocation fLoc;

    bool EatPage(plRegistryPageNode* keyNode) override
    {
        fLoc = keyNode->GetPageInfo().GetLocation();
        return true;
    }
};

uint32_t IGetBufferFormatSize(uint8_t format)
{
    uint32_t size = sizeof(float) * 6 + sizeof(uint32_t) * 2; // Position and normal, and two packed colors

    switch (format & plGBufferGroup::kSkinWeightMask) {
        case plGBufferGroup::kSkinNoWeights:
            break;
        case plGBufferGroup::kSkin1Weight:
            size += sizeof(float);
            break;
        default:
            hsAssert(false, "Invalid skin weight value in IGetBufferFormatSize()");
    }

    return size + sizeof(float) * 3 * plGBufferGroup::CalcNumUVs(format);
}

static int hsMain(std::vector<ST::string> args)
{
    plResManager* fResMgr = new plResManager();
    hsgResMgr::Init(fResMgr);

    fResMgr->AddSinglePage("dat/CustomAvatars_District_RandMiller.prp");

    plPageIt it;
    fResMgr->IterateAllPages(&it);

    // Load all the keys
    plRegistryPageNode* fPageNode = fResMgr->FindPage(it.fLoc);
    fResMgr->LoadPageKeys(fPageNode);

    // Find the DrawableSpans
    plKeyImp* dsKey = fPageNode->FindKey(CLASS_INDEX_SCOPED(plDrawableSpans), "CustomAvatars_RandMiller_0ffffffe_10BlendSpans");

    if (!dsKey)
        return 1;

    // Load the page
    plDrawableSpans* drawable = plDrawableSpans::Convert(dsKey->VerifyLoaded());

    if (drawable->GetBlendingSpanVector().Empty())
        return 2;

    // First, figure out which buffers we need to blend.
    constexpr size_t kMaxBufferGroups = 20;
    constexpr size_t kMaxVertexBuffers = 20;
    static char blendBuffers[kMaxBufferGroups][kMaxVertexBuffers];
    memset(blendBuffers, 0, kMaxBufferGroups * kMaxVertexBuffers * sizeof(**blendBuffers));

    hsAssert(kMaxBufferGroups >= drawable->GetNumBufferGroups(), "Bigger than we counted on num groups skin.");

    const hsBitVector& blendBits = drawable->GetBlendingSpanVector();
    const std::vector<plSpan*>& spans = drawable->GetSpanArray();
    for (size_t idx = 0; idx < spans.size(); idx++) {
        if (blendBits.IsBitSet(idx)) {
            const plVertexSpan &vSpan = *(plVertexSpan *)spans[idx];
            hsAssert(kMaxVertexBuffers > vSpan.fVBufferIdx, "Bigger than we counted on num buffers skin.");

            blendBuffers[vSpan.fGroupIdx][vSpan.fVBufferIdx] = 1;
        }
    }

    int32_t count = 1000;
    auto elapsed = ClockT::duration::zero();

    for (int32_t i = 0; i < count; ++i) {
        auto begin = ClockT::now();

        // Now go through each of the group/buffer (= a real vertex buffer) pairs we found,
        // and blend into it. We'll lock the buffer once, and then for each span that
        // uses it, set the matrix palette and and then do the blend for that span.
        // When we've done all the spans for a group/buffer, we unlock it and move on.
        for (size_t i = 0; i < kMaxBufferGroups; i++) {
            for (size_t j = 0; j < kMaxVertexBuffers; j++) {
                if (blendBuffers[i][j]) {
                    plGBufferGroup* group = drawable->GetBufferGroup(i);

                    uint8_t format = group->GetVertexFormat();

                    // All indexed skinning is currently done on CPU, so the source data
                    // will have indices, but we strip them out for the D3D buffer.
                    if (format & plGBufferGroup::kSkinIndices) {
                        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
                        format |= plGBufferGroup::kSkinNoWeights;       // Should do nothing, but just in case...
                    }

                    uint32_t vertSize = IGetBufferFormatSize(format); // vertex stride

                    uint8_t* destPtr = new uint8_t[vertSize * group->GetVertBufferCount(j)];

                    for (size_t idx = 0; idx < spans.size(); idx++) {
                        const plIcicle& span = *(plIcicle*)spans[idx];
                        if ((span.fGroupIdx == i) && (span.fVBufferIdx == j)) {
                            hsMatrix44* matrixPalette = drawable->GetMatrixPalette(span.fBaseMatrix);
                            matrixPalette[0] = span.fLocalToWorld;

                            uint8_t* ptr = group->GetVertBufferData(j);
                            ptr += span.fVStartIdx * group->GetVertexSize();

                            plSoftwareMeshSkinner::BlendVertBuffer(&span,
                                                    matrixPalette, span.fNumMatrices,
                                                    ptr,
                                                    group->GetVertexFormat(),
                                                    group->GetVertexSize(),
                                                    destPtr + span.fVStartIdx * vertSize,
                                                    vertSize,
                                                    span.fVLength,
                                                    span.fLocalUVWChans );
                        }
                    }
                    // Unlock and move on.
                }
            }
        }

        auto end = ClockT::now();
        elapsed += end - begin;
    }

    ST::printf("\n... Done!\n\n");

    auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    auto avg_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed / count);
    auto total_sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
    auto avg_sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed / count);

    ST::printf("Results:\n");
    ST::printf("Total: {.4f} seconds ({} us)\n", total_sec.count(), total_us.count());
    ST::printf("Average: {.4f} seconds ({} us)\n", avg_sec.count(), avg_us.count());
    ST::printf("Have a nice day!\n");

    return 0;
}
