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
//  pfGUIControlMod Definition                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIControlMod.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsGeometry3.h"
#include "hsMath.h"
#include "plPipeline.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "pfGameGUIMgr.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogMod.h"
#include "pfGUIListElement.h"   // Includes dropTargetProc
#include "pfGUIPopUpMenu.h"     // For skin, can we move that please? Thank you

#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "pnSceneObject/plAudioInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"

#include "plDrawable/plAccessGeometry.h"
#include "plDrawable/plAccessSpan.h"
#include "plDrawable/plAccessVtxSpan.h"
#include "plGImage/plDynamicTextMap.h"
#include "plMessage/plDeviceRecreateMsg.h"
#include "plMessage/plRenderMsg.h"
#include "plSurface/plLayer.h"

//// pfGUIColorScheme Functions //////////////////////////////////////////////

void    pfGUIColorScheme::IReset()
{
    fForeColor.Set( 1, 1, 1, 1 );
    fBackColor.Set( 0, 0, 0, 1 );
    fSelForeColor.Set( 1, 1, 1, 1 );
    fSelBackColor.Set( 0, 0, 1, 1 );
    fTransparent = false;
    fFontFace = "Times New Roman";
    fFontSize = 10;
    fFontFlags = 0;
}

pfGUIColorScheme::pfGUIColorScheme()
{
    IReset();
}

pfGUIColorScheme::pfGUIColorScheme( hsColorRGBA &foreColor, hsColorRGBA &backColor )
{
    IReset();
    fForeColor = foreColor;
    fBackColor = backColor;
}

pfGUIColorScheme::pfGUIColorScheme( const ST::string &face, uint8_t size, uint8_t fontFlags )
{
    IReset();
    fFontFace = face;
    fFontSize = size;
    fFontFlags = fontFlags;
}

void    pfGUIColorScheme::Read( hsStream *s )
{
    fForeColor.Read( s );
    fBackColor.Read( s );
    fSelForeColor.Read( s );
    fSelBackColor.Read( s );
    fTransparent = s->ReadBOOL();

    fFontFace = s->ReadSafeString();
    s->ReadByte(&fFontSize);
    s->ReadByte(&fFontFlags);
}

void    pfGUIColorScheme::Write( hsStream *s )
{
    fForeColor.Write( s );
    fBackColor.Write( s );
    fSelForeColor.Write( s );
    fSelBackColor.Write( s );
    s->WriteBOOL( fTransparent );

    s->WriteSafeString( fFontFace );
    s->WriteByte(fFontSize);
    s->WriteByte(fFontFlags);
}

//// Destructor //////////////////////////////////////////////////

