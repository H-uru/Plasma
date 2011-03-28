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
xInviteVisitTitle = "Invito a visitare %s"
xInviteVisitBody = "Vieni a visitare l'Età %s recandoti alla stazione Nexus MT e fai qualcosa...\n\nFirmato,\n%s (Proprietario)\n"
xRevokeVisitorTitle = "Visita a %s annullata"
xRevokeVisitorBody = "I tuoi privilegi di visitatore non sono più validi per l'Età %s.\n\n<Spiacente.>\n\nFirmato,\n%s (Proprietario)"
#--- KI configuration strings
xKIConfiguration = "Impostazioni KI"
xVolumeConfiguration = "Impostazioni Volume"
xOwnerConfiguration = "Impostazioni %s"
xOwnerVistors = "Visitatori %s"
xOwnerCoOwners = "Proprietari %s"
xDevicesFolderName = "Strumenti"
#--- KI add/create entry
#xCreateBuddyTitle = "<aggiungi ID amico>"
#--- KI leave messages
xLeaveGameMessageNormal = "Sei sicuro di voler abbandonare Uru?"
xLeaveGameMessageNano = xLeaveGameMessageNormal
xLeaveGameMessageMicro = xLeaveGameMessageNormal
#xLeaveGameMessageNormal = "Non te ne andare! Sei proprio sicuro di volerlo fare?"
#xLeaveGameMessageNano = "Non hai fatto granchè. Sei sicuro di volertene andare?"
#xLeaveGameMessageMicro = "Ci sono tante cose che non hai ancora fatto! Sei sicuro di volertene andare?"
#--- send to something error messages
xSendToErrorMessage1 = "Non posso spedirlo"
xSendToErrorMessage2 = "Giocatore non trovato"
xSendToErrorMessage3 = "Tipo di giocatore sconosciuto"
xSendToErrorMessage4 = "Elemento di diario errato"
xSendToErrorMessage5 = "Dev'essere solo testo"
xCommandErrorMessage1 = "Non so cosa significhi '%s'"
xKITimeBroke = "<collegamento temporale spezzato>"
#--- delete messages
xDeletePictureAsk = 'Sei sicuro di voler cancellare "%s"?'
xDeleteJournalAsk = 'Sei sicuro di voler cancellare "%s"?'
xDeletePlayerAsk = 'Sei sicuro di voler rimuovere "%s" dalla tua cartella "%s"?'
#--- KI is full error messages
xKIFullImagesError = "Il tuo KI è pieno, non può memorizzare altre immagini nel diario."
xKIFullNotesError = "Il tuo KI è pieno, non può memorizzare altre note nel diario."
xKIFullMarkersError = "Il tuo KI è pieno, non può memorizzare altri marker nel diario."
#--- CCR conversation
xCCRConversationStarted = '(inizio conversazione)'
xCCRConversationEnded = '(fine conversazione)'
xCCRNoCCRInContact = '(Nessun Assistente in contatto, messaggio non inviato)'
xCCRPetitionSent = '(%s inviato) %s'
#--- KI - Chat strings
xChatNoOneToReply = "(Non c'è nessuno con cui parlare.)"
xChatLeftTheAge = "(%s ha abbandonato l'Età)"
xChatLeftTheGame = '(%s ha abbandonato il gioco)'
xChatWentOffline = '(%s è offline e non può essere raggiunto in chat.)'
xChatCannotFindBuddy = "(Impossibile trovare '%s' in alcun elenco giocatori.)"
xChatBroadcastMsgRecvd = ""
xChatPrivateMsgRecvd = "Da "
xChatInterAgeMsgRecvd = "Da "
xChatInterAgePlayerRecvd = "%s in %s"
xChatBroadcastSendTo = ""
xChatPrivateSendTo = "A "
xChatInterAgeSendTo = "A "
xChatTOPrompt = "A:"
xChatAllAgeCommand = "/shout"
xChatClearAll = "/clearchat"
xChatPrivateCommand = "/p"
xChatNeighborsCommand = "/neighbors"
xChatBuddiesCommand = "/buddies"
xChatNoOneListening = "(Sei troppo lontano, forse è meglio gridare.)"
xChatInterAgeNotAvailable = "(Comunicatore Inter-Età non disponibile)"
xChatReplyCommand = "/reply"
xChatStartLogCommand = "/startlog"
xChatStopLogCommand = "/stoplog"
xChatLogStarted = "Chat.log avviato..."
xChatLogStopped = "...Chat.log interrotto."
xChatPetitionCommands = {   "/petition" : PtCCRPetitionType.kGeneralHelp,
                            "/generalhelp" : PtCCRPetitionType.kGeneralHelp,
                            "/bug" : PtCCRPetitionType.kBug,
                            "/feedback" : PtCCRPetitionType.kFeedback,
                            "/exploit" : PtCCRPetitionType.kExploit,
                            "/harass" : PtCCRPetitionType.kHarass,
                            "/stuck" : PtCCRPetitionType.kStuck,
                            "/technical" : PtCCRPetitionType.kTechnical
                        }
