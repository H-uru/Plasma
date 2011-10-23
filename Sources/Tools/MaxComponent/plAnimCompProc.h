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
#ifndef plAnimCompProc_h_inc
#define plAnimCompProc_h_inc

#include "max.h"
#include "iparamm2.h"

class plComponentBase;
class plMaxNode;

class plAnimCompProc : public ParamMap2UserDlgProc
{
protected:
    int fCompButtonID;
    int fCompParamID;
    int fNodeButtonID;
    int fNodeParamID;

    virtual void IPickComponent(IParamBlock2* pb)=0;
    virtual void IPickNode(IParamBlock2* pb, plComponentBase* comp);
    
    virtual void ILoadUser(HWND hWnd, IParamBlock2* pb)=0;
    virtual bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)=0;

    plMaxNode* IGetNode(IParamBlock2* pb);
    void IClearNode(IParamBlock2* pb);
    plComponentBase* IGetComp(IParamBlock2* pb);

    void ICompButtonPress(HWND hWnd, IParamBlock2* pb);
    void INodeButtonPress(HWND hWnd, IParamBlock2* pb);

    void IUpdateCompButton(HWND hWnd, IParamBlock2* pb);
    virtual void IUpdateNodeButton(HWND hWnd, IParamBlock2* pb);
    
public:
    plAnimCompProc();
    virtual void DeleteThis() {}

    virtual BOOL DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool GetCompAndNode(IParamBlock2* pb, plComponentBase*& comp, plMaxNode*& node);
};


///////////////////////////////////////////////////////////////////////////////////////////


class plMtlAnimProc : public ParamMap2UserDlgProc
{
protected:
    int fMtlButtonID;
    int fMtlParamID;
    int fNodeButtonID;
    int fNodeParamID;
    int fAnimComboID;
    int fAnimParamID;

    Mtl* IGetMtl(IParamBlock2* pb);

    virtual void IOnInitDlg(HWND hWnd, IParamBlock2* pb) {}
    virtual void ILoadUser(HWND hWnd, IParamBlock2* pb)=0;
    virtual bool IUserCommand(HWND hWnd, IParamBlock2* pb, int cmd, int resID)=0;

    virtual void IPickNode(IParamBlock2* pb);

    virtual void ISetNodeButtonText(HWND hWnd, IParamBlock2* pb);

public:
    plMtlAnimProc();
    void DeleteThis() {}

    BOOL DlgProc(TimeValue t, IParamMap2* pm, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void IMtlButtonPress(HWND hWnd, IParamBlock2* pb);
    void INodeButtonPress(HWND hWnd, IParamBlock2* pb);
    void IAnimComboChanged(HWND hWnd, IParamBlock2* pb);

    void IUpdateMtlButton(HWND hWnd, IParamBlock2* pb);
    void IUpdateNodeButton(HWND hWnd, IParamBlock2* pb);
    void ILoadAnimCombo(HWND hWnd, IParamBlock2* pb);
};

#endif // plAnimCompProc_h_inc