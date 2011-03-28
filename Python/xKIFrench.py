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
xInviteVisitTitle = "Invitation à %s"
xInviteVisitBody = "Venez visiter l'Âge %s en vous rendant au MT Nexus et en utilisant votre Lien privée\n\nSigné(e),\n%s (Propriétaire)\n"
xRevokeVisitorTitle = "Visite à %s supprimée"
xRevokeVisitorBody = "Votre droit de visite à l'Âge %s n'est plus valide.\n\n<Pardon>\n\nSigné,\n%s (Propriétaire)"
#--- KI configuration strings
xKIConfiguration = "Paramètres du KI"
xVolumeConfiguration = "Volume"
xOwnerConfiguration = "Paramètres de %s"
xOwnerVistors = "%s visiteurs"
xOwnerCoOwners = "%s propriétaires"
xDevicesFolderName = "Appareils"
#--- KI add/create entry
#xCreateBuddyTitle = "<ajouter l'ID d'un ami>"
#--- KI leave messages
xLeaveGameMessageNormal = "Êtes-vous sûr(e) de vouloir quitter Uru ?"
xLeaveGameMessageNano = xLeaveGameMessageNormal
xLeaveGameMessageMicro = xLeaveGameMessageNormal
#xLeaveGameMessageNormal = "Ne partez pas ! Êtes-vous sûr(e) de vouloir quitter ?"
#xLeaveGameMessageNano = "Vous n'avez pas fait grand-chose. Êtes-vous sûr(e) de vouloir quitter ?"
#xLeaveGameMessageMicro = "Il vous reste tant à découvrir ! Êtes-vous sûr(e) de vouloir quitter ?"
#--- send to something error messages
xSendToErrorMessage1 = "Envoi impossible"
xSendToErrorMessage2 = "Joueur introuvable"
xSendToErrorMessage3 = "Type de joueur inconnu"
xSendToErrorMessage4 = "Élément de journal incorrect"
xSendToErrorMessage5 = "Doit être en texte uniquement"
xCommandErrorMessage1 = "Impossible de '%s'"
xKITimeBroke = "<liaison temporelle rompue>"
#--- delete messages
xDeletePictureAsk = 'Êtes-vous sûr(e) de vouloir supprimer "%s" ?'
xDeleteJournalAsk = 'Êtes-vous sûr(e) de vouloir supprimer "%s" ?'
xDeletePlayerAsk = 'Êtes-vous sûr(e) de vouloir retirer "%s" de votre dossier "%s" ?'
#--- KI is full error messages
xKIFullImagesError = "Votre KI ne peut plus stocker les images dans votre journal. Il est plein."
xKIFullNotesError = "Votre KI ne peut plus stocker de texte dans votre journal. Il est plein."
xKIFullMarkersError = "Votre KI ne peut plus stocker les marqueurs dans votre journal. Il est plein."
#--- CCR conversation
xCCRConversationStarted = '(début de conversation)'
xCCRConversationEnded = '(fin de conversation)'
xCCRNoCCRInContact = '(Aucun CCR en contact, message non envoyé)'
xCCRPetitionSent = '(%s envoyé) %s'
#--- KI - Chat strings
xChatNoOneToReply = "(Il n'y a personne à qui répondre.)"
xChatLeftTheAge = "(%s a quitté l'Âge)"
xChatLeftTheGame = '(%s a quitté le jeu)'
xChatWentOffline = '(%s est hors ligne. Conversation impossible.)'
xChatCannotFindBuddy = "(Impossible de trouver '%s' dans les listes de joueurs.)"
xChatBroadcastMsgRecvd = ""
xChatPrivateMsgRecvd = "De "
xChatInterAgeMsgRecvd = "De "
xChatInterAgePlayerRecvd = "%s dans %s"
xChatBroadcastSendTo = ""
xChatPrivateSendTo = "À "
xChatInterAgeSendTo = "À "
xChatTOPrompt = "À :"
xChatAllAgeCommand = "/crier"
xChatClearAll = "/effacerconversation"
xChatPrivateCommand = "/p"
xChatNeighborsCommand = "/voisins"
xChatBuddiesCommand = "/amis"
xChatNoOneListening = "(Vous êtes trop loin. Vous devriez peut-être crier ?)"
xChatInterAgeNotAvailable = "(Commutateur Inter-Âge non disponible)"
xChatReplyCommand = "/répondre"
xChatStartLogCommand = "/démarrerlog"
xChatStopLogCommand = "/arrêterlog"
xChatLogStarted = "Début de Chat.log..."
xChatLogStopped = "...Fin de Chat.log."
xChatPetitionCommands = {   "/pétition" : PtCCRPetitionType.kGeneralHelp,
                            "/aide" : PtCCRPetitionType.kGeneralHelp,
                            "/bug" : PtCCRPetitionType.kBug,
                            "/opinion" : PtCCRPetitionType.kFeedback,
                            "/exploit" : PtCCRPetitionType.kExploit,
                            "/harcèlement" : PtCCRPetitionType.kHarass,
                            "/coincé" : PtCCRPetitionType.kStuck,
                            "/supporttechnique" : PtCCRPetitionType.kTechnical
                        }
