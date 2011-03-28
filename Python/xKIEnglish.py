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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """
"""
This module contains all the strings that need to localized for the KI
"""

from PlasmaConstants import *


#--- invitation notices (KI)
xInviteVisitTitle = "Invitation to visit %s"
xInviteVisitBody = "Please come visit %s Age by going to the MT Nexus station and using your Private Link.\n\nSigned,\n%s (Owner)\n"
xRevokeVisitorTitle = "Visitation to %s removed"
xRevokeVisitorBody = "Your visitation privileges are no longer valid for %s Age.\n\n<Sorry>\n\nSigned,\n%s (Owner)"
#--- KI configuration strings
xKIConfiguration = "KI Settings"
xVolumeConfiguration = "Volume Settings"
xOwnerConfiguration = "%s Settings"
xOwnerVistors = "%s visitors"
xOwnerCoOwners = "%s owners"
xDevicesFolderName = "Devices"
#--- KI leave messages
xLeaveGameMessageNormal = "Are you sure you want to quit Uru?"
xLeaveGameMessageNano = xLeaveGameMessageNormal
xLeaveGameMessageMicro = xLeaveGameMessageNormal
#xLeaveGameMessageNormal = "Please don't go! Are you sure you want to leave?"
#xLeaveGameMessageNano = "You haven't done very much. Are you sure you want to leave?"
#xLeaveGameMessageMicro = "There is so much you haven't done yet! Are you sure you want to leave?"
#--- send to something error messages
xSendToErrorMessage1 = "Can't send to that"
xSendToErrorMessage2 = "Player not found"
xSendToErrorMessage3 = "Unknown player type"
xSendToErrorMessage4 = "Bad journal element"
xSendToErrorMessage5 = "Must be text only"
xCommandErrorMessage1 = "Don't know how to '%s'"
xKITimeBroke = "<timelink broken>"
#--- delete messages
xDeletePictureAsk = 'Are you sure you want to delete "%s"?'
xDeleteJournalAsk = 'Are you sure you want to delete "%s"?'
xDeletePlayerAsk = 'Are you sure you want to remove "%s" from your "%s" folder?'
#--- KI is full error messages
xKIFullImagesError = "Your KI cannot store anymore images in your journal, it's full."
xKIFullNotesError = "Your KI cannot store anymore text notes in your journal, it's full."
xKIFullMarkersError = "Your KI cannot store anymore markers in your journal, it's full."
#--- CCR conversation
xCCRConversationStarted = '(conversation started)'
xCCRConversationEnded = '(conversation ended)'
xCCRNoCCRInContact = '(No CCR in contact, message not sent)'
xCCRPetitionSent = '(%s sent) %s'
#--- KI - Chat strings
xChatNoOneToReply = '(There is no one to reply to.)'
xChatLeftTheAge = '(%s has left the Age)'
xChatLeftTheGame = '(%s has left the Game)'
xChatWentOffline = '(%s is offline and cannot be reached for chatting.)'
xChatCannotFindBuddy = "(Can't find '%s' in any of the player lists.)"
xChatBroadcastMsgRecvd = ""
xChatPrivateMsgRecvd = "From "
xChatInterAgeMsgRecvd = "From "
xChatInterAgePlayerRecvd = "%s in %s"
xChatBroadcastSendTo = ""
xChatPrivateSendTo = "To "
xChatInterAgeSendTo = "To "
xChatTOPrompt = "TO:"
xChatAllAgeCommand = "/shout"
xChatClearAll = "/clearchat"
xChatPrivateCommand = "/p"
xChatNeighborsCommand = "/neighbors"
xChatBuddiesCommand = "/buddies"
xChatNoOneListening = "(You're standing too far away, perhaps if you shout?)"
xChatInterAgeNotAvailable = "(Inter-Age switcher not available)"
xChatReplyCommand = "/reply"
xChatStartLogCommand = "/startlog"
xChatStopLogCommand = "/stoplog"
xChatLogStarted = "Chat.log started..."
xChatLogStopped = "...Chat.log stopped."
xChatPetitionCommands = {   "/petition" : PtCCRPetitionType.kGeneralHelp,
                            "/generalhelp" : PtCCRPetitionType.kGeneralHelp,
                            "/bug" : PtCCRPetitionType.kBug,
                            "/feedback" : PtCCRPetitionType.kFeedback,
                            "/exploit" : PtCCRPetitionType.kExploit,
                            "/harass" : PtCCRPetitionType.kHarass,
                            "/stuck" : PtCCRPetitionType.kStuck,
                            "/technical" : PtCCRPetitionType.kTechnical
                        }
xChatCCRPetitionTitle = "Chat line petition"
xChatCCRCommand = "/ccr"
xChatCCRMsgRecvd = 'From CCR:'
xChatCCRSendTo = 'To CCR:'
xChatErrorMsgRecvd = 'Error:'
xChatCCRFromPlayer = 'From %d to CCR:'
xChatWeeBeeAFK = " (I'm on the surface, be back in a minute)"
xCCRHelpPopupMenu = [   ("Bug Report",PtCCRPetitionType.kBug),
                        ("Feedback and Suggestions",PtCCRPetitionType.kFeedback),
                        ("Exploits and Cheating Issues",PtCCRPetitionType.kExploit),
                        ("Harassment and Code of Conduct Issues",PtCCRPetitionType.kHarass),
                        ("Stuck",PtCCRPetitionType.kStuck),
                        ("Technical Issues",PtCCRPetitionType.kTechnical),
                        ("General Help",PtCCRPetitionType.kGeneralHelp)
                    ]
xCCRHelpPopupDefault = 6
#--- Offer link to
xOfferLinkToMessage = 'You have been offered a link to "%s" <alright!>. Do you want to go?'
#--- KI - neighborhood strings
xAgeOwnedStatusLine = "%d owner%s with %d visitor%s."
xPorPAgeOwnedStatusLine = "%d owner%s with %d visitor%s. And is %s."
xNeighborhoodBottomLine = "%s of %s"
xNeighborhoodNone = "No membership to any neighborhood"
xNeighborhoodNoName = "<no name>"
xNeighborhoodMakePorP = "Make %s"
# --- Player expanded strings
xPlayerInCleft = "Is online and lost in D'ni-Riltagamin."
xPlayerInCloset = "Is online and changing their clothes."
xPlayerInAge = "Is online and exploring in the %s Age."
xPlayerOffline = "Is offline."
#---- KI Journal strings
xJournalInitialMessage = "<enter message>"
xJournalInitialTitle = "<enter title>"
# --- KI IMage strings
xImageInitialTitle = "<enter caption>"

xFolderVisLists = "Age Visitors Lists:"
xFolderOwnLists = "Age Owners Lists:"

# --- Marker Game strings
xMarkerFolderPopupMenu = [   ("1 min",60),
                        ("2 min",120),
                        ("5 min",300),
                        ("10 min",600),
                    ]
xChatMarkerTOAllTeams = "TO:All Teams >"
xChatMarkerTOGreenTeam = "TO:GreenTeam >"
xChatMarkerTORedTeam = "TO:RedTeam >"
xChatMarkerAllTeams = "All Teams"
xChatMarkerGreenTeam = "Green Team"
xChatMarkerRedTeam = "Red Team"
xMarkerGamePrematureEnding = "Game master ended game!"
xMarkerGameCaptureGame = "Capture game"
xMarkerGameHoldGame = "Hold game"
xMarkerGameQuestGame = "Quest game"
xMarkerGameBegins = "Game begins!"
xMarkerGameGreenTeamWins = "Green team wins! %d to %d"
xMarkerGameTieGame = "Tie game.  %d to %d"
xMarkerGameRedTeamWins = "Red team wins! %d to %d"
xMarkerGameEnded = "Game ended... %s"
xMarkerGameResults = "Game results:"
xMarkerGameNoMarkers = "no markers"
xMarkerGameOneMarker = "one marker"
xMarkerGameNMarkers = "%d markers"
xMarkerGameCaptured = "captured"
xMarkerGameFoundMarker = "Found marker '%s'."
xMarkerGameLastMarker = "And that was the last marker."
xMarkerGameOneMoreLeft = "Only ONE marker left!"
xMarkerGameCaptures = "%s captures '%s'. %s"

xMarkerGameEditButton = "Edit Game"
xMarkerGamePlayButton = "Play Game"
xMarkerGameDoneEditButton = "Done Editing"
xMarkerGameAddMarkerButton = "Add Marker"
xMarkerGameMarkerListButton = "Marker List"
xMarkerGameRemoveMarkerButton = "Remove Marker"
xMarkerGameGoBackButton = "Go back"
xMarkerGameInviteButton = "Invite Player"
xMarkerGameStartGameButton = "Start Game"
xMarkerGameEndGameButton = "End Game"
xMarkerGameStopPlayingButton = "Stop Playing"
xMarkerGameResetGameButton = "Reset Game"
xMarkerGameStatusNoMarkers = "There are no markers"
xMarkerGameStatusOneMarker = "There is one marker"
xMarkerGameStatusNMarkers = "There are %d markers"
xMarkerGameStatusIn = " in %s"
xMarkerGameStatusAllFound = "All of the quest markers have been found."
xMarkerGameStatusNotAllFound = "Not all quest markers have been found."
#--- question on invite to marker game
xWaitingForStartText = "Waiting for start"
xTimeRemainingText = "Time remaining: %01d:%02d"
xMarkerGameMarkersRemaining = "Markers remaining: %d"
xMarkerGameMarkersUnclaimed = "Markers unclaimed: %d"
xMarkerGameGreenTeamScore = "GreenTeam(%d)"
xMarkerGameRedTeamScore = "RedTeam(%d)"
xMarkerGameNameCapture = "capture"
xMarkerGameInstructCapNoMarker = "However, since there are no markers in this game... everyone wins or loses!"
xMarkerGameInstructCapOneMarker = "The team that captures the lone marker before %d minute%s is up...wins!"
xMarkerGameInstructCapNMarkers = "The team that captures the most markers of the %d markers total before %d minute%s is up...wins!"
xMarkerGameNameHold = "hold"
xMarkerGameInstructHoldNoMarker = "However, since there are no markers in this game... everyone wins or loses!"
xMarkerGameInstructHoldNMarkers = "The team that captures and holds the most markers of the %d markers total when %d minute%s is up...wins!"
xMarkerGameNameQuest = "quest"
xMarkerGameInstructQuest = "Can't invite people to play... its a single player game."
xMarkerGameNameUnknown = "unknown style"
xMarkerGameQTitle = "Join %s's MarkerGame"
xMarkerGameQMessage = "    %s would like to invite you to play a round of %s game.\n    %s\n\nWould you like to play?"
xMarkerGameInviteRecvd = "Marker game invite received. Check Incoming."

# --- Yes/No Dialog
xYesNoYESbutton = "Yes"
xYesNoOKbutton = "Ok"
xYesNoAcceptButton = "Accept"
xYesNoDeclineButton = "Decline"
xYesNoQuitbutton = "Quit"
xYesNoNoButton = "No"

# ---- Option Menu strings
xOptMenuKeyMap = "Key Map"
xOptMenuGameSettings = "Game Settings"
xOptMenuURULive = "URU Live"
xOptMenuHelp = "Help"
xOptMenuCredits = "URU Credits"
xOptMenuQuit = "Quit URU"
xOptMenuOk = "Resume Game"
xOptMenuCancel = "Cancel"

xMoveForward = "Move Forward"
xMoveBackward = "Move Backward"
xRotateLeft = "Rotate Left"
xRotateRight = "Rotate Right"
xJump = "Jump"
xExitMode = "Exit Mode"
xPushToTalk = "Push To Talk"


# --- OK Dialog
xOKDialogDict = { # "code": "translation"
        "": "There seems to be a problem with the online connection. Please quit and try again.\n#01",
        "TERMINATED: Server LogOff. Reason: Logged In Elsewhere": "You have been disconnected because someone else is currently using your account.\n#02",
        "TERMINATED: Server LogOff. Reason: Timed Out": "There seems to be a problem with the online connection. Please quit and try again.\n#03",
        "TERMINATED: Server LogOff. Reason: Not Authenticated": "There was a problem connecting. Please verify your Account and Password and try again.\n#04",
        "TERMINATED: Server LogOff. Reason: Kicked Off": "There seems to be a problem with the online connection. Please quit and try again.\n#05",
        "TERMINATED: Server LogOff. Reason: Unknown Reason": "There seems to be a problem with the online connection. Please quit and try again.\n#06",
        "TERMINATED: Server LogOff. Reason: CCRs must use a protected lobby": "You have been kicked off since CCRs must connect thru a protected lobby.\n#07",
        "TERMINATED: Server LogOff. Reason: CCRs must have internal client code": "You have been kicked off since CCRs must have internal client code.\n#08",
        "TERMINATED: Server LogOff. Reason: UNKNOWN REASON CODE": "There seems to be a problem with the online connection. Please quit and try again.\n#09",
        "SERVER SILENCE": "There seems to be a problem with the online connection. Please quit and try again.\n#10",
        "BAD VERSION": "This is an old version of Uru. Please quit and update your game.\n#11",
	    "Player Disabled": "The player you've selected is not allowed into Uru. Please contact customer support for more information.\n#12",
	    "CAN'T FIND AGE": "There seems to be a problem with the online connection. Please quit and try again.\n#13",
	    "AUTH RESPONSE FAILED": "There was a problem connecting. Please verify your Account and Password and try again.\n#14",
        "AUTH TIMEOUT": "There seems to be a problem with the online connection. Please quit and try again.\n#15",
        "SDL Desc Problem": "There seems to be a problem with the online connection. Please quit and try again.\n#16",
        "Unspecified error": "There seems to be a problem with the online connection. Please quit and try again.\n#17",
		"Failed to send msg": "There seems to be a problem with the online connection. Please quit and try again.\n#18",
		"Authentication timed out": "There seems to be a problem with the online connection. Please quit and try again.\n#19",
		"Peer timed out": "There seems to be a problem with the online connection. Please quit and try again.\n#20",
		"Server silence": "There seems to be a problem with the online connection. Please quit and try again.\n#21",
		"Protocol version mismatch": "This is an old version of Uru. Please quit and update your game.\n#22",
		"Auth failed": "There was a problem connecting. Please verify your Account and Password and try again.\n#23",
		"Failed to create player": "There was a problem while creating your player. Please quit and try again.\n#24",
		"Invalid error code": "There seems to be a problem with the online connection. Please quit and try again.\n#25",
		"linking banned": "Your ability to link has been disabled\n#26",
		"linking restored": "Your ability to link has been restored\n#27",
		"silenced": "Your ability to chat has been disabled\n#28",
		"unsilenced": "Your ability to chat has been restored\n#29"
}


# --- xInvite strings
xInviteKeyAdded = "Invite key added: %s"
xMaxInvites = "Maximum invites exceeded"
xMissingInviteFolder = "Invite folder missing"
xInviteUsage = "Usage: /invite <inviteKey>"
xInviteAccepted = "Accepting Invite in progress with Friend: %s and Key: %s"
xAcceptUsage = "Usage: /accept <friendsName> <inviteKey>"
xCouldNotCast = "Couldn't cast list item to note"
xKeys = "Keys: "
xRemoveNodeFailed = "Remove Node Failed"
xInviteNotFound = "Invitation Not Found"
xUninviteUsage = "Usage: /uninvite <inviteKey>"
xDeletedInvitation = "Deleted invitation: "

# --- xKIExtChatCommands strings
xSitCmd = "sit"
xAfkCmd = "afk"
xInviteCmd = "invite"
xUninviteCmd = "uninvite"
xAcceptCmd = "accept"
xShowInvitesCmd = "showinvites"
xWaveCmd = "wave"
xWaveString = "%s waves"
xSneezeCmd = "sneeze"
xSneezeString = "%s sneezes"
xClapCmd = "clap"
xClapString = "%s claps %s hands"
xLaughCmd = "laugh"
xLaughString = "%s laughs"
xLOLCmd = "lol"
xLOLString = "%s starts to laugh"
xROTFLCmd = "rotfl"
xROTFLString = "%s roars with laughter"
xDanceCmd = "dance"
xDanceString = "%s does a dance"
xYesCmd = "yes"
xYesString = "%s nods %s head"
xNoCmd = "no"
xNoString = "%s shakes %s head"
xYawnCmd = "yawn"
xYawnString = "%s yawns"
xCheerCmd = "cheer"
xCheerString = "%s cheers"
xThanksCmd = "thanks"
xThanksString = "%s thanks you very much!"
xThxCmd = "thx"
xThxString = "%s thanks you"
xCryCmd = "cry"
xCryString = "<sniffle> %s is sad"
xCriesCmd = "cries"
xCriesString = "%s cries"
xDontKnowCmd = "dontknow"
xDontKnowString = "%s shrugs"
xShrugCmd = "shrug"
xShrugString = "%s shrugs"
xDunnoCmd = "dunno"
xDunnoString = "%s shrugs"
xPointCmd = "point"
xPointString = "%s points"

xHis = "his"
xHer = "her"

# latest strings to be translated (build 32)
xKISettingsFontSizeText = "Font Size:"
xKISettingChatFadeTimeText = "Chat Fade Time:"
xKISettingsOnlyBuddiesText = "Only accept private messages and KI mail from Buddies"
xKIDescriptionText = "Description:"
xMarkerGameOwnerTitle = "OWNER:"
xMarkerGameTimeText = "Game Time:"
xCCRAwayText = "CCR momentarily away"
xCCRPetitionTypeText = "Petition Type:"
xCCRSubjectText = "Subject:"
xCCRCommentText = "Comment:"
xCCRSubmitBtnText = "Submit"
xCCRCancelBtnText = "Cancel"

#latest strings to be translated (build 36)
xKIStatusNexusLinkAdded = "A link has been added to your Nexus"

# updated strings in build .37
xPlayerEnterID = "Enter player ID or name:"
xPlayerNumberOnly = "Please enter a player ID or current Age player name."
xPlayerNotYourself = "Can't be yourself."
#--- KI add/create entry
xCreateBuddyTitle = "<add buddy by ID or name if in Age>"

# new strings starting in build .37
xChatAddBuddyCmd = "/addbuddy"
xChatRemoveBuddyCmd = "/removebuddy"
xChatIgnoreCmd = "/ignore"
xChatUnIgnoreCmd = "/unignore"
xPlayerAlreadyAdded = "Player has already been added."
xPlayerNotFound = "Player not found."
xPlayerAdded = "Player added."
xPlayerRemoved = "Player removed."
