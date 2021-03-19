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
#include "plgDispatch.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "plAnimComponent.h"
#include "plAudioComponents.h"
#include "plMiscComponents.h"
#include "resource.h"
#include "MaxMain/plMaxNode.h"
#include "MaxMain/MaxAPI.h"

#include <string>
#include <vector>

#include "plNotetrackAnim.h"

#include "plGUIComponents.h"

#include "MaxMain/plPlasmaRefMsgs.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plDrawInterface.h"
#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGeometrySpan.h"
#include "plSurface/plLayerInterface.h"
#include "plSurface/plLayer.h"
#include "plSurface/hsGMaterial.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plDynamicTextMap.h"

#include "plMessage/plLayRefMsg.h"
#include "plMessage/plMatRefMsg.h"

#include "MaxMain/plPluginResManager.h"


#include "pnMessage/plObjRefMsg.h"
#include "pnMessage/plIntRefMsg.h"
#include "pnMessage/plNodeRefMsg.h"


#include "plScene/plSceneNode.h"
#include "MaxConvert/hsConverterUtils.h"
#include "MaxConvert/hsControlConverter.h"
#include "MaxConvert/hsMaterialConverter.h"
#include "MaxConvert/plLayerConverter.h"
#include "plInterp/plController.h"
#include "plInterp/plAnimEaseTypes.h"

// GUIDialog component.
#include "plScene/plPostEffectMod.h"
#include "pfGameGUIMgr/pfGameGUIMgr.h"
#include "pfGameGUIMgr/pfGUIDialogMod.h"
#include "pfGameGUIMgr/pfGUIControlMod.h"
#include "pfGameGUIMgr/pfGUIControlHandlers.h"
#include "pfGameGUIMgr/pfGUIButtonMod.h"
#include "pfGameGUIMgr/pfGUIDraggableMod.h"
#include "pfGameGUIMgr/pfGUIListBoxMod.h"
#include "pfGameGUIMgr/pfGUITextBoxMod.h"
#include "pfGameGUIMgr/pfGUIEditBoxMod.h"
#include "pfGameGUIMgr/pfGUIUpDownPairMod.h"
#include "pfGameGUIMgr/pfGUIKnobCtrl.h"
#include "pfGameGUIMgr/pfGUITagDefs.h"
#include "pfGameGUIMgr/pfGUIDragBarCtrl.h"
#include "pfGameGUIMgr/pfGUICheckBoxCtrl.h"
#include "pfGameGUIMgr/pfGUIRadioGroupCtrl.h"
#include "pfGameGUIMgr/pfGUIDynDisplayCtrl.h"
#include "pfGameGUIMgr/pfGUIMultiLineEditCtrl.h"
#include "pfGameGUIMgr/pfGUIProgressCtrl.h"
#include "pfGameGUIMgr/pfGUIClickMapCtrl.h"
#include "pfGameGUIMgr/pfGUIPopUpMenu.h"

// Location Related
#include "plAgeDescription/plAgeDescription.h"
#include "MaxMain/plMaxCFGFile.h"
#include "MaxMain/plAgeDescInterface.h"
#include "MaxConvert/plConvert.h"
#include "MaxPlasmaMtls/Layers/plDynamicTextLayer.h"
#include "MaxPlasmaMtls/Layers/plLayerTexBitmapPB.h"

#include "MaxMain/plMaxAccelerators.h"

#include "plPickMaterialMap.h"

#include "plInterp/plController.h"
#include "plAnimation/plMatrixChannel.h"

#include "MaxPlasmaMtls/Layers/plLayerTex.h"

#include "pfGUISkinComp.h"

#include "plResMgr/plLocalization.h"

#include "plPickLocalizationDlg.h"


void DummyCodeIncludeFuncGUI() {}



/////////////////////////////////////////////////////////////////////////////////////////////////
//// Helper Classes /////////////////////////////////////////////////////////////////////////////

//// Hit Callback for GUI Controls //////////////////////////////////////////////////////////////

class plGUICtrlHitCallback : public HitByNameDlgCallback
{
protected:
    INode*          fOwner;
    IParamBlock2*   fPB;
    ParamID         fNodeListID;
    BOOL            fRestrict;
    std::vector<Class_ID> fRestrictedIDs;
    TCHAR           fTitle[ 128 ];
    BOOL            fSingle;

public:
    plGUICtrlHitCallback(INode* owner, IParamBlock2 *pb, ParamID nodeListID, TCHAR *title = nullptr,
                            BOOL restricted = FALSE, Class_ID rID = GUI_BUTTON_CLASSID, BOOL single = TRUE )
        : fOwner( owner ), fPB( pb ), fNodeListID( nodeListID ), fRestrict( restricted ), fSingle( single )

    {
        fRestrictedIDs.emplace_back(rID);
        strcpy( fTitle, title );
    }

    plGUICtrlHitCallback(INode* owner, IParamBlock2 *pb, ParamID nodeListID, TCHAR *title,
                         std::vector<Class_ID> rID)
        : fOwner(owner), fPB(pb), fNodeListID(nodeListID), fRestrict(true),
          fRestrictedIDs(std::move(rID)), fSingle(TRUE)

    {
        strcpy( fTitle, title );
    }

    TCHAR *dialogTitle() override { return fTitle; }
    TCHAR *buttonText() override { return "OK"; }

    int filter(INode *node) override
    {
        if( node == fOwner )
            return FALSE;

        plComponentBase *comp = ((plMaxNodeBase*)node)->ConvertToComponent();

        // If this is an activator type component
        if( comp )
        {
            if ((fRestrict && std::find(fRestrictedIDs.cbegin(), fRestrictedIDs.cend(), comp->ClassID()) != fRestrictedIDs.cend())
                || (!fRestrict && plGUIControlBase::GetGUIComp(comp) != nullptr))
            {

                // And this wouldn't create a cyclical reference (Max doesn't like those)
                if (comp->TestForLoop(FOREVER, fPB) == REF_FAIL)
                    return FALSE;

                return TRUE;
            }
        }
        else if (fRestrict && std::find(fRestrictedIDs.cbegin(), fRestrictedIDs.cend(), node->ClassID()) != fRestrictedIDs.cend())
        {
            return TRUE;
        }

        return FALSE;
    }

    void proc(INodeTab &nodeTab) override
    {
        if ( nodeTab.Count() > 0 )
        {
            if( fSingle )
                fPB->SetValue( fNodeListID, TimeValue(0), nodeTab[0] );
            else
                fPB->Append( fNodeListID, nodeTab.Count(), &nodeTab[0] );
        }
    }

    BOOL showHiddenAndFrozen() override { return TRUE; }
    BOOL singleSelect() override { return TRUE; }
};

//// Single GUI Control Dialog Proc /////////////////////////////////////////////////////////////

class plGUISingleCtrlDlgProc : public ParamMap2UserDlgProc
{
protected:
    ParamID         fNodeID;
    int             fDlgItem;
    TCHAR           fTitle[ 128 ];
    std::vector<Class_ID> fClassesToSelect;

    ParamMap2UserDlgProc    *fProcChain;

public:

    int     GetHandledDlgItem() const { return fDlgItem; }

    static const Class_ID       kEndClassList; 

    plGUISingleCtrlDlgProc(ParamID nodeID, int dlgItem, TCHAR *title, Class_ID *restrict, ParamMap2UserDlgProc *parentProc = nullptr)
    {
        fNodeID = nodeID;
        fDlgItem = dlgItem;
        for( int i = 0; restrict[ i ] != kEndClassList; i++ )
            fClassesToSelect.emplace_back(restrict[i]);
//      fClassToSelect = restrict;
        strcpy( fTitle, title );
        fProcChain = parentProc;
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch ( msg )
        {
            case WM_INITDIALOG:
                {
                    IParamBlock2 *pb = map->GetParamBlock();

                    INode *node = pb->GetINode( fNodeID );
                    TSTR newName( node ? node->GetName() : "Pick" );
                    ::SetWindowText( ::GetDlgItem( hWnd, fDlgItem ), newName );
                }
                break;

            case WM_COMMAND:
                if( ( HIWORD( wParam ) == BN_CLICKED ) )
                {
                    if( LOWORD( wParam ) == fDlgItem )
                    {
                        IParamBlock2 *pb = map->GetParamBlock();
                        plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fNodeID, fTitle, fClassesToSelect );
                        GetCOREInterface()->DoHitByNameDialog( &hitCB );

                        INode* node = pb->GetINode( fNodeID );
                        TSTR newName( node ? node->GetName() : "Pick" );
                        ::SetWindowText( ::GetDlgItem(hWnd, fDlgItem ), newName );
                        map->Invalidate( fNodeID );
                        ::InvalidateRect(hWnd, nullptr, TRUE);
                        return TRUE;
                    }
                }
                break;
        }

        if( fProcChain )
            fProcChain->DlgProc( t, map, hWnd, msg, wParam, lParam );

        return FALSE;
    }

    void DeleteThis() override { }
};

const Class_ID      plGUISingleCtrlDlgProc::kEndClassList = Class_ID(); 

Class_ID    sSkinClassesToSelect[] = { GUI_SKIN_CLASSID, plGUISingleCtrlDlgProc::kEndClassList };


//// Multiple GUI Control Dialog Proc ///////////////////////////////////////////////////////////

class plGUIMultipleCtrlDlgProc : public ParamMap2UserDlgProc
{
protected:
    std::vector<plGUISingleCtrlDlgProc *>  fSingleProcs;
    std::vector<ParamMap2UserDlgProc *>    fProcs;

public:

    plGUIMultipleCtrlDlgProc(plGUISingleCtrlDlgProc **singleProcs, ParamMap2UserDlgProc **procs=nullptr)
    {
        for (int i = 0; singleProcs[i] != nullptr; i++)
            fSingleProcs.emplace_back(singleProcs[i]);
        if ( procs )
        {
            for (int i = 0; procs[i] != nullptr; i++)
                fProcs.emplace_back(procs[i]);
        }
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch ( msg )
        {
            case WM_INITDIALOG:
                for (plGUISingleCtrlDlgProc* proc : fSingleProcs)
                    proc->DlgProc(t, map, hWnd, msg, wParam, lParam);

                for (ParamMap2UserDlgProc* proc : fProcs)
                    proc->DlgProc(t, map, hWnd, msg, wParam, lParam);

                return TRUE;

            case WM_COMMAND:
                for (plGUISingleCtrlDlgProc* proc : fSingleProcs)
                {
                    if (proc->GetHandledDlgItem() == LOWORD(wParam))
                    {
                        proc->DlgProc(t, map, hWnd, msg, wParam, lParam);
                        break;
                    }
                }
                // and now do the procs that want more control
                for (ParamMap2UserDlgProc* proc : fProcs)
                    proc->DlgProc(t, map, hWnd, msg, wParam, lParam);

                return TRUE;
        }

        return FALSE;
    }

    void DeleteThis() override { }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUITag Component /////////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


class plGUITagProc : public ParamMap2UserDlgProc
{
protected:

    void    ILoadTags( HWND hWnd, IParamBlock2 *pb );

public:

    void DeleteThis() override { }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};

static plGUITagProc gGUITagProc;

// Class that accesses the paramblock below.
class plGUITagComponent : public plComponent
{
public:
    plGUITagComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefCurrIDSel = 64      // So we can share it among other components
    };

    static uint32_t GetTagIDOnNode( plMaxNode *node );
};

//Max desc stuff necessary below.
#define kGUITagClassID      Class_ID(0x77276e84, 0x24f360c5)
CLASS_DESC(plGUITagComponent, gGUITagDesc, "GUI ID Tag",  "GUITag", COMP_TYPE_GUI, kGUITagClassID )

ParamBlockDesc2 gGUITagBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUITag"), 0, &gGUITagDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_GUITAG, IDS_COMP_GUITAG, 0, 0, &gGUITagProc,

        plGUITagComponent::kRefCurrIDSel,   _T("currSel"),      TYPE_INT,       0, 0,
        end,

    end
);

void    plGUITagProc::ILoadTags( HWND hWnd, IParamBlock2 *pb )
{
    int     idx, idx2 = 0;
    char    str[] = "(none)";


    SendMessage( hWnd, CB_RESETCONTENT, 0, 0 );
    idx2 = idx = (int)SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)str);
    SendMessage( hWnd, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)0 );

    for( uint32_t i = 0; i < pfGameGUIMgr::GetNumTags(); i++ )
    {
        pfGUITag *tag = pfGameGUIMgr::GetTag( i );
        idx = (int)SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)tag->fName);
        SendMessage( hWnd, CB_SETITEMDATA, (WPARAM)idx, (LPARAM)tag->fID );

        if( tag->fID == pb->GetInt( plGUITagComponent::kRefCurrIDSel ) )
            idx2 = idx;
    }

    if( idx2 == 0 && pb->GetInt( plGUITagComponent::kRefCurrIDSel ) != 0 )
    {
        char    str[ 32 ];
        sprintf( str, "%d", pb->GetInt( plGUITagComponent::kRefCurrIDSel ) );
        SendMessage( hWnd, WM_SETTEXT, 0, (LPARAM)str );
    }
    else
        SendMessage( hWnd, CB_SETCURSEL, idx2, 0 );
}

// Callback enum proc for below
BOOL CALLBACK   GetEditCtrlEnumProc( HWND hWnd, LPARAM lParam )
{
    char    className[ 128 ];


    // ICK
    GetClassName( hWnd, className, sizeof( className ) - 1 );
    if( stricmp( className, "EDIT" ) == 0 )
    {
        HWND    *ptr = (HWND *)lParam;
        *ptr = hWnd;
        return FALSE;
    }
    return TRUE;
}

// Small proc that, given the handle of a combo box, returns the handle of the edit window for it
static HWND     GetEditCtrlFromComboBox( HWND combo )
{
    HWND    toReturn;

    EnumChildWindows( combo, GetEditCtrlEnumProc, (LPARAM)&toReturn );
    return toReturn;
}

// Small proc to only allow numbers in an edit box
static WNDPROC      sOriginalProc = nullptr;
LRESULT CALLBACK    SubclassedEditProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_CHAR:
            if( !isdigit( (TCHAR)wParam ) )
                return 0;
            break;

        case WM_GETDLGCODE:
            return DLGC_WANTALLKEYS;

        case WM_KEYDOWN:
            if( wParam == VK_RETURN )
            {
                // Do the same thing as when we lose focus--check our int value and make
                // sure it's big enough (don't worry about setting the paramBlock value,
                // that'll happen when the control loses focus)

                char    str[ 32 ];
                GetWindowText( hWnd, str, sizeof( str ) - 1 );
                int id = atoi( str );

                if( id < pfGameGUIMgr::GetHighestTag() + 1 )
                {
                    id = pfGameGUIMgr::GetHighestTag() + 1;
                    sprintf( str, "%d", id );
                    SetWindowText( hWnd, str );
                }
                SendMessage( hWnd, EM_SETSEL, 0, (LPARAM)-1 );
                return 0;
            }
            break;
    }

    return CallWindowProc( sOriginalProc, hWnd, msg, wParam, lParam );
}

INT_PTR plGUITagProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND    edit;
    BOOL    dummy1;


    switch( msg )
    {
        case WM_INITDIALOG:
            ILoadTags( GetDlgItem( hWnd, IDC_GUI_TAGCOMBO ), pmap->GetParamBlock() );

            // Set the edit control of the combo box to only accept number characters
            edit = GetEditCtrlFromComboBox( GetDlgItem( hWnd, IDC_GUI_TAGCOMBO ) );
            SetWindowLong( edit, GWL_STYLE, GetWindowLong( edit, GWL_STYLE ) | ES_WANTRETURN );
            sOriginalProc = (WNDPROC)SetWindowLongPtr(edit, GWLP_WNDPROC, (LONG_PTR)SubclassedEditProc);
            
            return TRUE;

        case WM_DESTROY:
            SetWindowLongPtr(GetDlgItem(hWnd, IDC_GUI_TAGCOMBO), GWLP_WNDPROC, (LONG_PTR)sOriginalProc);
            break;

        case WM_COMMAND:
            if( LOWORD( wParam ) == IDC_GUI_TAGCOMBO )
            {
                if( HIWORD( wParam ) == CBN_SELCHANGE )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_GUI_TAGCOMBO, CB_GETCURSEL, 0, 0);
                    if( idx == CB_ERR )
                    {
                        // Must be a custom one
                        int id = GetDlgItemInt( hWnd, IDC_GUI_TAGCOMBO, &dummy1, false );
                        pmap->GetParamBlock()->SetValue( plGUITagComponent::kRefCurrIDSel, 0, id );
                    }
                    else
                    {
                        pmap->GetParamBlock()->SetValue( plGUITagComponent::kRefCurrIDSel, 0, 
                                    (int)SendDlgItemMessage(hWnd, IDC_GUI_TAGCOMBO, CB_GETITEMDATA, idx, 0));
                    }
                }
                else if( HIWORD( wParam ) == CBN_KILLFOCUS )
                {
                    plMaxAccelerators::Enable();

                    // Make sure the number inside is valid
                    if( SendDlgItemMessage( hWnd, IDC_GUI_TAGCOMBO, CB_GETCURSEL, 0, 0 ) == CB_ERR )
                    {
                        int id = GetDlgItemInt( hWnd, IDC_GUI_TAGCOMBO, &dummy1, false );
                        if( id < pfGameGUIMgr::GetHighestTag() + 1 )
                        {
                            id = pfGameGUIMgr::GetHighestTag() + 1;
                            SetDlgItemInt( hWnd, IDC_GUI_TAGCOMBO, id, false );
                        }

                        pmap->GetParamBlock()->SetValue( plGUITagComponent::kRefCurrIDSel, 0, id );
                    }
                }
                else if( HIWORD( wParam ) == CBN_SETFOCUS )
                {
                    plMaxAccelerators::Disable();
                }
            }
            break;
    }
    return FALSE;
}

plGUITagComponent::plGUITagComponent()
{
    fClassDesc = &gGUITagDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUITagComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return true;
}

bool plGUITagComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{

    return true;
}

bool plGUITagComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    return true;
}

