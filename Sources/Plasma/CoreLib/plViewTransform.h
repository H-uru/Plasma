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
#ifndef plViewTransform_inc
#define plViewTransform_inc

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsPoint2.h"

class hsBounds3;
class hsStream;

// There's a lot here, but there's a lot one might want to do with view transforms.
// It's easiest to grab the structure thinking of it in terms of the different
// spaces you might want a point in. The ones supported here are:
//		Screen - this is actual pixel values
//		NDC - Normalized Device Coordinates, these are post W divide, so the
//			valid range is x = [-1..1], y = [-1..1], z = [0..1]
//		Camera - relative to the camera, with (0,0,-1) directly in front of the camera,
//			and (0, 1, 0) directly above the camera.
//		World - Universal world space.
//		Map - arbitrary mapping of NDC. Like from [(-1,-1,0)..(1,1,1)] => [(0,0,0)..(1,1,1)] (default).
//	Note that there is no object space here. There could be, but I wanted something more constant, more
//		world independent, so the ViewTransform remains constant unless the view changes. Whatever.
//
//	So we're broken into functional sections:
//	1) Queries on the state of this view transform, properties, matrix values, whatever. Note that you
//		generally shouldn't be reading a value (e.g. matrix) out of the ViewTransform, but let the
//		ViewTransform perform the operation you would with the matrix.
//	2) Setting state, properties, matrix values, whatever. There's a couple of really bizarre variants
//		(like the union and intersection of view frustums). Full support is available for perspective
//		or orthogonal views. An additional capability (not necessary) is offset transforms, useful
//		for rendering textures. If you don't what good they are, they probably aren't any good to you.
//	3) Conversions of points from one space to another. You may notice that there are a whole lot of them.
//		There is a conversion from each of the spaces above to each of the other spaces. That's 12
//		transformations right there. But Points and Vectors actually transform differently, so there
//		are different versions for those. Where they could be treated the same, there is an hsScalarTriple
//		version that does the actual work, and then casting versions to come and go from the right type.
//	4) Read and write (note these are NOT virtual).
//
//	More Notes:
//		This class has no virtual functions.
//		You must set the width and height for Screen queries to work (duh!).
//		ViewPort defaults to cover the entire width and height. Viewport only affects mapping, not clipping
//			(i.e. reducing the viewport width will still render the same stuff, just skinnier).
//		The actual data here is very small, this is mostly a collection of functions, so where possible,
//			just keep one of these to pass around, (e.g. rather than keeping track of FOV etc and passing
//			those around).
//
class plViewTransform
{
public:
	plViewTransform();
	~plViewTransform() {}

	void				Reset(); // resets to default state

	// Queries
	hsBool				GetOrthogonal() const { return IHasFlag(kOrthogonal); }
	hsBool				GetPerspective() const { return !GetOrthogonal(); }
	hsBool				GetViewPortRelative() const { return IHasFlag(kViewPortRelative); }

	// Next, all our matrices.
	const hsMatrix44&	GetCameraToWorld() const { return fCameraToWorld; }
	const hsMatrix44&	GetWorldToCamera() const { return fWorldToCamera; }
	const hsMatrix44&	GetCameraToNDC() const { return ICheckCameraToNDC(); }
	const hsMatrix44&	GetWorldToNDC() const { return ICheckWorldToNDC(); }

	hsPoint3			GetPosition() const { return GetCameraToWorld().GetTranslate(); }
	hsVector3			GetDirection() const { return *((hsVector3 *)&GetWorldToCamera().fMap[2]); }
	hsVector3			GetUp() const { return *((hsVector3*)&GetWorldToCamera().fMap[1]); }
	hsVector3			GetAcross() const { return *((hsVector3*)&GetWorldToCamera().fMap[0]); }

	UInt16				GetScreenWidth() const { return fWidth; }
	UInt16				GetScreenHeight() const { return fHeight; }

