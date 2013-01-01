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

#include "MaxComponent/plAnimObjInterface.h"
#include "MaxComponent/plMaxAnimUtils.h"

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

    SegmentSpec     *IGetSegmentSpec( void ) const;

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
    void DeleteThis() { delete this; }

    INode           *GetINode( void );
    plPassMtlBase   *GetParentMtl( void );
    void            SetParentMtl( plPassMtlBase *parent );
    void            SetNodeName( const char *parentName );

    // Create the dialog for this object and place it inside the given dialog, centering it in the given control if any
    bool    CreateAndEmbedDlg( IParamMap2 *parentMap, IMtlParams *parentParams, HWND frameCtrl = nil );

    // Release said dialog
    void    ReleaseDlg( void );

    // Switch underlying objects in the dialog (to avoid unnecessary deletion/recreations)
    void    SwitchDlg( plAnimStealthNode *toSwitchTo );

    // Get the actual window handle of the currently active dialog displaying us
    HWND    GetWinDlg( void ) const;

    // Interesting functions
    plString    GetSegmentName( void ) const;
    void        SetSegment( const char *name ); // nil for "entire animation"

    // Conversion from stealth's INode to the actual object
    static bool                 CanConvertToStealth( INode *objNode );
    static plAnimStealthNode    *ConvertToStealth( INode *objNode );

    ///////////////////////////////////////////////////////////////////////////////////////
    // Required Max functions
    //
    TCHAR* GetObjectName()      { return (TCHAR*)fClassDesc->ClassName(); }
    void InitNodeName(TSTR& s)  { s = fClassDesc->InternalName(); }
    void GetClassName(TSTR& s)  { s = fClassDesc->ClassName(); }
    Class_ID ClassID()          { return ANIMSTEALTH_CLASSID; }      

    RefTargetHandle Clone(RemapDir &remap);
    
    int NumRefs();
    RefTargetHandle GetReference(int i);
    void SetReference(int i, RefTargetHandle rtarg);
    RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message);
    
    // allow retreival of our paramblock from other plug-ins
    // and the max core
    int NumParamBlocks();
    IParamBlock2* GetParamBlock(int i);
    IParamBlock2* GetParamBlockByID(BlockID id);

    // We override because we don't want to be able to animate this sucker
    int         NumSubs() { return 0; }
    Animatable  *SubAnim( int i ) { return nil; }
    TSTR        SubAnimName( int i ) { return fClassDesc->ClassName(); }

    // plug-in mouse creation callback
    CreateMouseCallBack* GetCreateMouseCallBack();

    void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev);
    void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next);
//  void SelectionSetChanged(Interface *ip, IUtil *iu);
    
    void BuildMesh(TimeValue t);
    void FreeCaches();
    void GetLocalBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box);
    void GetWorldBoundBox(TimeValue t, INode *node, ViewExp *vpt, Box3 &box);
    int Display(TimeValue t, INode *node, ViewExp *vpt, int flags);
    int HitTest(TimeValue t, INode *node, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
    ObjectState Eval(TimeValue t) { return ObjectState(this); }

    IOResult Save(ISave* isave);
    IOResult Load(ILoad* iload);

    int CanConvertToType( Class_ID obtype ) { return ( obtype == ANIMSTEALTH_CLASSID ) ? 1 : 0; }

    const char *GetCategory() { return fClassDesc->Category(); }

    /// Parameter access

    bool    GetAutoStart( void ) const;
    void    SetAutoStart( bool b );

    bool        GetLoop( void ) const;
    plString    GetLoopName( void ) const;
    void        SetLoop( bool b, const plString &name );

    uint8_t     GetEaseInType( void ) const;
    float       GetEaseInLength( void ) const;
    float       GetEaseInMin( void ) const;
    float       GetEaseInMax( void ) const;
    void        SetEaseIn( uint8_t type, float length, float min, float max );

    uint8_t     GetEaseOutType( void ) const;
    float       GetEaseOutLength( void ) const;
    float       GetEaseOutMin( void ) const;
    float       GetEaseOutMax( void ) const;
    void        SetEaseOut( uint8_t type, float length, float min, float max );

    // Conversion stuff
    void        GetAllStopPoints( hsTArray<float> &out );
    float       GetSegStart( void ) const;
    float       GetSegEnd( void ) const;
    void        GetLoopPoints( float &start, float &end ) const;
    void        StuffToTimeConvert( plAnimTimeConvert &convert, float maxLength );

    // plAnimObjInterface functions
    virtual void    PickTargetNode( IParamBlock2 *destPB, ParamID destParamID, ParamID typeID );
    virtual bool    IsNodeRestricted( void ) { return true; }
    virtual plString GetIfaceSegmentName( bool allowNil );
    virtual bool    GetKeyList( INode *restrictedNode, hsTArray<plKey> &outKeys );
    virtual bool        MightRequireSeparateMaterial( void ) { return true; }

    // Convert time, called on the setupProps pass for each material applied to a node in the scene
    virtual bool    SetupProperties( plMaxNode *node, plErrorMsg *pErrMsg );
    virtual bool    ConvertDeInit( plMaxNode *node, plErrorMsg *pErrMsg );

    // Returns true if the parent material is applied to any node in the scene, false otherwise
    bool            IsParentUsedInScene( void );
};

//// Accessor for Parent's ParamBlock ////////////////////////////////////////

class plStealthNodeAccessor : public PBAccessor
{
    protected:

        void    ISetParent( ReferenceTarget *target, plPassMtlBase *parent );

        void    IHandleSet( PB2Value &v, ReferenceMaker *owner, ParamID id, int tabIndex, TimeValue t );

    public:

        plStealthNodeAccessor() { }
        static plStealthNodeAccessor    &GetInstance( void );
        
        virtual void    Set( PB2Value &v, ReferenceMaker *owner, ParamID id, int tabIndex, TimeValue t );
        virtual void    TabChanged( tab_changes changeCode, Tab<PB2Value> *tab, ReferenceMaker *owner, 
                                            ParamID id, int tabIndex, int count );
};

#endif //_plAnimStealthNode_h 