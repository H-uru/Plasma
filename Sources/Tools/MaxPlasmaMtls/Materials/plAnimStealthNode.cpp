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
//  plAnimStealthNode - Stealthy hidden INode that represents a single      //
//                      segment's worth of animation info for a material.   //
//                      Stored as an INode so they can be "selected"        //
//                      by components as targets of animation messages.     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "MaxMain/MaxAPI.h"

#include "pnKeyedObject/plKey.h"

#include "../resource.h"

#include "plAnimStealthNode.h"
#include "plPassMtlBase.h"

#include "MaxComponent/plMaxAnimUtils.h"
#include "MaxComponent/plPickNodeBase.h"

extern TCHAR *GetString( int id );
extern HINSTANCE hInstance;


//// Stealthy Class Desc /////////////////////////////////////////////////////

class plStealthClassDesc : public ClassDesc2
{
public:
    int             IsPublic() override     { return FALSE; }
    void*           Create(BOOL loading) override { return new plAnimStealthNode(loading); }
    const TCHAR*    ClassName() override    { return GetString( IDS_STEALTH_NAME ); }
    SClass_ID       SuperClassID() override { return HELPER_CLASS_ID; }
    Class_ID        ClassID() override      { return ANIMSTEALTH_CLASSID; }
    const TCHAR*    Category() override     { return nullptr; }
    const TCHAR*    InternalName() override { return _T("PlasmaAnimStealthInfo"); }
    HINSTANCE       HInstance() override    { return hInstance; }
};
static plStealthClassDesc sStealthClassDesc;
ClassDesc2* GetStealthClassDesc() { return &sStealthClassDesc; }

//// plStealthDlgProc /////////////////////////////////////////////////////////
//  Dialog proc for the anim stealth child dialog

class plStealthDlgProc : public ParamMap2UserDlgProc
{
protected:
    // Combo itemdata values
    enum
    {
        kName,      // Name of an animation/loop
        kDefault,   // Default combo value
        kInvalid,   // Invalid entry (couldn't find)
    };

    SegmentMap *fSegMap;

    HWND fhWnd;

public:
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    void DeleteThis() override { IDeleteSegMap(); }
    void SetThing(ReferenceTarget *m) override;

    void Update(TimeValue t, Interval &valid, IParamMap2 *pmap) override;

protected:
    // Set all the controls to their stored value
    void IInitControls( plAnimStealthNode *stealth, IParamBlock2 *pb);

    // Deletes all the allocated memory
    void IDeleteSegMap();

    void ILoadLoops(IParamBlock2 *pb);

    void ISetSel(HWND hCombo, const char *name);
};

const char *kAnimNameNone = ENTIRE_ANIMATION_NAME;

static plStealthDlgProc sStealthDlgProc;





//// Stealthy ParamBlock Desc ////////////////////////////////////////////////

static plEaseAccessor sEaseAccessor( plAnimStealthNode::kBlockPB, plAnimStealthNode::kPBEaseInMin, 
                                        plAnimStealthNode::kPBEaseInMax, plAnimStealthNode::kPBEaseInLength,
                                        plAnimStealthNode::kPBEaseOutMin, plAnimStealthNode::kPBEaseOutMax,
                                        plAnimStealthNode::kPBEaseOutLength );