xChatCCRPetitionTitle = "Pétition en ligne"
xChatCCRCommand = "/pac"
xChatCCRMsgRecvd = 'De PAC :'
xChatCCRSendTo = 'À PAC :'
xChatErrorMsgRecvd = 'Erreur :'
xChatCCRFromPlayer = 'De %d à PAC :'
xChatWeeBeeAFK = " (Je suis à la surface. Je serai de retour dans une minute)"
xCCRHelpPopupMenu = [   ("Rapport de bug",PtCCRPetitionType.kBug),
                        ("Remarques et suggestions",PtCCRPetitionType.kFeedback),
                        ("Exploits et problèmes de cheats",PtCCRPetitionType.kExploit),
                        ("Harcèlement et code de conduite",PtCCRPetitionType.kHarass),
                        ("Coincé",PtCCRPetitionType.kStuck),
                        ("Problèmes techniques",PtCCRPetitionType.kTechnical),
                        ("Aide générale",PtCCRPetitionType.kGeneralHelp)
                    ]
xCCRHelpPopupDefault = 6
#--- Offer link to
xOfferLinkToMessage = 'On vous propose une liaison vers "%s" <cool !>. Souhaitez-vous y aller ?'
#--- KI - neighborhood strings
xAgeOwnedStatusLine = "%d propriétaire%s et %d visiteur%s."
xPorPAgeOwnedStatusLine = "%d propriétaire%s et %d visiteur%s. Il s'agit d'un Âge %s."
xNeighborhoodBottomLine = "%s sur %s"
xNeighborhoodNone = "Aucune inscription dans aucun quartier"
xNeighborhoodNoName = "<aucun nom>"
xNeighborhoodMakePorP = "Rendre %s"
# --- Player expanded strings
#xPlayerEnterID = "Entrez votre numéro ID :"
#xPlayerNumberOnly = "Veuillez entrer un numéro."
#xPlayerNotYourself = "Vous ne pouvez pas vous ajouter en tant qu’ami."
xPlayerInCleft = "est en ligne, perdu(e) dans la crevasse."
xPlayerInCloset = "est en ligne et change de vêtements."
xPlayerInAge = "est en ligne et explore l'Âge %s."
xPlayerOffline = "est hors ligne."
#---- KI Journal strings
xJournalInitialMessage = "<écrire un message>"
xJournalInitialTitle = "<écrire un titre>"
# --- KI IMage strings
xImageInitialTitle = "<écrire une légende>"

xFolderVisLists = "Listes de visiteurs :"
xFolderOwnLists = "Listes de propriétaires de l'Âge :"

# --- Marker Game strings
xMarkerFolderPopupMenu = [   ("1 min",60),
                        ("2 min",120),
                        ("5 min",300),
                        ("10 min",600),
                    ]
