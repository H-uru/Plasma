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
#include "plCreatableIndex.h"
#include "plgDispatch.h"

#include "plComponentReg.h"
#include "plMiscComponents.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/plMaxNodeData.h"
#include "MaxMain/MaxAPI.h"

#include "resource.h"

#ifdef MAXASS_AVAILABLE
#   include "../../AssetMan/PublicInterface/MaxAssInterface.h"
#endif

#include "MaxMain/plPlasmaRefMsgs.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"

#include "MaxMain/plPluginResManager.h"

#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"

#include "plScene/plSceneNode.h"
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/hsControlConverter.h"
#include "plInterp/plController.h"

// Follow mod
#include "plInterp/plAnimPath.h"
#include "pfAnimation/plFollowMod.h"

//Player Attention Related
#include "pfCamera/plInterestingModifier.h"

//Player Start Position
#include "plModifier/plSpawnModifier.h"

// RunTime related (Sprites, Billboards, LightMaps, etc., etc.)
#include "pfAnimation/plViewFaceModifier.h" // ViewFace Comp

// Anim Related
#include "plMaxAnimUtils.h"

// CavView component.
#include "plScene/plPostEffectMod.h"

// Location Related
#include "plAgeDescription/plAgeDescription.h"
#include "plFile/plEncryptedStream.h"
#include "MaxMain/plMaxCFGFile.h"
#include "MaxMain/plAgeDescInterface.h"
#include "plResMgr/plPageInfo.h"

#include "plDrawable/plGeometrySpan.h"

#include "MaxConvert/plConvert.h"

// ImageLib
#include "plModifier/plImageLibMod.h"
#include "MaxPlasmaMtls/Layers/plLayerTex.h"
#include "MaxConvert/plLayerConverter.h"
#include "plGImage/plBitmap.h"

// NetSync
#include "pnNetCommon/plSDLTypes.h"
#include "MaxConvert/hsMaterialConverter.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

void DummyCodeIncludeFuncMisc() 
{
    RegisterNotification(plPageInfoComponent::NotifyProc, nullptr, NOTIFY_FILE_POST_OPEN);
    RegisterNotification(plPageInfoComponent::NotifyProc, nullptr, NOTIFY_SYSTEM_POST_RESET);
    RegisterNotification(plPageInfoComponent::NotifyProc, nullptr, NOTIFY_SYSTEM_POST_NEW);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Interesting Component
//
//

//Class that accesses the paramblock below.
class plInterestingComponent : public plComponent
{
public:
    plInterestingComponent();
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary below.
CLASS_DESC(plInterestingComponent, gInterestDesc, "Player Attention",  "PlayerAttention", COMP_TYPE_MISC, Class_ID(0x6f48a7, 0x7ab86088))

enum
{
    kInteresting, kCamInterestRadius, kCamInterestWeight
};

ParamBlockDesc2 gInterestBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
    1, _T("Player Attention"), 0, &gInterestDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_INTEREST, IDS_COMP_INTERESTS, 0, 0, nullptr,

    // params
    kInteresting,       _T("interesting"),      TYPE_STRING,        0, 0,   
        p_end,
    
    kCamInterestRadius, _T("CamInterestRadius"),   TYPE_FLOAT,     P_ANIMATABLE, 0,
        p_default, 100.0f,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, 
        IDC_COMP_INTEREST_EDIT1, IDC_COMP_INTEREST_SPIN1, 1.0f,
        p_end,

    kCamInterestWeight, _T("CamInterestWeight"),   TYPE_FLOAT, P_ANIMATABLE, 0,
        p_range, 0.0, 1.0,
        p_default, 1.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT,
        IDC_COMP_INTEREST_EDIT2,    IDC_COMP_INTEREST_SPIN2, 0.001f,
        p_end,

    p_end
);


plInterestingComponent::plInterestingComponent()
{
    fClassDesc = &gInterestDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plInterestingComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{

    plInterestingModifier* pMod = new plInterestingModifier;
    
    float loader = fCompPB->GetFloat(kCamInterestRadius);
    pMod->SetInterestRadius(loader);
    loader = fCompPB->GetFloat(kCamInterestWeight);
    pMod->SetInterestWeight(loader);

    node->AddModifier(pMod, IGetUniqueName(node));
    return true;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  PageInfo Component
//
//


class plPageInfoComponentProc : public ParamMap2UserDlgProc
{
protected:
    HWND fhDlg;
    IParamBlock2 *fPB;
    
    void ILoadPages()
    {
        HWND hPageCombo = GetDlgItem(fhDlg, IDC_COMP_LOCATION_PAGECOMBO);
        ComboBox_ResetContent(hPageCombo);

        int idx = ComboBox_GetCurSel( GetDlgItem( fhDlg, IDC_COMP_LOCATION_AGECOMBO ) );
        if( idx == CB_ERR )
            return;
        auto agePath = (ST::string*)ComboBox_GetItemData( GetDlgItem( fhDlg, IDC_COMP_LOCATION_AGECOMBO ), idx );
        if (agePath == nullptr || agePath->empty())
            return;

        // Get the age description
        plAgeDescription aged( *agePath );

        // Set the seqPrefix here. (Where else would you suggest?)
        fPB->SetValue( plPageInfoComponent::kInfoSeqPrefix, 0, (int)aged.GetSequencePrefix() );

        const MCHAR* curPage = fPB->GetStr(plPageInfoComponent::kInfoPage);
        if (curPage && *curPage == _M('\0'))
            curPage = nullptr;

        // Load the page combo and select the saved page (if it's in there)
        plAgePage *page;
        aged.SeekFirstPage();
        while ((page = aged.GetNextPage()) != nullptr)
        {
            int idx = ComboBox_AddString(hPageCombo, ST2T(page->GetName()));
            if (curPage && (page->GetName() == curPage))
                ComboBox_SetCurSel(hPageCombo, idx);
            ComboBox_SetItemData( hPageCombo, idx, (int)page->GetSeqSuffix() );
        }
    }

    void    IClearAges( HWND combo )
    {
        while( ComboBox_GetCount( combo ) > 0 )
        {
            delete (ST::string*)ComboBox_GetItemData( combo, 0 );
            ComboBox_DeleteString( combo, 0 );
        }
    }

    bool ILoadAges()
    {
        HWND hAgeCombo = GetDlgItem(fhDlg, IDC_COMP_LOCATION_AGECOMBO);
        IClearAges( hAgeCombo );

        std::vector<plFileName> ageFiles = plAgeDescInterface::BuildAgeFileList();

        const MCHAR* curAge = fPB->GetStr(plPageInfoComponent::kInfoAge);
        if (!curAge || *curAge == _M('\0'))
            curAge = _M("");

        for (const plFileName& ageFile : ageFiles)
        {
            ST::string ageName = ageFile.GetFileNameNoExt();

            int idx = ComboBox_AddString(hAgeCombo, ST2T(ageName));
            // Store the pathas the item data for later (so don't free it yet!)
            ComboBox_SetItemData(hAgeCombo, idx, new ST::string(ageFile.AsString()));

            if (ageName == curAge)
                ComboBox_SetCurSel( hAgeCombo, idx );
        }
        
        return true;
    }

public:
    void DeleteThis() override { }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                fhDlg = hWnd;
                fPB = map->GetParamBlock();
                ILoadAges();
                ILoadPages();
                return TRUE;
            }

        case WM_DESTROY:
            IClearAges( GetDlgItem( hWnd, IDC_COMP_LOCATION_AGECOMBO ) );
            return TRUE;

        case WM_COMMAND:
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_LOCATION_AGECOMBO)
            {
                HWND hAgeCombo = (HWND)lParam;
                int idx = ComboBox_GetCurSel(hAgeCombo);
                if (idx != CB_ERR)
                {
                    TCHAR buf[256];
                    ComboBox_GetText(hAgeCombo, buf, std::size(buf));
                    fPB->SetValue(plPageInfoComponent::kInfoAge, 0, buf);
                    fPB->SetValue(plPageInfoComponent::kInfoPage, 0, _M(""));
                    fPB->SetValue(plPageInfoComponent::kInfoSeqSuffix, 0, (int)-1 );
                    ILoadPages();
                }

                return TRUE;
            }
            if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMP_LOCATION_PAGECOMBO)
            {
                HWND hPageCombo = (HWND)lParam;
                int idx = ComboBox_GetCurSel(hPageCombo);
                if (idx != CB_ERR)
                {
                    TCHAR buf[256];
                    ComboBox_GetText(hPageCombo, buf, std::size(buf));
                    fPB->SetValue( plPageInfoComponent::kInfoPage, 0, buf );
                    fPB->SetValue(plPageInfoComponent::kInfoSeqSuffix, 0, (int)ComboBox_GetItemData(hPageCombo, idx));
                }

                return TRUE;
            }
            return FALSE;
        }

        return FALSE;
    }
};  

//  For the paramblock below.
static plPageInfoComponentProc gPageInfoCompProc;

//Max desc stuff necessary.
CLASS_DESC(plPageInfoComponent, gPageInfoDesc, "Page Info",  "PageInfo", COMP_TYPE_MISC, PAGEINFO_CID)

//Max paramblock2 stuff below.
ParamBlockDesc2 gPageInfoCompBk
(   
    plComponent::kBlkComp, _T("PageInfo"), 0, &gPageInfoDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_PAGEINFO, IDS_COMP_PAGEINFOS, 0,   0,  &gPageInfoCompProc,

    plPageInfoComponent::kInfoAge,  _T("ageName"),  TYPE_STRING,        0,  0,  
        p_end,

    plPageInfoComponent::kInfoPage, _T("pageName"), TYPE_STRING,        0,  0,  
        p_end,

    plPageInfoComponent::kInfoSeqPrefix, _T("sequencePrefix"), TYPE_INT, 0, 0,
        p_end,

    plPageInfoComponent::kInfoSeqSuffix, _T("sequenceSuffix"), TYPE_INT, 0, 0,
        p_end,

    plPageInfoComponent::kRefVolatile_PageInfoUpdated, _T( "pageInfoUpdated" ), TYPE_BOOL, 0, 0,
        p_default, 0,
        p_end,
        
    plPageInfoComponent::kItinerant,    _T("itinerant"), TYPE_BOOL, 0, 0,
        p_ui, TYPE_SINGLECHEKBOX, IDC_CHECK1,
        p_end,

    p_end
);