ParamBlockDesc2 plAnimStealthNode::sAnimStealthPB
(
    kBlockPB, _T("animStealth"), IDS_STEALTH_NAME, GetStealthClassDesc(),//nullptr,
                                        P_AUTO_CONSTRUCT + P_AUTO_UI, kRefParamBlock,

    // UI
    IDD_STEALTH_ANIM, IDS_STEALTH_NAME, 0, 0, &sStealthDlgProc,

    kPBName,            _T("animName"),     TYPE_STRING,        0, 0,
        end,

    kPBAutoStart,   _T("autoStart"),    TYPE_BOOL,          0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_AUTO_START,
        p_default,  FALSE,
        end,

    kPBLoop,        _T("loop"),         TYPE_BOOL,          0, 0,
        p_ui,       TYPE_SINGLECHEKBOX, IDC_LOOP,
        p_default,  TRUE,
        end,
    kPBLoopName,    _T("loopName"),     TYPE_STRING,        0, 0,
        end,

    // Anim Ease
    kPBEaseInType,  _T("easeInType"),   TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3, IDC_PASS_ANIM_EASE_IN_NONE, IDC_PASS_ANIM_EASE_IN_CONST_ACCEL, IDC_PASS_ANIM_EASE_IN_SPLINE,
        p_vals,     plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
        p_default,  plAnimEaseTypes::kNoEase,
        end,
    kPBEaseInLength,    _T("easeInLength"), TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, IDC_PASS_ANIM_EASE_IN_TIME, IDC_PASS_ANIM_EASE_IN_TIME_SPIN, 1.0,
        p_accessor, &sEaseAccessor,
        end,
    kPBEaseInMin,       _T("easeInMin"),    TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, IDC_PASS_ANIM_EASE_IN_MIN, IDC_PASS_ANIM_EASE_IN_MIN_SPIN, 1.0,
        p_accessor, &sEaseAccessor,
        end,
    kPBEaseInMax,   _T("easeInMax"),    TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, IDC_PASS_ANIM_EASE_IN_MAX, IDC_PASS_ANIM_EASE_IN_MAX_SPIN, 1.0,
        p_accessor, &sEaseAccessor,
        end,

    kPBEaseOutType, _T("easeOutType"),  TYPE_INT,       0, 0,
        p_ui,       TYPE_RADIO, 3, IDC_PASS_ANIM_EASE_OUT_NONE, IDC_PASS_ANIM_EASE_OUT_CONST_ACCEL, IDC_PASS_ANIM_EASE_OUT_SPLINE,
        p_vals,     plAnimEaseTypes::kNoEase, plAnimEaseTypes::kConstAccel, plAnimEaseTypes::kSpline,
        p_default,  plAnimEaseTypes::kNoEase,
        end,
    kPBEaseOutLength,   _T("easeOutLength"),    TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, IDC_PASS_ANIM_EASE_OUT_TIME, IDC_PASS_ANIM_EASE_OUT_TIME_SPIN, 1.0,
        p_accessor, &sEaseAccessor,
        end,
    kPBEaseOutMin,      _T("easeOutMin"),   TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, IDC_PASS_ANIM_EASE_OUT_MIN, IDC_PASS_ANIM_EASE_OUT_MIN_SPIN, 1.0,
        p_accessor, &sEaseAccessor,
        end,
    kPBEaseOutMax,  _T("easeOutMax"),   TYPE_FLOAT,     0, 0,   
        p_default, 1.0,
        p_range, 0.1, 99.0,
        p_ui,   TYPE_SPINNER,   EDITTYPE_POS_FLOAT, IDC_PASS_ANIM_EASE_OUT_MAX, IDC_PASS_ANIM_EASE_OUT_MAX_SPIN, 1.0,
        p_accessor, &sEaseAccessor,
        end,

    end
);

plAnimStealthNode::plAnimStealthNode(BOOL loading) : fClassDesc(), fParamBlock(), fParentMtl()
{
    fCachedSegMap = nullptr;
    fClassDesc = &sStealthClassDesc;
    fClassDesc->MakeAutoParamBlocks( this );
}

plAnimStealthNode::~plAnimStealthNode()
{
//  DeleteAllRefsFromMe();
}

CreateMouseCallBack *plAnimStealthNode::GetCreateMouseCallBack()
{
    return nullptr;
}

void    plAnimStealthNode::SetParentMtl( plPassMtlBase *parent )
{
    fParentMtl = parent;
}

bool    plAnimStealthNode::CanConvertToStealth( INode *objNode )
{
    return (ConvertToStealth(objNode) != nullptr);
}

plAnimStealthNode   *plAnimStealthNode::ConvertToStealth( INode *objNode )
{
    if (objNode == nullptr)
        return nullptr;

    Object *obj = objNode->GetObjectRef();
    if (obj == nullptr)
        return nullptr;

    if( obj->CanConvertToType( ANIMSTEALTH_CLASSID ) )
        return (plAnimStealthNode *)obj;

    return nullptr;
}


ST::string plAnimStealthNode::GetSegmentName() const
{
    const char *str = fParamBlock->GetStr( (ParamID)kPBName );
    if (str == nullptr || str[0] == 0)
        return ENTIRE_ANIMATION_NAME;
    return ST::string::from_utf8(str);
}

