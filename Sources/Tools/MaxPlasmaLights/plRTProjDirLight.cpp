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
//  plRTProjDirLight.cpp - Functions for the Projected Directional MAX light //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  8.2.2001 mcn - Created.                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plRealTimeLights.h"

#include "plRTProjDirLight.h"
#include "plRTProjDirLightClassDesc.h"
#include "MaxPlasmaMtls/Layers/plLayerTex.h"
#include "MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"
#include "MaxComponent/plMaxAnimUtils.h"
#include "plRTObjLightDesc.h"


//// Static ClassDesc2 Get Functions //////////////////////////////////////////

plRTProjDirLightDesc    plRTProjDirLightDesc::fStaticDesc;


//// Dialog Proc //////////////////////////////////////////////////////////////

class plProjDirDlgProc : public plBaseLightProc
{
    public:
        INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
        {
            IParamBlock2        *pb = map->GetParamBlock();
            plRTProjDirLight    *lite = (plRTProjDirLight *)pb->GetOwner();
            PBBitmap            *bitmap;
            plLayerTex          *layer = (plLayerTex *)lite->GetProjMap();
            ICustButton         *bmSelectBtn;


            switch( msg )
            {
                case WM_COMMAND:
                    if( LOWORD( wParam ) == IDC_PROJ_MAPNAME )
                    {
                        BOOL selectedNewBitmap = layer->HandleBitmapSelection();
                        if( selectedNewBitmap )
                        {
                            bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_PROJ_MAPNAME ) );
                            bitmap = layer->GetPBBitmap();
                            bmSelectBtn->SetText(bitmap != nullptr ? (MCHAR *)bitmap->bi.Filename() : _M(""));
                            ReleaseICustButton( bmSelectBtn );
                        }
                        return FALSE;
                    }
                    break;

                case WM_INITDIALOG:

                    // Set projection map bitmap name
                    bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_PROJ_MAPNAME ) );
                    if (bmSelectBtn != nullptr)
                    {
                        bitmap = (layer == nullptr) ? nullptr : layer->GetPBBitmap();
                        if (bitmap != nullptr)
                            bmSelectBtn->SetText( (MCHAR *)bitmap->bi.Filename() );
                        else
                            bmSelectBtn->SetText( _M( "<none>" ) );
                        ReleaseICustButton( bmSelectBtn );
                    }

                    break;
            }

            return plBaseLightProc::DlgProc( t, map, hWnd, msg, wParam, lParam );
        }
        void DeleteThis() override { }
};
static plProjDirDlgProc     gPPDirLiteDlgProc;

/// Include for the ParamBlock2 definition

#include "plRTProjDirLightPBDec.h"


///////////////////////////////////////////////////////////////////////////////
//// plRTProjPBAccessor Class Functions ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plRTProjPBAccessor  plRTProjPBAccessor::fAccessor;

void    plRTProjPBAccessor::Set( PB2Value& val, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t )
{
    plRTProjDirLight *obj = (plRTProjDirLight *)owner;
    IParamBlock2 *pb = obj->GetParamBlockByID( plRTProjDirLight::kBlkProj );
    
    switch( id )
    {
        case plRTProjDirLight::kProjMap:
/*          {
                if( pb->GetMap() )
                    pb->GetMap()->Invalidate( plRTProjDirLight::kProjMap );
                PBBitmap *thisMap = val.bm;
                obj->SetProjMap( &thisMap->bi );
            }
*/          break;
        default:
            break;
    }
}

void    plRTProjPBAccessor::Get( PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval &valid )
{
}

///////////////////////////////////////////////////////////////////////////////
//// Projected Directional Light //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plRTProjDirLight::plRTProjDirLight()
{
    fIP = nullptr;
    fLightPB = nullptr;
    fProjPB = nullptr;
    fClassDesc = plRTProjDirLightDesc::GetDesc();
    fClassDesc->MakeAutoParamBlocks( this );

    fLightPB->SetValue(kLightColor, 0,  Color(255,255,255));
    SetHSVColor(0, Point3(255, 255, 255));
    
    fTex = nullptr;
    meshBuilt = 0; 
    
    IBuildMeshes(true);
}

ObjLightDesc *plRTProjDirLight::CreateLightDesc( INode *n, BOOL forceShadowBuf )
{
    return new DirLight( n, forceShadowBuf );
}

RefTargetHandle plRTProjDirLight::Clone( RemapDir &remap )
{
    plRTProjDirLight *obj = new plRTProjDirLight;

    obj->ReplaceReference( kRefDirLight, fLightPB->Clone( remap ) );
    BaseClone( this, obj, remap );

    return obj;
}

//// SubAnim /////////////////////////////////////////////////////////////////

Animatable  *plRTProjDirLight::SubAnim( int i )
{
    switch( i )
    {           
        case 0:
            return (Animatable *)fLightPB;
        case 1:
            return (Animatable *)fProjPB;
        default: 
            return nullptr;
    }
}

