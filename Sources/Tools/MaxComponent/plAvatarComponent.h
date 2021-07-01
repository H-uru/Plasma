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
#ifndef __PLAVATARCOMPONENT_H__
#define __PLAVATARCOMPONENT_H__

#include "plAvatar/plCritterCommands.h"

#define AVATAR_CLASS_ID     Class_ID(0x49247847, 0xd3908fe)
#define LOD_AVATAR_CLASS_ID Class_ID(0x50100640, 0x72f94120)
#define CRITTER_CLASS_ID    Class_ID(0x3bd003b6, 0x66a85756)
#define PUPPET_CLASS_ID     Class_ID(0x26545a35, 0x2a5000b)

class plHKPhysical;
class plErrorMsg;
class plArmatureMod;
class plMaxNode;
class plSceneObject;
class plShadowCaster;
class hsGMaterial;

class AvatarCompDlgProc;
extern AvatarCompDlgProc gAvatarCompDlgProc;

/** \class plArmatureComponent
    Base class for all the components that creature armatures or, in the case of the
    compound controller, simply multi-channel animation controllers (plAGMasterMod.)
*/
class plArmatureComponent : public plComponent
{
private:
    void ISetArmatureSORecurse(plMaxNode *node, plSceneObject *so);
    plHKPhysical*  IConvertArmaturePhysicsProxy(plMaxNode *node, plMaxNode *proxyNode);

protected:
    plArmatureMod *fArmMod;

    plArmatureComponent() : fArmMod() { }
    plArmatureMod* IGenerateMyArmMod(plHKPhysical* myHKPhys, plMaxNode* node);
    bool IVerifyUsedNode(INode* thisNode, plErrorMsg *pErrMsg, bool isHull);

    virtual void IAttachModifiers(plMaxNode *node, plErrorMsg *pErrMsg) = 0;
    virtual void ISetupClothes(plMaxNode *node, plArmatureMod *mod, plErrorMsg *pErrMsg) {}
    virtual void ISetupAvatarRenderPropsRecurse(plMaxNode *node);
    virtual void IAttachShadowCastModifiersRecur(plMaxNode* node, plShadowCaster* caster); // Apply supplied shadowcaster modifier

public:
    enum    {
                kBounceEventGroupBoolTab_DEAD = 99,
                kReportEventGroupBoolTab_DEAD,
                kBounceEventGroupChoice_DEAD,
                kReportEventGroupChoice_DEAD,

                kBounceGroups,
                kReportGroups,
            };

    // Constants for brain selection UI
    enum
    {
        kBrainHuman,
        kBrainCritter,
        kMaxBrainType,
    };
    //static const char *BrainStrings[];

    bool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg) override;
    bool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg) override;
    bool Convert(plMaxNode* node, plErrorMsg* pErrMsg) override;

    void DeleteThis() override { delete this; }
};

class plAvatarComponent : public plArmatureComponent
{

private:
    void IAttachModifiers(plMaxNode *node, plErrorMsg *pErrMsg) override;



public:
        plAvatarComponent();
 
        // Do not change any of the entries in this list. Only add at the end.
        // All existing ones must be kept for backward compatibility.
        enum
        {
            kWeightRadio,       //Insert in v1, removed version 3 (14.07.01)
            kAgilityRadio,      //Insert in v1,    "      "     "     "
            kModelRoot,         //Insert in v1, removed version 2 (05/09/01)
            kPhysicsProxyFeet_DEAD, //Insert in v1,
            kFriction,          //Insert in v3,
            kMaxVelocity,       //Insert in v3,
            kAcceleration,      //Insert in v3,
            kTurnForce,         //Insert in v3,
            kWalkAnim,          //Insert in v3,
            kRunAnim,           //Insert in v3,
            kUseAnimationsBool, //Insert in v4,
            kRootNode,          //Insert in v5,
            kMeshNode,          //Insert in v5,
            kClothingGroup,     //Insert in v6,
            kBrainType,         //Insert in v7,
            kSkeleton,          //Insert in v8,
            kPhysicsProxyTorso_DEAD,
            kPhysicsProxyHead_DEAD,
            kPhysicsHeight,
            kPhysicsWidth,
            kBodyAgeName,
            kBodyFootstepSoundPage,
            kAnimationPrefix,
        };

        void ISetupClothes(plMaxNode *node, plArmatureMod *mod, plErrorMsg *pErrMsg) override;
        bool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg) override;
        bool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg) override { return (plArmatureComponent::PreConvert(node, pErrMsg)); }
        bool Convert(plMaxNode* node, plErrorMsg* pErrMsg) override { return (plArmatureComponent::Convert(node, pErrMsg)); }
        void DeleteThis() override { delete this; }


};


class plLODAvatarComponent : public plArmatureComponent
{

private:
    void IAttachModifiers(plMaxNode *node, plErrorMsg *pErrMsg);

protected:
    void ISetupClothes(plMaxNode *node, plArmatureMod *mod, plErrorMsg *pErrMsg) override;
    // Create a single shadowcaster modifier, and apply it to the 
    // sceneobject hierarchies for each LOD.
    virtual void IAttachShadowCastToLODs(plMaxNode* rootNode); 
    void IAttachShadowCastModifiersRecur(plMaxNode* node, plShadowCaster* caster) override;

    hsGMaterial *fMaterial;
    
public:
        VStringArray fLODLevels;  //Initialized in the CTR.


        // Do not change any of the entries in this list. Only add at the end.
        // All existing ones must be kept for backward compatibility.
        enum
        {
            kPhysicsProxyFeet_DEAD, //Insert in v1,
            kFriction,          //Insert in v1,
            kMaxVelocity,       //Insert in v1,
            kAcceleration,      //Insert in v1,
            kTurnForce,         //Insert in v1,
            kWalkAnim,          //Insert in v1,
            kRunAnim,           //Insert in v1,
            kUseAnimationsBool, //Insert in v1,
            kLODLevel,          //Insert in v1,
            kLODState,          //Insert in v1,
            kRootNodeTab,       //Insert in v1,
            kMeshNodeTab,       //Insert in v1,
            kRootNodeAddBtn,    //Insert in v1,
            kMeshNodeAddBtn,    //Insert in v1,
            kClothingGroup,     //Insert in v2, 
            kBrainType,         //Insert in v3,
            kGroupIdx,          //Insert in v4,
            kBoneList,          //Insert in v4,
            kGroupTotals,       //Insert in v4,
            kLastPick,          //Insert in v4,
            kSkeleton,          //Insert in v5,
            kMaterial,          //Insert in v6,
            kPhysicsProxyTorso_DEAD,
            kPhysicsProxyHead_DEAD,
            kPhysicsHeight,
            kPhysicsWidth,
            kBodyFootstepSoundPage,
            kAnimationPrefix,
        };

        enum
        {
                    kMaxNumLODLevels = 3,
        };

        plLODAvatarComponent();
        bool SetupProperties(plMaxNode* node, plErrorMsg* pErrMsg) override;
        bool PreConvert(plMaxNode* node, plErrorMsg* pErrMsg) override;
        bool Convert(plMaxNode* node, plErrorMsg* pErrMsg) override;
        void DeleteThis() override { delete this; }

        void RemoveBone(int index);
        void AddSelectedBone();
        int GetCurGroupIdx();
        int GetStartIndex(int group);
        int GetEndIndex(int group);
};

class plAnimatronicComponent : public plLODAvatarComponent
{

};




#endif