void    plAnimStealthNode::SetSegment( const char *name )
{
    if (name == nullptr || strcmp(name, ENTIRE_ANIMATION_NAME) == 0 || name[0] == 0)
        fParamBlock->SetValue( (ParamID)kPBName, 0, "" );
    else
        fParamBlock->SetValue( (ParamID)kPBName, 0, (char *)name );
}

void    plAnimStealthNode::SetNodeName( const char *parentName )
{
    INode *node = GetINode();
    if (node != nullptr)
    {
        char name[ 512 ], newName[ 512 ];
        sprintf(name, "%s : %s", parentName, GetSegmentName().c_str());

        if (GetCOREInterface()->GetINodeByName(name) != nullptr)
        {
            // For whatever reason, MakeNameUnique() doesn't ACTUALLY make a name unique!
            // So we just need to more or less do it ourselves...
            int i;
            for( i = 1; i < 1024; i++ )
            {
                sprintf( newName, "%s(%d)", name, i );
                if (GetCOREInterface()->GetINodeByName(newName) == nullptr)
                    break;
            }
            if( i == 1024 )
            {
                // You've got to be kidding me...
                char msg[ 2048 ];
                sprintf( msg, "WARNING: For some reason, we cannot find a unique name for the node '%s'. This"
                            " will most likely cause export problems. Exactly how many of these do we HAVE??",
                            name );
                hsMessageBox( msg, "WARNING!", hsMessageBoxNormal );
            }
        }
        else
            strcpy( newName, name );


        node->SetName( newName );
    }
}

int plAnimStealthNode::NumParamBlocks()
{
    return 1;
}

IParamBlock2 *plAnimStealthNode::GetParamBlock( int i )
{
    if( i == kRefParamBlock )
        return fParamBlock;

    return nullptr;
}

IParamBlock2 *plAnimStealthNode::GetParamBlockByID( BlockID id )
{
    if( fParamBlock && fParamBlock->ID() == id )
        return fParamBlock;

    return nullptr;
}

RefTargetHandle plAnimStealthNode::Clone(RemapDir &remap)
{
    plAnimStealthNode *obj = (plAnimStealthNode *)fClassDesc->Create( false );
    // Do the base clone
    BaseClone(this, obj, remap);
    // Copy our references
    if (fParamBlock)
        obj->ReplaceReference( kRefParamBlock, fParamBlock->Clone( remap ) );

    return obj;
}

void plAnimStealthNode::BuildMesh(TimeValue t)
{
}

void plAnimStealthNode::FreeCaches()
{
}

void plAnimStealthNode::GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box)
{
    box.MakeCube(Point3(0,0,0), 0);
}

void plAnimStealthNode::GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box)
{
    box.MakeCube(Point3(0,0,0), 0);
}

int plAnimStealthNode::Display(TimeValue t, INode *node, ViewExp *vpt, int flags)
{
    return 0;
}

int plAnimStealthNode::HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
    return 0;
}

int plAnimStealthNode::NumRefs()
{
    return 1;
}

RefTargetHandle plAnimStealthNode::GetReference( int i )
{
    if( i == kRefParamBlock )
        return fParamBlock;
    else if( i == kRefParentMtl )
        return fParentMtl;

    return nullptr;
}

void plAnimStealthNode::SetReference( int i, RefTargetHandle rtarg )
{
    if( i == kRefParamBlock )
        fParamBlock = (IParamBlock2 *)rtarg;
    else if( i == kRefParentMtl )
        fParentMtl = (plPassMtlBase *)rtarg;
}

RefResult plAnimStealthNode::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message)
{
    return REF_SUCCEED;
}

IOResult plAnimStealthNode::Save(ISave* isave)
{
    return IO_OK;
}

IOResult plAnimStealthNode::Load(ILoad* iload)
{
    return IO_OK;
}

plPassMtlBase   *plAnimStealthNode::GetParentMtl()
{
    return fParentMtl;
}

class plGetRefs : public DependentEnumProc
{
    public:

        hsTArray<ReferenceMaker *>  fList;

        plGetRefs() { }

        int proc(ReferenceMaker *rmaker) override
        {
            fList.Append( rmaker );
            return DEP_ENUM_CONTINUE;
        }
};

