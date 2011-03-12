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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbError.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exported functions
*
***/

//============================================================================
// These errors should only be used in debugging.  They should not be shown
// in release clients because they are not localized
const wchar * NetErrorToString (ENetError code) {

    static wchar * s_errors[] = {
		L"Success",							// kNetSuccess
        L"Internal Error",					// kNetErrInternalError
        L"No Response From Server",			// kNetErrTimeout
        L"Invalid Server Data",				// kNetErrBadServerData
        L"Age Not Found",					// kNetErrAgeNotFound
        L"Network Connection Failed",		// kNetErrConnectFailed
        L"Disconnected From Server",		// kNetErrDisconnected
        L"File Not Found",					// kNetErrFileNotFound
        L"Old Build",						// kNetErrOldBuildId
        L"Remote Shutdown",					// kNetErrRemoteShutdown
        L"Database Timeout",				// kNetErrTimeoutOdbc
        L"Account Already Exists",			// kNetErrAccountAlreadyExists
        L"Player Already Exists",			// kNetErrPlayerAlreadyExists
        L"Account Not Found",				// kNetErrAccountNotFound
        L"Player Not Found",				// kNetErrPlayerNotFound
        L"Invalid Parameter",				// kNetErrInvalidParameter
        L"Name Lookup Failed",				// kNetErrNameLookupFailed
        L"Logged In Elsewhere",				// kNetErrLoggedInElsewhere
        L"Vault Node Not Found",			// kNetErrVaultNodeNotFound
        L"Max Players On Account",			// kNetErrMaxPlayersOnAcct
		L"Authentication Failed",			// kNetErrAuthenticationFailed
		L"State Object Not Found",			// kNetErrStateObjectNotFound
		L"Login Denied",					// kNetErrLoginDenied
		L"Circular Reference",				// kNetErrCircularReference
		L"Account Not Activated",			// kNetErrAccountNotActivated
		L"Key Already Used",				// kNetErrKeyAlreadyUsed
		L"Key Not Found",					// kNetErrKeyNotFound
		L"Activation Code Not Found",		// kNetErrActivationCodeNotFound
		L"Player Name Invalid",				// kNetErrPlayerNameInvalid
		L"Not Supported",					// kNetErrNotSupported
		L"Service Forbidden",				// kNetErrServiceForbidden
		L"Auth Token Too Old",				// kNetErrAuthTokenTooOld
		L"Must Use GameTap Client",			// kNetErrMustUseGameTapClient
		L"Too Many Failed Logins",			// kNetErrTooManyFailedLogins
		L"GameTap: Connection Failed",		// kNetErrGameTapConnectionFailed
		L"GameTap: Too Many Auth Options",	// kNetErrGTTooManyAuthOptions
		L"GameTap: Missing Parameter",		// kNetErrGTMissingParameter
		L"GameTap: Server Error",			// kNetErrGTServerError
		L"Account has been banned",			// kNetErrAccountBanned
		L"Account kicked by CCR",			// kNetErrKickedByCCR
		L"Wrong score type for operation",	// kNetErrScoreWrongType
		L"Not enough points",				// kNetErrScoreNotEnoughPoints
		L"Non-fixed score already exists",	// kNetErrScoreAlreadyExists
		L"No score data found",				// kNetErrScoreNoDataFound
		L"Invite: Couldn't find player",	// kNetErrInviteNoMatchingPlayer
		L"Invite: Too many hoods",			// kNetErrInviteTooManyHoods
		L"Payments not up to date",			// kNetErrNeedToPay
		L"Server Busy",						// kNetErrServerBusy
		L"Vault Node Access Violation",		// kNetErrVaultNodeAccessViolation
    };
    COMPILER_ASSERT(arrsize(s_errors) == kNumNetErrors);
    
    if ((unsigned)code >= arrsize(s_errors)) {
        if (code == kNetPending)
            return L"Pending";
        return L"Unknown Error";
    }
    
    return s_errors[code];
}

//============================================================================
// These errors should only be used in debugging.  They should not be shown
// in release clients because they are not localized
const wchar * NetErrorAsString (ENetError code) {

	#define ERROR_STRING(e)	L#e
    static wchar * s_errors[] = {
		ERROR_STRING(kNetSuccess),
        ERROR_STRING(kNetErrInternalError),
        ERROR_STRING(kNetErrTimeout),
        ERROR_STRING(kNetErrBadServerData),
        ERROR_STRING(kNetErrAgeNotFound),
        ERROR_STRING(kNetErrConnectFailed),
        ERROR_STRING(kNetErrDisconnected),
        ERROR_STRING(kNetErrFileNotFound),
        ERROR_STRING(kNetErrOldBuildId),
        ERROR_STRING(kNetErrRemoteShutdown),
        ERROR_STRING(kNetErrTimeoutOdbc),
        ERROR_STRING(kNetErrAccountAlreadyExists),
        ERROR_STRING(kNetErrPlayerAlreadyExists),
        ERROR_STRING(kNetErrAccountNotFound),
        ERROR_STRING(kNetErrPlayerNotFound),
        ERROR_STRING(kNetErrInvalidParameter),
        ERROR_STRING(kNetErrNameLookupFailed),
        ERROR_STRING(kNetErrLoggedInElsewhere),
        ERROR_STRING(kNetErrVaultNodeNotFound),
        ERROR_STRING(kNetErrMaxPlayersOnAcct),
		ERROR_STRING(kNetErrAuthenticationFailed),
		ERROR_STRING(kNetErrStateObjectNotFound),
		ERROR_STRING(kNetErrLoginDenied),
		ERROR_STRING(kNetErrCircularReference),
		ERROR_STRING(kNetErrAccountNotActivated),
		ERROR_STRING(kNetErrKeyAlreadyUsed),
		ERROR_STRING(kNetErrKeyNotFound),
		ERROR_STRING(kNetErrActivationCodeNotFound),
		ERROR_STRING(kNetErrPlayerNameInvalid),
		ERROR_STRING(kNetErrNotSupported),
		ERROR_STRING(kNetErrServiceForbidden),
		ERROR_STRING(kNetErrAuthTokenTooOld),
		ERROR_STRING(kNetErrMustUseGameTapClient),
		ERROR_STRING(kNetErrTooManyFailedLogins),
		ERROR_STRING(kNetErrGameTapConnectionFailed),
		ERROR_STRING(kNetErrGTTooManyAuthOptions),
		ERROR_STRING(kNetErrGTMissingParameter),
		ERROR_STRING(kNetErrGTServerError),
		ERROR_STRING(kNetErrAccountBanned),
		ERROR_STRING(kNetErrKickedByCCR),
		ERROR_STRING(kNetErrScoreWrongType),
		ERROR_STRING(kNetErrScoreNotEnoughPoints),
		ERROR_STRING(kNetErrScoreAlreadyExists),
		ERROR_STRING(kNetErrScoreNoDataFound),
		ERROR_STRING(kNetErrInviteNoMatchingPlayer),
		ERROR_STRING(kNetErrInviteTooManyHoods),
		ERROR_STRING(kNetErrNeedToPay),
		ERROR_STRING(kNetErrServerBusy),
		ERROR_STRING(kNetErrVaultNodeAccessViolation),
    };
    COMPILER_ASSERT(arrsize(s_errors) == kNumNetErrors);
    
    if ((unsigned)code >= arrsize(s_errors)) {
        if (code == kNetPending)
            return ERROR_STRING(kNetPending);
        return L"ErrUnknown";
    }
    #undef ERROR_STRING
    
    return s_errors[code];
}
