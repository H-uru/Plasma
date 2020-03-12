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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plTransitionMgr - Class to handle fullscreen transitions (fades, etc)   //
//                                                                          //
//  Note: eventually, I would like to drive these transitions on material   //
//  animations (it's just a big screen-covering plate with a material,      //
//  after all). This would allow the artists to specify their own           //
//  transitions and do really cool effects. However, that would require     //
//  somehow loading the materials in, and I'm not sure exactly how to do    //
//  that....                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include "HeadSpin.h"
#include "plTransitionMgr.h"
#include "plPlates.h"

#include "plGImage/plMipmap.h"
#include "plSurface/plLayer.h"
#include "plSurface/hsGMaterial.h"
#include "plMessage/plLayRefMsg.h"
#include "pnMessage/plRefMsg.h"
#include "plMessage/plTransitionMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plgDispatch.h"
#include "hsGDeviceRef.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "plAudio/plAudioSystem.h"
#include "pnNetCommon/plNetApp.h"
#include "plNetClient/plLinkEffectsMgr.h"
#include "pnNetCommon/plNetApp.h"

#include "plStatusLog/plStatusLog.h"

//// Constructor/Destructor //////////////////////////////////////////////////

plTransitionMgr::plTransitionMgr()
{
    fEffectPlate = nil;
    fCurrentEffect = kIdle;
    fPlaying = false;
}

void    plTransitionMgr::Init()
{
    ICreatePlate();
    plgDispatch::Dispatch()->RegisterForExactType( plTransitionMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plLinkEffectBCMsg::Index(), GetKey() );
}

plTransitionMgr::~plTransitionMgr()
{
    int     i;


    for( i = 0; i < fCallbacks.GetCount(); i++ )
        hsRefCnt_SafeUnRef( fCallbacks[ i ] );

    if( fEffectPlate != nil )
        plPlateManager::Instance().DestroyPlate( fEffectPlate );

    if( fRegisteredForTime )
        plgDispatch::Dispatch()->UnRegisterForExactType( plTimeMsg::Index(), GetKey() );

    plgDispatch::Dispatch()->UnRegisterForExactType( plTransitionMsg::Index(), GetKey() );
    plgDispatch::Dispatch()->UnRegisterForExactType( plLinkEffectBCMsg::Index(), GetKey() );
}

//// ICreatePlate ////////////////////////////////////////////////////////////

void    plTransitionMgr::ICreatePlate()
{
    int     x, y;


    fEffectPlate = nil;

    // +0.01 to deal with the half-pixel antialiasing stuff
    plPlateManager::Instance().CreatePlate( &fEffectPlate, 0, 0, 2.01, 2.01 );
    fEffectPlate->SetDepth(2);

    // hack for now--create a black layer that we will animate the opacity on
    plMipmap *ourMip = fEffectPlate->CreateMaterial( 16, 16, true );
    for( y = 0; y < ourMip->GetHeight(); y++ )
    {
        uint32_t  *pixels = ourMip->GetAddr32( 0, y );
        for( x = 0; x < ourMip->GetWidth(); x++ )
            pixels[ x ] = 0xff000000;
    }

    fEffectPlate->SetVisible( false );
}

//// IStartFadeOut ///////////////////////////////////////////////////////////

void    plTransitionMgr::IStartFadeOut( float lengthInSecs, uint8_t effect )
{
    fCurrentEffect = effect; // default - kFadeOut;
    fEffectLength = lengthInSecs;

    // Special case for length 0--just jump straight to fadeout
    if( lengthInSecs == 0.f )
    {
        fCurrOpacity = 1.f;
        fLastTime = -1.f;
        fPlaying = false;
        plgAudioSys::SetGlobalFadeVolume( 0.f );
    }
    else
    {
        fCurrOpacity = 0;
        fOpacDelta = 1.f / lengthInSecs;
        fLastTime = -1.f;
        fPlaying = true;

        // Register for time message
        plgDispatch::Dispatch()->RegisterForExactType( plTimeMsg::Index(), GetKey() );
        fRegisteredForTime = true;
    }

    if( fEffectPlate == nil )
        ICreatePlate();
    fEffectPlate->SetVisible( true );

    plLayer *layer = (plLayer *)fEffectPlate->GetMaterial()->GetLayer( 0 );
    if( layer != nil )
    {
        layer->SetOpacity( fCurrOpacity );
    }
}

//// IStartFadeIn ////////////////////////////////////////////////////////////

void    plTransitionMgr::IStartFadeIn( float lengthInSecs, uint8_t effect )
{
    fCurrentEffect = effect; // default - kFadeIn;
    fEffectLength = lengthInSecs;
    fCurrOpacity = 1.f;
    fOpacDelta = -1.f / lengthInSecs;
    fLastTime = -1.f;
    fPlaying = true;

    // Register for time message
    plgDispatch::Dispatch()->RegisterForExactType( plTimeMsg::Index(), GetKey() );
    fRegisteredForTime = true;

    if( fEffectPlate == nil )
        ICreatePlate();
    fEffectPlate->SetVisible( true );

    plLayer *layer = (plLayer *)fEffectPlate->GetMaterial()->GetLayer( 0 );
    if( layer != nil )
    {
        layer->SetOpacity( fCurrOpacity );
    }
}

//// IStop ///////////////////////////////////////////////////////////////////

