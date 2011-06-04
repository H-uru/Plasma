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
#include "plAvBehaviors.h"
#include "plAvBrainHuman.h"
#include "plArmatureMod.h"
#include "plAGAnimInstance.h"
#include "../plMessage/plAvatarMsg.h"

#include "../plPipeline/plDebugText.h"

plArmatureBehavior::plArmatureBehavior() : fAnim(nil), fArmature(nil), fBrain(nil), fIndex((UInt8)-1), fFlags(0) {}

plArmatureBehavior::~plArmatureBehavior() 
{
	if (fAnim)
		fAnim->Detach();
}

void plArmatureBehavior::Init(plAGAnim *anim, hsBool loop, plArmatureBrain *brain, plArmatureModBase *armature, UInt8 index)
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

void plArmatureBehavior::SetStrength(hsScalar val, hsScalar rate /* = 0.f */)
{
	hsScalar oldStrength = GetStrength();
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

hsScalar plArmatureBehavior::GetStrength()
{
	return fStrength.Value();
}

void plArmatureBehavior::Rewind()
{
	if (fAnim)
		fAnim->SetCurrentTime(0.0f, true);
}

void plArmatureBehavior::DumpDebug(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt)
{
	hsScalar strength = GetStrength();
	const char *onOff = strength > 0 ? "on" : "off";
	char blendBar[11] = "||||||||||";
	int bars = (int)__min(10 * strength, 10);
	blendBar[bars] = '\0';

	if (fAnim)
	{
		const char *animName = fAnim->GetName();
		float time = fAnim->GetTimeConvert()->CurrentAnimTime();	
		sprintf(strBuf, "%20s %3s time: %5.2f %s", animName, onOff, time, blendBar);
	}
	else
		sprintf(strBuf, "         Behavior %2d %3s %s", fIndex, onOff, blendBar);

	debugTxt.DrawString(x, y, strBuf);
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