bool        plAnimStealthNode::IsParentUsedInScene()
{
    if (GetParentMtl() == nullptr)
        return false;

    // There are two possibilities: either a node uses us and thus has a ref to us,
    // or a multi-sub uses us that a node has a ref to us.

    // Note: we could do the loop as a helper function, but we only do it twice,
    // so it's not *really* worth the effort...
    //// NOTE: the following doesn't seem to work, but keeping here in case it ever does. 
    //// What really actually finds something is the enum dependents loop below
    const char *mtlName = GetParentMtl()->GetName();

    DependentIterator di(this);
    ReferenceMaker* item = di.Next();
    while (item != nullptr)
    {
        TSTR s;
        item->GetClassName( s );

        if( item->SuperClassID() == BASENODE_CLASS_ID && !CanConvertToStealth( (INode *)( item ) ) )
            return true;        // Horray, a node has a ref to us!

        else if( item->ClassID() == Class_ID(MULTI_CLASS_ID,0) )
        {
            // Multi-sub, run the refs on that guy (we only go one up)
            Mtl *multisub = (Mtl *)item;

            DependentIterator sub(multisub);
            ReferenceMaker* item2 = sub.Next();
            while (item2 != nullptr)
            {
                if( item2->SuperClassID() == BASENODE_CLASS_ID )
                    return true;        // Horray, a node has a ref to us!
                item2 = sub.Next();
            }

            // No go, keep trying
        }
        else if( item->SuperClassID() == MATERIAL_CLASS_ID )
        {
            int q = 0;
        }

        item = di.Next();
    }

    // Enum dependents
    plGetRefs callback;
    ENUMDEPENDENTS(GetParentMtl(), &callback);
    for(int i = 0; i < callback.fList.GetCount(); i++ )
    {
        ReferenceMaker *maker = callback.fList[ i ];

        TSTR s;
        maker->GetClassName( s );

        if( maker->SuperClassID() == BASENODE_CLASS_ID && !CanConvertToStealth( (INode *)maker ) )
            return true;        // Horray, a node has a ref to us!
    }
    return false;
}

INode *plAnimStealthNode::GetINode()
{
    // Go through the reflist looking for RefMakers with a ref to this component.
    // There should only be one INode in this list.
    DependentIterator di(this);
    ReferenceMaker* item = di.Next();
    while( item )
    {
        if( item->SuperClassID() == BASENODE_CLASS_ID )
            return (INode *)item;

        item = di.Next();
    }

    return nullptr;
}

void plStealthDlgProc::Update(TimeValue t, Interval& valid, IParamMap2* pmap)
{
    // Does the pmap match our pmap?

}

//// plStealthMouseOverrideProc //////////////////////////////////////////////
//  Because of wonderful linking problems with the MAX libraries, we can't
//  actually use CreateChildMParamMap2 like we should. So instead, we use
//  CreateChildCPParamMap2. However, *that* function calls the wrong interface
//  to handle untrapped mouse messages, with the result that clicking and 
//  dragging scrolls the command pane (where components are displayed) instead
//  of the material editor pane.
//  To override this, we subclass each dialog so that we can capture the mouse
//  messages before MAX processes them and then reroute them appropriately.
//  Note: because MAX already uses the window long of the given window, we can't
//  store the old proc of the window. However, since we always use 
//  CreateChildCPParamMap2, and because the MAX source code shows us that it
//  always uses the same dialog proc for all windows created with that function,
//  we can simply store the address of that proc the first time we subclass and
//  use it for restoring every time thereafter (see the following DlgProc)

static WNDPROC  sOldStealthDlgProc = nullptr;

static INT_PTR CALLBACK plStealthMouseOverrideProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    IParamMap2 *map = (IParamMap2 *)GetWindowLongPtr( hWnd, GWLP_USERDATA );

    switch( msg )
    {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            {
                // We don't want the COREInterface to process our mouse messages with RollupMouseMessage;
                // rather, we want IMtlParams to do it just like it would if we could actually call
                // CreateChildMParamMap2
                IParamBlock2 *pb = map->GetParamBlock();
                if (pb != nullptr)
                {
                    plAnimStealthNode *stealth = (plAnimStealthNode *)pb->GetOwner();
                    if (stealth != nullptr)
                    {
                        plPassMtlBase *mtl = (plPassMtlBase *)stealth->GetParentMtl();
                        mtl->fIMtlParams->RollupMouseMessage( hWnd, msg, wParam, lParam );
                    }
                }
            return 0;
            }
    }

    if (sOldStealthDlgProc != nullptr)
        return CallWindowProc( sOldStealthDlgProc, hWnd, msg, wParam, lParam );
    else
        return 0;
}

