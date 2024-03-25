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
//  pfGUIProgressCtrl Definition                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIProgressCtrl.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "plTimerCallbackManager.h"

#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plAGMasterMod.h"
#include "plInputCore/plInputInterface.h"
#include "plInterp/plAnimTimeConvert.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plMessage/plTimerCallbackMsg.h"
#include "plSurface/plLayerAnimation.h"

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

    fAnimationKeys.clear();
    uint32_t count = s->ReadLE32();
    fAnimationKeys.reserve(count);
    for (uint32_t i = 0; i < count; i++)
        fAnimationKeys.emplace_back(mgr->ReadKey(s));
    fAnimName = s->ReadSafeString();

    fAnimTimesCalced = false;
}

void    pfGUIProgressCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Write( s, mgr );

    s->WriteLE32((uint32_t)fAnimationKeys.size());
    for (const plKey& key : fAnimationKeys)
        mgr->WriteKey(s, key);
    s->WriteSafeString( fAnimName );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void    pfGUIProgressCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, bool force )
{
    pfGUIValueCtrl::UpdateBounds( invXformMatrix, force );
    if (!fAnimationKeys.empty())
        fBoundsValid = false;
}

//// SetAnimationKeys ////////////////////////////////////////////////////////

void    pfGUIProgressCtrl::SetAnimationKeys(const std::vector<plKey> &keys, const ST::string &name)
{
    fAnimationKeys = keys;
    fAnimName = name;
}

//// ICalcAnimTimes //////////////////////////////////////////////////////////
//  Loops through and computes the max begin and end for our animations. If
//  none of them are loaded and we're not already calced, returns false.

bool    pfGUIProgressCtrl::ICalcAnimTimes()
{
    if( fAnimTimesCalced )
        return true;

    float tBegin = 1e30f, tEnd = -1e30f;
    bool     foundOne = false;

    for (const plKey &animKey : fAnimationKeys)
    {
        // Handle AGMasterMods
        plAGMasterMod *mod = plAGMasterMod::ConvertNoRef(animKey->ObjectIsLoaded());
        if (mod != nullptr)
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
        plLayerAnimation *layer = plLayerAnimation::ConvertNoRef(animKey->ObjectIsLoaded());
        if (layer != nullptr)
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

    if (!fAnimationKeys.empty())
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
        msg->Send();
    }
}

void pfGUIProgressCtrl::AnimateToPercentage( float percent )
{
    // percent should be a value in range 0.0 to 1.0
    if (percent >= 0.0f && percent <= 1.0f)
    {
        pfGUIValueCtrl::SetCurrValue( (fMax - fMin) * percent + fMin );

        if (!fAnimationKeys.empty())
        {
            plAnimCmdMsg *msg = new plAnimCmdMsg();
            msg->SetCmd( plAnimCmdMsg::kPlayToPercentage ); 
            msg->SetAnimName( fAnimName );
            msg->fTime = percent;
            msg->AddReceivers( fAnimationKeys );
            msg->Send();

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
