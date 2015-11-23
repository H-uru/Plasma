# -*- coding: utf-8 -*-
""" *==LICENSE==*

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

 *==LICENSE==* """

from Plasma import *
from PlasmaConstants import *
from PlasmaKITypes import *
from PlasmaTypes import *

# detectors
detButtonRock = ptAttribActivatorList(1, "Rock ButtonClick det", netForce=True)
detButtonPaper = ptAttribActivatorList(2, "Paper ButtonClick det", netForce=True)
detButtonScissors = ptAttribActivatorList(3, "Scissors ButtonClick det", netForce=True)

detSitting0 = ptAttribActivator(50, "Player1 Sit det", netForce=True)
detSitting1 = ptAttribActivator(51, "Player2 Sit det", netForce=True)
detSitting2 = ptAttribActivator(52, "Player3 Sit det", netForce=True)
detSitting3 = ptAttribActivator(53, "Player4 Sit det", netForce=True)
detSitting4 = ptAttribActivator(54, "Player5 Sit det", netForce=True)

# Player 1 responders
respRock0 = ptAttribResponder(70, "Rock1 round win/lose resp", ['lose','win'], netForce=True)
respPaper0 = ptAttribResponder(71, "Paper1 round win/lose resp", ['lose','win'], netForce=True)
respScissors0 = ptAttribResponder(72, "Scissors1 round win/lose resp", ['lose','win'], netForce=True)

# Player 2 responders
respRock1 = ptAttribResponder(90, "Rock2 round win/lose resp", ['lose','win'], netForce=True)
respPaper1 = ptAttribResponder(91, "Paper2 round win/lose resp", ['lose','win'], netForce=True)
respScissors1 = ptAttribResponder(92, "Scissors2 round win/lose resp", ['lose','win'], netForce=True)

# Player 3 responders
respRock2 = ptAttribResponder(110, "Rock3 round win/lose resp", ['lose','win'], netForce=True)
respPaper2 = ptAttribResponder(111, "Paper3 round win/lose resp", ['lose','win'], netForce=True)
respScissors2 = ptAttribResponder(112, "Scissors3 round win/lose resp", ['lose','win'], netForce=True)

# Player 4 responders
respRock3 = ptAttribResponder(130, "Rock4 round win/lose resp", ['lose','win'], netForce=True)
respPaper3 = ptAttribResponder(131, "Paper4 round win/lose resp", ['lose','win'], netForce=True)
respScissors3 = ptAttribResponder(132, "Scissors4 round win/lose resp", ['lose','win'], netForce=True)

# Player 5 responders
respRock4 = ptAttribResponder(150, "Rock5 round win/lose resp", ['lose','win'], netForce=True)
respPaper4 = ptAttribResponder(151, "Paper5 round win/lose resp", ['lose','win'], netForce=True)
respScissors4 = ptAttribResponder(152, "Scissors5 round win/lose resp", ['lose','win'], netForce=True)

# Standing cam responder
respStandCam = ptAttribResponder(170, "Standing cam resp", ['0','1','2','3','4'], netForce=True)

# This is the main display responder...
GAME_WIN_STATES = ['rock', 'paper', 'scissors', 'stop']
respCountdown = ptAttribResponder(180, "Countdown Responder", ['countdown','stop','attractmode'], netForce=True)
respGame = ptAttribResponder(73, "Game win/lose resp", GAME_WIN_STATES, netForce=True)

lightAnimsOn = ptAttribAnimation(200, "lights", byObject=True, netForce=True)
clamshellAnim = ptAttribAnimation(201, "clamshells", byObject=True)

# init
initTable   = ptAttribResponder(202, "Init table responder")

# cameras
camera0 = ptAttribSceneobject(203, "seat 1 camera")
camera1 = ptAttribSceneobject(204, "seat 2 camera")
camera2 = ptAttribSceneobject(205, "seat 3 camera")
camera3 = ptAttribSceneobject(206, "seat 4 camera")
camera4 = ptAttribSceneobject(207, "seat 5 camera")

# seat clickables (so we can disable them while someone is sitting on them)
seatButton0 = ptAttribActivator(210, "Seat 1 clickable")
seatButton1 = ptAttribActivator(211, "Seat 2 clickable")
seatButton2 = ptAttribActivator(212, "Seat 3 clickable")
seatButton3 = ptAttribActivator(213, "Seat 4 clickable")
seatButton4 = ptAttribActivator(214, "Seat 5 clickable")

