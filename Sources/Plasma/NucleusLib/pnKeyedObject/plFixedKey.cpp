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
//////////////////////////////////////////////////////////////////////////////
//
//  plFixedKey
//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "plUoid.h"
#include <string.h>

#include "plFixedKey.h"
#include "plCreatableIndex.h"
#include "pnKeyedObject/plKey.h"
#include "pnFactory/plCreator.h"

//// plKeySeed ///////////////////////////////////////////////////////////////
//  Our seed struct. Defined here so it doesn't have to be in the .h file

struct plKeySeed
{
    plFixedKeyId  feFixedKey;
    // NOTE: The following fields are broken out to make adding to the fixed key list easier.
    // However, what they really are, are just the fields of plUoid (including plLocation)
    uint16_t    fType;
    plString    fObj;
   
    hsBool Match( plKeySeed *p ) 
    {  
        if( ( fType == p->fType ) && p->fObj.Compare( fObj, plString::kCaseInsensitive ) == 0 )
        {
            return true;
        }
        return false;
    }
};

// Rules for SeedList:
    // 1) Must be in the Same order as enum fixedKey
    // 2) For now at least, all your fixed keys get put into the kGlobalFixedLoc room, reserved just for fixed keys
    // 2) Be sure your ClassIndex CLASS_INDEX(plSceneObject) matches the type of object you want to have the fixedKey
    // 3) Make sure the Obj is unique for this location/Type Combo... (validated at runtime)

