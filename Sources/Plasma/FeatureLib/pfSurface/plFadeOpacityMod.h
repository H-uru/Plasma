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

#ifndef plFadeOpacityMod_inc
#define plFadeOpacityMod_inc

#include "hsGeometry3.h"
#include "../pnModifier/plSingleModifier.h"
#include "hsTemplates.h"

class plPipeline;
class plRenderMsg;
class plFadeOpacityLay;

class plFadeOpacityMod : public plSingleModifier
{
public:
	enum {
		kBoundsCenter	= 1
	};

protected:

	enum {
		kRefFadeLay
	};

	// Input parameters
	hsScalar		fFadeUp;
	hsScalar		fFadeDown;

	// Internal fade state
	enum FadeState {
		kUp			= 0,
		kDown		= 1,
		kFadeUp		= 2,
		kFadeDown	= 3,
		kImmediate	= 4
	};
	hsScalar		fOpCurrent;

	double			fStart;
	FadeState		fFade;
	UInt8			fSetup;

	hsPoint3		fLastEye;

	// The target layers
	hsTArray<plFadeOpacityLay*> fFadeLays;

	// A global to turn the whole thing off for debug/perf
	static hsBool	fLOSCheckDisabled;

	void		IOnRenderMsg(plRenderMsg* rend);
	hsBool		IReady();
	hsBool		IShouldCheck(plPipeline* pipe);
	hsPoint3	IGetOurPos();
	void		ICalcOpacity();
	void		ISetOpacity();
	void		IFadeUp();
	void		IFadeDown();
	void		ISetup(plSceneObject* so);

	// We only act in response to messages.
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

public:
	plFadeOpacityMod();
	virtual ~plFadeOpacityMod();

	CLASSNAME_REGISTER( plFadeOpacityMod );
	GETINTERFACE_ANY( plFadeOpacityMod, plSingleModifier );

	virtual void			SetKey(plKey k);

	virtual hsBool			MsgReceive(plMessage* msg);

	virtual void			Read(hsStream* s, hsResMgr* mgr);
	virtual void			Write(hsStream* s, hsResMgr* mgr);

	virtual void			SetTarget(plSceneObject* so);

	void FadeUp();
	void FadeDown();
	void Fade(hsBool up) { if( up ) FadeUp(); else FadeDown(); }

	void SetFadeUp(hsScalar f) { fFadeUp = f; }
	hsScalar GetFadeUp() const { return fFadeUp; }

	void SetFadeDown(hsScalar f) { fFadeDown = f; }
	hsScalar GetFadeDown() const { return fFadeDown; }

	static hsBool GetLOSCheckDisabled() { return fLOSCheckDisabled; }
	static void SetLOSCheckDisabled(hsBool on) { fLOSCheckDisabled = on; }
};

#endif // plFadeOpacityMod_inc
