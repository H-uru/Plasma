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

#ifndef _plAnimStealthNode_h
#define _plAnimStealthNode_h

#include <vector>

#include "MaxComponent/plAnimObjInterface.h"
#include "MaxComponent/plMaxAnimUtils.h"

#include "MaxMain/MaxCompat.h"

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define ANIMSTEALTH_CLASSID     Class_ID(0xd0272f8, 0x750349c9)

#define REFMSG_NOTETRACK_ADDED  REFMSG_USER + 1

class plAnimTimeConvert;
class plErrorMsg;
class plKey;
class plMaxNode;
class INode;
class NoteTrack;
class plPassMtlBase;

//// Class Def ///////////////////////////////////////////////////////////////

class plAnimStealthNode : public HelperObject, public plAnimObjInterface
{
protected:
    ClassDesc2   *fClassDesc;

    IParamBlock2    *fParamBlock;
    plPassMtlBase   *fParentMtl;

    static ParamBlockDesc2 sAnimStealthPB;

    bool            fPreppedForConvert;
    SegmentMap      *fCachedSegMap;

    SegmentSpec     *IGetSegmentSpec() const;

public:

    // Some IDs
    enum
    {
        kBlockPB = 0
    };

    enum Refs
    {
        kRefParamBlock,
        kRefParentMtl
    };
    // ParamBlock IDs
    enum ParamBlockIDs
    {
        kPBAutoStart,           // Start the Animation on load  (V2)
        kPBLoop,                // Start Looping at Begin Location
        kPBName,                // Name of the notetrack animation to play
        kPBLoopName,            // Name of the notetrack specified loop
        kPBEaseInType,
        kPBEaseOutType,
        kPBEaseInLength,
        kPBEaseOutLength,
        kPBEaseInMin,
        kPBEaseInMax,
        kPBEaseOutMin,
        kPBEaseOutMax
    };

    plAnimStealthNode( BOOL loading );
    virtual ~plAnimStealthNode();
    void DeleteThis() override { delete this; }

    INode           *GetINode();
    plPassMtlBase   *GetParentMtl();
    void            SetParentMtl( plPassMtlBase *parent );
    void            SetNodeName( const MCHAR* parentName );

    // Create the dialog for this object and place it inside the given dialog, centering it in the given control if any
    bool    CreateAndEmbedDlg(IParamMap2 *parentMap, IMtlParams *parentParams, HWND frameCtrl = nullptr);

    // Release said dialog
    void    ReleaseDlg();

    // Switch underlying objects in the dialog (to avoid unnecessary deletion/recreations)
    void    SwitchDlg( plAnimStealthNode *toSwitchTo );

    // Get the actual window handle of the currently active dialog displaying us
    HWND    GetWinDlg() const;

    // Interesting functions
    ST::string  GetSegmentName() const;
    void        SetSegment( const ST::string& name ); // nil for "entire animation"

    // Conversion from stealth's INode to the actual object
    static bool                 CanConvertToStealth( INode *objNode );
    static plAnimStealthNode    *ConvertToStealth( INode *objNode );

    ///////////////////////////////////////////////////////////////////////////////////////
    // Required Max functions
    //
    MAX14_CONST MCHAR* GetObjectName(MAX_NAME_LOCALIZED1 MAX_NAME_LOCALIZED_DEFAULT) MAX24_CONST override
    {
        return const_cast<MAX14_CONST MCHAR*>(fClassDesc->ClassName());
    }

    void InitNodeName(TSTR& s) override { s = fClassDesc->InternalName(); }
    void GetClassName(MSTR& s MAX_NAME_LOCALIZED2) MAX24_CONST override { s = fClassDesc->ClassName(); }
    Class_ID ClassID() override         { return ANIMSTEALTH_CLASSID; }

    RefTargetHandle Clone(RemapDir &remap) override;
    
