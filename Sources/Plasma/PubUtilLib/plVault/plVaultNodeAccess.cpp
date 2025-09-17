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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/PubUtilLib/plVault/plVaultNodeAccess.cpp
*   
***/

#include "Pch.h"

/*****************************************************************************
*
*   VaultTextNoteNode
*
***/

//============================================================================
enum EAgeInfoFields {
    kAgeFilename,
    kAgeInstName,
    kAgeUserName,
    kAgeDesc,
    kAgeInstGuid,
    kAgeLanguage,
    kAgeSequence,
    kNumAgeInfoFields
};

void VaultTextNoteNode::SetVisitInfo (const plAgeInfoStruct & info) {
    ST::string_stream str;

    for (unsigned i = 0; i < kNumAgeInfoFields; ++i) {
        switch (i) {
        case kAgeFilename:
            str << info.GetAgeFilename();
            break;
        case kAgeInstName:
            str << info.GetAgeInstanceName();
            break;
        case kAgeUserName:
            str << info.GetAgeUserDefinedName();
            break;
        case kAgeDesc:
            str << info.GetAgeDescription();
            break;
        case kAgeInstGuid:
            str << info.GetAgeInstanceGuid()->AsString();
            break;
        case kAgeLanguage:
            str << info.GetAgeLanguage();
            break;
        case kAgeSequence:
            str << info.GetAgeSequenceNumber();
            break;

        DEFAULT_FATAL(i);
        }

        if (i+1 != kNumAgeInfoFields)
            str << "|";
    }

    SetNoteText(str.to_string());
}

//============================================================================
bool VaultTextNoteNode::GetVisitInfo (plAgeInfoStruct * info) {
    std::vector<ST::string> toks = GetNoteText().split('|');
    hsAssert(toks.size() == kNumAgeInfoFields, "visit text note malformed--discarding");
    if (toks.size() != kNumAgeInfoFields)
        return false;

    if (!toks[kAgeFilename].empty())
        info->SetAgeFilename(toks[kAgeFilename]);
    if (!toks[kAgeInstName].empty())
        info->SetAgeInstanceName(toks[kAgeInstName]);
    if (!toks[kAgeUserName].empty())
        info->SetAgeUserDefinedName(toks[kAgeUserName]);
    if (!toks[kAgeDesc].empty())
        info->SetAgeDescription(toks[kAgeDesc]);
    if (!toks[kAgeInstGuid].empty()) {
        std::unique_ptr<plUUID> guid = std::make_unique<plUUID>(toks[kAgeInstGuid]);
        info->SetAgeInstanceGuid(guid.get());
    }
    if (!toks[kAgeLanguage].empty())
        info->SetAgeLanguage(toks[kAgeLanguage].to_uint());
    if (!toks[kAgeSequence].empty())
        info->SetAgeSequenceNumber(toks[kAgeSequence].to_uint());
    return true;
}


/*****************************************************************************
*
*   VaultSDLNode
*
***/

//============================================================================
bool VaultSDLNode::GetStateDataRecord (plStateDataRecord * rec, unsigned readOptions) {
    if (GetSDLData().empty())
        return false;

    hsRAMStream ram;
    ram.Write(GetSDLData().size(), GetSDLData().data());
    ram.Rewind();

    ST::string sdlRecName;
    int sdlRecVersion;
    if (!plStateDataRecord::ReadStreamHeader(&ram, &sdlRecName, &sdlRecVersion))
        return false;

    rec->SetDescriptor(sdlRecName, sdlRecVersion);

    // Note: Setting from default here results in a bug causing age SDL to
    // be incorrectly shown when immediately linking back to an age you linked
    // out of (relto door will be closed, window shut, etc).    
    // rec->SetFromDefaults(false);

    if (!rec->Read( &ram, 0, readOptions))
        return false;
        
    // If we converted the record to a newer version, re-save it.       
    if (rec->GetDescriptor()->GetVersion() != sdlRecVersion)
        SetStateDataRecord(rec, readOptions);
    
    return true;
}

//============================================================================
void VaultSDLNode::SetStateDataRecord (const plStateDataRecord * rec, unsigned writeOptions) {
    hsRAMStream ram;
    rec->WriteStreamHeader(&ram);
    rec->Write(&ram, 0, writeOptions);
    ram.Rewind();

    unsigned bytes = ram.GetEOF();
    uint8_t * buf = nullptr;
    buf = (uint8_t *)malloc(bytes);

    ram.CopyToMem(buf);
    SetSDLData(buf, bytes);
    free(buf);
}