xChatCCRPetitionTitle = "Petizione in chat"
xChatCCRCommand = "/ccr"
xChatCCRMsgRecvd = 'Da Assistenza:'
xChatCCRSendTo = 'A Assistenza:'
xChatErrorMsgRecvd = 'Errore:'
xChatCCRFromPlayer = 'Da %d a Assistenza:'
xChatWeeBeeAFK = " (Sono in superficie, torno tra un minuto)"
xCCRHelpPopupMenu = [   ("Rilevamento bug",PtCCRPetitionType.kBug),
                        ("Feedback e suggerimenti",PtCCRPetitionType.kFeedback),
                        ("Problemi di abuso e frode",PtCCRPetitionType.kExploit),
                        ("Problemi di disturbo e codice di condotta",PtCCRPetitionType.kHarass),
                        ("Blocchi",PtCCRPetitionType.kStuck),
                        ("Problemi tecnici",PtCCRPetitionType.kTechnical),
                        ("Help generale",PtCCRPetitionType.kGeneralHelp)
                    ]
xCCRHelpPopupDefault = 6
#--- Offer link to
xOfferLinkToMessage = 'Ti è stato offerto un collegamento a "%s" <evviva!>. Desideri andarci?'
#--- KI - neighborhood strings
xAgeOwnedStatusLine = "%d proprietari%s con %d visitatori%s."
xPorPAgeOwnedStatusLine = "%d proprietari%s con %d visitatori%s. Ed è %s."
xNeighborhoodBottomLine = "%s di %s"
xNeighborhoodNone = "Nessuna affiliazione a quartieri"
xNeighborhoodNoName = "<nessun nome>"
xNeighborhoodMakePorP = "Rendi %s"
# --- Player expanded strings
#xPlayerEnterID = "Inserisci ID giocatore:"
#xPlayerNumberOnly = "Inserisci solo un numero."
#xPlayerNotYourself = "A buddy can't be yourself."       # NEW #
xPlayerInCleft = "E' online ed è perduto nella Gola."
xPlayerInCloset = "E' online e si sta cambiando d'abito."
xPlayerInAge = "E' online e sta esplorando l'Età %s."
xPlayerOffline = "E' offline."
#---- KI Journal strings
xJournalInitialMessage = "<inserisci messaggio>"
xJournalInitialTitle = "<inserisci titolo>"
# --- KI IMage strings
xImageInitialTitle = "<inserisci didascalia>"

xFolderVisLists = "Elenco visitatori Età:"
xFolderOwnLists = "Elenco proprietari Età:"

# --- Marker Game strings
xMarkerFolderPopupMenu = [   ("1 min",60),
                        ("2 min",120),
                        ("5 min",300),
                        ("10 min",600),
                    ]
xChatMarkerTOAllTeams = "A: Tutte le squadre >"
xChatMarkerTOGreenTeam = "A: Squadra Verde >"
xChatMarkerTORedTeam = "A: Squadra Rossa >"
xChatMarkerAllTeams = "Tutte le squadre"
xChatMarkerGreenTeam = "Squadra Verde"
xChatMarkerRedTeam = "Squadra Rossa"
xMarkerGamePrematureEnding = "Il master ha terminato la partita!"
xMarkerGameCaptureGame = "Gioco cattura"
xMarkerGameHoldGame = "Gioco difesa"
xMarkerGameQuestGame = "Gioco ricerca"
xMarkerGameBegins = "La partita comincia!"
xMarkerGameGreenTeamWins = "La Squadra Verde vince! %d a %d"
xMarkerGameTieGame = "Pareggio.  %d a %d"
xMarkerGameRedTeamWins = "La Squadra Rossa vince! %d a %d"
xMarkerGameEnded = "Partita terminata... %s"
xMarkerGameResults = "Risultati:"
xMarkerGameNoMarkers = "nessun marker"
xMarkerGameOneMarker = "un marker"
xMarkerGameNMarkers = "%d marker"
xMarkerGameCaptured = "catturato/i"
xMarkerGameFoundMarker = "Trovato marker '%s'."
xMarkerGameLastMarker = "E questo era l'ultimo marker."
xMarkerGameOneMoreLeft = "Manca solo un marker!"
xMarkerGameCaptures = "%s cattura '%s'. %s"