# light sounds (for win and turn on animations)
respSeat0Sounds = ptAttribResponder(220, "Seat 1 sound resp", ['on','win'], netForce=True)
respSeat1Sounds = ptAttribResponder(221, "Seat 2 sound resp", ['on','win'], netForce=True)
respSeat2Sounds = ptAttribResponder(222, "Seat 3 sound resp", ['on','win'], netForce=True)
respSeat3Sounds = ptAttribResponder(223, "Seat 4 sound resp", ['on','win'], netForce=True)
respSeat4Sounds = ptAttribResponder(224, "Seat 5 sound resp", ['on','win'], netForce=True)

# SDL Variable Konstants
SDL_PLAYERS = "intSDLPlayers"
SDL_GAME_STATE = "intSDLState"

SDL_ROCKS = "intSDLPlayerNumRocks"
SDL_PAPERS = "intSDLPlayerNumPaper"
SDL_SCISSORS = "intSDLPlayerNumScissors"
SDL_CUR_SELECTION = "intSDLPlayerCurSelection"
SDL_WINNING_SELECTION = "intSDLGameWinner"

NUM_SEATS = 5

# Game States (logic)
GAME_ATTRACT_PLAYERS = 0
GAME_AWAIT_MOVES = 1
GAME_MOVE_COUNTDOWN = 2
GAME_SHOW_MOVES = 3

# Selections
SEL_NONE = 0
SEL_ROCK = 1 # Beetle
SEL_PAPER = 2 # Book
SEL_SCISSORS = 3 # Pen
SEL_NUM = 4

# Potential win/lose states
ROUND_SCORES = {
    SEL_ROCK: {
        SEL_SCISSORS: 1,
        SEL_NONE: 0,
        SEL_ROCK: 0,
        SEL_PAPER: -1,
    },
    SEL_PAPER: {
        SEL_ROCK: 1,
        SEL_NONE: 0,
        SEL_PAPER: 0,
        SEL_SCISSORS: -1,
    },
    SEL_SCISSORS: {
        SEL_PAPER: 1,
        SEL_NONE: 0,
        SEL_SCISSORS: 0,
        SEL_ROCK: -1,
    },
}

# Selection to round score
SCORE_VAR = {
    SEL_ROCK: SDL_ROCKS,
    SEL_PAPER: SDL_PAPERS,
    SEL_SCISSORS: SDL_SCISSORS,
}

# The name of our Heek Score
SCORE_NAME = "HeekPoints"
SCORE_DEFAULT = 100 # we track wins as points
SCORE_TYPE = PtGameScoreTypes.kAccumulative
SCORE_OP_NONE = 0
SCORE_OP_MINE = 1
SCORE_OP_HIGH = 2

# Imager Note Schtuff
# Unfortunately, there is no translation :(
IMAGER_NAME = "D'ni  Imager Right" # [sic]
IMAGER_NOTE_TITLE = "Neighborhood Heek Scores"
IMAGER_NOTE_PREFIX = "Top 10 Heek Players:\n"
MAX_NUM_HIGH_SCORES = 10

# Python Notifications
NOTIFY_HELLO = 0
NOTIFY_JOINLEAVE = 1
NOTIFY_SCORE_UPDATE = 2

# Rank stuff
RANK_UP = 100

# Delay leaderboard generation in the case of slow tail clients
LEADERBOARD_TIME = 5.0
CB_LEADERBOARD = 0

# Ugh. This is the time at which the clamshell anims are "open"
CLAMSHELL_OPEN = 25.0 / 30.0 # 25 frames

# Even more animation crap
LIGHT_FLASH_START = 10.0
LIGHT_FLASH_END = 25.0
LIGHT_ON_START = 0.0
LIGHT_ON_END = 10.0


class nb01RPSGame(ptResponder, object):
    """NG Ayoheek game logic. This uses normal Plasma facilities, such as SDL instead of the Game Mgr."""

    def _get_game_state(self):
        try:
            return self.SDL[SDL_GAME_STATE][0]
        except IndexError:
            return GAME_ATTRACT_PLAYERS
    def _set_game_state(self, value):
        self.SDL[SDL_GAME_STATE] = (value,)
    game_state = property(_get_game_state, _set_game_state)

    @property
    def num_players(self):
        """Gets the number of folks playing the game."""
        unique = set(self.SDL[SDL_PLAYERS]) - {0}
        return len(unique)

    @property
    def players(self):
        """Gets the folks playing the game."""
        return self.SDL[SDL_PLAYERS]

    @property
    def playing(self):
        """Gets if we are sitting at the table with a KI."""
        return PtGetLocalClientID() in self.SDL[SDL_PLAYERS]

    @property
    def points(self):
        """Gets our current Heek score"""
        if self._score is None:
            return SCORE_DEFAULT
        else:
            return max(0, self._score.getPoints())

    @property
    def seat_id(self):
        """Gets my seat ID or raise a ValueError"""
        return self.SDL[SDL_PLAYERS].index(PtGetLocalClientID())

    @property
    def seats_playing(self):
        """Gets a sequence of seats that are playing the game"""
        return (i for i, ki in enumerate(self.players) if ki != 0)

    @property
    def selections(self):
        """Gets a sequence of tuples (seat, selection)"""
        return ((i, selection) for i, selection in enumerate(self.SDL[SDL_CUR_SELECTION]))

    def _get_winning_selection(self):
        return self.SDL[SDL_WINNING_SELECTION][0]
    def _set_winning_selection(self, value):
        self.SDL[SDL_WINNING_SELECTION] = (value,)
    winning_selection = property(_get_winning_selection, _set_winning_selection)