	void				GetViewPort(hsPoint2& mins, hsPoint2& maxs) const;
	void				GetViewPort(int& loX, int& loY, int& hiX, int& hiY) const;
	int					GetViewPortWidth() const { return GetViewPortRight() - GetViewPortLeft(); }
	int					GetViewPortHeight() const { return GetViewPortBottom() - GetViewPortTop(); }
	int					GetViewPortLeft() const { return int(GetViewPortLoX()); }
	int					GetViewPortTop() const { return int(GetViewPortLoY()); }
	int					GetViewPortRight() const { return int(GetViewPortHiX()); }
	int					GetViewPortBottom() const { return int(GetViewPortHiY()); }
	float				GetViewPortLoX() const { return GetViewPortRelative() ? fViewPortX.fX * fWidth : fViewPortX.fX; }
	float				GetViewPortLoY() const { return GetViewPortRelative() ? fViewPortY.fX * fHeight : fViewPortY.fX; }
	float				GetViewPortHiX() const { return GetViewPortRelative() ? fViewPortX.fY * fWidth : fViewPortX.fY; }
	float				GetViewPortHiY() const { return GetViewPortRelative() ? fViewPortY.fY * fHeight : fViewPortY.fY; }

	hsPoint3			GetMapMin() const { return fMapMin; }
	hsPoint3			GetMapMax() const { return fMapMax; }
	void				GetMapping(hsPoint3& mapMin, hsPoint3& mapMax) const { mapMin = fMapMin; mapMax = fMapMax; }

	hsScalar			GetFovX() const;
	hsScalar			GetFovY() const;
	hsScalar			GetFovXDeg() const { return hsScalarRadToDeg(GetFovX()); }
	hsScalar			GetFovYDeg() const { return hsScalarRadToDeg(GetFovY()); }
	hsScalar			GetOrthoWidth() const { return fMax.fX - fMin.fX; }
	hsScalar			GetOrthoHeight() const { return fMax.fY - fMin.fY; }
	hsScalar			GetHither() const { return fMin.fZ; }
	hsScalar			GetYon() const { return fMax.fZ; }
	void				GetDepth(hsScalar& hither, hsScalar& yon) const { hither = GetHither(); yon = GetYon(); }

	// Setup.
	// First, our world to camera and back again.
	void				SetCameraTransform(const hsMatrix44& w2c, const hsMatrix44& c2w) { fWorldToCamera = w2c; fCameraToWorld = c2w; ISetFlag(kWorldToNDCSet, false); }

	// Next, what kind of projection.
	void				SetOrthogonal(hsBool on) { ISetFlag(kOrthogonal, on); InvalidateTransforms(); }
	void				SetPerspective(hsBool on) { SetOrthogonal(!on); }

	// Next, setting the scree/window/rendertarget size
	void				SetWidth(UInt16 w) { fWidth = w; }
	void				SetHeight(UInt16 h) { fHeight = h; }
	void				SetScreenSize(UInt16 w, UInt16 h) { SetWidth(w); SetHeight(h); }

	// Next, setting the viewport. You only need to do this if you want to use the screen functions above.
	// If you're passing in and getting out normalized device coordinates, skip this. If you don't set viewport,
	// Defaults to 0,0,width,height (i.e. the whole screen).
	void				SetViewPort(const hsPoint2& mins, const hsPoint2& maxs, hsBool relative=true);
	void				SetViewPort(float loX, float loY, float hiX, float hiY, hsBool relative=true) { SetViewPort(hsPoint2().Set(loX, loY), hsPoint2().Set(hiX, hiY), relative); }
	void				SetViewPort(UInt16 left, UInt16 top, UInt16 right, UInt16 bottom) { SetViewPort(hsScalar(left), hsScalar(top), hsScalar(right), hsScalar(bottom), false); }

	void				SetMapping(const hsPoint3& mins, const hsPoint3& maxs) { SetMapMin(mins); SetMapMax(maxs); }
	void				SetMapMin(const hsPoint3& mins) { fMapMin = mins; }
	void				SetMapMax(const hsPoint3& maxs) { fMapMax = maxs; }

