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
"""
This module is contains the datatypes and constants for
interfacing with the KI subsytem
"""

# OnKIMsg and PtSendKIMessage command types
kEnterChatMode=1        # start chat mode on KI, 'value' doesn't matter
kSetChatFadeDelay=2     # set the chat fade time, 'value' is the delay in seconds
kSetTextChatAdminMode=3 # set self to be admin mode, 'value' is 1 to go into Admin mode, 0 remove
kDisableKIandBB=4       # disable the KI and the blackbar, 'value' doesn't matter
kEnableKIandBB=5        # enable the KI and the blackbar, 'value' doesn't matter
kYesNoDialog=6          # request that the KI put up a yes/no dialog for someone else
kAddPlayerDevice=7      # add player interactive device to list of devices, 'value' is device name
kRemovePlayerDevice=8   # remove player interactive device from list, 'value' is device name
kUpgradeKILevel=9       # upgrade new level of KI (if already at that level, nevermind), 'value' is the new level
kDowngradeKILevel=10    # remove (drop) the KI level (if not at that level, nevermind), 'value' is the level to remove
kRateIt=11              # request for the KI to ask the user to Rate something
kSetPrivateChatChannel=12 # sets the private chat channel to number, for private rooms
kUnsetPrivateChatChannel=13 # undoes the the private chat channel.
kStartBookAlert=14      # start the book alert
kMiniBigKIToggle=15		# shortcut to toggling the miniKI/bigKI
kKIPutAway=16			# shortcut to hiding all of the KI
kChatAreaPageUp=17		# shortcut to paging up the chat area
kChatAreaPageDown=18	# shortcut to paging down the chat area
kChatAreaGoToBegin=19	# shortcut to going to the beginning of the chat area
kChatAreaGoToEnd=20		# shortcut to going to the end of the chat area
kKITakePicture=21		# shortcut to taking a picture in the KI
kKICreateJournalNote=22	# shortcut to creating a journal note in the KI
kKIToggleFade=23        # shortcut to toggle fade mode in the miniKI (only if miniKI only)
kKIToggleFadeEnable=24  # shortcut to toggling the enable flag for fading chat
kKIChatStatusMsg=25     # display a status message (net propagated) in the chat window
kKILocalChatStatusMsg=26 # display a status message (local only) in the chat window
kKIUpSizeFont=27        # up size the font in the KI (chatarea)
kKIDownSizeFont=28      # down size the font in the KI (chatarea)
kKIOpenYeehsaBook=29    # open the Yeehsa book, if not already open
kKIOpenKI=30            # open the KI a little at a time
kKIShowOptionsMenu=31   # show the options menu dialog
kKICreateMarker=32      # create a marker
kKICreateMarkerFolder=33 # create a marker folder(node) game in the current Age's journal folder
kKILocalChatErrorMsg=34  # display an error message (local only) in the chat window
kKIPhasedAllOn=35       # turn on all the phased KI functionality
kKIPhasedAllOff=36      # turn off all the phased KI functionality
kKIOKDialog=37          # display an OK dialog box (localized)
kDisableYeeshaBook=38   # don't allow linking with the Yeesha book (gameplay)
kEnableYeeshaBook=39    # re-allow linking with the Yeesha book
kQuitDialog=40          # put up Quit dialog
kTempDisableKIandBB=41  # temp disable KI and blackbar (done by av system)
kTempEnableKIandBB=42   # temp re-enable the KI and blackbar (done by av system)
kDisableEntireYeeshaBook=43     # disable the entire Yeeshabook, not for gameplay, but prevent linking
kEnableEntireYeeshaBook=44     # enable the entire Yeeshabook, not for gameplay
kKIOKDialogNoQuit=45               # display OK dialog in the KI without quiting afterwards
kGZUpdated=46                       # the GZ game was updated
kGZInRange=47                       # a GZ marker is in range
kGZOutRange=48                      # GZ markers are out of range
kUpgradeKIMarkerLevel=49        # upgrade the KI marker level
kKIShowMiniKI=50            # force the miniKI up
kGZFlashUpdate=51            # flash update to the GZ display on the miniKI (without saving)
kStartJournalAlert = 52 # start the journal alert
kAddJournalBook = 53    # add the journal book to the BB
kRemoveJournalBook = 54 # remove the journal book from the BB
kKIOpenJournalBook = 55 # show the journal book
kMGStartCGZGame = 56    # Start CGZ Marker Game
kMGStopCGZGame = 57     # Stop CGZ Marker Game
kKICreateMarkerNode = 58 #Creates the marker game vault Node
kStartKIAlert = 59      # start the KI alert
kUpdatePelletScore = 60 # Updates the pellet score
kFriendInviteSent = 61  # Result of friend invite received
kRegisterImager = 62    # Imagers send to register themselves with the KI