xMarkerGameEditButton = "Modifica gioco"
xMarkerGamePlayButton = "Gioca"
xMarkerGameDoneEditButton = "Modifiche terminate"
xMarkerGameAddMarkerButton = "Aggiungi marker"
xMarkerGameMarkerListButton = "Elenco marker"
xMarkerGameRemoveMarkerButton = "Togli marker"
xMarkerGameGoBackButton = "Indietro"
xMarkerGameInviteButton = "Invita giocatore"
xMarkerGameStartGameButton = "Inizio partita"
xMarkerGameEndGameButton = "Fine partita"
xMarkerGameStopPlayingButton = "Interrompi partita"
xMarkerGameResetGameButton = "Riavvia partita"
xMarkerGameStatusNoMarkers = "Non ci sono marker"
xMarkerGameStatusOneMarker = "C'è un marker"
xMarkerGameStatusNMarkers = "Ci sono %d marker"
xMarkerGameStatusIn = " in %s"
xMarkerGameStatusAllFound = "Tutti i marker della ricerca sono stati trovati."
xMarkerGameStatusNotAllFound = "Non tutti i marker della ricerca sono stati trovati."
#--- question on invite to marker game
xWaitingForStartText = "Attesa partenza"
xTimeRemainingText = "Tempo rimanente: %01d:%02d"
xMarkerGameMarkersRemaining = "Marker rimasti: %d"
xMarkerGameMarkersUnclaimed = "Marker non conquistati: %d"
xMarkerGameGreenTeamScore = "Squadra Verde (%d)"
xMarkerGameRedTeamScore = "Squadra Rossa(%d)"
xMarkerGameNameCapture = "cattura"
xMarkerGameInstructCapNoMarker = "Ma, siccome non ci sono marker in questa partita... tutti vincono o perdono!"
xMarkerGameInstructCapOneMarker = "La squadra che cattura l'unico marker entro %d minuto/i %s... vince!"
xMarkerGameInstructCapNMarkers = "La squadra che cattura più marker dei %d totali entro %d minuto/i %s... vince!"
xMarkerGameNameHold = "difesa"
xMarkerGameInstructHoldNoMarker = "Ma, siccome non ci sono marker in questa partita... tutti vincono o perdono!"
xMarkerGameInstructHoldNMarkers = "La squadra che cattura e difende più marker dei %d totali entro %d minuto/i %s... vince!"
xMarkerGameNameQuest = "ricerca"
xMarkerGameInstructQuest = "Non puoi invitare nessuno a giocare... è un gioco in solitaria."
xMarkerGameNameUnknown = "stile sconosciuto"
xMarkerGameQTitle = "Unisciti alla partita di %s"
xMarkerGameQMessage = "    %s vuole invitarti a giocare una partita di %s.\n    %s\n\nVuoi giocare?"
xMarkerGameInviteRecvd = "Marker game invite received. Check Incoming."     # NEW #

# --- Yes/No Dialog
xYesNoYESbutton = "Sì"
xYesNoOKbutton = "Ok"
xYesNoAcceptButton = "Accetta"
xYesNoDeclineButton = "Rifiuta"
xYesNoQuitbutton = "Esci"
xYesNoNoButton = "No"

# ---- Option Menu strings
xOptMenuKeyMap = "Mappatura tasti"
xOptMenuGameSettings = "Impostazioni gioco"
xOptMenuURULive = "URU Live"
xOptMenuHelp = "Aiuto"
xOptMenuCredits = "Riconoscimenti di URU"
xOptMenuQuit = "Esci da URU"
xOptMenuOk = "Riprendi partita"
xOptMenuCancel = "Annulla"

xMoveForward = "Avanti"
xMoveBackward = "Indietro"
xRotateLeft = "Ruota a sinistra"
xRotateRight = "Ruota a destra"
xJump = "Salto"
xExitMode = "Uscita"
xPushToTalk = "Premere per parlare"


