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
#include "HeadSpin.h"
#ifdef BINK_SDK_AVAILABLE
#include <BINK.h>
#endif
#include <d3d9.h>
#include "plDXPipeline.h"
#include "plBinkPlayer.h"
#include "plDXBufferRefs.h"
#include "hsPoint2.h"
#include "pnMessage/plMessage.h"
#include "plResMgr/plLocalization.h"

#include "hsTimer.h"

static const D3DMATRIX ident = { 1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 1.0f };

static const uint16_t kMaxVolume = 32768;
const U32   kDefaultBack    = 0;
const U32   kDefaultFore    = 1;
const uint32_t kBackTrack = 0;
const uint32_t kForeTrack = 1;

const int fSndTrack(0); // STUB

U32 plBinkPlayer::fTracks[2] = { kDefaultBack, kDefaultFore };
hsBool plBinkPlayer::fInit = false;


plBinkPlayer::plBinkPlayer() : 
fBink(nil), 
fFileName(nil), 
fPipeline(nil), 
fTexture(nil), 
fNumPrimitives(2)
{
    SetDefaults();

    memset(fVerts, 0, kNumVerts*sizeof(D3DVertex));
}

plBinkPlayer::~plBinkPlayer()
{
    delete [] fFileName;
    fFileName = nil;
    Stop();
}

void plBinkPlayer::SetBackGroundTrack(uint32_t t)
{
    fTracks[kBackTrack] = t;
}

void plBinkPlayer::SetForeGroundTrack(uint32_t t)
{
    fTracks[kForeTrack] = t;
}

uint32_t plBinkPlayer::GetForeGroundTrack()
{
    return fTracks[kForeTrack];
}

uint32_t plBinkPlayer::GetBackGroundTrack()
{
    return fTracks[kBackTrack];
}


hsBool plBinkPlayer::Init( hsWindowHndl hWnd )
{
    fInit = true;

    switch (plLocalization::GetLanguage())
    {
    case plLocalization::kEnglish:  SetForeGroundTrack(1); break;
    case plLocalization::kFrench:   SetForeGroundTrack(2); break;
    case plLocalization::kGerman:   SetForeGroundTrack(3); break;
    //case plLocalization::kSpanish:    SetForeGroundTrack(4); break;
    //case plLocalization::kItalian:    SetForeGroundTrack(5); break;
    case plLocalization::kJapanese: SetForeGroundTrack(2); break;
    }

#ifdef BINK_SDK_AVAILABLE
    if(BinkSoundUseWaveOut())
    {
        return true;    
    }
    BinkSetSoundTrack(0, nil);
#endif

    return true;
}

hsBool plBinkPlayer::DeInit()
{
    return true;
}

void plBinkPlayer::SetDefaults()
{
    fTextureSize[0] = fTextureSize[1] = 0;
    fPos.Set(0.f, 0.f);
    fScale.Set(1.f, 1.f);
    fVolume[kBackTrack] = kMaxVolume;
    fVolume[kForeTrack] = kMaxVolume;
    fColor.Set(1.f, 1.f, 1.f, 1.f);
    fCurrColor = fColor;

    fFadeFromTime = 0.f;
    fFadeFromColor.Set(0,0,0,0);
    fFadeToTime = 0.f;
    fFadeToColor.Set(0,0,0,0);
}

void plBinkPlayer::AddCallback(plMessage* msg)
{
    hsRefCnt_SafeRef(msg);
    fCallbacks.Append(msg);
}

void plBinkPlayer::SetFileName(const char* fileName)
{
    delete [] fFileName;
    fFileName = hsStrcpy(fileName);
}