# kUpgradeKILevel and kDowngradeKILevel levels
#kNanoKI = 0 Unused KI level.
kMicroKI = 1
kNormalKI = 2
kLowestKILevel = kMicroKI
kHighestKILevel = kNormalKI

# Upgrade levels for the KI marker
kKIMarkerNotUpgraded = 0
kKIMarkerFirstLevel = 1     # can play marker tag, but no GPS, can play first set of GZMarkers
kKIMarkerSecondLevel = 2    # can get to back room in GreatZero and play second set of GZmarkers
kKIMarkerNormalLevel = 3    # complete both GZmarker trials - has GPS

# GZ Marker and Calibration GZ Marker status
kGZMarkerInactive = "0"
kGZMarkerAvailable = "1"
kGZMarkerCaptured = "2"
kGZMarkerUploaded = "3"

# Calibration GZ Marker Games
kCGZMarkerInactive = "0"
kCGZMarkerAvailable = "1"
kCGZMarkerCaptured = "2"
kCGZMarkerUploaded = "3"
gCGZAllStates = [ kCGZMarkerInactive, kCGZMarkerAvailable, kCGZMarkerCaptured, kCGZMarkerUploaded  ]
kCGZFirstGame = 0
kCGZFinalGame = 3
kCGZToransGame = 0
kCGZHSpansGame = 1
kCGZVSpansGame = 2
kCGZActivateGZ = 3


# -- chronicle names and types
kChronicleKILevel = "PlayerKILevel"
kChronicleKILevelType = 2
kChronicleCensorLevel = "PlayerCensorLevel"
kChronicleCensorLevelType = 2
kChronicleKIMarkerLevel = "KIMarkerLevel"
kChronicleKIMarkerLevelType = 2
kChronicleGZGames = "GZGames"
kChronicleGZGamesType = 1
kChronicleGZMarkersAquired = "GZMarkersAquired"
kChronicleGZMarkersAquiredType = 1
kChronicleCalGZMarkersAquired = "CalGZMarkers"
kChronicleCalGZMarkersAquiredType = 1


def PtDetermineKILevel():
    "Get the KILevel"
    # assume that they have none...
    import Plasma
    import string
    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry(kChronicleKILevel)
    if type(entry) != type(None):
        level = string.atoi(entry.chronicleGetValue())
        # make sure it is a valid level
        if level >= kLowestKILevel and level <= kHighestKILevel:
            return level
    # if couldn't be determine... just assume lowest form
    return kMicroKI

def PtDetermineCensorLevel():
    "Get the KILevel"
    # assume that they have none...
    import Plasma
    import string
    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry(kChronicleCensorLevel)
    if type(entry) != type(None):
        level = string.atoi(entry.chronicleGetValue())
        return level
    # if couldn't be determine... just assume lowest form
    return 0

