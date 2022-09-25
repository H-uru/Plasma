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
//  plDXTextFont Class Functions                                             //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  2.19.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "plMetalTextFont.h"

#include "HeadSpin.h"
#include "hsWindows.h"

#include "plMetalPipeline.h"
#include "plPipeline/hsWinRef.h"


// Following number needs to be at least: 64 chars max in plTextFont drawn at any one time
//                                      * 4 primitives per char max (for bold text)
//                                      * 3 verts per primitive

//const uint32_t  kNumVertsInBuffer(32768);
const uint32_t    kNumVertsInBuffer(4608);

uint32_t                  plMetalTextFont::fBufferCursor = 0;

//// Constructor & Destructor /////////////////////////////////////////////////

plMetalTextFont::plMetalTextFont( plPipeline *pipe, plMetalDevice* device ) : plTextFont( pipe ), fTexture()
{
    fDevice = device;
    fPipeline = (plMetalPipeline *)pipe;
    CreateShared(&(fPipeline->fDevice));
}

plMetalTextFont::~plMetalTextFont()
{
    DestroyObjects();
}

//// ICreateTexture ///////////////////////////////////////////////////////////

void    plMetalTextFont::ICreateTexture( uint16_t *data )
{
    printf("Create texture\n");
    
    MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA8Unorm, fTextureWidth, fTextureHeight, false);
    
    fTexture->release();
    fTexture = fDevice->fMetalDevice->newTexture(descriptor);
    fTexture->setLabel(NS::MakeConstantString("Font texture"));
    
    struct InDataValues {
        uint8_t a: 4;
        uint8_t r: 4;
        uint8_t g: 4;
        uint8_t b: 4;
    };
    
    struct OutDataValues {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    
    uint32_t *outData = new uint32_t[fTextureWidth * fTextureHeight];
    for(int i = 0; i < fTextureWidth * fTextureHeight; i++) {
        InDataValues *in = (InDataValues *)(data + i);
        OutDataValues *out = (OutDataValues *)(outData + i);
        
        out->r = in->r * 255;
        out->b = in->b * 255;
        out->g = in->g * 255;
        out->a = in->a * 255;
    }
    
    fTexture->replaceRegion(MTL::Region(0, 0, fTextureWidth, fTextureHeight), 0, outData, 4 * fTextureWidth);
    delete[] outData;
    /*
    HRESULT         hr;
    D3DLOCKED_RECT  lockInfo;
    D3DCAPS9        d3dCaps;

    
    // Check to make sure we can support it
    fDevice->GetDeviceCaps( &d3dCaps );
    hsAssert( fTextureWidth <= d3dCaps.MaxTextureWidth, "Cannot initialize DX font--texture size too big" );

    // Create our texture object
    hr = fDevice->CreateTexture(fTextureWidth, fTextureHeight, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &fD3DTexture, nullptr);
    hsAssert( !FAILED( hr ), "Cannot create D3D texture" );

    // Lock the texture and write our values out
    fD3DTexture->LockRect(0, &lockInfo, nullptr, 0);
    memcpy( lockInfo.pBits, data, fTextureWidth * fTextureHeight * sizeof( uint16_t ) );
    fD3DTexture->UnlockRect( 0 );
     */
}

void plMetalTextFont::CreateShared(plMetalDevice* device)
{
}

void plMetalTextFont::ReleaseShared(MTL::Device* device)
{
}

//// IInitStateBlocks /////////////////////////////////////////////////////////

