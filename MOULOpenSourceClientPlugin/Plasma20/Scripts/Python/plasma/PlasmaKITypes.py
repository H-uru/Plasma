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
kKIShowCCRHelp=31       # show the CCR help dialog
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
kNanoKI=0
kMicroKI=1
kNormalKI=2
kLowestKILevel = kNanoKI
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
    return kNanoKI

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

def PtGetCGZGameState(whichGame):
    "Get the CGZ Game level"
    # assume that they have none...
    import Plasma
    import PlasmaTypes
    if whichGame >= kCGZFirstGame and whichGame <= kCGZFinalGame:
        vault = Plasma.ptVault()
        entry = vault.findChronicleEntry(kChronicleCalGZMarkersAquired)
        if type(entry) != type(None):
            allStates = entry.chronicleGetValue()
            PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtGetCGZGameLevel current chronicle is %s"%(allStates),level=PlasmaTypes.kDebugDumpLevel)
            state = kCGZMarkerInactive  # assume inactive
            try:
                state = allStates[whichGame]
            except LookupError:
                PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtGetCGZGameLevel - CGZ marker game not there? chron=%s"%(allStates),level=PlasmaTypes.kErrorLevel)
                pass
            return state
        else:
            PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtGetCGZGameLevel no chronicle yet",level=PlasmaTypes.kDebugDumpLevel)
    else:
        PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtGetCGZGameLevel - invalid CGZ game of %d"%(whichGame),level=PlasmaTypes.kErrorLevel)
        pass
    # if couldn't be determine... just assume lowest form
    return kCGZMarkerInactive

def PtSetCGZGameState(whichGame,state):
    "Get the CGZ Game level"
    # assume that they have none...
    import Plasma
    import PlasmaTypes
    if whichGame >= kCGZFirstGame and whichGame <= kCGZFinalGame:
        if type(state) == type("") and state in gCGZAllStates:
            PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtSetCGZGameLevel - setting game %d to %s"%(whichGame,state),level=PlasmaTypes.kDebugDumpLevel)
            vault = Plasma.ptVault()
            entry = vault.findChronicleEntry(kChronicleCalGZMarkersAquired)
            if type(entry) != type(None):
                allStates = entry.chronicleGetValue()
                newStates = ""
                for idx in range(kCGZFinalGame+1):
                    if idx == whichGame:
                        newStates += state
                    else:
                        try:
                            newStates += allStates[idx]
                        except LookupError:
                            newStates += kCGZMarkerInactive
                # make sure we get whatever is beyond this
                newStates += allStates[kCGZFinalGame+1:]
                entry.chronicleSetValue(newStates)
                entry.save()
            else:
                # create a new one
                newStates = ""
                for idx in range(kCGZFinalGame+1):
                    if idx == whichGame:
                        newStates += state
                    else:
                        newStates += kCGZMarkerInactive
                vault.addChronicleEntry(kChronicleCalGZMarkersAquired,kChronicleCalGZMarkersAquiredType,newStates)
        else:
            PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtSetCGZGameLevel - invalid CGZ game state of:",state,level=PlasmaTypes.kErrorLevel)
            pass
    else:
        PlasmaTypes.PtDebugPrint("PlasmaKITypes:PtSetCGZGameLevel - invalid CGZ game of %d"%(whichGame),level=PlasmaTypes.kErrorLevel)
        pass

def PtWhichCGZPlaying():
    "Has the player completed the CGZ stuff"
    import Plasma
    whichGame = -1
    state = kCGZMarkerInactive
    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry(kChronicleCalGZMarkersAquired)
    if type(entry) != type(None):
        allStates = entry.chronicleGetValue()
        if len(allStates) > kCGZFinalGame:
            state = kCGZMarkerUploaded
            for i in range(kCGZFinalGame+1):
                if allStates[i] == kCGZMarkerAvailable or allStates[i] == kCGZMarkerCaptured:
                    whichGame = i
                    state = allStates[i]
                    break
                if allStates[i] != kCGZMarkerUploaded:
                    state = kCGZMarkerInactive
    return (whichGame,state)

def PtIsCGZDone():
    "Has the player completed the CGZ stuff"
    import Plasma
    isDone = 0
    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry(kChronicleCalGZMarkersAquired)
    if type(entry) != type(None):
        allStates = entry.chronicleGetValue()
        if len(allStates) > kCGZFinalGame:
            # assume that we are going to find them all
            isDone = 1
            for i in range(kCGZFinalGame+1):
                if allStates[i] != kCGZMarkerUploaded:
                    isDone = 0
                    break
    return isDone