uint32_t plGUITagComponent::GetTagIDOnNode( plMaxNode *node )
{
    uint32_t  i;


    for( i = 0; i < node->NumAttachedComponents( false ); i++ )
    {
        plComponentBase *comp = node->GetAttachedComponent( i, false );
        if( comp->ClassID() == kGUITagClassID )
        {
            plGUITagComponent *tag = (plGUITagComponent *)comp;
            return tag->GetParamBlockByID( plComponent::kBlkComp )->GetInt( kRefCurrIDSel );
        }
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIColorScheme Component /////////////////////////////////////////////////////////////
//
//  Defines the color scheme for a single control or an entire dialog
//
/////////////////////////////////////////////////////////////////////////////////////////////////


class plGUIColorSchemeProc : public ParamMap2UserDlgProc
{
protected:

    void    ILoadFonts( HWND hWnd, IParamBlock2 *pb );

    static int CALLBACK IMyFontEnumProc( const ENUMLOGFONTEX *logFontData, const NEWTEXTMETRICEX *physFontData,
                                                    unsigned long fontType, LPARAM lParam );

public:

    void DeleteThis() override { }

    void Update(TimeValue t, Interval &valid, IParamMap2 *map) override;

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

};

static plGUIColorSchemeProc gGUIColorSchemeProc;

// Class that accesses the paramblock below.
class plGUIColorSchemeComp : public plComponent
{
public:
    plGUIColorSchemeComp();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefForeColor = 128,        // So we can share it among other components
        kRefBackColor,
        kRefSelForeColor,
        kRefSelBackColor,
        kRefUseAlphas,
        kRefFontFace,
        kRefFontSize,
        kRefFontBold,
        kRefFontItalic,

        kRefForeAlpha,
        kRefBackAlpha,
        kRefSelForeAlpha,
        kRefSelBackAlpha,

        kRefFontShadowed
    };

    static void     ConvertScheme( IParamBlock2 *pb, pfGUIColorScheme *destScheme, plErrorMsg *pErrMsg );
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIColorSchemeComp, gGUIColorSchemeDesc, "GUI Color Scheme",  "GUIColorScheme", COMP_TYPE_GUI, GUI_COLORSCHEME_CLASSID )

static ParamBlockDesc2  gGUIColorSchemeBk
(
    /// Main def
     plComponent::kBlkComp, _T("GUIColorScheme"), 0, &gGUIColorSchemeDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

    1, 
    plGUIDialogComponent::kSchemeRollout, IDD_COMP_GUISCHEME, IDS_COMP_GUISCHEME, 0, 0, &gGUIColorSchemeProc,   

    plGUIColorSchemeComp::kRefForeColor,    _T("foreColor"),        TYPE_RGBA,      0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_FGCOLOR,
        p_default, Color( 1.f, 1.f, 1.f ),
        end,

    plGUIColorSchemeComp::kRefBackColor,    _T("backColor"),        TYPE_RGBA,      0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_BGCOLOR,
        p_default, Color( 0.f, 0.f, 0.f ),
        end,

    plGUIColorSchemeComp::kRefSelForeColor, _T("selForeColor"),     TYPE_RGBA,      0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_SFGCOLOR,
        p_default, Color( 1.f, 1.f, 1.f ),
        end,

    plGUIColorSchemeComp::kRefSelBackColor, _T("selBackColor"),     TYPE_RGBA,      0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_COLORSWATCH, IDC_GUI_SBGCOLOR,
        p_default, Color( 0.f, 0.f, 1.f ),
        end,


    plGUIColorSchemeComp::kRefForeAlpha,    _T("foreAlpha"),        TYPE_FLOAT,         0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_FGAEDIT, IDC_GUI_FGALPHA, 4,
        p_range, 0.f, 1.f,
        p_default, 1.f,
        end,

    plGUIColorSchemeComp::kRefBackAlpha,    _T("backAlpha"),        TYPE_FLOAT,         0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_BGAEDIT, IDC_GUI_BGALPHA, 4,
        p_range, 0.f, 1.f,
        p_default, 1.f,
        end,

    plGUIColorSchemeComp::kRefSelForeAlpha, _T("selForeAlpha"),     TYPE_FLOAT,         0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_SFGAEDIT, IDC_GUI_SFGALPHA, 4,
        p_range, 0.f, 1.f,
        p_default, 1.f,
        end,

    plGUIColorSchemeComp::kRefSelBackAlpha, _T("selBackAlpha"),     TYPE_FLOAT,         0, 0,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SLIDER, EDITTYPE_FLOAT, IDC_GUI_SBGAEDIT, IDC_GUI_SBGALPHA, 4,
        p_range, 0.f, 1.f,
        p_default, 1.f,
        end,


    plGUIColorSchemeComp::kRefUseAlphas, _T("useAlphas"),       TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_USEALPHAS,
        p_enable_ctrls, 4, plGUIColorSchemeComp::kRefForeAlpha, plGUIColorSchemeComp::kRefBackAlpha, 
                            plGUIColorSchemeComp::kRefSelForeAlpha, plGUIColorSchemeComp::kRefSelBackAlpha,
        end,        


    plGUIColorSchemeComp::kRefFontFace, _T("fontFace"),     TYPE_STRING,        0, 0,
        p_default,  _T( "Times New Roman" ),
        end,        

    plGUIColorSchemeComp::kRefFontSize, _T("fontSize"),     TYPE_INT,       0, 0,
        p_ui,   plGUIDialogComponent::kSchemeRollout, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_FONTSIZE, IDC_GUI_FONTSIZE_SPIN, SPIN_AUTOSCALE,
        p_default, 10,
        end,

    plGUIColorSchemeComp::kRefFontBold, _T("fontBold"),     TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_FONTBOLD,
        end,        

    plGUIColorSchemeComp::kRefFontItalic, _T("fontItalic"),     TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_FONTITALIC,
        end,        

    plGUIColorSchemeComp::kRefFontShadowed, _T("fontShadowed"),     TYPE_BOOL,      0, 0,
        p_default,  FALSE,
        p_ui, plGUIDialogComponent::kSchemeRollout, TYPE_SINGLECHEKBOX, IDC_GUI_FONTSHADOWED,
        end,        

    end
);

int CALLBACK plGUIColorSchemeProc::IMyFontEnumProc( const ENUMLOGFONTEX *logFontData, const NEWTEXTMETRICEX *physFontData,
                                                    unsigned long fontType, LPARAM lParam )
{
    HWND    combo = (HWND)lParam;


    if( SendMessage( combo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)logFontData->elfLogFont.lfFaceName ) == CB_ERR )
        SendMessage( combo, CB_ADDSTRING, 0, (LPARAM)logFontData->elfLogFont.lfFaceName );

    return -1;
}

void    plGUIColorSchemeProc::ILoadFonts( HWND hWnd, IParamBlock2 *pb )
{
    LOGFONT logFont;


    logFont.lfCharSet = DEFAULT_CHARSET;
    strcpy( logFont.lfFaceName, "" );
    logFont.lfPitchAndFamily = 0;

    SendMessage( hWnd, CB_RESETCONTENT, 0, 0 );

    HDC hDC = GetDC(nullptr);
    EnumFontFamiliesEx( hDC, &logFont, (FONTENUMPROC)IMyFontEnumProc, (LPARAM)hWnd, 0 );
    ReleaseDC(nullptr, hDC);

    SendMessage( hWnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pb->GetStr( plGUIColorSchemeComp::kRefFontFace ) );
}

#define MAXTOCOLORREF( max ) RGB( max.r * 255.f, max.g * 255.f, max.b * 255.f )

void    plGUIColorSchemeProc::Update( TimeValue t, Interval &valid, IParamMap2 *pmap )
{
}

INT_PTR plGUIColorSchemeProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char            str[ 256 ];
    HWND            placeCtrl;
    PAINTSTRUCT     paintInfo;
    RECT            previewRect, r;
    HBRUSH          bgPattBrush = nullptr;
    Color           fgColor, bgColor, selFgColor, selBgColor;
    Color           hatchColor = Color( 0.4f, 0.4f, 0.4f ), blendedColor, whiteColor = Color( 0.7f, 0.7f, 0.7f );
    Color           blackColor = Color( 0, 0, 0 ), blendedColor2;
    float           fgAlpha, bgAlpha, selFgAlpha, selBgAlpha;
    char            previewString[] = "Preview";
    HFONT           font;


    switch( msg )
    {
        case WM_INITDIALOG:
            ILoadFonts( GetDlgItem( hWnd, IDC_GUI_FONTFACE ), pmap->GetParamBlock() );           
            return TRUE;

        case WM_DESTROY:
            break;

        case WM_COMMAND:
            if( LOWORD( wParam ) == IDC_GUI_FONTFACE )
            {
                if( HIWORD( wParam ) == CBN_SELCHANGE )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_GUI_FONTFACE, CB_GETCURSEL, 0, 0);

                    SendDlgItemMessage( hWnd, IDC_GUI_FONTFACE, CB_GETLBTEXT, idx, (LPARAM)str );

                    pmap->GetParamBlock()->SetValue( plGUIColorSchemeComp::kRefFontFace, 0, str );
                }
            }
            break;

        case CC_COLOR_CHANGE:
        case CC_COLOR_DROP:
            placeCtrl = ::GetDlgItem( hWnd, IDC_GUI_SCHEMEPREV );
            ::GetClientRect( placeCtrl, &previewRect );
            ::MapWindowPoints( placeCtrl, hWnd, (POINT *)&previewRect, 2 );
            ::InvalidateRect( hWnd, &previewRect, FALSE );
            break;

        case WM_PAINT:

            fgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefForeColor );
            bgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefBackColor );
            selFgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefSelForeColor );
            selBgColor = pmap->GetParamBlock()->GetColor( plGUIColorSchemeComp::kRefSelBackColor );

            fgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefForeAlpha );
            bgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefBackAlpha );
            selFgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefSelForeAlpha );
            selBgAlpha = pmap->GetParamBlock()->GetFloat( plGUIColorSchemeComp::kRefSelBackAlpha );
            if( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefUseAlphas ) == 0 )
                fgAlpha = bgAlpha = selFgAlpha = selBgAlpha = 1.f;

            placeCtrl = ::GetDlgItem( hWnd, IDC_GUI_SCHEMEPREV );
            ::GetClientRect( placeCtrl, &previewRect );
            ::MapWindowPoints( placeCtrl, hWnd, (POINT *)&previewRect, 2 );

            ::BeginPaint( hWnd, &paintInfo );
            ::SetBkMode( paintInfo.hdc, TRANSPARENT );

            int weight = pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontBold ) ? FW_BOLD : FW_NORMAL;
            bool italic = pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontItalic ) ? true : false;
            int nHeight = -MulDiv( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontSize ), GetDeviceCaps( paintInfo.hdc, LOGPIXELSY ), 72 );
            const char *face = pmap->GetParamBlock()->GetStr( plGUIColorSchemeComp::kRefFontFace );

            font = ::CreateFont( nHeight, 0, 0, 0, weight, italic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, face );
            SelectObject( paintInfo.hdc, font );

            // Left side
            r = previewRect;
            r.right = ( r.right + r.left ) >> 1;

            blendedColor = bgColor * bgAlpha + ( whiteColor * ( 1.f - bgAlpha ) );

            // doesn't like the Color to DWORD operator, so duplicating it here
            #define ColorToDWORD(color) RGB(FLto255(color.r),FLto255(color.g),FLto255(color.b))
            ::SetBkColor( paintInfo.hdc, ColorToDWORD(blendedColor) );

            blendedColor = bgColor * bgAlpha + ( hatchColor * ( 1.f - bgAlpha ) );
            bgPattBrush = CreateHatchBrush( HS_DIAGCROSS, MAXTOCOLORREF( blendedColor ) );

            ::FillRect( paintInfo.hdc, &r, bgPattBrush );
            if( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontShadowed ) )
            {
                blendedColor2 = blackColor * fgAlpha + ( blendedColor * ( 1.f - fgAlpha ) );
                ::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor2 ) );
                ::OffsetRect( &r, 1, 1 );
                ::DrawTextA( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
                ::OffsetRect( &r, -1, -1 );
            }

            blendedColor = fgColor * fgAlpha + ( blendedColor * ( 1.f - fgAlpha ) );
            ::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor ) );
            ::DrawTextA( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
            
            ::DeleteObject( bgPattBrush );

            // Right side
            r.left = r.right;
            r.right = previewRect.right;

            blendedColor = selBgColor * selBgAlpha + ( whiteColor * ( 1.f - selBgAlpha ) );
            ::SetBkColor( paintInfo.hdc, ColorToDWORD(blendedColor) );
            blendedColor = selBgColor * selBgAlpha + ( hatchColor * ( 1.f - selBgAlpha ) );
            bgPattBrush = CreateHatchBrush( HS_DIAGCROSS, MAXTOCOLORREF( blendedColor ) );

            ::FillRect( paintInfo.hdc, &r, bgPattBrush );
            if( pmap->GetParamBlock()->GetInt( plGUIColorSchemeComp::kRefFontShadowed ) )
            {
                blendedColor2 = blackColor * selFgAlpha + ( blendedColor * ( 1.f - selFgAlpha ) );
                ::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor2 ) );
                ::OffsetRect( &r, 1, 1 );
                ::DrawTextA( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
                ::OffsetRect( &r, -1, -1 );
            }
            blendedColor = selFgColor * selFgAlpha + ( blendedColor * ( 1.f - selFgAlpha ) );
            ::SetTextColor( paintInfo.hdc, MAXTOCOLORREF( blendedColor ) );
            ::DrawTextA( paintInfo.hdc, previewString, strlen( previewString ), &r, DT_CENTER | DT_VCENTER );
            
            ::DeleteObject( bgPattBrush );

            ::DeleteObject( font );

            ::EndPaint( hWnd, &paintInfo );

            return TRUE;
    }
    return FALSE;
}

plGUIColorSchemeComp::plGUIColorSchemeComp()
{
    fClassDesc = &gGUIColorSchemeDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIColorSchemeComp::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return true;
}

bool plGUIColorSchemeComp::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{

    return true;
}

bool plGUIColorSchemeComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    pfGUIControlMod *ctrl = plGUIControlBase::GrabControlFromObject( node );
    if (ctrl != nullptr)
    {
        pfGUIColorScheme *cs = new pfGUIColorScheme;
        ConvertScheme( fCompPB, cs, pErrMsg );
        ctrl->SetColorScheme( cs );
    }
    else
    {
        pErrMsg->Set( true, "GUI Color Scheme Error", "You have applied a GUI color scheme to an object (%s) without a GUI control. This scheme will be ignored.", node->GetName()).Show();
        pErrMsg->Set( false );
        return false;   
    }

    return true;
}

void    SMaxRGBAToPlasmaRGBA( Color maxRGB, hsColorRGBA &plasmaRGBA )
{
    plasmaRGBA.Set( maxRGB.r, maxRGB.g, maxRGB.b, 1.f );
}

void    plGUIColorSchemeComp::ConvertScheme( IParamBlock2 *pb, pfGUIColorScheme *destScheme, plErrorMsg *pErrMsg )
{
    SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefForeColor ), destScheme->fForeColor );
    SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefBackColor ), destScheme->fBackColor );
    SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefSelForeColor ), destScheme->fSelForeColor );
    SMaxRGBAToPlasmaRGBA( pb->GetColor( kRefSelBackColor ), destScheme->fSelBackColor );

    destScheme->fForeColor.a = pb->GetFloat( kRefForeAlpha );
    destScheme->fBackColor.a = pb->GetFloat( kRefBackAlpha );
    destScheme->fSelForeColor.a = pb->GetFloat( kRefSelForeAlpha );
    destScheme->fSelBackColor.a = pb->GetFloat( kRefSelBackAlpha );

    destScheme->fTransparent = pb->GetInt( kRefUseAlphas ) ? true : false;
    
    destScheme->SetFontFace( pb->GetStr( kRefFontFace ) );
    destScheme->fFontSize = pb->GetInt( kRefFontSize );
    destScheme->fFontFlags = 0;
    if( pb->GetInt( kRefFontBold ) )
        destScheme->fFontFlags |= pfGUIColorScheme::kFontBold;
    if( pb->GetInt( kRefFontItalic ) )
        destScheme->fFontFlags |= pfGUIColorScheme::kFontItalic;
    if( pb->GetInt( kRefFontShadowed ) )
        destScheme->fFontFlags |= pfGUIColorScheme::kFontShadowed;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIProxy Rollout /////////////////////////////////////////////////////////////////////////
//
//  Defines a proxy object to be used when calculating mouse-down locations and dynamic text
//  sizing. 
//
/////////////////////////////////////////////////////////////////////////////////////////////////

enum plProxyRefs
{
    kRefProxyNode = 196,
    kRefHideProxy,
    kRefBetterHitTests
};

//// DialogProc /////////////////////////////////////////////////////////////////////////////////

class plGUIProxyDlgProc : public ParamMap2UserDlgProc
{
public:

    plGUIProxyDlgProc()
    {
    }

    void DeleteThis() override { }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        IParamBlock2 *pblock = map->GetParamBlock();

        switch( msg )
        {
            case WM_COMMAND:
//              if( LOWORD( wParam ) == IDC_GUI_CLEAR )
//              {
//                  pblock->Reset( (ParamID)kRefProxyNode );
//                  return TRUE;
//              }
                break;
        }   
        return FALSE;
    }

};  

static plGUIProxyDlgProc    sGUIProxyDlgProc;

//// ParamBlock /////////////////////////////////////////////////////////////////////////////////
//  Note: we can't make this a real ParamBlock and do P_INCLUDE_PARAMS because, in Discreet's 
//  amazing method of doing things, we can't INCLUDE more than one ParamBlock in any other PB.
//  So either we chain them together here (and thus make them dependent on one another, which
//  is lame) or we just make the whole damned thing a #define, which is all P_INCLUDE_PARAMS
//  really does anyway.

#define sGUIProxyParamHeader        plGUIControlBase::kRollProxy, IDD_COMP_GUIPROXY, IDS_COMP_GUIPROXY, 0, APPENDROLL_CLOSED, &sGUIProxyDlgProc
//static ParamBlockDesc2    sSndEAXPropsParamTemplate
//(
    /// Main def
//  plComponent::kBlkComp + 1, _T("sndEAXProps"), 0, nullptr, P_AUTO_UI + P_MULTIMAP + P_AUTO_CONSTRUCT, plComponent::kRefComp,

//  1, 
//  kSndEAXParams, IDD_COMP_EAXBUFFER, IDS_COMP_EAXBUFFER, 0, 0, nullptr,

#define sGUIProxyParamTemplate \
                                                                                                            \
    kRefBetterHitTests, _T("guiBetterHitTests"), TYPE_BOOL, 0, 0,                                                       \
        p_ui, plGUIControlBase::kRollProxy, TYPE_SINGLECHEKBOX, IDC_GUI_BETTERHIT,                          \
        p_default, false,                                                                                   \
        end                                                                                             

//  , end
//);


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDialog Component //////////////////////////////////////////////////////////////////////
//
//  Defines a dialog box (i.e. a collection of controls) to be defined with the GUI manager at
//  runtime. Acts a lot like a CamView component, but it additionally handles a few other things.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class plGUIDialogProc : public ParamMap2UserDlgProc
{
protected:

    void    ILoadPages( HWND hWnd, IParamBlock2 *pb );

public:

    void DeleteThis() override { }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};
static plGUIDialogProc gGUIDialogProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUIDialogComponent, gGUIDialogDesc, "GUI Dialog",  "GUIDialog", COMP_TYPE_GUI, GUI_DIALOG_COMP_CLASS_ID )

ParamBlockDesc2 gGUIDialogBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDialog"), 0, &gGUIDialogDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    3, 
    plGUIDialogComponent::kMainRollout,     IDD_COMP_GUIDIALOG, IDS_COMP_GUIDIALOG, 0, 0, &gGUIDialogProc,
    plGUIDialogComponent::kTagIDRollout,    IDD_COMP_GUITAG,    IDS_COMP_GUITAG,    0, 0, &gGUITagProc,
    plGUIDialogComponent::kSchemeRollout,   IDD_COMP_GUISCHEME, IDS_COMP_GUISCHEME, 0, 0, &gGUIColorSchemeProc, 

    &gGUIColorSchemeBk,

        plGUIDialogComponent::kRefDialogName,   _T("DialogName"),       TYPE_STRING,        0, 0,
//          p_ui, plGUIDialogComponent::kMainRollout, TYPE_EDITBOX, IDC_GUIDLG_NAME,
            end,

        plGUIDialogComponent::kRefAgeName,  _T("ageName"),      TYPE_STRING,        0, 0,
            p_default, _T( "GUI" ),
            end,
            
        plGUIDialogComponent::kRefIsModal, _T("isModal"),       TYPE_BOOL,      0, 0,
            p_default,  FALSE,
            p_ui, plGUIDialogComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_COMP_GUI_MODAL,
            end,        

        plGUIDialogComponent::kRefVersion,  _T("version"),      TYPE_INT,       0, 0,
            p_ui,   plGUIDialogComponent::kMainRollout, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_VERSION, IDC_GUI_VERSION_SPIN, SPIN_AUTOSCALE,
            p_default, 0,
            end,

        plGUITagComponent::kRefCurrIDSel,   _T("currSel"),      TYPE_INT,       0, 0,
            end,

    end
);

plGUIDialogComponent::plGUIDialogComponent( bool dontInit )
{
    if( !dontInit )
    {
        fClassDesc = &gGUIDialogDesc;
        fClassDesc->MakeAutoParamBlocks(this);
    }
    fDialogMod = nullptr;
    fProcReceiver = nullptr;
}

