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
//  plPlates.cpp - Implementation of plates and plate manager               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "plPlates.h"

#include "plPipeline.h"
#include "hsGDeviceRef.h"
#include "plPipeDebugFlags.h"
#include "hsResMgr.h"

#include "pnMessage/plRefMsg.h"

#include "plClientResMgr/plClientResMgr.h"
#include "plGImage/plJPEG.h"
#include "plGImage/plPNG.h"
#include "plGImage/plMipmap.h"
#include "plSurface/plLayer.h"
#include "plSurface/hsGMaterial.h"
#include "plMessage/plLayRefMsg.h"



//////////////////////////////////////////////////////////////////////////////
//// plPlate Functions ///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

uint32_t  plPlate::fMagicUniqueKeyInt  = 0;

plPlate::plPlate( plPlate **owningHandle )
{
    fXformMatrix.Reset();
    fDepth = 1.0f;
    fMaterial = nullptr;
    fFlags = 0;
    fOpacity = 1.f;

    fNext = nullptr;
    fPrevPtr = nullptr;
    fOwningHandle = owningHandle;
    fMipmap = nullptr;
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

    fMaterial = nullptr;
    *fOwningHandle = nullptr;
}

//// SetPosition /////////////////////////////////////////////////////////////

void    plPlate::SetPosition( float x, float y, float z )
{
    if( z != -1.0f )
    {
        /// Gotta resort--let the manager do it
        plPlateManager::Instance().IResortPlate( this, ( z + 1.0f <= fDepth ) ? true : false );
        fDepth = z + 1.0f;
    }

    x *= fDepth / 1.0f;
    y *= fDepth / 1.0f;

    hsVector3 triple(x, y, fDepth);
    fXformMatrix.SetTranslate( &triple );
}

//// SetSize /////////////////////////////////////////////////////////////////

void    plPlate::SetSize( float width, float height, bool adjustByAspectRatio )
{
    width *= fDepth / 1.0f;
    height *= fDepth / 1.0f;

    hsVector3 size(adjustByAspectRatio ? (width * ((float)plPlateManager::Instance().GetPipeHeight() / (float)plPlateManager::Instance().GetPipeWidth())) : width,
                   height, 1.f);
    fXformMatrix.SetScale( &size );
}

//// SetTransform ////////////////////////////////////////////////////////////

void    plPlate::SetTransform( hsMatrix44 &matrix, bool reSort )
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
    ST::string      keyName;

    material = new hsGMaterial();
    keyName = ST::format("PlateBlank#{}", fMagicUniqueKeyInt++);
    hsgResMgr::ResMgr()->NewKey( keyName, material, plLocation::kGlobalFixedLoc );
    layer = material->MakeBaseLayer();
    layer->SetShadeFlags( layer->GetShadeFlags() | hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite | hsGMatState::kShadeReallyNoFog );
    layer->SetZFlags( layer->GetZFlags() | hsGMatState::kZNoZRead );
    layer->SetBlendFlags( layer->GetBlendFlags() | hsGMatState::kBlendAlpha );
    layer->SetOpacity( fOpacity );
    layer->SetUVWSrc(plLayerInterface::kUVWPassThru);

    hsgResMgr::ResMgr()->AddViaNotify(texture->GetKey(), new plGenRefMsg(layer->GetKey(), plRefMsg::kOnCreate, -1, plLayRefMsg::kTexture), plRefFlags::kActiveRef);

    SetMaterial(material);
}

//// SetOpacity //////////////////////////////////////////////////////////////

void    plPlate::SetOpacity( float opacity )
{
    if (fMaterial != nullptr && fMaterial->GetLayer(0) != nullptr)
    {
        plLayer *layer = (plLayer *)fMaterial->GetLayer( 0 );
        layer->SetOpacity( opacity );
    }

    fOpacity = opacity;
}

//// CreateMaterial /////////////////////////////////////////////////////
//  Creates a new material for this plate with either a specified texture 
//  or an empty, white-filled bitmap.

