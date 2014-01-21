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
//  pfGUIKnobCtrl Definition                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "pfGUIKnobCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "plInputCore/plInputInterface.h"
#include "pnMessage/plRefMsg.h"
#include "pfMessage/pfGameGUIMsg.h"
#include "plMessage/plAnimCmdMsg.h"
// #include "plAvatar/plAGModifier.h"
#include "plAvatar/plAGMasterMod.h"
#include "plAvatar/plAGAnimInstance.h"
#include "plSurface/plLayerAnimation.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIKnobCtrl::pfGUIKnobCtrl() :
    fDragStart(0.f, 0.f, 0.f),
    fDragging(false),
    fAnimStartPos(0.f, 0.f, 0.f),
    fAnimEndPos(0.f, 0.f, 0.f),
    fDragRangeMin(0.f),
    fDragRangeMax(0.f),
    fAnimBegin(0.f),
    fAnimEnd(0.f),
    fAnimTimesCalced(false)
{
    SetFlag( kWantsInterest );
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIKnobCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIValueCtrl::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIKnobCtrl::MsgReceive( plMessage *msg )
{
    return pfGUIValueCtrl::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIKnobCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Read(s, mgr);

    fAnimationKeys.Reset();
    uint32_t i, count = s->ReadLE32();
    for( i = 0; i < count; i++ )
        fAnimationKeys.Append( mgr->ReadKey( s ) );
    fAnimName = s->ReadSafeString();

    fAnimTimesCalced = false;

    fAnimStartPos.Read( s );
    fAnimEndPos.Read( s );
}

void    pfGUIKnobCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIValueCtrl::Write( s, mgr );

    uint32_t i, count = fAnimationKeys.GetCount();
    s->WriteLE32( count );
    for( i = 0; i < count; i++ )
        mgr->WriteKey( s, fAnimationKeys[ i ] );
    s->WriteSafeString( fAnimName );

    fAnimStartPos.Write( s );
    fAnimEndPos.Write( s );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void    pfGUIKnobCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, bool force )
{
    pfGUIValueCtrl::UpdateBounds( invXformMatrix, force );
    if( fAnimationKeys.GetCount() > 0 )
        fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void    pfGUIKnobCtrl::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    fDragStart = mousePt;
    fDragValue = fValue;
    fDragging = true;

    if( HasFlag( kMapToAnimationRange ) )
    {
        hsPoint3    scrnStart, scrnEnd;

        // At mouse-down, we take our local-space start and end points and
        // translate them by our parent object's local-to-world to get the
        // right points in world-space. We do this now because our parent
        // might be animated, which could complicate matters a tad.
        scrnStart = fAnimStartPos;
        scrnEnd = fAnimEndPos;

        plSceneObject *target = GetTarget();
        if( target != nil )
        {
            const plCoordinateInterface *ci = target->GetCoordinateInterface();
            if( ci != nil )
            {
                const plCoordinateInterface *parentCI = ci->GetParent();
                if( parentCI != nil )
                {
                    const hsMatrix44 &parentLocalToWorld = parentCI->GetLocalToWorld();
    
                    scrnStart = parentLocalToWorld * scrnStart;
                    scrnEnd = parentLocalToWorld * scrnEnd;
                }
            }
        }

        scrnStart = fDialog->WorldToScreenPoint( scrnStart );
        scrnEnd = fDialog->WorldToScreenPoint( scrnEnd );

        if( HasFlag( kLeftRightOrientation ) )
        {
            fDragRangeMin = scrnStart.fX;
            fDragRangeMax = scrnEnd.fX;
        }
        else
        {
            fDragRangeMin = scrnStart.fY;
            fDragRangeMax = scrnEnd.fY;
        }
    }
    else if( HasFlag( kMapToScreenRange ) )
    {
        fDragRangeMin = 0.f;
        fDragRangeMax = 1.f;
    }
    else
        fDragRangeMin = -1;
}