void    plMetalTextFont::IInitStateBlocks()
{
/*
    for( int i = 0; i < 2; i++ )
    {
        fDevice->BeginStateBlock();
        fDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        fDevice->SetRenderState( D3DRS_SRCBLEND,   D3DBLEND_SRCALPHA );
        fDevice->SetRenderState( D3DRS_DESTBLEND,  D3DBLEND_INVSRCALPHA );
        fDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
        fDevice->SetRenderState( D3DRS_ALPHAREF,         0x08 );
        fDevice->SetRenderState( D3DRS_ALPHAFUNC,  D3DCMP_GREATEREQUAL );
        fDevice->SetRenderState( D3DRS_FILLMODE,   D3DFILL_SOLID );
        fDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );

        fDevice->SetRenderState( D3DRS_ZENABLE,      TRUE );
        fDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_ALWAYS );
        fDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
        fDevice->SetRenderState( D3DRS_DEPTHBIAS, 0 );

        fDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
        fDevice->SetRenderState( D3DRS_CLIPPING,         TRUE );
        fDevice->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE,    FALSE );
        fDevice->SetRenderState( D3DRS_VERTEXBLEND,      FALSE );
        fDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
        fDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
        fDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
        fDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        fDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
        fDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
        fDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        fDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
        fDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
        fDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
        fDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );
        fDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        fDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
        fDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        fDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
        fDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

        if( i == 0 )
            fDevice->EndStateBlock( &fOldStateBlock );
        else
            fDevice->EndStateBlock( &fTextStateBlock );
    }
 */
}

//// DestroyObjects ///////////////////////////////////////////////////////////

void    plMetalTextFont::DestroyObjects()
{
    fInitialized = false;
}

//// IDrawPrimitive ///////////////////////////////////////////////////////////

void    plMetalTextFont::IDrawPrimitive( uint32_t count, plFontVertex *array )
{
    plFontVertex        *v;
    
    plMetalDevice::plMetalLinkedPipeline* linkedPipeline = plMetalTextFontPipelineState(fDevice).GetRenderPipelineState();
    
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(linkedPipeline->pipelineState);
    const uint maxCount = 4096/(sizeof(plFontVertex) * 3);
    uint drawm = 0;
    while(count > 0) {
        uint drawCount = MIN(maxCount, count);
        fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(array + (drawm * 3), drawCount * 3 * sizeof( plFontVertex ), 0);
    
        fPipeline->fDevice.CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), drawCount * 3);
        
        count -= drawCount;
        drawm += drawCount;
    }

    //if( !fBuffer )
     //   return;

    /// Lock the buffer and write to it
    /*if( fBufferCursor && (fBufferCursor + count * 3 < kNumVertsInBuffer) )
    {
        // We can lock part of it
        if( FAILED( fBuffer->Lock( fBufferCursor * sizeof( plFontVertex ),
                                    count * 3 * sizeof( plFontVertex ),
                                    (void **)&v, D3DLOCK_NOOVERWRITE ) ) )
        {
            hsAssert( false, "Failed to lock vertex buffer for writing" );
            return;
        }

        fBufferCursor += count * 3;
    }
    else
    {
        // Gotta start over
        FlushDraws();
        fBufferCursor = count * 3;

        if( FAILED( fBuffer->Lock( 0, count * 3 * sizeof( plFontVertex ),
                                    (void **)&v, D3DLOCK_DISCARD ) ) )
        {
            hsAssert( false, "Failed to lock vertex buffer for writing" );
            return;
        }
    }

    if (v != nullptr && array != nullptr)
    {
        memcpy( v, array, count * sizeof( plFontVertex ) * 3 );
    }

    fBuffer->Unlock();*/
}

//// IDrawLines ///////////////////////////////////////////////////////////////

void    plMetalTextFont::IDrawLines( uint32_t count, plFontVertex *array )
{
    plMetalDevice::plMetalLinkedPipeline* linkedPipeline = plMetalTextFontPipelineState(fDevice).GetRenderPipelineState();
    
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(linkedPipeline->pipelineState);
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(array, count * 2 * sizeof( plFontVertex ), 0);
    
    matrix_float4x4 mat = matrix_identity_float4x4;
    mat.columns[0][0] = 2.0f / (float)fPipe->Width();
    mat.columns[1][1] = -2.0f / (float)fPipe->Height();
    mat.columns[3][0] = -1.0;
    mat.columns[3][1] = 1.0;
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(&mat, sizeof( matrix_float4x4 ), 1);
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(fTexture, 0);
    
    fPipeline->fDevice.CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveTypeLine, NS::UInteger(0), count * 2);
    /*if( !fBuffer )
        return;

    if (count == 0 || array == nullptr)
        return;

    fDevice->SetVertexShader(nullptr);
    fDevice->SetFVF(kFVF);
    fDevice->SetStreamSource(0, fBuffer, 0, sizeof(plFontVertex));
    fDevice->DrawPrimitiveUP( D3DPT_LINELIST, count, (const void *)array, sizeof( plFontVertex ) );*/
}