#########
    def __init__(self):
        super(nb01RPSGame, self).__init__()
        self.id = 20000
        self.version = 10
        PtDebugPrint("nb01RPSGame.__init__():\tversion={}".format(self.version), level=kDebugDumpLevel)

        # This holds my Heek score. This prevents server-spamming.
        self._score = None # will get it from a CB

        # Because I designed the game score retrieval in a sucky way
        self._wantHighScores = False
        self._wantMyScore = False
        self._waitingOnScoreOp = SCORE_OP_NONE

        # Are the game buttons enabled or disabled?
        self._buttons_enabled = False

        # Have we played any rounds in the current game?
        self._round_played = False

        # These are manual (variable event) notify handlers.
        self._event_handlers = {
            NOTIFY_HELLO: self._OnHello,
            NOTIFY_JOINLEAVE: self._OnJoinLeave,
            NOTIFY_SCORE_UPDATE: self._OnGameOver,
        }

        # These get called when an SDL variable touches itself.
        self._sdl_cbs = {
            SDL_GAME_STATE: self._AdvanceGame,
            SDL_PLAYERS: self._IsTheGameAfoot,
        }

    def OnServerInitComplete(self):
        # Notification/logic helpers
        self._MakeAttribHelpers()
        initTable.run(self.key)

        # Request my score. It's OK if it doesn't exist yet. We'll create it when I finish a game
        self._RequestGameScore(mine=True)

        # Subscribe to SDL changes
        def notify_me(sdl, key):
            sdl.sendToClients(key)
            sdl.setFlags(key, True, True)
            sdl.setNotify(self.key, key, 0.0)
        notify_me(self.SDL, SDL_GAME_STATE)
        notify_me(self.SDL, SDL_PLAYERS)

        # If we don't skip the ownership check, folks will be wondering why their clicks do nothing...
        self.SDL.setFlags(SDL_CUR_SELECTION, True, True)
        self.SDL.setFlags(SDL_WINNING_SELECTION, True, True)

        # Make sure we start out sane
        if not PtGetPlayerList():
            self._KillEverything()
            self.SDL[SDL_PLAYERS] = tuple([0] * NUM_SEATS)
            self.game_state = GAME_ATTRACT_PLAYERS
        else:
            self._FFwdLights(SDL_ROCKS, SEL_ROCK)
            self._FFwdLights(SDL_PAPERS, SEL_PAPER)
            self._FFwdLights(SDL_SCISSORS, SEL_SCISSORS)
            for seat in self.seats_playing:
                self._seats[seat].disable()
            if self.game_state != GAME_ATTRACT_PLAYERS:
                for seat in self.seats_playing:
                    self._ChangeButtonState(seat, ff=True)
                respCountdown.run(self.key, "stop", fastforward=True)

