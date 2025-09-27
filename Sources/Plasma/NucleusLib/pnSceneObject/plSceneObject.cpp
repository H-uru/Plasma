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

#include "HeadSpin.h"
#include "plSceneObject.h"
#include "plDrawInterface.h"
#include "plSimulationInterface.h"
#include "plCoordinateInterface.h"
#include "plAudioInterface.h"
#include "pnDispatch/plDispatch.h"
#include "pnFactory/plFactory.h"
#include "pnModifier/plModifier.h"
#include "pnMessage/plMessage.h"
#include "pnMessage/plRefMsg.h"
#include "plDrawable.h"
#include "plPhysical.h"
#include "plAudible.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plCorrectionMsg.h"
#include "pnMessage/plWarpMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plAttachMsg.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plSimulationSynchMsg.h"
#include "pnMessage/plSimulationMsg.h"
#include "pnMessage/plNodeChangeMsg.h"
#include "pnMessage/plSelfDestructMsg.h"
#include "pnKeyedObject/plKey.h"

#include "hsStream.h"
#include "hsResMgr.h"
#include "plCreatableIndex.h" // For plLightInfo::Index(), so we don't have to include plLightInfo.h



int dbgCurrentTest = 0;

plSceneObject::plSceneObject()
:   fDrawInterface(),
    fSimulationInterface(),
    fCoordinateInterface(),
    fAudioInterface()
{
}

plSceneObject::~plSceneObject()
{
    SetDrawInterface(nullptr);
    SetSimulationInterface(nullptr);
    SetCoordinateInterface(nullptr);
    SetAudioInterface(nullptr);

    IRemoveAllGenerics();

    size_t knt = fModifiers.size();
    for (size_t i = 0; i < knt; i++)
    {
        if( fModifiers[i] )
            fModifiers[i]->RemoveTarget(this);
    }
}