TCHAR    plPageInfoComponent::fCurrExportedAge[ 256 ] = _T("");

plPageInfoComponent::plPageInfoComponent()
{
    fSeqNumValidated = false;
    fItinerant = false;
    fClassDesc = &gPageInfoDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plPageInfoComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    // Another component already created a location, don't override it
    if (pNode->GetRoomKey())
        return false;

    const MCHAR* age = fCompPB->GetStr(kInfoAge);
    const MCHAR* room = fCompPB->GetStr(kInfoPage);

    if (!age || *age == '\0' || !room || *room == '\0')
    {
        pErrMsg->Set(true,
                     "PageInfo Component Error",
                     ST::format("No Label for the Age, Chapter, or Page, on the Location component found on {}",
                                pNode->GetName())
                     ).Show();
        return false;   
    }

    // If we're only exporting a certain page, and this location isn't it, don't export this node
    const TCHAR* exportPage = plConvert::Instance().GetConvertSettings()->fExportPage;
    if (exportPage && _tcsicmp(room, exportPage))
    {
        pNode->SetCanConvert(false);
        return true;
    }

    // Check to make sure we don't try to export more than one age at a time
    if( *fCurrExportedAge == _T('\0') )
        _tcsncpy(fCurrExportedAge, age, std::size(fCurrExportedAge));
    else
    {
        if( _tcsicmp( fCurrExportedAge, age ) != 0 )
        {
            // Our only currently accepted exception (eh?) is GlobalClothing and GlobalAvatars
            if( (_tcsicmp( age, _T("GlobalAvatars") ) == 0 && _tcsicmp( fCurrExportedAge, _T("GlobalClothing") ) == 0 ) ||
                (_tcsicmp( age, _T("GlobalClothing") ) == 0 && _tcsicmp( fCurrExportedAge, _T("GlobalAvatars") ) == 0 ) )
            {
            }
            else
            {
                pErrMsg->Set( true, "PageInfo Component Error", 
                              ST::format( "The scene you are trying to export is attempting to export to both ages {} and"
                                          " {}. You are only allowed to export to one age at a time.",
                                          fCurrExportedAge, age )
                            ).Show();

                // Reset for next time
                return false;
            }
        }
    }

    // Make sure our sequence partitions are up-to-date
    IUpdateSeqNumbersFromAgeFile( pErrMsg );

    // Need to re-get our age and page name here, since IUpdate() might change them
    age = fCompPB->GetStr( kInfoAge );
    room = fCompPB->GetStr( kInfoPage );
    fItinerant = fCompPB->GetInt(kItinerant);

    // Build our sequence number
    int32_t newNum, seqNum;
    seqNum = plPageInfoUtils::CombineSeqNum( fCompPB->GetInt( kInfoSeqPrefix ), fCompPB->GetInt( kInfoSeqSuffix ) );
    newNum = plPluginResManager::ResMgr()->VerifySeqNumber( seqNum, age, room );
    if( newNum != seqNum )
    {
        if( !fSeqNumValidated && seqNum != 0 )
        {
            // What error was it, exactly?
            ST::string errMsg;
            const plPageInfo *lastPage = plPluginResManager::ResMgr()->GetLastVerifyPage();

            if( plPluginResManager::ResMgr()->GetLastVerifyError() == plPluginResManager::kErrRightPageWrongSeq )
            {
                errMsg = ST::format(
                    "The Page Info component for {}>{} applied to object {} is attempting to export with a sequence number "
                    "different from what is already exported. Further, the already-exported page {}>{} has "
                    "the same sequence number as this Page Info component now. This is most likely due to the .age "
                    "files having been changed since the old page was exported. The recommended solution would be to "
                    "delete the offending page data and export both pages again.\n\n"
                    "The exporter has assigned a valid temporary sequence number for page {}>{}, but this data should not be used "
                    "for playing over the network or released for external use.\n\n"
                    "\t(Original sequence #: 0x{X})\n\t(Temporary sequence #: 0x{X})",
                    age, room, pNode->GetName(),
                    lastPage->GetAge(), lastPage->GetPage(),
                    age, room, seqNum, newNum);
            }
            else if( plPluginResManager::ResMgr()->GetLastVerifyError() == plPluginResManager::kErrSeqAlreadyTaken )
            {
                errMsg = ST::format(
                    "The Page Info component for {}>{} applied to object {} is attempting to export with "
                    "an identical sequence number to the already-exported page {}>{}. This is usually due to "
                    "either page having been exported with an invalid or missing .age file. Please verify that both "
                    "pages have valid .age files and their sequence numbers do not conflict in the Age Description "
                    "Manager.\n\n"
                    "The exporter has assigned a valid temporary sequence number for page {}>{}, but this data should not be used "
                    "for playing over the network or released for external use.\n\n"
                    "\t(Original sequence #: 0x{X})\n\t(Temporary sequence #: 0x{X})",
                    age, room, pNode->GetName(),
                    lastPage->GetAge(), lastPage->GetPage(),
                    age, room, seqNum, newNum);
            }
            else if( plPluginResManager::ResMgr()->GetLastVerifyError() == plPluginResManager::kErrCantFindValid )
            {
                errMsg = ST::format(
                    "The Page Info component for {}>{} applied to object {} is attempting to export with "
                    "an invalid sequence number. The exporter could not find a valid, free sequence number to use, so this "
                    "page cannot be exported. Contact mcn (ext 264) immediately!\n\n"
                    "\t(Original sequence #: 0x{X})", 
                    age, room, pNode->GetName(), seqNum );
                pErrMsg->Set( true, "PageInfo Convert Error", errMsg ).Show(); 
                return false;
            }
            else
            {
                errMsg = ST::format(
                    "The Page Info component for {}>{} applied to object {} is attempting to export with "
                    "a sequence number that is invalid for an unknown reason.\n\n"
                    "The exporter has assigned a valid temporary sequence number, but this data should not be used "
                    "for playing over the network or released for external use.\n\n"
                    "\t(Original sequence #: 0x{X})\n\t(Temporary sequence #: 0x{X})",
                    age, room, pNode->GetName(), seqNum, newNum);
            }
            pErrMsg->Set( true, "PageInfo Convert Error", errMsg ).Show(); 
            pErrMsg->Set( false );
            fSeqNumValidated = true;
        }
        seqNum = newNum;
    }
    if (fItinerant)
        int i = 0;
    plKey roomKey = plPluginResManager::ResMgr()->NameToLoc(age, room, seqNum, fItinerant );

    if(!roomKey)
    {
        pErrMsg->Set(true,
            "PageInfo Convert Error",
            ST::format(
                "Location Component {} has a Missing Location.  Nuke the files in the dat directory and re-export.",
                pNode->GetName())
            ).Show();
        return false;
    }
    pNode->SetRoomKey(roomKey);
    

    if (!_tcscmp(age, _T("GlobalClothing")))
        ((plSceneNode *)roomKey->GetObjectPtr())->SetFilterGenericsOnly(true);
    
    return true;
}

bool plPageInfoComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    // Make sure we clear this flag so that the next time around it's clear
    fCompPB->SetValue( kRefVolatile_PageInfoUpdated, 0, (int)false );
    fSeqNumValidated = false;
        
    return true;
}

bool plPageInfoComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plKey snKey = node->GetRoomKey();
    plSceneObject *so = node->GetSceneObject();
    if(so)
    {
        if (node->GetSwappableGeom())
        {
            snKey->Release(so->GetKey());
            node->GetMaxNodeData()->SetSceneObject(nullptr);

            // Since child refs are now passive, this isn't needed.
            /*
            plMaxNode *parent = (plMaxNode *)node->GetParentNode();
            if (!parent->IsRootNode() && !parent->GetSwappableGeom())
            {
                parent->GetSceneObject()->GetKey()->Release(so->GetKey());
            }
            */
        }
    }

    return true;
}

const MCHAR* plPageInfoComponent::GetAgeName()
{
    return fCompPB->GetStr(ParamID(kInfoAge));
}

//// IVerifyLatestAgeAsset ///////////////////////////////////////////////////
//  Checks in assetMan to make sure we have the latest .age file to export 
//  with.

void    plPageInfoComponent::IVerifyLatestAgeAsset( const ST::string &ageName, const plFileName &localPath, plErrorMsg *errMsg )
{
#ifdef MAXASS_AVAILABLE
    plFileName ageFileName, assetPath;

   MaxAssInterface *assetMan = GetMaxAssInterface();
   if (assetMan == nullptr)
       return;      // No AssetMan available

    // Try to find it in assetMan
    ageFileName = ageName + ".age";
    jvUniqueId assetId;
    if (assetMan->FindAssetByFilename(ageFileName.AsString().c_str(), assetId))
    {
        // Get the latest version
        if (!assetMan->GetLatestVersionFile(assetId, assetPath, sizeof(assetPath)))
        {
            errMsg->Set( true, "PageInfo Convert Error",
                ST::format(
                    "Unable to update age file for '{}' because AssetMan was unable to get the latest version. Using local copy instead.",
                    ageName )
                ).Show();
            errMsg->Set( false );
            return;
        }

        // Got the latest version, just copy over and roll!
        plFileSystem::Unlink(localPath);
        plFileSystem::Copy(assetPath, localPath);
    }
    else
    {
        // Not found, so just assume it's a local one (no error)
    }
#endif
}

