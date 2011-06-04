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
#ifndef PL_AV_BEHAVIORS_H
#define PL_AV_BEHAVIORS_H

#include "hsTypes.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnTimer/plTimedValue.h"

class plAGAnim;
class plAGAnimInstance;
class plArmatureModBase;
class plArmatureBrain;
class plDebugText;


class plArmatureBehavior
{
public:
	plArmatureBehavior();
	virtual ~plArmatureBehavior();

	void Init(plAGAnim *anim, hsBool loop, plArmatureBrain *brain, plArmatureModBase *armature,  UInt8 index);
	virtual void Process(double time, float elapsed);
	virtual void SetStrength(hsScalar val, hsScalar rate = 0.f); // default instant change
	virtual hsScalar GetStrength();
	virtual void Rewind();
	void DumpDebug(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	enum
	{
		kBehaviorFlagNotifyOnStop = 0x01,
	};
	UInt32 fFlags;
	
protected:
	plAGAnimInstance *fAnim;
	plArmatureModBase *fArmature;
	plArmatureBrain *fBrain;
	plTimedValue<hsScalar> fStrength;
	UInt8 fIndex;

	virtual void IStart();
	virtual void IStop();
};

#endif
