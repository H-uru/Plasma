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
#ifndef plSimulationInterface_inc
#define plSimulationInterface_inc

#include "plObjInterface.h"

class plPhysical;
struct hsMatrix44;
class hsBounds3Ext;

class plSimulationInterface : public plObjInterface
{
public:
    // Props inc by 1 (bit shift in bitvector).
    enum plSimulationProperties {
        // prop 0 is always disable, , declared in plObjInterface
        kDisable                = 0,    // no more interaction. no interference detection
        kWeightless_DEAD,               // unaffected by gravity, but not massless
        kPinned,                        // stop moving. keep colliding.
        kWarp_DEAD,                     // keep moving, no colliding (a pattern is emerging here...)
        kUpright_DEAD,                  // stand upright (mainly for the player)
        kPassive,                       // don't push new positions to sceneobject
        kRotationForces_DEAD,           // rotate using forces 
        kCameraAvoidObject_DEAD,        // camera will try and fly around this obsticle
        kPhysAnim,                      // this object is animated, and the animation can apply force
        kStartInactive,                 // deactive this object at start time. will reactivate when hit
        kNoSynchronize,                 // don't synchronize this physical
        kSuppressed_DEAD,               // physical still exists but is not in simulation: no forces, contact, or reports
        kNoOwnershipChange,             // Don't automatically change net ownership on this physical when it is collided with
        kAvAnimPushable,                // Something the avatar should react to and push against
        kNumProps
    };

protected:
    plPhysical* fPhysical;

    friend class plSceneObject;

    void ISetSceneNode(const plKey& newNode) override;

public:
    plSimulationInterface();
    ~plSimulationInterface();

    CLASSNAME_REGISTER( plSimulationInterface );
    GETINTERFACE_ANY( plSimulationInterface, plObjInterface );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
    
    void    SetProperty(int prop, bool on) override;
    int32_t   GetNumProperties() const override { return kNumProps; }

    // Transform settable only, if you want it get it from the coordinate interface.
    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) override;

    // Bounds are gettable only, they are computed on the physical.
    const hsBounds3Ext GetLocalBounds();
    const hsBounds3Ext GetWorldBounds() const;
    const hsBounds3Ext GetMaxWorldBounds();
    void ClearLinearVelocity();

    bool MsgReceive(plMessage* msg) override;

    // Export only.
    void SetPhysical(plPhysical* phys);
    void ReleaseData() override;

    plPhysical* GetPhysical() const;
};


#endif // plSimulationInterface_inc
