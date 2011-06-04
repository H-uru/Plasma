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

#ifndef plShadowSlave_inc
#define plShadowSlave_inc

#include "hsMatrix44.h"
#include "hsBounds.h"
#include "plViewTransform.h"
#include "hsGeometry3.h"

class plShadowCaster;
class plVolumeIsect;
class plPipeline;

class plShadowSlave
{
public:

	UInt32				fIndex;

	hsMatrix44			fWorldToLight;
	hsMatrix44			fLightToWorld;

	hsPoint3			fLightPos;
	hsVector3			fLightDir;

	hsMatrix44			fWorldToTexture;
	hsMatrix44			fCastLUT;
	hsMatrix44			fRcvLUT;

	plViewTransform		fView;

	hsScalar			fPower;
	hsScalar			fBlurScale;

	hsBounds3Ext		fCasterWorldBounds;
	hsBounds3Ext		fWorldBounds;

	plVolumeIsect*		fIsect;

	UInt32				fWidth;
	UInt32				fHeight;

	hsScalar			fAttenDist;

	hsScalar			fPriority;

	UInt32				fFlags;

	enum SlaveFlag
	{
		kObeysLightGroups		= 0x1,
		kIncludesChars			= 0x2,
		kSelfShadow				= 0x4,
		kCastInCameraSpace		= 0x8,
		kReverseZ				= 0x10,
		kTwoSided				= 0x20,
		kReverseCull			= 0x40,
		kPositional				= 0x80
	};

	void SetFlag(SlaveFlag f, hsBool on) { if(on) fFlags |= f; else fFlags &= ~f; }
	hsBool HasFlag(SlaveFlag f) const { return 0 != (fFlags & f); }

	hsBool ObeysLightGroups() const { return HasFlag(kObeysLightGroups); }
	hsBool IncludesChars() const { return HasFlag(kIncludesChars); }
	hsBool SelfShadow() const { return HasFlag(kSelfShadow); }
	hsBool CastInCameraSpace() const { return HasFlag(kCastInCameraSpace); }
	hsBool ReverseZ() const { return HasFlag(kReverseZ); }
	hsBool TwoSided() const { return HasFlag(kTwoSided); }
	hsBool ReverseCull() const { return HasFlag(kReverseCull); }
	hsBool Positional() const { return HasFlag(kPositional); }

	virtual void Init() { fFlags = 0; }

	const plShadowCaster*		fCaster;

	// Internal to pipeline, no touch!!!
	int							fLightIndex;
	int							fLightRefIdx;
	int							fSelfShadowOn;
	void*						fPipeData; 

	bool				ISetupOrthoViewTransform();
	bool				ISetupPerspViewTransform();

	virtual bool		SetupViewTransform(plPipeline* pipe) = 0;
};

class plDirectShadowSlave : public plShadowSlave
{
	virtual bool		SetupViewTransform(plPipeline* pipe) { return ISetupOrthoViewTransform(); }
};

class plPointShadowSlave : public plShadowSlave
{
	virtual bool		SetupViewTransform(plPipeline* pipe) { return ISetupPerspViewTransform(); }
};

#endif // plShadowSlave_inc
