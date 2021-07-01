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
//  plPassMtlBase - Base class for all Plasma MAX materials                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsBitVector.h"

#include "MaxMain/MaxAPI.h"

#include "../resource.h"

#include "plPassMtlBase.h"
#include "plPassBaseParamIDs.h"
#include "plNoteTrackWatcher.h"
#include "plAnimStealthNode.h"

#include "MaxComponent/plMaxAnimUtils.h"
#include "MaxMain/plPlasmaRefMsgs.h"

// For converting from a MAX Mtl
#include "plPassMtl.h"
#include "plBumpMtl.h"
#include "plDecalMtl.h"

using namespace plPassBaseParamIDs;

IMtlParams *plPassMtlBase::fIMtlParams = nullptr;

//// plPostLoadHandler ///////////////////////////////////////////////////////
//  Small class to keep track of all the materials to update after load

class plPostLoadHandler
{
    static bool fLoading;
    static std::vector<plPassMtlBase *> fPostLoads;

    public:
        static bool IsLoading() { return fLoading; }

        static void PostLoadFixupFunction( void *param, NotifyInfo *info )
        {
            fLoading = false;

            for (plPassMtlBase* mtl : fPostLoads)
                mtl->PostLoadAnimPBFixup();

            fPostLoads.clear();
            UnRegisterNotification( PostLoadFixupFunction, param, NOTIFY_FILE_POST_OPEN );
            UnRegisterNotification( PostLoadFixupFunction, param, NOTIFY_FILE_POST_MERGE );
        }

        static void AddPostLoad( plPassMtlBase *mtl )
        {
            fLoading = true;

            if (fPostLoads.empty())
            {
                RegisterNotification( PostLoadFixupFunction, mtl, NOTIFY_FILE_POST_OPEN );
                RegisterNotification( PostLoadFixupFunction, mtl, NOTIFY_FILE_POST_MERGE );
            }

            mtl->SetLoadingFlag( true );
            fPostLoads.emplace_back(mtl);
        }

        static void RemovePostLoad( plPassMtlBase *mtl )
        {
            auto iter = std::find(fPostLoads.cbegin(), fPostLoads.cend(), mtl);
            if (iter != fPostLoads.cend())
                fPostLoads.erase(iter);
        }
};
std::vector<plPassMtlBase *> plPostLoadHandler::fPostLoads;
bool plPostLoadHandler::fLoading = false;


plPassMtlBase::plPassMtlBase(BOOL loading) : fNTWatcher(), fBasicPB(), fAdvPB(), fLayersPB(), fAnimPB(),
                                 fLoading( loading )
{
    fNTWatcher = new plNoteTrackWatcher( this );
    Reset();
}

plPassMtlBase::~plPassMtlBase()
{
    if( fLoading )
        plPostLoadHandler::RemovePostLoad( this );

    // Force the watcher's parent pointer to nil, otherwise the de-ref will attempt to re-delete us
    fNTWatcher->SetReference(plNoteTrackWatcher::kRefParentMtl, nullptr);
    delete fNTWatcher;
    fNTWatcher = nullptr;

    // Manually delete our notetrack refs, otherwise there'll be hell to pay
    for (size_t i = 0; i < fNotetracks.size(); i++)
    {
        if (fNotetracks[i] != nullptr)
            DeleteReference( kRefNotetracks + i );
    }
}

void    plPassMtlBase::Reset()
{
    fIValid.SetEmpty();
}

//// Stealth Accessors ///////////////////////////////////////////////////////

int     plPassMtlBase::GetNumStealths()
{
    return IGetNumStealths( true );
}

plAnimStealthNode   *plPassMtlBase::GetStealth( int index )
{
    return IGetStealth( index, false );
}

int plPassMtlBase::IGetNumStealths( bool update )
{
    if( update )
        IUpdateAnimNodes();

    return fAnimPB->Count( (ParamID)kPBAnimStealthNodes );
}

plAnimStealthNode   *plPassMtlBase::IGetStealth( int index, bool update )
{
    if( update )
        IUpdateAnimNodes();

    return (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, index );
}

plAnimStealthNode   *plPassMtlBase::IFindStealth( const ST::string &segmentName )
{
    int     i;


    for( i = 0; i < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); i++ )
    {
        plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, i );
        ST::string name = node->GetSegmentName();

        if (node != nullptr && name.compare(segmentName) == 0)
        {
            return node;
        }
    }

    return nullptr;
}