hsBool plBinkPlayer::Start(plPipeline* pipe,  hsWindowHndl hWnd) 
{
#ifdef BINK_SDK_AVAILABLE
    // If we haven't got a movie to play, we're done.
    if( !fFileName )
        return false;

    // If we're already playing, there's nothing to do.
    if( fBink )
        return true;

    HRESULT hr;
    fPipeline = plDXPipeline::ConvertNoRef( pipe );

    if( !fInit )
        Init(hWnd);

    BinkSetSoundTrack(2, fTracks);
    fBink = BinkOpen(fFileName, BINKSNDTRACK);
    if( !fBink ) 
        return Stop();

    BinkSetVolume(fBink, fTracks[kBackTrack], fVolume[kBackTrack]);
    BinkSetVolume(fBink, fTracks[kForeTrack], fVolume[kForeTrack]);

    D3DFORMAT format = D3DFMT_A8R8G8B8; // STUB

#if 0
    fTextureSize[0] = 512;
    fTextureSize[1] = 512;
#else
    fTextureSize[0] = fBink->Width;
    fTextureSize[1] = fBink->Height;
#endif
    
    hr = fPipeline->fD3DDevice->CreateTexture(fTextureSize[0], fTextureSize[1], 1, 0, format, D3DPOOL_MANAGED, &fTexture, NULL); 
    if(FAILED(hr)) 
        return Stop(); 

    ISetVerts();

    fNumPrimitives = 2;

    fCurrColor = fColor;
    if( fFadeFromTime > 0 )
    {
        fCurrColor = fFadeFromColor;
        fFadeState = kFadeFrom;
        fFadeStart = hsTimer::GetSeconds();
        fFadeParm = 0.f;
        BinkPause(fBink, true);
    }

    return true;
#else
    return false;
#endif /* BINK_SDK_AVAILABLE */
}

void plBinkPlayer::ISetVerts()
{
#ifdef BINK_SDK_AVAILABLE
    float sizeX = float(fBink->Width) / float(fPipeline->Width());
    float sizeY = float(fBink->Height) / float(fPipeline->Height());

    sizeX *= fScale.fX * 2.f;
    sizeY *= fScale.fY * 2.f;

    fVerts[0].x = fPos.fX - sizeX * 0.5f;
    fVerts[0].y = fPos.fY - sizeY * 0.5f;
    fVerts[0].z = 0.5f;
    fVerts[0].u = 0.0f;
    fVerts[0].v = 1.0f;

    fVerts[1] = fVerts[0];
    fVerts[1].x += sizeX ;
    fVerts[1].u = 1.0f;

    fVerts[2] = fVerts[0];
    fVerts[2].y += sizeY ;
    fVerts[2].v = 0.0f;

    fVerts[3] = fVerts[0];
    fVerts[3].x += sizeX;
    fVerts[3].y += sizeY;
    fVerts[3].u = 1.0f;
    fVerts[3].v = 0.0f;
#endif
}

hsBool plBinkPlayer::IGetFrame()
{
#ifdef BINK_SDK_AVAILABLE
    HRESULT hr;
    D3DLOCKED_RECT locked_rect;
    // Get the next frame

    BinkDoFrame( fBink );

    hr = fTexture->LockRect(0, &locked_rect, nil, 0);
    if(FAILED(hr))
        return Stop();

    U32 copyFlags = BINKSURFACE32;
    BinkCopyToBuffer( fBink, locked_rect.pBits, locked_rect.Pitch, fTextureSize[1], 0, 0,  copyFlags);

    hr = fTexture->UnlockRect(0);
    if(FAILED(hr))
        return Stop();
    
    if( !IAtEnd() )
        BinkNextFrame(fBink);

    return true;
#else
    return false;
#endif
}

hsBool plBinkPlayer::ICheckFadingFrom()
{
    if( fFadeState == kFadeFrom )
    {
        fFadeParm = (float)((hsTimer::GetSeconds() - fFadeStart) / fFadeFromTime);
        if( fFadeParm >= 1.f )
        {
            // We're done, go into normal mode
            fFadeState = kFadeNone;
#ifdef BINK_SDK_AVAILABLE
            BinkPause(fBink, false);
#endif
        }
        else
        {
            fCurrColor = fColor - fFadeFromColor;
            fCurrColor *= fFadeParm;
            fCurrColor += fFadeFromColor;
        }
    }
    return true;
}

hsBool plBinkPlayer::INotFadingFrom()
{
    return (fFadeState != kFadeFrom) && (fFadeState != kFadeFromPaused);
}

