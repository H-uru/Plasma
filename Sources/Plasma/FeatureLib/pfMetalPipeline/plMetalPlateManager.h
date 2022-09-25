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

#ifndef plMetalPlateManager_hpp
#define plMetalPlateManager_hpp

#include <stdio.h>
#include "plPipeline/plPlates.h"
#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include "hsPoint2.h"
#include "plMetalPipelineState.h"

class plMetalPipeline;
class plMetalDevice;

class plMetalPlatePipelineState : public plMetalPipelineState
{
public:
    plMetalPlatePipelineState(plMetalDevice* device): plMetalPipelineState(device) { };
    virtual bool IsEqual(const plMetalPipelineState &p) const override;
    virtual uint16_t GetID() const override { return 5; };
    virtual plMetalPipelineState* Clone() override;
    virtual const MTL::Function*  GetVertexFunction(MTL::Library* library) override;
    virtual const MTL::Function*  GetFragmentFunction(MTL::Library* library) override;
    virtual const NS::String*     GetDescription() override;
    
    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) override;
    
    void ConfigureVertexDescriptor(MTL::VertexDescriptor *vertexDescriptor) override;
    
    void GetFunctionConstants(MTL::FunctionConstantValues *) const override;
    
};

class plMetalPlateManager : public plPlateManager
{
    friend class plMetalPipeline;
public:
    plMetalPlateManager(plMetalPipeline* pipe);
    void IDrawToDevice(plPipeline *pipe) override;
    void ICreateGeometry();
    void IReleaseGeometry();
    void EncodeDraw(MTL::RenderCommandEncoder *encoder);
    ~plMetalPlateManager();
private:
    struct plateVertexBuffer {
        hsPoint2 vertices[4];
        hsPoint2 uv[4];
    };
    MTL::Buffer *fVtxBuffer;
    MTL::Buffer *idxBuffer;
    MTL::DepthStencilState *fDepthState;
};

#endif /* plMetalPlateManager_hpp */