//// IVerifyStealthPresent ///////////////////////////////////////////////////
//  Ensures that we have a stealth for the given segment.

plAnimStealthNode   *plPassMtlBase::IVerifyStealthPresent( const ST::string &animName )
{
    // If we're in the middle of loading, don't check
    if (plPostLoadHandler::IsLoading())
        return nullptr;

    plAnimStealthNode *stealth = IFindStealth( animName );
    if (stealth == nullptr)
    {
        // New segment, add a new stealth node
        stealth = (plAnimStealthNode *)GetCOREInterface()->CreateInstance( HELPER_CLASS_ID, ANIMSTEALTH_CLASSID );
        INode *node = GetCOREInterface()->CreateObjectNode( stealth );
        stealth->SetSegment((animName.compare(ENTIRE_ANIMATION_NAME) != 0) ? animName : "");
        stealth->SetNodeName( GetName() );
        node->Freeze( true );

        // Skip the attach, since we might not find a valid INode. This will leave the node attached to the scene
        // root, which is fine. Since we just care about it being SOMEWHERE in the scene hierarchy
        /*
        if( fAnimPB->Count( (ParamID)kPBAnimStealthNodes ) > 0 )
        {
            plAnimStealthNode *first = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, 0 );
            first->GetINode()->AttachChild( node );
        }
        */

        fAnimPB->Append( (ParamID)kPBAnimStealthNodes, 1, (ReferenceTarget **)&stealth );

        ST::string realName = stealth->GetSegmentName();

        fStealthsChanged = true;
    }
    else
    {
        // Exists already, we're ok
        stealth->SetParentMtl( this );
    }
    return stealth;
}

//// Change Callbacks ////////////////////////////////////////////////////////

void    plPassMtlBase::RegisterChangeCallback( plMtlChangeCallback *callback )
{
    if (std::find(fChangeCallbacks.cbegin(), fChangeCallbacks.cend(), callback) == fChangeCallbacks.cend())
        fChangeCallbacks.emplace_back(callback);
}

void    plPassMtlBase::UnregisterChangeCallback( plMtlChangeCallback *callback )
{
    auto iter = std::find(fChangeCallbacks.cbegin(), fChangeCallbacks.cend(), callback);
    if (iter != fChangeCallbacks.cend())
        fChangeCallbacks.erase(iter);
}

//// IUpdateAnimNodes ////////////////////////////////////////////////////////
//  Updates the list of stealth nodes in the anim paramBlock to match our
//  list of anim segments.

void    plPassMtlBase::IUpdateAnimNodes()
{
    // Our beautiful hack, to make sure we don't update until we actually are loaded
    if( fLoading )
        return;

    SegmentMap *segMap = GetAnimSegmentMap(this, nullptr);

    std::vector<plAnimStealthNode *> goodNodes;

    // Keep track of whether we change anything
    fStealthsChanged = false;

    // Verify one for "entire animation"
    plAnimStealthNode *stealth = IVerifyStealthPresent( ENTIRE_ANIMATION_NAME );
    goodNodes.emplace_back(stealth);

    // Verify segment nodes
    if (segMap != nullptr)
    {
        for( SegmentMap::iterator i = segMap->begin(); i != segMap->end(); i++ )
        {
            SegmentSpec *spec = (*i).second;

            if( spec->fType == SegmentSpec::kAnim )
            {
                plAnimStealthNode *stealth = IVerifyStealthPresent( spec->fName );
                goodNodes.emplace_back(stealth);
            }
        }

        DeleteSegmentMap( segMap );
    }

    // Remove nodes that no longer have segments
    for (int idx = 0; idx < fAnimPB->Count((ParamID)kPBAnimStealthNodes); )
    {
        plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );

        if (node != nullptr && std::find(goodNodes.cbegin(), goodNodes.cend(), node) == goodNodes.cend())
        {
            fAnimPB->Delete( (ParamID)kPBAnimStealthNodes, idx, 1 );
//          GetCOREInterface()->DeleteNode( node->GetINode() );
            fStealthsChanged = true;
        }
        else
            idx++;
    }

    if( fStealthsChanged )
    {
        // Yup, our list of stealths got updated. Notify everyone of such.
        for (plMtlChangeCallback* callback : fChangeCallbacks)
            callback->SegmentListChanged();
    }
}

