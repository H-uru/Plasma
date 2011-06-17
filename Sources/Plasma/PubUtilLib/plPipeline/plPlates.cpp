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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plPlates.cpp - Implementation of plates and plate manager               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "hsWindows.h"
#include "hsTypes.h"
#include "plPipeline.h"
#include "plPlates.h"

#include "plJPEG/plJPEG.h"
#include "plGImage/plPNG.h"
#include "plGImage/plMipmap.h"
#include "plSurface/plLayer.h"
#include "plSurface/hsGMaterial.h"
#include "plMessage/plLayRefMsg.h"
#include "pnMessage/plRefMsg.h"
#include "hsGDeviceRef.h"
#include "hsResMgr.h"
#include "plPipeDebugFlags.h"
#include "plClientResMgr/plClientResMgr.h"


// A bit of a hack so that we will have the correct instance in the SceneViewer
static HINSTANCE gHInstance = GetModuleHandle(nil);

void SetHInstance(void *instance)
{
    gHInstance = (HINSTANCE)instance;
}

//////////////////////////////////////////////////////////////////////////////
//// plPlate Functions ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

UInt32  plPlate::fMagicUniqueKeyInt  = 0;

plPlate::plPlate( plPlate **owningHandle )
{
    fXformMatrix.Reset();
    fDepth = 1.0f;
    fMaterial = nil;
    fFlags = 0;
    fOpacity = 1.f;

    fNext = nil;
    fPrevPtr = nil;
    fOwningHandle = owningHandle;
    fMipmap = nil;
    memset( fTitle, 0, sizeof( fTitle ) );
}

plPlate::~plPlate()
{
    if( fFlags & kFlagLocalMaterial )
        fMaterial->GetKey()->UnRefObject();
    else
    {
        hsRefCnt_SafeUnRef( fMaterial );
    }

    fMaterial = nil;
    *fOwningHandle = nil;
}

//// SetPosition /////////////////////////////////////////////////////////////

void    plPlate::SetPosition( hsScalar x, hsScalar y, hsScalar z )
{
    hsVector3   triple;


    if( z != -1.0f )
    {
        /// Gotta resort--let the manager do it
        plPlateManager::Instance().IResortPlate( this, ( z + 1.0f <= fDepth ) ? true : false );
        fDepth = z + 1.0f;
    }

    x *= fDepth / 1.0f;
    y *= fDepth / 1.0f;
    triple.fX = x;
    triple.fY = y;
    triple.fZ = fDepth;

    fXformMatrix.SetTranslate( &triple );
}

//// SetSize /////////////////////////////////////////////////////////////////

void    plPlate::SetSize( hsScalar width, hsScalar height, bool adjustByAspectRation )
{
    hsVector3   size;

    width *= fDepth / 1.0f;
    height *= fDepth / 1.0f;

    size.fX = adjustByAspectRation ? (width * ((hsScalar)plPlateManager::Instance().GetPipeHeight() / (hsScalar)plPlateManager::Instance().GetPipeWidth())) : width;
    size.fY = height;
    size.fZ = 1.0f;

    fXformMatrix.SetScale( &size );
}

//// SetTransform ////////////////////////////////////////////////////////////

void    plPlate::SetTransform( hsMatrix44 &matrix, hsBool reSort )
{
    fXformMatrix = matrix;
    if( reSort )
        plPlateManager::Instance().IResortPlate( this, false );
}

//// SetMaterial /////////////////////////////////////////////////////////////

void    plPlate::SetMaterial( hsGMaterial *material )
{
    hsRefCnt_SafeAssign( fMaterial, material );
}

