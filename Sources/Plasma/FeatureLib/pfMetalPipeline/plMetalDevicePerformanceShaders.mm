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

#include <stdio.h>
#include "plMetalDevice.h"
#include <Metal/Metal.h>
#include <MetalPerformanceShaders/MetalPerformanceShaders.h>

void plMetalDevice::EncodeBlur(MTL::CommandBuffer* commandBuffer, MTL::Texture* texture, float sigma)
{
    //FIXME: Blurring currently ends a pass - and restarting a pass will possibly clear one or more buffers
    //Technically shadow blurring only happens at the end of the render pass though...
    CurrentRenderCommandEncoder()->endEncoding();
    fCurrentRenderTargetCommandEncoder->release();
    fCurrentRenderTargetCommandEncoder = nil;
    
    //look up the shader by sigma value
    MPSImageGaussianBlur *blur = (MPSImageGaussianBlur *)fBlurShaders[sigma];
    
    //we don't have one, need to create one
    if (!blur) {
        blur = [[MPSImageGaussianBlur alloc] initWithDevice:(id<MTLDevice>)fMetalDevice sigma:sigma];
        fBlurShaders[sigma] = (NS::Object*)blur;
    }
    
    //we'd like to do the blur in place, but Metal might not let us.
    //if it allocates a new texture, we'll have to glit that data back to the original
    id<MTLTexture> destTexture = (id<MTLTexture>)texture;
    bool result = [blur encodeToCommandBuffer:(id<MTLCommandBuffer>)commandBuffer inPlaceTexture:(id<MTLTexture>*)&destTexture fallbackCopyAllocator:^ id<MTLTexture> (MPSKernel * kernel, id<MTLCommandBuffer> commandBuffer, id<MTLTexture> texture) {
        //this copy allocator will release the original texture - that texture is important, don't let it
        [texture retain];
        MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::texture2DDescriptor((MTL::PixelFormat)texture.pixelFormat, texture.width, texture.height, false);
        descriptor->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite);
        return (id<MTLTexture>)fMetalDevice->newTexture(descriptor)->autorelease();
    }];
    
    //did Metal change our original texture?
    if (destTexture != (id<MTLTexture>)texture) {
        //we'll need to blit the dest texture back to the source
        //we just committed a compute pass, buffer should be free for us to create
        //a blit encoder
        id<MTLBlitCommandEncoder> blitEncoder = [(id<MTLCommandBuffer>)GetCurrentCommandBuffer() blitCommandEncoder];
        [blitEncoder copyFromTexture:destTexture sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0, 0, 0) sourceSize:MTLSizeMake(destTexture.width, destTexture.height, 0) toTexture:(id<MTLTexture>)texture destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(0, 0, 0)];
        [blitEncoder endEncoding];
    }
}
