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
#ifndef PLAVATARCLOTHING_INC
#define PLAVATARCLOTHING_INC

#include "pnUtils/pnUtils.h"
#include "pnNetCommon/plSynchedObject.h"
#include "hsColorRGBA.h"
#include "hsBitVector.h"
#include "plClothingLayout.h"

class hsGMaterial;
class plMipmap;
class plLayer;
class hsStream;
class plSceneObject;
class plGeometrySpan;
class plClothingLayout;
class plClothingElement;
class plArmatureMod;
class plSharedMesh;
class plStateDataRecord;
class plDXPipeline;

class plClothingItemOptions
{
public:
    hsColorRGBA fTint1;
    hsColorRGBA fTint2;

    plClothingItemOptions() { fTint1.Set(1.f, 1.f, 1.f, 1.f); fTint2.Set(1.f, 1.f, 1.f, 1.f); }

    hsBool IsMatch(plClothingItemOptions *other) { return fTint1 == other->fTint1 && fTint2 == other->fTint2; }
};

class plClothingItem : public hsKeyedObject
{
public:
    enum
    {
        kLODHigh,
        kLODMedium,
        kLODLow,
        kMaxNumLODLevels,
    };

    // If you change the format of a clothing item, chances are you need
    // to change plClothingMgr::IsLRMatch() as well
    char *fName;
    plSharedMesh *fMeshes[kMaxNumLODLevels];
    hsTArray<plMipmap **> fTextures;
    hsTArray<char *> fElementNames;
    hsTArray<plClothingElement *> fElements;
    uint8_t fGroup;   // Each avatar can wear one of the available groups
    uint8_t fType;    // Each group has multiple types of clothes (shirt/pants/etc)
    uint8_t fTileset;
    uint8_t fSortOrder;
    char *fDescription;
    char *fCustomText;
    plMipmap *fThumbnail;
    plClothingItem *fAccessory; // Forced accessory to always wear with this item.
    uint8_t fDefaultTint1[3];
    uint8_t fDefaultTint2[3];
    
    char *fAccessoryName; // Export only



    plClothingItem();
    ~plClothingItem();
    
    CLASSNAME_REGISTER( plClothingItem );
    GETINTERFACE_ANY( plClothingItem, hsKeyedObject );

    void SetName(char *name) { delete fName; fName = hsStrcpy(name); }
    const char* GetName() { return fName; }
    hsBool CanWearWith(plClothingItem *item);
    hsBool WearBefore(plClothingItem *item); // Should we come before the arg item? (texture gen order)
    hsBool HasBaseAlpha();
    hsBool HasSameMeshes(plClothingItem *other);

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr); 

    virtual hsBool MsgReceive(plMessage* msg);
};

class plClosetItem
{
public:
    plClosetItem() : fItem(nil) {}

    plClothingItem *fItem;
    plClothingItemOptions fOptions;

    hsBool IsMatch(plClosetItem *other);
};

class plClothingBase : public hsKeyedObject
{
public:
    char *fName;
    plMipmap *fBaseTexture;
    char *fLayoutName;

    plClothingBase();
    ~plClothingBase();

    CLASSNAME_REGISTER( plClothingBase );
    GETINTERFACE_ANY( plClothingBase, hsKeyedObject );
    
    void SetLayoutName(char *name) { delete fLayoutName; fLayoutName = hsStrcpy(name); }

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);

    virtual hsBool MsgReceive(plMessage* msg);
};

class plClothingOutfit : public plSynchedObject
{
    friend class plDXPipeline;

public:
    plArmatureMod *fAvatar;
    plLayer *fTargetLayer;
    hsGMaterial *fMaterial; // Needed to tell swapped geometry what material to use.
    hsTArray<plClothingItem*> fItems;
    hsTArray<plClothingItemOptions*> fOptions;
    plClothingBase *fBase;
    uint8_t fGroup;
    bool fSynchClients;     // set true if the next synch should be bcast
    hsColorRGBA fSkinTint;
    float fSkinBlends[plClothingElement::kLayerSkinLast - plClothingElement::kLayerSkinFirst]; // Controls the opacity between skin textures.

    plClothingOutfit();
    ~plClothingOutfit();

    CLASSNAME_REGISTER( plClothingOutfit );
    GETINTERFACE_ANY( plClothingOutfit, plSynchedObject );

    void SaveCustomizations(hsBool retry = true);
    void AddItem(plClothingItem *item, hsBool update = true, hsBool broadcast = true, hsBool netForce=false);
    void RemoveItem(plClothingItem *item, hsBool update = true, hsBool netForce=false);
    void TintItem(plClothingItem *item, float red, float green, float blue, hsBool update = true, hsBool broadcast = true, 
                  hsBool netForce = false, hsBool retry = true, uint8_t fLayer = plClothingElement::kLayerTint1);
    void TintSkin(float red, float green, float blue,
                  hsBool update = true, hsBool broadcast = true);
    void MorphItem(plClothingItem *item, uint8_t layer, uint8_t delta, float weight, hsBool retry = true);
    void SetAge(float age, hsBool update = true, hsBool broadcast = true);
    void SetSkinBlend(float blend, uint8_t layer, hsBool update = true, hsBool broadcast = true);
    float GetSkinBlend(uint8_t layer);     
    hsColorRGBA GetItemTint(plClothingItem *item, uint8_t layer = 2) const;
    float GetAge() const { return fSkinBlends[0]; }
    hsTArray<plClothingItem*> &GetItemList() { return fItems; }
    hsTArray<plClothingItemOptions*> &GetOptionList() { return fOptions; }

