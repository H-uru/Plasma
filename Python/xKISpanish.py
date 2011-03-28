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
xInviteVisitTitle = "Invitación para visitar %s"
xInviteVisitBody = "Por favor, ven a visitar la Era %s yendo a la estación Nexo MT y realizando algo...\n\nFirmado,\n%s (Propietario)\n"
xRevokeVisitorTitle = "Visita a %s eliminada"
xRevokeVisitorBody = "Tus privilegios de visita no son válidos en la Era %s.\n\n<Lo sentimos>\n\nFirmado,\n%s (Propietario)"
#--- KI configuration strings
xKIConfiguration = "Ajustes del KI"
xVolumeConfiguration = "Ajustes del volumen"
xOwnerConfiguration = "Ajustes de %s "
xOwnerVistors = "Visitantes de %s"
xOwnerCoOwners = "Propietarios de %s"
xDevicesFolderName = "Dispositivos"
#--- KI add/create entry
#xCreateBuddyTitle = "<añadir ID del compañero>"
#--- KI leave messages
xLeaveGameMessageNormal = "¡No te vayas por favor! ¿Seguro que quieres salir de Uru?"
xLeaveGameMessageNano = xLeaveGameMessageNormal
xLeaveGameMessageMicro = xLeaveGameMessageNormal
#xLeaveGameMessageNormal = "¡No te vayas por favor! ¿Seguro que quieres salir?"
#xLeaveGameMessageNano = "No has hecho gran cosa. ¿Seguro que quieres salir?"
#xLeaveGameMessageMicro = "¡Aún te queda mucho por hacer! ¿Seguro que quieres salir?"
#--- send to something error messages
xSendToErrorMessage1 = "No se puede enviar a ese"
xSendToErrorMessage2 = "Jugador no encontrado"
xSendToErrorMessage3 = "Tipo de jugador desconocido"
xSendToErrorMessage4 = "Elemento erróneo del diario"
xSendToErrorMessage5 = "Tiene que ser sólo texto"
xCommandErrorMessage1 = "No sé cómo '%s'"
xKITimeBroke = "<conexión de tiempo perdida>"
#--- delete messages
xDeletePictureAsk = '¿Seguro que quieres eliminar "%s"?'
xDeleteJournalAsk = '¿Seguro que quieres eliminar "%s"?'
xDeletePlayerAsk = '¿Seguro que quieres suprimir "%s" de tu carpeta "%s"?'
#--- KI is full error messages
xKIFullImagesError = "Tu KI no puede guardar más imágenes en tu diario, está lleno."
xKIFullNotesError = "Tu KI no puede guardar más notas de texto en tu diario, está lleno."
xKIFullMarkersError = "Tu KI no puede guardar más marcadores en tu diario, está lleno."
#--- CCR conversation
xCCRConversationStarted = '(conversación iniciada)'
xCCRConversationEnded = '(conversación finalizada)'
xCCRNoCCRInContact = '(No hay ningún CCR contactado, mensaje no enviado)'
xCCRPetitionSent = '(%s enviado) %s'
#--- KI - Chat strings
xChatNoOneToReply = '(No hay nadie a quien responder.)'
xChatLeftTheAge = '(%s ha abandonado la Era)'
xChatLeftTheGame = '(%s ha abandonado la partida)'
xChatWentOffline = '(%s no está en contacto y no se puede localizarse para charlar.)'
xChatCannotFindBuddy = "(No puedo encontrar a '%s' en ninguna de las listas de jugadores.)"
xChatBroadcastMsgRecvd = ""
xChatPrivateMsgRecvd = "De "
xChatInterAgeMsgRecvd = "De "
xChatInterAgePlayerRecvd = "%s en %s"
xChatBroadcastSendTo = ""
xChatPrivateSendTo = "A "
xChatInterAgeSendTo = "A "
xChatTOPrompt = "A:"
xChatAllAgeCommand = "/grito"
xChatClearAll = "/autorizar chat"
xChatPrivateCommand = "/p"
xChatNeighborsCommand = "/sectores"
xChatBuddiesCommand = "/amigos"
xChatNoOneListening = "(Estás demasiado lejos, ¿quizás si subes la voz?)"
xChatInterAgeNotAvailable = "(Interruptor de Entre-Eras no disponible)"
xChatReplyCommand = "/responder"
xChatStartLogCommand = "/iniciar registro"
xChatStopLogCommand = "/parar registro"
xChatLogStarted = "Registro de Chat iniciado..."
xChatLogStopped = "...Registro de Chat detenido."
xChatPetitionCommands = {   "/petición" : PtCCRPetitionType.kGeneralHelp,
                            "/ayudageneral" : PtCCRPetitionType.kGeneralHelp,
                            "/error" : PtCCRPetitionType.kBug,
                            "/comentario" : PtCCRPetitionType.kFeedback,
                            "/proeza" : PtCCRPetitionType.kExploit,
                            "/acoso" : PtCCRPetitionType.kHarass,
                            "/bloqueado" : PtCCRPetitionType.kStuck,
                            "/técnico" : PtCCRPetitionType.kTechnical
                        }
