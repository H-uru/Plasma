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
xInviteVisitTitle = "Einladung %s zu besuchen"
xInviteVisitBody = "Sie sind herzlich eingeladen, die Welt %s zu besuchen, indem sie sich zur MT Nexus-Station begeben ...\n\nGruß,\n%s (Eigentümer)\n"
xRevokeVisitorTitle = "Einladung an %s zurückgezogen"
xRevokeVisitorBody = "Ihre Besucherrechte für die Welt %s gelten nicht mehr.\n\n<Tut mir leid.>\n\nGruß,\n%s (Eigentümer)"
#--- KI configuration strings
xKIConfiguration = "KI-Einstellungen"
xVolumeConfiguration = "Lautstärke-Einstellungen"
xOwnerConfiguration = "%s Einstellungen"
xOwnerVistors = "%s Besucher"
xOwnerCoOwners = "%s Eigentümer"
xDevicesFolderName = "Geräte"
#--- KI add/create entry
#xCreateBuddyTitle = "<Freund über ID hinzufügen>"
#--- KI leave messages
xLeaveGameMessageNormal = "Uru wirklich verlassen?"
xLeaveGameMessageNano = xLeaveGameMessageNormal
xLeaveGameMessageMicro = xLeaveGameMessageNormal
#xLeaveGameMessageNormal = "Bitte gehen Sie nicht! Wollen Sie Uru wirklich verlassen?"
#xLeaveGameMessageNano = "Sie haben noch nicht viel erreicht. Wollen Sie Uru wirklich verlassen?"
#xLeaveGameMessageMicro = "Es gibt noch so viel zu entdecken! Wollen Sie Uru wirklich verlassen?"
#--- send to something error messages
xSendToErrorMessage1 = "Senden nicht möglich"
xSendToErrorMessage2 = "Spieler nicht gefunden"
xSendToErrorMessage3 = "Unbekannte Spielerart"
xSendToErrorMessage4 = "Ungültiges Journalelement"
xSendToErrorMessage5 = "Darf nur Text enthalten"
xCommandErrorMessage1 = "'%s' nicht möglich - unbekannter Befehl"
xKITimeBroke = "<Verbindung gestört>"
#--- delete messages
xDeletePictureAsk = '"%s" wirklich löschen?'
xDeleteJournalAsk = '"%s" wirklich löschen?'
xDeletePlayerAsk = '"%s" wirklich aus Ordner "%s" löschen?'
#--- KI is full error messages
xKIFullImagesError = "Ihre KI kann keine weiteren Bilder im Journal speichern. Sie ist augelastet."
xKIFullNotesError = "Ihre KI kann keine weiteren Textnotizen im Journal speichern. Sie ist ausgelastet."
xKIFullMarkersError = "Ihre KI kann keine weiteren Marker im Journal speichern. Sie ist augelastet."
#--- CCR conversation
xCCRConversationStarted = '(Konversation begonnen)'
xCCRConversationEnded = '(Konversation beendet)'
xCCRNoCCRInContact = '(Kein Kontakt mit Kundenbetreuung, Nachricht nicht gesendet)'
xCCRPetitionSent = '(%s gesendet) %s'
#--- KI - Chat strings
xChatNoOneToReply = '(Es gibt niemand, dem man antworten könnte.)'
xChatLeftTheAge = '(%s hat die Welt verlassen)'
xChatLeftTheGame = '(%s hat das Spiel verlassen)'
xChatWentOffline = '(%s ist offline und nicht für einen Chat verfügbar.)'
xChatCannotFindBuddy = "('%s' in keiner Spielerliste gefunden.)"
xChatBroadcastMsgRecvd = ""
xChatPrivateMsgRecvd = "Von "
xChatInterAgeMsgRecvd = "Von "
xChatInterAgePlayerRecvd = "%s in %s"
xChatBroadcastSendTo = ""
xChatPrivateSendTo = "An "
xChatInterAgeSendTo = "An "
xChatTOPrompt = "AN:"
xChatAllAgeCommand = "/schreien"
xChatClearAll = "/chatlöschen"
xChatPrivateCommand = "/p"
xChatNeighborsCommand = "/nachbarn"
xChatBuddiesCommand = "/freunde"
xChatNoOneListening = "(Sie sind zu weit weg. Vielleicht sollten Sie schreien.)"
xChatInterAgeNotAvailable = "(Welten-Umschalter nicht verfügbar)"
xChatReplyCommand = "/antworten"
xChatStartLogCommand = "/protokollan"
xChatStopLogCommand = "/protokollaus"
xChatLogStarted = "Chat.log aktiviert..."
xChatLogStopped = "...Chat.log gestoppt."
xChatPetitionCommands = {   "/petition" : PtCCRPetitionType.kGeneralHelp,
                            "/hilfe" : PtCCRPetitionType.kGeneralHelp,
                            "/bug" : PtCCRPetitionType.kBug,
                            "/feedback" : PtCCRPetitionType.kFeedback,
                            "/exploit" : PtCCRPetitionType.kExploit,
                            "/belästigung" : PtCCRPetitionType.kHarass,
                            "/spielproblem" : PtCCRPetitionType.kStuck,
                            "/technisch" : PtCCRPetitionType.kTechnical
                        }