//// IUpdateSeqNumbersFromAgeFile ////////////////////////////////////////////
//  With the new sequence numbers, it's vital that our sequence numbers that
//  we use to export are synched up with the latest .age files. This function
//  makes sure that we're synched before we start using 'em.

void    plPageInfoComponent::IUpdateSeqNumbersFromAgeFile( plErrorMsg *errMsg )
{
    // Check to see if we've updated already
    if( fCompPB->GetInt( kRefVolatile_PageInfoUpdated ) )
        return; // Already updated this pass!

    // Mark us as updated
    fCompPB->SetValue( kRefVolatile_PageInfoUpdated, 0, (int)true );

    plFileName ageFolder = plPageInfoUtils::GetAgeFolder();
    if (!ageFolder.IsValid())
    {
        errMsg->Set( true,
                     "PageInfo Convert Error",
                     ST::format( "There was a problem converting the PageInfo Component {} (the age folder couldn't be located). "
                                 "The exporter will assign a temporary sequence number to this page, but you'll be lucky if it works at all.",
                                 GetINode()->GetName() )
                    ).Show();
        errMsg->Set( false );
        fCompPB->SetValue( kInfoSeqPrefix, 0, 0 );
        fCompPB->SetValue( kInfoSeqSuffix, 0, 0 );
        return;
    }
    const MCHAR* curAge = fCompPB->GetStr( kInfoAge );
    if( !curAge || *curAge == '\0' )
    {
        errMsg->Set( true,
                     "PageInfo Convert Error",
                     ST::format( "There was a problem converting the PageInfo Component {} (no age name was selected). "
                                 "The exporter will assign a temporary sequence number to this page, but you'll be lucky if it works at all.",
                                 GetINode()->GetName())
                   ).Show();
        errMsg->Set( false );
        fCompPB->SetValue( kInfoSeqPrefix, 0, 0 );
        fCompPB->SetValue( kInfoSeqSuffix, 0, 0 );
        return;
    }
    plFileName path = plFileName::Join(ageFolder, ST::format("{}.age", curAge));

    IVerifyLatestAgeAsset( curAge, path, errMsg );
    std::unique_ptr<plAgeDescription> aged(plPageInfoUtils::GetAgeDesc(curAge));

    if (!aged)
    {
        errMsg->Set( true,
                     "PageInfo Convert Error",
                     ST::format( "There was a problem converting the PageInfo Component {} (the age name \"{}\" is invalid). "
                                 "The exporter will assign a temporary sequence number to this page, but you'll be lucky if it works at all.",
                                 GetINode()->GetName(), curAge )
                    ).Show();
        errMsg->Set( false );
        fCompPB->SetValue( kInfoSeqPrefix, 0, 0 );
        fCompPB->SetValue( kInfoSeqSuffix, 0, 0 );
        return;
    }

    // Update based on the age file now
    fCompPB->SetValue( kInfoSeqPrefix, 0, (int)aged->GetSequencePrefix() );

    // Find our page
    const MCHAR* compPBPageName = fCompPB->GetStr( kInfoPage );
    if (compPBPageName == nullptr)
    {
        errMsg->Set( true,
                     "PageInfo Convert Error",
                     ST::format( "There was a problem converting the PageInfo Component {} (no page name was specified). "
                                 "The exporter will assign a temporary sequence number to this page, but you'll be lucky if it works at all.",
                                 GetINode()->GetName() )
                    ).Show();
        errMsg->Set( false );
        fCompPB->SetValue( kInfoSeqPrefix, 0, 0 );
        fCompPB->SetValue( kInfoSeqSuffix, 0, 0 );
        return;
    }

    plAgePage   *page;
    aged->SeekFirstPage();

    while ((page = aged->GetNextPage()) != nullptr)
    {
        if( page->GetName().compare_i( compPBPageName ) == 0 )
        {
            fCompPB->SetValue( kInfoSeqSuffix, 0, (int)page->GetSeqSuffix() );

            // Also re-copy the page name, just to make sure the case is correct
            fCompPB->SetValue( kInfoPage, 0, ST2M(page->GetName()) );
            return;
        }
    }

    // If we got here, the page name is invalid
    ST::string msg = ST::format(
        "There was a problem converting the PageInfo Component {} (the page \"{}\" wasn't found in the age file for age \"{}\"). "
        "The exporter will assign a temporary sequence number to this page, but you'll be lucky if it works at all.",
        GetINode()->GetName(), compPBPageName, curAge);
    errMsg->Set( true, "PageInfo Convert Error", msg ).Show();
    errMsg->Set( false );
    fCompPB->SetValue( kInfoSeqPrefix, 0, 0 );
    fCompPB->SetValue( kInfoSeqSuffix, 0, 0 );
}

plFileName plPageInfoUtils::GetAgeFolder()
{
    static plFileName ageFolder;

    if (!ageFolder.IsValid())
    {
        plFileName plasmaPath = plMaxConfig::GetClientPath();
        if (!plasmaPath.IsValid())
            return "";

        ageFolder = plFileName::Join(plasmaPath, plAgeDescription::kAgeDescPath);
    }

    return ageFolder;
}

int32_t   plPageInfoUtils::CombineSeqNum( int prefix, int suffix )
{
    hsAssert(abs(prefix) < 0xFF, "Sequence prefix must be less then the max 8-bit number");
    hsAssert(suffix <= 0xFFFF, "Sequence suffix must be less then the max 16-bit number");
    hsAssert(suffix >= 0, "Sequence suffix must be unsigned");
    if( prefix < 0 )
        return -( ( ( -prefix ) << 16 ) + suffix );
    else
        return ( prefix << 16 ) + suffix;
}

int32_t   plPageInfoUtils::GetCommonSeqNumFromNormal( int32_t normalSeqNumber, int whichCommonPage )
{
    int     prefix;
    const int kFirstCommonSeqSuffix = 0xffff;

    hsAssert( whichCommonPage < plAgeDescription::kNumCommonPages, "Invalid common page index in GetCommonSeqNumFromNormal()" );

    if( normalSeqNumber < 0 )
    {
        prefix = -( (-normalSeqNumber) >> 16 );
    }
    else
        prefix = normalSeqNumber >> 16;
    
    return CombineSeqNum( prefix, kFirstCommonSeqSuffix - whichCommonPage );
}

int32_t   plPageInfoUtils::GetSeqNumFromAgeDesc( const ST::string& ageName, const ST::string& pageName )
{
    int             seqPrefix, seqSuffix = 0;
    plAgeDescription *aged = GetAgeDesc( ageName );
    if (aged == nullptr)
    {
        // ???? This ain't good...attempt to get the resMgr to give us a temporary seqNum...
        return 0;
    }

    seqPrefix = aged->GetSequencePrefix();

    // Find our page
    plAgePage *page;
    aged->SeekFirstPage();
    while ((page = aged->GetNextPage()) != nullptr)
    {
        if (page->GetName().compare_i(pageName) == 0)
        {
            seqSuffix = page->GetSeqSuffix();
            break;
        }
    }

    delete aged;

    return CombineSeqNum( seqPrefix, seqSuffix );
}

plAgeDescription *plPageInfoUtils::GetAgeDesc( const ST::string &ageName )
{
    plFileName ageFolder = plPageInfoUtils::GetAgeFolder();
    if (!ageFolder.IsValid() || ageName.empty())
        return nullptr;

    plAgeDescription* aged = new plAgeDescription;
    if (aged->ReadFromFile(plFileName::Join(ageFolder, ageName + ".age")))
        return aged;
    else
    {
        delete aged;
        return nullptr;
    }
}

const MCHAR* LocCompGetPage(plComponentBase* comp)
{
    if (!comp)
        return nullptr;

    const MCHAR* page = nullptr;
    
    if (comp->ClassID() == PAGEINFO_CID)
    {
        IParamBlock2* pb = comp->GetParamBlockByID(plComponentBase::kBlkComp);
        page = pb->GetStr(plPageInfoComponent::kInfoPage);
    }
    
    if (page && *page != _M('\0'))
        return page;
    
    return nullptr;
}

static const TCHAR* CheckPageInfoCompsRecur(plMaxNode *node)
{
    plComponentBase *comp = node->ConvertToComponent();
    if (comp && comp->ClassID() == PAGEINFO_CID)
    {
        plPageInfoComponent* pageComp = (plPageInfoComponent*)comp;
        return pageComp->GetAgeName();
    }
    
    for (int i = 0; i < node->NumberOfChildren(); i++)
    {
        auto result = CheckPageInfoCompsRecur((plMaxNode*)node->GetChildNode(i));
        if (result)
            return result;
    }
    return nullptr;
}