pfGUIControlMod::~pfGUIControlMod()
{
    ISetHandler(nullptr);
    SetDropTargetHdlr(nullptr);
    SetColorScheme(nullptr);
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIControlMod::IEval( double secs, float del, uint32_t dirty )
{
//  UpdateBounds();
    return false;
}

//// GetBounds ///////////////////////////////////////////////////////////////

const hsBounds3 &pfGUIControlMod::GetBounds()
{
    UpdateBounds();
    return fBounds; 
}

//// SetTransform ////////////////////////////////////////////////////////////
//  Override from plModifier so we can update our bounds

void    pfGUIControlMod::SetTransform( const hsMatrix44 &l2w, const hsMatrix44 &w2l )
{
    fBoundsValid = false;
}


//// GetVectorAngle //////////////////////////////////////////////////////////

static float GetVectorAngle( const hsPoint3 &basePt, const hsPoint3 &pointA, const hsPoint3 &pointB )
{
    hsVector3   vectorA( &pointA, &basePt ), vectorB( &pointB, &basePt );

    float dot = vectorA * vectorB;
    hsVector3 cross = vectorA % vectorB;
    float crossLen = cross.fZ;

    return atan2( crossLen, dot );
}

//// CreateConvexHull ////////////////////////////////////////////////////////
//  Algorithm is Graham's scan algorithm:
//          R.L. Graham, "An efficient algorithm for determining the convex hull of a finite 
//                      planar set", Info. Proc. Lett. 1, 132-133 (1972). 
//  Note: THIS WILL DESTROY YOUR INPOINTS ARRAY.

static bool     CreateConvexHull( hsPoint3 *inPoints, int &numPoints )
{
    int         i, j, pointA, pointB, pointC;
    float    *angles;

    if( numPoints < 3 )
        return false;

    // Step 1: Find a point interior to our hull. Easiest is average of all our input points...
    // (plus: set the Zs of all the points to the Z of the first point, since we want to be
    //  working in 2D)
    hsPoint3    avgPoint = inPoints[ 0 ];
    for( i = 1; i < numPoints; i++ )
    {
        avgPoint += inPoints[ i ];
        inPoints[ i ].fZ = inPoints[ 0 ].fZ;
    }
    avgPoint.fX /= numPoints;
    avgPoint.fY /= numPoints;
    avgPoint.fZ /= numPoints;

    // Step 2: Sort all the in points by the angle to the X axis (vector <1,0>).
    //   Step A: Calculate all the angles

    angles = new float[ numPoints ];
    hsPoint3    xAxisPoint( avgPoint.fX + 1, avgPoint.fY, avgPoint.fZ );
    for( i = 0; i < numPoints; i++ )
        angles[ i ] = GetVectorAngle( avgPoint, inPoints[ i ], xAxisPoint );

    //   Step B: Bubble sort by the angles
    for( i = 0; i < numPoints - 1; i++ )
    {
        for( j = i + 1; j < numPoints; j++ )
        {
            if( angles[ j ] < angles[ i ] )
            {
                float tempAngle = angles[ j ];
                angles[ j ] = angles[ i ];
                angles[ i ] = tempAngle;

                hsPoint3 tempPt = inPoints[ j ];
                inPoints[ j ] = inPoints[ i ];
                inPoints[ i ] = tempPt;
            }
        }
    }

    // Step 3: Eliminate non-convex points to form the hull
    for( pointA = 0, pointB = 1, pointC = 2; pointA < numPoints && numPoints > 3; )
    {
        // Two cases of wrap-around...
        if( pointC >= numPoints )
            pointC -= numPoints;
        else if( pointC < 0 )
            pointC += numPoints;
        if( pointB >= numPoints )
            pointB -= numPoints;
        else if( pointB < 0 )
            pointB += numPoints;

        // For points A, B, and C, find the interior angle between them
        float angle = GetVectorAngle( inPoints[ pointB ], inPoints[ pointA ], inPoints[ pointC ] );
        
        // If the angle is < 180, then it's a good angle and we can advance all our points by 1...
        // Note: we have a tolerance so that we don't get points that form edges that are pretty darned close...
        constexpr float tolerance = hsConstants::pi<float> / 90.f;
        if (angle > tolerance && angle < hsConstants::pi<float> - tolerance)
        {
            pointA++;
            pointB++;
            pointC++;
        }
        else
        {
            // Angle is > 180 degrees, this is bad. This means our middle point doesn't belong,
            // so we need to remove it
            for( i = pointB; i < numPoints - 1; i++ )
                inPoints[ i ] = inPoints[ i + 1 ];
            numPoints--;
            if( pointC > pointB )
                pointC--;
            // There's one case where point B and C could've wrapped around and so deleting that point
            // actually moves point A down by 1...
            if( pointA > pointB )
                pointA--;

            // Back up the points by 1 if possible (so we can keep checking to make sure we're still convex).
            // If not, just increment C up
            if( pointA > 0 )
            {
                pointA--;
                pointB--;
            }
            else
                pointC++;
        }
    }

    delete [] angles;

    return true;
}

//// GetObjectPoints /////////////////////////////////////////////////////////
//  Retrieves ALL of the points of a sceneObject's meshes. And I mean ALL of 
//  'em...

static void GetObjectPoints(plSceneObject *so, std::vector<hsPoint3> &outPoints)
{
    const plDrawInterface* di = so->GetDrawInterface();
    if( !di )
        return;

    // The following uses mf's spiffy plAccessGeometry/Spans stuff, which, in
    // one word, kicksAss.
    std::vector<plAccessSpan> spans;
    plAccessGeometry::Instance()->OpenRO( di, spans );

    outPoints.clear();
    for (const plAccessSpan& span : spans)
    {
        const plAccessVtxSpan& vtxSrc = span.AccessVtx();
        plAccPositionIterator iterSrc( &vtxSrc );

        for( iterSrc.Begin(); iterSrc.More(); iterSrc.Advance() )
            outPoints.emplace_back(*iterSrc.Position());
    }
    
    if (plAccessGeometry::Instance())
        plAccessGeometry::Instance()->Close( spans );
}
    
//// PointsOnSameSide ////////////////////////////////////////////////////////
//  Given two ends of a line segment and two points, tells you whether the
//  two points are on the same side of the line. Used in PointInTriangle().

static bool     PointsOnSameSide( const hsPoint3 &line1, const hsPoint3 &line2, const hsPoint3 &pointA, const hsPoint3 &pointB )
{
    hsVector3 baseVec( &line2, &line1 );
    hsVector3   cp1 = hsVector3( &pointA, &line1 ) % baseVec;
    hsVector3   cp2 = hsVector3( &pointB, &line1 ) % baseVec;
    return ( cp1.fZ * cp2.fZ > 0 ) ? true : false;
}

//// PointInTriangle /////////////////////////////////////////////////////////
//  Given three points that define a triangle and a fourth point, tells you
//  whether the fourth point is inside the triangle.

static bool     PointInTriangle( hsPoint3 tri1, hsPoint3 tri2, hsPoint3 tri3, const hsPoint3 &testPoint )
{
    tri1.fZ = tri2.fZ = tri3.fZ = testPoint.fZ;
    if( PointsOnSameSide( tri1, tri2, testPoint, tri3 ) &&
        PointsOnSameSide( tri2, tri3, testPoint, tri1 ) &&
        PointsOnSameSide( tri3, tri1, testPoint, tri2 ) )
        return true;
    return false;
}

//// PointInBounds ///////////////////////////////////////////////////////////
//  Tells you whether said point is in the control's bounds.

bool    pfGUIControlMod::PointInBounds( const hsPoint3 &point )
{
    UpdateBounds();

    if( fBounds.GetType() != kBoundsEmpty && fBounds.GetType() != kBoundsUninitialized && fBounds.IsInside( &point ) )
    {
        if (!fBoundsPoints.empty())
        {
            // We have a more-accurate bounds set, so use it
            for (size_t i = 1; i < fBoundsPoints.size() - 1; i++)
            {
                // Test the triangle (0,i,i+1)
                if( PointInTriangle( fBoundsPoints[ 0 ], fBoundsPoints[ i ], fBoundsPoints[ i + 1 ], point ) )
                    return true;
            }
            return false;
        }
        else
            return true;
    }
    return false;
}

//// CalcInitialBounds ///////////////////////////////////////////////////////
//  Called by the dialog once as soon as the dialog adds the control, so that
//  initial bounds for the control can be calced. This is used for initing
//  any dynmaic text maps, since we want to use the initial bounds to do so
//  instead of any currently animated state of the bounds.

void    pfGUIControlMod::CalcInitialBounds()
{
    UpdateBounds(nullptr, true);
    fInitialBounds = fBounds;
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void    pfGUIControlMod::UpdateBounds( hsMatrix44 *invXformMatrix, bool force )
{
    hsMatrix44  xformMatrix, projMatrix;
    hsPoint3    corners[ 8 ];

    if( ( !fBoundsValid || force ) && fDialog && GetTarget() )
    {
        plDrawInterface *DI = IGetTargetDrawInterface( 0 );
        if (DI == nullptr)
            return;

        if( HasFlag( kBetterHitTesting ) )
        {
            std::vector<hsPoint3> scrnPoints;

            // Create a list of points to make a 2D convex hull from
            GetObjectPoints( GetTarget(), scrnPoints );
            hsMatrix44 l2w = GetTarget()->GetLocalToWorld();
            for (hsPoint3& point : scrnPoints)
            {
                point = l2w * point;
                point = fDialog->WorldToScreenPoint(point);
            }

            // Now create a convex hull from them, assuming the Zs are all the same
            int numPoints = (int)scrnPoints.size();
            if (!CreateConvexHull(scrnPoints.data(), numPoints))
                return;

            // Copy & store. Also recalc our bounding box just for fun
            fBounds.MakeEmpty();
            fBoundsPoints.resize(numPoints);
            for (int i = 0; i < numPoints; i++)
            {
                fBoundsPoints[ i ] = scrnPoints[ i ];
                fBounds.Union( &fBoundsPoints[ i ] );
            }
        }
        else
        {
            fBounds.MakeEmpty();

            hsBounds3Ext worldBounds = DI->GetLocalBounds();
            hsMatrix44 l2w = GetTarget()->GetLocalToWorld();
            worldBounds.Transform( &l2w );

            worldBounds.GetCorners( corners );
            for (int i = 0; i < 8; i++)
            {
                hsPoint3 scrnPt = fDialog->WorldToScreenPoint( corners[ i ] );
                fBounds.Union( &scrnPt );
            }
        }

        // Calc center Z
//      if( !fCenterValid )
        {
#if 0
            corners[ 1 ] = GetTarget()->GetLocalToWorld().GetTranslate();
            float w = corners[ 1 ].fX * fXformMatrix.fMap[3][0]
                    + corners[ 1 ].fY * fXformMatrix.fMap[3][1]
                    + corners[ 1 ].fZ * fXformMatrix.fMap[3][2]
                    + 1.f * fXformMatrix.fMap[3][3];
            corners[ 1 ] = fXformMatrix * corners[ 1 ];

            corners[ 1 ].fX = ( ( corners[ 1 ].fX / corners[ 1 ].fZ ) + 1.f ) / 2.f;
            corners[ 1 ].fY = ( ( corners[ 1 ].fY / corners[ 1 ].fZ ) + 1.f ) / 2.f;
            fScreenCenter = corners[ 1 ];

//          fScreenCenter.fZ = w;


            corners[ 1 ] = GetTarget()->GetLocalToWorld().GetTranslate();
            fDialog->WorldToScreenPoint( corners[ 1 ].fX, corners[ 1 ].fY, corners[ 1 ].fZ, fScreenCenter );
            fCenterValid = true;
#else
            corners[ 1 ] = GetTarget()->GetLocalToWorld().GetTranslate();
            fScreenCenter = fDialog->WorldToScreenPoint( corners[ 1 ] );
            corners[ 1 ] = fScreenCenter;
            fCenterValid = true;
#endif
        }

        fScreenMinZ = fBounds.GetMins().fZ;

        // Manually change the bounds so we know the z ranges from at least -1 to 1, suitable for us testing against for clicks
        corners[ 0 ] = fBounds.GetCenter();
        corners[ 0 ].fZ = -1.f;
        fBounds.Union( &corners[ 0 ] );
        corners[ 0 ].fZ = 1.f;
        fBounds.Union( &corners[ 0 ] );

        fBoundsValid = true;
    }
}

//// SetObjectCenter /////////////////////////////////////////////////////////
//  Given the x/y coordinates in 0..1 space, recalcs the sceneObject position
//  and moves the object to match, retaining the stored fCenterZ coordinate

void    pfGUIControlMod::SetObjectCenter( float x, float y )
{
    hsMatrix44  xformMatrix, l2p, p2l;
    hsPoint3    center;


    if( x > 1.f )
        x = 1.f;
    else if( x < 0.f )
        x = 0.f;
    if( y > 1.f )
        y = 1.f;
    else if( y < 0.f )
        y = 0.f;
    
    if( fDialog && GetTarget() )
    {
        plCoordinateInterface *CI = IGetTargetCoordinateInterface( 0 );
        if (CI == nullptr)
            return;

//      if( !fInvXformValid )
//          UpdateBounds();

        l2p = GetTarget()->GetLocalToWorld();
hsPoint3 oldPt = l2p.GetTranslate();

hsPoint3 oldScrnPt = fDialog->WorldToScreenPoint( oldPt );
hsPoint3 oldPtRedux;
fDialog->ScreenToWorldPoint( oldScrnPt.fX, oldScrnPt.fY, oldScrnPt.fZ, oldPtRedux );

        fDialog->ScreenToWorldPoint( x, y, fScreenCenter.fZ, center );

        l2p.SetTranslate( &center );
        l2p.GetInverse( &p2l );

        GetTarget()->SetTransform( l2p, p2l );

        fScreenCenter.fX = x;
        fScreenCenter.fY = y;
    }
}

void    pfGUIControlMod::SetTarget( plSceneObject *object )
{
    plSingleModifier::SetTarget( object );

    UpdateBounds();
}

//// MsgReceive //////////////////////////////////////////////////////////////

#include "plProfile.h"
plProfile_CreateTimer("Gui", "RenderSetup", GUITime);

bool    pfGUIControlMod::MsgReceive( plMessage *msg )
{
    plRenderMsg* rend = plRenderMsg::ConvertNoRef( msg );
    plDeviceRecreateMsg* device = plDeviceRecreateMsg::ConvertNoRef(msg);
    if (rend || device) {
        plPipeline* pipe = rend ? rend->Pipeline() : device->Pipeline();

        plProfile_BeginLap(GUITime, this->GetKey()->GetUoid().GetObjectName());
        ISetUpDynTextMap(pipe);
        plProfile_EndLap(GUITime, this->GetKey()->GetUoid().GetObjectName());

        if (rend)
            plgDispatch::Dispatch()->UnRegisterForExactType(plRenderMsg::Index(), GetKey());
        return true;
    }

    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if (refMsg != nullptr)
    {
        if( refMsg->fType == kRefDynTextMap )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fDynTextMap = plDynamicTextMap::ConvertNoRef( refMsg->GetRef() );

                // These tell us when we need to (re-)initialize the DTM
                plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), GetKey() );
                plgDispatch::Dispatch()->RegisterForExactType( plDeviceRecreateMsg::Index(), GetKey() );
            }
            else
            {
                fDynTextMap = nullptr;
                plgDispatch::Dispatch()->UnRegisterForExactType( plDeviceRecreateMsg::Index(), GetKey() );
            }
            return true;
        }
        else if( refMsg->fType == kRefDynTextLayer )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fDynTextLayer = plLayerInterface::ConvertNoRef( refMsg->GetRef() );
            else
                fDynTextLayer = nullptr;
            return true;
        }
        else if( refMsg->fType == kRefProxy )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fProxy = plSceneObject::ConvertNoRef( refMsg->GetRef() );
            else
                fProxy = nullptr;
            return true;
        }
        else if( refMsg->fType == kRefSkin )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
                fSkin = pfGUISkin::ConvertNoRef( refMsg->GetRef() );
            else
                fSkin = nullptr;

            return true;
        }
    }

    return plSingleModifier::MsgReceive( msg );
}