//// NameChanged /////////////////////////////////////////////////////////////
//  Notify from NTWatcher so we can update the names of our stealth nodes

void    plPassMtlBase::NameChanged()
{
    for( int idx = 0; idx < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); idx++ )
    {
        plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );
        if (node != nullptr)
            node->SetNodeName( GetName() );
    }
}

//// NoteTrackAdded/Removed //////////////////////////////////////////////////
//  Notifies from NTWatcher so we can update our list of stealths

void    plPassMtlBase::NoteTrackAdded()
{
    // Make a ref to our new notetrack
    for (int i = 0; i < NumNoteTracks(); i++)
    {
        NoteTrack *track = GetNoteTrack( i );

        if (std::find(fNotetracks.cbegin(), fNotetracks.cend(), track) == fNotetracks.cend())
        {
            ReplaceReference(kRefNotetracks + fNotetracks.size(), track);
            break;
        }
    }

    for (plMtlChangeCallback* callback : fChangeCallbacks)
        callback->NoteTrackListChanged();
    IUpdateAnimNodes();
}

void    plPassMtlBase::NoteTrackRemoved()
{
    hsBitVector stillThere;


    // Make a ref to our new notetrack
    for (int i = 0; i < NumNoteTracks(); i++)
    {
        NoteTrack *track = GetNoteTrack( i );

        auto iter = std::find(fNotetracks.cbegin(), fNotetracks.cend(), track);
        if (iter != fNotetracks.cend())
            stillThere.Set(iter - fNotetracks.cbegin());
    }

    for (size_t i = 0; i < fNotetracks.size(); i++)
    {
        if (!stillThere.IsBitSet(i) && fNotetracks[i] != nullptr)
        {
//          DeleteReference( kRefNotetracks + i );
            SetReference(kRefNotetracks + i, nullptr);
        }
    }

    for (plMtlChangeCallback* callback : fChangeCallbacks)
        callback->NoteTrackListChanged();
    IUpdateAnimNodes();
}


//////////////////////////////////////////////////////////////////////////////
//// MAX Ref Stuff ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// NumRefs /////////////////////////////////////////////////////////////////

int plPassMtlBase::NumRefs()
{
    return 4 + (int)fNotetracks.size();
}

//// GetReference ////////////////////////////////////////////////////////////

RefTargetHandle plPassMtlBase::GetReference( int i )
{
    if (i >= kRefNotetracks && i < kRefNotetracks + (int)fNotetracks.size())
        return fNotetracks[ i - kRefNotetracks ];
    return nullptr;
}

//// SetReference ////////////////////////////////////////////////////////////

void plPassMtlBase::SetReference(int i, RefTargetHandle rtarg)
{
    if( i >= kRefNotetracks )
    {
        if (i - kRefNotetracks >= (int)fNotetracks.size())
            fNotetracks.resize(i - kRefNotetracks + 1);
        fNotetracks[i - kRefNotetracks] = (NoteTrack*)rtarg;
    }
}

//// NotifyRefChanged ////////////////////////////////////////////////////////

RefResult plPassMtlBase::NotifyRefChanged( MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget,
                                           PartID &partID, RefMessage message MAX_REF_PROPAGATE )
{
    switch( message )
    {
        case REFMSG_CHANGE:
            fIValid.SetEmpty();

            // see if this message came from a changing parameter in the pblock,
            // if so, limit rollout update to the changing item
            if (hTarget == fBasicPB || hTarget == fAdvPB || hTarget == fLayersPB || hTarget == fAnimPB)
            {
                IParamBlock2 *pb = (IParamBlock2*)hTarget;

                ParamID changingParam = pb->LastNotifyParamID();
                pb->GetDesc()->InvalidateUI( changingParam );
                
                // And let the SceneWatcher know that the material on some of it's
                // referenced objects changed.
                if (MAX_REF_PROPAGATE_VALUE)
                    NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_MAT );
            }
            else 
            {
                // Was it a notetrack ref?
                if (std::find(fNotetracks.cbegin(), fNotetracks.cend(), (NoteTrack *)hTarget) != fNotetracks.cend())
                {
                    // Yup, so update our notetrack list
                    IUpdateAnimNodes();
                }
            }

            break;

        case REFMSG_TARGET_DELETED:
            NoteTrackRemoved();
            break;

    }

    return REF_SUCCEED;
}