void plPageInfoComponent::NotifyProc(void *param, NotifyInfo *info)
{
    if (info->intcode == NOTIFY_FILE_POST_OPEN)
    {
        const TCHAR* ageName = CheckPageInfoCompsRecur((plMaxNode*)GetCOREInterface()->GetRootNode());
        if (ageName != nullptr)
            _tcsncpy(fCurrExportedAge, ageName, std::size(fCurrExportedAge));
    }
    else if (info->intcode == NOTIFY_SYSTEM_POST_RESET ||
        info->intcode == NOTIFY_SYSTEM_POST_NEW)
    {
        fCurrExportedAge[0] = 0;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Room Component
//
//

class plRoomComponent : public plComponent
{
public:
    plRoomComponent();

    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
};

//Max desc stuff necessary.
OBSOLETE_CLASS_DESC(plRoomComponent, gRoomDesc, "Location",  "Location", COMP_TYPE_MISC, ROOM_CID)

enum
{
    kLocRoom,
    kLocAge,
    kLocDistrict,
    kLocSeqNumber
};

//Max paramblock2 stuff below.
ParamBlockDesc2 gRoomCompBk
(   
    1, _T("Location"), 0, &gRoomDesc, P_AUTO_CONSTRUCT+ P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_ROOM, IDS_COMP_ROOMS, 0,   0,  nullptr,

    // params
    kLocAge,            _T("Age"),      TYPE_STRING,        0, 0,
        p_ui,   TYPE_EDITBOX, IDC_COMP_ROOM_AGE_TEXTBOX,
        p_end,

    kLocDistrict,       _T("District"),     TYPE_STRING,        0, 0,   
        p_ui,   TYPE_EDITBOX,   IDC_COMP_ROOM_DISTRICT_TEXTBOX,
        p_end,

    kLocRoom,           _T("Room"),     TYPE_STRING,        0, 0,   
        p_ui,   TYPE_EDITBOX,   IDC_COMP_ROOM_ROOM_TEXTBOX,
        p_end,

    kLocSeqNumber,      _T("seqNumber"), TYPE_INT, 0, 0,
        p_ui,   TYPE_SPINNER, EDITTYPE_POS_INT, IDC_ROOM_SEQEDIT, IDC_ROOM_SEQSPIN, SPIN_AUTOSCALE,
        p_end,

    p_end
);

plRoomComponent::plRoomComponent()
{
    fClassDesc = &gRoomDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plRoomComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  View Facing Component
//
//

//Class that accesses the paramblock below.
class plViewFacingComponent : public plComponent
{
public:
    plViewFacingComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

//Max desc stuff necessary below.
CLASS_DESC(plViewFacingComponent, gViewFacingDesc, "Billboard",  "Billboard", COMP_TYPE_GRAPHICS, Class_ID(0x7fab4d1f, 0x30f95438))

//
// Block not necessary, kept for backwards compat.
//
enum
{
    kTypeofView, kViewFaceScaleX, kViewFaceScaleY, kViewFaceScaleZ
};

ParamBlockDesc2 gViewFacingBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
    1, _T("View Facing"), 0, &gViewFacingDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,
#if 0
    IDD_COMP_VIEWFACE, IDS_COMP_VIEWFACES, 0, 0, nullptr,

    kTypeofView,    _T("ViewType"),     TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 4,  IDC_RADIO_VF1,  IDC_RADIO_VF2,  IDC_RADIO_VF3,  IDC_RADIO_VF4, 
        end,

    kViewFaceScaleX, _T("ViewFaceScaleX"),      TYPE_FLOAT, 0, 0,
        p_default, 0.0f,
        p_range, 0.0, 1500.0,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT, 
        IDC_COMP_VFSCALE_EDIT1, IDC_COMP_VFSCALE_SPIN1, 0.1f,
        end,

    kViewFaceScaleY, _T("ViewFaceScaleY"),      TYPE_FLOAT, 0, 0,
        p_default, 0.0f,
        p_range, 0.0, 1500.0,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT, 
        IDC_COMP_VFSCALE_EDIT2, IDC_COMP_VFSCALE_SPIN2, 0.1f,
        end,

    kViewFaceScaleZ, _T("ViewFaceScaleZ"),      TYPE_FLOAT, 0, 0,
        p_default, 0.0f,
        p_range, 0.0, 1500.0,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT, 
        IDC_COMP_VFSCALE_EDIT3, IDC_COMP_VFSCALE_SPIN3, 0.1f,
        end,
#endif
    p_end
);






plViewFacingComponent::plViewFacingComponent()
{
    fClassDesc = &gViewFacingDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

static bool NodeHasTMAnimation(plMaxNode* node)
{
    if( node->GetUnBounded() )
        return true;

    plPhysicalProps* props = node->GetPhysicalProps();
    if( props && props->IsUsed() )
    {
        if( (props->GetMass() > 0) && !props->GetPinned() )
            return true;
    }

    return node->GetTMController() && node->GetTMController()->IsAnimated();
}

static bool FindTMAnimatedChildrenRecur(plMaxNode* node)
{
    if( !node->CanConvert() )
        return false;

    if( NodeHasTMAnimation(node) )
        return true;

    int i;
    for( i = 0; i < node->NumChildren(); i++ )
    {
        if( FindTMAnimatedChildrenRecur((plMaxNode*)node->GetChildNode(i)) )
            return true;
    }
    return false;
}

static void FindRecursiveBounds(plMaxNode* node, hsBounds3Ext& bnd)
{
    if( !node->CanConvert() )
        return;

    int i;
    for( i = 0; i < node->NumChildren(); i++ )
    {
        FindRecursiveBounds((plMaxNode*)node->GetChildNode(i), bnd);
    }

    const TimeValue currTime(0);

    Object *obj = node->EvalWorldState(currTime).obj;
    if( !obj )
        return;

    if( obj->CanConvertToType(triObjectClassID) )
    {
        TriObject   *meshObj = (TriObject *)obj->ConvertToType(currTime, triObjectClassID);
        if( !meshObj )
            return;

        Matrix3 l2w = node->GetObjectTM(currTime);
        Box3 box = meshObj->mesh.getBoundingBox(&l2w);

        if( !box.IsEmpty() )
        {
            bnd.Union(&hsPoint3(box.Min().x, box.Min().y, box.Min().z));
            bnd.Union(&hsPoint3(box.Max().x, box.Max().y, box.Max().z));
        }

        if( meshObj != obj )
            meshObj->DeleteThis();
    }
    return;
}

static bool FindMaxBounds(plMaxNode* node, hsBounds3Ext& bnd)
{
    bnd.MakeEmpty();

    // First, look from the node up to the root. If anything is animated, we can't do this.
    plMaxNode* parent = node;
    while( !parent->IsRootNode() )
    {
        // We shouldn't ever hit this, but whatever.
        if( !parent->CanConvert() )
            return false;

        if( NodeHasTMAnimation(parent) )
            return false;
        parent = (plMaxNode*)parent->GetParentNode();
    }

    // Second, look down the children. If any of them are animated, we can't do this.
    if( FindTMAnimatedChildrenRecur(node) )
        return false;

    // Now find the recursive world space bounds of us and all our children.
    FindRecursiveBounds(node, bnd);

    // Translate to local about our pivot
    hsMatrix44 l2w = node->GetLocalToWorld44();

    // Expand them to be symmetric about local origin.
    bnd.MakeSymmetric(&l2w.GetTranslate());

    return true;
}

bool plViewFacingComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{

    plViewFaceModifier* pMod = new plViewFaceModifier;

    hsBounds3Ext maxBnd;
    if( FindMaxBounds(node, maxBnd) )
        pMod->SetMaxBounds(maxBnd);
    
//  int ChosenType = fCompPB->GetInt(kTypeofView);
//  switch(ChosenType)
//  {
//  case 0:
//      pMod->SetFlag(plViewFaceModifier::kPivotFace);
//      break;
//  case 1:
//      pMod->SetFlag(plViewFaceModifier::kPivotFavorY);
//      break;
//  case 2:
        pMod->SetFlag(plViewFaceModifier::kPivotY);
//      break;
//  case 3:
//      pMod->SetFlag(plViewFaceModifier::kPivotTumble);
//      break;
//  }

#if 0
    if(fCompPB->GetFloat(kViewFaceScaleX) || fCompPB->GetFloat(kViewFaceScaleY) || fCompPB->GetFloat(kViewFaceScaleZ))
    {
        pMod->SetFlag(plViewFaceModifier::kScale);
        
        hsVector3 scale(1.f, 1.f, 1.f);
        scale.fX = fCompPB->GetFloat(kViewFaceScaleX);
        scale.fY = fCompPB->GetFloat(kViewFaceScaleY);
        scale.fZ = fCompPB->GetFloat(kViewFaceScaleZ);
        pMod->SetScale(scale);
    }
#endif
    pMod->SetOrigTransform(node->GetLocalToParent44(), node->GetParentToLocal44());
    node->AddModifier(pMod, IGetUniqueName(node));
    return true;
}

bool plViewFacingComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    pNode->SetForceLocal(true);
    pNode->SetMovable(true);
    return true;
}






/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Sprite Component
//
//

//Class that accesses the paramblock below.
class plSpriteComponent : public plComponent
{
public:
    plSpriteComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

//Max desc stuff necessary below.
CLASS_DESC(plSpriteComponent, gSpriteDesc, "Sprite",  "Sprite", COMP_TYPE_GRAPHICS, Class_ID(0x1e18192b, 0x312f579b))

//
// Block not necessary, kept for backwards compat.
//
ParamBlockDesc2 gSpriteBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
    1, _T("Sprite"), 0, &gSpriteDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

    p_end
);

plSpriteComponent::plSpriteComponent()
{
    fClassDesc = &gSpriteDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plSpriteComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plViewFaceModifier* pMod = new plViewFaceModifier;

    hsBounds3Ext maxBnd;
    if( FindMaxBounds(node, maxBnd) )
        pMod->SetMaxBounds(maxBnd);

    pMod->SetFlag(plViewFaceModifier::kPivotFace);
    pMod->SetOrigTransform(node->GetLocalToParent44(), node->GetParentToLocal44());
    node->AddModifier(pMod, IGetUniqueName(node));
    return true;
}

bool plSpriteComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    pNode->SetForceLocal(true);
    pNode->SetMovable(true);
    return true;
}





/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Occlusion Component
//
//


enum    {
            kOccTwoSidedChekbox
        };


class plOcclusionComponentProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            return TRUE;

//////////////////
        case WM_COMMAND:
            {
                if (LOWORD(wParam) == IDC_COMP_OCCLUSION_CKBX)
                {
                    return TRUE;
                }
            }
            
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plOcclusionComponentProc gOccProc;

//Class that accesses the paramblock below.
class plOcclusionComponent : public plComponent
{
public:
    plOcclusionComponent();

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    void CollectNonDrawables(INodeTab& nonDrawables) override { AddTargetsToList(nonDrawables); }
};

CLASS_DESC(plOcclusionComponent, gOcclusionDesc, "Occlusion",  "Occlusion", COMP_TYPE_GRAPHICS, Class_ID(0x18c454df, 0x1ecd40f5))

ParamBlockDesc2 gOcclusionBk
(
    plComponent::kBlkComp, _T("Occlusion"), 0, &gOcclusionDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_OCCLUSION, IDS_COMP_OCCLUSIONS,  0, 0, &gOccProc,

    kOccTwoSidedChekbox, _T("TwoSided"), TYPE_BOOL, 0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_OCCLUSION_CKBX,
        p_end,

    p_end
);

plOcclusionComponent::plOcclusionComponent()
{
    fClassDesc = &gOcclusionDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plOcclusionComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

bool plOcclusionComponent::PreConvert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    bool twoSided = fCompPB->GetInt(kOccTwoSidedChekbox);
    bool isHole = false;
    return node->ConvertToOccluder(pErrMsg, twoSided, isHole);
}

bool plOcclusionComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    pNode->SetDrawable(false);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  CamView Component
//
//

//Class that accesses the paramblock below.
class plCamViewComponent : public plComponent
{
    bool            fBogus;

    void            IMakeEveryoneOpaqueRecur(plMaxNode* node);
    void            IMakeEveryoneOpaque(plMaxNode* node);
public:
    plCamViewComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

//Max desc stuff necessary below.
CLASS_DESC(plCamViewComponent, gCamViewDesc, "Camera View",  "CamView", COMP_TYPE_GRAPHICS, Class_ID(0x5e9f0243, 0xe7f2c08))

enum
{
    kWhateverCheckBox
};

ParamBlockDesc2 gCamViewBk
(
    plComponent::kBlkComp, _T("CamView"), 0, &gCamViewDesc, P_AUTO_CONSTRUCT/* + P_AUTO_UI*/, plComponent::kRefComp,

//  IDD_COMP_CAMVIEW, IDS_COMP_CAMVIEWS, 0, 0, nullptr,

    p_end
);

plCamViewComponent::plCamViewComponent()
:   fBogus(false)
{
    fClassDesc = &gCamViewDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plCamViewComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    TimeValue timeVal(0);

    Object* obj = node->EvalWorldState(timeVal).obj;

    GenCamera* cam = nullptr;
    if( obj->CanConvertToType(Class_ID(LOOKAT_CAM_CLASS_ID, 0)) )
        cam = (GenCamera *) obj->ConvertToType(timeVal, Class_ID(LOOKAT_CAM_CLASS_ID, 0));
    else 
    if( obj->CanConvertToType(Class_ID(SIMPLE_CAM_CLASS_ID, 0)) ) 
        cam = (GenCamera *) obj->ConvertToType(timeVal, Class_ID(SIMPLE_CAM_CLASS_ID, 0));

    if( !cam )
    {
        hsAssert(false, "Should have checked for the camera in PreConvert");
        return false;
    }

    plPostEffectMod* mod = new plPostEffectMod;

    float hither = cam->GetEnvRange(timeVal, ENV_NEAR_RANGE); 
    if( hither < 0.5f )
        hither = 0.5f;
    float yon = cam->GetEnvRange(timeVal, ENV_FAR_RANGE);
    mod->SetHither(hither);
    mod->SetYon(yon);

    // radians
    float fov = cam->GetFOV(timeVal);
    // convert
    int FOVType = cam->GetFOVType();
    float fovX, fovY;
    switch(FOVType)
    {
    case 0: // FOV_W
        {
            fovX = fov;
            fovY = fovX *3.f / 4.f;
        }
        break;
    case 1: // FOV_H
        {
            fovY = fov;
            fovX = fovY * 4.f / 3.f;
        }
        break;
    }
    fovX = hsRadiansToDegrees(fovX);
    fovY = hsRadiansToDegrees(fovY);
    mod->SetFovX(fovX);
    mod->SetFovY(fovY);

    plKey sceneNodeKey = node->GetRoomKey();

    mod->SetNodeKey(sceneNodeKey);

    node->AddModifier(mod, IGetUniqueName(node));

    return true;
}

bool plCamViewComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    TimeValue timeVal(0);
    Object* obj = node->EvalWorldState(timeVal).obj;

    if( obj->CanConvertToType(Class_ID(LOOKAT_CAM_CLASS_ID, 0))
        || obj->CanConvertToType(Class_ID(SIMPLE_CAM_CLASS_ID, 0)) )
        fBogus = false;
    else
        fBogus = true;

    if( fBogus )
        pErrMsg->Set(true, node->GetName(), "CamView component attached to non-camera").CheckAndAsk();

    if( !fBogus )
        IMakeEveryoneOpaque(node);

    return true;
}

void plCamViewComponent::IMakeEveryoneOpaque(plMaxNode* node)
{
    plMaxNode* root = (plMaxNode *)node->GetInterface()->GetRootNode();

    int i;
    for( i = 0; i < root->NumberOfChildren(); i++ )
        IMakeEveryoneOpaqueRecur((plMaxNode*)(root->GetChildNode(i)));

}

void plCamViewComponent::IMakeEveryoneOpaqueRecur(plMaxNode* node)
{
    if( node->CanConvert() )
    {
        node->SetNoSpanReSort(true);
        node->SetNoSpanSort(true);

        int i;
        for( i = 0; i < node->NumberOfChildren(); i++ )
        {
            IMakeEveryoneOpaqueRecur((plMaxNode *)(node->GetChildNode(i)));
        }
    }
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plCamViewComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    node->SetForceLocal(true);
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Follow Component
//
//

enum    
{
    kAffectX,
    kLeaderTypeRadio,
    kLeaderObjectSel,
    kAffectY,
    kAffectZ,
    kAffectRotate
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plLeaderObjAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if( (id == kLeaderObjectSel) )
        {
            plComponentBase *comp = (plComponentBase*)owner;
            comp->NotifyDependents(FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED);
        }
    }
};
plLeaderObjAccessor gLeaderObjAccessor;

class plFollowComponentProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                    IParamBlock2 *pb = map->GetParamBlock();
                    map->SetTooltip(kLeaderObjectSel, TRUE, _M("Press the button, & select the object to follow in one of the Viewports"));
                    if( pb->GetInt(kLeaderTypeRadio) == int32_t(plFollowMod::kObject) )
                        map->Enable(kLeaderObjectSel, TRUE);
                    else
                        map->Enable(kLeaderObjectSel, FALSE);
            }
            return true;

//////////////////
        case WM_COMMAND:
            {
                if( (LOWORD(wParam) == IDC_F_RADIO_PLAYER)
                    ||(LOWORD(wParam) == IDC_F_RADIO_LISTENER)
                    || (LOWORD(wParam) == IDC_F_RADIO_CAMERA)
                    || (LOWORD(wParam) == IDC_F_RADIO_OBJECT) )
                {
                    IParamBlock2 *pb = map->GetParamBlock();
                    if( pb->GetInt(kLeaderTypeRadio) == int32_t(plFollowMod::kObject) )
                        map->Enable(kLeaderObjectSel, TRUE);
                    else
                        map->Enable(kLeaderObjectSel, FALSE);
                    
                    return TRUE;
                }
            }
            
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plFollowComponentProc gFollowProc;

//Class that accesses the paramblock below.
class plFollowComponent : public plComponent
{
private:
    bool            fValid;

    plFollowMod*    IMakeFollowMod(plMaxNode* pNode, plErrorMsg* pErrMsg);

public:
    plFollowComponent();

    bool SetupProperties(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};



CLASS_DESC(plFollowComponent, gFollowDesc, "Follow",  "Follow", COMP_TYPE_GRAPHICS, Class_ID(0x44262418, 0x73ee145b))



ParamBlockDesc2 gFollowBk
(
    plComponent::kBlkComp, _T("Follow"), 0, &gFollowDesc, P_AUTO_CONSTRUCT+P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_FOLLOW, IDS_COMP_FOLLOWS,  0, 0, &gFollowProc, 

    kAffectX,  _T("X"), TYPE_BOOL,      0, 0,
        p_default,  TRUE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_FOLLOW_X,
        p_end,

    kAffectY,  _T("Y"), TYPE_BOOL,      0, 0,
        p_default,  TRUE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_FOLLOW_Y,
        p_end,

    kAffectZ,  _T("Z"), TYPE_BOOL,      0, 0,
        p_default,  TRUE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_FOLLOW_Z,
        p_end,

    kAffectRotate,  _T("Rotate"), TYPE_BOOL,        0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_FOLLOW_ROTATE,
        p_end,

    kLeaderTypeRadio, _T("LeaderType"),     TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 4,  IDC_F_RADIO_PLAYER, IDC_F_RADIO_CAMERA,     IDC_F_RADIO_LISTENER,               IDC_F_RADIO_OBJECT, 
        p_vals,                     plFollowMod::kLocalPlayer,  plFollowMod::kCamera,   plFollowMod::kListener, plFollowMod::kObject,       
        p_default, plFollowMod::kLocalPlayer,
        p_end,

    kLeaderObjectSel, _T("ObjectChoice"),   TYPE_INODE,     0, 0,
        p_ui,   TYPE_PICKNODEBUTTON, IDC_COMP_FOLLOW_CHOOSE_OBJECT,
        p_prompt, IDS_COMP_LINE_CHOSE_OBJECT,
        p_accessor, &gLeaderObjAccessor,
        p_end,

    p_end

);

plFollowComponent::plFollowComponent()
:   fValid(false)
{
    fClassDesc = &gFollowDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

plFollowMod* plFollowComponent::IMakeFollowMod(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    plFollowMod::FollowLeaderType lType = plFollowMod::FollowLeaderType(fCompPB->GetInt(kLeaderTypeRadio));

    plFollowMod* follow = new plFollowMod;

    hsgResMgr::ResMgr()->NewKey(IGetUniqueName(pNode), follow, pNode->GetLocation());

    if( plFollowMod::kObject == lType )
    {
        if (fCompPB->GetINode(kLeaderObjectSel) != nullptr)
        {
            plMaxNode* targNode = (plMaxNode*)fCompPB->GetINode(kLeaderObjectSel);

            if( targNode->CanConvert() )
            {
                plSceneObject* targObj = targNode->GetSceneObject();
                if( targObj )
                {
                    plGenRefMsg* refMsg = new plGenRefMsg(follow->GetKey(), plRefMsg::kOnCreate, 0, plFollowMod::kRefLeader);
                    hsgResMgr::ResMgr()->AddViaNotify(targObj->GetKey(), refMsg, plRefFlags::kPassiveRef);

                    follow->SetType(plFollowMod::kObject);
                }
            }
        }
    }
    else
    {
        follow->SetType(lType);
    }

    uint32_t mode = 0;
    if( fCompPB->GetInt(kAffectX) )
        mode |= plFollowMod::kPositionX;
    if( fCompPB->GetInt(kAffectY) )
        mode |= plFollowMod::kPositionY;
    if( fCompPB->GetInt(kAffectZ) )
        mode |= plFollowMod::kPositionZ;
    if( fCompPB->GetInt(kAffectRotate) )
        mode |= plFollowMod::kRotate;

    if( !mode )
        mode = plFollowMod::kFullTransform;

    follow->SetMode(mode);

    return follow;
}

bool plFollowComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !fValid )
        return true;

    plFollowMod* follow = IMakeFollowMod(node, pErrMsg);

    if( follow )
        node->AddModifier(follow, IGetUniqueName(node));

    return true;
}

bool plFollowComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    fValid = false;

    if( plFollowMod::kObject == fCompPB->GetInt(kLeaderTypeRadio) )
    {
        if( !fCompPB->GetINode(kLeaderObjectSel) )
        {
            return true;
        }
    }
    fValid = true;
    pNode->SetForceLocal(true);
    pNode->SetMovable(true);
    return true;
}