	// Next, variants on setting up our projection matrix.
	// Depth is pretty uniform.
	void				SetDepth(hsScalar hither, hsScalar yon) { fMin.fZ = hither; fMax.fZ = yon; InvalidateTransforms(); }
	void				SetDepth(const hsPoint2& d) { SetDepth(d.fX, d.fY); }
	void				SetHither(hsScalar hither) { fMin.fZ = hither; InvalidateTransforms(); }
	void				SetYon(hsScalar yon) { fMax.fZ = yon; InvalidateTransforms(); }

	// Garden variety symmetric fov uses either of this first batch. Unless you're doing some funky projection, you don't even
	// need to look through the rest.
	// Degrees - all are full angles, < 180 degrees
	void				SetFovDeg(const hsPoint2& deg) { SetFovDeg(deg.fX, deg.fY); }
	void				SetFovDeg(hsScalar degX, hsScalar degY) { SetFovXDeg(degX); SetFovYDeg(degY); }
	void				SetFovXDeg(hsScalar deg) { SetFovX(hsScalarDegToRad(deg)); }
	void				SetFovYDeg(hsScalar deg) { SetFovY(hsScalarDegToRad(deg)); }

	// Radians - all are full angles, < PI
	void				SetFov(const hsPoint2& rad) { SetFov(rad.fX, rad.fY); }
	void				SetFov(hsScalar radX, hsScalar radY) { SetFovX(radX); SetFovY(radY); }
	void				SetFovX(hsScalar rad) { SetHalfWidth(hsTan(rad * 0.5f)); }
	void				SetFovY(hsScalar rad) { SetHalfHeight(hsTan(rad * 0.5f)); }

	// For orthogonal projection, don't call SetWidth(hsTan(fovRads)), because hsTan(f)/2 != hsTan(f/2)
	// For non-centered, call SetWidths/Heights() directly.
	void				SetWidth(hsScalar w) { SetHalfWidth(w * 0.5f); }
	void				SetHeight(hsScalar h) { SetHalfHeight(h * 0.5f); }

	// The rest do no interpretation, just stuff the values passed in.
	void				SetHalfWidth(hsScalar hw) { SetWidths(-hw, hw); }
	void				SetHalfHeight(hsScalar hh) { SetHeights(-hh, hh); }
	void				SetWidths(hsScalar minW, hsScalar maxW) { fMin.fX = minW; fMax.fX = maxW; InvalidateTransforms(); }
	void				SetHeights(hsScalar minH, hsScalar maxH) { fMin.fY = minH; fMax.fY = maxH; InvalidateTransforms(); }
	void				SetWidths(const hsPoint2& w) { SetWidths(w.fX, w.fY); }
	void				SetHeights(const hsPoint2& h) { SetHeights(h.fX, h.fY); }
	void				SetView(const hsPoint3& mins, const hsPoint3& maxs) { fMax = maxs; fMin = mins; InvalidateTransforms(); }

	// Take a CAMERA SPACE bounding box and sets up the projection to just surround it.
	// Note this doesn't swivel the camera around to see the box, it offsets the projection.
	// Return false if there isn't a projection that will capture any of the bnd. This
	// can be from the bnd being behind the camera.
	hsBool				SetProjection(const hsBounds3& cBnd);
	hsBool				SetProjectionWorld(const hsBounds3& wBnd);

	// This lets you create insane projection matrices. Note that it won't change the answer on anything like
	// GetFov().
	void				PreMultCameraToNDC(const hsMatrix44& m) { fCameraToNDC = m * GetCameraToNDC(); }
	void				PostMultCameraToNDC(const hsMatrix44& m) { fCameraToNDC = GetCameraToNDC() * m; }
	void				Recalc() { InvalidateTransforms(); }

	// These do the obvious, constructing a single view that encompasses either the intersection or union
	// of what the two views will see. The boolean is performed in axis aligned camera space, which lines
	// up nicely with screen space. Note that this only makes sense for two ViewTransforms with identical
	// CameraToWorld's (which isn't checked).
	hsBool				Intersect(const plViewTransform& view);
	hsBool				Union(const plViewTransform& view);

