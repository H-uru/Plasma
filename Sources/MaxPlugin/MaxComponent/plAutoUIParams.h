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

#ifndef plAutoUIParams_inc
#define plAutoUIParams_inc

#include <unordered_set>

#include <string_theory/string>
#include <tchar.h>

class plKey;
class plComponentBase;

class plAutoUIParam
{
protected:
    ParamID fID;
    ST::string fName;
    HWND fhDlg;
    int fHeight;

    std::vector<HWND> fControlVec;

    ParamID fVisID;
    std::unordered_set<ST::string> fVisStates;

public:
    // Types returned by GetParamType
    enum
    {
        kTypeNone,

        kTypeBool,
        kTypeFloat,
        kTypeInt,
        kTypeString,

        kTypeSceneObj,
        kTypeActivator,
        kTypeComponent,
        kTypeDynamicText,
        kTypeGUIDialog,
        kTypeExcludeRegion,
        kTypeAnimation,
        kTypeBehavior,
        kTypeMaterial,
        kTypeGUIPopUpMenu,
        kTypeGUISkin,
        kTypeWaterComponent,
        kTypeDropDownList,
        kTypeSwimCurrentInterface,
        kTypeClusterComponent,
        kTypeMaterialAnimation,
        kTypeGrassComponent,
        kTypeLayer
    };

    plAutoUIParam(ParamID id, ST::string name);
    virtual ~plAutoUIParam();

    int Create(HWND hDlg, IParamBlock2 *pb, int yOffset);
    virtual int  CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) = 0;
    virtual bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) = 0;

    ParamID GetID() { return fID; }

    virtual void Show(int yOffset);
    void Hide();
    int GetHeight();

    void SetVisInfo(ParamID id, std::unordered_set<ST::string> states);
    bool CheckVisibility(ParamID id, const ST::string& state);

    virtual int GetParamType();
    virtual bool GetBool(IParamBlock2 *pb);
    virtual float GetFloat(IParamBlock2 *pb);
    virtual int GetInt(IParamBlock2 *pb);
    virtual const MCHAR* GetString(IParamBlock2 *pb);

    virtual int GetCount(IParamBlock2 *pb);
    virtual plKey GetKey(IParamBlock2 *pb, int idx=0);
    virtual plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);

protected:
    int  ISizeControl(HWND hDlg, HWND hControl, int w, int h, int y, int x=3);
    HWND ICreateControl(HWND hDlg, const TCHAR* className, const TCHAR* wndName=nullptr, DWORD style=0, DWORD exStyle=0);
    void ISetControlFont(HWND hControl);

    int IAddStaticText(HWND hDlg, int y, const TCHAR* text);
};

class plCheckBoxParam : public plAutoUIParam
{
protected:
    HWND fhCheck;
    
public:
    plCheckBoxParam(ParamID id, ST::string name);
    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    bool GetBool(IParamBlock2 *pb) override;
};

class plSpinnerParam : public plAutoUIParam
{
protected:
    HWND fhSpinner;
    bool fIsFloat;  // True if this is a float spinner, false if it is an int
    
public:
    plSpinnerParam(ParamID id, ST::string name, bool isFloat);
    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    float GetFloat(IParamBlock2 *pb) override;
    int GetInt(IParamBlock2 *pb) override;

    void Show(int yOffset) override;
};

class plEditParam : public plAutoUIParam
{
protected:
    HWND fhEdit;
    int fLines;
    
public:
    plEditParam(ParamID id, ST::string name, int lines);
    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    const MCHAR* GetString(IParamBlock2 *pb) override;
    void Show(int yOffset) override;
};

class plPickListParam : public plAutoUIParam
{
protected:
    HWND fhList;
    HWND fhAdd;
    HWND fhRemove;
    std::vector<Class_ID> fCIDs;    
    
public:
    plPickListParam(ParamID id, ST::string name, std::vector<Class_ID>* filter);
    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    int GetCount(IParamBlock2 *pb) override;
    plKey GetKey(IParamBlock2 *pb, int idx=0) override;

    void Show(int yOffset) override;

protected:
    void IUpdateList(IParamBlock2 *pb);
};