//============================================================================
void VaultSDLNode::InitStateDataRecord (const ST::string& sdlRecName, unsigned writeOptions) {
    {
        plStateDataRecord * rec = new plStateDataRecord;
        bool exists = GetStateDataRecord(rec, 0);
        delete rec;
        if (exists)
            return;
    }

    if (plStateDescriptor * des = plSDLMgr::GetInstance()->FindDescriptor(sdlRecName, plSDL::kLatestVersion)) {
        plStateDataRecord rec(des);
        rec.SetFromDefaults(false);
        SetStateDataRecord(&rec, writeOptions|plSDL::kDontWriteDirtyFlag);
    }
}


/*****************************************************************************
*
*   VaultImageNode
*
***/

//============================================================================
void VaultImageNode::StuffImage (plMipmap * src, int dstType) {
    hsRAMStream ramStream;
    bool compressSuccess = false;

    switch (dstType) {
        case kJPEG:
            plJPEG::Instance().SetWriteQuality(70/*percent*/);
            compressSuccess = plJPEG::Instance().WriteToStream(&ramStream, src);
            break;
        case kPNG:
            compressSuccess = plPNG::Instance().WriteToStream(&ramStream, src);
            break;
        default:
            break;
    }

    if (compressSuccess) {
        unsigned bytes = ramStream.GetEOF();
        uint8_t * buffer = (uint8_t *)malloc(bytes);
        ramStream.CopyToMem(buffer);
        SetImageData(buffer, bytes);
        SetImageType(dstType);
        free(buffer);
    } else {
        SetImageData(nullptr, 0);
        SetImageType(kNone);
    }
}

//============================================================================
bool VaultImageNode::ExtractImage (plMipmap ** dst) {
    hsRAMStream ramStream;
    ramStream.Write(GetImageData().size(), GetImageData().data());
    ramStream.Rewind();

    switch (GetImageType()) {
        case kJPEG:
            (*dst) = plJPEG::Instance().ReadFromStream(&ramStream);
            break;

        case kPNG:
            (*dst) = plPNG::Instance().ReadFromStream(&ramStream);
            break;

        case kNone:
        default:
            (*dst) = nullptr;
            break;
    }
    return ((*dst) != nullptr);
}


/*****************************************************************************
*
*   VaultAgeLinkNode
*
***/

struct MatchesSpawnPointTitle
{
    ST::string fTitle;
    MatchesSpawnPointTitle( const ST::string & title ):fTitle( title ){}
    bool operator ()( const plSpawnPointInfo & p ) const { return ( p.fTitle==fTitle ); }
};
struct MatchesSpawnPointName
{
    ST::string fName;
    MatchesSpawnPointName( const ST::string & name ):fName( name ){}
    bool operator ()( const plSpawnPointInfo & p ) const { return ( p.fSpawnPt==fName ); }
};

//============================================================================
bool VaultAgeLinkNode::CopyTo (plAgeLinkStruct * link) {
    if (hsRef<RelVaultNode> me = VaultGetNode(base->GetNodeId())) {
        if (hsRef<RelVaultNode> info = me->GetChildNode(plVault::kNodeType_AgeInfo, 1)) {
            VaultAgeInfoNode access(info);
            access.CopyTo(link->GetAgeInfo());
            return true;
        }
    }
    link->Clear();
    return false;
}

//============================================================================
void VaultAgeLinkNode::AddSpawnPoint (const plSpawnPointInfo & point) {

    plSpawnPointVec points;
    GetSpawnPoints( &points );
    if ( std::find_if( points.begin(), points.end(), MatchesSpawnPointTitle( point.fTitle ) )!=points.end() )
        return;

    // only check to see if the titles are the same... 
    //... so we can add the same spawnpoint as long as they have different titles
        //if ( std::find_if( points.begin(), points.end(), MatchesSpawnPointName( point.fSpawnPt.c_str() ) )!=points.end() )
        //  return;

    points.push_back( point );
    SetSpawnPoints( points );
}

//============================================================================
void VaultAgeLinkNode::RemoveSpawnPoint (const ST::string & spawnPtName) {

    plSpawnPointVec points;
    GetSpawnPoints( &points );                                                  
    plSpawnPointVec::iterator it = std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) );
    while ( it!=points.end() )
    {
        points.erase( it );
        SetSpawnPoints( points );
        it = std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) );
    }
}

//============================================================================
bool VaultAgeLinkNode::HasSpawnPoint (const ST::string & spawnPtName) const {

    plSpawnPointVec points;
    GetSpawnPoints( &points );                                                  
    return ( std::find_if( points.begin(), points.end(), MatchesSpawnPointName( spawnPtName ) )!=points.end() );
}

