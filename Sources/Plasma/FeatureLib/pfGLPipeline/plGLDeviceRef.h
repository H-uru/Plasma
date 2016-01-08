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
#ifndef _plGLDeviceRef_inc_
#define _plGLDeviceRef_inc_

#include "HeadSpin.h"
#include "hsGDeviceRef.h"

#include <epoxy/gl.h>

// Helper macro for logging GL Errors
#ifdef HS_DEBUGGING
#   include "plStatusLog/plStatusLog.h"
#   define LOG_GL_ERROR_CHECK(message) \
        do { \
            GLenum e; \
            if ((e = glGetError()) != GL_NO_ERROR) { \
                plStatusLog::AddLineSF("pipeline.log", "{}: {}", message, uint32_t(e)); \
            } \
        } while(0);
#else
#   define LOG_GL_ERROR_CHECK(message)
#endif

class plGBufferGroup;
class plBitmap;

class plGLDeviceRef : public hsGDeviceRef
{
public:
    GLuint fRef;

protected:
    plGLDeviceRef*  fNext;
    plGLDeviceRef** fBack;

public:
    void            Unlink();
    void            Link(plGLDeviceRef **back);
    plGLDeviceRef*  GetNext() { return fNext; }
    bool            IsLinked() { return fBack != nullptr; }

    bool HasFlag(uint32_t f) const { return 0 != (fFlags & f); }
    void SetFlag(uint32_t f, bool on) { if(on) fFlags |= f; else fFlags &= ~f; }

    virtual void Release() = 0;

    plGLDeviceRef();
    virtual ~plGLDeviceRef();
};

class plGLVertexBufferRef : public plGLDeviceRef
{
public:
    uint32_t        fCount;
    uint32_t        fIndex;
    uint32_t        fVertexSize;
    int32_t         fOffset;
    uint8_t         fFormat;

    plGBufferGroup* fOwner;
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


    void                    Link(plGLVertexBufferRef** back ) { plGLDeviceRef::Link((plGLDeviceRef**)back); }
    plGLVertexBufferRef*    GetNext() { return (plGLVertexBufferRef*)fNext; }


    plGLVertexBufferRef()
        : plGLDeviceRef(), fCount(), fIndex(), fVertexSize(), fOffset(),
        fFormat(), fOwner(), fData(), fRefTime()
    {}

    virtual ~plGLVertexBufferRef();
    void Release() override;
};


class plGLIndexBufferRef : public plGLDeviceRef
{
public:
    uint32_t            fCount;
    uint32_t            fIndex;
    int32_t             fOffset;
    plGBufferGroup*     fOwner;
    uint32_t            fRefTime;

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


    void                Link(plGLIndexBufferRef** back) { plGLDeviceRef::Link((plGLDeviceRef**)back); }
    plGLIndexBufferRef* GetNext() { return (plGLIndexBufferRef*)fNext; }


    plGLIndexBufferRef()
        : plGLDeviceRef(), fCount(), fIndex(), fOffset(), fOwner(), fRefTime()
    {}

    virtual ~plGLIndexBufferRef();
    void Release() override;
};


class plGLTextureRef : public plGLDeviceRef
{
public:
    plBitmap*       fOwner;
    uint32_t        fLevels;
    GLuint          fMapping;
    GLuint          fFormat;
    GLuint          fDataType;
    GLuint          fDataFormat;

    void             Link(plGLTextureRef** back) { plGLDeviceRef::Link((plGLDeviceRef**)back); }
    plGLTextureRef*  GetNext() { return (plGLTextureRef*)fNext; }

    plGLTextureRef()
        : plGLDeviceRef(), fOwner(), fLevels(1), fMapping(), fFormat(), fDataType(), fDataFormat()
    {}

    virtual ~plGLTextureRef();
    void Release() override;
};


#endif // _plGLDeviceRef_inc_

