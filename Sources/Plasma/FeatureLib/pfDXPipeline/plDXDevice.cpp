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

#include "plDXDevice.h"
#include "plDXPipeline.h"

#include "plPipeline/plRenderTarget.h"

#include "plDXBufferRefs.h"
#include "plDXTextureRef.h"
#include "plDXLightRef.h"
#include "plDXRenderTargetRef.h"
#include "plDXVertexShader.h"
#include "plDXPixelShader.h"

#include <string_theory/string>

//// Macros for D3D error handling
#define INIT_ERROR_CHECK(cond, errMsg) if (FAILED(fSettings.fDXError = cond)) { return ICreateFail(ST_LITERAL(errMsg)); }

#if 1       // DEBUG
#define STRONG_ERROR_CHECK( cond ) if( FAILED( fPipeline->fSettings.fDXError = cond ) ) { fPipeline->IGetD3DError(); fPipeline->IShowErrorMessage(); }   
#define WEAK_ERROR_CHECK( cond )    STRONG_ERROR_CHECK( cond )
#else
#define STRONG_ERROR_CHECK( cond ) if( FAILED( fPipeline->fSettings.fDXError = cond ) ) { fPipeline->IGetD3DError(); }    
#define WEAK_ERROR_CHECK( cond )    cond
#endif


/// Macros for getting/setting data in a D3D vertex buffer
template<typename T>
static inline void inlCopy(uint8_t*& src, uint8_t*& dst)
{
    T* src_ptr = reinterpret_cast<T*>(src);
    T* dst_ptr = reinterpret_cast<T*>(dst);
    *dst_ptr = *src_ptr;
    src += sizeof(T);
    dst += sizeof(T);
}

static inline void inlCopy(const uint8_t*& src, uint8_t*& dst, size_t sz)
{
    memcpy(dst, src, sz);
    src += sz;
    dst += sz;
}

template<typename T, size_t N>
static inline void inlSkip(uint8_t*& src)
{
    src += sizeof(T) * N;
}


static D3DMATRIX d3dIdentityMatrix{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

D3DMATRIX& IMatrix44ToD3DMatrix(D3DMATRIX& dst, const hsMatrix44& src)
{
    if (src.fFlags & hsMatrix44::kIsIdent) {
        dst = d3dIdentityMatrix;
    } else {
        dst.m[0][0] = src.fMap[0][0];
        dst.m[1][0] = src.fMap[0][1];
        dst.m[2][0] = src.fMap[0][2];
        dst.m[3][0] = src.fMap[0][3];

        dst.m[0][1] = src.fMap[1][0];
        dst.m[1][1] = src.fMap[1][1];
        dst.m[2][1] = src.fMap[1][2];
        dst.m[3][1] = src.fMap[1][3];

        dst.m[0][2] = src.fMap[2][0];
        dst.m[1][2] = src.fMap[2][1];
        dst.m[2][2] = src.fMap[2][2];
        dst.m[3][2] = src.fMap[2][3];

        dst.m[0][3] = src.fMap[3][0];
        dst.m[1][3] = src.fMap[3][1];
        dst.m[2][3] = src.fMap[3][2];
        dst.m[3][3] = src.fMap[3][3];
    }

    return dst;
}


plDXDevice::plDXDevice()
:   fD3DDevice(), fD3DMainSurface(), fD3DDepthSurface(), fD3DBackBuff(),
    fCurrCullMode(D3DCULL_CW), fManagedAlloced(), fAllocUnManaged()
{
}

void plDXDevice::SetRenderTarget(plRenderTarget* target)
{
    IDirect3DSurface9* main;
    IDirect3DSurface9* depth;
    plDXRenderTargetRef* ref = nullptr;


    if (target != nullptr)
    {
        ref = (plDXRenderTargetRef*)target->GetDeviceRef();
        if (ref == nullptr || ref->IsDirty())
            ref = (plDXRenderTargetRef*)fPipeline->MakeRenderTargetRef(target);
    }

    if (ref == nullptr || ref->GetColorSurface() == nullptr)
    {
        /// Set to main screen
        main = fD3DMainSurface;
        depth = fD3DDepthSurface;
    }
    else
    {
        /// Set to this target
        main = ref->GetColorSurface();
        depth = ref->fD3DDepthSurface;
    }

    if (main != fCurrD3DMainSurface || depth != fCurrD3DDepthSurface)
    {
        fCurrD3DMainSurface = main;
        fCurrD3DDepthSurface = depth;
        fD3DDevice->SetRenderTarget(0, main);
        fD3DDevice->SetDepthStencilSurface(depth);
    }

    fPipeline->IInvalidateState();

    SetViewport();
}

void plDXDevice::SetViewport()
{
    D3DVIEWPORT9 vp = { (DWORD)fPipeline->GetViewTransform().GetViewPortLeft(),
                        (DWORD)fPipeline->GetViewTransform().GetViewPortTop(),
                        (DWORD)fPipeline->GetViewTransform().GetViewPortWidth(),
                        (DWORD)fPipeline->GetViewTransform().GetViewPortHeight(),
                        0.f, 1.f };

    WEAK_ERROR_CHECK(fD3DDevice->SetViewport(&vp));
}

/**
 * Calculate the vertex stride from the given format.
 */
static uint32_t GetBufferFormatSize(uint8_t format)
{
    uint32_t  size = sizeof(float) * 6 + sizeof(uint32_t) * 2; // Position and normal, and two packed colors

    switch (format & plGBufferGroup::kSkinWeightMask) {
        case plGBufferGroup::kSkinNoWeights:
            break;
        case plGBufferGroup::kSkin1Weight:
            size += sizeof(float);
            break;
        default:
            hsAssert(false, "Invalid skin weight value in GetBufferFormatSize()");
    }

    size += sizeof(float) * 3 * plGBufferGroup::CalcNumUVs(format);

    return size;
}

void plDXDevice::SetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, VertexBufferRef* vRef)
{
    // Initialize to nullptr, in case something goes wrong.
    vRef->fD3DBuffer = nullptr;

    uint8_t format = owner->GetVertexFormat();

    // All indexed skinning is currently done on CPU, so the source data
    // will have indices, but we strip them out for the D3D buffer.
    if (format & plGBufferGroup::kSkinIndices) {
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights;       // Should do nothing, but just in case...
        vRef->SetSkinned(true);
        vRef->SetVolatile(true);
    }

    uint32_t vertSize = GetBufferFormatSize(format); // vertex stride
    uint32_t numVerts = owner->GetVertBufferCount(idx);

    vRef->fDevice = fD3DDevice;

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

    owner->SetVertexBufferRef(idx, vRef);
    hsRefCnt_SafeUnRef(vRef);
}

