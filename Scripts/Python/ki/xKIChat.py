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
import random
import inspect

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
        self.chatTextColor = None

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

        # Set a fake player name to avoid errors.
        self.playerName = None

        # Message History
        self.MessageHistoryIs = -1 # Current position in message history (up/down key)
        self.MessageHistoryList = [] # Contains our message history
        self.MessageCurrentLine = "" # Hold current line while navigating message history

    @property
    def chatArea(self):
        if self.KILevel < kNormalKI:
            return self.microChatArea
        else:
            return self.miniChatArea

    #######
    # GUI #
    #######

    ## Toggles the state of the miniKI icon in the Blackbar.
    # Changing the value will initiate a chain reaction of events to display
    # the miniKI.
    def ClearBBMini(self, value=-1):

        if self.KILevel == kNormalKI:
            mmRG = KIBlackbar.dialog.getControlModFromTag(kGUI.MiniMaximizeRGID)
            mmRG.setValue(value)

    ## Check if the chat is faded out.
    def IsFaded(self):

        if self.KILevel >= kMicroKI:
            if not BigKI.dialog.isEnabled() and (KIMini.dialog.isEnabled() or KIMicro.dialog.isEnabled()):
                if self.fadeMode == kChat.FadeNotActive:
                    return True
        return False

    ############
    # Chatting #
    ############

    # Update the current player name.
    def _setPlayerName(self, value):
        value = re.escape(value if value else "Anonymous Coward")

        # (?:^|[\s\W](?<!\/\/|\w\.)) - non-capturing group: line start or whitespace or non-word character
        #                              with lookbehind that excludes matches preceded by // or word character and .
        #                              trying to prevent mentions that are part of a URL
        # (?P<mention>({value}\s?)+) - named capture group: one or more occurrence of name, optionally split by a space
        # (?=$|[\s\W])               - lookahead ensures match is followed by line end or whitespace or non-word character
        regex = rf"(?:^|[\s\W](?<!\/\/|\w\.))(?P<mention>({value}\s?)+)(?=$|[\s\W])"
        PtDebugPrint(f"xKIChat: The chat mention regex is now `{regex}`", level = kWarningLevel)
        self._chatMentionRegex = re.compile(regex, re.IGNORECASE)
    playerName = property(None, _setPlayerName)

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
        caret = mKIdialog.getControlModFromTag(kGUI.ChatCaretID)
        chatEdit = mKIdialog.getControlModFromTag(kGUI.ChatEditboxID)
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
        msg = message.casefold()

        # Get any selected players.
        userListBox = KIMini.dialog.getControlModFromTag(kGUI.PlayerList)
        iSelect = userListBox.getSelection()
        selPlyrList = []

        # Is it a reply to a private message?
        if msg.startswith(PtGetLocalizedString("KI.Commands.ChatReply")) or msg.startswith("/r "):
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
                if PIKA is not None and PIKA.hasPlayer(self.toReplyToLastPrivatePlayerID[1]):
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
            if (msg.startswith("/r ")):
                message = message[len("/r "):]
            else:
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
            if len(pWords) > 1 and pWords[0].casefold() == PtGetLocalizedString("KI.Commands.ChatPrivate"):
                # Try to find the buddy in the DPL online lists.
                for player in self.BKPlayerList:
                    # Is the player in this Age?
                    if isinstance(player, ptPlayer):
                        plyrName = player.getPlayerName().casefold()
                        if pWords[1].casefold().startswith(plyrName + " "):
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
                            plyrName = ePlyr.playerGetName().casefold()
                            if pWords[1].casefold().startswith(plyrName + " "):
                                selPlyrList.append(ptPlayer(ePlyr.playerGetName(), ePlyr.playerGetID()))
                                cFlags.private = True
                                cFlags.interAge = True
                                foundBuddy = True
                                # Add this player's current Age.
                                message = pre + pWords[1][len(plyrName) + 1:]
                                self.AddPlayerToRecents(ePlyr.playerGetID())
                                break
            if not foundBuddy:
                PtDebugPrint("xKIChat.SendMessage(): \"/p\" command can't find player.", level=kDebugDumpLevel)
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
                    PtDebugPrint("xKIChat.SendMessage(): Private message to \"{}\".".format(toPlyr.getPlayerName()), level=kDebugDumpLevel)

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
                    fldrType = toPlyr.getFolderType()
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
            PtDebugPrint("xKIChat.AddChatLine(): Message = \"{}\".".format(message), player, cFlags, level=kDebugDumpLevel)
        except UnicodeEncodeError:
            pass

        # Fix for Character of Doom (CoD).
        (message, RogueCount) = re.subn("[\x00-\x08\x0a-\x1f]", "", message)

        if self.KILevel == kMicroKI:
            mKIdialog = KIMicro.dialog
        else:
            mKIdialog = KIMini.dialog
        pretext = ""
        headerColor = kColors.ChatHeaderBroadcast
        bodyColor = kColors.ChatMessage
        mentionColor = kColors.ChatMessageMention
        contextPrefix = ""
        censorLevel = self.GetCensorLevel()
        hasMention = False

        # Is it an object to represent the flags?
        if isinstance(cFlags, ChatFlags):
            # Is it subtitles for current audio?
            if cFlags.subtitle:
                headerColor = kColors.AudioSubtitleHeader
                contextPrefix = PtGetLocalizedString("KI.Chat.SubtitleContextPrefix")
                if player is not None:
                    # add subtitle speaker's name if it was provided
                    # add any leading pretext to match broadcast player messages
                    pretext = f"{PtGetLocalizedString('KI.Chat.BroadcastMsgRecvd')}{player}"
                player = None

            # Is it a status message?
            elif cFlags.status:
                bodyColor = kColors.ChatHeaderStatus
                player = None

            # Is it an inter-Age message?
            elif cFlags.interAge:
                if cFlags.private:
                    if cFlags.admin:
                        headerColor = kColors.ChatHeaderError
                        contextPrefix = PtGetLocalizedString("KI.Chat.AdminContextPrefix")
                    else:
                        headerColor = kColors.ChatHeaderPrivate
                        contextPrefix = PtGetLocalizedString("KI.Chat.PrivateContextPrefix")
                    forceKI = True
                else:
                    if cFlags.neighbors:
                        headerColor = kColors.ChatHeaderNeighbors
                        contextPrefix = PtGetLocalizedString("KI.Chat.NeighborsContextPrefix")
                    else:
                        headerColor = kColors.ChatHeaderBuddies
                        contextPrefix = PtGetLocalizedString("KI.Chat.BuddiesContextPrefix")

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
                                    if not buddies.hasPlayer(buddyID):
                                        PtDebugPrint("xKIChat.AddChatLine(): Add unknown buddy {} to recents.".format(buddyID))
                                        self.AddPlayerToRecents(buddyID)
                        except ValueError:
                            pass

                    # PM Processing: Save playerID and flash client window
                    if cFlags.private:
                        self.lastPrivatePlayerID = (player.getPlayerName(), player.getPlayerID(), 1)
                        PtFlashWindow()
                    # Are we mentioned in the message?
                    elif self._chatMentionRegex.search(message) is not None:
                        hasMention = True
                        contextPrefix = PtGetLocalizedString("KI.Chat.MentionContextPrefix")
                        PtFlashWindow()

            # Is it a ccr broadcast?
            elif cFlags.ccrBcast:
                headerColor = kColors.ChatHeaderAdmin
                contextPrefix = PtGetLocalizedString("KI.Chat.CCRContextPrefix")
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
                    contextPrefix = PtGetLocalizedString("KI.Chat.AdminContextPrefix")
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
                    contextPrefix = PtGetLocalizedString("KI.Chat.AdminContextPrefix")
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
                    if self._chatMentionRegex.search(message) is not None:
                        hasMention = True
                        contextPrefix = PtGetLocalizedString("KI.Chat.MentionContextPrefix")
                        forceKI = True
                        PtFlashWindow()

            # Is it a private message?
            elif cFlags.private:
                if cFlags.toSelf:
                    headerColor = kColors.ChatHeaderPrivate
                    contextPrefix = PtGetLocalizedString("KI.Chat.PrivateContextPrefix")
                    pretext = PtGetLocalizedString("KI.Chat.PrivateSendTo")
                else:
                    if not self.CheckIfCanPM(player.getPlayerID()):
                        return
                    headerColor = kColors.ChatHeaderPrivate
                    contextPrefix = PtGetLocalizedString("KI.Chat.PrivateContextPrefix")
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
                contextPrefix = PtGetLocalizedString("KI.Chat.ErrorContextPrefix")
                pretext = PtGetLocalizedString("KI.Chat.ErrorMsgRecvd")
            elif cFlags == kChat.AudioSubtitle:
                headerColor = kColors.AudioSubtitleHeader
                contextPrefix = PtGetLocalizedString("KI.Chat.SubtitleContextPrefix")
                if player is not None:
                    # add subtitle speaker's name if it was provided
                    # add any leading pretext to match broadcast player messages
                    pretext = f"{PtGetLocalizedString('KI.Chat.BroadcastMsgRecvd')}{player}"
                player = None
            else:
                headerColor = kColors.ChatHeaderBroadcast
                pretext = PtGetLocalizedString("KI.Chat.BroadcastMsgRecvd")

        if forceKI:
            if not self.KIDisabled and not mKIdialog.isEnabled():
                mKIdialog.show()
        if player is not None:
            separator = "" if not pretext or pretext.endswith(" ") else " "
            chatHeaderFormatted = "{}{}{}:".format(pretext, separator, player.getPlayerName())
            chatMessageFormatted = " {}".format(message)
        else:
            # It must be a subtitle, status or error message.
            chatHeaderFormatted = pretext
            if not pretext:
                chatMessageFormatted = "{}".format(message)
            else:
                chatMessageFormatted = " {}".format(message)

        if hasMention:
            chatMentions = [(i.start("mention"), i.end("mention"), i.group("mention")) for i in self._chatMentionRegex.finditer(chatMessageFormatted)]
        else:
            chatMentions = []

        # if we have an override chat color set, use it instead of the various default chat colors
        if self.chatTextColor:
            headerColor = self.chatTextColor
            bodyColor = self.chatTextColor
            mentionColor = self.chatTextColor

        for chatArea in (self.miniChatArea, self.microChatArea):
            with PtBeginGUIUpdate(chatArea):
                savedPosition = chatArea.getScrollPosition()
                wasAtEnd = chatArea.isAtEnd()
                chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
                chatArea.insertColor(headerColor)
                chatArea.insertString(f"\n{contextPrefix if self.chatTextColor else ''}{chatHeaderFormatted}")
                chatArea.insertColor(bodyColor)

                lastInsert = 0

                # If we have player name mentions, we change text colors mid-message
                for start, end, mention in chatMentions:
                    if start > lastInsert:
                        # Insert normal text up to the current name mention position
                        chatArea.insertString(chatMessageFormatted[lastInsert:start], censorLevel=censorLevel)

                    lastInsert = end
                    
                    chatArea.insertColor(mentionColor)
                    chatArea.insertString(mention, censorLevel=censorLevel, urlDetection=False)
                    chatArea.insertColor(bodyColor)

                # If there is remaining text to display after last mention, write it
                # Or if it was just a plain message with no mention of player's name
                if lastInsert != len(chatMessageFormatted):
                    chatArea.insertString(chatMessageFormatted[lastInsert:], censorLevel=censorLevel)

                chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)

                # If the chat is overflowing, erase the first line.
                if chatArea.getBufferSize() > kChat.MaxChatSize:
                    while chatArea.getBufferSize() > kChat.MaxChatSize and chatArea.getBufferSize() > 0:
                        PtDebugPrint("xKIChat.AddChatLine(): Max chat buffer size reached. Removing top line.", level=kDebugDumpLevel)
                        chatArea.deleteLinesFromTop(1)
                        if savedPosition > 0:
                            # this is only accurate if the deleted line only occupied one line in the control (wasn't soft-wrapped), but that tends to be the usual case
                            savedPosition -= 1

                # Presentation options for the current KI Chat Area
                if not wasAtEnd and chatArea == self.chatArea:
                    # scroll back to where we were
                    chatArea.setScrollPosition(savedPosition)
                    # flash the down arrow to indicate that new chat has come in
                    self.incomingChatFlashState = 3
                    PtAtTimeCallback(self.key, 0.0, kTimers.IncomingChatFlash)

        # Write to the log file.
        if self.chatLogFile is not None and self.chatLogFile.isOpen():
            self.chatLogFile.write(f"{contextPrefix}{chatHeaderFormatted[0:]}{chatMessageFormatted}")

        # Update the fading controls.
        self.ResetFadeState()

    ## Display a status message to the player (or players if net-propagated).
    def DisplayStatusMessage(self, message, netPropagate=0):

        cFlags = ChatFlags(0)
        cFlags.toSelf = True
        cFlags.status = True
        cFlags.lockey = isinstance(message, LocKey)

        localPlayer = PtGetLocalPlayer()

        if netPropagate:
            plyrList = self.GetPlayersInChatDistance()
            if len(plyrList) > 0:
                PtSendRTChat(localPlayer, plyrList, message if not cFlags.lockey else ":".join(message), cFlags.flags)

        # the message is just a localization key and needs processing before display
        if cFlags.lockey:
            message = PtGetLocalizedString(message.message, [localPlayer.getPlayerName(), PtGetLocalizedString(message.pronoun)])

        self.AddChatLine(None, message, cFlags)

    ###########
    # Players #
    ###########

    ## Adds a player to the recent players folder.
    def AddPlayerToRecents(self, playerID):

        PIKAFolder = ptVault().getPeopleIKnowAboutFolder()
        if PIKAFolder:
            PIKA = PIKAFolder.upcastToPlayerInfoListNode()
            if PIKA is not None:
                if not PIKA.hasPlayer(playerID):
                    PIKA.addPlayer(playerID)
                    childRefList = PIKAFolder.getChildNodeRefList()
                    numPeople = len(childRefList)
                    if numPeople > kLimits.MaxRecentPlayerListSize:
                        peopleToRemove = []
                        numToRemove = numPeople - kLimits.MaxRecentPlayerListSize
                        for i in range(numToRemove):
                            peopleToRemove.append(childRefList[i].getChild())
                        for person in peopleToRemove:
                            PIKAFolder.removeNode(person)
                    return True
        return False

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
                return buddies.hasPlayer(playerID)
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

        if flags & kRTChatAudioSubtitleMsg:
            self.__dict__["subtitle"] = True
        else:
            self.__dict__["subtitle"] = False

        if flags & kRTChatLocKeyMsg:
            self.__dict__["lockey"] = True
        else:
            self.__dict__["lockey"] = False

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

        elif name == "subtitle":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatAudioSubtitleMsg
            if value:
                self.__dict__["flags"] |= kRTChatAudioSubtitleMsg

        elif name == "lockey":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatLocKeyMsg
            if value:
                self.__dict__["flags"] |= kRTChatLocKeyMsg

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
        if self.subtitle:
            string += "subtitle "
        if self.lockey:
            string += "lockey "
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

        msg = message.casefold()

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
        for command, function in commands.items():
            if msg.startswith(command):
                callableCommandFn = getattr(self, function)
                numParams = len(inspect.signature(callableCommandFn).parameters)
                PtDebugPrint(f"xKI.CommandsProcessor: Processing {command} function with {numParams} parameters", level=kDebugDumpLevel)
                theMessage = message.split(" ", numParams)
                if len(theMessage) > 1 and theMessage[1]:
                    callableCommandFn(*theMessage[1:])
                else:
                    callableCommandFn(None)
                return None

        # Is it a simple text-based command?
        for command, text in kCommands.Text.items():
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
            # get the command word, trimming the / off the front
            commandWord = words[0][1:].casefold()
            if not commandWord or commandWord.isspace():
                # no command after the /, so short-circuit trying to do or send anything
                return None

            try:
                emote = xKIExtChatCommands.xChatEmoteXlate[commandWord]
                if emote[0] in xKIExtChatCommands.xChatEmoteLoop:
                    PtAvatarEnterAnimMode(emote[0])
                else:
                    PtEmoteAvatar(emote[0])

                pronounKey = xLocTools.GetLocalAvatarPossessivePronounLocKey()
                self.chatMgr.DisplayStatusMessage(LocKey(emote[1], pronounKey), netPropagate=True)

                # Get remaining message string after emote command
                message = message[len(words[0]):]
                if message == "" or message.isspace():
                    return None
                return message[1:]
            except LookupError:
                try:
                    command = xKIExtChatCommands.xChatExtendedChat[commandWord]
                    if isinstance(command, str):
                        # Retrieved command is just a plain string
                        args = message[len(words[0]):]
                        PtConsole(command + args)
                    else:
                        # Retrieved command is not a string (it's a function)
                        try:
                            # Get remaining message string after chat command
                            args = message[len(words[0]) + 1:]
                            if args:
                                try:
                                    retDisp = command(args)
                                except TypeError:
                                    retDisp = command()

                                    # Command took no args, so return args as new message
                                    if args == "" or args.isspace():
                                        return None
                                    return args
                            else:
                                retDisp = command()

                            # Check type of return value from command and display appropriately
                            if isinstance(retDisp, str):
                                self.chatMgr.DisplayStatusMessage(retDisp)
                            elif isinstance(retDisp, tuple):
                                if retDisp[0]:
                                    self.chatMgr.AddChatLine(None, retDisp[1], kChat.SystemMessage)
                                else:
                                    self.chatMgr.DisplayStatusMessage(retDisp[1])
                        except:
                            PtDebugPrint("xKIChat.commandsProcessor(): Chat command function did not run.", command, level=kErrorLevel)
                except LookupError:
                    firstWordLower = words[0].casefold()
                    if firstWordLower in xKIExtChatCommands.xChatSpecialHandledCommands:
                        # Get remaining message string after special chat command
                        remainingMsg = message[len(words[0]):]

                        # If remaining message is empty, try to do mousefree KI folder selection instead
                        if remainingMsg == "" or remainingMsg.isspace():
                            userListBox = KIMini.dialog.getControlModFromTag(kGUI.PlayerList)
                            caret = KIMini.dialog.getControlModFromTag(kGUI.ChatCaretID)
                            privateChbox = KIMini.dialog.getControlModFromTag(kGUI.miniPrivateToggle)

                            # Handling for selecting Age Players, Buddies, or Neighbors
                            folderName = None
                            if firstWordLower == PtGetLocalizedString("KI.Commands.ChatAge"):
                                folderName = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kAgeMembersFolder)
                                caretValue = ">"
                            elif firstWordLower == PtGetLocalizedString("KI.Commands.ChatBuddies"):
                                folderName = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kBuddyListFolder)
                                caretValue = PtGetLocalizedString("KI.Chat.TOPrompt") + folderName + " >"
                            elif firstWordLower == PtGetLocalizedString("KI.Commands.ChatNeighbors"):
                                folderName = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)
                                caretValue = PtGetLocalizedString("KI.Chat.TOPrompt") + folderName + " >"

                            # Only try to find in list if it was one of the 3 expected folders, not a reply command
                            if folderName is not None:
                                try:
                                    folderIdx = next((i for i in range(userListBox.getNumElements()) if userListBox.getElement(i).casefold() == folderName.casefold()))
                                except StopIteration:
                                    # Indicate an error to the user here because the KI folder was not found for some reason.
                                    self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.CommandError", [message]), kChat.SystemMessage)
                                    pass
                                else:
                                    userListBox.setSelection(folderIdx)
                                    caret.setString(caretValue)
                                    privateChbox.setChecked(False)
                                
                            # Don't send an actual message because it was just a command with nothing after it
                            return None

                        # Return full message with special command still prefixed
                        return message
                    else:
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.CommandError", [message]), kChat.SystemMessage)
                return None

        # Prevent sending blank/whitespace message
        if message == "" or message.isspace():
            return None
        return message

    ## Extract the player ID from a chat's params.
    def GetPID(self, params):

        if not params:
            return 0
        try:
            return int(params)
        except ValueError:
            for player in self.chatMgr.BKPlayerList:
                # Age Players
                if isinstance(player, ptPlayer):
                    plyrName = player.getPlayerName().casefold()
                    if params.casefold() == plyrName:
                        return player.getPlayerID()
                # Buddies or Neighbors
                elif isinstance(player, ptVaultNodeRef):
                    plyrInfoNode = player.getChild().upcastToPlayerInfoNode()
                    if plyrInfoNode is not None:
                        plyrName = plyrInfoNode.playerGetName().casefold()
                        if params.casefold() == plyrName:
                            return plyrInfoNode.playerGetID()
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
                    if buddies.hasPlayer(pID):
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.AlreadyAdded"), kChat.SystemMessage)
                    else:
                        buddies.addPlayer(pID)
                        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotYourself"), kChat.SystemMessage)
        else:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)

    ## Clear the chat.
    def ClearChat(self, params):
        self.chatMgr.miniChatArea.clearBuffer()
        self.chatMgr.microChatArea.clearBuffer()

    ## Ignores a player.
    def IgnorePlayer(self, player):

        pID = self.GetPID(player)
        if pID:
            localplayer = PtGetLocalPlayer()
            if pID != localplayer.getPlayerID():
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if ignores is not None:
                    if ignores.hasPlayer(pID):
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.AlreadyAdded"), kChat.SystemMessage)
                    else:
                        ignores.addPlayer(pID)
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
            if buddies.hasPlayer(pID):
                buddies.removePlayer(pID)
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
                            buddies.removePlayer(PLR.playerGetID())
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
            if ignores.hasPlayer(pID):
                ignores.removePlayer(pID)
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
                            ignores.removePlayer(PLR.playerGetID())
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
        PtDebugPrint("-- Logs dumped to \"{}\" at {}. --".format(destination, currentTime))
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

    ## Update the Chronicle's KI Text Color values.
    # This uses an RGB triplet with parts separated by "," or an empty string for the default chat color.
    def UpdateTextColorChronicle(self):

        vault = ptVault()
        newVal = ""
        if isinstance(self.chatMgr.chatTextColor, ptColor):
            newVal = f"{self.chatMgr.chatTextColor.getRed()},{self.chatMgr.chatTextColor.getGreen()},{self.chatMgr.chatTextColor.getBlue()}"

        PtDebugPrint(f"xKIChat.UpdateTextColorChronicle(): Setting KI Text Color chronicle to: \"{newVal}\".", level=kWarningLevel)
        vault.addChronicleEntry(kChronicleKITextColor, kChronicleKITextColorType, newVal)

    ## Overrides the chat area text colors to be a single user-selected color
    def SetTextColor(self, arg1, arg2=None, arg3=None):

        clamp = lambda val, minVal, maxVal: max(minVal, min(val, maxVal))

        if not arg1:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.MalformedChatSetTextColorCmd"), kChat.SystemMessage)
            return

        try:
            colorRed = int(arg1)
            colorBlue = int(arg2)
            colorGreen = int(arg3)
        except (TypeError, ValueError):
            # arguments weren't valid integer numbers, so we will try treating them as floats below
            pass
        else:
            # args were a valid set of integer numbers
            colorRed = clamp(colorRed, 0, 255)
            colorBlue = clamp(colorBlue, 0, 255)
            colorGreen = clamp(colorGreen, 0, 255)
            self.chatMgr.chatTextColor = ptColor(colorRed / 255.0, colorBlue / 255.0, colorGreen / 255.0)
            self.UpdateTextColorChronicle()
            self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.TextColorSetValue", [f"({colorRed}, {colorBlue}, {colorGreen})"]))
            return

        try:
            colorRed = float(arg1)
            colorBlue = float(arg2)
            colorGreen = float(arg3)
        except (TypeError, ValueError):
            # arguments weren't valid float numbers, so we will try treating arg1 as a string below
            pass
        else:
            # args were a valid set of float numbers
            colorRed = clamp(colorRed, 0.0, 1.0)
            colorBlue = clamp(colorBlue, 0.0, 1.0)
            colorGreen = clamp(colorGreen, 0.0, 1.0)
            self.chatMgr.chatTextColor = ptColor(colorRed, colorBlue, colorGreen)
            self.UpdateTextColorChronicle()
            self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.TextColorSetValue", [f"({colorRed}, {colorBlue}, {colorGreen})"]))
            return

        newColor = arg1.strip().casefold()
        if newColor in kColorNames:
            # newColor is a valid localized color name with a corresponding function on the ptColor class
            colorFn = getattr(ptColor(), kColorNames[newColor])
            self.chatMgr.chatTextColor = colorFn()
            self.UpdateTextColorChronicle()
            self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.TextColorSetValue", [newColor]))
        elif newColor == PtGetLocalizedString("KI.Commands.ChatSetTextColorDefaultArg"):
            # user requested resetting text colors to defaults
            self.chatMgr.chatTextColor = None
            self.UpdateTextColorChronicle()
            self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.TextColorSetDefault"))
        else:
            hexColor = newColor.lstrip("#")
            if len(hexColor) == 3:
                # turn RGB into RRGGBB
                hexColor = f"{hexColor[0]}{hexColor[0]}{hexColor[1]}{hexColor[1]}{hexColor[2]}{hexColor[2]}"

            if len(hexColor) == 6:
                try:
                    colorRed = int(hexColor[0:2], 16) / 255.0
                    colorBlue = int(hexColor[2:4], 16) / 255.0
                    colorGreen = int(hexColor[4:6], 16) / 255.0
                except ValueError:
                    # argument was not a valid hexadecimal number
                    self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.MalformedChatSetTextColorCmd"), kChat.SystemMessage)
                    return
                else:
                    self.chatMgr.chatTextColor = ptColor(colorRed, colorBlue, colorGreen)
                    self.UpdateTextColorChronicle()
                    self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.TextColorSetValue", [f"#{hexColor}"]))
            else: 
                # argument was not 3 or 6 characters long, so it cannot be a valid hexadecimal color
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Errors.MalformedChatSetTextColorCmd"), kChat.SystemMessage)
                return

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
                PtDebugPrint("xKIChat.SaveColumns(): Found Jalak's python component.", level=kDebugDumpLevel)
                SendNote(self.chatMgr.key, script, "SaveColumns;" + fName)
                return
        PtDebugPrint("xKIChat.SaveColumns(): Did not find Jalak's python component.", level=kErrorLevel)

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
                PtDebugPrint("xKIChat.LoadColumns(): Found Jalak's python component.", level=kDebugDumpLevel)
                SendNote(self.chatMgr.key, script, "LoadColumns;" + fName)
                return
        PtDebugPrint("xKIChat.LoadColumns(): Did not find Jalak's python component.", level=kErrorLevel)

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

        # Load the Age-specific text.
        ageInfo = PtGetAgeInfo()
        if ageInfo is None:
            return

        currentAge = ageInfo.getAgeFilename()
        see = ""
        exits = " North and West."
        people = ""
        peopleVerb = "is"
        if currentAge in kEasterEggs:
            see = kEasterEggs[currentAge]["see"]

            if not kEasterEggs[currentAge]["exits"]:
                exits = "... well, there are no exits."
            else:
                exits = kEasterEggs[currentAge]["exits"]

            if "people" in kEasterEggs[currentAge]:
                people = kEasterEggs[currentAge]["people"]

        # Find the nearby people if kEasterEggs didn't define people text override for the Age.
        if not people:
            playerList = self.chatMgr.GetPlayersInChatDistance(minPlayers=-1)
            playerListLen = len(playerList)
            peopleVerb = "are" if playerListLen > 1 else "is"

            if playerListLen == 0:
                people = " nobody in particular."
            else:
                # concatenate player names together with commas (using "and" before the last name)
                people = ", ".join((
                    f"{' ' if idx == 0 else ''}{'and ' if playerListLen > 1 and playerListLen == idx + 1 else ''}"
                    f"{player.getPlayerName()}{'.' if playerListLen == idx + 1 else ''}"
                    for idx, player in enumerate(playerList)
                ))

        ## Display the info.
        self.chatMgr.AddChatLine(None, f"{GetAgeName()}: {see} Standing near you {peopleVerb}{people} There are exits to the{exits}", 0)

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
        elif currentAge == "GoMePubNew":
            if self.chatMgr.gFeather == 13:
                self.chatMgr.AddChatLine(None, "You search... and find a \"Yellow\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 13:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Rukh\" feather.", 0)
        elif currentAge == "ChisoPreniv":
            if self.chatMgr.gFeather == 14:
                self.chatMgr.AddChatLine(None, "You search... and find a \"Raven\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 14:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Yellow\" feather.", 0)
        elif currentAge == "VeeTsah":
            if self.chatMgr.gFeather == 15:
                self.chatMgr.AddChatLine(None, "You search... and find a \"Dove\" feather and put it in your pocket.", 0)
                self.chatMgr.gFeather += 1
            elif self.chatMgr.gFeather > 15:
                self.chatMgr.AddChatLine(None, "You search... but find no other feathers.", 0)
            else:
                self.chatMgr.AddChatLine(None, "You search... but then suddenly stop when you realize that you are missing the \"Raven\" feather.", 0)
        else:
            self.chatMgr.AddChatLine(None, "There are no feathers here.", 0)
            return
        vault = ptVault()
        entry = vault.findChronicleEntry("feather")
        if entry is None:
            vault.addChronicleEntry("feather", 1, str(self.chatMgr.gFeather))
            entry = vault.findChronicleEntry("feather")
        entry.setValue(str(self.chatMgr.gFeather))
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
                if self.chatMgr.gFeather > 13:
                    pOut += " and a \"Yellow\" feather"
                if self.chatMgr.gFeather > 14:
                    pOut += " and a \"Raven\" feather"
                if self.chatMgr.gFeather > 15:
                    pOut += " and a \"Dove\" feather"
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
            if child.getName() == kChron.Party:
                party = child
                break

        # Let's see what we need to do
        if not params:
            # No params = LINK ME!
            if party and party.getValue():
                data = party.getValue().split(";", 3)
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
                    party.setValue("")
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
                    party.setName(kChron.Party)
                    party.setValue(data)
                    party.save() # creates node on server (and blocks) so we can add it to the global inbox
                    inbox.addNode(party)
                else:
                    party.setValue(data)
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
        if not PtIsSolo() and not PtIsInternalRelease():
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
            if player.getPlayerName().casefold() == name.casefold():
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

    ## Rolls the specified dice and displays the results
    def RollDice(self, dice_str):
        if not dice_str:
            roll = random.randint(1, 6)
            PtSendKIMessage(kKIChatStatusMsg, "{} rolled a single six-sided die with a result of {}.".format(PtGetLocalPlayer().getPlayerName(), roll))
            return

        # Handle special dice types
        if dice_str.casefold() == "fate":
            fate = [random.randint(-1, 1) for x in range(4)]
            PtSendKIMessage(kKIChatStatusMsg, "{} rolled fate values of {} for a total of {}.".format(PtGetLocalPlayer().getPlayerName(), fate, sum(fate)))
            return

        # Parse common dice notation
        dice_opt = re.match(r"^(\d+)d(\d+)$", dice_str)
        if not dice_opt:
            self.chatMgr.AddChatLine(None, "I'm sorry, I don't know how to roll {}.".format(dice_str), kChat.SystemMessage)
            return
        num_dice = int(dice_opt.groups()[0])
        num_face = int(dice_opt.groups()[1])

        if num_dice < 1:
            self.chatMgr.AddChatLine(None, "Cannot roll less than a single die.", kChat.SystemMessage)
            return
        if num_dice > 10:
            self.chatMgr.AddChatLine(None, "Cannot roll more than 10 dice at a time.", kChat.SystemMessage)
            return
        if num_face < 2:
            self.chatMgr.AddChatLine(None, "Cannot roll a die with less than two faces.", kChat.SystemMessage)
            return
        if num_face > 100:
            self.chatMgr.AddChatLine(None, "Cannot roll a die with more than 100 faces.", kChat.SystemMessage)
            return

        roll = [random.randint(1, num_face) for x in range(num_dice)]
        if num_dice == 1:
            PtSendKIMessage(kKIChatStatusMsg, "{} rolled a single {}-sided die with a result of {}.".format(PtGetLocalPlayer().getPlayerName(), num_face, roll[0]))
        else:
            PtSendKIMessage(kKIChatStatusMsg, "{} rolled {}d{} with a result of {} for a total of {}.".format(PtGetLocalPlayer().getPlayerName(), num_dice, num_face, roll, sum(roll)))        