def PtDetermineGZ():
    "Get the current GZ states"
    import Plasma
    import PlasmaTypes
    import string
    GZPlaying = 0
    MarkerToGetColor = 'off'
    MarkerGottenColor = 'off'
    MarkerToGetNumber = 0
    MarkerGottenNumber = 0
    KIMarkerLevel = PtDetermineKIMarkerLevel()
    if KIMarkerLevel > kKIMarkerNotUpgraded:
        # see if they are playing a CGZ game
        (whichGame,state) = PtWhichCGZPlaying()
        if KIMarkerLevel < kKIMarkerNormalLevel or (KIMarkerLevel == kKIMarkerNormalLevel and whichGame != -1):
            vault = Plasma.ptVault()
            # is there a chronicle for the GZ games?
            entry = vault.findChronicleEntry(kChronicleGZGames)
            if type(entry) != type(None):
                gameString = entry.chronicleGetValue()
                PlasmaTypes.PtDebugPrint("PtDetermineGZ: - game string is %s" % (gameString),level=PlasmaTypes.kDebugDumpLevel)
                args = gameString.split()
                if len(args) == 3:
                    try:
                        GZPlaying = string.atoi(args[0])
                        colors = args[1].split(':')
                        MarkerGottenColor = colors[0]
                        MarkerToGetColor = colors[1]
                        outof = args[2].split(':')
                        MarkerGottenNumber = string.atoi(outof[0])
                        MarkerToGetNumber = string.atoi(outof[1])
                    except ValueError:
                        PlasmaTypes.PtDebugPrint("xKI:GZ - error trying to read GZGames Chronicle",level=PlasmaTypes.kErrorLevel)
                        # we don't know which one it errored on, so just reset them all
                        GZPlaying = 0
                        MarkerToGetColor = 'off'
                        MarkerGottenColor = 'off'
                        MarkerToGetNumber = 0
                        MarkerGottenNumber = 0
                else:
                    PlasmaTypes.PtDebugPrint("xKI:GZ - error GZGames string formation error",level=PlasmaTypes.kErrorLevel)
                    pass
        else:
            # can't be playing a GZGame!
            # ...might be a MarkerTag game... let the KI determine that.
            pass
    PlasmaTypes.PtDebugPrint("PtDetermineGZ: - returning game=%d colors=%s:%s markers=%d:%d" % (GZPlaying, MarkerGottenColor, MarkerToGetColor ,MarkerGottenNumber, MarkerToGetNumber),level=PlasmaTypes.kDebugDumpLevel)
    return (GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber)

def PtCaptureGZMarker(GZMarkerInRange):
    import Plasma
    import PlasmaTypes
    # get current GZ Game state
    (GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber) = PtDetermineGZ()
    # make sure there is room for the capture marker
    if GZPlaying and MarkerToGetNumber > MarkerGottenNumber:
        # set the marker status to 'gotten'
        #   ...in the GZ marker chronicle
        vault = Plasma.ptVault()
        # is there a chronicle for the GZ games?
        entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
        if type(entry) != type(None):
            markers = entry.chronicleGetValue()
            markerIdx = GZMarkerInRange - 1
            if markerIdx >= 0 and markerIdx < len(markers):
                # Set the marker to "captured"
                PlasmaTypes.PtDebugPrint("PtCaptureGZMarker: starting with '%s' changing %d to '%s'" % (markers,GZMarkerInRange,kGZMarkerCaptured),level=PlasmaTypes.kDebugDumpLevel)
                if len(markers)-(markerIdx+1) != 0:
                    markers = markers[:markerIdx] + kGZMarkerCaptured + markers[-(len(markers)-(markerIdx+1)):]
                else:
                    markers = markers[:markerIdx] + kGZMarkerCaptured
                #PlasmaTypes.PtDebugPrint("xKI: out string is '%s'" % (markers),level=PlasmaTypes.kDebugDumpLevel)
                entry.chronicleSetValue(markers)
                entry.save()
                # update the marker Gotten count
                totalGotten = markers.count(kGZMarkerCaptured)
                KIMarkerLevel = PtDetermineKIMarkerLevel()
                if KIMarkerLevel > kKIMarkerFirstLevel:
                    # if this is the second wave of markers (or beyond)
                    totalGotten -= 5
                    if totalGotten < 0:
                        totalGotten = 0
                if totalGotten > MarkerToGetNumber:
                    totalGotten = MarkerToGetNumber
                MarkerGottenNumber = totalGotten
                # save update to chronicle
                PtUpdateGZGamesChonicles(GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber)
            else:
                PlasmaTypes.PtDebugPrint("PtCaptureGZMarker: invalid marker serial number of %d" % (gGZMarkerInRange),level=PlasmaTypes.kErrorLevel )
                pass
        else:
            PlasmaTypes.PtDebugPrint("PtCaptureGZMarker: no chronicle entry found",level=PlasmaTypes.kErrorLevel )
            pass
    else:
        PlasmaTypes.PtDebugPrint("PtCaptureGZMarker: no game or this game is complete",level=PlasmaTypes.kErrorLevel )
        pass