//////////////////////////////////////////////////////////////////////////////
//// Standard IO (or non-standard, as the case may be) ///////////////////////
//////////////////////////////////////////////////////////////////////////////

//// PostLoadAnimPBFixup /////////////////////////////////////////////////////
//  Takes the old version of the anim paramblock and translates it into the
//  new version.
//  Note that there's an interesting (?) side effect of this: for new materials,
//  we'll incorrectly detect them as the "old" format and fix-up them as well.
//  This means that we'll end up with the same defaults for (Entire Animation)
//  that we had for the old materials. We can easily change the defaults by
//  changing the defaults for the old paramblock though.
//  Also, we go ahead and re-work the stealth parent pointers here, since it
//  appears to be the ONLY time we can do it and have it reliably work. ARRGH!
//
//  History of the options that we CAN'T do and why (and hence why this):
//      - ParamBlock accessor doesn't work. For some reason, the accessor isn't
//        always called on load, even with P_CALLSETS_ON_LOAD specified
//      - Doing it on Load() doesn't work, because neither the ParamBlocks are
//        guaranteed to be fully loaded (with their tabs filled) nor are the
//        notetracks necessarily attached yet
//      - Notetracks can also possibly be attached BEFORE load, which doesn't
//        do us a damned bit of good, so we need to make sure we're fully
//        loaded before we run this function. Unfortunately, the only time
//        we're guaranteed THAT is by a FILE_POST_OPEN notify. (post-load
//        callbacks don't work because they're called right after our object
//        is loaded but not necessarily before the notetracks are attached)

void    plPassMtlBase::PostLoadAnimPBFixup()
{
    SetLoadingFlag( false );

#ifdef MCN_UPGRADE_OLD_ANIM_BLOCKS
    if( fAnimPB->Count( (ParamID)kPBAnimStealthNodes ) == 0 )
    {
        // Yup, old style. So our update process looks like this:
        // 1) Create stealths for all our segments as we are now
        // 2) Set the parameters on all of them to our old defaults (no autostart,
        //    loop on entire, no ease).
        // 3) Copy the old paramblock values to the single stealth indicated by
        //    the old PB

        // Step 1...
        IUpdateAnimNodes();

        // Step 2...
        for( int i = 0; i < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); i++ )
        {
            plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, i );
            ST::string name = node->GetSegmentName();
            node->SetAutoStart( false );
            node->SetLoop( true, ENTIRE_ANIMATION_NAME );
            node->SetEaseIn( plAnimEaseTypes::kNoEase, 1.f, 1.f, 1.f );
            node->SetEaseOut( plAnimEaseTypes::kNoEase, 1.f, 1.f, 1.f );
        }

        // Step 3...
        const MCHAR* oldSel = fAnimPB->GetStr( (ParamID)kPBAnimName );
        if (oldSel == nullptr)
            oldSel = _M(ENTIRE_ANIMATION_NAME);
        plAnimStealthNode *myNew = IFindStealth( M2ST( oldSel ) );
        if (myNew != nullptr)
        {
            myNew->SetAutoStart( (bool)fAnimPB->GetInt( (ParamID)kPBAnimAutoStart ) );
            myNew->SetLoop( (bool)fAnimPB->GetInt( (ParamID)kPBAnimLoop ),
                            M2ST(fAnimPB->GetStr( (ParamID)kPBAnimLoopName ) ) );
            myNew->SetEaseIn( (uint8_t)fAnimPB->GetInt( (ParamID)kPBAnimEaseInType ),
                                (float)fAnimPB->GetFloat( (ParamID)kPBAnimEaseInLength ),
                                (float)fAnimPB->GetFloat( (ParamID)kPBAnimEaseInMin ),
                                (float)fAnimPB->GetFloat( (ParamID)kPBAnimEaseInMax ) );
            myNew->SetEaseOut( (uint8_t)fAnimPB->GetInt( (ParamID)kPBAnimEaseOutType ),
                                (float)fAnimPB->GetFloat( (ParamID)kPBAnimEaseOutLength ),
                                (float)fAnimPB->GetFloat( (ParamID)kPBAnimEaseOutMin ),
                                (float)fAnimPB->GetFloat( (ParamID)kPBAnimEaseOutMax ) );
        }               
    }