INT_PTR plStealthDlgProc::DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    IParamBlock2 *pb = map->GetParamBlock();
    plAnimStealthNode *stealth = (plAnimStealthNode *)pb->GetOwner();

    switch (msg)
    {
        case WM_INITDIALOG:
        {
            // Install our override proc so we can capture mouse messages ourselves.
            // Note that the first time, we grab the old proc so we can restore with that
            // one every time after, since they should always be the same proc
            WNDPROC old = (WNDPROC)SetWindowLongPtr( hWnd, DWLP_DLGPROC, (LONG_PTR)plStealthMouseOverrideProc );
            if (sOldStealthDlgProc == nullptr)
                sOldStealthDlgProc = old;

            fhWnd = hWnd;
            IInitControls( stealth, pb );

            return TRUE;
        }

        case WM_DESTROY:
            // Restore our old proc
            SetWindowLongPtr( hWnd, DWLP_DLGPROC, (LONG_PTR)sOldStealthDlgProc );
            break;

        case WM_ENABLE:
            // The entire dialog was either enabled or disabled. 
            break;

    case WM_COMMAND:
        // Loop selection changed
        if( LOWORD( wParam ) == IDC_LOOPS && HIWORD( wParam ) == CBN_SELCHANGE )
        {
            // If a loop is selected, save it
            HWND hCombo = (HWND)lParam;
            int sel = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            if( sel != CB_ERR )
            {
                if( SendMessage( hCombo, CB_GETITEMDATA, sel, 0 ) == kName )
                {
                    char buf[256];
                    SendMessage( hCombo, CB_GETLBTEXT, sel, (LPARAM)buf );
                    pb->SetValue( (ParamID)plAnimStealthNode::kPBLoopName, 0, buf );
                }
                else
                    pb->SetValue( (ParamID)plAnimStealthNode::kPBLoopName, 0, "" );
            }

            return TRUE;
        }

        // Auto-start or loop checkbox checked
        if( LOWORD( wParam ) == IDC_LOOP && HIWORD( wParam ) == BN_CLICKED )
        {
            BOOL checked = ( SendMessage( (HWND)lParam, BM_GETCHECK, 0, 0 ) == BST_CHECKED );

            pb->SetValue( plAnimStealthNode::kPBLoop, 0, checked );
            EnableWindow( GetDlgItem( hWnd, IDC_LOOPS ), checked );
            return TRUE;
        }

        // Refresh clicked
        else if( LOWORD( wParam ) == IDC_REFRESH_ANIMS && HIWORD( wParam ) == BN_CLICKED )
        {
            IInitControls( stealth, pb );
            return TRUE;
        }

        break;
    }

    return FALSE;
}

void plStealthDlgProc::SetThing(ReferenceTarget *m)
{
    plAnimStealthNode *stealth = (plAnimStealthNode *)m;
    IParamBlock2 *pb = stealth->GetParamBlockByID( plAnimStealthNode::kBlockPB );

    IInitControls( stealth, pb );
}

void plStealthDlgProc::IDeleteSegMap()
{
    // If we have a segment map, delete the memory associated with it
    DeleteSegmentMap( fSegMap );
    fSegMap = nullptr;
}

void plStealthDlgProc::ISetSel(HWND hCombo, const char *name)
{
    // If there is a name, try and set that
    if( name && strcmp( name, "" ) )
    {
        int idx = (int)SendMessage(hCombo, CB_FINDSTRINGEXACT, -1, (LPARAM)name);
        // If we can't find the saved name add a "not found" entry, so they know what it was
        if( idx == -1 )
        {
            char buf[256];
            sprintf( buf, "(not found) %s", name );
            idx = (int)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
            SendMessage( hCombo, CB_SETITEMDATA, idx, kInvalid );
        }

        SendMessage( hCombo, CB_SETCURSEL, idx, 0 );
    }
    // No name, set it to none
    else
    {
        int count = (int)SendMessage(hCombo, CB_GETCOUNT, 0, 0);
        for( int i = 0; i < count; i++ )
        {
            if( SendMessage( hCombo, CB_GETITEMDATA, i, 0 ) == kDefault )
                SendMessage( hCombo, CB_SETCURSEL, i, 0 );
        }
    }
}