#########
    def OnAvatarPage(self, avatar, loading, lastOut):
        if lastOut or loading:
            return

        if self.sceneobject.isLocallyOwned():
            kinum = PtGetClientIDFromAvatarKey(avatar.getKey())
            try:
                seat = self.players.index(kinum)
            except ValueError:
                return
            PtDebugPrint("nb01RPSGame.OnAvatarPage():\tPlayer #{} is leaving during the game!".format(kinum), level=kWarningLevel)
            self._CleanupPosition(seat)
            self.SDL.setIndex(SDL_PLAYERS, seat, 0)

    def OnGameScoreMsg(self, msg):
        if isinstance(msg, ptGameScoreListMsg):
            try:
                scores = msg.getScores()
            except:
                scores = None

            if self._waitingOnScoreOp == SCORE_OP_MINE:
                if scores:
                    self._GotMyScore(scores[0])
                self._wantMyScore = False
            elif self._waitingOnScoreOp == SCORE_OP_HIGH:
                if scores:
                    self._GotHoodHeekScores(scores)
                else:
                    PtDebugPrint("nb01RPSGame.OnGameScoreMsg():\tProblem fetching leaderboard")
                self._wantHighScores = False
            else:
                raise RuntimeError("Invalid score operation: {}".format(self._waitingOnScoreOp))
            self._waitingOnScoreOp = SCORE_OP_NONE
            self._RequestGameScore()
        elif isinstance(msg, ptGameScoreUpdateMsg):
            self._GotMyScore(msg.getScore())

    def OnSDLNotify(self, VARname, SDLname, playerID, tag):
        if VARname in self._sdl_cbs:
            self._sdl_cbs[VARname](playerID)
        else:
            raise RuntimeError("Got an SDL notify for {}, but no CB".format(VARname))

    def OnNotify(self, state, id, events):
        """Handle Plasma Notification Messages"""

        # Clicked on the chair or clicked to get up.
        if self._HandleNotify(state, id, events, self._seats, self._OnWantToSit):
            return

        # Finished sitting down.
        if self._HandleNotify(state, id, events, self._sitting, self._OnSitDown):
            return

        # A rock/paper/sics button was mashed
        if self._HandleNotify(state, id, events, self._buttons, self._OnRPSChoice):
            return

        # The countdown ended. It's time to do stuff...
        if id == respCountdown.id and self.game_state == GAME_MOVE_COUNTDOWN:
            if self.sceneobject.isLocallyOwned():
                respCountdown.run(self.key, "stop")
                self.game_state = GAME_SHOW_MOVES
            return

        # The choice animation finished... Time to reenable the junk.
        if self._HandleNotify(state, id, events, self._allBoardResps, self._OnChoiceAnimFinished):
            return

        # Done spinning the victory. Let them rematch...
        if id == respGame.id:
            if self.sceneobject.isLocallyOwned():
                self.game_state = GAME_AWAIT_MOVES
                self._KillEverything()
            return

        # If we got here, then this might be a variable event. Let's proc those.
        if events:
            if self._HandleVariableNotify(events):
                return

    def OnTimer(self, id):
        if id == CB_LEADERBOARD and self.sceneobject.isLocallyOwned():
            self._UpdateImager()


#########
    def _OnWantToSit(self, state, seat, events):
        """Someone wants to sit down"""
        self._seats[seat].disable()

    def _OnSitDown(self, state, seat, events):
        """We have started the sit/stand animation."""

        # Adjust the cutscene camera as appropriate
        if PtWasLocallyNotified(self.key):
            if state:
                self._cameras[seat].value.pushCutsceneCamera(1, PtGetLocalAvatar().getKey())
            else:
                self._cameras[seat].value.popCutsceneCamera(PtGetLocalAvatar().getKey())

        # Manage game state if standing up
        if state:
            if PtWasLocallyNotified(self.key):
                PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                self._JoinTheGame(seat)
        else:
            self._seats[seat].enable()
            if PtWasLocallyNotified(self.key):
                respStandCam.run(self.key, str(seat))
                PtSendKIMessage(kEnableEntireYeeshaBook, 0)
                self._LeaveTheGame(seat)

    def _OnRPSChoice(self, state, which, events):
        """We picked a thingo!"""
        if PtWasLocallyNotified(self.key):
            self._round_played = True
        if self.sceneobject.isLocallyOwned():
            avatar = PtFindAvatar(events)
            seat = self.players.index(PtGetClientIDFromAvatarKey(avatar.getKey()))
            self.SDL.setIndex(SDL_CUR_SELECTION, seat, which + 1)
            # Waiting on moves? You just got one. COUNTDOWN!
            if self.game_state == GAME_AWAIT_MOVES:
                self.game_state = GAME_MOVE_COUNTDOWN

    def _OnChoiceAnimFinished(self, state, which, events):
        """The choice animation completed. It's time to proceed..."""
        if self.sceneobject.isLocallyOwned():
            if self.winning_selection == SEL_NONE:
                self.game_state = GAME_AWAIT_MOVES
            else:
                respGame.run(self.key, GAME_WIN_STATES[self.winning_selection - 1])


