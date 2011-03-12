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

#ifndef plRenderRequest_inc
#define plRenderRequest_inc

#include "hsMatrix44.h"
#include "hsColorRGBA.h"
#include "plViewTransform.h"
#include "hsRefCnt.h"
#include "hsBitVector.h"

#include "../pnKeyedObject/plKey.h"
#include "../plMessage/plRenderRequestMsg.h"

class plRenderTarget;
class plPageTreeMgr;
class hsStream;
class hsResMgr;
class plDrawable;
class hsGMaterial;
class plPipeline;

class plRenderRequest : public plRenderRequestBase
{
public:
protected:
	UInt32					fRenderState; // Or'ed from plPipeline::RenderStateSettings::kRender*

	plDrawable*				fClearDrawable;
	plRenderTarget*			fRenderTarget;
	plPageTreeMgr*			fPageMgr;

	hsGMaterial*			fOverrideMat;
	hsGMaterial*			fEraseMat;

	plKey					fAck;

	hsScalar				fPriority;

	UInt32					fDrawableMask;
	UInt32					fSubDrawableMask;

	hsColorRGBA				fClearColor;
	hsScalar				fClearDepth;

	hsScalar				fFogStart;

	hsMatrix44				fLocalToWorld;
	hsMatrix44				fWorldToLocal;

	plViewTransform			fViewTransform;

	hsBitVector				fVisForce;

	UInt32					fUserData;
	hsBool					fIgnoreOccluders;

public:
	plRenderRequest();
	~plRenderRequest();

	hsBool			GetRenderSelect() const { return !fVisForce.Empty(); }
	hsBool			GetRenderCharacters() const;

	void			SetRenderState(UInt32 st) { fRenderState = st; }
	UInt32			GetRenderState() const { return fRenderState; }

	void			SetDrawableMask(UInt32 m) { fDrawableMask = m; }
	UInt32			GetDrawableMask() const { return fDrawableMask; }

	void			SetSubDrawableMask(UInt32 m) { fSubDrawableMask = m; }
	UInt32			GetSubDrawableMask() const { return fSubDrawableMask; }

	void			RequestAck(plKey key) { fAck = key; }
	plKey			GetAck() const { return fAck; }

	plDrawable*				GetClearDrawable() const { return fClearDrawable; }
	void					SetClearDrawable(plDrawable* d) { fClearDrawable = d; }

	hsGMaterial*			GetOverrideMat() const { return fOverrideMat; }
	void					SetOverrideMat(hsGMaterial* m) { fOverrideMat = m; }

	hsGMaterial*			GetEraseMat() const { return fEraseMat; }
	void					SetEraseMat(hsGMaterial* m) { fEraseMat = m; }

	plRenderTarget*			GetRenderTarget() const { return fRenderTarget; }
	void					SetRenderTarget(plRenderTarget* t);

	plPageTreeMgr*			GetPageTreeMgr() const { return fPageMgr; }
	void					SetPageTreeMgr(plPageTreeMgr* mgr) { fPageMgr = mgr; }

	const hsBitVector&		GetVisForce() const { return fVisForce; }
	void					SetVisForce(const hsBitVector& b);

	const hsMatrix44&	GetLocalToWorld() const { return fLocalToWorld; }
	const hsMatrix44&	GetWorldToLocal() const { return fWorldToLocal; }
	const hsMatrix44&	GetWorldToCamera() const { return fViewTransform.GetWorldToCamera(); }
	const hsMatrix44&	GetCameraToWorld() const { return fViewTransform.GetCameraToWorld(); }

	const plViewTransform&	GetViewTransform() const { return fViewTransform; }

	hsScalar GetHither() const { return fViewTransform.GetHither(); }
	hsScalar GetYon() const { return fViewTransform.GetYon(); }

	hsScalar GetFovX() const { return fViewTransform.GetFovXDeg(); }
	hsScalar GetFovY() const { return fViewTransform.GetFovYDeg(); }

	hsScalar GetSizeX() const { return fViewTransform.GetOrthoWidth(); }
	hsScalar GetSizeY() const { return fViewTransform.GetOrthoHeight(); }

	UInt16 GetScreenWidth() const { return fViewTransform.GetScreenWidth(); }
	UInt16 GetScreenHeight() const { return fViewTransform.GetScreenHeight(); }

	const hsColorRGBA& GetClearColor() const { return fClearColor; }
	hsScalar GetClearDepth() const { return fClearDepth; }
	// FogStart
	// negative => use current settings (default)
	// 0 => no fog == fog starts at yon
	// 1 => fog starts at camera.
	// Fog start greater than 1 is legal. Fog always linear.
	hsScalar GetFogStart() const { return fFogStart; }

	hsScalar GetPriority() const { return fPriority; }

	void SetLocalTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

	void SetViewTransform(const plViewTransform& v) { fViewTransform = v; }

	void SetCameraTransform(const hsMatrix44& w2c, const hsMatrix44& c2w) { fViewTransform.SetCameraTransform(w2c, c2w); }

	void SetPerspective(hsBool on=true) { fViewTransform.SetPerspective(on); }
	void SetOrthogonal(hsBool on=true) { fViewTransform.SetOrthogonal(on); }

	void SetHither(hsScalar f) { fViewTransform.SetHither(f); }
	void SetYon(hsScalar f) { fViewTransform.SetYon(f); }
	
	void SetFovX(hsScalar f) { fViewTransform.SetFovXDeg(f); }
	void SetFovY(hsScalar f) { fViewTransform.SetFovYDeg(f); }

	void SetSizeX(hsScalar f) { fViewTransform.SetWidth(f); }
	void SetSizeY(hsScalar f) { fViewTransform.SetHeight(f); }

	void SetClearColor(const hsColorRGBA& c) { fClearColor = c; }
	void SetClearDepth(hsScalar d) { fClearDepth = d; }
	// FogStart
	// negative => use current settings (default)
	// 0 => no fog == fog starts at yon
	// 1 => fog starts at camera.
	// Fog start greater than 1 is legal. Fog always linear.
	void SetFogStart(hsScalar d) { fFogStart = d; } 

	void SetPriority(hsScalar p) { fPriority = p; }

	virtual void Read(hsStream* s, hsResMgr* mgr);
	virtual void Write(hsStream* s, hsResMgr* mgr);

	void SetUserData(UInt32 n) { fUserData = n; }
	UInt32 GetUserData() const { return fUserData; }
	
	void SetIgnoreOccluders(hsBool b) { fIgnoreOccluders = b; }
	hsBool GetIgnoreOccluders() { return fIgnoreOccluders; }

	// This function is called after the render request is processed by the client
	virtual void	Render(plPipeline* pipe, plPageTreeMgr* pageMgr);
};

#endif // plRenderRequest_inc