xChatMarkerTOAllTeams = "À : Toutes les équipes >"
xChatMarkerTOGreenTeam = "À : Équipe verte >"
xChatMarkerTORedTeam = "À : Équipe rouge >"
xChatMarkerAllTeams = "Toutes les équipes"
xChatMarkerGreenTeam = "Équipe verte"
xChatMarkerRedTeam = "Équipe rouge"
xMarkerGamePrematureEnding = "Le Maître de jeu a interrompu la partie !"
xMarkerGameCaptureGame = "Partie Capture"
xMarkerGameHoldGame = "Partie Défense"
xMarkerGameQuestGame = "Partie Quête"
xMarkerGameBegins = "Que la partie commence !"
xMarkerGameGreenTeamWins = "L'équipe verte a gagné ! %d à %d"
xMarkerGameTieGame = "Égalité.  %d à %d"
xMarkerGameRedTeamWins = "L'équipe rouge a gagné ! %d à %d"
xMarkerGameEnded = "Fin de la partie... %s"
xMarkerGameResults = "Résultats de la partie :"
xMarkerGameNoMarkers = "aucun marqueur"
xMarkerGameOneMarker = "un marqueur"
xMarkerGameNMarkers = "%d marqueurs"
xMarkerGameCaptured = "capturé(s)"
xMarkerGameFoundMarker = "Marqueur '%s' trouvé."
xMarkerGameLastMarker = "C'était le dernier marqueur."
xMarkerGameOneMoreLeft = "Il ne reste plus qu'un marqueur !"
xMarkerGameCaptures = "%s a capturé '%s'. %s"

xMarkerGameEditButton = "Modifier la partie"
xMarkerGamePlayButton = "Jouer"
xMarkerGameDoneEditButton = "Modification terminée"
xMarkerGameAddMarkerButton = "Ajouter un marqueur"
xMarkerGameMarkerListButton = "Liste des marqueurs"
xMarkerGameRemoveMarkerButton = "Supprimer un marqueur"
xMarkerGameGoBackButton = "Précédent"
xMarkerGameInviteButton = "Inviter un joueur"
xMarkerGameStartGameButton = "Commencer la partie"
xMarkerGameEndGameButton = "Terminer la partie"
xMarkerGameStopPlayingButton = "Arrêter de jouer"
xMarkerGameResetGameButton = "Réinitialiser la partie"
xMarkerGameStatusNoMarkers = "Il n'y a aucun marqueur"
xMarkerGameStatusOneMarker = "Il y a un marqueur"
xMarkerGameStatusNMarkers = "Il y a %d marqueurs"
xMarkerGameStatusIn = " dans %s"
xMarkerGameStatusAllFound = "Tous les marqueurs de la quête ont été trouvés."
xMarkerGameStatusNotAllFound = "Tous les marqueurs de la quête n'ont pas été trouvés."
#--- question on invite to marker game
xWaitingForStartText = "En attente du démarrage"
xTimeRemainingText = "Temps restant : %01d:%02d"
xMarkerGameMarkersRemaining = "Marqueurs restants : %d"
xMarkerGameMarkersUnclaimed = "Marqueurs non demandés : %d"
xMarkerGameGreenTeamScore = "Équipeverte(%d)"
xMarkerGameRedTeamScore = "Équiperouge(%d)"
xMarkerGameNameCapture = "capture"
xMarkerGameInstructCapNoMarker = "Cependant, puisqu'il n'y a aucun marqueur dans cette partie... tout le monde gagne et tout le monde perd !"
xMarkerGameInstructCapOneMarker = "L'équipe qui capture l'unique marqueur en moins de %d minute%s... gagne !"
xMarkerGameInstructCapNMarkers = "L'équipe qui capture le plus grand nombre de marqueurs parmi les %d marqueurs en moins de %d minute%s... gagne !"
xMarkerGameNameHold = "défense"
xMarkerGameInstructHoldNoMarker = "Cependant, puisqu'il n'y a aucun marqueur dans cette partie... tout le monde gagne et tout le monde perd !"
xMarkerGameInstructHoldNMarkers = "L'équipe qui capture et défend le plus grand nombre de marqueurs parmi les %d marqueurs en moins de %d minute%s... gagne !"
xMarkerGameNameQuest = "quête"
xMarkerGameInstructQuest = "Impossible d'inviter des joueurs... c'est un partie en solo."
xMarkerGameNameUnknown = "style inconnu"
xMarkerGameQTitle = "Rejoindre le Jeu de marqueurs de %s"
xMarkerGameQMessage = "    %s souhaiterait vous inviter pour une partie de %s.\n    %s\n\nsouhaitez-vous jouer ?"
xMarkerGameInviteRecvd = "Invitation à un Jeu de marqueurs reçue. Vérifiez vos messages."

