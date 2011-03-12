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

#ifndef plLayer_inc
#define plLayer_inc

#include "hsTemplates.h"
#include "plLayerInterface.h"

class plLayer : public plLayerInterface
{
protected:
public:
	plLayer();
	virtual ~plLayer();

	CLASSNAME_REGISTER( plLayer );
	GETINTERFACE_ANY( plLayer, plLayerInterface );

	virtual UInt32			Eval(double secs, UInt32 frame, UInt32 ignore);

	virtual void			Read(hsStream* s, hsResMgr* mgr);
	virtual void			Write(hsStream* s, hsResMgr* mgr);

	virtual hsBool			MsgReceive(plMessage* msg);

	// Flat layer specifics
	plLayer& InitToDefault();

	void SetBlendFlags(UInt32 f) { fState->fBlendFlags = f; }
	void SetClampFlags(UInt32 f) { fState->fClampFlags = f; }
	void SetShadeFlags(UInt32 f) { fState->fShadeFlags = f; }
	void SetZFlags(UInt32 f) { fState->fZFlags = f; }
	void SetMiscFlags(UInt32 f) { fState->fMiscFlags = f; }
	void SetState(const hsGMatState& state);

	void SetTexture(plBitmap* t) { *fTexture = t; }

	void SetPreshadeColor(const hsColorRGBA& col) { *fPreshadeColor = col; }
	void SetRuntimeColor( const hsColorRGBA& col ) { *fRuntimeColor = col; }
	void SetAmbientColor(const hsColorRGBA& col) { *fAmbientColor = col; }
	void SetSpecularColor(const hsColorRGBA& col) { *fSpecularColor = col; }
	void SetOpacity(hsScalar a) { *fOpacity = a; }
	void SetTransform(const hsMatrix44& xfm);
	void SetUVWSrc(UInt32 chan) { *fUVWSrc = chan; }
	void SetLODBias(hsScalar f) { *fLODBias = f; }
	void SetSpecularPower(hsScalar f) { *fSpecularPower = f; }

	void SetVertexShader(plShader* shader) { *fVertexShader = shader; }
	void SetPixelShader(plShader* shader) { *fPixelShader = shader; }

	void SetBumpEnvMatrix(const hsMatrix44& xfm);

	static plLayerInterface* DefaultLayer();

	// Copies all the fields from the original layer given, not including the texture
	void CloneNoTexture( plLayerInterface *original );
};

#endif // plLayerInterfaceStack_inc
