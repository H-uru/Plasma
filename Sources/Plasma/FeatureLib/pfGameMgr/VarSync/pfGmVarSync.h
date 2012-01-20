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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/VarSync/pfGmVarSync.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_VARSYNC_PFGMVARSYNC_H
#error "Header $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/VarSync/pfGmVarSync.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_VARSYNC_PFGMVARSYNC_H


/*****************************************************************************
*
*   pfGmVarSync interface
*
***/
class pfGmVarSync : public pfGameCli {

    // Encapsulate all implementation details such as member fields
    // in an opaque friend class, in this case that's IVarSync.
    friend struct IVarSync;
    struct IVarSync * internal;

    //========================================================================
    // Required subclass methods
    //--------------------------
    void Recv           (GameMsgHeader * msg, void * param);
    void OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg);
    void OnPlayerLeft   (const Srv2Cli_Game_PlayerLeft & msg);
    void OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg);
    void OnOwnerChange  (const Srv2Cli_Game_OwnerChange & msg);
    //========================================================================

public:
    #pragma warning(push, 0)
    // These macros produce warnings on W4
    CLASSNAME_REGISTER(pfGmVarSync);
    GETINTERFACE_ANY(pfGmVarSync, pfGameCli);
    #pragma warning(pop)
    
    pfGmVarSync (unsigned gameId, plKey receiver);
    ~pfGmVarSync ();
    
    //========================================================================
    // Game methods
    //-------------
    void SetStringVar   (unsigned long id, const wchar_t* val);
    void SetNumericVar  (unsigned long id, double val);
    void RequestAllVars ();
    void CreateStringVar    (const wchar_t* name, const wchar_t* val);
    void CreateNumericVar   (const wchar_t* name, double val);
    //========================================================================
};

