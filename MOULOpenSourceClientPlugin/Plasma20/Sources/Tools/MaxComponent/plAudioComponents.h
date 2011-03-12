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
#define SOUND_3D_COMPONENT_ID	Class_ID(0x1be5543f, 0x746f3a97)
#define BGND_MUSIC_COMPONENT_ID	Class_ID(0x16b5b2a, 0x33c75095)
#define RANDOM_SOUND_COMPONENT_ID Class_ID(0x35033e37, 0x499568fb)
#define GUI_SOUND_COMPONENT_ID	Class_ID(0x446f1ada, 0x6c594a8d)
#define EAX_LISTENER_COMPONENT_ID	Class_ID(0x514f4b0a, 0x24672153)
#define SOUND_PHYS_COMP_ID	Class_ID(0x29415900, 0x1ade37a5)

#include "../pnKeyedObject/plKey.h"
#include "../../AssetMan/PublicInterface/AssManBaseTypes.h"
#include "hsTemplates.h"

class plComponentBase;
class plMaxNode;
class plSoundBuffer;
class plSound;
class plAudioBaseComponentProc;

namespace plAudioComp
{
	// Can't call these until after PreConvert
	int GetSoundModIdx(plComponentBase *comp, plMaxNode *node);
	plKey GetRandomSoundKey(plComponentBase *comp, plMaxNode *node);

	bool	IsSoundComponent(plComponentBase *comp);
	bool	IsLocalOnly( plComponentBase *comp );
}

class plBaseSoundEmitterComponent : public plComponent
{
	public:
		plBaseSoundEmitterComponent();
		virtual ~plBaseSoundEmitterComponent();

		RefTargetHandle Clone(RemapDir &remap);

		IOResult Save(ISave* isave);
		IOResult Load(ILoad* iload);

		enum WhichSound
		{
			kBaseSound,
			kCoverSound
		};

		virtual void	SetSoundAssetId( WhichSound which, jvUniqueId assetId, const TCHAR *fileName );
		virtual jvUniqueId GetSoundAssetID( WhichSound which );
		virtual TCHAR	*GetSoundFileName( WhichSound which );

		// Internal setup and write-only set properties on the MaxNode. No reading
		// of properties on the MaxNode, as it's still indeterminant.
		hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);

		hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg, Class_ID classToConvert );
		hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) = 0;

		hsBool DeInit( plMaxNode *node, plErrorMsg *pErrMsg );

		virtual hsBool	ConvertGrouped( plMaxNode *baseNode, hsTArray<plBaseSoundEmitterComponent *> &groupArray, plErrorMsg *pErrMsg ) { return false; }

		int GetSoundIdx(plMaxNode *node)
		{
			if (fIndices.find(node) != fIndices.end())
				return fIndices[node];
			return -1;
		}

		static plSoundBuffer	*GetSourceBuffer( const char *fileName, plMaxNode *node, UInt32 srcBufferFlags );
		static hsBool			LookupLatestAsset( const char *waveName, char *retPath, plErrorMsg *errMsg );

		virtual void	UpdateSoundFileSelection( void );

		// Loads the given combo box with category selections and sets the ParamID for the category parameter.
		// Returns false if there are no categories to choose for this component
		virtual hsBool	UpdateCategories( HWND dialogBox, int &categoryID, ParamID &paramID );

		virtual hsBool	IsLocalOnly( void ) const { return true; }

		// Virtuals for handling animated volumes
		virtual hsBool	AddToAnim( plAGAnim *anim, plMaxNode *node );
		virtual bool	AllowUnhide() { return fAllowUnhide; }

		// Flags this component to create a grouped sound instead of a normal sound
		void			SetCreateGrouped( plMaxNode *baseNode, int commonSoundIdx );

		// Grabs the current sound volume
		virtual hsScalar	GetSoundVolume( void ) const;

	protected:
		jvUniqueId	fSoundAssetId; // used for the AssMan
		jvUniqueId	fCoverSoundAssetID;
		hsBool		fAssetsUpdated;

		friend class plAudioBaseComponentProc;

		static UInt32		fWarningFlags;
		bool				fAllowUnhide;
	
		hsBool				fCreateGrouped;

		enum Warnings
		{
			kSrcBufferInvalid = 1,
			kCoverBufferInvalid,
			kCoverBufferWrongFormat,
			kMergeSourceFormatMismatch
		};

		void	IUpdateAssets( void );

		static void		IShowError( UInt32 type, const char *errMsg, const char *nodeName, plErrorMsg *pErrMsg );

		std::map<plMaxNode*, int> fIndices;
		std::map<plMaxNode*, bool> fValidNodes;

		bool IValidate(plMaxNode *node, plErrorMsg *pErrMsg);

		void	IConvertOldVolume( void );
		float	IGetDigitalVolume( void ) const;		// In scale 0. to 1., not in db
		void	IGrabFadeValues( plSound *sound );
		void	IGrabSoftRegion( plSound *sound, plErrorMsg *pErrMsg );
		void	IGrabEAXParams( plSound *sound, plErrorMsg *pErrMsg );

		virtual UInt32 ICalcSourceBufferFlags() const;

		static plSoundBuffer *IGetSourceBuffer( const char *fileName, plMaxNode *srcNode, UInt32 srcBufferFlags );

		plSoundBuffer	*IProcessSourceBuffer( plMaxNode *maxNode, plErrorMsg *errMsg );

		virtual hsBool	IAllowStereoFiles( void ) const { return true; }
		virtual hsBool	IAllowMonoFiles( void ) const { return true; }
		virtual hsBool	IHasWaveformProps( void ) const { return true; }

		// Returns pointers to arrays defining the names and konstants for the supported categories
		// for this component. Name array should have an extra "" entry at the end. Returns false if none supported
		virtual hsBool	IGetCategoryList( char **&catList, int *&catKonstantList ) { return false; }

		void	ISetBaseParameters( plSound *destSound, plErrorMsg *pErrMsg );
};

class plRandomSoundMod;

class plRandomSoundComponent : public plComponent
{
protected:
	hsBool	ICheckForSounds(plMaxNode* node);

public:
	std::map<plMaxNode*, plRandomSoundMod*> fSoundMods;

	plRandomSoundComponent();
	void DeleteThis() { delete this; }

	// SetupProperties - Internal setup and write-only set properties on the MaxNode. No reading
	// of properties on the MaxNode, as it's still indeterminant.
	hsBool SetupProperties(plMaxNode *pNode, plErrorMsg *pErrMsg);

	hsBool PreConvert(plMaxNode *pNode, plErrorMsg *pErrMsg);
	hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);

	void RemoveSound(int index);
	void AddSelectedSound();
	int GetCurGroupIdx();
	int GetStartIndex(int group);
	int GetEndIndex(int group);

};

