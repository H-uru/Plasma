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

//We need to define these once for Metal somewhere in a cpp file
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <string_theory/format>
#include "plMetalDevice.h"
#include "plMetalPipeline.h"
#include "ShaderTypes.h"


#include "hsThread.h"
#include "plDrawable/plGBufferGroup.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plPipeline/plRenderTarget.h"

matrix_float4x4* hsMatrix2SIMD(const hsMatrix44& src, matrix_float4x4* dst, bool swapOrder)
{
    if (src.fFlags & hsMatrix44::kIsIdent)
    {
        memcpy(dst, &matrix_identity_float4x4, sizeof(float) * 16);
    }
    else
    {
        //SIMD is column major, hsMatrix44 is row major.
        //We need to flip.
        if(swapOrder) {
            dst->columns[0][0] = src.fMap[0][0];
            dst->columns[1][0] = src.fMap[0][1];
            dst->columns[2][0] = src.fMap[0][2];
            dst->columns[3][0] = src.fMap[0][3];
            
            dst->columns[0][1] = src.fMap[1][0];
            dst->columns[1][1] = src.fMap[1][1];
            dst->columns[2][1] = src.fMap[1][2];
            dst->columns[3][1] = src.fMap[1][3];
            
            dst->columns[0][2] = src.fMap[2][0];
            dst->columns[1][2] = src.fMap[2][1];
            dst->columns[2][2] = src.fMap[2][2];
            dst->columns[3][2] = src.fMap[2][3];
            
            dst->columns[0][3] = src.fMap[3][0];
            dst->columns[1][3] = src.fMap[3][1];
            dst->columns[2][3] = src.fMap[3][2];
            dst->columns[3][3] = src.fMap[3][3];
        } else {
            memcpy(dst, &src.fMap, sizeof(matrix_float4x4));
        }
    }

    return dst;
}


bool plMetalDevice::InitDevice()
{
    //FIXME: Should Metal adopt InitDevice like OGL?
    hsAssert(0, "InitDevice not implemented for Metal rendering");
}

void plMetalDevice::Shutdown()
{
    //FIXME: Should Metal adopt Shutdown like OGL?
    hsAssert(0, "Shutdown not implemented for Metal rendering");
}

void plMetalDevice::Clear(bool shouldClearColor, simd_float4 clearColor, bool shouldClearDepth, float clearDepth) {
    
    if (shouldClearColor) {
        fClearColor = clearColor;
    }
    fShouldClearColor = shouldClearColor;
    
    if (shouldClearDepth) {
        fClearDepth = clearDepth;
    }
    
    if (fCurrentRenderTargetCommandEncoder) {
        
        printf("Ending render pass, allowing a new one to lazily be created\n");
        
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
    
}

void plMetalDevice::BeginNewRenderPass() {
    
    printf("Beginning new render pass\n");
    
    //lazilly create the screen render encoder if it does not yet exist
    if (!fCurrentOffscreenCommandBuffer && !fCurrentRenderTargetCommandEncoder) {
        SetRenderTarget(NULL);
    }
    
    if (fCurrentRenderTargetCommandEncoder) {
        //if we have an existing render target, submit it's commands and release it
        //if we need to come back to this render target, we can always create a new render
        //pass descriptor and submit more commands
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
    
    printf("Setting up render pass descriptor\n");
    
    MTL::RenderPassDescriptor *renderPassDescriptor = MTL::RenderPassDescriptor::renderPassDescriptor();
    renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentFragmentOutputTexture);
    renderPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    renderPassDescriptor->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(fClearColor.x, fClearColor.y, fClearColor.z, fClearColor.w));
    if (fShouldClearColor) {
        renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
    } else {
        renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
    }
    
    if (fCurrentRenderTarget) {
        if ( fCurrentRenderTarget->GetZDepth() ) {
            plMetalRenderTargetRef* deviceTarget= (plMetalRenderTargetRef *)fCurrentRenderTarget->GetDeviceRef();
            renderPassDescriptor->depthAttachment()->setTexture(deviceTarget->fDepthBuffer);
            renderPassDescriptor->depthAttachment()->setClearDepth(fClearDepth);
            renderPassDescriptor->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
            renderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        }
        fCurrentRenderTargetCommandEncoder = fCurrentOffscreenCommandBuffer->renderCommandEncoder(renderPassDescriptor)->retain();
    } else {
        renderPassDescriptor->depthAttachment()->setTexture(fCurrentDrawableDepthTexture);
        renderPassDescriptor->depthAttachment()->setClearDepth(fClearDepth);
        renderPassDescriptor->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
        renderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        fCurrentRenderTargetCommandEncoder = fCurrentCommandBuffer->renderCommandEncoder(renderPassDescriptor)->retain();
    }
    
}