void plDXDevice::CheckStaticVertexBuffer(VertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx)
{
    if (fAllocUnManaged)
        return;

    hsAssert(!vRef->Volatile(), "Creating a managed vertex buffer for a volatile buffer ref");

    if (!vRef->fD3DBuffer) {
        // Okay, haven't done this one.
        DWORD fvfFormat = fPipeline->IGetBufferD3DFormat(vRef->fFormat);

        D3DPOOL poolType = D3DPOOL_MANAGED;
        //DWORD usage = D3DUSAGE_WRITEONLY;
        DWORD usage = 0;
        const int numVerts = vRef->fCount;
        const int vertSize = vRef->fVertexSize;
        fManagedAlloced = true;

        if (FAILED(fD3DDevice->CreateVertexBuffer(numVerts * vertSize,
                                                  usage,
                                                  fvfFormat,
                                                  poolType,
                                                  &vRef->fD3DBuffer, nullptr)))
        {
            hsAssert(false, "CreateVertexBuffer() call failed!");
            vRef->fD3DBuffer = nullptr;
            return;
        }
        PROFILE_POOL_MEM(poolType, numVerts * vertSize, true, ST_LITERAL("VtxBuff"));

        // Fill in the vertex data.
        FillStaticVertexBufferRef(vRef, owner, idx);

        // This is currently a no op, but this would let the buffer know it can
        // unload the system memory copy, since we have a managed version now.
        owner->PurgeVertBuffer(idx);
    }
}

