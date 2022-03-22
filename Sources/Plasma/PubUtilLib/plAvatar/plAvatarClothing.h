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

#include "hsBitVector.h"
#include "hsColorRGBA.h"

#include <string_theory/string>
#include <vector>

#include "pnNetCommon/plSynchedObject.h"

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
class plMetalPipeline;

struct plClothingItemOptions
{
    hsColorRGBA fTint1;
    hsColorRGBA fTint2;

    plClothingItemOptions() { fTint1.Set(1.f, 1.f, 1.f, 1.f); fTint2.Set(1.f, 1.f, 1.f, 1.f); }

    bool IsMatch(const plClothingItemOptions *other) const
    {
        return fTint1 == other->fTint1 && fTint2 == other->fTint2;
    }
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
    ST::string fName;
    plSharedMesh *fMeshes[kMaxNumLODLevels];
    std::vector<plMipmap **> fTextures;
    std::vector<ST::string> fElementNames;
    std::vector<plClothingElement *> fElements;
    uint8_t fGroup;   // Each avatar can wear one of the available groups
    uint8_t fType;    // Each group has multiple types of clothes (shirt/pants/etc)
    uint8_t fTileset;
    uint8_t fSortOrder;
    ST::string fDescription;
    ST::string fCustomText;
    plMipmap *fThumbnail;
    plClothingItem *fAccessory; // Forced accessory to always wear with this item.
    uint8_t fDefaultTint1[3];
    uint8_t fDefaultTint2[3];

    ST::string fAccessoryName; // Export only



    plClothingItem();
    ~plClothingItem();
    
    CLASSNAME_REGISTER( plClothingItem );
    GETINTERFACE_ANY( plClothingItem, hsKeyedObject );

    void SetName(const ST::string &name) { fName = name; }
    ST::string GetName() const { return fName; }
    bool CanWearWith(plClothingItem *item);
    bool WearBefore(plClothingItem *item); // Should we come before the arg item? (texture gen order)
    bool HasBaseAlpha();
    bool HasSameMeshes(const plClothingItem *other) const;

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;
};

struct plClosetItem
{
    plClosetItem() : fItem() { }

    plClothingItem *fItem;
    plClothingItemOptions fOptions;

    bool IsMatch(const plClosetItem *other) const;
};

class plClothingBase : public hsKeyedObject
{
public:
    ST::string fName;
    plMipmap *fBaseTexture;
    ST::string fLayoutName;

    plClothingBase();

    CLASSNAME_REGISTER( plClothingBase );
    GETINTERFACE_ANY( plClothingBase, hsKeyedObject );
    
    void SetLayoutName(const ST::string &name) { fLayoutName = name; }

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;
};

class plClothingOutfit : public plSynchedObject
{
    friend class plDXPipeline;
    friend class plMetalPipeline;

public:
    plArmatureMod *fAvatar;
    plLayer *fTargetLayer;
    hsGMaterial *fMaterial; // Needed to tell swapped geometry what material to use.
    // TODO: Consider a tuple?
    std::vector<plClothingItem*> fItems;
    std::vector<plClothingItemOptions*> fOptions;
    plClothingBase *fBase;
    uint8_t fGroup;
    bool fSynchClients;     // set true if the next synch should be bcast
    hsColorRGBA fSkinTint;
    float fSkinBlends[plClothingElement::kLayerSkinLast - plClothingElement::kLayerSkinFirst]; // Controls the opacity between skin textures.

    plClothingOutfit();
    ~plClothingOutfit();

    CLASSNAME_REGISTER( plClothingOutfit );
    GETINTERFACE_ANY( plClothingOutfit, plSynchedObject );

    void SaveCustomizations(bool retry = true);
    void AddItem(plClothingItem *item, bool update = true, bool broadcast = true, bool netForce=false);
    void RemoveItem(plClothingItem *item, bool update = true, bool netForce=false);
    void TintItem(plClothingItem *item, float red, float green, float blue, bool update = true, bool broadcast = true, 
                  bool netForce = false, bool retry = true, uint8_t fLayer = plClothingElement::kLayerTint1);
    void TintSkin(float red, float green, float blue,
                  bool update = true, bool broadcast = true);
    void MorphItem(plClothingItem *item, uint8_t layer, uint8_t delta, float weight, bool retry = true);
    void SetAge(float age, bool update = true, bool broadcast = true);
    void SetSkinBlend(float blend, uint8_t layer, bool update = true, bool broadcast = true);
    float GetSkinBlend(uint8_t layer);     
    hsColorRGBA GetItemTint(plClothingItem *item, uint8_t layer = 2) const;
    float GetAge() const { return fSkinBlends[0]; }
    const std::vector<plClothingItem*> &GetItemList() const { return fItems; }
    const std::vector<plClothingItemOptions*> &GetOptionList() const { return fOptions; }

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;
    bool DirtySynchState(const ST::string& SDLStateName, uint32_t synchFlags) override;