void plMetalDevice::SetRenderTarget(plRenderTarget* target)
{
    if( fCurrentRenderTargetCommandEncoder ) {
        //if we have an existing render target, submit it's commands and release it
        //if we need to come back to this render target, we can always create a new render
        //pass descriptor and submit more commands
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
    
    if( fCurrentOffscreenCommandBuffer ) {
        fCurrentOffscreenCommandBuffer->enqueue();
        fCurrentOffscreenCommandBuffer->commit();
        fCurrentOffscreenCommandBuffer->release();
        fCurrentOffscreenCommandBuffer = nil;
    }
    
    fCurrentRenderTarget = target;
    
    if ( fCurrentRenderTarget && fShouldClearColor == false ) {
        // clear if a clear color wasn't already set
        fClearColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
        fShouldClearColor = true;
        fClearDepth = 1.0;
    }
    
    if(fCurrentRenderTarget) {
        printf("Setting render target\n");
        plMetalRenderTargetRef *deviceTarget= (plMetalRenderTargetRef *)target->GetDeviceRef();
        fCurrentOffscreenCommandBuffer = fCommandQueue->commandBuffer();
        fCurrentOffscreenCommandBuffer->retain();
        fCurrentFragmentOutputTexture = deviceTarget->fTexture;
        
        if(deviceTarget->fDepthBuffer) {
            fCurrentDepthFormat = MTL::PixelFormatDepth32Float_Stencil8;
        } else {
            fCurrentDepthFormat = MTL::PixelFormatInvalid;
        }
    } else {
        printf("Setting drawable target\n");
        fCurrentFragmentOutputTexture = fCurrentDrawable->texture();
        fCurrentDepthFormat = MTL::PixelFormatDepth32Float_Stencil8;
    }
}

plMetalDevice::plMetalDevice()
:   fErrorMsg(nullptr),
    fActiveThread(hsThread::ThisThreadHash()),
    fCurrentDrawable(nullptr),
    fCommandQueue(nullptr),
    fCurrentRenderTargetCommandEncoder(nullptr),
    fCurrentDrawableDepthTexture(nullptr),
    fCurrentFragmentOutputTexture(nullptr),
    fCurrentCommandBuffer(nullptr),
    fCurrentRenderTarget(nullptr)
    {
    fClearColor = {0.0, 0.0, 0.0, 1.0};
        
    fMetalDevice = MTL::CreateSystemDefaultDevice();
    fCommandQueue = fMetalDevice->newCommandQueue();
    
    //set up all the depth stencil states
    MTL::DepthStencilDescriptor *depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(true);
    depthDescriptor->setLabel(NS::String::string("No Z Read", NS::UTF8StringEncoding));
    fNoZReadStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    depthDescriptor->setDepthWriteEnabled(false);
    depthDescriptor->setLabel(NS::String::string("No Z Write", NS::UTF8StringEncoding));
    fNoZWriteStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(false);
    depthDescriptor->setLabel(NS::String::string("No Z Read or Write", NS::UTF8StringEncoding));
    fNoZReadOrWriteStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    depthDescriptor->setLabel(NS::String::string("Z Read and Write", NS::UTF8StringEncoding));
    depthDescriptor->setDepthWriteEnabled(true);
    fDefaultStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionGreaterEqual);
    depthDescriptor->setLabel(NS::String::string("Reverse Z", NS::UTF8StringEncoding));
    depthDescriptor->setDepthWriteEnabled(true);
    fReverseZStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->release();
}

void plMetalDevice::SetViewport() {
    CurrentRenderCommandEncoder()->setViewport({ (double)fPipeline->GetViewTransform().GetViewPortLeft(),
        (double)fPipeline->GetViewTransform().GetViewPortTop(),
        (double)fPipeline->GetViewTransform().GetViewPortWidth(),
        (double)fPipeline->GetViewTransform().GetViewPortHeight(),
        0.f, 1.f });
}

bool plMetalDevice::BeginRender() {
    if (fActiveThread == hsThread::ThisThreadHash()) {
        return true;
    }

    fActiveThread = hsThread::ThisThreadHash();
    
    return true;
}

static uint32_t  IGetBufferFormatSize(uint8_t format)
{
    uint32_t  size = sizeof( float ) * 6 + sizeof( uint32_t ) * 2; // Position and normal, and two packed colors

    switch (format & plGBufferGroup::kSkinWeightMask)
    {
        case plGBufferGroup::kSkinNoWeights:
            break;
        case plGBufferGroup::kSkin1Weight:
            size += sizeof(float);
            break;
        default:
            hsAssert( false, "Invalid skin weight value in IGetBufferFormatSize()" );
    }

    size += sizeof( float ) * 3 * plGBufferGroup::CalcNumUVs(format);

    return size;
}

void plMetalDevice::SetupVertexBufferRef(plGBufferGroup *owner, uint32_t idx, plMetalDevice::VertexBufferRef *vRef)
{
    uint8_t format = owner->GetVertexFormat();
    
    if (format & plGBufferGroup::kSkinIndices) {
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights;       // Should do nothing, but just in case...
        vRef->SetSkinned(true);
        vRef->SetVolatile(true);
    }
    
    uint32_t vertSize = vertSize = IGetBufferFormatSize(format); // vertex stride
    uint32_t numVerts = owner->GetVertBufferCount(idx);

    vRef->fOwner = owner;
    vRef->fCount = numVerts;
    vRef->fVertexSize = vertSize;
    vRef->fFormat = format;
    vRef->fRefTime = 0;

    vRef->SetDirty(true);
    vRef->SetRebuiltSinceUsed(true);
    vRef->fData = nullptr;

    vRef->SetVolatile(vRef->Volatile() || owner->AreVertsVolatile());

    vRef->fIndex = idx;
    
    const uint32_t vertStart = owner->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = owner->GetVertBufferEnd(idx) * vertSize - vertStart;
    
    owner->SetVertexBufferRef(idx, vRef);

    hsRefCnt_SafeUnRef(vRef);
}