def PtDetermineKIMarkerLevel():
    "Get the KIMarkerLevel"
    # assume that they have none...
    import Plasma
    import string
    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry(kChronicleKIMarkerLevel)
    if type(entry) != type(None):
        level = string.atoi(entry.chronicleGetValue())
        return level
    # if couldn't be determine... just assume lowest form
    return kKIMarkerNotUpgraded


def PtAmPlayingCGZM():
    chron = PtGetMarkerGameChronicle()
    if chron is None:
        return False
    return chron.getValue() == "cgz"

def PtFindCreateMarkerChronicle(name, subchron=None, default=None):
    import Plasma
    def _GetChron(needle, haystack):
        for i in haystack.getChildNodeRefList():
            child = i.getChild().upcastToChronicleNode()
            if child is None:
                continue
            if child.getName() == needle:
                return child
        else:
            chron = Plasma.ptVaultChronicleNode()
            chron.setName(needle)
            if default is not None:
                chron.setValue(str(default))
            haystack.addNode(chron)
            return chron

    chron = _GetChron(name, PtGetMarkerGameChronicle())
    if subchron is None:
        return chron
    return _GetChron(subchron, chron)

def PtGetCGZM():
    chron = PtFindCreateMarkerChronicle("CGZ-Mission")
    mission = chron.getValue()
    if mission:
        return int(mission)
    return -1

def PtGetCGZStartTime():
    chron = PtFindCreateMarkerChronicle("CGZ-StartTime")
    time = chron.getValue()
    if time:
        return int(time)
    return 0

def PtGetMarkerQuestCaptures(name):
    chron = PtFindCreateMarkerChronicle("Quest", name)
    caps = chron.getValue().split(',')
    if caps:
        return { int(i): True for i in caps if i.isdigit() }
    else:
        return {}

def PtGetMarkerGameChronicle():
    import Plasma
    vault = Plasma.ptVault()
    chron = vault.findChronicleEntry("MarkerBrain")
    if chron is None:
        vault.addChronicleEntry("MarkerBrain", 0, "")
        return vault.findChronicleEntry("MarkerBrain")
    return chron

def PtGetTimePlayingCGZ():
    import Plasma
    return Plasma.PtGetServerTime() - PtGetCGZStartTime()

def PtIsCGZMComplete():
    if PtAmPlayingCGZM():
        captures = PtGetMarkerQuestCaptures("cgz")
        if not captures:
            return False
        for i in captures.itervalues():
            if not i:
                return False
        return True
    else:
        return False

def PtSetCGZM(mission):
    chron = PtGetMarkerGameChronicle()
    chron.setValue("" if mission == -1 else "cgz")
    chron.save()

    chron = PtFindCreateMarkerChronicle("CGZ-Mission")
    chron.setValue(str(mission))
    chron.save()

def PtSetMarkerQuestCaptures(name, captures):
    chron = PtFindCreateMarkerChronicle("Quest", name)
    if captures:
        chron.setValue(','.join(( str(key) for key, value in captures.iteritems() if value )))
    else:
        chron.setValue("")
    chron.save()

def PtUpdateCGZStartTime(time=None):
    chron = PtFindCreateMarkerChronicle("CGZ-StartTime")
    if time is None:
        import Plasma
        time = Plasma.PtGetServerTime()
    chron.setValue(str(time))
    chron.save()

# OnRTChat flags (see pfKIMsg.h)
kRTChatPrivate = 0x01
kRTChatAdmin = 0x02
kRTChatPrivateAdmin= kRTChatPrivate | kRTChatAdmin
kRTChatGlobal = 0x04
kRTChatInterAge = 0x08
kRTChatStatusMsg = 0x10
kRTChatNeighborsMsg = 0x20

# flags channel mask
kRTChatFlagMask = 65535
kRTChatChannelMask = 65280
kRTChatNoChannel = 255

# OnCCRMsg flags
kCCRBeginCommunication=1
kCCRChat=2
kCCREndCommunication=3
kCCRReturnChatMsg=4
