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
def PtAcceptInviteInGame(friendName,inviteKey):
    """Sends a VaultTask to the server to perform the invite"""
    pass

def PtAmCCR():
    """Returns true if local player is a CCR"""
    pass

def PtAtTimeCallback(selfkey,time,id):
    """This will create a timer callback that will call OnTimer when complete
- 'selfkey' is the ptKey of the PythonFile component
- 'time' is how much time from now (in seconds) to call back
- 'id' is an integer id that will be returned in the OnTimer call"""
    pass

def PtAttachObject(child,parent):
    """Attach child to parent based on ptKey or ptSceneobject
- childKey is the ptKey or ptSceneobject of the one being attached
- parentKey is the ptKey or ptSceneobject of the one being attached to
(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)"""
    pass

def PtAvatarEnterAFK():
    """Tells the local avatar to enter AwayFromKeyboard idle loop (netpropagated)"""
    pass

def PtAvatarEnterLookingAtKI():
    """Tells the local avatar to enter looking at KI idle loop (netpropagated)"""
    pass

def PtAvatarEnterUsePersBook():
    """Tells the local avatar to enter using their personal book idle loop (netpropagated)"""
    pass

def PtAvatarExitAFK():
    """Tells the local avatar to exit AwayFromKeyboard idle loop (netpropagated)"""
    pass

def PtAvatarExitLookingAtKI():
    """Tells the local avatar to exit looking at KI idle loop (netpropagated)"""
    pass

def PtAvatarExitUsePersBook():
    """Tells the local avatar to exit using their personal book idle loop (netpropagated)"""
    pass

def PtAvatarSitOnGround():
    """Tells the local avatar to sit on ground and enter sit idle loop (netpropagated)"""
    pass

def PtAvatarSpawnNext():
    """Send the avatar to the next spawn point"""
    pass

def PtCanShadowCast():
    """Can we cast shadows?"""
    pass

def PtChangeAvatar(gender):
    """Change the local avatar's gender (or clothing type)"""
    pass

def PtChangePassword(password):
    """Changes the current account's password"""
    pass

def PtChangePlayerName(name):
    """Change the local avatar's name"""
    pass

def PtCheckVisLOS(startPoint,endPoint):
    """Does LOS check from start to end"""
    pass

def PtCheckVisLOSFromCursor():
    """Does LOS check from where the mouse cursor is, into the screen"""
    pass

def PtClearCameraStack():
    """clears all cameras"""
    pass

def PtClearOfferBookMode():
    """Cancel the offer book interface"""
    pass

def PtClearPrivateChatList(memberKey):
    """Remove the local avatar from private vox messaging, and / or clear members from his chat list"""
    pass

def PtClearTimerCallbacks(key):
    """This will remove timer callbacks to the specified key"""
    pass

def PtConsole(command):
    """This will execute 'command' as if it were typed into the Plasma console."""
    pass

def PtConsoleNet(command,netForce):
    """This will execute 'command' on the console, over the network, on all clients.
If 'netForce' is true then force command to be sent over the network."""
    pass

def PtCreateDir(directory):
    """Creates the directory and all parent folders. Returns false on failure"""
    pass

def PtCreatePlayer(playerName, avatarShape, invitation):
    """Creates a new player"""
    pass

def PtCreatePlayerW(playerName, avatarShape, invitation):
    """Unicode version of PtCreatePlayer"""
    pass

def PtCreatePublicAge(ageInfo, cbObject=None):
    """Create a public instance of the given age.
cbObject, if supplied should have a member called publicAgeCreated(self,ageInfo)"""
    pass

def PtDebugAssert(cond, msg):
    """Debug only: Assert if condition is false."""
    pass

def PtDeletePlayer(playerInt):
    """Deletes a player associated with the current account"""
    pass

def PtDetachObject(child,parent):
    """Detach child from parent based on ptKey or ptSceneobject
- child is the ptKey or ptSceneobject of the one being detached
- parent is the ptKey or ptSceneobject of the one being detached from
(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)"""
    pass

def PtDirtySynchClients(selfKey,SDLStateName,flags):
    """DO NOT USE - handled by ptSDL"""
    pass

def PtDirtySynchState(selfKey,SDLStateName,flags):
    """DO NOT USE - handled by ptSDL"""
    pass

def PtDisableAvatarCursorFade():
    """Disable the avatar cursor fade"""
    pass

def PtDisableAvatarJump():
    """Disable the ability of the avatar to jump"""
    pass

def PtDisableControlKeyEvents(selfKey):
    """Disable the control key events from calling OnControlKeyEvent"""
    pass

def PtDisableForwardMovement():
    """Disable the ability of the avatar to move forward"""
    pass

def PtDisableMouseMovement():
    """Disable avatar mouse movement input"""
    pass

def PtDisableMovementKeys():
    """Disable avatar movement input"""
    pass

def PtDisableRenderScene():
    """UNKNOWN"""
    pass

def PtDisableShadows():
    """Turns shadows off"""
    pass

def PtDumpLogs(folder):
    """Dumps all current log files to the specified folder (a sub-folder to the log folder)"""
    pass

def PtEmoteAvatar(emote):
    """Play an emote on the local avatar (netpropagated)"""
    pass

def PtEnableAvatarCursorFade():
    """Enable the avatar cursor fade"""
    pass

def PtEnableAvatarJump():
    """Enable the ability of the avatar to jump"""
    pass

def PtEnableControlKeyEvents(selfKey):
    """Enable control key events to call OnControlKeyEvent(controlKey,activateFlag)"""
    pass

def PtEnableForwardMovement():
    """Enable the ability of the avatar to move forward"""
    pass

def PtEnableMouseMovement():
    """Enable avatar mouse movement input"""
    pass

def PtEnableMovementKeys():
    """Enable avatar movement input"""
    pass

def PtEnablePlanarReflections(on):
    """Enables/disables planar reflections"""
    pass

def PtEnableRenderScene():
    """UNKNOWN"""
    pass

def PtEnableShadows():
    """Turns shadows on"""
    pass

def PtExcludeRegionSet(senderKey,regionKey,state):
    """This will set the state of an exclude region
- 'senderKey' is a ptKey of the PythonFile component
- 'regionKey' is a ptKey of the exclude region
- 'state' is either kExRegRelease or kExRegClear"""
    pass

def PtExcludeRegionSetNow(senderKey,regionKey,state):
    """This will set the state of an exclude region immediately on the server
- 'senderKey' is a ptKey of the PythonFile component
- 'regionKey' is a ptKey of the exclude region
- 'state' is either kExRegRelease or kExRegClear"""
    pass

def PtFadeIn(lenTime, holdFlag, noSound=0):
    """Fades screen in for lenTime seconds"""
    pass

def PtFadeLocalAvatar(fade):
    """Fade (or unfade) the local avatar"""
    pass

def PtFadeOut(lenTime, holdFlag, noSound=0):
    """Fades screen out for lenTime seconds"""
    pass

def PtFakeLinkAvatarToObject(avatar,object):
    """Pseudo-links avatar to object within the same age
"""
    pass

def PtFileExists(filename):
    """Returns true if the specified file exists"""
    pass

def PtFindSceneobject(name,ageName):
    """This will try to find a sceneobject based on its name and what age its in
- it will return a ptSceneObject if found- if not found then a NameError exception will happen"""
    pass

def PtFirstPerson():
    """is the local avatar in first person mode"""
    pass

def PtFogSetDefColor(color):
    """Sets default fog color"""
    pass

def PtFogSetDefExp(end,density):
    """Set exp fog values"""
    pass

def PtFogSetDefExp2(end,density):
    """Set exp2 fog values"""
    pass

def PtFogSetDefLinear(start,end,density):
    """Set linear fog values"""
    pass

def PtForceCursorHidden():
    """Forces the cursor to hide, overriding everything.
Only call if other methods won't work. The only way to show the cursor after this call is PtForceMouseShown()"""
    pass

def PtForceCursorShown():
    """Forces the cursor to show, overriding everything.
Only call if other methods won't work. This is the only way to show the cursor after a call to PtForceMouseHidden()"""
    pass

def PtGMTtoDniTime(gtime):
    """Converts GMT time (passed in) to D'Ni time"""
    pass

def PtGUICursorDimmed():
    """Dimms the GUI cursor"""
    pass

def PtGUICursorOff():
    """Turns the GUI cursor off"""
    pass

def PtGUICursorOn():
    """Turns the GUI cursor on"""
    pass

def PtGetAccountName():
    """Returns the account name for the current account"""
    pass

def PtGetAccountPlayerList():
    """Returns list of players associated with the current account"""
    pass

def PtGetAgeInfo():
    """Returns ptAgeInfoStruct of the current Age"""
    pass

def PtGetAgeName():
    """DEPRECIATED - use ptDniInfoSource instead"""
    pass

def PtGetAgeSDL():
    """Returns the global ptSDL for the current Age"""
    pass

def PtGetAgeTime():
    """DEPRECIATED - use ptDniInfoSource instead"""
    pass

def PtGetAgeTimeOfDayPercent():
    """Returns the current age time of day as a percent (0 to 1)"""
    pass

def PtGetAvatarKeyFromClientID(clientID):
    """From an integer that is the clientID, find the avatar and return its ptKey"""
    pass

def PtGetCameraNumber(x):
    """Returns camera x's name from stack"""
    pass

def PtGetClientIDFromAvatarKey(avatarKey):
    """From a ptKey that points at an avatar, return the players clientID (integer)"""
    pass

def PtGetClientName(avatarKey=None):
    """This will return the name of the client that is owned by the avatar
- avatarKey is the ptKey of the avatar to get the client name of.
If avatarKey is omitted then the local avatar is used"""
    pass

def PtGetControlEvents(on, key):
    """Registers or unregisters for control event messages"""
    pass

def PtGetDefaultDisplayParams():
    """Returns the default resolution and display settings"""
    pass

def PtGetDefaultSpawnPoint():
    """Returns the default spawnpoint definition (as a ptSpawnPointInfo)"""
    pass

def PtGetDesktopColorDepth():
    """Returns desktop ColorDepth"""
    pass

def PtGetDesktopHeight():
    """Returns desktop height"""
    pass

def PtGetDesktopWidth():
    """Returns desktop width"""
    pass

def PtGetDialogFromString(dialogName):
    """Get a ptGUIDialog from its name"""
    pass

def PtGetDialogFromTagID(tagID):
    """Returns the dialog associated with the tagID"""
    pass

def PtGetDniTime():
    """Returns current D'Ni time"""
    pass

def PtGetFrameDeltaTime():
    """Returns the amount of time that has elapsed since last frame."""
    pass

def PtGetGameTime():
    """Returns the system game time (frame based) in seconds."""
    pass

def PtGetInitPath():
    """Returns the unicode path to the client's init directory. Do NOT convert to a standard string."""
    pass

def PtGetLanguage():
    """Returns the current language as a PtLanguage enum"""
    pass

def PtGetLocalAvatar():
    """This will return a ptSceneobject of the local avatar
- if there is no local avatar a NameError exception will happen."""
    pass

def PtGetLocalClientID():
    """Returns our local client ID number"""
    pass

def PtGetLocalKILevel():
    """returns local player's ki level"""
    pass

def PtGetLocalPlayer():
    """Returns a ptPlayer object of the local player"""
    pass

def PtGetLocalizedString(name, arguments=None):
    """Returns the localized string specified by name (format is Age.Set.Name) and substitutes the arguments in the list of strings passed in as arguments."""
    pass

def PtGetMouseTurnSensitivity():
    """Returns the sensitivity"""
    pass

def PtGetNumCameras():
    """returns camera stack size"""
    pass

def PtGetNumParticles(key):
    """Key is the key of scene object host to particle system"""
    pass

def PtGetNumRemotePlayers():
    """Returns the number of remote players in this Age with you."""
    pass

def PtGetPlayerList():
    """Returns a list of ptPlayer objects of all the remote players"""
    pass

def PtGetPlayerListDistanceSorted():
    """Returns a list of ptPlayers, sorted by distance"""
    pass

def PtGetPrevAgeInfo():
    """Returns ptAgeInfoStruct of previous age visited"""
    pass

def PtGetPrevAgeName():
    """Returns filename of previous age visited"""
    pass

def PtGetPublicAgeList(ageName, cbObject=None):
    """Get list of public ages for the given age name.
cbObject, if supplied should have a method called gotPublicAgeList(self,ageList). ageList is a list of tuple(ptAgeInfoStruct,nPlayersInAge)"""
    pass

def PtGetPythonLoggingLevel():
    """Returns the current level of python logging"""
    pass

def PtGetServerTime():
    """Returns the current time on the server (which is GMT)"""
    pass

def PtGetShadowVisDistance():
    """Returns the maximum shadow visibility distance"""
    pass

def PtGetSupportedDisplayModes():
    """Returns a list of supported resolutions"""
    pass

def PtGetTime():
    """Returns the number of seconds since the game was started."""
    pass

def PtGetUserPath():
    """Returns the unicode path to the client's root user directory. Do NOT convert to a standard string."""
    pass

def PtHideDialog(dialogName):
    """Hide a GUI dialog by name (does not unload dialog)"""
    pass

def PtIsActivePlayerSet():
    """Returns whether or not an active player is set"""
    pass

def PtIsCCRAway():
    """Returns current status of CCR dept"""
    pass

def PtIsClickToTurn():
    """Is click-to-turn on?"""
    pass

def PtIsCurrentBrainHuman():
    """Returns whether the local avatar current brain is the human brain"""
    pass

def PtIsDemoMode():
    """Returns whether the game is in Demo mode or not"""
    pass

def PtIsDialogLoaded(dialogName):
    """Test to see if a GUI dialog is loaded, by name"""
    pass

def PtIsEnterChatModeKeyBound():
    """Returns whether the EnterChatMode is bound to a key"""
    pass

def PtIsGUIModal():
    """Returns true if the GUI is displaying a modal dialog and blocking input"""
    pass

def PtIsInternalRelease():
    """Returns whether the client is an internal build or not"""
    pass

def PtIsMouseInverted():
    """Is the mouse currently inverted?"""
    pass

def PtIsShadowsEnabled():
    """Returns whether shadows are currently turned on"""
    pass

def PtIsSinglePlayerMode():
    """Returns whether the game is in single player mode or not"""
    pass

def PtIsSubscriptionActive():
    """Returns true if the current player is a paying subscriber"""
    pass

def PtKillParticles(timeRemaining,pctToKill,particleSystem):
    """Tells particleSystem to kill pctToKill percent of its particles"""
    pass

def PtLimitAvatarLOD(LODlimit):
    """Sets avatar's LOD limit"""
    pass

def PtLoadAvatarModel(modelName, spawnPoint, userStr = ""):
    """Loads an avatar model at the given spawn point. Assigns the user specified string to it."""
    pass

def PtLoadBookGUI(guiName):
    """Loads the gui specified, a gui must be loaded before it can be used. If the gui is already loaded, doesn't do anything"""
    pass

def PtLoadDialog(dialogName,selfKey=None,ageName=""):
    """Loads a GUI dialog by name and optionally set the Notify proc key
If the dialog is already loaded then it won't load it again"""
    pass

def PtLoadJPEGFromDisk(filename,width,height):
    """The image will be resized to fit the width and height arguments. Set to 0 if resizing is not desired.
Returns a pyImage of the specified file."""
    pass

def PtLocalAvatarIsMoving():
    """Returns true if the local avatar is moving (a movement key is held down)"""
    pass

def PtLocalAvatarRunKeyDown():
    """Returns true if the run key is being held down for the local avatar"""
    pass

def PtMaxListenDistSq():
    """Returns the maximum distance (squared) of the listen range"""
    pass

def PtMaxListenListSize():
    """Returns the maximum listen number of players"""
    pass

def PtNotifyOffererLinkAccepted(offerer):
    """Tell the offerer that we accepted the link offer"""
    pass

def PtNotifyOffererLinkCompleted(offerer):
    """Tell the offerer that we completed the link"""
    pass

