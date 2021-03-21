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

#include "plFixedKey.h"

#include <string_view>

#include "HeadSpin.h"
#include "plCreatableIndex.h"

#include "plKey.h"
#include "plUoid.h"


#include "pnFactory/plCreator.h"

//// plKeySeed ///////////////////////////////////////////////////////////////
//  Our seed struct. Defined here so it doesn't have to be in the .h file

struct plKeySeed
{
    plFixedKeyId feFixedKey;
    // NOTE: The following fields are broken out to make adding to the fixed key list easier.
    // However, what they really are, are just the fields of plUoid (including plLocation)
    uint16_t fType;
    std::string_view fObj;
};

// Rules for SeedList:
    // 1) Must be in the Same order as enum fixedKey
    // 2) For now at least, all your fixed keys get put into the kGlobalFixedLoc room, reserved just for fixed keys
    // 2) Be sure your ClassIndex CLASS_INDEX(plSceneObject) matches the type of object you want to have the fixedKey
    // 3) Make sure the Obj is unique for this location/Type Combo... (validated at runtime)

static constexpr plKeySeed SeedList[] = {
    //  Key Enum                        Type                                            Obj

    { kFirst_Fixed_KEY,                 CLASS_INDEX_SCOPED( plSceneObject ),            "kFirst_Fixed_KEY",             },

    { kLOSObject_KEY,                   CLASS_INDEX_SCOPED( plLOSDispatch ),            "kLOSObject_KEY",               },
    { kTimerCallbackManager_KEY,        CLASS_INDEX_SCOPED( plTimerCallbackManager ),   "kTimerCallbackManager_KEY",    },
    { kConsoleObject_KEY,               CLASS_INDEX_SCOPED( pfConsole ),                "kConsoleObject_KEY",           },
    { kAudioSystem_KEY,                 CLASS_INDEX_SCOPED( plAudioSystem ),            "kAudioSystem_KEY",             },
    { kInput_KEY,                       CLASS_INDEX_SCOPED( plInputManager ),           "kInput_KEY",                   },
    { kClient_KEY,                      CLASS_INDEX_SCOPED( plClient ),                 "kClient_KEY",                  },
    { kNetClientMgr_KEY,                CLASS_INDEX_SCOPED( plNetClientMgr ),           "kNetClientMgr_KEY",            },
    { kListenerMod_KEY,                 CLASS_INDEX_SCOPED( plListener ),               "kListenerMod_KEY",             },
    { kTransitionMgr_KEY,               CLASS_INDEX_SCOPED( plTransitionMgr ),          "kTransitionMgr_KEY",           },
    { kLinkEffectsMgr_KEY,              CLASS_INDEX_SCOPED( plLinkEffectsMgr ),         "kLinkEffectsMgr_KEY",          },
    { kGameGUIMgr_KEY,                  CLASS_INDEX_SCOPED( pfGameGUIMgr ),             "kGameGUIMgr_KEY",              },
    { kGameGUIDynamicDlg_KEY,           CLASS_INDEX_SCOPED( plSceneNode ),              "kGameGUIDynamicDlg_KEY",       },
    { kVirtualCamera1_KEY,              CLASS_INDEX_SCOPED( plVirtualCam1 ),            "kVirtualCamera_KEY",           },
    { kDefaultCameraMod1_KEY,           CLASS_INDEX_SCOPED( plCameraModifier1 ),        "kDefaultCameraMod1_KEY",       },
    { kKIGUIGlue_KEY,                   CLASS_INDEX_SCOPED( pfKI ),                     "kKIGUIGlue_KEY",               },
    { kClothingMgr_KEY,                 CLASS_INDEX_SCOPED( plClothingMgr ),            "kClothingMgr_KEY",             },
    { kInputInterfaceMgr_KEY,           CLASS_INDEX_SCOPED( plInputInterfaceMgr ),      "kInputInterfaceMgr_KEY",       },
    { kAVIWriter_KEY,                   CLASS_INDEX_SCOPED( plAVIWriter ),              "kAVIWriter_KEY",               },
    { kResManagerHelper_KEY,            CLASS_INDEX_SCOPED( plResManagerHelper ),       "kResManagerHelper_KEY",        },
    { kAvatarMgr_KEY,                   CLASS_INDEX_SCOPED( plAvatarMgr ),              "kAvatarMgr_KEY",               },
    { kSimulationMgr_KEY,               CLASS_INDEX_SCOPED( plSimulationMgr ),          "kSimulationMgr_KEY",           },
    { kTransitionCamera_KEY,            CLASS_INDEX_SCOPED( plCameraModifier1 ),        "kTransitionCamera_KEY",        },
    { kCCRMgr_KEY,                      CLASS_INDEX_SCOPED( plCCRMgr ),                 "kCCRMgr_KEY",                  },
    { kNetClientCloneRoom_KEY,          CLASS_INDEX_SCOPED( plSceneNode ),              "kNetClientCloneRoom_KEY",      },
    { kMarkerMgr_KEY,                   CLASS_INDEX_SCOPED( pfMarkerMgr ),              "kMarkerMgr_KEY",               },
    { kAutoProfile_KEY,                 CLASS_INDEX_SCOPED( plAutoProfile ),            "kAutoProfile_KEY",             },
    { kGlobalVisMgr_KEY,                CLASS_INDEX_SCOPED( plVisMgr ),                 "kGlobalVisMgr_KEY",            },
    { kFontCache_KEY,                   CLASS_INDEX_SCOPED( plFontCache ),              "kFontCache_KEY",               },
    { kRelevanceMgr_KEY,                CLASS_INDEX_SCOPED( plRelevanceMgr ),           "kRelevanceMgr_KEY",            },
    { kJournalBookMgr_KEY,              CLASS_INDEX_SCOPED( pfJournalBook ),            "kJournalBookMgr_KEY",          },
    { kAgeLoader_KEY,                   CLASS_INDEX_SCOPED( plAgeLoader),               "kAgeLoader_KEY",               },
    { kBuiltIn3rdPersonCamera_KEY,      CLASS_INDEX_SCOPED( plCameraModifier1 ),        "kBuiltIn3rdPersonCamera_KEY",  },

    { kLast_Fixed_KEY,                  CLASS_INDEX_SCOPED( plSceneObject ),            "kLast_Fixed_KEY",              }
};