#########
    def _AdvanceGame(self, playerID):
        """Handles game state changes."""

        # Synchronize the clamshell state
        enable_buttons = self.game_state in (GAME_AWAIT_MOVES, GAME_MOVE_COUNTDOWN)
        if self._buttons_enabled != enable_buttons:
            for i in self.seats_playing:
                self._ChangeButtonState(i, enable_buttons)
            self._buttons_enabled = enable_buttons

        # From henceforth, the game master is the only one who does anything
        if not self.sceneobject.isLocallyOwned():
            return

        # Not enough players? Bail. This is not valid anymore...
        elif not self.num_players > 1:
            return

        # If we're waiting for moves, show the attract loop because it flashes once someone starts
        # the actual countdown
        if self.game_state == GAME_AWAIT_MOVES:
            respCountdown.run(self.key, "attractmode")

        # Is it time to count down?
        elif self.game_state == GAME_MOVE_COUNTDOWN:
            respCountdown.run(self.key, "countdown")

        # Time to show da moves?
        elif self.game_state == GAME_SHOW_MOVES:
            self._FinishRound()

    def _FinishRound(self):
        """Determine round winners and display the round selections"""
        # Calculate round scores. Remember this can be 5 on 5, so special scoring abound...
        scores = { i: 0 for i, selection in self.selections if selection != SEL_NONE }
        for seat in scores:
            my_selection = self.SDL[SDL_CUR_SELECTION][seat]
            for their_selection in self.SDL[SDL_CUR_SELECTION]:
                scores[seat] += ROUND_SCORES[my_selection][their_selection]

        # The highest scores are winners -- if it's over 0
        high_score = max(scores.itervalues())
        if high_score > 0:
            winners = [seat for seat, score in scores.viewitems() if score == high_score]
            game_winners = []
        else:
            winners, game_winners = [], []

        PtDebugPrint("nb01RPSGame._FinishRound():\tGetting ready to show moves...", level=kWarningLevel)
        for seat, selection in self.selections:
            if selection == SEL_NONE:
                continue

            win = seat in winners
            state = "win" if win else "lose"
            PtDebugPrint("nb01RPSGame._FinishRound():\t- Position {}: {}".format(seat, state), level=kWarningLevel)
            if win:
                var = SCORE_VAR[selection]
                old_score = self.SDL[var][seat]
                new_score = old_score + high_score
                won_game = new_score > 2
                if won_game:
                    game_winners.append(seat)
                    self.winning_selection = selection
                self._UpdateScoreLights(seat, selection, old_score, new_score, win=won_game)
                self.SDL.setIndex(var, seat, new_score)
            self._boardResps[selection - 1][seat].run(self.key, state)
        PtDebugPrint("nb01RPSGame._FinishRound():\tAnd that's it!", level=kWarningLevel)

        # To prevent weird issues, send out a notify spec'ing which seats won the game. Let them
        # fool around with their scores...
        if game_winners:
            win_points = len(list(self.seats_playing)) - len(game_winners)
            notify = { "type": NOTIFY_SCORE_UPDATE }
            for seat, kinum in enumerate(self.players):
                if kinum != 0:
                    notify[str(kinum)] = win_points if seat in game_winners else -1
            self._SendPyNotifyMsg(notify)

        # Done with that shit. Nuke the selections for sanity.
        self.SDL[SDL_CUR_SELECTION] = tuple([0] * NUM_SEATS)

    def _IsTheGameAfoot(self, playerID):
        """Detects how many players are at the table and starts/kills the game as appropriate."""
        players = self.num_players
        PtDebugPrint("nb01RPSGame._IsTheGameAfoot():\tThere are now {} player(s)".format(players), level=kWarningLevel)
        if players < 2:
            # There's not enough players to continue playing, so we need to murder the game state.
            self._round_played = False
            self._KillEverything()
            self.game_state = GAME_ATTRACT_PLAYERS
        else:
            # We have enough folks to begin playing. If we were previously in attract mode, it's
            # time to enable the board. We'll do this per-player in their SDL callback.
            if self.sceneobject.isLocallyOwned() and self.game_state == GAME_ATTRACT_PLAYERS:
                self.game_state = GAME_AWAIT_MOVES


