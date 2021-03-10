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
//  plDXBufferRefs.h - Hardware Vertex and Index Buffer DeviceRef            //
//                      Definitions                                          //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  4.25.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXBufferRefs_h
#define _plDXBufferRefs_h

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "plDXDeviceRef.h"
#include "plDrawable/plGBufferGroup.h"

#include "hsWindows.h"
#include <d3d9.h>

struct IDirect3DVertexShader9;

//// Definitions //////////////////////////////////////////////////////////////

class plDXVertexBufferRef : public plDXDeviceRef
{
    public:
        IDirect3DVertexBuffer9* fD3DBuffer;
        uint32_t                  fCount;
        uint32_t                  fIndex;
        uint32_t                  fVertexSize;
        int32_t                   fOffset;
        uint8_t                   fFormat;

        plGBufferGroup*         fOwner;
        uint8_t*                  fData;
        IDirect3DDevice9*       fDevice; // For releasing the VertexShader

        uint32_t                  fRefTime;

        enum {
            kRebuiltSinceUsed   = 0x10, // kDirty = 0x1 is in hsGDeviceRef
            kVolatile           = 0x20,
            kSkinned            = 0x40
        };

        bool HasFlag(uint32_t f) const { return 0 != (fFlags & f); }
        void SetFlag(uint32_t f, bool on) { if(on) fFlags |= f; else fFlags &= ~f; }

        bool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
        void SetRebuiltSinceUsed(bool b) { SetFlag(kRebuiltSinceUsed, b); }

        bool Volatile() const { return HasFlag(kVolatile); }
        void SetVolatile(bool b) { SetFlag(kVolatile, b); }

        bool Skinned() const { return HasFlag(kSkinned); }
        void SetSkinned(bool b) { SetFlag(kSkinned, b); }

        bool Expired(uint32_t t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
        void SetRefTime(uint32_t t) { fRefTime = t; }

        void                    Link( plDXVertexBufferRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
        plDXVertexBufferRef*    GetNext() { return (plDXVertexBufferRef *)fNext; }

        plDXVertexBufferRef() :
            fD3DBuffer(),
            fCount(),
            fIndex(),
            fVertexSize(),
            fOffset(),
            fOwner(),
            fData(),
            fFormat(),
            fRefTime(),
            fDevice()
        {
        }

        virtual ~plDXVertexBufferRef();
        void    Release() override;
};

class plDXIndexBufferRef : public plDXDeviceRef
{
    public:
        IDirect3DIndexBuffer9*  fD3DBuffer;
        uint32_t                  fCount;
        uint32_t                  fIndex;
        int32_t                   fOffset;
        plGBufferGroup*         fOwner;
        uint32_t                  fRefTime;
        D3DPOOL                 fPoolType;

        enum {
            kRebuiltSinceUsed   = 0x10, // kDirty = 0x1 is in hsGDeviceRef
            kVolatile           = 0x20
        };

        bool HasFlag(uint32_t f) const { return 0 != (fFlags & f); }
        void SetFlag(uint32_t f, bool on) { if(on) fFlags |= f; else fFlags &= ~f; }

        bool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
        void SetRebuiltSinceUsed(bool b) { SetFlag(kRebuiltSinceUsed, b); }

        bool Volatile() const { return HasFlag(kVolatile); }
        void SetVolatile(bool b) { SetFlag(kVolatile, b); }

        bool Expired(uint32_t t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
        void SetRefTime(uint32_t t) { fRefTime = t; }

        void                    Link( plDXIndexBufferRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
        plDXIndexBufferRef* GetNext() { return (plDXIndexBufferRef *)fNext; }

        plDXIndexBufferRef() :
            fD3DBuffer(),
            fCount(),
            fIndex(),
            fOffset(),
            fOwner(),
            fRefTime(),
            fPoolType(D3DPOOL_MANAGED)
        {
        }

        virtual ~plDXIndexBufferRef();
        void    Release() override;
};


#endif // _plDXBufferRefs_h