void plSceneObject::Read(hsStream* stream, hsResMgr* mgr)
{
    plSynchedObject::Read(stream, mgr);

    // Interfaces will attach themselves to us on read.
    // DI
    mgr->ReadKeyNotifyMe(stream, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    // SI
    mgr->ReadKeyNotifyMe(stream, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    // CI
    mgr->ReadKeyNotifyMe(stream, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    // AI
    mgr->ReadKeyNotifyMe(stream, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);

    uint32_t nGen = stream->ReadLE32();
    fGenerics.clear();
    for (uint32_t i = 0; i < nGen; i++)
    {
        mgr->ReadKeyNotifyMe(stream, new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface), plRefFlags::kActiveRef);
    }

    plObjRefMsg* refMsg;

    size_t nOldMods = fModifiers.size();     // existng modifiers created during interface loading
    uint32_t nNewMods = stream->ReadLE32();
    fModifiers.resize(nOldMods + nNewMods);    // reserve space for new modifiers+existing modifiers
    for (size_t i = nOldMods; i < nOldMods + nNewMods; i++)
    {
        refMsg = new plObjRefMsg(GetKey(), plRefMsg::kOnCreate, static_cast<int8_t>(i), plObjRefMsg::kModifier);
        mgr->ReadKeyNotifyMe(stream,refMsg, plRefFlags::kActiveRef);
    }

    plKey nodeKey = mgr->ReadKey(stream);
//  SetSceneNode(nodeKey);
    fSceneNode = nodeKey;

}

void plSceneObject::Write(hsStream* stream, hsResMgr* mgr)
{
    plSynchedObject::Write(stream, mgr);

    mgr->WriteKey(stream, fDrawInterface);
    mgr->WriteKey(stream, fSimulationInterface);
    mgr->WriteKey(stream, fCoordinateInterface);
    mgr->WriteKey(stream, fAudioInterface);

    stream->WriteLE32((uint32_t)fGenerics.size());
    for (plObjInterface* generic : fGenerics)
        mgr->WriteKey(stream, generic);

    for (hsSsize_t i = fModifiers.size() - 1; i >= 0; --i) {
        if (fModifiers[i]->GetKey() == nullptr) {
            fModifiers[i]->RemoveTarget(this);
            fModifiers.erase(fModifiers.begin() + i);
        }
    }

    stream->WriteLE32((uint32_t)fModifiers.size());
    for (plModifier* modifier : fModifiers)
        mgr->WriteKey(stream, modifier);

    mgr->WriteKey(stream, fSceneNode);
}

//// ReleaseData //////////////////////////////////////////////////////////////
//  Called by SceneViewer to release the data for this sceneObject (really
//  just a switchboard).

void    plSceneObject::ReleaseData()
{
    if( fDrawInterface )
        fDrawInterface->ReleaseData();
    if( fSimulationInterface )
        fSimulationInterface->ReleaseData();
    if( fCoordinateInterface )
        fCoordinateInterface->ReleaseData();
    if( fAudioInterface )
        fAudioInterface->ReleaseData();

    for (plObjInterface* generic : fGenerics)
    {
        if (generic)
            generic->ReleaseData();
    }
}

void plSceneObject::FlushTransform()
{
    if( fCoordinateInterface )
        fCoordinateInterface->FlushTransform();
}

#include "plProfile.h"

plProfile_CreateTimer("SOTrans", "Object", SOTrans);
plProfile_CreateTimer("   SODITrans", "Object", SODITrans);
plProfile_CreateTimer("   SOSITrans", "Object", SOSITrans);
plProfile_CreateTimer("   SOAITrans", "Object", SOAITrans);
plProfile_CreateTimer("   SOGITrans", "Object", SOGITrans);
plProfile_CreateTimer("   SOMOTrans", "Object", SOMOTrans);


void plSceneObject::ISetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    plProfile_BeginTiming(SOTrans);
    plProfile_BeginTiming(SODITrans);
    if( fDrawInterface )
        fDrawInterface->SetTransform(l2w, w2l);
    plProfile_EndTiming(SODITrans);

    plProfile_BeginTiming(SOSITrans);
    if( fSimulationInterface )
    {
        if(fCoordinateInterface)
        {
            uint16_t whyTransformed = fCoordinateInterface->GetReasons();
            if(whyTransformed != plCoordinateInterface::kReasonPhysics)
            {
                // if we were transformed by anything but physics, let physics know
                // otherwise we're not even going to tell physics
                fSimulationInterface->SetTransform(l2w, w2l);
            }
            fCoordinateInterface->ClearReasons();
        } else {
            // if there's no coordinate interface, there's no reason to move the simulation interface
        }
    }
    plProfile_EndTiming(SOSITrans);

    plProfile_BeginTiming(SOAITrans);
    if( fAudioInterface )
        fAudioInterface->SetTransform(l2w, w2l);
    plProfile_EndTiming(SOAITrans);

    plProfile_BeginTiming(SOGITrans);
    for (plObjInterface* generic : fGenerics)
    {
        if (generic)
            generic->SetTransform(l2w, w2l);
    }
    plProfile_EndTiming(SOGITrans);


    plProfile_BeginTiming(SOMOTrans);
    for (plModifier* modifier : fModifiers)
    {
        if (modifier)
            modifier->SetTransform(l2w, w2l);
    }
    plProfile_EndTiming(SOMOTrans);
    plProfile_EndTiming(SOTrans);
}

void plSceneObject::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    if( fCoordinateInterface )
        fCoordinateInterface->SetTransform(l2w, w2l);
}

hsMatrix44 plSceneObject::GetLocalToWorld() const
{
    hsMatrix44 l2w;
    if( fCoordinateInterface )
        l2w = fCoordinateInterface->GetLocalToWorld();
    else
        l2w.Reset();
    return l2w;
}

hsMatrix44 plSceneObject::GetWorldToLocal() const
{
    hsMatrix44 w2l;
    if( fCoordinateInterface )
        w2l = fCoordinateInterface->GetWorldToLocal();
    else
        w2l.Reset();
    return w2l;
}

hsMatrix44 plSceneObject::GetLocalToParent() const
{
    hsMatrix44 l2p;
    if( fCoordinateInterface )
        l2p = fCoordinateInterface->GetLocalToParent();
    else
        l2p.Reset();
    return l2p;
}

hsMatrix44 plSceneObject::GetParentToLocal() const
{
    hsMatrix44 p2l;
    if( fCoordinateInterface )
        p2l = fCoordinateInterface->GetParentToLocal();
    else
        p2l.Reset();
    return p2l;
}

