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

#ifndef _plAudioComponents_h_inc_
#define _plAudioComponents_h_inc_

#define SOUND_3D_COMPONENT_ID   Class_ID(0x1be5543f, 0x746f3a97)
#define BGND_MUSIC_COMPONENT_ID Class_ID(0x16b5b2a, 0x33c75095)
#define RANDOM_SOUND_COMPONENT_ID Class_ID(0x35033e37, 0x499568fb)
#define GUI_SOUND_COMPONENT_ID  Class_ID(0x446f1ada, 0x6c594a8d)
#define EAX_LISTENER_COMPONENT_ID   Class_ID(0x514f4b0a, 0x24672153)
#define SOUND_PHYS_COMP_ID  Class_ID(0x29415900, 0x1ade37a5)

#include <map>
#include <vector>

#include <string_theory/format>

#include "plComponent.h"
#include "pnKeyedObject/plKey.h"

#ifdef MAXASS_AVAILABLE
#   include "../../AssetMan/PublicInterface/AssManBaseTypes.h"
#   define plAudioId jvUniqueId
#else
    struct plAudioId {
        uint32_t id;

        plAudioId() : id() { }
    };
#endif

class plComponentBase;
class plMaxNode;
class plSoundBuffer;
class plSound;
class plAudioBaseComponentProc;
class plFileName;

namespace plAudioComp
{
    // Can't call these until after PreConvert
    int GetSoundModIdx(plComponentBase *comp, plMaxNode *node);
    plKey GetRandomSoundKey(plComponentBase *comp, plMaxNode *node);

    bool    IsSoundComponent(plComponentBase *comp);
    bool    IsLocalOnly( plComponentBase *comp );
};

class plBaseSoundEmitterComponent : public plComponent
{
    public:
        plBaseSoundEmitterComponent();
        virtual ~plBaseSoundEmitterComponent();

        RefTargetHandle Clone(RemapDir &remap) override;

        IOResult Save(ISave* isave) override;
        IOResult Load(ILoad* iload) override;

        enum WhichSound
        {
            kBaseSound,
            kCoverSound
        };

        virtual void    SetSoundAssetId( WhichSound which, plAudioId assetId, const MCHAR *fileName );
        virtual plAudioId GetSoundAssetID( WhichSound which );
        virtual const MCHAR* GetSoundFileName( WhichSound which );

        // Internal setup and write-only set properties on the MaxNode. No reading
        // of properties on the MaxNode, as it's still indeterminant.
        bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;

        bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg, Class_ID classToConvert);
        bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override = 0;

        bool DeInit(plMaxNode *node, plErrorMsg *pErrMsg) override;

        virtual bool ConvertGrouped(plMaxNode *baseNode, std::vector<plBaseSoundEmitterComponent *> &groupArray, plErrorMsg *pErrMsg) { return false; }

        int GetSoundIdx(plMaxNode *node)
        {
            if (fIndices.find(node) != fIndices.end())
                return fIndices[node];
            return -1;
        }

        static plSoundBuffer    *GetSourceBuffer( const plFileName &fileName, plMaxNode *node, uint32_t srcBufferFlags );
        static bool             LookupLatestAsset( const char *waveName, char *retPath, plErrorMsg *errMsg );

        virtual void    UpdateSoundFileSelection();

        // Loads the given combo box with category selections and sets the ParamID for the category parameter.
        // Returns false if there are no categories to choose for this component
        virtual bool    UpdateCategories( HWND dialogBox, int &categoryID, ParamID &paramID );

        virtual bool    IsLocalOnly() const { return true; }

        // Virtuals for handling animated volumes
        bool    AddToAnim(plAGAnim *anim, plMaxNode *node) override;
        bool    AllowUnhide() override { return fAllowUnhide; }

        // Flags this component to create a grouped sound instead of a normal sound
        void            SetCreateGrouped( plMaxNode *baseNode, int commonSoundIdx );

        // Grabs the current sound volume
        virtual float    GetSoundVolume() const;

    protected:
        plAudioId   fSoundAssetId; // used for the AssMan
        plAudioId   fCoverSoundAssetID;
        bool        fAssetsUpdated;

        friend class plAudioBaseComponentProc;

        static uint32_t       fWarningFlags;
        bool                fAllowUnhide;
    
        bool                fCreateGrouped;

        enum Warnings
        {
            kSrcBufferInvalid = 1,
            kCoverBufferInvalid,
            kCoverBufferWrongFormat,
            kMergeSourceFormatMismatch
        };

        void    IUpdateAssets();

        template<typename... _ArgsT>
        static void IShowError(uint32_t type, plErrorMsg* pErrMsg, const char* fmt, _ArgsT&&... args)
        {
            IShowError(type, pErrMsg, ST::format(fmt, std::forward<_ArgsT>(args)...));
        }

        static void     IShowError( uint32_t type, plErrorMsg* pErrMsg, ST::string msg);

        std::map<plMaxNode*, int> fIndices;
        std::map<plMaxNode*, bool> fValidNodes;

        bool IValidate(plMaxNode *node, plErrorMsg *pErrMsg);

        void    IConvertOldVolume();
        float   IGetDigitalVolume() const;        // In scale 0. to 1., not in db
        void    IGrabFadeValues( plSound *sound );
        void    IGrabSoftRegion( plSound *sound, plErrorMsg *pErrMsg );
        void    IGrabEAXParams( plSound *sound, plErrorMsg *pErrMsg );

        virtual uint32_t ICalcSourceBufferFlags() const;

        static plSoundBuffer *IGetSourceBuffer( const plFileName &fileName, plMaxNode *srcNode, uint32_t srcBufferFlags );

        plSoundBuffer   *IProcessSourceBuffer( plMaxNode *maxNode, plErrorMsg *errMsg );

        virtual bool    IAllowStereoFiles() const { return true; }
        virtual bool    IAllowMonoFiles() const { return true; }
        virtual bool    IHasWaveformProps() const { return true; }

        // Returns pointers to arrays defining the names and konstants for the supported categories
        // for this component. Name array should have an extra "" entry at the end. Returns false if none supported
        virtual bool    IGetCategoryList( TCHAR **&catList, int *&catKonstantList ) { return false; }

        void    ISetBaseParameters( plSound *destSound, plErrorMsg *pErrMsg );
};

class plRandomSoundMod;

class plRandomSoundComponent : public plComponent
{
protected:
    bool    ICheckForSounds(plMaxNode* node);

public:
    std::map<plMaxNode*, plRandomSoundMod*> fSoundMods;

    plRandomSoundComponent();
    void DeleteThis() override { delete this; }

    // SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
    // of properties on the MaxNode, as it's still indeterminant.
    bool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg) override;

    bool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;

    void RemoveSound(int index);
    void AddSelectedSound();
    int GetCurGroupIdx();
    int GetStartIndex(int group);
    int GetEndIndex(int group);

};

#endif // _plAudioComponents_h_inc_

