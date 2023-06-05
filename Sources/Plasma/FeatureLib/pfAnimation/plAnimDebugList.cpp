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

#include "plAnimDebugList.h"

#include "hsResMgr.h"
#include "hsTimer.h"

#include <string_theory/format>

#include "pnSceneObject/plSceneObject.h"

#include "plAnimation/plAGAnim.h"
#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plAGMasterMod.h"
#include "plPipeline/plDebugText.h"
#include "plResMgr/plKeyFinder.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerAnimation.h"

void plAnimDebugList::AddObjects(const ST::string &subString)
{
    std::vector<plKey> keys;

    plKeyFinder::Instance().ReallyStupidSubstringSearch(subString, hsGMaterial::Index(), keys);
    for (const plKey& key : keys)
    {
        if (std::find(fMaterialKeys.cbegin(), fMaterialKeys.cend(), key) == fMaterialKeys.cend())
            fMaterialKeys.emplace_back(key);
    }

    keys.clear();
    plKeyFinder::Instance().ReallyStupidSubstringSearch(subString, plSceneObject::Index(), keys);
    for (const plKey& key : keys)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
        if (so)
        {
            const plAGMasterMod *agMod = plAGMasterMod::ConvertNoRef(so->GetModifierByType(plAGMasterMod::Index()));
            if (agMod && std::find(fSOKeys.cbegin(), fSOKeys.cend(), so->GetKey()) == fSOKeys.cend())
                fSOKeys.emplace_back(so->GetKey());
        }
    }
}

void plAnimDebugList::RemoveObjects(const ST::string &subString)
{
    for (auto iter = fMaterialKeys.begin(); iter != fMaterialKeys.end(); )
    {
        if ((*iter)->GetName().contains(subString))
            iter = fMaterialKeys.erase(iter);
        else
            ++iter;
    }

    for (auto iter = fSOKeys.begin(); iter != fSOKeys.end(); )
    {
        if ((*iter)->GetName().contains(subString))
            iter = fSOKeys.erase(iter);
        else
            ++iter;
    }
}

void plAnimDebugList::ShowReport()
{
    if (!fEnabled)
        return;

    plDebugText     &txt = plDebugText::Instance();

    int y, x;
    const int yOff=10, startY=40, startX=10;
    ST::string str;

    x = startX;
    y = startY;
    txt.DrawString(x, y, "Material Animations:", 255, 255, 255, 255, plDebugText::kStyleBold);
    y += yOff;
    for (const plKey& matKey : fMaterialKeys)
    {
        hsGMaterial *mat = hsGMaterial::ConvertNoRef(matKey->ObjectIsLoaded());
        if (!mat)
            continue;

        for (size_t j = 0; j < mat->GetNumLayers(); j++)
        {
            plLayerInterface *layer = mat->GetLayer(j)->BottomOfStack();
            while (layer != nullptr)
            {
                plLayerAnimation *layerAnim = plLayerAnimation::ConvertNoRef(layer);
                if (layerAnim)
                {
                    str = ST::format("{}: {} {.3f} ({.3f})", mat->GetKeyName(), layerAnim->GetKeyName(),
                            layerAnim->GetTimeConvert().CurrentAnimTime(),
                            layerAnim->GetTimeConvert().WorldToAnimTimeNoUpdate(hsTimer::GetSysSeconds()));
                    txt.DrawString(x, y, str);
                    y += yOff;
                }
                layer = layer->GetOverLay();
            }
        }
    }
    y += yOff;
    txt.DrawString(x, y, "AGMaster Anims", 255, 255, 255, 255, plDebugText::kStyleBold);
    y += yOff;

    for (const plKey& soKey : fSOKeys)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(soKey->ObjectIsLoaded());
        if (!so)
            continue;

        plAGMasterMod *mod = const_cast<plAGMasterMod*>(plAGMasterMod::ConvertNoRef(so->GetModifierByType(plAGMasterMod::Index())));
        if (!mod)
            continue;

        str = ST::format("  {}", so->GetKeyName());
        txt.DrawString(x, y, str);
        y += yOff;

        for (int j = 0; j < mod->GetNumATCAnimations(); j++)
        {
            plAGAnimInstance *anim = mod->GetATCAnimInstance(j);
            str = ST::format("    {}: {.3f} ({.3f})", anim->GetAnimation()->GetName(),
                    anim->GetTimeConvert()->CurrentAnimTime(),
                    anim->GetTimeConvert()->WorldToAnimTimeNoUpdate(hsTimer::GetSysSeconds()));
            txt.DrawString(x, y, str);
            y += yOff;
        }
    }
}