void plSceneObject::IAddModifier(plModifier* mo, hsSsize_t idx)
{
    if( !mo )
        return;

    if (idx < 0)
        idx = (hsSsize_t)fModifiers.size();
    if (size_t(idx + 1) > fModifiers.size())
        fModifiers.resize(idx + 1);
    fModifiers[idx] = mo;
    mo->AddTarget(this);
}

void plSceneObject::IRemoveModifier(plModifier* mo)
{
    if( !mo )
        return;

    const auto idx = std::find(fModifiers.cbegin(), fModifiers.cend(), mo);
    if (idx != fModifiers.cend())
    {
        mo->RemoveTarget(this);
        fModifiers.erase(idx);
    }
}

void plSceneObject::ISetInterface(plObjInterface* iface)
{
    hsAssert(iface, "Setting nil interface");
    if( plDrawInterface::ConvertNoRef(iface) )
        ISetDrawInterface(plDrawInterface::ConvertNoRef(iface));
    else if( plSimulationInterface::ConvertNoRef(iface) )
        ISetSimulationInterface(plSimulationInterface::ConvertNoRef(iface));
    else if( plCoordinateInterface::ConvertNoRef(iface) )
        ISetCoordinateInterface(plCoordinateInterface::ConvertNoRef(iface));
    else if( plAudioInterface::ConvertNoRef(iface) )
        ISetAudioInterface(plAudioInterface::ConvertNoRef(iface));
    else
        IAddGeneric(iface);

    if( iface )
        iface->ISetSceneNode(GetSceneNode());
}

void plSceneObject::IRemoveInterface(int16_t idx, plObjInterface* who)
{
    if( plFactory::DerivesFrom(plDrawInterface::Index(), idx) )
        ISetDrawInterface(nullptr);
    else if( plFactory::DerivesFrom(plSimulationInterface::Index(), idx) )
        ISetSimulationInterface(nullptr);
    else if( plFactory::DerivesFrom(plCoordinateInterface::Index(), idx) )
        ISetCoordinateInterface(nullptr);
    else if( plFactory::DerivesFrom(plAudioInterface::Index(), idx) )
        ISetAudioInterface(nullptr);
    else
        IRemoveGeneric(who);
}

bool plSceneObject::IPropagateToModifiers(plMessage* msg)
{
    bool retVal = false;

    for (plModifier* modifier : fModifiers)
    {
        if (modifier)
            retVal |= modifier->MsgReceive(msg);
    }
    return retVal;
}

bool plSceneObject::Eval(double secs, float delSecs)
{
    uint32_t dirty = ~0L;
    bool retVal = false;
    for (plModifier* modifier : fModifiers)
    {
        if (modifier)
            retVal |= modifier->IEval(secs, delSecs, dirty);
    }
    return retVal;
}

void plSceneObject::SetSceneNode(const plKey& newNode)
{
    plKey curNode=GetSceneNode();
    if( curNode == newNode )
        return;
    if( fDrawInterface )
        fDrawInterface->ISetSceneNode(newNode);
    if( fSimulationInterface )
        fSimulationInterface->ISetSceneNode(newNode);
    if( fAudioInterface )
        fAudioInterface->ISetSceneNode(newNode);
    if( fCoordinateInterface )
        fCoordinateInterface->ISetSceneNode(newNode);

    for (plObjInterface* generic : fGenerics)
    {
        if (generic)
            generic->ISetSceneNode(newNode);
    }

    if( newNode )
    {
        plNodeRefMsg* refMsg = new plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kObject);
        plKey key = GetKey();   // for linux build
        hsgResMgr::ResMgr()->AddViaNotify(key, refMsg, plRefFlags::kActiveRef);
    }
    if( curNode)
    {
        curNode->Release(GetKey());
    }
    fSceneNode = newNode;
}

plKey plSceneObject::GetSceneNode() const
{
    return fSceneNode;
}

void plSceneObject::SetNetGroup(plNetGroupId netGroup)
{
    if( !fCoordinateInterface )
        plSynchedObject::SetNetGroup(netGroup);
    else
        fCoordinateInterface->ISetNetGroupRecur(netGroup);
}

