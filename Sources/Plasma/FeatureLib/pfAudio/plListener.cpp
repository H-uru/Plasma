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

#include "plListener.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsResMgr.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plAudio/plAudioSystem.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plPhysicalControllerCore.h"
#include "plMessage/plListenerMsg.h"
#include "plPipeline/plDebugText.h"

#include "pfCamera/plVirtualCamNeu.h"

bool        plListener::fPrintDbgInfo = false;

bool plListener::IEval(double secs, float del, uint32_t dirty)
{
//  if (!plgAudioSys::Active())
//      return true;
    plSceneObject *pRefObject = nullptr;

    int y = 16 + 12, x = 400;
    if( fPrintDbgInfo ) 
        plDebugText::Instance().DrawString(x, 16, ST_LITERAL("Listener:"), (uint32_t)0xffffffff, plDebugText::kStyleBold);

    // Get the avatar's SceneObject
    plKey key = plNetClientApp::GetInstance()->GetLocalPlayerKey();
    if(key)
        pRefObject = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());

    if (pRefObject == nullptr && fVCam == nullptr)
    {
        // We don't have a position to init by, so do NOT eval yet!!!
        if( fPrintDbgInfo ) 
            plDebugText::Instance().DrawString(x, y, ST_LITERAL("Not eval-ing yet"), (uint32_t)0xffffffff);
        return true;
    }

    // Changed 2.19.02 mcn - Basing it off the head bone isn't what we really want, esp. since
    // it isn't what the camera uses. What we *really* want is a head-ish-positioned non-bobbing node
    // that we base both off of. Until then, we're just going to have to use the avatar's root (i.e. his
    // feet) and add in an appropriate height. See plAvBrain.cpp::BindAudioListener() for the other half
    // of the hack.
    // Note the 2nd: since GetAxis() is buggy, we'll just add in a constant vector. Of course, this implies
    // that the avatar is always oriented up, but then it also implies he's always of constant height, so
    // there.
    const hsVector3 kAvatarHeightVector = hsVector3( 0, 0, 6.33f ); // isn't *everyone* 6'4"?

    /// Collect the current values for our parameters
    hsPoint3    position;
    hsVector3   velocity, dir, up;

    enum 
    {
        kInvalid = 0,
        kVCam,
        kObject
    } facingType = kInvalid, posType = kInvalid, velType = kInvalid;

    // Facing
    if( fFacingRatio == 1.f )
    {
        if (pRefObject != nullptr && pRefObject->GetCoordinateInterface())
        {
            hsMatrix44 facingL2W = pRefObject->GetCoordinateInterface()->GetLocalToWorld();
            dir = facingL2W.GetAxis( hsMatrix44::kView );
            up = facingL2W.GetAxis( hsMatrix44::kUp );

            facingType = kObject;
        }
    }
    else if (fVCam != nullptr)
    {
        dir = hsVector3( fVCam->GetCameraPOA() - fVCam->GetCameraPos() );
        up = fVCam->GetCameraUp();
        facingType = kVCam;
    }

    // Position
    if( fPosRatio == 1.f )
    {
        if (pRefObject != nullptr && pRefObject->GetCoordinateInterface())
        {
            position = pRefObject->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
            position += kAvatarHeightVector;
            posType = kObject;
        }
    }
    else if (fVCam != nullptr)
    {
        position = fVCam->GetCameraPos();
        posType = kVCam;
    }

    // Velocity
    if( fVelRatio == 1.f )
    {
        if (pRefObject != nullptr)
        {
            plArmatureMod* arm = plAvatarMgr::GetInstance()->GetLocalAvatar();
            if (arm)
            {
                plPhysicalControllerCore* controller = arm->GetController();
                if (controller)
                {
                    velocity = controller->GetLinearVelocity();
                    velType = kObject;
                }
            }
        }
    }
    else if (fVCam != nullptr)
    {
        // Darn, can't do it
    }

    if( facingType == kInvalid || posType == kInvalid || velType == kInvalid )
    {
        if( fPrintDbgInfo ) 
            plDebugText::Instance().DrawString(x, y, ST_LITERAL("Not eval-ing: missing one or more parameter bases"), (uint32_t)0xff0000ff);
        return true;
    }

    // Got the params, now construct and send out the message, as well as update the audio system
    plListenerMsg* msg = new plListenerMsg;
    msg->SetDirection( dir );
    msg->SetUp( up );
    msg->SetPosition( position );
    msg->SetVelocity( velocity );

    plgAudioSys::SetListenerOrientation( dir, up );
    plgAudioSys::SetListenerPos( position );
    plgAudioSys::SetListenerVelocity( velocity );

    if( fPrintDbgInfo ) 
    {
        ST::string str;
        str = ST::format("Direction: ({3.2f},{3.2f},{3.2f}) from {}", dir.fX, dir.fY, dir.fZ,
                         (facingType == kObject) ? pRefObject->GetKeyName() : "VCam");
        plDebugText::Instance().DrawString(x, y, str, (uint32_t)0xffffffff);
        y += 12;

        str = ST::format("Up: ({3.2f},{3.2f},{3.2f}) from {}", up.fX, up.fY, up.fZ,
                         (facingType == kObject) ? pRefObject->GetKeyName() : "VCam");
        plDebugText::Instance().DrawString(x, y, str, (uint32_t)0xffffffff);
        y += 12;

        str = ST::format("Position: ({3.2f},{3.2f},{3.2f}) from {}", position.fX, position.fY, position.fZ,
                         (posType == kObject) ? pRefObject->GetKeyName() : "VCam");
        plDebugText::Instance().DrawString(x, y, str, (uint32_t)0xffffffff);
        y += 12;

        str = ST::format("Velocity: ({3.2f},{3.2f},{3.2f}) from {}", velocity.fX, velocity.fY, velocity.fZ,
                         (velType == kObject) ? pRefObject->GetKeyName() : "VCam");
        plDebugText::Instance().DrawString(x, y, str, (uint32_t)0xffffffff);
        y += 12;
    }
    plgDispatch::MsgSend( msg );

    return true;
}