//// FlushDraws ///////////////////////////////////////////////////////////////
//  Flushes out and finishes any drawing left to be done.

void    plMetalTextFont::FlushDraws()
{
    /*if( !fBuffer )
        return;

    if( fBufferCursor > 0 )
    {
        fDevice->SetVertexShader(nullptr);
        fDevice->SetFVF(kFVF);
        fDevice->SetStreamSource( 0, fBuffer, 0, sizeof( plFontVertex ) );
        fDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, fBufferCursor / 3 );
        fBufferCursor = 0;
    }*/
}

//// SaveStates ///////////////////////////////////////////////////////////////

void    plMetalTextFont::SaveStates()
{
    
    matrix_float4x4 mat = matrix_identity_float4x4;
    mat.columns[0][0] = 2.0f / (float)fPipe->Width();
    mat.columns[1][1] = -2.0f / (float)fPipe->Height();
    mat.columns[3][0] = -1.0;
    mat.columns[3][1] = 1.0;
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(&mat, sizeof( matrix_float4x4 ), 1);
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(fTexture, 0);
    /*if( !fInitialized )
        IInitObjects();

    if (fOldStateBlock)
        fOldStateBlock->Capture();
    if (fTextStateBlock)
        fTextStateBlock->Apply();

    fDevice->SetTexture( 0, fD3DTexture );
    fDevice->SetTransform( D3DTS_TEXTURE0, &d3dIdentityMatrix );

    /// Set up the transform matrices so that the vertices can range (0-screenWidth,0-screenHeight)
    fDevice->SetTransform( D3DTS_WORLD, &d3dIdentityMatrix );
    fDevice->SetTransform( D3DTS_VIEW, &d3dIdentityMatrix );
    D3DMATRIX  mat;
    mat = d3dIdentityMatrix;
    mat.m[0][0] = 2.0f / (float)fPipe->Width();
    mat.m[1][1] = -2.0f / (float)fPipe->Height();
    mat.m[3][0] = -1.0;
    mat.m[3][1] = 1.0;
    fDevice->SetTransform( D3DTS_PROJECTION, &mat );*/
}

//// RestoreStates ////////////////////////////////////////////////////////////

void    plMetalTextFont::RestoreStates()
{
    /*if (fOldStateBlock)
        fOldStateBlock->Apply();
    
    fDevice->SetTexture(0, nullptr);
    fDevice->SetTransform( D3DTS_TEXTURE0, &d3dIdentityMatrix );*/
}



bool plMetalTextFontPipelineState::IsEqual(const plMetalPipelineState &p) const {
    return true;
}

plMetalPipelineState *plMetalTextFontPipelineState::Clone() {
    return new plMetalTextFontPipelineState(fDevice);
}

const MTL::Function *plMetalTextFontPipelineState::GetVertexFunction(MTL::Library *library) {
    return library->newFunction(NS::MakeConstantString("textFontVertexShader"));
}

const MTL::Function *plMetalTextFontPipelineState::GetFragmentFunction(MTL::Library *library) {
    return library->newFunction(NS::MakeConstantString("textFontFragmentShader"));
}

const NS::String *plMetalTextFontPipelineState::GetDescription() {
    return NS::MakeConstantString("Font Rendering");
}

void plMetalTextFontPipelineState::ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) {
    
    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
}

void plMetalTextFontPipelineState::ConfigureVertexDescriptor(MTL::VertexDescriptor *vertexDescriptor) {
    return;
}

void plMetalTextFontPipelineState::GetFunctionConstants(MTL::FunctionConstantValues *) const {
    return;
}
