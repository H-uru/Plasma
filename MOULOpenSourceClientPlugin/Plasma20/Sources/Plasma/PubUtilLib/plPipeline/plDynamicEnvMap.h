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

#ifndef plDynamicEnvMap_inc
#define plDynamicEnvMap_inc

#include "plCubicRenderTarget.h"
#include "../plScene/plRenderRequest.h"
#include "hsBitVector.h"

class plRenderRequestMsg;
class hsStream;
class plMessage;
class plVisRegion;
class plGenRefMsg;
class hsResMgr;
class plCameraModifier1;
class plSceneObject;
class plBitmap;
class plLayer;

class plDynamicEnvMap : public plCubicRenderTarget
{
public:
	enum {
		kRefVisSet,
		kRefRootNode,
	};
protected:

	plRenderRequest				fReqs[6];
	plRenderRequestMsg*			fReqMsgs[6];

	plSceneObject*				fRootNode;
	hsPoint3					fPos;
	hsScalar					fHither;
	hsScalar					fYon;
	hsScalar					fFogStart;
	hsColorRGBA					fColor;

	hsScalar					fRefreshRate;
	double						fLastRefresh;
	int							fLastRender;
	int							fOutStanding;

	hsBitVector					fVisSet;
	hsTArray<plVisRegion*>		fVisRegions;
	hsTArray<char *>			fVisRegionNames;
	hsBool						fIncCharacters;

	void	IUpdatePosition();
	hsBool	INeedReRender();

	void ISetupRenderRequests();
	void ISubmitRenderRequests();
	void ISubmitRenderRequest(int i);
	void ICheckForRefresh(double t, plPipeline *pipe);
	
	hsBool	IOnRefMsg(plGenRefMsg* refMsg);

public:
	plDynamicEnvMap();
	plDynamicEnvMap(UInt16 width, UInt16 height, UInt8 bitDepth, UInt8 zDepth = -1, UInt8 sDepth = -1);

	virtual ~plDynamicEnvMap();

	CLASSNAME_REGISTER( plDynamicEnvMap );
	GETINTERFACE_ANY( plDynamicEnvMap, plCubicRenderTarget );

	virtual void	Read(hsStream* s, hsResMgr* mgr);
	virtual void	Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	void ReRender();

	void Init();

	void		SetPosition(const hsPoint3& pos);
	void		SetHither(hsScalar f);
	void		SetYon(hsScalar f);
	void		SetFogStart(hsScalar f);
	void		SetColor(const hsColorRGBA& col);
	void		SetRefreshRate(hsScalar secs);

	hsPoint3	GetPosition() const;
	hsScalar	GetHither() const { return fHither; }
	hsScalar	GetYon() const { return fYon; }
	hsScalar	GetFogStart() const { return fFogStart; }
	hsColorRGBA	GetColor() const { return fColor; }
	hsScalar	GetRefreshRate() const { return 6.f * fRefreshRate; }

	void		AddVisRegion(plVisRegion* reg); // Will just send a ref

	void		SetIncludeCharacters(hsBool b);
	hsBool		GetIncludeCharacters() const { return fIncCharacters; }
	void		SetVisRegionName(char *name){ fVisRegionNames.Push(name); }
};

////////////////////////////////////////////////////////////////////////////
// Yes, it's lame that a lot of this code is nearly the same as
// plDynamicEnvMap, but this derives from plRenderTarget, not plCubicRenderTarget
// and I don't want to touch multiple inheritance.
class plDynamicCamMap : public plRenderTarget
{
public:
	enum 
	{
		kRefVisSet,
		kRefCamera,
		kRefRootNode,
		kRefTargetNode,
		kRefDisableTexture,
		kRefMatLayer,
	};

	hsScalar					fHither;
	hsScalar					fYon;
	hsScalar					fFogStart;
	hsColorRGBA					fColor;

protected:
	plRenderRequest				fReq;
	plRenderRequestMsg*			fReqMsg;

	hsScalar					fRefreshRate;
	double						fLastRefresh;
	int							fOutStanding;

	hsBitVector					fVisSet;
	hsTArray<plVisRegion*>		fVisRegions;
	hsTArray<char *>			fVisRegionNames;	// this allows us to specify vis-regions in other pages.	
	hsBool						fIncCharacters;
	plCameraModifier1*			fCamera;
	plSceneObject*				fRootNode;
	hsTArray<plSceneObject*>	fTargetNodes;

	// Extra info for swapping around textures when reflections are disabled.
	plBitmap*					fDisableTexture;
	hsTArray<plLayer*>			fMatLayers;
	static UInt8				fFlags;
	enum 
	{
		kReflectionCapable	= 0x01,
		kReflectionEnabled	= 0x02,
		kReflectionMask		= kReflectionCapable | kReflectionEnabled,
	};

	hsBool	INeedReRender();

	void ISetupRenderRequest(plPipeline *pipe);
	void ISubmitRenderRequest(plPipeline *pipe);
	void ICheckForRefresh(double t, plPipeline *pipe);
	void IPrepTextureLayers();

	hsBool	IOnRefMsg(plRefMsg* refMsg);

public:
	plDynamicCamMap();
	plDynamicCamMap(UInt16 width, UInt16 height, UInt8 bitDepth, UInt8 zDepth = -1, UInt8 sDepth = -1);

	virtual ~plDynamicCamMap();

	CLASSNAME_REGISTER( plDynamicCamMap );
	GETINTERFACE_ANY( plDynamicCamMap, plRenderTarget );

	virtual void	Read(hsStream* s, hsResMgr* mgr);
	virtual void	Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	void ReRender();
	void Init();

	void		SetIncludeCharacters(hsBool b);
	void		SetRefreshRate(hsScalar secs);
	void		AddVisRegion(plVisRegion* reg);
	void		SetVisRegionName(char *name){ fVisRegionNames.Push(name); }

	static hsBool	GetEnabled() { return (fFlags & kReflectionEnabled) != 0; }
	static void		SetEnabled(hsBool enable);
	static hsBool	GetCapable() { return (fFlags & kReflectionCapable) != 0; }
	static void		SetCapable(hsBool capable);
};

#endif // plDynamicEnvMap_inc
