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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plLayer_inc
#define plLayer_inc

#include "plLayerInterface.h"

class plLayer : public plLayerInterface
{
protected:
public:
    plLayer();
    virtual ~plLayer();

    CLASSNAME_REGISTER( plLayer );
    GETINTERFACE_ANY( plLayer, plLayerInterface );

    uint32_t          Eval(double secs, uint32_t frame, uint32_t ignore) override;

    void            Read(hsStream* s, hsResMgr* mgr) override;
    void            Write(hsStream* s, hsResMgr* mgr) override;

    bool            MsgReceive(plMessage* msg) override;

    // Flat layer specifics
    plLayer& InitToDefault();

    void SetBlendFlags(uint32_t f) { fState->fBlendFlags = f; }
    void SetClampFlags(uint32_t f) { fState->fClampFlags = f; }
    void SetShadeFlags(uint32_t f) { fState->fShadeFlags = f; }
    void SetZFlags(uint32_t f) { fState->fZFlags = f; }
    void SetMiscFlags(uint32_t f) { fState->fMiscFlags = f; }
    void SetState(const hsGMatState& state);

    void SetTexture(plBitmap* t) { *fTexture = t; }

    void SetPreshadeColor(const hsColorRGBA& col) { *fPreshadeColor = col; }
    void SetRuntimeColor( const hsColorRGBA& col ) { *fRuntimeColor = col; }
    void SetAmbientColor(const hsColorRGBA& col) { *fAmbientColor = col; }
    void SetSpecularColor(const hsColorRGBA& col) { *fSpecularColor = col; }
    void SetOpacity(float a) { *fOpacity = a; }
    void SetTransform(const hsMatrix44& xfm);
    void SetUVWSrc(uint32_t chan) { *fUVWSrc = chan; }
    void SetLODBias(float f) { *fLODBias = f; }
    void SetSpecularPower(float f) { *fSpecularPower = f; }

    void SetVertexShader(plShader* shader) { *fVertexShader = shader; }
    void SetPixelShader(plShader* shader) { *fPixelShader = shader; }

    void SetBumpEnvMatrix(const hsMatrix44& xfm);

    static plLayerInterface* DefaultLayer();

    // Copies all the fields from the original layer given, not including the texture
    void CloneNoTexture( plLayerInterface *original );
};

#endif // plLayerInterfaceStack_inc
