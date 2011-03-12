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
#include "plSynchedValue.h"

#ifdef USE_SYNCHED_VALUES
#include "../pnKeyedObject/plKey.h"
#include "hsBitVector.h"
#include "../pnSceneObject/plSceneObject.h"
#include "hsResMgr.h"
#include "../pnKeyedObject/plUoid.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#define ISaveOrLoadSimpleType() \
{ \
	if (save) \
		stream->WriteSwap(v); \
	else \
		stream->ReadSwap(&v); \
	return v; \
}

//
// baseclass save/load methods for various types
//
hsBitVector plSynchedValueBase::ISaveOrLoad(hsBitVector& v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
{
	if (save)
		v.Write(stream);
	else
		v.Read(stream);
	return v;
}

hsScalar plSynchedValueBase::ISaveOrLoad(hsScalar v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	ISaveOrLoadSimpleType();

double plSynchedValueBase::ISaveOrLoad(double v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	ISaveOrLoadSimpleType();

Int32 plSynchedValueBase::ISaveOrLoad(Int32 v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	ISaveOrLoadSimpleType();

UInt32 plSynchedValueBase::ISaveOrLoad(UInt32 v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	ISaveOrLoadSimpleType();

int plSynchedValueBase::ISaveOrLoad(int v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	ISaveOrLoadSimpleType();

bool plSynchedValueBase::ISaveOrLoad(bool v, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
	ISaveOrLoadSimpleType();

//
// save or load a key.  return the key.
//
const plKey plSynchedValueBase::ISaveOrLoad(const plKey key, hsBool32 save, hsStream* stream, hsResMgr* mgr)
{
	if (save)
	{	
		if (key)
		{
			stream->WriteByte(1);
			// I need to write a key to MY stream...
#if 0		// DEBUG
			Int32 len = hsStrlen(key->GetName());
			stream->WriteSwap32(len);
			stream->Write(len, key->GetName());
#endif
			key->GetUoid().Write(stream);
		}
		else
		{
			stream->WriteByte(0);
		}
		return key;
	}
	else
	{	
		Int32 has=stream->ReadByte();
        if (has)
		{
			// read a key from MY stream
#if 0		// DEBUG
			Int32 len = stream->ReadSwap32();
			char tmp[256];
			hsAssert(len<256, "key name overflow");
			stream->Read(len, tmp);
#endif
			plUoid uoid;
			uoid.Read(stream);
            return mgr->FindKey(uoid);
		}
        else
            return (nil);
    }
	return nil;
}

hsKeyedObject* plSynchedValueBase::ISaveOrLoad(hsKeyedObject* obj, hsBool32 save, hsStream* stream, hsResMgr* mgr) 
{
	plKey key = obj ? obj->GetKey() : nil;
	key = ISaveOrLoad(key, save, stream, mgr);
	return key ? key->ObjectIsLoaded() : nil;
}

plSceneNode* plSynchedValueBase::ISaveOrLoad(plSceneNode* obj, hsBool32 save, hsStream* stream, hsResMgr* mgr)
{ 
	// return plSceneNode::ConvertNoRef(ISaveOrLoad(hsKeyedObject::ConvertNoRef(obj), save, stream, mgr)); 
	hsAssert(false, "SceneNode synchedValues currently not implemented");
	return nil;
}

plSceneObject* plSynchedValueBase::ISaveOrLoad(plSceneObject* obj, hsBool32 save, hsStream* stream, hsResMgr* mgr)
{ return plSceneObject::ConvertNoRef(ISaveOrLoad(hsKeyedObject::ConvertNoRef(obj), save, stream, mgr)); }

plCoordinateInterface* plSynchedValueBase::ISaveOrLoad(const plCoordinateInterface* cInt, hsBool32 save, hsStream* stream, hsResMgr* mgr)
{
	plSceneObject* obj = ISaveOrLoad(cInt ? cInt->fOwner : nil, save, stream, mgr);
	return obj ? obj->fCoordinateInterface : nil;
}

#else

// dummy function to prevent a linker warning complaining about no public symbols if the
// contents of the file get compiled out via pre-processor
void _preventLNK4221WarningStub()
{
}

#endif	// USE_SYNCHED_VALUES

