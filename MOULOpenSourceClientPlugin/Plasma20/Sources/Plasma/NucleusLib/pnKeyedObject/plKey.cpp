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
//////////////////////////////////////////////////////////////////////////////
//
//	plKey - An opaque pointer to actual key data, so that we can keep track
//			of how many people have pointers (plKeys) to key data (plKeyImp)
//			and destroy the key data when it's been fully unreffed.
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plKey.h"
#include "plUoid.h"
#include <string.h>
#include "hsResMgr.h"

#define TRACK_REFS 0 // MEMLEAKFISH

#if TRACK_REFS
#include "plCreatableIndex.h"
#include "plClassIndexMacros.h"
#include "plTweak.h"

int mlfTrack = 1;

static const char* keyNameToLookFor = "AgeSDLHook";
static const UInt16 CLASS_TO_TRACK = CLASS_INDEX_SCOPED(plSceneObject);
static const int kCloneID = 0;
static const int kClonePlayerID = 0;
static plKeyData* lastData = nil;
static const int kLocSeq = -1;

class keyDataFriend : public plKeyData
{
public:
	UInt16 RefCount() const { return fRefCount; }
};

static int IsTracked(const plKeyData* keyData)
{
	if( mlfTrack && keyData )
	{
		if( keyData->GetUoid().GetObjectName() && !stricmp(keyData->GetUoid().GetObjectName(), keyNameToLookFor)
			&& (keyData->GetUoid().GetClassType() == CLASS_TO_TRACK) )
		{
			if( (kCloneID < 0)
				||(kCloneID == keyData->GetUoid().GetCloneID()) )
			{
				if( (kLocSeq < 0)
					||(kLocSeq == keyData->GetUoid().GetLocation().GetSequenceNumber()) )
				{
					plConst(UInt16) kMinRefCount(0);
					const keyDataFriend* kdf = (keyDataFriend*)keyData;
					if( kdf->RefCount() > kMinRefCount )
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

static const char* CloneString(const plKeyData* keyData)
{
	static char buff[256];
	if( keyData )
	{
		sprintf(buff, "CID:%d, CPID:%d LOC:%d", 
			keyData->GetUoid().GetCloneID(), 
			keyData->GetUoid().GetClonePlayerID(), 
			keyData->GetUoid().GetLocation().GetSequenceNumber());
	}
	else
	{
		sprintf(buff, "nil");
	}
	return buff;
}
#endif


plKey::plKey() : fKeyData(nil)
{
}

plKey::plKey(void* ptr) : fKeyData(nil)
{
	hsAssert(!ptr, "Attempting to publically construct a non-nil key");
}

plKey::plKey(const plKey& rhs) : fKeyData(rhs.fKeyData)
{
#if TRACK_REFS	// FOR DEBUGGING ONLY
	if( IsTracked(fKeyData) )
	{
		char msg[ 512 ];
		sprintf( msg, "C: Key %s %s is being constructed using the plKey(plKey&) constructor", keyNameToLookFor, CloneString(fKeyData) );
		//hsAssert( false, msg );
		hsStatusMessageF(msg);
	}
#endif
	IIncRef();
}

plKey::plKey(plKeyData* data, hsBool ignore) : fKeyData(data)
{
#if TRACK_REFS	// FOR DEBUGGING ONLY
	if( IsTracked(fKeyData) )
	{
		char msg[ 512 ];
		sprintf( msg, "C: Key %s %s is being constructed using the plKey(plKeyData*, hsBool) constructor", keyNameToLookFor, CloneString(fKeyData) );
		//hsAssert( false, msg );
		hsStatusMessageF(msg);
	}
#endif
	IIncRef();
}

plKey::~plKey()
{
#if TRACK_REFS	// FOR DEBUGGING ONLY
	if( IsTracked(fKeyData) )
	{
		char msg[ 512 ];
		sprintf( msg, "D: Key %s %s is being destructed", keyNameToLookFor, CloneString(fKeyData) );
		//hsAssert( false, msg );
		hsStatusMessageF(msg);
	}
#endif
	IDecRef();
}

plKey &plKey::operator=( const plKey &rhs )
{
#if TRACK_REFS	// FOR DEBUGGING ONLY
	if( fKeyData != rhs.fKeyData )
	{
		if( IsTracked(rhs.fKeyData) )
		{
			char msg[ 512 ];
			if (fKeyData == nil)
				sprintf( msg, "=: Key %s %s is being assigned to a nil key", keyNameToLookFor, CloneString(rhs.fKeyData) );
			else
				sprintf( msg, "=: Key %s %s is being assigned to %s", keyNameToLookFor, CloneString(rhs.fKeyData), fKeyData->GetUoid().GetObjectName() );
			//hsAssert( false, msg );
			hsStatusMessageF(msg);
		}
		if( IsTracked(fKeyData) )
		{
			char msg[ 512 ];
			if (fKeyData == nil)
				sprintf( msg, "=: Nil key is being assigned to %s %s", keyNameToLookFor, CloneString(fKeyData) );
			else
				sprintf( msg, "=: Key %s %s is being assigned to %s", fKeyData->GetUoid().GetObjectName(), CloneString(fKeyData), keyNameToLookFor );
			//hsAssert( false, msg );
			hsStatusMessageF(msg);
		}
	}
#endif
	if( fKeyData != rhs.fKeyData )
	{
		IDecRef();
		fKeyData = rhs.fKeyData;
		IIncRef();
	}

	return *this;
}

hsBool plKey::operator==( const plKey &rhs ) const
{
	return fKeyData == rhs.fKeyData;
}

hsBool plKey::operator==( const plKeyData *rhs ) const
{
	return fKeyData == rhs;
}

plKeyData	*plKey::operator->() const
{
	return fKeyData;
}

plKeyData	&plKey::operator*() const
{
	return *fKeyData;
}

void plKey::IIncRef()
{
	if (!fKeyData)
		return;

	hsAssert(fKeyData->fRefCount < 0xffff, "Too many refs to plKeyImp");
	fKeyData->fRefCount++;

#if TRACK_REFS	// FOR DEBUGGING ONLY
	if( IsTracked(fKeyData) )
	{
		char msg[ 512 ];
		plConst(int) kMaxCnt(30);
		if( fKeyData->fRefCount > kMaxCnt )
			*msg = 0;
		if( lastData && (fKeyData != lastData) )
			*msg = 0;
		lastData = fKeyData;
		sprintf( msg, "+: Key %s %s is being reffed! Refcount: %d", keyNameToLookFor, CloneString(fKeyData), fKeyData->fRefCount );
		//hsAssert( false, msg );
		hsStatusMessageF(msg);
	}
#endif

	if (fKeyData->fRefCount == 1)
		hsgResMgr::ResMgr()->IKeyReffed((plKeyImp*)fKeyData);
}

void plKey::IDecRef()
{
	if (!fKeyData)
		return;

	hsAssert(fKeyData->fRefCount, "Dec'ing ref on unreffed key");
	fKeyData->fRefCount--;

#if TRACK_REFS	// FOR DEBUGGING ONLY
	if( IsTracked(fKeyData) )
	{
		char msg[ 512 ];
		plConst(int) kMinCnt(2);
		if( fKeyData->fRefCount < kMinCnt )
			*msg = 0;
		if( lastData && (fKeyData != lastData) )
			*msg = 0;
		lastData = fKeyData;
		sprintf( msg, "-: Key %s %s is being de-reffed! Refcount: %d", keyNameToLookFor, CloneString(fKeyData), fKeyData->fRefCount );
		//hsAssert( false, msg );
		hsStatusMessageF(msg);
		if( fKeyData->fRefCount == 0 )
			*msg = 0;
	}
#endif

	if (fKeyData->fRefCount == 0)
		hsgResMgr::ResMgr()->IKeyUnreffed((plKeyImp*)fKeyData);
}

//// plKeyData ///////////////////////////////////////////////////////////////
//	Our base class of key data

plKeyData::plKeyData()
{
	fRefCount = 0;
}

plKeyData::~plKeyData()
{
	//	hsAssert(fRefCount == 0, "Deleting key data when someone still has a ref to it");
}