    virtual void Read(hsStream* s, hsResMgr* mgr);
    virtual void Write(hsStream* s, hsResMgr* mgr);
    hsBool DirtySynchState(const char* SDLStateName, uint32_t synchFlags);

    void StripAccessories();
    void WearDefaultClothing();
    void WearDefaultClothingType(uint32_t clothingType);
    void WearMaintainerOutfit();
    void WearRandomOutfit();
    void RemoveMaintainerOutfit();

    hsBool ReadItems(hsStream* s, hsResMgr* mgr, hsBool broadcast = true);
    void WriteItems(hsStream* s, hsResMgr* mgr);
    
    void ForceUpdate(bool retry);       // send updateTexture msg

    virtual hsBool MsgReceive(plMessage* msg);

    void IInstanceSharedMeshes(plClothingItem *item);
    void IRemoveSharedMeshes(plClothingItem *item);

    void ReadFromVault();
    void WriteToVault();
    void WriteToVault(const ARRAY(plStateDataRecord*) & SDRs);

    void SetupMorphSDL();

    // XXX Don't use this. Temp function for a temp HACK console command.
    void DirtyTileset(int tileset);

protected:
    hsBitVector fDirtyItems;
    hsBool fVaultSaveEnabled;
    bool fMorphsInitDone;

    void IAddItem(plClothingItem *item);
    void IRemoveItem(plClothingItem *item);
    hsBool ITintItem(plClothingItem *item, hsColorRGBA color, uint8_t layer);
    hsBool IMorphItem(plClothingItem *item, uint8_t layer, uint8_t delta, float weight);
    void IHandleMorphSDR(plStateDataRecord *sdr);
        
    void IUpdate();

};

class plClothingMgr : public hsKeyedObject
{
protected:
    static plClothingMgr *fInstance;

    hsTArray<plClothingElement*> fElements;
    hsTArray<plClothingItem*> fItems;
    hsTArray<plClothingLayout*> fLayouts;
    
    void IInit();
    void IAddItem(plClothingItem *item);

public:
    plClothingMgr();
    ~plClothingMgr();

    CLASSNAME_REGISTER( plClothingMgr );
    GETINTERFACE_ANY( plClothingMgr, hsKeyedObject );

    plClothingLayout *GetLayout(char *name);
    plClothingElement *FindElementByName(char *name);


    // Functions that just relate to the clothing you have permission to wear (closet)
    void AddItemsToCloset(hsTArray<plClosetItem> &items);
    void GetClosetItems(hsTArray<plClosetItem> &out);

    // Functions that relate to all existing clothing
    plClothingItem *FindItemByName(const char *name);
    hsTArray<plClothingItem*>& GetItemList() { return fItems; }
    void GetItemsByGroup(uint8_t group, hsTArray<plClothingItem*> &out);
    void GetItemsByGroupAndType(uint8_t group, uint8_t type, hsTArray<plClothingItem*> &out);
    void GetAllWithSameMesh(plClothingItem *item, hsTArray<plClothingItem*> &out);
    
    // Give an array of items (from one of the above functions, for example)
    // and this will yank out items so that only item is in the array for each mesh.
    void FilterUniqueMeshes(hsTArray<plClothingItem*> &items);

    // For a pair of items that go together (ie gloves) give us one, we'll give you the other
    plClothingItem *GetLRMatch(plClothingItem *item);
    hsBool IsLRMatch(plClothingItem *item1, plClothingItem *item2);

    static void ChangeAvatar(char *name);
    
    static plClothingMgr *GetClothingMgr() { return fInstance; }    
    static void Init();
    static void DeInit();

    //virtual void Read(hsStream* s, hsResMgr* mgr);
    //virtual void Write(hsStream* s, hsResMgr* mgr);

    virtual hsBool MsgReceive(plMessage* msg);
    

    // NOTE:
    // NOTE:
    // NOTE: The following two enums are duplicated in Python, so...
    //       DON'T CHANGE THEIR CURRENT VALUES, ADD NEW ONES ON THE END OF THE LIST!
    enum
    {
        kClothingBaseMale = 0,
        kClothingBaseFemale,
        kClothingBaseNoOptions, // Custom avatars
        kMaxGroup
    };
    enum
    {
        kTypePants = 0,
        kTypeShirt,
        kTypeLeftHand,
        kTypeRightHand,
        kTypeFace,
        kTypeHair,
        kTypeLeftFoot,
        kTypeRightFoot,
        kTypeAccessory,
        kMaxType,
    };

    static const char *GroupStrings[];
    static const char *TypeStrings[];
};





#endif
