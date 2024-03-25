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

#ifndef pnNbError_inc
#define pnNbError_inc

#include "HeadSpin.h"
#include "pnNbConst.h"

/*****************************************************************************
*
*   Net Error
*
***/

// These codes may not be changed unless ALL servers, clients and databases
// are simultaneously updated; so basically forget it =)
enum ENetError: int32_t {
    // codes <= 0 are not errors
    kNetPending                     = -1,
    kNetSuccess                     = 0,

    // codes > 0 are errors
    kNetErrInternalError            = 1,
    kNetErrTimeout                  = 2,
    kNetErrBadServerData            = 3,
    kNetErrAgeNotFound              = 4,
    kNetErrConnectFailed            = 5,
    kNetErrDisconnected             = 6,
    kNetErrFileNotFound             = 7,
    kNetErrOldBuildId               = 8,
    kNetErrRemoteShutdown           = 9,
    kNetErrTimeoutOdbc              = 10,
    kNetErrAccountAlreadyExists     = 11,
    kNetErrPlayerAlreadyExists      = 12,
    kNetErrAccountNotFound          = 13,
    kNetErrPlayerNotFound           = 14,
    kNetErrInvalidParameter         = 15,
    kNetErrNameLookupFailed         = 16,
    kNetErrLoggedInElsewhere        = 17,
    kNetErrVaultNodeNotFound        = 18,
    kNetErrMaxPlayersOnAcct         = 19,
    kNetErrAuthenticationFailed     = 20,
    kNetErrStateObjectNotFound      = 21,
    kNetErrLoginDenied              = 22,
    kNetErrCircularReference        = 23,
    kNetErrAccountNotActivated      = 24,
    kNetErrKeyAlreadyUsed           = 25,
    kNetErrKeyNotFound              = 26,
    kNetErrActivationCodeNotFound   = 27,
    kNetErrPlayerNameInvalid        = 28,
    kNetErrNotSupported             = 29,
    kNetErrServiceForbidden         = 30,
    kNetErrAuthTokenTooOld          = 31,
    kNetErrMustUseGameTapClient     = 32,
    kNetErrTooManyFailedLogins      = 33,
    kNetErrGameTapConnectionFailed  = 34,
    kNetErrGTTooManyAuthOptions     = 35,
    kNetErrGTMissingParameter       = 36,
    kNetErrGTServerError            = 37,
    kNetErrAccountBanned            = 38,
    kNetErrKickedByCCR              = 39,
    kNetErrScoreWrongType           = 40,
    kNetErrScoreNotEnoughPoints     = 41,
    kNetErrScoreAlreadyExists       = 42,
    kNetErrScoreNoDataFound         = 43,
    kNetErrInviteNoMatchingPlayer   = 44,
    kNetErrInviteTooManyHoods       = 45,
    kNetErrNeedToPay                = 46,
    kNetErrServerBusy               = 47,
    kNetErrVaultNodeAccessViolation = 48,
    
    // Be sure to add strings for additional error codes in pnNbError.cpp
    // (don't worry, the compiler will tell you if one is missing ;)
    kNumNetErrors,
};

static_assert(sizeof(ENetError) == sizeof(int32_t), "ENetError must be sizeof int32_t");

#define IS_NET_ERROR(a)     (((int)(a)) > kNetSuccess)
#define IS_NET_SUCCESS(a)   (((int)(a)) == kNetSuccess)


const wchar_t * NetErrorToString (ENetError code);    // user-friendly string
const wchar_t * NetErrorAsString (ENetError code);    // string version of enum identifier

#endif // pnNbError_inc