void plStealthDlgProc::IInitControls( plAnimStealthNode *stealth, IParamBlock2 *pb )
{
    IDeleteSegMap();

    if (stealth->GetParentMtl() != nullptr)
    {
        fSegMap = GetAnimSegmentMap(stealth->GetParentMtl(), nullptr);

        ILoadLoops( pb );
    }
    else
    {
        // ?? What should we do?
        fSegMap = nullptr;
        hsStatusMessage( "No parent material yet in plStealthDlgProc::IInitControls()...not good..." );
    }

    // Enable/disable the loop dropdown
    EnableWindow( GetDlgItem( fhWnd, IDC_LOOPS ), pb->GetInt( (ParamID)plAnimStealthNode::kPBLoop ) );
}

void plStealthDlgProc::ILoadLoops(IParamBlock2 *pb)
{
    HWND hLoops = GetDlgItem( fhWnd, IDC_LOOPS );
    SendMessage( hLoops, CB_RESETCONTENT, 0, 0 );

    // Add the default option
    int defIdx = (int)SendMessage(hLoops, CB_ADDSTRING, 0, (LPARAM)ENTIRE_ANIMATION_NAME);
    SendMessage( hLoops, CB_SETITEMDATA, defIdx, kDefault );

    ST::string segName = ST::string::from_utf8( pb->GetStr( (ParamID)plAnimStealthNode::kPBName ) );
    if (segName.empty() || fSegMap == nullptr)
    {
        // Default of "entire animation", no other loop options
        SendMessage( hLoops, CB_SETCURSEL, defIdx, 0 );
        return;
    }
    
    SegmentSpec *animSpec = (*fSegMap)[ segName ];
    if( animSpec && fSegMap )
    {
        // for each segment we found: 
        for( SegmentMap::iterator i = fSegMap->begin(); i != fSegMap->end(); i++ )
        {
            SegmentSpec *spec = (*i).second;

            if( spec->fType == SegmentSpec::kLoop )
            {
                // If the loop is contained by the animation, add it
                if( (spec->fStart == -1 || spec->fStart >= animSpec->fStart) &&
                    (spec->fEnd   == -1 || spec->fEnd   <= animSpec->fEnd) )
                {
                    // Add the name
                    int idx = (int)SendMessage(hLoops, CB_ADDSTRING, 0, (LPARAM)spec->fName.c_str());
                    SendMessage( hLoops, CB_SETITEMDATA, idx, kName );
                }       
            }
        }
    }

    ISetSel( hLoops, pb->GetStr( (ParamID)plAnimStealthNode::kPBLoopName ) );
}

void plAnimStealthNode::BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev)
{
    fClassDesc->BeginEditParams(ip, this, flags, prev);
}

void plAnimStealthNode::EndEditParams(IObjParam *ip, ULONG flags, Animatable *next)
{
    fClassDesc->EndEditParams(ip, this, flags, next);
}

//// ReleaseDlg //////////////////////////////////////////////////////////////

void    plAnimStealthNode::ReleaseDlg()
{
    IParamMap2 *map = fParamBlock->GetMap();
    fParamBlock->SetMap(nullptr);
    if (map != nullptr)
        DestroyChildCPParamMap2( map );
}

//// SwitchDlg ///////////////////////////////////////////////////////////////
//  Switch underlying objects in the dialog (to avoid unnecessary deletion/
//  recreations)

void    plAnimStealthNode::SwitchDlg( plAnimStealthNode *toSwitchTo )
{
    IParamMap2 *map = fParamBlock->GetMap();

    fParamBlock->SetMap(nullptr);
    toSwitchTo->fParamBlock->SetMap( map );

    map->SetParamBlock( toSwitchTo->fParamBlock );
    map->SetThing( (ReferenceTarget *)toSwitchTo );
    map->Invalidate();
    map->UpdateUI( 0 );
}

//// CreateAndEmbedDlg ///////////////////////////////////////////////////////
//  Create the dialog for this object and place it inside the given dialog, 
//  centering it in the given control if any.