xChatCCRPetitionTitle = "Solicitud de línea de Chat"
xChatCCRCommand = "/ccr"
xChatCCRMsgRecvd = 'De CCR:'
xChatCCRSendTo = 'A CCR:'
xChatErrorMsgRecvd = 'Error:'
xChatCCRFromPlayer = 'De %d a CCR:'
xChatWeeBeeAFK = " (Estoy en la superficie, volveré en un minuto)"
xCCRHelpPopupMenu = [   ("Informe de errores",PtCCRPetitionType.kBug),
                        ("Comentarios y sugerencias",PtCCRPetitionType.kFeedback),
                        ("Temas de Abusos y Trampas",PtCCRPetitionType.kExploit),
                        ("Problemas de Molestias y Código de Conducta",PtCCRPetitionType.kHarass),
                        ("Bloqueado",PtCCRPetitionType.kStuck),
                        ("Problemas técnicos",PtCCRPetitionType.kTechnical),
                        ("Ayuda general",PtCCRPetitionType.kGeneralHelp)
                    ]
xCCRHelpPopupDefault = 6
#--- Offer link to
xOfferLinkToMessage = 'Te han dado la oportunidad de conectarte con "%s" <¡genial!>. ¿Quieres ir?'
#--- KI - neighborhood strings
xAgeOwnedStatusLine = "%d propietarios%s con %d visitantes%s."
xPorPAgeOwnedStatusLine = "%d propietarios%s con %d visitantes%s. Y son %s."
xNeighborhoodBottomLine = "%s de %s"
xNeighborhoodNone = "No hay asociaciones en ningún sector"
xNeighborhoodNoName = "<sin nombre>"
xNeighborhoodMakePorP = "Hacer %s"
# --- Player expanded strings
#xPlayerEnterID = "Introduce el ID del jugador:"
#xPlayerNumberOnly = "Por favor, introduce sólo el número."
#xPlayerNotYourself = "A buddy can't be yourself."           # NEW #
xPlayerInCleft = "Está en línea y perdido en la Grieta."
xPlayerInCloset = "Está en línea y cambiándose de ropa."
xPlayerInAge = "Está en línea y explorando la Era %s."
xPlayerOffline = "Está desconectado."
#---- KI Journal strings
xJournalInitialMessage = "<introducir mensaje>"
xJournalInitialTitle = "<introducir título>"
# --- KI IMage strings
xImageInitialTitle = "<introducir imagen>"

xFolderVisLists = "Lista de visitantes de la Era:"
xFolderOwnLists = "Listas de dueños de la Era:"

# --- Marker Game strings
xMarkerFolderPopupMenu = [   ("1 min.",60),
                        ("2 min.",120),
                        ("5 min.",300),
                        ("10 min.",600),
                    ]
xChatMarkerTOAllTeams = "A: Todos los equipos >"
xChatMarkerTOGreenTeam = "A: El equipo verde >"
xChatMarkerTORedTeam = "A: El equipo rojo >"
xChatMarkerAllTeams = "Todos los equipos"
xChatMarkerGreenTeam = "Equipo verde"
xChatMarkerRedTeam = "Equipo rojo"
xMarkerGamePrematureEnding = "¡El anfitrión del juego finalizó la partida!"
xMarkerGameCaptureGame = "Partida de Atrapar"
xMarkerGameHoldGame = "Partida de Espera"
xMarkerGameQuestGame = "Partida de Búsqueda"
xMarkerGameBegins = "¡Empieza la partida!"
xMarkerGameGreenTeamWins = "¡El equipo verde gana! %d a %d"
xMarkerGameTieGame = "Partida empatada.  %d a %d"
xMarkerGameRedTeamWins = "¡El equipo rojo gana! %d a %d"
xMarkerGameEnded = "Partida finalizada... %s"
xMarkerGameResults = "Resultado de la partida:"
xMarkerGameNoMarkers = "sin marcadores"
xMarkerGameOneMarker = "un marcador"
xMarkerGameNMarkers = "marcadores %d "
xMarkerGameCaptured = "atrapado"
xMarkerGameFoundMarker = "Marcador encontrado '%s'."
xMarkerGameLastMarker = "Y ese fue el último marcador."
xMarkerGameOneMoreLeft = "¡Sólo queda UN marcador!"
xMarkerGameCaptures = "%s capturas '%s'. %s"