plMipmap    *plPlate::CreateMaterial( uint32_t width, uint32_t height, bool withAlpha, plMipmap* texture )
{
    plLayer         *layer;
    hsGMaterial     *material;
    ST::string      keyName;


    if (texture)
    {
        fMipmap = texture;
    }
    else
    {
        /// Create a new bitmap
        fMipmap = new plMipmap( width, height, withAlpha ? plMipmap::kARGB32Config : plMipmap::kRGB32Config, 1 );
        memset( fMipmap->GetImage(), 0xff, height * fMipmap->GetRowBytes() );
        keyName = ST::format("PlateBitmap#{}", fMagicUniqueKeyInt++);
        hsgResMgr::ResMgr()->NewKey( keyName, fMipmap, plLocation::kGlobalFixedLoc );
        fMipmap->SetFlags( fMipmap->GetFlags() | plMipmap::kDontThrowAwayImage );
    }

    /// NOW create a layer wrapper and a material for that layer
    material = new hsGMaterial();
    keyName = ST::format("PlateBlank#{}", fMagicUniqueKeyInt++);
    hsgResMgr::ResMgr()->NewKey( keyName, material, plLocation::kGlobalFixedLoc );
    layer = material->MakeBaseLayer();
    layer->SetShadeFlags( layer->GetShadeFlags() | hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite | hsGMatState::kShadeReallyNoFog );
    layer->SetZFlags( layer->GetZFlags() | hsGMatState::kZNoZRead );
    layer->SetBlendFlags( layer->GetBlendFlags() | hsGMatState::kBlendAlpha );
    layer->SetOpacity( fOpacity );

    hsgResMgr::ResMgr()->AddViaNotify( fMipmap->GetKey(), new plLayRefMsg( layer->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

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

void plPlate::CreateFromResource(const ST::string& resName)
{
    if (!resName.empty())
    {
        plMipmap* resTexture = new plMipmap;
        resTexture->CopyFrom(plClientResMgr::Instance().getResource(resName));

        ST::string keyName = ST::format("PlateResource#{}", fMagicUniqueKeyInt++);
        hsgResMgr::ResMgr()->NewKey(keyName, resTexture, plLocation::kGlobalFixedLoc);
        CreateMaterial(resTexture->GetWidth(), resTexture->GetHeight(), true, resTexture);
    }
    else
    {
        // Null resource request - Create a blank Material instead
        CreateMaterial(32, 32, true);
    }
}

void plPlate::ReloadFromResource(const ST::string& resName)
{
    if (!resName.empty())
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
    hsAssert(fNext == nullptr && fPrevPtr == nullptr, "Trying to link a plate that's already linked");

    
    /// Advance back as far as we need to go
    while (*back != nullptr && (*back)->fDepth > fDepth)
        back = &( (*back)->fNext );

    /// Link!
    fNext = *back;
    if( *back )
        (*back)->fPrevPtr = &fNext;
    fPrevPtr = back;
    *back = this;
}

bool plPlate::IsVisible()
{
    // return not-visible if our material is not valid
    if (fMaterial->GetNumLayers() == 0)
        return false;
    plLayerInterface* layer = fMaterial->GetLayer(0);
    if (layer->GetTexture() == nullptr)
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
    SetLabelText(nullptr);
}

plGraphPlate::~plGraphPlate()
{
}

//// IMakePow2 ///////////////////////////////////////////////////////////////

uint32_t  plGraphPlate::IMakePow2( uint32_t value )
{
    int         i;


    for( i = 0; value != 0; i++, value >>= 1 );
    return 1 << i;
}

//// SetDataRange ////////////////////////////////////////////////////////////

void    plGraphPlate::SetDataRange( uint32_t min, uint32_t max, uint32_t width )
{
    uint32_t      height;


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

void    plGraphPlate::SetDataLabels( uint32_t min, uint32_t max )
{
    fLabelMin = min;
    fLabelMax = max;
}

//// ClearData ///////////////////////////////////////////////////////////////

void    plGraphPlate::ClearData()
{
    uint32_t  *bits = (uint32_t *)fMipmap->GetImage(), *ptr;
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

    if (fMaterial->GetLayer(0) != nullptr && fMaterial->GetLayer(0)->GetTexture())
    {
        hsGDeviceRef    *ref = fMaterial->GetLayer( 0 )->GetTexture()->GetDeviceRef();
        if (ref != nullptr)
            ref->SetDirty( true );
    }
}

//// AddData /////////////////////////////////////////////////////////////////
//  Scroll graph data left by one, then add the new value

void    plGraphPlate::AddData( int32_t value, int32_t value2, int32_t value3, int32_t value4 )
{
    std::vector<int32_t> values;
    if (value != -1)
        values.push_back(value);
    if (value2 != -1)
        values.push_back(value2);
    if (value3 != -1)
        values.push_back(value3);
    if (value4 != -1)
        values.push_back(value4);
    AddData(std::move(values));
}

void    plGraphPlate::AddData( std::vector<int32_t> values )
{
    hsAssert(fMipmap != nullptr, "Trying to add data to an uninitialized plGraphPlate");

    fMipmap->SetCurrLevel( 0 );

    uint32_t  *bits = (uint32_t *)fMipmap->GetImage(), *ptr;
    uint32_t  *minDPos = fMipmap->GetAddr32( 3, fMipmap->GetHeight() - 3 - 10 );
    uint32_t  *maxDPos = fMipmap->GetAddr32( 3, 2 );
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
        values[ i ] = (uint32_t)( (float)values[ i ] * ( fMipmap->GetHeight() - 4 ) / ( fLabelMax - fLabelMin + 1 ) );

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

        memmove( ptr, ptr + 1, ( fMipmap->GetWidth() - 5 ) * sizeof( uint32_t ) );
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

    if (fMaterial->GetLayer(0) != nullptr && fMaterial->GetLayer(0)->GetTexture() != nullptr)
    {
        hsGDeviceRef    *ref = fMaterial->GetLayer( 0 )->GetTexture()->GetDeviceRef();
        if (ref != nullptr)
            ref->SetDirty( true );
    }
}

//// SetColors ///////////////////////////////////////////////////////////////

void    plGraphPlate::SetColors( uint32_t bgHexColor, uint32_t axesHexColor, uint32_t dataHexColor, uint32_t graphHexColor )
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

void    plGraphPlate::SetDataColors( uint32_t hexColor1, uint32_t hexColor2, uint32_t hexColor3, uint32_t hexColor4 )
{
    std::vector<uint32_t> colors;
    colors.push_back(hexColor1);
    colors.push_back(hexColor2);
    colors.push_back(hexColor3);
    colors.push_back(hexColor4);
    SetDataColors(colors);
}

void    plGraphPlate::SetDataColors( const std::vector<uint32_t> & hexColors )
{
    fDataHexColors = hexColors;
}

//// SetLabelText ////////////////////////////////////////////////////////////

void    plGraphPlate::SetLabelText(const char *text1, const char *text2, const char *text3, const char *text4 )
{
    std::vector<std::string> strings;
    if (text1 != nullptr)
        strings.push_back(text1);
    else
        strings.push_back("");

    if (text2 != nullptr)
        strings.push_back(text2);
    else
        strings.push_back("");

    if (text3 != nullptr)
        strings.push_back(text3);
    else
        strings.push_back("");

    if (text4 != nullptr)
        strings.push_back(text4);
    else
        strings.push_back("");
    SetLabelText(strings);
}

//// IDrawNumber /////////////////////////////////////////////////////////////

void    plGraphPlate::IDrawNumber( uint32_t number, uint32_t *dataPtr, uint32_t stride, uint32_t color )
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

void    plGraphPlate::IDrawDigit( char digit, uint32_t *dataPtr, uint32_t stride, uint32_t color )
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

    
    char    *digData = digits[uint8_t(digit)];
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

plPlateManager  *plPlateManager::fInstance = nullptr;


//// Destructor  /////////////////////////////////////////////////////////////

plPlateManager::~plPlateManager()
{
    while (fPlates != nullptr)
        DestroyPlate( fPlates );

    fInstance = nullptr;
}

//// CreatePlate /////////////////////////////////////////////////////////////

void    plPlateManager::CreatePlate( plPlate **handle )
{
    plPlate     *plate = new plPlate( handle );


    plate->ILink( &fPlates );
    *handle = plate;
}

void    plPlateManager::CreatePlate( plPlate **handle, float width, float height )
{
    CreatePlate( handle );
    (*handle)->SetSize( width, height );
}

void    plPlateManager::CreatePlate( plPlate **handle, float x, float y, float width, float height )
{
    CreatePlate( handle );
    (*handle)->SetPosition( x, y );
    (*handle)->SetSize( width, height );
}

void    plPlateManager::CreateGraphPlate( plGraphPlate **handle )
{
    plGraphPlate    *plate = new plGraphPlate( (plPlate **)handle );


    plate->ILink( &fPlates );
    *handle = plate;
}

//// DestroyPlate ////////////////////////////////////////////////////////////

void    plPlateManager::DestroyPlate( plPlate *plate )
{
    if (plate != nullptr)
    {
        plate->IUnlink();
        delete plate;
    }
}

//// GetPipeWidth/Height /////////////////////////////////////////////////////

uint32_t  plPlateManager::GetPipeWidth()
{
    return fOwner->Width();
}

uint32_t  plPlateManager::GetPipeHeight()
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

void    plPlateManager::SetPlateScreenPos( plPlate *plate, uint32_t x, uint32_t y )
{
    float   cX = ( (float)x / fOwner->Width() ) * 2.0f - 1.0f;
    float   cY = ( (float)y / fOwner->Height() ) * 2.0f - 1.0f;

    plate->SetPosition( cX, cY );
}

void    plPlateManager::SetPlatePixelSize( plPlate *plate, uint32_t pWidth, uint32_t pHeight )
{
    float width = (float)pWidth / fOwner->Width() * 2.0f;
    float height = (float)pHeight / fOwner->Height() * 2.0f;

    plate->SetSize(width, height);
}