    void StripAccessories();
    void WearDefaultClothing(bool broadcast = false);
    void WearDefaultClothingType(uint32_t clothingType, bool broadcast = false);
    void WearMaintainerOutfit();
    void WearRandomOutfit();
    void RemoveMaintainerOutfit();

    bool ReadItems(hsStream* s, hsResMgr* mgr, bool broadcast = true);
    void WriteItems(hsStream* s, hsResMgr* mgr);
    
    void ForceUpdate(bool retry);       // send updateTexture msg

    bool MsgReceive(plMessage* msg) override;

    void IInstanceSharedMeshes(plClothingItem *item);
    void IRemoveSharedMeshes(plClothingItem *item);

    /** This will load the avatar clothing. If a clothing file is set,
     *  we will load from the file, otherwise from the vault.
     */
    bool ReadClothing();

    void WriteToVault();
    void WriteToVault(const std::vector<plStateDataRecord*> & SDRs);

    /** Write the avatar clothing to a file */
    bool WriteToFile(const plFileName &filename);

    void SetupMorphSDL();

    // XXX Don't use this. Temp function for a temp HACK console command.
    void DirtyTileset(int tileset);

    /** Instruct this plClothingOutfit to read clothing from the given file */
    void SetClothingFile(const plFileName &file) { fClothingFile = file; }

    /** Returns the clothing file of this outfit. If there is none, an empty string
     *  will be returned.
     */
    plFileName GetClothingFile() const { return fClothingFile; }

protected:
    hsBitVector fDirtyItems;
    bool fVaultSaveEnabled;
    bool fMorphsInitDone;
    plFileName fClothingFile;

    void IAddItem(plClothingItem *item);
    void IRemoveItem(plClothingItem *item);
    bool ITintItem(plClothingItem *item, hsColorRGBA color, uint8_t layer);
    bool IMorphItem(plClothingItem *item, uint8_t layer, uint8_t delta, float weight);
    void IHandleMorphSDR(plStateDataRecord *sdr);

    bool IReadFromVault();

    /** Read the avatar clothing from a file.
     *  A local avatar will change the clothing group to the one in the file.
     *  A local avatar will be invisible if the file does not exist. (used in the Startup age)
     */
    bool IReadFromFile(const plFileName &filename);
        
    void IUpdate();

};

class plClothingMgr : public hsKeyedObject
{
private:
    static plClothingMgr *fInstance;

    std::vector<plClothingElement*> fElements;
    std::vector<plClothingItem*> fItems;
    std::vector<plClothingLayout*> fLayouts;
    
    void IInit();
    void IAddItem(plClothingItem *item);

public:
    plClothingMgr() = default;
    ~plClothingMgr();

    CLASSNAME_REGISTER( plClothingMgr );
    GETINTERFACE_ANY( plClothingMgr, hsKeyedObject );

    plClothingLayout *GetLayout(const ST::string &name) const;
    plClothingElement *FindElementByName(const ST::string &name) const;


    // Functions that just relate to the clothing you have permission to wear (closet)
    void AddItemsToCloset(const std::vector<plClosetItem> &items);
    void GetClosetItems(std::vector<plClosetItem> &out);

    // Functions that relate to all existing clothing
    plClothingItem *FindItemByName(const ST::string &name) const;
    const std::vector<plClothingItem*>& GetItemList() const { return fItems; }
    void GetItemsByGroup(uint8_t group, std::vector<plClothingItem*> &out);
    void GetItemsByGroupAndType(uint8_t group, uint8_t type, std::vector<plClothingItem*> &out);
    void GetAllWithSameMesh(plClothingItem *item, std::vector<plClothingItem*> &out);
    
    // Give an array of items (from one of the above functions, for example)
    // and this will yank out items so that only item is in the array for each mesh.
    void FilterUniqueMeshes(std::vector<plClothingItem*> &items);

    // For a pair of items that go together (ie gloves) give us one, we'll give you the other
    plClothingItem *GetLRMatch(plClothingItem *item);
    bool IsLRMatch(plClothingItem *item1, plClothingItem *item2);

    static void ChangeAvatar(const ST::string& name, const plFileName &clothingFile = {});
    
    static plClothingMgr *GetClothingMgr() { return fInstance; }    
    static void Init();
    static void DeInit();

    //void Read(hsStream* s, hsResMgr* mgr) override;
    //void Write(hsStream* s, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;
    

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

    static const ST::string GroupStrings[];
    static const ST::string TypeStrings[];
};





#endif