xMarkerGameEditButton = "Editar partida"
xMarkerGamePlayButton = "Jugar partida"
xMarkerGameDoneEditButton = " Edición realizada"
xMarkerGameAddMarkerButton = "Añadir marcador"
xMarkerGameMarkerListButton = "Lista de marcadores"
xMarkerGameRemoveMarkerButton = "Eliminar marcador"
xMarkerGameGoBackButton = "Volver"
xMarkerGameInviteButton = "Invitar a un jugador"
xMarkerGameStartGameButton = "Iniciar partida"
xMarkerGameEndGameButton = "Finalizar partida"
xMarkerGameStopPlayingButton = "Detener el juego"
xMarkerGameResetGameButton = "Reiniciar la partida"
xMarkerGameStatusNoMarkers = "No hay marcadores"
xMarkerGameStatusOneMarker = "Sólo hay un marcador"
xMarkerGameStatusNMarkers = "Hay %d marcadores"
xMarkerGameStatusIn = " en %s"
xMarkerGameStatusAllFound = "Todos los marcadores de la búsqueda han sido localizados."
xMarkerGameStatusNotAllFound = "No se han localizado todos los marcadores de búsqueda."
#--- question on invite to marker game
xWaitingForStartText = "Esperando para comenzar"
xTimeRemainingText = "Tiempo restante: %01d:%02d"
xMarkerGameMarkersRemaining = "Marcadores restantes: %d"
xMarkerGameMarkersUnclaimed = "Marcadores sin reclamar: %d"
xMarkerGameGreenTeamScore = "EquipoVerde(%d)"
xMarkerGameRedTeamScore = "EquipoRojo(%d)"
xMarkerGameNameCapture = "captura"
xMarkerGameInstructCapNoMarker = "Como no hay marcadores en esta partida... ¡todo el mundo ganará o perderá!"
xMarkerGameInstructCapOneMarker = "El equipo que capture el único marcador antes de que se agote un tiempo de %d minuto%s... ¡gana!"
xMarkerGameInstructCapNMarkers = "¡El equipo que capture el mayor número de marcadores del total de %d marcadores antes de que se agote un tiempo de %d minuto%s... ¡gana!"
xMarkerGameNameHold = "espera"
xMarkerGameInstructHoldNoMarker = "Como no hay marcadores en esta partida... ¡todo el mundo ganará o perderá!"
xMarkerGameInstructHoldNMarkers = "¡El equipo que capture y mantenga el mayor número de marcadores del total de %d marcadores cuando se agote un tiempo de %d minuto%s... ¡gana!"
xMarkerGameNameQuest = "búsqueda"
xMarkerGameInstructQuest = "No se puede invitar a nadie a jugar... es una partida para un solo jugador."
xMarkerGameNameUnknown = "estilo desconocido"
xMarkerGameQTitle = "Unirse a Partida Marcador %s"
xMarkerGameQMessage = "    %s le gustaría invitarte a jugar una ronda de la partida %s.\n    %s\n\n¿Deseas jugar?"
xMarkerGameInviteRecvd = "Marker game invite received. Check Incoming."     # NEW #

# --- Yes/No Dialog
xYesNoYESbutton = "Sí"
xYesNoOKbutton = "Vale"
xYesNoAcceptButton = "Aceptar"
xYesNoDeclineButton = "Declinar"
xYesNoQuitbutton = "Salir"
xYesNoNoButton = "No"

# ---- Option Menu strings
xOptMenuKeyMap = "Mapa clave"
xOptMenuGameSettings = "Ajustes del juego"
xOptMenuURULive = "URU Live"
xOptMenuHelp = "Ayuda"
xOptMenuCredits = "Créditos de URU"
xOptMenuQuit = "Salir de URU"
xOptMenuOk = "Reanudar Partida"
xOptMenuCancel = "Cancelar"