namespace plFixedKeyValidator
{
    static constexpr bool ValidateBounds()
    {
        if (kFirst_Fixed_KEY != 0)
            return false;
        for (size_t i = kFirst_Fixed_KEY; i < kLast_Fixed_KEY; ++i) {
            if (SeedList[i].feFixedKey != i)
                return false;
        }
        if (std::size(SeedList) - 1 != kLast_Fixed_KEY)
            return false;
        return true;
    }

    static constexpr bool CheckForDupes()
    {
        // Would be nice to use something like std::any_of for this,
        // but that did not become constexpr until C++20.
        for (size_t i = kFirst_Fixed_KEY; i < kLast_Fixed_KEY; i++) {
            for (size_t j = i + 1; j < kLast_Fixed_KEY; j++) {
                if (SeedList[i].feFixedKey == SeedList[j].feFixedKey)
                    return false;
                if (SeedList[i].fType == SeedList[j].fType &&
                    SeedList[i].fObj == SeedList[j].fObj)
                    return false;
            }
        }
        return true;
    }
};

static_assert(plFixedKeyValidator::ValidateBounds(),
              "The fixed key table is invalid (a fixed key is misplaced).");
static_assert(plFixedKeyValidator::CheckForDupes(),
              "The fixed key table in invalid (there are duplicate fixed keys).");

//// The plUoid Fixed-Key Constructor ////////////////////////////////////////
//  Put here because a) it's fixedKey dependant and b) it ensures that this
//  file gets compiled.

plUoid::plUoid(plFixedKeyId fixedkey)
{
    hsAssert(fixedkey < kLast_Fixed_KEY, "Request for Fixed key is out of Range");

    Invalidate();

    fLocation = plLocation::kGlobalFixedLoc;
    fClassType = SeedList[fixedkey].fType;
    fObjectName = SeedList[fixedkey].fObj;
    fObjectID = 0;
    fCloneID = 0;
    fClonePlayerID = 0;
}