void    pfGUIKnobCtrl::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
    fDragging = false;
    HandleMouseDrag( mousePt, modifiers );
}

void    pfGUIKnobCtrl::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
    float oldValue = fValue, newValue = fDragValue;

    if( fDragRangeMin != -1 )
    {
        if( HasFlag( kLeftRightOrientation ) )
        {
            if( mousePt.fX < fDragRangeMin )
                newValue = fMin;
            else if( mousePt.fX > fDragRangeMax )
                newValue = fMax;
            else
                newValue = ( ( mousePt.fX - fDragRangeMin ) / ( fDragRangeMax - fDragRangeMin ) ) *
                            ( fMax - fMin ) + fMin;
        }
        else
        {
            if( mousePt.fY > fDragRangeMin )
                newValue = fMin;
            else if( mousePt.fY < fDragRangeMax )
                newValue = fMax;
            else
                newValue = ( (  fDragRangeMin - mousePt.fY) / ( fDragRangeMin - fDragRangeMax ) ) *
                            ( fMax - fMin ) + fMin;
        }

        if( HasFlag( kReverseValues ) )
            SetCurrValue( fMax - ( newValue - fMin ) );
        else
            SetCurrValue( newValue );
    }
    else
    {
        float diff;
        if( HasFlag( kLeftRightOrientation ) )
            diff = ( mousePt.fX - fDragStart.fX ) * 20.f;
        else
            diff = ( fDragStart.fY - mousePt.fY ) * 20.f;

        if( HasFlag( kReverseValues ) )
            SetCurrValue( fDragValue - diff );
        else
            SetCurrValue( fDragValue + diff );
    }

    // !fDragging = We're mousing-up, so if we're still dragging, we need to not have the only-
    // on-mouse-up flag set. Just FYI
    if( !fDragging || !HasFlag( kTriggerOnlyOnMouseUp ) )
        DoSomething();
}

//// SetAnimationKeys ////////////////////////////////////////////////////////

void    pfGUIKnobCtrl::SetAnimationKeys( hsTArray<plKey> &keys, const plString &name )
{
    fAnimationKeys = keys;
    fAnimName = name;
}

//// ICalcAnimTimes //////////////////////////////////////////////////////////
//  Loops through and computes the max begin and end for our animations. If
//  none of them are loaded and we're not already calced, returns false.

bool    pfGUIKnobCtrl::ICalcAnimTimes( void )
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

void    pfGUIKnobCtrl::SetCurrValue( float v )
{
    int old = (int)fValue;
    pfGUIValueCtrl::SetCurrValue( v );

//  if( old == (int)fValue )
//      return;

    if( fAnimationKeys.GetCount() > 0 )
    {
        ICalcAnimTimes();

        float tLength = fAnimEnd - fAnimBegin;
        float newTime = fMin;

        if (fMin != fMax) // Protect against div by zero
        {
            if( HasFlag( kReverseValues ) )
                newTime = ( ( fMax - fValue ) / ( fMax - fMin ) ) * tLength + fAnimBegin;
            else
                newTime = ( ( fValue - fMin ) / ( fMax - fMin ) ) * tLength + fAnimBegin;
        }
        plAnimCmdMsg *msg = new plAnimCmdMsg();
        msg->SetCmd( plAnimCmdMsg::kGoToTime ); 
        msg->SetAnimName( fAnimName );
        msg->fTime = newTime;
        msg->AddReceivers( fAnimationKeys );
        plgDispatch::MsgSend( msg );
    }
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

uint32_t      pfGUIKnobCtrl::IGetDesiredCursor( void ) const
{
    if( HasFlag( kLeftRightOrientation ) )
    {
        if( fDragging )
            return plInputInterface::kCursorLeftRightDragging;

        return plInputInterface::kCursorLeftRightDraggable;
    }
    else
    {
        if( fDragging )
            return plInputInterface::kCursorUpDownDragging;

        return plInputInterface::kCursorUpDownDraggable;
    }
}