pfGUIDialogMod  *plGUIDialogComponent::IMakeDialog()
{
    return new pfGUIDialogMod();
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIDialogComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    TimeValue timeVal( 0 );
    Object* obj = node->EvalWorldState( timeVal ).obj;

    fDialogMod = nullptr;

    if( obj->CanConvertToType( Class_ID( LOOKAT_CAM_CLASS_ID, 0 ) ) ||
        obj->CanConvertToType( Class_ID( SIMPLE_CAM_CLASS_ID, 0 ) ) )
    {
        // We're applied to a camera. Do our camera stuff
        node->SetForceLocal( true );
    }
    else
    {
        // We're applied to a normal object.
        node->SetNoSpanReSort(true);
        node->SetNoSpanSort(true);
    }

    /// Either way, we mangle our own location component. None of this user-defined-location stuff.

    const char *dialogName = fCompPB->GetStr( kRefDialogName );
    if (dialogName == nullptr || *dialogName == 0)
    {
        pErrMsg->Set(true, "GUI Dialog Component Error", "No dialog name specified on GUI Dialog component (object: %s)", node->GetName()).Show();
        return false;   
    }

    const char *ageName = fCompPB->GetStr(kRefAgeName);
    int32_t seqNum = plPageInfoUtils::GetSeqNumFromAgeDesc( ageName, dialogName );
    int32_t newNum = plPluginResManager::ResMgr()->VerifySeqNumber( seqNum, ageName, dialogName );
    if( newNum != seqNum )
    {
        if( !fSeqNumValidated )
        {
            plLocation pageLoc = plPluginResManager::ResMgr()->FindLocation( ageName, dialogName );
            int32_t pageSeqNum = pageLoc.GetSequenceNumber();
            char errMsg[ 512 ];
            sprintf( errMsg, "The sequence number stored by the resource manager (0x%X) for page %s, District, %s does not match\n"
                            "the sequence number stored in the .age file (0x%X). Forcing it to use the one in the .age file", 
                                pageSeqNum, ageName, dialogName, seqNum );
            pErrMsg->Set( true, "PageInfo Convert Error", errMsg ).Show(); 
            pErrMsg->Set( false );
            fSeqNumValidated = true;
        }
        // force the component to use the sequence number in the .age file
        //seqNum = newNum;
    }

    plKey roomKey = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ), seqNum );
    if( !roomKey )
    {
        pErrMsg->Set( true, "GUI Dialog Component Error", "GUI Dialog Component %s has a Missing Location.  Nuke the files in the dat directory and re-export.",((INode*)node)->GetName()).Show(); 
        return false;
    }

    node->SetRoomKey( roomKey );

    // Also, we make sure this node will never be fogged (affects material convert)
    node->SetIsGUI( true );

    return true;
}

bool plGUIDialogComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
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
        // Not applied to a camera, so applied to a normal object. Since this is valid (we also act
        // as a location component), just return
        return true;
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

    // Should already be created from SetupProperties...
    // Note: can't just grab the node's room key, 'cause we might not be on the right node!
    plKey sceneNodeKey = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ), 
                                                        fCompPB->GetStr( kRefDialogName ), (uint32_t)-1 );
    mod->SetNodeKey( sceneNodeKey );

//  node->AddModifier(mod);
    // Note: we do NOT add this to the sceneObject, we don't want it actually associated with
    // a sceneObject. Instead, we just grab the LocalToWorld() off the sceneObject, since that's 
    // all we want
    hsMatrix44 l2w = node->GetLocalToWorld44();
    hsMatrix44 w2l = node->GetWorldToLocal44();
    mod->SetWorldToCamera( w2l, l2w );

    // Add it to the sceneNode as a generic interface, so it gets loaded with the sceneNode
    plLocation nodeLoc = sceneNodeKey->GetUoid().GetLocation();

    plKey modKey = hsgResMgr::ResMgr()->NewKey( ST::string::from_utf8( fCompPB->GetStr( kRefDialogName ) ), mod, nodeLoc );
    hsgResMgr::ResMgr()->AddViaNotify( modKey, new plNodeRefMsg( sceneNodeKey, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric ), plRefFlags::kActiveRef );

    // Also add our dialog mod to the scene node in the same way
    hsgResMgr::ResMgr()->AddViaNotify( fDialogMod->GetKey(), new plNodeRefMsg( sceneNodeKey, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric ), plRefFlags::kActiveRef );

    /// Already created our mod, just gotta fill it out
    fDialogMod->SetRenderMod( mod );
    fDialogMod->SetName( fCompPB->GetStr( kRefDialogName ) );
    if( fCompPB->GetInt( kRefIsModal ) )
        fDialogMod->SetFlag( pfGUIDialogMod::kModal );
    fDialogMod->SetProcReceiver(fProcReceiver);
    fDialogMod->SetVersion( fCompPB->GetInt( kRefVersion ) );

    plGUIColorSchemeComp::ConvertScheme( fCompPB, fDialogMod->GetColorScheme(), pErrMsg );

    return true;
}

bool plGUIDialogComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    TimeValue timeVal(0);
    Object* obj = node->EvalWorldState(timeVal).obj;

    if( obj->CanConvertToType(Class_ID(LOOKAT_CAM_CLASS_ID, 0))
        || obj->CanConvertToType(Class_ID(SIMPLE_CAM_CLASS_ID, 0)) )
    {
        // Don't do this. -mf
//      IMakeEveryoneOpaque(node);

        // Make a blank dialog modifier, which will be filled in on convert. Do 
        // this as a separate step so the dialog controls can query and get the dialog
        // mod to store a ref to

        fDialogMod = IMakeDialog();

        // Note: can't just grab the node's room key, 'cause we might not be on the right node!
        plKey sceneNodeKey = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ),
                                                            fCompPB->GetStr( kRefDialogName ), (uint32_t)-1 );

        plLocation nodeLoc = sceneNodeKey->GetUoid().GetLocation();
        plKey dlgKey = hsgResMgr::ResMgr()->NewKey( ST::string::from_utf8( fCompPB->GetStr( kRefDialogName ) ), fDialogMod, nodeLoc );

        fDialogMod->SetSceneNodeKey( sceneNodeKey );

        // See if there's a tag to be had
        uint32_t id = fCompPB->GetInt( plGUITagComponent::kRefCurrIDSel );
        if( id > 0 )
            fDialogMod->SetTagID( id );

        fProcReceiver = nullptr;
    }
    else
    {
    }

    return true;
}

void plGUIDialogComponent::IMakeEveryoneOpaque(plMaxNode* node)
{
    plMaxNode* root = (plMaxNode *)node->GetInterface()->GetRootNode();

    int i;
    for( i = 0; i < root->NumberOfChildren(); i++ )
        IMakeEveryoneOpaqueRecur((plMaxNode*)(root->GetChildNode(i)));

}

void plGUIDialogComponent::IMakeEveryoneOpaqueRecur(plMaxNode* node)
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

plKey   plGUIDialogComponent::GetModifierKey()
{
    if (fDialogMod != nullptr)
        return fDialogMod->GetKey();

    return nullptr;
}

bool    plGUIDialogComponent::SetNotifyReceiver( plKey key )
{
    if (fProcReceiver != nullptr)
        return false;

    fProcReceiver = key;
    return true;
}

pfGUIDialogMod  *plGUIDialogComponent::GetNodeDialog( plMaxNode *childNode )
{
    uint32_t i, numComp = childNode->NumAttachedComponents( false );
    for( i = 0; i < numComp; i++ )
    {
        plComponentBase *comp = childNode->GetAttachedComponent( i );
        if( comp->ClassID() == GUI_DIALOG_COMP_CLASS_ID )
            return ( (plGUIDialogComponent *)comp )->GetModifier();
    }

    return nullptr;
}

void    plGUIDialogProc::ILoadPages( HWND hWnd, IParamBlock2 *pb )
{
    plAgeDescription    *aged = plPageInfoUtils::GetAgeDesc( pb->GetStr( plGUIDialogComponent::kRefAgeName ) );

    if (aged == nullptr)
        return;

    plAgePage   *page;
    const char    *selPageName = pb->GetStr( plGUIDialogComponent::kRefDialogName );
    aged->SeekFirstPage();
    ComboBox_ResetContent( hWnd );

    while ((page = aged->GetNextPage()) != nullptr)
    {
        int idx = ComboBox_AddString( hWnd, page->GetName().c_str() );
        if( selPageName && page->GetName().compare_i( selPageName ) == 0 )
            ComboBox_SetCurSel( hWnd, idx );
    }

    delete aged;
}

INT_PTR plGUIDialogProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch( msg )
    {
        case WM_INITDIALOG:
            // Load the age combo box
            {
                int     idx, selIdx = 0;
                HWND    ageCombo = GetDlgItem( hWnd, IDC_GUIDLG_AGE );

                std::vector<plFileName> ageList = plAgeDescInterface::BuildAgeFileList();
                ComboBox_ResetContent( ageCombo );
                for (const plFileName& ageFile : ageList)
                {
                    ST::string ageName = ageFile.GetFileNameNoExt();

                    idx = ComboBox_AddString( ageCombo, ageName.c_str() );
                    if( ageName.compare_i( pmap->GetParamBlock()->GetStr( plGUIDialogComponent::kRefAgeName ) ) == 0 )
                    {
                        selIdx = idx;
                    }
                }
                ComboBox_SetCurSel( ageCombo, selIdx );
            }

            ILoadPages( GetDlgItem( hWnd, IDC_GUIDLG_NAME ), pmap->GetParamBlock() );           
            return TRUE;

        case WM_DESTROY:
            break;

        case WM_COMMAND:
            if( HIWORD( wParam ) == CBN_SELCHANGE )
            {
                if( LOWORD( wParam ) == IDC_GUIDLG_NAME )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_GUIDLG_NAME, CB_GETCURSEL, 0, 0);
                    if( idx != CB_ERR )
                    {
                        char    name[ 256 ];
                        ComboBox_GetLBText( GetDlgItem( hWnd, IDC_GUIDLG_NAME ), idx, name );
                        pmap->GetParamBlock()->SetValue( plGUIDialogComponent::kRefDialogName, 0, name );
                    }
                }
                else if( LOWORD( wParam ) == IDC_GUIDLG_AGE )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_GUIDLG_AGE, CB_GETCURSEL, 0, 0);
                    if( idx != CB_ERR )
                    {
                        char    name[ 256 ];
                        ComboBox_GetLBText( GetDlgItem( hWnd, IDC_GUIDLG_AGE ), idx, name );
                        pmap->GetParamBlock()->SetValue( plGUIDialogComponent::kRefAgeName, 0, name );
                    }

                    ILoadPages( GetDlgItem( hWnd, IDC_GUIDLG_NAME ), pmap->GetParamBlock() );           
                }
            }
            break;
    }
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIControl Component Base Class //////////////////////////////////////////////////////////
//
//  Defines a base class for all GUI control components.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void    plGUIControlBase::CollectNonDrawables( INodeTab &nonDrawables )
{
/*  if( ICanHaveProxy() )
    {
        bool hideProxy = fCompPB->GetInt( (ParamID)kRefHideProxy ) ? true : false;
        if( hideProxy )
        {
            INode   *node = fCompPB->GetINode( (ParamID)kRefProxyNode );
            if (node != nullptr)
                nonDrawables.Append( 1, &node );
        }
    }
*/
}

pfGUIDialogMod  *plGUIControlBase::IGetDialogMod( plMaxNode *node )
{
    uint32_t      i;


    for( i = 0; i < node->NumAttachedComponents( false ); i++ )
    {
        plComponentBase *comp = node->GetAttachedComponent( i, false );
        if( comp->ClassID() == GUI_DIALOG_COMP_CLASS_ID )
        {
            // Found it!
            pfGUIDialogMod  *dlgMod = ((plGUIDialogComponent *)comp)->GetModifier();
            return dlgMod;
        }
    }

    return nullptr;
}

bool plGUIControlBase::SetupProperties( plMaxNode *pNode, plErrorMsg *pErrMsg )
{
    if( INeedsDynamicText() )
    {
        // If we're going to be using a dynamic text layer, we need to make sure the material
        // is unique for every node we're applied to
        pNode->SetForceMaterialCopy( true );
    }

    return true;
}

bool plGUIControlBase::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    // Create a new control
    fControl = IGetNewControl();

    // Add it as a modifier to this node
    node->AddModifier( fControl, IGetUniqueName(node) );

    // Look for any tag IDs
    uint32_t id = plGUITagComponent::GetTagIDOnNode( node );
    if( id > 0 )
        fControl->SetTagID( id );

    // Now add it to our list of converted nodes
    auto iter = std::find(fTargetNodes.cbegin(), fTargetNodes.cend(), node);
    if (iter == fTargetNodes.cend())
    {
        fTargetNodes.emplace_back(node);
        fTargetControls.emplace_back(fControl);
    }
    else
    {
        auto idx = iter - fTargetNodes.cbegin();
        fTargetControls[idx] = fControl;
    }

    return true;
}

bool plGUIControlBase::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    // Error check--make sure we're in the same room as our parent (can get confusing with the wrong
    // parent-child relationships)
    if( !node->GetParentNode()->IsRootNode() )
    {
        plMaxNode *parent = (plMaxNode *)node->GetParentNode();
        if( parent->GetRoomKey() != node->GetRoomKey() )
        {
            pErrMsg->Set( true, "GUI Control Component Error", "The object %s is assigned to a different GUI dialog than its parent. Make sure both this object and its parent belong to the same GUI dialog (this control will be ignored).", node->GetName() ).Show(); 
            pErrMsg->Set( false );
            return false;
        }
    }

    pfGUIDialogMod *dialog = IGetDialogMod( node );
    if (dialog == nullptr)
    {
        pErrMsg->Set( true, "GUI Control Component Error", "The object %s has a GUI control applied but not a GUI Dialog Component. Apply a GUI Dialog Component to this object.", node->GetName() ).Show(); 
        pErrMsg->Set( false );
        return false;
    }

    // Grab fControl from the modifier list on the node, since fControl isn't valid
    // between PreConvert() and Convert() (it might get called multiple times, once per node applied)
    auto iter = std::find(fTargetNodes.cbegin(), fTargetNodes.cend(), node);
    if (iter == fTargetNodes.cend())
    {
        pErrMsg->Set( true, "GUI Control Component Error", "The object %s somehow skipped the GUI control Pre-convert stage. Inform a programmer immediately and seek shelter.", node->GetName() ).Show(); 
        pErrMsg->Set( false );
        return false;
    }
    
    auto idx = iter - fTargetNodes.cbegin();
    fControl = fTargetControls[idx];

    dialog->AddControlOnExport( fControl );

    if( IHasProcRollout() )
    {
        // Also common for all controls: process the Procedure rollout--i.e. what kind of control proc do we get?
        switch( fCompPB->GetInt( kRefChoice ) )
        {
            case 0:
                // Console command
                fControl->SetHandler( new pfGUIConsoleCmdProc( fCompPB->GetStr( kRefConsoleCmd ) ) );
                break;

            case 1:
                // Inherit from parent dialog - this is a runtime flag, so we don't bother actually setting
                // a handler here, except to ensure it's nil
                fControl->SetHandler(nullptr);
                fControl->SetFlag( pfGUIControlMod::kInheritProcFromDlg );
                break;

            case 2:
                fControl->SetHandler( new pfGUICloseDlgProc() );
                break;

            case 3:
                // Do nothing. Just set a nil proc, but do NOT inherit from the dialog
                fControl->SetHandler(nullptr);
                fControl->ClearFlag( pfGUIControlMod::kInheritProcFromDlg );
                break;
        }
    }

    if( INeedsDynamicText() )
    {
        // We're a control that dynamically creates text, so look for the first dynamic layer 
        // (and hopefully the ONLY one) and store it on the control
        Mtl *maxMaterial = hsMaterialConverter::Instance().GetBaseMtl( node );
        std::vector<plExportMaterialData> *mtlArray = hsMaterialConverter::Instance().CreateMaterialArray(maxMaterial, node, 0);
        
        plDynamicTextMap *dynText = nullptr;
        plLayerInterface *layerIFace = nullptr;

        for (size_t i = 0; i < mtlArray->size() && dynText == nullptr; i++)
        {
            hsGMaterial *plasmaMat = (*mtlArray)[ 0 ].fMaterial;

            for (size_t j = 0; j < plasmaMat->GetNumLayers(); j++)
            {
                layerIFace = plasmaMat->GetLayer( j );
                dynText = plDynamicTextMap::ConvertNoRef( layerIFace->GetTexture() );
                if (dynText != nullptr)
                    break;
            }
        }

        if (dynText == nullptr)
        {
            pErrMsg->Set( true, "GUI Component Error", "The object %s needs a Plasma Dynamic Text Layer in its material. "
                "This control will not function properly until you apply one.", node->GetName() ).Show();
            pErrMsg->Set( false );
        }
        else
            fControl->SetDynTextMap( layerIFace, dynText );

        delete mtlArray;
    }

    if( ICanHaveProxy() )
    {
        // No proxy objects just yet, just options for better hit testing
        if( fCompPB->GetInt( kRefBetterHitTests ) )
            fControl->SetFlag( pfGUIControlMod::kBetterHitTesting );
    }

    return true;
}

pfGUIControlMod *plGUIControlBase::GrabControlFromObject( INode *node )
{
    uint32_t  i;
    plMaxNodeBase   *maxNode = (plMaxNodeBase *)node;


    for( i = 0; i < maxNode->NumAttachedComponents( false ); i++ )
    {
        plComponentBase *comp = maxNode->GetAttachedComponent( i, false );
        pfGUIControlMod *ctrl = ConvertCompToControl( comp, maxNode );
        if (ctrl != nullptr)
            return ctrl;
    }

    return nullptr;
}

// Given an INode, gives you a pointer to the GUI component if it actually is one, nil otherwise
plGUIControlBase    *plGUIControlBase::GetGUIComp( INode *node )
{
    if (node == nullptr)
        return nullptr;

    return GetGUIComp( ( ( plMaxNodeBase *)node )->ConvertToComponent() );
}

plGUIControlBase    *plGUIControlBase::GetGUIComp( plComponentBase *comp )
{
    if (comp == nullptr)
        return nullptr;

    if( comp->ClassID() == GUI_UPDOWNPAIR_CLASSID ||
        comp->ClassID() == GUI_BUTTON_CLASSID ||
        comp->ClassID() == GUI_DRAGGABLE_CLASSID ||
        comp->ClassID() == GUI_LISTBOX_CLASSID ||
        comp->ClassID() == GUI_TEXTBOX_CLASSID ||
        comp->ClassID() == GUI_EDITBOX_CLASSID ||
        comp->ClassID() == GUI_KNOBCTRL_CLASSID ||
        comp->ClassID() == GUI_DRAGBAR_CLASSID ||
        comp->ClassID() == GUI_CHECKBOX_CLASSID ||
        comp->ClassID() == GUI_RADIOGROUP_CLASSID ||
        comp->ClassID() == GUI_DYNDISPLAY_CLASSID ||
        comp->ClassID() == GUI_MULTILINE_CLASSID ||
        comp->ClassID() == GUI_PROGRESS_CLASSID ||
        comp->ClassID() == GUI_CLICKMAP_CLASSID )
    {
        return (plGUIControlBase *)comp;
    }

    return nullptr;
}

pfGUIControlMod *plGUIControlBase::GrabControlMod( INode *node, INode *sceneObjectNode )
{
    if (node == nullptr)
        return nullptr;

    plComponentBase *comp = ( ( plMaxNodeBase *)node )->ConvertToComponent();
    return ConvertCompToControl( comp, sceneObjectNode );
}

pfGUIControlMod *plGUIControlBase::ConvertCompToControl( plComponentBase *comp, INode *sceneObjectNode )
{
    plGUIControlBase    *base = GetGUIComp( comp );
    if (base != nullptr)
    {
        if (sceneObjectNode == nullptr)
        {
            // Not good, but if you select a component like this, it better only be applied to one object,
            // hence will only have one fTargetControl
            if (!base->fTargetControls.empty())
                return base->fTargetControls[ 0 ];
        }
        else
        {
            auto iter = std::find(base->fTargetNodes.cbegin(), base->fTargetNodes.cend(), (plMaxNode *)sceneObjectNode);
            if (iter == base->fTargetNodes.cend())
                return nullptr;

            auto idx = iter - base->fTargetNodes.cbegin();
            return base->fTargetControls[idx];
        }
    }

    return nullptr;
}

const char  *plGUIControlBase::ISetSoundIndex( ParamID checkBoxID, ParamID sndCompID, uint8_t guiCtrlEvent, plMaxNode *maxNode )
{
    if( fCompPB->GetInt( checkBoxID ) )
    {
        plMaxNode *sndNode = (plMaxNode *)fCompPB->GetReferenceTarget( sndCompID );
        if (sndNode != nullptr)
        {
            plComponentBase *comp = sndNode->ConvertToComponent();
            if (comp != nullptr)
            {
                int idx = plAudioComp::GetSoundModIdx( comp, maxNode );
                if( idx != -1 )
                {
                    fControl->SetSoundIndex( guiCtrlEvent, idx );
                    return nullptr;
                }
                else
                    return "The selected sound component could not be found on GUI control %s. Make sure you have a sound component on the same object selected.";
            }
            else
                return "The selected sound node on GUI control %s could not be converted to a component. Make sure you have a sound component selected.";
        }
        else
            return "The GUI control %s has a sound event enabled but no sound component selected. Make sure you have a sound component on the same object selected.";
    }

    return nullptr;
}


//// ParamBlock for Control Proc Rollout ////////////////////////////////////////////////////////

