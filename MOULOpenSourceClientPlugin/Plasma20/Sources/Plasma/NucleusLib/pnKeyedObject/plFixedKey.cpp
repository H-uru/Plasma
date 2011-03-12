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
//////////////////////////////////////////////////////////////////////////////
//
//	plFixedKey
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsUtils.h"
#include "plUoid.h"
#include <string.h>

//// plKeySeed ///////////////////////////////////////////////////////////////
//	Our seed struct. Defined here so it doesn't have to be in the .h file

struct plKeySeed
{
	plFixedKeyId  feFixedKey;
	// NOTE: The following fields are broken out to make adding to the fixed key list easier.
	// However, what they really are, are just the fields of plUoid (including plLocation)
	UInt16		fType;
	const char	*fObj;
   
	hsBool Match( plKeySeed *p ) 
	{  
		if( ( fType == p->fType ) && stricmp( p->fObj, fObj ) == 0 )
		{
			return true;
		}
		return false;
	}
};

#include "plFixedKey.h"
#include "plCreatableIndex.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnFactory/plCreator.h"

// Rules for SeedList:
	// 1) Must be in the Same order as enum fixedKey
	// 2) For now at least, all your fixed keys get put into the kGlobalFixedLoc room, reserved just for fixed keys
	// 2) Be sure your ClassIndex CLASS_INDEX(plSceneObject) matches the type of object you want to have the fixedKey
	// 3) Make sure the Obj is unique for this location/Type Combo... (validated at runtime)

plKeySeed SeedList[] = {
		//	Key Enum					Type											Obj

	{ kFirst_Fixed_KEY,					CLASS_INDEX_SCOPED( plSceneObject ),			"kFirst_Fixed_KEY" },

	{ kLOSObject_KEY,					CLASS_INDEX_SCOPED( plLOSDispatch ),			"kLOSObject_KEY",				},
	{ kTimerCallbackManager_KEY,		CLASS_INDEX_SCOPED( plTimerCallbackManager ),	"kTimerCallbackManager_KEY",	},
	{ kConsoleObject_KEY,				CLASS_INDEX_SCOPED( pfConsole ),				"kConsoleObject_KEY",			},
	{ kAudioSystem_KEY,					CLASS_INDEX_SCOPED( plAudioSystem ),			"kAudioSystem_KEY",				},
	{ kInput_KEY,						CLASS_INDEX_SCOPED( plInputManager ),			"kInput_KEY",					},
	{ kClient_KEY,						CLASS_INDEX_SCOPED( plClient ),					"kClient_KEY",					},
	{ kNetClientMgr_KEY,				CLASS_INDEX_SCOPED( plNetClientMgr ),			"kNetClientMgr_KEY",			},
	{ kListenerMod_KEY,					CLASS_INDEX_SCOPED( plListener ),				"kListenerMod_KEY",				},
	{ kTransitionMgr_KEY,				CLASS_INDEX_SCOPED( plTransitionMgr ),			"kTransitionMgr_KEY",			},
	{ kLinkEffectsMgr_KEY,				CLASS_INDEX_SCOPED( plLinkEffectsMgr ),			"kLinkEffectsMgr_KEY",	   		},
	{ kGameGUIMgr_KEY,					CLASS_INDEX_SCOPED( pfGameGUIMgr ),				"kGameGUIMgr_KEY",	   			},
	{ kGameGUIDynamicDlg_KEY,			CLASS_INDEX_SCOPED( plSceneNode ),				"kGameGUIDynamicDlg_KEY",	    },
	{ kVirtualCamera1_KEY,				CLASS_INDEX_SCOPED( plVirtualCam1 ),			"kVirtualCamera_KEY",		    },
	{ kDefaultCameraMod1_KEY,			CLASS_INDEX_SCOPED( plCameraModifier1 ),		"kDefaultCameraMod1_KEY",	    },
	{ kKIGUIGlue_KEY,					CLASS_INDEX_SCOPED( pfKI ),						"kKIGUIGlue_KEY",			    },
	{ kClothingMgr_KEY,					CLASS_INDEX_SCOPED( plClothingMgr ),			"kClothingMgr_KEY",			    },
	{ kInputInterfaceMgr_KEY,			CLASS_INDEX_SCOPED( plInputInterfaceMgr ),		"kInputInterfaceMgr_KEY",	    },
	{ kAVIWriter_KEY,					CLASS_INDEX_SCOPED( plAVIWriter ),				"kAVIWriter_KEY",				},
	{ kResManagerHelper_KEY,			CLASS_INDEX_SCOPED( plResManagerHelper ),		"kResManagerHelper_KEY",		},
	{ kAvatarMgr_KEY,					CLASS_INDEX_SCOPED( plAvatarMgr ),				"kAvatarMgr_KEY",				},
	{ kSimulationMgr_KEY,				CLASS_INDEX_SCOPED( plSimulationMgr ),			"kSimulationMgr_KEY",			},
	{ kTransitionCamera_KEY,			CLASS_INDEX_SCOPED( plCameraModifier1 ),		"kTransitionCamera_KEY",		},
	{ kCCRMgr_KEY,						CLASS_INDEX_SCOPED( plCCRMgr ),					"kCCRMgr_KEY",					},
	{ kNetClientCloneRoom_KEY,			CLASS_INDEX_SCOPED( plSceneNode ),				"kNetClientCloneRoom_KEY",		},
	{ kMarkerMgr_KEY,					CLASS_INDEX_SCOPED( pfMarkerMgr ),				"kMarkerMgr_KEY",				},
	{ kAutoProfile_KEY,					CLASS_INDEX_SCOPED( plAutoProfile ),			"kAutoProfile_KEY",				},
	{ kGlobalVisMgr_KEY,				CLASS_INDEX_SCOPED( plVisMgr ),					"kGlobalVisMgr_KEY",			},
	{ kFontCache_KEY,					CLASS_INDEX_SCOPED( plFontCache ),				"kFontCache_KEY",				},
	{ kRelevanceMgr_KEY,				CLASS_INDEX_SCOPED( plRelevanceMgr ),			"kRelevanceMgr_KEY",			},
	{ kJournalBookMgr_KEY,				CLASS_INDEX_SCOPED( pfJournalBook ),			"kJournalBookMgr_KEY"			},
	{ kAgeLoader_KEY,					CLASS_INDEX_SCOPED( plAgeLoader),				"kAgeLoader_KEY"				},
	{ kBuiltIn3rdPersonCamera_KEY,		CLASS_INDEX_SCOPED( plCameraModifier1 ),		"kBuiltIn3rdPersonCamera_KEY",	},
	{ kSecurePreloader_KEY,				CLASS_INDEX_SCOPED( pfSecurePreloader ),		"kSecurePreloader_KEY",			},
	

	{ kLast_Fixed_KEY,					CLASS_INDEX_SCOPED( plSceneObject ),			"kLast_Fixed_KEY",				}
};


