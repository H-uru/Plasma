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
#include "plDrawInterface.h"
#include "plDrawable.h"
#include "hsBounds.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "plSceneObject.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plDISpansMsg.h"

plDrawInterface::plDrawInterface()
{
}

plDrawInterface::~plDrawInterface()
{

}

void plDrawInterface::SetDrawableMeshIndex(size_t which, size_t index)
{
    ICheckDrawableIndex(which);

    fDrawableIndices[which] = index; 
}

void plDrawInterface::SetProperty(int prop, bool on)
{
    plObjInterface::SetProperty(prop, on);

    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        if( fDrawables[i] )
            fDrawables[i]->SetProperty(fDrawableIndices[i], prop, on);
    }
}

void plDrawInterface::ISetSceneNode(const plKey& newNode)
{
    for (plDrawable* draw : fDrawables)
    {
        if (draw)
            draw->SetSceneNode(newNode);
    }
}

void plDrawInterface::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    if( !GetProperty(kDisable) )
    {
        for (size_t i = 0; i < fDrawables.size(); i++)
        {
            if( fDrawables[i] )
                fDrawables[i]->SetTransform( fDrawableIndices[i], l2w, w2l );
        }
    }
}

const hsBounds3Ext plDrawInterface::GetLocalBounds() const
{
    hsBounds3Ext retVal;
    retVal.MakeEmpty();
    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        if( fDrawables[i] )
            retVal.Union(&fDrawables[i]->GetLocalBounds(fDrawableIndices[i]));
    }
    return retVal;
}

const hsBounds3Ext plDrawInterface::GetWorldBounds() const
{
    hsBounds3Ext retVal;
    retVal.MakeEmpty();
    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        if( fDrawables[i] )
            retVal.Union(&fDrawables[i]->GetWorldBounds(fDrawableIndices[i]));
    }
    return retVal;
}

const hsBounds3Ext plDrawInterface::GetMaxWorldBounds() const
{
    hsBounds3Ext retVal;
    retVal.MakeEmpty();
    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        if( fDrawables[i] )
            retVal.Union(&fDrawables[i]->GetMaxWorldBounds(fDrawableIndices[i]));
    }
    return retVal;
}

void plDrawInterface::Read(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Read(s, mgr);

    uint32_t nDrawables = s->ReadLE32();
    if (nDrawables > 0) 
        ICheckDrawableIndex(nDrawables-1);

    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        fDrawableIndices[i] = s->ReadLE32();

        plIntRefMsg* refMsg = new plIntRefMsg(GetKey(), plRefMsg::kOnCreate, i, plIntRefMsg::kDrawable);
        mgr->ReadKeyNotifyMe(s,refMsg, plRefFlags::kActiveRef);
    }

    uint32_t nReg = s->ReadLE32();
    fRegions.resize(nReg);
    for (uint32_t i = 0; i < nReg; i++)
    {
        plGenRefMsg* refMsg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, kRefVisRegion);
        mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
    }
}

void plDrawInterface::Write(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Write(s, mgr);

    s->WriteLE32((uint32_t)fDrawables.size());
    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        s->WriteLE32(fDrawableIndices[i]);
        
        mgr->WriteKey(s, fDrawables[i]);
    }
    
    s->WriteLE32((uint32_t)fRegions.size());
    for (hsKeyedObject* region : fRegions)
        mgr->WriteKey(s, region);
}

//// ReleaseData //////////////////////////////////////////////////////////////
//  Called by SceneViewer to release the data for this given object (when
//  its parent sceneObject is deleted).

void    plDrawInterface::ReleaseData()
{
    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        if( fDrawables[i] && (fDrawableIndices[i] != uint32_t(-1)) )
        {
            plDISpansMsg* diMsg = new plDISpansMsg(fDrawables[i]->GetKey(), plDISpansMsg::kRemovingSpan, fDrawableIndices[i], 0);
            diMsg->SetSender(GetKey());
            diMsg->Send();
        }
        //fDrawableIndices[i] = uint32_t(-1);
    }
    fDrawables.clear();
    fDrawableIndices.clear();
}

void plDrawInterface::ICheckDrawableIndex(size_t which)
{
    if (which >= fDrawableIndices.size())
    {
        fDrawables.resize(which + 1);
        fDrawableIndices.resize(which + 1, uint32_t(-1));
    }
}

void plDrawInterface::ISetDrawable(size_t which, plDrawable* dr)
{
    ICheckDrawableIndex(which);
    fDrawables[which] = dr;
    
    if( dr )
        dr->SetSceneNode(GetSceneNode());

    // We might read the vis regions before the drawables, so
    // we have to check for any already loaded.
    ISetVisRegions(which);
    
#ifdef HS_DEBUGGING
    if( fDrawableIndices[which] != (uint32_t)-1 )
    {
        plDISpansMsg* diMsg = new plDISpansMsg(dr->GetKey(), plDISpansMsg::kAddingSpan, fDrawableIndices[which], 0);
        diMsg->SetSender(GetKey());
        diMsg->Send();
    }
#endif
}