xChatCCRPetitionTitle = "Chat-Petition"
xChatCCRCommand = "/ccr"
xChatCCRMsgRecvd = 'Von Kundenbetreuung:'
xChatCCRSendTo = 'An Kundenbetreuung:'
xChatErrorMsgRecvd = 'Fehler:'
xChatCCRFromPlayer = 'Von %d an Kundenbetreuung:'
xChatWeeBeeAFK = " (Ich bin an der Oberfläche, komme gleich zurück)"
xCCRHelpPopupMenu = [   ("Bug-Report",PtCCRPetitionType.kBug),
                        ("Feedback und Vorschläge",PtCCRPetitionType.kFeedback),
                        ("Ausnutzen von Programmfehlern und Schummeln",PtCCRPetitionType.kExploit),
                        ("Belästigung und andere Benehmensfragen",PtCCRPetitionType.kHarass),
                        ("Probleme mit der Spiellösung",PtCCRPetitionType.kStuck),
                        ("Technische Probleme",PtCCRPetitionType.kTechnical),
                        ("Allgemeine Hilfe",PtCCRPetitionType.kGeneralHelp)
                    ]
xCCRHelpPopupDefault = 6
#--- Offer link to
xOfferLinkToMessage = 'Euch wurde eine Verbindung nach "%s" angeboten. Wollt Ihr sie benutzen?'
#--- KI - neighborhood strings
xAgeOwnedStatusLine = "%d Eigentümer%s mit %d Besucher%s."
xPorPAgeOwnedStatusLine = "%d Eigentümer%s mit %d Besucher%s. Und ist %s."
xNeighborhoodBottomLine = "%s von %s"
xNeighborhoodNone = "Keine Mitgliedschaft in einer Nachbarschaft"
xNeighborhoodNoName = "<kein Name>"
xNeighborhoodMakePorP = "Als %s markieren"
# --- Player expanded strings
#xPlayerEnterID = "Spieler-ID eingeben:"
#xPlayerNumberOnly = "Bitte geben Sie nur eine Nummer ein."
#xPlayerNotYourself = "Sie können sich nicht selbst auswählen."
xPlayerInCleft = "Ist online und hat sich in der Spalte verirrt."
xPlayerInCloset = "Ist online und wechselt die Kleidung."
xPlayerInAge = "Ist online und erforscht die Welt %s."
xPlayerOffline = "Ist offline."
#---- KI Journal strings
xJournalInitialMessage = "<Text eingeben>"
xJournalInitialTitle = "<Titel eingeben>"
# --- KI IMage strings
xImageInitialTitle = "<Kommentar eingeben>"

xFolderVisLists = "Welt-Besucherliste:"
xFolderOwnLists = "Welt-Eigentümerliste:"

# --- Marker Game strings
xMarkerFolderPopupMenu = [   ("1 Min.",60),
                        ("2 Min.",120),
                        ("5 Min.",300),
                        ("10 Min.",600),
                    ]