def PtNotifyOffererLinkRejected(offerer):
    """Tell the offerer that we rejected the link offer"""
    pass

def PtPageInNode(nodeName, ageName=""):
    """Pages in node, or a list of nodes"""
    pass

def PtPageOutNode(nodeName):
    """Pages out a node"""
    pass

def PtPrintToScreen(message):
    """Prints 'message' to the status log, for debug only."""
    pass

def PtRateIt(chronicleName,dialogPrompt,onceFlag):
    """Shows a dialog with dialogPrompt and stores user input rating into chronicleName"""
    pass

def PtRebuildCameraStack(name,ageName):
    """Push camera with this name on the stack"""
    pass

def PtRecenterCamera():
    """re-centers the camera"""
    pass

def PtRemovePublicAge(ageInstanceGuid, cbObject=None):
    """Remove a public instance of the given age.
cbObject, if supplied should have a member called publicAgeRemoved(self,ageInstanceGuid)"""
    pass

def PtRequestLOSScreen(selfKey,ID,xPos,yPos,distance,what,reportType):
    """Request a LOS check from a point on the screen"""
    pass

def PtSaveScreenShot(fileName,width=640,height=480,quality=75):
    """Takes a screenshot with the specified filename, size, and quality"""
    pass

def PtSendChatToCCR(message,CCRPlayerID):
    """Sends a chat message to a CCR that has contacted this player"""
    pass

def PtSendKIGZMarkerMsg(markerNumber,sender):
    """Same as PtSendKIMessageInt except 'sender' could get a notify message back
"""
    pass

def PtSendKIMessage(command,value):
    """Sends a command message to the KI frontend.
See PlasmaKITypes.py for list of commands"""
    pass

def PtSendKIMessageInt(command,value):
    """Same as PtSendKIMessage except the value is guaranteed to be a UInt32
(for things like player IDs)"""
    pass

def PtSendPetitionToCCR(message,reason=0,title=""):
    """Sends a petition with a message to the CCR group"""
    pass

def PtSendPrivateChatList(chatList):
    """Lock the local avatar into private vox messaging, and / or add new members to his chat list"""
    pass

def PtSendRTChat(fromPlayer,toPlayerList,message,flags):
    """Sends a realtime chat message to the list of ptPlayers
If toPlayerList is an empty list, it is a broadcast message"""
    pass

def PtSetActivePlayer(playerInt):
    """Sets the active player associated with the current account"""
    pass

def PtSetAlarm(secs, cbObject, cbContext):
    """secs is the amount of time before your alarm goes off.
cbObject is a python object with the method onAlarm(int context)
cbContext is an integer."""
    pass

def PtSetBehaviorLoopCount(behaviorKey,stage,loopCount,netForce):
    """This will set the loop count for a particular stage in a multistage behavior"""
    pass

def PtSetBehaviorNetFlags(behKey, netForce, netProp):
    """Sets net flags on the associated behavior"""
    pass

def PtSetClearColor(red,green,blue):
    """Set the clear color"""
    pass

def PtSetClickToTurn(state):
    """Turns on click-to-turn"""
    pass

def PtSetGamma2(gamma):
    """Set the gamma with gamma2 rules"""
    pass

def PtSetGlobalClickability(enable):
    """Enable or disable all clickables on the local client"""
    pass

def PtSetGraphicsOptions(width, height, colordepth, windowed, numAAsamples, numAnisoSamples, VSync):
    """Set the graphics options"""
    pass

def PtSetLightAnimStart(key,name,start):
    """ Key is the key of scene object host to light, start is a bool. Name is the name of the light to manipulate"""
    pass

def PtSetLightValue(key,name,r,g,b,a):
    """ Key is the key of scene object host to light. Name is the name of the light to manipulate"""
    pass

def PtSetMouseInverted():
    """Inverts the mouse"""
    pass

def PtSetMouseTurnSensitivity(sensitivity):
    """Set the mouse sensitivity"""
    pass

def PtSetMouseUninverted():
    """Uninverts the mouse"""
    pass

def PtSetOfferBookMode(selfkey,ageFilename,ageInstanceName):
    """Put us into the offer book interface"""
    pass

def PtSetParticleDissentPoint(x, y, z, particlesys):
    """Sets the dissent point of the particlesys to x,y,z"""
    pass

def PtSetParticleOffset(x,y,z,particlesys):
    """Sets the particlesys particle system's offset"""
    pass

def PtSetPythonLoggingLevel(level):
    """Sets the current level of python logging"""
    pass

def PtSetShadowVisDistance(distance):
    """Set the maximum shadow visibility distance"""
    pass

def PtSetShareSpawnPoint(spawnPoint):
    """This sets the desired spawn point for the receiver to link to"""
    pass

def PtShootBulletFromObject(selfkey, gunObj, radius, range):
    """Shoots a bullet from an object"""
    pass

def PtShootBulletFromScreen(selfkey, xPos, yPos, radius, range):
    """Shoots a bullet from a position on the screen"""
    pass

def PtShowDialog(dialogName):
    """Show a GUI dialog by name (does not load dialog)"""
    pass

def PtStartScreenCapture(selfKey,width=800,height=600):
    """Starts a capture of the screen"""
    pass

def PtToggleAvatarClickability(on):
    """Turns on and off our avatar's clickability"""
    pass

def PtTransferParticlesToObject(objFrom, objTo, num):
    """Transfers num particles from objFrom to objTo"""
    pass

def PtUnLoadAvatarModel(avatarKey):
    """Unloads the specified avatar model"""
    pass

def PtUnloadAllBookGUIs():
    """Unloads all loaded guis except for the default one"""
    pass

def PtUnloadBookGUI(guiName):
    """Unloads the gui specified. If the gui isn't loaded, doesn't do anything"""
    pass

def PtUnloadDialog(dialogName):
    """This will unload the GUI dialog by name. If not loaded then nothing will happen"""
    pass

def PtUpgradeVisitorToExplorer(playerInt):
    """Upgrades the player to explorer status"""
    pass

def PtUsingUnicode():
    """Returns true if the current language is a unicode language (like Japanese)"""
    pass

def PtValidateKey(key):
    """Returns true(1) if 'key' is valid and loaded,
otherwise returns false(0)"""
    pass

def PtWasLocallyNotified(selfKey):
    """Returns 1 if the last notify was local or 0 if the notify originated on the network"""
    pass

def PtWearDefaultClothing(key):
    """Forces the avatar to wear the default clothing set"""
    pass

def PtWearDefaultClothingType(key,type):
    """Forces the avatar to wear the default clothing of the specified type"""
    pass

def PtWearMaintainerSuit(key,wearOrNot):
    """Wears or removes the maintainer suit of clothes"""
    pass

def PtWhatGUIControlType(guiKey):
    """Returns the control type of the key passed in"""
    pass

def PtYesNoDialog(selfkey,dialogMessage):
    """This will display a Yes/No dialog to the user with the text dialogMessage
This dialog _has_ to be answered by the user.
And their answer will be returned in a Notify message."""
    pass

class ptAgeInfoStruct:
    """Class to hold AgeInfo struct data"""
    def __init__(self):
        """None"""
        pass

    def copyFrom(self,other):
        """Copies data from one ptAgeInfoStruct or ptAgeInfoStructRef to this one"""
        pass

    def getAgeFilename(self):
        """Gets the Age's filename"""
        pass

    def getAgeInstanceGuid(self):
        """Get the Age's instance GUID"""
        pass

    def getAgeInstanceName(self):
        """Get the instance name of the Age"""
        pass

    def getAgeLanguage(self):
        """Gets the age's language (integer)"""
        pass

    def getAgeSequenceNumber(self):
        """Gets the unique sequence number"""
        pass

    def getAgeUserDefinedName(self):
        """Gets the user defined part of the Age name"""
        pass

    def getDisplayName(self):
        """Returns a string that is the displayable name of the age instance"""
        pass

    def setAgeFilename(self,filename):
        """Sets the filename of the Age"""
        pass

    def setAgeInstanceGuid(self,guid):
        """Sets the Age instance's GUID"""
        pass

    def setAgeInstanceName(self,instanceName):
        """Sets the instance name of the Age"""
        pass

    def setAgeLanguage(self,lang):
        """Sets the age's language (integer)"""
        pass

    def setAgeSequenceNumber(self,seqNumber):
        """Sets the unique sequence number"""
        pass

    def setAgeUserDefinedName(self,udName):
        """Sets the user defined part of the Age"""
        pass

class ptAgeInfoStructRef:
    """Class to hold AgeInfo struct data"""
    def __init__(self):
        """None"""
        pass

    def copyFrom(self,other):
        """Copies data from one ptAgeInfoStruct or ptAgeInfoStructRef to this one"""
        pass

    def getAgeFilename(self):
        """Gets the Age's filename"""
        pass

    def getAgeInstanceGuid(self):
        """Get the Age's instance GUID"""
        pass

    def getAgeInstanceName(self):
        """Get the instance name of the Age"""
        pass

    def getAgeSequenceNumber(self):
        """Gets the unique sequence number"""
        pass

    def getAgeUserDefinedName(self):
        """Gets the user defined part of the Age name"""
        pass

    def getDisplayName(self):
        """Returns a string that is the displayable name of the age instance"""
        pass

    def setAgeFilename(self,filename):
        """Sets the filename of the Age"""
        pass

    def setAgeInstanceGuid(self,guid):
        """Sets the Age instance's GUID"""
        pass

    def setAgeInstanceName(self,instanceName):
        """Sets the instance name of the Age"""
        pass

    def setAgeSequenceNumber(self,seqNumber):
        """Sets the unique sequence number"""
        pass

    def setAgeUserDefinedName(self,udName):
        """Sets the user defined part of the Age"""
        pass

class ptAgeLinkStruct:
    """Class to hold the data of the AgeLink structure"""
    def __init__(self):
        """None"""
        pass

    def copyFrom(self,other):
        """Copies data from one ptAgeLinkStruct or ptAgeLinkStructRef to this one"""
        pass

    def getAgeInfo(self):
        """Returns a ptAgeInfoStructRef of the AgeInfo for this link"""
        pass

    def getLinkingRules(self):
        """Returns the linking rules of this link"""
        pass

    def getParentAgeFilename(self):
        """Returns a string of the parent age filename"""
        pass

    def getSpawnPoint(self):
        """Gets the spawn point ptSpawnPointInfoRef of this link"""
        pass

    def setAgeInfo(self,ageInfo):
        """Sets the AgeInfoStruct from the data in ageInfo (a ptAgeInfoStruct)"""
        pass

    def setLinkingRules(self,rule):
        """Sets the linking rules for this link"""
        pass

    def setParentAgeFilename(self,filename):
        """Sets the parent age filename for child age links"""
        pass

    def setSpawnPoint(self,spawnPtInfo):
        """Sets the spawn point of this link (a ptSpawnPointInfo or ptSpawnPointInfoRef)"""
        pass

class ptAgeLinkStructRef:
    """Class to hold the data of the AgeLink structure"""
    def __init__(self):
        """None"""
        pass

    def copyFrom(self,other):
        """Copies data from one ptAgeLinkStruct or ptAgeLinkStructRef to this one"""
        pass

    def getAgeInfo(self):
        """Returns a ptAgeInfoStructRef of the AgeInfo for this link"""
        pass

    def getLinkingRules(self):
        """Returns the linking rules of this link"""
        pass

    def getSpawnPoint(self):
        """Gets the spawn point ptSpawnPointInfoRef of this link"""
        pass

    def setAgeInfo(self,ageInfo):
        """Sets the AgeInfoStruct from the data in ageInfo (a ptAgeInfoStruct)"""
        pass

    def setLinkingRules(self,rule):
        """Sets the linking rules for this link"""
        pass

    def setSpawnPoint(self,spawnPtInfo):
        """Sets the spawn point of this link (a ptSpawnPointInfo or ptSpawnPointInfoRef)"""
        pass

class ptAgeVault:
    """Accessor class to the Age's vault"""
    def __init__(self):
        """None"""
        pass

    def addChronicleEntry(self,name,type,value):
        """Adds a chronicle entry with the specified type and value"""
        pass

    def addDevice(self,deviceName,cb=None,cbContext=0):
        """Adds a device to the age"""
        pass

    def findChronicleEntry(self,entryName):
        """Returns the named ptVaultChronicleNode"""
        pass

    def getAgeDevicesFolder(self):
        """Returns a ptVaultFolderNode of the inboxes for the devices in this Age."""
        pass

    def getAgeGuid(self):
        """Returns the current Age's guid as a string."""
        pass

    def getAgeInfo(self):
        """Returns a ptVaultAgeInfoNode of the this Age"""
        pass

    def getAgeSDL(self):
        """Returns the age's SDL (ptSDLStateDataRecord)"""
        pass

    def getAgesIOwnFolder(self):
        """(depreciated, use getBookshelfFolder) Returns a ptVaultFolderNode that contain the Ages I own"""
        pass

    def getBookshelfFolder(self):
        """Personal age only: Returns a ptVaultFolderNode that contains the owning player's AgesIOwn age list"""
        pass

    def getChronicleFolder(self):
        """Returns a ptVaultFolderNode"""
        pass

    def getDevice(self,deviceName):
        """Returns the specified device (ptVaultTextNoteNode)"""
        pass

    def getDeviceInbox(self,deviceName):
        """Returns a ptVaultFolderNode of the inbox for the named device in this age."""
        pass

    def getPeopleIKnowAboutFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the players the Age knows about(?)."""
        pass

    def getPublicAgesFolder(self):
        """Returns a ptVaultFolderNode that contains all the public Ages"""
        pass

    def getSubAgeLink(self,ageInfo):
        """Returns a ptVaultAgeLinkNode to 'ageInfo' (a ptAgeInfoStruct) for this Age."""
        pass

    def getSubAgesFolder(self):
        """Returns a ptVaultFolderNode of sub Age's folder."""
        pass

    def hasDevice(self,deviceName):
        """Does a device with this name exist?"""
        pass

    def removeDevice(self,deviceName):
        """Removes a device from the age"""
        pass

    def setDeviceInbox(self,deviceName,inboxName,cb=None,cbContext=0):
        """Set's the device's inbox"""
        pass

    def updateAgeSDL(self,pyrec):
        """Updates the age's SDL"""
        pass

class ptAnimation:
    """Plasma animation class"""
    def __init__(self,key=None):
        """None"""
        pass

    def addKey(self,key):
        """Adds an animation modifier to the list of receiver keys"""
        pass

    def backwards(self,backwardsFlag):
        """Turn on and off playing the animation backwards"""
        pass

    def getFirstKey(self):
        """This will return a ptKey object that is the first receiver (target)
However, if the parent is not a modifier or not loaded, then None is returned."""
        pass

    def incrementBackward(self):
        """Step the animation backward a frame"""
        pass

    def incrementForward(self):
        """Step the animation forward a frame"""
        pass

    def looped(self,loopedFlag):
        """Turn on and off looping of the animation"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

    def play(self):
        """Plays the animation"""
        pass

    def playRange(self,start,end):
        """Play the animation from start to end"""
        pass

    def playToPercentage(self,zeroToOne):
        """Play the animation to the specified percentage (0 to 1)"""
        pass

    def playToTime(self,time):
        """Play the animation to the specified time"""
        pass

    def resume(self):
        """Resumes the animation from where it was stopped last"""
        pass

    def sender(self,selfKey):
        """Sets the sender of the messages being sent to the animation modifier"""
        pass

    def setAnimName(self,name):
        """Sets the animation notetrack name (or (Entire Animation))"""
        pass

    def setLoopEnd(self,loopEnd):
        """Sets the loop ending position
- 'loopEnd' is the number of seconds from the absolute beginning of the animation"""
        pass

    def setLoopStart(self,loopStart):
        """Sets the loop starting position