void plMetalDevice::CheckStaticVertexBuffer(plMetalDevice::VertexBufferRef *vRef, plGBufferGroup *owner, uint32_t idx)
{
    hsAssert(!vRef->Volatile(), "Creating a managed vertex buffer for a volatile buffer ref");
    
    if (!vRef->GetBuffer())
    {
        FillVertexBufferRef(vRef, owner, idx);
        
        // This is currently a no op, but this would let the buffer know it can
        // unload the system memory copy, since we have a managed version now.
        owner->PurgeVertBuffer(idx);
    }
}

void plMetalDevice::FillVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    const uint32_t vertSize = ref->fVertexSize;
    const uint32_t vertStart = group->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = group->GetVertBufferEnd(idx) * vertSize - vertStart;
    
    if(ref->GetBuffer()) {
        assert(size <= ref->GetBuffer()->length());
    }
    
    if (!size)
    {
        return;
    }
    
    ref->SetBuffer(fMetalDevice->newBuffer(size, MTL::StorageModeManaged)->autorelease());
    uint8_t* buffer = (uint8_t*) ref->GetBuffer()->contents();

    if (ref->fData)
    {
        memcpy(buffer, ref->fData + vertStart, size);
    }
    else
    {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size, "Trailing dead space on non-interleaved data not supported");
        
        uint8_t* ptr = buffer;

        const uint32_t vertSmallSize = group->GetVertexLiteStride() - sizeof(hsPoint3) * 2;
        uint8_t* srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* const srcCPtr = group->GetColorBufferData(idx);

        const int numCells = group->GetNumCells(idx);
        int i;
        for (i = 0; i < numCells; i++)
        {
            plGBufferCell* cell = group->GetCell(idx, i);

            if (cell->fColorStart == uint32_t(-1))
            {
                /// Interleaved, do straight copy
                memcpy(ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize);
                ptr += cell->fLength * vertSize;
                assert(size <= cell->fLength * vertSize);
            }
            else
            {
                hsStatusMessage("Non interleaved data");

                /// Separated, gotta interleave
                uint8_t* tempVPtr = srcVPtr + cell->fVtxStart;
                plGBufferColor* tempCPtr = srcCPtr + cell->fColorStart;
                int j;
                for( j = 0; j < cell->fLength; j++ )
                {
                    memcpy( ptr, tempVPtr, sizeof( hsPoint3 ) * 2 );
                    ptr += sizeof( hsPoint3 ) * 2;
                    tempVPtr += sizeof( hsPoint3 ) * 2;

                    memcpy( ptr, &tempCPtr->fDiffuse, sizeof( uint32_t ) );
                    ptr += sizeof( uint32_t );
                    memcpy( ptr, &tempCPtr->fSpecular, sizeof( uint32_t ) );
                    ptr += sizeof( uint32_t );

                    memcpy( ptr, tempVPtr, vertSmallSize );
                    ptr += vertSmallSize;
                    tempVPtr += vertSmallSize;
                    tempCPtr++;
                }
            }
        }

        hsAssert((ptr - buffer) == size, "Didn't fill the buffer?");
    }

    /// Unlock and clean up
    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plMetalDevice::FillVolatileVertexBufferRef(plMetalDevice::VertexBufferRef *ref, plGBufferGroup *group, uint32_t idx)
{
    uint8_t* dst = ref->fData;
    uint8_t* src = group->GetVertBufferData(idx);

    size_t uvChanSize = plGBufferGroup::CalcNumUVs(group->GetVertexFormat()) * sizeof(float) * 3;
    uint8_t numWeights = (group->GetVertexFormat() & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < ref->fCount; ++i) {
        memcpy(dst, src, sizeof(hsPoint3)); // pre-pos
        dst += sizeof(hsPoint3);
        src += sizeof(hsPoint3);

        src += numWeights * sizeof(float); // weights

        if (group->GetVertexFormat() & plGBufferGroup::kSkinIndices)
            src += sizeof(uint32_t); // indices

        memcpy(dst, src, sizeof(hsVector3)); // pre-normal
        dst += sizeof(hsVector3);
        src += sizeof(hsVector3);

        memcpy(dst, src, sizeof(uint32_t) * 2); // diffuse & specular
        dst += sizeof(uint32_t) * 2;
        src += sizeof(uint32_t) * 2;

        // UVWs
        memcpy(dst, src, uvChanSize);
        src += uvChanSize;
        dst += uvChanSize;
    }
}

void plMetalDevice::SetupIndexBufferRef(plGBufferGroup *owner, uint32_t idx, plMetalDevice::IndexBufferRef *iRef)
{
    uint32_t numIndices = owner->GetIndexBufferCount(idx);
    iRef->fCount = numIndices;
    iRef->fOwner = owner;
    iRef->fIndex = idx;
    iRef->fRefTime = 0;

    iRef->SetDirty(true);
    iRef->SetRebuiltSinceUsed(true);

    owner->SetIndexBufferRef(idx, iRef);
    hsRefCnt_SafeUnRef(iRef);

    iRef->SetVolatile(owner->AreIdxVolatile());
}