bool    plAnimStealthNode::CreateAndEmbedDlg( IParamMap2 *parentMap, IMtlParams *parentParams, HWND frameCtrl )
{
    IParamMap2 *map = CreateChildCPParamMap2( fParamBlock, GetCOREInterface(), hInstance,
                                            parentMap, MAKEINTRESOURCE( IDD_STEALTH_ANIM ),
                                            nullptr, &sStealthDlgProc);
    fParamBlock->SetMap( map );

    if (frameCtrl != nullptr)
    {
        HWND child = fParamBlock->GetMap()->GetHWnd();
        RECT childFrame, centerFrame;

        ::GetClientRect( child, &childFrame );
        ::GetWindowRect( frameCtrl, &centerFrame );
        ::MapWindowPoints(nullptr, parentMap->GetHWnd(), (POINT *)&centerFrame, 2);

        int frameWidth = centerFrame.right - centerFrame.left;
        int frameHeight = centerFrame.bottom - centerFrame.top;
        int childWidth = childFrame.right - childFrame.left;
        int childHeight = childFrame.bottom - childFrame.top;

        ::OffsetRect( &childFrame, ( frameWidth - childWidth ) >> 1, ( frameHeight - childHeight ) >> 1 );      
        ::OffsetRect( &childFrame, centerFrame.left, centerFrame.top );     

        ::SetWindowPos(child, nullptr, childFrame.left, childFrame.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    return true;
}

//// GetWinDlg ///////////////////////////////////////////////////////////////
//  Get the actual window handle of the currently active dialog displaying us

HWND    plAnimStealthNode::GetWinDlg() const
{
    IParamMap2 *map = fParamBlock->GetMap();
    if (map != nullptr)
        return map->GetHWnd();

    return nullptr;
}

//// Picker Dialog for Restricted Animation Components //////////////////////////////////////////

class plPickAnimStealthNode : public plPickMtlNode
{
protected:
    ParamID fTypeID;

    void IAddUserType(HWND hList) override
    {
        int type = fPB->GetInt(fTypeID);

        int idx = ListBox_AddString( hList, kUseParamBlockNodeString );
        if (type == plAnimObjInterface::kUseParamBlockNode && !fPB->GetINode(fNodeParamID))
            ListBox_SetCurSel(hList, idx);


        idx = ListBox_AddString( hList, kUseOwnerNodeString );
        if (type == plAnimObjInterface::kUseOwnerNode)
            ListBox_SetCurSel(hList, idx);
    }

    void ISetUserType(plMaxNode* node, const char* userType) override
    {
        if( strcmp( userType, kUseParamBlockNodeString ) == 0 )
        {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseParamBlockNode);
        }
        else if( strcmp(userType, kUseOwnerNodeString ) == 0 )
        {
            ISetNodeValue(nullptr);
            fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseOwnerNode);
        }
        else
            fPB->SetValue(fTypeID, 0, plAnimObjInterface::kUseParamBlockNode);
    }

public:
    plPickAnimStealthNode(IParamBlock2* pb, ParamID nodeParamID, ParamID typeID, Mtl *mtl) :
      plPickMtlNode(pb, nodeParamID, mtl), fTypeID(typeID)
    {
    }
};

//// plAnimObjInterface Functions ////////////////////////////////////////////

void    plAnimStealthNode::PickTargetNode( IParamBlock2 *destPB, ParamID destParamID, ParamID typeID )
{
    plPickAnimStealthNode   pick( destPB, destParamID, typeID, (Mtl *)GetParentMtl() );
    pick.DoPick();
}

ST::string plAnimStealthNode::GetIfaceSegmentName( bool allowNil )
{
    // When sending messages to material animations, they're already addressed for the right
    // layer, no need for a segment name
    return ST::string();
}

//// Parameter Access Functions //////////////////////////////////////////////

bool    plAnimStealthNode::GetAutoStart() const   { return (bool)fParamBlock->GetInt( (ParamID)kPBAutoStart ); }
void    plAnimStealthNode::SetAutoStart( bool b )       { fParamBlock->SetValue( (ParamID)kPBAutoStart, 0, (int)b ); };

bool        plAnimStealthNode::GetLoop() const                    { return fParamBlock->GetInt( (ParamID)kPBLoop ); }
ST::string  plAnimStealthNode::GetLoopName() const                { return ST::string::from_utf8( fParamBlock->GetStr( (ParamID)kPBLoopName ) ); }
void        plAnimStealthNode::SetLoop( bool b, const ST::string &name )
{
    fParamBlock->SetValue( (ParamID)kPBLoop, 0, (int)b );
    fParamBlock->SetValue( (ParamID)kPBLoopName, 0, (char *)name.c_str() );
}

