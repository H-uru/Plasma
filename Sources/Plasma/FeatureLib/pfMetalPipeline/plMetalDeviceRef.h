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
#ifndef _plMetalDeviceRef_inc_
#define _plMetalDeviceRef_inc_

#include "HeadSpin.h"
#include "hsGDeviceRef.h"
#include <Metal/Metal.hpp>
#include <vector>

class plGBufferGroup;
class plBitmap;
class plRenderTarget;


class plMetalDeviceRef : public hsGDeviceRef
{
protected:
    plMetalDeviceRef*  fNext;
    plMetalDeviceRef** fBack;
    
public:
    void            Unlink();
    void            Link(plMetalDeviceRef **back);
    plMetalDeviceRef*  GetNext() { return fNext; }
    bool            IsLinked() { return fBack != nullptr; }
    
    bool HasFlag(uint32_t f) const { return 0 != (fFlags & f); }
    void SetFlag(uint32_t f, bool on) { if(on) fFlags |= f; else fFlags &= ~f; }
    
    virtual void Release() = 0;

    plMetalDeviceRef();
    virtual ~plMetalDeviceRef();
};

/*
 The buffer pool stores and recycles buffers so that Plasma can encode GPU commands and render in parallel. That means we can't touch buffers the GPU is using, and if a pass or frame rewrites a buffer we have to make sure it's not stomping on something that is already attached to a frame. Because Metal can triple buffer, the first dimension of caching is hard coded to 3. Some ages will also rewrite buffers an unspecified number of times between render passes. For example: A reflection render and a main render might have different index buffers. So the second dimension of caching uses an unbounded vector that will hold enough buffers to render in any one age.
 
 Buffer pools do not allocate buffers, they only store them. The outside caller is responsible for allocating a buffer and then setting it. The buffer pool will retain any buffers within the pool, and automatically release them when they are overwritten or the pool is deallocated.
 
 Because buffers are only stored on write, and no allocations happen within the pool, overhead is kept low for static buffers. Completely static buffers will never expand the pool if they only write once.
 */
class plMetalBufferPoolRef : public plMetalDeviceRef {
public:
    uint32_t        fCurrentFrame;
    uint32_t        fCurrentPass;
    uint32_t        fLastWriteFrameTime;
    
    plMetalBufferPoolRef() :
    plMetalDeviceRef(),
        fLastWriteFrameTime(0),
        fCurrentPass(0),
        fCurrentFrame(0),
        fBuffer(nullptr)
    {
    }
    
    //Prepare for write must be called anytime a new pass is going to write a buffer. It moves internal record keeping to reflect that either a new frame or new pass is about to write to the pool.
    void PrepareForWrite() {
        //if we've moved frames since the last time a write happened, reset our current pass index to 0, otherwise increment the current pass
        if(fLastWriteFrameTime != fFrameTime) {
            fCurrentPass = 0;
            fLastWriteFrameTime = fFrameTime;
            fCurrentFrame = (++fCurrentFrame % 3);
        } else {
            fCurrentPass++;
        }
        
        //update the current buffer focused, if the is no buffer to focus set it to null
        uint currentSize = fBuffers[fCurrentFrame].size();
        if(fCurrentPass < currentSize) {
            fBuffer = fBuffers[fCurrentFrame][fCurrentPass];
        } else {
            fBuffer = nullptr;
        }
    }
    
    static void SetFrameTime(uint32_t frameTime) { fFrameTime = frameTime; };
    
    MTL::Buffer* GetBuffer() { return fBuffer; };
    
    void SetBuffer(MTL::Buffer* buffer) {
        fBuffer = buffer->retain();
        uint currentSize = fBuffers[fCurrentFrame].size();
        //if the current vector doesn't have enough room for the entry, resize it
        if(fCurrentPass >= currentSize) {
            fBuffers[fCurrentFrame].resize(++currentSize);
        } else if(fBuffers[fCurrentFrame][fCurrentPass]) {
            //if we're replacing an existing entry, release the old one
            fBuffers[fCurrentFrame][fCurrentPass]->release();
        }
        fBuffers[fCurrentFrame][fCurrentPass] = fBuffer;
    }
    
    void Release() {
        for(int i=0; i<3; i++) {
            for (auto buffer : fBuffers[i]) {
                buffer->release();
            }
        }
        fBuffer = nullptr;
    }
    
private:
    static uint32_t fFrameTime;
    MTL::Buffer*    fBuffer;
    std::vector<MTL::Buffer*> fBuffers[3];
};