void plDXDevice::FillStaticVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    IDirect3DVertexBuffer9* vertexBuff = ref->fD3DBuffer;

    if (!vertexBuff)
        // We most likely already warned about this earlier, best to just quietly return now
        return;

    const uint32_t vertSize = ref->fVertexSize;
    const uint32_t vertStart = group->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = group->GetVertBufferEnd(idx) * vertSize - vertStart;
    if (!size)
        return;

    /// Lock the buffer
    uint8_t* ptr;
    if (FAILED(vertexBuff->Lock(vertStart, size, (void **)&ptr, group->AreVertsVolatile() ? D3DLOCK_DISCARD : 0)))
        hsAssert(false, "Failed to lock vertex buffer for writing");

    if (ref->fData) {
        memcpy(ptr, ref->fData + vertStart, size);
    } else {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size, "Trailing dead space on non-interleaved data not supported");

        const uint32_t vertSmallSize = group->GetVertexLiteStride() - sizeof(hsPoint3) * 2;
        uint8_t* srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* const srcCPtr = group->GetColorBufferData(idx);

        const int numCells = group->GetNumCells(idx);
        for (int i = 0; i < numCells; i++) {
            plGBufferCell   *cell = group->GetCell(idx, i);

            if (cell->fColorStart == uint32_t(-1)) {
                /// Interleaved, do straight copy
                memcpy(ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize);
                ptr += cell->fLength * vertSize;
            } else {
                /// Separated, gotta interleave
                uint8_t* tempVPtr = srcVPtr + cell->fVtxStart;
                plGBufferColor* tempCPtr = srcCPtr + cell->fColorStart;

                for (int j = 0; j < cell->fLength; j++) {
                    memcpy(ptr, tempVPtr, sizeof(hsPoint3) * 2);
                    ptr += sizeof(hsPoint3) * 2;
                    tempVPtr += sizeof(hsPoint3) * 2;

                    memcpy(ptr, &tempCPtr->fDiffuse, sizeof(uint32_t));
                    ptr += sizeof(uint32_t);
                    memcpy(ptr, &tempCPtr->fSpecular, sizeof(uint32_t));
                    ptr += sizeof(uint32_t);

                    memcpy(ptr, tempVPtr, vertSmallSize);
                    ptr += vertSmallSize;
                    tempVPtr += vertSmallSize;
                    tempCPtr++;
                }
            }
        }
    }

    /// Unlock and clean up
    vertexBuff->Unlock();
    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plDXDevice::FillVolatileVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    uint8_t* dst = ref->fData;
    uint8_t* src = group->GetVertBufferData(idx);

    size_t uvChanSize = plGBufferGroup::CalcNumUVs(group->GetVertexFormat()) * sizeof(float) * 3;
    uint8_t numWeights = (group->GetVertexFormat() & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < ref->fCount; ++i) {
        inlCopy<hsPoint3>(src, dst); // pre-pos
        src += numWeights * sizeof(float); // weights
        if (group->GetVertexFormat() & plGBufferGroup::kSkinIndices)
            inlSkip<uint32_t, 1>(src); // indices
        inlCopy<hsVector3>(src, dst); // pre-normal
        inlCopy<uint32_t>(src, dst); // diffuse
        inlCopy<uint32_t>(src, dst); // specular

        // UVWs
        memcpy(dst, src, uvChanSize);
        src += uvChanSize;
        dst += uvChanSize;
    }
}

void plDXDevice::SetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, IndexBufferRef* iRef)
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

void plDXDevice::CheckIndexBuffer(IndexBufferRef* iRef)
{
    if (!iRef->fD3DBuffer && iRef->fCount) {
        D3DPOOL poolType = fAllocUnManaged ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
        DWORD usage = D3DUSAGE_WRITEONLY;
        iRef->SetVolatile(false);
        if (FAILED(fD3DDevice->CreateIndexBuffer(sizeof(uint16_t) * iRef->fCount,
                                                    usage,
                                                    D3DFMT_INDEX16,
                                                    poolType,
                                                    &iRef->fD3DBuffer, nullptr)))
        {
            hsAssert(false, "CreateIndexBuffer() call failed!");
            iRef->fD3DBuffer = nullptr;
            return;
        }
        PROFILE_POOL_MEM(poolType, sizeof(uint16_t) * iRef->fCount, true, ST_LITERAL("IndexBuff"));

        iRef->fPoolType = poolType;
        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

void plDXDevice::FillIndexBufferRef(IndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx)
{
    uint32_t startIdx = owner->GetIndexBufferStart(idx);
    uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);
    if (!size)
        return;

    DWORD lockFlags = iRef->Volatile() ? D3DLOCK_DISCARD : 0;
    uint16_t* destPtr = nullptr;
    if (FAILED(iRef->fD3DBuffer->Lock(startIdx * sizeof(uint16_t), size, (void **)&destPtr, lockFlags))) {
        hsAssert(false, "Cannot lock index buffer for writing");
        return;
    }

    memcpy(destPtr, owner->GetIndexBufferData(idx) + startIdx, size);

    iRef->fD3DBuffer->Unlock();
    iRef->SetDirty(false);
}

void plDXDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    D3DMATRIX mat;

    IMatrix44ToD3DMatrix(mat, src);

    fD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);
}

void plDXDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    D3DMATRIX mat;

    IMatrix44ToD3DMatrix(mat, src);

    fD3DDevice->SetTransform(D3DTS_VIEW, &mat);
}

void plDXDevice::SetLocalToWorldMatrix(const hsMatrix44& src)
{
    D3DMATRIX mat;

    IMatrix44ToD3DMatrix(mat, src);

    fD3DDevice->SetTransform(D3DTS_WORLD, &mat);
}

ST::string plDXDevice::GetErrorString() const
{
    return fPipeline->fSettings.fErrorStr;
}

void plDXDevice::BeginAllocUnManaged()
{
    // Flush out all managed resources to make room for unmanaged resources.
    fD3DDevice->EvictManagedResources();

    fManagedAlloced = false;
    fAllocUnManaged = true; // we're currently only allocating POOL_DEFAULT
}

void plDXDevice::EndAllocUnManaged()
{
    fAllocUnManaged = false;

    // Flush the (should be empty) resource manager to reset its internal allocation pool.
    fD3DDevice->EvictManagedResources();
}
