//
//  plMetalDeviceRef.cpp
//  CoreLib
//
//  Created by Colin Cornaby on 11/10/21.
//

#include "plMetalDeviceRef.h"



void plMetalDeviceRef::Unlink() {
    hsAssert(fBack, "plGLDeviceRef not in list");

    if (fNext)
        fNext->fBack = fBack;
    *fBack = fNext;

    fBack = nullptr;
    fNext = nullptr;
    
}

void plMetalDeviceRef::Link(plMetalDeviceRef **back) {
    hsAssert(fNext == nullptr && fBack == nullptr, "Trying to link a plMetalDeviceRef that's already linked");

    fNext = *back;
    if (*back)
        (*back)->fBack = &fNext;
    fBack = back;
    *back = this;
}

plMetalDeviceRef::~plMetalDeviceRef()
{
    if (fNext != nullptr || fBack != nullptr)
        Unlink();
}

plMetalVertexBufferRef::~plMetalVertexBufferRef()
{
    Release();
}


void plMetalVertexBufferRef::Release()
{
    SetDirty(true);
}

plMetalTextureRef::~plMetalTextureRef()
{
    //fTexture->release();
    Release();
}


void plMetalTextureRef::Release()
{
    SetDirty(true);
}


plMetalIndexBufferRef::~plMetalIndexBufferRef()
{
    Release();
}


void plMetalIndexBufferRef::Release()
{
    SetDirty(true);
}


plMetalRenderTargetRef::~plMetalRenderTargetRef() {
    Release();
}

void plMetalRenderTargetRef::Release()
{
    SetDirty(true);
}
