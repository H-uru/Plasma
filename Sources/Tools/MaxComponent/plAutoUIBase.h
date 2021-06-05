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
#ifndef plAutoUIBase_h_inc
#define plAutoUIBase_h_inc

#include <unordered_set>
#include <vector>


class ParamBlockDesc2;
class IParamBlock2;
class Class_ID;

class plAutoUIParam;
class plAutoUIClassDesc;

namespace ST { class string; }

class plAutoUIBase
{
protected:
    HWND fhDlg;
    std::vector<plAutoUIParam*> fParams;

    ParamBlockDesc2 *fDesc;
    IParamBlock2 *fPBlock;
    ST::string fName;

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
    //    spaces, punctuations, etc.  The convention for multi-uint16_t names is to use
    //    studly-caps, eg, paintRadius."
    //  (Note: if this is nil, one will be generated from 'name'.)
    //
    // 'name' is the name that will show up in the user interface (a copy is made, so you
    // can free the pointer after this function returns if it was allocated)
    //
    void AddCheckBox    (int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        bool def=false);
    void AddFloatSpinner(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        float def=0.f, float min=0.f, float max=1.f);
    void AddIntSpinner  (int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        int def=1, int min=0, int max=1);
    void AddEditBox     (int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        const ST::string& def={}, int lines=1);
    void AddPickNodeList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        std::vector<Class_ID>* filter=nullptr);
    void AddPickNodeButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        std::vector<Class_ID>* filter=nullptr, bool canConvertToType=false);
    void AddPickComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        std::vector<Class_ID>* filter=nullptr, bool canConvertToType=false);
    void AddPickComponentList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates,
                        std::vector<Class_ID>* filter=nullptr);
    void AddPickActivatorList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickActivatorButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickDynamicTextButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickGUIDialogButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickExcludeRegionButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickAnimationButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickBehaviorButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickMaterialButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickMaterialAnimationButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickWaterComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickSwimCurrentInterfaceButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickClusterComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);

    void AddPickGUIPopUpMenuButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    void AddPickGUISkinButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);
    
    void AddDropDownList(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates, std::vector<ST::string> options);
    void AddPickGrassComponentButton(int16_t id, const ST::string& scriptName, const ST::string& name, int vid, std::unordered_set<ST::string> vstates);

    void CreateAutoRollup(IParamBlock2 *pb);
    void DestroyAutoRollup();

    ST::string GetName() const { return fName; }

protected:
    ST::string IMakeScriptName(const ST::string& fullName);
    void ICreateControls();

    static INT_PTR CALLBACK ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // plAutoUIBase_h_inc