#########
    def _CleanupPosition(self, seat):
        self._seats[seat].enable()
        if self.sceneobject.isLocallyOwned():
            self.SDL.setIndex(SDL_ROCKS, seat, 0)
            self.SDL.setIndex(SDL_PAPERS, seat, 0)
            self.SDL.setIndex(SDL_SCISSORS, seat, 0)
            self.SDL.setIndex(SDL_CUR_SELECTION, seat, 0)

        for i in xrange(SEL_NUM):
            self._ToggleScoreLight(seat, i, 0, off=True)
            self._ToggleScoreLight(seat, i, 1, off=True)

        if self._buttons_enabled:
            self._ChangeButtonState(seat, enable=False)

    def _FFwdLights(self, sdlvar, rps):
        for i in xrange(NUM_SEATS):
            if self.SDL[sdlvar]:
                count = min(self.SDL[sdlvar][i], 2)
                for j in xrange(count):
                    self._ToggleScoreLight(i, rps, j, ff=True)

    def _OnGameOver(self, **kwargs):
        """The game is over--update our score"""
        if not len(kwargs) > 1:
            PtDebugPrint("nb01RPSGame._OnGameOver(): Received a suspicious game over message. Ignoring...")
            return

        points = kwargs.get(str(PtGetLocalClientID()), None)
        if points is not None:
            PtDebugPrint("nb01RPSGame._OnGameOver():\tAdding {} points to our Heek score".format(points), level=kWarningLevel)
            self._ChangeMyScore(points)
        self._round_played = False

    def _GetRank(self, points):
        if points == 0:
            return 0
        rank = 1
        neededToRankUp = RANK_UP
        while True:
            neededToRankUp += rank * RANK_UP
            if points <= neededToRankUp:
                return rank
            rank += 1

    def _GotMyScore(self, score):
        self._score = score

        # Clearly, something interesting happened. Update the imager.
        if self.sceneobject.isLocallyOwned():
            PtAtTimeCallback(self.key, LEADERBOARD_TIME, CB_LEADERBOARD)

        # If we're playing the game we need to crow about the new score
        if self.playing:
            points = max(0, score.getPoints())
            loc = "Heek.Messages.Rank" if points == 1 else "Heek.Messages.RankPlural"
            rank = self._GetRank(points)
            self._SendLocalStatusMsg(PtGetLocalizedString(loc, [str(rank), str(points)]))

    def _JoinTheGame(self, seat):
        """Joins the local player to the game"""
        # To play heek, you need to have a KI from Gahreesen
        if PtDetermineKILevel() > kMicroKI:
            join = {
                "type": NOTIFY_JOINLEAVE,
                "join": 1,
                "seat": seat,
                "client": PtGetLocalClientID(),
            }
            self._SendPyNotifyMsg(join)

            if self.num_players == 0:
                self._SendLocalStatusMsg(PtGetLocalizedString("Heek.Messages.SinglePlayerWarn"))
            elif self.num_players > 1 and self.game_state != GAME_ATTRACT_PLAYERS:
                self._ChangeButtonState(seat, force=True)
            hello = {
                "type": NOTIFY_HELLO,
                "client": PtGetLocalClientID(),
                "points": self.points,
            }
            self._SendPyNotifyMsg(hello)
        else:
            self._SendLocalStatusMsg(PtGetLocalizedString("Heek.Messages.NoKI"))

    def _KillEverything(self):
        """Resets the game state to the beginning. Note that we do not touch the game mode."""
        self._NukeLights()

        if self.sceneobject.isLocallyOwned():
            respCountdown.run(self.key, "attractmode")
            self.winning_selection = SEL_NONE

            def reset_sdl(sdl, name):
                """Resets a player/seat SDL value to zero."""
                sdl[name] = tuple([0] * NUM_SEATS)
            reset_sdl(self.SDL, SDL_ROCKS)
            reset_sdl(self.SDL, SDL_PAPERS)
            reset_sdl(self.SDL, SDL_SCISSORS)
            reset_sdl(self.SDL, SDL_CUR_SELECTION)

    def _LeaveTheGame(self, seat):
        # To play heek, you need to have a KI from Gahreesen
        if PtDetermineKILevel() > kMicroKI:
            notify = {
                "type": NOTIFY_JOINLEAVE,
                "join": 0,
                "seat": seat,
                "client": PtGetLocalClientID(),
            }
            self._SendPyNotifyMsg(notify)

            # If we're actively playing, and we left before game over, then we deduct a point.
            if self.num_players > 1 and self._round_played:
                PtDebugPrint("nb01RPSGame._LeaveTheGame():\tYou lose because you gave up!")
                self._ChangeMyScore(-1)
            self._round_played = False

    def _OnHello(self, client, points):
        """We have a new player joining us!"""

        if self.playing:
            loc = "Heek.Messages.Welcome" if points == 1 else "Heek.Messages.WelcomePlural"
            rank = self._GetRank(points)
            player = PtGetClientName(PtGetAvatarKeyFromClientID(client))
            self._SendLocalStatusMsg(PtGetLocalizedString(loc, [player, str(rank), str(points)]))

    def _OnJoinLeave(self, seat, client, join):
        """Synchronizes access to the players SDL variable"""
        if self.sceneobject.isLocallyOwned():
            value = client if join else 0
            self.SDL.setIndex(SDL_PLAYERS, seat, value)

        if not join:
            self._CleanupPosition(seat)

    def _SendLocalStatusMsg(self, msg):
        """Sends a status chat message (purple text) to the local player's KI."""
        PtSendKIMessage(kKILocalChatStatusMsg, msg)