bool plFollowComponent::PreConvert(plMaxNode* pNode, plErrorMsg* pErrMsg)
{
    if( !fValid )
        return true;
    fValid = false;

    if( plFollowMod::kObject == fCompPB->GetInt(kLeaderTypeRadio) )
    {
        plMaxNode* followNode = (plMaxNode*)fCompPB->GetINode(kLeaderObjectSel);
        if( !followNode->CanConvert() )
        {
            return true;
        }
    }
    fValid = true;


    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Unleash Component
//
//

//Class that accesses the paramblock below.
class plUnleashComponent : public plComponent
{
public:
    plUnleashComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

//Max desc stuff necessary below.
CLASS_DESC(plUnleashComponent, gUnleashDesc, "Unleash Satan",  "UnleashSatan", COMP_TYPE_GRAPHICS, Class_ID(0x5d937fa8, 0x1001411a))

ParamBlockDesc2 gUnleashBk
(
    plComponent::kBlkComp, _T("Unleash"), 0, &gUnleashDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_UNLEASH, IDS_COMP_UNLEASH, 0, 0, nullptr,

    p_end
);

plUnleashComponent::plUnleashComponent()
{
    fClassDesc = &gUnleashDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plUnleashComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    pNode->SetRunTimeLight(true);

    pNode->SetForcePreShade(true);

    return true;
}

bool plUnleashComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ForceRTLight Component
//
//

//Class that accesses the paramblock below.
class plForceRTLightComponent : public plComponent
{
public:
    plForceRTLightComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
};

//Max desc stuff necessary below.
CLASS_DESC(plForceRTLightComponent, gForceRTLightDesc, "Force RT Light",  "ForceRTLight", COMP_TYPE_GRAPHICS, Class_ID(0x1485091b, 0x42852fb5))

ParamBlockDesc2 gForceRTLightBk
(
    plComponent::kBlkComp, _T("ForceRTLight"), 0, &gForceRTLightDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_FORCE_RTLIGHT, IDS_COMP_FORCE_RTLIGHT, 0, 0, nullptr,

    p_end
);

plForceRTLightComponent::plForceRTLightComponent()
{
    fClassDesc = &gForceRTLightDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plForceRTLightComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    pNode->SetRunTimeLight(true);
    pNode->SetNoPreShade(true);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Terrain Optimise Component (currently just dices).
//
//
//Class that accesses the paramblock below.
class plGeoDiceComponent : public plComponent
{
public:
    enum {
        kActive,
        kMaxFaces,
        kMaxSize,
        kMinFaces,
        kOverride
    };
public:
    plGeoDiceComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }
};

static const int kDefMaxFaces(1000);
static const float kDefMaxSize(250.f);
static const int kDefMinFaces(300);

class plGeoDiceComponentProc : public ParamMap2UserDlgProc
{
public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = map->GetParamBlock();
                if( !pb->GetInt(plGeoDiceComponent::kOverride) )
                {
                    pb->SetValue(plGeoDiceComponent::kMaxFaces, t, kDefMaxFaces);
                    pb->SetValue(plGeoDiceComponent::kMaxSize, t, kDefMaxSize);
                    pb->SetValue(plGeoDiceComponent::kMinFaces, t, kDefMinFaces);

                    map->Enable(plGeoDiceComponent::kMaxFaces, FALSE);
                    map->Enable(plGeoDiceComponent::kMaxSize, FALSE);
                    map->Enable(plGeoDiceComponent::kMinFaces, FALSE);
                }
                else
                {
                    map->Enable(plGeoDiceComponent::kMaxFaces, TRUE);
                    map->Enable(plGeoDiceComponent::kMaxSize, TRUE);
                    map->Enable(plGeoDiceComponent::kMinFaces, TRUE);
                }
            }
            return TRUE;

//////////////////
        case WM_COMMAND:
            {
                if( LOWORD(wParam) == IDC_COMP_GEO_DICE_OVERRIDE )
                {
                    IParamBlock2 *pb = map->GetParamBlock();
                    if( !pb->GetInt(plGeoDiceComponent::kOverride) )
                    {
                        pb->SetValue(plGeoDiceComponent::kMaxFaces, t, kDefMaxFaces);
                        pb->SetValue(plGeoDiceComponent::kMaxSize, t, kDefMaxSize);
                        pb->SetValue(plGeoDiceComponent::kMinFaces, t, kDefMinFaces);

                        map->Enable(plGeoDiceComponent::kMaxFaces, FALSE);
                        map->Enable(plGeoDiceComponent::kMaxSize, FALSE);
                        map->Enable(plGeoDiceComponent::kMinFaces, FALSE);
                    }
                    else
                    {
                        map->Enable(plGeoDiceComponent::kMaxFaces, TRUE);
                        map->Enable(plGeoDiceComponent::kMaxSize, TRUE);
                        map->Enable(plGeoDiceComponent::kMinFaces, TRUE);
                    }
                    
                    return TRUE;
                }
            }
            
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plGeoDiceComponentProc gGeoDiceProc;


//Max desc stuff necessary below.
CLASS_DESC(plGeoDiceComponent, gGeoDiceDesc, "Optimize Terrain",  "OptimizeTerrain", COMP_TYPE_GRAPHICS, Class_ID(0x6f7a5713, 0x19595142))

ParamBlockDesc2 gGeoDiceBk
(
    plComponent::kBlkComp, _T("GeoDice"), 0, &gGeoDiceDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_GEO_DICE, IDS_COMP_GEO_DICE, 0, 0, &gGeoDiceProc,

    plGeoDiceComponent::kActive,  _T("Active"), TYPE_BOOL,      0, 0,
        p_default,  TRUE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_GEO_DICE_ACTIVE,
        p_end,

    plGeoDiceComponent::kMaxFaces, _T("MaxFaces"), TYPE_INT, 0, 0,
        p_ui,   TYPE_SPINNER, EDITTYPE_INT, IDC_COMP_GEO_DICE_MAXFACES, IDC_COMP_GEO_DICE_MAXFACES_SPIN,    1.f,
        p_default, 1000,
        p_range, 10, 10000,
        p_end,

    plGeoDiceComponent::kMaxSize, _T("MaxSize"),        TYPE_FLOAT, 0, 0,
        p_default, 100.0f,
        p_range, 0.0, 10000.0,
        p_ui,   TYPE_SPINNER, EDITTYPE_FLOAT, 
        IDC_COMP_GEO_DICE_MAXSIZE, IDC_COMP_GEO_DICE_MAXSIZE_SPIN, 0.1f,
        p_end,

    plGeoDiceComponent::kMinFaces, _T("MinFaces"), TYPE_INT, 0, 0,
        p_ui,   TYPE_SPINNER, EDITTYPE_INT, IDC_COMP_GEO_DICE_MINFACES, IDC_COMP_GEO_DICE_MINFACES_SPIN,    1.f,
        p_default, 300,
        p_range, 0, 5000,
        p_end,

    plGeoDiceComponent::kOverride,  _T("Override"), TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_COMP_GEO_DICE_OVERRIDE,
        p_end,

    p_end
);

plGeoDiceComponent::plGeoDiceComponent()
{
    fClassDesc = &gGeoDiceDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGeoDiceComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    if( fCompPB->GetInt(kActive) )
    {
        pNode->SetGeoDice(true, fCompPB->GetInt(kMaxFaces), fCompPB->GetFloat(kMaxSize), fCompPB->GetInt(kMinFaces));
    }
    return true;
}


///
///
/// reference point component
/// put this on a dummy for a handy reference point you can use in python
///
///

class plReferencePointComponent : public plComponent
{
public:
    plReferencePointComponent();

    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return true; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
};

CLASS_DESC(plReferencePointComponent, gReferencePointDesc, "Reference Point",  "RefPoint", COMP_TYPE_MISC, Class_ID(0x3c9c6f71, 0x5774fc5))

//Max paramblock2 stuff below.
ParamBlockDesc2 gReferencePointBk
(   
    1, _T("reference"), 0, &gReferencePointDesc, P_AUTO_CONSTRUCT, plComponent::kRefComp,

    p_end
);

plReferencePointComponent::plReferencePointComponent()
{
    fClassDesc = &gReferencePointDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool plReferencePointComponent::SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg)
{
    // all we need is a coordinate interface...
    pNode->SetForceLocal(true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
//

class plNetSyncComponent : public plComponent
{
protected:
    void ISetNetSync(plSynchedObject* so);
    void ISetSDLType(plSynchedObject* so, int radioVal, const char* sdlName);
    void ISetMtl(hsGMaterial* mtl);

public:
    plNetSyncComponent();

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode* node, plErrorMsg* errMsg) override;
    bool Convert(plMaxNode* node, plErrorMsg* errMsg) override { return true; }
    bool DeInit(plMaxNode* node, plErrorMsg* errMsg) override;
};

CLASS_DESC(plNetSyncComponent, gNetSyncDesc, "Net Sync", "NetSync", COMP_TYPE_MISC, Class_ID(0x4d1b2d6f, 0x28fe08db))

enum
{
    kNetSyncLocalOnly,
    kNetSyncOverride,
    kNetSyncPhys,
    kNetSyncAnim,
    kNetSyncSnd,
    kNetSyncMat,
    kNetSyncResp,
    kNetSyncXReg,
};

enum
{
    kNetSyncRadioAllow,
    kNetSyncRadioNoSave,
    kNetSyncRadioDeny,
};

class plNetSyncComponentProc : public ParamMap2UserDlgProc
{
protected:
    void IEnableSDL(IParamMap2* map, bool enable)
    {
        map->Enable(kNetSyncPhys, enable);
        map->Enable(kNetSyncAnim, enable);
        map->Enable(kNetSyncSnd,  enable);
        map->Enable(kNetSyncMat,  enable);
        map->Enable(kNetSyncResp, enable);
        map->Enable(kNetSyncXReg, enable);
    }

    void IEnableCtrls(IParamMap2* map)
    {
        IParamBlock2* pb = map->GetParamBlock();
        if (pb->GetInt(kNetSyncLocalOnly))
        {
            map->Enable(kNetSyncOverride, FALSE);
            IEnableSDL(map, false);
        }
        else if (pb->GetInt(kNetSyncOverride))
        {
            map->Enable(kNetSyncLocalOnly, FALSE);
            IEnableSDL(map, true);
        }
        else
        {
            map->Enable(kNetSyncOverride, TRUE);
            map->Enable(kNetSyncLocalOnly, TRUE);
            IEnableSDL(map, false);
        }
    }

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            IEnableCtrls(map);
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_LOCAL_ONLY_CHECK ||
                LOWORD(wParam) == IDC_OVERRIDE_CHECK)
            {
                IEnableCtrls(map);
                return TRUE;
            }
            break;
        }

        return FALSE;
    }
    void DeleteThis() override { }
};
static plNetSyncComponentProc gNetSyncProc;