//============================================================================
bool VaultAgeLinkNode::HasSpawnPoint (const plSpawnPointInfo & point) const {

    return HasSpawnPoint(point.GetName());
}

//============================================================================
void VaultAgeLinkNode::GetSpawnPoints (plSpawnPointVec * out) const {

    ST::string str = ST::string::from_utf8(reinterpret_cast<const char*>(GetSpawnPoints().data()),
                                           GetSpawnPoints().size());
    std::vector<ST::string> izer = str.tokenize(";");
    for (auto token1 = izer.begin(); token1 != izer.end(); ++token1)
    {
        plSpawnPointInfo point;
        std::vector<ST::string> izer2 = token1->tokenize(":");
        if ( izer2.size() > 0)
            point.fTitle = std::move(izer2[0]);
        if ( izer2.size() > 1)
            point.fSpawnPt = std::move(izer2[1]);
        if ( izer2.size() > 2)
            point.fCameraStack = std::move(izer2[2]);

        out->push_back(point);
    }
}

//============================================================================
void VaultAgeLinkNode::SetSpawnPoints (const plSpawnPointVec & in) {

    ST::string_stream ss;
    for ( unsigned i=0; i<in.size(); i++ ) {
        ss
            << in[i].fTitle << ":"
            << in[i].fSpawnPt << ":"
            << in[i].fCameraStack << ";";
    }
    ST::string blob = ss.to_string();
    SetSpawnPoints(reinterpret_cast<const uint8_t *>(blob.c_str()), blob.size());
}

/*****************************************************************************
*
*   VaultAgeInfoNode
*
***/

//============================================================================
void VaultAgeInfoNode::CopyFrom (const plAgeInfoStruct * info) {
    // age filename
    SetAgeFilename(info->HasAgeFilename() ? info->GetAgeFilename() : "");

    // age instance name
    SetAgeInstanceName(info->HasAgeInstanceName() ? info->GetAgeInstanceName() : "");

    // age user-defined name
    SetAgeUserDefinedName(info->HasAgeUserDefinedName() ? info->GetAgeUserDefinedName() : "");

    // age description
    SetAgeDescription(info->HasAgeDescription() ? info->GetAgeDescription() : "");

    // age sequence number
    SetAgeSequenceNumber(info->GetAgeSequenceNumber());

    // age instance guid
    SetAgeInstanceGuid(*info->GetAgeInstanceGuid());

    // age language
    SetAgeLanguage(info->GetAgeLanguage());
}

//============================================================================
void VaultAgeInfoNode::CopyTo (plAgeInfoStruct * info) const {
    // age filename
    info->SetAgeFilename(GetAgeFilename());

    // age instance name
    info->SetAgeInstanceName(GetAgeInstanceName());

    // age user-defined name
    info->SetAgeUserDefinedName(GetAgeUserDefinedName());

    // age description
    info->SetAgeDescription(GetAgeDescription());

    // age sequence number
    info->SetAgeSequenceNumber(GetAgeSequenceNumber());

    // age instance guid
    plUUID uuid(GetAgeInstanceGuid());
    info->SetAgeInstanceGuid(&uuid);

    // age language
    info->SetAgeLanguage(GetAgeLanguage());
}

//============================================================================
void VaultMarkerGameNode::GetMarkerData(std::vector<VaultMarker>& data) const
{
    if (base->GetBlob_1().size() < sizeof(uint32_t))
        return;

    hsReadOnlyStream stream(base->GetBlob_1().size(), base->GetBlob_1().data());
    uint32_t size = stream.ReadLE32();
    data.reserve(size);

    for (uint32_t i = 0; i < size; ++i) {
        VaultMarker marker;
        marker.id = stream.ReadLE32();
        marker.age = stream.ReadSafeString();
        marker.pos.Read(&stream);
        marker.description = stream.ReadSafeString();
        data.push_back(marker);
    }
}

//============================================================================
void VaultMarkerGameNode::SetMarkerData(const std::vector<VaultMarker>& data)
{
    hsRAMStream stream;
    stream.WriteLE32((uint32_t)data.size());
    for (const VaultMarker& marker : data) {
        stream.WriteLE32(marker.id);
        stream.WriteSafeString(marker.age);
        marker.pos.Write(&stream);
        stream.WriteSafeString(marker.description);
    }

    // copies the buffer
    base->SetBlob_1(reinterpret_cast<const uint8_t*>(stream.GetData()), stream.GetEOF());
}