#########
    def _ChangeButtonState(self, seat, enable=True, ff=False, force=False):
        clamshell = self._shutters[seat]
        if enable:
            begin, end = 0.0, CLAMSHELL_OPEN
            dCall = "enable"
        else:
            begin, end = CLAMSHELL_OPEN, 0.0
            dCall = "disable"

        # Beware the cleverness!
        for anim in clamshell:
            if ff:
                anim.skipToTime(end)
            else:
                anim.playRange(begin, end)
        if (self.playing and seat == self.seat_id) or force:
            for button in self._buttons:
                getattr(button.value[seat], dCall)()

    def _NukeLights(self):
        for i in self._lights:
            for j in i:
                for k in j:
                    k.playRange(0.0, 0.0)

    def _ToggleScoreLight(self, seat, selection, light, win=False, off=False, ff=False):
        if win:
            start, end = LIGHT_FLASH_START, LIGHT_FLASH_END
        elif off:
            start, end = LIGHT_ON_START, LIGHT_ON_START
        else:
            start, end = LIGHT_ON_START, LIGHT_ON_END
        if ff:
            self._lights[seat][selection - 1][light].playRange(end, end)
        else:
            self._lights[seat][selection - 1][light].playRange(start, end)

    def _UpdateScoreLights(self, seat, selection, old_score, new_score, win):
        PtDebugPrint("nb01RPSGame._UpdateScoreLights():\tOld: {}, New: {}".format(old_score, new_score), level=kWarningLevel)
        start = 0 if win else min(2, old_score)
        end = min(2, new_score)
        for i in xrange(start, end):
            self._ToggleScoreLight(seat, selection, i, win=win)

        state = "win" if win else "on"
        self._sounds[seat].run(self.key, state)


#########
    def _ChangeMyScore(self, points):
        if self._score is None:
            ptGameScore.createPlayerScore(SCORE_NAME, SCORE_TYPE, SCORE_DEFAULT + points, self.key)
        else:
            self._score.addPoints(points, self.key)

    def _FixupScores(self, scores):
        """There can actually be multiple scores of the same name for one player. I blame eap."""
        score_dict = {}
        for score in scores:
            owner = score.getOwnerID()
            try:
                score_dict[owner] += score.getPoints()
            except LookupError:
                score_dict[owner] = score.getPoints()
        return score_dict

    def _FindCreateHeekScoreNote(self):
        vault = ptAgeVault()
        if vault is None:
            PtDebugPrint("nb01RPSGame._FindCreateHeekScoreNote():\tAin't got no age vault!")
            return None
        inbox = vault.getDeviceInbox(IMAGER_NAME)
        if inbox is None:
            # The age is probably initing. It's not a huge loss for us to not create this note yet.
            return None

        for i in inbox.getChildNodeRefList():
            note = i.getChild().upcastToTextNoteNode()
            if note is None:
                continue
            if note.getTitle() == IMAGER_NOTE_TITLE:
                return note
        else:
            note = ptVaultTextNoteNode(0)
            note.setTitle(IMAGER_NOTE_TITLE)
            inbox.addNode(note)
            return note

    def _GetPlayerNameFromPlayerInfoID(self, infoID):
        """Use this if the player is an age owner, but may not be in the age atm"""
        vault = ptAgeVault()
        if vault is None:
            PtDebugPrint("nb01RPSGame._GetPlayerNameFromPlayerInfoID():\tAin't got no age vault!")
            return None
        owners = vault.getAgeInfo().getAgeOwnersFolder()

        # REMEMBER: game scores use the player info ID. Therefore, we have to manually search through
        # all of the child nodes
        for i in owners.getChildNodeRefList():
            info = i.getChild().upcastToPlayerInfoNode()
            if info is None:
                continue
            if infoID == info.getID():
                return info.playerGetName()
        else:
            PtDebugPrint("nb01RPSGame._GetPlayerNameFromPlayerInfoID():\tFailed to find PlayerInfo {}".format(infoID))
            return ""

    def _GotHoodHeekScores(self, scores):
        if not scores:
            PtDebugPrint("nb01RPSGame._GotHoodHeekScores():\tHmmm... No scores. Oh well.")
            return

        note = self._FindCreateHeekScoreNote()
        if note is not None:
            text = IMAGER_NOTE_PREFIX
            num = 1
            fixed_scores = self._FixupScores(scores)
            for owner in sorted(fixed_scores, key=lambda x: fixed_scores[x], reverse=True):
                owner_name = self._GetPlayerNameFromPlayerInfoID(owner)
                if not owner_name:
                    continue
                score = fixed_scores[owner]
                if score < 1:
                    break
                text += "  {} - {} has {} points\n".format(num, owner_name, score)
                num += 1
                if num > MAX_NUM_HIGH_SCORES:
                    break
            note.setText(text)
            note.save()

    def _RequestGameScore(self, mine=None, high=None):
        if mine is not None:
            self._wantMyScore = mine
        if high is not None:
            self._wantHighScores = high

        if self._wantMyScore:
            if self._waitingOnScoreOp == SCORE_OP_NONE:
                self._waitingOnScoreOp = SCORE_OP_MINE
                ptGameScore.findPlayerScores(SCORE_NAME, self.key)
        if self._wantHighScores:
            if self._waitingOnScoreOp == SCORE_OP_NONE:
                self._waitingOnScoreOp = SCORE_OP_HIGH
                ptGameScore.findAgeHighScores(SCORE_NAME, MAX_NUM_HIGH_SCORES * 2, self.key)

    def _UpdateImager(self):
        """Begins the process of updating the Top 10 Heek Scores thingy"""
        self._RequestGameScore(high=True)


