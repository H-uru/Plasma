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
#ifndef plAnimComponent_inc
#define plAnimComponent_inc

#include "pnKeyedObject/plKey.h"

#include "plComponent.h"
#include "plComponentReg.h"
#include "plAnimObjInterface.h"
#include "plNotetrackDlgComp.h"

#include <iparamm2.h>
#include <map>


#define ANIM_COMP_CID Class_ID(0x32e77ab, 0x28a80383)
#define ANIM_GROUP_COMP_CID Class_ID(0x341a57fc, 0x4cda6c64)
#define ANIM_COMPRESS_COMP_CID Class_ID(0x116d3175, 0x4e465807)

//This enum is necessary and can only be appended.
//This is used in the ParamBlock2Desc.
enum
{
        kAnimRadio_DEAD,
        kAnimAutoStart,         // Start the Animation on load  (V2)
        kAnimLoop,              // Start Looping at Begin Location
        kAnimBegin_DEAD,
        kAnimEnd_DEAD,
        kAnimLoopSegCkBx_DEAD,
        kAnimLoopSegBeg_DEAD,
        kAnimLoopSegEnd_DEAD,
        kAnimName,              // Name of the notetrack animation to play
        kAnimLoopSegBegBox_DEAD,
        kAnimLoopSegEndBox_DEAD,
        kAnimUseGlobal,
        kAnimGlobalName,
        kAnimLoopName,          // Name of the notetrack specified loop
        kAnimEaseInType,
        kAnimEaseOutType,
        kAnimEaseInLength,
        kAnimEaseOutLength,
        kAnimEaseInMin,
        kAnimEaseInMax,
        kAnimEaseOutMin,
        kAnimEaseOutMax,
        kAnimPhysAnim,
};

class plAGAnim;
class plAGApplicator;
class plAGMasterMod;
class plComponentBase;
class plController;
class plEaseAccessor;
class plMaxNode;
class plMsgForwarder;
class plSimpleTMModifier;
class plLightModifier;

class plAnimComponentBase : public plComponent, public plAnimObjInterface
{
protected:
    std::map<plMaxNode*, plAGMasterMod*> fMods;
    std::map<plMaxNode*, plLightModifier*> fLightMods;
    bool fNeedReset;

public:
    plAnimComponentBase();
    void DeleteThis() override { delete this; }

    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;

    virtual plKey GetModKey(plMaxNode *node)=0;
    ST::string GetAnimName();
    static bool IsAnimComponent(plComponentBase *comp);

    std::map<plMaxNode*, plAGAnim*> fAnims;

    // Static function for setting up scalar controllers
    static void SetupCtl( plAGAnim *anim, plController *ctl, plAGApplicator *app, plMaxNode *node );

    // Static function to grab the animObjInterface for a given INode, regardless of type
    static plAnimObjInterface   *GetAnimInterface( INode *node );

    // plAnimObjInterface functions
    void    PickTargetNode(IParamBlock2 *destPB, ParamID destParamID, ParamID typeID) override;
    bool    IsNodeRestricted() override { return true; }
    ST::string GetIfaceSegmentName(bool allowNil) override;

protected:
    bool IAddTMToAnim(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg);
    bool IAddLightToAnim(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg);
    bool IConvertNodeSegmentBranch(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg);
    bool IMakePersistent(plMaxNode *node, plAGAnim *anim, plErrorMsg *pErrMsg);
};

class plAnimComponent : public plAnimComponentBase
{
public:
    plAnimComponent();
    plKey GetModKey(plMaxNode *node) override;
    bool  GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys) override;
};

class plAnimGroupedComponent : public plAnimComponentBase
{
protected:
    plMsgForwarder *fForward;

public:
    plAnimGroupedComponent();

    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    plKey GetModKey(plMaxNode *node) override;

    bool    IsNodeRestricted() override { return false; }
    bool    GetKeyList(INode *restrictedNode, std::vector<plKey> &outKeys) override;
};

//// Dialog Proc For Anim Selection /////////////////////////////////////////////////////////////
//  Derive from this guy to handle selection of an animation. Also, you can pass a pointer in
//  to another dialog proc to chain procs together (this proc will execute before the chained
//  one).

class plPlasmaAnimSelectDlgProc : public ParamMap2UserDlgProc
{
protected:
    ParamID         fParamID;
    int             fDlgItem;

    bool            fUseNode;
    ParamID         fNodeParamID;
    ParamID         fTypeParamID;
    int             fNodeDlgItem;

    TCHAR           fTitle[ 128 ];

    ParamMap2UserDlgProc    *fChain;

    void    IUpdateNodeBtn( HWND hWnd, IParamBlock2 *pb );

public:

    int     GetHandledDlgItem() const { return fDlgItem; }

    // No node restriction version
    plPlasmaAnimSelectDlgProc(ParamID paramID, int dlgItem, TCHAR *promptTitle, ParamMap2UserDlgProc *chainedDlgProc = nullptr);

    // Node restricted version
    plPlasmaAnimSelectDlgProc(ParamID paramID, int dlgItem, ParamID nodeParamID, ParamID typeParamID, int nodeDlgItem, TCHAR *promptTitle, ParamMap2UserDlgProc *chainedDlgProc = nullptr);

    void    SetThing(ReferenceTarget *m) override;
    void    Update(TimeValue t, Interval &valid, IParamMap2 *map) override;
    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

    void DeleteThis() override;
};

class plAnimComponentProc : public ParamMap2UserDlgProc
{
protected:
    plComponentNoteTrackDlg fNoteTrackDlg;
    IParamBlock2 *fPB;
    
    void EnableGlobal(HWND hWnd, bool enable);
    
public:
    static void FillAgeGlobalComboBox(HWND box, const char *varName);
    static void SetBoxToAgeGlobal(HWND box, const char *varName);

    INT_PTR DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    void Update(TimeValue t, Interval &valid, IParamMap2 *map) override;
    void DeleteThis() override;
};  

class plAnimCompressComp : public plComponent
{
public:
    plAnimCompressComp();
    void DeleteThis() override { delete this; }

    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    //bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    enum
    {
        kCompressionNone,
        kCompressionLow,
        kCompressionHigh,
    };

    enum
    {
        kAnimCompressLevel,
        kAnimCompressThreshold,
    };
};

#endif // plAnimComponent_inc