hsBool plBinkPlayer::ICheckFadingTo()
{
    if( IAtEnd() && (fFadeState != kFadeToPaused) )
    {
        if( fFadeState != kFadeTo )
        {
            if( fFadeToTime > 0 )
            {
                fFadeState = kFadeTo;
                fFadeStart = hsTimer::GetSeconds();
#ifdef BINK_SDK_AVAILABLE
                BinkPause(fBink, true);
#endif
            }
            else
            {
                fFadeState = kFadeNone;
                return true;
            }
        }
        else
        {
            fFadeParm = (float)((hsTimer::GetSeconds() - fFadeStart) / fFadeToTime);
            if( fFadeParm >= 1.f )
            {
                fFadeState = kFadeNone;
#ifdef BINK_SDK_AVAILABLE
                BinkPause(fBink, false);
#endif
            }
            else
            {
                fCurrColor = fFadeToColor - fColor;
                fCurrColor *= fFadeParm;
                fCurrColor += fColor;
            }
        }
        return false;
    }
    return true;
}

hsBool plBinkPlayer::INotFadingTo()
{
    return (fFadeState != kFadeTo) && (fFadeState != kFadeToPaused);
}

hsBool plBinkPlayer::IAtEnd()
{
#ifdef BINK_SDK_AVAILABLE
    return fBink->FrameNum == fBink->Frames;
#else
    return true;
#endif
}

hsBool plBinkPlayer::NextFrame()
{
#ifndef BINK_SDK_AVAILABLE
    return false;
#endif
    if( !fFileName )
        return false;

    if( !fBink )
        return true;

    ICheckFadingFrom();

    ICheckFadingTo();

    // check for end of movie
    if( IAtEnd() && INotFadingTo() )
    {
        return Stop();
    }

#ifdef BINK_SDK_AVAILABLE
    if ( !BinkWait( fBink ) )  
    {
        if( !IGetFrame() )
            return false;
    }
#endif

    if( fFadeState == kFadeNone )
        fCurrColor = fColor;

    return IBlitFrame();
}

hsBool plBinkPlayer::Pause(hsBool on)
{
    if( fBink )
    {
        switch( fFadeState )
        {
        case kFadeNone:
#ifdef BINK_SDK_AVAILABLE
            BinkPause(fBink, on);
#endif
            break;
        case kFadeFrom:
            if( on )
                fFadeState = kFadeFromPaused;
            break;
        case kFadeFromPaused:
            if( !on )
            {
                fFadeState = kFadeFrom;
                fFadeStart = hsTimer::GetSeconds() - fFadeParm * fFadeFromTime;
            }
            break;
        case kFadeTo:
            if( on )
                fFadeState = kFadeToPaused;
            break;
        case kFadeToPaused:
            if( !on )
            {
                fFadeState = kFadeTo;
                fFadeStart = hsTimer::GetSeconds() - fFadeParm * fFadeToTime;
            }
            break;
        }
        return true;
    }
    return false;
}

void plBinkPlayer::ISendCallbacks()
{
    int i;
    for( i = 0; i < fCallbacks.GetCount(); i++ )
        fCallbacks[i]->Send();
    fCallbacks.Reset();
}

hsBool plBinkPlayer::Stop()
{
    ISendCallbacks();
    fPipeline = nil;

    if(fTexture)
    {
        fTexture->Release();
        fTexture = nil;
    }

    delete [] fFileName;
    fFileName = nil;

    if( fBink )
    {
#ifdef BINK_SDK_AVAILABLE
        BinkClose( fBink );
#endif
        fBink = nil;
    }

    return false;
}

void plBinkPlayer::SetColor(const hsColorRGBA& color)
{
    fColor = color;
}

void plBinkPlayer::SetPosition(float x, float y)
{
    fPos.Set(x, y);
    if( fBink )
        ISetVerts();
}

void plBinkPlayer::SetScale(float x, float y)
{
    fScale.Set(x, y);
    if( fBink )
        ISetVerts();
}

float plBinkPlayer::IGetVolume(int background) const
{
    int t = background ? kBackTrack : kForeTrack;
    return float(fVolume[t]) / float(kMaxVolume);
}

void plBinkPlayer::ISetVolume(float v, int background)
{
    int track = background ? kBackTrack : kForeTrack;
    uint16_t volume;
    if( v < 0 )
        volume = 0;
    else if( v >= 1.f )
        volume = kMaxVolume;
    else
        volume = uint16_t(v * float(kMaxVolume));

    if( volume != fVolume[track] )
    {
        fVolume[track] = volume;
#ifdef BINK_SDK_AVAILABLE
        if( fBink )
            BinkSetVolume(fBink, fTracks[track], fVolume[track]);
#endif
    }
}

