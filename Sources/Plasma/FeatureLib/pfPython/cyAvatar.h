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
#ifndef cyAvatar_h
#define cyAvatar_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyAvatar
//
// PURPOSE: Class wrapper to map animation functions to plasma2 message
//

#include <vector>

#include "pnKeyedObject/plKey.h"

#include "pyGlueHelpers.h"

class plFileName;
class pySceneObject;
class pyColor;
class plMipmap;
class plClothingItem;
class plArmatureMod;
class plMorphSequence;
namespace ST { class string; }

class cyAvatar
{
protected:
    plKey           fSender;
    std::vector<plKey> fRecvr;
    bool          fNetForce;

    const plArmatureMod* IFindArmatureMod(const plKey& avObj);
    plKey IFindArmatureModKey(const plKey& avObj);

protected:
    cyAvatar() : fNetForce() { }
    cyAvatar(plKey sender, plKey recvr=nullptr);

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptAvatar);
    PYTHON_CLASS_NEW_DEFINITION;
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyAvatar object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyAvatar); // converts a PyObject to a cyAvatar (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);
    static void AddPlasmaMethods(PyObject* m);
    static void AddPlasmaConstantsClasses(PyObject *m);

    // setters
    void SetSender(plKey sender);
    void AddRecvr(plKey recvr);
    void SetNetForce(bool state) { fNetForce = state; }

    // oneShot Avatar (must already be there)
    void OneShot(pyKey &seekKey, float duration, bool usePhysics,
                   const ST::string &animName, bool drivable, bool reversible);

    // oneShot Avatar 
    void RunBehavior(pyKey &behKey, bool netForce, bool netProp);
    void RunBehaviorAndReply(pyKey& behKey, pyKey& replyKey, bool netForce, bool netProp);
    bool RunCoopAnim(pyKey& targetKey, ST::string activeAvatarAnim, ST::string targetAvatarAnim, float range, float dist, bool move);

    // for the multistage behaviors
    void NextStage(pyKey &behKey, float transTime, bool setTime, float newTime,
                        bool setDirection, bool isForward, bool netForce);
    void PreviousStage(pyKey &behKey, float transTime, bool setTime, float newTime,
                        bool setDirection, bool isForward, bool netForce);
    void GoToStage(pyKey &behKey, int32_t stage, float transTime, bool setTime, float newTime,
                        bool setDirection, bool isForward, bool netForce);

    // static behavior functions:
    static void SetLoopCount(pyKey &behKey, int32_t stage, int32_t loopCount, bool netForce);

    void SetSenderKey(const pyKey &pKey);

    // seek Avatar (must already be there)
    //void Seek(pyKey &seekKey, float duration, bool usePhysics);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetAvatarClothingGroup
    //  PARAMETERS : 
    //
    //  PURPOSE    : Return what clothing group the avatar is in
    //
    int32_t GetAvatarClothingGroup();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetEntireClothingList
    //  PARAMETERS : 
    //
    //  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
    //
    std::vector<ST::string> GetEntireClothingList(int32_t clothing_type);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetClosetClothingList
    //  PARAMETERS : 
    //
    //  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
    //
    std::vector<PyObject*> GetClosetClothingList(int32_t clothing_type);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetAvatarClothingList
    //  PARAMETERS : 
    //
    //  PURPOSE    : Return a list of items being worn by this avatar
    //
    std::vector<PyObject*> GetAvatarClothingList();
    
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetWardrobeClothingList
    //  PARAMETERS : 
    //
    //  PURPOSE    : Return a list of items that are in the avatars closet
    //
    std::vector<PyObject*> GetWardrobeClothingList();
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : AddWardrobeClothingItem
    //  PARAMETERS : clothing_name - the name of the clothing item to add to your wardrobe
    //             : tint1 - layer one color
    //             : tint2 - layer two color
    //
    //  PURPOSE    : To add a clothing item to the avatar's wardrobe (closet)
    //
    void AddWardrobeClothingItem(const ST::string& clothing_name,pyColor& tint1,pyColor& tint2);
    
    
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetUniqueMeshList
    //  PARAMETERS : clothing_type - the type of clothing to get
    //
    //  PURPOSE    : Return a list of unique clothing items (each has a different mesh)
    //             : that belong to the specific type
    //
    std::vector<PyObject*> GetUniqueMeshList(int32_t clothing_type);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetAllWithSameMesh
    //  PARAMETERS : clothing_name - the name of the mesh to get the textures of
    //
    //  PURPOSE    : Return a list of clothing items that have the same mesh as
    //             : the item passed in
    //
    std::vector<PyObject*> GetAllWithSameMesh(const ST::string& clothing_name);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetMatchingClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Return the clothing item that matches this one
    //
    PyObject* GetMatchingClothingItem(const ST::string& clothing_name);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : WearClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
    //             : returns 0, if clothing item was not found
    //
    bool WearClothingItem(const ST::string& clothing_name);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : RemoveClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Remove (take off) a particular piece of clothing based on name of clothing item
    //             : returns 0, if clothing item was not found
    //
    bool RemoveClothingItem(const ST::string& clothing_name);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TintClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Tint a clothing item, i.e. change the color of it
    //
    bool TintClothingItem(const ST::string& clothing_name, pyColor& tint);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TintClothingItemLayer
    //  PARAMETERS : clothing_name   - name of the clothing item to change the color of
    //             : tint   - what color to change it to
    //             : layer  - which layer to change (1 or 2)
    //
    //  PURPOSE    : Tint a clothing item, i.e. change the color of it
    //
    bool TintClothingItemLayer(const ST::string& clothing_name, pyColor& tint, uint8_t layer);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : WearClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
    //             : returns 0, if clothing item was not found
    //
    bool WearClothingItemU(const ST::string& clothing_name, bool update);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : RemoveClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Remove (take off) a particular piece of clothing based on name of clothing item
    //             : returns 0, if clothing item was not found
    //
    bool RemoveClothingItemU(const ST::string& clothing_name, bool update);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TintClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Tint a clothing item, i.e. change the color of it
    //
    bool TintClothingItemU(const ST::string& clothing_name, pyColor& tint, bool update);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TintClothingItemLayer
    //  PARAMETERS : clothing_name   - name of the clothing item to change the color of
    //             : tint   - what color to change it to
    //             : whatpart  - which layer to change (1 or 2)
    //
    //  PURPOSE    : Tint a clothing item, i.e. change the color of it
    //
    bool TintClothingItemLayerU(const ST::string& clothing_name, pyColor& tint, uint8_t layer, bool update);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetClothingItemParameterString
    //  PARAMETERS : 
    //
    //  PURPOSE    : Get the custom parameter string for a clothing item
    //
    ST::string GetClothingItemParameterString(const ST::string& clothing_name);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetTintClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
    //
    PyObject* GetTintClothingItem(const ST::string& clothing_name);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetTintClothingItem
    //  PARAMETERS : 
    //
    //  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
    //
    PyObject* GetTintClothingItemL(const ST::string& clothing_name, uint8_t layer);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TintSkin
    //  PARAMETERS : 
    //
    //  PURPOSE    : Tint the skin of the player's avatar
    //
    void TintSkin(pyColor& tint);
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : TintSkinU
    //  PARAMETERS : 
    //
    //  PURPOSE    : Tint the skin of the player's avatar with optional update flag
    //
    void TintSkinU(pyColor& tint, bool update);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetTintSkin
    //  PARAMETERS : 
    //
    //  PURPOSE    : Get the tint of the skin of the player's avatar
    //
    PyObject* GetTintSkin();

    plMorphSequence* LocalMorphSequence();
    
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : SetMorph
    //  PARAMETERS : clothing_name - the name of the clothing to morph
    //             : layer - the layer to affect
    //             : value - what the new value should be (clipped between -1 and 1)
    //
    //  PURPOSE    : Set the morph value of a specific layer of clothing
    //
    void SetMorph(const ST::string& clothing_name, uint8_t layer, float value);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetMorph
    //  PARAMETERS : clothing_name - the name of the clothing to get the vaule from
    //             : layer - the layer to get the value from
    //
    //  PURPOSE    : Returns the current morph value of the specific layer of clothing
    //
    float GetMorph(const ST::string& clothing_name, uint8_t layer);
    
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : SetSkinBlend
    //  PARAMETERS : layer - the layer to affect
    //             : value - what the new value should be (clipped between 0 and 1)
    //
    //  PURPOSE    : Set the skin blend for the specified layer
    //
    void SetSkinBlend(uint8_t layer, float value);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetSkinBlend
    //  PARAMETERS : layer - the layer to get the blend for
    //
    //  PURPOSE    : Returns the current layer's skin blend
    //
    float GetSkinBlend(uint8_t layer);
    
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : SaveClothing
    //  PARAMETERS :
    //
    //  PURPOSE    : Saves the current clothing to the vault (including morphs)
    //
    void SaveClothing();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : EnterSubWorld
    //  PARAMETERS : object  - a sceneobject that is in the subworld
    //
    //  PURPOSE    : Place the Avatar into the subworld of the sceneobject specified
    //
    void EnterSubWorld(pySceneObject& object);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : ExitSubWorld
    //  PARAMETERS : (none)
    //
    //  PURPOSE    : Exit the avatar from the subworld, back into the ... <whatever> world
    //
    void ExitSubWorld();

    void PlaySimpleAnimation(const ST::string& animName);

    bool SaveClothingToFile(plFileName filename);
    bool LoadClothingFromFile(plFileName filename);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : ChangeAvatar
    //  PARAMETERS : gender name  - is a string of the name of the gender to go to
    //
    //  PURPOSE    : Change the local avatar's gender.
    //
    //  Valid genders:
    //    Male
    //    Female
    //
    static void ChangeAvatar(const ST::string& genderName);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : ChangePlayerName
    //  PARAMETERS : name  - is a string of the new name for the player
    //
    //  PURPOSE    : Change the local player's avatar name
    //
    static void ChangePlayerName(const ST::string& playerName);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : Emote
    //  PARAMETERS : emoteName - name of the emote to play on the avatar
    //
    //  PURPOSE    : plays an emote on the local avatar (net propagated)
    //
    static bool Emote(const ST::string& emoteName);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : Sit
    //  PARAMETERS : none
    //
    //  PURPOSE    : Makes the avatar sit down on the ground where they are.
    //               The avatar will automatically stand when the user tries to move.
    //
    static bool Sit();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : EnterKiMode
    //  PARAMETERS : none
    //
    //  PURPOSE    : Makes the avatar appear to be using the ki.
    //
    static bool EnterKiMode();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : ExitKiMode
    //  PARAMETERS : none
    //
    //  PURPOSE    : Makes the avatar stop appearing to use the ki.
    //               May cause problems if EnterKiMode() was not called earlier.
    //
    static bool ExitKiMode();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : EnterAFKMode
    //  PARAMETERS : none
    //
    //  PURPOSE    : Tell the avatar to enter the AFK mode (sitting, head down)
    //
    static bool EnterAFKMode();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : ExitKiMode
    //  PARAMETERS : none
    //
    //  PURPOSE    : Tell the avatar to exit the AFK mode
    //               May cause problems if EnterKiMode() was not called earlier.
    //
    static bool ExitAFKMode();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : EnterPBMode
    //  PARAMETERS : none
    //
    //  PURPOSE    : Enter the personal book mode...stay until further notice.
    //
    static bool EnterPBMode();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : ExitPBMode
    //  PARAMETERS : none
    //
    //  PURPOSE    : Leave the personal book mode. Currently leaves any mode; will become
    //             : more specific in future version
    //
    static bool ExitPBMode();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : EnterAnimMode
    //  PARAMETERS : animName - string
    //
    //  PURPOSE    : Makes the avatar enter a custom anim loop.
    //
    static bool EnterAnimMode(const ST::string& animName);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : GetCurrentMode()
    //  PARAMETERS : none
    //
    //  PURPOSE    : Leave the personal book mode. Currently leaves any mode; will become
    //             : more specific in future version
    //
    int GetCurrentMode();

    
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : (En/Dis)ableMovementControls()
    //  PARAMETERS : none
    //
    //  PURPOSE    : Suspend input on the local avatar
    //            
    static void EnableMovementControls();
    static void DisableMovementControls();
    static void EnableMouseMovement();
    static void DisableMouseMovement();
    static void EnableAvatarJump();
    static void DisableAvatarJump();
    static void EnableForwardMovement();
    static void DisableForwardMovement();

    static bool LocalAvatarRunKeyDown();
    static bool LocalAvatarIsMoving();
    
    static void SetMouseTurnSensitivity(float val);
    static float GetMouseTurnSensitivity();

    static void SpawnNext();
    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : RegisterForBehaviorNotify()
    //  PARAMETERS : none
    //
    //  PURPOSE    : To register for notifies from the avatar for any kind of behavior notify
    //
    void RegisterForBehaviorNotify(pyKey &selfKey);

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : UnRegisterForBehaviorNotify()
    //  PARAMETERS : none
    //
    //  PURPOSE    : To remove the registeration for notifies from the avatar
    //
    void UnRegisterForBehaviorNotify(pyKey &selfKey);

    static bool IsCurrentBrainHuman();

    /////////////////////////////////////////////////////////////////////////////
    //
    //  Function   : SetDontPanicLink
    //  PARAMETERS : value
    //
    //  PURPOSE    : Disables panic linking to Personal Age (warps the avatar back to the start instead)
    //
    void SetDontPanicLink(bool value);
};

#endif  // cyAvatar_h