//// SubAnimName /////////////////////////////////////////////////////////////

MSTR    plRTProjDirLight::SubAnimName( int i ) 
{ 
    switch( i ) 
    {   
        case 0:
            return fLightPB->GetLocalName();
        case 1:
            return fProjPB->GetLocalName();
        default:
            return _M("");
    }
}

//// GetReference ////////////////////////////////////////////////////////////

RefTargetHandle plRTProjDirLight::GetReference( int i )
{
    switch( i )
    {
        case kRefMainRollout:
            return (RefTargetHandle)fLightPB;
        case kRefProjRollout:
            return (RefTargetHandle)fProjPB;
        default:
            return plRTLightBase::GetReference( i );
    }
}       

//// SetReference ////////////////////////////////////////////////////////////

void    plRTProjDirLight::SetReference( int ref, RefTargetHandle rtarg )
{
    switch( ref )
    {       
        case kRefMainRollout:
            fLightPB = (IParamBlock2 *)rtarg; 
            break;
        case kRefProjRollout:
            fProjPB = (IParamBlock2 *)rtarg;
            break;

        default:
            plRTLightBase::SetReference( ref, rtarg );
            break;
    }   
}

//// ParamBlock Functions ////////////////////////////////////////////////////

int plRTProjDirLight::NumParamBlocks()
{
    return plRTLightBase::NumParamBlocks() + 1;
}

IParamBlock2    *plRTProjDirLight::GetParamBlock( int i )
{
    switch ( i )
    {
        case 2: return fProjPB;
        default: return plRTLightBase::GetParamBlock( i );
    }
}

IParamBlock2    *plRTProjDirLight::GetParamBlockByID( BlockID id )
{
    if( fProjPB->ID() == id )
        return fProjPB;
    else
        return plRTLightBase::GetParamBlockByID( id );
}

//// GetProjMap //////////////////////////////////////////////////////////////

Texmap  *plRTProjDirLight::GetProjMap() 
{ 
    // If we don't have one, create one
    plLayerTex  *layer = (plLayerTex *)fProjPB->GetTexmap( kTexmap, 0 );
    if (layer == nullptr || layer->ClassID() != LAYER_TEX_CLASS_ID)
    {
        layer = new plLayerTex;
        fProjPB->SetValue( kTexmap, 0, (Texmap *)layer );

        IParamBlock2 *bitmapPB = layer->GetParamBlockByID( plLayerTex::kBlkBitmap );
        bitmapPB->SetValue( kBmpUseBitmap, 0, 1 );
    }

    // Backwards compatability here
    PBBitmap    *bitmap = fProjPB->GetBitmap( kProjMap, 0 );
    if (bitmap != nullptr)
    {
        layer->SetBitmap( &bitmap->bi );
        fProjPB->SetValue(kProjMap, 0, (PBBitmap *)nullptr);
    }

    if( layer )
    {
        auto dbgTexName = layer->GetName();

        IParamBlock2 *bitmapPB = layer->GetParamBlockByID(plLayerTex::kBlkBitmap);
        hsAssert(bitmapPB, "LayerTex with no param block");

        int noCompress = fProjPB->GetInt(kProjNoCompress);
        int noMip = fProjPB->GetInt(kProjNoMip);
        bitmapPB->SetValue(kBmpNonCompressed, TimeValue(0), noCompress);
        bitmapPB->SetValue(kBmpNoFilter, TimeValue(0), noMip);
    }
    return (Texmap *)layer;
}
    
//// IBuildMeshes ////////////////////////////////////////////////////////////

void    plRTProjDirLight::IBuildMeshes( BOOL isnew ) 
{
    BuildStaticMeshes();

    fMesh = staticMesh[ plRTLightBase::RT_OMNI + 1 ];
}

//// GetLocalBoundBox ////////////////////////////////////////////////////////

void    plRTProjDirLight::GetLocalBoundBox( TimeValue t, INode *node, ViewExp *vpt, Box3 &box )
{
    Point3  loc = node->GetObjectTM( t ).GetTrans();
    float   scaleFactor = vpt->NonScalingObjectSize() * vpt->GetVPWorldWidth( loc ) / 360.0f;
    float   width, height, depth;

    box = fMesh.getBoundingBox();
    // Because we want to scale about the origin, not the box center, we have to do this funky offset
    Point3  boxCenter = box.Center();
    box.Translate( -boxCenter );
    box.Scale( scaleFactor );
    boxCenter *= scaleFactor;
    box.Translate( boxCenter );

    if( ( extDispFlags & EXT_DISP_ONLY_SELECTED ) )
    {
        fProjPB->GetValue( kWidth, t, width, FOREVER );
        fProjPB->GetValue( kHeight, t, height, FOREVER );
        fProjPB->GetValue( kRange, t, depth, FOREVER );
        width /= 2.f;
        height /= 2.f;

        box += Point3( -width, -height, 0.f );
        box += Point3( width, height, -depth );
    }
}

