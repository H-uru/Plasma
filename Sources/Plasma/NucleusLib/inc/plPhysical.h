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
#ifndef plPhysical_inc
#define plPhysical_inc

#include "pnNetCommon/plSynchedObject.h"

struct hsMatrix44;
struct hsPoint3;
struct hsVector3;
class hsQuat;
class plPhysicalSndGroup;
class plDrawableSpans;
class hsGMaterial;

// Primary interface object for physics functionality. A physical corresponds to
// a single rigid body in a simulation. (Note that there can be multiple
// simulations.) The plPhysical is reached through the simulation interface on a
// plSceneObject
//
// Any function that ends with 'Sim' gets or sets a simulation space value.  If
// the physical is in the main world, this will be the same as a global value,
// but if it's in a subworld, it will be relative to that.
class plPhysical : public plSynchedObject
{
public:
    CLASSNAME_REGISTER(plPhysical);
    GETINTERFACE_ANY(plPhysical, plSynchedObject);

    virtual plPhysical& SetProperty(int prop, bool b) = 0;
    virtual bool        GetProperty(int prop) const = 0;

    virtual void SetObjectKey(plKey oKey) = 0;
    virtual plKey GetObjectKey() const = 0;

    // These two should only be called by the SceneNode
    virtual void SetSceneNode(plKey node) = 0;
    virtual plKey GetSceneNode() const = 0;

    virtual bool GetLinearVelocitySim(hsVector3& vel) const = 0;
    virtual void SetLinearVelocitySim(const hsVector3& vel, bool wakeup=true) = 0;
    virtual void ClearLinearVelocity() = 0;

    virtual bool GetAngularVelocitySim(hsVector3& vel) const = 0;
    virtual void SetAngularVelocitySim(const hsVector3& vel, bool wakeup=true) = 0;

    /** Standard plasma transform interface, in global coordinates by convention.
    If you send in the same matrix that the physical last sent out in its correction message,
    it will be ignored as an "echo" -- UNLESS you set force to true, in which case the transform
    will be applied regardless.
    Set force to true if you don't want the transform to be ignored for any reason. Without it,
    this will ignore the incoming transform if it's the same one it sent out last time. */
    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, bool force=false) = 0;
    virtual void GetTransform(hsMatrix44& l2w, hsMatrix44& w2l) = 0;

    // From plSimDefs::Group
    virtual int GetGroup() const = 0;
    virtual void SetGroup(int group) = 0;

    // Flags in plSimDefs::plLOSDB
    virtual void      AddLOSDB(uint16_t flag) = 0;
    virtual void      RemoveLOSDB(uint16_t flag) = 0;
    virtual uint16_t  GetAllLOSDBs() = 0;
    virtual bool      IsInLOSDB(uint16_t flag) = 0;

    // Return the key of our subworld. May be a nil key.
    virtual plKey GetWorldKey() const = 0;

    virtual plPhysicalSndGroup* GetSoundGroup() const = 0;

    virtual void GetPositionSim(hsPoint3& pos) const = 0;

    /** Send the current simulation location to the coordinate interface.
    If the simulation is a subworld, this will also factor in the subworld's
    transform. The sent transform is global. "isSynchUpdate" tells us if this
    is an update due to receiving remote state.*/
    virtual void SendNewLocation(bool synchTransform = false, bool isSynchUpdate = false) = 0;

    // For the physics SDL only.  For Set, any of the values may be set to nil, if
    // they're not used.
    virtual void GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV) = 0;
    virtual void SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV) = 0;

    /** Resets the physical to its default state. */
    virtual void ResetSyncState() = 0;

    virtual float GetMass() = 0;

    virtual plDrawableSpans* CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo) = 0;
};

#endif // plPhysical_inc