//Max paramblock2 stuff below.
ParamBlockDesc2 gHighSDLBk
(   
    plComponent::kBlkComp, _T("NetSync"), 0, &gNetSyncDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_NETSYNC, IDS_COMP_NETSYNC, 0, 0, &gNetSyncProc,

    kNetSyncLocalOnly, _T("LocalOnly"), TYPE_BOOL, 0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_LOCAL_ONLY_CHECK,
        p_end,

    kNetSyncOverride, _T("Override"), TYPE_BOOL, 0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_OVERRIDE_CHECK,
        p_end,

    kNetSyncPhys,   _T("PhysSDL"),      TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_PHYS_ALLOW_RADIO, IDC_PHYS_NOSAVE_RADIO, IDC_PHYS_DENY_RADIO,
        p_vals,     kNetSyncRadioAllow, kNetSyncRadioNoSave, kNetSyncRadioDeny,
        p_default,  kNetSyncRadioAllow,
        p_end,
    
    kNetSyncAnim,   _T("AnimSDL"),      TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_ANIM_ALLOW_RADIO, IDC_ANIM_NOSAVE_RADIO, IDC_ANIM_DENY_RADIO,
        p_vals,     kNetSyncRadioAllow, kNetSyncRadioNoSave, kNetSyncRadioDeny,
        p_default,  kNetSyncRadioAllow,
        p_end,

    kNetSyncSnd,    _T("SndSDL"),       TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_SND_ALLOW_RADIO, IDC_SND_NOSAVE_RADIO, IDC_SND_DENY_RADIO,
        p_vals,     kNetSyncRadioAllow, kNetSyncRadioNoSave, kNetSyncRadioDeny,
        p_default,  kNetSyncRadioAllow,
        p_end,

    kNetSyncMat,    _T("MatSDL"),       TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_MAT_ALLOW_RADIO, IDC_MAT_NOSAVE_RADIO, IDC_MAT_DENY_RADIO,
        p_vals,     kNetSyncRadioAllow, kNetSyncRadioNoSave, kNetSyncRadioDeny,
        p_default,  kNetSyncRadioAllow,
        p_end,

    kNetSyncResp,   _T("RespSDL"),      TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_RESP_ALLOW_RADIO, IDC_RESP_NOSAVE_RADIO, IDC_RESP_DENY_RADIO,
        p_vals,     kNetSyncRadioAllow, kNetSyncRadioNoSave, kNetSyncRadioDeny,
        p_default,  kNetSyncRadioAllow,
        p_end,

    kNetSyncXReg,   _T("XRegSDL"),      TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3,  IDC_XREG_ALLOW_RADIO, IDC_XREG_NOSAVE_RADIO, IDC_XREG_DENY_RADIO,
        p_vals,     kNetSyncRadioAllow, kNetSyncRadioNoSave, kNetSyncRadioDeny,
        p_default,  kNetSyncRadioAllow,
        p_end,

    p_end
);

