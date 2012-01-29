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
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/TicTacToe/pfGmTicTacToe.cpp
*   
***/

#define USES_GAME_TICTACTOE
#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

struct ITicTacToe {
    pfGmTicTacToe * gameCli;
    char            board[3][3];
    char            myself;
    char            other;
    
    ITicTacToe (pfGmTicTacToe * gameCli);

    // pfGameCli event notification handlers
    void Recv           (GameMsgHeader * msg, void * param);
    void OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg);
    void OnPlayerLeft   (const Srv2Cli_Game_PlayerLeft & msg);
    void OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg);
    void OnOwnerChange  (const Srv2Cli_Game_OwnerChange & msg);

    // TicTacToe network message handlers   
    void RecvGameStarted    (const Srv2Cli_TTT_GameStarted & msg, void * param);
    void RecvGameOver       (const Srv2Cli_TTT_GameOver & msg, void * param);
    void RecvMoveMade       (const Srv2Cli_TTT_MoveMade & msg, void * param);
};


/*****************************************************************************
*
*   Factory functions
*
***/

//============================================================================
static pfGameCli * TicTacToeFactory (
    unsigned    gameId,
    plKey       receiver
) {
    return NEWZERO(pfGmTicTacToe)(gameId, receiver);
}

//============================================================================
AUTO_INIT_FUNC(RegisterTicTacToeFactory) {

    static GameTypeReg reg = {
        TicTacToeFactory,
        kGameTypeId_TicTacToe,
        L"Tic-Tac-Toe"
    };

    GameMgrRegisterGameType(reg);
}


/*****************************************************************************
*
*   ITicTacToe
*
***/

//============================================================================
ITicTacToe::ITicTacToe (pfGmTicTacToe * gameCli)
:   gameCli(gameCli)
{
    // Fill the board with space chars
    memset(board, ' ', sizeof(board));
}

//============================================================================
void ITicTacToe::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void ITicTacToe::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void ITicTacToe::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {
    
    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void ITicTacToe::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void ITicTacToe::RecvGameStarted (const Srv2Cli_TTT_GameStarted & msg, void * param) {
    // player that goes first is shown as X's.
    if (msg.yourTurn) {
        myself  = 'X';
        other   = 'O';
    }
    else {
        myself  = 'O';
        other   = 'X';
    }

    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());
}

//============================================================================
void ITicTacToe::RecvGameOver (const Srv2Cli_TTT_GameOver & msg, void * param) {
    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());

    delete gameCli;   // we're done
}

//============================================================================
void ITicTacToe::RecvMoveMade (const Srv2Cli_TTT_MoveMade & msg, void * param) {
    // Update the board with the appropriate piece  
    if (msg.playerId == NetCommGetPlayer()->playerInt)
        board[msg.row][msg.col] = myself;
    else
        board[msg.row][msg.col] = other;

    pfGameCliMsg * gameCliMsg = NEWZERO(pfGameCliMsg);
    gameCliMsg->Set(gameCli, msg);
    gameCliMsg->Send(gameCli->GetReceiver());
}
                                                                

/*****************************************************************************
*
*   pfGmTicTacToe
*
***/

//============================================================================
pfGmTicTacToe::pfGmTicTacToe (
    unsigned    gameId,
    plKey       receiver
)
:   pfGameCli(gameId, receiver)
{
    internal = NEWZERO(ITicTacToe)(this);
}

//============================================================================
pfGmTicTacToe::~pfGmTicTacToe () {

    delete internal;
}

//============================================================================
void pfGmTicTacToe::Recv (GameMsgHeader * msg, void * param) {

    #define DISPATCH(a) case kSrv2Cli_TTT_##a: {                    \
        const Srv2Cli_TTT_##a & m = *(const Srv2Cli_TTT_##a *)msg;  \
        internal->Recv##a(m, param);                                \
    }                                                               \
    break;                                                          //
    switch (msg->messageId) {
        DISPATCH(GameStarted);
        DISPATCH(GameOver);
        DISPATCH(MoveMade);
        DEFAULT_FATAL(msg->messageId);
    }
    #undef DISPATCH
}

//============================================================================
void pfGmTicTacToe::OnPlayerJoined (const Srv2Cli_Game_PlayerJoined & msg) {

    internal->OnPlayerJoined(msg);
}

//============================================================================
void pfGmTicTacToe::OnPlayerLeft (const Srv2Cli_Game_PlayerLeft & msg) {

    internal->OnPlayerLeft(msg);
}

//============================================================================
void pfGmTicTacToe::OnInviteFailed (const Srv2Cli_Game_InviteFailed & msg) {

    internal->OnInviteFailed(msg);
}

//============================================================================
void pfGmTicTacToe::OnOwnerChange (const Srv2Cli_Game_OwnerChange & msg) {

    internal->OnOwnerChange(msg);
}

//============================================================================
void pfGmTicTacToe::MakeMove (unsigned row, unsigned col) {

    Cli2Srv_TTT_MakeMove msg;
    msg.messageId       = kCli2Srv_TTT_MakeMove;
    msg.messageBytes    = sizeof(msg);
    msg.recvGameId      = GetGameId();  // send to GameSrv on server
    msg.transId         = 0;
    msg.row             = (uint8_t)row;
    msg.col             = (uint8_t)col;
    
    GameMgrSend(&msg);
}

//============================================================================
void pfGmTicTacToe::ShowBoard () {

    // Technically, we should stuff the board into a plMessage and
    // have our receiver handle how the board is shown, but heck,
    // this is just a little demo and not quite worth the effort.

#if 0 // Max doesn't have the console, and can't link with it anyway, so I'm removing this code since "this is just a little demo and not quite worth the effort"
    pfConsole::AddLine ("\n");
    pfConsole::AddLineF("\\i %c | %c | %c", internal->board[0][0], internal->board[0][1], internal->board[0][2]);
    pfConsole::AddLine ("\\i---+---+---");
    pfConsole::AddLineF("\\i %c | %c | %c", internal->board[1][0], internal->board[1][1], internal->board[1][2]);
    pfConsole::AddLine ("\\i---+---+---");
    pfConsole::AddLineF("\\i %c | %c | %c", internal->board[2][0], internal->board[2][1], internal->board[2][2]);
    pfConsole::AddLine ("\n");
#endif // 0
}
