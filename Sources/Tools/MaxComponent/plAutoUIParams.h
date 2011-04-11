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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "max.h"
#include "iparamb2.h"
#include "hsTypes.h"
#include "hsTemplates.h"
#include <vector>

class plKey;
class plComponentBase;

class plAutoUIParam
{
protected:
	ParamID fID;
	char *fName;
	HWND fhDlg;
	int fHeight;

	std::vector<HWND> fControlVec;

	ParamID fVisID;
	std::vector<std::string> fVisStates;

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
	};

	plAutoUIParam(ParamID id, const char *name);
	virtual ~plAutoUIParam();

	int Create(HWND hDlg, IParamBlock2 *pb, int yOffset);
	virtual int  CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset) = 0;
	virtual bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb) = 0;

	ParamID GetID() { return fID; }

	virtual void Show(int yOffset);
	void Hide();
	int GetHeight();

	void SetVisInfo(ParamID id, std::vector<std::string>* states);
	bool CheckVisibility(ParamID id, std::string state);

	virtual int GetParamType();
	virtual hsBool GetBool(IParamBlock2 *pb);
	virtual float GetFloat(IParamBlock2 *pb);
	virtual int GetInt(IParamBlock2 *pb);
	virtual const char* GetString(IParamBlock2 *pb);

	virtual int GetCount(IParamBlock2 *pb);
	virtual plKey GetKey(IParamBlock2 *pb, int idx=0);
	virtual plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);

protected:
	int  ISizeControl(HWND hDlg, HWND hControl, int w, int h, int y, int x=3);
	HWND ICreateControl(HWND hDlg, const char *className, const char *wndName=nil, DWORD style=0, DWORD exStyle=0);
	void ISetControlFont(HWND hControl);

	int IAddStaticText(HWND hDlg, int y, const char *text);
};

class plCheckBoxParam : public plAutoUIParam
{
protected:
	HWND fhCheck;
	
public:
	plCheckBoxParam(ParamID id, const char *name);
	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	hsBool GetBool(IParamBlock2 *pb);
};

class plSpinnerParam : public plAutoUIParam
{
protected:
	HWND fhSpinner;
	bool fIsFloat;	// True if this is a float spinner, false if it is an int
	
public:
	plSpinnerParam(ParamID id, const char *name, bool isFloat);
	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	float GetFloat(IParamBlock2 *pb);
	int GetInt(IParamBlock2 *pb);

	void Show(int yOffset);
};

class plEditParam : public plAutoUIParam
{
protected:
	HWND fhEdit;
	int fLines;
	
public:
	plEditParam(ParamID id, const char *name, int lines);
	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	const char* GetString(IParamBlock2 *pb);
	void Show(int yOffset);
};

class plPickListParam : public plAutoUIParam
{
protected:
	HWND fhList;
	HWND fhAdd;
	HWND fhRemove;
	std::vector<Class_ID> fCIDs;	
	
public:
	plPickListParam(ParamID id, const char *name, std::vector<Class_ID>* filter);
	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	int GetCount(IParamBlock2 *pb);
	plKey GetKey(IParamBlock2 *pb, int idx=0);

	void Show(int yOffset);

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
	plPickButtonParam(ParamID id, const char *name, std::vector<Class_ID>* filter, bool canConvertToType);

	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	int GetCount(IParamBlock2 *pb);
	plKey GetKey(IParamBlock2 *pb, int idx=0);

	void SetPickNode(INode *node, IParamBlock2 *pb);

	void Show(int yOffset);
};

class plPickComponentButtonParam : public plPickButtonParam
{
public:
	plPickComponentButtonParam(ParamID id, const char *name, std::vector<Class_ID>* filter, bool canConvertToType);

	int GetParamType();
	plComponentBase* GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickComponentListParam : public plPickListParam
{
public:
	plPickComponentListParam(ParamID id, const char *name, std::vector<Class_ID>* filter);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickActivatorButtonParam : public plPickButtonParam
{
public:
	plPickActivatorButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickActivatorListParam : public plPickListParam
{
public:
	plPickActivatorListParam(ParamID id, const char *name);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickDynamicTextButtonParam : public plPickButtonParam
{
public:
	plPickDynamicTextButtonParam(ParamID id, const char *name);

	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	const char* GetString(IParamBlock2 *pb);
	int GetCount(IParamBlock2 *pb);
	plKey GetKey(IParamBlock2 *pb, int idx=0);
};

class plPickSingleComponentButtonParam : public plPickButtonParam
{
protected:
	int			fMyType;
	Class_ID	fClassToPick;
public:
	plPickSingleComponentButtonParam(ParamID id, const char *name, int myType, Class_ID myClassToPick);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickExcludeRegionButtonParam : public plPickButtonParam
{
public:
	plPickExcludeRegionButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickWaterComponentButtonParam : public plPickButtonParam
{
public:
	plPickWaterComponentButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickSwimCurrentInterfaceButtonParam : public plPickButtonParam
{
public:
	plPickSwimCurrentInterfaceButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickClusterComponentButtonParam : public plPickButtonParam
{
public:
	plPickClusterComponentButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickAnimationButtonParam : public plPickButtonParam
{
public:
	plPickAnimationButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickBehaviorButtonParam : public plPickButtonParam
{
public:
	plPickBehaviorButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};

class plPickMaterialButtonParam : public plPickButtonParam
{
public:
	plPickMaterialButtonParam(ParamID id, const char *name);

	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	const char* GetString(IParamBlock2 *pb);
	int GetCount(IParamBlock2 *pb);
	plKey GetKey(IParamBlock2 *pb, int idx=0);
};

class plPickMaterialAnimationButtonParam : public plPickButtonParam
{
protected:
	hsTArray<plKey> fKeys;

public:
	plPickMaterialAnimationButtonParam(ParamID id, const char *name);

	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	const char* GetString(IParamBlock2 *pb);
	int GetCount(IParamBlock2 *pb);
	plKey GetKey(IParamBlock2 *pb, int idx=0);

	void CreateKeyArray(IParamBlock2* pb);
	void DestroyKeyArray();
};

class plDropDownListParam : public plAutoUIParam
{
protected:
	HWND fhList;
	std::vector<std::string> fOptions;	
	
public:
	plDropDownListParam(ParamID id, const char *name, std::vector<std::string>* options);
	int CreateControls(HWND hDlg, IParamBlock2 *pb, int yOffset);
	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	int GetCount(IParamBlock2 *pb);
	const char* GetString(IParamBlock2 *pb);
	
	void Show(int yOffset);

protected:
	void IUpdateList(IParamBlock2 *pb);
};

class plPickGrassComponentButtonParam : public plPickButtonParam
{
public:
	plPickGrassComponentButtonParam(ParamID id, const char *name);

	bool IsMyMessage(UINT msg, WPARAM wParam, LPARAM lParam, IParamBlock2 *pb);

	int GetParamType();
	plComponentBase *GetComponent(IParamBlock2 *pb, int idx=0);
};