void    plListener::ISetRef( const plKey &ref, bool binding, int type )
{
    if( binding )
        hsgResMgr::ResMgr()->AddViaNotify( ref, new plGenRefMsg( GetKey(), plGenRefMsg::kOnReplace, -1, type ), plRefFlags::kPassiveRef );
    else
        GetKey()->Release( ref );
}

void    plListener::IEnsureVCamValid()
{
    if( fPosRatio == 1.f && fFacingRatio == 1.f && fVelRatio == 1.f )
    {
        // All of our params are purely using objects, so we don't need a virtual camera pointer at all
        if (fVCam != nullptr)
            ISetRef( fVCam->GetKey(), false, kRefVCam );
    }
    else
    {
        // One or more of our params are using the vcam as a basis, so make sure we have it
        if (fVCam == nullptr)
        {
            plVirtualCam1 *vCam = plVirtualCam1::Instance();
            if (vCam == nullptr)
            {
                hsAssert( false, "Unable to grab virtual camera instance; no basis for listener!!!" );
                return;
            }

            ISetRef( vCam->GetKey(), true, kRefVCam );
        }
    }
}

void    plListener::ICheckAudio() const
{
    if ((fPosRatio < 1.f || fFacingRatio < 1.f || fVelRatio < 1.f) && fVCam == nullptr)
        plgAudioSys::SetMuted( true );
}

bool plListener::MsgReceive(plMessage* msg)
{
    plSetListenerMsg *setMsg = plSetListenerMsg::ConvertNoRef( msg );
    if (setMsg != nullptr)
    {
        if( setMsg->GetType() & plSetListenerMsg::kVCam )
        {
            // Reset any ratios
            if( setMsg->GetType() & plSetListenerMsg::kPosition )
                fPosRatio = 0.f;

            if( setMsg->GetType() & plSetListenerMsg::kVelocity )
                fVelRatio = 0.f;

            if( setMsg->GetType() & plSetListenerMsg::kFacing )
                fFacingRatio = 0.f;

            IEnsureVCamValid();
        }
        else
        {
            if( setMsg->GetType() & plSetListenerMsg::kPosition )
                fPosRatio = 1.f;

            if( setMsg->GetType() & plSetListenerMsg::kVelocity )
                fVelRatio = 1.f;

            if( setMsg->GetType() & plSetListenerMsg::kFacing )
                fFacingRatio = 1.f;

            if( fPosRatio > 0.f || fVelRatio > 0.f || fFacingRatio > 0.f )
                // Need this, so store it now
                ISetRef( setMsg->GetSrcKey(), setMsg->IsBinding(), kRefObject );
        }

        return true;
    }

    plEvalMsg* pEMsg = plEvalMsg::ConvertNoRef(msg);
    if (pEMsg)
    {   
        IEval(pEMsg->GetTimeStamp(), pEMsg->DelSeconds(), true);

        if( fInitMe )
        {
            // By default, position and orientation are camera based
            plSetListenerMsg *set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nullptr, true);
            set->Send();
            set = new plSetListenerMsg(plSetListenerMsg::kVCam | plSetListenerMsg::kPosition, nullptr, true);
            set->Send();

            fInitMe = false;
        }

        return true;
    }

    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef( msg );
    if (refMsg != nullptr)
    {
        if( refMsg->fType == kRefVCam )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fVCam = plVirtualCam1::ConvertNoRef( refMsg->GetRef() );
            }
            else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
            {
                if( plVirtualCam1::ConvertNoRef( refMsg->GetRef() ) == fVCam )
                    fVCam = nullptr;
            }
            ICheckAudio();
        }

        return true;
    }

    return plSingleModifier::MsgReceive(msg);
}