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

#include "plMetalTextFont.h"

#include "HeadSpin.h"
#include "hsWindows.h"
#include "plMetalPipeline.h"
#include "plPipeline/hsWinRef.h"

// Following number needs to be at least: 64 chars max in plTextFont drawn at any one time
//                                      * 4 primitives per char max (for bold text)
//                                      * 3 verts per primitive

constexpr uint32_t kNumVertsInBuffer = 4608;

uint32_t plMetalTextFont::fBufferCursor = 0;

//// Constructor & Destructor /////////////////////////////////////////////////

plMetalTextFont::plMetalTextFont(plPipeline *pipe, plMetalDevice* device) : plTextFont(pipe),
                                                                            fTexture()
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

void plMetalTextFont::ICreateTexture(uint16_t *data)
{
    MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA8Unorm, fTextureWidth, fTextureHeight, false);

    fTexture->release();
    fTexture = fDevice->fMetalDevice->newTexture(descriptor);
    fTexture->setLabel(MTLSTR("Font texture"));

    struct InDataValues
    {
        uint8_t a : 4;
        uint8_t r : 4;
        uint8_t g : 4;
        uint8_t b : 4;
    };

    struct OutDataValues
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    auto outData = std::make_unique<uint32_t[]>(fTextureWidth * fTextureHeight);
    for (size_t i = 0; i < fTextureWidth * fTextureHeight; i++) {
        InDataValues*   in = (InDataValues*)(data + i);
        OutDataValues*  out = (OutDataValues*)(outData.get() + i);

        out->r = in->r * 255;
        out->b = in->b * 255;
        out->g = in->g * 255;
        out->a = in->a * 255;
    }

    fTexture->replaceRegion(MTL::Region(0, 0, fTextureWidth, fTextureHeight), 0, outData.get(), 4 * fTextureWidth);
}

void plMetalTextFont::CreateShared(plMetalDevice* device)
{
}

void plMetalTextFont::ReleaseShared(MTL::Device* device)
{
}

//// IInitStateBlocks /////////////////////////////////////////////////////////

void plMetalTextFont::IInitStateBlocks()
{
}

//// DestroyObjects ///////////////////////////////////////////////////////////

void plMetalTextFont::DestroyObjects()
{
    fInitialized = false;
}

//// IDrawPrimitive ///////////////////////////////////////////////////////////

void plMetalTextFont::IDrawPrimitive(uint32_t count, plFontVertex* array)
{
    plFontVertex* v;

    plMetalDevice::plMetalLinkedPipeline* linkedPipeline = plMetalTextFontPipelineState(fDevice).GetRenderPipelineState();

    fPipeline->fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(linkedPipeline->pipelineState);
    constexpr size_t    maxCount = 4096 / (sizeof(plFontVertex) * 3);
    
    uint drawn = 0;
    while (count > 0) {
        uint drawCount = std::min(uint(maxCount), uint(count));
        fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(array + (drawn * 3), drawCount * 3 * sizeof(plFontVertex), 0);

        fPipeline->fDevice.CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), drawCount * 3);

        count -= drawCount;
        drawn += drawCount;
    }
}

//// IDrawLines ///////////////////////////////////////////////////////////////

void plMetalTextFont::IDrawLines(uint32_t count, plFontVertex* array)
{
    plMetalDevice::plMetalLinkedPipeline* linkedPipeline = plMetalTextFontPipelineState(fDevice).GetRenderPipelineState();

    fPipeline->fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(linkedPipeline->pipelineState);
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(array, count * 2 * sizeof(plFontVertex), 0);

    matrix_float4x4 mat = matrix_identity_float4x4;
    mat.columns[0][0] = 2.0f / (float)fPipe->Width();
    mat.columns[1][1] = -2.0f / (float)fPipe->Height();
    mat.columns[3][0] = -1.0;
    mat.columns[3][1] = 1.0;
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(&mat, sizeof(matrix_float4x4), 1);
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(fTexture, 0);

    fPipeline->fDevice.CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveTypeLine, NS::UInteger(0), count * 2);
}

//// FlushDraws ///////////////////////////////////////////////////////////////
//  Flushes out and finishes any drawing left to be done.

void plMetalTextFont::FlushDraws()
{
    // Metal don't flush
}

//// SaveStates ///////////////////////////////////////////////////////////////

void plMetalTextFont::SaveStates()
{
    matrix_float4x4 mat = matrix_identity_float4x4;
    mat.columns[0][0] = 2.0f / (float)fPipe->Width();
    mat.columns[1][1] = -2.0f / (float)fPipe->Height();
    mat.columns[3][0] = -1.0;
    mat.columns[3][1] = 1.0;
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setVertexBytes(&mat, sizeof(matrix_float4x4), 1);
    fPipeline->fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(fTexture, 0);
}

//// RestoreStates ////////////////////////////////////////////////////////////

void plMetalTextFont::RestoreStates()
{
}

bool plMetalTextFontPipelineState::IsEqual(const plMetalPipelineState& p) const
{
    return true;
}

plMetalPipelineState* plMetalTextFontPipelineState::Clone()
{
    return new plMetalTextFontPipelineState(fDevice);
}

const MTL::Function* plMetalTextFontPipelineState::GetVertexFunction(MTL::Library* library)
{
    return library->newFunction(MTLSTR("textFontVertexShader"));
}

const MTL::Function* plMetalTextFontPipelineState::GetFragmentFunction(MTL::Library* library)
{
    return library->newFunction(MTLSTR("textFontFragmentShader"));
}

const NS::String* plMetalTextFontPipelineState::GetDescription()
{
    return MTLSTR("Font Rendering");
}

void plMetalTextFontPipelineState::ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor)
{
    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
}

void plMetalTextFontPipelineState::ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor)
{
    return;
}

void plMetalTextFontPipelineState::GetFunctionConstants(MTL::FunctionConstantValues*) const
{
    return;
}
