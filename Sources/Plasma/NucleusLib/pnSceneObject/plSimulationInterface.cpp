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
#include "plSimulationInterface.h"

#include "plgDispatch.h"
#include "plPhysical.h"
#include "hsBounds.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plSceneObject.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plWarpMsg.h"
#include "pnMessage/plSimulationSynchMsg.h"
#include "pnMessage/plSimulationMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnKeyedObject/plKey.h"

plSimulationInterface::plSimulationInterface() : fPhysical()
{
}

plSimulationInterface::~plSimulationInterface()
{
}

void plSimulationInterface::ISetSceneNode(const plKey& newNode)
{
    if (fPhysical)
        fPhysical->SetSceneNode(newNode);
}

void plSimulationInterface::SetProperty(int prop, bool on)
{
    plObjInterface::SetProperty(prop, on);      // set the property locally

    if (fPhysical)
        fPhysical->SetProperty(prop, on ? true : false);
}

void plSimulationInterface::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    if (fPhysical)
        fPhysical->SetTransform(l2w, w2l);
}

void plSimulationInterface::ClearLinearVelocity()
{
    if (fPhysical)
        fPhysical->ClearLinearVelocity();
}

void plSimulationInterface::Read(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Read(s, mgr);

    // fProps is already read by plObjInterface, but we'll do it again here to
    // avoid breaking the format
    fProps.Read(s);
    // Also unnecessary
    (void)s->ReadLE32();

    plIntRefMsg* refMsg = new plIntRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kPhysical, 0);
    mgr->ReadKeyNotifyMe(s, refMsg, plRefFlags::kActiveRef);
}

void plSimulationInterface::Write(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Write(s, mgr);

    // Legacy crap
    fProps.Write(s);
    s->WriteLE32(0);

    mgr->WriteKey(s, fPhysical);
}

void plSimulationInterface::ReleaseData()
{
    plPhysical *physical = fPhysical;

    if (physical)
    {
        // To get rid of our data, we need to release our active ref and tell the SceneNode
        // to dump it. It will autodestruct after those two active refs are released, unless
        // someone else has a ref on it as well (in which case we don't want to be nuking it
        // anyway).
        if (physical->GetSceneNode())
        {
            plKey nodeKey = physical->GetSceneNode();

            nodeKey->Release(physical->GetKey());
        }

        GetKey()->Release(physical->GetKey());
    }
}

bool plSimulationInterface::MsgReceive(plMessage* msg)
{
    plIntRefMsg* intRefMsg = plIntRefMsg::ConvertNoRef(msg);
    if (intRefMsg)
    {
        switch (intRefMsg->fType)
        {
        case plIntRefMsg::kPhysical:
            if (intRefMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove))
            {
                plPhysical* phys = plPhysical::ConvertNoRef(intRefMsg->GetRef());
                // *** for some reason, we sometimes get a null pointer here
                // *** if we do, we're just going to ignore it for now
                if (phys)
                {
                    hsAssert(phys == fPhysical, "Removing Physical I don't have");
                    fPhysical = nullptr;
                }
            }
            else
            {
                fPhysical = plPhysical::ConvertNoRef(intRefMsg->GetRef());
            }
            return true;
        }
    }

    plEnableMsg* pEnableMsg = plEnableMsg::ConvertNoRef(msg);
    if (pEnableMsg)
    {
        SetProperty(kDisable, pEnableMsg->Cmd(kDisable));
        SetProperty(kPinned, pEnableMsg->Cmd(kDisable));
        return true;
    }

    plWarpMsg* pWarpMsg = plWarpMsg::ConvertNoRef(msg);
    if (pWarpMsg)
    {
        if (fPhysical)
        {
            hsMatrix44 l2w = pWarpMsg->GetTransform();
            hsMatrix44 inv;
            l2w.GetInverse(&inv);
            SetTransform(l2w, inv);
            if (pWarpMsg->GetWarpFlags() & plWarpMsg::kZeroVelocity)
            {
                ClearLinearVelocity();
            }
        }
    }

    plSimulationMsg* pSimMsg = plSimulationMsg::ConvertNoRef(msg);
    if (pSimMsg)
    {
        if (fPhysical)
            fPhysical->MsgReceive(pSimMsg);
    }
    
    return plObjInterface::MsgReceive(msg);
}

// Export only. Use messages for runtime.
void plSimulationInterface::SetPhysical(plPhysical* ph)
{
    fPhysical = ph;
}

plPhysical* plSimulationInterface::GetPhysical() const
{
    return fPhysical;
}

