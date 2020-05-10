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

import re
import time

# Plasma Engine.
from Plasma import *
from PlasmaConstants import *
from PlasmaKITypes import *
from PlasmaNetConstants import *
from PlasmaTypes import *
from PlasmaVaultConstants import *

import xCensor
import xLocTools

# xKI sub-modules.
from . import xKIExtChatCommands
from .xKIConstants import *
from .xKIHelpers import *


## A class to process all the RT Chat functions of the KI.
class xKIChat(object):

    ## Set up the chat manager's default state.
    def __init__(self, StartFadeTimer, ResetFadeState, FadeCompletely, GetCensorLevel):

        # Set the default properties.
        self.chatLogFile = None
        self.gFeather = 0
        self.isAdmin = False
        self.isChatting = False
        self.lastPrivatePlayerID = None
        self.logFile = None
        self.onlyGetPMsFromBuddies = False
        self.onlyAllowBuddiesOnRequest = False
        self.privateChatChannel = 0
        self.toReplyToLastPrivatePlayerID = None

        # Fading & blinking globals.
        self.currentFadeTick = 0
        self.fadeEnableFlag = True
        self.fadeMode = kChat.FadeNotActive
        self.ticksOnFull = 30
        self.incomingChatFlashState = 0

        # Set the properties from the KI.
        self.key = None  # Plasma has to be initialized for this.
        self.BKPlayerList = []
        self.KIDisabled = False
        self.KILevel = kMicroKI
        self.StartFadeTimer = StartFadeTimer
        self.ResetFadeState = ResetFadeState
        self.FadeCompletely = FadeCompletely
        self.GetCensorLevel = GetCensorLevel

        # Add the commands processor.
        self.commandsProcessor = CommandsProcessor(self)

        # Message History
        self.MessageHistoryIs = -1 # Current position in message history (up/down key)
        self.MessageHistoryList = [] # Contains our message history

    #######
    # GUI #
    #######

    ## Toggles the state of the miniKI icon in the Blackbar.
    # Changing the value will initiate a chain reaction of events to display
    # the miniKI.
    def ClearBBMini(self, value=-1):

        if self.KILevel == kNormalKI:
            mmRG = ptGUIControlRadioGroup(KIBlackbar.dialog.getControlFromTag(kGUI.MiniMaximizeRGID))
            mmRG.setValue(value)

    ## Check if the chat is faded out.
    def IsFaded(self):

        if self.KILevel >= kMicroKI:
            if not BigKI.dialog.isEnabled() and (KIMini.dialog.isEnabled() or KIMicro.dialog.isEnabled()):
                if self.fadeMode == kChat.FadeNotActive:
                    return True
        return False

    ## Scroll the chat in the specified direction on the miniKI.
    # Possible directions are to scroll up, down, to the beginning and to the
    # end.
    def ScrollChatArea(self, direction):

        if self.KILevel < kNormalKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        self.ResetFadeState()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatarea.moveCursor(direction)

    ############
    # Chatting #
    ############

    ## Make the player enter or exit chat mode.
    # Chat mode means the player's keyboard input is being sent to the chat.
    def ToggleChatMode(self, entering, firstChar=None):
        # Reset selection in message history
        self.MessageHistoryIs = -1

        if self.KILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
            KIMicro.dialog.show()
        else:
            mKIdialog = KIMini.dialog
        caret = ptGUIControlTextBox(mKIdialog.getControlFromTag(kGUI.ChatCaretID))
        chatEdit = ptGUIControlEditBox(mKIdialog.getControlFromTag(kGUI.ChatEditboxID))
        if entering:
            self.isChatting = True
            if not KIMini.dialog.isEnabled():
                self.ClearBBMini(0)
            if firstChar:
                chatEdit.setString(firstChar)
                chatEdit.end()
            else:
                chatEdit.clearString()
            chatEdit.show()
            caret.show()
            mKIdialog.setFocus(chatEdit.getKey())
            self.toReplyToLastPrivatePlayerID = self.lastPrivatePlayerID
            self.ResetFadeState()
        else:
            caret.hide()
            chatEdit.hide()
            self.isChatting = False
            self.StartFadeTimer()

    ## Sends a chat message appropriately to other players.
    # A broadcast message gets sent to everyone in the Age (purple header), a
    # private message is sent to one player (yellow header), and so on.
    def SendMessage(self, message):

        # Set up the chat message's flags.
        cFlags = ChatFlags(0)
        cFlags.toSelf = True
        listenerOnly = True
        nobodyListening = False
        goesToFolder = None
        if self.isAdmin:
            cFlags.admin = True
        ## The prefix for inter-Age chat.
        pre = "<<" + GetAgeName() + ">>"

        # If the player is in AFK mode, make him exit it.
        if PtGetLocalAvatar().avatar.getCurrentMode() == PtBrainModes.kAFK:
            PtAvatarExitAFK()

        # Add the input to the local chat message history.
        self.MessageHistoryList.insert(0, message)
        if (len(self.MessageHistoryList) > kMessageHistoryListMax):
            self.MessageHistoryList.pop()

        # Check for special commands.
        message = self.commandsProcessor(message)
        if not message:
            return
        msg = message.lower()

        # Get any selected players.
        userListBox = ptGUIControlListBox(KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        iSelect = userListBox.getSelection()
        selPlyrList = []

        # Is it a reply to a private message?
        if msg.startswith(PtGetLocalizedString("KI.Commands.ChatReply")):
            if self.toReplyToLastPrivatePlayerID is None:
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.NoOneToReply"), kChat.SystemMessage)
                return
            # If it is local, check to see if the player is still in the Age.
            if not self.toReplyToLastPrivatePlayerID[2]:
                ageMembers = PtGetPlayerListDistanceSorted()
                hasPlayer = False
                for member in ageMembers:
                    if member.getPlayerID() == self.toReplyToLastPrivatePlayerID[1]:
                        hasPlayer = True
                        break
                if not hasPlayer:
                    self.lastPrivatePlayerID = None
                    self.AddChatLine(None, PtGetLocalizedString("KI.Chat.LeftTheAge", [str(self.toReplyToLastPrivatePlayerID[0])]), kChat.SystemMessage)
                    return
            # If it is not local, check to see if the player is still online.
            else:
                vault = ptVault()
                PIKA = vault.getPeopleIKnowAboutFolder()
                if PIKA is not None and PIKA.playerlistHasPlayer(self.toReplyToLastPrivatePlayerID[1]):
                    PIKArefs = PIKA.getChildNodeRefList()
                    for PIKAref in PIKArefs:
                        PIKAelem = PIKAref.getChild()
                        PIKAelem = PIKAelem.upcastToPlayerInfoNode()
                        if PIKAelem is not None and PIKAelem.playerGetID() == self.toReplyToLastPrivatePlayerID[1]:
                            if not PIKAelem.playerIsOnline():
                                self.lastPrivatePlayerID = None
                                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.LeftTheGame", [str(self.toReplyToLastPrivatePlayerID[0])]), kChat.SystemMessage)
                                return
                            break
            message = message[len(PtGetLocalizedString("KI.Commands.ChatReply")) + 1:]
            # What they selected doesn't matter if they're replying.
            selPlyrList = [ptPlayer(self.toReplyToLastPrivatePlayerID[0], self.toReplyToLastPrivatePlayerID[1])]
            cFlags.private = True
            if self.toReplyToLastPrivatePlayerID[2]:
                cFlags.interAge = True
                message = pre + message

        # Is it a private message sent with "/p"?
        elif msg.startswith(PtGetLocalizedString("KI.Commands.ChatPrivate")):
            pWords = message.split(" ", 1)
            foundBuddy = False
            # Make sure it's still just a "/p".
            if len(pWords) > 1 and pWords[0] == PtGetLocalizedString("KI.Commands.ChatPrivate"):
                # Try to find the buddy in the DPL online lists.
                for player in self.BKPlayerList:
                    # Is the player in this Age?
                    if isinstance(player, ptPlayer):
                        plyrName = player.getPlayerName()
                        if pWords[1].startswith(plyrName + " "):
                            selPlyrList.append(player)
                            cFlags.private = True
                            foundBuddy = True
                            # Remove the "/p buddyname" from the message.
                            message = pWords[1][len(plyrName) + 1:]
                            self.AddPlayerToRecents(player.getPlayerID())
                            break
                    # Is the player in another Age?
                    elif isinstance(player, ptVaultNodeRef):
                        ePlyr = player.getChild()
                        ePlyr = ePlyr.upcastToPlayerInfoNode()
                        if ePlyr is not None:
                            plyrName = ePlyr.playerGetName()
                            if pWords[1].startswith(plyrName + " "):
                                selPlyrList.append(ptPlayer(ePlyr.playerGetName(), ePlyr.playerGetID()))
                                cFlags.private = True
                                cFlags.interAge = True
                                foundBuddy = True
                                # Add this player's current Age.
                                message = pre + pWords[1][len(plyrName) + 1:]
                                self.AddPlayerToRecents(ePlyr.playerGetID())
                                break
            if not foundBuddy:
                PtDebugPrint(u"xKIChat.SendMessage(): \"/p\" command can't find player.", level=kDebugDumpLevel)
                # Note: because there's no way of knowing the player's name
                #(might have spaces), just don't try to display it.
                self.AddChatLine(None, "(Can't find the player in any of the player lists.)", kChat.SystemMessage)
                return

        # Is it a message to the player's neighbors?
        elif msg.startswith(PtGetLocalizedString("KI.Commands.ChatNeighbors")):
            cFlags.neighbors = True
            message = message[len(PtGetLocalizedString("KI.Commands.ChatNeighbors")) + 1:]
            neighbors = GetNeighbors()
            if neighbors is not None:
                selPlyrList = self.GetOnlinePlayers(neighbors.getChildNodeRefList())
            if len(selPlyrList) == 0:
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in your neighbor list"]), kChat.SystemMessage)
                return
            cFlags.interAge = True
            message = pre + message
            goesToFolder = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)

        # Is it a message to the player's buddies?
        elif msg.startswith(PtGetLocalizedString("KI.Commands.ChatBuddies")):
            vault = ptVault()
            buddies = vault.getBuddyListFolder()
            message = message[len(PtGetLocalizedString("KI.Commands.ChatBuddies")) + 1:]
            if buddies is not None:
                selPlyrList = self.GetOnlinePlayers(buddies.getChildNodeRefList())
            if len(selPlyrList) == 0:
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in your buddy list"]), kChat.SystemMessage)
                return
            cFlags.interAge = True
            message = pre + message
            goesToFolder = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)

        # Otherwise, the message has a selected destination.
        else:
            if iSelect >= 0 and iSelect < len(self.BKPlayerList):
                toPlyr = self.BKPlayerList[iSelect]

                # Is it a player?
                if isinstance(toPlyr, ptPlayer):
                    selPlyrList.append(toPlyr)
                    cFlags.private = True
                    self.AddPlayerToRecents(toPlyr.getPlayerID())
                    PtDebugPrint(u"xKIChat.SendMessage(): Private message to \"{}\".".format(toPlyr.getPlayerName()), level=kDebugDumpLevel)

                # Is it a player (possibly in another Age)?
                elif isinstance(toPlyr, ptVaultNodeRef):
                    ePlyr = toPlyr.getChild()
                    ePlyr = ePlyr.upcastToPlayerInfoNode()
                    if ePlyr is not None:
                        if not ePlyr.playerIsOnline():
                            self.AddChatLine(None, PtGetLocalizedString("KI.Chat.WentOffline", [ePlyr.playerGetName()]), kChat.SystemMessage)
                            return
                        selPlyrList.append(ptPlayer(ePlyr.playerGetName(), ePlyr.playerGetID()))
                        cFlags.private = True
                        self.AddPlayerToRecents(ePlyr.playerGetID())
                        if not PtValidateKey(PtGetAvatarKeyFromClientID(ePlyr.playerGetID())):
                            cFlags.interAge = True
                            message = pre + message

                # Is it a list of players?
                elif isinstance(toPlyr, ptVaultPlayerInfoListNode):
                    fldrType = toPlyr.folderGetType()
                    # If it's a list of Age Owners, they must be neighbors.
                    if fldrType == PtVaultStandardNodes.kAgeOwnersFolder:
                        fldrType = PtVaultStandardNodes.kHoodMembersFolder
                        cFlags.neighbors = True

                    # Special rules for AllPlayers: ccr message and NOT directed!
                    if fldrType == PtVaultStandardNodes.kAllPlayersFolder:
                        selPlyrList = []
                        listenerOnly = False
                        cFlags.admin = 1
                        cFlags.ccrBcast = 1
                        pre = ""
                    else:
                        selPlyrList = self.GetOnlinePlayers(toPlyr.getChildNodeRefList())
                        if len(selPlyrList) == 0:
                            self.AddChatLine(None, PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in list"]), kChat.SystemMessage)
                            return
                        cFlags.interAge = 1

                    message = pre + message
                    goesToFolder = xLocTools.FolderIDToFolderName(fldrType)

                # Is it a folder of players within listening distance?
                elif isinstance(toPlyr, KIFolder):
                    listenerOnly = False

        # Add message to player's private chat channel.
        cFlags.channel = self.privateChatChannel
        if len(selPlyrList) == 0 and listenerOnly:
            if nobodyListening:
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.NoOneListening"), kChat.SystemMessage)
        else:
            PtSendRTChat(PtGetLocalPlayer(), selPlyrList, message, cFlags.flags)
        sender = PtGetLocalPlayer()
        if cFlags.private:
            sender = selPlyrList[0]
        if cFlags.interAge:
            if goesToFolder:
                sender = ptPlayer(str(goesToFolder), 0)
            else:
                sender = selPlyrList[0]
        elif goesToFolder:
            sender = ptPlayer(str(goesToFolder), 0)
        self.AddChatLine(sender, message, cFlags)

    #######################
    # Displaying messages #
    #######################

    ## Adds a line to the RT chat.
    def AddChatLine(self, player, message, cFlags, forceKI=True):

        try:
            PtDebugPrint(u"xKIChat.AddChatLine(): Message = \"{}\".".format(message), player, cFlags, level=kDebugDumpLevel)
        except UnicodeEncodeError:
            pass

        # Fix for Character of Doom (CoD).
        (message, RogueCount) = re.subn("[\x00-\x08\x0a-\x1f]", "", message)

        # Censor the chat message to their taste
        message = xCensor.xCensor(message, self.GetCensorLevel())

        if self.KILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        pretext = U""
        headerColor = kColors.ChatHeaderBroadcast
        bodyColor = kColors.ChatMessage

        # Is it an object to represent the flags?
        if isinstance(cFlags, ChatFlags):

            # Is it a status message?
            if cFlags.status:
                bodyColor = kColors.ChatHeaderStatus
                player = None

            # Is it an inter-Age message?
            elif cFlags.interAge:
                if cFlags.private:
                    if cFlags.admin:
                        headerColor = kColors.ChatHeaderError
                    else:
                        headerColor = kColors.ChatHeaderPrivate
                    forceKI = True
                else:
                    if cFlags.neighbors:
                        headerColor = kColors.ChatHeaderNeighbors
                    else:
                        headerColor = kColors.ChatHeaderBuddies
                if cFlags.toSelf:
                    pretext = PtGetLocalizedString("KI.Chat.InterAgeSendTo")
                    if message[:2] == "<<":
                        try:
                            idx = message.index(">>")
                            message = message[idx + 2:]
                        except ValueError:
                            pass
                else:
                    if not self.CheckIfCanPM(player.getPlayerID()):
                        return
                    pretext = PtGetLocalizedString("KI.Chat.InterAgeMsgRecvd")
                    forceKI = True
                    if message[:2] == "<<":
                        try:
                            idx = message.index(">>")
                            player = ptPlayer(PtGetLocalizedString("KI.Chat.InterAgePlayerRecvd", [player.getPlayerName(), message[2:idx]]), player.getPlayerID())
                            message = message[idx + 2:]
                            if not cFlags.private and not cFlags.neighbors:
                                buddies = ptVault().getBuddyListFolder()
                                if buddies is not None:
                                    buddyID = player.getPlayerID()
                                    if not buddies.playerlistHasPlayer(buddyID):
                                        PtDebugPrint(u"xKIChat.AddChatLine(): Add unknown buddy {} to recents.".format(buddyID))
                                        self.AddPlayerToRecents(buddyID)
                        except ValueError:
                            pass

                    # PM Processing: Save playerID and flash client window
                    if cFlags.private:
                        self.lastPrivatePlayerID = (player.getPlayerName(), player.getPlayerID(), 1)
                        PtFlashWindow()
                    # Are we mentioned in the message?
                    elif message.lower().find(PtGetLocalPlayer().getPlayerName().lower()) >= 0:
                        bodyColor = kColors.ChatMessageMention
                        PtFlashWindow()

            # Is it a ccr broadcast?
            elif cFlags.ccrBcast:
                headerColor = kColors.ChatHeaderAdmin
                if cFlags.toSelf:
                    pretext = PtGetLocalizedString("KI.Chat.PrivateSendTo")
                else:
                    pretext = PtGetLocalizedString("KI.Chat.PrivateMsgRecvd")
                forceKI = True
                self.AddPlayerToRecents(player.getPlayerID())

            # Is it an admin message?
            elif cFlags.admin:
                if cFlags.private:
                    headerColor = kColors.ChatHeaderError
                    if cFlags.toSelf:
                        pretext = PtGetLocalizedString("KI.Chat.PrivateSendTo")
                    else:
                        pretext = PtGetLocalizedString("KI.Chat.PrivateMsgRecvd")
                    forceKI = True

                    # PM Processing: Save playerID and flash client window
                    self.lastPrivatePlayerID = (player.getPlayerName(), player.getPlayerID(), 0)
                    self.AddPlayerToRecents(player.getPlayerID())
                    PtFlashWindow()
                else:
                    headerColor = kColors.ChatHeaderAdmin
                    forceKI = True

            # Is it a broadcast message?
            elif cFlags.broadcast:
                if cFlags.toSelf:
                    headerColor = kColors.ChatHeaderBroadcast
                    pretext = PtGetLocalizedString("KI.Chat.BroadcastSendTo")
                else:
                    headerColor = kColors.ChatHeaderBroadcast
                    pretext = PtGetLocalizedString("KI.Chat.BroadcastMsgRecvd")
                    self.AddPlayerToRecents(player.getPlayerID())

                    # Are we mentioned in the message?
                    if message.lower().find(PtGetClientName().lower()) >= 0:
                        bodyColor = kColors.ChatMessageMention
                        forceKI = True
                        PtFlashWindow()

            # Is it a private message?
            elif cFlags.private:
                if cFlags.toSelf:
                    headerColor = kColors.ChatHeaderPrivate
                    pretext = PtGetLocalizedString("KI.Chat.PrivateSendTo")
                else:
                    if not self.CheckIfCanPM(player.getPlayerID()):
                        return
                    headerColor = kColors.ChatHeaderPrivate
                    pretext = PtGetLocalizedString("KI.Chat.PrivateMsgRecvd")
                    forceKI = True

                    # PM Processing: Save playerID and flash client window
                    self.lastPrivatePlayerID = (player.getPlayerName(), player.getPlayerID(), 0)
                    self.AddPlayerToRecents(player.getPlayerID())
                    PtFlashWindow()

        # Otherwise, cFlags is just a number.
        else:
            if cFlags == kChat.SystemMessage:
                headerColor = kColors.ChatHeaderError
                pretext = PtGetLocalizedString("KI.Chat.ErrorMsgRecvd")
            else:
                headerColor = kColors.ChatHeaderBroadcast
                pretext = PtGetLocalizedString("KI.Chat.BroadcastMsgRecvd")

        if forceKI:
            if not self.KIDisabled and not mKIdialog.isEnabled():
                mKIdialog.show()
        if player is not None:
            separator = "" if pretext.endswith(" ") else " "
            chatHeaderFormatted = U"{}{}{}:".format(pretext, separator, player.getPlayerNameW())
            chatMessageFormatted = U" {}".format(message)
        else:
            # It must be a status or error message.
            chatHeaderFormatted = pretext
            if not pretext:
                chatMessageFormatted = U"{}".format(message)
            else:
                chatMessageFormatted = U" {}".format(message)

        chatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatArea.beginUpdate()
        savedPosition = chatArea.getScrollPosition()
        wasAtEnd = chatArea.isAtEnd()
        chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
        chatArea.insertColor(headerColor)

        # Added unicode support here.
        chatArea.insertStringW(U"\n{}".format(chatHeaderFormatted))
        chatArea.insertColor(bodyColor)
        chatArea.insertStringW(chatMessageFormatted)
        chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)

        # Write to the log file.
        if self.chatLogFile is not None and self.chatLogFile.isOpen():
            self.chatLogFile.write(chatHeaderFormatted[0:] + chatMessageFormatted)

        # If the chat is overflowing, erase the first line.
        if chatArea.getBufferSize() > kChat.MaxChatSize:
            while chatArea.getBufferSize() > kChat.MaxChatSize and chatArea.getBufferSize() > 0:
                PtDebugPrint(u"xKIChat.AddChatLine(): Max chat buffer size reached. Removing top line.", level=kDebugDumpLevel)
                chatArea.deleteLinesFromTop(1)
                if savedPosition > 0:
                    # this is only accurate if the deleted line only occupied one line in the control (wasn't soft-wrapped), but that tends to be the usual case
                    savedPosition -= 1
        if not wasAtEnd:
            # scroll back to where we were
            chatArea.setScrollPosition(savedPosition)
            # flash the down arrow to indicate that new chat has come in
            self.incomingChatFlashState = 3
            PtAtTimeCallback(self.key, 0.0, kTimers.IncomingChatFlash)
        chatArea.endUpdate()

        # Copy all the data to the miniKI if the user upgrades it.
        if self.KILevel == kMicroKI:
            chatArea2 = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatArea2.beginUpdate()
            chatArea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatArea2.insertColor(headerColor)

            # Added unicode support here.
            chatArea2.insertStringW(U"\n{}".format(chatHeaderFormatted))
            chatArea2.insertColor(bodyColor)
            chatArea2.insertStringW(chatMessageFormatted)
            chatArea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)

            if chatArea2.getBufferSize() > kChat.MaxChatSize:
                while chatArea2.getBufferSize() > kChat.MaxChatSize and chatArea2.getBufferSize() > 0:
                    chatArea2.deleteLinesFromTop(1)
            chatArea2.endUpdate()

        # Update the fading controls.
        self.ResetFadeState()

    ## Display a status message to the player (or players if net-propagated).
    def DisplayStatusMessage(self, statusMessage, netPropagate=0):

        cFlags = ChatFlags(0)
        cFlags.toSelf = True
        cFlags.status = True
        if netPropagate:
            plyrList = self.GetPlayersInChatDistance()
            if len(plyrList) > 0:
                PtSendRTChat(PtGetLocalPlayer(), plyrList, statusMessage, cFlags.flags)
        self.AddChatLine(None, statusMessage, cFlags)

    ###########
    # Players #
    ###########

    ## Adds a player to the recent players folder.
    def AddPlayerToRecents(self, playerID):

        vault = ptVault()
        PIKAFolder = vault.getPeopleIKnowAboutFolder()
        if PIKAFolder:
            PIKA = PIKAFolder.upcastToPlayerInfoListNode()
            if PIKA is not None:
                if not PIKA.playerlistHasPlayer(playerID):
                    PIKA.playerlistAddPlayer(playerID)
                    childRefList = PIKAFolder.getChildNodeRefList()
                    numPeople = len(childRefList)
                    if numPeople > kLimits.MaxRecentPlayerListSize:
                        peopleToRemove = []
                        numToRemove = numPeople - kLimits.MaxRecentPlayerListSize
                        for i in range(numToRemove):
                            peopleToRemove.append(childRefList[i].getChild())
                        for person in peopleToRemove:
                            PIKAFolder.removeNode(person)

    ## Returns a list of players within chat distance.
    def GetPlayersInChatDistance(self, minPlayers=8):

        plyrList = []
        agePlayers = PtGetPlayerListDistanceSorted()
        for player in agePlayers:
            if player.getPlayerID() != 0:
                if player.getDistanceSq() < PtMaxListenDistSq():
                    plyrList.append(player)
                else:
                    if len(plyrList) <= minPlayers:
                        plyrList.append(player)
        return plyrList

    ## Find the player's online buddy or neighbor list.
    def GetOnlinePlayers(self, pList):

        outList = []
        for player in pList:
            if isinstance(player, ptVaultNodeRef):
                ePlayer = player.getChild().upcastToPlayerInfoNode()
                if ePlayer is not None:
                    if ePlayer.playerIsOnline():
                        outList.append(ptPlayer(ePlayer.playerGetName(), ePlayer.playerGetID()))
        return outList

    ## Determine if the player is a buddy, if PMs should come only through
    # them. Returns True if any player can send PMs.
    def CheckIfCanPM(self, playerID):

        if self.onlyGetPMsFromBuddies:
            buddies = ptVault().getBuddyListFolder()
            if buddies is not None:
                return buddies.playerlistHasPlayer(playerID)
            return False
        return True


