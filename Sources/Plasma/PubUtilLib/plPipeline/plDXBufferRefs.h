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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	plDXBufferRefs.h - Hardware Vertex and Index Buffer DeviceRef			 //
//						Definitions											 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	4.25.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXBufferRefs_h
#define _plDXBufferRefs_h

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "plDXDeviceRef.h"

struct IDirect3DVertexShader9;

//// Definitions //////////////////////////////////////////////////////////////

class plDXVertexBufferRef : public plDXDeviceRef
{
	public:
		IDirect3DVertexBuffer9*	fD3DBuffer;
		UInt32					fCount;
		UInt32					fIndex;
		UInt32					fVertexSize;
		Int32					fOffset;
		UInt8					fFormat;

		plGBufferGroup*			fOwner;
		UInt8*					fData;
		IDirect3DDevice9*		fDevice; // For releasing the VertexShader

		UInt32					fRefTime;

		enum {
			kRebuiltSinceUsed	= 0x10, // kDirty = 0x1 is in hsGDeviceRef
			kVolatile			= 0x20,
			kSkinned			= 0x40
		};

		hsBool HasFlag(UInt32 f) const { return 0 != (fFlags & f); }
		void SetFlag(UInt32 f, hsBool on) { if(on) fFlags |= f; else fFlags &= ~f; }

		hsBool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
		void SetRebuiltSinceUsed(hsBool b) { SetFlag(kRebuiltSinceUsed, b); }

		hsBool Volatile() const { return HasFlag(kVolatile); }
		void SetVolatile(hsBool b) { SetFlag(kVolatile, b); }

		hsBool Skinned() const { return HasFlag(kSkinned); }
		void SetSkinned(hsBool b) { SetFlag(kSkinned, b); }

		hsBool Expired(UInt32 t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
		void SetRefTime(UInt32 t) { fRefTime = t; }

		void					Link( plDXVertexBufferRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
		plDXVertexBufferRef*	GetNext() { return (plDXVertexBufferRef *)fNext; }

		plDXVertexBufferRef() :
			fD3DBuffer(nil),
			fCount(0),
			fIndex(0),
			fVertexSize(0),
			fOffset(0),
			fOwner(nil),
			fData(nil),
			fFormat(0),
			fRefTime(0),
			fDevice(nil)
		{
		}

		virtual ~plDXVertexBufferRef();
		void	Release();
};

class plDXIndexBufferRef : public plDXDeviceRef
{
	public:
		IDirect3DIndexBuffer9*	fD3DBuffer;
		UInt32					fCount;
		UInt32					fIndex;
		Int32					fOffset;
		plGBufferGroup*			fOwner;
		UInt32					fRefTime;
		D3DPOOL					fPoolType;

		enum {
			kRebuiltSinceUsed	= 0x10, // kDirty = 0x1 is in hsGDeviceRef
			kVolatile			= 0x20
		};

		hsBool HasFlag(UInt32 f) const { return 0 != (fFlags & f); }
		void SetFlag(UInt32 f, hsBool on) { if(on) fFlags |= f; else fFlags &= ~f; }

		hsBool RebuiltSinceUsed() const { return HasFlag(kRebuiltSinceUsed); }
		void SetRebuiltSinceUsed(hsBool b) { SetFlag(kRebuiltSinceUsed, b); }

		hsBool Volatile() const { return HasFlag(kVolatile); }
		void SetVolatile(hsBool b) { SetFlag(kVolatile, b); }

		hsBool Expired(UInt32 t) const { return Volatile() && (IsDirty() || (fRefTime != t)); }
		void SetRefTime(UInt32 t) { fRefTime = t; }

		void					Link( plDXIndexBufferRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
		plDXIndexBufferRef*	GetNext() { return (plDXIndexBufferRef *)fNext; }

		plDXIndexBufferRef() :
			fD3DBuffer(nil),
			fCount(0),
			fIndex(0),
			fOffset(0),
			fOwner(nil),
			fRefTime(0),
			fPoolType(D3DPOOL_MANAGED)
		{
		}

		virtual ~plDXIndexBufferRef();
		void	Release();
};


#endif // _plDXBufferRefs_h