xChatMarkerTOAllTeams = "An: Alle Teams >"
xChatMarkerTOGreenTeam = "AN: Grünes Team >"
xChatMarkerTORedTeam = "AN: Rotes Team >"
xChatMarkerAllTeams = "Alle Teams"
xChatMarkerGreenTeam = "Grünes Team"
xChatMarkerRedTeam = "Rotes Team"
xMarkerGamePrematureEnding = "Der Spielleiter hat das Spiel beendet!"
xMarkerGameCaptureGame = "Erobern-Spiel"
xMarkerGameHoldGame = "Halten-Spiel"
xMarkerGameQuestGame = "Aufgaben-Spiel"
xMarkerGameBegins = "Das Spiel beginnt!"
xMarkerGameGreenTeamWins = "Das grüne Team gewinnt! %d zu %d"
xMarkerGameTieGame = "Unentschieden: %d zu %d"
xMarkerGameRedTeamWins = "Das rote Team gewinnt! %d zu %d"
xMarkerGameEnded = "Spielende... %s"
xMarkerGameResults = "Ergebnis:"
xMarkerGameNoMarkers = "Keine Marker"
xMarkerGameOneMarker = "Ein Marker"
xMarkerGameNMarkers = "%d Marker"
xMarkerGameCaptured = "erobert"
xMarkerGameFoundMarker = "Marker gefunden '%s'."
xMarkerGameLastMarker = "Und das war der letzte Marker."
xMarkerGameOneMoreLeft = "Nur noch EIN Marker!"
xMarkerGameCaptures = "%s erobert '%s'. %s"

xMarkerGameEditButton = "Spiel bearbeiten"
xMarkerGamePlayButton = "Spiel beginnen"
xMarkerGameDoneEditButton = "Bearbeiten beenden"
xMarkerGameAddMarkerButton = "Marker hinzufügen"
xMarkerGameMarkerListButton = "Markerliste"
xMarkerGameRemoveMarkerButton = "Marker entfernen"
xMarkerGameGoBackButton = "Zurück"
xMarkerGameInviteButton = "Spieler einladen"
xMarkerGameStartGameButton = "Spiel starten"
xMarkerGameEndGameButton = "Spiel beenden"
xMarkerGameStopPlayingButton = "Spiel abbrechen"
xMarkerGameResetGameButton = "Spiel zurücksetzen"
xMarkerGameStatusNoMarkers = "Es gibt keine Marker"
xMarkerGameStatusOneMarker = "Es gibt einen Marker"
xMarkerGameStatusNMarkers = "Es gibt %s Marker"
xMarkerGameStatusIn = " in %s"
xMarkerGameStatusAllFound = "Alle Aufgaben-Marker wurden gefunden."
xMarkerGameStatusNotAllFound = "Nicht alle Aufgaben-Marker wurden gefunden."
#--- question on invite to marker game
xWaitingForStartText = "Warte auf Start"
xTimeRemainingText = "Verbleibende Zeit: %01d:%02d"
xMarkerGameMarkersRemaining = "Verbleibende Marker: %d"
xMarkerGameMarkersUnclaimed = "Freie Marker: %d"
xMarkerGameGreenTeamScore = "Grünes Team(%d)"
xMarkerGameRedTeamScore = "Rotes Team(%d)"
xMarkerGameNameCapture = "Erobern"
xMarkerGameInstructCapNoMarker = "Da es in diesen Spiel keine Marker gibt, haben alle gewonnen oder verloren!"
xMarkerGameInstructCapOneMarker = "Das Team, das den einzelnen Marker erobert ehe %d Minute%s um sind, gewinnt!"
xMarkerGameInstructCapNMarkers = "Das Team, das in %d Minute%s die meisten Marker erobert, gewinnt!"
xMarkerGameNameHold = "Halten"
xMarkerGameInstructHoldNoMarker = "Da es in diesen Spiel keine Marker gibt, haben alle gewonnen oder verloren!"
xMarkerGameInstructHoldNMarkers = "Das Team, das nach %d Minute%s die meisten Marker erobert hat und hält, gewinnt!"
xMarkerGameNameQuest = "Aufgabe"
xMarkerGameInstructQuest = "Einladung nicht möglich - dies ist ein Einzelspieler-Spiel"
xMarkerGameNameUnknown = "Unbekannte Art"
xMarkerGameQTitle = "Marker-Spiel von %s anschließen"
xMarkerGameQMessage = "    %s hat Sie zu einer Runde '%s' eingeladen.\n    %s\n\nWollen Sie mitspielen?"
xMarkerGameInviteRecvd = "Einladung zu einem Spiel erhalten. Sehen Sie Ihre Nachrichten durch."