## A object to hold and manipulate flags for a chat message.
class ChatFlags:

    # Create a container for the flags.
    def __init__(self, flags):

        self.__dict__["flags"] = flags

        self.__dict__["broadcast"] = True
        self.__dict__["toSelf"] = False

        if flags & kRTChatPrivate:
            self.__dict__["private"] = True
            self.__dict__["broadcast"] = False
        else:
            self.__dict__["private"] = False

        if flags & kRTChatAdmin:
            self.__dict__["admin"] = True
        else:
            self.__dict__["admin"] = False

        if flags & kRTChatGlobal:
            self.__dict__["ccrBcast"] = True
        else:
            self.__dict__["ccrBcast"] = False

        if flags & kRTChatInterAge:
            self.__dict__["interAge"] = True
        else:
            self.__dict__["interAge"] = False

        if flags & kRTChatStatusMsg:
            self.__dict__["status"] = True
        else:
            self.__dict__["status"] = False

        if flags & kRTChatNeighborsMsg:
            self.__dict__["neighbors"] = True
        else:
            self.__dict__["neighbors"] = False

        self.__dict__["channel"] = (kRTChatChannelMask & flags) / 256

    def __setattr__(self, name, value):

        if name == "broadcast" and value:
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatPrivate

        elif name == "ccrBcast":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatGlobal
            if value:
                self.__dict__["flags"] |= kRTChatGlobal
            else:
                self.__dict__["flags"] &= ~kRTChatGlobal

        elif name == "private":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatPrivate
            if value:
                self.__dict__["flags"] |= kRTChatPrivate
                self.__dict__["broadcast"] = False
            else:
                self.__dict__["broadcast"] = True

        elif name == "admin":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatAdmin
            if value:
                self.__dict__["flags"] |= kRTChatAdmin

        elif name == "interAge":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatInterAge
            if value:
                self.__dict__["flags"] |= kRTChatInterAge

        elif name == "status":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatStatusMsg
            if value:
                self.__dict__["flags"] |= kRTChatStatusMsg

        elif name == "neighbors":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatNeighborsMsg
            if value:
                self.__dict__["flags"] |= kRTChatNeighborsMsg

        elif name == "channel":
            flagsNoChannel = self.__dict__["flags"] & kRTChatNoChannel
            self.__dict__["flags"] = flagsNoChannel + (value * 256)

        self.__dict__[name] = value

    def __repr__(self):

        string = "ChatFlag: "
        if self.toSelf:
            string += "toSelf "
        if self.broadcast:
            string += "broadcast "
        if self.private:
            string += "private "
        if self.admin:
            string += "admin "
        if self.interAge:
            string += "interAge "
        if self.status:
            string += "status "
        if self.neighbors:
            string += "neighbors "
        if self.ccrBcast:
            string += "ccrBcast "
        string += "channel = {} ".format(self.channel)
        string += "flags = {}".format(self.flags)
        return string


