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

#include "hsTypes.h"
#include "plLayer.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "hsMatrix44.h"
#include "hsGMatState.inl"
#include "../plMessage/plLayRefMsg.h"
#include "../plGImage/plBitmap.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "plShader.h"

#include "plPipeline.h"
#include "plgDispatch.h"
#include "../pnMessage/plPipeResMakeMsg.h"

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

	fTransform = TRACKED_NEW hsMatrix44;
	fTransform->Reset();

	fPreshadeColor = TRACKED_NEW hsColorRGBA;
	fRuntimeColor = TRACKED_NEW hsColorRGBA;
	fAmbientColor = TRACKED_NEW hsColorRGBA;
	fSpecularColor = TRACKED_NEW hsColorRGBA;
	fOpacity = TRACKED_NEW hsScalar;
	
	fState = TRACKED_NEW hsGMatState;
	fState->Reset();

	fUVWSrc = TRACKED_NEW UInt32;
	fLODBias = TRACKED_NEW hsScalar;
	fSpecularPower = TRACKED_NEW hsScalar;

	fTexture = TRACKED_NEW plBitmap*;
	*fTexture = nil;

	fVertexShader = TRACKED_NEW plShader*;
	*fVertexShader = nil;

	fPixelShader = TRACKED_NEW plShader*;
	*fPixelShader = nil;

	fBumpEnvXfm = TRACKED_NEW hsMatrix44;
	fBumpEnvXfm->Reset();
}

plLayer::~plLayer()
{
}

UInt32 plLayer::Eval(double secs, UInt32 frame, UInt32 ignore) 
{ 
	return UInt32(0); 
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

	*fUVWSrc = s->ReadSwap32();
	*fOpacity = s->ReadSwapScalar();
	*fLODBias = s->ReadSwapScalar();
	*fSpecularPower = s->ReadSwapScalar();

	plLayRefMsg* refMsg = TRACKED_NEW plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture);	
	mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef); 

#if 1 // For read/write shaders
	refMsg = TRACKED_NEW plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kVertexShader);	
	mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef);

	refMsg = TRACKED_NEW plLayRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kPixelShader);	
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
	
	s->WriteSwap32(*fUVWSrc);
	s->WriteSwapScalar(*fOpacity);
	s->WriteSwapScalar(*fLODBias);
	s->WriteSwapScalar(*fSpecularPower);


    mgr->WriteKey(s, GetTexture());
    mgr->WriteKey(s, GetVertexShader());
    mgr->WriteKey(s, GetPixelShader());

	fBumpEnvXfm->Write(s);
}

hsBool plLayer::MsgReceive(plMessage* msg)
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
					*fTexture = nil;
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
					*fVertexShader = nil;
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
					*fPixelShader = nil;
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

	*fTexture = nil;

	SetRuntimeColor(hsColorRGBA().Set(0.5f, 0.5f, 0.5f, 1.f));
	SetPreshadeColor(hsColorRGBA().Set(0.5f, 0.5f, 0.5f, 1.f));
	SetAmbientColor(hsColorRGBA().Set(0,0,0,1.f));
	SetOpacity(1.f);

	fTransform->Reset();

	SetUVWSrc(0);
	SetLODBias(-1.f);
	SetSpecularColor( hsColorRGBA().Set(0,0,0,1.f));
	SetSpecularPower(1.f);

	*fVertexShader = nil;
	*fPixelShader = nil;

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
//	Copies all the fields from the original layer given, not including the 
//	texture

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

