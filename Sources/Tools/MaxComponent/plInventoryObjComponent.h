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
#ifndef plInventoryObjComponent_inc
#define plInventoryObjComponent_inc

#include <map>
#include <vector>

#include "plClickableComponent.h"

class plErrorMsg;
class plKey;
class plMaxNode;

#define INVENTORYOBJCOMP_CID Class_ID(0x425e1687, 0x4a126b91)

//!  Inventory Object Component Class

/*!
    This Class is the rudimentary 'Inventoriable' component.  It is derived from the Clickable
    component in order to access the mouse controllers to allow mouse state changes when the cursor
    is over an object that can be taken.

        member functions:
            
            bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);

            bool PreConvert(plMaxNode *node, plErrorMsg* pErrMsg);
            bool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

            virtual void AddReceiverKey(plKey pKey);
            virtual plKey GetLogicKey(plMaxNode* node);
            const LogicKeys& GetLogicKeys();
        \sa SetupProperties(), PreConvert(), AddReceiverKey(), GetLogicKey() and GetLogicKeys()
*/






class plInventoryObjComponent : public plClickableComponent
{
public:
    typedef std::map<plMaxNode*, plKey> LogicKeys;
protected:
    
    std::vector<plKey> fReceivers;
    LogicKeys fLogicModKeys;

public:

    //! Constructor function for class
    /*!
        Herein the ClassDesc2 object that is used extensively by the ParamBlock2
        has gained accessibiltiy.  Auto-Creation of the UI is done here as well.
    
    */
    plInventoryObjComponent();

    // Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.

    //! plInventoryObjComponent PreConvert, takes in two variables and return a bool.
    /*! 
        Calls the function MaybeMakeLocal() and Sets Drawable to false.

        Takes in two variables, being:
        \param node a plMaxNode ptr.
        \param pErrMsg a pErrMsg ptr.

        \return A bool expressing the success of the operation.
        \sa DeleteThis(), plPhysicalCoreComponent(), Convert(), GetParamVals(), MaybeMakeLocal() and FixUpPhysical()
    */

    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *node, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    void AddReceiverKey(plKey key, plMaxNode* node=nullptr) override;
    plKey GetLogicKey(plMaxNode* node) override;
    const LogicKeys& GetLogicKeys();

};

#endif // plInventoryObjComponent_inc
