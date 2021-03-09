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

#ifndef plShadowMaster_inc
#define plShadowMaster_inc

#include <vector>

#include "pnSceneObject/plObjInterface.h"

class plShadowCaster;
class plShadowSlave;

struct hsMatrix44;
class hsBounds3Ext;
class hsStream;
class hsResMgr;
class plMessage;
class plLightInfo;
class plShadowCastMsg;

// This helper class was borne out of the pl*ShadowMaster classes' abuse
// of the implementation details of hsTArray<T>. This pool type allows a
// user to track a subset of "in-use" items separately from the full set
// of allocated items in a collection. TODO: Eliminate the need for this
// ugly hack altogether.
template <class T>
class hsPoolVector
{
public:
    hsPoolVector() : fUsed() { }

    // Access the underlying pool
    std::vector<T>& pool() { return fPool; }
    const std::vector<T>& pool() const { return fPool; }

    size_t size() const noexcept { return fUsed; }
    bool empty() const noexcept { return fUsed == 0; }

    T& front() { return fPool.front(); }
    const T& front() const { return fPool.front(); }
    T& back() { return fPool[fUsed - 1]; }
    const T& back() const { return fPool[fUsed - 1]; }

    T& operator[](size_t pos) { return fPool[pos]; }
    const T& operator[](size_t pos) const { return fPool[pos]; }

    // Return an existing item after the last used, or add a new
    // element with the specified createItem() callback if we're
    // already at capacity.
    template <class CreateItem>
    T& next(CreateItem createItem)
    {
        if (fUsed == fPool.size()) {
            fUsed++;
            return fPool.emplace_back(createItem());
        }
        return fPool[fUsed++];
    }

    void pop_back() noexcept
    {
        fUsed--;
    }

    void clear() noexcept
    {
        fUsed = 0;
    }

private:
    std::vector<T>  fPool;
    size_t          fUsed;
};

class plShadowMaster : public plObjInterface
{
public:
    // Props inc by 1 (bit shift in bitvector).
    enum plDrawProperties {
        kDisable                = 0,
        kSelfShadow,

        kNumProps               // last in the list
    };


protected:
    // Global clamp on shadow map size and stuff
    static uint32_t                   fGlobalMaxSize;
    static float                 fGlobalMaxDist;
    static float                 fGlobalVisParm;

    // Constant parameter(s) for this master.
    float                        fAttenDist;
    float                        fMaxDist;
    float                        fMinDist;

    uint32_t                          fMaxSize;
    uint32_t                          fMinSize;

    float                        fPower;

    // Temp data used for one frame and recycled.
    hsPoolVector<plShadowSlave*>    fSlavePool;
    plLightInfo*                    fLightInfo;

    // These are specific to the projection type (perspective or orthogonal), so have to
    // be implemented by the derived class.
    virtual void IComputeWorldToLight(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
    virtual void IComputeProjections(plShadowCastMsg* castMsg, plShadowSlave* slave) const = 0;
    virtual void IComputeISect(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;
    virtual void IComputeBounds(const hsBounds3Ext& bnd, plShadowSlave* slave) const = 0;

    // Base class implementations of the rest. These might need to be overridden later, especially
    // on computing the width and height. That's really specific to the projection type, but
    // to get started I'll probably just always return 256x256.
    virtual void IComputeCasterBounds(const plShadowCaster* caster, hsBounds3Ext& casterBnd);
    virtual void IComputeWidthAndHeight(plShadowCastMsg* castMsg, plShadowSlave* slave) const;
    virtual void IComputeLUT(plShadowCastMsg* castMsg, plShadowSlave* slave) const;
    virtual float IComputePower(const plShadowCaster* caster, const hsBounds3Ext& casterBnd) const;

    virtual plShadowSlave* ILastChanceToBail(plShadowCastMsg* castMsg, plShadowSlave* slave);

    virtual plShadowSlave* ICreateShadowSlave(plShadowCastMsg* castMsg, const hsBounds3Ext& casterBnd, float power);

    virtual plShadowSlave* INewSlave(const plShadowCaster* caster) = 0;
    virtual plShadowSlave* INextSlave(const plShadowCaster* caster);

    virtual plShadowSlave* IRecycleSlave(plShadowSlave* slave);
    plLightInfo* ISetLightInfo();

    virtual void IBeginRender();
    virtual bool IOnCastMsg(plShadowCastMsg* castMsg);

public:
    plShadowMaster();
    virtual ~plShadowMaster();

    CLASSNAME_REGISTER( plShadowMaster );
    GETINTERFACE_ANY( plShadowMaster, plObjInterface );

    bool MsgReceive(plMessage* msg) override;

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override { }

    int32_t GetNumProperties() const override { return kNumProps; }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    // These are usually handled internally, activating on read and deactivating
    // on destruct. Made public in case they need to be manually handled, like
    // on dynamic construction and use.
    void Deactivate() const;
    void Activate() const;

    // These should only be useful on scene conversion.
    float GetAttenDist() const { return fAttenDist; }
    void SetAttenDist(float d) { fAttenDist = d; }

    float GetMaxDist() const { return fMaxDist; }
    float GetMinDist() const { return fMinDist; }
    void SetMaxDist(float m);

    uint32_t GetMaxSize() const { return fMaxSize; }
    uint32_t GetMinSize() const { return fMinSize; }
    void SetMaxSize(uint32_t s) { fMaxSize = s; }
    void SetMinSize(uint32_t s) { fMinSize = s; }

    float GetPower() const { return fPower; }
    void SetPower(float f) { fPower = f; }

    static void SetGlobalMaxSize(uint32_t s) ;
    static uint32_t GetGlobalMaxSize() { return fGlobalMaxSize; }

    static void SetGlobalMaxDist(float s) { fGlobalMaxDist = s; }
    static float GetGlobalMaxDist() { return fGlobalMaxDist; }

    static void SetGlobalShadowQuality(float s);
    static float GetGlobalShadowQuality() { return fGlobalVisParm; }
};



#endif // plShadowMaster_inc
