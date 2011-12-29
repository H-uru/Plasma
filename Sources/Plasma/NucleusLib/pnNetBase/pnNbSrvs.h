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

#ifndef pnNbSrvs_inc
#define pnNbSrvs_inc

#include "hsTypes.h"
#include "pnNbConst.h"

/*****************************************************************************
*
*   Server types
*
***/

// These codes may not be changed unless ALL servers and clients are
// simultaneously replaced; so basically forget it =)
enum ESrvType {
    kSrvTypeNone        = 0,

    kSrvTypeClient      = 1,
    kSrvTypeAuth        = 2,
    kSrvTypeGame        = 3,
    kSrvTypeVault       = 4,
    kSrvTypeDb          = 5,
    kSrvTypeMcp         = 6,
    kSrvTypeState       = 7,
    kSrvTypeFile        = 8,
    kSrvTypeLog         = 9,
    kSrvTypeDll         = 10,
    kSrvTypeScore       = 11,
    kSrvTypeCsr         = 12,
    kSrvTypeGateKeeper  = 13,

    kNumSrvTypes,

    // Enforce network message field size
    kNetSrvForceDword = (unsigned)-1
};


/*****************************************************************************
*
*   Front-end server hostnames
*
***/

unsigned GetAuthSrvHostnames (const wchar *** addrs);   // returns addrCount
void SetAuthSrvHostname (const wchar addr[]);

unsigned GetFileSrvHostnames (const wchar *** addrs);   // returns addrCount
void SetFileSrvHostname (const wchar addr[]);

unsigned GetCsrSrvHostnames (const wchar *** addrs);    // returns addrCount
void SetCsrSrvHostname (const wchar addr[]);

unsigned GetGateKeeperSrvHostnames (const wchar *** addrs); // returns addrCount
void SetGateKeeperSrvHostname (const wchar addr[]);

const wchar *GetServerStatusUrl ();
void SetServerStatusUrl (const wchar url[]);

const wchar *GetServerSignupUrl ();
void SetServerSignupUrl (const wchar url[]);

const wchar *GetServerDisplayName ();
void SetServerDisplayName (const wchar name[]);

#endif // pnNbSrvs_inc