# --- OK Dialog
# The following is all NEW, don't translate the \n followed by a number (not that it would translate)
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
xInviteKeyAdded = "Aggiunta chiave di invito: %s"
xMaxInvites = "Numero massimo di inviti raggiunto"
xMissingInviteFolder = "Cartella inviti mancanti"
xInviteUsage = "Uso: /invite <ChiaveInvito>"
xInviteAccepted = "Accettazione dell'invito in corso per l'amico: %s con chiave: %s"
xAcceptUsage = "Uso: /accept <NomeAmico> <ChiaveInvito>"
xCouldNotCast = "Impossibile lanciare elenco oggetti da annotare"
xKeys = "Chiavi: "
xRemoveNodeFailed = "Rimozione nodo fallita"
xInviteNotFound = "Invito non trovato"
xUninviteUsage = "Uso: /uninvite <ChiaveInvito>"
xDeletedInvitation = "Invito annullato: "

# --- xKIExtChatCommands strings
xSitCmd = "siedi"
xAfkCmd = "afk"
xInviteCmd = "invita"
xUninviteCmd = "disinvita"
xAcceptCmd = "accetta"
xShowInvitesCmd = "mostrainviti"
xWaveCmd = "saluta"
xWaveString = "%s saluta"
xSneezeCmd = "starnuto"
xSneezeString = "%s starnutisce"
xClapCmd = "applaudi"
xClapString = "%s applaude"
xLaughCmd = "ridi"
xLaughString = "%s ride"
xLOLCmd = "lol"
xLOLString = "%s ridacchia"
xROTFLCmd = "rotfl"
xROTFLString = "%s ride a crepapelle"
xDanceCmd = "balla"
xDanceString = "%s balla"
xYesCmd = "sì"
xYesString = "%s annuisce"
xNoCmd = "no"
xNoString = "%s scuote la testa"
xYawnCmd = "sbadiglia"
xYawnString = "%s sbadiglia"
xCheerCmd = "congratula"
xCheerString = "%s si congratula"
xThanksCmd = "grazie"
xThanksString = "%s ti ringrazia!"
xThxCmd = "grz"
xThxString = "%s ti ringrazia"
xCryCmd = "piangi"
xCryString = "<sniff> %s è triste"
xCriesCmd = "grida"
xCriesString = "%s grida"
xDontKnowCmd = "nonso"
xDontKnowString = "%s si gratta la testa"
xShrugCmd = "incerto"
xShrugString = "%s è incerto"
xDunnoCmd = "mah"
xDunnoString = "%s è indeciso"
xPointCmd = "indica"
xPointString = "%s indica"

# latest strings to be translated (build 32)
xKISettingsFontSizeText = "I:Font Size:"
xKISettingChatFadeTimeText = "I:Chat Fade Time:"
xKISettingsOnlyBuddiesText = "I:Only accept private messages and KI mail from Buddies"
xKIDescriptionText = "I:Description:"
xMarkerGameOwnerTitle = "I:OWNER:"
xMarkerGameTimeText = "I:Game Time:"
xCCRAwayText = "I:CCR momentarily away"
xCCRPetitionTypeText = "I:Petition Type:"
xCCRSubjectText = "I:Subject:"
xCCRCommentText = "I:Comment:"
xCCRSubmitBtnText = "I:Submit"
xCCRCancelBtnText = "I:Cancel"

#latest strings to be translated (build 36)
xKIStatusNexusLinkAdded = "I:A link has been added to your Nexus"

# updated strings in build .37
xPlayerEnterID = "I:Enter player ID or name:"
xPlayerNumberOnly = "I:Please enter a player ID or current Age player name."
xPlayerNotYourself = "I:Can't be yourself."
#--- KI add/create entry
xCreateBuddyTitle = "<I:add buddy by ID or name if in Age>"

# new strings starting in build .37
xChatAddBuddyCmd = "/addbuddy"
xChatRemoveBuddyCmd = "/removebuddy"
xChatIgnoreCmd = "/ignore"
xChatUnIgnoreCmd = "/unignore"
xPlayerAlreadyAdded = "I:Player has already been added."
xPlayerNotFound = "I:Player not found."
xPlayerAdded = "I:Player added."
xPlayerRemoved = "I:Player removed."