# --- Yes/No Dialog
xYesNoYESbutton = "Oui"
xYesNoOKbutton = "Ok"
xYesNoAcceptButton = "Accepter"
xYesNoDeclineButton = "Refuser"
xYesNoQuitbutton = "Quitter"
xYesNoNoButton = "Non"

# ---- Option Menu strings
xOptMenuKeyMap = "Assigner les commandes"
xOptMenuGameSettings = "Paramètres du jeu"
xOptMenuURULive = "URU Live"
xOptMenuHelp = "Aide"
xOptMenuCredits = "Crédits URU"
xOptMenuQuit = "Quitter URU"
xOptMenuOk = "Reprendre la partie"
xOptMenuCancel = "Annuler"

xMoveForward = "Avancer"
xMoveBackward = "Reculer"
xRotateLeft = "Pivoter à gauche"
xRotateRight = "Pivoter à droite"
xJump = "Sauter"
xExitMode = "Quitter un mode"
xPushToTalk = "Appuyer pour parler"


# --- OK Dialog
# The following is all NEW, don't translate the \n followed by a number (not that it would translate)
xOKDialogDict = { # "code": "translation"
        "": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#01",
        "TERMINATED: Server LogOff. Reason: Logged In Elsewhere": "Vous avez été déconnecté(e) car quelqu’un d’autre utilise déjà votre compte.\n#02",
        "TERMINATED: Server LogOff. Reason: Timed Out": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#03",
        "TERMINATED: Server LogOff. Reason: Not Authenticated": "Un problème est survenu pendant la connexion. Veuillez vérifier votre compte et votre mot de passe et réessayer.\n#04",
        "TERMINATED: Server LogOff. Reason: Kicked Off": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#05",
        "TERMINATED: Server LogOff. Reason: Unknown Reason": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#06",
        "TERMINATED: Server LogOff. Reason: CCRs must use a protected lobby": "Vous avez été refusé(e) car les PAC doivent se connecter par le biais d’une salle protégée.\n#07",
        "TERMINATED: Server LogOff. Reason: CCRs must have internal client code": "Vous avez été refusé(e) car les PAC doivent disposer d’un code client interne.\n#08",
        "TERMINATED: Server LogOff. Reason: UNKNOWN REASON CODE": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#09",
        "SERVER SILENCE": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#10",
        "BAD VERSION": "Cette version d’Uru est trop ancienne. Veuillez quitter le programme et mettre le jeu à jour.\n#11",
	    "Player Disabled": "Le joueur que vous avez sélectionné n’est pas autorisé à Uru. Veuillez contacter le support clientèle pour plus d’informations.\n#12",
	    "CAN'T FIND AGE": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#13",
	    "AUTH RESPONSE FAILED": "Un problème est survenu pendant la connexion. Veuillez vérifier votre compte et votre mot de passe et réessayer.\n#14",
        "AUTH TIMEOUT": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#15",
        "SDL Desc Problem": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#16",
        "Unspecified error": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#17",
		"Failed to send msg": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#18",
		"Authentication timed out": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#19",
		"Peer timed out": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#20",
		"Server silence": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#21",
		"Protocol version mismatch": "Cette version d’Uru est trop ancienne. Veuillez quitter le programme et mettre le jeu à jour.\n#22",
		"Auth failed": "Un problème est survenu pendant la connexion. Veuillez vérifier votre compte et votre mot de passe et réessayer.\n#23",
		"Failed to create player": "Un problème est survenu pendant la création de votre joueur. Veuillez quitter le programme et réessayer.\n#24",
		"Invalid error code": "Il semble y avoir un problème avec la connexion. Veuillez quitter le programme et réessayer.\n#25",
		"linking banned": "Votre capacité de liaison a été désactivée\n#26",
		"linking restored": "Votre capacité de liaison a été rétablie\n#27",
		"silenced": "Votre aptitude à la discussion a été désactivée\n#28",
		"unsilenced": "Votre aptitude à la discussion a été rétablie\n#29"
}