- 'loopStart' is the number of seconds from the absolute beginning of the animation"""
        pass

    def skipToBegin(self):
        """Skip to the beginning of the animation (don't play)"""
        pass

    def skipToEnd(self):
        """Skip to the end of the animation (don't play)"""
        pass

    def skipToLoopBegin(self):
        """Skip to the beginning of the animation loop (don't play)"""
        pass

    def skipToLoopEnd(self):
        """Skip to the end of the animation loop (don't play)"""
        pass

    def skipToTime(self,time):
        """Skip the animation to time (don't play)"""
        pass

    def speed(self,speed):
        """Sets the animation playback speed"""
        pass

    def stop(self):
        """Stops the animation"""
        pass

class ptAudioControl:
    """Accessor class to the Audio controls"""
    def __init__(self):
        """None"""
        pass

    def canSetMicLevel(self):
        """Can the microphone level be set? Returns 1 if true otherwise returns 0."""
        pass

    def disable(self):
        """Disabled audio"""
        pass

    def enable(self):
        """Enables audio"""
        pass

    def enableVoiceChat(self,state):
        """Enables or disables voice chat."""
        pass

    def enableVoiceCompression(self,state):
        """Enables or disables voice compression."""
        pass

    def enableVoiceNetBroadcast(self,state):
        """Enables or disables voice over network broadcast."""
        pass

    def enableVoiceRecording(self,state):
        """Enables or disables voice recording."""
        pass

    def getAmbienceVolume(self):
        """Returns the volume (0.0 to 1.0) for the Ambiance."""
        pass

    def getAudioDeviceName(self,index):
        """Gets the name of audio device for the given index"""
        pass

    def getDeviceName(self):
        """Gets the name for the device being used by the audio system"""
        pass

    def getGUIVolume(self):
        """Returns the volume (0.0 to 1.0) for the GUI dialogs."""
        pass

    def getHighestMode(self):
        """Gets the highest possible audio system mode"""
        pass

    def getMicLevel(self):
        """Returns the microphone recording level (0.0 to 1.0)."""
        pass

    def getMode(self):
        """Gets the audio system mode"""
        pass

    def getMusicVolume(self):
        """Returns the volume (0.0 to 1.0) for the Music."""
        pass

    def getNPCVoiceVolume(self):
        """Returns the volume (0.0 to 1.0) for the NPC's voice."""
        pass

    def getNumAudioDevices(self):
        """Returns the number of available audio devices."""
        pass

    def getPriorityCutoff(self):
        """Returns current sound priority"""
        pass

    def getSoundFXVolume(self):
        """Returns the volume (0.0 to 1.0) for the Sound FX."""
        pass

    def getVoiceVolume(self):
        """Returns the volume (0.0 to 1.0) for the Voices."""
        pass

    def hideIcons(self):
        """Hides (disables) the voice recording icons."""
        pass

    def isEnabled(self):
        """Is the audio enabled? Returns 1 if true otherwise returns 0."""
        pass

    def isHardwareAccelerated(self):
        """Is audio hardware acceleration enabled? Returns 1 if true otherwise returns 0."""
        pass

    def isMuted(self):
        """Are all sounds muted? Returns 1 if true otherwise returns 0."""
        pass

    def isUsingEAXAcceleration(self):
        """Is EAX sound acceleration enabled? Returns 1 if true otherwise returns 0."""
        pass

    def isVoiceCompressionEnabled(self):
        """Is voice compression enabled? Returns 1 if true otherwise returns 0."""
        pass

    def isVoiceNetBroadcastEnabled(self):
        """Is voice over net enabled? Returns 1 if true otherwise returns 0."""
        pass

    def isVoiceRecordingEnabled(self):
        """Is voice recording enabled? Returns 1 if true otherwise returns 0."""
        pass

    def muteAll(self):
        """Mutes all sounds."""
        pass

    def pushToTalk(self,state):
        """Enables or disables 'push-to-talk'."""
        pass

    def recordFrame(self,size):
        """Sets the voice packet frame size."""
        pass

    def recordSampleRate(self,sampleRate):
        """Sets the recording sample rate."""
        pass

    def setAmbienceVolume(self,volume):
        """Sets the Ambience volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        pass

    def setDeviceName(self,devicename,restart):
        """Sets the device name for the audio system, and optionally restarts it"""
        pass

    def setGUIVolume(self,volume):
        """Sets the GUI dialog volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        pass

    def setLoadOnDemand(self,state):
        """Enables or disables the load on demand for sounds."""
        pass

    def setMicLevel(self,level):
        """Sets the microphone recording level (0.0 to 1.0)."""
        pass

    def setMode(self,mode):
        """Sets the audio system mode"""
        pass

    def setMusicVolume(self,volume):
        """Sets the Music volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        pass

    def setNPCVoiceVolume(self,volume):
        """Sets the NPC's voice volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        pass

    def setPriorityCutoff(self,priority):
        """Sets the sound priority"""
        pass

    def setSoundFXVolume(self,volume):
        """Sets the SoundFX volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        pass

    def setTwoStageLOD(self,state):
        """Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
...Less of a performance hit, harder on memory."""
        pass

    def setVoiceVolume(self,volume):
        """Sets the Voice volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        pass

    def showIcons(self):
        """Shows (enables) the voice recording icons."""
        pass

    def squelchLevel(self,level):
        """Sets the squelch level."""
        pass

    def supportsEAX(self):
        """Returns true or false based on whether or not a the device specified supports EAX"""
        pass

    def unmuteAll(self):
        """Unmutes all sounds."""
        pass

    def useEAXAcceleration(self,state):
        """Enables or disables EAX sound acceleration (requires hardware acceleration)."""
        pass

    def useHardwareAcceleration(self,state):
        """Enables or disables audio hardware acceleration."""
        pass

class ptAvatar:
    """Plasma avatar class"""
    def __init__(self):
        """None"""
        pass

    def addWardrobeClothingItem(self,clothing_name,tint1,tint2):
        """To add a clothing item to the avatar's wardrobe (closet)"""
        pass

    def enterSubWorld(self,sceneobject):
        """Places the avatar into the subworld of the ptSceneObject specified"""
        pass

    def exitSubWorld(self):
        """Exits the avatar from the subWorld where it was"""
        pass

    def getAllWithSameMesh(self,clothing_name):
        """Returns a lilst of all clothing items that use the same mesh as the specified one"""
        pass

    def getAvatarClothingGroup(self):
        """Returns what clothing group the avatar belongs to.
It is also a means to determine if avatar is male or female"""
        pass

    def getAvatarClothingList(self):
        """Returns a list of clothes that the avatar is currently wearing."""
        pass

    def getClosetClothingList(self,clothing_type):
        """Returns a list of clothes for the avatar that are in specified clothing group."""
        pass

    def getCurrentMode(self):
        """Returns current brain mode for avatar"""
        pass

    def getEntireClothingList(self,clothing_type):
        """Gets the entire list of clothing available. 'clothing_type' not used
NOTE: should use getClosetClothingList"""
        pass

    def getMatchingClothingItem(self,clothingName):
        """Finds the matching clothing item that goes with 'clothingName'
Used to find matching left and right gloves and shoes."""
        pass

    def getMorph(self,clothing_name,layer):
        """Get the current morph value"""
        pass

    def getSkinBlend(self,layer):
        """Get the current skin blend value"""
        pass

    def getTintClothingItem(self,clothing_name,layer=1):
        """Returns a ptColor of a particular item of clothing that the avatar is wearing.
The color will be a ptColor object."""
        pass

    def getTintSkin(self):
        """Returns a ptColor of the current skin tint for the avatar"""
        pass

    def getUniqueMeshList(self,clothing_type):
        """Returns a list of unique clothing items of the desired type (different meshes)"""
        pass

    def getWardrobeClothingList(self):
        """Return a list of items that are in the avatars closet"""
        pass

    def gotoStage(self,behaviorKey,stage,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce):
        """Tells a multistage behavior to go to a particular stage"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

    def nextStage(self,behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce):
        """Tells a multistage behavior to go to the next stage (Why does Matt like so many parameters?)"""
        pass

    def oneShot(self,seekKey,duration,usePhysicsFlag,animationName,drivableFlag,reversibleFlag):
        """Plays a one-shot animation on the avatar"""
        pass

    def playSimpleAnimation(self,animName):
        """Play simple animation on avatar"""
        pass

    def previousStage(self,behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce):
        """Tells a multistage behavior to go to the previous stage"""
        pass

    def registerForBehaviorNotify(self,selfKey):
        """This will register for behavior notifies from the avatar"""
        pass

    def removeClothingItem(self,clothing_name,update=1):
        """Tells the avatar to remove a particular item of clothing."""
        pass

    def runBehavior(self,behaviorKey,netForceFlag):
        """Runs a behavior on the avatar. Can be a single or multi-stage behavior."""
        pass

    def runBehaviorSetNotify(self,behaviorKey,replyKey,netForceFlag):
        """Same as runBehavior, except send notifications to specified keyed object"""
        pass

    def saveClothing(self):
        """Saves the current clothing options (including morphs) to the vault"""
        pass

    def setMorph(self,clothing_name,layer,value):
        """Set the morph value (clipped between -1 and 1)"""
        pass

    def setReplyKey(self,key):
        """Sets the sender's key"""
        pass

    def setSkinBlend(self,layer,value):
        """Set the skin blend (value between 0 and 1)"""
        pass

    def tintClothingItem(self,clothing_name,tint,update=1):
        """Tells the avatar to tint(color) a particular item of clothing that they are already wearing.
'tint' is a ptColor object"""
        pass

    def tintClothingItemLayer(self,clothing_name,tint,layer,update=1):
        """Tells the avatar to tint(color) a particular layer of a particular item of clothing."""
        pass

    def tintSkin(self,tint,update=1):
        """Tints all of the skin on the avatar, with the ptColor tint"""
        pass

    def unRegisterForBehaviorNotify(self,selfKey):
        """This will unregister behavior notifications"""
        pass

    def wearClothingItem(self,clothing_name,update=1):
        """Tells the avatar to wear a particular item of clothing.
And optionally hold update until later (for applying tinting before wearing)."""
        pass

class ptBook:
    """Creates a new book"""
    def __init__(self,esHTMLSource,coverImage=None,callbackKey=None,guiName=''):
        """None"""
        pass

    def allowPageTurning(self,allow):
        """Turns on and off the ability to flip the pages in a book"""
        pass

    def close(self):
        """Closes the book"""
        pass

    def closeAndHide(self):
        """Closes the book and hides it once it finishes animating"""
        pass

    def getCurrentPage(self):
        """Returns the currently shown page"""
        pass

    def getEditableText(self):
        """Returns the editable text currently contained in the book."""
        pass

    def getMovie(self,index):
        """Grabs a ptAnimation object representing the movie indexed by index. The index is the index of the movie in the source code"""
        pass

    def goToPage(self,page):
        """Flips the book to the specified page"""
        pass

    def hide(self):
        """Hides the book"""
        pass

    def nextPage(self):
        """Flips the book to the next page"""
        pass

    def open(self,startingPage):
        """Opens the book to the specified page"""
        pass

    def previousPage(self):
        """Flips the book to the previous page"""
        pass

    def setEditable(self,editable):
        """Turn book editing on or off. If the book GUI does not support editing, nothing will happen"""
        pass

    def setEditableText(self,text):
        """Sets the book's editable text."""
        pass

    def setGUI(self,guiName):
        """Sets the gui to be used by the book, if the requested gui is not loaded, it will use the default
Do not call while the book is open!"""
        pass

    def setPageMargin(self,margin):
        """Sets the text margin for the book"""
        pass

    def setSize(self,width,height):
        """Sets the size of the book (width and height are floats from 0 to 1)"""
        pass

    def show(self,startOpened):
        """Shows the book closed, or open if the the startOpened flag is true"""
        pass

class ptCCRAge:
    """CCR only: CCR age info struct"""
    def __init__(self):
        """None"""
        pass

class ptCCRMgr:
    """CCR only: accessor class to the CCR manager"""
    def __init__(self):
        """None"""
        pass

    def banLinking(self,pid, banFlag):
        """Set the ban linking flag for a player"""
        pass

    def beginCommunication(self,pid, message):
        """Begin a CCR communication with a player"""
        pass

    def clippingOff(self):
        """Disables clipping for this player"""
        pass

    def clippingOn(self):
        """Enables clipping for this player"""
        pass

    def endCommunication(self,pid):
        """End CCR communications with a player"""
        pass

    def getClipping(self):
        """Is clipping on for this player? Returns 1 if true otherwise returns 0"""
        pass

    def getErrorString(self,errorNumber):
        """Returns the error string that corresponds to 'errorNumber'"""
        pass

    def getLevel(self):
        """Returns the current CCR level for this player"""
        pass

    def getPlayerInfo(self,player, cbObject, cbContext):
        """Finds a player that matches 'player' (which is an id or name)."""
        pass

    def linkPlayerHere(self,pid):
        """Links player to where I am"""
        pass

    def linkPlayerToAge(self,ageInfoStruct,pid):
        """Links player to a specified age"""
        pass

    def linkToAge(self,age,pid):
        """Links to player's version of age"""
        pass

    def linkToMyNeighborhoodAge(self,pid):
        """Links this player to their neighborhood"""
        pass

    def linkToMyPersonalAge(self,pid):
        """Links this player to their personal Age."""
        pass

    def linkToPlayersAge(self,pid):
        """Link to where the player is"""
        pass

    def logMessage(self,message):
        """Logs 'message' somewhere...?"""
        pass

    def makeInvisible(self,level):
        """Makes this player invisible to 'level'"""
        pass

    def sendCommunication(self,pid, message):
        """Send a CCR communication to a player"""
        pass

    def setAwayStatus(self,awayFlag):
        """Set the away flag for CCRs"""
        pass

    def silencePlayer(self,pid, silenceFlag):
        """Set the silence player flag for a player"""
        pass

    def systemMessage(self):
        """Params message
Send a system wide CCR message"""
        pass

    def toggleClipping(self):
        """Toggles clipping for this player"""
        pass

    def warpPlayerHere(self,pid):
        """warp the player to here"""
        pass

    def warpToPlayer(self,pid):
        """warp to where the player is"""
        pass

class ptCCRPlayerInfo:
    """CCR only: CCR player info struct"""
    def __init__(self):
        """None"""
        pass

class ptCamera:
    """Plasma camera class"""
    def __init__(self):
        """None"""
        pass

    def controlKey(self,controlKey,activateFlag):
        """Send a control key to the camera as if it was hit by the user.
This is for sending things like pan-up, pan-down, zoom-in, etc."""
        pass

    def disableFirstPersonOverride(self):
        """Does _not_ allow the user to override the camera to go to first person camera."""
        pass

    def enableFirstPersonOverride(self):
        """Allows the user to override the camera and go to a first person camera."""
        pass

    def getFOV(self):
        """Returns the current camera's FOV(h)"""
        pass

    def isSmootherCam(self):
        """Returns true if we are using the faster cams thing"""
        pass

    def isStayInFirstPerson(self):
        """Are we staying in first person?"""
        pass

    def isWalkAndVerticalPan(self):
        """Returns true if we are walking and chewing gum"""
        pass

    def restore(self,cameraKey):
        """Restores camera to saved one"""
        pass

    def save(self,cameraKey):
        """Saves the current camera and sets the camera to cameraKey"""
        pass

    def set(self,cameraKey,time,save):
        """DO NOT USE"""
        pass

    def setFOV(self,fov, time):
        """Sets the current cameras FOV (based on h)"""
        pass

    def setSmootherCam(self,state):
        """Set the faster cams thing"""
        pass

    def setStayInFirstPerson(self,state):
        """Set Stay In First Person Always"""
        pass

    def setWalkAndVerticalPan(self,state):
        """Set Walk and chew gum"""
        pass

    def undoFirstPerson(self):
        """If the user has overridden the camera to be in first person, this will take them out of first person.
If the user didn't override the camera, then this will do nothing."""
        pass

class ptCluster:
    """Creates a new ptCluster"""
    def __init__(self,ey):
        """None"""
        pass

    def setVisible(self,isible):
        """Shows or hides the cluster object"""
        pass

class ptColor:
    """Plasma color class"""
    def __init__(self,red=0, green=0, blue=0, alpha=0):
        """None"""
        pass

    def black(self):
        """Sets the color to be black
Example: black = ptColor().black()"""
        pass

    def blue(self):
        """Sets the color to be blue
Example: blue = ptColor().blue()"""
        pass

    def brown(self):
        """Sets the color to be brown
Example: brown = ptColor().brown()"""
        pass

    def cyan(self):
        """Sets the color to be cyan
Example: cyan = ptColor.cyan()"""
        pass

    def darkbrown(self):
        """Sets the color to be darkbrown
Example: darkbrown = ptColor().darkbrown()"""
        pass

    def darkgreen(self):
        """Sets the color to be darkgreen
Example: darkgreen = ptColor().darkgreen()"""
        pass

    def darkpurple(self):
        """Sets the color to be darkpurple
Example: darkpurple = ptColor().darkpurple()"""
        pass

    def getAlpha(self):
        """Get the alpha blend component of the color"""
        pass

    def getBlue(self):
        """Get the blue component of the color"""
        pass

    def getGreen(self):
        """Get the green component of the color"""
        pass

    def getRed(self):
        """Get the red component of the color"""
        pass

    def gray(self):
        """Sets the color to be gray
Example: gray = ptColor().gray()"""
        pass

    def green(self):
        """Sets the color to be green
Example: green = ptColor().green()"""
        pass

    def magenta(self):
        """Sets the color to be magenta
Example: magenta = ptColor().magenta()"""
        pass

    def maroon(self):
        """Sets the color to be maroon
Example: maroon = ptColor().maroon()"""
        pass

    def navyblue(self):
        """Sets the color to be navyblue
Example: navyblue = ptColor().navyblue()"""
        pass

    def orange(self):
        """Sets the color to be orange
Example: orange = ptColor().orange()"""
        pass

    def pink(self):
        """Sets the color to be pink
Example: pink = ptColor().pink()"""
        pass

    def red(self):
        """Sets the color to be red
Example: red = ptColor().red()"""
        pass

    def setAlpha(self,alpha):
        """Set the alpha blend component of the color. 0.0 to 1.0"""
        pass

    def setBlue(self,blue):
        """Set the blue component of the color. 0.0 to 1.0"""
        pass

    def setGreen(self,green):
        """Set the green component of the color. 0.0 to 1.0"""
        pass

    def setRed(self,red):
        """Set the red component of the color. 0.0 to 1.0"""
        pass

    def slateblue(self):
        """Sets the color to be slateblue
Example: slateblue = ptColor().slateblue()"""
        pass

    def steelblue(self):
        """Sets the color to be steelblue
Example: steelblue = ptColor().steelblue()"""
        pass

    def tan(self):
        """Sets the color to be tan
Example: tan = ptColor().tan()"""
        pass

    def white(self):
        """Sets the color to be white
Example: white = ptColor().white()"""
        pass

    def yellow(self):
        """Sets the color to be yellow
Example: yellow = ptColor().yellow()"""
        pass

class ptCritterBrain:
    """Object to manipulate critter brains"""
    def __init__(self):
        """None"""
        pass

    def addBehavior(self,animName, behaviorName, loop = 1, randomStartPos = 1, fadeInLen = 2.0, fadeOutLen = 2.0):
        """Adds a new animation to the brain as a behavior with the specified name and parameters. If multiple animations are assigned to the same behavior, they will be randomly picked from when started."""
        pass

    def addReceiver(self,key):
        """Tells the brain that the specified key wants AI messages"""
        pass

    def animationName(self,behavior):
        """Returns the animation name associated with the specified integral behavior."""
        pass

    def atGoal(self):
        """Are we currently are our final destination?"""
        pass

    def avoidingAvatars(self):
        """Are we currently avoiding avatars while pathfinding?"""
        pass

    def behaviorName(self,behavior):
        """Returns the behavior name associated with the specified integral behavior."""
        pass

    def canHearAvatar(self,avatarID):
        """Returns whether this brain can hear the avatar with the specified id."""
        pass

    def canSeeAvatar(self,avatarID):
        """Returns whether this brain can see the avatar with the specified id."""
        pass

    def curBehavior(self):
        """Returns the current integral behavior the brain is running."""
        pass

    def currentGoal(self):
        """Returns the current ptPoint that the brain is running towards."""
        pass

    def getHearingDistance(self):
        """Returns how far away the brain can hear."""
        pass

    def getLocallyControlled(self):
        """Are we the one making AI decisions? NOTE: Not set automatically, some python script needs to tell the brain this using setLocallyControlled()."""
        pass

    def getSightCone(self):
        """Returns the width of the brain's field of view in radians."""
        pass

    def getSightDistance(self):
        """Returns how far the brain can see."""
        pass

    def getStopDistance(self):
        """Returns how far away from the goal we could be and still be considered there."""
        pass

    def goToGoal(self,newGoal, avoidingAvatars = 0):
        """Tells the brain to start running towards the specified location, avoiding avatars it can see or hear if told to."""
        pass

    def idleBehaviorName(self):
        """Returns the name of the brain's idle behavior."""
        pass

    def nextBehavior(self):
        """Returns the behavior the brain will be switching to next frame. (-1 if no change)"""
        pass

    def playersICanHear(self):
        """Returns a list of player ids which this brain can hear."""
        pass

    def playersICanSee(self):
        """Returns a list of player ids which this brain can see."""
        pass

    def removeReceiver(self,key):
        """Tells the brain that the specified key no longer wants AI messages"""
        pass

    def runBehaviorName(self):
        """Returns the name of the brain's run behavior."""
        pass

    def runningBehavior(self,behaviorName):
        """Returns true if the named behavior is running."""
        pass

    def setHearingDistance(self,dist):
        """Set how far away the brain can hear (360 degree field of hearing)."""
        pass

    def setLocallyControlled(self,local):
        """Tells the brain that we are the ones making all the AI decisions, and to prop location and other information to the server."""
        pass

    def setSightCone(self,radians):
        """Set how wide the brain's field of view is in radians. Note that it is the total angle of the cone, half on one side of the brain's line of sight, half on the other."""
        pass

    def setSightDistance(self,dist):
        """Set how far away the brain can see."""
        pass

    def setStopDistance(self,dist):
        """Set how far away from the goal we should be when we are considered there and stop running."""
        pass

    def startBehavior(self,behaviorName, fade = 1):
        """Starts playing the named behavior. If fade is true, it will fade out the previous behavior and fade in the new one. If false, they will immediately switch."""
        pass

    def vectorToPlayer(self,avatarID):
        """Returns the vector between us and the specified player."""
        pass

class ptDniCoordinates:
    """Constructor for a D'Ni coordinate"""
    def __init__(self):
        """None"""
        pass

    def fromPoint(self,pt):
        """Update these coordinates with the specified ptPoint3"""
        pass

    def getHSpans(self):
        """Returns the HSpans component of the coordinate"""
        pass

    def getTorans(self):
        """Returns the Torans component of the coordinate"""
        pass

    def getVSpans(self):
        """Returns the VSpans component of the coordinate"""
        pass

    def update(self):
        """Update these coordinates with the players current position"""
        pass

class ptDniInfoSource:
    """DO NOT USE"""
    def __init__(self):
        """None"""
        pass

    def getAgeCoords(self):
        """Current coords of the player in current age as a ptDniCoordinates"""
        pass

    def getAgeGuid(self):
        """Unique identifier for this age instance"""
        pass

    def getAgeName(self):
        """Name of current age"""
        pass

    def getAgeTime(self):
        """Current time in current age (tbd)"""
        pass

class ptDraw:
    """Plasma Draw class"""
    def __init__(self):
        """None"""
        pass

    def disable(self):
        """Disables the draw on the sceneobject attached
In other words, makes it invisible"""
        pass

    def enable(self,state=1):
        """Sets the draw enable for the sceneobject attached"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

class ptDynamicMap:
    """Creates a ptDynamicMap object"""
    def __init__(self,key=None):
        """None"""
        pass

    def addKey(self,key):
        """Add a receiver... in other words a DynamicMap"""
        pass

    def calcTextExtents(self,text):
        """Calculates the extent of the specified text, returns it as a (width, height) tuple"""
        pass

    def clearKeys(self):
        """Clears the receiver list"""
        pass

    def clearToColor(self,color):
        """Clear the DynamicMap to the specified color
- 'color' is a ptColor object"""
        pass

    def drawImage(self,x,y,image,respectAlphaFlag):
        """Draws a ptImage object on the dynamicTextmap starting at the location x,y"""
        pass

    def drawImageClipped(self,x,y,image,cx,cy,cw,ch,respectAlphaFlag):
        """Draws a ptImage object clipped to cx,cy with cw(width),ch(height)"""
        pass

    def drawText(self,x,y,text):
        """Draw text at a specified location
- x,y is the point to start drawing the text
- 'text' is a string of the text to be drawn"""
        pass

    def drawTextW(self,x,y,text):
        """Unicode version of drawText"""
        pass

    def fillRect(self,left,top,right,bottom,color):
        """Fill in the specified rectangle with a color
- left,top,right,bottom define the rectangle
- 'color' is a ptColor object"""
        pass

    def flush(self):
        """Flush all the commands that were issued since the last flush()"""
        pass

    def frameRect(self,left,top,right,bottom,color):
        """Frame a rectangle with a specified color
- left,top,right,bottom define the rectangle
- 'color' is a ptColor object"""
        pass

    def getHeight(self):
        """Returns the height of the dynamicTextmap"""
        pass

    def getImage(self):
        """Returns a pyImage associated with the dynamicTextmap"""
        pass

    def getWidth(self):
        """Returns the width of the dynamicTextmap"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object
This only applies when NetPropagate is set to true"""
        pass

    def netPropagate(self,propagateFlag):
        """Specify whether this object needs to use messages that are sent on the network
- The default is for this to be false."""
        pass

    def purgeImage(self):
        """Purge the DynamicTextMap images"""
        pass

    def sender(self,sender):
        """Set the sender of the message being sent to the DynamicMap"""
        pass

    def setClipping(self,clipLeft,clipTop,clipRight,clipBottom):
        """Sets the clipping rectangle
- All drawtext will be clipped to this until the
unsetClipping() is called"""
        pass

    def setFont(self,facename,size):
        """Set the font of the text to be written
- 'facename' is a string with the name of the font
- 'size' is the point size of the font to use"""
        pass

    def setJustify(self,justify):
        """Sets the justification of the text. (justify is a PtJustify)"""
        pass

    def setLineSpacing(self,spacing):
        """Sets the line spacing (in pixels)"""
        pass

    def setTextColor(self,color, blockRGB=0):
        """Set the color of the text to be written
- 'color' is a ptColor object
- 'blockRGB' must be true if you're trying to render onto a transparent or semi-transparent color"""
        pass

    def setWrapping(self,wrapWidth,wrapHeight):
        """Set where text will be wrapped horizontally and vertically
- All drawtext commands will be wrapped until the
unsetWrapping() is called"""
        pass

    def unsetClipping(self):
        """Stop the clipping of text"""
        pass

    def unsetWrapping(self):
        """Stop text wrapping"""
        pass

class ptGUIControl:
    """Base class for all GUI controls"""
    def __init__(self,controlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlButton(ptGUIControl):
    """Plasma GUI Control Button class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getNotifyType(self):
        """Returns this button's notify type. See PtButtonNotifyTypes"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isButtonDown(self):
        """Is the button down? Returns 1 for true otherwise returns 0"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setNotifyType(self,kind):
        """Sets this button's notify type. See PtButtonNotifyTypes"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlCheckBox(ptGUIControl):
    """Plasma GUI Control Checkbox class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isChecked(self):
        """Is this checkbox checked? Returns 1 for true otherwise returns 0"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setChecked(self,checkedState):
        """Sets this checkbox to the 'checkedState'"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlClickMap(ptGUIControl):
    """Plasma GUI control Click Map"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getLastMouseDragPoint(self):
        """Returns the last point the mouse was dragged to"""
        pass

    def getLastMousePoint(self):
        """Returns the last point the mouse was at"""
        pass

    def getLastMouseUpPoint(self):
        """Returns the last point the mouse was released at"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlDragBar(ptGUIControl):
    """Plasma GUI Control DragBar class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def anchor(self):
        """Don't allow this dragbar object to be moved by the user.
Drop anchor!"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isAnchored(self):
        """Is this dragbar control anchored? Returns 1 if true otherwise returns 0"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

    def unanchor(self):
        """Allow the user to drag this control around the screen.
Raise anchor."""
        pass

class ptGUIControlDraggable(ptGUIControl):
    """Plasma GUI control for something draggable"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getLastMousePoint(self):
        """Returns the last point we were dragged to"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def stopDragging(self,cancelFlag):
        """UNKNOWN"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlDynamicText(ptGUIControl):
    """Plasma GUI Control DynamicText class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getMap(self,index):
        """Returns a specific ptDynamicText attached to this contol
If there is no map at 'index' then a KeyError exception will be raised"""
        pass

    def getNumMaps(self):
        """Returns the number of ptDynamicText maps attached"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlEditBox(ptGUIControl):
    """Plasma GUI Control Editbox class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def clearString(self):
        """Clears the editbox."""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def end(self):
        """Sets the cursor in the editbox to the after the last character."""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getLastKeyCaptured(self):
        """Gets the last capture key"""
        pass

    def getLastModifiersCaptured(self):
        """Gets the last modifiers flags captured"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getString(self):
        """Returns the sting that the user typed in."""
        pass

    def getStringW(self):
        """Unicode version of getString."""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def home(self):
        """Sets the cursor in the editbox to before the first character."""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setChatMode(self,state):
        """Set the Chat mode on this control"""
        pass

    def setColor(self,foreColor,backColor):
        """Sets the fore and back color of the editbox."""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setLastKeyCapture(self,key, modifiers):
        """Set last key captured"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setSelectionColor(self,foreColor,backColor):
        """Sets the selection color of the editbox."""
        pass

    def setSpecialCaptureKeyMode(self,state):
        """Set the Capture mode on this control"""
        pass

    def setString(self,text):
        """Pre-sets the editbox to a atring."""
        pass

    def setStringSize(self,size):
        """Sets the maximum size of the string that can be inputted by the user."""
        pass

    def setStringW(self,text):
        """Unicode version of setString."""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

    def wasEscaped(self):
        """If the editbox was escaped then return 1 else return 0"""
        pass

class ptGUIControlValue(ptGUIControl):
    """Plasma GUI Control Value class  - knobs, spinners"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getMax(self):
        """Returns the maximum of the control."""
        pass

    def getMin(self):
        """Returns the minimum of the control."""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getStep(self):
        """Returns the step increment of the control."""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def getValue(self):
        """Returns the current value of the control."""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setStep(self,step):
        """Sets the step increment of the control."""
        pass

    def setValue(self,value):
        """Sets the current value of the control."""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlKnob(ptGUIControlValue):
    """Plasma GUI control for knob"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getMax(self):
        """Returns the maximum of the control."""
        pass

    def getMin(self):
        """Returns the minimum of the control."""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getStep(self):
        """Returns the step increment of the control."""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def getValue(self):
        """Returns the current value of the control."""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setStep(self,step):
        """Sets the step increment of the control."""
        pass

    def setValue(self,value):
        """Sets the current value of the control."""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlListBox(ptGUIControl):
    """Plasma GUI Control List Box class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def add2StringsWithColors(self,text1,color1,text2,color2,respectAlpha):
        """Doesn't work right - DONT USE"""
        pass

    def addBranch(self,name,initiallyOpen):
        """UNKNOWN"""
        pass

    def addBranchW(self,name,initiallyOpen):
        """Unicode version of addBranch"""
        pass

    def addImage(self,image,respectAlphaFlag):
        """Appends an image item to the listbox"""
        pass

    def addImageAndSwatchesInBox(self,image,x,y,width,height,respectAlpha,primary,secondary):
        """Add the image and color swatches to the list"""
        pass

    def addImageInBox(self,image,x,y,width,height,respectAlpha):
        """Appends an image item to the listbox, centering within the box dimension."""
        pass

    def addSelection(self,item):
        """Adds item to selection list"""
        pass

    def addString(self,text):
        """Appends a list item 'text' to the listbox."""
        pass

    def addStringInBox(self,text,min_width,min_height):
        """Adds a text list item that has a minimum width and height"""
        pass

    def addStringW(self,text):
        """Unicode version of addString."""
        pass

    def addStringWithColor(self,text,color,inheritAlpha):
        """Adds a colored string to the list box"""
        pass

    def addStringWithColorWithSize(self,text,color,inheritAlpha,fontsize):
        """Adds a text list item with a color and different font size"""
        pass

    def allowNoSelect(self):
        """Allows the listbox to have no selection"""
        pass

    def clearAllElements(self):
        """Removes all the items from the listbox, making it empty."""
        pass

    def clickable(self):
        """Sets this listbox to be clickable by the user."""
        pass

    def closeBranch(self):
        """UNKNOWN"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def disallowNoSelect(self):
        """The listbox must always have a selection"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def findString(self,text):
        """Finds and returns the index of the item that matches 'text' in the listbox."""
        pass

    def findStringW(self,text):
        """Unicode version of findString."""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getBranchList(self):
        """get a list of branches in this list (index,isShowingChildren)"""
        pass

    def getElement(self,index):
        """Get the string of the item at 'index' in the listbox."""
        pass

    def getElementW(self,index):
        """Unicode version of getElement."""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getNumElements(self):
        """Return the number of items in the listbox."""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getScrollPos(self):
        """Returns the current scroll position in the listbox."""
        pass

    def getScrollRange(self):
        """Returns the max scroll position"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getSelection(self):
        """Returns the currently selected list item in the listbox."""
        pass

    def getSelectionList(self):
        """Returns the current selection list"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def lock(self):
        """Locks the updates to a listbox, so a number of operations can be performed
NOTE: an unlock() call must be made before the next lock() can be."""
        pass

    def refresh(self):
        """Refresh the display of the listbox (after updating contents)."""
        pass

    def removeElement(self,index):
        """Removes element at 'index' in the listbox."""
        pass

    def removeSelection(self,item):
        """Removes item from selection list"""
        pass

    def scrollToBegin(self):
        """Scrolls the listbox to the beginning of the list"""
        pass

    def scrollToEnd(self):
        """Scrolls the listbox to the end of the list"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setElement(self,index,text):
        """Set a particular item in the listbox to a string."""
        pass

    def setElementW(self,index,text):
        """Unicode version of setElement."""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setGlobalSwatchEdgeOffset(self,offset):
        """Sets the edge offset of the color swatches"""
        pass

    def setGlobalSwatchSize(self,size):
        """Sets the size of the color swatches"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setScrollPos(self,pos):
        """Sets the scroll position of the listbox to 'pos'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setSelection(self,selectionIndex):
        """Sets the current selection in the listbox."""
        pass

    def setStringJustify(self,index,justify):
        """Sets the text justification"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

    def unclickable(self):
        """Makes this listbox not clickable by the user.
Useful when just displaying a list that is not really selectable."""
        pass

    def unlock(self):
        """Unlocks updates to a listbox and does any saved up changes"""
        pass

class ptGUIControlMultiLineEdit(ptGUIControl):
    """Plasma GUI Control Multi-line edit class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def clearBuffer(self):
        """Clears all text from the multi-line edit control."""
        pass

    def clickable(self):
        """Sets this listbox to be clickable by the user."""
        pass

    def deleteChar(self):
        """Deletes a character at the current cursor position."""
        pass

    def deleteLinesFromTop(self,numLines):
        """Deletes the specified number of lines from the top of the text buffer"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def disableScrollControl(self):
        """Disables the scroll control if there is one"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def enableScrollControl(self):
        """Enables the scroll control if there is one"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getBufferLimit(self):
        """Returns the current buffer limit"""
        pass

    def getBufferSize(self):
        """Returns the size of the buffer"""
        pass

    def getEncodedBuffer(self):
        """Returns the encoded buffer in a python buffer object. Do NOT use result with setEncodedBufferW."""
        pass

    def getEncodedBufferW(self):
        """Unicode version of getEncodedBuffer. Do NOT use result with setEncodedBuffer."""
        pass

    def getFontSize(self):
        """Returns the current default font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getString(self):
        """Gets the string of the edit control."""
        pass

    def getStringW(self):
        """Unicode version of getString."""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def insertChar(self,c):
        """Inserts a character at the current cursor position."""
        pass

    def insertCharW(self,c):
        """Unicode version of insertChar."""
        pass

    def insertColor(self,color):
        """Inserts an encoded color object at the current cursor position.
'color' is a ptColor object."""
        pass

    def insertString(self,string):
        """Inserts a string at the current cursor position."""
        pass

    def insertStringW(self,string):
        """Unicode version of insertString"""
        pass

    def insertStyle(self,style):
        """Inserts an encoded font style at the current cursor position."""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isLocked(self):
        """Is the multi-line edit control locked? Returns 1 if true otherwise returns 0"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def lock(self):
        """Locks the multi-line edit control so the user cannot make changes."""
        pass

    def moveCursor(self,direction):
        """Move the cursor in the specified direction (see PtGUIMultiLineDirection)"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setBufferLimit(self,bufferLimit):
        """Sets the buffer max for the editbox"""
        pass

    def setEncodedBuffer(self,bufferObject):
        """Sets the edit control to the encoded buffer in the python buffer object. Do NOT use with a result from getEncodedBufferW."""
        pass

    def setEncodedBufferW(self,bufferObject):
        """Unicode version of setEncodedBuffer. Do NOT use with a result from getEncodedBuffer."""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the default font size for the edit control"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setScrollPosition(self,topLine):
        """Sets the what line is the top line."""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setString(self,asciiText):
        """Sets the multi-line edit control string."""
        pass

    def setStringW(self,unicodeText):
        """Unicode version of setString."""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

    def unclickable(self):
        """Makes this listbox not clickable by the user.
Useful when just displaying a list that is not really selectable."""
        pass

    def unlock(self):
        """Unlocks the multi-line edit control so that the user can make changes."""
        pass

class ptGUIControlProgress(ptGUIControlValue):
    """Plasma GUI control for progress bar"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def animateToPercent(self,percent):
        """Sets the value of the control and animates to that point."""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getMax(self):
        """Returns the maximum of the control."""
        pass

    def getMin(self):
        """Returns the minimum of the control."""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getStep(self):
        """Returns the step increment of the control."""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def getValue(self):
        """Returns the current value of the control."""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setStep(self,step):
        """Sets the step increment of the control."""
        pass

    def setValue(self,value):
        """Sets the current value of the control."""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlRadioGroup(ptGUIControl):
    """Plasma GUI Control Radio Group class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def getValue(self):
        """Returns the current selection of the radio group."""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setValue(self,value):
        """Sets the current selection to 'value'"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlTextBox(ptGUIControl):
    """Plasma GUI Control Textbox class"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the current forecolor"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getString(self):
        """Returns the string that the TextBox is set to (in case you forgot)"""
        pass

    def getStringJustify(self):
        """Returns current justify"""
        pass

    def getStringW(self):
        """Unicode version of getString"""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,color):
        """Sets the text backcolor to 'color', which is a ptColor object."""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,size):
        """Don't use"""
        pass

    def setForeColor(self,color):
        """Sets the text forecolor to 'color', which is a ptColor object."""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setString(self,text):
        """Sets the textbox string to 'text'"""
        pass

    def setStringJustify(self,justify):
        """Sets current justify"""
        pass

    def setStringW(self,text):
        """Unicode version of setString"""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIControlUpDownPair(ptGUIControlValue):
    """Plasma GUI control for up/down pair"""
    def __init__(self,ctrlKey):
        """None"""
        pass

    def disable(self):
        """Disables this GUI control"""
        pass

    def enable(self,flag=1):
        """Enables this GUI control"""
        pass

    def focus(self):
        """Gets focus for this GUI control"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        pass

    def getMax(self):
        """Returns the maximum of the control."""
        pass

    def getMin(self):
        """Returns the minimum of the control."""
        pass

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        pass

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getStep(self):
        """Returns the step increment of the control."""
        pass

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        pass

    def getValue(self):
        """Returns the current value of the control."""
        pass

    def hide(self):
        """Hides this GUI control"""
        pass

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        pass

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        pass

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        pass

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        pass

    def refresh(self):
        """UNKNOWN"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        pass

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        pass

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def setStep(self,step):
        """Sets the step increment of the control."""
        pass

    def setValue(self,value):
        """Sets the current value of the control."""
        pass

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        pass

    def show(self):
        """Shows this GUI control"""
        pass

    def unFocus(self):
        """Releases focus for this GUI control"""
        pass

class ptGUIDialog:
    """Plasma GUI dialog class"""
    def __init__(self,dialogKey):
        """None"""
        pass

    def disable(self):
        """Disables this dialog"""
        pass

    def enable(self,enableFlag=1):
        """Enable this dialog"""
        pass

    def getBackColor(self):
        """Returns the back color as a ptColor object"""
        pass

    def getBackSelectColor(self):
        """Returns the select back color as a ptColor object"""
        pass

    def getControlFromIndex(self,index):
        """Returns the ptKey of the control with the specified index (not tag ID!)"""
        pass

    def getControlFromTag(self,tagID):
        """Returns the ptKey of the control with the specified tag ID"""
        pass

    def getFontSize(self):
        """Returns the font size"""
        pass

    def getForeColor(self):
        """Returns the fore color as a ptColor object"""
        pass

    def getKey(self):
        """Returns this dialog's ptKey"""
        pass

    def getName(self):
        """Returns the dialog's name"""
        pass

    def getNumControls(self):
        """Returns the number of controls in this dialog"""
        pass

    def getSelectColor(self):
        """Returns the select color as a ptColor object"""
        pass

    def getTagID(self):
        """Returns this dialog's tag ID"""
        pass

    def getVersion(self):
        """UNKNOWN"""
        pass

    def hide(self):
        """Hides the dialog"""
        pass

    def isEnabled(self):
        """Is this dialog currently enabled?"""
        pass

    def noFocus(self):
        """Makes sure no control has input focus"""
        pass

    def refreshAllControls(self):
        """Tells the dialog to redraw all its controls"""
        pass

    def setBackColor(self,red,green,blue,alpha):
        """Sets the back color, -1 means don't change"""
        pass

    def setBackSelectColor(self,red,green,blue,alpha):
        """Sets the select back color, -1 means don't change"""
        pass

    def setFocus(self,ctrlKey):
        """Sets the control that has input focus"""
        pass

    def setFontSize(self,fontSize):
        """Sets the font size"""
        pass

    def setForeColor(self,red,green,blue,alpha):
        """Sets the fore color, -1 means don't change"""
        pass

    def setSelectColor(self,red,green,blue,alpha):
        """Sets the select color, -1 means don't change"""
        pass

    def show(self):
        """Shows the dialog"""
        pass

    def showNoReset(self):
        """Show dialog without resetting clickables"""
        pass

    def updateAllBounds(self):
        """Tells the dialog to recompute all the bounds for its controls"""
        pass

class ptGUIPopUpMenu:
    """Takes three diferent argument lists:
gckey
name,screenOriginX,screenOriginY
name,parent,screenOriginX,screenOriginY"""
    def __init__(self,arg1,arg2=None,arg3=None,arg4=None):
        """None"""
        pass

    def addConsoleCmdItem(self,name,consoleCmd):
        """Adds a new item to the menu that fires a console command"""
        pass

    def addConsoleCmdItemW(self,name,consoleCmd):
        """Unicode version of addConsoleCmdItem"""
        pass

    def addNotifyItem(self,name):
        """Adds a new item ot the mneu"""
        pass

    def addNotifyItemW(self,name):
        """Unicode version of addNotifyItem"""
        pass

    def addSubMenuItem(self,name,subMenu):
        """Adds a submenu to this menu"""
        pass

    def addSubMenuItemW(self,name,subMenu):
        """Unicode version of addSubMenuItem"""
        pass

    def disable(self):
        """Disables this menu"""
        pass

    def enable(self,state=1):
        """Enables/disables this menu"""
        pass

    def getBackColor(self):
        """Returns the background color"""
        pass

    def getBackSelectColor(self):
        """Returns the background selection color"""
        pass

    def getForeColor(self):
        """Returns the foreground color"""
        pass

    def getKey(self):
        """Returns this menu's key"""
        pass

    def getName(self):
        """Returns this menu's name"""
        pass

    def getSelectColor(self):
        """Returns the selection color"""
        pass

    def getTagID(self):
        """Returns this menu's tag id"""
        pass

    def getVersion(self):
        """UNKNOWN"""
        pass

    def hide(self):
        """Hides this menu"""
        pass

    def isEnabled(self):
        """Returns whether this menu is enabled or not"""
        pass

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        pass

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        pass

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        pass

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        pass

    def show(self):
        """Shows this menu"""
        pass

class ptGUISkin:
    """Plasma GUI Skin object"""
    def __init__(self,key):
        """None"""
        pass

    def getKey(self):
        """Returns this object's ptKey"""
        pass

class ptGameScore:
    """Game score manager"""
    def __init__(self):
        """None"""
        pass

    def addPoints(self,numPoints):
        """Adds points to the score"""
        pass

    def getCreatedTime(self):
        """Returns a the score creation time."""
        pass

    def getGameName(self):
        """Returns a the score game name."""
        pass

    def getGameType(self):
        """Returns a the score game type."""
        pass

    def getOwnerID(self):
        """Returns a the score owner id."""
        pass

    def getScoreID(self):
        """Returns the score id."""
        pass

    def getValue(self):
        """Returns a the score owner value."""
        pass

    def setPoints(self,numPoints):
        """Sets the number of points in the score
Don't use to add/remove points, use only to reset values!"""
        pass

    def transferPoints(self,dest, numPoints):
        """Transfers points from one score to another"""
        pass

class ptGrassShader:
    """Plasma Grass Shader class"""
    def __init__(self,key):
        """None"""
        pass

    def getWaveDirection(self,waveNum):
        """Gets the wave waveNum's direction as a tuple of x,y. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        pass

    def getWaveDistortion(self,waveNum):
        """Gets the wave waveNum's distortion as a tuple of x,y,z. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        pass

    def getWaveSpeed(self,waveNum):
        """Gets the wave waveNum's speed as a float. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        pass

    def resetWaves(self):
        """Resets wave data to 0"""
        pass

    def setWaveDirection(self,waveNum, direction):
        """Sets the wave waveNum's direction as a tuple of x,y. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        pass

    def setWaveDistortion(self,waveNum, distortion):
        """Sets the wave waveNum's distortion as a tuple of x,y,z. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        pass

    def setWaveSpeed(self,waveNum, speed):
        """Sets the wave waveNum's speed as a float. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        pass

class ptImage:
    """Plasma image class"""
    def __init__(self,imgKey):
        """None"""
        pass

    def getColorLoc(self,color):
        """Returns the ptPoint3 where the specified color is located"""
        pass

    def getHeight(self):
        """Returns the height of the image"""
        pass

    def getPixelColor(self,x,y):
        """Returns the ptColor at the specified location (float from 0 to 1)"""
        pass

    def getWidth(self):
        """Returns the width of the image"""
        pass

    def saveAsJPEG(self,filename,quality=75):
        """Saves this image to disk as a JPEG file"""
        pass

class ptInputInterface:
    """Plasma input interface class"""
    def __init__(self):
        """None"""
        pass

    def popTelescope(self):
        """pops off the telescope interface and gos back to previous interface"""
        pass

    def pushTelescope(self):
        """pushes on the telescope interface"""
        pass

class ptKey:
    """Plasma Key class"""
    def __init__(self):
        """None"""
        pass

    def disable(self):
        """Sends a disable message to whatever this ptKey is pointing to"""
        pass

    def enable(self):
        """Sends an enable message to whatever this ptKey is pointing to"""
        pass

    def getName(self):
        """Get the name of the object that this ptKey is pointing to"""
        pass

    def getParentKey(self):
        """This will return a ptKey object that is the parent of this modifer
However, if the parent is not a modifier or not loaded, then None is returned."""
        pass

    def getSceneObject(self):
        """This will return a ptSceneobject object that is associated with this ptKey
However, if this ptKey is _not_ a sceneobject, then unpredicatable results will ensue"""
        pass

    def isAttachedToClone(self):
        """Returns whether the python file mod is attached to a clone"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

class ptKeyMap:
    """Accessor class to the Key Mapping functions"""
    def __init__(self):
        """None"""
        pass

    def bindKey(self):
        """Params key1,key2,action
Bind keys to an action"""
        pass

    def bindKeyToConsoleCommand(self,keyStr1, command):
        """Binds key to console command"""
        pass

    def convertCharToControlCode(self,controlCodeString):
        """Convert string version of control code to number"""
        pass

    def convertCharToFlags(self,charString):
        """Convert char string to flags"""
        pass

    def convertCharToVKey(self,charString):
        """Convert char string to virtual key"""
        pass

    def convertControlCodeToString(self):
        """Params controlCode
Convert control code to character string"""
        pass

    def convertVKeyToChar(self,virtualKey,flags):
        """Convert virtual key and shift flags to string"""
        pass

    def getBindingFlags1(self):
        """Params controlCode
Returns modifier flags for controlCode"""
        pass

    def getBindingFlags2(self):
        """Params controlCode
Returns modifier flags for controlCode"""
        pass

    def getBindingFlagsConsole(self,command):
        """Returns modifier flags for the console command mapping"""
        pass

    def getBindingKey1(self):
        """Params controlCode
Returns key code for controlCode"""
        pass

    def getBindingKey2(self):
        """Params controlCode
Returns key code for controlCode"""
        pass

    def getBindingKeyConsole(self,command):
        """Returns key for console command mapping"""
        pass

    def writeKeyMap(self):
        """Forces write of the keymap file"""
        pass

class ptMarkerMgr:
    """Marker manager accessor class"""
    def __init__(self):
        """None"""
        pass

    def addMarker(self,x, y, z, id, justCreated):
        """Add a marker in the specified location with the specified id"""
        pass

    def areLocalMarkersShowing(self):
        """Returns true if we are showing the markers on this local machine"""
        pass

    def captureQuestMarker(self,id, captured):
        """Sets a marker as captured or not"""
        pass

    def captureTeamMarker(self,id, team):
        """Sets a marker as captured by the specified team (0 = not captured)"""
        pass

    def clearSelectedMarker(self):
        """Unselects the selected marker"""
        pass

    def getMarkersRespawn(self):
        """Returns whether markers respawn after being captured, or not"""
        pass

    def getSelectedMarker(self):
        """Returns the id of the selected marker"""
        pass

    def hideMarkersLocal(self):
        """Hides the markers on your machine, so you can no longer see where they are"""
        pass

    def removeAllMarkers(self):
        """Removes all markers"""
        pass

    def removeMarker(self,id):
        """Removes the specified marker from the game"""
        pass

    def setMarkersRespawn(self,respawn):
        """Sets whether markers respawn after being captured, or not"""
        pass

    def setSelectedMarker(self,id):
        """Sets the selected marker to the one with the specified id"""
        pass

    def showMarkersLocal(self):
        """Shows the markers on your machine, so you can see where they are"""
        pass

class ptMatrix44:
    """Plasma Matrix44 class"""
    def __init__(self):
        """None"""
        pass

    def copy(self):
        """Copies the matrix and returns the copy"""
        pass

    def getAdjoint(self,adjointMat):
        """Returns the adjoint of the matrix"""
        pass

    def getData(self):
        """Returns the matrix in tuple form"""
        pass

    def getDeterminant(self):
        """Get the matrix's determinant"""
        pass

    def getInverse(self,inverseMat):
        """Returns the inverse of the matrix"""
        pass

    def getParity(self):
        """Get the parity of the matrix"""
        pass

    def getTranslate(self,vector):
        """Returns the translate vector of the matrix (and sets vector to it as well)"""
        pass

    def getTranspose(self,transposeMat):
        """Returns the transpose of the matrix"""
        pass

    def make(self,fromPt, atPt, upVec):
        """Creates the matrix from from and at points, and the up vector"""
        pass

    def makeRotateMat(self,axis,radians):
        """Makes the matrix a rotation matrix"""
        pass

    def makeScaleMat(self,scale):
        """Makes the matrix a scaling matrix"""
        pass

    def makeTranslateMat(self,trans):
        """Makes the matrix a translation matrix"""
        pass

    def makeUpPreserving(self,fromPt, atPt, upVec):
        """Creates the matrix from from and at points, and the up vector (perserving the up vector)"""
        pass

    def reset(self):
        """Reset the matrix to identity"""
        pass

    def right(self):
        """Returns the right vector of the matrix"""
        pass

    def rotate(self,axis,radians):
        """Rotates the matrix by radians around the axis"""
        pass

    def scale(self,scale):
        """Scales the matrix by the vector"""
        pass

    def setData(self,mat):
        """Sets the matrix using tuples"""
        pass

    def translate(self,vector):
        """Translates the matrix by the vector"""
        pass

    def up(self):
        """Returns the up vector of the matrix"""
        pass

    def view(self):
        """Returns the view vector of the matrix"""
        pass

class ptMoviePlayer:
    """Accessor class to play in the MoviePlayer"""
    def __init__(self,movieName,selfKey):
        """None"""
        pass

    def pause(self):
        """Pauses the movie"""
        pass

    def play(self):
        """Plays the movie"""
        pass

    def playPaused(self):
        """Plays movie, but pauses at first frame"""
        pass

    def resume(self):
        """Resumes movie after pausing"""
        pass

    def setCenter(self,x,y):
        """Sets the center of the movie"""
        pass

    def setColor(self,color):
        """Sets the color of the movie"""
        pass

    def setOpacity(self,opacity):
        """Sets the opacity of the movie"""
        pass

    def setScale(self,width,height):
        """Sets the width and height scale of the movie"""
        pass

    def setVolume(self,volume):
        """Set the volume of the movie"""
        pass

    def stop(self):
        """Stops the movie"""
        pass

class ptNetLinkingMgr:
    """Constructor to get access to the net link manager"""
    def __init__(self):
        """None"""
        pass

    def getCurrAgeLink(self):
        """Get the ptAgeLinkStruct for the current age"""
        pass

    def getPrevAgeLink(self):
        """Get the ptAgeLinkStruct for the previous age"""
        pass

    def isEnabled(self):
        """True if linking is enabled."""
        pass

    def linkPlayerHere(self,pid):
        """link player(pid) to where I am"""
        pass

    def linkPlayerToAge(self,ageLink,pid):
        """Link player(pid) to ageLink"""
        pass

    def linkToAge(self,ageLink):
        """Links to ageLink (ptAgeLinkStruct)"""
        pass

    def linkToMyNeighborhoodAge(self):
        """Link to my Neighborhood Age"""
        pass

    def linkToMyPersonalAge(self):
        """Link to my Personal Age"""
        pass

    def linkToMyPersonalAgeWithYeeshaBook(self):
        """Link to my Personal Age with the YeeshaBook"""
        pass

    def linkToPlayersAge(self,pid):
        """Link me to where player(pid) is"""
        pass

    def setEnabled(self,enable):
        """Enable/Disable linking."""
        pass

class ptNotify:
    """Creates a Notify message
- selfKey is ptKey of your PythonFile modifier"""
    def __init__(self,selfKey):
        """None"""
        pass

    def addActivateEvent(self,activeFlag,activateFlag):
        """Add an activate event record to the notify message"""
        pass

    def addCallbackEvent(self,eventNumber):
        """Add a callback event record to the notify message"""
        pass

    def addCollisionEvent(self,enterFlag,hitterKey,hitteeKey):
        """Add a collision event record to the Notify message"""
        pass

    def addContainerEvent(self,enteringFlag,containerKey,containedKey):
        """Add a container event record to the notify message"""
        pass

    def addControlKeyEvent(self,keynumber,downFlag):
        """Add a keyboard event record to the Notify message"""
        pass

    def addFacingEvent(self,enabledFlag,facerKey, faceeKey, dotProduct):
        """Add a facing event record to the Notify message"""
        pass

    def addPickEvent(self,enabledFlag,pickerKey,pickeeKey,hitPoint):
        """Add a pick event record to the Notify message"""
        pass

    def addReceiver(self,key):
        """Add a receivers key to receive this Notify message"""
        pass

    def addResponderState(self,state):
        """Add a responder state event record to the notify message"""
        pass

    def addVarKey(self,name,key):
        """Add a ptKey variable event record to the Notify message
This event record is used to pass a ptKey variable to another python program"""
        pass

    def addVarNumber(self,name,number):
        """Add a number variable event record to the Notify message
This event record is used to pass a number variable to another python program"""
        pass

    def clearReceivers(self):
        """Remove all the receivers that this Notify message has
- receivers are automatically added if from a ptAttribActivator"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

    def netPropagate(self,netFlag):
        """Sets the net propagate flag - default to set"""
        pass

    def send(self):
        """Send the notify message"""
        pass

    def setActivate(self,state):
        """Set the activate state to true(1.0) or false(0.0)"""
        pass

    def setType(self,type):
        """Sets the message type"""
        pass

class ptParticle:
    """Plasma particle system class"""
    def __init__(self):
        """None"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

    def setGeneratorLife(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setHeightSize(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setInitPitchRange(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setInitYawRange(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setParticleLifeMaximum(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setParticleLifeMinimum(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setParticlesPerSecond(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setScaleMaximum(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setScaleMinimum(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setVelocityMaximum(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setVelocityMinimum(self,value):
        """NEEDS DOCSTRING"""
        pass

    def setWidthSize(self,value):
        """NEEDS DOCSTRING"""
        pass

class ptPhysics:
    """Plasma physics class"""
    def __init__(self):
        """None"""
        pass

    def angularImpulse(self,impulseVector):
        """Add the given vector (representing a rotation axis and magnitude) to
the attached sceneobject's velocity"""
        pass

    def damp(self,damp):
        """Reduce all velocities on the object (0 = all stop, 1 = no effect)"""
        pass

    def disable(self):
        """Disables physics on the sceneobject attached"""
        pass

    def disableCollision(self):
        """Disables collision detection on the attached sceneobject"""
        pass

    def enable(self,state=1):
        """Sets the physics enable state for the sceneobject attached"""
        pass

    def enableCollision(self):
        """Enables collision detection on the attached sceneobject"""
        pass

    def force(self,forceVector):
        """Applies the specified force to the attached sceneobject"""
        pass

    def forceWithOffset(self,forceVector,offsetPt):
        """Applies the specified offsetted force to the attached sceneobject"""
        pass

    def impulse(self,impulseVector):
        """Adds the given vector to the attached sceneobject's velocity"""
        pass

    def impulseWithOffset(self,impulseVector,offsetPt):
        """Adds the given vector to the attached sceneobject's velocity
with the specified offset"""
        pass

    def move(self,direction,distance):
        """Moves the attached sceneobject the specified distance in the specified direction"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        pass

    def rotate(self,radians,axis):
        """Rotates the attached sceneobject the specified radians around the specified axis"""
        pass

    def shiftMass(self,offsetVector):
        """Shifts the attached sceneobject's center to mass in the specified direction and distance"""
        pass

    def suppress(self,doSuppress):
        """Completely remove the physical, but keep it around so it
can be added back later."""
        pass

    def torque(self,torqueVector):
        """Applies the specified torque to the attached sceneobject"""
        pass

    def warp(self,position):
        """Warps the sceneobject to a specified location.
'position' can be a ptPoint3 or a ptMatrix44"""
        pass

    def warpObj(self,objkey):
        """Warps the sceneobject to match the location and orientation of the specified object"""
        pass

class ptPlayer:
    """And optionally __init__(name,playerID)"""
    def __init__(self,avkey,name,playerID,distanceSq):
        """None"""
        pass

    def getDistanceSq(self):
        """Returns the distance to remote player from local player"""
        pass

    def getPlayerID(self):
        """Returns the unique player ID"""
        pass

    def getPlayerName(self):
        """Returns the name of the player"""
        pass

    def isCCR(self):
        """Is this player a CCR?"""
        pass

    def isServer(self):
        """Is this player a server?"""
        pass

class ptPoint3:
    """Plasma Point class"""
    def __init__(self,x=0, y=0, z=0):
        """None"""
        pass

    def copy(self):
        """Returns a copy of the point in another ptPoint3 object"""
        pass

    def distance(self,other):
        """Computes the distance from this point to 'other' point"""
        pass

    def distanceSq(self,other):
        """Computes the distance squared from this point to 'other' point
- this function is faster than distance(other)"""
        pass

    def getX(self):
        """Returns the 'x' component of the point"""
        pass

    def getY(self):
        """Returns the 'y' component of the point"""
        pass

    def getZ(self):
        """Returns the 'z' component of the point"""
        pass

    def setX(self,x):
        """Sets the 'x' component of the point"""
        pass

    def setY(self,y):
        """Sets the 'y' component of the point"""
        pass

    def setZ(self,z):
        """Sets the 'z' component of the point"""
        pass

    def zero(self):
        """Sets the 'x','y' and the 'z' component to zero"""
        pass

class ptSDL:
    """SDL accessor"""
    def __init__(self):
        """None"""
        pass

    def sendToClients(self,key):
        """Sets it so changes to this key are sent to the
server AND the clients. (Normally it just goes
to the server.)"""
        pass

    def setDefault(self,key,value):
        """Like setitem, but doesn't broadcast over the net.
Only use for setting defaults that everyone will
already know (from reading it off disk)"""
        pass

    def setFlags(self,name,sendImmediate,skipOwnershipCheck):
        """Sets the flags for a variable in this SDL"""
        pass

    def setIndex(self,key,idx,value):
        """Sets the value at a specific index in the tuple,
so you don't have to pass the whole thing in"""
        pass

    def setIndexNow(self,key,idx,value):
        """Same as setIndex but sends immediately"""
        pass

    def setNotify(self,selfkey,key,tolerance):
        """Sets the OnSDLNotify to be called when 'key'
SDL variable changes by 'tolerance' (if number)"""
        pass

    def setTagString(self,name,tag):
        """Sets the tag string for a variable"""
        pass

class ptSDLStateDataRecord:
    """Basic SDL state data record class"""
    def __init__(self):
        """None"""
        pass

    def findVar(self,name):
        """Finds and returns the specified ptSimpleStateVariable"""
        pass

    def getName(self):
        """Returns our record's name"""
        pass

    def getVarList(self):
        """Returns the names of the vars we hold as a list of strings"""
        pass

    def setFromDefaults(self,timeStampNow):
        """Sets all our vars to their defaults"""
        pass

class ptSceneobject:
    """Plasma Sceneobject class"""
    def __init__(self,objKey, selfKey):
        """None"""
        pass

    def addKey(self,key):
        """Mostly used internally.
Add another sceneobject ptKey"""
        pass

    def animate(self):
        """If we can animate, start animating"""
        pass

    def avatarVelocity(self):
        """Returns the velocity of the first attached avatar scene object"""
        pass

    def fastForwardAttachedResponder(self,state):
        """Fast forward the attached responder to the specified state"""
        pass

    def findObject(self,name):
        """Find a particular object in just the sceneobjects that are attached"""
        pass

    def getKey(self):
        """Get the ptKey of this sceneobject
If there are more then one attached, get the first one"""
        pass

    def getLocalToParent(self):
        """Returns ptMatrix44 of the local to parent transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        pass

    def getLocalToWorld(self):
        """Returns ptMatrix44 of the local to world transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        pass

    def getName(self):
        """Returns the name of the sceneobject (Max name)
- If there are more than one sceneobject attached, return just the first one"""
        pass

    def getParentToLocal(self):
        """Returns ptMatrix44 of the parent to local transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        pass

    def getPythonMods(self):
        """Returns list of ptKeys of the python modifiers attached to this sceneobject"""
        pass

    def getResponderState(self):
        """Return the responder state (if we are a responder)"""
        pass

    def getResponders(self):
        """Returns list of ptKeys of the responders attached to this sceneobject"""
        pass

    def getSoundIndex(self,sndComponentName):
        """Get the index of the requested sound component"""
        pass

    def getWorldToLocal(self):
        """Returns ptMatrix44 of the world to local transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        pass

    def isAvatar(self):
        """Returns true if the scene object is an avatar"""
        pass

    def isHuman(self):
        """Returns true if the scene object is a human avatar"""
        pass

    def isLocallyOwned(self):
        """Returns true(1) if this object is locally owned by this client
or returns false(0) if it is not or don't know"""
        pass

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object
- Setting the netForce flag on a sceneobject will also set the netForce flag on
its draw, physics, avatar, particle objects"""
        pass

    def playAnimNamed(self,animName):
        """Play the attached named animation"""
        pass

    def popCamera(self,avKey):
        """Pop the camera stack and go back to the previous camera"""
        pass

    def popCutsceneCamera(self,avKey):
        """Pop the camera stack and go back to previous camera."""
        pass

    def position(self):
        """Returns the scene object's current position"""
        pass

    def pushCamera(self,avKey):
        """Switch to this object (if it is a camera)"""
        pass

    def pushCameraCut(self,avKey):
        """Switch to this object, cutting the view (if it is a camera)"""
        pass

    def pushCutsceneCamera(self,cutFlag,avKey):
        """Switch to this object (assuming that it is actually a camera)"""
        pass

    def rewindAnimNamed(self,animName):
        """Rewind the attached named animation"""
        pass

    def right(self):
        """Returns the scene object's current right vector"""
        pass

    def runAttachedResponder(self,state):
        """Run the attached responder to the specified state"""
        pass

    def setSoundFilename(self,index, filename, isCompressed):
        """Sets the sound attached to this sceneobject to use the specified sound file."""
        pass

    def setTransform(self,local2world,world2local):
        """Set our current transforms"""
        pass

    def stopAnimNamed(self,animName):
        """Stop the attached named animation"""
        pass

    def up(self):
        """Returns the scene object's current up vector"""
        pass

    def view(self):
        """Returns the scene object's current view vector"""
        pass

    def volumeSensorIgnoreExtraEnters(self,ignore):
        """Tells the volume sensor attached to this object to ignore extra enters (default), or not (hack for garrison)."""
        pass

class ptScoreMgr:
    """Game score manager"""
    def __init__(self):
        """None"""
        pass

    def createGlobalScore(self,gameName, gameType, value):
        """Creates a score and returns it"""
        pass

    def createNeighborhoodScore(self,gameName, gameType, value):
        """Creates a score and returns it"""
        pass

    def createPlayerScore(self,gameName, gameType, value):
        """Creates a score and returns it"""
        pass

    def deleteScore(self,scoreId):
        """Deletes the specified score"""
        pass

    def getGlobalScores(self,gameName):
        """Returns a list of scores associated with the specified game."""
        pass

    def getNeighborhoodScores(self,gameName):
        """Returns a list of scores associated with the specified game."""
        pass

    def getPlayerScores(self,gameName):
        """Returns a list of scores associated with the specified game."""
        pass

    def getRankList(self,ownerInfoId, scoreGroup, parentFolderId, gameName, timePeriod, numResults, pageNumber, sortDesc):
        """Returns a list of scores and rank"""
        pass

class ptSimpleStateVariable:
    """Basic SDL state data record class"""
    def __init__(self):
        """None"""
        pass

    def getBool(self,idx=0):
        """Returns a boolean variable's value"""
        pass

    def getByte(self,idx=0):
        """Returns a byte variable's value"""
        pass

    def getDefault(self):
        """Returns the variable's default"""
        pass

    def getDisplayOptions(self):
        """Returns the variable's display options"""
        pass

    def getDouble(self,idx=0):
        """Returns a double variable's value"""
        pass

    def getFloat(self,idx=0):
        """Returns a float variable's value"""
        pass

    def getInt(self,idx=0):
        """Returns an int variable's value"""
        pass

    def getShort(self,idx=0):
        """Returns a short variable's value"""
        pass

    def getString(self,idx=0):
        """Returns a string variable's value"""
        pass

    def getType(self):
        """Returns the variable's type"""
        pass

    def isAlwaysNew(self):
        """Is this variable always new?"""
        pass

    def isInternal(self):
        """Is this an internal variable?"""
        pass

    def setBool(self,val,idx=0):
        """Sets a boolean variable's value"""
        pass

    def setByte(self,val,idx=0):
        """Sets a byte variable's value"""
        pass

    def setDouble(self,val,idx=0):
        """Sets a double variable's value"""
        pass

    def setFloat(self,val,idx=0):
        """Sets a float variable's value"""
        pass

    def setInt(self,val,idx=0):
        """Sets an int variable's value"""
        pass

    def setShort(self,val,idx=0):
        """Sets a short variable's value"""
        pass

    def setString(self,val,idx=0):
        """Sets a string variable's value"""
        pass

class ptSpawnPointInfo:
    """Class to hold spawn point data"""
    def __init__(self,title=None,spawnPt=None):
        """None"""
        pass

    def getCameraStack(self):
        """Returns the camera stack for this spawnpoint as a string"""
        pass

    def getName(self):
        """Returns the spawnpoint's name"""
        pass

    def getTitle(self):
        """Returns the spawnpoint's title"""
        pass

    def setCameraStack(self,stack):
        """Sets the spawnpoint's camera stack (as a string)"""
        pass

    def setName(self,name):
        """Sets the spawnpoint's name"""
        pass

    def setTitle(self,title):
        """Sets the spawnpoint's title"""
        pass

class ptSpawnPointInfoRef:
    """Class to hold spawn point data"""
    def __init__(self):
        """None"""
        pass

    def getCameraStack(self):
        """Returns the camera stack for this spawnpoint as a string"""
        pass

    def getName(self):
        """Returns the spawnpoint's name"""
        pass

    def getTitle(self):
        """Returns the spawnpoint's title"""
        pass

    def setCameraStack(self,stack):
        """Sets the spawnpoint's camera stack (as a string)"""
        pass

    def setName(self,name):
        """Sets the spawnpoint's name"""
        pass

    def setTitle(self,title):
        """Sets the spawnpoint's title"""
        pass

class ptStatusLog:
    """A status log class"""
    def __init__(self):
        """None"""
        pass

    def close(self):
        """Close the status log file"""
        pass

    def isOpen(self):
        """Returns whether the status log is currently opened"""
        pass

    def open(self,logName,numLines,flags):
        """Open a status log for writing to
'logname' is the name of the log file (example: special.log)
'numLines' is the number of lines to display on debug screen
'flags' is a PlasmaConstants.PtStatusLogFlags"""
        pass

    def write(self,text,color=None):
        """If the status log is open, write 'text' to log
'color' is the display color in debug screen"""
        pass

class ptStream:
    """A basic stream class"""
    def __init__(self):
        """None"""
        pass

    def close(self):
        """Close the status log file"""
        pass

    def isOpen(self):
        """Returns whether the stream file is currently opened"""
        pass

    def open(self,fileName,flags):
        """Open a stream file for reading or writing"""
        pass

    def readlines(self):
        """Reads a list of strings from the file"""
        pass

    def writelines(self,lines):
        """Write a list of strings to the file"""
        pass

class ptSwimCurrentInterface:
    """Creates a new ptSwimCurrentInterface"""
    def __init__(self,key):
        """None"""
        pass

    def disable(self):
        """UNKNOWN"""
        pass

    def enable(self):
        """UNKNOWN"""
        pass

class ptVault:
    """Accessor class to the player's vault"""
    def __init__(self):
        """None"""
        pass

    def addChronicleEntry(self,entryName,type,string):
        """Adds an entry to the player's chronicle with a value of 'string'."""
        pass

    def amAgeCzar(self,ageInfo):
        """Are we the czar (WTH is this?) of the specified age?"""
        pass

    def amAgeOwner(self,ageInfo):
        """Are we the owner of the specified age?"""
        pass

    def amCzarOfCurrentAge(self):
        """Are we the czar (WTH is this?) of the current age?"""
        pass

    def amOwnerOfCurrentAge(self):
        """Are we the owner of the current age?"""
        pass

    def createNeighborhood(self):
        """Creates a new neighborhood"""
        pass

    def findChronicleEntry(self,entryName):
        """Returns a ptVaultNode of type kNodeTypeChronicle of the current player's chronicle entry by entryName."""
        pass

    def findNode(self,templateNode):
        """Find the node matching the template"""
        pass

    def getAgeJournalsFolder(self):
        """Returns a ptVaultFolderNode of the current player's age journals folder."""
        pass

    def getAgesICanVisitFolder(self):
        """Returns a ptVaultFolderNode of ages I can visit"""
        pass

    def getAgesIOwnFolder(self):
        """Returns a ptVaultFolderNode of ages that I own"""
        pass

    def getAvatarClosetFolder(self):
        """Do not use.
Returns a ptVaultFolderNode of the avatars outfit in their closet."""
        pass

    def getAvatarOutfitFolder(self):
        """Do not use.
Returns a ptVaultFolderNode of the avatars outfit."""
        pass

    def getBuddyListFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the current player's buddy list folder."""
        pass

    def getChronicleFolder(self):
        """Returns a ptVaultFolderNode of the current player's chronicle folder."""
        pass

    def getGlobalInbox(self):
        """Returns a ptVaultFolderNode of the global inbox folder."""
        pass

    def getIgnoreListFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the current player's ignore list folder."""
        pass

    def getInbox(self):
        """Returns a ptVaultFolderNode of the current player's inbox folder."""
        pass

    def getInviteFolder(self):
        """Returns a ptVaultFolderNode of invites"""
        pass

    def getKIUsage(self):
        """Returns a tuple with usage statistics of the KI (# of pics, # of text notes, # of marker games)"""
        pass

    def getLinkToCity(self):
        """Returns a ptVaultAgeLinkNode that will go to the city"""
        pass

    def getLinkToMyNeighborhood(self):
        """Returns a ptVaultAgeLinkNode that will go to my neighborhood"""
        pass

    def getOwnedAgeLink(self,ageInfo):
        """Returns a ptVaultAgeLinkNode to my owned age(ageInfo)"""
        pass

    def getPeopleIKnowAboutFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the current player's people I know about (Recent) list folder."""
        pass

    def getPlayerInfo(self):
        """Returns a ptVaultNode of type kNodeTypePlayerInfo of the current player"""
        pass

    def getPsnlAgeSDL(self):
        """Returns the personal age SDL"""
        pass

    def getVisitAgeLink(self,ageInfo):
        """Returns a ptVaultAgeLinkNode for a visitor to age(ageInfo)"""
        pass

    def inMyNeighborhoodAge(self):
        """Are we in the player's neighborhood age?"""
        pass

    def inMyPersonalAge(self):
        """Are we in the player's personal age?"""
        pass

    def invitePlayerToAge(self,link,playerID):
        """Sends an invitation to visit the age to the specified player"""
        pass

    def offerLinkToPlayer(self,link,playerID):
        """Offer a one-time link to the specified player"""
        pass

    def registerMTStation(self,stationName,mtSpawnPoint):
        """Registers this player at the specified mass-transit point"""
        pass

    def registerOwnedAge(self,link):
        """Registers the specified age as owned by the player"""
        pass

    def registerVisitAge(self,link):
        """Register this age as visitable by this player"""
        pass

    def sendToDevice(self,node,deviceName):
        """Sends a ptVaultNode object to an Age's device by deviceName."""
        pass

    def setAgePublic(self,ageInfo,makePublic):
        """Makes the specified age public or private"""
        pass

    def unInvitePlayerToAge(self,guid,playerID):
        """Revokes the invitation to visit the age"""
        pass

    def unRegisterOwnedAge(self,ageFilename):
        """Unregisters the specified age so it's no longer owned by this player"""
        pass

    def unRegisterVisitAge(self,guid):
        """Unregisters the specified age so it can no longer be visited by this player"""
        pass

    def updatePsnlAgeSDL(self,pyrec):
        """Updates the personal age SDL to the specified data"""
        pass

class ptVaultNode:
    """Vault node class"""
    def __init__(self):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultFolderNode(ptVaultNode):
    """Plasma vault folder node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def folderGetName(self):
        """LEGACY
Returns the folder's name"""
        pass

    def folderGetType(self):
        """LEGACY
Returns the folder type (of the standard folder types)"""
        pass

    def folderSetName(self,name):
        """LEGACY
Set the folder name"""
        pass

    def folderSetType(self,type):
        """LEGACY
Set the folder type"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getFolderName(self):
        """Returns the folder's name"""
        pass

    def getFolderNameW(self):
        """Unicode version of getFolerName"""
        pass

    def getFolderType(self):
        """Returns the folder type (of the standard folder types)"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setFolderName(self,name):
        """Set the folder name"""
        pass

    def setFolderNameW(self,name):
        """Unicode version of setFolderName"""
        pass

    def setFolderType(self,type):
        """Set the folder type"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultAgeInfoListNode(ptVaultFolderNode):
    """Plasma vault age info list node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addAge(self,ageID):
        """Adds ageID to list of ages"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def folderGetName(self):
        """LEGACY
Returns the folder's name"""
        pass

    def folderGetType(self):
        """LEGACY
Returns the folder type (of the standard folder types)"""
        pass

    def folderSetName(self,name):
        """LEGACY
Set the folder name"""
        pass

    def folderSetType(self,type):
        """LEGACY
Set the folder type"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getFolderName(self):
        """Returns the folder's name"""
        pass

    def getFolderNameW(self):
        """Unicode version of getFolerName"""
        pass

    def getFolderType(self):
        """Returns the folder type (of the standard folder types)"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasAge(self,ageID):
        """Returns whether ageID is in the list of ages"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAge(self,ageID):
        """Removes ageID from list of ages"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setFolderName(self,name):
        """Set the folder name"""
        pass

    def setFolderNameW(self,name):
        """Unicode version of setFolderName"""
        pass

    def setFolderType(self,type):
        """Set the folder type"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultAgeInfoNode(ptVaultNode):
    """Plasma vault age info node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def asAgeInfoStruct(self):
        """Returns this ptVaultAgeInfoNode as a ptAgeInfoStruct"""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getAgeDescription(self):
        """Returns the description of the age"""
        pass

    def getAgeFilename(self):
        """Returns the age filename"""
        pass

    def getAgeID(self):
        """Returns the age ID"""
        pass

    def getAgeInstanceGuid(self):
        """Returns the age instance guid"""
        pass

    def getAgeInstanceName(self):
        """Returns the instance name of the age"""
        pass

    def getAgeLanguage(self):
        """Returns the age's language (integer)"""
        pass

    def getAgeOwnersFolder(self):
        """Returns a ptVaultPlayerInfoList of the players that own this age"""
        pass

    def getAgeSDL(self):
        """Returns a ptVaultSDLNode of the age's SDL"""
        pass

    def getAgeSequenceNumber(self):
        """Returns the sequence number of this instance of the age"""
        pass

    def getAgeUserDefinedName(self):
        """Returns the user define part of the age name"""
        pass

    def getCanVisitFolder(self):
        """Returns a ptVaultPlayerInfoList of the players that can visit this age"""
        pass

    def getChildAgesFolder(self):
        """Returns a ptVaultFolderNode of the child ages of this age"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getCzar(self):
        """Returns ptVaultPlayerInfoNode of the player that is the Czar"""
        pass

    def getCzarID(self):
        """Returns the ID of the age's czar"""
        pass

    def getDisplayName(self):
        """Returns the displayable version of the age name"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getParentAgeLink(self):
        """Returns ptVaultAgeLinkNode of the age's parent age, or None if not a child age"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def isPublic(self):
        """Returns whether the age is Public or Not"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setAgeDescription(self,description):
        """Sets the description of the age"""
        pass

    def setAgeFilename(self,fileName):
        """Sets the filename"""
        pass

    def setAgeID(self,ageID):
        """Sets the age ID"""
        pass

    def setAgeInstanceGuid(self,guid):
        """Sets the age instance GUID"""
        pass

    def setAgeInstanceName(self,instanceName):
        """Sets the instance name"""
        pass

    def setAgeLanguage(self,lang):
        """Sets the age's language (integer)"""
        pass

    def setAgeSequenceNumber(self,seqNumber):
        """Sets the sequence number"""
        pass

    def setAgeUserDefinedName(self,udname):
        """Sets the user defined part of the name"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultAgeLinkNode(ptVaultNode):
    """Plasma vault age link node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def addSpawnPoint(self,point):
        """Adds the specified ptSpawnPointInfo or ptSpawnPointInfoRef"""
        pass

    def asAgeLinkStruct(self):
        """Returns this ptVaultAgeLinkNode as a ptAgeLinkStruct"""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getAgeInfo(self):
        """Returns the ageInfo as a ptAgeInfoStruct"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getLocked(self):
        """Returns whether the link is locked or not"""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getSpawnPoints(self):
        """Returns a list of ptSpawnPointInfo objects"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def getVolatile(self):
        """Returns whether the link is volatile or not"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def hasSpawnPoint(self,spawnPtName):
        """Returns true if this link has the specified spawn point"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def removeSpawnPoint(self,point):
        """Removes the specified spawn point based on a ptSpawnPointInfo, ptSpawnPointInfoRef, or string"""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setLocked(self,state):
        """Sets whether the link is locked or not"""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def setVolatile(self,state):
        """Sets the state of the volitility of the link"""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultChronicleNode(ptVaultNode):
    """Plasma vault chronicle node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def chronicleGetName(self):
        """LEGACY: Returns the name of the chronicle node."""
        pass

    def chronicleGetType(self):
        """LEGACY: Returns the user defined type of the chronicle node."""
        pass

    def chronicleGetValue(self):
        """LEGACY: Returns the value as a string of this chronicle node."""
        pass

    def chronicleSetName(self,name):
        """LEGACY: Sets the name of the chronicle node."""
        pass

    def chronicleSetType(self,type):
        """LEGACY: Sets this chronicle node to a user defined type."""
        pass

    def chronicleSetValue(self,value):
        """LEGACY: Sets the chronicle to a value that is a string"""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getEntryType(self):
        """Returns the user defined type of the chronicle node."""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getName(self):
        """Returns the name of the chronicle node."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def getValue(self):
        """Returns the value as a string of this chronicle node."""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setEntryType(self,type):
        """Sets this chronicle node to a user defined type."""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setName(self,name):
        """Sets the name of the chronicle node."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def setValue(self,value):
        """Sets the chronicle to a value that is a string"""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultImageNode(ptVaultNode):
    """Plasma vault image node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getImage(self):
        """Returns the image(ptImage) of this image node"""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getTitle(self):
        """Returns the title (caption) of this image node"""
        pass

    def getTitleW(self):
        """Unicode version of getTitle"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def imageGetImage(self):
        """LEGACY
Returns the image(ptImage) of this image node"""
        pass

    def imageGetTitle(self):
        """LEGACY
Returns the title (caption) of this image node"""
        pass

    def imageSetImage(self,image):
        """LEGACY
Sets the image(ptImage) of this image node"""
        pass

    def imageSetTitle(self,title):
        """LEGACY
Sets the title (caption) of this image node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setImage(self,image):
        """Sets the image(ptImage) of this image node"""
        pass

    def setImageFromBuf(self,buf):
        """Sets our image from a buffer"""
        pass

    def setImageFromScrShot(self):
        """Grabs a screenshot and stuffs it into this node"""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setTitle(self,title):
        """Sets the title (caption) of this image node"""
        pass

    def setTitleW(self,title):
        """Unicode version of setTitle"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultMarkerGameNode(ptVaultNode):
    """Plasma vault age info node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getGameGuid(self):
        """Returns the marker game's guid"""
        pass

    def getGameName(self):
        """Returns the marker game's name"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setGameGuid(self,guid):
        """Sets the marker game's guid"""
        pass

    def setGameName(self,name):
        """Sets marker game's name"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultNodeRef:
    """Vault node relationship pseudo class"""
    def __init__(self):
        """None"""
        pass

    def beenSeen(self):
        """Returns true until we reimplement this"""
        pass

    def getChild(self):
        """Returns a ptVaultNode that is the child of this reference"""
        pass

    def getChildID(self):
        """Returns id of the child node"""
        pass

    def getParent(self):
        """Returns a ptVaultNode that is the parent of the reference"""
        pass

    def getParentID(self):
        """Returns id of the parent node"""
        pass

    def getSaver(self):
        """Returns a ptVaultPlayerInfoNode of player that created this relationship"""
        pass

    def getSaverID(self):
        """Returns id of player that created this relationship"""
        pass

    def setSeen(self):
        """Does nothing until we reimplement this"""
        pass

class ptVaultPlayerInfoListNode(ptVaultFolderNode):
    """Plasma vault player info list node"""
    def __init__(self,n=0):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def addPlayer(self,playerID):
        """Adds playerID player to this player info list node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def folderGetName(self):
        """LEGACY
Returns the folder's name"""
        pass

    def folderGetType(self):
        """LEGACY
Returns the folder type (of the standard folder types)"""
        pass

    def folderSetName(self,name):
        """LEGACY
Set the folder name"""
        pass

    def folderSetType(self,type):
        """LEGACY
Set the folder type"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getFolderName(self):
        """Returns the folder's name"""
        pass

    def getFolderNameW(self):
        """Unicode version of getFolerName"""
        pass

    def getFolderType(self):
        """Returns the folder type (of the standard folder types)"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getPlayer(self,playerID):
        """Gets the player info node for the specified player."""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def hasPlayer(self,playerID):
        """Returns whether the 'playerID' is a member of this player info list node."""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def playerlistAddPlayer(self,playerID):
        """LEGACY: Adds playerID player to this player info list node."""
        pass

    def playerlistGetPlayer(self,playerID):
        """LEGACY: Gets the player info node for the specified player."""
        pass

    def playerlistHasPlayer(self,playerID):
        """LEGACY: Returns whether the 'playerID' is a member of this player info list node."""
        pass

    def playerlistRemovePlayer(self,playerID):
        """LEGACY: Removes playerID player from this player info list node."""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def removePlayer(self,playerID):
        """Removes playerID player from this player info list node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setFolderName(self,name):
        """Set the folder name"""
        pass

    def setFolderNameW(self,name):
        """Unicode version of setFolderName"""
        pass

    def setFolderType(self,type):
        """Set the folder type"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def sort(self):
        """Sorts the player list by some means...?"""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultPlayerInfoNode(ptVaultNode):
    """Plasma vault folder node"""
    def __init__(self):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def playerGetAgeGuid(self):
        """Returns the guid as a string of where the player is for this player info node."""
        pass

    def playerGetAgeInstanceName(self):
        """Returns the name of the Age where the player is for this player info node."""
        pass

    def playerGetCCRLevel(self):
        """Returns the ccr level of the player for this player info node."""
        pass

    def playerGetID(self):
        """Returns the player ID for this player info node."""
        pass

    def playerGetName(self):
        """Returns the player name of this player info node."""
        pass

    def playerIsOnline(self):
        """Returns the online status of the player for this player info node."""
        pass

    def playerSetAgeGuid(self,guidString):
        """Not sure this should be used. Sets the guid for this player info node."""
        pass

    def playerSetAgeInstanceName(self,name):
        """Not sure this should be used. Sets the name of the age where the player is for this player info node."""
        pass

    def playerSetID(self,playerID):
        """Not sure this should be used. Sets the playerID for this player info node."""
        pass

    def playerSetName(self,name):
        """Not sure this should be used. Sets the player name of this player info node."""
        pass

    def playerSetOnline(self,state):
        """Not sure this should be used. Sets the state of the player online status for this player info node."""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultSDLNode(ptVaultNode):
    """Plasma vault SDL node"""
    def __init__(self):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getIdent(self):
        """UNKNOWN"""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getStateDataRecord(self):
        """Returns the ptSDLStateDataRecord associated with this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def initStateDataRecord(self,filename,flags):
        """Read the SDL Rec from File if needed"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setIdent(self,v):
        """UNKNOWN"""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setStateDataRecord(self,rec,writeOptions=0):
        """Sets the ptSDLStateDataRecord"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultSystemNode(ptVaultNode):
    """Plasma vault system node"""
    def __init__(self):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVaultTextNoteNode(ptVaultNode):
    """Plasma vault text note node"""
    def __init__(self):
        """None"""
        pass

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        pass

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        pass

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        pass

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        pass

    def getClientID(self):
        """Returns the client's ID."""
        pass

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        pass

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        pass

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        pass

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        pass

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        pass

    def getCreatorNode(self):
        """Returns the creator's node"""
        pass

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        pass

    def getDeviceInbox(self):
        """Returns a ptVaultFolderNode"""
        pass

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        pass

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        pass

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        pass

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        pass

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        pass

    def getSubType(self):
        """Returns the subtype of this text note node."""
        pass

    def getText(self):
        """Returns the text of this text note node."""
        pass

    def getTextW(self):
        """Unicode version of getText."""
        pass

    def getTitle(self):
        """Returns the title of this text note node."""
        pass

    def getTitleW(self):
        """Unicode version of getTitle"""
        pass

    def getType(self):
        """Returns the type of text note for this text note node."""
        pass

    def hasNode(self,id):
        """Returns true if node if a child node"""
        pass

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        pass

    def noteGetSubType(self):
        """LEGACY
Returns the subtype of this text note node."""
        pass

    def noteGetText(self):
        """LEGACY
Returns the text of this text note node."""
        pass

    def noteGetTitle(self):
        """LEGACY
Returns the title of this text note node."""
        pass

    def noteGetType(self):
        """LEGACY
Returns the type of text note for this text note node."""
        pass

    def noteSetSubType(self,subType):
        """LEGACY
Sets the subtype of the this text note node."""
        pass

    def noteSetText(self,text):
        """LEGACY
Sets text of the this text note node."""
        pass

    def noteSetTitle(self,title):
        """LEGACY
Sets the title of this text note node."""
        pass

    def noteSetType(self,type):
        """LEGACY
Sets the type of text note for this text note node."""
        pass

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        pass

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        pass

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        pass

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        pass

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        pass

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        pass

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        pass

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        pass

    def setDeviceInbox(self,inboxName,cb=None,cbContext=0):
        """Sets the device inbox"""
        pass

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        pass

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        pass

    def setSubType(self,subType):
        """Sets the subtype of the this text note node."""
        pass

    def setText(self,text):
        """Sets text of the this text note node."""
        pass

    def setTextW(self,text):
        """Unicode version of setText"""
        pass

    def setTitle(self,title):
        """Sets the title of this text note node."""
        pass

    def setTitleW(self,title):
        """Unicode version of setTitle"""
        pass

    def setType(self,type):
        """Sets the type of text note for this text note node."""
        pass

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        pass

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        pass

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        pass

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        pass

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        pass

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        pass

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        pass

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        pass

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        pass

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        pass

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        pass

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        pass

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        pass

class ptVector3:
    """Plasma Point class"""
    def __init__(self,x=0, y=0, z=0):
        """None"""
        pass

    def add(self,other):
        """Adds other to the current vector"""
        pass

    def copy(self):
        """Copies the vector into another one (which it returns)"""
        pass

    def crossProduct(self,other):
        """Finds the cross product between other and this vector"""
        pass

    def dotProduct(self,other):
        """Finds the dot product between other and this vector"""
        pass

    def getX(self):
        """Returns the 'x' component of the vector"""
        pass

    def getY(self):
        """Returns the 'y' component of the vector"""
        pass

    def getZ(self):
        """Returns the 'z' component of the vector"""
        pass

    def length(self):
        """Returns the length of the vector"""
        pass

    def lengthSq(self):
        """Returns the length of the vector, squared
- this function is faster then length(other)"""
        pass

    def normalize(self):
        """Normalizes the vector to length 1"""
        pass

    def scale(self,scale):
        """Scale the vector by scale"""
        pass

    def setX(self,x):
        """Sets the 'x' component of the vector"""
        pass

    def setY(self,y):
        """Sets the 'y' component of the vector"""
        pass

    def setZ(self,z):
        """Sets the 'z' component of the vector"""
        pass

    def subtract(self,other):
        """Subtracts other from the current vector"""
        pass

    def zero(self):
        """Zeros the vector's components"""
        pass

class ptWaveSet:
    """Creates a new ptWaveSet"""
    def __init__(self,ey):
        """None"""
        pass

    def getDepthFalloff(self):
        """Returns the attribute's value"""
        pass

    def getEnvCenter(self):
        """Returns the attribute's value"""
        pass

    def getEnvRadius(self):
        """Returns the attribute's value"""
        pass

    def getGeoAmpOverLen(self):
        """Returns the attribute's value"""
        pass

    def getGeoAngleDev(self):
        """Returns the attribute's value"""
        pass

    def getGeoChop(self):
        """Returns the attribute's value"""
        pass

    def getGeoMaxLength(self):
        """Returns the attribute's value"""
        pass

    def getGeoMinLength(self):
        """Returns the attribute's value"""
        pass

    def getMaxAtten(self):
        """Returns the attribute's value"""
        pass

    def getMinAtten(self):
        """Returns the attribute's value"""
        pass

    def getOpacFalloff(self):
        """Returns the attribute's value"""
        pass

    def getOpacOffset(self):
        """Returns the attribute's value"""
        pass

    def getReflFalloff(self):
        """Returns the attribute's value"""
        pass

    def getReflOffset(self):
        """Returns the attribute's value"""
        pass

    def getRippleScale(self):
        """Returns the attribute's value"""
        pass

    def getSpecularEnd(self):
        """Returns the attribute's value"""
        pass

    def getSpecularMute(self):
        """Returns the attribute's value"""
        pass

    def getSpecularNoise(self):
        """Returns the attribute's value"""
        pass

    def getSpecularStart(self):
        """Returns the attribute's value"""
        pass

    def getSpecularTint(self):
        """Returns the attribute's value"""
        pass

    def getTexAmpOverLen(self):
        """Returns the attribute's value"""
        pass

    def getTexAngleDev(self):
        """Returns the attribute's value"""
        pass

    def getTexChop(self):
        """Returns the attribute's value"""
        pass

    def getTexMaxLength(self):
        """Returns the attribute's value"""
        pass

    def getTexMinLength(self):
        """Returns the attribute's value"""
        pass

    def getWaterHeight(self):
        """Returns the attribute's value"""
        pass

    def getWaterOffset(self):
        """Returns the attribute's value"""
        pass

    def getWaterOpacity(self):
        """Returns the attribute's value"""
        pass

    def getWaterTint(self):
        """Returns the attribute's value"""
        pass

    def getWaveFalloff(self):
        """Returns the attribute's value"""
        pass

    def getWaveOffset(self):
        """Returns the attribute's value"""
        pass

    def getWindDir(self):
        """Returns the attribute's value"""
        pass

    def setDepthFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setEnvCenter(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setEnvRadius(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setGeoAmpOverLen(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setGeoAngleDev(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setGeoChop(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setGeoMaxLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setGeoMinLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setMaxAtten(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setMinAtten(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setOpacFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setOpacOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setReflFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setReflOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setRippleScale(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setSpecularEnd(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setSpecularMute(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setSpecularNoise(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setSpecularStart(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setSpecularTint(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setTexAmpOverLen(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setTexAngleDev(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setTexChop(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setTexMaxLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setTexMinLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWaterHeight(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWaterOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWaterOpacity(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWaterTint(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWaveFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWaveOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

    def setWindDir(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        pass