void plPlate::SetTexture(plBitmap *texture)
{
    plLayer         *layer;
    hsGMaterial     *material;
    char            keyName[ 128 ];

    material = TRACKED_NEW hsGMaterial();
    sprintf( keyName, "PlateBlank#%d", fMagicUniqueKeyInt++ );
    hsgResMgr::ResMgr()->NewKey( keyName, material, plLocation::kGlobalFixedLoc );
    layer = material->MakeBaseLayer();
    layer->SetShadeFlags( layer->GetShadeFlags() | hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite | hsGMatState::kShadeReallyNoFog );
    layer->SetZFlags( layer->GetZFlags() | hsGMatState::kZNoZRead );
    layer->SetBlendFlags( layer->GetBlendFlags() | hsGMatState::kBlendAlpha );
    layer->SetOpacity( fOpacity );
    layer->SetUVWSrc(plLayerInterface::kUVWPassThru);

    hsgResMgr::ResMgr()->AddViaNotify(texture->GetKey(), TRACKED_NEW plGenRefMsg(layer->GetKey(), plRefMsg::kOnCreate, -1, plLayRefMsg::kTexture), plRefFlags::kActiveRef);

    SetMaterial(material);
}

//// SetOpacity //////////////////////////////////////////////////////////////

void    plPlate::SetOpacity( hsScalar opacity )
{
    if( fMaterial != nil && fMaterial->GetLayer( 0 ) != nil )
    {
        plLayer *layer = (plLayer *)fMaterial->GetLayer( 0 );
        layer->SetOpacity( opacity );
    }

    fOpacity = opacity;
}

//// CreateMaterial /////////////////////////////////////////////////////
//  Creates a new material for this plate with either a specified texture 
//  or an empty, white-filled bitmap.