static ParamBlockDesc2  sGUIControlProcParamTemplate
(
    /// Main def
    plGUIControlBase::kBlkProc, _T("GUIControlProc"), 0, nullptr, P_AUTO_UI + P_MULTIMAP + P_AUTO_CONSTRUCT, plComponent::kRefComp,

    1, 
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,

    plGUIControlBase::kRefChoice,   _T("which"), TYPE_INT,      0, 0,
        p_ui,   plGUIControlBase::kRollProc, TYPE_RADIO, 4, IDC_GUI_CONRADIO, IDC_GUI_INHERITRADIO, IDC_GUI_CLOSERADIO, IDC_GUI_NILRADIO,
        p_default, 1,
        end,

    plGUIControlBase::kRefConsoleCmd,   _T("ConsoleCmd"),       TYPE_STRING,        0, 0,
        p_ui,   plGUIControlBase::kRollProc, TYPE_EDITBOX, IDC_GUI_CONCMD,
        end,

    end
);


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIButton Component //////////////////////////////////////////////////////////////////////
//
//  Defines a dialog button to be defined with the GUI manager at runtime. Belongs to exactly 
//  one dialog, defined by parent-child relationship, also at runtime.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

class plGUIButtonProc : public ParamMap2UserDlgProc
{
public:

    void DeleteThis() override { }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};


static plGUIButtonProc gGUIButtonProc;


// Class that accesses the paramblock below.
class plGUIButtonComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIButtonMod; }
    bool            ICanHaveProxy() override { return true; }

public:
    plGUIButtonComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefConCmdRadio,
        kRefPythonRadio,
        kRefConsoleCmd,
        kRefAnimate,
        kRefAnimation,
        kRefMouseOverAnimate,
        kRefMouseOverAnimation,
        kRefMouseDownSound,
        kRefMouseDownSoundComp,
        kRefMouseUpSound,
        kRefMouseUpSoundComp,
        kRefMouseOverSound,
        kRefMouseOverSoundComp,
        kRefMouseOffSound,
        kRefMouseOffSoundComp,
        kRefAnimationNode,
        kRefAnimationNodeType,
        kRefMouseOverAnimationNode,
        kRefMouseOverAnimationNodeType,
        kRefDraggableChild,
        kRefUseDraggableChild,
        kRefNotifyType
    };
};

INT_PTR plGUIButtonProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch( msg )
    {
        case WM_INITDIALOG:
            SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_RESETCONTENT, 0, 0 );
            SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_ADDSTRING, 0, (LPARAM)"Button Up" );
            SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_ADDSTRING, 0, (LPARAM)"Button Down" );
            SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_ADDSTRING, 0, (LPARAM)"Button Down and Up" );
            SendMessage( GetDlgItem( hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE ), CB_SETCURSEL, pmap->GetParamBlock()->GetInt( plGUIButtonComponent::kRefNotifyType ), 0 );
            return TRUE;

        case WM_DESTROY:
            break;

        case WM_COMMAND:
            if( LOWORD( wParam ) == IDC_COMBO_BUTTON_NOTIFYTYPE )
            {
                if( HIWORD( wParam ) == CBN_SELCHANGE )
                {
                    int idx = (int)SendDlgItemMessage(hWnd, IDC_COMBO_BUTTON_NOTIFYTYPE, CB_GETCURSEL, 0, 0);
                    pmap->GetParamBlock()->SetValue( plGUIButtonComponent::kRefNotifyType, 0, idx );
                }
            }
            break;
    }
    return FALSE;
}

class plGUIButtonAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if( id == plGUIButtonComponent::kRefAnimation ||
            id == plGUIButtonComponent::kRefMouseOverAnimation ||
            id == plGUIButtonComponent::kRefMouseDownSoundComp ||
            id == plGUIButtonComponent::kRefMouseUpSoundComp ||
            id == plGUIButtonComponent::kRefMouseOverSoundComp ||
            id == plGUIButtonComponent::kRefMouseOffSoundComp ||
            id == plGUIButtonComponent::kRefAnimationNode ||
            id == plGUIButtonComponent::kRefMouseOverAnimationNode )
        {
            plGUIButtonComponent *comp = (plGUIButtonComponent *)owner;
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
        }
    }
};

Class_ID    sBtnClassesToSelect[] = { ANIM_COMP_CID, ANIM_GROUP_COMP_CID, plGUISingleCtrlDlgProc::kEndClassList };
Class_ID    sBtnSndClassesToSelect[] = { GUI_SOUND_COMPONENT_ID, plGUISingleCtrlDlgProc::kEndClassList };

Class_ID    sBtnDragClassesToSelect[] = { GUI_DRAGGABLE_CLASSID, plGUISingleCtrlDlgProc::kEndClassList };

static plGUIButtonAccessor sGUIButtonAccessor;

static plGUISingleCtrlDlgProc sGUIButtonSndAProc( plGUIButtonComponent::kRefMouseDownSoundComp, IDC_GUI_MDOWNSNDCOMP,
                                            "Select the sound to play when the mouse clicks this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUIButtonSndBProc( plGUIButtonComponent::kRefMouseUpSoundComp, IDC_GUI_MUPSNDCOMP,
                                            "Select the sound to play when the mouse lets up on this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUIButtonSndCProc( plGUIButtonComponent::kRefMouseOverSoundComp, IDC_GUI_MOVERSNDCOMP,
                                            "Select the sound to play when the mouse moves over this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUIButtonSndDProc( plGUIButtonComponent::kRefMouseOffSoundComp, IDC_GUI_MOFFSNDCOMP,
                                            "Select the sound to play when the mouse moves off of this button", sBtnSndClassesToSelect );

static plGUISingleCtrlDlgProc sGUIButtonDragChildProc( plGUIButtonComponent::kRefDraggableChild, IDC_GUI_DRAGCHILD,
                                            "Select the draggable to use when the mouse is dragged off of this button", sBtnDragClassesToSelect );

static plGUISingleCtrlDlgProc   *sGUIButtonSubProcs[] = { &sGUIButtonSndAProc, &sGUIButtonSndBProc, 
                                                          &sGUIButtonSndCProc, &sGUIButtonSndDProc, 
                                                          &sGUIButtonDragChildProc, nullptr };
static ParamMap2UserDlgProc *sGUIButtonSubSubProcs[] = { &gGUIButtonProc, nullptr };

static plGUIMultipleCtrlDlgProc sGUIButtonSels( sGUIButtonSubProcs, sGUIButtonSubSubProcs );

static plPlasmaAnimSelectDlgProc    sGUIButtonAnimA( plGUIButtonComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
                                                    plGUIButtonComponent::kRefAnimationNode, plGUIButtonComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
                                                    "Select the animation to play when this button is clicked", &sGUIButtonSels );
static plPlasmaAnimSelectDlgProc    sGUIButtonProc( plGUIButtonComponent::kRefMouseOverAnimation, IDC_GUI_COMPSELBTN2, 
                                                    plGUIButtonComponent::kRefMouseOverAnimationNode, plGUIButtonComponent::kRefMouseOverAnimationNodeType, IDC_GUI_ANIMNODESEL2, 
                                                    "Select the animation to play when the mouse moves over this button", &sGUIButtonAnimA );


#define GUI_SOUND_REF( comp, evt, allCapsEvt )      \
        comp##::kRefMouse##evt##Sound,  _T( "mouse##evt##Sound" ), TYPE_BOOL, 0, 0,                 \
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_M##allCapsEvt##SND,      \
            p_default, FALSE,                                                                       \
            p_enable_ctrls, 1, comp##::kRefMouse##evt##SoundComp,                                   \
            end,                                                                                    \
        comp##::kRefMouse##evt##SoundComp, _T("mouse##evt##SoundComp"), TYPE_INODE,     0, 0,       \
            p_accessor, &sGUIButtonAccessor,                                                        \
            end

//Max desc stuff necessary below.
CLASS_DESC(plGUIButtonComponent, gGUIButtonDesc, "GUI Button",  "GUIButton", COMP_TYPE_GUI, GUI_BUTTON_CLASSID )

ParamBlockDesc2 gGUIButtonBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIButton"), 0, &gGUIButtonDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_INCLUDE_PARAMS + P_MULTIMAP, plComponent::kRefComp,

    3,
    plGUIControlBase::kRollMain, IDD_COMP_GUIBUTTON, IDS_COMP_GUIBUTTON, 0, 0, &sGUIButtonProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    sGUIProxyParamHeader,

    &sGUIControlProcParamTemplate,

        plGUIButtonComponent::kRefAnimate,  _T( "animate" ), TYPE_BOOL, 0, 0,
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMATE,
            p_default, FALSE,
            p_enable_ctrls, 1, plGUIButtonComponent::kRefAnimation,
            end,

        plGUIButtonComponent::kRefAnimation, _T("animation"),   TYPE_INODE,     0, 0,
            p_prompt, IDS_COMP_GUI_SELECTANIM,
            p_accessor, &sGUIButtonAccessor,
            end,

        plGUIButtonComponent::kRefMouseOverAnimate, _T( "mouseOverAnimate" ), TYPE_BOOL, 0, 0,
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_MOUSEOVERANIM,
            p_default, FALSE,
            p_enable_ctrls, 1, plGUIButtonComponent::kRefMouseOverAnimation,
            end,

        plGUIButtonComponent::kRefMouseOverAnimation, _T("mouseOverAnimation"), TYPE_INODE,     0, 0,
            p_prompt, IDS_COMP_GUI_SELECTMOUSEOVERANIM,
            p_accessor, &sGUIButtonAccessor,
            end,

        GUI_SOUND_REF( plGUIButtonComponent, Down, DOWN ),
        GUI_SOUND_REF( plGUIButtonComponent, Up, UP ),
        GUI_SOUND_REF( plGUIButtonComponent, Over, OVER ),
        GUI_SOUND_REF( plGUIButtonComponent, Off, OFF ),

    sGUIProxyParamTemplate,

        plGUIButtonComponent::kRefAnimationNode, _T("animationNode"),   TYPE_INODE,     0, 0,
            p_accessor, &sGUIButtonAccessor,
            end,

        plGUIButtonComponent::kRefAnimationNodeType, _T("animationNodeType"),   TYPE_INT,       0, 0,
            p_default, plAnimObjInterface::kUseOwnerNode,
            end,

        plGUIButtonComponent::kRefMouseOverAnimationNode, _T("moAnimationNode"),    TYPE_INODE,     0, 0,
            p_accessor, &sGUIButtonAccessor,
            end,

        plGUIButtonComponent::kRefMouseOverAnimationNodeType, _T("moAnimationNodeType"),    TYPE_INT,       0, 0,
            p_default, plAnimObjInterface::kUseOwnerNode,
            end,

        plGUIButtonComponent::kRefUseDraggableChild,    _T( "useDragChild" ), TYPE_BOOL, 0, 0,
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_USEDRAGCHILD,
            p_default, FALSE,
            p_enable_ctrls, 1, plGUIButtonComponent::kRefDraggableChild,
            end,

        plGUIButtonComponent::kRefDraggableChild,   _T( "dragChild" ), TYPE_INODE, 0, 0,
            p_accessor, &sGUIButtonAccessor,
            end,

        plGUIButtonComponent::kRefNotifyType,   _T("notifyType"),       TYPE_INT,       0, 0,
            p_default, pfGUIButtonMod::kNotifyOnUp,
            end,
    end
);

plGUIButtonComponent::plGUIButtonComponent()
{
    fClassDesc = &gGUIButtonDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIButtonComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    if( fCompPB->GetInt( kRefAnimate ) )
    {
        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
        if (iface != nullptr && iface->MightRequireSeparateMaterial())
        {
            INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                    ? fCompPB->GetINode( kRefAnimationNode )
                                    : (INode *)node;

            if (restrict != nullptr)
            {
                node->SetForceMaterialCopy( true );
            }
        }
    }

    if( fCompPB->GetInt( kRefMouseOverAnimate ) )
    {
        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefMouseOverAnimation ) );
        if (iface != nullptr && iface->MightRequireSeparateMaterial())
        {
            INode *restrict = ( fCompPB->GetInt( kRefMouseOverAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                    ? fCompPB->GetINode( kRefMouseOverAnimationNode )
                                    : (INode *)node;

            if (restrict != nullptr)
            {
                node->SetForceMaterialCopy( true );
            }
        }
    }

    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIButtonComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIButtonComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIButtonMod *button = (pfGUIButtonMod *)fControl;

    // set the notify type
    button->SetNotifyType(fCompPB->GetInt( kRefNotifyType ));

    if( fCompPB->GetInt( kRefAnimate ) )
    {
        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
        if (iface != nullptr)
        {
            INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                    ? fCompPB->GetINode( kRefAnimationNode )
                                    : (INode *)node;


            std::vector<plKey> keys;
            if (iface->GetKeyList(restrict, keys) && !keys.empty())
                button->SetAnimationKeys(keys, iface->GetIfaceSegmentName(false));
        }
    }

    if( fCompPB->GetInt( kRefMouseOverAnimate ) )
    {
        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefMouseOverAnimation ) );
        if (iface != nullptr)
        {
            INode *restrict = ( fCompPB->GetInt( kRefMouseOverAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                    ? fCompPB->GetINode( kRefMouseOverAnimationNode )
                                    : (INode *)node;

            std::vector<plKey> keys;
            if (iface->GetKeyList(restrict, keys) && !keys.empty())
                button->SetMouseOverAnimKeys( keys, iface->GetIfaceSegmentName( false ) );
        }
    }

    // Do sound stuff
    const char *errMsg1 = ISetSoundIndex( kRefMouseDownSound, kRefMouseDownSoundComp, pfGUIButtonMod::kMouseDown, node );
    const char *errMsg2 = ISetSoundIndex( kRefMouseUpSound, kRefMouseUpSoundComp, pfGUIButtonMod::kMouseUp, node );
    const char *errMsg3 = ISetSoundIndex( kRefMouseOverSound, kRefMouseOverSoundComp, pfGUIButtonMod::kMouseOver, node );
    const char *errMsg4 = ISetSoundIndex( kRefMouseOffSound, kRefMouseOffSoundComp, pfGUIButtonMod::kMouseOff, node );

    const char *errMsg = (errMsg1 != nullptr) ? errMsg1 : (errMsg2 != nullptr) ? errMsg2 : (errMsg3 != nullptr) ? errMsg3 : errMsg4;
    if (errMsg != nullptr)
    {
        pErrMsg->Set( true, "GUI Sound Event Error", errMsg, node->GetName() ).Show();
        pErrMsg->Set( false );
    }

    if( fCompPB->GetInt( kRefUseDraggableChild ) )
    {
        pfGUIDraggableMod *dragChild = pfGUIDraggableMod::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefDraggableChild ) ) );
        if (dragChild != nullptr)
        {
            hsgResMgr::ResMgr()->AddViaNotify( dragChild->GetKey(), 
                                new plGenRefMsg( button->GetKey(), plRefMsg::kOnCreate, -1, pfGUIButtonMod::kRefDraggable ), plRefFlags::kActiveRef );
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUICheckBox Component //////////////////////////////////////////////////////////////////////
//
//  Defines a dialog button to be defined with the GUI manager at runtime. Belongs to exactly 
//  one dialog, defined by parent-child relationship, also at runtime.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUICheckBoxComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUICheckBoxCtrl; }
    bool            ICanHaveProxy() override { return true; }

public:
    plGUICheckBoxComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefConCmdRadio,
        kRefPythonRadio,
        kRefConsoleCmd,
        kRefAnimate,
        kRefAnimation,
        kRefAnimationNode,
        kRefAnimationNodeType,
        kRefMouseDownSound,
        kRefMouseDownSoundComp,
        kRefMouseUpSound,
        kRefMouseUpSoundComp,
        kRefMouseOverSound,
        kRefMouseOverSoundComp,
        kRefMouseOffSound,
        kRefMouseOffSoundComp,
    };
};

class plGUICheckBoxAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if( id == plGUICheckBoxComponent::kRefAnimation ||
            id == plGUICheckBoxComponent::kRefMouseDownSoundComp ||
            id == plGUICheckBoxComponent::kRefMouseUpSoundComp ||
            id == plGUICheckBoxComponent::kRefMouseOverSoundComp ||
            id == plGUICheckBoxComponent::kRefMouseOffSoundComp )
        {
            plGUICheckBoxComponent *comp = (plGUICheckBoxComponent *)owner;
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
        }
    }
};
static plGUICheckBoxAccessor    sGUICheckBoxAccessor;

static plGUISingleCtrlDlgProc sGUICheckSndAProc( plGUICheckBoxComponent::kRefMouseDownSoundComp, IDC_GUI_MDOWNSNDCOMP,
                                            "Select the sound to play when the mouse clicks this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUICheckSndBProc( plGUICheckBoxComponent::kRefMouseUpSoundComp, IDC_GUI_MUPSNDCOMP,
                                            "Select the sound to play when the mouse lets up on this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUICheckSndCProc( plGUICheckBoxComponent::kRefMouseOverSoundComp, IDC_GUI_MOVERSNDCOMP,
                                            "Select the sound to play when the mouse moves over this button", sBtnSndClassesToSelect );
static plGUISingleCtrlDlgProc sGUICheckSndDProc( plGUICheckBoxComponent::kRefMouseOffSoundComp, IDC_GUI_MOFFSNDCOMP,
                                            "Select the sound to play when the mouse moves off of this button", sBtnSndClassesToSelect );

static plGUISingleCtrlDlgProc   *sGUICheckSubProcs[] = { &sGUICheckSndAProc, &sGUICheckSndBProc, 
                                                         &sGUICheckSndCProc, &sGUICheckSndDProc, nullptr };

static plGUIMultipleCtrlDlgProc sGUICheckSels( sGUICheckSubProcs );

static plPlasmaAnimSelectDlgProc    sGUICheckBoxProc( plGUICheckBoxComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
                                                    plGUICheckBoxComponent::kRefAnimationNode, plGUICheckBoxComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
                                                    "Select the animation to play when this check box is clicked", &sGUICheckSels );

//Max desc stuff necessary below.
CLASS_DESC(plGUICheckBoxComponent, gGUICheckBoxDesc, "GUI CheckBox",  "GUICheckBox", COMP_TYPE_GUI, GUI_CHECKBOX_CLASSID )

ParamBlockDesc2 gGUICheckBoxBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUICheckBox"), 0, &gGUICheckBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    3,
    plGUIControlBase::kRollMain, IDD_COMP_GUIBUTTON, IDS_COMP_GUICHECKBOX, 0, 0, &sGUICheckBoxProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    sGUIProxyParamHeader,

    &sGUIControlProcParamTemplate,

        plGUICheckBoxComponent::kRefAnimate,    _T( "animate" ), TYPE_BOOL, 0, 0,
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMATE,
            p_default, FALSE,
            p_enable_ctrls, 1, plGUIButtonComponent::kRefAnimation,
            end,

        plGUICheckBoxComponent::kRefAnimation, _T("animation"), TYPE_INODE,     0, 0,
            p_prompt, IDS_COMP_GUI_SELECTANIM,
            p_accessor, &sGUICheckBoxAccessor,
            end,

    sGUIProxyParamTemplate,

        plGUICheckBoxComponent::kRefAnimationNode, _T("animationNode"), TYPE_INODE,     0, 0,
            p_accessor, &sGUIButtonAccessor,
            end,

        plGUICheckBoxComponent::kRefAnimationNodeType, _T("animationNodeType"), TYPE_INT,       0, 0,
            p_default, plAnimObjInterface::kUseOwnerNode,
            end,

        GUI_SOUND_REF( plGUICheckBoxComponent, Down, DOWN ),
        GUI_SOUND_REF( plGUICheckBoxComponent, Up, UP ),
        GUI_SOUND_REF( plGUICheckBoxComponent, Over, OVER ),
        GUI_SOUND_REF( plGUICheckBoxComponent, Off, OFF ),

    end
);

