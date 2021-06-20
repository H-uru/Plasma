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

#ifndef plSceneObject_inc
#define plSceneObject_inc

#include "hsBitVector.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnNetCommon/plSynchedObject.h"
#include "hsStream.h"

class plObjInterface;
class plDrawInterface;
class plSimulationInterface;
class plCoordinateInterface;
class plAudioInterface;
class plMessage;
class hsStream;
class hsResMgr;
class plMessage;
class plDispatchBase;
struct hsMatrix44;
class plModifier;

// The following two aren't dragging the Conversion side into the runtime.
// They are just to let the converter do things we don't want done at runtime.
// Nice that we can make friends with these opaque classes.
class plMaxNode;
class plMaxNodeBase;

class plSceneObject : public plSynchedObject {
    plDrawInterface*            GetVolatileDrawInterface() { return fDrawInterface; }
    plSimulationInterface*      GetVolatileSimulationInterface() { return fSimulationInterface; }
    plCoordinateInterface*      GetVolatileCoordinateInterface() { return fCoordinateInterface; }
    plAudioInterface*           GetVolatileAudioInterface() { return fAudioInterface; }
    plObjInterface*             GetVolatileGenericInterface(uint16_t classIdx) const;

    plModifier*                 GetVolatileModifier(size_t i) { return fModifiers[i]; }

    plKey                       fSceneNode;

    friend class plModifier;
    friend class plCoordinateInterface;
    friend class plObjInterface;
    friend class plMaxNode;
    friend class plMaxNodeBase;

    bool IMsgHandle(plMessage* msg);
protected:

    plDrawInterface*            fDrawInterface;
    plSimulationInterface*      fSimulationInterface;
    plCoordinateInterface*      fCoordinateInterface;
    plAudioInterface*           fAudioInterface;

    std::vector<plModifier*>    fModifiers;

    std::vector<plObjInterface*> fGenerics;

    void                    ISetDrawInterface(plDrawInterface* di);
    void                    ISetSimulationInterface(plSimulationInterface* si);
    void                    ISetAudioInterface(plAudioInterface* ai);
    void                    ISetCoordinateInterface(plCoordinateInterface* ci);

    void                    IAddGeneric(plObjInterface* gen);
    void                    IRemoveGeneric(plObjInterface* gen);
    void                    IRemoveAllGenerics();
    void                    IPropagateToGenerics(plMessage* msg);
    void                    IPropagateToGenerics(const hsBitVector& types, plMessage* msg);

    void                    IAddModifier(plModifier* mo, hsSsize_t i);
    void                    IRemoveModifier(plModifier* mo);
    bool                    IPropagateToModifiers(plMessage* msg);

    void                    ISetInterface(plObjInterface* iface);
    void                    IRemoveInterface(plObjInterface* iface);
    void                    IRemoveInterface(int16_t idx, plObjInterface* iface=nullptr);

    void                    ISetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

public:

    plSceneObject();
    virtual ~plSceneObject();

    CLASSNAME_REGISTER( plSceneObject );
    GETINTERFACE_ANY( plSceneObject, plSynchedObject );

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    virtual const plDrawInterface*              GetDrawInterface() const { return fDrawInterface; }
    virtual const plSimulationInterface*        GetSimulationInterface() const { return fSimulationInterface; }
    virtual const plCoordinateInterface*        GetCoordinateInterface() const { return fCoordinateInterface; }
    virtual const plAudioInterface*             GetAudioInterface() const { return fAudioInterface; }

    size_t                  GetNumGenerics() const { return fGenerics.size(); }
    const plObjInterface*   GetGeneric(size_t i) const { return fGenerics[i]; }

    plObjInterface*         GetGenericInterface(uint16_t classIdx) const { return GetVolatileGenericInterface(classIdx); }

    size_t                  GetNumModifiers() const { return fModifiers.size(); }
    const plModifier*       GetModifier(size_t i) const { return fModifiers[i]; }
    const plModifier*       GetModifierByType(uint16_t classIdx) const;

    bool MsgReceive(plMessage* msg) override;
    virtual bool Eval(double secs, float del);

    void                    SetSceneNode(const plKey& newNode);
    plKey                   GetSceneNode() const;

    // Network only strange function. Do not emulate or generalize this functionality.
    void SetNetGroup(plNetGroupId netGroup) override;

    virtual void    ReleaseData();

    // Force an immediate re-sync of the transforms in the hierarchy this object belongs to,
    // as opposed to waiting for the plTransformMsg to resync.
    void FlushTransform(); 
    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);
    hsMatrix44 GetLocalToWorld() const;
    hsMatrix44 GetWorldToLocal() const;
    hsMatrix44 GetLocalToParent() const;
    hsMatrix44 GetParentToLocal() const;

    bool IsFinal() override;  // "is ready to process Loads"

    // Export only
    virtual void SetDrawInterface(plDrawInterface* di);
    virtual void SetSimulationInterface(plSimulationInterface* si);
    virtual void SetAudioInterface(plAudioInterface* ai);
    virtual void SetCoordinateInterface(plCoordinateInterface* ci);

    virtual void AddModifier(plModifier* mo);
    virtual void RemoveModifier(plModifier* mo);
};

#endif // plSceneObject_inc
