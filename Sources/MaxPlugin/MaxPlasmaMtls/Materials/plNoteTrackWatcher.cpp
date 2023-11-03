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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plNoteTrackWatcher - Dummy object to watch for notetrack additions or   //
//                       removals from the main material that owns it.      //
//                       All of this is required because MAX will notify    //
//                       an object's dependents about notetrack actions but //
//                       NOT the object itself, and the Add/DeleteNoteTrack //
//                       functions are non-virtual. ARRRGH!                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "../resource.h"

#include "plNoteTrackWatcher.h"
#include "plPassMtlBase.h"

//// Watcher Class Desc //////////////////////////////////////////////////////

plNoteTrackWatcher::plNoteTrackWatcher(plPassMtlBase *parentMtl) : fParentMtl()
{
    fNoteTrackCount = parentMtl->NumNoteTracks();
    ReplaceReference(kRefParentMtl, parentMtl);
}

plNoteTrackWatcher::~plNoteTrackWatcher()
{
    if (fParentMtl != nullptr)
    {
        fParentMtl->fNTWatcher = nullptr;
        DeleteReference( kRefParentMtl );
    }
    DeleteAllRefsFromMe();
}

BOOL    plNoteTrackWatcher::IsRealDependency( ReferenceTarget *rtarg )
{
    if( rtarg == fParentMtl )
        return false;

    return true;
}

int plNoteTrackWatcher::NumRefs()
{
    return 1;
}

RefTargetHandle plNoteTrackWatcher::GetReference( int i )
{
    if( i == kRefParentMtl )
        return fParentMtl;

    return nullptr;
}

void plNoteTrackWatcher::SetReference( int i, RefTargetHandle rtarg )
{
    if( i == kRefParentMtl )
        fParentMtl = (plPassMtlBase *)rtarg;
}

RefResult plNoteTrackWatcher::NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
                                               PartID& partID, RefMessage message MAX_REF_PROPAGATE)
{
    switch( message )
    {
        case REFMSG_SUBANIM_STRUCTURE_CHANGED:
            if (hTarget == fParentMtl && fParentMtl != nullptr)
            {
                // Structure of parent material changed--did it gain or lose a notetrack?
                int oldCount = fNoteTrackCount;
                fNoteTrackCount = fParentMtl->NumNoteTracks();
                if( oldCount != fNoteTrackCount )
                {
                    // Is it an addition?
                    if( fNoteTrackCount > oldCount )
                        // Yes, notify parent.
                        fParentMtl->NoteTrackAdded();
                    else 
                        // Deletion, also notify parent
                        fParentMtl->NoteTrackRemoved();
                }
            }
            break;

        case REFMSG_NODE_NAMECHANGE:
            if( hTarget == fParentMtl )
            {
                fParentMtl->NameChanged();
            }
            break;

        case REFMSG_TARGET_DELETED:
            fParentMtl = nullptr;
            break;
    }

    return REF_SUCCEED;
}
