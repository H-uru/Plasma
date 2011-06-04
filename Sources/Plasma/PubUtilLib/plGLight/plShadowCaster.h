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

#ifndef plShadowCaster_inc
#define plShadowCaster_inc

#include "../pnModifier/plMultiModifier.h"
#include "hsBounds.h"
#include "hsTemplates.h"

class plDrawableSpans;
class plSpan;
class plMessage;
class hsStream;
class hsResMgr;
class plShadowMaster;
class plRenderMsg;

class plShadowCaster : public plMultiModifier
{
public:
	enum {
		kNone			= 0x0,
		kSelfShadow		= 0x1,
		kPerspective	= 0x2,
		kLimitRes		= 0x4
	};
	class DrawSpan
	{
	public:
		DrawSpan& Set(plDrawableSpans* dr, const plSpan* sp, UInt32 idx) { fDraw = (dr); fSpan = (sp); fIndex = (idx); return *this; }

		plDrawableSpans*	fDraw;
		const plSpan*		fSpan;
		UInt32				fIndex;
	};
protected:

	// Global state to just turn off the whole gig. Not just
	// debugging, we'll probably want a user option for this.
	static hsBool		fShadowCastDisabled;
	static hsBool		fCanShadowCast;


	// Properties really just to be read and written,
	// never expected to change. Anything that might be
	// triggered should go into plMultiModifier::fProps,
	// to be network synced.
	UInt8				fCastFlags;

	hsScalar			fBoost;
	hsScalar			fAttenScale;
	hsScalar			fBlurScale;

	// Casting attributes calculated each frame.
	hsScalar			fMaxOpacity;
	hsTArray<DrawSpan>	fSpans;

	friend plShadowMaster;

	void ICollectAllSpans();

	hsBool IOnRenderMsg(plRenderMsg* msg);

	friend class plDXPipeline;
	static void SetCanShadowCast(hsBool b) { fCanShadowCast = b; }
public:
	plShadowCaster();
	virtual ~plShadowCaster();

	CLASSNAME_REGISTER( plShadowCaster );
	GETINTERFACE_ANY( plShadowCaster, plMultiModifier );
	
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return true; }

	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	hsScalar MaxOpacity() const { return fMaxOpacity; }
	const hsTArray<DrawSpan>& Spans() const { return fSpans; }

	hsBool	GetSelfShadow() const { return 0 != (fCastFlags & kSelfShadow); }
	void	SetSelfShadow(hsBool on) { if(on) fCastFlags |= kSelfShadow; else fCastFlags &= ~kSelfShadow; }

	hsBool	GetPerspective() const { return 0 != (fCastFlags & kPerspective); }
	void	SetPerspective(hsBool on) { if(on) fCastFlags |= kPerspective; else fCastFlags &= ~kPerspective; }

	hsBool	GetLimitRes() const { return 0 != (fCastFlags & kLimitRes); }
	void	SetLimitRes(hsBool on) { if(on) fCastFlags |= kLimitRes; else fCastFlags &= ~kLimitRes; }

	hsScalar GetAttenScale() const { return fAttenScale; }
	void SetAttenScale(hsScalar s) { fAttenScale = s; }

	hsScalar GetBlurScale() const { return fBlurScale; }
	void SetBlurScale(hsScalar s) { fBlurScale = s; }

	hsScalar GetBoost() const { return fBoost; }
	void SetBoost(hsScalar s) { fBoost = s; }

	// These are usually handled internally, activating on read and deactivating
	// on destruct. Made public in case they need to be manually handled, like
	// on dynamic construction and use.
	void Deactivate() const;
	void Activate() const;

	static void DisableShadowCast(hsBool on=true) { fShadowCastDisabled = on; }
	static void EnableShadowCast(hsBool on=true) { fShadowCastDisabled = !on; }
	static void ToggleShadowCast() { fShadowCastDisabled = !fShadowCastDisabled; }
	static hsBool ShadowCastDisabled() { return !CanShadowCast() || fShadowCastDisabled; }

	static hsBool CanShadowCast() { return fCanShadowCast; }
};

typedef plShadowCaster::DrawSpan plShadowCastSpan;

#endif // plShadowCaster_inc
