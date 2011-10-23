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
#include "max.h"
#include "python.h"
#include <vector>

class plAutoUIBlock;

class plPythonMgr
{
protected:
    plPythonMgr();

    bool IQueryPythonFile(char *fileName);

    void IAddBool(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddInt(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddFloat(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddString(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddSceneObj(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddSceneObjList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddActivator(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddActivatorList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddResponder(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddResponderList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddDynamicText(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddGUIDialog(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddExcludeRegion(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddAnimation(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddBehavior(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddMaterial(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddGUIPopUpMenu(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddGUISkin(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddWaterComponent(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddSwimCurrentInterface(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddDropDownList(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddClusterComponent(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddMaterialAnimation(plAutoUIBlock *autoUI, PyObject *tuple, char *paramName, int id, int vid, std::vector<std::string>* vstates);
    void IAddGrassComponent(plAutoUIBlock *autoUI, PyObject *objTuple, std::string paramName, int id, int vid, std::vector<std::string>* vstates);

public:
    static plPythonMgr& Instance();

    void LoadPythonFiles();
};