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
#ifndef PL_AV_BRAIN_SWIM_H
#define PL_AV_BRAIN_SWIM_H

#include "plAvBrain.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

class plArmatureMod;
class plAntiGravAction;
class plControlEventMsg;
class plLOSRequestMsg;
class plSwimRegionInterface;
class plSwimmingController;
class plAvBrainSwim : public plArmatureBrain
{
public:
	plAvBrainSwim();
	virtual ~plAvBrainSwim();

	CLASSNAME_REGISTER( plAvBrainSwim );
	GETINTERFACE_ANY( plAvBrainSwim, plArmatureBrain );

	virtual void Activate(plArmatureModBase *avMod);
	hsBool Apply(double time, hsScalar elapsed);
	virtual void Deactivate();
	virtual void Suspend();
	virtual void Resume();
	virtual void DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);
	hsBool MsgReceive(plMessage *msg);
	bool IsWalking();
	bool IsWading();
	bool IsSwimming();
	hsScalar GetSurfaceDistance() { return fSurfaceDistance; }

	plSwimmingController *fCallbackAction;
	static const hsScalar kMinSwimDepth;
	
protected:
	void IStartWading();
	void IStartSwimming(bool is2D);
	hsBool IProcessSwimming2D(double time, float elapsed);
	hsBool IProcessSwimming3D(double time, float elapsed);
	hsBool IProcessWading(double time, float elapsed);
	hsBool IProcessClimbingOut(double time, float elapsed);
	hsBool IProcessBehaviors(double time, float elapsed);

	virtual hsBool IInitAnimations();
	bool IAttachAction();
	bool IDetachAction();
	void IProbeSurface();
	hsBool IHandleControlMsg(plControlEventMsg* msg);
	float IGetTargetZ();

	float fSurfaceDistance;
	plLOSRequestMsg *fSurfaceProbeMsg;
	plSwimRegionInterface *fCurrentRegion;
	
	enum Mode {
		kWading,
		kSwimming2D,
		kSwimming3D,
		kClimbingOut,
		kAbort,
		kWalking,
	} fMode;

	enum
	{
		kTreadWater,
		kSwimForward,
		kSwimForwardFast,
		kSwimBack,
		kSwimLeft,
		kSwimRight,
		kSwimTurnLeft,
		kSwimTurnRight,
		kTreadTurnLeft,
		kTreadTurnRight,
		kSwimBehaviorMax,
	};
};

#endif PL_AV_BRAIN_SWIM_H
