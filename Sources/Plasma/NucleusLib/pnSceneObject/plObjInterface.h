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

#ifndef plInterface_inc
#define plInterface_inc

#include "pnKeyedObject/hsKeyedObject.h"
#include "plSceneObject.h"
#include "hsStream.h"
#include "pnNetCommon/plSynchedObject.h"
#include "hsBitVector.h"

class hsResMgr;
class plDrawInterface;
class plSimulationInterface;
class plCoordinateInterface;
class plAudioInterface;

//
// Since interfaces are keyed objects with valid uoids and
// may have dynamic data (coordinate interface), they are 
// synched (saved) objects
//
class plObjInterface : public plSynchedObject
{
protected:
    enum {
        kDisable        = 0x0 // Derived interfaces duplicate this, so if you add more here, they need to know.
    };
    friend class plSynchedValueBase;
    friend class plSceneObject;
protected:
    plSceneObject*              fOwner;
    hsBitVector                 fProps;

    // SetSceneNode just called by owner. If we're an interface to external data,
    // we need to pass the change on. Otherwise, do nothing.
    virtual void    ISetSceneNode(const plKey& node) {} 
    plSceneObject*  IGetOwner() const { return fOwner; }
    virtual void    ISetOwner(plSceneObject* owner);
    void            ISetAllProperties(const hsBitVector& b);    

    plDrawInterface* IGetOwnerDrawInterface() { return fOwner ? fOwner->GetVolatileDrawInterface() : nullptr; }
    plSimulationInterface* IGetOwnerSimulationInterface() { return fOwner ? fOwner->GetVolatileSimulationInterface() : nullptr; }
    plCoordinateInterface* IGetOwnerCoordinateInterface() { return fOwner ? fOwner->GetVolatileCoordinateInterface() : nullptr; }
    plAudioInterface* IGetOwnerAudioInterface() { return fOwner ? fOwner->GetVolatileAudioInterface() : nullptr; }
public:

    plObjInterface();
    ~plObjInterface();

    CLASSNAME_REGISTER( plObjInterface );
    GETINTERFACE_ANY( plObjInterface, plSynchedObject );

    bool MsgReceive(plMessage* msg) override;

    const plSceneObject* GetOwner() const { return IGetOwner(); }
    plKey GetOwnerKey() const { return IGetOwner() ? IGetOwner()->GetKey() : nullptr; }
    
    virtual plKey GetSceneNode() const { return IGetOwner() ? IGetOwner()->GetSceneNode() : nullptr; }

    // override SetProperty to pass the prop down to the pool objects 
    virtual void    SetProperty(int prop, bool on) { fProps.SetBit(prop, on); }
    
    // shouldn't need to override GetProperty()
    bool  GetProperty(int prop) const { return fProps.IsBitSet(prop); }
    virtual int32_t   GetNumProperties() const = 0;

    virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l) = 0;

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    virtual void    ReleaseData() { }
};


#endif // plInterface_inc
