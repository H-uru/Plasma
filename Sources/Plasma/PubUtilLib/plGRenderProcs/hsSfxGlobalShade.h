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


#ifndef hsSfxGlobalShade_inc
#define hsSfxGlobalShade_inc

#include "hsBiExpander.h"
#include "hsGRenderProcs.h"
#include "hsColorRGBA.h"
#include "hsGeometry3.h"

class hsGLayer;

class hsSfxGlobalShade : public hsGRenderProcs {
public:
	enum {
		kNone				= 0x0,
		kFromFog			= 0x1,
		kFromClear			= 0x2,
		kFromLights			= 0x4,
		kSourceMask			= kFromFog | kFromClear | kFromLights,
		kScalarIntensity	= 0x8,
		kAffectDiffuse		= 0x10
	};
protected:

	// Constants from which to work.
	UInt32					fGSFlags;

	hsColorRGBA				fAmbient;
	hsColorRGBA				fDiffuse;

	// Calculated each invocation.
	hsColorRGBA				fIntensity;
	
	hsGLayer*				fCurrentLayer;
	hsColorRGBA				fRestoreColor;

	void					ISetIntensity(hsPoint3& pos);
	void					ISetFromFog(hsPoint3& pos);
	void					ISetFromClear(hsPoint3& pos);
	void					ISetFromLights(hsPoint3& pos);
	hsColorRGBA				ISumLights(hsPoint3& pos);
public:
	hsSfxGlobalShade();
	virtual ~hsSfxGlobalShade();

	virtual void ProcessPreInterpShadeVerts(hsExpander<hsGShadeVertex*>& vList);

	virtual hsBool32 BeginObject(plPipeline* pipe, plDrawable* obj);
	virtual void EndObject();

	virtual void Read(hsStream* s);
	virtual void Write(hsStream* s);

	virtual const char* GetLabel() const { return "hsSfxGlobalShade"; }

	virtual ProcType GetType() const { return kTypeGlobalShade; }

	void SetAmbient(const hsColorRGBA& col) { fAmbient = col; }
	hsColorRGBA GetAmbient() const { return fAmbient; }

	void SetDiffuse(const hsColorRGBA& col) { fDiffuse = col; }
	hsColorRGBA GetDiffuse() const { return fDiffuse; }

	void SetSource(UInt32 f) { fGSFlags &= ~kSourceMask; fGSFlags |= f; }
	UInt32 GetSource() { return fGSFlags & kSourceMask; }

	void SetScalar(hsBool32 on) { if(on)fGSFlags |= kScalarIntensity; else fGSFlags &= ~kScalarIntensity; }
	hsBool32 GetScalar() { return 0 != (fGSFlags & kScalarIntensity); }

	void SetAffectDiffuse(hsBool32 on) { if(on)fGSFlags |= kAffectDiffuse; else fGSFlags &= ~kAffectDiffuse; }
	hsBool32 GetAffectDiffuse() { return 0 != (fGSFlags & kAffectDiffuse); }

	CLASSNAME_REGISTER( hsSfxGlobalShade );
	GETINTERFACE_ANY( hsSfxGlobalShade, hsGRenderProcs );

};

#endif // hsSfxGlobalShade_inc