# --- Yes/No Dialog
xYesNoYESbutton = "Ja"
xYesNoOKbutton = "Ok"
xYesNoAcceptButton = "Annehmen"
xYesNoDeclineButton = "Ablehnen"
xYesNoQuitbutton = "Abbrechen"
xYesNoNoButton = "Nein"

# ---- Option Menu strings
xOptMenuKeyMap = "Tastatur"
xOptMenuGameSettings = "Spieleinstellungen"
xOptMenuURULive = "URU Live"
xOptMenuHelp = "Hilfe"
xOptMenuCredits = "URU Credits"
xOptMenuQuit = "URU verlassen"
xOptMenuOk = "Spiel fortsetzen"
xOptMenuCancel = "Abbrechen"

xMoveForward = "Vorwärts"
xMoveBackward = "Rückwärts"
xRotateLeft = "Nach links drehen"
xRotateRight = "Nach rechts drehen"
xJump = "Springen"
xExitMode = "Modus verlassen"
xPushToTalk = "Drücken zum Sprechen"


# --- OK Dialog
# The following is all NEW, don't translate the \n followed by a number (not that it would translate)
xOKDialogDict = { # "code": "translation"
        "": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#01",
        "TERMINATED: Server LogOff. Reason: Logged In Elsewhere": "Ihre Verbindung wurde getrennt, da Ihr Konto bereits benutzt wird.\n#02",
        "TERMINATED: Server LogOff. Reason: Timed Out": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#03",
        "TERMINATED: Server LogOff. Reason: Not Authenticated": "Bei der Verbindungsherstellung ist ein Problem aufgetreten. Bitte überprüfen Sie Kontonamen und Passwort und versuchen Sie es erneut.\n#04",
        "TERMINATED: Server LogOff. Reason: Kicked Off": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#05",
        "TERMINATED: Server LogOff. Reason: Unknown Reason": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#06",
        "TERMINATED: Server LogOff. Reason: CCRs must use a protected lobby": "Ihre Verbindung wurde getrennt, da Kundenbetreuer eine geschützte Lobby verwenden müssen.\n#07",
        "TERMINATED: Server LogOff. Reason: CCRs must have internal client code": " Ihre Verbindung wurde getrennt, da Kundenbetreuer einen internen Kundencode verwenden müssen.\n#08",
        "TERMINATED: Server LogOff. Reason: UNKNOWN REASON CODE": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut..\n#09",
        "SERVER SILENCE": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#10",
        "BAD VERSION": "Dies ist eine alte Uru-Version. Bitte aktualisieren Sie Ihre Version.\n#11",
	    "Player Disabled": "Der von Ihnen gewählte charakter ist ungültig. Bitte wenden Sie sich an den Kundendienst.\n#12",
	    "CAN'T FIND AGE": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#13",
	    "AUTH RESPONSE FAILED": "Bei der Verbindungsherstellung ist ein Problem aufgetreten. Bitte überprüfen Sie Kontonamen und Passwort und versuchen Sie es erneut.\n#14",
        "AUTH TIMEOUT": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#15",
        "SDL Desc Problem": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#16",
        "Unspecified error": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#17",
		"Failed to send msg": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#18",
		"Authentication timed out": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#19",
		"Peer timed out": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#20",
		"Server silence": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#21",
		"Protocol version mismatch": "Dies ist eine alte Uru-Version. Bitte aktualisieren Sie Ihre Version.\n#22",
		"Auth failed": "Bei der Verbindungsherstellung ist ein Problem aufgetreten. Bitte überprüfen Sie Kontonamen und Passwort und versuchen Sie es erneut.\n#23",
		"Failed to create player": "Bei der Erstellung Ihres Spielers ist ein Fehler aufgetreten. Bitte versuchen Sie es erneut.\n#24",
		"Invalid error code": "Es scheint ein Problem mit der Verbindung vorzuliegen. Bitte versuchen Sie es erneut.\n#25",
		"linking banned": "Ihre Buch-Verbindungen wurden deaktiviert\n#26",
		"linking restored": "Ihre Buch-Verbindungen wurden wiederhergestellt\n#27",
		"silenced": "Ihre Chat-Funktion wurde deaktiviert\n#28",
		"unsilenced": "Ihre Chat-Funktion wurde wiederhergestellt\n#29"
}