//// ISetUpDynTextMap ////////////////////////////////////////////////////////
//  Given a pointer to a dynamic text map, regurgitates it so it matches our
//  screen res and fun stuff like that. Also sets the layer transform to give
//  us a 1:1 textel-pixel ratio, which we like.

bool    pfGUIControlMod::ISetUpDynTextMap( plPipeline *pipe )
{
    if (fDynTextMap == nullptr)
    {
        hsAssert( false, "Trying to set up a nil dynamicTextMap in a GUI control" );
        return true;
    }
    if (fDynTextLayer == nullptr || fInitialBounds.GetType() == kBoundsUninitialized)//|| fDialog == nullptr)
        return false;

    uint32_t scrnWidth, scrnHeight;
    if( !HasFlag( kScaleTextWithResolution ) )
    {
        // Scale so that there is a 1:1 pixel:textel ratio
        scrnWidth = pipe->Width();
        scrnHeight = pipe->Height();
    }
    else
    {
        // Scale with the resolution so that we take up the same % of screen space no matter what resolution
        // Assume a base "resolution" of 1024xX, where X is such that the ratio "1024/X = scrnWidth/scrnHt" holds
        const int kBaseScaleRes = 1024;
        const int kBaseScaleHeightRes = 768;
        scrnWidth = kBaseScaleRes;
        scrnHeight = kBaseScaleHeightRes;
        // we are going to just force things to be in 4 by 3 ratio...
        // ...cause it seems to work better.
///////     scrnHeight = ( pipe->Height() * kBaseScaleRes ) / pipe->Width();
    }

    const hsBounds3 &bounds = fInitialBounds;//GetBounds();
    uint16_t width = (uint16_t)(( bounds.GetMaxs().fX - bounds.GetMins().fX ) * scrnWidth);
    uint16_t height = (uint16_t)(( bounds.GetMaxs().fY - bounds.GetMins().fY ) * scrnHeight);

    // Allow derived controls to allocate some extra scratch space if desired
    // (Do it this way so we can pass in our current calculated dimensions for them to play with)
    uint16_t extraW = width, extraH = height;
    IGrowDTMDimsToDesiredSize( extraW, extraH );
    extraW -= width;
    extraH -= height;

    fDynTextMap->Reset();
    fDynTextMap->Create( width, height, HasFlag( kXparentBgnd ), extraW, extraH, true );

    fDynTextMap->SetFont( GetColorScheme()->fFontFace, GetColorScheme()->fFontSize, GetColorScheme()->fFontFlags,
                            HasFlag( kXparentBgnd ) ? false : true );
    fDynTextMap->SetTextColor( GetColorScheme()->fForeColor, 
                            ( HasFlag( kXparentBgnd ) && GetColorScheme()->fBackColor.a == 0.f ) ? true : false );

    // Now we gotta set the texture transform on the layer so our texture comes
    // out with 1:1 mapping from textel to pixel
    plLayer *layer = (plLayer *)fDynTextLayer;
    layer->SetTransform( fDynTextMap->GetLayerTransform() );
    layer->SetBlendFlags( layer->GetBlendFlags() | hsGMatState::kBlendAlphaPremultiplied );

    // Let the derived classes do their things
    IPostSetUpDynTextMap();

    // Do our first update
    IUpdate();

    return true;
}

