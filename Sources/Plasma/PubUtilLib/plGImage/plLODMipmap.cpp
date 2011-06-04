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

#include "hsTypes.h"
#include "plLODMipmap.h"

#include "hsResMgr.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnMessage/plRefMsg.h"

#include "../plPipeline/hsGDeviceRef.h"


plLODMipmap::plLODMipmap() 
:	fBase(nil), 
	fLOD(0)
{
	int i;
	for( i = 0; i < kNumLODs; i++ )
		fDeviceRefs[i] = nil;
}

plLODMipmap::plLODMipmap(plMipmap* mip)
:	fBase(nil),
	fLOD(0)
{
	int i;
	for( i = 0; i < kNumLODs; i++ )
		fDeviceRefs[i] = nil;

	hsgResMgr::ResMgr()->NewKey(mip->GetKey()->GetName(), this, mip->GetKey()->GetUoid().GetLocation());

	// Need some kind of reffing assignment for the mipmap here
	fBase = mip;
	fLevelSizes = TRACKED_NEW UInt32[fBase->GetNumLevels()];

	ISetup();

	hsgResMgr::ResMgr()->AddViaNotify(mip->GetKey(), TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefBase), plRefFlags::kActiveRef);

}

plLODMipmap::~plLODMipmap()
{
	// And matching unreffing of the mipmap here.
	fImage = nil;

	int i;
	for( i = 0; i < kNumLODs; i++ )
	{
		hsRefCnt_SafeUnRef(fDeviceRefs[i]);
		fDeviceRefs[i] = nil;
	}
}

hsGDeviceRef* plLODMipmap::GetDeviceRef() const 
{ 
	return fDeviceRefs[GetLOD()]; 
}

void plLODMipmap::SetDeviceRef( hsGDeviceRef *const devRef )
{
	hsRefCnt_SafeAssign(fDeviceRefs[GetLOD()], devRef);
}


void plLODMipmap::SetLOD(int lod)
{
	hsAssert(fBase, "UnInitialized");

	const kMaxLOD = 5;
	if( lod > kMaxLOD )
		lod = kMaxLOD;
	if( lod >= fBase->GetNumLevels() )
		lod = fBase->GetNumLevels() - 1;

	if( fLOD != lod )
	{
		fLOD = lod;
		ISetup();
	}
}


void plLODMipmap::ISetup()
{
	hsAssert(fBase, "UnInitialized");
	hsAssert(fBase->GetNumLevels() > GetLOD(), "Skipping all mip levels");

	fBase->SetCurrLevel(GetLOD());

	// plBitmap
	fPixelSize = fBase->GetPixelSize();	
//	fSpace = fBase->fSpace;		
	fFlags = fBase->GetFlags();		

	fCompressionType = fBase->fCompressionType;
	if( !fBase->IsCompressed() )
	{
		fUncompressedInfo = fBase->fUncompressedInfo;
	}
	else
	{
		fDirectXInfo = fBase->fDirectXInfo;
	}

	// plMipmap
	fImage = fBase->GetCurrLevelPtr();
	fWidth = fBase->GetCurrWidth();
	fHeight = fBase->GetCurrHeight();

	fRowBytes = fBase->GetRowBytes();

	if( !fLevelSizes )
		fLevelSizes = TRACKED_NEW UInt32[fBase->GetNumLevels()];
		
	fNumLevels = fBase->GetNumLevels() - GetLOD();
	fNumLevels = 1;

	fTotalSize = 0;
	int i;
	for( i = 0; i < fNumLevels; i++ )
	{
		fLevelSizes[i] = fBase->GetLevelSize(i + GetLOD());
		fTotalSize += fBase->GetLevelSize(i + GetLOD());
	}

	ISetupCurrLevel();

	IMarkDirty();
}

void plLODMipmap::ISetupCurrLevel()
{
	fCurrLevelPtr = fBase->GetCurrLevelPtr();
	fCurrLevel = (UInt8)(fBase->GetCurrLevel());
	fCurrLevelWidth = fBase->GetCurrWidth();
	fCurrLevelHeight = fBase->GetCurrHeight();
	fCurrLevelRowBytes = fBase->GetRowBytes();
}

void plLODMipmap::INilify()
{
	fBase = nil;

	fPixelSize = 0;
	fSpace = kNoSpace;
	fFlags = 0;

	fCompressionType = kUncompressed;
	fUncompressedInfo.fType = UncompressedInfo::kRGB8888;

	// plMipmap
	fImage = nil;
	fWidth = 0;
	fHeight = 0;

	fRowBytes = 0;

	fNumLevels = 0;
	fTotalSize = 0;

	fCurrLevelPtr = nil;
	fCurrLevel = 0;
	fCurrLevelWidth = 0;
	fCurrLevelHeight = 0;
	fCurrLevelRowBytes = 0;

	delete [] fLevelSizes;
	fLevelSizes = nil;

	int i;
	for( i = 0; i < kNumLODs; i++ )
	{
		hsRefCnt_SafeUnRef(fDeviceRefs[i]);
		fDeviceRefs[i] = nil;
	}
}

void plLODMipmap::IMarkDirty()
{
	int i;
	for( i = 0; i < kNumLODs; i++ )
	{
		if( fDeviceRefs[i] )
			fDeviceRefs[i]->SetDirty(true);
	}
}

void plLODMipmap::SetCurrLevel(UInt8 level)
{
	fBase->SetCurrLevel(level + GetLOD());

	ISetupCurrLevel();
}

void plLODMipmap::Reset()
{
	fBase->Reset();
	ISetup();
}

void plLODMipmap::ScaleNicely(UInt32 *destPtr, UInt16 destWidth, UInt16 destHeight,
							UInt16 destStride, plMipmap::ScaleFilter filter) const
{
	fBase->ScaleNicely(destPtr, destWidth, destHeight, destStride, filter);
}

hsBool plLODMipmap::ResizeNicely(UInt16 newWidth, UInt16 newHeight, plMipmap::ScaleFilter filter)
{
	hsBool retVal = fBase->ResizeNicely(newWidth, newHeight, filter);
	ISetup();
	return retVal;
}

void plLODMipmap::CopyFrom(const plMipmap *source)
{
	fBase->CopyFrom(source);
	ISetup();
}

void plLODMipmap::Composite(plMipmap *source, UInt16 x, UInt16 y, CompositeOptions *options) 
{ 
	fBase->Composite(source, x, y, options); 
	IMarkDirty();
}

hsBool plLODMipmap::MsgReceive(plMessage *msg)
{
	plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
	if( ref )
	{
		if( ref->fType == kRefBase )
		{
			INilify();
			if( ref->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
			{
				fBase = plMipmap::ConvertNoRef(ref->GetRef());
				ISetup();
			}

			return true;
		}
	}

	return plMipmap::MsgReceive(msg);
}

void plLODMipmap::Read(hsStream *s, hsResMgr *mgr)
{
	INilify();
	hsKeyedObject::Read(s, mgr);
	mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefBase), plRefFlags::kActiveRef); // fBase
}

void plLODMipmap::Write(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Write(s, mgr);
	mgr->WriteKey(s, fBase);
}
