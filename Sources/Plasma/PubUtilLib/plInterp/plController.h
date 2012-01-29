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
#ifndef HSCONTROLLER_inc
#define HSCONTROLLER_inc

#include "HeadSpin.h"
#include "pnFactory/plCreatable.h"
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
    uint8_t fNumSubControllers;   
    plControllerCacheInfo **fSubControllers;

    uint32_t fKeyIndex;
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

    virtual void Interp(float time, float* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsScalarTriple* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsScaleValue* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsQuat* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsMatrix33* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsMatrix44* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsColorRGBA* result, plControllerCacheInfo *cache = nil) const {}
    virtual void Interp(float time, hsAffineParts* parts, plControllerCacheInfo *cache = nil) const {}

    virtual plControllerCacheInfo* CreateCache() const { return nil; } // Caller must handle deleting the pointer.
    virtual float GetLength() const = 0;
    virtual void GetKeyTimes(hsTArray<float> &keyTimes) const = 0;
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
    uint8_t fType;
    void *fKeys; // Need to pay attend to fType to determine what these actually are
    uint32_t fNumKeys;
    mutable uint32_t fLastKeyIdx;

public:
    plLeafController() : fType(hsKeyFrame::kUnknownKeyFrame), fKeys(nil), fNumKeys(0), fLastKeyIdx(0) {}
    virtual ~plLeafController();

    CLASSNAME_REGISTER( plLeafController );
    GETINTERFACE_ANY( plLeafController, plController );

    void Interp(float time, float* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsScalarTriple* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsScaleValue* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsQuat* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsMatrix33* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsMatrix44* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsColorRGBA* result, plControllerCacheInfo *cache = nil) const;

    virtual plControllerCacheInfo* CreateCache() const;
    float GetLength() const;
    uint32_t GetStride() const;

    hsPoint3Key *GetPoint3Key(uint32_t i) const;
    hsBezPoint3Key *GetBezPoint3Key(uint32_t i) const;
    hsScalarKey *GetScalarKey(uint32_t i) const;
    hsBezScalarKey *GetBezScalarKey(uint32_t i) const;
    hsScaleKey *GetScaleKey(uint32_t i) const;
    hsBezScaleKey *GetBezScaleKey(uint32_t i) const;
    hsQuatKey *GetQuatKey(uint32_t i) const;
    hsCompressedQuatKey32 *GetCompressedQuatKey32(uint32_t i) const;
    hsCompressedQuatKey64 *GetCompressedQuatKey64(uint32_t i) const;
    hsG3DSMaxKeyFrame *Get3DSMaxKey(uint32_t i) const;
    hsMatrix33Key *GetMatrix33Key(uint32_t i) const;
    hsMatrix44Key *GetMatrix44Key(uint32_t i) const;

    uint8_t GetType() const { return fType; }
    uint32_t GetNumKeys() const { return fNumKeys; }
    void *GetKeyBuffer() const { return fKeys; }
    void GetKeyTimes(hsTArray<float> &keyTimes) const;
    void AllocKeys(uint32_t n, uint8_t type);
    void QuickScalarController(int numKeys, float* times, float* values, uint32_t valueStrides);
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
    plCompoundController(); // allocs leaf controllers
    ~plCompoundController();

    CLASSNAME_REGISTER( plCompoundController );
    GETINTERFACE_ANY( plCompoundController, plController );

    void Interp(float time, hsQuat* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsScalarTriple* result, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsAffineParts* parts, plControllerCacheInfo *cache = nil) const;
    void Interp(float time, hsColorRGBA* result, plControllerCacheInfo *cache = nil) const;

    plControllerCacheInfo* CreateCache() const;
    plController *GetXController() const { return fXController; }
    plController *GetYController() const { return fYController; }
    plController *GetZController() const { return fZController; }
    plController *GetPosController() const { return fXController; }
    plController *GetRotController() const { return fYController; }
    plController *GetScaleController() const { return fZController; }
    plController *GetController(int32_t i) const;
    float GetLength() const;
    void GetKeyTimes(hsTArray<float> &keyTimes) const;
    hsBool AllKeysMatch() const;
    hsBool PurgeRedundantSubcontrollers();

    void SetXController(plController *c) { delete fXController; fXController = c; }
    void SetYController(plController *c) { delete fYController; fYController = c; }
    void SetZController(plController *c) { delete fZController; fZController = c; }
    void SetPosController(plController *c) { delete fXController; fXController = c; }
    void SetRotController(plController *c) { delete fYController; fYController = c; }
    void SetScaleController(plController *c) { delete fZController; fZController = c; }
    void SetController(int32_t i, plController* c);

    void Read(hsStream* s, hsResMgr* mgr);
    void Write(hsStream* s, hsResMgr* mgr);
};

#endif