//// Get/SetColorScheme //////////////////////////////////////////////////////

pfGUIColorScheme    *pfGUIControlMod::GetColorScheme() const
{
    if (fColorScheme == nullptr)
        return fDialog->GetColorScheme();

    return fColorScheme;
}

void    pfGUIControlMod::SetColorScheme( pfGUIColorScheme *newScheme )
{
    if (fColorScheme != nullptr)
    {
        hsRefCnt_SafeUnRef( fColorScheme );
        fColorScheme = nullptr;
    }

    fColorScheme = newScheme;
    if (fColorScheme != nullptr)
        hsRefCnt_SafeRef( fColorScheme );
}

//// SetDynTextMap ///////////////////////////////////////////////////////////
//  EXPORT ONLY

void    pfGUIControlMod::SetDynTextMap( plLayerInterface *layer, plDynamicTextMap *dynText )
{
    hsgResMgr::ResMgr()->AddViaNotify( layer->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, pfGUIControlMod::kRefDynTextLayer ), plRefFlags::kActiveRef );
    hsgResMgr::ResMgr()->AddViaNotify( dynText->GetKey(), new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, pfGUIControlMod::kRefDynTextMap ), plRefFlags::kActiveRef );
}

//// SetEnabled //////////////////////////////////////////////////////////////

