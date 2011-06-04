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
#ifndef cyAvatar_h
#define cyAvatar_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyAvatar
//
// PURPOSE: Class wrapper to map animation functions to plasma2 message
//
#include "hsStlUtils.h"
#include "hsTemplates.h"
#include "hsBitVector.h"
#include "../pnKeyedObject/plKey.h"

#include <python.h>
#include "pyGlueHelpers.h"


class plKey;
class pyKey;
class pySceneObject;
class pyColor;
class plMipmap;
class plClothingItem;
class plArmatureMod;
class plMorphSequence;

class cyAvatar
{
protected:
	plKey			fSender;
	hsTArray<plKey>	fRecvr;
	hsBool			fNetForce;

	virtual const plArmatureMod* IFindArmatureMod(plKey avObj);
	virtual plKey IFindArmatureModKey(plKey avObj);
	
// XX	static bool IEnterGenericMode(const char *enterAnim, const char *idleAnim, const char *exitAnim, bool autoExit);
// XX	static bool IExitTopmostGenericMode();

protected:
	cyAvatar() {}
	cyAvatar(plKey sender,plKey recvr=nil);

public:

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptAvatar);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject* New(PyObject* sender, PyObject* recvr = nil);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyAvatar object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyAvatar); // converts a PyObject to a cyAvatar (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);
	static void AddPlasmaMethods(std::vector<PyMethodDef> &methods);
	static void AddPlasmaConstantsClasses(PyObject *m);

	// setters
	void SetSender(plKey &sender);
	void AddRecvr(plKey &recvr);
	virtual void SetNetForce(hsBool state);

	// oneShot Avatar (must already be there)
	virtual void OneShot(pyKey &seekKey, float duration, hsBool usePhysics,
				   const char *animName, hsBool drivable, hsBool reversible);

	// oneShot Avatar 
	virtual void RunBehavior(pyKey &behKey, hsBool netForce, hsBool netProp);
	virtual void RunBehaviorAndReply(pyKey& behKey, pyKey& replyKey, hsBool netForce, hsBool netProp);

	// for the multistage behaviors
	virtual void NextStage(pyKey &behKey, hsScalar transTime, hsBool setTime, hsScalar newTime,
						hsBool setDirection, bool isForward, hsBool netForce);
	virtual void PreviousStage(pyKey &behKey, hsScalar transTime, hsBool setTime, hsScalar newTime,
						hsBool setDirection, bool isForward, hsBool netForce);
	virtual void GoToStage(pyKey &behKey, Int32 stage, hsScalar transTime, hsBool setTime, hsScalar newTime,
						hsBool setDirection, bool isForward, hsBool netForce);

	// static behavior functions:
	static void SetLoopCount(pyKey &behKey, Int32 stage, Int32 loopCount, hsBool netForce);

	virtual void SetSenderKey(pyKey &pKey);

	// seek Avatar (must already be there)
	//virtual void Seek(pyKey &seekKey, float duration, hsBool usePhysics);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetAvatarClothingGroup
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return what clothing group the avatar is in
	//
	virtual Int32 GetAvatarClothingGroup();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetEntireClothingList
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
	//
	virtual std::vector<std::string> GetEntireClothingList(Int32 clothing_type);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetClosetClothingList
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
	//
	virtual std::vector<PyObject*> GetClosetClothingList(Int32 clothing_type);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetAvatarClothingList
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return a list of items being worn by this avatar
	//
	virtual std::vector<PyObject*> GetAvatarClothingList();
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetWardrobeClothingList
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return a list of items that are in the avatars closet
	//
	virtual std::vector<PyObject*> GetWardrobeClothingList();
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : AddWardrobeClothingItem
	//  PARAMETERS : clothing_name - the name of the clothing item to add to your wardrobe
	//             : tint1 - layer one color
	//             : tint2 - layer two color
	//
	//  PURPOSE    : To add a clothing item to the avatar's wardrobe (closet)
	//
	virtual void AddWardrobeClothingItem(const char* clothing_name,pyColor& tint1,pyColor& tint2);
	
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//	Function   : GetUniqueMeshList
	//	PARAMETERS : clothing_type - the type of clothing to get
	//
	//	PURPOSE    : Return a list of unique clothing items (each has a different mesh)
	//			   : that belong to the specific type
	//
	virtual std::vector<PyObject*> GetUniqueMeshList(Int32 clothing_type);

	/////////////////////////////////////////////////////////////////////////////
	//
	//	Function   : GetAllWithSameMesh
	//	PARAMETERS : clothing_name - the name of the mesh to get the textures of
	//
	//	PURPOSE	   : Return a list of clothing items that have the same mesh as
	//			   : the item passed in
	//
	virtual std::vector<PyObject*> GetAllWithSameMesh(const char* clothing_name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetMatchingClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Return the clothing item that matches this one
	//
	virtual PyObject* GetMatchingClothingItem(const char* clothing_name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : WearClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
	//             : returns 0, if clothing item was not found
	//
	virtual hsBool WearClothingItem(const char* clothing_name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RemoveClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Remove (take off) a particular piece of clothing based on name of clothing item
	//             : returns 0, if clothing item was not found
	//
	virtual hsBool RemoveClothingItem(const char* clothing_name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TintClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Tint a clothing item, i.e. change the color of it
	//
	virtual hsBool TintClothingItem(const char* clothing_name, pyColor& tint);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TintClothingItemLayer
	//  PARAMETERS : clothing_name   - name of the clothing item to change the color of
	//             : tint   - what color to change it to
	//             : layer  - which layer to change (1 or 2)
	//
	//  PURPOSE    : Tint a clothing item, i.e. change the color of it
	//
	virtual hsBool TintClothingItemLayer(const char* clothing_name, pyColor& tint, UInt8 layer);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : WearClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
	//             : returns 0, if clothing item was not found
	//
	virtual hsBool WearClothingItemU(const char* clothing_name, hsBool update);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : RemoveClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Remove (take off) a particular piece of clothing based on name of clothing item
	//             : returns 0, if clothing item was not found
	//
	virtual hsBool RemoveClothingItemU(const char* clothing_name, hsBool update);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TintClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Tint a clothing item, i.e. change the color of it
	//
	virtual hsBool TintClothingItemU(const char* clothing_name, pyColor& tint, hsBool update);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TintClothingItemLayer
	//  PARAMETERS : clothing_name   - name of the clothing item to change the color of
	//             : tint   - what color to change it to
	//             : whatpart  - which layer to change (1 or 2)
	//
	//  PURPOSE    : Tint a clothing item, i.e. change the color of it
	//
	virtual hsBool TintClothingItemLayerU(const char* clothing_name, pyColor& tint, UInt8 layer, hsBool update);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetClothingItemParameterString
	//  PARAMETERS : 
	//
	//  PURPOSE    : Get the custom parameter string for a clothing item
	//
	virtual const char* GetClothingItemParameterString(const char* clothing_name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetTintClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
	//
	virtual PyObject* GetTintClothingItem(const char* clothing_name);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetTintClothingItem
	//  PARAMETERS : 
	//
	//  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
	//
	virtual PyObject* GetTintClothingItemL(const char* clothing_name, UInt8 layer);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TintSkin
	//  PARAMETERS : 
	//
	//  PURPOSE    : Tint the skin of the player's avatar
	//
	virtual void TintSkin(pyColor& tint);
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : TintSkinU
	//  PARAMETERS : 
	//
	//  PURPOSE    : Tint the skin of the player's avatar with optional update flag
	//
	virtual void TintSkinU(pyColor& tint, hsBool update);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetTintSkin
	//  PARAMETERS : 
	//
	//  PURPOSE    : Get the tint of the skin of the player's avatar
	//
	virtual PyObject* GetTintSkin();

	virtual plMorphSequence* LocalMorphSequence();
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//	Function   : SetMorph
	//	PARAMETERS : clothing_name - the name of the clothing to morph
	//			   : layer - the layer to affect
	//			   : value - what the new value should be (clipped between -1 and 1)
	//
	//	PURPOSE	   : Set the morph value of a specific layer of clothing
	//
	virtual void SetMorph(const char* clothing_name, UInt8 layer, float value);

	/////////////////////////////////////////////////////////////////////////////
	//
	//	Function   : GetMorph
	//	PARAMETERS : clothing_name - the name of the clothing to get the vaule from
	//			   : layer - the layer to get the value from
	//
	//	PURPOSE    : Returns the current morph value of the specific layer of clothing
	//
	virtual float GetMorph(const char* clothing_name, UInt8 layer);
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//	Function   : SetSkinBlend
	//	PARAMETERS : layer - the layer to affect
	//			   : value - what the new value should be (clipped between 0 and 1)
	//
	//	PURPOSE	   : Set the skin blend for the specified layer
	//
	virtual void SetSkinBlend(UInt8 layer, float value);

	/////////////////////////////////////////////////////////////////////////////
	//
	//	Function   : GetSkinBlend
	//	PARAMETERS : layer - the layer to get the blend for
	//
	//	PURPOSE	   : Returns the current layer's skin blend
	//
	virtual float GetSkinBlend(UInt8 layer);
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : SaveClothing
	//  PARAMETERS :
	//
	//  PURPOSE	   : Saves the current clothing to the vault (including morphs)
	//
	virtual void SaveClothing();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : EnterSubWorld
	//  PARAMETERS : object  - a sceneobject that is in the subworld
	//
	//  PURPOSE    : Place the Avatar into the subworld of the sceneobject specified
	//
	virtual void EnterSubWorld(pySceneObject& object);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ExitSubWorld
	//  PARAMETERS : (none)
	//
	//  PURPOSE    : Exit the avatar from the subworld, back into the ... <whatever> world
	//
	virtual void ExitSubWorld();

	virtual void PlaySimpleAnimation(const char* animName);

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
	static void ChangeAvatar(const char* genderName);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ChangePlayerName
	//  PARAMETERS : name  - is a string of the new name for the player
	//
	//  PURPOSE    : Change the local player's avatar name
	//
	static void ChangePlayerName(const char* playerName);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Emote
	//  PARAMETERS : emoteName - name of the emote to play on the avatar
	//
	//  PURPOSE    : plays an emote on the local avatar (net propagated)
	//
	static bool Emote(const char* emoteName);

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : Sit
	//  PARAMETERS : none
	//
	//  PURPOSE    : Makes the avatar sit down on the ground where they are.
	//				 The avatar will automatically stand when the user tries to move.
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
	//				 May cause problems if EnterKiMode() was not called earlier.
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
	//				 May cause problems if EnterKiMode() was not called earlier.
	//
	static bool ExitAFKMode();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : EnterPBMode
	//  PARAMETERS : none
	//
	//  PURPOSE    : Enter the personal book mode...stay until further notice.
	//
	static bool cyAvatar::EnterPBMode();

	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : ExitPBMode
	//  PARAMETERS : none
	//
	//  PURPOSE    : Leave the personal book mode. Currently leaves any mode; will become
	//			   : more specific in future version
	//
	static bool cyAvatar::ExitPBMode();
	
	/////////////////////////////////////////////////////////////////////////////
	//
	//  Function   : GetCurrentMode()
	//  PARAMETERS : none
	//
	//  PURPOSE    : Leave the personal book mode. Currently leaves any mode; will become
	//			   : more specific in future version
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
	
	static void	SetMouseTurnSensitivity(hsScalar val);
	static hsScalar	GetMouseTurnSensitivity();

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

	static hsBool IsCurrentBrainHuman();

};

#endif  // cyAvatar_h