	// Convenience to move from one space to another.
	// Screen means pixels - Default is mapping NDC -> [0..1]. Z value of pixel is NDC Z.
	// NDC is the ([-1..1],[-1..1],[0..1]) Normalized device coordinates.
	// Camera is camera space.
	// World is world space.
	// Past that, you're on your own.
	hsScalarTriple		ScreenToNDC(const hsScalarTriple& scrP) const;
	hsScalarTriple		ScreenToCamera(const hsScalarTriple& scrP) const { return NDCToCamera(ScreenToNDC(scrP)); }

	hsPoint3			ScreenToNDC(const hsPoint3& scrP) const { return hsPoint3(ScreenToNDC(hsScalarTriple(scrP))); }
	hsPoint3			ScreenToCamera(const hsPoint3& scrP) const { return hsPoint3(ScreenToCamera(hsScalarTriple(scrP))); }
	hsPoint3			ScreenToWorld(const hsPoint3& scrP) const { return CameraToWorld(ScreenToCamera(scrP)); }

	hsVector3			ScreenToNDC(const hsVector3& scrV) const { return hsVector3(ScreenToNDC(hsScalarTriple(scrV))); }
	hsVector3			ScreenToCamera(const hsVector3& scrV) const { return hsVector3(ScreenToCamera(hsScalarTriple(scrV))); }
	hsVector3			ScreenToWorld(const hsVector3& scrV) const { return CameraToWorld(ScreenToCamera(scrV)); }

	hsScalarTriple		NDCToScreen(const hsScalarTriple& ndc) const;
	hsScalarTriple		NDCToCamera(const hsScalarTriple& ndc) const;

	hsPoint3			NDCToScreen(const hsPoint3& ndc) const { return hsPoint3(NDCToScreen(hsScalarTriple(ndc))); }
	hsPoint3			NDCToCamera(const hsPoint3& ndc) const { return hsPoint3(NDCToCamera(hsScalarTriple(ndc))); }
	hsPoint3			NDCToWorld(const hsPoint3& ndc) const { return CameraToWorld(NDCToCamera(ndc)); }

	hsVector3			NDCToScreen(const hsVector3& ndc) const { return hsVector3(NDCToScreen(hsScalarTriple(ndc))); }
	hsVector3			NDCToCamera(const hsVector3& ndc) const { return hsVector3(NDCToCamera(hsScalarTriple(ndc))); }
	hsVector3			NDCToWorld(const hsVector3& ndc) const { return CameraToWorld(NDCToCamera(ndc)); }

	hsScalarTriple		CameraToScreen(const hsScalarTriple& camP) const { return NDCToScreen(CameraToNDC(camP)); }
	hsScalarTriple		CameraToNDC(const hsScalarTriple& camP) const;

	hsPoint3			CameraToScreen(const hsPoint3& camP) const { return hsPoint3(CameraToScreen(hsScalarTriple(camP))); }
	hsPoint3			CameraToNDC(const hsPoint3& camP) const { return hsPoint3(CameraToNDC(hsScalarTriple(camP))); }
	hsPoint3			CameraToWorld(const hsPoint3& camP) const { return GetCameraToWorld() * camP; }

	hsVector3			CameraToScreen(const hsVector3& camP) const { return hsVector3(CameraToScreen(hsScalarTriple(camP))); }
	hsVector3			CameraToNDC(const hsVector3& camP) const { return hsVector3(CameraToNDC(hsScalarTriple(camP))); }
	hsVector3			CameraToWorld(const hsVector3& camV) const { return GetCameraToWorld() * camV; }

	hsPoint3			WorldToScreen(const hsPoint3& worldP) const { return (hsPoint3)CameraToScreen(WorldToCamera(worldP)); }
	hsPoint3			WorldToNDC(const hsPoint3& worldP) const { return CameraToNDC(WorldToCamera(worldP)); }
	hsPoint3			WorldToCamera(const hsPoint3& worldP) const { return GetWorldToCamera() * worldP; }

	hsVector3			WorldToScreen(const hsVector3& worldV) const { return (hsVector3)CameraToScreen(WorldToCamera(worldV)); }
	hsVector3			WorldToNDC(const hsVector3& worldP) const { return CameraToNDC(WorldToCamera(worldP)); }
	hsVector3			WorldToCamera(const hsVector3& worldV) const { return GetWorldToCamera() * worldV; }

