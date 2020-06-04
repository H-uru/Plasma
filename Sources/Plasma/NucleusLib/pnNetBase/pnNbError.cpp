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

#include "pnNbError.h"
#include <iterator>


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
// These errors should only be used in debugging.  They should not be shown
// in release clients because they are not localized
const wchar_t * NetErrorToString (ENetError code) {

    static const wchar_t* s_errors[] = {
        L"Success",                         // kNetSuccess
        L"Internal Error",                  // kNetErrInternalError
        L"No Response From Server",         // kNetErrTimeout
        L"Invalid Server Data",             // kNetErrBadServerData
        L"Age Not Found",                   // kNetErrAgeNotFound
        L"Network Connection Failed",       // kNetErrConnectFailed
        L"Disconnected From Server",        // kNetErrDisconnected
        L"File Not Found",                  // kNetErrFileNotFound
        L"Old Build",                       // kNetErrOldBuildId
        L"Remote Shutdown",                 // kNetErrRemoteShutdown
        L"Database Timeout",                // kNetErrTimeoutOdbc
        L"Account Already Exists",          // kNetErrAccountAlreadyExists
        L"Player Already Exists",           // kNetErrPlayerAlreadyExists
        L"Account Not Found",               // kNetErrAccountNotFound
        L"Player Not Found",                // kNetErrPlayerNotFound
        L"Invalid Parameter",               // kNetErrInvalidParameter
        L"Name Lookup Failed",              // kNetErrNameLookupFailed
        L"Logged In Elsewhere",             // kNetErrLoggedInElsewhere
        L"Vault Node Not Found",            // kNetErrVaultNodeNotFound
        L"Max Players On Account",          // kNetErrMaxPlayersOnAcct
        L"Authentication Failed",           // kNetErrAuthenticationFailed
        L"State Object Not Found",          // kNetErrStateObjectNotFound
        L"Login Denied",                    // kNetErrLoginDenied
        L"Circular Reference",              // kNetErrCircularReference
        L"Account Not Activated",           // kNetErrAccountNotActivated
        L"Key Already Used",                // kNetErrKeyAlreadyUsed
        L"Key Not Found",                   // kNetErrKeyNotFound
        L"Activation Code Not Found",       // kNetErrActivationCodeNotFound
        L"Player Name Invalid",             // kNetErrPlayerNameInvalid
        L"Not Supported",                   // kNetErrNotSupported
        L"Service Forbidden",               // kNetErrServiceForbidden
        L"Auth Token Too Old",              // kNetErrAuthTokenTooOld
        L"Must Use GameTap Client",         // kNetErrMustUseGameTapClient
        L"Too Many Failed Logins",          // kNetErrTooManyFailedLogins
        L"GameTap: Connection Failed",      // kNetErrGameTapConnectionFailed
        L"GameTap: Too Many Auth Options",  // kNetErrGTTooManyAuthOptions
        L"GameTap: Missing Parameter",      // kNetErrGTMissingParameter
        L"GameTap: Server Error",           // kNetErrGTServerError
        L"Account has been banned",         // kNetErrAccountBanned
        L"Account kicked by CCR",           // kNetErrKickedByCCR
        L"Wrong score type for operation",  // kNetErrScoreWrongType
        L"Not enough points",               // kNetErrScoreNotEnoughPoints
        L"Non-fixed score already exists",  // kNetErrScoreAlreadyExists
        L"No score data found",             // kNetErrScoreNoDataFound
        L"Invite: Couldn't find player",    // kNetErrInviteNoMatchingPlayer
        L"Invite: Too many hoods",          // kNetErrInviteTooManyHoods
        L"Payments not up to date",         // kNetErrNeedToPay
        L"Server Busy",                     // kNetErrServerBusy
        L"Vault Node Access Violation",     // kNetErrVaultNodeAccessViolation
    };
    static_assert(std::size(s_errors) == kNumNetErrors,
                  "Number of Net Error descriptions and total Net Error count are not equal");
    
    if ((unsigned)code >= std::size(s_errors)) {
        if (code == kNetPending)
            return L"Pending";
        return L"Unknown Error";
    }
    
    return s_errors[code];
}

//============================================================================
// These errors should only be used in debugging.  They should not be shown
// in release clients because they are not localized
const wchar_t * NetErrorAsString (ENetError code) {

    static const wchar_t* s_errors[] = {
        L"kNetSuccess",
        L"kNetErrInternalError",
        L"kNetErrTimeout",
        L"kNetErrBadServerData",
        L"kNetErrAgeNotFound",
        L"kNetErrConnectFailed",
        L"kNetErrDisconnected",
        L"kNetErrFileNotFound",
        L"kNetErrOldBuildId",
        L"kNetErrRemoteShutdown",
        L"kNetErrTimeoutOdbc",
        L"kNetErrAccountAlreadyExists",
        L"kNetErrPlayerAlreadyExists",
        L"kNetErrAccountNotFound",
        L"kNetErrPlayerNotFound",
        L"kNetErrInvalidParameter",
        L"kNetErrNameLookupFailed",
        L"kNetErrLoggedInElsewhere",
        L"kNetErrVaultNodeNotFound",
        L"kNetErrMaxPlayersOnAcct",
        L"kNetErrAuthenticationFailed",
        L"kNetErrStateObjectNotFound",
        L"kNetErrLoginDenied",
        L"kNetErrCircularReference",
        L"kNetErrAccountNotActivated",
        L"kNetErrKeyAlreadyUsed",
        L"kNetErrKeyNotFound",
        L"kNetErrActivationCodeNotFound",
        L"kNetErrPlayerNameInvalid",
        L"kNetErrNotSupported",
        L"kNetErrServiceForbidden",
        L"kNetErrAuthTokenTooOld",
        L"kNetErrMustUseGameTapClient",
        L"kNetErrTooManyFailedLogins",
        L"kNetErrGameTapConnectionFailed",
        L"kNetErrGTTooManyAuthOptions",
        L"kNetErrGTMissingParameter",
        L"kNetErrGTServerError",
        L"kNetErrAccountBanned",
        L"kNetErrKickedByCCR",
        L"kNetErrScoreWrongType",
        L"kNetErrScoreNotEnoughPoints",
        L"kNetErrScoreAlreadyExists",
        L"kNetErrScoreNoDataFound",
        L"kNetErrInviteNoMatchingPlayer",
        L"kNetErrInviteTooManyHoods",
        L"kNetErrNeedToPay",
        L"kNetErrServerBusy",
        L"kNetErrVaultNodeAccessViolation",
    };
    static_assert(std::size(s_errors) == kNumNetErrors,
                  "Number of string-ized Net Errors and total Net Error count are not equal");
    
    if ((unsigned)code >= std::size(s_errors)) {
        if (code == kNetPending)
            return L"kNetPending";
        return L"ErrUnknown";
    }
    
    return s_errors[code];
}