uint8_t       plAnimStealthNode::GetEaseInType() const      { return (uint8_t)fParamBlock->GetInt( (ParamID)kPBEaseInType ); }
float    plAnimStealthNode::GetEaseInLength() const    { return (float)fParamBlock->GetFloat( (ParamID)kPBEaseInLength ); }
float    plAnimStealthNode::GetEaseInMin() const       { return (float)fParamBlock->GetFloat( (ParamID)kPBEaseInMin ); }
float    plAnimStealthNode::GetEaseInMax() const       { return (float)fParamBlock->GetFloat( (ParamID)kPBEaseInMax ); }
void        plAnimStealthNode::SetEaseIn( uint8_t type, float length, float min, float max )
{
    fParamBlock->SetValue( (ParamID)kPBEaseInType, 0, (int)type );
    fParamBlock->SetValue( (ParamID)kPBEaseInLength, 0, (float)length );
    fParamBlock->SetValue( (ParamID)kPBEaseInMin, 0, (float)min );
    fParamBlock->SetValue( (ParamID)kPBEaseInMax, 0, (float)max );
}

uint8_t       plAnimStealthNode::GetEaseOutType() const     { return (uint8_t)fParamBlock->GetInt( (ParamID)kPBEaseOutType ); }
float    plAnimStealthNode::GetEaseOutLength() const   { return (float)fParamBlock->GetFloat( (ParamID)kPBEaseOutLength ); }
float    plAnimStealthNode::GetEaseOutMin() const      { return (float)fParamBlock->GetFloat( (ParamID)kPBEaseOutMin ); }
float    plAnimStealthNode::GetEaseOutMax() const      { return (float)fParamBlock->GetFloat( (ParamID)kPBEaseOutMax ); }
void        plAnimStealthNode::SetEaseOut( uint8_t type, float length, float min, float max )
{
    fParamBlock->SetValue( (ParamID)kPBEaseOutType, 0, (int)type );
    fParamBlock->SetValue( (ParamID)kPBEaseOutLength, 0, (float)length );
    fParamBlock->SetValue( (ParamID)kPBEaseOutMin, 0, (float)min );
    fParamBlock->SetValue( (ParamID)kPBEaseOutMax, 0, (float)max );
}

//// Parent Accessor Functions ///////////////////////////////////////////////

plStealthNodeAccessor   &plStealthNodeAccessor::GetInstance()
{
    static plStealthNodeAccessor    instance;
    return instance;
}

void    plStealthNodeAccessor::ISetParent( ReferenceTarget *target, plPassMtlBase *parent )
{
    if (target != nullptr && target->ClassID() == ANIMSTEALTH_CLASSID)
    {
        ( (plAnimStealthNode *)target )->SetParentMtl( parent );
    }
}

void    plStealthNodeAccessor::TabChanged( tab_changes changeCode, Tab<PB2Value> *tab, ReferenceMaker *owner, 
                                            ParamID id, int tabIndex, int count )
{
    if( changeCode == tab_insert || changeCode == tab_append )
    {
        if( owner->SuperClassID() != MATERIAL_CLASS_ID )
            return;
        plPassMtlBase *mtl = (plPassMtlBase *)owner;

        while( count > 0 )
        {
            ISetParent( (*tab)[ tabIndex ].r, mtl );
            tabIndex++;
            count--;
        }
    }
    else if( changeCode == tab_delete || changeCode == tab_ref_deleted )
    {
        // How are we supposed to handle this if we don't even get a stinkin pointer??
    }
}

void    plStealthNodeAccessor::Set( PB2Value &v, ReferenceMaker *owner, ParamID id, int tabIndex, TimeValue t )
{
    // Bit of error checking
    if( owner->SuperClassID() != MATERIAL_CLASS_ID )
        return;

    plPassMtlBase *mtl = (plPassMtlBase *)owner;

    IParamBlock2 *pb = mtl->fAnimPB;

    // A stealth node paramBlock value just got set. First make sure we 
    // un-set the old stealth's parent
    ISetParent(pb->GetReferenceTarget(id, tabIndex), nullptr);
        
    // So make sure that the stealth node that was just added gets its parent mtl set properly
    ISetParent( v.r, mtl );
}

