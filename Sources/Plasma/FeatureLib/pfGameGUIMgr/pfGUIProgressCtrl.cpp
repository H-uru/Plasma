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
//  pfGUIProgressCtrl Definition                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pfGUIProgressCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "plInputCore/plInputInterface.h"
#include "pnMessage/plRefMsg.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plTimerCallbackMsg.h"
// #include "plAvatar/plAGModifier.h"
#include "plAvatar/plAGMasterMod.h"
#include "plAvatar/plAGAnimInstance.h"
#include "plSurface/plLayerAnimation.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnTimer/plTimerCallbackManager.h"

#include "plgDispatch.h"
#include "hsResMgr.h"

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIProgressCtrl::pfGUIProgressCtrl() : fStopSoundTimer(99)
{
    fAnimTimesCalced = false;
    fPlaySound = true;
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIProgressCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIValueCtrl::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIProgressCtrl::MsgReceive( plMessage *msg )
{
    plTimerCallbackMsg *timerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
    if (timerMsg)
    {
        if (timerMsg->fID == fStopSoundTimer)
        {
            // we've finished animating, stop the sound that's playing
            StopSound(kAnimateSound);
        }
    }
    return pfGUIValueCtrl::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIProgressCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Read(s, mgr);

    fAnimationKeys.Reset();
    uint32_t i, count = s->ReadLE32();
    for( i = 0; i < count; i++ )
        fAnimationKeys.Append( mgr->ReadKey( s ) );
    fAnimName = s->ReadSafeString_TEMP();

    fAnimTimesCalced = false;
}

void    pfGUIProgressCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Write( s, mgr );

    uint32_t i, count = fAnimationKeys.GetCount();
    s->WriteLE32( count );
    for( i = 0; i < count; i++ )
        mgr->WriteKey( s, fAnimationKeys[ i ] );
    s->WriteSafeString( fAnimName );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void    pfGUIProgressCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, bool force )
{
    pfGUIValueCtrl::UpdateBounds( invXformMatrix, force );
    if( fAnimationKeys.GetCount() > 0 )
        fBoundsValid = false;
}

//// SetAnimationKeys ////////////////////////////////////////////////////////

void    pfGUIProgressCtrl::SetAnimationKeys( hsTArray<plKey> &keys, const plString &name )
{
    fAnimationKeys = keys;
    fAnimName = name;
}

//// ICalcAnimTimes //////////////////////////////////////////////////////////
//  Loops through and computes the max begin and end for our animations. If
//  none of them are loaded and we're not already calced, returns false.

bool    pfGUIProgressCtrl::ICalcAnimTimes( void )
{
    if( fAnimTimesCalced )
        return true;

    float tBegin = 1e30, tEnd = -1e30;
    bool     foundOne = false;

    for( int i = 0; i < fAnimationKeys.GetCount(); i++ )
    {
        // Handle AGMasterMods
        plAGMasterMod *mod = plAGMasterMod::ConvertNoRef( fAnimationKeys[ i ]->ObjectIsLoaded() );
        if( mod != nil )
        {
            for( int j = 0; j < mod->GetNumAnimations(); j++ )
            {
                float begin = mod->GetAnimInstance( j )->GetTimeConvert()->GetBegin();
                float end = mod->GetAnimInstance( j )->GetTimeConvert()->GetEnd();
                if( begin < tBegin )
                    tBegin = begin;
                if( end > tEnd )
                    tEnd = end;
            }
            foundOne = true;
        }
        // Handle layer animations
        plLayerAnimation *layer = plLayerAnimation::ConvertNoRef( fAnimationKeys[ i ]->ObjectIsLoaded() );
        if( layer != nil )
        {
            float begin = layer->GetTimeConvert().GetBegin();
            float end = layer->GetTimeConvert().GetEnd();
            if( begin < tBegin )
                tBegin = begin;
            if( end > tEnd )
                tEnd = end;
            foundOne = true;
        }
    }

    if( foundOne )
    {
        fAnimBegin = tBegin;
        fAnimEnd = tEnd;

        fAnimTimesCalced = true;
    }

    return fAnimTimesCalced;
}

//// SetCurrValue ////////////////////////////////////////////////////////////

void    pfGUIProgressCtrl::SetCurrValue( float v )
{
    int old = (int)fValue;

    pfGUIValueCtrl::SetCurrValue( v );

//  if( old == (int)fValue )
//      return;

    if( fAnimationKeys.GetCount() > 0 )
    {
        ICalcAnimTimes();

        float tLength = fAnimEnd - fAnimBegin;
        float newTime;

        if( HasFlag( kReverseValues ) )
            newTime = ( ( fMax - fValue ) / ( fMax - fMin ) ) * tLength + fAnimBegin;
        else
            newTime = ( ( fValue - fMin ) / ( fMax - fMin ) ) * tLength + fAnimBegin;

        plAnimCmdMsg *msg = new plAnimCmdMsg();
        msg->SetCmd( plAnimCmdMsg::kGoToTime ); 
        msg->SetAnimName( fAnimName );
        msg->fTime = newTime;
        msg->AddReceivers( fAnimationKeys );
        plgDispatch::MsgSend( msg );
    }
}

void pfGUIProgressCtrl::AnimateToPercentage( float percent )
{
    // percent should be a value in range 0.0 to 1.0
    if (percent >= 0.0f && percent <= 1.0f)
    {
        pfGUIValueCtrl::SetCurrValue( (fMax - fMin) * percent + fMin );

        if( fAnimationKeys.GetCount() > 0 )
        {
            plAnimCmdMsg *msg = new plAnimCmdMsg();
            msg->SetCmd( plAnimCmdMsg::kPlayToPercentage ); 
            msg->SetAnimName( fAnimName );
            msg->fTime = percent;
            msg->AddReceivers( fAnimationKeys );
            plgDispatch::MsgSend( msg );

            if (fPlaySound)
            {
                // play the sound, looping
                PlaySound(kAnimateSound, true);

                // setup a timer to call back when we finish animating
                float elapsedTime = (fAnimEnd - fAnimBegin) * percent;
                plTimerCallbackMsg *timerMsg = new plTimerCallbackMsg(GetKey(), fStopSoundTimer);
                plgTimerCallbackMgr::NewTimer(elapsedTime, timerMsg);
            }
        }
    }
}