void    pfGUIControlMod::SetEnabled( bool e )
{
    if( e == fEnabled )
        return;

    fEnabled = e;
    IUpdate();
}

//// SetFocused //////////////////////////////////////////////////////////////

void    pfGUIControlMod::SetFocused( bool e )
{
    if( e == fFocused )
        return;

    fFocused = e;
    IUpdate();
}

//// SetInteresting //////////////////////////////////////////////////////////

void    pfGUIControlMod::SetInteresting( bool i )
{
    if( i == fInteresting )
        return;

    fInteresting = i;
    IUpdate();

    if ( fNotifyOnInteresting && fDialog && fDialog->GetHandler() )
        fDialog->GetHandler()->OnInterestingEvent(this);

}

//// SetVisible //////////////////////////////////////////////////////////////

void    pfGUIControlMod::SetVisible( bool vis )
{
    if( vis == fVisible )
        return;

    fVisible = vis;
    if (fTarget)
    {
        plEnableMsg *msg = new plEnableMsg();
        msg->SetCmd( fVisible ? plEnableMsg::kEnable : plEnableMsg::kDisable );
        msg->SetCmd( plEnableMsg::kDrawable );
        msg->AddReceiver( fTarget->GetKey() );
        plgDispatch::MsgSend( msg );
    }

    if( !fVisible && fFocused )
        fDialog->SetFocus(nullptr);
}

