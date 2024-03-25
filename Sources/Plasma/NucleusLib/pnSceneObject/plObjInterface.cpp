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
#include "plObjInterface.h"
#include "hsResMgr.h"
#include "pnKeyedObject/plKey.h"
#include "plSceneObject.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plEnableMsg.h"


plObjInterface::plObjInterface()
:   fOwner()
{
}

plObjInterface::~plObjInterface()
{
}

void plObjInterface::ISetOwner(plSceneObject* owner)
{
    if( fOwner != owner )
    {
        fOwner = owner;
        if( fOwner )
            fOwner->ISetInterface(this);
    }
}

void plObjInterface::Read(hsStream* s, hsResMgr* mgr)
{
    plSynchedObject::Read(s, mgr);

    mgr->ReadKeyNotifyMe(s, new plIntRefMsg(GetKey(), plRefMsg::kOnCreate, 0, plIntRefMsg::kOwner), plRefFlags::kPassiveRef);

    fProps.Read(s);
}

void plObjInterface::Write(hsStream* s, hsResMgr* mgr)
{
    plSynchedObject::Write(s, mgr);

    mgr->WriteKey(s, fOwner);
    fProps.Write(s);
}

bool plObjInterface::MsgReceive(plMessage* msg)
{
    plEnableMsg* enaMsg = plEnableMsg::ConvertNoRef(msg);
    if( enaMsg )
    {
        SetProperty(kDisable, enaMsg->Cmd(plEnableMsg::kDisable));
        return true;
    }
    plIntRefMsg* intRefMsg = plIntRefMsg::ConvertNoRef(msg);
    if( intRefMsg )
    {
        switch( intRefMsg->fType )
        {
        case plIntRefMsg::kOwner:
            if( intRefMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                plSceneObject* owner = plSceneObject::ConvertNoRef(intRefMsg->GetRef());
                ISetOwner(owner);
            }
            else if( intRefMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                ISetOwner(nullptr);
            }
            break;
        }
    }
    return plSynchedObject::MsgReceive(msg);
}

//
// assign and update my properties from the bitVector passed in
//
void plObjInterface::ISetAllProperties(const hsBitVector& b)
{
//  if (&b != &fProps)  // don't copy if they are the same variable 

        fProps = b;

    int i;
    for(i=0;i<GetNumProperties(); i++)
        SetProperty(i, GetProperty(i));
}

