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

#include "plAvBehaviors.h"

#include <algorithm>
#include <string_theory/format>

#include "plAvBrainHuman.h"
#include "plArmatureMod.h"

#include "plAnimation/plAGAnimInstance.h"
#include "plInterp/plAnimTimeConvert.h"
#include "plPipeline/plDebugText.h"

plArmatureBehavior::plArmatureBehavior()
    : fAnim(), fArmature(), fBrain(), fIndex((uint8_t)-1), fFlags()
{ }

plArmatureBehavior::~plArmatureBehavior() 
{
    if (fAnim)
        fAnim->Detach();
}

void plArmatureBehavior::Init(plAGAnim *anim, bool loop, plArmatureBrain *brain, plArmatureModBase *armature, uint8_t index)
{
    fArmature = armature;
    fBrain = brain;
    fIndex = index;
    fStrength.Set(0);
    if (anim)
    {   
        fAnim = fArmature->AttachAnimationBlended(anim, 0, 0, true);
        fAnim->SetLoop(loop);
    }
}

void plArmatureBehavior::Process(double time, float elapsed)
{
}

void plArmatureBehavior::SetStrength(float val, float rate /* = 0.f */)
{
    float oldStrength = GetStrength();
    if (rate == 0)
        fStrength.Set(val);
    else
        fStrength.Set(val, fabs(val - oldStrength) / rate);

    if (fAnim)
        fAnim->Fade(val, rate);
    if (val > 0 && oldStrength == 0)
        IStart();
    else if (val == 0 && oldStrength > 0)
        IStop();
}

float plArmatureBehavior::GetStrength()
{
    return fStrength.Value();
}

void plArmatureBehavior::Rewind()
{
    if (fAnim)
        fAnim->SetCurrentTime(0.0f, true);
}

void plArmatureBehavior::DumpDebug(int &x, int &y, int lineHeight, plDebugText &debugTxt)
{
    float strength = GetStrength();
    const char *onOff = strength > 0 ? "on" : "off";
    char blendBar[] = "||||||||||";
    int bars = std::min(static_cast<int>(10 * strength), 10);
    blendBar[bars] = '\0';

    ST::string details;
    if (fAnim)
    {
        float time = fAnim->GetTimeConvert()->CurrentAnimTime();
        details = ST::format("{>20} {>3} time: {5.2f} {}", fAnim->GetName(), onOff, time, blendBar);
    }
    else
        details = ST::format("         Behavior {2} {>3} {}", fIndex, onOff, blendBar);

    debugTxt.DrawString(x, y, details);
    y += lineHeight; 
}


void plArmatureBehavior::IStart()
{
    if (fAnim) 
    {           
        fAnim->SetCurrentTime(0.0f, true);
        fAnim->Start();
    }
}
    
void plArmatureBehavior::IStop()
{
    if (fFlags & kBehaviorFlagNotifyOnStop)
        fBrain->OnBehaviorStop(fIndex);
}