plGUICheckBoxComponent::plGUICheckBoxComponent()
{
    fClassDesc = &gGUICheckBoxDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUICheckBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    if( fCompPB->GetInt( kRefAnimate ) )
    {
        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
        if (iface != nullptr && iface->MightRequireSeparateMaterial())
        {
            INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                    ? fCompPB->GetINode( kRefAnimationNode )
                                    : (INode *)node;

            if (restrict != nullptr)
            {
                node->SetForceMaterialCopy( true );
            }
        }
    }

    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUICheckBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUICheckBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUICheckBoxCtrl *button = (pfGUICheckBoxCtrl *)fControl;
    
    if( fCompPB->GetInt( kRefAnimate ) )
    {
        plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
        if (iface != nullptr)
        {
            INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                    ? fCompPB->GetINode( kRefAnimationNode )
                                    : (INode *)node;


            std::vector<plKey> keys;
            if (iface->GetKeyList(restrict, keys) && !keys.empty())
                button->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
        }
    }

    // Do sound stuff
    const char *errMsg1 = ISetSoundIndex( kRefMouseDownSound, kRefMouseDownSoundComp, pfGUICheckBoxCtrl::kMouseDown, node );
    const char *errMsg2 = ISetSoundIndex( kRefMouseUpSound, kRefMouseUpSoundComp, pfGUICheckBoxCtrl::kMouseUp, node );
    const char *errMsg3 = ISetSoundIndex( kRefMouseOverSound, kRefMouseOverSoundComp, pfGUICheckBoxCtrl::kMouseOver, node );
    const char *errMsg4 = ISetSoundIndex( kRefMouseOffSound, kRefMouseOffSoundComp, pfGUICheckBoxCtrl::kMouseOff, node );

    const char *errMsg = (errMsg1 != nullptr) ? errMsg1 : (errMsg2 != nullptr) ? errMsg2 : (errMsg3 != nullptr) ? errMsg3 : errMsg4;
    if (errMsg != nullptr)
    {
        pErrMsg->Set( true, "GUI Sound Event Error", errMsg, node->GetName() ).Show();
        pErrMsg->Set( false );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDraggable Component ///////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIDraggableComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIDraggableMod; }
    bool            ICanHaveProxy() override { return true; }

public:
    plGUIDraggableComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefReportDragging,
        kRefHideCursor,
        kRefAlwaysSnap
    };
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIDraggableComponent, gGUIDraggableDesc, "GUI Draggable",  "GUIDraggable", COMP_TYPE_GUI, GUI_DRAGGABLE_CLASSID )

ParamBlockDesc2 gGUIDraggableBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDraggable"), 0, &gGUIDraggableDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    3,
    plGUIControlBase::kRollMain, IDD_COMP_GUIDRAGGABLE, IDS_COMP_GUIDRAGGABLE, 0, 0, nullptr,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    sGUIProxyParamHeader,
    
    &sGUIControlProcParamTemplate,

    sGUIProxyParamTemplate,

    plGUIDraggableComponent::kRefReportDragging, _T("reportWhileDragging"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REPORTDRAG,
        p_default, FALSE,
        end,

    plGUIDraggableComponent::kRefHideCursor, _T("hideCursorWhileDragging"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_HIDECURSOR,
        p_default, FALSE,
        end,

    plGUIDraggableComponent::kRefAlwaysSnap, _T("alwaysSnapBackToStart"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SNAPSTART,
        p_default, FALSE,
        end,

    end
);

plGUIDraggableComponent::plGUIDraggableComponent()
{
    fClassDesc = &gGUIDraggableDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIDraggableComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    node->SetForceLocal( true );
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIDraggableComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIDraggableComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIDraggableMod *ctrl = (pfGUIDraggableMod *)fControl;
    
    if( fCompPB->GetInt( kRefReportDragging ) )
        ctrl->SetFlag( pfGUIDraggableMod::kReportDragging );

    if( fCompPB->GetInt( kRefHideCursor ) )
        ctrl->SetFlag( pfGUIDraggableMod::kHideCursorWhileDragging );

    if( fCompPB->GetInt( kRefAlwaysSnap ) )
        ctrl->SetFlag( pfGUIDraggableMod::kAlwaysSnapBackToStart );

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIKnobCtrl Component ///////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIKnobCtrlComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIKnobCtrl; }
    bool            ICanHaveProxy() override { return true; }

    bool    IGrabAnimationRange( plMaxNode *node, plErrorMsg *pErrMsg, hsMatrix44 &startL2W, hsMatrix44 &endL2W );

public:
    plGUIKnobCtrlComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefMinValue,
        kRefMaxValue,
        kRefStep,
        kReverseValues,
        kRefOrientation,
        kRefMouseMapping,
        kRefTriggerOnMouseUp,
        kRefAnimation,
        kRefAnimationNode,
        kRefAnimationNodeType
    };
};

static plPlasmaAnimSelectDlgProc    sGUIKnobCtrlProc( plGUIKnobCtrlComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
                                                    plGUIKnobCtrlComponent::kRefAnimationNode, plGUIKnobCtrlComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
                                                    "Select the animation to use when displaying this knob control", nullptr);

//Max desc stuff necessary below.
CLASS_DESC(plGUIKnobCtrlComponent, gGUIKnobCtrlDesc, "GUI Knob Control",  "GUIKnobCtrl", COMP_TYPE_GUI, GUI_KNOBCTRL_CLASSID )

ParamBlockDesc2 gGUIKnobCtrlBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIKnobCtrl"), 0, &gGUIKnobCtrlDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    3,
    plGUIControlBase::kRollMain, IDD_COMP_GUIKNOB, IDS_COMP_GUIKNOB, 0, 0, &sGUIKnobCtrlProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    sGUIProxyParamHeader,
    
    &sGUIControlProcParamTemplate,
    
    plGUIKnobCtrlComponent::kRefMinValue, _T("minValue"),   TYPE_FLOAT, 0, 0,   
        p_default, 0.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_LOWER, IDC_GUI_LOWER_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIKnobCtrlComponent::kRefMaxValue, _T("maxValue"),   TYPE_FLOAT, 0, 0,   
        p_default, 10.0f,
        p_range, -10000.f, 10000.f,         // WHY do we even need to specify this?
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_UPPER, IDC_GUI_UPPER_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIKnobCtrlComponent::kRefStep, _T("step"),   TYPE_FLOAT, 0, 0,   
        p_default, 1.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_STEP, IDC_GUI_STEP_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIKnobCtrlComponent::kReverseValues, _T("reverseValues"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REVERSE,
        p_default, FALSE,
        end,

    plGUIKnobCtrlComponent::kRefOrientation,    _T("orientation"), TYPE_INT,        0, 0,
        p_ui,   plGUIControlBase::kRollMain, TYPE_RADIO, 2, IDC_ORIENTATION_RADIO, IDC_ORIENTATION_RADIO2,
        p_default, 0,
        end,
    
    plGUIKnobCtrlComponent::kRefMouseMapping, _T("mouseMapping"), TYPE_INT, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_RADIO, 3, IDC_GUI_MOUSEMAPREL, IDC_GUI_MOUSEMAPANIM, IDC_GUI_MOUSEMAPSCRN,
        p_default, 0,
        end,

    plGUIKnobCtrlComponent::kRefTriggerOnMouseUp, _T("triggerOnMouseUp"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_TRIGGERONUP,
        p_default, FALSE,
        end,
        
    sGUIProxyParamTemplate,

    plGUIKnobCtrlComponent::kRefAnimation, _T("animation"), TYPE_INODE,     0, 0,
        p_prompt, IDS_COMP_GUI_SELECTANIM,
        end,

    plGUIKnobCtrlComponent::kRefAnimationNode, _T("animationNode"), TYPE_INODE,     0, 0,
        end,

    plGUIKnobCtrlComponent::kRefAnimationNodeType, _T("animationNodeType"), TYPE_INT,       0, 0,
        p_default, plAnimObjInterface::kUseOwnerNode,
        end,

    end
);

plGUIKnobCtrlComponent::plGUIKnobCtrlComponent()
{
    fClassDesc = &gGUIKnobCtrlDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

bool    plGUIKnobCtrlComponent::IGrabAnimationRange( plMaxNode *node, plErrorMsg *pErrMsg, hsMatrix44 &startL2W, hsMatrix44 &endL2W )
{
    bool    result = false;


    // Get the affine parts and the TM Controller
    plSceneObject *obj = node->GetSceneObject();
    hsAffineParts * parts = new hsAffineParts;
    plController* tmc = hsControlConverter::Instance().ConvertTMAnim(obj, node, parts);

    if (tmc)
    {
        plMatrixControllerChannel *channel = new plMatrixControllerChannel(tmc, parts);

        float length = tmc->GetLength();

        startL2W = channel->Value( 0.f );
        endL2W = channel->Value( length );

        delete channel;
        result = true;
    }

    delete parts;   // We copy this over, so no need to keep it around
    return result;
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIKnobCtrlComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    node->SetForceLocal( true );

    plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
    if (iface != nullptr && iface->MightRequireSeparateMaterial())
    {
        INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                ? fCompPB->GetINode( kRefAnimationNode )
                                : (INode *)node;

        if (restrict != nullptr)
        {
            node->SetForceMaterialCopy( true );
        }
    }

    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIKnobCtrlComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

// For hackery below (see warning below)
#include "plAnimation/plAGMasterMod.h"

bool plGUIKnobCtrlComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIKnobCtrl *ctrl = (pfGUIKnobCtrl *)fControl;
    
    ctrl->SetRange( fCompPB->GetFloat( kRefMinValue ), fCompPB->GetFloat( kRefMaxValue ) );
    ctrl->SetStep( fCompPB->GetFloat( kRefStep ) );

    if( fCompPB->GetInt( kReverseValues ) )
        ctrl->SetFlag( pfGUIKnobCtrl::kReverseValues );

    // Get the animation to use
    plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
    if (iface != nullptr)
    {
        INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                ? fCompPB->GetINode( kRefAnimationNode )
                                : (INode *)node;


        std::vector<plKey> keys;
        if (iface->GetKeyList(restrict, keys) && !keys.empty())
            ctrl->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
    }
    else
    {
        // HACKERY WARNING: Old knobs assumed the animation was just the same one applied to our node,
        // so to avoid breaking old formats, if we can't grab an animObjInterface, we just grab the key
        // of the master mod of our node, like we would've before
        plAGMasterMod   *master = node->GetAGMasterMod();
        ctrl->SetAnimationKeys({master->GetKey()}, ENTIRE_ANIMATION_NAME);
    }

    if( fCompPB->GetInt( kRefOrientation ) == 1 )
        ctrl->SetFlag( pfGUIKnobCtrl::kLeftRightOrientation );

    hsMatrix44 startL2W, endL2W;
    switch( fCompPB->GetInt( kRefMouseMapping ) )
    {
        case 0:     // Default, normal (old) relative behavior
            break;
        case 1:     // Map to the range of animation positions
            if( !IGrabAnimationRange( node, pErrMsg, startL2W, endL2W ) )
            {
                pErrMsg->Set( true, "Unable to grab animation range for the GUI Knob Control %s. The Map-To-Screen-Range feature will be disabled.", node->GetName() ).Show();
                pErrMsg->Set( false );
            }
            else
            {
                hsPoint3 startPos = startL2W.GetTranslate();
                hsPoint3 endPos = endL2W.GetTranslate();

                ctrl->SetScreenRange( startPos, endPos );
                ctrl->SetFlag( pfGUIKnobCtrl::kMapToAnimationRange );
            }
            break;
        case 2:     // Map to a range on the screen
            ctrl->SetFlag( pfGUIKnobCtrl::kMapToScreenRange );
            break;
    }

    if( fCompPB->GetInt( kRefTriggerOnMouseUp ) )
        ctrl->SetFlag( pfGUIKnobCtrl::kTriggerOnlyOnMouseUp );

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIListBox Component /////////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIListBoxComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIListBoxMod; }
    bool            INeedsDynamicText() override { return true; }

public:
    plGUIListBoxComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefUseScroll,
        kRefScrollCtrl,
        kRefSingleSelect,
        kRefXparentBgnd,
        kRefDragDropSource,
        kRefDisableKeys,
        kRefAllow2DElementGrid,
        kRefScrollLeftToRight,
        kRefScaleWithRes,
        kRefPassClicksThrough,
        kRefEnableTreeBehavior,
        kRefSkin,
        kRefHandsOffMultiSelect
    };
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plGUIListBoxAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if( id == plGUIListBoxComponent::kRefScrollCtrl )
        {
            plGUIListBoxComponent *comp = (plGUIListBoxComponent *)owner;
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
        }
    }
};

Class_ID    sScrollingClassesToSelect[] = { GUI_UPDOWNPAIR_CLASSID, GUI_KNOBCTRL_CLASSID, plGUISingleCtrlDlgProc::kEndClassList };

static plGUIListBoxAccessor sGUIListBoxAccessor;
static plGUISingleCtrlDlgProc sGUIListBoxProc( plGUIListBoxComponent::kRefScrollCtrl, IDC_GUI_COMPSELBTN,
                                            "Select the control to use for scrolling this list box", sScrollingClassesToSelect );

static plGUISingleCtrlDlgProc sGUILBSkinSelectProc( plGUIListBoxComponent::kRefSkin, IDC_GUI_SKIN,
                                            "Select the skin to use for this list box", sSkinClassesToSelect,
                                            &sGUIListBoxProc );

//Max desc stuff necessary below.
CLASS_DESC(plGUIListBoxComponent, gGUIListBoxDesc, "GUI List Box",  "GUIListBox", COMP_TYPE_GUI, GUI_LISTBOX_CLASSID )

ParamBlockDesc2 gGUIListBoxBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIListBox"), 0, &gGUIListBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUILISTBOX, IDS_COMP_GUILISTBOX, 0, 0, &sGUILBSkinSelectProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,  

    plGUIListBoxComponent::kRefUseScroll,   _T( "enableScrolling" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCROLLCTRL,
        p_default, FALSE,
        p_enable_ctrls, 1, plGUIListBoxComponent::kRefScrollCtrl,
        end,

    plGUIListBoxComponent::kRefScrollCtrl, _T("scrollControl"), TYPE_INODE,     0, 0,
        p_prompt, IDS_COMP_GUI_SELECTSCROLL,
        p_accessor, &sGUIListBoxAccessor,
        end,

    plGUIListBoxComponent::kRefSingleSelect,    _T( "singleSelect" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SINGLESEL,
        p_default, FALSE,
        end,

    plGUIListBoxComponent::kRefXparentBgnd, _T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
        p_default, FALSE,
        end,
    
    plGUIListBoxComponent::kRefDragDropSource,  _T( "dragDropCapable" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_DRAGDROPSRC,
        p_default, FALSE,
        end,

    plGUIListBoxComponent::kRefDisableKeys, _T( "disableKeys" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_DISABLEKEYS,
        p_default, FALSE,
        end,
    
    plGUIListBoxComponent::kRefAllow2DElementGrid,  _T( "allow2DElementGrid" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ALLOWMULTIROW,
        p_default, FALSE,
        end,
    
    plGUIListBoxComponent::kRefScrollLeftToRight,   _T( "scrollLeftToRight" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCROLLL2R,
        p_default, FALSE,
        end,

    plGUIListBoxComponent::kRefScaleWithRes,    _T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
        p_default, FALSE,
        end,
                
    plGUIListBoxComponent::kRefPassClicksThrough,   _T( "passClicksThru" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_PASSTHRU,
        p_default, FALSE,
        end,
        
    plGUIListBoxComponent::kRefEnableTreeBehavior,  _T( "makeLikeATree" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ENABLETREE,
        p_default, FALSE,
        end,
                
    plGUIListBoxComponent::kRefSkin, _T("skin"),    TYPE_INODE,     0, 0,
        end,

    plGUIListBoxComponent::kRefHandsOffMultiSelect, _T( "handsOffMultiSelect" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_HANDSOFF,
        p_default, FALSE,
        end,
        
    end
);

plGUIListBoxComponent::plGUIListBoxComponent()
{
    fClassDesc = &gGUIListBoxDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIListBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIListBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIListBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIListBoxMod *ctrl = (pfGUIListBoxMod *)fControl;
    
    if( fCompPB->GetInt( kRefUseScroll ) )
    {
        // Get the scrolling control to use
        pfGUIValueCtrl *scroll = pfGUIValueCtrl::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefScrollCtrl ) ) );
        if (scroll != nullptr)
        {
            hsgResMgr::ResMgr()->AddViaNotify( scroll->GetKey(), new plGenRefMsg( ctrl->GetKey(), 
                                        plRefMsg::kOnCreate, -1, pfGUIListBoxMod::kRefScrollCtrl ), plRefFlags::kActiveRef );
        }
    }

    if( fCompPB->GetInt( kRefSingleSelect ) )
        ctrl->SetSingleSelect( true );

    if( fCompPB->GetInt( kRefXparentBgnd ) )
        ctrl->SetFlag( pfGUIListBoxMod::kXparentBgnd );

    if( fCompPB->GetInt( kRefDragDropSource ) )
        ctrl->SetFlag( pfGUIListBoxMod::kDragAndDropCapable );

    if( fCompPB->GetInt( kRefDisableKeys ) )
        ctrl->SetFlag( pfGUIListBoxMod::kDisableKeyActions );

    if( fCompPB->GetInt( kRefAllow2DElementGrid ) )
        ctrl->SetFlag( pfGUIListBoxMod::kAllowMultipleElementsPerRow );

    if( fCompPB->GetInt( kRefScrollLeftToRight ) )
        ctrl->SetFlag( pfGUIListBoxMod::kScrollLeftToRight );
    
    if( fCompPB->GetInt( kRefScaleWithRes ) )
        ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

    if( fCompPB->GetInt( kRefPassClicksThrough ) )
        ctrl->SetFlag( pfGUIListBoxMod::kAllowMousePassThrough );

    if( fCompPB->GetInt( kRefEnableTreeBehavior ) )
        ctrl->SetFlag( pfGUIListBoxMod::kGrowLeavesAndProcessOxygen );

    if( fCompPB->GetInt( kRefHandsOffMultiSelect ) )
        ctrl->SetFlag( pfGUIListBoxMod::kHandsOffMultiSelect );
    
    INode *sNode = fCompPB->GetINode( kRefSkin );
    if (sNode != nullptr)
    {
        plComponentBase *comp = ( (plMaxNode *)sNode )->ConvertToComponent();
        if (comp != nullptr)
        {
            Class_ID nodeID = comp->ClassID();
            hsAssert( nodeID == GUI_SKIN_CLASSID, "Bad node param in GUIMenu::Convert()" );

            plGUISkinComp *skin = (plGUISkinComp *)comp;
            hsgResMgr::ResMgr()->AddViaNotify( skin->GetConvertedSkin()->GetKey(), new plGenRefMsg( ctrl->GetKey(), plRefMsg::kOnCreate, -1, pfGUIControlMod::kRefSkin ), plRefFlags::kActiveRef );
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUITextBox Component /////////////////////////////////////////////////////////////////////
//
//  GUI element that displays a block of wrapped text.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUITextBoxComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUITextBoxMod; }
    bool            INeedsDynamicText() override { return true; }

public:
    plGUITextBoxComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefInitText,
        kRefFontSize,
        kRefXparentBgnd,
        kRefJustify,
        kRefScaleWithRes,
        kRefUseLocalization,
        kRefLocalizationPath
    };
};

class plGUITextBoxProc : public ParamMap2UserDlgProc
{
private:
    std::vector<std::string> fTranslations;
    int fCurLanguage;
    void ISetTranslation(int lang, std::string text)
    {
        while (lang >= fTranslations.size())
            fTranslations.push_back("");
        fTranslations[lang] = text;
    }
protected:

public:

    void DeleteThis() override { }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        int i;
        switch( msg )
        {
            case WM_INITDIALOG:
                // make sure there is a string to get
                if ( pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefInitText ) )
                {
                    fTranslations = plLocalization::StringToLocal(pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefInitText ) );
                    SetDlgItemText( hWnd, IDC_GUI_INITTEXT, fTranslations[0].c_str() );
                }
                else
                    // if there is no text, then there is nothing to translate
                    SetDlgItemText( hWnd, IDC_GUI_INITTEXT, pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefInitText ) );
                SendMessage( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), CB_RESETCONTENT, 0, 0 );
                for (i=0; i<plLocalization::kNumLanguages; i++)
                    SendMessage( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), CB_ADDSTRING, 0, (LPARAM)plLocalization::GetLanguageName((plLocalization::Language)i) );
                SendMessage( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), CB_SETCURSEL, 0, 0 );
                fCurLanguage = 0;

                SetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, pmap->GetParamBlock()->GetStr( plGUITextBoxComponent::kRefLocalizationPath ) );

                if ( pmap->GetParamBlock()->GetInt( plGUITextBoxComponent::kRefUseLocalization ) != 0 )
                {
                    // disable standard text, enable loc path
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), false );
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), false );
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), true );
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), true );
                    CheckDlgButton( hWnd, IDC_GUI_USE_LOCALIZATION, BST_CHECKED );
                }
                else
                {
                    // enable standard text, disable loc path
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), true );
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), true );
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), false );
                    EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), false );
                    CheckDlgButton( hWnd, IDC_GUI_USE_LOCALIZATION, BST_UNCHECKED );
                }
                return TRUE;

            case WM_DESTROY:
                break;

            case WM_COMMAND:
                if( LOWORD( wParam ) == IDC_GUI_INITTEXT )
                {
                    if( HIWORD( wParam ) == EN_CHANGE )
                    {
                        int strLen = (int)SendDlgItemMessage(hWnd, IDC_GUI_INITTEXT, WM_GETTEXTLENGTH, 0, 0);
                        if( strLen > 0 )
                        {
                            char *str = new char[ strLen + 1 ];
                            GetDlgItemText( hWnd, IDC_GUI_INITTEXT, str, strLen + 1 );
                            str[ strLen ] = 0;
                            ISetTranslation(fCurLanguage,str);
                            delete [] str;

                            std::string translation = plLocalization::LocalToString(fTranslations);
                            str = new char[ translation.length() + 1 ];
                            strcpy(str,translation.c_str());
                            str[translation.length()] = 0;
                
                            pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefInitText, 0, str );
                            delete [] str;
                        }
                    }
                    else if( HIWORD( wParam ) == EN_KILLFOCUS )
                    {
                        plMaxAccelerators::Enable();
                    }
                    else if( HIWORD( wParam ) == EN_SETFOCUS )
                    {
                        plMaxAccelerators::Disable();
                    }
                }
                else if( LOWORD( wParam ) == IDC_GUI_LOCALIZATION_PATH )
                {
                    if( HIWORD( wParam ) == EN_CHANGE )
                    {
                        int strLen = (int)SendDlgItemMessage(hWnd, IDC_GUI_LOCALIZATION_PATH, WM_GETTEXTLENGTH, 0, 0);
                        if( strLen > 0 )
                        {
                            char *str = new char[ strLen + 1 ];
                            GetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, str, strLen + 1 );
                            str[ strLen ] = 0;
                            pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefLocalizationPath, 0, str );
                            delete [] str;
                        }
                    }
                    else if( HIWORD( wParam ) == EN_KILLFOCUS )
                    {
                        plMaxAccelerators::Enable();
                    }
                    else if( HIWORD( wParam ) == EN_SETFOCUS )
                    {
                        plMaxAccelerators::Disable();
                    }
                }
                else if( LOWORD( wParam ) == IDC_GUI_LANGUAGE )
                {
                    if( HIWORD( wParam ) == CBN_SELCHANGE )
                    {
                        int idx = (int)SendDlgItemMessage(hWnd, IDC_GUI_LANGUAGE, CB_GETCURSEL, 0, 0);
                        if (idx >= fTranslations.size())
                            SetDlgItemText( hWnd, IDC_GUI_INITTEXT, "" );
                        else
                            SetDlgItemText( hWnd, IDC_GUI_INITTEXT, fTranslations[idx].c_str() );
                        fCurLanguage = idx;
                    }
                }
                else if( LOWORD( wParam ) == IDC_GUI_SELECT_LOC_PATH )
                {
                    char value[512];
                    GetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, value, 512 );
                    plPickLocalizationDlg dlg( value );
                    if( dlg.DoPick() )
                    {
                        pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefLocalizationPath, 0, (char*)dlg.GetValue() );
                        SetDlgItemText( hWnd, IDC_GUI_LOCALIZATION_PATH, (char*)dlg.GetValue() );
                    }
                }
                else if( LOWORD( wParam ) == IDC_GUI_USE_LOCALIZATION )
                {
                    // enable/disable the appropriate values
                    bool useLoc = ( IsDlgButtonChecked( hWnd, IDC_GUI_USE_LOCALIZATION ) == BST_CHECKED );
                    pmap->GetParamBlock()->SetValue( plGUITextBoxComponent::kRefUseLocalization, 0, useLoc ? 1 : 0 );

                    if ( useLoc )
                    {
                        // disable standard text, enable loc path
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), false );
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), false );
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), true );
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), true );
                    }
                    else
                    {
                        // enable standard text, disable loc path
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_INITTEXT ), true );
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_LANGUAGE ), true );
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_LOCALIZATION_PATH ), false );
                        EnableWindow( GetDlgItem( hWnd, IDC_GUI_SELECT_LOC_PATH ), false );
                    }
                }
                break;
        }
        return FALSE;
    }
};
static plGUITextBoxProc gGUITextBoxProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUITextBoxComponent, gGUITextBoxDesc, "GUI Text Box",  "GUITextBox", COMP_TYPE_GUI, GUI_TEXTBOX_CLASSID )

