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

#include "plLayer.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsGDeviceRef.h"
#include "hsGMatState.inl"
#include "hsMatrix44.h"
#include "plPipeline.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plShader.h"

#include "pnMessage/plPipeResMakeMsg.h"

#include "plGImage/plBitmap.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plLayRefMsg.h"

plLayer::plLayer()
{
    fOwnedChannels = kTransform
                | kPreshadeColor
                | kRuntimeColor
                | kAmbientColor
                | kOpacity
                | kState
                | kUVWSrc
                | kLODBias
                | kSpecularColor
                | kSpecularPower
                | kTexture
                | kVertexShader
                | kPixelShader
                | kBumpEnvXfm;

    fTransform = new hsMatrix44;
    fTransform->Reset();

    fPreshadeColor = new hsColorRGBA;
    fRuntimeColor = new hsColorRGBA;
    fAmbientColor = new hsColorRGBA;
    fSpecularColor = new hsColorRGBA;
    fOpacity = new float;
    
    fState = new hsGMatState;
    fState->Reset();

    fUVWSrc = new uint32_t;
    fLODBias = new float;
    fSpecularPower = new float;

    fTexture = new plBitmap*;
    *fTexture = nullptr;

    fVertexShader = new plShader*;
    *fVertexShader = nullptr;

    fPixelShader = new plShader*;
    *fPixelShader = nullptr;

    fBumpEnvXfm = new hsMatrix44;
    fBumpEnvXfm->Reset();
}

plLayer::~plLayer()
{
}

uint32_t plLayer::Eval(double secs, uint32_t frame, uint32_t ignore) 
{ 
    return uint32_t(0); 
}

void plLayer::Read(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Read(s, mgr);

    fState->Read(s);

    fTransform->Read(s);
    fPreshadeColor->Read(s);
    fRuntimeColor->Read( s );
    fAmbientColor->Read(s);
    fSpecularColor->Read( s );

    *fUVWSrc = s->ReadLE32();
    *fOpacity = s->ReadLEFloat();
    *fLODBias = s->ReadLEFloat();
    *fSpecularPower = s->ReadLEFloat();

    plLayRefMsg* refMsg = new plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture); 
    mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef); 

#if 1 // For read/write shaders
    refMsg = new plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kVertexShader); 
    mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef);

    refMsg = new plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kPixelShader);  
    mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef);

    fBumpEnvXfm->Read(s);
#endif // For read/write shaders
}

void plLayer::Write(hsStream* s, hsResMgr* mgr)
{
    plLayerInterface::Write(s, mgr);

    fState->Write(s);

    fTransform->Write(s);
    fPreshadeColor->Write(s);
    fRuntimeColor->Write( s );
    fAmbientColor->Write(s);
    fSpecularColor->Write( s );
    
    s->WriteLE32(*fUVWSrc);
    s->WriteLEFloat(*fOpacity);
    s->WriteLEFloat(*fLODBias);
    s->WriteLEFloat(*fSpecularPower);


    mgr->WriteKey(s, GetTexture());
    mgr->WriteKey(s, GetVertexShader());
    mgr->WriteKey(s, GetPixelShader());

    fBumpEnvXfm->Write(s);
}

bool plLayer::MsgReceive(plMessage* msg)
{
    plLayRefMsg* refMsg = plLayRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        switch( refMsg->fType )
        {
        case plLayRefMsg::kTexture:
            {
                if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                {
                    plBitmap *tex = plBitmap::ConvertNoRef(refMsg->GetRef());
                    *fTexture = tex;
                    if( tex )
                        plgDispatch::Dispatch()->RegisterForExactType(plPipeTexMakeMsg::Index(), GetKey());
                    else
                        plgDispatch::Dispatch()->UnRegisterForExactType(plPipeTexMakeMsg::Index(), GetKey());
                }
                else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                {
                    *fTexture = nullptr;
                    plgDispatch::Dispatch()->UnRegisterForExactType(plPipeTexMakeMsg::Index(), GetKey());
                }
            }
            return true;
        case plLayRefMsg::kVertexShader:
            {
                if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                {
                    plShader* shader = plShader::ConvertNoRef(refMsg->GetRef());
                    *fVertexShader = shader;
                }
                else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                {
                    *fVertexShader = nullptr;
                }
            }
            return true;
        case plLayRefMsg::kPixelShader:
            {
                if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                {
                    plShader* shader = plShader::ConvertNoRef(refMsg->GetRef());
                    *fPixelShader = shader;
                }
                else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                {
                    *fPixelShader = nullptr;
                }
            }
            return true;
        }
    }
    plPipeTexMakeMsg* texMake = plPipeTexMakeMsg::ConvertNoRef(msg);
    if( texMake )
    {
        texMake->Pipeline()->CheckTextureRef(this);
        return true;
    }
    return plLayerInterface::MsgReceive(msg);
}

void plLayer::SetState(const hsGMatState& s)
{
    *fState = s;
}

void plLayer::SetTransform(const hsMatrix44& xfm) 
{ 
    *fTransform = xfm; 
}

void plLayer::SetBumpEnvMatrix(const hsMatrix44& xfm)
{
    *fBumpEnvXfm = xfm;
}


plLayer& plLayer::InitToDefault()
{
    fState->Reset();

    *fTexture = nullptr;

    SetRuntimeColor(hsColorRGBA().Set(0.5f, 0.5f, 0.5f, 1.f));
    SetPreshadeColor(hsColorRGBA().Set(0.5f, 0.5f, 0.5f, 1.f));
    SetAmbientColor(hsColorRGBA().Set(0,0,0,1.f));
    SetOpacity(1.f);

    fTransform->Reset();

    SetUVWSrc(0);
    SetLODBias(-1.f);
    SetSpecularColor( hsColorRGBA().Set(0,0,0,1.f));
    SetSpecularPower(1.f);

    *fVertexShader = nullptr;
    *fPixelShader = nullptr;

    fBumpEnvXfm->Reset();

    return *this;
}


plLayerInterface* plLayer::DefaultLayer()
{
    static plLayer defLayer;
    defLayer.InitToDefault();
    return &defLayer;
}

//// CloneNoTexture ///////////////////////////////////////////////////////////
//  Copies all the fields from the original layer given, not including the 
//  texture

void plLayer::CloneNoTexture( plLayerInterface *original )
{
    SetBlendFlags( original->GetBlendFlags() );
    SetClampFlags( original->GetClampFlags() );
    SetShadeFlags( original->GetShadeFlags() );
    SetZFlags( original->GetZFlags() );
    SetMiscFlags( original->GetMiscFlags() );
    SetState( original->GetState() );

    SetPreshadeColor( original->GetPreshadeColor() );
    SetRuntimeColor( original->GetRuntimeColor() );
    SetAmbientColor( original->GetAmbientColor() );
    SetSpecularColor( original->GetSpecularColor() );
    SetOpacity( original->GetOpacity() );
    SetTransform( original->GetTransform() );
    SetUVWSrc( original->GetUVWSrc() );
    SetLODBias( original->GetLODBias() );
    SetSpecularPower( original->GetSpecularPower() );

    SetVertexShader( original->GetVertexShader() );
    SetPixelShader( original->GetPixelShader() );
    SetBumpEnvMatrix( original->GetBumpEnvMatrix() );
}

