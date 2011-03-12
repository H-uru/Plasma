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
#ifndef plProfileManager_h_inc
#define plProfileManager_h_inc

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "plProfile.h"

class plProfileManager 
{
protected:
	friend class plProfileManagerFull;

	typedef std::vector<plProfileVar*> VarVec;
	VarVec fVars;

	double fLastAvgTime;

	UInt32 fProcessorSpeed;

	plProfileManager();

public:
	~plProfileManager();

	static plProfileManager& Instance();

	void AddTimer(plProfileVar* var);	// Called by plProfileVar

	void BeginFrame();	// Call begin frame on all timers
	void EndFrame();	// Call end frame on all timers

	void SetAvgTime(UInt32 avgMS);

	UInt32 GetProcessorSpeed() { return fProcessorSpeed; }

	// Backdoor for hack timers in calculated profiles
	static UInt32 GetTime();
};

class plProfileLaps
{
protected:
	class LapInfo : public plProfileBase
	{
	protected:

	public:
		bool fUsedThisFrame;
		LapInfo(const char* name) { fName = name; fDisplayFlags = kDisplayTime; }
		bool operator<(const LapInfo& rhs) const { return fLastAvg < rhs.fLastAvg; }

		void BeginTiming(UInt32 value) { fValue -= value; }
		void EndTiming(UInt32 value) { fValue += value; fTimerSamples++; }
	};
	std::vector<LapInfo> fLapTimes;

	LapInfo* IFindLap(const char* lapName);

public:
	void BeginLap(UInt32 curValue, const char* name);
	void EndLap(UInt32 curValue, const char* name);

	void BeginFrame();
	void EndFrame();
	void UpdateAvgs();

	int GetNumLaps();
	plProfileBase* GetLap(int i);
};

#endif // plProfileManager_h_inc
