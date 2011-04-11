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
#ifndef plAutoUIBase_h_inc
#define plAutoUIBase_h_inc

//#include "max.h"
//#include "plComponentBase.h"
//#include "plComponentReg.h"

#include "hsTypes.h"
#include "hsWindows.h"
#include <vector>

class ParamBlockDesc2;
class IParamBlock2;
class Class_ID;

class plAutoUIParam;
class plAutoUIClassDesc;

class plAutoUIBase
{
protected:
	HWND fhDlg;
	std::vector<plAutoUIParam*> fParams;

	ParamBlockDesc2 *fDesc;
	IParamBlock2 *fPBlock;
	char *fName;

	HWND fhRollup;

	plAutoUIBase();

public:
	virtual ~plAutoUIBase();

	/////////////////////////////////////////////////////////////////////////////////////
	// Add control (and associated data).  These should only be called at DLL load time.
	// The id parameter must be unique, and while you can add and delete id's, you can
	// never reuse an id.
	//
	// 'scriptName' is a MaxScript visible name.  Here's what the help file says:
	//   "They should begin with an alpha character, have only alphanumerics, and have no
	//    spaces, punctuations, etc.  The convention for multi-word names is to use
	//    studly-caps, eg, paintRadius."
	//  (Note: if this is nil, one will be generated from 'name'.)
	//
	// 'name' is the name that will show up in the user interface (a copy is made, so you
	// can free the pointer after this function returns if it was allocated)
	//
	void AddCheckBox	(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						hsBool def=false);
	void AddFloatSpinner(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						hsScalar def=0.f, hsScalar min=0.f, hsScalar max=1.f);
	void AddIntSpinner	(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						int def=1, int min=0, int max=1);
	void AddEditBox		(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						const char *def=nil, int lines=1);
	void AddPickNodeList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						std::vector<Class_ID>* filter=nil);
	void AddPickNodeButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						std::vector<Class_ID>* filter=nil, bool canConvertToType=false);
	void AddPickComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						std::vector<Class_ID>* filter=nil, bool canConvertToType=false);
	void AddPickComponentList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates,
						std::vector<Class_ID>* filter=nil);
	void AddPickActivatorList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickActivatorButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickDynamicTextButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickGUIDialogButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickExcludeRegionButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickAnimationButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickBehaviorButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickMaterialButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickMaterialAnimationButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickWaterComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickSwimCurrentInterfaceButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickClusterComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);

	void AddPickGUIPopUpMenuButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	void AddPickGUISkinButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);
	
	void AddDropDownList(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates, std::vector<std::string>* options = nil);
	void AddPickGrassComponentButton(Int16 id, const char *scriptName, const char *name, int vid, std::vector<std::string>* vstates);

	void CreateAutoRollup(IParamBlock2 *pb);
	void DestroyAutoRollup();

	const char *GetName() { return fName; }

protected:
	char *IMakeScriptName(const char *fullName);
	void ICreateControls();

	static BOOL CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // plAutoUIBase_h_inc