void    pfGUIControlMod::Refresh()
{
    IUpdate();
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIControlMod::Read( hsStream *s, hsResMgr *mgr )
{
    plSingleModifier::Read(s, mgr);
    s->ReadLE32(&fTagID);
    fVisible = s->ReadBool();

    // Read the handler in
    ISetHandler( pfGUICtrlProcWriteableObject::Read( s ) );

    // Read in the dynTextMap if there is one
    if( s->ReadBool() )
    {
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDynTextLayer ), plRefFlags::kActiveRef );
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDynTextMap ), plRefFlags::kActiveRef );
    }
    else
    {
        fDynTextLayer = nullptr;
        fDynTextMap = nullptr;
    }

    if( s->ReadBool() )
    {
        SetColorScheme(nullptr);
        fColorScheme = new pfGUIColorScheme();
        fColorScheme->Read( s );
    }

    // Read in our sound indices
    uint8_t count = s->ReadByte();
    if( count == 0 )
        fSoundIndices.clear();
    else
    {
        fSoundIndices.assign(count, 0);
        for (uint8_t i = 0; i < count; i++)
            fSoundIndices[ i ] = (int)s->ReadLE32();
    }

    if( HasFlag( kHasProxy ) )
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefProxy ), plRefFlags::kActiveRef );

    mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );
}