class plPickButtonParam : public plAutoUIParam
{
protected:
    ICustButton *fButton;
    std::vector<Class_ID> fCIDs;
    bool fCanConvertToType;
    HWND fhRemove;

public:
    plPickButtonParam(ParamID id, ST::string name, std::vector<Class_ID>* filter, bool canConvertToType);

    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    int GetCount(IParamBlock2 *pb) override;
    plKey GetKey(IParamBlock2 *pb, int idx=0) override;

    void SetPickNode(INode *node, IParamBlock2 *pb);

    void Show(int yOffset) override;
};

class plPickComponentButtonParam : public plPickButtonParam
{
public:
    plPickComponentButtonParam(ParamID id, ST::string name, std::vector<Class_ID>* filter, bool canConvertToType);

    int GetParamType() override;
    plComponentBase* GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickComponentListParam : public plPickListParam
{
public:
    plPickComponentListParam(ParamID id, ST::string name, std::vector<Class_ID>* filter);
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickActivatorButtonParam : public plPickButtonParam
{
public:
    plPickActivatorButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickActivatorListParam : public plPickListParam
{
public:
    plPickActivatorListParam(ParamID id, ST::string name);
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickDynamicTextButtonParam : public plPickButtonParam
{
public:
    plPickDynamicTextButtonParam(ParamID id, ST::string name);

    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    const MCHAR* GetString(IParamBlock2 *pb) override;
    int GetCount(IParamBlock2 *pb) override;
    plKey GetKey(IParamBlock2 *pb, int idx=0) override;
};

class plPickSingleComponentButtonParam : public plPickButtonParam
{
protected:
    int         fMyType;
    Class_ID    fClassToPick;
public:
    plPickSingleComponentButtonParam(ParamID id, ST::string name, int myType, Class_ID myClassToPick);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickExcludeRegionButtonParam : public plPickButtonParam
{
public:
    plPickExcludeRegionButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickWaterComponentButtonParam : public plPickButtonParam
{
public:
    plPickWaterComponentButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickSwimCurrentInterfaceButtonParam : public plPickButtonParam
{
public:
    plPickSwimCurrentInterfaceButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickClusterComponentButtonParam : public plPickButtonParam
{
public:
    plPickClusterComponentButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickAnimationButtonParam : public plPickButtonParam
{
public:
    plPickAnimationButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickBehaviorButtonParam : public plPickButtonParam
{
public:
    plPickBehaviorButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

class plPickMaterialButtonParam : public plPickButtonParam
{
public:
    plPickMaterialButtonParam(ParamID id, ST::string name);

    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    const MCHAR* GetString(IParamBlock2 *pb) override;
    int GetCount(IParamBlock2 *pb) override;
    plKey GetKey(IParamBlock2 *pb, int idx=0) override;
};

class plPickMaterialAnimationButtonParam : public plPickButtonParam
{
protected:
    std::vector<plKey> fKeys;

public:
    plPickMaterialAnimationButtonParam(ParamID id, ST::string name);

    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    const MCHAR* GetString(IParamBlock2 *pb) override;
    int GetCount(IParamBlock2 *pb) override;
    plKey GetKey(IParamBlock2 *pb, int idx=0) override;

    void CreateKeyArray(IParamBlock2* pb);
    void DestroyKeyArray();
};

class plPickLayerButtonParam : public plPickMaterialButtonParam
{
public:
    plPickLayerButtonParam(ParamID id, ST::string name)
        : plPickMaterialButtonParam(id, std::move(name))
    { }

    int GetParamType() override { return kTypeLayer; }
    plKey GetKey(IParamBlock2 *pb, int idx = 0) override;
};

class plDropDownListParam : public plAutoUIParam
{
protected:
    HWND fhList;
    std::vector<ST::string> fOptions;
    
public:
    plDropDownListParam(ParamID id, ST::string name, std::vector<ST::string> options = {});
    int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) override;
    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    int GetCount(IParamBlock2 *pb) override;
    const MCHAR* GetString(IParamBlock2 *pb) override;
    
    void Show(int yOffset) override;

protected:
    void IUpdateList(IParamBlock2 *pb);
};

class plPickGrassComponentButtonParam : public plPickButtonParam
{
public:
    plPickGrassComponentButtonParam(ParamID id, ST::string name);

    bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) override;

    int GetParamType() override;
    plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0) override;
};

#endif