plNetSyncComponent::plNetSyncComponent()
{
    fClassDesc = &gNetSyncDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plNetSyncComponent::SetupProperties(plMaxNode* node, plErrorMsg* errMsg)
{
    // make all sdl types on this object Volatile
    bool override = (fCompPB->GetInt(kNetSyncOverride) != 0);
    node->SetOverrideHighLevelSDL(override);

    return true;
}

void plNetSyncComponent::ISetSDLType(plSynchedObject* so, int radioVal, const char* sdlName)
{
    switch (radioVal)
    {
    case kNetSyncRadioNoSave:
        so->AddToSDLVolatileList(sdlName);  // make volatile a type of persistence
        break;

    case kNetSyncRadioDeny:
        so->AddToSDLExcludeList(sdlName);   // disable a type of persistence
        break;
    }
}

void plNetSyncComponent::ISetNetSync(plSynchedObject* so)
{
    // For local only, disable everything and exit
    bool localOnly = (fCompPB->GetInt(kNetSyncLocalOnly) != 0);
    if (localOnly)
    {
        so->SetLocalOnly(true);
        return;
    }

    int physVal = fCompPB->GetInt(kNetSyncPhys);
    int animVal = fCompPB->GetInt(kNetSyncAnim);
    int sndVal  = fCompPB->GetInt(kNetSyncSnd);
    int matVal  = fCompPB->GetInt(kNetSyncMat);
    int respVal = fCompPB->GetInt(kNetSyncResp);
    int xregVal = fCompPB->GetInt(kNetSyncXReg);

    // If all are not saved, use that optimization
    if  (
        physVal == kNetSyncRadioNoSave &&
        animVal == kNetSyncRadioNoSave &&
        sndVal  == kNetSyncRadioNoSave &&
        matVal  == kNetSyncRadioNoSave &&
        respVal == kNetSyncRadioNoSave &&
        xregVal == kNetSyncRadioNoSave
        )
    {
        so->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
    }
    // If all are volatile, use that optimization
    else if (
        physVal == kNetSyncRadioDeny &&
        animVal == kNetSyncRadioDeny &&
        sndVal  == kNetSyncRadioDeny &&
        matVal  == kNetSyncRadioDeny &&
        respVal == kNetSyncRadioDeny &&
        xregVal == kNetSyncRadioDeny
        )
    {
        so->SetSynchFlagsBit(plSynchedObject::kExcludeAllPersistentState);
    }
    // If it's a mix, we need to set them individually
    else
    {
        ISetSDLType(so, physVal, kSDLPhysical);
        ISetSDLType(so, animVal, kSDLAGMaster);
        ISetSDLType(so, sndVal,  kSDLSound);
        ISetSDLType(so, matVal,  kSDLLayer);
        ISetSDLType(so, respVal, kSDLResponder);
        ISetSDLType(so, xregVal, kSDLXRegion);
    }
}

void plNetSyncComponent::ISetMtl(hsGMaterial* mtl)
{
    for (size_t i = 0; i < mtl->GetNumLayers(); i++)
    {
        plLayerInterface* layer = mtl->GetLayer(i);
        while (layer)
        {
            if (layer->ClassIndex() == CLASS_INDEX_SCOPED(plLayerAnimation) ||
                layer->ClassIndex() == CLASS_INDEX_SCOPED(plLayerSDLAnimation))
            {
                ISetNetSync(layer);
            }

            layer = layer->GetAttached();
        }
    }
}

// We're cheating and using DeInit as an extra pass, since everything should be done at that point
bool plNetSyncComponent::DeInit(plMaxNode* node, plErrorMsg* errMsg)
{
    plSceneObject* so = node->GetSceneObject();
    if (!so)
        return false;

    // Set sync options on the sceneobject
    ISetNetSync(so);

    // Then on the textures...
    Mtl* maxMaterial = hsMaterialConverter::Instance().GetBaseMtl(node);
    if (maxMaterial)
    {
        std::vector<hsGMaterial*> matArray;

        // Get the textures from the material converter
        if (hsMaterialConverter::Instance().IsMultiMat(maxMaterial))
        {
            int numMaterials = maxMaterial->NumSubMtls();
            for (int i = 0; i < numMaterials; i++)
                hsMaterialConverter::Instance().GetMaterialArray(maxMaterial->GetSubMtl(i), node, matArray, i);
        }
        else
            hsMaterialConverter::Instance().GetMaterialArray(maxMaterial, node, matArray);

        // Set sync on the textures we found
        for (hsGMaterial* mtl : matArray)
        {
            if (mtl)
                ISetMtl(mtl);
        }
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Image Lib Component
//
//

class pfImageLibProc : public ParamMap2UserDlgProc
{
protected:

    void    IRefreshImageList( HWND hDlg, pfImageLibComponent *comp )
    {
        HWND ctrl = GetDlgItem( hDlg, IDC_IMAGE_LIST );
        HDC dc = GetDC( ctrl );

        LONG maxWidth = 0;
        SendMessage( ctrl, LB_RESETCONTENT, 0, 0 );
    
        for( int i = 0; i < comp->GetNumBitmaps(); i++ )
        {
            plLayerTex *layer = comp->GetBitmap( i );
            if (layer != nullptr)
            {
                const TCHAR* str = layer->GetPBBitmap()->bi.Filename();
                int idx = (int)SendMessage(ctrl, LB_ADDSTRING, 0, (LPARAM)str);
                SendMessage( ctrl, LB_SETITEMDATA, (WPARAM)idx, (LPARAM)i );

                SIZE strSize;
                GetTextExtentPoint32( dc, str, _tcslen(str), &strSize );
                if( strSize.cx > maxWidth )
                    maxWidth = strSize.cx;
            }
        }
        SendMessage(ctrl, LB_SETHORIZONTALEXTENT, (WPARAM)maxWidth, 0);
        ReleaseDC( ctrl, dc );

        EnableWindow( GetDlgItem( hDlg, IDC_IMAGE_EDIT ), false );
        EnableWindow( GetDlgItem( hDlg, IDC_IMAGE_REMOVE ), false );

        CheckDlgButton(hDlg, IDC_IL_COMPRESS, BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_IL_FORCEPOW2, BST_UNCHECKED);
        EnableWindow(GetDlgItem(hDlg, IDC_IL_COMPRESS), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_IL_FORCEPOW2), FALSE);
    }

public:

    void DeleteThis() override { }

//  virtual void    Update( TimeValue t, Interval &valid, IParamMap2 *map );

    INT_PTR DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        IParamBlock2            *pb = pmap->GetParamBlock();
        pfImageLibComponent     *comp = (pfImageLibComponent *)pb->GetOwner();


        switch( msg )
        {
            case WM_INITDIALOG:
                // Fill our list with bitmap filenames
                comp->Validate();
                IRefreshImageList( hWnd, comp );
                return TRUE;

            case WM_DESTROY:
                break;

            case WM_COMMAND:
                if( LOWORD( wParam ) == IDC_IMAGE_ADD )
                {
                    plLayerTex *newLayer = new plLayerTex;

                    if( newLayer->HandleBitmapSelection() )
                    {
                        comp->AppendBitmap( newLayer );
                        IRefreshImageList( hWnd, comp );
                    }
                }
                else if( LOWORD( wParam ) == IDC_IMAGE_EDIT )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_IMAGE_LIST, LB_GETCURSEL, 0, 0);
                    if( idx != LB_ERR )
                    {
                        idx = (int)SendDlgItemMessage(hWnd, IDC_IMAGE_LIST, LB_GETITEMDATA, (WPARAM)idx, 0);
                        plLayerTex *layer = comp->GetBitmap( idx );
                        if (layer != nullptr && layer->HandleBitmapSelection())
                        {
                            IRefreshImageList( hWnd, comp );
                        }
                    }
                }               
                else if( LOWORD( wParam ) == IDC_IMAGE_REMOVE )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_IMAGE_LIST, LB_GETCURSEL, 0, 0);
                    if( idx != LB_ERR )
                    {
                        idx = (int)SendDlgItemMessage(hWnd, IDC_IMAGE_LIST, LB_GETITEMDATA, (WPARAM)idx, 0);
                        comp->RemoveBitmap( idx );
                        IRefreshImageList( hWnd, comp );
                    }
                    return FALSE;
                }
                else if( LOWORD( wParam ) == IDC_IMAGE_LIST && HIWORD( wParam ) == LBN_SELCHANGE )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_IMAGE_LIST, LB_GETCURSEL, 0, 0);
                    EnableWindow( GetDlgItem( hWnd, IDC_IMAGE_EDIT ), idx != LB_ERR );
                    EnableWindow( GetDlgItem( hWnd, IDC_IMAGE_REMOVE ), idx != LB_ERR );

                    EnableWindow(GetDlgItem(hWnd, IDC_IL_COMPRESS), TRUE);
                    CheckDlgButton(hWnd, IDC_IL_COMPRESS, comp->GetCompress(idx) ? BST_CHECKED : BST_UNCHECKED);
                }
                else if (LOWORD(wParam) == IDC_IL_COMPRESS && HIWORD(wParam) == BN_CLICKED)
                {
                    bool checked = (IsDlgButtonChecked(hWnd, IDC_IL_COMPRESS) == BST_CHECKED);

                    int sel = ListBox_GetCurSel(GetDlgItem(hWnd, IDC_IMAGE_LIST));
                    comp->SetCompress(sel, checked);
                }
                break;

        }
        return FALSE;
    }
};
static pfImageLibProc   gImageLibProc;