	hsScalarTriple		NDCToMap(const hsScalarTriple& ndcP) const;
	hsScalarTriple		CameraToMap(const hsScalarTriple& camP) const { return NDCToMap(CameraToNDC(camP)); }

	hsPoint3			NDCToMap(const hsPoint3& ndcP) const { return hsPoint3(NDCToMap(hsScalarTriple(ndcP))); }
	hsPoint3			CameraToMap(const hsPoint3& camP) const { return hsPoint3(CameraToMap(hsScalarTriple(camP))); }
	hsPoint3			WorldToMap(const hsPoint3& worldP) const { return CameraToMap(WorldToCamera(worldP)); }

	hsVector3			NDCToMap(const hsVector3& ndcP) const { return hsVector3(NDCToMap(hsScalarTriple(ndcP))); }
	hsVector3			CameraToMap(const hsVector3& camP) const { return hsVector3(CameraToMap(hsScalarTriple(camP))); }
	hsVector3			WorldToMap(const hsVector3& worldP) const { return CameraToMap(WorldToCamera(worldP)); }

	void				Read(hsStream* s);
	void				Write(hsStream* s);

protected:
	enum
	{
		kNone				= 0x0,
		kOrthogonal			= 0x1,
		kSymmetric			= 0x2,
		kCameraToNDCSet		= 0x4,
		kWorldToNDCSet		= 0x8,
		kSetMask			= kCameraToNDCSet | kWorldToNDCSet,
		kViewPortRelative	= 0x10
	};

	mutable UInt32			fFlags;

	hsMatrix44				fCameraToWorld;
	hsMatrix44				fWorldToCamera;

	hsPoint3				fMin; // minTanX/X, minTanY/Y, hither
	hsPoint3				fMax; // maxTanX/X, maxTanY/Y, yon

	// Screen (or rendertarget) dimensions in pixels.
	UInt16					fWidth; 
	UInt16					fHeight;

	// Viewport can be stored as fraction of screen size, so the view transform's viewport
	// can be set up independent of the size of the window it's applied to. 
	hsPoint3		fViewPortX; // min, max, 1 / (max-min)
	hsPoint3		fViewPortY; // min, max, 1 / (max-min)

	// For arbitrary mapping (unconfined to pixel coords or NDC), just set what you want
	// to map to.
	hsPoint3		fMapMin;
	hsPoint3		fMapMax;

	// Some mutables. These are just the calculated from the above (e.g. fov, depth, perspective, etc).
	mutable hsMatrix44		fCameraToNDC;
	mutable hsMatrix44		fWorldToNDC;

	// Have to set a limit here on the smallest the hither plane can be.
	static const hsScalar	kMinHither;

	void				ISetCameraToNDC() const;
	hsBool				ICameraToNDCSet() const { return IHasFlag(kCameraToNDCSet); }
	const hsMatrix44&	ICheckCameraToNDC() const { if( !ICameraToNDCSet() ) ISetCameraToNDC(); return fCameraToNDC; }

	void				ISetWorldToNDC() const { fWorldToNDC = GetCameraToNDC() * fWorldToCamera; ISetFlag(kWorldToNDCSet); }
	hsBool				IWorldToNDCSet() const { return IHasFlag(kWorldToNDCSet); }
	const hsMatrix44&	ICheckWorldToNDC() const { if( !IWorldToNDCSet() ) ISetWorldToNDC(); return fWorldToNDC; }

	hsBool				IGetMaxMinsFromBnd(const hsBounds3& bnd, hsPoint3& mins, hsPoint3& maxs) const;

	void				InvalidateTransforms() { ISetFlag(kCameraToNDCSet|kWorldToNDCSet, false); }

	// Flags - generic
	hsBool				IHasFlag(UInt32 f) const { return 0 != (fFlags & f); }
	void				ISetFlag(UInt32 f, hsBool on=true) const { if(on) fFlags |= f; else fFlags &= ~f; }

};

#endif // plViewTransform_inc
