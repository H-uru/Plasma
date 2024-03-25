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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plCubicRenderTargetModifier Class Functions                              //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  7.20.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "plCubicRenderTargetModifier.h"
#include "plCubicRenderTarget.h"

#include "hsBounds.h"
#include "plgDispatch.h"
#include "plDrawable.h"
#include "plPipeline.h"
#include "hsResMgr.h"
#include "hsTimer.h"

#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plRenderRequestMsg.h"
#include "plScene/plRenderRequest.h"



//// Constructor & Destructor /////////////////////////////////////////////////

plCubicRenderTargetModifier::plCubicRenderTargetModifier()
{
    fTarget = nullptr;
    fCubic = nullptr;

    fRequests[0] = fRequests[1] = fRequests[2] = fRequests[3] = fRequests[4] = fRequests[5] = nullptr;
}

plCubicRenderTargetModifier::~plCubicRenderTargetModifier()
{
    int     i;


    for( i = 0; i < 6; i++ )
        delete fRequests[ i ];
}

//// ICreateRenderRequest /////////////////////////////////////////////////////
//  Creates a blank renderRequest to use for fun stuff.

void    plCubicRenderTargetModifier::ICreateRenderRequest( int face )
{
    plRenderRequest *rr = fRequests[ face ];
    hsColorRGBA     c;
    
    
    if (rr == nullptr)
        rr = fRequests[ face ] = new plRenderRequest;

    uint32_t renderState 
        = plPipeline::kRenderNormal
        | plPipeline::kRenderClearColor
        | plPipeline::kRenderClearDepth;
    rr->SetRenderState( renderState );

    rr->SetDrawableMask( plDrawable::kNormal );
    rr->SetSubDrawableMask( plDrawable::kSubAllTypes );

    rr->SetHither(0.3f); // MF_HORSE ????
    rr->SetYon(1000.f); // MF_HORSE ????

    rr->SetFovX( 90 );
    rr->SetFovY( 90 );

    c.Set( 0, 0, 0, 1 );
    rr->SetClearColor( c );
    rr->SetClearDepth( 1.f );

    rr->SetClearDrawable(nullptr);
    rr->SetRenderTarget( fCubic->GetFace( face ) );
}

//// IEval ////////////////////////////////////////////////////////////////////

bool    plCubicRenderTargetModifier::IEval( double secs, float del, uint32_t dirty )
{
    hsPoint3    center;

    plRenderRequestMsg  *msg;


    if (fCubic == nullptr || fTarget == nullptr)
        return true;

    /// Get center point for RT
    plCoordinateInterface   *ci = IGetTargetCoordinateInterface( 0 );
    if (ci == nullptr)
    {
        plDrawInterface *di = IGetTargetDrawInterface( 0 );
        center = di->GetWorldBounds().GetCenter();
    }
    else
        center = ci->GetLocalToWorld().GetTranslate();

    /// Set camera position of RT to this center
    fCubic->SetCameraMatrix(center);

    /// Submit render requests!
    for (int i = 0; i < 6; i++)
    {
        if (fRequests[i] != nullptr)
        {
            fRequests[ i ]->SetCameraTransform(fCubic->GetWorldToCamera(i), fCubic->GetCameraToWorld(i));

            msg = new plRenderRequestMsg(nullptr, fRequests[i]);
            plgDispatch::MsgSend( msg );
        }
    }

    /// Done!
    return true;
}

//// MsgReceive ///////////////////////////////////////////////////////////////

bool    plCubicRenderTargetModifier::MsgReceive( plMessage* msg )
{
    plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
    if (eval)
    {
        const double secs = eval->DSeconds();
        const float del = eval->DelSeconds();
        IEval(secs, del, 0);
        return true;
    }

    plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plSceneObject* scene = plSceneObject::ConvertNoRef(refMsg->GetRef());
        if (scene)
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
                AddTarget(scene);
            else
                RemoveTarget(scene);
        }

        plCubicRenderTarget* cubic = plCubicRenderTarget::ConvertNoRef(refMsg->GetRef());
        if (cubic)
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace))
            {
                fCubic = cubic;
                for (int i = 0; i < 6; i++)
                    ICreateRenderRequest(i);
            }
            else
            {
                fCubic = nullptr;
                for (int i = 0; i < 6; i++)
                {
                    delete fRequests[i];
                    fRequests[i] = nullptr;
                }
            }
        }

        return true;
    }

    return plModifier::MsgReceive(msg);
}

//// AddTarget ////////////////////////////////////////////////////////////////

void    plCubicRenderTargetModifier::AddTarget( plSceneObject *so )
{
    if (fTarget != nullptr)
        RemoveTarget( fTarget );

    fTarget = so;
    plgDispatch::Dispatch()->RegisterForExactType( plEvalMsg::Index(), GetKey() );
}

//// RemoveTarget /////////////////////////////////////////////////////////////

void    plCubicRenderTargetModifier::RemoveTarget( plSceneObject *so )
{
    fTarget = nullptr;
}

//// Read /////////////////////////////////////////////////////////////////////

void    plCubicRenderTargetModifier::Read( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Read( s, mgr );

    plGenRefMsg* msg;
    msg = new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, 0 ); // SceneObject
    mgr->ReadKeyNotifyMe( s, msg, plRefFlags::kActiveRef );

    msg = new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, 0, 0 ); // cubicRT
    mgr->ReadKeyNotifyMe( s, msg, plRefFlags::kActiveRef );
}

//// Write ////////////////////////////////////////////////////////////////////

void    plCubicRenderTargetModifier::Write( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Write(s, mgr);

    mgr->WriteKey( s, fTarget ); // Write the SceneNode
    mgr->WriteKey( s, fCubic ); // Write the cubicRT
}