class plMetalVertexBufferRef : public plMetalBufferPoolRef
{
public:
    plGBufferGroup* fOwner;
    uint32_t        fCount;
    uint32_t        fIndex;
    uint32_t        fVertexSize;
    int32_t         fOffset;
    uint8_t         fFormat;
    uint8_t*        fData;
    
    uint32_t        fRefTime;
    
    enum {
        kRebuiltSinceUsed   = 0x10, // kDirty = 0x1 is in hsGDeviceRef
        kVolatile           = 0x20,
        kSkinned            = 0x40
    };

    bool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
    void SetRebuiltSinceUsed(bool b) { SetFlag(kRebuiltSinceUsed, b); }

    bool Volatile() const { return HasFlag(kVolatile); }
    void SetVolatile(bool b) { SetFlag(kVolatile, b); }

    bool Skinned() const { return HasFlag(kSkinned); }
    void SetSkinned(bool b) { SetFlag(kSkinned, b); }

    bool Expired(uint32_t t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
    void SetRefTime(uint32_t t) { fRefTime = t; }
    
    plMetalVertexBufferRef() :
    plMetalBufferPoolRef(),
        fCount(0),
        fIndex(0),
        fVertexSize(0),
        fOffset(0),
        fOwner(nullptr),
        fData(nullptr),
        fFormat(0),
        fRefTime(0)
    {
    }
    
    virtual ~plMetalVertexBufferRef();
    
    
    void                    Link(plMetalVertexBufferRef** back ) { plMetalDeviceRef::Link((plMetalDeviceRef**)back); }
    plMetalVertexBufferRef*    GetNext() { return (plMetalVertexBufferRef*)fNext; }
    
    void Release();
};


class plMetalIndexBufferRef : public plMetalBufferPoolRef
{
public:
    uint32_t        fCount;
    uint32_t        fIndex;
    plGBufferGroup* fOwner;
    uint32_t        fRefTime;
    uint32_t        fLastWriteFrameTime;
    
    enum {
        kRebuiltSinceUsed   = 0x10, // kDirty = 0x1 is in hsGDeviceRef
        kVolatile           = 0x20
    };
    
    bool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
    void SetRebuiltSinceUsed(bool b) { SetFlag(kRebuiltSinceUsed, b); }

    bool Volatile() const { return HasFlag(kVolatile); }
    void SetVolatile(bool b) { SetFlag(kVolatile, b); }

    bool Expired(uint32_t t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
    void SetRefTime(uint32_t t) { fRefTime = t; }
    
    void Release();
    
    void                Link(plMetalIndexBufferRef** back) { plMetalDeviceRef::Link((plMetalDeviceRef**)back); }
    plMetalIndexBufferRef* GetNext() { return (plMetalIndexBufferRef*)fNext; }
    virtual ~plMetalIndexBufferRef();
    
    plMetalIndexBufferRef():
    plMetalBufferPoolRef(),
    fCount(0),
    fIndex(0),
    fRefTime(0),
    fLastWriteFrameTime(0),
    fOwner(nullptr) {
    }
};


class plMetalTextureRef : public plMetalDeviceRef
{
public:
    plBitmap*       fOwner;
    
    int32_t        fLevels;
    MTL::Texture*   fTexture;
    MTL::PixelFormat fFormat;
    
    void                Link(plMetalTextureRef** back) { plMetalDeviceRef::Link((plMetalDeviceRef**)back); }
    plMetalTextureRef*  GetNext() { return (plMetalTextureRef*)fNext; }
    
    plMetalTextureRef() :
        plMetalDeviceRef(),
        fOwner(nullptr),
        fTexture(nullptr),
        fLevels(1)
    {
    }
    
    virtual ~plMetalTextureRef();
    
    void Release();
};



class plMetalRenderTargetRef: public plMetalTextureRef
{
public:
    MTL::Texture*   fDepthBuffer;

    void Link(plMetalRenderTargetRef** back) { plMetalDeviceRef::Link((plMetalDeviceRef**)back); }
    plMetalRenderTargetRef*    GetNext() { return (plMetalRenderTargetRef*)fNext; }
    
    plMetalRenderTargetRef() : fDepthBuffer(nullptr)
    {
    }

    virtual ~plMetalRenderTargetRef();

    void Release();

    virtual void SetOwner(plRenderTarget* targ) { fOwner = (plBitmap*)targ; }
};


#endif // _plGLDeviceRef_inc_

