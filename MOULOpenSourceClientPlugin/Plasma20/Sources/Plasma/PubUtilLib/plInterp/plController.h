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
#ifndef HSCONTROLLER_inc
#define HSCONTROLLER_inc

#include "HeadSpin.h"
#include "../pnFactory/plCreatable.h"
#include "hsColorRGBA.h"
#include "hsKeys.h"
#include "hsTemplates.h"

class hsResMgr;

struct hsScaleValue;
struct hsScalarKey;
struct hsPoint3Key;
struct hsScalarTriple;
struct hsMatrix33;
struct hsMatrix44;
class hsQuat;
class hsAffineParts;
class plScalarCurve;
class plAnimTimeConvert;
class plCompoundController;

//
//////////////////////////////////////////////////////////////
// base controller class.
// Controllers correspond to Max controllers.
// Some are leaf controllers which actually have keys, these can also have
// multiple ease and multiplier controllers.
// Some are compound controllers, which just contain other (leaf) controllers.
// Leaf controllers have lists of keys (or plCurves which are just wrappers for
// the lists of keys).
//

class plControllerCacheInfo
{
public:
	UInt8 fNumSubControllers;	
	plControllerCacheInfo **fSubControllers;

	UInt32 fKeyIndex;
	plAnimTimeConvert *fAtc;

	plControllerCacheInfo();
	~plControllerCacheInfo();

	void SetATC(plAnimTimeConvert *atc);
};

//
//////////////////////////////////////////////////////////////
// defines base methods
//
class plController : public plCreatable
{
public:
	CLASSNAME_REGISTER( plController );
	GETINTERFACE_ANY( plController, plCreatable );

	virtual void Interp(hsScalar time, hsScalar* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsScalarTriple* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsScaleValue* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsQuat* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsMatrix33* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsMatrix44* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsColorRGBA* result, plControllerCacheInfo *cache = nil) const {}
	virtual void Interp(hsScalar time, hsAffineParts* parts, plControllerCacheInfo *cache = nil) const {}

	virtual plControllerCacheInfo* CreateCache() const { return nil; } // Caller must handle deleting the pointer.
	virtual hsScalar GetLength() const = 0;
	virtual void GetKeyTimes(hsTArray<hsScalar> &keyTimes) const = 0;
	virtual hsBool AllKeysMatch() const = 0;

	// Checks each of our subcontrollers (if we have any) and deletes any that
	// are nothing but matching keys. Returns true if this controller itself
	// is redundant.
	virtual hsBool PurgeRedundantSubcontrollers() = 0;
};

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

class plLeafController : public plController 
{
	friend class plCompoundController;

protected:
	UInt8 fType;
	void *fKeys; // Need to pay attend to fType to determine what these actually are
	UInt32 fNumKeys;
	mutable UInt32 fLastKeyIdx;

public:
	plLeafController() : fType(hsKeyFrame::kUnknownKeyFrame), fKeys(nil), fNumKeys(0), fLastKeyIdx(0) {}
	virtual ~plLeafController();

	CLASSNAME_REGISTER( plLeafController );
	GETINTERFACE_ANY( plLeafController, plController );

	void Interp(hsScalar time, hsScalar* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsScalarTriple* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsScaleValue* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsQuat* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsMatrix33* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsMatrix44* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsColorRGBA* result, plControllerCacheInfo *cache = nil) const;

	virtual plControllerCacheInfo* CreateCache() const;
	hsScalar GetLength() const;
	UInt32 GetStride() const;

	hsPoint3Key *GetPoint3Key(UInt32 i) const;
	hsBezPoint3Key *GetBezPoint3Key(UInt32 i) const;
	hsScalarKey *GetScalarKey(UInt32 i) const;
	hsBezScalarKey *GetBezScalarKey(UInt32 i) const;
	hsScaleKey *GetScaleKey(UInt32 i) const;
	hsBezScaleKey *GetBezScaleKey(UInt32 i) const;
	hsQuatKey *GetQuatKey(UInt32 i) const;
	hsCompressedQuatKey32 *GetCompressedQuatKey32(UInt32 i) const;
	hsCompressedQuatKey64 *GetCompressedQuatKey64(UInt32 i) const;
	hsG3DSMaxKeyFrame *Get3DSMaxKey(UInt32 i) const;
	hsMatrix33Key *GetMatrix33Key(UInt32 i) const;
	hsMatrix44Key *GetMatrix44Key(UInt32 i) const;

	UInt8 GetType() const { return fType; }
	UInt32 GetNumKeys() const { return fNumKeys; }
	void *GetKeyBuffer() const { return fKeys; }
	void GetKeyTimes(hsTArray<hsScalar> &keyTimes) const;
	void AllocKeys(UInt32 n, UInt8 type);
	void QuickScalarController(int numKeys, hsScalar* times, hsScalar* values, UInt32 valueStrides);
	hsBool AllKeysMatch() const;
	hsBool PurgeRedundantSubcontrollers();

	void Read(hsStream* s, hsResMgr* mgr);
	void Write(hsStream* s, hsResMgr* mgr);
};


////////////////////////////////////////////////////////////////////////////////
// NON-LEAF (container) CONTROLLERS
////////////////////////////////////////////////////////////////////////////////
//

class plCompoundController : public plController
{
private:
	plController* fXController;
	plController* fYController;
	plController* fZController;

public:
	plCompoundController();	// allocs leaf controllers
	~plCompoundController();

	CLASSNAME_REGISTER( plCompoundController );
	GETINTERFACE_ANY( plCompoundController, plController );

	void Interp(hsScalar time, hsQuat* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsScalarTriple* result, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsAffineParts* parts, plControllerCacheInfo *cache = nil) const;
	void Interp(hsScalar time, hsColorRGBA* result, plControllerCacheInfo *cache = nil) const;

	plControllerCacheInfo* CreateCache() const;
	plController *GetXController() const { return fXController; }
	plController *GetYController() const { return fYController; }
	plController *GetZController() const { return fZController; }
	plController *GetPosController() const { return fXController; }
	plController *GetRotController() const { return fYController; }
	plController *GetScaleController() const { return fZController; }
	plController *GetController(Int32 i) const;
	hsScalar GetLength() const;
	void GetKeyTimes(hsTArray<hsScalar> &keyTimes) const;
	hsBool AllKeysMatch() const;
	hsBool PurgeRedundantSubcontrollers();

	void SetXController(plController *c) { delete fXController; fXController = c; }
	void SetYController(plController *c) { delete fYController; fYController = c; }
	void SetZController(plController *c) { delete fZController; fZController = c; }
	void SetPosController(plController *c) { delete fXController; fXController = c; }
	void SetRotController(plController *c) { delete fYController; fYController = c; }
	void SetScaleController(plController *c) { delete fZController; fZController = c; }
	void SetController(Int32 i, plController* c);

	void Read(hsStream* s, hsResMgr* mgr);
	void Write(hsStream* s, hsResMgr* mgr);
};

#endif