void plDrawInterface::IRemoveDrawable(plDrawable *dr)
{
    auto iter = std::find(fDrawables.cbegin(), fDrawables.cend(), dr);
    if (iter != fDrawables.cend())
    {
        size_t idx = std::distance(fDrawables.cbegin(), iter);
        fDrawables[idx] = nullptr;
        fDrawableIndices[idx] = uint32_t(-1);
    }
    else
    {
        hsAssert(false, "Trying to remove a drawable that doesn't belong to us");
    }
}

void plDrawInterface::ISetVisRegion(hsKeyedObject* reg, bool on)
{
    for (size_t i = 0; i < fDrawables.size(); i++)
    {
        if( fDrawables[i] && (fDrawableIndices[i] != uint32_t(-1)) )
        {
            fDrawables[i]->SetDISpanVisSet(fDrawableIndices[i], reg, on);
        }
    }
    auto idx = std::find(fRegions.cbegin(), fRegions.cend(), reg);
    if( on )
    {
        if (idx == fRegions.cend())
            fRegions.emplace_back(reg);
    }
    else
    {
        if (idx != fRegions.cend())
            fRegions.erase(idx);
    }
}

void plDrawInterface::ISetVisRegions(size_t iDraw)
{
    if( fDrawables[iDraw] && (fDrawableIndices[iDraw] != uint32_t(-1)) )
    {
        for (hsKeyedObject* region : fRegions)
        {
            fDrawables[iDraw]->SetDISpanVisSet(fDrawableIndices[iDraw], region, true);
        }
    }
}

// Export only. Use messages for runtime
void plDrawInterface::SetDrawable(size_t which, plDrawable *dr)
{
    if( dr )
    {
        // This is a little convoluted, but it makes GCC happy and doesn't hurt anybody.
        plIntRefMsg* intRefMsg = new plIntRefMsg(GetKey(), plRefMsg::kOnCreate, which, plIntRefMsg::kDrawable);
        plRefMsg* refMsg = intRefMsg;
//      hsgResMgr::ResMgr()->SendRef(dr->GetKey(), intRefMsg, plRefFlags::kActiveRef); // THIS WON'T COMPILE UNDER GCC
        hsgResMgr::ResMgr()->SendRef(dr, refMsg, plRefFlags::kActiveRef);
    }
    else
    {
        ISetDrawable(which, nullptr);
    }
}

bool plDrawInterface::MsgReceive(plMessage* msg)
{
    plIntRefMsg* intRefMsg = plIntRefMsg::ConvertNoRef(msg);
    if( intRefMsg )
    {
        switch( intRefMsg->fType )
        {
        case plIntRefMsg::kDrawable:
            if( intRefMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                IRemoveDrawable(plDrawable::ConvertNoRef(intRefMsg->GetRef()));
            }
            else
            {
                ISetDrawable(intRefMsg->fWhich, plDrawable::ConvertNoRef(intRefMsg->GetRef()));
            }
            return true;
        default:
            break;
        }
    }
    plGenRefMsg* genRefMsg = plGenRefMsg::ConvertNoRef(msg);
    if( genRefMsg )
    {
        switch( genRefMsg->fType )
        {
        case kRefVisRegion:
            if( genRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
                ISetVisRegion(genRefMsg->GetRef(), true);
            else
                ISetVisRegion(genRefMsg->GetRef(), false);
            break;
        default:
            break;
        }
        return true;
    }
    plEnableMsg* pEnableMsg = plEnableMsg::ConvertNoRef( msg );
    if (pEnableMsg)
    {
        SetProperty(kDisable, pEnableMsg->Cmd(plEnableMsg::kDisable));
        if( GetOwner() )
            SetTransform(GetOwner()->GetLocalToWorld(), GetOwner()->GetWorldToLocal());
        return true;
    }
    return plObjInterface::MsgReceive(msg);
}

void plDrawInterface::SetUpForParticleSystem(uint32_t maxNumEmitters, uint32_t maxNumParticles,
                                             hsGMaterial *material, const std::vector<plKey>& lights)
{
    hsAssert(fDrawables[0] != nullptr, "No drawable to use for particle system!");
    SetDrawableMeshIndex( 0, fDrawables[0]->CreateParticleSystem( maxNumEmitters, maxNumParticles, material ) );
    for (const plKey& light : lights)
    {
        hsgResMgr::ResMgr()->AddViaNotify(light, new plGenRefMsg(fDrawables[0]->GetKey(), plRefMsg::kOnCreate, fDrawableIndices[0], plDrawable::kMsgPermaLightDI), plRefFlags::kPassiveRef);
    }

    ISetVisRegions(0);
}

void    plDrawInterface::ResetParticleSystem()
{
    hsAssert(fDrawables[0] != nullptr, "No drawable to use for particle system!");
    fDrawables[0]->ResetParticleSystem( fDrawableIndices[0] );
}

void    plDrawInterface::AssignEmitterToParticleSystem( plParticleEmitter *emitter )
{
    hsAssert(fDrawables[0] != nullptr, "No drawable to use for particle system!");
    fDrawables[0]->AssignEmitterToParticleSystem( fDrawableIndices[0], emitter );
}