const plModifier* plSceneObject::GetModifierByType(uint16_t classIdx) const
{
    for (plModifier* modifier : fModifiers)
    {
        if (modifier && plFactory::DerivesFrom(classIdx, modifier->ClassIndex()))
            return modifier;
    }

    return nullptr;
}

bool plSceneObject::MsgReceive(plMessage* msg)
{

#if 0   // objects are only in the nullptr room when they are being paged out
    // TEMP - until we have another way to neutralize objects
    // for an object in the 'nullptr' room, ignore most msgs
    if (GetSceneNode() == nullptr && !plNodeChangeMsg::ConvertNoRef(msg) &&
        !plRefMsg::ConvertNoRef(msg)&&
        !plSelfDestructMsg::ConvertNoRef(msg))
        return false;
#endif

    bool retVal = false;
    // If it's a bcast, let our own dispatcher find who's interested.
    plTransformMsg* trans;
    plEvalMsg* eval = plEvalMsg::ConvertNoRef(msg);
    plAttachMsg* att = nullptr;
    if( eval )
    {
        // Switched things over so that modifiers register for the eval message themselves,
        // and can be smart about not evaluating if they know it's unneccessary.

        //Eval(eval->DSeconds(), eval->DelSeconds());
        return true;
    }
    else
    if((trans = plTransformMsg::ConvertNoRef(msg))) // also catches the derived plDelayedTransformMsg
    {
        if( fCoordinateInterface )
        {
            // flush any dirty transforms
            fCoordinateInterface->ITransformChanged(false, 0, trans->ClassIndex() == plTransformMsg::Index());
        }
        return true;
    }
    else
    if((att = plAttachMsg::ConvertNoRef(msg)))
    {
        if( fCoordinateInterface )
        {
            plSceneObject *child = plSceneObject::ConvertNoRef(att->GetRef());
            
            if( child )
            {
                if( !fCoordinateInterface )
                {
                    // If we have no coordinate interface, we could make ourselves one here,
                    // but for now it's an error.
                    hsAssert(false, "Trying to attach a child when we have no coordinateInterface");
                    return true;
                }
                if( !child->GetVolatileCoordinateInterface() )
                {
                    // If the child has no coordinate interface, we could add one to it here,
                    // but for now it's an error.
                    hsAssert(false, "Trying to attach a child who has no coordinateInterface");
                    return true;
                }
                plIntRefMsg* intRefMsg = new plIntRefMsg(fCoordinateInterface->GetKey(), att->GetContext(), -1, plIntRefMsg::kChildObject);
                intRefMsg->SetRef(child);
                hsgResMgr::ResMgr()->AddViaNotify(intRefMsg, plRefFlags::kPassiveRef);
            }
        }
    }
    else // Am I the final destination?
    {
        retVal = IMsgHandle(msg);
    }

    if( msg->HasBCastFlag(plMessage::kPropagateToModifiers) )
    {
        retVal |= IPropagateToModifiers(msg);
    }
        
    if (msg->HasBCastFlag(plMessage::kPropagateToChildren))
    {
        const plCoordinateInterface* ci = GetCoordinateInterface();
        for (size_t i = 0; i < ci->GetNumChildren(); i++)
        {
            plSceneObject* child = (plSceneObject*)ci->GetChild(i)->GetOwner();
            if (child)
                retVal |= child->MsgReceive(msg);
        }
    }
    
    // Might want an option to propagate messages to children here.

    return plSynchedObject::MsgReceive(msg);
}