void    pfGUIControlMod::Write( hsStream *s, hsResMgr *mgr )
{
    if( HasFlag( kHasProxy ) && !fProxy )
        ClearFlag( kHasProxy );

    plSingleModifier::Write( s, mgr );
    s->WriteLE32(fTagID);
    s->WriteBool( fVisible );

    // Write the handler out (if it's not a writeable, damn you)
    pfGUICtrlProcWriteableObject::Write( (pfGUICtrlProcWriteableObject *)fHandler, s );

    // Write out the dynTextMap
    if (fDynTextMap != nullptr)
    {
        s->WriteBool( true );
        mgr->WriteKey( s, fDynTextLayer->GetKey() );
        mgr->WriteKey( s, fDynTextMap->GetKey() );
    }
    else
        s->WriteBool( false );

    if (fColorScheme != nullptr)
    {
        s->WriteBool( true );
        fColorScheme->Write( s );
    }
    else
        s->WriteBool( false );

    // Write out our sound indices
    s->WriteByte((uint8_t)fSoundIndices.size());
    for (int idx : fSoundIndices)
        s->WriteLE32(idx);

    if( HasFlag( kHasProxy ) )
        mgr->WriteKey( s, fProxy->GetKey() );

    mgr->WriteKey( s, fSkin );
}

//// HandleKeyPress/Event ////////////////////////////////////////////////////

bool    pfGUIControlMod::HandleKeyPress( wchar_t key, uint8_t modifiers ) 
{
    return false; 
}

bool    pfGUIControlMod::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers ) 
{
    return false; 
}

