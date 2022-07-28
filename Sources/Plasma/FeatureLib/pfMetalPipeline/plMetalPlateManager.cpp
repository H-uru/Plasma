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

#include "plMetalPlateManager.h"
#include "plMetalPipeline.h"
#include <assert.h>
#include "ShaderTypes.h"

plMetalPlateManager::plMetalPlateManager(plMetalPipeline* pipe)
    : plPlateManager(pipe),
    fVtxBuffer(0)
{
    //Compile the shaders and link our pipeline for plates
    MTL::Library *library = pipe->fDevice.fMetalDevice->newDefaultLibrary();
    MTL::Function *fragFunction = library->newFunction(
                                                    NS::String::string("fragmentShader", NS::ASCIIStringEncoding)
                                                    );
    MTL::Function *vertFunction = library->newFunction(
                                                    NS::String::string("plateVertexShader", NS::ASCIIStringEncoding)
                                                    );
    MTL::RenderPipelineDescriptor *descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    descriptor->setFragmentFunction(fragFunction);
    descriptor->setVertexFunction(vertFunction);
    descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    descriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    descriptor->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float_Stencil8);
    descriptor->setSampleCount(pipe->fDevice.fSampleCount);
    
    //create the descriptor of the vertex array
    MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(VertexAttributePosition);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setBufferIndex(VertexAttributeTexcoord);
    vertexDescriptor->attributes()->object(1)->setOffset(0);
    
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(float) * 2);
    vertexDescriptor->layouts()->object(1)->setStride(sizeof(float) * 2);
    
    descriptor->setVertexDescriptor(vertexDescriptor);
    
    MTL::DepthStencilDescriptor *depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(false);
    fDepthState = pipe->fDevice.fMetalDevice->newDepthStencilState(depthDescriptor);
    depthDescriptor->release();
    
    NS::Error *error;
    fPlateRenderPipelineState = pipe->fDevice.fMetalDevice->newRenderPipelineState(descriptor, &error);
    library->release();
    fragFunction->release();
    vertFunction->release();
    descriptor->release();
}

void plMetalPlateManager::ICreateGeometry()
{
    plMetalPipeline *pipeline = (plMetalPipeline *)fOwner;
    if(!fVtxBuffer) {
        struct plateVertexBuffer vertexBuffer;
        vertexBuffer.vertices[0].Set(-0.5f, -0.5f);
        vertexBuffer.uv[0].Set(0.0f, 0.0f);

        vertexBuffer.vertices[1].Set(-0.5f, 0.5f);
        vertexBuffer.uv[1].Set(0.0f, 1.0f);

        vertexBuffer.vertices[2].Set(0.5f, -0.5f);
        vertexBuffer.uv[2].Set(1.0f, 0.0f);

        vertexBuffer.vertices[3].Set(0.5f, 0.5f);
        vertexBuffer.uv[3].Set(1.0f, 1.0f);

        uint16_t indices[6] = {0, 1, 2, 1, 2, 3};
        
        fVtxBuffer = pipeline->fDevice.fMetalDevice->newBuffer(&vertexBuffer, sizeof(plateVertexBuffer), MTL::StorageModeManaged);
        fVtxBuffer->retain();
        idxBuffer = pipeline->fDevice.fMetalDevice->newBuffer(&indices, sizeof(uint16_t) * 6, MTL::StorageModeManaged);
    }
}

void plMetalPlateManager::encodeVertexBuffer(MTL::RenderCommandEncoder *encoder) {
    encoder->setVertexBuffer(fVtxBuffer, 0, VertexAttributePosition);
    encoder->setVertexBuffer(fVtxBuffer, offsetof(plateVertexBuffer, uv), VertexAttributeTexcoord);
    
    encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, idxBuffer, 0);
}

void plMetalPlateManager::IReleaseGeometry()
{
    if (fVtxBuffer)
    {
        fVtxBuffer->release();
        fVtxBuffer = nullptr;
    }
}

void plMetalPlateManager::IDrawToDevice(plPipeline *pipe) {
    plMetalPipeline *pipeline = (plMetalPipeline *)pipe;
    plPlate* plate = nullptr;
    
    for (plate = fPlates; plate != nullptr; plate = plate->GetNext()) {
        if (plate->IsVisible()) {
            pipeline->IDrawPlate(plate);
        }
    }
}

plMetalPlateManager::~plMetalPlateManager()
{
    IReleaseGeometry();
}
