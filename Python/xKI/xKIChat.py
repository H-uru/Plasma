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

# Plasma Engine.
from Plasma import *
from PlasmaConstants import *
from PlasmaKITypes import *
from PlasmaTypes import *

# xKI sub-modules.
from xKIConstants import *


## A class to process all the RT Chat functions of the KI.
class xKIChat(object):

    ## Set up the chat manager's default state.
    def __init__(self, BigKI, KIBlackbar, KIMicro, KIMini, GetAgeName, fading):

        # Add the GUI dialogs.
        self.BigKI = BigKI
        self.KIBlackbar = KIBlackbar
        self.KIMicro = KIMicro
        self.KIMini = KIMini

        # Set the default properties.
        self.autoShout = False
        self.logFile = None
        self.isAdmin = False
        self.isChatting = False
        self.lastPrivatePlayerID = None
        self.onlyGetPMsFromBuddies = False
        self.onlyAllowBuddiesOnRequest = False
        self.privateChatChannel = 0
        self.toReplyToLastPrivatePlayerID = None

        # Fading globals.
        self.currentFadeTick = 0
        self.fadeEnableFlag = 1
        self.fadeMode = kChat.FadeNotActive
        self.ticksOnFull = 30

        # Set the properties from the KI.
        self.BKPlayerList = []
        self.KIDisabled = False
        self.KILevel = kMicroKI
        self.GetAgeName = GetAgeName
        self.StartFadeTimer = fading[0]
        self.KillFadeTimer = fading[1]
        self.FadeCompletely = fading[2]

        # Add the commands processor.
        self.commandsProcessor = CommandsProcessor(self)

    #######
    # GUI #
    #######

    ## Make the miniKI icon unclickable in the Blackbar.
    def ClearBBMini(self, value = -1):

        if self.KILevel == kNormalKI:
            mmRG = ptGUIControlRadioGroup(self.KIBlackbar.dialog.getControlFromTag(kGUI.MiniMaximizeRGID))
            mmRG.setValue(value)

    ## Check if the chat is faded out.
    def IsFaded(self):

        if self.KILevel >= kMicroKI:
            if not self.BigKI.dialog.isEnabled() and (self.KIMini.dialog.isEnabled() or self.KIMicro.dialog.isEnabled()):
                if self.fadeMode == kChat.FadeNotActive:
                    return True
        return False

    ## Scroll the chat in the specified direction on the miniKI.
    # Possible directions are to scroll up, down, to the beginning and to the
    # end.
    def ScrollChatArea(self, direction):

        if self.KILevel < kNormalKI:
            mKIdialog = self.KIMicro.dialog
        else:
            mKIdialog = self.KIMini.dialog
        self.KillFadeTimer()
        self.StartFadeTimer()
        chatarea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatarea.moveCursor(direction)

    ############
    # Chatting #
    ############

    ## Make the player enter or exit chat mode.
    # Chat mode means the player's keyboard input is being sent to the chat.
    def ToggleChatMode(self, entering, firstChar=None):

        if self.KILevel == kMicroKI:
            mKIdialog = self.KIMicro.dialog
            self.KIMicro.dialog.show()
        else:
            mKIdialog = self.KIMini.dialog
        caret = ptGUIControlTextBox(mKIdialog.getControlFromTag(kGUI.ChatCaretID))
        chatEdit = ptGUIControlEditBox(mKIdialog.getControlFromTag(kGUI.ChatEditboxID))
        if entering:
            self.isChatting = True
            if not self.KIMini.dialog.isEnabled():
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
            self.KillFadeTimer()
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
        pre = "<<" + self.GetAgeName() + ">>"
            
        # If the player is in AFK mode, make him exit it.
        if PtGetLocalAvatar().avatar.getCurrentMode() == PtBrainModes.kAFK:
            PtAvatarExitAFK()

        try:
            message = unicode(message, kCharSet)
        except UnicodeError:
            message = None
            self.AddChatLine(None, PtGetLocalizedString("KI.Errors.TextOnly"), kChat.SystemMessage)

        # Check for special commands.
        message = self.commandsProcessor(message)

        if not message:
            return

        # Get any selected players.
        userListBox = ptGUIControlListBox(self.KIMini.dialog.getControlFromTag(kGUI.PlayerList))
        privateChbox = ptGUIControlCheckBox(self.KIMini.dialog.getControlFromTag(kGUI.miniPrivateToggle))
        iSelect = userListBox.getSelection()
        selPlyrList = []

        # Is it a message to all players in the current Age?
        if message.startswith(PtGetLocalizedString("KI.Commands.ChatAllAge")):
            listenerOnly = False
            message = message[len(PtGetLocalizedString("KI.Commands.ChatAllAge")) + 1:]
        
        # Is it a reply to a private message?
        elif message.startswith(PtGetLocalizedString("KI.Commands.ChatReply")):
            if self.toReplyToLastPrivatePlayerID is None:
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.NoOneToReply"), kChat.SystemMessage)
                return
            # If it is local, check to see if the player is still in the Age.
            if not self.toReplyToLastPrivatePlayerID[2]:
                ageMembers = PtGetPlayerListDistanceSorted()
                hasPlayer = False
                for member in ageMembers:
                    if member.getPlayerID() == self.toReplyToLastPrivatePlayerID[1]:
                        hasplayer = True
                        break
                if not hasplayer:
                    self.lastPrivatePlayerID = None
                    self.AddChatLine(None, PtGetLocalizedString("KI.Chat.LeftTheAge", [str(self.toReplyToLastPrivatePlayerID[0])]), kChat.SystemMessage)
                    return
            # If it is not local, check to see if the player is still online.
            else:
                cFlags.interAge = True
                message = pre + message
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

        # Is it a private message sent with "/p"?
        elif message.startswith(PtGetLocalizedString("KI.Commands.ChatPrivate")):
            pWords = message.split(maxsplit=1)
            foundBuddy = False
            # Make sure it's still just a "/p".
            if len(pWords) > 1 and pWords[0] == PtGetLocalizedString("KI.Commands.ChatPrivate"):
                # Try to find the buddy in the DPL online lists.
                for player in self.BKPlayerList:
                    # Is the player in this Age?
                    if isinstance(player, ptPlayer):
                        plyrName = player.getPlayerName()
                        if pWords[1] == plyrName:
                            selPlyrList.append(player)
                            cFlags.private = True
                            foundBuddy = True
                            # Remove the "/p buddyname" from the message.
                            message = message[message.find(pWords[1]) + len(pWords[1]) + 1:]
                            self.AddPlayerToRecents(player.getPlayerID())
                            break
                    # Is the player in another Age?
                    elif isinstance(player, ptVaultNodeRef):
                        ePlyr = player.getChild()
                        ePlyr = ePlyr.upcastToPlayerInfoNode()
                        if ePlyr is not None:
                            plyrName = ePlyr.playerGetName()
                            if pWords[1] == plyrName:
                                selPlyrList.append(ptPlayer(ePlyr.playerGetName(), ePlyr.playerGetID()))
                                cFlags.private = True
                                cFlags.interAge = True
                                foundBuddy = True
                                # Add this player's current Age.
                                message = pre + message[message.find(pWords[1]) + len(pWords[1]) + 1:]
                                self.AddPlayerToRecents(ePlyr.playerGetID())
                                break
            if not foundBuddy:
                PtDebugPrint("xKIChat.SendMessage(): \"/p\" command can't find player \"{0}\".".format(pWords[1]), level=kDebugDumpLevel)
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.CannotFindBuddy", [pWords[1]]), kChat.SystemMessage)
                return

        # Is it a message to the player's neighbors?
        elif message.startswith(PtGetLocalizedString("KI.Commands.ChatNeighbors")):
            cFlags.neighbors = True
            message = message[len(PtGetLocalizedString("KI.Commands.ChatNeighbors")) + 1:]
            neighbors = self.GetOnlineNeighbors()
            if neighbors is not None:
                selPlyrList = self.GetOnlineBuddies(neighbors.getChildNodeRefList())
            if len(selPlyrList) == 0:
                self.AddChatLine(None, PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in your neighbor list"]), kChat.SystemMessage)
                return
            cFlags.interAge = True
            message = pre + message
            goesToFolder = xLocTools.FolderIDToFolderName(PtVaultStandardNodes.kHoodMembersFolder)

        # Is it a message to the player's buddies?
        elif message.startswith(PtGetLocalizedString("KI.Commands.ChatBuddies")):
            vault = ptVault()
            buddies = vault.getBuddyListFolder()
            message = message[len(PtGetLocalizedString("KI.Commands.ChatBuddies")) + 1:]
            if buddies is not None:
                selPlyrList = self.GetOnlineBuddies(buddies.getChildNodeRefList())
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
                    PtDebugPrint("xKIChat.SendMessage(): Private message to \"{0}\".".format(toPlyr.getPlayerName()), level=kDebugDumpLevel)

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
                    selPlyrList = self.GetOnlineBuddies(toPlyr.getChildNodeRefList())
                    if len(selPlyrList) == 0:
                        self.AddChatLine(None, PtGetLocalizedString("KI.Chat.WentOffline", ["Everyone in list"]), kChat.SystemMessage)
                        return
                    cFlags.interAge = 1
                    message = pre + message
                    goesToFolder = xLocTools.FolderIDToFolderName(fldrType)
                
                # Is it a folder of players within listening distance?
                elif isinstance(toPlyr, KIFolder):
                    if self.autoShout:
                        listenerOnly = False
                    else:
                        listenerOnly = True
                        selPlyrList = self.GetPlayersInChatDistance()
                        agePlayers = PtGetPlayerListDistanceSorted()
                        if len(agePlayers) > 0 and len(selPlyrList) == 0:
                            nobodyListening = True

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
            PtDebugPrint("xKIChat.AddChatLine(): Message = \"{0}\".".format(message), player, cFlags, level=kDebugDumpLevel)
        except UnicodeDecodeError:
            pass

        # Fix for Character of Doom (CoD).
        (message, RogueCount) = re.subn("[\x00-\x08\x0a-\x1f]", "", message)

        if self.KILevel == kMicroKI:
            mKIdialog = self.KIMicro.dialog
        else:
            mKIdialog = self.KIMini.dialog
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
                            message = message[idx+2:]
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
                            message = message[idx+2:]
                            if not cFlags.private and not cFlags.neighbors:
                                buddies = ptVault().getBuddyListFolder()
                                if buddies is not None:
                                    buddyID = player.getPlayerID()
                                    if not buddies.playerlistHasPlayer(buddyID):
                                        PtDebugPrint("xKIChat.AddChatLine(): Add unknown buddy {0} to recents.".format(buddyID))
                                        self.AddPlayerToRecents(buddyID)
                        except ValueError:
                            pass
                    # Save the player's ID for replying.
                    if cFlags.private:
                        self.lastPrivatePlayerID = (player.getPlayerName(), player.getPlayerID(), 1)
                        self.AddPlayerToRecents(player.getPlayerID())

            # Is it an admin message?
            elif cFlags.admin:
                if cFlags.private:
                    headerColor = kColors.ChatHeaderError
                    pretext = PtGetLocalizedString("KI.Chat.PrivateMsgRecvd")
                    forceKI = True
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
                    # Save the player's ID for replying.
                    self.lastPrivatePlayerID = (player.getPlayerName(), player.getPlayerID(), 0)
                    self.AddPlayerToRecents(player.getPlayerID())

        # Otherwise, cFlags is just a number.
        else:
            if cFlags == kChat.SystemMessage:
                headerColor = kColors.ChatHeaderError
                pretext = PtGetLocalizedString("KI.Chat.ErrorMsgRecvd")
            else:
                headerColor = kColors.ChatHeaderBroadcast
                pretext = PtGetLocalizedString("KI.Chat.BroadcastMsgRecvd")

        # If the KI is being forced open, flash the window for the player.
        if forceKI:
            PtFlashWindow()
        if forceKI and not self.KIDisabled and not mKIdialog.isEnabled():
            mKIdialog.show()
        if player is not None:
            chatHeaderFormatted = pretext + unicode(player.getPlayerName()) + U":"
            chatMessageFormatted =  U" " + message
        else:
            # It must be a status or error message.
            chatHeaderFormatted = pretext
            if pretext == U"":
                chatMessageFormatted = message
            else:
                chatMessageFormatted = " " + message

        chatArea = ptGUIControlMultiLineEdit(mKIdialog.getControlFromTag(kGUI.ChatDisplayArea))
        savedPosition = chatArea.getScrollPosition()
        wasAtEnd = chatArea.isAtEnd()
        chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
        chatArea.insertStringW(U"\n")
        chatArea.insertColor(headerColor)

        # Added unicode support here.
        chatArea.insertStringW(chatHeaderFormatted)
        chatArea.insertColor(bodyColor)
        chatArea.insertStringW(chatMessageFormatted)
        chatArea.moveCursor(PtGUIMultiLineDirection.kBufferEnd)

        # Scroll the chat by the number of new lines.
        if not wasAtEnd:
            chatArea.setScrollPosition(savedPosition)

        # Write to the log file.
        if self.chatLogFile is not None and self.chatLogFile.isOpen():
            self.chatLogFile.write(chatHeaderFormatted[0:] + chatMessageFormatted)

        # If the chat is overflowing, erase the first line.
        if chatArea.getBufferSize() > kChat.MaxChatSize:
            while chatArea.getBufferSize() > kChat.MaxChatSize and chatArea.getBufferSize() > 0:
                PtDebugPrint("xKIChat.AddChatLine(): Max chat buffer size reached. Removing top line.", level=kDebugDumpLevel)
                chatArea.deleteLinesFromTop(1)

        # Copy all the data to the miniKI if the user upgrades it.
        if self.KILevel == kMicroKI:
            chatArea2 = ptGUIControlMultiLineEdit(self.KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
            chatArea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)
            chatArea2.insertStringW(U"\n")
            chatArea2.insertColor(headerColor)

            # Added unicode support here.
            chatArea2.insertStringW(chatHeaderFormatted)
            chatArea2.insertColor(bodyColor)
            chatArea2.insertStringW(chatMessageFormatted)
            chatArea2.moveCursor(PtGUIMultiLineDirection.kBufferEnd)

            if chatArea2.getBufferSize() > kChat.MaxChatSize:
                while chatArea2.getBufferSize() > kChat.MaxChatSize and chatArea2.getBufferSize() > 0:
                    chatArea2.deleteLinesFromTop(1)

        # Update the fading controls.
        mKIdialog.refreshAllControls()
        if not self.isChatting:
            self.KillFadeTimer()
            self.StartFadeTimer()

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
    def AddPlayerToRecents(self):

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

    ## Find the player's online buddy list.
    def GetOnlineBuddies(self, pList):

        outList = []
        for bud in pList:
            if isinstance(bud, ptVaultNodeRef):
                eBud = bud.getChild().upcastToPlayerInfoNode()
                if eBud is not None:
                    if eBud.playerIsOnline():
                        outList.append(ptPlayer(eBud.playerGetName(), eBud.playerGetID()))
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

        self.flags = flags

        self.broadcast = True
        self.toSelf = True

        if self.flags & kRTChatPrivate:
            self.__dict__["private"] = True
            self.__dict__["broadcast"] = False
        else:
            self.__dict__["private"] = False

        if self.flags & kRTChatAdmin:
            self.__dict__["admin"] = True
        else:
            self.__dict__["admin"] = False

        if self.flags & kRTChatInterAge:
            self.__dict__["interAge"] = True
        else:
            self.__dict__["interAge"] = False

        if self.flags & kRTChatStatusMsg:
            self.__dict__["status"] = True
        else:
            self.__dict__["status"] = False

        if self.flags & kRTChatNeighborsMsg:
            self.__dict__["neighbors"] = True
        else:
            self.__dict__["neighbors"] = False

        self.__dict__["channel"] = (kRTChatChannelMask & flags) / 256

    def __setattr__(self, name, value):

        if name == "broadcast" and value:
            self.__dict__['flags'] &= kRTChatFlagMask ^ kRTChatPrivate

        elif name == "private":
            self.__dict__["flags"] &= kRTChatFlagMask ^ kRTChatPrivate
            if value:
                self.__dict__['flags'] |= kRTChatPrivate
                self.__dict__['broadcast'] = False
            else:
                self.__dict__['broadcast'] = True

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
        string += "channel = {0} ".format(self.channel)
        string += "flags = {0}".format(self.flags)
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

        for command, function in kCommands.LocalizedCommands:
            if message.lower().startswith(command + " "):
                theMessage = message.split(maxsplit=1)
                if len(theMessage) > 1:
                    params = theMessage[0]
                else:
                    params = None
                self.__dict__[function](params)
                return None
        return message

    ## Extract the player ID from a chat's params.
    def GetPID(self, params):

        if not params:
            return 0
        try:
            pid = long(int(params))
            return pid
        except ValueError:
            for player in BKPlayerList:
                if isinstance(player, ptPlayer):
                    plyrName = player.getPlayerName()
                    if params == plyrName:
                        return player.getPlayerID()
            return 0

    ## Add a player as a buddy.
    def AddBuddy(player):

        pid = self.GetPID(player)
        if pid:
            localPlayer = PtGetLocalPlayer()
            if pid != localPlayer.getPlayerID():
                vault = ptVault()
                buddies = vault.getBuddyListFolder()
                if buddies is not None:
                    if buddies.playerlistHasPlayer(pid):
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.AlreadyAdded"), kChat.SystemMessage)
                    else:
                        buddies.playerlistAddPlayer(pid)
                        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotYourself"), kChat.SystemMessage)
        else:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)
        return None

    ## Clear the chat.
    def ClearChat(self):

        chatAreaU = ptGUIControlMultiLineEdit(self.chatMgr.KIMicro.dialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatAreaM = ptGUIControlMultiLineEdit(self.chatMgr.KIMini.dialog.getControlFromTag(kGUI.ChatDisplayArea))
        chatAreaU.clearBuffer()
        chatAreaM.clearBuffer()
        return None

    ## Ignores a player.
    def IgnorePlayer(self, player):

        pid = self.GetPID(player)
        if pid:
            localplayer = PtGetLocalPlayer()
            if pid != localplayer.getPlayerID():
                vault = ptVault()
                ignores = vault.getIgnoreListFolder()
                if ignores is not None:
                    if ignores.playerlistHasPlayer(pid):
                        self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.AlreadyAdded"), kChat.SystemMessage)
                    else:
                        ignores.playerlistAddPlayer(pid)
                        self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Added"))
            else:
                self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotYourself"), kChat.SystemMessage)
        else:
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)
        return None

    ## Remove a player from this player's buddies.
    def RemoveBuddy(self, player):

        pid = self.GetPID(player)
        if pid:
            vault = ptVault()
            buddies = vault.getBuddyListFolder()
            if buddies is not None:
                if buddies.playerlistHasPlayer(pid):
                    buddies.playerlistRemovePlayer(pid)
                    self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
                else:
                    self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NotFound"), kChat.SystemMessage)
        else:
            # Check the ignore list.
            vault = ptVault()
            buddies = vault.getBuddyListFolder()
            if buddies is not None:
                buddyRefs = buddies.getChildNodeRefList()
                for plyr in buddyRefs:
                    if isinstance(plyr, ptVaultNodeRef):
                        PLR = plyr.getChild()
                        PLR = PLR.upcastToPlayerInfoNode()
                        if PLR is not None and PLR.getType() == PtVaultNodeTypes.kPlayerInfoNode:
                            if player == PLR.playerGetName():
                                buddies.playerlistRemovePlayer(PLR.playerGetID())
                                self.chatMgr.DisplayStatusMessage(PtGetLocalizedString("KI.Player.Removed"))
                                return None
            self.chatMgr.AddChatLine(None, PtGetLocalizedString("KI.Player.NumberOnly"), kChat.SystemMessage)
        return None

    ## Start logging the chat.
    def StartLog(self):

        if self.chatLogFile is None:
            self.chatLogFile = ptStatusLog()
        self.chatLogFile.open("Chat.log", 30, int(PtStatusLogFlags.kAppendToLast) + int(PtStatusLogFlags.kTimestamp))
        self.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.LogStarted"))
        return None

    ## Stop logging the chat.
    def StopLog(self):
        
        if self.chatLogFile is not None:
            if self.chatLogFile.isOpen():
                self.DisplayStatusMessage(PtGetLocalizedString("KI.Chat.LogStopped"))
            self.chatLogFile.close()
        return None
    
    ## Remove a player from this player's ignore list.