#endif // MCN_UPGRADE_OLD_ANIM_BLOCKS


    // Make sure the parent is set tho. Note: we have to do this because, for some *(#$&(* reason,
    // when we're loading a file, MAX can somehow add the stealths to our tab list WITHOUT calling
    // the accessor for it (and the tab is empty on the CallSetsOnLoad() pass, for some reason).
    for( int i = 0; i < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); i++ )
    {
        plAnimStealthNode *node = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, i );
        node->SetParentMtl( this );
    }
}


#define MTL_HDR_CHUNK 0x4000

//// Load ////////////////////////////////////////////////////////////////////
//  Our actual MAX load function

IOResult plPassMtlBase::Load(ILoad *iload)
{
    plPostLoadHandler::AddPostLoad( this );

    IOResult res;
    int id;
    while (IO_OK==(res=iload->OpenChunk()))
    {
        switch(id = iload->CurChunkID())
        {
            case MTL_HDR_CHUNK:
                res = MtlBase::Load(iload);
                break;
        }
        iload->CloseChunk();
        if (res!=IO_OK) 
            return res;
    }

    return IO_OK;
}

//// Save ////////////////////////////////////////////////////////////////////
//  The MAX flip-side

IOResult plPassMtlBase::Save(ISave *isave)
{ 
    IOResult res;
    isave->BeginChunk(MTL_HDR_CHUNK);
    res = MtlBase::Save(isave);
    if (res!=IO_OK) return res;
    isave->EndChunk();

    return IO_OK;
}   

//// ICloneBase //////////////////////////////////////////////////////////////

void    plPassMtlBase::ICloneBase( plPassMtlBase *target, RemapDir &remap )
{
    *((MtlBase*)target) = *((MtlBase*)this); 
    ICloneRefs( target, remap );

    for( int idx = 0; idx < fAnimPB->Count( (ParamID)kPBAnimStealthNodes ); idx++ )
    {
        IParamBlock2 *pb = target->fAnimPB;
        plAnimStealthNode *stealth = (plAnimStealthNode *)fAnimPB->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );
        pb->SetValue( (ParamID)kPBAnimStealthNodes, 0, remap.CloneRef( stealth ), idx );

        stealth = (plAnimStealthNode *)pb->GetReferenceTarget( (ParamID)kPBAnimStealthNodes, 0, idx );
        INode *node = GetCOREInterface()->CreateObjectNode( stealth );
        stealth->SetNodeName( GetName() );
        node->Freeze( true );
    }

    BaseClone(this, target, remap);
    target->fIValid.SetEmpty(); 
}

//// ConvertToPassMtl ////////////////////////////////////////////////////////
//  Static convert to our plPassMtlBase type, if possible

plPassMtlBase   *plPassMtlBase::ConvertToPassMtl( Mtl *mtl )
{
    if (mtl == nullptr)
        return nullptr;

    if( mtl->ClassID() == PASS_MTL_CLASS_ID 
        || mtl->ClassID() == BUMP_MTL_CLASS_ID
        || mtl->ClassID() == DECAL_MTL_CLASS_ID )
    {
        return (plPassMtlBase *)mtl;
    }

    return nullptr;
}

//// SetupProperties /////////////////////////////////////////////////////////

bool    plPassMtlBase::SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg )
{
    bool ret = true;

    // Call SetupProperties on all our animStealths if we have any
    int i, count = IGetNumStealths();
    for( i = 0; i < count; i++ )
    {
        if( !IGetStealth( i, false )->SetupProperties( node, pErrMsg ) )
            ret = false;
    }

    return ret;
}

//// ConvertDeInit ///////////////////////////////////////////////////////////

bool    plPassMtlBase::ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg )
{
    bool ret = true;

    // Call ConvertDeInit on all our animStealths if we have any
    int i, count = IGetNumStealths();
    for( i = 0; i < count; i++ )
    {
        if( !IGetStealth( i, false )->ConvertDeInit( node, pErrMsg ) )
            ret = false;
    }

    return ret;
}