ParamBlockDesc2 gGUITextBoxBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUITextBox"), 0, &gGUITextBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUITEXTBOX, IDS_COMP_GUITEXTBOX, 0, 0, &gGUITextBoxProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,  

        plGUITextBoxComponent::kRefInitText,    _T("InitText"),     TYPE_STRING,        0, 0,
//          p_ui,   plGUIControlBase::kRollMain, TYPE_EDITBOX, IDC_GUI_INITTEXT,
            end,

        plGUITextBoxComponent::kRefXparentBgnd, _T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
            p_default, FALSE,
            end,
    
        plGUITextBoxComponent::kRefJustify, _T("justify"), TYPE_INT,        0, 0,
            p_ui,   plGUIControlBase::kRollMain, TYPE_RADIO, 3, IDC_JUSTIFYRADIO, IDC_JUSTRADIO2, IDC_JUSTRADIO3, 
            p_default, 0,
            end,

        plGUITextBoxComponent::kRefScaleWithRes,    _T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
            p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
            p_default, FALSE,
            end,

        plGUITextBoxComponent::kRefUseLocalization, _T( "useLocalization" ), TYPE_BOOL, 0, 0,
            p_default, FALSE,
            end,

        plGUITextBoxComponent::kRefLocalizationPath,_T( "localizationPath" ),TYPE_STRING, 0, 0,
            end,
                
    end
);

plGUITextBoxComponent::plGUITextBoxComponent()
{
    fClassDesc = &gGUITextBoxDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUITextBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUITextBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUITextBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUITextBoxMod *ctrl = (pfGUITextBoxMod *)fControl;

    ctrl->SetText( fCompPB->GetStr( kRefInitText ) );

    if( fCompPB->GetInt( kRefXparentBgnd ) )
        ctrl->SetFlag( pfGUITextBoxMod::kXparentBgnd );

    int just = fCompPB->GetInt( kRefJustify );
    if( just == 1 )
        ctrl->SetFlag( pfGUITextBoxMod::kCenterJustify );
    else if( just == 2 )
        ctrl->SetFlag( pfGUITextBoxMod::kRightJustify );

    if( fCompPB->GetInt( kRefScaleWithRes ) )
        ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

    ctrl->SetUseLocalizationPath( fCompPB->GetInt( kRefUseLocalization ) != 0 );
    ctrl->SetLocalizationPath( fCompPB->GetStr( kRefLocalizationPath ) );

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIEditBox Component /////////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIEditBoxComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIEditBoxMod; }
    bool            INeedsDynamicText() override { return true; }

public:
    plGUIEditBoxComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefXparentBgnd,
        kRefScaleWithRes
    };
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIEditBoxComponent, gGUIEditBoxDesc, "GUI Edit Box",  "GUIEditBox", COMP_TYPE_GUI, GUI_EDITBOX_CLASSID )

ParamBlockDesc2 gGUIEditBoxBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIEditBox"), 0, &gGUIEditBoxDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUIEDITBOX, IDS_COMP_GUIEDITBOX, 0, 0, nullptr,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,  

    plGUIEditBoxComponent::kRefXparentBgnd, _T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
        p_default, FALSE,
        end,
    
    plGUIEditBoxComponent::kRefScaleWithRes,    _T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
        p_default, FALSE,
        end,
            
    end
);

plGUIEditBoxComponent::plGUIEditBoxComponent()
{
    fClassDesc = &gGUIEditBoxDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIEditBoxComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIEditBoxComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIEditBoxComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIEditBoxMod *ctrl = (pfGUIEditBoxMod *)fControl;
    
    if( fCompPB->GetInt( kRefXparentBgnd ) )
        ctrl->SetFlag( pfGUIEditBoxMod::kXparentBgnd );

    if( fCompPB->GetInt( kRefScaleWithRes ) )
        ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIUpDownPair Component //////////////////////////////////////////////////////////////////
//
//  GUI grouping element that uses two buttons to alter a value up and down
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIUpDownPairComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIUpDownPairMod; }

public:
    plGUIUpDownPairComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefMinValue,
        kRefMaxValue,
        kRefStep,
        kRefUpControl,
        kRefDownControl
    };
};

//// Dialog proc ////////////////////////////////////////////////////////////////////////////////

class plGUIUDPairDlgProc : public ParamMap2UserDlgProc
{
protected:
    ParamID         fUpNodeID, fDownNodeID;
    int             fUpDlgItem, fDownDlgItem;
    TCHAR           fTitle[ 128 ];

public:
    plGUIUDPairDlgProc( ParamID upNodeID, int upDlgItem, ParamID downNodeID, int downDlgItem, TCHAR *title )
    {
        fUpNodeID = upNodeID;
        fDownNodeID = downNodeID;
        fUpDlgItem = upDlgItem;
        fDownDlgItem = downDlgItem;
        strcpy( fTitle, title );
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch ( msg )
        {
            case WM_INITDIALOG:
                {
                    IParamBlock2 *pb = map->GetParamBlock();

                    INode *node = pb->GetINode( fUpNodeID );
                    TSTR newName( node ? node->GetName() : "Pick" );
                    ::SetWindowText( ::GetDlgItem( hWnd, fUpDlgItem ), newName );

                    node = pb->GetINode( fDownNodeID );
                    TSTR newName2( node ? node->GetName() : "Pick" );
                    ::SetWindowText( ::GetDlgItem( hWnd, fDownDlgItem ), newName2 );
                }
                return TRUE;

            case WM_COMMAND:
                if( ( HIWORD( wParam ) == BN_CLICKED ) )
                {
                    if( LOWORD( wParam ) == fUpDlgItem )
                    {
                        IParamBlock2 *pb = map->GetParamBlock();
                        plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fUpNodeID, fTitle, true, GUI_BUTTON_CLASSID );
                        GetCOREInterface()->DoHitByNameDialog( &hitCB );

                        INode* node = pb->GetINode( fUpNodeID );
                        TSTR newName( node ? node->GetName() : "Pick" );
                        ::SetWindowText( ::GetDlgItem(hWnd, fUpDlgItem ), newName );
                        map->Invalidate( fUpNodeID );
                        ::InvalidateRect(hWnd, nullptr, TRUE);
                    }
                    else if( LOWORD( wParam ) == fDownDlgItem )
                    {
                        IParamBlock2 *pb = map->GetParamBlock();
                        plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fDownNodeID, fTitle, true, GUI_BUTTON_CLASSID );
                        GetCOREInterface()->DoHitByNameDialog( &hitCB );

                        INode* node = pb->GetINode( fDownNodeID );
                        TSTR newName( node ? node->GetName() : "Pick" );
                        ::SetWindowText( ::GetDlgItem(hWnd, fDownDlgItem ), newName );
                        map->Invalidate( fDownDlgItem );
                        ::InvalidateRect(hWnd, nullptr, TRUE);
                    }
                }
                return TRUE;
        }

        return FALSE;
    }

    void DeleteThis() override { }
};

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plGUIUDAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if( id == plGUIUpDownPairComponent::kRefUpControl
            || id == plGUIUpDownPairComponent::kRefDownControl )
        {
            plGUIUpDownPairComponent *comp = (plGUIUpDownPairComponent *)owner;
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
        }
    }
};

static plGUIUDAccessor sGUIUDAccessor;
static plGUIUDPairDlgProc sGUIUDPairDlgProc( plGUIUpDownPairComponent::kRefUpControl, IDC_GUI_COMPSELBTN,
                                            plGUIUpDownPairComponent::kRefDownControl, IDC_GUI_COMPSELBTN2,
                                            "Select the control to use in this pair" );

//Max desc stuff necessary below.
CLASS_DESC(plGUIUpDownPairComponent, gGUIUDPairDesc, "GUI Up/Down Pair",  "GUIUDPair", COMP_TYPE_GUI, GUI_UPDOWNPAIR_CLASSID )

ParamBlockDesc2 gGUIUDPairBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIUDPair"), 0, &gGUIUDPairDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUIUDSCROLL, IDS_COMP_GUIUDSCROLL, 0, 0, &sGUIUDPairDlgProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,  

    plGUIUpDownPairComponent::kRefUpControl, _T("upControl"),   TYPE_INODE,     0, 0,
        p_prompt, IDS_COMP_GUI_SELECTUDCTRL,
        p_accessor, &sGUIUDAccessor,
        end,

    plGUIUpDownPairComponent::kRefDownControl, _T("downControl"),   TYPE_INODE,     0, 0,
        p_prompt, IDS_COMP_GUI_SELECTUDCTRL,
        p_accessor, &sGUIUDAccessor,
        end,

    plGUIUpDownPairComponent::kRefMinValue, _T("minValue"), TYPE_FLOAT, 0, 0,   
        p_default, 0.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_LOWER, IDC_GUI_LOWER_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIUpDownPairComponent::kRefMaxValue, _T("maxValue"), TYPE_FLOAT, 0, 0,   
        p_default, 10.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_UPPER, IDC_GUI_UPPER_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIUpDownPairComponent::kRefStep, _T("step"), TYPE_FLOAT, 0, 0,   
        p_default, 1.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_STEP, IDC_GUI_STEP_SPIN, SPIN_AUTOSCALE,
        end,

    end
);

plGUIUpDownPairComponent::plGUIUpDownPairComponent()
{
    fClassDesc = &gGUIUDPairDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIUpDownPairComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIUpDownPairComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIUpDownPairComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIUpDownPairMod *ctrl = (pfGUIUpDownPairMod *)fControl;

    // Get the child controls
    pfGUIButtonMod *up = pfGUIButtonMod::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefUpControl ) ) );
    pfGUIButtonMod *down = pfGUIButtonMod::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefDownControl ) ) );

    ctrl->SetControls( up, down );

    ctrl->SetRange( fCompPB->GetFloat( kRefMinValue ), fCompPB->GetFloat( kRefMaxValue ) );
    ctrl->SetStep( fCompPB->GetFloat( kRefStep ) );

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDragBar Component ///////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIDragBarComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIDragBarCtrl; }
    bool            ICanHaveProxy() override { return true; }

public:
    plGUIDragBarComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIDragBarComponent, gGUIDragBarDesc, "GUI Dialog Drag Bar",  "GUIDragBar", COMP_TYPE_GUI, GUI_DRAGBAR_CLASSID )

ParamBlockDesc2 gGUIDragBarBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDragBar"), 0, &gGUIDragBarDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    3,
    plGUIControlBase::kRollMain, IDD_COMP_GUIDRAGBAR, IDS_COMP_GUIDRAGBAR, 0, 0, nullptr,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    sGUIProxyParamHeader,
    
    &sGUIControlProcParamTemplate,  

    sGUIProxyParamTemplate,

    end
);

plGUIDragBarComponent::plGUIDragBarComponent()
{
    fClassDesc = &gGUIDragBarDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIDragBarComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    node->SetForceLocal( true );
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIDragBarComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIDragBarComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIDragBarCtrl *ctrl = (pfGUIDragBarCtrl *)fControl;
    
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIRadioGroup Component //////////////////////////////////////////////////////////////////
//
//  GUI grouping element that ensures that only one of a group of check boxes is checked at any
//  one time, and takes on the value of whichever one is currently checked, or -1 if none.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIRadioGroupAccessor;
class plGUIRadioGroupComponent : public plGUIControlBase
{
    friend class plGUIRadioGroupAccessor;

protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIRadioGroupCtrl; }

public:
    plGUIRadioGroupComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefCheckBoxes,
        kRefDefaultSel,
        kRefAllowNoSel
    };
};

//// Dialog proc ////////////////////////////////////////////////////////////////////////////////

class plGUIRadioGroupProc : public ParamMap2UserDlgProc
{
protected:

public:
    plGUIRadioGroupProc()
    {
    }

    void    SetSpinnerRange( IParamMap2 *pMap )
    {
        if (pMap == nullptr)
            return;

        HWND    hWnd = pMap->GetHWnd();
        if (hWnd == nullptr)
            return;

        ISpinnerControl *spin = GetISpinner( GetDlgItem( hWnd, IDC_GUI_DEFSEL_SPIN ) );

        int minValue = pMap->GetParamBlock()->GetInt( plGUIRadioGroupComponent::kRefAllowNoSel ) ? -1 : 0;
        int maxValue = pMap->GetParamBlock()->Count( plGUIRadioGroupComponent::kRefCheckBoxes );

        spin->SetLimits( minValue, maxValue );

        ReleaseISpinner( spin );
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch ( msg )
        {
            case WM_INITDIALOG:
                {
                    SetSpinnerRange( map );
                }
                return TRUE;

            case WM_COMMAND:
                if( ( HIWORD( wParam ) == BN_CLICKED ) )
                {
                    if( LOWORD( wParam ) == IDC_GUI_ADDCHECK )
                    {
                        IParamBlock2 *pb = map->GetParamBlock();

                        plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, plGUIRadioGroupComponent::kRefCheckBoxes, 
                                    "Select a check box to add to this radio group", true, GUI_CHECKBOX_CLASSID, false );

                        GetCOREInterface()->DoHitByNameDialog( &hitCB );

                        map->Invalidate( plGUIRadioGroupComponent::kRefCheckBoxes );
                    }
                }
                return TRUE;
        }

        return FALSE;
    }

    void DeleteThis() override { }
};
static plGUIRadioGroupProc sGUIRadioGroupProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUIRadioGroupComponent, gGUIRadioGroupDesc, "GUI Radio Group",  "GUIRadioGroup", COMP_TYPE_GUI, GUI_RADIOGROUP_CLASSID )

// When one of our parameters that is a ref changes, send out the component ref
// changed message.  Normally, messages from component refs are ignored since
// they pass along all the messages of the ref, which generates a lot of false
// converts.
class plGUIRadioGroupAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        plGUIRadioGroupComponent *comp = (plGUIRadioGroupComponent *)owner;
        IParamBlock2    *pBlock = comp->fCompPB;

        if( id == plGUIRadioGroupComponent::kRefCheckBoxes )
        {
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
            sGUIRadioGroupProc.SetSpinnerRange( pBlock->GetMap( plGUIControlBase::kRollMain ) );
        }
        else if( id == plGUIRadioGroupComponent::kRefAllowNoSel )
            sGUIRadioGroupProc.SetSpinnerRange( pBlock->GetMap( plGUIControlBase::kRollMain ) );
    }

    void    TabChanged(tab_changes changeCode, Tab<PB2Value> *tab, ReferenceMaker *owner, ParamID id, int tabIndex, int count) override
    {
        plGUIRadioGroupComponent *comp = (plGUIRadioGroupComponent *)owner;
        IParamBlock2    *pBlock = comp->fCompPB;

        if( id == plGUIRadioGroupComponent::kRefCheckBoxes )
        {
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
            sGUIRadioGroupProc.SetSpinnerRange( pBlock->GetMap( plGUIControlBase::kRollMain ) );
        }
    }

};

static plGUIRadioGroupAccessor sGUIRadioGroupAccessor;

ParamBlockDesc2 gGUIRadioGroupBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIRadioGroup"), 0, &gGUIRadioGroupDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUIRADIOGROUP, IDS_COMP_GUIRADIOGROUP, 0, 0, &sGUIRadioGroupProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,  

    plGUIRadioGroupComponent::kRefCheckBoxes,   _T("checkBoxes"),   TYPE_INODE_TAB, 0,      0, 0,
        p_ui,           plGUIControlBase::kRollMain, TYPE_NODELISTBOX, IDC_GUI_CHECKLIST, 0, 0, IDC_GUI_DELCHECK,
        p_accessor,     &sGUIRadioGroupAccessor,
        end,

    plGUIRadioGroupComponent::kRefDefaultSel, _T("defaultSelection"),   TYPE_INT,   0, 0,   
        p_default, 0,
        p_range, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_INT, IDC_GUI_DEFSEL, IDC_GUI_DEFSEL_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIRadioGroupComponent::kRefAllowNoSel,   _T( "allowNoSel" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ALLOWNONE,
        p_default, FALSE,
        p_accessor, &sGUIRadioGroupAccessor,
        end,

    end
);