//// DrawConeAndLine /////////////////////////////////////////////////////////

int     plRTProjDirLight::DrawConeAndLine( TimeValue t, INode* inode, GraphicsWindow *gw, int drawing ) 
{
    Matrix3 tm = inode->GetObjectTM( t );
    gw->setTransform( tm );
    gw->clearHitCode();

    if( extDispFlags & EXT_DISP_ONLY_SELECTED )
        DrawCone( t, gw, 500 );

    return gw->checkHitCode();
}

//// DrawCone ////////////////////////////////////////////////////////////////
//  Function called by MAX to render the cone shape in the viewport for this
//  light. Note that this is the cone, not the actual object itself!

void    plRTProjDirLight::DrawCone( TimeValue t, GraphicsWindow *gw, float dist ) 
{
    Point3  nearPts[ 5 ], farPts[ 5 ], u[ 3 ];
    int     i;
    float   width, height;


    // Get values
    fProjPB->GetValue( kWidth, t, width, FOREVER );
    fProjPB->GetValue( kHeight, t, height, FOREVER );
    fProjPB->GetValue( kRange, t, dist, FOREVER );

    // Draw a rectangle showing the extents of the light (width, height and depth)
    IBuildRectangle( width, height, 0, nearPts );
    IBuildRectangle( width, height, -dist, farPts );

    gw->setColor( LINE_COLOR, GetUIColor( COLOR_HOTSPOT ) );

    gw->polyline(4, nearPts, nullptr, nullptr, true, nullptr);
    gw->polyline(4, farPts, nullptr, nullptr, true, nullptr);
    for( i = 0; i < 4; i++ )
    {
        u[ 0 ] = nearPts[ i ];
        u[ 1 ] = farPts[ i ];
        gw->polyline(2, u, nullptr, nullptr, false, nullptr);
    }
}

//// IBuildRectangle /////////////////////////////////////////////////////////

void    plRTProjDirLight::IBuildRectangle( float width, float height, float z, Point3 *pts )
{
    width /= 2.f;
    height /= 2.f;

    pts[ 0 ] = Point3( -width, -height, z );
    pts[ 1 ] = Point3( width, -height, z );
    pts[ 2 ] = Point3( width, height, z );
    pts[ 3 ] = Point3( -width, height, z );
}

//// GetFallsize /////////////////////////////////////////////////////////////
//  To get using-light-as-camera-viewport to work

float   plRTProjDirLight::GetFallsize( TimeValue t, Interval &valid )
{
    if (fProjPB == nullptr)
        return -1.f;

    return fProjPB->GetFloat( kWidth, t );
}

//// GetAspect ///////////////////////////////////////////////////////////////
//  To get using-light-as-camera-viewport to work

float   plRTProjDirLight::GetAspect( TimeValue t, Interval &valid )
{
    if (fProjPB == nullptr)
        return -1.f;

    return( fProjPB->GetFloat( kHeight, t ) / fProjPB->GetFloat( kWidth, t ) );
}

//// GetTDist ////////////////////////////////////////////////////////////////
//  To get using-light-as-camera-viewport to work

float   plRTProjDirLight::GetTDist( TimeValue t, Interval &valid )
{
    return 0.f;
    if (fProjPB == nullptr)
        return -1.f;

    return fProjPB->GetFloat( kRange, t );
}

//// SetFallsize /////////////////////////////////////////////////////////////
//  To get using-light-as-camera-viewport to work

void    plRTProjDirLight::SetFallsize( TimeValue time, float f )
{
    if (fProjPB != nullptr)
        fProjPB->SetValue( kWidth, time, f );
}

//// EvalLightState //////////////////////////////////////////////////////////
//  To get using-light-as-camera-viewport to work

RefResult plRTProjDirLight::EvalLightState( TimeValue t, Interval& valid, LightState *ls )
{
    ls->type = DIRECT_LGT;
    if( fLightPB->GetInt( kLightOn, t ) )
        ls->color = GetRGBColor( t, valid );
    else
        ls->color = Color( 0, 0, 0 );
    ls->on = fLightPB->GetInt( kLightOn, t );
    ls->intens = GetIntensity( t, valid );
    
    float   fall = fProjPB->GetFloat( kWidth, t );
    if( fall < fProjPB->GetFloat( kHeight, t ) )
        fall = fProjPB->GetFloat( kHeight, t );

    ls->hotsize = ls->fallsize = fall / 2.f;
    ls->useNearAtten = false;
    ls->useAtten = false;
    ls->shape = RECT_LIGHT;

    ls->aspect = fProjPB->GetFloat( kHeight, t ) / fProjPB->GetFloat( kWidth, t );
    ls->overshoot = false;
    ls->shadow = false;
    ls->ambientOnly = false;
    ls->affectDiffuse = fLightPB->GetInt( kAffectDiffuse, t );
    ls->affectSpecular = fLightPB->GetInt( kSpec, t );

    return REF_SUCCEED;
}