//// plFixedKeyValidator /////////////////////////////////////////////////////
//	Static class that validates the fixed key list on startup, to make sure
//	you didn't mess up the array.

class plFixedKeyValidator
{
	private:
		static plFixedKeyValidator	fValidator;

		plFixedKeyValidator::plFixedKeyValidator()
		{
			// verify that each Seed is in the correct spot...via the enum...
			int i;
			for (i= kFirst_Fixed_KEY; i < kLast_Fixed_KEY; i++)
			{
				plKeySeed *p= &SeedList[i];
				hsAssert(i == p->feFixedKey, "The fixed key table in plFixedKey.h is invalid (a fixed key is misplaced). Please ensure the list follows the given restrictions.");
			}
				// check for duplicates
			for (i= kFirst_Fixed_KEY; i < kLast_Fixed_KEY; i++)
			{
				for (int c = i+1; c < kLast_Fixed_KEY; c++)
				{	
					hsAssert(!SeedList[i].Match(&SeedList[c]), 
							"The fixed key table in plFixedKey.h is invalid (there are duplicate fixed keys). Please ensure the list follows the given restrictions.");
				}
			}
		}
};

plFixedKeyValidator	plFixedKeyValidator::fValidator;

//// The plUoid Fixed-Key Constructor ////////////////////////////////////////
//	Put here because a) it's fixedKey dependant and b) it ensures that this
//	file gets compiled.

plUoid::plUoid(plFixedKeyId fixedkey)
{
	hsAssert(fixedkey < kLast_Fixed_KEY, "Request for Fixed key is out of Range");

	fObjectName = nil;
	Invalidate();

	plKeySeed* p= &SeedList[fixedkey];

	fLocation = plLocation::kGlobalFixedLoc;
	fClassType = p->fType;
	fObjectName = hsStrcpy(p->fObj);
	fObjectID = 0;
	fCloneID = 0;
	fClonePlayerID = 0;
}