xMoveForward = "Mover hacia adelante"
xMoveBackward = "Mover hacia atrás"
xRotateLeft = "Girar a la izquierda"
xRotateRight = "Girar a la derecha"
xJump = "Saltar"
xExitMode = "Modo Salir"
xPushToTalk = "Pulsa para hablar"


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
xInviteKeyAdded = "Añadida clave de invitación: %s"
xMaxInvites = "Se ha excedido en máximo de invitaciones"
xMissingInviteFolder = "Falta la carpeta de Invitación"
xInviteUsage = "Uso: /invitar <inviteKey>"
xInviteAccepted = "Aceptando la invitación de un amigo en progreso: %s y Clave: %s"
xAcceptUsage = "Uso: /acceptar <friendsName> <inviteKey>"
xCouldNotCast = "No se pudo asignar objeto de la lista a la nota"
xKeys = "Claves: "
xRemoveNodeFailed = "Fallo al suprimir el Nodo"
xInviteNotFound = "Invitación no encontrada"
xUninviteUsage = "Uso: /no invitar <inviteKey>"
xDeletedInvitation = "Eliminar invitación: "

# --- xKIExtChatCommands strings
xSitCmd = "aquí"
xAfkCmd = "vuelvo en seguida"
xInviteCmd = "invitar"
xUninviteCmd = "no invitar"
xAcceptCmd = "acceptar"
xShowInvitesCmd = "mostrarinvitados"
xWaveCmd = "saludar"
xWaveString = "%s saludos"
xSneezeCmd = "estornudar"
xSneezeString = "%s estornudos"
xClapCmd = "aplaudir"
xClapString = "%s aplaude"
xLaughCmd = "reír"
xLaughString = "%s ríe"
xLOLCmd = "reír en alto"
xLOLString = "%s empieza a reírse"
xROTFLCmd = "desternillarse"
xROTFLString = "%s se desternilla"
xDanceCmd = "bailar"
xDanceString = "%s baila"
xYesCmd = "sí"
xYesString = "%s asiente con la cabeza"
xNoCmd = "no"
xNoString = "%s niega con la cabeza"
xYawnCmd = "bostezar"
xYawnString = "%s bosteza"
xCheerCmd = "vitorear"
xCheerString = "%s vítores"
xThanksCmd = "gracias"
xThanksString = "%s ¡te da las gracias!"
xThxCmd = "gracias"
xThxString = "%s te lo agradece"
xCryCmd = "llorar"
xCryString = "<sollozo> %s está triste"
xCriesCmd = "llora"
xCriesString = "%s llora"
xDontKnowCmd = "no sé"
xDontKnowString = "%s se encoge de hombros"
xShrugCmd = "encogerse de hombros"
xShrugString = "%s se encoge de hombros"
xDunnoCmd = "no sé"
xDunnoString = "%s se encoge de hombros"
xPointCmd = "apuntar"
xPointString = "%s apunta"

# latest strings to be translated (build 32)
xKISettingsFontSizeText = "S:Font Size:"
xKISettingChatFadeTimeText = "S:Chat Fade Time:"
xKISettingsOnlyBuddiesText = "S:Only accept private messages and KI mail from Buddies"
xKIDescriptionText = "S:Description:"
xMarkerGameOwnerTitle = "S:OWNER:"
xMarkerGameTimeText = "S:Game Time:"
xCCRAwayText = "S:CCR momentarily away"
xCCRPetitionTypeText = "S:Petition Type:"
xCCRSubjectText = "S:Subject:"
xCCRCommentText = "S:Comment:"
xCCRSubmitBtnText = "S:Submit"
xCCRCancelBtnText = "S:Cancel"

#latest strings to be translated (build 36)
xKIStatusNexusLinkAdded = "S:A link has been added to your Nexus"

# updated strings in build .37
xPlayerEnterID = "S:Enter player ID or name:"
xPlayerNumberOnly = "S:Please enter a player ID or current Age player name."
xPlayerNotYourself = "S:Can't be yourself."
#--- KI add/create entry
xCreateBuddyTitle = "<S:add buddy by ID or name if in Age>"

# new strings starting in build .37
xChatAddBuddyCmd = "/addbuddy"
xChatRemoveBuddyCmd = "/removebuddy"
xChatIgnoreCmd = "/ignore"
xChatUnIgnoreCmd = "/unignore"
xPlayerAlreadyAdded = "S:Player has already been added."
xPlayerNotFound = "S:Player not found."
xPlayerAdded = "S:Player added."
xPlayerRemoved = "S:Player removed."
