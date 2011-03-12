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

#include <stdlib.h>

#include "hsTypes.h"
#include "plRandomCommandMod.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "plgDispatch.h"
#include "hsTimer.h"
#include "hsUtils.h"

static const hsScalar kRandNormalize = 1.f / 32767.f;

plRandomCommandMod::plRandomCommandMod()
{
	fState = 0;
	fMode = kNormal;
	fCurrent = -1;

	fMinDelay = 0;
	fMaxDelay = 0;
}

plRandomCommandMod::~plRandomCommandMod()
{
}

// return how many are left to choose from
int plRandomCommandMod::IExcludeSelections(int ncmds)
{
	if( fMode & kCoverall )
	{
		int nLeft = ncmds;

		fExcluded.SetBit(fCurrent);

		// Count how many haven't been played.
		int i;
		for( i = ncmds-1; i >= 0; --i )
		{
			if( fExcluded.IsBitSet(i) )
				nLeft--;
		}

		// If we're out of cmds, and OneCycle is set,
		// we're out of cmds until the next play.
		// Go ahead and reset for that.
		// If we're out and OneCycle isn't set, then go ahead 
		// and set up for a new cycle.
		if( !nLeft )
		{
			fExcluded.Clear();
			if( fMode & kNoRepeats )
				fExcluded.SetBit(fCurrent);

			if( fMode & kOneCycle )
				return 0;

			nLeft = ncmds;
			if( ( fMode & kNoRepeats ) && ncmds > 1 )
				nLeft--;
		}

		return nLeft;
	}
	double currTime = hsTimer::GetSysSeconds();
	fExcluded.Clear();
	int i;
	for( i = 0; i < fEndTimes.GetCount(); i++ )
	{
		if( fEndTimes[i] > currTime )
		{
			ncmds--;
			fExcluded.SetBit(i);
		}
	}
	if( fMode & kNoRepeats )
	{
		ncmds--;
		fExcluded.SetBit(fCurrent);
		return ncmds;
	}

	return ncmds;
}

hsScalar plRandomCommandMod::IGetDelay(hsScalar len) const
{
	hsScalar r = float(hsRand() * kRandNormalize);

	hsScalar delay = fMinDelay + (fMaxDelay - fMinDelay) * r;

	if( fMode & kDelayFromEnd )
		delay += len;

	if( delay < 0 )
		delay = fmodf(len, -delay);

	return delay;
}

hsBool plRandomCommandMod::ISelectNext(int ncmds)
{
	if( fMode & kSequential )
	{
		if( ++fCurrent >= ncmds )
		{
			if( fMode & kOneCycle )
			{
				fCurrent = -1;
				return false;
			}
			fCurrent = 0;
		}
		return true;
	}
	hsScalar r = float(hsRand() * kRandNormalize);

	int nSelect = ncmds;
	
	if( fCurrent >= 0 )
		nSelect = IExcludeSelections(ncmds);

	if( !nSelect )
		return false;

	int nth = int(r * (float(nSelect)-1.e-3f));
	
	int iNext = 0;
	int i;
	for( i = 0; i < ncmds; i++ )
	{
		if( !fExcluded.IsBitSet(i) )
		{
			if( !nth-- )
			{
				iNext = i;
				break;
			}
		}
	}
	fCurrent = iNext;

	return true;
}

void plRandomCommandMod::IStart() 
{ 
	if( !IStopped() )
		return;

	fState &= ~kStopped; 
	IPlayNextIfMaster();
}

void plRandomCommandMod::IStop() 
{ 
	fState |= kStopped; 
}

hsBool plRandomCommandMod::IStopped() const 
{ 
	return 0 != (fState & kStopped); 
}

void plRandomCommandMod::IPlayNextIfMaster()
{
	if( !fTarget )
		IRetry(2.f);
	
	if( fTarget->IsLocallyOwned() == plSynchedObject::kNo )		// if this object is a proxy, it should just wait for network cmds 
		return;

	if( IStopped() )
		return;

	IPlayNext();
}

hsBool plRandomCommandMod::MsgReceive(plMessage* msg)
{
	// plAnimCmdMsg - interpret start/stop appropriately.
	// could overinterpret set loop points to limit range of
	// cmds we use to a window of the total set.
	plAnimCmdMsg* anim = plAnimCmdMsg::ConvertNoRef(msg);
	if( anim )
	{
		if( anim->GetSender() != GetKey() )
		{
#if 0
			hsStatusMessageF("someone triggered me, remote=%d\n", 
				msg->HasBCastFlag(plMessage::kNetNonLocal));
#endif
			if( anim->Cmd(plAnimCmdMsg::kContinue) )
				IStart();
			if( anim->Cmd(plAnimCmdMsg::kStop) )
				IStop();
			if( anim->Cmd(plAnimCmdMsg::kToggleState) )
			{
				if( IStopped() )
					IStart();
				else
					IStop();
			}
		}
		else
		{
			
#if 0
			hsStatusMessageF("play next if master, remote=%d\n", 
				msg->HasBCastFlag(plMessage::kNetNonLocal));
#endif
			IPlayNextIfMaster();
		}
		return true;
	}

	// Don't understand, pass on to base class.
	return plSingleModifier::MsgReceive(msg);
}

void plRandomCommandMod::IReset()
{
	fCurrent = -1;
	fExcluded.Clear();

	if( !IStopped() )
		IRetry(0);
}

void plRandomCommandMod::Read(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Read(s, mgr);

	fMode = s->ReadByte();
	fState = s->ReadByte();

	fMinDelay = s->ReadSwapScalar();
	fMaxDelay = s->ReadSwapScalar();

	IReset();
}

void plRandomCommandMod::Write(hsStream* s, hsResMgr* mgr)
{
	plSingleModifier::Write(s, mgr);

	s->WriteByte(fMode);
	s->WriteByte(fState);

	s->WriteSwapScalar(fMinDelay);
	s->WriteSwapScalar(fMaxDelay);
}

void plRandomCommandMod::IRetry(hsScalar secs)
{
	IStop();

	double t = hsTimer::GetSysSeconds() + secs;

	plAnimCmdMsg* msg = TRACKED_NEW plAnimCmdMsg(nil, GetKey(), &t);
	msg->SetCmd(plAnimCmdMsg::kContinue);
	plgDispatch::MsgSend(msg);
}
