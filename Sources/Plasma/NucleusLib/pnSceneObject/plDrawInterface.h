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

#ifndef plDrawInterface_inc
#define plDrawInterface_inc

#include "plObjInterface.h"

class plDrawable;
class hsStream;
class hsResMgr;
struct hsMatrix44;
class hsBounds3Ext;
class hsGMaterial;
class plParticleEmitter;

class plDrawInterface : public plObjInterface
{
public:
    // Props inc by 1 (bit shift in bitvector).
    enum plDrawProperties {
        kDisable                = 0,

        kNumProps               // last in the list
    };
    enum {
        kRefVisRegion
    };

protected:
    // TODO: Maybe these should be stored as a tuple or struct...
    std::vector<plDrawable*>    fDrawables;
    std::vector<uint32_t>       fDrawableIndices;

    std::vector<hsKeyedObject*> fRegions;

    void ISetVisRegions(size_t iDraw);
    void ISetVisRegion(hsKeyedObject* ref, bool on);
    void ISetDrawable(size_t which, plDrawable* dr);
    void IRemoveDrawable(plDrawable* dr);
    void ISetSceneNode(const plKey& newNode) override;
    virtual void ICheckDrawableIndex(size_t which);

    friend class plSceneObject;

public:
    plDrawInterface();
    virtual ~plDrawInterface();

    CLASSNAME_REGISTER( plDrawInterface );
    GETINTERFACE_ANY( plDrawInterface, plObjInterface );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    void        SetProperty(int prop, bool on) override;
    int32_t       GetNumProperties() const override { return kNumProps; }

    // Transform settable only, if you want it get it from the coordinate interface.
    void        SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;

    // Bounds are gettable only, they are computed on the drawable.
    const hsBounds3Ext GetLocalBounds() const;
    const hsBounds3Ext GetWorldBounds() const;
    const hsBounds3Ext GetMaxWorldBounds() const;

    bool MsgReceive(plMessage* msg) override;

    void    ReleaseData() override;

    /// Funky particle system functions
    void    SetUpForParticleSystem(uint32_t maxNumEmitters, uint32_t maxNumParticles,
                                   hsGMaterial *material, const std::vector<plKey>& lights);
    void    ResetParticleSystem();
    void    AssignEmitterToParticleSystem( plParticleEmitter *emitter );

    /// EXPORT-ONLY
    void    SetDrawable(size_t which, plDrawable* dr);
    plDrawable* GetDrawable(size_t which) const { return which < fDrawables.size() ? fDrawables[which] : nullptr; }
    size_t  GetNumDrawables() const { return fDrawables.size(); }
    // Sets the triMesh index to be used when referring to our spans in the drawable
    void    SetDrawableMeshIndex(size_t which, size_t index);
    uint32_t  GetDrawableMeshIndex(size_t which) const { return which < fDrawableIndices.size() ? fDrawableIndices[which] : uint32_t(-1); }
};


#endif // plDrawInterface_inc