# --- xInvite strings
xInviteKeyAdded = "Einladnugsschlüssel hinzugefügt: %s"
xMaxInvites = "Maximale Zahl von Einladungen erreicht"
xMissingInviteFolder = "Einladungsordner nicht vorhanden"
xInviteUsage = "Syntax: /einladung <Einladungsschlüssel>"
xInviteAccepted = "Einladung von Freund: %s mit Schlüssel: %s akzeptieren?"
xAcceptUsage = "Syntax: /annehmen <Name des Freundes> <Schlüssel>"
xCouldNotCast = "Listenelement konnte nicht verarbeitet werden"
xKeys = "Schlüssel: "
xRemoveNodeFailed = "Entfernen von Node gescheitert"
xInviteNotFound = "Einladung nicht gefunden"
xUninviteUsage = "Syntax: /einladunglöschen <Einladungsschlüssel>"
xDeletedInvitation = "Gelöschte Einladung: "

# --- xKIExtChatCommands strings
xSitCmd = "sit"
xAfkCmd = "afk"
xInviteCmd = "einladung"
xUninviteCmd = "einladunglöschen"
xAcceptCmd = "annehmen"
xShowInvitesCmd = "einladungsliste"
xWaveCmd = "winken"
xWaveString = "%s winkt"
xSneezeCmd = "niesen"
xSneezeString = "%s niest"
xClapCmd = "klatschen"
xClapString = "%s klatscht"
xLaughCmd = "lachen"
xLaughString = "%s lacht"
xLOLCmd = "lol"
xLOLString = "%s beginnt zu laut lachen"
xROTFLCmd = "rotfl"
xROTFLString = "%s lacht schallend"
xDanceCmd = "tanzen"
xDanceString = "%s tanzt"
xYesCmd = "ja"
xYesString = "%s nickt"
xNoCmd = "nein"
xNoString = "%s schüttelt den Kopf"
xYawnCmd = "gähnen"
xYawnString = "%s gähnt"
xCheerCmd = "jubeln"
xCheerString = "%s jubelt"
xThanksCmd = "vielendank"
xThanksString = "%s dankt Ihnen herzlich!"
xThxCmd = "danke"
xThxString = "%s dankt Ihnen"
xCryCmd = "traurig"
xCryString = "<schnüff> %s ist traurig"
xCriesCmd = "weinen"
xCriesString = "%s weint"
xDontKnowCmd = "ratlos"
xDontKnowString = "%s zuckt mit den Schultern"
xShrugCmd = "schulternzucken"
xShrugString = "%s zuckt mit den Schultern"
xDunnoCmd = "weißnicht"
xDunnoString = "%s zuckt mit den Schultern"
xPointCmd = "punkte"
xPointString = "%s points"

xKISettingsFontSizeText = "Fontgröße:"
xKISettingChatFadeTimeText = "Chatblende:"
xKISettingsOnlyBuddiesText = "Nur private und KI-Nachrichten von Freunden annehmen"
xKIDescriptionText = "Beschreibung:"
xMarkerGameOwnerTitle = "BESITZER:"
xMarkerGameTimeText = "Spielzeit:"
xCCRAwayText = "KB derzeit nicht verfügbar"
xCCRPetitionTypeText = "Petitionsart:"
xCCRSubjectText = "Betreff:"
xCCRCommentText = "Kommentar:"
xCCRSubmitBtnText = "Senden"
xCCRCancelBtnText = "Abbruch"

#latest strings to be translated (build 36)
xKIStatusNexusLinkAdded = "Ein link wurde zu ihrer KI zugeführt."

# updated strings in build .37
xPlayerEnterID = "G:Enter player ID or name:"
xPlayerNumberOnly = "G:Please enter a player ID or current Age player name."
xPlayerNotYourself = "G:Can't be yourself."
#--- KI add/create entry
xCreateBuddyTitle = "<G:add buddy by ID or name if in Age>"

# new strings starting in build .37
xChatAddBuddyCmd = "/addbuddy"
xChatRemoveBuddyCmd = "/removebuddy"
xChatIgnoreCmd = "/ignore"
xChatUnIgnoreCmd = "/unignore"
xPlayerAlreadyAdded = "G:Player has already been added."
xPlayerNotFound = "G:Player not found."
xPlayerAdded = "G:Player added."
xPlayerRemoved = "G:Player removed."