    int NumRefs() override;
    RefTargetHandle GetReference(int i) override;
    void SetReference(int i, RefTargetHandle rtarg) override;
    RefResult NotifyRefChanged(MAX_REF_INTERVAL changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message MAX_REF_PROPAGATE) override;
    
    // allow retreival of our paramblock from other plug-ins
    // and the max core
    int NumParamBlocks() override;
    IParamBlock2* GetParamBlock(int i) override;
    IParamBlock2* GetParamBlockByID(BlockID id) override;

    // We override because we don't want to be able to animate this sucker
    int         NumSubs() override { return 0; }
    Animatable  *SubAnim(int i) override { return nullptr; }
    MSTR        SubAnimName(int i MAX_NAME_LOCALIZED2) override { return fClassDesc->ClassName(); }

    // plug-in mouse creation callback
    CreateMouseCallBack* GetCreateMouseCallBack() override;

    void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev) override;
    void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next) override;
//  void SelectionSetChanged(Interface *ip, IUtil *iu);
    
    void BuildMesh(TimeValue t);
    void FreeCaches() override;
    void GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;
    void GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box) override;
    int Display(TimeValue t, INode *node, ViewExp *vpt, int flags) override;
    int HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) override;
    ObjectState Eval(TimeValue t) override { return ObjectState(this); }

    IOResult Save(ISave* isave) override;
    IOResult Load(ILoad* iload) override;

    int CanConvertToType(Class_ID obtype) override { return (obtype == ANIMSTEALTH_CLASSID) ? 1 : 0; }

    const MCHAR* GetCategory() { return fClassDesc->Category(); }

    /// Parameter access

    bool    GetAutoStart() const;
    void    SetAutoStart( bool b );

    bool        GetLoop() const;
    ST::string  GetLoopName() const;
    void        SetLoop( bool b, const ST::string &name );

    uint8_t     GetEaseInType() const;
    float       GetEaseInLength() const;
    float       GetEaseInMin() const;
    float       GetEaseInMax() const;
    void        SetEaseIn( uint8_t type, float length, float min, float max );

    uint8_t     GetEaseOutType() const;
    float       GetEaseOutLength() const;
    float       GetEaseOutMin() const;
    float       GetEaseOutMax() const;
    void        SetEaseOut( uint8_t type, float length, float min, float max );

    // Conversion stuff
    void        GetAllStopPoints(std::vector<float> &out);
    float       GetSegStart() const;
    float       GetSegEnd() const;
    void        GetLoopPoints( float &start, float &end ) const;
    void        StuffToTimeConvert( plAnimTimeConvert &convert, float maxLength );

    // plAnimObjInterface functions
    void    PickTargetNode(IParamBlock2 *destPB, ParamID destParamID, ParamID typeID) override;
    bool    IsNodeRestricted() override { return true; }
    ST::string GetIfaceSegmentName(bool allowNil) override;
    bool    GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys) override;
    bool        MightRequireSeparateMaterial() override { return true; }

    // Convert time, called on the setupProps pass for each material applied to a node in the scene
    virtual bool    SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg );
    virtual bool    ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg );

    // Returns true if the parent material is applied to any node in the scene, false otherwise
    bool            IsParentUsedInScene();
};

//// Accessor for Parent's ParamBlock ////////////////////////////////////////

class plStealthNodeAccessor : public PBAccessor
{
    protected:

        void    ISetParent( ReferenceTarget *target, plPassMtlBase *parent );

        void    IHandleSet( PB2Value &v, ReferenceMaker *owner, ParamID id, int tabIndex, TimeValue t );

    public:

        plStealthNodeAccessor() { }
        static plStealthNodeAccessor    &GetInstance();
        
        void    Set(PB2Value &v, ReferenceMaker *owner, ParamID id, int tabIndex, TimeValue t) override;
        void    TabChanged(tab_changes changeCode, Tab<PB2Value> *tab, ReferenceMaker *owner,
                           ParamID id, int tabIndex, int count) override;
};

#endif //_plAnimStealthNode_h
