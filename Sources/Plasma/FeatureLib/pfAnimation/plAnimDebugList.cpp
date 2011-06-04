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
#include "hsStlUtils.h"
#include "hsResMgr.h"
#include "hsTemplates.h"
#include "hsTimer.h"
#include "plAnimDebugList.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerAnimation.h"
#include "../plAvatar/plAGMasterMod.h"
#include "../plAvatar/plAGAnimInstance.h"
#include "../plAvatar/plAGAnim.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plPipeline/plDebugText.h"

void plAnimDebugList::AddObjects(char *subString)
{
	std::vector<plKey> keys;
	std::vector<plKey>::iterator i;

	plKeyFinder::Instance().ReallyStupidSubstringSearch(subString, hsGMaterial::Index(), keys);
	for (i = keys.begin(); i != keys.end(); i++)
	{
		if (fMaterialKeys.Find((*i)) == fMaterialKeys.kMissingIndex)
			fMaterialKeys.Append((*i));
	}

	keys.clear();
	plKeyFinder::Instance().ReallyStupidSubstringSearch(subString, plSceneObject::Index(), keys);
	for (i = keys.begin(); i != keys.end(); i++)
	{
		plSceneObject *so = plSceneObject::ConvertNoRef((*i)->ObjectIsLoaded());
		if (so)
		{
			const plAGMasterMod *agMod = plAGMasterMod::ConvertNoRef(so->GetModifierByType(plAGMasterMod::Index()));
			if (agMod && fSOKeys.Find(so->GetKey()) == fSOKeys.kMissingIndex)
				fSOKeys.Append(so->GetKey());
		}
	}
}

void plAnimDebugList::RemoveObjects(char *subString)
{
	int i;
	for (i = fMaterialKeys.GetCount() - 1; i >= 0; i--)
	{
		if (strstr(fMaterialKeys[i]->GetName(), subString))
			fMaterialKeys.Remove(i);
	}

	for (i = fSOKeys.GetCount() - 1; i >= 0; i--)
	{
		if (strstr(fSOKeys[i]->GetName(), subString))
			fSOKeys.Remove(i);
	}
}

void plAnimDebugList::ShowReport()
{
	if (!fEnabled)
		return;

	plDebugText		&txt = plDebugText::Instance();

	int y,x,i,j;
	const int yOff=10, startY=40, startX=10;
	char str[256];

	x = startX;
	y = startY;
	txt.DrawString(x, y, "Material Animations:", 255, 255, 255, 255, plDebugText::kStyleBold);
	y += yOff;
	for (i = 0; i < fMaterialKeys.GetCount(); i++)
	{
		hsGMaterial *mat = hsGMaterial::ConvertNoRef(fMaterialKeys[i]->ObjectIsLoaded());
		if (!mat)
			continue;

		for (j = 0; j < mat->GetNumLayers(); j++)
		{
			plLayerInterface *layer = mat->GetLayer(j)->BottomOfStack();
			while (layer != nil)
			{
				plLayerAnimation *layerAnim = plLayerAnimation::ConvertNoRef(layer);
				if (layerAnim)
				{
					sprintf(str, "%s: %s %.3f (%.3f)", mat->GetKeyName(), layerAnim->GetKeyName(), 
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
	
	for (i = 0; i < fSOKeys.GetCount(); i++)
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(fSOKeys[i]->ObjectIsLoaded());
		if (!so)
			continue;

		plAGMasterMod *mod = const_cast<plAGMasterMod*>(plAGMasterMod::ConvertNoRef(so->GetModifierByType(plAGMasterMod::Index())));
		if (!mod)
			continue;

		sprintf(str, "  %s", so->GetKeyName());
		txt.DrawString(x, y, str);
		y += yOff;

		for (j = 0; j < mod->GetNumATCAnimations(); j++)
		{
			plAGAnimInstance *anim = mod->GetATCAnimInstance(j);
			sprintf(str, "    %s: %.3f (%.3f)", anim->GetAnimation()->GetName(), 
					anim->GetTimeConvert()->CurrentAnimTime(),
					anim->GetTimeConvert()->WorldToAnimTimeNoUpdate(hsTimer::GetSysSeconds()));
			txt.DrawString(x, y, str);
			y += yOff;
		}
	}
}