## Processes KI Chat commands.
class CommandsProcessor:

    ## Link the processor with the chat manager.
    def __init__(self, chatMgr):

        self.chatMgr = chatMgr

    ## Called when the processor needs to process a message.
    # Returns the appropriate message and performs all the necessary operations
    # to apply the command.
    def __call__(self, message):

        msg = message.lower()

        # Load all available commands.
        commands = dict()
        commands.update(kCommands.Localized)
        if PtGetAgeName() == "Jalak":
            commands.update(kCommands.Jalak)
        if PtIsInternalRelease():
            commands.update(kCommands.Internal)
        commands.update(kCommands.EasterEggs)
        commands.update(kCommands.Other)

        # Does the message contain a standard command?
        for command, function in commands.iteritems():
            if msg.startswith(command):
                theMessage = message.split(" ", 1)
                if len(theMessage) > 1 and theMessage[1]:
                    params = theMessage[1]
                else:
                    params = None
                getattr(self, function)(params)
                return None

        # Is it a simple text-based command?
        for command, text in kCommands.Text.iteritems():
            if msg.startswith(command):
                self.chatMgr.AddChatLine(None, text, 0)
                return None

        # Is it another text-based easter-egg command?
        if msg.startswith("/get "):
            v = "is"
            if message[-1:] == "s":
                v = "are"
            self.chatMgr.AddChatLine(None, "The %s %s too heavy to lift. Maybe you should stick to feathers." % (message[len("/get "):], v), 0)
            return None
        elif PtIsInternalRelease() and msg.startswith("/system "):
            send = message[len("/system "):]
            cFlags = ChatFlags(0)
            cFlags.admin = 1
            cFlags.ccrBcast = 1
            cFlags.toSelf = 1
            PtSendRTChat(PtGetLocalPlayer(), [], send, cFlags.flags)
            fldr = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAllPlayersFolder)
            self.chatMgr.AddChatLine(ptPlayer(fldr, 0), send, cFlags)
            return None

        # Is it an emote, a "/me" or invalid command?
        if message.startswith("/"):
            words = message.split()
            try:
                emote = xKIExtChatCommands.xChatEmoteXlate[unicode(words[0][1:].lower())]
                if emote[0] in xKIExtChatCommands.xChatEmoteLoop:
                    PtAvatarEnterAnimMode(emote[0])
                else:
                    PtEmoteAvatar(emote[0])
                if PtGetLanguage() == PtLanguage.kEnglish:
                    avatar = PtGetLocalAvatar()
                    gender = avatar.avatar.getAvatarClothingGroup()
                    if gender > kFemaleClothingGroup:
                        gender = kMaleClothingGroup
                    hisHer = PtGetLocalizedString("KI.EmoteStrings.His")
                    if gender == kFemaleClothingGroup:
                        hisHer = PtGetLocalizedString("KI.EmoteStrings.Her")
                    statusMsg = PtGetLocalizedString(emote[1], [PtGetLocalPlayer().getPlayerName(), hisHer])
                else:
                    statusMsg = PtGetLocalizedString(emote[1], [PtGetLocalPlayer().getPlayerName()])
                self.chatMgr.DisplayStatusMessage(statusMsg, 1)
                message = message[len(words[0]):]
                if message == "":
                    return None
                return message[1:]
            except LookupError:
                try:
                    command = xKIExtChatCommands.xChatExtendedChat[unicode(words[0][1:].lower())]
                    if isinstance(command, str):
                        args = message[len(words[0]):]
                        PtConsole(command + args)
                    else:
                        try:
                            args = message[len(words[0]) + 1:]
                            if args:
                                try:
                                    retDisp = command(args)
                                except TypeError:
                                    retDisp = command()
                                    return args
                            else:
                                retDisp = command()
                            if isinstance(retDisp, unicode) or isinstance(retDisp, str):
                                self.chatMgr.DisplayStatusMessage(retDisp)
                            elif isinstance(retDisp, tuple):
                                if retDisp[0]:
                                    self.chatMgr.AddChatLine(None, retDisp[1], kChat.SystemMessage)
                                else:
                                    self.chatMgr.DisplayStatusMessage(retDisp[1])
                        except:
                            PtDebugPrint(u"xKIChat.commandsProcessor(): Chat command function did not run.", command, level=kErrorLevel)
                except LookupError:
                    if unicode(words[0].lower()) in xKIExtChatCommands.xChatSpecialHandledCommands:
                        return message
                    else:
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.CommandError", [message]), kChat.SystemMessage)
                return None

        return message

    ## Extract the player ID from a chat's params.
    def GetPID(self, params):

        if not params:
            return 0
        try:
            pID = long(int(params))
            return pID
        except ValueError:
            for player in self.chatMgr.BKPlayerList:
                if isinstance(player, ptPlayer):
                    plyrName = player.getPlayerName()
                    if params == plyrName:
                        return player.getPlayerID()
            return 0

    #~~~~~~~~~~~~~~~~~~~#
    # Standard Commands #
    #~~~~~~~~~~~~~~~~~~~#

    ## Add a player as a buddy.
    def AddBuddy(self, player):

        pID = self.GetPID(player)
        if pID:
            localPlayer = PtGetLocalPlayer()
            if pID != localPlayer.getPlayerID():
                vault = ptVault()
                buddies = vault.getBuddyListFolder()
                if buddies is not None:
                    if buddies.playerlistHasPlayer(pID):
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.AlreadyAdded"), kChat.SystemMessage)
                    else:
                        buddies.playerlistAddPlayer(pID)
                        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotYourself"), kChat.SystemMessage)
        else:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)

    ## Clear the chat.
    def ClearChat(self, params):

        chatAreaU = ptGUIControlMultiLineEdit(KIMicro.dialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatAreaM = ptGUIControlMultiLineEdit(KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatAreaU.clearBuffer()
        chatAreaM.clearBuffer()

    ## Ignores a player.
    def IgnorePlayer(self, player):

        pID = self.GetPID(player)
        if pID:
            localplayer = PtGetLocalPlayer()
            if pID != localplayer.getPlayerID():
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if ignores is not None:
                    if ignores.playerlistHasPlayer(pID):
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.AlreadyAdded"), kChat.SystemMessage)
                    else:
                        ignores.playerlistAddPlayer(pID)
                        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotYourself"), kChat.SystemMessage)
        else:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)

    ## Remove a player from this player's buddies.
    def RemoveBuddy(self, player):

        pID = self.GetPID(player)
        vault = ptVault()
        buddies = vault.getBuddyListFolder()
        if buddies is None:
            return

        # Is it a number?
        if pID:
            if buddies.playerlistHasPlayer(pID):
                buddies.playerlistRemovePlayer(pID)
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotFound"), kChat.SystemMessage)
        # Or is it a username?
        else:
            buddyRefs = buddies.getChildNodeRefList()
            for plyr in buddyRefs:
                if isinstance(plyr, ptVaultNodeRef):
                    PLR = plyr.getChild()
                    PLR = PLR.upcastToPlayerInfoNode()
                    if PLR is not None and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                        if player == PLR.playerGetName():
                            buddies.playerlistRemovePlayer(PLR.playerGetID())
                            self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
                            return
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)

    ## Start logging the chat.
    def StartLog(self, params):

        if self.chatMgr.chatLogFile is None:
            self.chatMgr.chatLogFile = ptStatusLog()
        self.chatMgr.chatLogFile.open("Chat.log", 30, int(PtStatusLogFlags.kAppendToLast) + int(PtStatusLogFlags.kTimestamp))
        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.LogStarted"))

    ## Stop logging the chat.
    def StopLog(self, params):

        if self.chatMgr.chatLogFile is not None:
            if self.chatMgr.chatLogFile.isOpen():
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.LogStopped"))
            self.chatMgr.chatLogFile.close()

    ## Remove a player from this player's ignore list.
    def UnignorePlayer(self, player):

        pID = self.GetPID(player)
        vault = ptVault()
        ignores = vault.getIgnoreListFolder()
        if ignores is None:
            return

        # Is it a number?
        if pID:
            if ignores.playerlistHasPlayer(pID):
                ignores.playerlistRemovePlayer(pID)
                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotFound"), kChat.SystemMessage)
        # Or is it a username?
        else:
            ignorerefs = ignores.getChildNodeRefList()
            for plyr in ignorerefs:
                if isinstance(plyr, ptVaultNodeRef):
                    PLR = plyr.getChild()
                    PLR = PLR.upcastToPlayerInfoNode()
                    if PLR is not None and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                        if player == PLR.playerGetName():
                            ignores.playerlistRemovePlayer(PLR.playerGetID())
                            self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
                            return
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)

    ## Dumps logs to the specified destination.
    def DumpLogs(self, destination):

        if not destination:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.MalformedLogDumpCmd"), kChat.SystemMessage)
            return
        destination = destination.strip()
        currentTime = time.strftime("%d %b %Y %H:%M:%S (GMT)", time.gmtime())
        PtDebugPrint(u"-- Logs dumped to \"{}\" at {}. --".format(destination, currentTime))
        # Use a timer to allow for a final message to be logged.
        self.chatMgr.logDumpDest = destination  # So the timer can get at it.
        PtAtTimeCallback(self.chatMgr.key, 0.25, kTimers.DumpLogs)

    ## Changes the user's password.
    def ChangePassword(self, newPassword):

        if not newPassword:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.BadPassword"), kChat.SystemMessage)
            return
        newPassword = newPassword.strip()
        if len(newPassword) > 15:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.PasswordTooLong"), kChat.SystemMessage)
            return
        PtChangePassword(newPassword)

    #~~~~~~~~~~~~~~~~#
    # Jalak Commands #
    #~~~~~~~~~~~~~~~~#

    ## Save the player's current Jalak columns to a file.
    def SaveColumns(self, columnsFile):

        if columnsFile:
            fName = columnsFile.strip()
            fName = fName + ".txt"
        else:
            fName = "JalakColumns.txt"
        obj = PtFindSceneobject("JalakDONOTTOUCH", "Jalak")
        pythonScripts = obj.getPythonMods()
        for script in pythonScripts:
            if script.getName() == kJalakPythonComponent:
                PtDebugPrint(u"xKIChat.SaveColumns(): Found Jalak's python component.", level=kDebugDumpLevel)
                SendNote(self.chatMgr.key, script, "SaveColumns;" + fName)
                return
        PtDebugPrint(u"xKIChat.SaveColumns(): Did not find Jalak's python component.", level=kErrorLevel)

    ## Load the player's Jalak columns from a file.
    def LoadColumns(self, columnsFile):

        if columnsFile:
            fName = columnsFile.strip()
            fName = fName + ".txt"
        else:
            fName = "JalakColumns.txt"
        obj = PtFindSceneobject("JalakDONOTTOUCH", "Jalak")
        pythonScripts = obj.getPythonMods()
        for script in pythonScripts:
            if script.getName() == kJalakPythonComponent:
                PtDebugPrint(u"xKIChat.LoadColumns(): Found Jalak's python component.", level=kDebugDumpLevel)
                SendNote(self.chatMgr.key, script, "LoadColumns;" + fName)
                return
        PtDebugPrint(u"xKIChat.LoadColumns(): Did not find Jalak's python component.", level=kErrorLevel)

    #~~~~~~~~~~~~~~~~~~~#
    # Internal Commands #
    #~~~~~~~~~~~~~~~~~~~#

    ## Reset the Cleft to its default setting.
    def RevisitCleft(self, params):

        vault = ptVault()
        chron = vault.findChronicleEntry("CleftSolved")
        if chron is not None:
            chronFolder = vault.getChronicleFolder()
            if chronFolder is not None:
                chronFolder.removeNode(chron)

    ## Restart the game with key points reset.
    def RestartGame(self, params):

        vault = ptVault()
        chron = vault.findChronicleEntry("InitialAvCustomizationsDone")
        if chron is not None:
            chronFolder = vault.getChronicleFolder()
            if chronFolder is not None:
                chronFolder.removeNode(chron)
        chron = vault.findChronicleEntry("IntroPlayed")
        if chron is not None:
            chronFolder = vault.getChronicleFolder()
            if chronFolder is not None:
                chronFolder.removeNode(chron)
        chron = vault.findChronicleEntry("CleftSolved")
        if chron is not None:
            chronFolder = vault.getChronicleFolder()
            if chronFolder is not None:
                chronFolder.removeNode(chron)

    #~~~~~~~~~~~~~~~~~~~~~~#
    # Easter Eggs Commands #
    #~~~~~~~~~~~~~~~~~~~~~~#

    ## Look around for exits and informational text.
    def LookAround(self, params):

        # Find the nearby people.
        playerList = self.chatMgr.GetPlayersInChatDistance(minPlayers=-1)
        people = "nobody in particular"
        if len(playerList) > 0:
            people = ""
            for player in playerList:
                people += player.getPlayerName() + ", "
            people = people[:-2]

        # Load the Age-specific text.
        ageInfo = PtGetAgeInfo()
        if ageInfo is None:
            return
        currentAge = ageInfo.getAgeFilename()
        see = ""
        exits = " North and West."
        if currentAge in kEasterEggs:
            see = kEasterEggs[currentAge]["see"]
            if not kEasterEggs[currentAge]["exits"]:
                exits = "... well, there are no exits."
            else:
                exits = " " + kEasterEggs[currentAge]["exits"]
            if "people" in kEasterEggs[currentAge]:
                people = kEasterEggs[currentAge]["people"]

        ## Display the info.
        self.chatMgr.AddChatLine(None, "{}: {} Standing near you is {}. There are exits to the{}".format(GetAgeName(), see, people, exits), 0)

    ## Get a feather in the current Age.
    def GetFeather(self, params):

        ageInfo = PtGetAgeInfo()
        if ageInfo is None:
            return
        currentAge = ageInfo.getAgeFilename()
        if currentAge == "Gira":
            if self.chatMgr.gFeather < 7:
                self.chatMgr.AddChatLine(None, "You pick up a plain feather and put it in your pocket. I know you didn't see yourself do that... trust me, you have a feather in your pocket.", 0)
                self.chatMgr.gFeather += 1
            else:
                self.chatMgr.AddChatLine(None, "You can only carry seven plain feathers.", 0)
        elif currentAge == "EderDelin":
            if self.chatMgr.gFeather == 7:
                self.chatMgr.AddChatLine(None, "You search... and find the \"Red\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 7:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing seven plain feathers.", 0)
        elif currentAge == "Dereno":
            if self.chatMgr.gFeather == 8:
                self.chatMgr.AddChatLine(None, "You search... and find the \"Blue\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 8:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Red\" feather.", 0)
        elif currentAge == "Payiferen":
            if self.chatMgr.gFeather == 9:
                self.chatMgr.AddChatLine(None, "You search... and find the \"Black\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 9:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Blue\" feather.", 0)
        elif currentAge == "Ercana":
            if self.chatMgr.gFeather == 10:
                self.chatMgr.AddChatLine(None, "You search... and find the \"Silver\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 10:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Black\" feather.", 0)
        elif currentAge == "Jalak":
            if self.chatMgr.gFeather == 11:
                self.chatMgr.AddChatLine(None, "You search... and find the \"Duck\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 11:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Silver\" feather.", 0)
        elif currentAge == "Ahnonay":
            if self.chatMgr.gFeather == 12:
                self.chatMgr.AddChatLine(None, "You search... and find a large \"Rukh\" feather (how could you have missed it?) and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 12:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Duck\" feather.", 0)
        else:
            self.chatMgr.AddChatLine(None, "There are no feathers here.", 0)
            return
        vault = ptVault()
        entry = vault.findChronicleEntry("feather")
        if entry is None:
            vault.addChronicleEntry("feather", 1, str(self.chatMgr.gFeather))
            entry = vault.findChronicleEntry("feather")
        entry.chronicleSetValue(str(self.chatMgr.gFeather))
        entry.save()

    ## Looks for feathers in the player's "inventory".
    def LookForFeathers(self, params):

        if self.chatMgr.gFeather:
            if self.chatMgr.gFeather == 1:
                self.chatMgr.AddChatLine(None, "You see a feather!", 0)
            else:
                pFeathers = self.chatMgr.gFeather
                if pFeathers > 7:
                    pFeathers = 7
                pOut = "You see {} plain feathers".format(pFeathers)
                if self.chatMgr.gFeather > 7:
                    pOut += " and a \"Red\" feather"
                if self.chatMgr.gFeather > 8:
                    pOut += " and a \"Blue\" feather"
                if self.chatMgr.gFeather > 9:
                    pOut += " and a \"Black\" feather"
                if self.chatMgr.gFeather > 10:
                    pOut += " and a \"Silver\" feather"
                if self.chatMgr.gFeather > 11:
                    pOut += " and a \"Duck\" feather"
                if self.chatMgr.gFeather > 12:
                    pOut += " and a large \"Rukh\" feather (sticking out of your pocket)"
                pOut += "."
                self.chatMgr.AddChatLine(None, pOut, 0)
        else:
            self.chatMgr.AddChatLine(None, "There is nothing there but lint.", 0)

    ##################
    # Other Commands #
    ##################

    def PartyTime(self, params):
        """Implements the `/party` command"""

        # First, find the PartyAge chronicle in the global inbox
        party = None
        vault = ptVault()
        inbox = vault.getGlobalInbox()
        for childRef in inbox.getChildNodeRefList():
            child = childRef.getChild()
            if not child:
                continue
            child = child.upcastToChronicleNode()
            if not child:
                continue
            if child.chronicleGetName() == kChron.Party:
                party = child
                break

        # Let's see what we need to do
        if not params:
            # No params = LINK ME!
            if party and party.chronicleGetValue():
                data = party.chronicleGetValue().split(";", 3)
                ageInfo = ptAgeInfoStruct()
                ageInfo.setAgeFilename(data[0])
                ageInfo.setAgeInstanceGuid(data[1])

                ageLink = ptAgeLinkStruct()
                ageLink.setAgeInfo(ageInfo)
                ageLink.setLinkingRules(PtLinkingRules.kBasicLink)
                ageLink.setSpawnPoint(ptSpawnPointInfo(data[2], data[2]))

                # Player is not really linking--this is an OOC cheat.
                nlm = ptNetLinkingMgr()
                nlm.linkToAge(ageLink, linkInSfx=False, linkOutSfx=False)
            else:
                self.chatMgr.AddChatLine(None, "There is no party to crash!", kChat.SystemMessage)
                return
        elif PtIsInternalRelease():
            try:
                PtFindSceneobject(params, PtGetAgeName())
            except NameError:
                # Garbage SO = kill party
                if party:
                    party.chronicleSetValue("")
                    party.save()
                    self.chatMgr.AddChatLine(None, "Party Crashed.", 0)
                else:
                    self.chatMgr.AddChatLine(None, "No party. Your link-in-point is invalid.", kChat.SystemMessage)
            else:
                # Got a LIP... Need to set the chronicle
                ageInfo = PtGetAgeInfo()
                data = "%s;%s;%s" % (ageInfo.getAgeFilename(), ageInfo.getAgeInstanceGuid(), params)
                if not party:
                    party = ptVaultChronicleNode()
                    party.chronicleSetName(kChron.Party)
                    party.chronicleSetValue(data)
                    party.save() # creates node on server (and blocks) so we can add it to the global inbox
                    inbox.addNode(party)
                else:
                    party.chronicleSetValue(data)
                    party.save()

    ## Export the local avatar's clothing to a file
    def SaveClothing(self, file):
        if not file:
            self.chatMgr.AddChatLine(None, "Usage: /loadclothing <name>", kChat.SystemMessage)
            return
        file = file + ".clo"
        if PtGetLocalAvatar().avatar.saveClothingToFile(file):
            self.chatMgr.AddChatLine(None, "Outfit exported to " + file, 0)
        else:
            self.chatMgr.AddChatLine(None, "Could not export to " + file, kChat.SystemMessage)

    ## Import the local avatar's clothing from a file
    def LoadClothing(self, file):
        if not file:
            self.chatMgr.AddChatLine(None, "Usage: /loadclothing <name>", kChat.SystemMessage)
            return
        if PtGetPlayerList() and not PtIsInternalRelease():
            self.chatMgr.AddChatLine(None, "You have to be alone to change your clothes!", kChat.SystemMessage)
            return
        file = file + ".clo"
        if PtGetLocalAvatar().avatar.loadClothingFromFile(file):
            self.chatMgr.AddChatLine(None, "Outfit imported from " + file, 0)
        else:
            self.chatMgr.AddChatLine(None, file + " not found", kChat.SystemMessage)

    ## Example function for a coop animation
    def CoopExample(self, name):
        if not name:
            self.chatMgr.AddChatLine(None, "Usage: /threaten <playername>", kChat.SystemMessage)
            return
        targetKey = None;
        for player in PtGetPlayerList():
            if player.getPlayerName().lower() == name.lower():
                name = player.getPlayerName()
                targetKey = PtGetAvatarKeyFromClientID(player.getPlayerID())
                break
        if targetKey is None:
            self.chatMgr.AddChatLine(None, name + " not found", kChat.SystemMessage)
            return
        if PtGetLocalAvatar().avatar.runCoopAnim(targetKey, "ShakeFist", "Cower"):
            self.chatMgr.DisplayStatusMessage(PtGetClientName() + " threatens " + name, 1)
        else:
            self.chatMgr.AddChatLine(None, "You are too far away", kChat.SystemMessage)

    ## Sets a reward for the current marker game
    def MarkerGameReward(self, reward):
        markerMgr = self.chatMgr.markerGameManager
        if not markerMgr.is_game_loaded or not hasattr(markerMgr, "reward"):
            self.chatMgr.AddChatLine(None, "There is no active marker game that can grant rewards.", kChat.SystemMessage)
            return
        if reward:
            self.chatMgr.AddChatLine(None, "Old Reward: '{}'".format(markerMgr.reward), 0)
            try:
                markerMgr.reward = reward
            except:
                self.chatMgr.AddChatLine(None, "Failed to set reward.", kChat.SystemMessage)
                raise
            else:
                self.chatMgr.AddChatLine(None, "New Reward: '{}'".format(reward), 0)
        elif markerMgr.reward:
            self.chatMgr.AddChatLine(None, "Current Reward: '{}'".format(markerMgr.reward), 0)
        else:
            self.chatMgr.AddChatLine(None, "This game has no associated reward.", 0)