void plMetalDevice::CheckIndexBuffer(plMetalDevice::IndexBufferRef *iRef)
{
    if(!iRef->GetBuffer() && iRef->fCount) {
        iRef->SetVolatile(false);
        
        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

void plMetalDevice::FillIndexBufferRef(plMetalDevice::IndexBufferRef *iRef, plGBufferGroup *owner, uint32_t idx)
{
    uint32_t startIdx = owner->GetIndexBufferStart(idx);
    uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);

    if (!size)
    {
        return;
    }
    
    iRef->PrepareForWrite();
    MTL::Buffer* indexBuffer = iRef->GetBuffer();
    if(!indexBuffer || indexBuffer->length() < size) {
        indexBuffer = fMetalDevice->newBuffer(size, MTL::ResourceStorageModeManaged)->autorelease();
        iRef->SetBuffer(indexBuffer);
    }
    
    memcpy(indexBuffer->contents(), owner->GetIndexBufferData(idx), size);
    indexBuffer->didModifyRange(NS::Range(0, size));

    iRef->SetDirty(false);
}

void plMetalDevice::SetupTextureRef(plLayerInterface *layer, plBitmap *img, plMetalDevice::TextureRef *tRef)
{
    tRef->fOwner = img;
    
    plBitmap* imageToCheck = img;
    
    //if it's a cubic texture, check the first face. The root img will give a false format that will cause us to decode wrong.
    plCubicEnvironmap* cubicImg = dynamic_cast<plCubicEnvironmap*>(img);
    if(cubicImg) {
        imageToCheck = cubicImg->GetFace(0);
    }

    if (imageToCheck->IsCompressed()) {
        switch (imageToCheck->fDirectXInfo.fCompressionType) {
        case plBitmap::DirectXInfo::kDXT1:
                tRef->fFormat = MTL::PixelFormatBC1_RGBA;
            break;
        case plBitmap::DirectXInfo::kDXT5:
                tRef->fFormat = MTL::PixelFormatBC3_RGBA;
            break;
        }
    } else {
        switch (imageToCheck->fUncompressedInfo.fType) {
        case plBitmap::UncompressedInfo::kRGB8888:
            tRef->fFormat = MTL::PixelFormatBGRA8Unorm;
            break;
        case plBitmap::UncompressedInfo::kRGB4444:
            //we'll convert this on load to 8 bits per channel
            //Metal doesn't support 4 bits per channel on all hardware
            tRef->fFormat = MTL::PixelFormatBGRA8Unorm;
            break;
        case plBitmap::UncompressedInfo::kRGB1555:
            tRef->fFormat = MTL::PixelFormatBGR5A1Unorm;
            break;
        case plBitmap::UncompressedInfo::kInten8:
            tRef->fFormat = MTL::PixelFormatR8Uint;
            break;
        case plBitmap::UncompressedInfo::kAInten88:
            tRef->fFormat = MTL::PixelFormatRG8Uint;
            break;
        }
    }

    tRef->SetDirty(true);

    img->SetDeviceRef(tRef);
    hsRefCnt_SafeUnRef(tRef);
}

void plMetalDevice::CheckTexture(plMetalDevice::TextureRef *tRef)
{
    if (!tRef->fTexture)
    {
        tRef->SetDirty(true);
    }
}

uint plMetalDevice::ConfigureAllowedLevels(plMetalDevice::TextureRef *tRef, plMipmap *mipmap)
{
    if (mipmap->IsCompressed()) {
        mipmap->SetCurrLevel(tRef->fLevels);
        while ((mipmap->GetCurrWidth() | mipmap->GetCurrHeight()) & 0x03) {
            tRef->fLevels--;
            hsAssert(tRef->fLevels >= 0, "How was this ever compressed?" );
            mipmap->SetCurrLevel(tRef->fLevels);
        }
    }
}

void plMetalDevice::PopulateTexture(plMetalDevice::TextureRef *tRef, plMipmap *img, uint slice)
{
    
    if(img->GetKeyName() == "RightDTMap2_dynText") {
        printf("hi");
    }
    if (img->IsCompressed()) {
        
        for (int lvl = 0; lvl <= tRef->fLevels; lvl++) {
            img->SetCurrLevel(lvl);
            
                switch (img->fDirectXInfo.fCompressionType) {
                case plBitmap::DirectXInfo::kDXT1:
                        tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), img->GetCurrWidth() * 2, 0);
                    break;
                case plBitmap::DirectXInfo::kDXT5:
                        tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), img->GetCurrWidth() * 4, 0);
                    break;
                }
        }
    } else {
        for (int lvl = 0; lvl <= tRef->fLevels; lvl++) {
            img->SetCurrLevel(lvl);
            
            if(img->GetCurrLevelPtr()) {
                if(img->fUncompressedInfo.fType == plBitmap::UncompressedInfo::kRGB4444) {
                    
                    struct RGBA4444Component {
                        unsigned r:4;
                        unsigned g:4;
                        unsigned b:4;
                        unsigned a:4;
                    };
                    
                    RGBA4444Component *in = (RGBA4444Component *)img->GetCurrLevelPtr();
                    simd_uint4 *out = (simd_uint4 *) malloc(img->GetCurrHeight() * img->GetCurrWidth() * 4);
                    
                    for(int i=0; i<(img->GetCurrWidth() * img->GetCurrHeight()); i++) {
                        out[i].r = in[i].r;
                        out[i].g = in[i].g;
                        out[i].b = in[i].b;
                        out[i].a = in[i].a;
                    }
                    
                    tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, out, img->GetCurrWidth() * 4, 0);
                    
                    free(out);
                } else {
                    tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), img->GetCurrWidth() * 4, 0);
                }
            } else {
                printf("Texture with no image data?\n");
            }
        }
    }
    tRef->fTexture->setLabel(NS::String::string(img->GetKeyName().c_str(), NS::UTF8StringEncoding));
    tRef->SetDirty(false);
}

