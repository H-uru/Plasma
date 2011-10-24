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
#include "hsResMgr.h"
#include "hsTemplates.h"
#include "plSharedMesh.h"
#include "plGeometrySpan.h"
#include "plInstanceDrawInterface.h"
#include "plDrawableSpans.h"
#include "plMorphSequence.h"

plSharedMesh::plSharedMesh() : fMorphSet(nil), fFlags(0)
{
}

plSharedMesh::~plSharedMesh()
{
    hsAssert(fActiveInstances.GetCount() == 0, "Tried to delete a shared mesh that has active instances.");
    
    while (fSpans.GetCount() > 0)
        delete fSpans.Pop();
}
/*
void plSharedMesh::CreateInstance(plSceneObject *so, UInt8 boneIndex)
{   
plDrawInterface *di = so->GetVolatileDrawInterface();

  //    hsAssert((fActiveInstances.GetCount == 0) || 
  //             (di->GetDrawable() == fActiveInstances[0]->GetDrawInterface()->GetDrawable()),
  //             "Trying to share a mesh between two seperate drawables. No can do.");
  
    
      fActiveInstances.Append(so);
      }
      
        void plSharedMesh::RemoveInstance(plSceneObject *so)
        {
        so->GetVolatileDrawInterface()->ReleaseData();
        
          fActiveInstances.RemoveItem(so);
          }
*/

hsBool plSharedMesh::MsgReceive(plMessage* msg)
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
    if (refMsg)
    {
        plMorphDataSet *set = plMorphDataSet::ConvertNoRef(refMsg->GetRef());
        if (set)
        {
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fMorphSet = plMorphDataSet::ConvertNoRef(refMsg->GetRef());
            }
            else if( refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
            {
                fMorphSet = nil;
            }
            return true;
        }
    }
    
    return hsKeyedObject::MsgReceive(msg);
}

// Currently, active instances are not meant to be created at export and written to disk.
void plSharedMesh::Read(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Read(s, mgr);
    
    int i;
    fSpans.SetCount(s->ReadLE32());
    for (i = 0; i < fSpans.GetCount(); i++)
    {
        fSpans[i] = TRACKED_NEW plGeometrySpan;
        fSpans[i]->Read(s);
    }

    mgr->ReadKeyNotifyMe(s, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef);
    fFlags = s->ReadByte();
}

void plSharedMesh::Write(hsStream* s, hsResMgr* mgr)
{
    hsKeyedObject::Write(s, mgr);

    int i;
    s->WriteLE32(fSpans.GetCount());
    for (i = 0; i < fSpans.GetCount(); i++)
        fSpans[i]->Write(s);

    mgr->WriteKey(s, (fMorphSet ? fMorphSet->GetKey() : nil));
    s->WriteByte(fFlags);
}

//////////////////////////////////////////////////////////////////////////////////////

plSharedMeshBCMsg::plSharedMeshBCMsg() : plMessage(), fMesh(nil), fIsAdding(true) { SetBCastFlag(plMessage::kBCastByExactType); }