def PtVerifyGZMarker():
    import Plasma
    import PlasmaTypes
    # get current GZ Game state
    (GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber) = PtDetermineGZ()
    # make sure there is room for the capture marker
    if GZPlaying:
        # set the marker status to 'gotten'
        #   ...in the GZ marker chronicle
        vault = Plasma.ptVault()
        # is there a chronicle for the GZ games?
        entry = vault.findChronicleEntry(kChronicleGZMarkersAquired)
        if type(entry) != type(None):
            markers = entry.chronicleGetValue()
            # get what was really gotten
            totalGotten = markers.count(kGZMarkerCaptured)
            KIMarkerLevel = PtDetermineKIMarkerLevel()
            if KIMarkerLevel > kKIMarkerFirstLevel:
                # if this is the second wave of markers (or beyond)
                totalGotten -= 5
                if totalGotten < 0:
                    totalGotten = 0
            if totalGotten > MarkerToGetNumber:
                totalGotten = MarkerToGetNumber
            if totalGotten != MarkerGottenNumber:
                PlasmaTypes.PtDebugPrint("PtVerifyGZMarker: Error! Gotten different than real. They say=%d We say=%d"%(MarkerGottenNumber,totalGotten),level=PlasmaTypes.kErrorLevel )
                MarkerGottenNumber = totalGotten
                # save update to chronicle
                PtUpdateGZGamesChonicles(GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber)
                Plasma.PtSendKIMessage(kGZUpdated,0)
    return (GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber)

def PtUpdateGZGamesChonicles(GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber):
    "Update the GZ chronicle variable"
    import Plasma
    import PlasmaTypes
    vault = Plasma.ptVault()
    # is there a chronicle for the GZ games?
    entry = vault.findChronicleEntry(kChronicleGZGames)
    try:
        upstring = "%d %s:%s %d:%d" % (GZPlaying,MarkerGottenColor,MarkerToGetColor,MarkerGottenNumber,MarkerToGetNumber)
        if type(entry) != type(None):
            entry.chronicleSetValue(upstring)
            entry.save()
        else:
            # if there is none, then just add another entry
            vault.addChronicleEntry(kChronicleGZGames,kChronicleGZGamesType,upstring)
    except TypeError:
        if type(GZPlaying) != type(0):
            PlasmaTypes.PtDebugPrint("PtUpdateGZGamesChronicle: GZPlaying wrong type (should be integer)",level=PlasmaTypes.kErrorLevel )
            pass
        if type(MarkerToGetColor) != type(""):
            PlasmaTypes.PtDebugPrint("PtUpdateGZGamesChronicle: GZPlaying wrong type (should be string)",level=PlasmaTypes.kErrorLevel )
            pass
        if type(MarkerGottenColor) != type(""):
            PlasmaTypes.PtDebugPrint("PtUpdateGZGamesChronicle: GZPlaying wrong type (should be string)",level=PlasmaTypes.kErrorLevel )
            pass
        if type(MarkerToGetNumber) != type(0):
            PlasmaTypes.PtDebugPrint("PtUpdateGZGamesChronicle: GZPlaying wrong type (should be integer)",level=PlasmaTypes.kErrorLevel )
            pass
        if type(MarkerGottenNumber) != type(0):
            PlasmaTypes.PtDebugPrint("PtUpdateGZGamesChronicle: GZPlaying wrong type (should be integer)",level=PlasmaTypes.kErrorLevel )
            pass
        pass


# OnRTChat flags
# NOTE: kRTChatInterAge = 8 is being used in cyMisc.cpp SendRTChat (hard coded) so don't change here unless you change that too
kRTChatPrivate=1
kRTChatAdmin=2
kRTChatPrivateAdmin=3
kRTChatInterAge=8
kRTChatStatusMsg=16
kRTChatNeighborsMsg=32

# flags channel mask
kRTChatFlagMask = 65535
kRTChatChannelMask = 65280
kRTChatNoChannel = 255

# OnCCRMsg flags
kCCRBeginCommunication=1
kCCRChat=2
kCCREndCommunication=3
kCCRReturnChatMsg=4