void plMetalDevice::MakeTextureRef(plMetalDevice::TextureRef *tRef, plLayerInterface *layer, plMipmap *img)
{
    if (!img->GetImage()) {
        return;
    }
    
    tRef->fLevels = img->GetNumLevels() - 1;
    if(!tRef->fTexture) {
        ConfigureAllowedLevels(tRef, img);
        //texture doesn't exist yet, create it
        bool supportsMipMap = tRef->fLevels;
        MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::texture2DDescriptor(tRef->fFormat, img->GetWidth(), img->GetHeight(), supportsMipMap);
        descriptor->setUsage(MTL::TextureUsageShaderRead);
        //if device has unified memory, set storage mode to shared
        if(fMetalDevice->hasUnifiedMemory()) {
            descriptor->setStorageMode(MTL::StorageModeShared);
        }
        descriptor->setUsage(MTL::TextureUsageShaderRead);
        //Metal gets mad if we set this with 0, only set it if we know there are mipmaps
        if(supportsMipMap) {
            descriptor->setMipmapLevelCount(tRef->fLevels + 1);
        }
        tRef->fTexture = fMetalDevice->newTexture(descriptor);
    }
    PopulateTexture( tRef, img, 0);
}

void plMetalDevice::MakeCubicTextureRef(plMetalDevice::TextureRef *tRef, plLayerInterface *layer, plCubicEnvironmap *img)
{
    MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::textureCubeDescriptor(tRef->fFormat, img->GetFace(0)->GetWidth(), tRef->fLevels != 0);
    
    if (tRef->fLevels != 0) {
        descriptor->setMipmapLevelCount(tRef->fLevels + 1);
    }
    descriptor->setUsage(MTL::TextureUsageShaderRead);
    //if device has unified memory, set storage mode to shared
    if(fMetalDevice->hasUnifiedMemory()) {
        descriptor->setStorageMode(MTL::StorageModeShared);
    }
    
    tRef->fTexture = fMetalDevice->newTexture(descriptor);
    
    static const uint kFaceMapping[] = {
        1, // kLeftFace
        0, // kRightFace
        4, // kFrontFace
        5, // kBackFace
        2, // kTopFace
        3  // kBottomFace
    };
    for (size_t i = 0; i < 6; i++) {
        PopulateTexture( tRef, img->GetFace(i), kFaceMapping[i]);
    }
}

void plMetalDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    hsMatrix2SIMD(src, &fMatrixProj);
}

void plMetalDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    hsMatrix44 inv;
    src.GetInverse(&inv);
    
    hsMatrix2SIMD(src, &fMatrixW2C);
    hsMatrix2SIMD(inv, &fMatrixC2W);
}

void plMetalDevice::SetLocalToWorldMatrix(const hsMatrix44& src, bool swapOrder)
{
    hsMatrix44 inv;
    src.GetInverse(&inv);
    
    hsMatrix2SIMD(src, &fMatrixL2W, swapOrder);
    hsMatrix2SIMD(inv, &fMatrixW2L, swapOrder);
}

plMetalDevice::plPipelineStateAtrributes::plPipelineStateAtrributes(const plMetalVertexBufferRef * vRef, const uint32_t blendFlags, const MTL::PixelFormat outputPixelFormat, const MTL::PixelFormat outputDepthFormat, const plShaderID::ID vertexShaderID, const plShaderID::ID fragmentShaderID, const int forShadows, const uint numLayers)
{
    numUVs = plGBufferGroup::CalcNumUVs(vRef->fFormat);
    numWeights = (vRef->fFormat & plGBufferGroup::kSkinWeightMask) >> 4;
    hasSkinIndices = (vRef->fFormat & plGBufferGroup::kSkinIndices);
    outputFormat = outputPixelFormat;
    this->depthFormat = outputDepthFormat;
    this->blendFlags = blendFlags;
    this->vertexShaderID = vertexShaderID;
    this->fragmentShaderID = fragmentShaderID;
    this->forShadows = forShadows;
    this->numLayers = numLayers;
}


std::condition_variable * plMetalDevice::prewarmPipelineStateFor(plMetalVertexBufferRef * vRef, uint32_t blendFlags, uint32_t numLayers, plShaderID::ID vertexShaderID, plShaderID::ID fragmentShaderID, bool forShadows)
{
    plPipelineStateAtrributes attributes = plPipelineStateAtrributes(vRef, blendFlags, fCurrentFragmentOutputTexture->pixelFormat(), fCurrentDepthFormat, vertexShaderID, fragmentShaderID, forShadows, numLayers);
    //only render thread is allowed to prewarm, no race conditions around
    //fConditionMap creation
    if(!fPipelineStateMap[attributes] && fConditionMap[attributes]) {
        std::condition_variable *condOut;
        StartRenderPipelineBuild(attributes, &condOut);
        return condOut;
    }
    return nullptr;
}