#########
    def _MakeAttribHelpers(self):
        # Let's make some tuples of logic objects. We'll use these to figure out which player did the deed.
        self._sitting = self._MakeAttribTuple("detSitting") # you are done sitting down

        self._buttons = (detButtonRock, detButtonPaper, detButtonScissors) # offset -1 from the indices above
        self._rocks = self._MakeAttribTuple("respRock")
        self._papers = self._MakeAttribTuple("respPaper")
        self._scissors = self._MakeAttribTuple("respScissors")
        self._boardResps = (self._rocks, self._papers, self._scissors)
        self._allBoardResps = self._rocks + self._papers + self._scissors

        self._cameras = self._MakeAttribTuple("camera")
        self._seats = self._MakeAttribTuple("seatButton") # you clicked to sit down
        self._sounds = self._MakeAttribTuple("respSeat", "Sounds")

        # These are the various shutters that we have to (ugh) manually animate.
        self._shutters = self._MakeShutterAnimTuple(clamshellAnim)

        # And the terrible indicator lights
        self._lights = self._MakeLightAnimTuple(lightAnimsOn)

    def _MakeAttribTuple(self, prefix, suffix=""):
        """This dirty method makes a tuple of game logic attributes, given a prefix and an
           optional suffix.
        """
        temp = [None] * 5
        for i in xrange(len(temp)):
            temp[i] = globals()["{}{}{}".format(prefix, i, suffix)]
        return tuple(temp)

    def _MakeShutterAnimTuple(self, anim):
        temp = [None] * 5
        for i in xrange(len(temp)):
            seat_shutters = [None] * 3
            for j in xrange(len(seat_shutters)):
                name = "buttonshutter{0}{1}".format(i+1, j+1)
                seat_shutters[j] = anim.byObject[name]
            temp[i] = tuple(seat_shutters)
        return tuple(temp)

    def _MakeLightAnimTuple(self, anim):
        temp = []
        for i in xrange(NUM_SEATS):
            seat_categories = []
            cat_names = ('B', 'G', 'R')
            for j in xrange(SEL_NUM - 1):
                category = []
                for k in xrange(2):
                    light = (i + 1) * 10 + k + 1
                    name = "GTdummy{}Glare{}".format(cat_names[j], light)
                    category.append(anim.byObject[name])
                seat_categories.append(category)
            temp.append(seat_categories)
        return tuple(temp)


#########
    def _HandleNotify(self, state, id, events, attribs, call):
        """If a notify is in attribs, fire off call with the state, seat ID, events"""
        seat = self._WasAttribs(id, attribs)
        if seat != -1:
            call(bool(state), seat, events)
            return True
        else:
            return False

    def _HandleVariableNotify(self, events):
        if not events[0][0] == kVariableEvent:
            # not what we want. go away...
            return

        # let's make a kwargs dict
        args = {}
        for event in events:
            args[event[1]] = event[3]

        # Now, let's fire it off!
        type = args["type"]
        del args["type"]
        if type not in self._event_handlers:
            PtDebugPrint("nb01RPSGame._HandleVariableNotify():\tPyEvent '{}' doesn't have a handler!".format(type))
            return False
        try:
            self._event_handlers[type](**args)
        except TypeError:
            PtDebugPrint("nb01RPSGame._HandleVariableNotify():\tPyEvent '{}' has bad kwargs".format(type))
        return True

    def _SendPyNotifyMsg(self, contents):
        """Sends variable events to everyone. The contents will be unpacked as method arguments."""
        notify = ptNotify(self.key)
        notify.clearReceivers()
        notify.addReceiver(self.key)
        notify.netForce(True)
        notify.netPropagate(True)
        notify.setActivate(True)
        for key, value in contents.iteritems():
            notify.addVarNumber(key, value)
        notify.send() # whoosh... off it goes

    def _WasAttribs(self, id, attribs):
        for i in xrange(len(attribs)):
            if attribs[i].id == id:
                return i
        return -1