bool plSceneObject::IMsgHandle(plMessage* msg)
{
    // To start with, plSceneObject only handles messages to add or remove
    // references. Current references are other plSceneObjects and plModifiers
    plObjRefMsg* refMsg = plObjRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        switch( refMsg->fType )
        {
        case plObjRefMsg::kModifier:
            {
                if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                {
                    plModifier* mod = plModifier::ConvertNoRef(refMsg->GetRef());
                    IAddModifier(mod, refMsg->fWhich);
                }
                else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                {
                    plModifier* mod = (plModifier*)refMsg->GetRef();
                    IRemoveModifier(mod);
                }
            }
            return true;
        case plObjRefMsg::kInterface:
            {
                if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                {
                    plObjInterface* oi = plObjInterface::ConvertNoRef(refMsg->GetRef());
                    ISetInterface(oi);
                }
                else
                if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
                {
                    plObjInterface* oi = (plObjInterface*)refMsg->GetRef();
                    // TODO - This will crash if oi's already been deleted
                    IRemoveInterface(oi->ClassIndex(), oi);
                }
            }
            return true;
        }


        return false;
    }
    plNodeChangeMsg* nodeChange = plNodeChangeMsg::ConvertNoRef(msg);
    if( nodeChange )
    {
        SetSceneNode(nodeChange->GetNodeKey());
        return true;
    }
    
    if( plIntRefMsg::ConvertNoRef(msg) )
    {
        if (fCoordinateInterface)
            fCoordinateInterface->MsgReceive(msg);
        if (fAudioInterface)
            fAudioInterface->MsgReceive(msg);
        return true;
    }

    if (plCorrectionMsg::ConvertNoRef(msg))
    {
        hsAssert(fCoordinateInterface, "Unimplemented, also need to register this one we just made with the resource manager");
        
        if (fCoordinateInterface)
            fCoordinateInterface->MsgReceive(msg);

        return true;
    }
    
    // check for generic enable/disable message (passed to interfaces)

    plEnableMsg* pEnableMsg = plEnableMsg::ConvertNoRef( msg );
    if (pEnableMsg)
    {
        if( pEnableMsg->Cmd(plEnableMsg::kDrawable) )
        {
            plDrawInterface* di = GetVolatileDrawInterface();
            if( di )
                di->MsgReceive(msg);
            plObjInterface* li = GetVolatileGenericInterface(CLASS_INDEX_SCOPED(plLightInfo));
            if( li )
                li->MsgReceive(msg);
        }
        if ( pEnableMsg->Cmd(plEnableMsg::kAll) && GetDrawInterface() )
            GetVolatileDrawInterface()->MsgReceive(msg);
        
        if ( pEnableMsg->Cmd( plEnableMsg::kPhysical ) || pEnableMsg->Cmd( plEnableMsg::kAll) )
        {
            if ( GetSimulationInterface() )
            {
                GetVolatileSimulationInterface()->MsgReceive(msg);
            }
            else
            {
                // if someone is trying to disable the physics on a sceneobject that doesn't have a physics interface...
                // they might be trying to disable the avatar when the PhysX controller is being used
                // ...so, look to see if this is an avatar object, and tell the avatar to disable the physics
                IPropagateToModifiers(msg);
            }
        }
        
        if ( (pEnableMsg->Cmd( plEnableMsg::kAudible ) || pEnableMsg->Cmd( plEnableMsg::kAll )) && GetAudioInterface() )
            GetVolatileAudioInterface()->MsgReceive(msg);
        
        if( pEnableMsg->Cmd( plEnableMsg::kAll ) )
        {   
            IPropagateToGenerics(pEnableMsg);
        }
        else if( pEnableMsg->Cmd( plEnableMsg::kByType ) )
        {   
            IPropagateToGenerics(pEnableMsg->Types(), pEnableMsg);
        }
        return true;
    }
    
    // warp message
    if( plWarpMsg::ConvertNoRef(msg) )
    {
        // if there's a simulation interface, it needs to know about the warp
        // *** it would probably be better if it got this from the coordinate interface, as
        // *** only the coordinate interface knows to propagate it to children.
    //  if(fSimulationInterface)
    //  {
    //      fSimulationInterface->MsgReceive(msg);
    //  } 

        // the coordinate interface always gets the warp 
        if (fCoordinateInterface) 
        {
            fCoordinateInterface->MsgReceive(msg);
        }
        return true;
    }

    if ( plSimulationMsg::ConvertNoRef(msg) )
    {
        if(fSimulationInterface)
        {
            fSimulationInterface->MsgReceive(msg);
        }
        return true;
    }

    // audio message
    if (plSoundMsg::ConvertNoRef(msg) )
    {
        if( fAudioInterface )
            return(GetVolatileAudioInterface()->MsgReceive(msg));
        return true;
    }

    return false;
}

