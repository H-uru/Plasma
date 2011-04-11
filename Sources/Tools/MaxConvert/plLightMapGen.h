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

#ifndef plLightMapGen_inc
#define plLightMapGen_inc

#include "hsTemplates.h"

#include <vector>

class plMaxLightContext;
class plRenderGlobalContext;
class plLayerInterface;
class plMaxNode;
class hsGMaterial;
class plGeometrySpan;
class plMipmap;
class plBitmap;
struct hsColorRGBA;
struct hsPoint3;
struct hsVector3;
struct hsMatrix44;
class plErrorMsg;
class plConvertSettings;
class hsBounds3Ext;
class plLightMapComponent;

class plLightMapInfo
{
public:
	ObjLightDesc*		fObjLiDesc;
	INode*				fLiNode;
	int					fResetShadowType;
	float				fResetMapRange;
	float				fMapRange;
	hsBool				fNewRender;
};

class plLightMapGen
{
protected:
	Interface*					fInterface;
	TimeValue					fTime;

	int							fUVWSrc;
	float						fScale;

	float						fMapRange;
	
	int							fWidth;
	int							fHeight;

	bool						fRecalcLightMaps;

	Renderer*					fRenderer;
#ifdef MF_NEW_RGC
	RenderGlobalContext*		fRGC;
#else // MF_NEW_RGC
	plRenderGlobalContext*		fRGC;
#endif // MF_NEW_RGC
	RendParams*					fRP;
	
	hsTArray<plLightMapInfo>	fAllLights;
	hsTArray<plLightMapInfo*>	fActiveLights;

	mutable hsTArray<plLayerInterface*>	fCreatedLayers;
	mutable hsTArray<plMipmap*>			fPreppedMipmaps;

	mutable hsTArray<plBitmap*> fNewMaps;	// Mipmaps created this session (not loaded from disk)

	std::vector<plLightMapComponent*> fSharedComponents; // HACK so we can get rid of key refs before deleting bitmaps

	hsBounds3Ext	IGetBoundsLightSpace(INode* node, INode* liNode);
	hsBool			IDirAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node);
	hsBool			ISpotAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node);
	hsBool			IOmniAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node);
	hsBool			ILightAffectsNode(plLightMapInfo* liInfo, LightObject* liObj, INode* node);

	hsBool		IPrepLight(plLightMapInfo* liInfo, INode* node);
	hsBool		IGetLight(INode* node);
	hsBool		IFindLightsRecur(INode* node);
	hsBool		IFindActiveLights(plMaxNode* node);
	hsBool		IReleaseActiveLights();
	hsBool		IReleaseAllLights();

	int			IPowerOfTwo(int sz) const;
	hsBool		ISelectBitmapDimension(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, hsTArray<plGeometrySpan *> &spans);
	hsBool		ICompressLightMaps();
	
	hsBool		IsFresh(plBitmap* map) const;

	hsBool				IAddToLightMap(plLayerInterface* lay, plMipmap* src) const;
	plMipmap*			IMakeAccumBitmap(plLayerInterface* lay) const;
	void				IInitBitmapColor(plMipmap* bitmap, const hsColorRGBA& col) const;
	plLayerInterface*	IGetLightMapLayer(plMaxNode* node, plGeometrySpan& span);
	plLayerInterface*	IMakeLightMapLayer(plMaxNode* node, plGeometrySpan& span);
	int					IGetUVWSrc() const { return fUVWSrc; }

	UInt32		IShadePoint(plMaxLightContext& ctx, const Color& amb, const hsPoint3& p, const hsVector3& n);
	hsBool		IShadeVerts(plMaxLightContext& ctx, const Color& amb, const hsPoint3 pt[3], const hsVector3 norm[3], const hsPoint3 uv[3], plMipmap* bitmap);
	hsBool		IShadeFace(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, plGeometrySpan& span, int iFace, plMipmap* bitmap);
	hsBool		IShadeSpan(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, plGeometrySpan& spans);
	hsBool		IShadeGeometrySpans(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, hsTArray<plGeometrySpan *> &spans);

	hsBool		IWantsMaps(plMaxNode* node);
	hsBool		IValidateUVWSrc(hsTArray<plGeometrySpan *> &spans) const;

public:
	plLightMapGen();
	virtual ~plLightMapGen();

#ifdef MF_NEW_RGC
	void SetRGC(RenderGlobalContext* rgc); // Don't call this ever ever ever
#endif // MF_NEW_RGC

	hsBool		Open(Interface* ip, TimeValue t, bool forceRegen=true);
	hsBool		InitNode(INode* node, hsBool softShadow=true); // unnecessary when using MakeMaps()
	hsBool		Update(TimeValue t);

	void SetUVWSrc(int i) { fUVWSrc = i; }
	int GetUVWSrc() const { return fUVWSrc; }

	void SetScale(float f) { fScale = f; }
	float GetScale() const { return fScale; }

	// Calls to either the global or single must be wrapped in
	// a call to Open and a call to close. That is, you must first
	// call Open(), then you can make maps all day, but at the end
	// of the day, you need to call Close(). Also, if the scene
	// lighting changes, you need to call Close() and then Open() again.
	// With the possibility of lights getting deleted from the scene,
	// you're best off calling Open(), making as many maps as you want
	// for now, call Close(), and if you decide later you want more,
	// re-open. There's no protection in here from a user deleting
	// a light (or any other node) while the shader is Open. For your
	// own safety and the safety of your fellow passengers, don't
	// return control to the user until the system is Closed.
	hsBool		MakeMaps(plMaxNode* node, const hsMatrix44& l2w, const hsMatrix44& w2l, hsTArray<plGeometrySpan *>& spans, plErrorMsg *pErrMsg, plConvertSettings *settings);

	Color		ShadowPoint(plMaxLightContext& ctx);
	Color		ShadePoint(plMaxLightContext& ctx); // ctx already contains pos & norm
	Color		ShadePoint(plMaxLightContext& ctx, const Point3& p, const Point3& n);
	Color		ShadePoint(plMaxLightContext& ctx, const hsPoint3& p, const hsVector3& n);

	hsBool		DeInitNode();
	hsBool		Close();

	static plLightMapGen& Instance();
};

#endif // plLightMapGen_inc