plGUIRadioGroupComponent::plGUIRadioGroupComponent()
{
    fClassDesc = &gGUIRadioGroupDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIRadioGroupComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}


bool plGUIRadioGroupComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIRadioGroupComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIRadioGroupCtrl *ctrl = (pfGUIRadioGroupCtrl *)fControl;

    int i;
    ctrl->ClearControlList();
    for( i = 0; i < fCompPB->Count( kRefCheckBoxes ); i++ )
    {
        pfGUICheckBoxCtrl *cb = pfGUICheckBoxCtrl::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefCheckBoxes, 0, i ) ) );
        if (cb != nullptr)
            ctrl->AddControl( cb );
    }

    if( fCompPB->GetInt( kRefAllowNoSel ) )
        ctrl->SetFlag( pfGUIRadioGroupCtrl::kAllowNoSelection );

    ctrl->SetDefaultValue( fCompPB->GetInt( kRefDefaultSel ) );

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIDynDisplay Component //////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIDynDisplayComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIDynDisplayCtrl; }
    bool            IHasProcRollout() override { return false; }

public:
    plGUIDynDisplayComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefDynLayer
    };
};

//// Dialog proc ////////////////////////////////////////////////////////////////////////////////

class plGUIDynDisplayProc : public ParamMap2UserDlgProc
{
protected:

public:
    plGUIDynDisplayProc()
    {
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch ( msg )
        {
            case WM_INITDIALOG:
                {
                    IParamBlock2 *pb = map->GetParamBlock();

                    Texmap *tmap = pb->GetTexmap( plGUIDynDisplayComponent::kRefDynLayer );
                    if (tmap != nullptr)
                        SetDlgItemText( hWnd, IDC_GUI_PICKMAT, (const char *)tmap->GetName() );
                    else
                        SetDlgItemText( hWnd, IDC_GUI_PICKMAT, "Pick" );
                }
                return TRUE;

            case WM_COMMAND:
                if( ( HIWORD( wParam ) == BN_CLICKED ) )
                {
                    if( LOWORD( wParam ) == IDC_GUI_PICKMAT )
                    {
                        IParamBlock2 *pb = map->GetParamBlock();

                        if( plPickMaterialMap::PickTexmap( pb, plGUIDynDisplayComponent::kRefDynLayer ) )
                        {
                            Texmap *tmap = pb->GetTexmap( plGUIDynDisplayComponent::kRefDynLayer );
                            if (tmap != nullptr)
                                SetDlgItemText( hWnd, IDC_GUI_PICKMAT, (const char *)tmap->GetName() );
                            else
                                SetDlgItemText( hWnd, IDC_GUI_PICKMAT, "Pick" );

                            map->Invalidate( plGUIDynDisplayComponent::kRefDynLayer );
                        }
                    }
                }
                return TRUE;
        }

        return FALSE;
    }

    void DeleteThis() override { }
};
static plGUIDynDisplayProc  sGUIDynDisplayProc;

//Max desc stuff necessary below.
CLASS_DESC(plGUIDynDisplayComponent, gGUIDynDisplayDesc, "GUI Dynamic Display",  "GUIDynDisplay", COMP_TYPE_GUI, GUI_DYNDISPLAY_CLASSID )

ParamBlockDesc2 gGUIDynDisplayBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIDynDisplay"), 0, &gGUIDynDisplayDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP, plComponent::kRefComp,

    1,
    plGUIControlBase::kRollMain, IDD_COMP_GUIDYNDISPLAY, IDS_COMP_GUIDYNDISPLAY, 0, 0, &sGUIDynDisplayProc,

    plGUIDynDisplayComponent::kRefDynLayer, _T("dynLayer"), TYPE_TEXMAP, 0, 0,
//      p_ui, plGUIControlBase::kRollMain, TYPE_TEXMAPBUTTON, IDC_GUI_COMPSELBTN,
        end,

    end
);

plGUIDynDisplayComponent::plGUIDynDisplayComponent()
{
    fClassDesc = &gGUIDynDisplayDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIDynDisplayComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIDynDisplayComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIDynDisplayComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIDynDisplayCtrl *ctrl = (pfGUIDynDisplayCtrl *)fControl;
    
    Texmap *tmap = fCompPB->GetTexmap( plGUIDynDisplayComponent::kRefDynLayer );
    plPlasmaMAXLayer *pLayer = plPlasmaMAXLayer::GetPlasmaMAXLayer( tmap );
    if (pLayer == nullptr /*|| pLayer->ClassID() != DYN_TEXT_LAYER_CLASS_ID */)
    {

        pErrMsg->Set(true, "GUI Control Component Error", "The texmap selected for the Dynamic Display Control on object \"%s\" is not a Plasma Dynamic Text Layer. Please fix.", node->GetName() ).Show();
        return false;   
    }
    
    const std::vector<hsMaterialConverter::DoneMaterialData> &materials = hsMaterialConverter::Instance().DoneMaterials();

    uint32_t i,count = pLayer->GetNumConversionTargets();
    for( i = 0; i < count; i++ )
    {
        plLayerInterface *layIface = pLayer->GetConversionTarget( i );
        
        ctrl->AddLayer( layIface );

        plDynamicTextMap *map = plDynamicTextMap::ConvertNoRef( layIface->GetTexture() );
        if (map != nullptr)
            ctrl->AddMap( map );

        bool found = false;
        for (const hsMaterialConverter::DoneMaterialData& mat : materials)
        {
            hsGMaterial *curMaterial = mat.fHsMaterial;
            for (size_t lay = 0; lay < curMaterial->GetNumLayers(); lay++)
            {
                if (layIface->BottomOfStack() == curMaterial->GetLayer(lay))
                {
                    ctrl->AddMaterial(curMaterial);
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIMultiLineEdit Component ///////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIMultiLineEditComp : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIMultiLineEditCtrl; }
    bool            INeedsDynamicText() override { return true; }

public:
    plGUIMultiLineEditComp();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefXparentBgnd,
        kRefScaleWithRes,
        kRefUseScroll,
        kRefScrollCtrl,
    };
};

static plGUISingleCtrlDlgProc sGUIMultiLineProc( plGUIMultiLineEditComp::kRefScrollCtrl, IDC_GUI_COMPSELBTN,
                                            "Select the control to use for scrolling this multi-line edit box", sScrollingClassesToSelect );

//Max desc stuff necessary below.
CLASS_DESC(plGUIMultiLineEditComp, gGUIMultiLineEditDesc, "GUI Multi-Line Edit Box",  "GUIMultiLineEdit", COMP_TYPE_GUI, GUI_MULTILINE_CLASSID )

ParamBlockDesc2 gGUIMultiLineEditBoxBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIMultiLineEdit"), 0, &gGUIMultiLineEditDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUIMULTILINE, IDS_COMP_GUIMULTILINE, 0, 0, &sGUIMultiLineProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,  

    plGUIMultiLineEditComp::kRefXparentBgnd,    _T( "xparentBgnd" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_XPARENT,
        p_default, FALSE,
        end,
    
    plGUIMultiLineEditComp::kRefScaleWithRes,   _T( "scaleWithRes" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
        p_default, FALSE,
        end,
            
    plGUIMultiLineEditComp::kRefUseScroll,  _T( "enableScrolling" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_SCROLLCTRL,
        p_default, FALSE,
        p_enable_ctrls, 1, plGUIMultiLineEditComp::kRefScrollCtrl,
        end,

    plGUIMultiLineEditComp::kRefScrollCtrl, _T("scrollControl"),    TYPE_INODE,     0, 0,
        p_prompt, IDS_COMP_GUI_SELECTSCROLL,
        p_accessor, &sGUIListBoxAccessor,
        end,

    end
);

plGUIMultiLineEditComp::plGUIMultiLineEditComp()
{
    fClassDesc = &gGUIMultiLineEditDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIMultiLineEditComp::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIMultiLineEditComp::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIMultiLineEditComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIMultiLineEditCtrl *ctrl = (pfGUIMultiLineEditCtrl *)fControl;
    
    if( fCompPB->GetInt( kRefXparentBgnd ) )
        ctrl->SetFlag( pfGUIControlMod::kXparentBgnd );

    if( fCompPB->GetInt( kRefScaleWithRes ) )
        ctrl->SetFlag( pfGUIControlMod::kScaleTextWithResolution );

    if( fCompPB->GetInt( kRefUseScroll ) )
    {
        // Get the scrolling control to use
        pfGUIValueCtrl *scroll = pfGUIValueCtrl::ConvertNoRef( GrabControlMod( fCompPB->GetINode( kRefScrollCtrl ) ) );
        if (scroll != nullptr)
        {
            hsgResMgr::ResMgr()->AddViaNotify( scroll->GetKey(), new plGenRefMsg( ctrl->GetKey(), 
                                        plRefMsg::kOnCreate, -1, pfGUIMultiLineEditCtrl::kRefScrollCtrl ), plRefFlags::kActiveRef );
        }
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIProgressCtrl Component ///////////////////////////////////////////////////////////////////
//
//  GUI element that can be dragged anywhere in the 2D viewing plane.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIProgressCtrlComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIProgressCtrl; }
    bool            ICanHaveProxy() override { return false; }

public:
    plGUIProgressCtrlComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefMinValue,
        kRefMaxValue,
        kRefStep,
        kReverseValues,
        kRefOrientation,
        kRefMouseMapping,
        kRefTriggerOnMouseUp,
        kRefAnimation,
        kRefAnimationNode,
        kRefAnimationNodeType,
        kRefAnimateSound,
        kRefAnimateSoundComp
    };
};

class plGUIProgressCtrlAccessor : public PBAccessor
{
public:
    void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t) override
    {
        if( id == plGUIProgressCtrlComponent::kRefAnimateSoundComp )
        {
            plGUIProgressCtrlComponent *comp = (plGUIProgressCtrlComponent *)owner;
            comp->NotifyDependents( FOREVER, PART_ALL, REFMSG_USER_COMP_REF_CHANGED );
        }
    }
};

static plGUIProgressCtrlAccessor    sGUIProgressCtrlAccessor;

class plGUISoundDlgProc : public ParamMap2UserDlgProc
{
protected:
    ParamID         fSoundID;
    int             fSoundItem;
    TCHAR           fTitle[ 128 ];
    
public:
    plGUISoundDlgProc( ParamID soundID, int soundItem, TCHAR *title )
    {
        fSoundID = soundID;
        fSoundItem = soundItem;
        strcpy( fTitle, title );
    }

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override
    {
        switch ( msg )
        {
        case WM_INITDIALOG:
            {
                IParamBlock2 *pb = map->GetParamBlock();
                
                INode *node = pb->GetINode( fSoundID );
                TSTR newName( node ? node->GetName() : "Pick" );
                ::SetWindowText( ::GetDlgItem( hWnd, fSoundItem ), newName );
            }
            return TRUE;
            
        case WM_COMMAND:
            if( ( HIWORD( wParam ) == BN_CLICKED ) )
            {
                if( LOWORD( wParam ) == fSoundItem )
                {
                    IParamBlock2 *pb = map->GetParamBlock();
                    plGUICtrlHitCallback hitCB( (INode *)pb->GetOwner(), pb, fSoundID, fTitle, true, GUI_SOUND_COMPONENT_ID );
                    GetCOREInterface()->DoHitByNameDialog( &hitCB );
                    
                    INode* node = pb->GetINode( fSoundID );
                    TSTR newName( node ? node->GetName() : "Pick" );
                    ::SetWindowText( ::GetDlgItem(hWnd, fSoundItem ), newName );
                    map->Invalidate( fSoundID );
                    ::InvalidateRect(hWnd, nullptr, TRUE);
                    return TRUE;
                }
            }
            break;
        }
        
        return FALSE;
    }
    
    void DeleteThis() override { }
};

static plGUISoundDlgProc            sGUIProgressCtrlSndProc( plGUIProgressCtrlComponent::kRefAnimateSoundComp, IDC_GUI_ANIMSNDCOMP,
                                            "Select the sound to play when this control animates" );

static plPlasmaAnimSelectDlgProc    sGUIProgressCtrlProc( plGUIProgressCtrlComponent::kRefAnimation, IDC_GUI_COMPSELBTN, 
                                                    plGUIProgressCtrlComponent::kRefAnimationNode, plGUIProgressCtrlComponent::kRefAnimationNodeType, IDC_GUI_ANIMNODESEL, 
                                                    "Select the animation to use when displaying this knob control", &sGUIProgressCtrlSndProc );

//Max desc stuff necessary below.
CLASS_DESC(plGUIProgressCtrlComponent, gGUIProgressCtrlDesc, "GUI Progress Control",  "GUIProgressCtrl", COMP_TYPE_GUI, GUI_PROGRESS_CLASSID )

ParamBlockDesc2 gGUIProgressCtrlBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIProgressCtrl"), 0, &gGUIProgressCtrlDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUIPROGRESS, IDS_COMP_GUIPROGRESS, 0, 0, &sGUIProgressCtrlProc,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,
    
    plGUIProgressCtrlComponent::kRefMinValue, _T("minValue"),   TYPE_FLOAT, 0, 0,   
        p_default, 0.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_LOWER, IDC_GUI_LOWER_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIProgressCtrlComponent::kRefMaxValue, _T("maxValue"),   TYPE_FLOAT, 0, 0,   
        p_default, 10.0f,
        p_range, -10000.f, 10000.f,         // WHY do we even need to specify this?
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_UPPER, IDC_GUI_UPPER_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIProgressCtrlComponent::kRefStep, _T("step"),   TYPE_FLOAT, 0, 0,   
        p_default, 1.0f,
        p_ui, plGUIControlBase::kRollMain, TYPE_SPINNER,    EDITTYPE_POS_FLOAT, IDC_GUI_STEP, IDC_GUI_STEP_SPIN, SPIN_AUTOSCALE,
        end,

    plGUIProgressCtrlComponent::kReverseValues, _T("reverseValues"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REVERSE,
        p_default, FALSE,
        end,

    plGUIProgressCtrlComponent::kRefAnimation, _T("animation"), TYPE_INODE,     0, 0,
        p_prompt, IDS_COMP_GUI_SELECTANIM,
        end,

    plGUIProgressCtrlComponent::kRefAnimationNode, _T("animationNode"), TYPE_INODE,     0, 0,
        end,

    plGUIProgressCtrlComponent::kRefAnimationNodeType, _T("animationNodeType"), TYPE_INT,       0, 0,
        p_default, plAnimObjInterface::kUseOwnerNode,
        end,

    plGUIProgressCtrlComponent::kRefAnimateSound,   _T( "animateSound" ), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_ANIMSND,
        p_default, FALSE,
        p_enable_ctrls, 1, plGUIProgressCtrlComponent::kRefAnimateSoundComp,
        end,
        
    plGUIProgressCtrlComponent::kRefAnimateSoundComp, _T("animateSoundComp"),   TYPE_INODE,     0, 0,
        p_accessor, &sGUIProgressCtrlAccessor,
        end,

    end
);

plGUIProgressCtrlComponent::plGUIProgressCtrlComponent()
{
    fClassDesc = &gGUIProgressCtrlDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIProgressCtrlComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    node->SetForceLocal( true );

    plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
    if (iface != nullptr && iface->MightRequireSeparateMaterial())
    {
        INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                ? fCompPB->GetINode( kRefAnimationNode )
                                : (INode *)node;

        if (restrict != nullptr)
        {
            node->SetForceMaterialCopy( true );
        }
    }

    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIProgressCtrlComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

// For hackery below (see warning below)
#include "plAnimation/plAGMasterMod.h"

bool plGUIProgressCtrlComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIProgressCtrl *ctrl = (pfGUIProgressCtrl *)fControl;
    
    ctrl->SetRange( fCompPB->GetFloat( kRefMinValue ), fCompPB->GetFloat( kRefMaxValue ) );
    ctrl->SetStep( fCompPB->GetFloat( kRefStep ) );

    if( fCompPB->GetInt( kReverseValues ) )
        ctrl->SetFlag( pfGUIProgressCtrl::kReverseValues );

    // Get the animation to use
    plAnimObjInterface *iface = plAnimComponentBase::GetAnimInterface( fCompPB->GetINode( kRefAnimation ) );
    if (iface != nullptr)
    {
        INode *restrict = ( fCompPB->GetInt( kRefAnimationNodeType ) == plAnimObjInterface::kUseParamBlockNode ) 
                                ? fCompPB->GetINode( kRefAnimationNode )
                                : (INode *)node;


        std::vector<plKey> keys;
        if (iface->GetKeyList(restrict, keys) && !keys.empty())
            ctrl->SetAnimationKeys( keys, iface->GetIfaceSegmentName( false ) );
    }
    else
    {
        // HACKERY WARNING: Old knobs assumed the animation was just the same one applied to our node,
        // so to avoid breaking old formats, if we can't grab an animObjInterface, we just grab the key
        // of the master mod of our node, like we would've before
        plAGMasterMod   *master = node->GetAGMasterMod();
        ctrl->SetAnimationKeys({master->GetKey()}, ENTIRE_ANIMATION_NAME);
    }

    const char *errMsg = ISetSoundIndex( kRefAnimateSound, kRefAnimateSoundComp, pfGUIProgressCtrl::kAnimateSound, node );
    if (errMsg != nullptr)
    {
        pErrMsg->Set( true, "GUI Sound Event Error", errMsg, node->GetName() ).Show();
        pErrMsg->Set( false );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIClickMap Component ///////////////////////////////////////////////////////////////////
//
//  GUI element that just keeps track of where on its surface (from 0-1) that it was clicked.
//
/////////////////////////////////////////////////////////////////////////////////////////////////


// Class that accesses the paramblock below.
class plGUIClickMapComponent : public plGUIControlBase
{
protected:

    pfGUIControlMod *IGetNewControl() override { return new pfGUIClickMapCtrl; }
    bool            ICanHaveProxy() override { return false; }

public:
    plGUIClickMapComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kRefReportDragging
    };
};

//Max desc stuff necessary below.
CLASS_DESC(plGUIClickMapComponent, gGUIClickMapDesc, "GUI Clickable Map",  "GUIClickMap", COMP_TYPE_GUI, GUI_CLICKMAP_CLASSID )

ParamBlockDesc2 gGUIClickMapBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIClickMap"), 0, &gGUIClickMapDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    2,
    plGUIControlBase::kRollMain, IDD_COMP_GUICLICKMAP, IDS_COMP_GUICLICKMAP, 0, 0, nullptr,
    plGUIControlBase::kRollProc, IDD_COMP_GUIPROCROLLOUT, IDS_COMP_GUIPROCROLLOUT, 0, 0, nullptr,
    
    &sGUIControlProcParamTemplate,

    plGUIClickMapComponent::kRefReportDragging, _T("reportWhileDragging"), TYPE_BOOL, 0, 0,
        p_ui, plGUIControlBase::kRollMain, TYPE_SINGLECHEKBOX, IDC_GUI_REPORTDRAG,
        p_default, FALSE,
        end,

    end
);

plGUIClickMapComponent::plGUIClickMapComponent()
{
    fClassDesc = &gGUIClickMapDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIClickMapComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    node->SetForceLocal( true );
    return plGUIControlBase::SetupProperties( node, pErrMsg );
}

bool plGUIClickMapComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    return plGUIControlBase::PreConvert( node, pErrMsg );
}

bool plGUIClickMapComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    if( !plGUIControlBase::Convert( node, pErrMsg ) )
        return false;

    pfGUIClickMapCtrl *ctrl = (pfGUIClickMapCtrl *)fControl;
    
    if( fCompPB->GetInt( kRefReportDragging ) )
        ctrl->SetFlag( pfGUIClickMapCtrl::kReportDragging );

    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUISkin Component ////////////////////////////////////////////////////////////////////
//
//  Defines a skin to use when rendering certain GUI controls (just menus for now)
//
/////////////////////////////////////////////////////////////////////////////////////////////////


class pfGUISkinProc : public ParamMap2UserDlgProc
{
protected:

public:

    void DeleteThis() override { }

//  virtual void    Update( TimeValue t, Interval &valid, IParamMap2 *map );

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};

static pfGUISkinProc gGUISkinProc;

// Component defined in pfGUISkinProc.h

#define kDeclSkinRectValues( ref ) (plGUISkinComp::##ref + 0), _T("f##ref##.left"), TYPE_INT, 0, 0, p_default, 0, end, \
                                    (plGUISkinComp::##ref + 1), _T("f##ref##.top"), TYPE_INT, 0, 0, p_default, 0, end, \
                                    (plGUISkinComp::##ref + 2), _T("f##ref##.width"), TYPE_INT, 0, 0, p_default, 8, end, \
                                    (plGUISkinComp::##ref + 3), _T("f##ref##.height"), TYPE_INT, 0, 0, p_default, 8, end

#define kSetSkinRectValues( pb, ref, l, t, w, h ) { pb->SetValue( ref + 0, 0, (int) l ); \
                                                    pb->SetValue( ref + 1, 0, (int) t ); \
                                                    pb->SetValue( ref + 2, 0, (int) r ); \
                                                    pb->SetValue( ref + 3, 0, (int) b ); }


//Max desc stuff necessary below.
CLASS_DESC(plGUISkinComp, gGUISkinDesc, "GUI Skin",  "GUISkin", COMP_TYPE_GUI, GUI_SKIN_CLASSID )

static ParamBlockDesc2  gGUISkinBk
(
    /// Main def
     plComponent::kBlkComp, _T("GUISkin"), 0, &gGUISkinDesc, P_AUTO_CONSTRUCT + P_AUTO_UI, plComponent::kRefComp,

    IDD_COMP_GUISKIN, IDS_COMP_GUISKIN, 0, 0, &gGUISkinProc,    

    plGUISkinComp::kRefBitmap,  _T("bitmap"),       TYPE_TEXMAP,        0, 0,
        end,

    kDeclSkinRectValues( kRefUpLeftCorner ),
    kDeclSkinRectValues( kRefTopSpan ),
    kDeclSkinRectValues( kRefUpRightCorner ),
    kDeclSkinRectValues( kRefRightSpan ),
    kDeclSkinRectValues( kRefLowerRightCorner ),
    kDeclSkinRectValues( kRefBottomSpan ),
    kDeclSkinRectValues( kRefLowerLeftCorner ),
    kDeclSkinRectValues( kRefLeftSpan ),
    kDeclSkinRectValues( kRefMiddleFill ),
    kDeclSkinRectValues( kRefSelectedFill ),
    kDeclSkinRectValues( kRefSubMenuArrow ),
    kDeclSkinRectValues( kRefSelectedSubMenuArrow ),
    kDeclSkinRectValues( kRefTreeButtonClosed ),
    kDeclSkinRectValues( kRefTreeButtonOpen ),

    plGUISkinComp::kRefItemMargin,  _T("itemMargin"),       TYPE_INT,       0, 0,
        p_ui,   TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_IMARGIN, IDC_GUI_IMARGIN_SPIN, SPIN_AUTOSCALE,
        p_default, 1,
        end,

    plGUISkinComp::kRefBorderMargin,    _T("borderMargin"),     TYPE_INT,       0, 0,
        p_ui,   TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_BMARGIN, IDC_GUI_BMARGIN_SPIN, SPIN_AUTOSCALE,
        p_default, 4,
        end,

    end
);

// Editor proc
extern HINSTANCE hInstance;

INT_PTR pfGUISkinProc::DlgProc(TimeValue t, IParamMap2 *pmap, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    IParamBlock2        *pb = pmap->GetParamBlock();
    plGUISkinComp       *comp = (plGUISkinComp *)pb->GetOwner();
    PBBitmap            *bitmap;
    plLayerTex          *layer = comp->GetSkinBitmap();
    ICustButton         *bmSelectBtn;

    switch( msg )
    {
        case WM_INITDIALOG:
            // Set projection map bitmap name
            bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_GUI_SKINBMAP ) );
            if (bmSelectBtn != nullptr)
            {
                bitmap = (layer == nullptr) ? nullptr : layer->GetPBBitmap();
                if (bitmap != nullptr)
                    bmSelectBtn->SetText( (TCHAR *)bitmap->bi.Filename() );
                else
                    bmSelectBtn->SetText( _T( "<none>" ) );
                ReleaseICustButton( bmSelectBtn );
            }

            return TRUE;

        case WM_DESTROY:
            break;

        case WM_COMMAND:
            if( LOWORD( wParam ) == IDC_GUI_EDITELEM )
            {
                bitmap = (layer == nullptr) ? nullptr : layer->GetPBBitmap();
                if (bitmap != nullptr)
                {
                    pfGUISkinEditProc   proc( comp );
                    DialogBox( hInstance, MAKEINTRESOURCE( IDD_COMP_SKINEDIT ), GetCOREInterface()->GetMAXHWnd(), proc.DlgProc );
                }
            }

            else if( LOWORD( wParam ) == IDC_GUI_SKINBMAP )
            {
                BOOL selectedNewBitmap = layer->HandleBitmapSelection();
                if( selectedNewBitmap )
                {
                    bmSelectBtn = GetICustButton( GetDlgItem( hWnd, IDC_GUI_SKINBMAP ) );
                    bitmap = layer->GetPBBitmap();
                    bmSelectBtn->SetText(bitmap != nullptr ? (TCHAR *)bitmap->bi.Filename() : "");
                    ReleaseICustButton( bmSelectBtn );
                }
                return FALSE;
            }
            break;

    }
    return FALSE;
}

plKey   plGUISkinComp::GetConvertedSkinKey() const
{
    if (fConvertedSkin != nullptr)
        return fConvertedSkin->GetKey();

    return nullptr;
}

uint32_t  plGUISkinComp::GetNumMtls() const
{
    return 1;
}

Texmap  *plGUISkinComp::GetMtl( uint32_t idx )
{
    return (Texmap *)GetSkinBitmap();
}

//// GetSkinBitmap ///////////////////////////////////////////////////////////

plLayerTex  *plGUISkinComp::GetSkinBitmap()
{  
    // If we don't have one, create one
    plLayerTex  *layer = (plLayerTex *)fCompPB->GetTexmap( kRefBitmap, 0 );
    if (layer == nullptr || layer->ClassID() != LAYER_TEX_CLASS_ID)
    {
        layer = new plLayerTex;

        fCompPB->SetValue( kRefBitmap, 0, (Texmap *)layer );
    }
    if( layer )
    {
        IParamBlock2* bitmapPB = layer->GetParamBlockByID( plLayerTex::kBlkBitmap );
        if( bitmapPB->GetInt(kBmpScaling) != kScalingNone )
            bitmapPB->SetValue(kBmpScaling, TimeValue(0), kScalingNone);
    }


    return layer;
}
    
plGUISkinComp::plGUISkinComp()
{
    fClassDesc = &gGUISkinDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUISkinComp::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    fConvertedSkin = nullptr;
    return true;
}

bool plGUISkinComp::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    // Create and assign key here, so other components can grab the key later
    if (fConvertedSkin != nullptr)
        return true;        // Only convert once, since we don't care what node we're on

    Texmap *texture = fCompPB->GetTexmap( kRefBitmap );
    if (texture == nullptr || texture->ClassID() != LAYER_TEX_CLASS_ID || ((plLayerTex *)texture)->GetPBBitmap() == nullptr)
    {
        pErrMsg->Set( true, "GUI Skin Convert Error", 
                            "The GUI skin component %s doesn't have a mipmap associated with it. This skin will not "
                            "be exported.", GetINode()->GetName() ).CheckAndAsk();
        pErrMsg->Set( false );
        return true;
    }

    fConvertedSkin = new pfGUISkin();
    hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), fConvertedSkin, node->GetLocation() );

    return true;
}