void plSceneObject::IPropagateToGenerics(const hsBitVector& types, plMessage* msg)
{
    hsBitIterator iter(types);
    for (plObjInterface* generic : fGenerics)
    {
        if (generic)
        {
            for( iter.Begin(); !iter.End(); iter.Advance() )
            {
                if (plFactory::DerivesFrom(iter.Current(), generic->ClassIndex()))
                {
                    generic->MsgReceive(msg);
                    break;
                }
            }
        }
    }
}

void plSceneObject::IPropagateToGenerics(plMessage* msg)
{
    for (plObjInterface* generic : fGenerics)
    {
        if (generic)
            generic->MsgReceive(msg);
    }
}

plObjInterface* plSceneObject::GetVolatileGenericInterface(uint16_t classIdx) const
{
    for (plObjInterface* generic : fGenerics)
    {
        if (generic && plFactory::DerivesFrom(classIdx, generic->ClassIndex()))
            return generic;
    }
    return nullptr;
}

void plSceneObject::IAddGeneric(plObjInterface* gen)
{
    if( gen )
    {
        const auto idx = std::find(fGenerics.cbegin(), fGenerics.cend(), gen);
        if (idx == fGenerics.cend())
        {
            fGenerics.emplace_back(gen);
            gen->ISetOwner(this);
        }
    }
}

void plSceneObject::IRemoveGeneric(plObjInterface* gen)
{
    if( gen )
    {
        const auto idx = std::find(fGenerics.cbegin(), fGenerics.cend(), gen);
        if (idx != fGenerics.cend())
        {
            gen->ISetOwner(nullptr);
            fGenerics.erase(idx);
        }
    }
}

void plSceneObject::IRemoveAllGenerics()
{
    for (plObjInterface* generic : fGenerics)
    {
        if (generic)
            generic->ISetOwner(nullptr);
    }
    fGenerics.clear();
}

void plSceneObject::ISetDrawInterface(plDrawInterface* di) 
{ 
    if( fDrawInterface != di )
    {
        if( fDrawInterface )
            fDrawInterface->ISetOwner(nullptr);

        fDrawInterface = di;
        if( di )
            di->ISetOwner(this);
    }
}

void plSceneObject::ISetSimulationInterface(plSimulationInterface* si) 
{ 
    if( fSimulationInterface != si )
    {
        if( fSimulationInterface )
            fSimulationInterface->ISetOwner(nullptr);

        fSimulationInterface = si;
        if( si )
            si->ISetOwner(this);
    }
}

void plSceneObject::ISetAudioInterface(plAudioInterface* ai) 
{ 
    if( fAudioInterface != ai )
    {
        if( fAudioInterface )
            fAudioInterface->ISetOwner(nullptr);
        fAudioInterface = ai;
        if( ai )
            ai->ISetOwner(this);
    }
}

void plSceneObject::ISetCoordinateInterface(plCoordinateInterface* ci) 
{ 
    if( fCoordinateInterface != ci )
    {
        if( fCoordinateInterface )
            fCoordinateInterface->ISetOwner(nullptr);

        fCoordinateInterface = ci;
        if( ci )
            ci->ISetOwner(this);
    }
}

//
// "is ready to process Loads"?  Check base class and modifiers.
//
bool  plSceneObject::IsFinal()
{
    if (!plSynchedObject::IsFinal())
        return false;

    for (plModifier* modifier : fModifiers)
        if (modifier && !modifier->IsFinal())
            return false;

    return true;
}

// Export only. Interfaces perm on object.
void plSceneObject::SetDrawInterface(plDrawInterface* di) 
{ 
    ISetDrawInterface(di);
}

void plSceneObject::SetSimulationInterface(plSimulationInterface* si) 
{ 
    ISetSimulationInterface(si);
}

void plSceneObject::SetAudioInterface(plAudioInterface* ai) 
{ 
    ISetAudioInterface(ai);
}

void plSceneObject::SetCoordinateInterface(plCoordinateInterface* ci) 
{ 
    ISetCoordinateInterface(ci);
}

void plSceneObject::AddModifier(plModifier* mo)
{
    IAddModifier(mo, (hsSsize_t)fModifiers.size());
}

void plSceneObject::RemoveModifier(plModifier* mo)
{
    IRemoveModifier(mo);
}