plMipmap    *plPlate::CreateMaterial( UInt32 width, UInt32 height, hsBool withAlpha, plMipmap* texture )
{
    plLayer         *layer;
    hsGMaterial     *material;
    char            keyName[ 128 ];


    if (texture)
    {
        fMipmap = texture;
    }
    else
    {
        /// Create a new bitmap
        fMipmap = TRACKED_NEW plMipmap( width, height, withAlpha ? plMipmap::kARGB32Config : plMipmap::kRGB32Config, 1 );
        memset( fMipmap->GetImage(), 0xff, height * fMipmap->GetRowBytes() );
        sprintf( keyName, "PlateBitmap#%d", fMagicUniqueKeyInt++ );
        hsgResMgr::ResMgr()->NewKey( keyName, fMipmap, plLocation::kGlobalFixedLoc );
        fMipmap->SetFlags( fMipmap->GetFlags() | plMipmap::kDontThrowAwayImage );
    }

    /// NOW create a layer wrapper and a material for that layer
    material = TRACKED_NEW hsGMaterial();
    sprintf( keyName, "PlateBlank#%d", fMagicUniqueKeyInt++ );
    hsgResMgr::ResMgr()->NewKey( keyName, material, plLocation::kGlobalFixedLoc );
    layer = material->MakeBaseLayer();
    layer->SetShadeFlags( layer->GetShadeFlags() | hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite | hsGMatState::kShadeReallyNoFog );
    layer->SetZFlags( layer->GetZFlags() | hsGMatState::kZNoZRead );
    layer->SetBlendFlags( layer->GetBlendFlags() | hsGMatState::kBlendAlpha );
    layer->SetOpacity( fOpacity );

    hsgResMgr::ResMgr()->AddViaNotify( fMipmap->GetKey(), TRACKED_NEW plLayRefMsg( layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

    // Set up a ref to these. Since we don't have a key, we use the
    // generic RefObject() (and matching UnRefObject() when we're done).
    // If we had a key, we would use myKey->AddViaNotify(otherKey) and myKey->Release(otherKey).
    material->GetKey()->RefObject();

    /// Set this as our new material and return the bitmap
    fFlags |= kFlagLocalMaterial;
    fMaterial = material;
    return fMipmap;
}

//// CreateFromResource //////////////////////////////////////////////////////
//  Creates a plate's material from a resource of the given name.

void plPlate::CreateFromResource(const char *resName)
{
    if (resName)
    {
        plMipmap* resTexture = TRACKED_NEW plMipmap;
        resTexture->CopyFrom(plClientResMgr::Instance().getResource(resName));

        char keyName[128];
        sprintf( keyName, "PlateResource#%d", fMagicUniqueKeyInt++ );
        hsgResMgr::ResMgr()->NewKey(keyName, resTexture, plLocation::kGlobalFixedLoc);
        CreateMaterial(resTexture->GetWidth(), resTexture->GetHeight(), true, resTexture);
    }
    else
    {
        // Null resource request - Create a blank Material instead
        CreateMaterial(32, 32, true);
    }
}

void plPlate::ReloadFromResource(const char *resName)
{
    if (resName)
    {
        fMipmap->CopyFrom(plClientResMgr::Instance().getResource(resName));
    }
}

//// ILink ///////////////////////////////////////////////////////////////////
//  Links a plate into a plate list, but also sorts by decreasing depth,
//  so the plate won't actually necessarily be added after the pointer
//  given.

void    plPlate::ILink( plPlate **back )
{
    hsAssert( fNext == nil && fPrevPtr == nil, "Trying to link a plate that's already linked" );

    
    /// Advance back as far as we need to go
    while( *back != nil && (*back)->fDepth > fDepth )
        back = &( (*back)->fNext );

    /// Link!
    fNext = *back;
    if( *back )
        (*back)->fPrevPtr = &fNext;
    fPrevPtr = back;
    *back = this;
}

hsBool plPlate::IsVisible()
{
    // return not-visible if our material is not valid
    if (fMaterial->GetNumLayers() == 0)
        return false;
    plLayerInterface* layer = fMaterial->GetLayer(0);
    if (layer->GetTexture() == nil)
        return false;

    // cursory check of material indicates it's valid, return our visible flag status
    return ( fFlags & kFlagVisible ) ? true : false;
}


//////////////////////////////////////////////////////////////////////////////
//// plGraphPlate Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Constructor & Destructor ////////////////////////////////////////////////

plGraphPlate::plGraphPlate( plPlate **owningHandle ) : plPlate( owningHandle )
{
    fFlags |= kFlagIsAGraph;
    SetLabelText( nil );
}

plGraphPlate::~plGraphPlate()
{
}

//// IMakePow2 ///////////////////////////////////////////////////////////////

UInt32  plGraphPlate::IMakePow2( UInt32 value )
{
    int         i;


    for( i = 0; value != 0; i++, value >>= 1 );
    return 1 << i;
}

//// SetDataRange ////////////////////////////////////////////////////////////

void    plGraphPlate::SetDataRange( UInt32 min, UInt32 max, UInt32 width )
{
    UInt32      height;


    width = IMakePow2( width + 4 );
    height = IMakePow2( max - min + 1 + 4 );

    CreateMaterial( width, height, true );
    fMin = min;
    fMax = max;

    SetDataLabels( fMin, fMax );
    SetColors();
    SetDataColors();
    ClearData();
}

//// SetDataLabels ///////////////////////////////////////////////////////////

void    plGraphPlate::SetDataLabels( UInt32 min, UInt32 max )
{
    fLabelMin = min;
    fLabelMax = max;
}

//// ClearData ///////////////////////////////////////////////////////////////

void    plGraphPlate::ClearData( void )
{
    UInt32  *bits = (UInt32 *)fMipmap->GetImage(), *ptr;
    int     i;


    // Background color
    for( i = 0; i < fMipmap->GetWidth() * fMipmap->GetHeight(); bits[ i ] = fBGHexColor, i++ );

    // Axes
    ptr = bits + fMipmap->GetWidth();
    *ptr = fAxesHexColor;
    for( ptr++, i = 0; i < fMipmap->GetHeight() - 4; i++, ptr += fMipmap->GetWidth() )
        *ptr = fAxesHexColor;
    for( i = 0; i < fMipmap->GetWidth() - 4; ptr[ i ] = fAxesHexColor, i++ );
    ptr += fMipmap->GetWidth() - 1;
    ptr[ 0 ] = fAxesHexColor;
    ptr[ fMipmap->GetWidth() - 5 + 1 ] = fAxesHexColor;

    if( fMaterial->GetLayer( 0 ) != nil && fMaterial->GetLayer( 0 )->GetTexture() )
    {
        hsGDeviceRef    *ref = fMaterial->GetLayer( 0 )->GetTexture()->GetDeviceRef();
        if( ref != nil )
            ref->SetDirty( true );
    }
}

//// AddData /////////////////////////////////////////////////////////////////
//  Scroll graph data left by one, then add the new value

void    plGraphPlate::AddData( Int32 value, Int32 value2, Int32 value3, Int32 value4 )
{
    std::vector<Int32> values;
    if (value != -1)
        values.push_back(value);
    if (value2 != -1)
        values.push_back(value2);
    if (value3 != -1)
        values.push_back(value3);
    if (value4 != -1)
        values.push_back(value4);
    AddData(values);
}

void    plGraphPlate::AddData( std::vector<Int32> values )
{
    hsAssert( fMipmap != nil, "Trying to add data to an uninitialized plGraphPlate" );

    fMipmap->SetCurrLevel( 0 );

    UInt32  *bits = (UInt32 *)fMipmap->GetImage(), *ptr;
    UInt32  *minDPos = fMipmap->GetAddr32( 3, fMipmap->GetHeight() - 3 - 10 );
    UInt32  *maxDPos = fMipmap->GetAddr32( 3, 2 );
    int     i, j;
    std::vector<int> lows, his;
    float   lineCtr, lineInc;
    int     lastLineInt, lineInt, bumpCtr;

    // make sure we have enough colors
    if (values.size() > fDataHexColors.size())
    {
        for (i=fDataHexColors.size(); i<values.size(); i++)
            fDataHexColors.push_back(0xff00ff00); // make it a nice green color
    }

    // make sure we have enough "last values"
    if (values.size() > fLastValues.size())
    {
        for (i=fLastValues.size(); i<values.size(); i++)
            fLastValues.push_back(0);
    }
    
    // Scale values
    for (i=0; i<values.size(); i++)
    {
        lows.push_back(0);
        his.push_back(0);
        
        if( values[ i ] > fLabelMax )
            values[ i ] = fLabelMax;
        else if( values[ i ] < fLabelMin )
            values[ i ] = fLabelMin;
        values[ i ] = (UInt32)( (float)values[ i ] * ( fMipmap->GetHeight() - 4 ) / ( fLabelMax - fLabelMin + 1 ) );

        if( values[ i ] < fLastValues[ i ] )
        {
            lows[ i ] = values[ i ] - 1;
            his[ i ] = fLastValues[ i ];
        }
        else
        {
            lows[ i ] = fLastValues[ i ] - 1;
            his[ i ] = values[ i ];
        }
    }

    lineCtr = 0;
    lastLineInt = 0;
    bumpCtr = 0;
    lineInc = 8.0f / ( fMipmap->GetHeight() - 4 );
    IDrawNumber( fLabelMin, minDPos, fMipmap->GetWidth(), fBGHexColor );
    IDrawNumber( fLabelMax, maxDPos, fMipmap->GetWidth(), fBGHexColor );
    for( i = 0, ptr = bits + fMipmap->GetWidth() + 2, j = fMipmap->GetHeight() - 4; i < fMipmap->GetHeight() - 4; i++, j-- )
    {
        lineInt = (int)lineCtr;
        if( lineInt != lastLineInt )
            bumpCtr = 2;

        memmove( ptr, ptr + 1, ( fMipmap->GetWidth() - 5 ) * sizeof( UInt32 ) );
        int dataIndex;
        bool dataPlotted = false;
        for (dataIndex = 0; dataIndex < values.size(); dataIndex++)
        {
            if( j >= lows[ dataIndex ] && j <= his[ dataIndex ] )
            {
                ptr[ fMipmap->GetWidth() - 5 ] = fDataHexColors[ dataIndex ];
                dataPlotted = true;
                break;
            }
        }

        if (!dataPlotted)
        {
            if( bumpCtr > 0 )
            {
                if( lineInt == 4 )
                    ptr[ fMipmap->GetWidth() - 5 ] = fGraphHexColor | 0xff000000;
                else
                    ptr[ fMipmap->GetWidth() - 5 ] = fGraphHexColor;
                bumpCtr--;
            }
            else
                ptr[ fMipmap->GetWidth() - 5 ] = fBGHexColor;
        }

        ptr += fMipmap->GetWidth();

        lastLineInt = lineInt;
        lineCtr += lineInc;
    }
    IDrawNumber( fLabelMin, minDPos, fMipmap->GetWidth(), fAxesHexColor );
    IDrawNumber( fLabelMax, maxDPos, fMipmap->GetWidth(), fAxesHexColor );

    fLastValues = values;

    if( fMaterial->GetLayer( 0 ) != nil && fMaterial->GetLayer( 0 )->GetTexture() != nil )
    {
        hsGDeviceRef    *ref = fMaterial->GetLayer( 0 )->GetTexture()->GetDeviceRef();
        if( ref != nil )
            ref->SetDirty( true );
    }
}

//// SetColors ///////////////////////////////////////////////////////////////

void    plGraphPlate::SetColors( UInt32 bgHexColor, UInt32 axesHexColor, UInt32 dataHexColor, UInt32 graphHexColor )
{
    fBGHexColor = bgHexColor;
    fAxesHexColor = axesHexColor;
    if (fDataHexColors.size() == 0)
        fDataHexColors.push_back(dataHexColor);
    else
        fDataHexColors[ 0 ] = dataHexColor;
    fGraphHexColor = graphHexColor;

    ClearData();
}

//// SetDataColors ///////////////////////////////////////////////////////////

void    plGraphPlate::SetDataColors( UInt32 hexColor1, UInt32 hexColor2, UInt32 hexColor3, UInt32 hexColor4 )
{
    std::vector<UInt32> colors;
    colors.push_back(hexColor1);
    colors.push_back(hexColor2);
    colors.push_back(hexColor3);
    colors.push_back(hexColor4);
    SetDataColors(colors);
}

void    plGraphPlate::SetDataColors( const std::vector<UInt32> & hexColors )
{
    fDataHexColors = hexColors;
}

//// SetLabelText ////////////////////////////////////////////////////////////

void    plGraphPlate::SetLabelText( char *text1, char *text2, char *text3, char *text4 )
{
    std::vector<std::string> strings;
    if( text1 != nil )
        strings.push_back(text1);
    else
        strings.push_back("");

    if( text2 != nil )
        strings.push_back(text2);
    else
        strings.push_back("");

    if( text3 != nil )
        strings.push_back(text3);
    else
        strings.push_back("");

    if( text4 != nil )
        strings.push_back(text4);
    else
        strings.push_back("");
    SetLabelText(strings);
}

void    plGraphPlate::SetLabelText( const std::vector<std::string> & text )
{
    fLabelText = text;
}

//// IDrawNumber /////////////////////////////////////////////////////////////

void    plGraphPlate::IDrawNumber( UInt32 number, UInt32 *dataPtr, UInt32 stride, UInt32 color )
{
    char    str[ 16 ];
    int     i;


    sprintf( str, "%d", number );
    for( i = 0; str[ i ] != 0; i++ )
    {
        IDrawDigit( str[ i ] - '0', dataPtr, stride, color );
        dataPtr += 7;
    }
}

//// IDrawDigit //////////////////////////////////////////////////////////////

void    plGraphPlate::IDrawDigit( char digit, UInt32 *dataPtr, UInt32 stride, UInt32 color )
{
    /// Yes, I know this is ugly. Move it into another file if you like.
    char    digits[ 10 ][ 5 * 3 ] =
        { { 1,1,1,
            1,0,1,
            1,0,1,
            1,0,1,
            1,1,1 },
        { 0,1,0,
          1,1,0,
          0,1,0,
          0,1,0,
          1,1,1 },
        { 2,2,2,
          0,0,2,
          0,2,0,
          2,0,0,
          2,2,2 },
        { 3,3,3,
          0,0,3,
          3,3,3,
          0,0,3,
          3,3,3 },
        { 4,0,4,
          4,0,4,
          4,4,4,
          0,0,4,
          0,0,4 },
        { 5,5,5,
          5,0,0,
          5,5,5,
          0,0,5,
          5,5,5 },
        { 6,6,6,
          6,0,0,
          6,6,6,
          6,0,6,
          6,6,6 },
        { 7,7,7,
          0,0,7,
          0,0,7,
          0,0,7,
          0,0,7 },
        { 8,8,8,
          8,0,8,
          8,8,8,
          8,0,8,
          8,8,8 },
        { 9,9,9,
          9,0,9,
          9,9,9,
          0,0,9,
          0,0,9 } };

    
    char    *digData = digits[ digit ];
    int     i, j;


    for( i = 0; i < 5; i++ )
    {
        for( j = 0; j < 6; j += 2 )
        {
            if( *digData )
            {
                dataPtr[ j ] = color;
                dataPtr[ j + 1 ] = color;
                dataPtr[ j + stride ] = color;
                dataPtr[ j + stride + 1 ] = color;
            }
            digData++;
        }
        dataPtr += stride + stride;
    }
}


//////////////////////////////////////////////////////////////////////////////
//// plPlateManager Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plPlateManager  *plPlateManager::fInstance = nil;


//// Destructor  /////////////////////////////////////////////////////////////

plPlateManager::~plPlateManager()
{
    while( fPlates != nil )
        DestroyPlate( fPlates );

    fInstance = nil;
}

//// CreatePlate /////////////////////////////////////////////////////////////

void    plPlateManager::CreatePlate( plPlate **handle )
{
    plPlate     *plate = TRACKED_NEW plPlate( handle );


    plate->ILink( &fPlates );
    *handle = plate;
}

void    plPlateManager::CreatePlate( plPlate **handle, hsScalar width, hsScalar height )
{
    CreatePlate( handle );
    (*handle)->SetSize( width, height );
}

void    plPlateManager::CreatePlate( plPlate **handle, hsScalar x, hsScalar y, hsScalar width, hsScalar height )
{
    CreatePlate( handle );
    (*handle)->SetPosition( x, y );
    (*handle)->SetSize( width, height );
}

void    plPlateManager::CreateGraphPlate( plGraphPlate **handle )
{
    plGraphPlate    *plate = TRACKED_NEW plGraphPlate( (plPlate **)handle );


    plate->ILink( &fPlates );
    *handle = plate;
}

//// DestroyPlate ////////////////////////////////////////////////////////////

void    plPlateManager::DestroyPlate( plPlate *plate )
{
    if( plate != nil )
    {
        plate->IUnlink();
        delete plate;
    }
}

//// GetPipeWidth/Height /////////////////////////////////////////////////////

UInt32  plPlateManager::GetPipeWidth( void )
{
    return fOwner->Width();
}

UInt32  plPlateManager::GetPipeHeight( void )
{
    return fOwner->Height();
}

//// DrawToDevice ////////////////////////////////////////////////////////////

void    plPlateManager::DrawToDevice( plPipeline *pipe )
{
    if( !pipe->IsDebugFlagSet(plPipeDbg::kFlagNoPlates) )
        IDrawToDevice( pipe );
}

//// IResortPlate ////////////////////////////////////////////////////////////

void    plPlateManager::IResortPlate( plPlate *plate, bool fromCurrent )
{
    plPlate     **start = &fPlates;


    if( fromCurrent )
        start = plate->fPrevPtr;

    plate->IUnlink();
    plate->ILink( start );
}

//// SetPlateScreenPos ///////////////////////////////////////////////////////

void    plPlateManager::SetPlateScreenPos( plPlate *plate, UInt32 x, UInt32 y )
{
    float   cX = ( (float)x / fOwner->Width() ) * 2.0f - 1.0f;
    float   cY = ( (float)y / fOwner->Height() ) * 2.0f - 1.0f;

    plate->SetPosition( cX, cY );
}

void    plPlateManager::SetPlatePixelSize( plPlate *plate, UInt32 pWidth, UInt32 pHeight )
{
    float width = (float)pWidth / fOwner->Width() * 2.0f;
    float height = (float)pHeight / fOwner->Height() * 2.0f;

    plate->SetSize(width, height);
}
