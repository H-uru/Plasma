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

#ifndef plRandomCommandMod_inc
#define plRandomCommandMod_inc

#include "../pnModifier/plSingleModifier.h"
#include "hsTemplates.h"

class plRandomCommandMod : public plSingleModifier
{
public:
	enum {
		kNormal				= 0x0,	// randomly select the next
		kNoRepeats			= 0x1,	// random, but no cmd twice in a row
		kCoverall			= 0x2,	// random, but no cmd played twice till all cmds played
		kOneCycle			= 0x4,	// after playing through all cmds, stop
		kOneCmd				= 0x8,	// after playing a random cmd, stop until started again.
		kDelayFromEnd		= 0x10,
		kSequential			= 0x20
	};

	enum {
		kStopped			= 0x1
	};
protected:

	// These are only lightly synched, the only synched state is whether
	// they are currently active.
	UInt8							fState;

	hsBitVector						fExcluded;
	Int8							fCurrent;
	UInt8							fMode; // static, if it becomes dynamic, move to SynchedValue
	hsTArray<double>				fEndTimes;

	hsScalar						fMinDelay;
	hsScalar						fMaxDelay;
	
	void			IStart();
	virtual void	IStop();
	hsBool			IStopped() const;
	void			IRetry(hsScalar secs);
	virtual void	IPlayNextIfMaster();

	void			IReset();
	
	hsScalar		IGetDelay(hsScalar len) const;		
	
	int				IExcludeSelections(int ncmds);
	hsBool			ISelectNext(int nAnim); // return false if we should stop, else set fCurrent to next index

	// Once fCurrent is set to the next animation index to play, 
	// IPlayNext() does whatever it takes to actually play it.
	virtual void		IPlayNext() = 0;

	// We only act in response to messages.
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

public:
	plRandomCommandMod();
	~plRandomCommandMod();

	CLASSNAME_REGISTER( plRandomCommandMod );
	GETINTERFACE_ANY( plRandomCommandMod, plSingleModifier );

	virtual hsBool	MsgReceive(plMessage* pMsg);
	
	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	// Export only
	void	SetMode(UInt8 m) { fMode = m; }
	UInt8	GetMode() const { return fMode; }

	void	SetState(UInt8 s) { fState = s; }
	UInt8	GetState() const { return fState; }

	void		SetMinDelay(hsScalar f) { fMinDelay = f; }
	hsScalar	GetMinDelay() const { return fMinDelay; }

	void		SetMaxDelay(hsScalar f) { fMaxDelay = f; }
	hsScalar	GetMaxDelay() const { return fMaxDelay; }
};


#endif // plRandomCommandMod_inc