bool plGUISkinComp::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    // Actually do the work of converting all the skin data
    if (fConvertedSkin == nullptr)
        return true;        // Eh?


    fConvertedSkin->SetMargins( fCompPB->GetInt( kRefItemMargin ), fCompPB->GetInt( kRefBorderMargin ) );

    uint32_t  i;
    for( i = 0; i < pfGUISkin::kNumElements; i++ )
    {
        ParamID     id = ( i * 4 ) + kRefUpLeftCorner;

        fConvertedSkin->SetElement( i, fCompPB->GetInt( id + 0 ), fCompPB->GetInt( id + 1 ), 
                                    fCompPB->GetInt( id + 2 ), fCompPB->GetInt( id + 3 ) );
    }

    plLayerTex *layer= (plLayerTex *)fCompPB->GetTexmap( kRefBitmap );
    if (layer != nullptr)
    {
        PBBitmap *texture = layer->GetPBBitmap();
        if (texture != nullptr)
        {
            plBitmap *bMap = plLayerConverter::Instance().CreateSimpleTexture( texture->bi.Name(), fConvertedSkin->GetKey()->GetUoid().GetLocation(), 0, plMipmap::kForceNonCompressed | plMipmap::kAlphaChannelFlag | plMipmap::kNoMaxSize );
            if (bMap != nullptr && plMipmap::ConvertNoRef(bMap) != nullptr)
            {
                hsgResMgr::ResMgr()->AddViaNotify( bMap->GetKey(), new plGenRefMsg( fConvertedSkin->GetKey(), 
                                        plRefMsg::kOnCreate, -1, pfGUISkin::kRefMipmap ), plRefFlags::kActiveRef );
            }
        }
    }

    return true;
}

bool plGUISkinComp::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fConvertedSkin = nullptr;
    return true; 
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//// plGUIPopUpMenu Component ///////////////////////////////////////////////////////////////////
//
//  Defines a pop-up menu, with an auto-anchor to the sceneObject it's attached to
//
/////////////////////////////////////////////////////////////////////////////////////////////////


/*class plGUIMenuProc : public ParamMap2UserDlgProc
{
protected:

    void    ILoadPages( HWND hWnd, IParamBlock2 *pb );

public:

    void DeleteThis() override { }

    BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
};
static plGUIMenuProc gGUIMenuProc;
*/

static plGUISingleCtrlDlgProc sGUISkinSelectProc( plGUIMenuComponent::kRefSkin, IDC_GUI_SKIN,
                                            "Select the skin to use for this pop-up menu", sSkinClassesToSelect,
                                            &gGUIDialogProc );

//Max desc stuff necessary below.
CLASS_DESC(plGUIMenuComponent, gGUIMenuDesc, "GUI Menu",  "GUIMenu", COMP_TYPE_GUI, GUI_MENUANCHOR_CLASSID )

ParamBlockDesc2 gGUIMenuBk
(   // KLUDGE: not the defined block ID, but kept for backwards compat.
 plComponent::kBlkComp, _T("GUIMenu"), 0, &gGUIMenuDesc, P_AUTO_CONSTRUCT + P_AUTO_UI + P_MULTIMAP + P_INCLUDE_PARAMS, plComponent::kRefComp,

    3, 
    plGUIMenuComponent::kMainRollout,   IDD_COMP_GUIMENUANCHOR, IDS_COMP_GUIMENUANCHOR, 0, 0, &sGUISkinSelectProc,
    plGUIMenuComponent::kTagIDRollout,  IDD_COMP_GUITAG,    IDS_COMP_GUITAG,    0, 0, &gGUITagProc,
    plGUIMenuComponent::kSchemeRollout, IDD_COMP_GUISCHEME, IDS_COMP_GUISCHEME, 0, 0, &gGUIColorSchemeProc, 

    &gGUIColorSchemeBk,

        plGUIMenuComponent::kRefDialogName, _T("MenuName"),     TYPE_STRING,        0, 0,
//          p_ui, plGUIMenuComponent::kMainRollout, TYPE_EDITBOX, IDC_GUIDLG_NAME,
            end,

        plGUIMenuComponent::kRefAgeName,    _T("ageName"),      TYPE_STRING,        0, 0,
            p_default, _T( "GUI" ),
            end,
            
        plGUIMenuComponent::kRefVersion,    _T("version"),      TYPE_INT,       0, 0,
            p_ui,   plGUIMenuComponent::kMainRollout, TYPE_SPINNER, EDITTYPE_POS_INT, IDC_GUI_VERSION, IDC_GUI_VERSION_SPIN, SPIN_AUTOSCALE,
            p_default, 0,
            end,

        plGUITagComponent::kRefCurrIDSel,   _T("currSel"),      TYPE_INT,       0, 0,
            end,

        plGUIMenuComponent::kRefSkin, _T("skin"),   TYPE_INODE,     0, 0,
            end,

        plGUIMenuComponent::kRefNeverClose, _T("neverClose"),       TYPE_BOOL,      0, 0,
            p_default,  FALSE,
            p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_NEVERCLOSE,
            end,        

        plGUIMenuComponent::kRefModalOutside, _T("modalOutside"),       TYPE_BOOL,      0, 0,
            p_default,  FALSE,
            p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_MODALOUTSIDE,
            end,        

        plGUIMenuComponent::kRefOpenOnHover, _T("openSubsOnHover"),     TYPE_BOOL,      0, 0,
            p_default,  FALSE,
            p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_HOVER,
            end,        

        plGUIMenuComponent::kRefAlignment, _T("alignment"), TYPE_INT, 0, 0,
            p_default, 3,
            p_ui, plGUIMenuComponent::kMainRollout, TYPE_RADIO, 4, IDC_ALIGNRADIO1, IDC_ALIGNRADIO2, IDC_ALIGNRADIO3, IDC_ALIGNRADIO4,
            end,

        plGUIMenuComponent::kRefScaleWithScreenRes, _T("maintainSizeAcrossRes"),        TYPE_BOOL,      0, 0,
            p_default,  FALSE,
            p_ui, plGUIMenuComponent::kMainRollout, TYPE_SINGLECHEKBOX, IDC_GUI_SCALERES,
            end,        

    end
);

plGUIMenuComponent::plGUIMenuComponent() : plGUIDialogComponent( true )
{
    fClassDesc = &gGUIMenuDesc;
    fClassDesc->MakeAutoParamBlocks(this);
}

pfGUIDialogMod  *plGUIMenuComponent::IMakeDialog()
{
    return new pfGUIPopUpMenu();
}

plKey   plGUIMenuComponent::GetConvertedMenuKey() const
{
    if (fConvertedMenu == nullptr)
        return nullptr;

    return fConvertedMenu->GetKey();
}

// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
// of properties on the MaxNode, as it's still indeterminant.
bool plGUIMenuComponent::SetupProperties(plMaxNode *node,  plErrorMsg *pErrMsg)
{
//  return plGUIDialogComponent::SetupProperties( node, pErrMsg );
    fConvertedMenu = nullptr;
    return true;
}

bool plGUIMenuComponent::Convert(plMaxNode *node, plErrorMsg *pErrMsg)
{
    pfGUIPopUpMenu *menu = fConvertedMenu;

//  bool    b = plGUIDialogComponent::Convert( node, pErrMsg );
//  if( b )
    {
//      pfGUIPopUpMenu *menu = pfGUIPopUpMenu::ConvertNoRef( fDialogMod );
//      hsAssert(menu != nullptr, "Somehow got a bad poitner in GUIMenu::Convert()");

        INode *sNode = fCompPB->GetINode( kRefSkin );
        if (sNode != nullptr)
        {
            plComponentBase *comp = ( (plMaxNode *)sNode )->ConvertToComponent();
            if (comp != nullptr)
            {
                Class_ID nodeID = comp->ClassID();
                hsAssert( nodeID == GUI_SKIN_CLASSID, "Bad node param in GUIMenu::Convert()" );

                plGUISkinComp *skin = (plGUISkinComp *)comp;
                menu->SetSkin( skin->GetConvertedSkin() );
            }
        }

        if( fCompPB->GetInt( kRefNeverClose ) )
            menu->SetFlag( pfGUIPopUpMenu::kStayOpenAfterClick );

        if( fCompPB->GetInt( kRefModalOutside ) )
            menu->SetFlag( pfGUIPopUpMenu::kModalOutsideMenus );

        if( fCompPB->GetInt( kRefOpenOnHover ) )
            menu->SetFlag( pfGUIPopUpMenu::kOpenSubMenusOnHover );

        if( fCompPB->GetInt( kRefScaleWithScreenRes ) )
            menu->SetFlag( pfGUIPopUpMenu::kScaleWithResolution );

        switch( fCompPB->GetInt( kRefAlignment ) )
        {
            case 0: menu->SetAlignment( pfGUIPopUpMenu::kAlignUpLeft ); break;
            case 1: menu->SetAlignment( pfGUIPopUpMenu::kAlignUpRight ); break;
            case 2: menu->SetAlignment( pfGUIPopUpMenu::kAlignDownLeft ); break;
            case 3: menu->SetAlignment( pfGUIPopUpMenu::kAlignDownRight ); break;
        }
    }

    // Note: we use the owning dialog of our anchor object as the context, i.e. who translates
    // our point at runtime into screen coordinates
    menu->SetOriginAnchor( node->GetSceneObject(), plGUIDialogComponent::GetNodeDialog( node ) );

    const plLocation &loc = menu->GetKey()->GetUoid().GetLocation();

    // Create the rendermod
    plPostEffectMod *renderMod = new plPostEffectMod;
    hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), renderMod, loc );

    renderMod->SetHither( 0.5f );
    renderMod->SetYon( 200.f );
    renderMod->SetNodeKey( fConvertedNode );

    float scrnWidth = 20.f;

    // fovX should be such that scrnWidth is the projected width at z=100
    float fovX = atan( scrnWidth / ( 2.f * 100.f ) ) * 2.f;
    float fovY = fovX;// * 3.f / 4.f;

    renderMod->SetFovX(hsRadiansToDegrees(fovX));
    renderMod->SetFovY(hsRadiansToDegrees(fovY));


    hsgResMgr::ResMgr()->AddViaNotify( renderMod->GetKey(), new plNodeRefMsg( fConvertedNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kGeneric ), plRefFlags::kActiveRef );
    hsgResMgr::ResMgr()->AddViaNotify( fConvertedNode, new plGenRefMsg( renderMod->GetKey(), plRefMsg::kOnCreate, 0, plPostEffectMod::kNodeRef ), plRefFlags::kPassiveRef );        

    menu->SetRenderMod( renderMod );
    menu->SetName( fCompPB->GetStr( kRefDialogName ) );

    // Create the dummy scene object to hold the menu
    plSceneObject   *newObj = new plSceneObject;
    hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), newObj, loc );

    // *#&$(*@&#$ need a coordIface...
    plCoordinateInterface *newCI = new plCoordinateInterface;
    hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), newCI, loc );


    hsgResMgr::ResMgr()->AddViaNotify( menu->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );      

    hsgResMgr::ResMgr()->AddViaNotify( newCI->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );        
    hsgResMgr::ResMgr()->AddViaNotify( renderMod->GetKey(), new plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, -1, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );     

    newObj->SetSceneNode( fConvertedNode );
    menu->SetSceneNodeKey( fConvertedNode );

    {
        hsMatrix44 l2w, w2l;
        l2w.Reset();
        l2w.GetInverse( &w2l );
        newObj->SetTransform( l2w, w2l );
    }

    // Should be done now...
    return true;
}

bool plGUIMenuComponent::PreConvert(plMaxNode *node,  plErrorMsg *pErrMsg)
{
    // Create and assign key here, so other components can grab the key later
    if (fConvertedMenu != nullptr)
        return true;        // Only convert once, since we don't care what node we're on

    /// Create an entirely new sceneNode for us
    int32_t seqNum = plPageInfoUtils::GetSeqNumFromAgeDesc( fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ) );
    int32_t newNum = plPluginResManager::ResMgr()->VerifySeqNumber( seqNum, fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ) );
    if( newNum != seqNum )
    {
        if( !fSeqNumValidated )
        {
            char errMsg[ 512 ];
            sprintf( errMsg, "GUI Menu Component %s has an invalid location sequence number (0x%X). Temporarily using a valid one (0x%X).", 
                                node->GetName(), seqNum, newNum );
            pErrMsg->Set( true, "PageInfo Convert Error", errMsg ).Show(); 
            pErrMsg->Set( false );
            fSeqNumValidated = true;
        }
        seqNum = newNum;
    }

    fConvertedNode = plPluginResManager::ResMgr()->NameToLoc( fCompPB->GetStr( kRefAgeName ), fCompPB->GetStr( kRefDialogName ), seqNum );
    if( !fConvertedNode )
    {
        pErrMsg->Set( true, "GUI Menu Component Error", "GUI MenuComponent %s has a Missing Location.  Nuke the files in the dat directory and re-export.",((INode*)node)->GetName()).Show(); 
        return false;
    }

    fConvertedMenu = new pfGUIPopUpMenu();
    hsgResMgr::ResMgr()->NewKey( IGetUniqueName(node), fConvertedMenu, fConvertedNode->GetUoid().GetLocation() );

    return true;

//  return plGUIDialogComponent::PreConvert( node, pErrMsg );
}

bool plGUIMenuComponent::DeInit(plMaxNode *node, plErrorMsg *pErrMsg)
{
    fConvertedMenu = nullptr;
    fConvertedNode = nullptr;
    return true;
}