void    plTransitionMgr::IStop( bool aboutToStartAgain /*= false*/ )
{
    int     i;

    plgDispatch::Dispatch()->UnRegisterForExactType( plTimeMsg::Index(), GetKey() );
    fRegisteredForTime = false;

    if( fPlaying )
    {
        if( !fHoldAtEnd && fEffectPlate != nil && !aboutToStartAgain )
            fEffectPlate->SetVisible( false );

        // finish the opacity to the end opacity
        if( fEffectPlate != nil )
        {
            plLayer *layer = (plLayer *)fEffectPlate->GetMaterial()->GetLayer( 0 );
            if( layer != nil )
            {
                layer->SetOpacity( (fCurrentEffect == kFadeIn || fCurrentEffect == kTransitionFadeIn) ? 0.f : 1.f );
            }
        }

        if( !aboutToStartAgain )
        {
            if (!fNoSoundFade)
                plgAudioSys::SetGlobalFadeVolume( (fCurrentEffect == kFadeIn || fCurrentEffect == kTransitionFadeIn) ? 1.f : 0.f );
        }

        for( i = 0; i < fCallbacks.GetCount(); i++ )
        {
            fCallbacks[ i ]->SetSender( GetKey() );
            plgDispatch::MsgSend( fCallbacks[ i ] );
        }
        fCallbacks.Reset();

        fPlaying = false;
    }
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    plTransitionMgr::MsgReceive( plMessage* msg )
{
    int         i;


    plTimeMsg   *time = plTimeMsg::ConvertNoRef( msg );
    if( time != nil )
    {
        if( !fPlaying )
            return false;

        if (fLastTime < 0)
        {
            // Semi-hack. We trigger transitions after we've finished loading.
            // Problem is the loading all happens in one really long frame, so that
            // if we record the time we started, we'll instantly be done next frame,
            // even though we triggered just at the "end" of the last frame.
            // 
            // So instead we don't start the clock until we get our first plTimeMsg.

            
            fLastTime = (float)(time->DSeconds());
            return false;
        }

        fEffectLength -= (float)( time->DSeconds() - fLastTime );//*/time->DelSeconds();
        if( fEffectLength < 0 )
            IStop();
        else
        {
            // Grab the layer so we can set the opacity
            fCurrOpacity += (float)(fOpacDelta * ( time->DSeconds() - fLastTime ));//*/time->DelSeconds();
            if( fEffectPlate == nil )
                ICreatePlate();

            plLayer *layer = (plLayer *)fEffectPlate->GetMaterial()->GetLayer( 0 );
            if( layer != nil )
            {
                layer->SetOpacity( fCurrOpacity );
            }

            // Let the audiosystem handle fading in sounds
            if(!fNoSoundFade)
                plgAudioSys::SetGlobalFadeVolume( 1.f - fCurrOpacity );
            

            fLastTime = (float)(time->DSeconds());
        }
        
        return false;
    }
    
    plTransitionMsg *effect = plTransitionMsg::ConvertNoRef( msg );
    if( effect != nil )
    {
        if( fRegisteredForTime )
            IStop( true );

        for( i = 0; i < effect->GetNumCallbacks(); i++ )
        {
            plEventCallbackMsg *pMsg = effect->GetEventCallback( i );
            hsRefCnt_SafeRef( pMsg );
            fCallbacks.Append( pMsg );
        }
        
        fHoldAtEnd = effect->GetHoldState();

        fNoSoundFade = false;

        switch(effect->GetEffect())
        {
        case plTransitionMsg::kFadeInNoSound:
            fNoSoundFade = true;
            // fall through
        case plTransitionMsg::kFadeIn:
            IStartFadeIn( effect->GetLengthInSecs(), kTransitionFadeIn );
            break;
        case plTransitionMsg::kFadeOutNoSound:
            fNoSoundFade = true;
            // fall through
        case plTransitionMsg::kFadeOut:
            IStartFadeOut( effect->GetLengthInSecs(), kTransitionFadeOut );
            break;
        }

        return false;
    }

    plLinkEffectBCMsg *link = plLinkEffectBCMsg::ConvertNoRef( msg );
    if( link != nil )
    {
        const float kScreenFadeTime = 3.f; // seconds

        // Go ahead and auto-trigger based on link FX messages
        if( plNetClientApp::GetInstance() != nil && link->fLinkKey == plNetClientApp::GetInstance()->GetLocalPlayerKey() )
        {
            if( fRegisteredForTime )
                IStop( true );

            if( link->HasLinkFlag( plLinkEffectBCMsg::kLeavingAge ) )
            {
                plNetApp::GetInstance()->DebugMsg("Local player linking out, fading screen\n");

                fHoldAtEnd = true;
                IStartFadeOut( kScreenFadeTime );
            }
            else
            {
                plNetApp::GetInstance()->DebugMsg("Local player linking in, fading screen\n");

                fHoldAtEnd = false;
                IStartFadeIn( kScreenFadeTime );
            }   

            if (link->HasLinkFlag(plLinkEffectBCMsg::kSendCallback))
            {
                plLinkEffectsMgr *mgr;
                if( ( mgr = plLinkEffectsMgr::ConvertNoRef( link->GetSender()->ObjectIsLoaded() ) ) != nil )
                {
                    plEventCallbackMsg *cback = plEventCallbackMsg::ConvertNoRef( mgr->WaitForEffect( link->fLinkKey ) );
//                  hsRefCnt_SafeRef( cback ); // mgr has given us ownership, his ref is now ours. No need for another. -mf-
                    fCallbacks.Append( cback );
                }
            }
        }
        return true;
    }
    return hsKeyedObject::MsgReceive( msg );
}