# --- xInvite strings
xInviteKeyAdded = "Touche d'invitation ajoutée : %s"
xMaxInvites = "Nombre max d'invitations atteint"
xMissingInviteFolder = "Dossier d'invitation manquant"
xInviteUsage = "Utilisation : /inviter <Mot de passe>"
xInviteAccepted = "Invitation en cours d'acceptation avec l'Ami : %s et la Touche : %s"
xAcceptUsage = "Utilisation : /accepter <Nom de l'ami> <Mot de passe>"
xCouldNotCast = "Impossible de diffuser l'élément de liste vers la note"
xKeys = "Touches : "
xRemoveNodeFailed = "Échec de la suppression du Noeud"
xInviteNotFound = "Invitation introuvable"
xUninviteUsage = "Utilisation : /supinvit <Mot de passe>"
xDeletedInvitation = "Invitation supprimée : "

# --- xKIExtChatCommands strings
xSitCmd = "assis"
xAfkCmd = "absent"
xInviteCmd = "inviter"
xUninviteCmd = "supinvit"
xAcceptCmd = "accepter"
xShowInvitesCmd = "montrerinvit"
xWaveCmd = "signemain"
xWaveString = "%s fait un signe de la main"
xSneezeCmd = "éternuer"
xSneezeString = "%s éternue"
xClapCmd = "applaudir"
xClapString = "%s applaudit"
xLaughCmd = "rire"
xLaughString = "%s rit"
xLOLCmd = "mdr"
xLOLString = "%s est mort de rire"
xROTFLCmd = "mdrept"
xROTFLString = "%s est mort de rire écroulé par terre"
xDanceCmd = "danser"
xDanceString = "%s danse"
xYesCmd = "oui"
xYesString = "%s acquiesce de la tête"
xNoCmd = "non"
xNoString = "%s refuse de la tête"
xYawnCmd = "bâiller"
xYawnString = "%s bâille"
xCheerCmd = "salut"
xCheerString = "%s salue"
xThanksCmd = "merci"
xThanksString = "%s vous remercie bien !"
xThxCmd = "mrc"
xThxString = "%s vous remercie"
xCryCmd = "pleurer"
xCryString = "<snif> %s est triste"
xCriesCmd = "pleurnicher"
xCriesString = "%s pleurniche"
xDontKnowCmd = "jenesaispas"
xDontKnowString = "%s hausse les épaules"
xShrugCmd = "épaules"
xShrugString = "%s hausse les épaules"
xDunnoCmd = "chaispas"
xDunnoString = "%s hausse les épaules"
xPointCmd = "pointer"
xPointString = "%s pointe du doigt"

xKISettingsFontSizeText = "Taille de police :"
xKISettingChatFadeTimeText = "Temps de fondu des discussions :"
xKISettingsOnlyBuddiesText = "Uniquement accepter les messages privés et les e-mails des Amis"
xKIDescriptionText = "Description :"
xMarkerGameOwnerTitle = "PROPRIÉTAIRE :"
xMarkerGameTimeText = "Temps de jeu :"
xCCRAwayText = "PAC momentanément absent"
xCCRPetitionTypeText = "Type de pétition :"
xCCRSubjectText = "Objet :"
xCCRCommentText = "Commentaire :"
xCCRSubmitBtnText = "Valider"
xCCRCancelBtnText = "Annuler"

#latest strings to be translated (build 36)
xKIStatusNexusLinkAdded = "Un lien a été ajouté a votre Nexus"

# updated strings in build .37
xPlayerEnterID = "F:Enter player ID or name:"
xPlayerNumberOnly = "F:Please enter a player ID or current Age player name."
xPlayerNotYourself = "F:Can't be yourself."
#--- KI add/create entry
xCreateBuddyTitle = "<F:add buddy by ID or name if in Age>"

# new strings starting in build .37
xChatAddBuddyCmd = "/addbuddy"
xChatRemoveBuddyCmd = "/removebuddy"
xChatIgnoreCmd = "/ignore"
xChatUnIgnoreCmd = "/unignore"
xPlayerAlreadyAdded = "F:Player has already been added."
xPlayerNotFound = "F:Player not found."
xPlayerAdded = "F:Player added."
xPlayerRemoved = "F:Player removed."