void plMetalDevice::StartRenderPipelineBuild(plPipelineStateAtrributes &attributes, std::condition_variable **condOut)
{
    /*
     Shader building requires both knowledge of the vertex buffer layout and the fragment shader details. For now it lives here. The caching and threading mechanism should be factored out so that OpenGL can share them. Vector buffer dependencies should be factored out so we only need material details. That also means we can use the threading to create these earlier in a render pass.
     */
    int vertOffset = 0;
    int skinWeightOffset = vertOffset + (sizeof(float) * 3);
    if(attributes.hasSkinIndices) {
        skinWeightOffset += sizeof(uint32_t);
    }
    int normOffset = skinWeightOffset + (sizeof(float) * attributes.numWeights);
    int colorOffset = normOffset + (sizeof(float) * 3);
    int baseUvOffset = colorOffset + (sizeof(uint32_t) * 2);
    int stride = baseUvOffset + (sizeof(float) * 3 * attributes.numUVs);
    
    MTL::Library *library = fMetalDevice->newDefaultLibrary();
    
    MTL::FunctionConstantValues *functionContents = MTL::FunctionConstantValues::alloc()->init();
    functionContents->setConstantValue(&attributes.numUVs, MTL::DataTypeUShort, FunctionConstantNumUVs);
    functionContents->setConstantValue(&attributes.numLayers, MTL::DataTypeUShort, FunctionConstantNumLayers);
    MTL::Function *fragFunction;
    MTL::Function *vertFunction;
    
    if(!attributes.vertexShaderID && !attributes.fragmentShaderID) {
        if(attributes.forShadows == 1) {
            fragFunction = library->newFunction(
                                                               NS::String::string("shadowFragmentShader", NS::ASCIIStringEncoding),
                                                               functionContents,
                                                               (NS::Error **)NULL
                                                            );
        } else if(attributes.forShadows == 2) {
            fragFunction = library->newFunction(
                                                               NS::String::string("shadowCastFragmentShader", NS::ASCIIStringEncoding),
                                                               functionContents,
                                                               (NS::Error **)NULL
                                                            );
        } else {
            fragFunction = library->newFunction(
                                                           NS::String::string("pipelineFragmentShader", NS::ASCIIStringEncoding),
                                                           functionContents,
                                                           (NS::Error **)NULL
                                                        );
        }
        vertFunction = library->newFunction(
                                                           NS::String::string("pipelineVertexShader", NS::ASCIIStringEncoding),
                                                           functionContents,
                                                           (NS::Error **)NULL
                                                        );
    } else if(attributes.vertexShaderID && attributes.fragmentShaderID) {
        switch(attributes.vertexShaderID) {
            case plShaderID::vs_WaveFixedFin7:
                vertFunction = library->newFunction(
                                                                   NS::String::string("vs_WaveFixedFin7", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::vs_CompCosines:
                vertFunction = library->newFunction(
                                                                   NS::String::string("vs_CompCosines", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::vs_BiasNormals:
                vertFunction = library->newFunction(
                                                                   NS::String::string("vs_BiasNormals", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::vs_GrassShader:
                vertFunction = library->newFunction(
                                                                   NS::String::string("vs_GrassShader", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::vs_WaveDecEnv_7:
                vertFunction = library->newFunction(
                                                                   NS::String::string("vs_WaveDecEnv_7", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            default:
                hsAssert(0, "unknown shader requested");
        }
        
        switch(attributes.fragmentShaderID) {
            case plShaderID::ps_WaveFixed:
                fragFunction = library->newFunction(
                                                                   NS::String::string("ps_WaveFixed", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::ps_MoreCosines:
                fragFunction = library->newFunction(
                                                                   NS::String::string("ps_CompCosines", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::ps_BiasNormals:
                fragFunction = library->newFunction(
                                                                   NS::String::string("ps_BiasNormals", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::ps_GrassShader:
                fragFunction = library->newFunction(
                                                                   NS::String::string("ps_GrassShader", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            case plShaderID::ps_WaveDecEnv:
                fragFunction = library->newFunction(
                                                                   NS::String::string("ps_WaveDecEnv", NS::ASCIIStringEncoding),
                                                                   functionContents,
                                                                   (NS::Error **)NULL
                                                                );
                break;
            default:
                hsAssert(0, "unknown shader requested");
        }
    } else {
        hsAssert(0, "Pipeline only supports both fragment and vertex shaders together");
    }
    
    MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
    
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setOffset(vertOffset);
    
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setOffset(normOffset);
    
    for(int i=0; i<attributes.numUVs; i++) {
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord+i)->setFormat(MTL::VertexFormatFloat3);
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord+i)->setBufferIndex(0);
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord+i)->setOffset(baseUvOffset + (i * sizeof(float) * 3));
    }
    
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setFormat(MTL::VertexFormatUChar4);
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setOffset(colorOffset);
    
    vertexDescriptor->layouts()->object(VertexAttributePosition)->setStride(stride);
    
    MTL::RenderPipelineDescriptor *descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    
    descriptor->setDepthAttachmentPixelFormat(attributes.depthFormat);
    
    
    descriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    // No color, just writing out Z values.
    if(attributes.forShadows == 1) {
        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
        descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
        descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
    } else if(attributes.forShadows == 2) {
        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorZero);
        descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
        descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
        descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
    } else if (attributes.blendFlags & hsGMatState::kBlendNoColor) {
        //printf("glBlendFunc(GL_ZERO, GL_ONE);\n");
        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorZero);
        descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
        descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
    } else {
        switch (attributes.blendFlags & hsGMatState::kBlendMask)
        {
            // Detail is just a special case of alpha, handled in construction of the texture
            // mip chain by making higher levels of the chain more transparent.
            case hsGMatState::kBlendDetail:
            case hsGMatState::kBlendAlpha:
                if (attributes.blendFlags & hsGMatState::kBlendInvertFinalAlpha) {
                    if (attributes.blendFlags & hsGMatState::kBlendAlphaPremultiplied) {
                        //printf("glBlendFunc(GL_ONE, GL_SRC_ALPHA);\n");
                        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                        descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
                        descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                        descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                    } else {
                        //printf("glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);\n");
                        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                        descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                        descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                        descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                    }
                } else {
                    if (attributes.blendFlags & hsGMatState::kBlendAlphaPremultiplied) {
                        //printf("glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);\n");
                        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                    } else {
                        //printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);\n");
                        descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                    }
                    descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                }
                break;

            // Multiply the final color onto the frame buffer.
            case hsGMatState::kBlendMult:
                if (attributes.blendFlags & hsGMatState::kBlendInvertFinalColor) {
                    //printf("glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);\n");
                    descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorZero);
                    descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
                    descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
                    descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceColor);
                } else {
                    //printf("glBlendFunc(GL_ZERO, GL_SRC_COLOR);\n");
                    descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorZero);
                    descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
                    descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorSourceColor);
                    descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceColor);
                }
                break;

            // Add final color to FB.
            case hsGMatState::kBlendAdd:
                //printf("glBlendFunc(GL_ONE, GL_ONE);\n");
                descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
                break;

            // Multiply final color by FB color and add it into the FB.
            case hsGMatState::kBlendMADD:
                //printf("glBlendFunc(GL_DST_COLOR, GL_ONE);\n");
                descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorDestinationColor);
                descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorDestinationColor);
                descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
                break;

            // Final color times final alpha, added into the FB.
            case hsGMatState::kBlendAddColorTimesAlpha:
                if (attributes.blendFlags & hsGMatState::kBlendInvertFinalAlpha) {
                    //printf("glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);\n");
                    descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                    descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                    descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                    descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
                } else {
                    //printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE);\n");
                    descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                    descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                    descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                    descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
                }
                break;

            // Overwrite final color onto FB
            case 0:
                //printf("glBlendFunc(GL_ONE, GL_ZERO);\n");
                descriptor->colorAttachments()->object(0)->setRgbBlendOperation(MTL::BlendOperationAdd);
                //printf("glBlendFunc(GL_ONE, GL_ZERO);\n");
                descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
                descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
                
                /*descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
                descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
                descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);*/
                break;

            default:
                {
                    /*hsAssert(false, "Too many blend modes specified in material");
                    plLayer* lay = plLayer::ConvertNoRef(fCurrMaterial->GetLayer(fCurrLayerIdx)->BottomOfStack());
                    if( lay )
                    {
                        if( lay->GetBlendFlags() & hsGMatState::kBlendAlpha )
                        {
                            lay->SetBlendFlags((lay->GetBlendFlags() & ~hsGMatState::kBlendMask) | hsGMatState::kBlendAlpha);
                        }
                        else
                        {
                            lay->SetBlendFlags((lay->GetBlendFlags() & ~hsGMatState::kBlendMask) | hsGMatState::kBlendAdd);
                        }
                    }*/
                }
                break;
        }
    }
    
    descriptor->colorAttachments()->object(0)->setPixelFormat(attributes.outputFormat);
    
    descriptor->setFragmentFunction(fragFunction);
    descriptor->setVertexFunction(vertFunction);
    descriptor->setVertexDescriptor(vertexDescriptor);
    std::string label = "Render Pipeline: " + std::to_string(attributes.numUVs) + "UVs, " + std::to_string(attributes.numWeights) + " skin weight";
    descriptor->setLabel(NS::String::string(label.c_str(), NS::UTF8StringEncoding));
    
    functionContents->release();
    
    __block std::condition_variable *newCondition = new std::condition_variable();
    fConditionMap[attributes] = newCondition;
    if(condOut) {
        *condOut = newCondition;
    }
    
    fMetalDevice->newRenderPipelineState(descriptor, ^(MTL::RenderPipelineState *pipelineState, NS::Error *error){
        if(error) {
            hsAssert(0, error->localizedDescription()->cString(NS::UTF8StringEncoding));
            //leave the condition in place for now, we don't want to
            //retry if the shader is defective. the condition will
            //prevent retries
        } else {
            //update the pipeline state, if it's null just set null
            pipelineState->retain();
            
            plMetalLinkedPipeline *linkedPipeline = new plMetalLinkedPipeline();
            linkedPipeline->pipelineState = pipelineState;
            linkedPipeline->fragFunction = fragFunction;
            linkedPipeline->vertexFunction = vertFunction;
            
            fPipelineStateMap[attributes] = linkedPipeline;
        }
        //signal that we're done
        newCondition->notify_all();
    });
    descriptor->release();
    library->release();
}

plMetalDevice::plMetalLinkedPipeline* plMetalDevice::pipelineStateFor(const plMetalVertexBufferRef * vRef, uint32_t blendFlags, uint32_t numLayers, plShaderID::ID vertexShaderID, plShaderID::ID fragmentShaderID, int forShadows)
{
    plPipelineStateAtrributes attributes = plPipelineStateAtrributes(vRef, blendFlags, fCurrentFragmentOutputTexture->pixelFormat(), fCurrentDepthFormat, vertexShaderID, fragmentShaderID, forShadows, numLayers);
    plMetalLinkedPipeline* renderState = fPipelineStateMap[attributes];
    
    //if it exists, return it, we're done
    if(renderState) {
        return renderState;
    }
    
    //check and see if we're already building it. If so, wait.
    //Note: even if it already exists, this lock will be kept, and it will
    //let us through. This is to prevent race conditions where the render state
    //was null, but maybe in the time it took us to get here the state compiled.
    std::condition_variable *alreadyBuildingCondition = fConditionMap[attributes];
    if(alreadyBuildingCondition) {
        std::unique_lock<std::mutex> lock(fPipelineCreationMtx);
        alreadyBuildingCondition->wait(lock);
        
        //should be returning the render state here, if not it failed to build
        //we'll allow the null return
        return fPipelineStateMap[attributes];
    }
    
    //it doesn't exist, start a build and wait
    //only render thread is allowed to start builds,
    //shouldn't be race conditions here
    StartRenderPipelineBuild(attributes, &alreadyBuildingCondition);
    std::unique_lock<std::mutex> lock(fPipelineCreationMtx);
    alreadyBuildingCondition->wait(lock);
    
    //should be returning the render state here, if not it failed to build
    //we'll allow the null return
    return fPipelineStateMap[attributes];
}

void plMetalDevice::CreateNewCommandBuffer(CA::MetalDrawable* drawable)
{
    printf("Creating new command bufffer\n");
    fCurrentCommandBuffer = fCommandQueue->commandBuffer();
    fCurrentCommandBuffer->retain();
    
    //cache the depth buffer, we'll just clear it every time.
    if(fCurrentDrawableDepthTexture == nullptr ||
       drawable->texture()->width() != fCurrentDrawableDepthTexture->width() ||
       drawable->texture()->height() != fCurrentDrawableDepthTexture->height()
       ) {
        if(fCurrentDrawableDepthTexture) {
            fCurrentDrawableDepthTexture->release();
        }
        
        MTL::TextureDescriptor *depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float_Stencil8,
                                                        drawable->texture()->width(),
                                                        drawable->texture()->height(),
                                                        false);
        if(fMetalDevice->hasUnifiedMemory()) {
            depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
        }   else {
            depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
        }
        depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
        
        fCurrentDrawableDepthTexture = fMetalDevice->newTexture(depthTextureDescriptor);
    }
    
    fCurrentDrawable = drawable->retain();
}

MTL::CommandBuffer* plMetalDevice::GetCurrentCommandBuffer()
{
    return fCurrentCommandBuffer;
}

void plMetalDevice::SubmitCommandBuffer()
{
    printf("Submitting command bufffer\n");
    fCurrentRenderTargetCommandEncoder->endEncoding();
    fCurrentRenderTargetCommandEncoder->release();
    fCurrentRenderTargetCommandEncoder = nil;
    
    fCurrentCommandBuffer->presentDrawable(fCurrentDrawable);
    fCurrentCommandBuffer->enqueue();
    fCurrentCommandBuffer->commit();
    //as we more tightly manage resource sync we may be able to avoid waiting for the frame to complete
    //fCurrentCommandBuffer->waitUntilCompleted();
    fCurrentCommandBuffer->release();
    fCurrentCommandBuffer = nil;
    
    fCurrentDrawable->release();
    fCurrentDrawable = nil;
    
    fClearColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    fShouldClearColor = false;
    fClearDepth = 1.0;
}

MTL::RenderCommandEncoder* plMetalDevice::CurrentRenderCommandEncoder()
{
    //return the current render command encoder
    //if a framebuffer wasn't set, assume screen, emulating GL
    if(fCurrentRenderTargetCommandEncoder) {
        return fCurrentRenderTargetCommandEncoder;
    }
    
    if (!fCurrentRenderTargetCommandEncoder) {
        printf("Asked for command encoder, but one not present. Creating...");
        BeginNewRenderPass();
        
        fClearColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
        fShouldClearColor = false;
        fClearDepth = 1.0;
    }
    
    return fCurrentRenderTargetCommandEncoder;
}

CA::MetalDrawable* plMetalDevice::GetCurrentDrawable()
{
    return fCurrentDrawable;
}