//// IScreenToLocalPt ////////////////////////////////////////////////////////

void    pfGUIControlMod::IScreenToLocalPt( hsPoint3 &pt )
{
    const hsBounds3 &bnds = GetBounds();

    pt.fX -= bnds.GetMins().fX;
    pt.fY -= bnds.GetMins().fY;
    pt.fX /= bnds.GetMaxs().fX - bnds.GetMins().fX;
    pt.fY /= bnds.GetMaxs().fY - bnds.GetMins().fY;
}

//// ISetHandler /////////////////////////////////////////////////////////////

void    pfGUIControlMod::ISetHandler( pfGUICtrlProcObject *h, bool clearInheritFlag )
{
    if( fHandler && fHandler->DecRef() )
        delete fHandler;

    fHandler = h; 
    if( fHandler )
        fHandler->IncRef();

    if( clearInheritFlag )
        ClearFlag( kInheritProcFromDlg );
}

//// DoSomething /////////////////////////////////////////////////////////////

void    pfGUIControlMod::DoSomething()
{
    if (fEnabled && fHandler != nullptr)
        fHandler->DoSomething( this );
}

//// HandleExtendedEvent /////////////////////////////////////////////////////

void    pfGUIControlMod::HandleExtendedEvent( uint32_t event )
{
    if (fEnabled && fHandler != nullptr)
        fHandler->HandleExtendedEvent( this, event );
}

//// SetDropTargetHdlr ///////////////////////////////////////////////////////

void    pfGUIControlMod::SetDropTargetHdlr( pfGUIDropTargetProc *h )
{
    if( fDropTargetHdlr && fDropTargetHdlr->DecRef() )
        delete fDropTargetHdlr;

    fDropTargetHdlr = h; 
    if( fDropTargetHdlr )
        fDropTargetHdlr->IncRef();
}

//// SetSoundIndex ///////////////////////////////////////////////////////////
//  Associates the given GUI event with an index of a sound on the target SO's
//  audioInterface. The guiCtrlEvent is specific to each type of control.

void    pfGUIControlMod::SetSoundIndex( uint8_t guiCtrlEvent, int soundIndex )
{
    if (fSoundIndices.size() < guiCtrlEvent + 1)
        fSoundIndices.resize(guiCtrlEvent + 1);

    fSoundIndices[ guiCtrlEvent ] = soundIndex + 1; // We +1, since 0 means no sound
}

//// IPlaySound //////////////////////////////////////////////////////////////
//  Sends a sound play message with the soundIndex associated with the given
//  event.

void    pfGUIControlMod::IPlaySound( uint8_t guiCtrlEvent, bool loop /* = false */ )
{
    if (guiCtrlEvent >= fSoundIndices.size() || fSoundIndices[guiCtrlEvent] == 0)
        return;

    if (GetTarget() == nullptr || GetTarget()->GetAudioInterface() == nullptr)
        return;

    plSoundMsg  *msg = new plSoundMsg;
    msg->fIndex = fSoundIndices[ guiCtrlEvent ] - 1;
    msg->SetCmd( plSoundMsg::kGoToTime );
    msg->fTime = 0.f;
    msg->SetCmd( plSoundMsg::kPlay );
    if (loop)
    {
        msg->fLoop = true;
        msg->SetCmd( plSoundMsg::kSetLooping );
    }
    msg->Send( GetTarget()->GetAudioInterface()->GetKey() );
}

void    pfGUIControlMod::IStopSound(uint8_t guiCtrlEvent)
{
    if (guiCtrlEvent >= fSoundIndices.size() || fSoundIndices[guiCtrlEvent] == 0)
        return;

    if (GetTarget() == nullptr || GetTarget()->GetAudioInterface() == nullptr)
        return;

    plSoundMsg *msg = new plSoundMsg;
    msg->fIndex = fSoundIndices[guiCtrlEvent] - 1;
    msg->SetCmd(plSoundMsg::kStop);
    msg->Send(GetTarget()->GetAudioInterface()->GetKey());
}