#define _TCFL _TEMP_CONVERT_FROM_LITERAL
plKeySeed SeedList[] = {
        //  Key Enum                    Type                                            Obj

    { kFirst_Fixed_KEY,                 CLASS_INDEX_SCOPED( plSceneObject ),            _TCFL("kFirst_Fixed_KEY")              },

    { kLOSObject_KEY,                   CLASS_INDEX_SCOPED( plLOSDispatch ),            _TCFL("kLOSObject_KEY"),               },
    { kTimerCallbackManager_KEY,        CLASS_INDEX_SCOPED( plTimerCallbackManager ),   _TCFL("kTimerCallbackManager_KEY"),    },
    { kConsoleObject_KEY,               CLASS_INDEX_SCOPED( pfConsole ),                _TCFL("kConsoleObject_KEY"),           },
    { kAudioSystem_KEY,                 CLASS_INDEX_SCOPED( plAudioSystem ),            _TCFL("kAudioSystem_KEY"),             },
    { kInput_KEY,                       CLASS_INDEX_SCOPED( plInputManager ),           _TCFL("kInput_KEY"),                   },
    { kClient_KEY,                      CLASS_INDEX_SCOPED( plClient ),                 _TCFL("kClient_KEY"),                  },
    { kNetClientMgr_KEY,                CLASS_INDEX_SCOPED( plNetClientMgr ),           _TCFL("kNetClientMgr_KEY"),            },
    { kListenerMod_KEY,                 CLASS_INDEX_SCOPED( plListener ),               _TCFL("kListenerMod_KEY"),             },
    { kTransitionMgr_KEY,               CLASS_INDEX_SCOPED( plTransitionMgr ),          _TCFL("kTransitionMgr_KEY"),           },
    { kLinkEffectsMgr_KEY,              CLASS_INDEX_SCOPED( plLinkEffectsMgr ),         _TCFL("kLinkEffectsMgr_KEY"),          },
    { kGameGUIMgr_KEY,                  CLASS_INDEX_SCOPED( pfGameGUIMgr ),             _TCFL("kGameGUIMgr_KEY"),              },
    { kGameGUIDynamicDlg_KEY,           CLASS_INDEX_SCOPED( plSceneNode ),              _TCFL("kGameGUIDynamicDlg_KEY"),       },
    { kVirtualCamera1_KEY,              CLASS_INDEX_SCOPED( plVirtualCam1 ),            _TCFL("kVirtualCamera_KEY"),           },
    { kDefaultCameraMod1_KEY,           CLASS_INDEX_SCOPED( plCameraModifier1 ),        _TCFL("kDefaultCameraMod1_KEY"),       },
    { kKIGUIGlue_KEY,                   CLASS_INDEX_SCOPED( pfKI ),                     _TCFL("kKIGUIGlue_KEY"),               },
    { kClothingMgr_KEY,                 CLASS_INDEX_SCOPED( plClothingMgr ),            _TCFL("kClothingMgr_KEY"),             },
    { kInputInterfaceMgr_KEY,           CLASS_INDEX_SCOPED( plInputInterfaceMgr ),      _TCFL("kInputInterfaceMgr_KEY"),       },
    { kAVIWriter_KEY,                   CLASS_INDEX_SCOPED( plAVIWriter ),              _TCFL("kAVIWriter_KEY"),               },
    { kResManagerHelper_KEY,            CLASS_INDEX_SCOPED( plResManagerHelper ),       _TCFL("kResManagerHelper_KEY"),        },
    { kAvatarMgr_KEY,                   CLASS_INDEX_SCOPED( plAvatarMgr ),              _TCFL("kAvatarMgr_KEY"),               },
    { kSimulationMgr_KEY,               CLASS_INDEX_SCOPED( plSimulationMgr ),          _TCFL("kSimulationMgr_KEY"),           },
    { kTransitionCamera_KEY,            CLASS_INDEX_SCOPED( plCameraModifier1 ),        _TCFL("kTransitionCamera_KEY"),        },
    { kCCRMgr_KEY,                      CLASS_INDEX_SCOPED( plCCRMgr ),                 _TCFL("kCCRMgr_KEY"),                  },
    { kNetClientCloneRoom_KEY,          CLASS_INDEX_SCOPED( plSceneNode ),              _TCFL("kNetClientCloneRoom_KEY"),      },
    { kMarkerMgr_KEY,                   CLASS_INDEX_SCOPED( pfMarkerMgr ),              _TCFL("kMarkerMgr_KEY"),               },
    { kAutoProfile_KEY,                 CLASS_INDEX_SCOPED( plAutoProfile ),            _TCFL("kAutoProfile_KEY"),             },
    { kGlobalVisMgr_KEY,                CLASS_INDEX_SCOPED( plVisMgr ),                 _TCFL("kGlobalVisMgr_KEY"),            },
    { kFontCache_KEY,                   CLASS_INDEX_SCOPED( plFontCache ),              _TCFL("kFontCache_KEY"),               },
    { kRelevanceMgr_KEY,                CLASS_INDEX_SCOPED( plRelevanceMgr ),           _TCFL("kRelevanceMgr_KEY"),            },
    { kJournalBookMgr_KEY,              CLASS_INDEX_SCOPED( pfJournalBook ),            _TCFL("kJournalBookMgr_KEY")           },
    { kAgeLoader_KEY,                   CLASS_INDEX_SCOPED( plAgeLoader),               _TCFL("kAgeLoader_KEY")                },
    { kBuiltIn3rdPersonCamera_KEY,      CLASS_INDEX_SCOPED( plCameraModifier1 ),        _TCFL("kBuiltIn3rdPersonCamera_KEY"),  },
    { kSecurePreloader_KEY,             CLASS_INDEX_SCOPED( pfSecurePreloader ),        _TCFL("kSecurePreloader_KEY"),         },
    

    { kLast_Fixed_KEY,                  CLASS_INDEX_SCOPED( plSceneObject ),            _TCFL("kLast_Fixed_KEY"),              }
};
#undef _TCFL


//// plFixedKeyValidator /////////////////////////////////////////////////////
//  Static class that validates the fixed key list on startup, to make sure
//  you didn't mess up the array.

class plFixedKeyValidator
{
    private:
        static plFixedKeyValidator  fValidator;

        plFixedKeyValidator()
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

plFixedKeyValidator plFixedKeyValidator::fValidator;

//// The plUoid Fixed-Key Constructor ////////////////////////////////////////
//  Put here because a) it's fixedKey dependant and b) it ensures that this
//  file gets compiled.

plUoid::plUoid(plFixedKeyId fixedkey)
{
    hsAssert(fixedkey < kLast_Fixed_KEY, "Request for Fixed key is out of Range");

    Invalidate();

    plKeySeed* p= &SeedList[fixedkey];

    fLocation = plLocation::kGlobalFixedLoc;
    fClassType = p->fType;
    fObjectName = p->fObj;
    fObjectID = 0;
    fCloneID = 0;
    fClonePlayerID = 0;
}

