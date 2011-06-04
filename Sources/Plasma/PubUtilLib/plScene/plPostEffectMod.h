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

#ifndef plPostEffectMod_inc
#define plPostEffectMod_inc

#include "../pnModifier/plSingleModifier.h"

#include "hsMatrix44.h"
#include "hsBitVector.h"

class plSceneNode;
class plPageTreeMgr;
class plMessage;
class plRenderTarget;
class plRenderRequest;
class plViewTransform;

class plPostEffectMod : public plSingleModifier
{
public:
	enum plPostEffectModStates {
		kEnabled = 0
	};

	enum {
		kNodeRef = 0x0
	};
protected:

	hsBitVector				fState;

	hsScalar				fHither;
	hsScalar				fYon;

	hsScalar				fFovX;
	hsScalar				fFovY;

	plKey					fNodeKey;
	plPageTreeMgr*			fPageMgr;

	plRenderTarget*			fRenderTarget;
	plRenderRequest*		fRenderRequest;

	hsMatrix44				fDefaultW2C, fDefaultC2W;


	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty); // called only by owner object's Eval()

	void			ISetupRenderRequest();
	void			IDestroyRenderRequest();
	void			IUpdateRenderRequest();

	void			IRegisterForRenderMsg(hsBool on);
	void			ISubmitRequest();

	void			IAddToPageMgr(plSceneNode* node);
	void			IRemoveFromPageMgr(plSceneNode* node);

	void			ISetEnable(hsBool on);
	hsBool			IIsEnabled() const;

public:
	plPostEffectMod();
	virtual ~plPostEffectMod();

	CLASSNAME_REGISTER( plPostEffectMod );
	GETINTERFACE_ANY( plPostEffectMod, plSingleModifier );


	virtual hsBool	MsgReceive(plMessage* pMsg);
	
	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	void		GetDefaultWorldToCamera( hsMatrix44 &w2c, hsMatrix44 &c2w );

	// Export only
	void		SetNodeKey(plKey key) { fNodeKey = key; }
	plKey		GetNodeKey() const { return fNodeKey; }

	void		SetHither(hsScalar h) { fHither = h; }
	void		SetYon(hsScalar y) { fYon = y; }
	void		SetFovX(hsScalar f) { fFovX = f; }
	void		SetFovY(hsScalar f) { fFovY = f; }

	hsScalar	GetHither() const { return fHither; }
	hsScalar	GetYon() const { return fYon; }
	hsScalar	GetFovX() const { return fFovX; }
	hsScalar	GetFovY() const { return fFovY; }

	plPageTreeMgr* GetPageMgr() const { return fPageMgr; }

	const plViewTransform& GetViewTransform();

	// If translating from a scene object, send WorldToLocal() and LocalToWorld(), in that order
	void		SetWorldToCamera( hsMatrix44 &w2c, hsMatrix44 &c2w );

	// Very bad
	void		EnableLightsOnRenderRequest( void );
};

#endif // plPostEffectMod_inc