hsBool plBinkPlayer::IBlitFrame()
{
    HRESULT hr;

    plViewTransform resetTransform = fPipeline->GetViewTransform();

    fPipeline->fD3DDevice->SetTransform(D3DTS_WORLD, &ident);
    fPipeline->fD3DDevice->SetTransform(D3DTS_VIEW, &ident);
    fPipeline->fD3DDevice->SetTransform(D3DTS_PROJECTION, &ident);

    if( !ISetRenderState() )
        return Stop();

    hr = fPipeline->fD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, fNumPrimitives, fVerts, sizeof(D3DVertex));
    if(FAILED(hr))
        return Stop();

    fPipeline->SetViewTransform(resetTransform);

    return true;
}

hsBool plBinkPlayer::ISetRenderState()
{
    HRESULT hr;
    
    hr = fPipeline->fD3DDevice->SetTexture(0, fTexture);
    if(FAILED(hr)) 
        return Stop();

    fPipeline->fSettings.fCurrVertexShader = NULL;
    hr = fPipeline->fD3DDevice->SetVertexShader(NULL);
    if(FAILED(hr)) 
        return Stop();
    fPipeline->fD3DDevice->SetFVF(fPipeline->fSettings.fCurrFVFFormat = (D3DFVF_XYZ | D3DFVF_TEX1));
    fPipeline->fLayerState[0].fMiscFlags |= hsGMatState::kMiscTwoSided;
    fPipeline->fD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    fPipeline->fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    fPipeline->ISetLocalToWorld(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());

    fPipeline->fD3DDevice->SetTransform(D3DTS_TEXTURE0, &ident);
    fPipeline->fLayerTransform[0] = false;

    fPipeline->fD3DDevice->SetStreamSource(0, nil, 0, 0);
    hsRefCnt_SafeUnRef(fPipeline->fSettings.fCurrVertexBuffRef);
    fPipeline->fSettings.fCurrVertexBuffRef = nil;

    fPipeline->fD3DDevice->SetIndices(nil);
    hsRefCnt_SafeUnRef(fPipeline->fSettings.fCurrIndexBuffRef);
    fPipeline->fSettings.fCurrIndexBuffRef = nil;

    if( !fPipeline->fCurrD3DLiteState )
    {
        fPipeline->fD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
        fPipeline->fCurrD3DLiteState = true;
    }

    static D3DMATERIAL9 mat;
    mat.Diffuse.r = mat.Diffuse.g = mat.Diffuse.b = 0;
    mat.Diffuse.a = fCurrColor.a;
    mat.Ambient.r = mat.Ambient.g = mat.Ambient.b = mat.Ambient.a = 1.f;
    fPipeline->fD3DDevice->SetMaterial( &mat );
    fPipeline->fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
    fPipeline->fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
    fPipeline->fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
    fPipeline->fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
    fPipeline->fD3DDevice->SetRenderState( D3DRS_AMBIENT, fCurrColor.ToARGB32());

    // Always alpha blending to get started. Should be smarter and only
    // have blend on when we're blending.
    fPipeline->fD3DDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
    fPipeline->fD3DDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
//  fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

//    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    fPipeline->fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    fPipeline->fLayerState[0].fBlendFlags = hsGMatState::kBlendAlpha | hsGMatState::kBlendNoTexAlpha;
    fPipeline->IStageStop(1);

    fPipeline->fD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    fPipeline->fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    fPipeline->fLayerState[0].fZFlags = hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead;

    fPipeline->fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    fPipeline->fCurrFog.fEnvPtr = nil;

    fPipeline->fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    fPipeline->fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    fPipeline->fLayerState[0].fClampFlags |= hsGMatState::kClampTexture;

#ifdef HS_DEBUGGING
    DWORD nPass;
    DWORD error;
    error = fPipeline->fD3DDevice->ValidateDevice(&nPass);
    if( FAILED(error) )
    {
        error = fPipeline->fD3DDevice->ValidateDevice(&nPass);
    }
#endif // HS_DEBUGGING

    return true;
}