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

#ifndef plOccluder_inc
#define plOccluder_inc

#include <vector>

#include "hsBounds.h"
#include "hsBitVector.h"
#include "hsMatrix44.h"

#include "pnSceneObject/plObjInterface.h"

class plCullPoly;
class plDrawableSpans;
class hsGMaterial;
class plOccluderProxy;
class plVisRegion;

class plOccluder : public plObjInterface
{
public:
    enum {
        kDisable        = 0x0,

        kNumProps
    };
    enum {
        kRefVisRegion
    };
protected:
    std::vector<plCullPoly>     fPolys;

    plOccluderProxy*            fProxyGen;

    hsBitVector                 fVisSet;
    std::vector<plVisRegion*>   fVisRegions;
    hsBitVector                 fVisNot;

    float                    fPriority;
    hsBounds3Ext                fWorldBounds;

    plKey                       fSceneNode;

    virtual float            IComputeSurfaceArea();
    virtual void                IComputeBounds();

    virtual std::vector<plCullPoly>& IGetLocalPolyList() { return fPolys; }

    void    ISetSceneNode(const plKey& node) override;

    void            IAddVisRegion(plVisRegion* reg);
    void            IRemoveVisRegion(plVisRegion* reg);

public:
    plOccluder();
    virtual ~plOccluder();

    CLASSNAME_REGISTER( plOccluder );
    GETINTERFACE_ANY( plOccluder, plObjInterface);

    bool        MsgReceive(plMessage* msg) override;

    virtual float GetPriority() const { return fPriority; }

    bool InVisSet(const hsBitVector& visSet) const { return fVisSet.Overlap(visSet); }
    bool InVisNot(const hsBitVector& visNot) const { return fVisNot.Overlap(visNot); }

    virtual const hsBounds3Ext& GetWorldBounds() const { return fWorldBounds; }

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;
    virtual const hsMatrix44& GetLocalToWorld() const;
    virtual const hsMatrix44& GetWorldToLocal() const;

    virtual void SetPolyList(const std::vector<plCullPoly>& list);
    virtual const std::vector<plCullPoly>& GetWorldPolyList() const { return fPolys; }
    virtual const std::vector<plCullPoly>& GetLocalPolyList() const { return fPolys; }

    int32_t   GetNumProperties() const override { return kNumProps; }

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    // Visualization
    virtual plDrawableSpans*    CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo);

    // Export only function to initialize.
    virtual void ComputeFromPolys();

    // These two should only be called internally and on export/convert
    plKey GetSceneNode() const override { return fSceneNode; }
};

class plMobileOccluder : public plOccluder
{
protected:
    hsMatrix44              fLocalToWorld;
    hsMatrix44              fWorldToLocal;

    hsBounds3Ext            fLocalBounds;

    std::vector<plCullPoly> fOrigPolys;

    void            IComputeBounds() override;

    std::vector<plCullPoly>& IGetLocalPolyList() override { return fOrigPolys; }

public:

    plMobileOccluder();
    virtual ~plMobileOccluder();

    CLASSNAME_REGISTER( plMobileOccluder );
    GETINTERFACE_ANY( plMobileOccluder, plOccluder );

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;
    const hsMatrix44& GetLocalToWorld() const override { return fLocalToWorld; }
    const hsMatrix44& GetWorldToLocal() const override { return fWorldToLocal; }

    void SetPolyList(const std::vector<plCullPoly>& list) override;

    const std::vector<plCullPoly>& GetLocalPolyList() const override { return fOrigPolys; }

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    // Export only function to initialize.
    void ComputeFromPolys() override;
};

#endif // plOccluder_inc