//Max desc stuff necessary below.
CLASS_DESC(pfImageLibComponent, gImageLibDesc, "Image Library",  "ImageLib", COMP_TYPE_MISC, IMAGE_LIB_CID )

ParamBlockDesc2 gImageLibBlock
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
    1, _T("Image Lib"), 0, &gImageLibDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_IMAGELIB, IDS_COMP_IMAGELIB, 0, 0, &gImageLibProc,

    // params
    pfImageLibComponent::kRefImageList,     _T("imageList"), TYPE_TEXMAP_TAB, 0,        0, 0,   
        p_end,

    pfImageLibComponent::kCompressImage,    _T("compress"),     TYPE_BOOL_TAB, 0,       0, 0,
        p_default, 1,
        p_end,

    p_end
);

pfImageLibComponent::pfImageLibComponent()
{
    fClassDesc = &gImageLibDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

void pfImageLibComponent::Validate()
{
    if (fCompPB->Count(kCompressImage) != fCompPB->Count(kRefImageList))
        fCompPB->SetCount(kCompressImage, fCompPB->Count(kRefImageList));
}

int pfImageLibComponent::GetNumBitmaps() const
{
    return fCompPB->Count( (ParamID)kRefImageList );
}

plLayerTex  *pfImageLibComponent::GetBitmap( int idx )
{  
    // If we don't have one, create one
    plLayerTex  *layer = (plLayerTex *)fCompPB->GetTexmap( (ParamID)kRefImageList, 0, idx );
    if (layer == nullptr || layer->ClassID() != LAYER_TEX_CLASS_ID)
    {
        layer = new plLayerTex;
        fCompPB->SetValue( (ParamID)kRefImageList, 0, (Texmap *)layer, idx );
    }

    return layer;
}

int     pfImageLibComponent::AppendBitmap( plLayerTex *tex )
{
    int idx = GetNumBitmaps();
    fCompPB->Resize( (ParamID)kRefImageList, idx + 1 );
    fCompPB->SetValue( (ParamID)kRefImageList, 0, (Texmap *)tex, idx );

    fCompPB->Resize(kCompressImage, idx + 1);

    return idx;
}

void    pfImageLibComponent::RemoveBitmap( int idx )
{
    fCompPB->Delete( (ParamID)kRefImageList, idx, 1 );
    fCompPB->Delete(kCompressImage, idx, 1);
}

bool pfImageLibComponent::GetCompress(int idx)
{
    return (fCompPB->GetInt(kCompressImage, 0, idx) != 0);
}

void pfImageLibComponent::SetCompress(int idx, bool compress)
{
    fCompPB->SetValue(kCompressImage, 0, compress, idx);
}

bool pfImageLibComponent::SetupProperties(plMaxNode *pNode,  plErrorMsg *pErrMsg)
{
    Validate();
    return true;
}

bool pfImageLibComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    plImageLibMod *lib = new plImageLibMod;
    node->AddModifier( lib, IGetUniqueName(node) );

    int i;
    for( i = 0; i < GetNumBitmaps(); i++ )
    {
        plLayerTex *layer = GetBitmap( i );
        if (layer != nullptr)
        {
            PBBitmap *texture = layer->GetPBBitmap();
            if (texture != nullptr)
            {
                uint32_t flags = plBitmap::kAlphaChannelFlag;

                plBitmap *bMap;
                if (fCompPB->GetInt(kCompressImage, 0, i) == 0)
                {
                    flags |= plBitmap::kForceNonCompressed;
                    bMap = plLayerConverter::Instance().CreateSimpleTexture( M2ST(texture->bi.Name()), lib->GetKey()->GetUoid().GetLocation(), 0, flags );
                }
                else // compress using PNG compression scheme
                    bMap = plLayerConverter::Instance().CreateSimpleTexture( M2ST(texture->bi.Name()), lib->GetKey()->GetUoid().GetLocation(), 0, flags, true );
                if (bMap != nullptr)
                {
                    hsgResMgr::ResMgr()->AddViaNotify( bMap->GetKey(), new plGenRefMsg( lib->GetKey(), 
                                            plRefMsg::kOnCreate, lib->GetNumImages(), plImageLibMod::kRefImage ), plRefFlags::kActiveRef );
                }
            }
        }
    }
    
    return true;
}



