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

#include "plLayerSDLModifier.h"

#include "hsMatrix44.h"

#include "plSDL/plSDL.h"
#include "plSurface/plLayerAnimation.h"

// static vars
char plLayerSDLModifier::kStrAtc[]="atc";
char plLayerSDLModifier::kStrPassThruChannels[]="passThruChannels";
char plLayerSDLModifier::kStrTransform[]="transform";
char plLayerSDLModifier::kStrChannelData[]="channelData";
//char plLayerSDLModifier::kStrPreShadeColor[]="preshadeColor";
//char plLayerSDLModifier::kStrRuntimeColor[]="runtimeColor";
//char plLayerSDLModifier::kStrAmbientColor[]="ambientColor";
//char plLayerSDLModifier::kStrOpacity[]="opacity";

plKey plLayerSDLModifier::GetStateOwnerKey() const
{ 
    return fLayerAnimation ? fLayerAnimation->GetKey() : nullptr;
}

//
// Copy atc from current state into sdl
//
void plLayerSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
    plLayerAnimation* layer=GetLayerAnimation();
    hsAssert(layer, "nil layer animation");

    plSDStateVariable* atcVar = dstState->FindSDVar(kStrAtc);
    plStateDataRecord* atcStateDataRec = atcVar->GetStateDataRecord(0);
    IPutATC(atcStateDataRec, &layer->GetTimeConvert());

    int passThru=layer->fPassThruChannels;
    dstState->FindVar(kStrPassThruChannels)->Set(passThru);

    int transformSize = 0;
    if (layer->fTransform && (layer->fOwnedChannels & plLayerInterface::kTransform))
        transformSize = 16;

    plSimpleStateVariable *transformVar = dstState->FindVar(kStrTransform);
    if (transformVar->GetCount() != transformSize)
        transformVar->Alloc(transformSize);
    
    if (transformSize > 0)
    {
        int i,j;
        for(i=0;i<4;i++)
        {
            for(j=0;j<4;j++)
            {
                float f=layer->fTransform->fMap[i][j];
                transformVar->Set(&f, i*4+j);
            }
        }
    }

    int channelDataSize = 0;
    if (layer->fPreshadeColor && (layer->fOwnedChannels & plLayerInterface::kPreshadeColor))
        channelDataSize += 3;
    if (layer->fRuntimeColor && (layer->fOwnedChannels & plLayerInterface::kRuntimeColor))
        channelDataSize += 3;
    if (layer->fAmbientColor && (layer->fOwnedChannels & plLayerInterface::kAmbientColor))
        channelDataSize += 3;
    if (layer->fOpacity && (layer->fOwnedChannels & plLayerInterface::kOpacity))
        channelDataSize += 1;
    
    plSimpleStateVariable *channelVar = dstState->FindVar(kStrChannelData);
    if (channelVar->GetCount() != channelDataSize)
        channelVar->Alloc(channelDataSize);

    int channelIdx = 0;
    if (layer->fPreshadeColor && (layer->fOwnedChannels & plLayerInterface::kPreshadeColor))
    {
        channelVar->Set((uint8_t)(layer->fPreshadeColor->r * 255), channelIdx++);
        channelVar->Set((uint8_t)(layer->fPreshadeColor->g * 255), channelIdx++);
        channelVar->Set((uint8_t)(layer->fPreshadeColor->b * 255), channelIdx++);
    }       
    if (layer->fRuntimeColor && (layer->fOwnedChannels & plLayerInterface::kRuntimeColor))
    {
        channelVar->Set((uint8_t)(layer->fRuntimeColor->r * 255), channelIdx++);
        channelVar->Set((uint8_t)(layer->fRuntimeColor->g * 255), channelIdx++);
        channelVar->Set((uint8_t)(layer->fRuntimeColor->b * 255), channelIdx++);
    }       
    if (layer->fAmbientColor && (layer->fOwnedChannels & plLayerInterface::kAmbientColor))
    {
        channelVar->Set((uint8_t)(layer->fAmbientColor->r * 255), channelIdx++);
        channelVar->Set((uint8_t)(layer->fAmbientColor->g * 255), channelIdx++);
        channelVar->Set((uint8_t)(layer->fAmbientColor->b * 255), channelIdx++);
    }           
    if (layer->fOpacity && (layer->fOwnedChannels & plLayerInterface::kOpacity))
        channelVar->Set((uint8_t)(*layer->fOpacity * 255), channelIdx++);
}

//
// Change the object's animation state to reflect what is specified in the 
// stateDataRecord.
//
void plLayerSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
    plLayerAnimation* layer=GetLayerAnimation();
    hsAssert(layer, "nil layer animation");
    
    plSDStateVariable* atcVar = srcState->FindSDVar(kStrAtc);
    plStateDataRecord* atcStateDataRec = atcVar->GetStateDataRecord(0);
    ISetCurrentATC(atcStateDataRec, &layer->GetTimeConvert());
    
    int pass;
    if (srcState->FindVar(kStrPassThruChannels)->Get(&pass))
        layer->fPassThruChannels=pass;
    
    plSimpleStateVariable *transformVar = srcState->FindVar(kStrTransform);
    if (transformVar->IsUsed() && transformVar->GetCount() == 16)
    {
        int i,j;
        for(i=0;i<4;i++)
        {
            for(j=0;j<4;j++)
            {
                float f;
                srcState->FindVar(kStrTransform)->Get(&f, i*4+j);
                layer->fTransform->fMap[i][j]=f;
            }
        }
    }
    
    plSimpleStateVariable *channelVar = srcState->FindVar(kStrChannelData);
    int channelIdx = 0;
    uint8_t val;
    if (layer->fPreshadeColor && (layer->fOwnedChannels & plLayerInterface::kPreshadeColor))
    {
        channelVar->Get(&val, channelIdx++);
        layer->fPreshadeColor->r = val / 255.f;
        channelVar->Get(&val, channelIdx++);
        layer->fPreshadeColor->g = val / 255.f;
        channelVar->Get(&val, channelIdx++);
        layer->fPreshadeColor->b = val / 255.f;
    }       
    if (layer->fRuntimeColor && (layer->fOwnedChannels & plLayerInterface::kRuntimeColor))
    {
        channelVar->Get(&val, channelIdx++);
        layer->fRuntimeColor->r = val / 255.f;
        channelVar->Get(&val, channelIdx++);
        layer->fRuntimeColor->g = val / 255.f;
        channelVar->Get(&val, channelIdx++);
        layer->fRuntimeColor->b = val / 255.f;
    }       
    if (layer->fAmbientColor && (layer->fOwnedChannels & plLayerInterface::kAmbientColor))
    {
        channelVar->Get(&val, channelIdx++);
        layer->fAmbientColor->r = val / 255.f;
        channelVar->Get(&val, channelIdx++);
        layer->fAmbientColor->g = val / 255.f;
        channelVar->Get(&val, channelIdx++);
        layer->fAmbientColor->b = val / 255.f;
    }       
    if (layer->fOpacity && (layer->fOwnedChannels & plLayerInterface::kOpacity))
    {
        channelVar->Get(&val, channelIdx++);
        *layer->fOpacity = val / 255.f;
    }

    layer->fCurrentTime = -1.f; // force an eval
}

