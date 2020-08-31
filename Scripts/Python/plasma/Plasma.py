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
def PtAcceptInviteInGame(friendName,inviteKey):
    """Sends a VaultTask to the server to perform the invite"""
    raise NotImplementedError()

def PtAmCCR():
    """Returns true if local player is a CCR"""
    raise NotImplementedError()

def PtAtTimeCallback(selfkey,time,id):
    """This will create a timer callback that will call OnTimer when complete
- 'selfkey' is the ptKey of the PythonFile component
- 'time' is how much time from now (in seconds) to call back
- 'id' is an integer id that will be returned in the OnTimer call"""
    raise NotImplementedError()

def PtAttachObject(child,parent):
    """Attach child to parent based on ptKey or ptSceneobject
- childKey is the ptKey or ptSceneobject of the one being attached
- parentKey is the ptKey or ptSceneobject of the one being attached to
(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)"""
    raise NotImplementedError()

def PtAvatarEnterAFK():
    """Tells the local avatar to enter AwayFromKeyboard idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarEnterAnimMode(animName):
    """Enter a custom anim loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarEnterLookingAtKI():
    """Tells the local avatar to enter looking at KI idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarEnterUsePersBook():
    """Tells the local avatar to enter using their personal book idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarExitAFK():
    """Tells the local avatar to exit AwayFromKeyboard idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarExitAnimMode(animName):
    """Exit custom anim loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarExitLookingAtKI():
    """Tells the local avatar to exit looking at KI idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarExitUsePersBook():
    """Tells the local avatar to exit using their personal book idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarSitOnGround():
    """Tells the local avatar to sit on ground and enter sit idle loop (netpropagated)"""
    raise NotImplementedError()

def PtAvatarSpawnNext():
    """Send the avatar to the next spawn point"""
    raise NotImplementedError()

def PtCanShadowCast():
    """Can we cast shadows?"""
    raise NotImplementedError()

def PtChangeAvatar(gender):
    """Change the local avatar's gender (or clothing type)"""
    raise NotImplementedError()

def PtChangePassword(password):
    """Changes the current account's password"""
    raise NotImplementedError()

def PtChangePlayerName(name):
    """Change the local avatar's name"""
    raise NotImplementedError()

def PtCheckVisLOS(startPoint,endPoint):
    """Does LOS check from start to end"""
    raise NotImplementedError()

def PtCheckVisLOSFromCursor():
    """Does LOS check from where the mouse cursor is, into the screen"""
    raise NotImplementedError()

def PtClearCameraStack():
    """clears all cameras"""
    raise NotImplementedError()

def PtClearOfferBookMode():
    """Cancel the offer book interface"""
    raise NotImplementedError()

def PtClearPrivateChatList(memberKey):
    """Remove the local avatar from private vox messaging, and / or clear members from his chat list"""
    raise NotImplementedError()

def PtClearTimerCallbacks(key):
    """This will remove timer callbacks to the specified key"""
    raise NotImplementedError()

def PtConsole(command):
    """This will execute 'command' as if it were typed into the Plasma console."""
    raise NotImplementedError()

def PtConsoleNet(command,netForce):
    """This will execute 'command' on the console, over the network, on all clients.
If 'netForce' is true then force command to be sent over the network."""
    raise NotImplementedError()

def PtCreateDir(directory):
    """Creates the directory and all parent folders. Returns false on failure"""
    raise NotImplementedError()

def PtCreatePlayer(playerName, avatarShape, invitation):
    """Creates a new player"""
    raise NotImplementedError()

def PtCreatePublicAge(ageInfo, cbObject=None):
    """Create a public instance of the given age.
cbObject, if supplied should have a member called publicAgeCreated(self,ageInfo)"""
    raise NotImplementedError()

def PtDebugAssert(cond, msg):
    """Debug only: Assert if condition is false."""
    raise NotImplementedError()

def PtDebugPrint(*msgs, **kwargs):
    """Prints msgs to the Python log given the message's level"""
    raise NotImplementedError()

def PtDeletePlayer(playerInt):
    """Deletes a player associated with the current account"""
    raise NotImplementedError()

def PtDetachObject(child,parent):
    """Detach child from parent based on ptKey or ptSceneobject
- child is the ptKey or ptSceneobject of the one being detached
- parent is the ptKey or ptSceneobject of the one being detached from
(both arguments must be ptKeys or ptSceneobjects, you cannot mix types)"""
    raise NotImplementedError()

def PtDirtySynchClients(selfKey,SDLStateName,flags):
    """DO NOT USE - handled by ptSDL"""
    raise NotImplementedError()

def PtDirtySynchState(selfKey,SDLStateName,flags):
    """DO NOT USE - handled by ptSDL"""
    raise NotImplementedError()

def PtDisableAvatarCursorFade():
    """Disable the avatar cursor fade"""
    raise NotImplementedError()

def PtDisableAvatarJump():
    """Disable the ability of the avatar to jump"""
    raise NotImplementedError()

def PtDisableControlKeyEvents(selfKey):
    """Disable the control key events from calling OnControlKeyEvent"""
    raise NotImplementedError()

def PtDisableForwardMovement():
    """Disable the ability of the avatar to move forward"""
    raise NotImplementedError()

def PtDisableMouseMovement():
    """Disable avatar mouse movement input"""
    raise NotImplementedError()

def PtDisableMovementKeys():
    """Disable avatar movement input"""
    raise NotImplementedError()

def PtDisableRenderScene():
    """UNKNOWN"""
    raise NotImplementedError()

def PtDisableShadows():
    """Turns shadows off"""
    raise NotImplementedError()

def PtDumpLogs(folder):
    """Dumps all current log files to the specified folder (a sub-folder to the log folder)"""
    raise NotImplementedError()

def PtEmoteAvatar(emote):
    """Play an emote on the local avatar (netpropagated)"""
    raise NotImplementedError()

def PtEnableAvatarCursorFade():
    """Enable the avatar cursor fade"""
    raise NotImplementedError()

def PtEnableAvatarJump():
    """Enable the ability of the avatar to jump"""
    raise NotImplementedError()

def PtEnableControlKeyEvents(selfKey):
    """Enable control key events to call OnControlKeyEvent(controlKey,activateFlag)"""
    raise NotImplementedError()

def PtEnableForwardMovement():
    """Enable the ability of the avatar to move forward"""
    raise NotImplementedError()

def PtEnableMouseMovement():
    """Enable avatar mouse movement input"""
    raise NotImplementedError()

def PtEnableMovementKeys():
    """Enable avatar movement input"""
    raise NotImplementedError()

def PtEnablePlanarReflections(on):
    """Enables/disables planar reflections"""
    raise NotImplementedError()

def PtEnableRenderScene():
    """UNKNOWN"""
    raise NotImplementedError()

def PtEnableShadows():
    """Turns shadows on"""
    raise NotImplementedError()

def PtExcludeRegionSet(senderKey,regionKey,state):
    """This will set the state of an exclude region
- 'senderKey' is a ptKey of the PythonFile component
- 'regionKey' is a ptKey of the exclude region
- 'state' is either kExRegRelease or kExRegClear"""
    raise NotImplementedError()

def PtExcludeRegionSetNow(senderKey,regionKey,state):
    """This will set the state of an exclude region immediately on the server
- 'senderKey' is a ptKey of the PythonFile component
- 'regionKey' is a ptKey of the exclude region
- 'state' is either kExRegRelease or kExRegClear"""
    raise NotImplementedError()

def PtFadeIn(lenTime, holdFlag, noSound=0):
    """Fades screen in for lenTime seconds"""
    raise NotImplementedError()

def PtFadeLocalAvatar(fade):
    """Fade (or unfade) the local avatar"""
    raise NotImplementedError()

def PtFadeOut(lenTime, holdFlag, noSound=0):
    """Fades screen out for lenTime seconds"""
    raise NotImplementedError()

def PtFakeLinkAvatarToObject(avatar,object):
    """Pseudo-links avatar to object within the same age
"""
    raise NotImplementedError()

def PtFileExists(filename):
    """Returns true if the specified file exists"""
    raise NotImplementedError()

def PtFindSceneobject(name,ageName):
    """This will try to find a sceneobject based on its name and what age its in
- it will return a ptSceneObject if found- if not found then a NameError exception will happen"""
    raise NotImplementedError()

def PtFirstPerson():
    """is the local avatar in first person mode"""
    raise NotImplementedError()

def PtFlashWindow():
    """Flashes the client window if it is not focused"""
    raise NotImplementedError()

def PtFogSetDefColor(color):
    """Sets default fog color"""
    raise NotImplementedError()

def PtFogSetDefExp(end,density):
    """Set exp fog values"""
    raise NotImplementedError()

def PtFogSetDefExp2(end,density):
    """Set exp2 fog values"""
    raise NotImplementedError()

def PtFogSetDefLinear(start,end,density):
    """Set linear fog values"""
    raise NotImplementedError()

def PtForceCursorHidden():
    """Forces the cursor to hide, overriding everything.
Only call if other methods won't work. The only way to show the cursor after this call is PtForceMouseShown()"""
    raise NotImplementedError()

def PtForceCursorShown():
    """Forces the cursor to show, overriding everything.
Only call if other methods won't work. This is the only way to show the cursor after a call to PtForceMouseHidden()"""
    raise NotImplementedError()

def PtGMTtoDniTime(gtime):
    """Converts GMT time (passed in) to D'Ni time"""
    raise NotImplementedError()

def PtGUICursorDimmed():
    """Dimms the GUI cursor"""
    raise NotImplementedError()

def PtGUICursorOff():
    """Turns the GUI cursor off"""
    raise NotImplementedError()

def PtGUICursorOn():
    """Turns the GUI cursor on"""
    raise NotImplementedError()

def PtGetAccountName():
    """Returns the account name for the current account"""
    raise NotImplementedError()

def PtGetAccountPlayerList():
    """Returns list of players associated with the current account"""
    raise NotImplementedError()

def PtGetAgeInfo():
    """Returns ptAgeInfoStruct of the current Age"""
    raise NotImplementedError()

def PtGetAgeName():
    """DEPRECIATED - use ptDniInfoSource instead"""
    raise NotImplementedError()

def PtGetAgeSDL():
    """Returns the global ptSDL for the current Age"""
    raise NotImplementedError()

def PtGetAgeTime():
    """DEPRECIATED - use ptDniInfoSource instead"""
    raise NotImplementedError()

def PtGetAgeTimeOfDayPercent():
    """Returns the current age time of day as a percent (0 to 1)"""
    raise NotImplementedError()

def PtGetAvatarKeyFromClientID(clientID):
    """From an integer that is the clientID, find the avatar and return its ptKey"""
    raise NotImplementedError()

def PtGetCameraNumber(x):
    """Returns camera x's name from stack"""
    raise NotImplementedError()

def PtGetClientIDFromAvatarKey(avatarKey):
    """From a ptKey that points at an avatar, return the players clientID (integer)"""
    raise NotImplementedError()

def PtGetClientName(avatarKey=None):
    """This will return the name of the client that is owned by the avatar
- avatarKey is the ptKey of the avatar to get the client name of.
If avatarKey is omitted then the local avatar is used"""
    raise NotImplementedError()

def PtGetControlEvents(on, key):
    """Registers or unregisters for control event messages"""
    raise NotImplementedError()

def PtGetDefaultDisplayParams():
    """Returns the default resolution and display settings"""
    raise NotImplementedError()

def PtGetDefaultSpawnPoint():
    """Returns the default spawnpoint definition (as a ptSpawnPointInfo)"""
    raise NotImplementedError()

def PtGetDesktopColorDepth():
    """Returns desktop ColorDepth"""
    raise NotImplementedError()

def PtGetDesktopHeight():
    """Returns desktop height"""
    raise NotImplementedError()

def PtGetDesktopWidth():
    """Returns desktop width"""
    raise NotImplementedError()

def PtGetDialogFromString(dialogName):
    """Get a ptGUIDialog from its name"""
    raise NotImplementedError()

def PtGetDialogFromTagID(tagID):
    """Returns the dialog associated with the tagID"""
    raise NotImplementedError()

def PtGetDniTime():
    """Returns current D'Ni time"""
    raise NotImplementedError()

def PtGetFrameDeltaTime():
    """Returns the amount of time that has elapsed since last frame."""
    raise NotImplementedError()

def PtGetGameTime():
    """Returns the system game time (frame based) in seconds."""
    raise NotImplementedError()

def PtGetInitPath():
    """Returns the unicode path to the client's init directory. Do NOT convert to a standard string."""
    raise NotImplementedError()

def PtGetLanguage():
    """Returns the current language as a PtLanguage enum"""
    raise NotImplementedError()

def PtGetLocalAvatar():
    """This will return a ptSceneobject of the local avatar
- if there is no local avatar a NameError exception will happen."""
    raise NotImplementedError()

def PtGetLocalClientID():
    """Returns our local client ID number"""
    raise NotImplementedError()

def PtGetLocalKILevel():
    """returns local player's ki level"""
    raise NotImplementedError()

def PtGetLocalPlayer():
    """Returns a ptPlayer object of the local player"""
    raise NotImplementedError()

def PtGetLocalizedString(name, arguments=None):
    """Returns the localized string specified by name (format is Age.Set.Name) and substitutes the arguments in the list of strings passed in as arguments."""
    raise NotImplementedError()

def PtGetMouseTurnSensitivity():
    """Returns the sensitivity"""
    raise NotImplementedError()

def PtGetNPCCount():
    """This will return the number of NPCs in the current age"""
    raise NotImplementedError()

def PtGetNPCByID(npcID):
    """This will return the NPC with a specific ID"""
    raise NotImplementedError()

def PtGetNumCameras():
    """returns camera stack size"""
    raise NotImplementedError()

def PtGetNumParticles(key):
    """Key is the key of scene object host to particle system"""
    raise NotImplementedError()

def PtGetNumRemotePlayers():
    """Returns the number of remote players in this Age with you."""
    raise NotImplementedError()

def PtGetPlayerList():
    """Returns a list of ptPlayer objects of all the remote players"""
    raise NotImplementedError()

def PtGetPlayerListDistanceSorted():
    """Returns a list of ptPlayers, sorted by distance"""
    raise NotImplementedError()

def PtGetPrevAgeInfo():
    """Returns ptAgeInfoStruct of previous age visited"""
    raise NotImplementedError()

def PtGetPrevAgeName():
    """Returns filename of previous age visited"""
    raise NotImplementedError()

def PtGetPublicAgeList(ageName, cbObject=None):
    """Get list of public ages for the given age name.
cbObject, if supplied should have a method called gotPublicAgeList(self,ageList). ageList is a list of tuple(ptAgeInfoStruct,nPlayersInAge)"""
    raise NotImplementedError()

def PtGetPythonLoggingLevel():
    """Returns the current level of python logging"""
    raise NotImplementedError()

def PtGetServerTime():
    """Returns the current time on the server (which is GMT)"""
    raise NotImplementedError()

def PtGetShadowVisDistance():
    """Returns the maximum shadow visibility distance"""
    raise NotImplementedError()

def PtGetSupportedDisplayModes():
    """Returns a list of supported resolutions"""
    raise NotImplementedError()

def PtGetTime():
    """Returns the number of seconds since the game was started."""
    raise NotImplementedError()

def PtGetUserPath():
    """Returns the unicode path to the client's root user directory. Do NOT convert to a standard string."""
    raise NotImplementedError()

def PtHideDialog(dialogName):
    """Hide a GUI dialog by name (does not unload dialog)"""
    raise NotImplementedError()

def PtIsActivePlayerSet():
    """Returns whether or not an active player is set"""
    raise NotImplementedError()

def PtIsCCRAway():
    """Returns current status of CCR dept"""
    raise NotImplementedError()

def PtIsClickToTurn():
    """Is click-to-turn on?"""
    raise NotImplementedError()

def PtIsCurrentBrainHuman():
    """Returns whether the local avatar current brain is the human brain"""
    raise NotImplementedError()

def PtIsDemoMode():
    """Returns whether the game is in Demo mode or not"""
    raise NotImplementedError()

def PtIsDialogLoaded(dialogName):
    """Test to see if a GUI dialog is loaded, by name"""
    raise NotImplementedError()

def PtIsEnterChatModeKeyBound():
    """Returns whether the EnterChatMode is bound to a key"""
    raise NotImplementedError()

def PtIsGUIModal():
    """Returns true if the GUI is displaying a modal dialog and blocking input"""
    raise NotImplementedError()

def PtIsInternalRelease():
    """Returns whether the client is an internal build or not"""
    raise NotImplementedError()

def PtIsMouseInverted():
    """Is the mouse currently inverted?"""
    raise NotImplementedError()

def PtIsShadowsEnabled():
    """Returns whether shadows are currently turned on"""
    raise NotImplementedError()

def PtIsSinglePlayerMode():
    """Returns whether the game is in single player mode or not"""
    raise NotImplementedError()

def PtKillParticles(timeRemaining,pctToKill,particleSystem):
    """Tells particleSystem to kill pctToKill percent of its particles"""
    raise NotImplementedError()

def PtLimitAvatarLOD(LODlimit):
    """Sets avatar's LOD limit"""
    raise NotImplementedError()

def PtLoadAvatarModel(modelName, spawnPoint, userStr = ""):
    """Loads an avatar model at the given spawn point. Assigns the user specified string to it."""
    raise NotImplementedError()

def PtLoadBookGUI(guiName):
    """Loads the gui specified, a gui must be loaded before it can be used. If the gui is already loaded, doesn't do anything"""
    raise NotImplementedError()

def PtLoadDialog(dialogName,selfKey=None,ageName=""):
    """Loads a GUI dialog by name and optionally set the Notify proc key
If the dialog is already loaded then it won't load it again"""
    raise NotImplementedError()

def PtLoadJPEGFromDisk(filename,width,height):
    """The image will be resized to fit the width and height arguments. Set to 0 if resizing is not desired.
Returns a pyImage of the specified file."""
    raise NotImplementedError()

def PtLocalAvatarIsMoving():
    """Returns true if the local avatar is moving (a movement key is held down)"""
    raise NotImplementedError()

def PtLocalAvatarRunKeyDown():
    """Returns true if the run key is being held down for the local avatar"""
    raise NotImplementedError()

def PtMaxListenDistSq():
    """Returns the maximum distance (squared) of the listen range"""
    raise NotImplementedError()

def PtMaxListenListSize():
    """Returns the maximum listen number of players"""
    raise NotImplementedError()

def PtNotifyOffererLinkAccepted(offerer):
    """Tell the offerer that we accepted the link offer"""
    raise NotImplementedError()

def PtNotifyOffererLinkCompleted(offerer):
    """Tell the offerer that we completed the link"""
    raise NotImplementedError()

def PtNotifyOffererLinkRejected(offerer):
    """Tell the offerer that we rejected the link offer"""
    raise NotImplementedError()

def PtPageInNode(nodeName, ageName=""):
    """Pages in node, or a list of nodes"""
    raise NotImplementedError()

def PtPageOutNode(nodeName):
    """Pages out a node"""
    raise NotImplementedError()

def PtPrintToScreen(message):
    """Prints 'message' to the status log, for debug only."""
    raise NotImplementedError()

def PtRateIt(chronicleName,dialogPrompt,onceFlag):
    """Shows a dialog with dialogPrompt and stores user input rating into chronicleName"""
    raise NotImplementedError()

def PtRebuildCameraStack(name,ageName):
    """Push camera with this name on the stack"""
    raise NotImplementedError()

def PtRecenterCamera():
    """re-centers the camera"""
    raise NotImplementedError()

def PtRemovePublicAge(ageInstanceGuid, cbObject=None):
    """Remove a public instance of the given age.
cbObject, if supplied should have a member called publicAgeRemoved(self,ageInstanceGuid)"""
    raise NotImplementedError()

def PtRequestLOSScreen(selfKey,ID,xPos,yPos,distance,what,reportType):
    """Request a LOS check from a point on the screen"""
    raise NotImplementedError()

def PtSaveScreenShot(fileName,width=640,height=480,quality=75):
    """Takes a screenshot with the specified filename, size, and quality"""
    raise NotImplementedError()

def PtSendChatToCCR(message,CCRPlayerID):
    """Sends a chat message to a CCR that has contacted this player"""
    raise NotImplementedError()

def PtSendKIGZMarkerMsg(markerNumber,sender):
    """Same as PtSendKIMessageInt except 'sender' could get a notify message back
"""
    raise NotImplementedError()

def PtSendKIMessage(command,value):
    """Sends a command message to the KI frontend.
See PlasmaKITypes.py for list of commands"""
    raise NotImplementedError()

def PtSendKIMessageInt(command,value):
    """Same as PtSendKIMessage except the value is guaranteed to be a UInt32
(for things like player IDs)"""
    raise NotImplementedError()

def PtSendPetitionToCCR(message,reason=0,title=""):
    """Sends a petition with a message to the CCR group"""
    raise NotImplementedError()

def PtSendPrivateChatList(chatList):
    """Lock the local avatar into private vox messaging, and / or add new members to his chat list"""
    raise NotImplementedError()

def PtSendRTChat(fromPlayer,toPlayerList,message,flags):
    """Sends a realtime chat message to the list of ptPlayers
If toPlayerList is an empty list, it is a broadcast message"""
    raise NotImplementedError()

def PtSetActivePlayer(playerInt):
    """Sets the active player associated with the current account"""
    raise NotImplementedError()

def PtSetAlarm(secs, cbObject, cbContext):
    """secs is the amount of time before your alarm goes off.
cbObject is a python object with the method onAlarm(int context)
cbContext is an integer."""
    raise NotImplementedError()

def PtSetBehaviorLoopCount(behaviorKey,stage,loopCount,netForce):
    """This will set the loop count for a particular stage in a multistage behavior"""
    raise NotImplementedError()

def PtSetBehaviorNetFlags(behKey, netForce, netProp):
    """Sets net flags on the associated behavior"""
    raise NotImplementedError()

def PtSetClearColor(red,green,blue):
    """Set the clear color"""
    raise NotImplementedError()

def PtSetClickToTurn(state):
    """Turns on click-to-turn"""
    raise NotImplementedError()

def PtSetGamma2(gamma):
    """Set the gamma with gamma2 rules"""
    raise NotImplementedError()

def PtSetGlobalClickability(enable):
    """Enable or disable all clickables on the local client"""
    raise NotImplementedError()

def PtSetGraphicsOptions(width, height, colordepth, windowed, numAAsamples, numAnisoSamples, VSync):
    """Set the graphics options"""
    raise NotImplementedError()

def PtSetLightAnimStart(key,name,start):
    """ Key is the key of scene object host to light, start is a bool. Name is the name of the light to manipulate"""
    raise NotImplementedError()

def PtSetLightValue(key,name,r,g,b,a):
    """ Key is the key of scene object host to light. Name is the name of the light to manipulate"""
    raise NotImplementedError()

def PtSetMouseInverted():
    """Inverts the mouse"""
    raise NotImplementedError()

def PtSetMouseTurnSensitivity(sensitivity):
    """Set the mouse sensitivity"""
    raise NotImplementedError()

def PtSetMouseUninverted():
    """Uninverts the mouse"""
    raise NotImplementedError()

def PtSetOfferBookMode(selfkey,ageFilename,ageInstanceName):
    """Put us into the offer book interface"""
    raise NotImplementedError()

def PtSetParticleDissentPoint(x, y, z, particlesys):
    """Sets the dissent point of the particlesys to x,y,z"""
    raise NotImplementedError()

def PtSetParticleOffset(x,y,z,particlesys):
    """Sets the particlesys particle system's offset"""
    raise NotImplementedError()

def PtSetPythonLoggingLevel(level):
    """Sets the current level of python logging"""
    raise NotImplementedError()

def PtSetShadowVisDistance(distance):
    """Set the maximum shadow visibility distance"""
    raise NotImplementedError()

def PtSetShareSpawnPoint(spawnPoint):
    """This sets the desired spawn point for the receiver to link to"""
    raise NotImplementedError()

def PtShootBulletFromObject(selfkey, gunObj, radius, range):
    """Shoots a bullet from an object"""
    raise NotImplementedError()

def PtShootBulletFromScreen(selfkey, xPos, yPos, radius, range):
    """Shoots a bullet from a position on the screen"""
    raise NotImplementedError()

def PtShowDialog(dialogName):
    """Show a GUI dialog by name (does not load dialog)"""
    raise NotImplementedError()

def PtStartScreenCapture(selfKey,width=800,height=600):
    """Starts a capture of the screen"""
    raise NotImplementedError()

def PtToggleAvatarClickability(on):
    """Turns on and off our avatar's clickability"""
    raise NotImplementedError()

def PtTransferParticlesToObject(objFrom, objTo, num):
    """Transfers num particles from objFrom to objTo"""
    raise NotImplementedError()

def PtUnLoadAvatarModel(avatarKey):
    """Unloads the specified avatar model"""
    raise NotImplementedError()

def PtUnloadAllBookGUIs():
    """Unloads all loaded guis except for the default one"""
    raise NotImplementedError()

def PtUnloadBookGUI(guiName):
    """Unloads the gui specified. If the gui isn't loaded, doesn't do anything"""
    raise NotImplementedError()

def PtUnloadDialog(dialogName):
    """This will unload the GUI dialog by name. If not loaded then nothing will happen"""
    raise NotImplementedError()


def PtUsingUnicode():
    """Returns true if the current language is a unicode language (like Japanese)"""
    raise NotImplementedError()

def PtValidateKey(key):
    """Returns true(1) if 'key' is valid and loaded,
otherwise returns false(0)"""
    raise NotImplementedError()

def PtWasLocallyNotified(selfKey):
    """Returns 1 if the last notify was local or 0 if the notify originated on the network"""
    raise NotImplementedError()

def PtWearDefaultClothing(key):
    """Forces the avatar to wear the default clothing set"""
    raise NotImplementedError()

def PtWearDefaultClothingType(key,type):
    """Forces the avatar to wear the default clothing of the specified type"""
    raise NotImplementedError()

def PtWearMaintainerSuit(key,wearOrNot):
    """Wears or removes the maintainer suit of clothes"""
    raise NotImplementedError()

def PtWhatGUIControlType(guiKey):
    """Returns the control type of the key passed in"""
    raise NotImplementedError()

def PtYesNoDialog(selfkey,dialogMessage):
    """This will display a Yes/No dialog to the user with the text dialogMessage
This dialog _has_ to be answered by the user.
And their answer will be returned in a Notify message."""
    raise NotImplementedError()

class ptAgeInfoStruct:
    """Class to hold AgeInfo struct data"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def copyFrom(self,other):
        """Copies data from one ptAgeInfoStruct or ptAgeInfoStructRef to this one"""
        raise NotImplementedError()

    def getAgeFilename(self):
        """Gets the Age's filename"""
        raise NotImplementedError()

    def getAgeInstanceGuid(self):
        """Get the Age's instance GUID"""
        raise NotImplementedError()

    def getAgeInstanceName(self):
        """Get the instance name of the Age"""
        raise NotImplementedError()

    def getAgeLanguage(self):
        """Gets the age's language (integer)"""
        raise NotImplementedError()

    def getAgeSequenceNumber(self):
        """Gets the unique sequence number"""
        raise NotImplementedError()

    def getAgeUserDefinedName(self):
        """Gets the user defined part of the Age name"""
        raise NotImplementedError()

    def getDisplayName(self):
        """Returns a string that is the displayable name of the age instance"""
        raise NotImplementedError()

    def setAgeFilename(self,filename):
        """Sets the filename of the Age"""
        raise NotImplementedError()

    def setAgeInstanceGuid(self,guid):
        """Sets the Age instance's GUID"""
        raise NotImplementedError()

    def setAgeInstanceName(self,instanceName):
        """Sets the instance name of the Age"""
        raise NotImplementedError()

    def setAgeLanguage(self,lang):
        """Sets the age's language (integer)"""
        raise NotImplementedError()

    def setAgeSequenceNumber(self,seqNumber):
        """Sets the unique sequence number"""
        raise NotImplementedError()

    def setAgeUserDefinedName(self,udName):
        """Sets the user defined part of the Age"""
        raise NotImplementedError()

class ptAgeInfoStructRef:
    """Class to hold AgeInfo struct data"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def copyFrom(self,other):
        """Copies data from one ptAgeInfoStruct or ptAgeInfoStructRef to this one"""
        raise NotImplementedError()

    def getAgeFilename(self):
        """Gets the Age's filename"""
        raise NotImplementedError()

    def getAgeInstanceGuid(self):
        """Get the Age's instance GUID"""
        raise NotImplementedError()

    def getAgeInstanceName(self):
        """Get the instance name of the Age"""
        raise NotImplementedError()

    def getAgeSequenceNumber(self):
        """Gets the unique sequence number"""
        raise NotImplementedError()

    def getAgeUserDefinedName(self):
        """Gets the user defined part of the Age name"""
        raise NotImplementedError()

    def getDisplayName(self):
        """Returns a string that is the displayable name of the age instance"""
        raise NotImplementedError()

    def setAgeFilename(self,filename):
        """Sets the filename of the Age"""
        raise NotImplementedError()

    def setAgeInstanceGuid(self,guid):
        """Sets the Age instance's GUID"""
        raise NotImplementedError()

    def setAgeInstanceName(self,instanceName):
        """Sets the instance name of the Age"""
        raise NotImplementedError()

    def setAgeSequenceNumber(self,seqNumber):
        """Sets the unique sequence number"""
        raise NotImplementedError()

    def setAgeUserDefinedName(self,udName):
        """Sets the user defined part of the Age"""
        raise NotImplementedError()

class ptAgeLinkStruct:
    """Class to hold the data of the AgeLink structure"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def copyFrom(self,other):
        """Copies data from one ptAgeLinkStruct or ptAgeLinkStructRef to this one"""
        raise NotImplementedError()

    def getAgeInfo(self):
        """Returns a ptAgeInfoStructRef of the AgeInfo for this link"""
        raise NotImplementedError()

    def getLinkingRules(self):
        """Returns the linking rules of this link"""
        raise NotImplementedError()

    def getParentAgeFilename(self):
        """Returns a string of the parent age filename"""
        raise NotImplementedError()

    def getSpawnPoint(self):
        """Gets the spawn point ptSpawnPointInfoRef of this link"""
        raise NotImplementedError()

    def setAgeInfo(self,ageInfo):
        """Sets the AgeInfoStruct from the data in ageInfo (a ptAgeInfoStruct)"""
        raise NotImplementedError()

    def setLinkingRules(self,rule):
        """Sets the linking rules for this link"""
        raise NotImplementedError()

    def setParentAgeFilename(self,filename):
        """Sets the parent age filename for child age links"""
        raise NotImplementedError()

    def setSpawnPoint(self,spawnPtInfo):
        """Sets the spawn point of this link (a ptSpawnPointInfo or ptSpawnPointInfoRef)"""
        raise NotImplementedError()

class ptAgeLinkStructRef:
    """Class to hold the data of the AgeLink structure"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def copyFrom(self,other):
        """Copies data from one ptAgeLinkStruct or ptAgeLinkStructRef to this one"""
        raise NotImplementedError()

    def getAgeInfo(self):
        """Returns a ptAgeInfoStructRef of the AgeInfo for this link"""
        raise NotImplementedError()

    def getLinkingRules(self):
        """Returns the linking rules of this link"""
        raise NotImplementedError()

    def getSpawnPoint(self):
        """Gets the spawn point ptSpawnPointInfoRef of this link"""
        raise NotImplementedError()

    def setAgeInfo(self,ageInfo):
        """Sets the AgeInfoStruct from the data in ageInfo (a ptAgeInfoStruct)"""
        raise NotImplementedError()

    def setLinkingRules(self,rule):
        """Sets the linking rules for this link"""
        raise NotImplementedError()

    def setSpawnPoint(self,spawnPtInfo):
        """Sets the spawn point of this link (a ptSpawnPointInfo or ptSpawnPointInfoRef)"""
        raise NotImplementedError()

class ptAgeVault:
    """Accessor class to the Age's vault"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addChronicleEntry(self,name,type,value):
        """Adds a chronicle entry with the specified type and value"""
        raise NotImplementedError()

    def addDevice(self,deviceName,cb=None,cbContext=0):
        """Adds a device to the age"""
        raise NotImplementedError()

    def findChronicleEntry(self,entryName):
        """Returns the named ptVaultChronicleNode"""
        raise NotImplementedError()

    def getAgeDevicesFolder(self):
        """Returns a ptVaultFolderNode of the inboxes for the devices in this Age."""
        raise NotImplementedError()

    def getAgeGuid(self):
        """Returns the current Age's guid as a string."""
        raise NotImplementedError()

    def getAgeInfo(self):
        """Returns a ptVaultAgeInfoNode of the this Age"""
        raise NotImplementedError()

    def getAgeSDL(self):
        """Returns the age's SDL (ptSDLStateDataRecord)"""
        raise NotImplementedError()

    def getAgesIOwnFolder(self):
        """(depreciated, use getBookshelfFolder) Returns a ptVaultFolderNode that contain the Ages I own"""
        raise NotImplementedError()

    def getBookshelfFolder(self):
        """Personal age only: Returns a ptVaultFolderNode that contains the owning player's AgesIOwn age list"""
        raise NotImplementedError()

    def getChronicleFolder(self):
        """Returns a ptVaultFolderNode"""
        raise NotImplementedError()

    def getDevice(self,deviceName):
        """Returns the specified device (ptVaultTextNoteNode)"""
        raise NotImplementedError()

    def getDeviceInbox(self,deviceName):
        """Returns a ptVaultFolderNode of the inbox for the named device in this age."""
        raise NotImplementedError()

    def getPeopleIKnowAboutFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the players the Age knows about(?)."""
        raise NotImplementedError()

    def getPublicAgesFolder(self):
        """Returns a ptVaultFolderNode that contains all the public Ages"""
        raise NotImplementedError()

    def getSubAgeLink(self,ageInfo):
        """Returns a ptVaultAgeLinkNode to 'ageInfo' (a ptAgeInfoStruct) for this Age."""
        raise NotImplementedError()

    def getSubAgesFolder(self):
        """Returns a ptVaultFolderNode of sub Age's folder."""
        raise NotImplementedError()

    def hasDevice(self,deviceName):
        """Does a device with this name exist?"""
        raise NotImplementedError()

    def removeDevice(self,deviceName):
        """Removes a device from the age"""
        raise NotImplementedError()

    def setDeviceInbox(self,deviceName,inboxName,cb=None,cbContext=0):
        """Set's the device's inbox"""
        raise NotImplementedError()

    def updateAgeSDL(self,pyrec):
        """Updates the age's SDL"""
        raise NotImplementedError()

class ptAnimation:
    """Plasma animation class"""
    def __init__(self,key=None):
        """None"""
        raise NotImplementedError()

    def addKey(self,key):
        """Adds an animation modifier to the list of receiver keys"""
        raise NotImplementedError()

    def backwards(self,backwardsFlag):
        """Turn on and off playing the animation backwards"""
        raise NotImplementedError()

    def getFirstKey(self):
        """This will return a ptKey object that is the first receiver (target)
However, if the parent is not a modifier or not loaded, then None is returned."""
        raise NotImplementedError()

    def incrementBackward(self):
        """Step the animation backward a frame"""
        raise NotImplementedError()

    def incrementForward(self):
        """Step the animation forward a frame"""
        raise NotImplementedError()

    def looped(self,loopedFlag):
        """Turn on and off looping of the animation"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

    def play(self):
        """Plays the animation"""
        raise NotImplementedError()

    def playRange(self,start,end):
        """Play the animation from start to end"""
        raise NotImplementedError()

    def playToPercentage(self,zeroToOne):
        """Play the animation to the specified percentage (0 to 1)"""
        raise NotImplementedError()

    def playToTime(self,time):
        """Play the animation to the specified time"""
        raise NotImplementedError()

    def resume(self):
        """Resumes the animation from where it was stopped last"""
        raise NotImplementedError()

    def sender(self,selfKey):
        """Sets the sender of the messages being sent to the animation modifier"""
        raise NotImplementedError()

    def setAnimName(self,name):
        """Sets the animation notetrack name (or (Entire Animation))"""
        raise NotImplementedError()

    def setLoopEnd(self,loopEnd):
        """Sets the loop ending position
- 'loopEnd' is the number of seconds from the absolute beginning of the animation"""
        raise NotImplementedError()

    def setLoopStart(self,loopStart):
        """Sets the loop starting position
- 'loopStart' is the number of seconds from the absolute beginning of the animation"""
        raise NotImplementedError()

    def skipToBegin(self):
        """Skip to the beginning of the animation (don't play)"""
        raise NotImplementedError()

    def skipToEnd(self):
        """Skip to the end of the animation (don't play)"""
        raise NotImplementedError()

    def skipToLoopBegin(self):
        """Skip to the beginning of the animation loop (don't play)"""
        raise NotImplementedError()

    def skipToLoopEnd(self):
        """Skip to the end of the animation loop (don't play)"""
        raise NotImplementedError()

    def skipToTime(self,time):
        """Skip the animation to time (don't play)"""
        raise NotImplementedError()

    def speed(self,speed):
        """Sets the animation playback speed"""
        raise NotImplementedError()

    def stop(self):
        """Stops the animation"""
        raise NotImplementedError()

class ptAudioControl:
    """Accessor class to the Audio controls"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def canSetMicLevel(self):
        """Can the microphone level be set? Returns 1 if true otherwise returns 0."""
        raise NotImplementedError()

    def disable(self):
        """Disabled audio"""
        raise NotImplementedError()

    def enable(self):
        """Enables audio"""
        raise NotImplementedError()

    def enableVoiceChat(self,state):
        """Enables or disables voice chat."""
        raise NotImplementedError()

    def enableVoiceRecording(self,state):
        """Enables or disables voice recording."""
        raise NotImplementedError()

    def getAmbienceVolume(self):
        """Returns the volume (0.0 to 1.0) for the Ambiance."""
        raise NotImplementedError()

    def getCaptureDevice(self):
        """Gets the name for the capture device being used by the audio system."""
        raise NotImplementedError()

    def getCaptureDevices(self):
        """Gets the name of all available audio capture devices."""
        raise NotImplementedError()

    def getFriendlyDeviceName(self, devicename):
        """Returns the provided device name without any OpenAL prefixes applied."""
        raise NotImplementedError()

    def getGUIVolume(self):
        """Returns the volume (0.0 to 1.0) for the GUI dialogs."""
        raise NotImplementedError()

    def getHighestMode(self):
        """Gets the highest possible audio system mode"""
        raise NotImplementedError()

    def getMicLevel(self):
        """Returns the microphone recording level (0.0 to 1.0)."""
        raise NotImplementedError()

    def getMusicVolume(self):
        """Returns the volume (0.0 to 1.0) for the Music."""
        raise NotImplementedError()

    def getNPCVoiceVolume(self):
        """Returns the volume (0.0 to 1.0) for the NPC's voice."""
        raise NotImplementedError()

    def getNumAudioDevices(self):
        """Returns the number of available audio devices."""
        raise NotImplementedError()

    def getPlaybackDevice(self):
        """Gets the name for the device being used by the audio system."""
        raise NotImplementedError()

    def getPlaybackDevices(self):
        """Gets the names of all available audio playback devices."""
        raise NotImplementedError()

    def getPriorityCutoff(self):
        """Returns current sound priority"""
        raise NotImplementedError()

    def getSoundFXVolume(self):
        """Returns the volume (0.0 to 1.0) for the Sound FX."""
        raise NotImplementedError()

    def getVoiceVolume(self):
        """Returns the volume (0.0 to 1.0) for the Voices."""
        raise NotImplementedError()

    def hideIcons(self):
        """Hides (disables) the voice recording icons."""
        raise NotImplementedError()

    def isEAXSupported(self):
        """Returns true or false based on whether or not a the device specified supports EAX"""
        raise NotImplementedError()

    def isEnabled(self):
        """Is the audio enabled? Returns 1 if true otherwise returns 0."""
        raise NotImplementedError()

    def isMuted(self):
        """Are all sounds muted? Returns 1 if true otherwise returns 0."""
        raise NotImplementedError()

    def isUsingEAXAcceleration(self):
        """Is EAX sound acceleration enabled? Returns 1 if true otherwise returns 0."""
        raise NotImplementedError()

    def isVoiceRecordingEnabled(self):
        """Is voice recording enabled? Returns 1 if true otherwise returns 0."""
        raise NotImplementedError()

    def muteAll(self):
        """Mutes all sounds."""
        raise NotImplementedError()

    def pushToTalk(self,state):
        """Enables or disables 'push-to-talk'."""
        raise NotImplementedError()

    def setAmbienceVolume(self,volume):
        """Sets the Ambience volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        raise NotImplementedError()

    def setCaptureDevice(self, devicename):
        """Sets the audio capture device by name."""
        raise NotImplementedError()

    def setGUIVolume(self,volume):
        """Sets the GUI dialog volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        raise NotImplementedError()

    def setLoadOnDemand(self,state):
        """Enables or disables the load on demand for sounds."""
        raise NotImplementedError()

    def setMicLevel(self,level):
        """Sets the microphone recording level (0.0 to 1.0)."""
        raise NotImplementedError()

    def setMusicVolume(self,volume):
        """Sets the Music volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        raise NotImplementedError()

    def setNPCVoiceVolume(self,volume):
        """Sets the NPC's voice volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        raise NotImplementedError()

    def setPlaybackDevice(self,devicename,restart):
        """Sets audio system output device by name, and optionally restarts it"""
        raise NotImplementedError()

    def setPriorityCutoff(self,priority):
        """Sets the sound priority"""
        raise NotImplementedError()

    def setSoundFXVolume(self,volume):
        """Sets the SoundFX volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        raise NotImplementedError()

    def setTwoStageLOD(self,state):
        """Enables or disables two-stage LOD, where sounds can be loaded into RAM but not into sound buffers.
...Less of a performance hit, harder on memory."""
        raise NotImplementedError()

    def setVoiceVolume(self,volume):
        """Sets the Voice volume (0.0 to 1.0) for the game.
This only sets the volume for this game session."""
        raise NotImplementedError()

    def showIcons(self):
        """Shows (enables) the voice recording icons."""
        raise NotImplementedError()

    def squelchLevel(self,level):
        """Sets the squelch level."""
        raise NotImplementedError()

    def unmuteAll(self):
        """Unmutes all sounds."""
        raise NotImplementedError()

    def useEAXAcceleration(self,state):
        """Enables or disables EAX sound acceleration (requires hardware acceleration)."""
        raise NotImplementedError()

class ptAvatar:
    """Plasma avatar class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addWardrobeClothingItem(self,clothing_name,tint1,tint2):
        """To add a clothing item to the avatar's wardrobe (closet)"""
        raise NotImplementedError()

    def enterSubWorld(self,sceneobject):
        """Places the avatar into the subworld of the ptSceneObject specified"""
        raise NotImplementedError()

    def exitSubWorld(self):
        """Exits the avatar from the subWorld where it was"""
        raise NotImplementedError()

    def getAllWithSameMesh(self,clothing_name):
        """Returns a lilst of all clothing items that use the same mesh as the specified one"""
        raise NotImplementedError()

    def getAvatarClothingGroup(self):
        """Returns what clothing group the avatar belongs to.
It is also a means to determine if avatar is male or female"""
        raise NotImplementedError()

    def getAvatarClothingList(self):
        """Returns a list of clothes that the avatar is currently wearing."""
        raise NotImplementedError()

    def getClosetClothingList(self,clothing_type):
        """Returns a list of clothes for the avatar that are in specified clothing group."""
        raise NotImplementedError()

    def getCurrentMode(self):
        """Returns current brain mode for avatar"""
        raise NotImplementedError()

    def getEntireClothingList(self,clothing_type):
        """Gets the entire list of clothing available. 'clothing_type' not used
NOTE: should use getClosetClothingList"""
        raise NotImplementedError()

    def getMatchingClothingItem(self,clothingName):
        """Finds the matching clothing item that goes with 'clothingName'
Used to find matching left and right gloves and shoes."""
        raise NotImplementedError()

    def getMorph(self,clothing_name,layer):
        """Get the current morph value"""
        raise NotImplementedError()

    def getSkinBlend(self,layer):
        """Get the current skin blend value"""
        raise NotImplementedError()

    def getTintClothingItem(self,clothing_name,layer=1):
        """Returns a ptColor of a particular item of clothing that the avatar is wearing.
The color will be a ptColor object."""
        raise NotImplementedError()

    def getTintSkin(self):
        """Returns a ptColor of the current skin tint for the avatar"""
        raise NotImplementedError()

    def getUniqueMeshList(self,clothing_type):
        """Returns a list of unique clothing items of the desired type (different meshes)"""
        raise NotImplementedError()

    def getWardrobeClothingList(self):
        """Return a list of items that are in the avatars closet"""
        raise NotImplementedError()

    def gotoStage(self,behaviorKey,stage,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce):
        """Tells a multistage behavior to go to a particular stage"""
        raise NotImplementedError()

    def loadClothingFromFile(self,filename):
        """Load avatar clothing from a file"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

    def nextStage(self,behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce):
        """Tells a multistage behavior to go to the next stage (Why does Matt like so many parameters?)"""
        raise NotImplementedError()

    def oneShot(self,seekKey,duration,usePhysicsFlag,animationName,drivableFlag,reversibleFlag):
        """Plays a one-shot animation on the avatar"""
        raise NotImplementedError()

    def playSimpleAnimation(self,animName):
        """Play simple animation on avatar"""
        raise NotImplementedError()

    def previousStage(self,behaviorKey,transitionTime,setTimeFlag,newTime,SetDirectionFlag,isForward,netForce):
        """Tells a multistage behavior to go to the previous stage"""
        raise NotImplementedError()

    def registerForBehaviorNotify(self,selfKey):
        """This will register for behavior notifies from the avatar"""
        raise NotImplementedError()

    def removeClothingItem(self,clothing_name,update=1):
        """Tells the avatar to remove a particular item of clothing."""
        raise NotImplementedError()

    def runBehavior(self,behaviorKey,netForceFlag):
        """Runs a behavior on the avatar. Can be a single or multi-stage behavior."""
        raise NotImplementedError()

    def runBehaviorSetNotify(self,behaviorKey,replyKey,netForceFlag):
        """Same as runBehavior, except send notifications to specified keyed object"""
        raise NotImplementedError()

    def runCoopAnim(self,targetKey,activeAvatarAnim,targetAvatarAnim,range=6,dist=3,move=1):
        """Seek near another avatar and run animations on both."""
        raise NotImplementedError()

    def saveClothing(self):
        """Saves the current clothing options (including morphs) to the vault"""
        raise NotImplementedError()

    def saveClothingToFile(self,filename):
        """Save avatar clothing to a file"""
        raise NotImplementedError()

    def setMorph(self,clothing_name,layer,value):
        """Set the morph value (clipped between -1 and 1)"""
        raise NotImplementedError()

    def setReplyKey(self,key):
        """Sets the sender's key"""
        raise NotImplementedError()

    def setSkinBlend(self,layer,value):
        """Set the skin blend (value between 0 and 1)"""
        raise NotImplementedError()

    def tintClothingItem(self,clothing_name,tint,update=1):
        """Tells the avatar to tint(color) a particular item of clothing that they are already wearing.
'tint' is a ptColor object"""
        raise NotImplementedError()

    def tintClothingItemLayer(self,clothing_name,tint,layer,update=1):
        """Tells the avatar to tint(color) a particular layer of a particular item of clothing."""
        raise NotImplementedError()

    def tintSkin(self,tint,update=1):
        """Tints all of the skin on the avatar, with the ptColor tint"""
        raise NotImplementedError()

    def unRegisterForBehaviorNotify(self,selfKey):
        """This will unregister behavior notifications"""
        raise NotImplementedError()

    def wearClothingItem(self,clothing_name,update=1):
        """Tells the avatar to wear a particular item of clothing.
And optionally hold update until later (for applying tinting before wearing)."""
        raise NotImplementedError()

class ptBook:
    """Creates a new book"""
    def __init__(self,esHTMLSource,coverImage=None,callbackKey=None,guiName=''):
        """None"""
        raise NotImplementedError()

    def allowPageTurning(self,allow):
        """Turns on and off the ability to flip the pages in a book"""
        raise NotImplementedError()

    def close(self):
        """Closes the book"""
        raise NotImplementedError()

    def closeAndHide(self):
        """Closes the book and hides it once it finishes animating"""
        raise NotImplementedError()

    def getCurrentPage(self):
        """Returns the currently shown page"""
        raise NotImplementedError()

    def getEditableText(self):
        """Returns the editable text currently contained in the book."""
        raise NotImplementedError()

    def getMovie(self,index):
        """Grabs a ptAnimation object representing the movie indexed by index. The index is the index of the movie in the source code"""
        raise NotImplementedError()

    def goToPage(self,page):
        """Flips the book to the specified page"""
        raise NotImplementedError()

    def hide(self):
        """Hides the book"""
        raise NotImplementedError()

    def nextPage(self):
        """Flips the book to the next page"""
        raise NotImplementedError()

    def open(self,startingPage):
        """Opens the book to the specified page"""
        raise NotImplementedError()

    def previousPage(self):
        """Flips the book to the previous page"""
        raise NotImplementedError()

    def setEditable(self,editable):
        """Turn book editing on or off. If the book GUI does not support editing, nothing will happen"""
        raise NotImplementedError()

    def setEditableText(self,text):
        """Sets the book's editable text."""
        raise NotImplementedError()

    def setGUI(self,guiName):
        """Sets the gui to be used by the book, if the requested gui is not loaded, it will use the default
Do not call while the book is open!"""
        raise NotImplementedError()

    def setPageMargin(self,margin):
        """Sets the text margin for the book"""
        raise NotImplementedError()

    def setSize(self,width,height):
        """Sets the size of the book (width and height are floats from 0 to 1)"""
        raise NotImplementedError()

    def show(self,startOpened):
        """Shows the book closed, or open if the the startOpened flag is true"""
        raise NotImplementedError()

class ptCCRAge:
    """CCR only: CCR age info struct"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

class ptCCRMgr:
    """CCR only: accessor class to the CCR manager"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def banLinking(self,pid, banFlag):
        """Set the ban linking flag for a player"""
        raise NotImplementedError()

    def beginCommunication(self,pid, message):
        """Begin a CCR communication with a player"""
        raise NotImplementedError()

    def clippingOff(self):
        """Disables clipping for this player"""
        raise NotImplementedError()

    def clippingOn(self):
        """Enables clipping for this player"""
        raise NotImplementedError()

    def endCommunication(self,pid):
        """End CCR communications with a player"""
        raise NotImplementedError()

    def getClipping(self):
        """Is clipping on for this player? Returns 1 if true otherwise returns 0"""
        raise NotImplementedError()

    def getErrorString(self,errorNumber):
        """Returns the error string that corresponds to 'errorNumber'"""
        raise NotImplementedError()

    def getLevel(self):
        """Returns the current CCR level for this player"""
        raise NotImplementedError()

    def getPlayerInfo(self,player, cbObject, cbContext):
        """Finds a player that matches 'player' (which is an id or name)."""
        raise NotImplementedError()

    def linkPlayerHere(self,pid):
        """Links player to where I am"""
        raise NotImplementedError()

    def linkPlayerToAge(self,ageInfoStruct,pid):
        """Links player to a specified age"""
        raise NotImplementedError()

    def linkToAge(self,age,pid):
        """Links to player's version of age"""
        raise NotImplementedError()

    def linkToMyNeighborhoodAge(self,pid):
        """Links this player to their neighborhood"""
        raise NotImplementedError()

    def linkToMyPersonalAge(self,pid):
        """Links this player to their personal Age."""
        raise NotImplementedError()

    def linkToPlayersAge(self,pid):
        """Link to where the player is"""
        raise NotImplementedError()

    def logMessage(self,message):
        """Logs 'message' somewhere...?"""
        raise NotImplementedError()

    def makeInvisible(self,level):
        """Makes this player invisible to 'level'"""
        raise NotImplementedError()

    def sendCommunication(self,pid, message):
        """Send a CCR communication to a player"""
        raise NotImplementedError()

    def setAwayStatus(self,awayFlag):
        """Set the away flag for CCRs"""
        raise NotImplementedError()

    def silencePlayer(self,pid, silenceFlag):
        """Set the silence player flag for a player"""
        raise NotImplementedError()

    def systemMessage(self):
        """Params message
Send a system wide CCR message"""
        raise NotImplementedError()

    def toggleClipping(self):
        """Toggles clipping for this player"""
        raise NotImplementedError()

    def warpPlayerHere(self,pid):
        """warp the player to here"""
        raise NotImplementedError()

    def warpToPlayer(self,pid):
        """warp to where the player is"""
        raise NotImplementedError()

class ptCCRPlayerInfo:
    """CCR only: CCR player info struct"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

class ptCamera:
    """Plasma camera class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def controlKey(self,controlKey,activateFlag):
        """Send a control key to the camera as if it was hit by the user.
This is for sending things like pan-up, pan-down, zoom-in, etc."""
        raise NotImplementedError()

    def disableFirstPersonOverride(self):
        """Does _not_ allow the user to override the camera to go to first person camera."""
        raise NotImplementedError()

    def enableFirstPersonOverride(self):
        """Allows the user to override the camera and go to a first person camera."""
        raise NotImplementedError()

    def getFOV(self):
        """Returns the current camera's FOV(h)"""
        raise NotImplementedError()

    def isSmootherCam(self):
        """Returns true if we are using the faster cams thing"""
        raise NotImplementedError()

    def isStayInFirstPerson(self):
        """Are we staying in first person?"""
        raise NotImplementedError()

    def isWalkAndVerticalPan(self):
        """Returns true if we are walking and chewing gum"""
        raise NotImplementedError()

    def restore(self,cameraKey):
        """Restores camera to saved one"""
        raise NotImplementedError()

    def save(self,cameraKey):
        """Saves the current camera and sets the camera to cameraKey"""
        raise NotImplementedError()

    def set(self,cameraKey,time,save):
        """DO NOT USE"""
        raise NotImplementedError()

    def setFOV(self,fov, time):
        """Sets the current cameras FOV (based on h)"""
        raise NotImplementedError()

    def setSmootherCam(self,state):
        """Set the faster cams thing"""
        raise NotImplementedError()

    def setStayInFirstPerson(self,state):
        """Set Stay In First Person Always"""
        raise NotImplementedError()

    def setWalkAndVerticalPan(self,state):
        """Set Walk and chew gum"""
        raise NotImplementedError()

    def undoFirstPerson(self):
        """If the user has overridden the camera to be in first person, this will take them out of first person.
If the user didn't override the camera, then this will do nothing."""
        raise NotImplementedError()

class ptCluster:
    """Creates a new ptCluster"""
    def __init__(self,ey):
        """None"""
        raise NotImplementedError()

    def setVisible(self,isible):
        """Shows or hides the cluster object"""
        raise NotImplementedError()

class ptColor:
    """Plasma color class"""
    def __init__(self,red=0, green=0, blue=0, alpha=0):
        """None"""
        raise NotImplementedError()

    def black(self):
        """Sets the color to be black
Example: black = ptColor().black()"""
        raise NotImplementedError()

    def blue(self):
        """Sets the color to be blue
Example: blue = ptColor().blue()"""
        raise NotImplementedError()

    def brown(self):
        """Sets the color to be brown
Example: brown = ptColor().brown()"""
        raise NotImplementedError()

    def cyan(self):
        """Sets the color to be cyan
Example: cyan = ptColor.cyan()"""
        raise NotImplementedError()

    def darkbrown(self):
        """Sets the color to be darkbrown
Example: darkbrown = ptColor().darkbrown()"""
        raise NotImplementedError()

    def darkgreen(self):
        """Sets the color to be darkgreen
Example: darkgreen = ptColor().darkgreen()"""
        raise NotImplementedError()

    def darkpurple(self):
        """Sets the color to be darkpurple
Example: darkpurple = ptColor().darkpurple()"""
        raise NotImplementedError()

    def getAlpha(self):
        """Get the alpha blend component of the color"""
        raise NotImplementedError()

    def getBlue(self):
        """Get the blue component of the color"""
        raise NotImplementedError()

    def getGreen(self):
        """Get the green component of the color"""
        raise NotImplementedError()

    def getRed(self):
        """Get the red component of the color"""
        raise NotImplementedError()

    def gray(self):
        """Sets the color to be gray
Example: gray = ptColor().gray()"""
        raise NotImplementedError()

    def green(self):
        """Sets the color to be green
Example: green = ptColor().green()"""
        raise NotImplementedError()

    def magenta(self):
        """Sets the color to be magenta
Example: magenta = ptColor().magenta()"""
        raise NotImplementedError()

    def maroon(self):
        """Sets the color to be maroon
Example: maroon = ptColor().maroon()"""
        raise NotImplementedError()

    def navyblue(self):
        """Sets the color to be navyblue
Example: navyblue = ptColor().navyblue()"""
        raise NotImplementedError()

    def orange(self):
        """Sets the color to be orange
Example: orange = ptColor().orange()"""
        raise NotImplementedError()

    def pink(self):
        """Sets the color to be pink
Example: pink = ptColor().pink()"""
        raise NotImplementedError()

    def red(self):
        """Sets the color to be red
Example: red = ptColor().red()"""
        raise NotImplementedError()

    def setAlpha(self,alpha):
        """Set the alpha blend component of the color. 0.0 to 1.0"""
        raise NotImplementedError()

    def setBlue(self,blue):
        """Set the blue component of the color. 0.0 to 1.0"""
        raise NotImplementedError()

    def setGreen(self,green):
        """Set the green component of the color. 0.0 to 1.0"""
        raise NotImplementedError()

    def setRed(self,red):
        """Set the red component of the color. 0.0 to 1.0"""
        raise NotImplementedError()

    def slateblue(self):
        """Sets the color to be slateblue
Example: slateblue = ptColor().slateblue()"""
        raise NotImplementedError()

    def steelblue(self):
        """Sets the color to be steelblue
Example: steelblue = ptColor().steelblue()"""
        raise NotImplementedError()

    def tan(self):
        """Sets the color to be tan
Example: tan = ptColor().tan()"""
        raise NotImplementedError()

    def white(self):
        """Sets the color to be white
Example: white = ptColor().white()"""
        raise NotImplementedError()

    def yellow(self):
        """Sets the color to be yellow
Example: yellow = ptColor().yellow()"""
        raise NotImplementedError()

class ptCritterBrain:
    """Object to manipulate critter brains"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addBehavior(self,animName, behaviorName, loop = 1, randomStartPos = 1, fadeInLen = 2.0, fadeOutLen = 2.0):
        """Adds a new animation to the brain as a behavior with the specified name and parameters. If multiple animations are assigned to the same behavior, they will be randomly picked from when started."""
        raise NotImplementedError()

    def addReceiver(self,key):
        """Tells the brain that the specified key wants AI messages"""
        raise NotImplementedError()

    def animationName(self,behavior):
        """Returns the animation name associated with the specified integral behavior."""
        raise NotImplementedError()

    def atGoal(self):
        """Are we currently are our final destination?"""
        raise NotImplementedError()

    def avoidingAvatars(self):
        """Are we currently avoiding avatars while pathfinding?"""
        raise NotImplementedError()

    def behaviorName(self,behavior):
        """Returns the behavior name associated with the specified integral behavior."""
        raise NotImplementedError()

    def canHearAvatar(self,avatarID):
        """Returns whether this brain can hear the avatar with the specified id."""
        raise NotImplementedError()

    def canSeeAvatar(self,avatarID):
        """Returns whether this brain can see the avatar with the specified id."""
        raise NotImplementedError()

    def curBehavior(self):
        """Returns the current integral behavior the brain is running."""
        raise NotImplementedError()

    def currentGoal(self):
        """Returns the current ptPoint that the brain is running towards."""
        raise NotImplementedError()

    def getHearingDistance(self):
        """Returns how far away the brain can hear."""
        raise NotImplementedError()

    def getSceneObject(self):
        """Returns the ptSceneObject this brain controls."""
        raise NotImplementedError()

    def getSightCone(self):
        """Returns the width of the brain's field of view in radians."""
        raise NotImplementedError()

    def getSightDistance(self):
        """Returns how far the brain can see."""
        raise NotImplementedError()

    def getStopDistance(self):
        """Returns how far away from the goal we could be and still be considered there."""
        raise NotImplementedError()

    def goToGoal(self,newGoal, avoidingAvatars = 0):
        """Tells the brain to start running towards the specified location, avoiding avatars it can see or hear if told to."""
        raise NotImplementedError()

    def idleBehaviorName(self):
        """Returns the name of the brain's idle behavior."""
        raise NotImplementedError()

    def nextBehavior(self):
        """Returns the behavior the brain will be switching to next frame. (-1 if no change)"""
        raise NotImplementedError()

    def playersICanHear(self):
        """Returns a list of player ids which this brain can hear."""
        raise NotImplementedError()

    def playersICanSee(self):
        """Returns a list of player ids which this brain can see."""
        raise NotImplementedError()

    def removeReceiver(self,key):
        """Tells the brain that the specified key no longer wants AI messages"""
        raise NotImplementedError()

    def runBehaviorName(self):
        """Returns the name of the brain's run behavior."""
        raise NotImplementedError()

    def runningBehavior(self,behaviorName):
        """Returns true if the named behavior is running."""
        raise NotImplementedError()

    def setHearingDistance(self,dist):
        """Set how far away the brain can hear (360 degree field of hearing)."""
        raise NotImplementedError()

    def setSightCone(self,radians):
        """Set how wide the brain's field of view is in radians. Note that it is the total angle of the cone, half on one side of the brain's line of sight, half on the other."""
        raise NotImplementedError()

    def setSightDistance(self,dist):
        """Set how far away the brain can see."""
        raise NotImplementedError()

    def setStopDistance(self,dist):
        """Set how far away from the goal we should be when we are considered there and stop running."""
        raise NotImplementedError()

    def startBehavior(self,behaviorName, fade = 1):
        """Starts playing the named behavior. If fade is true, it will fade out the previous behavior and fade in the new one. If false, they will immediately switch."""
        raise NotImplementedError()

    def vectorToPlayer(self,avatarID):
        """Returns the vector between us and the specified player."""
        raise NotImplementedError()

class ptDniCoordinates:
    """Constructor for a D'Ni coordinate"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def fromPoint(self,pt):
        """Update these coordinates with the specified ptPoint3"""
        raise NotImplementedError()

    def getHSpans(self):
        """Returns the HSpans component of the coordinate"""
        raise NotImplementedError()

    def getTorans(self):
        """Returns the Torans component of the coordinate"""
        raise NotImplementedError()

    def getVSpans(self):
        """Returns the VSpans component of the coordinate"""
        raise NotImplementedError()

    def update(self):
        """Update these coordinates with the players current position"""
        raise NotImplementedError()

class ptDniInfoSource:
    """DO NOT USE"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getAgeCoords(self):
        """Current coords of the player in current age as a ptDniCoordinates"""
        raise NotImplementedError()

    def getAgeGuid(self):
        """Unique identifier for this age instance"""
        raise NotImplementedError()

    def getAgeName(self):
        """Name of current age"""
        raise NotImplementedError()

    def getAgeTime(self):
        """Current time in current age (tbd)"""
        raise NotImplementedError()

class ptDraw:
    """Plasma Draw class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables the draw on the sceneobject attached
In other words, makes it invisible"""
        raise NotImplementedError()

    def enable(self,state=1):
        """Sets the draw enable for the sceneobject attached"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

class ptDynamicMap:
    """Creates a ptDynamicMap object"""
    def __init__(self,key=None):
        """None"""
        raise NotImplementedError()

    def addKey(self,key):
        """Add a receiver... in other words a DynamicMap"""
        raise NotImplementedError()

    def calcTextExtents(self,text):
        """Calculates the extent of the specified text, returns it as a (width, height) tuple"""
        raise NotImplementedError()

    def clearKeys(self):
        """Clears the receiver list"""
        raise NotImplementedError()

    def clearToColor(self,color):
        """Clear the DynamicMap to the specified color
- 'color' is a ptColor object"""
        raise NotImplementedError()

    def drawImage(self,x,y,image,respectAlphaFlag):
        """Draws a ptImage object on the dynamicTextmap starting at the location x,y"""
        raise NotImplementedError()

    def drawImageClipped(self,x,y,image,cx,cy,cw,ch,respectAlphaFlag):
        """Draws a ptImage object clipped to cx,cy with cw(width),ch(height)"""
        raise NotImplementedError()

    def drawText(self,x,y,text):
        """Draw text at a specified location
- x,y is the point to start drawing the text
- 'text' is a string of the text to be drawn"""
        raise NotImplementedError()

    def fillRect(self,left,top,right,bottom,color):
        """Fill in the specified rectangle with a color
- left,top,right,bottom define the rectangle
- 'color' is a ptColor object"""
        raise NotImplementedError()

    def flush(self):
        """Flush all the commands that were issued since the last flush()"""
        raise NotImplementedError()

    def frameRect(self,left,top,right,bottom,color):
        """Frame a rectangle with a specified color
- left,top,right,bottom define the rectangle
- 'color' is a ptColor object"""
        raise NotImplementedError()

    def getHeight(self):
        """Returns the height of the dynamicTextmap"""
        raise NotImplementedError()

    def getImage(self):
        """Returns a pyImage associated with the dynamicTextmap"""
        raise NotImplementedError()

    def getWidth(self):
        """Returns the width of the dynamicTextmap"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object
This only applies when NetPropagate is set to true"""
        raise NotImplementedError()

    def netPropagate(self,propagateFlag):
        """Specify whether this object needs to use messages that are sent on the network
- The default is for this to be false."""
        raise NotImplementedError()

    def purgeImage(self):
        """Purge the DynamicTextMap images"""
        raise NotImplementedError()

    def sender(self,sender):
        """Set the sender of the message being sent to the DynamicMap"""
        raise NotImplementedError()

    def setClipping(self,clipLeft,clipTop,clipRight,clipBottom):
        """Sets the clipping rectangle
- All drawtext will be clipped to this until the
unsetClipping() is called"""
        raise NotImplementedError()

    def setFont(self,facename,size):
        """Set the font of the text to be written
- 'facename' is a string with the name of the font
- 'size' is the point size of the font to use"""
        raise NotImplementedError()

    def setJustify(self,justify):
        """Sets the justification of the text. (justify is a PtJustify)"""
        raise NotImplementedError()

    def setLineSpacing(self,spacing):
        """Sets the line spacing (in pixels)"""
        raise NotImplementedError()

    def setTextColor(self,color, blockRGB=0):
        """Set the color of the text to be written
- 'color' is a ptColor object
- 'blockRGB' must be true if you're trying to render onto a transparent or semi-transparent color"""
        raise NotImplementedError()

    def setWrapping(self,wrapWidth,wrapHeight):
        """Set where text will be wrapped horizontally and vertically
- All drawtext commands will be wrapped until the
unsetWrapping() is called"""
        raise NotImplementedError()

    def unsetClipping(self):
        """Stop the clipping of text"""
        raise NotImplementedError()

    def unsetWrapping(self):
        """Stop text wrapping"""
        raise NotImplementedError()

class ptGameScore:
    """Plasma Game Score"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addPoints(self, points, key=None):
        """Adds points to the score"""
        raise NotImplementedError()

    @staticmethod
    def createAgeScore(scoreName, type, points=0, key=None):
        """Creates a new score associated with this age"""
        raise NotImplementedError()

    @staticmethod
    def createGlobalScore(scoreName, type, points=0, key=None):
        """Creates a new global score"""
        raise NotImplementedError()

    @staticmethod
    def createPlayerScore(scoreName, type, points=0, key=None):
        """Creates a new score associated with this player"""
        raise NotImplementedError()

    @staticmethod
    def createScore(ownerID, scoreName, type, points=0, key=None):
        """Creates a new score for an arbitrary owner"""
        raise NotImplementedError()

    @staticmethod
    def findAgeScores(scoreName, key):
        """Finds matching scores for this age"""
        raise NotImplementedError()

    @staticmethod
    def findAgeHighScores(name, maxScores, key):
        """Finds the highest matching scores for the current age's owners"""
        raise NotImplementedError()

    @staticmethod
    def findGlobalScores(scoreName, key):
        """Finds matching global scores"""
        raise NotImplementedError()

    @staticmethod
    def findGlobalHighScores(name, maxScores, key):
        """Finds the highest matching scores"""
        raise NotImplementedError()

    @staticmethod
    def findPlayerScores(scoreName, key):
        """Finds matching player scores"""
        raise NotImplementedError()

    @staticmethod
    def findScores(ownerID, scoreName, key):
        """Finds matching scores for an arbitrary owner"""
        raise NotImplementedError()

    def getGameType(self):
        """Returns the score game type."""
        raise NotImplementedError()

    def getName(self):
        """Returns the score game name."""
        raise NotImplementedError()

    def getOwnerID(self):
        """Returns the score game owner."""
        raise NotImplementedError()

    def getPoints(self):
        """Returns the number of points in this score"""
        raise NotImplementedError()

    def remove(self):
        """Removes this score from the server"""
        raise NotImplementedError()

    def setPoints(self):
        """Sets the number of points in the score
        Don't use to add/remove points, use only to reset values!"""
        raise NotImplementedError()

    def transferPoints(self, dest, points=0, key=None):
        """Transfers points from this score to another"""
        raise NotImplementedError()

class ptGameScoreMsg:
    """Game Score operation callback message"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

class ptGameScoreListMsg(ptGameScoreMsg):
    """Game Score message for scores found on the server"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getName(self):
        """Returns the template score name"""
        raise NotImplementedError()

    def getOwnerID(self):
        """Returns the template score ownerID"""
        raise NotImplementedError()

    def getScores(self):
        """Returns a list of scores found by the server"""
        raise NotImplementedError()

class ptGameScoreTransferMsg(ptGameScoreMsg):
    """Game Score message indicating a score point transfer"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getDestination(self):
        """Returns the score points were transferred to"""
        raise NotImplementedError()

    def getSource(self):
        """Returns the score points were transferred from"""
        raise NotImplementedError()

class ptGameScoreUpdateMsg(ptGameScoreMsg):
    """Game Score message for a score update operation"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getScore(self):
        """Returns the updated game score"""
        raise NotImplementedError()

class ptGUIControl:
    """Base class for all GUI controls"""
    def __init__(self,controlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlButton(ptGUIControl):
    """Plasma GUI Control Button class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getNotifyType(self):
        """Returns this button's notify type. See PtButtonNotifyTypes"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isButtonDown(self):
        """Is the button down? Returns 1 for true otherwise returns 0"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setNotifyType(self,kind):
        """Sets this button's notify type. See PtButtonNotifyTypes"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlCheckBox(ptGUIControl):
    """Plasma GUI Control Checkbox class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isChecked(self):
        """Is this checkbox checked? Returns 1 for true otherwise returns 0"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setChecked(self,checkedState):
        """Sets this checkbox to the 'checkedState'"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlClickMap(ptGUIControl):
    """Plasma GUI control Click Map"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getLastMouseDragPoint(self):
        """Returns the last point the mouse was dragged to"""
        raise NotImplementedError()

    def getLastMousePoint(self):
        """Returns the last point the mouse was at"""
        raise NotImplementedError()

    def getLastMouseUpPoint(self):
        """Returns the last point the mouse was released at"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlDragBar(ptGUIControl):
    """Plasma GUI Control DragBar class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def anchor(self):
        """Don't allow this dragbar object to be moved by the user.
Drop anchor!"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isAnchored(self):
        """Is this dragbar control anchored? Returns 1 if true otherwise returns 0"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

    def unanchor(self):
        """Allow the user to drag this control around the screen.
Raise anchor."""
        raise NotImplementedError()

class ptGUIControlDraggable(ptGUIControl):
    """Plasma GUI control for something draggable"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getLastMousePoint(self):
        """Returns the last point we were dragged to"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def stopDragging(self,cancelFlag):
        """UNKNOWN"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlDynamicText(ptGUIControl):
    """Plasma GUI Control DynamicText class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getMap(self,index):
        """Returns a specific ptDynamicText attached to this contol
If there is no map at 'index' then a KeyError exception will be raised"""
        raise NotImplementedError()

    def getNumMaps(self):
        """Returns the number of ptDynamicText maps attached"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlEditBox(ptGUIControl):
    """Plasma GUI Control Editbox class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def clearString(self):
        """Clears the editbox."""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def end(self):
        """Sets the cursor in the editbox to the after the last character."""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getLastKeyCaptured(self):
        """Gets the last capture key"""
        raise NotImplementedError()

    def getLastModifiersCaptured(self):
        """Gets the last modifiers flags captured"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getString(self):
        """Returns the sting that the user typed in."""
        raise NotImplementedError()

    def getStringW(self):
        """Unicode version of getString."""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def home(self):
        """Sets the cursor in the editbox to before the first character."""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setChatMode(self,state):
        """Set the Chat mode on this control"""
        raise NotImplementedError()

    def setColor(self,foreColor,backColor):
        """Sets the fore and back color of the editbox."""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setLastKeyCapture(self,key, modifiers):
        """Set last key captured"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setSelectionColor(self,foreColor,backColor):
        """Sets the selection color of the editbox."""
        raise NotImplementedError()

    def setSpecialCaptureKeyMode(self,state):
        """Set the Capture mode on this control"""
        raise NotImplementedError()

    def setString(self,text):
        """Pre-sets the editbox to a atring."""
        raise NotImplementedError()

    def setStringSize(self,size):
        """Sets the maximum size of the string that can be inputted by the user."""
        raise NotImplementedError()

    def setStringW(self,text):
        """Unicode version of setString."""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

    def wasEscaped(self):
        """If the editbox was escaped then return 1 else return 0"""
        raise NotImplementedError()

class ptGUIControlValue(ptGUIControl):
    """Plasma GUI Control Value class  - knobs, spinners"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getMax(self):
        """Returns the maximum of the control."""
        raise NotImplementedError()

    def getMin(self):
        """Returns the minimum of the control."""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getStep(self):
        """Returns the step increment of the control."""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def getValue(self):
        """Returns the current value of the control."""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setStep(self,step):
        """Sets the step increment of the control."""
        raise NotImplementedError()

    def setValue(self,value):
        """Sets the current value of the control."""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlKnob(ptGUIControlValue):
    """Plasma GUI control for knob"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getMax(self):
        """Returns the maximum of the control."""
        raise NotImplementedError()

    def getMin(self):
        """Returns the minimum of the control."""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getStep(self):
        """Returns the step increment of the control."""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def getValue(self):
        """Returns the current value of the control."""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setStep(self,step):
        """Sets the step increment of the control."""
        raise NotImplementedError()

    def setValue(self,value):
        """Sets the current value of the control."""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlListBox(ptGUIControl):
    """Plasma GUI Control List Box class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def add2StringsWithColors(self,text1,color1,text2,color2,respectAlpha):
        """Doesn't work right - DONT USE"""
        raise NotImplementedError()

    def addBranch(self,name,initiallyOpen):
        """UNKNOWN"""
        raise NotImplementedError()

    def addBranchW(self,name,initiallyOpen):
        """Unicode version of addBranch"""
        raise NotImplementedError()

    def addImage(self,image,respectAlphaFlag):
        """Appends an image item to the listbox"""
        raise NotImplementedError()

    def addImageAndSwatchesInBox(self,image,x,y,width,height,respectAlpha,primary,secondary):
        """Add the image and color swatches to the list"""
        raise NotImplementedError()

    def addImageInBox(self,image,x,y,width,height,respectAlpha):
        """Appends an image item to the listbox, centering within the box dimension."""
        raise NotImplementedError()

    def addSelection(self,item):
        """Adds item to selection list"""
        raise NotImplementedError()

    def addString(self,text):
        """Appends a list item 'text' to the listbox."""
        raise NotImplementedError()

    def addStringInBox(self,text,min_width,min_height):
        """Adds a text list item that has a minimum width and height"""
        raise NotImplementedError()

    def addStringW(self,text):
        """Unicode version of addString."""
        raise NotImplementedError()

    def addStringWithColor(self,text,color,inheritAlpha):
        """Adds a colored string to the list box"""
        raise NotImplementedError()

    def addStringWithColorWithSize(self,text,color,inheritAlpha,fontsize):
        """Adds a text list item with a color and different font size"""
        raise NotImplementedError()

    def allowNoSelect(self):
        """Allows the listbox to have no selection"""
        raise NotImplementedError()

    def clearAllElements(self):
        """Removes all the items from the listbox, making it empty."""
        raise NotImplementedError()

    def clickable(self):
        """Sets this listbox to be clickable by the user."""
        raise NotImplementedError()

    def closeBranch(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def disallowNoSelect(self):
        """The listbox must always have a selection"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def findString(self,text):
        """Finds and returns the index of the item that matches 'text' in the listbox."""
        raise NotImplementedError()

    def findStringW(self,text):
        """Unicode version of findString."""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getBranchList(self):
        """get a list of branches in this list (index,isShowingChildren)"""
        raise NotImplementedError()

    def getElement(self,index):
        """Get the string of the item at 'index' in the listbox."""
        raise NotImplementedError()

    def getElementW(self,index):
        """Unicode version of getElement."""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getNumElements(self):
        """Return the number of items in the listbox."""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getScrollPos(self):
        """Returns the current scroll position in the listbox."""
        raise NotImplementedError()

    def getScrollRange(self):
        """Returns the max scroll position"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getSelection(self):
        """Returns the currently selected list item in the listbox."""
        raise NotImplementedError()

    def getSelectionList(self):
        """Returns the current selection list"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def lock(self):
        """Locks the updates to a listbox, so a number of operations can be performed
NOTE: an unlock() call must be made before the next lock() can be."""
        raise NotImplementedError()

    def refresh(self):
        """Refresh the display of the listbox (after updating contents)."""
        raise NotImplementedError()

    def removeElement(self,index):
        """Removes element at 'index' in the listbox."""
        raise NotImplementedError()

    def removeSelection(self,item):
        """Removes item from selection list"""
        raise NotImplementedError()

    def scrollToBegin(self):
        """Scrolls the listbox to the beginning of the list"""
        raise NotImplementedError()

    def scrollToEnd(self):
        """Scrolls the listbox to the end of the list"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setElement(self,index,text):
        """Set a particular item in the listbox to a string."""
        raise NotImplementedError()

    def setElementW(self,index,text):
        """Unicode version of setElement."""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setGlobalSwatchEdgeOffset(self,offset):
        """Sets the edge offset of the color swatches"""
        raise NotImplementedError()

    def setGlobalSwatchSize(self,size):
        """Sets the size of the color swatches"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setScrollPos(self,pos):
        """Sets the scroll position of the listbox to 'pos'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setSelection(self,selectionIndex):
        """Sets the current selection in the listbox."""
        raise NotImplementedError()

    def setStringJustify(self,index,justify):
        """Sets the text justification"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

    def unclickable(self):
        """Makes this listbox not clickable by the user.
Useful when just displaying a list that is not really selectable."""
        raise NotImplementedError()

    def unlock(self):
        """Unlocks updates to a listbox and does any saved up changes"""
        raise NotImplementedError()

class ptGUIControlMultiLineEdit(ptGUIControl):
    """Plasma GUI Control Multi-line edit class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def beginUpdate(self):
        """Signifies that the control will be updated heavily starting now, so suppress all redraws"""

    def clearBuffer(self):
        """Clears all text from the multi-line edit control."""
        raise NotImplementedError()

    def clickable(self):
        """Sets this listbox to be clickable by the user."""
        raise NotImplementedError()

    def deleteChar(self):
        """Deletes a character at the current cursor position."""
        raise NotImplementedError()

    def deleteLinesFromTop(self,numLines):
        """Deletes the specified number of lines from the top of the text buffer"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def disableScrollControl(self):
        """Disables the scroll control if there is one"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def enableScrollControl(self):
        """Enables the scroll control if there is one"""
        raise NotImplementedError()

    def endUpdate(self, redraw=True):
        """Signifies that the massive updates are over. We can now redraw."""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getBufferLimit(self):
        """Returns the current buffer limit"""
        raise NotImplementedError()

    def getBufferSize(self):
        """Returns the size of the buffer"""
        raise NotImplementedError()

    def getEncodedBuffer(self):
        """Returns the encoded buffer in a python buffer object. Do NOT use result with setEncodedBufferW."""
        raise NotImplementedError()

    def getEncodedBufferW(self):
        """Unicode version of getEncodedBuffer. Do NOT use result with setEncodedBuffer."""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the current default font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getScrollPosition(self):
        """Returns what line is the top line."""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getString(self):
        """Gets the string of the edit control."""
        raise NotImplementedError()

    def getStringW(self):
        """Unicode version of getString."""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def insertChar(self,c):
        """Inserts a character at the current cursor position."""
        raise NotImplementedError()

    def insertCharW(self,c):
        """Unicode version of insertChar."""
        raise NotImplementedError()

    def insertColor(self,color):
        """Inserts an encoded color object at the current cursor position.
'color' is a ptColor object."""
        raise NotImplementedError()

    def insertString(self,string):
        """Inserts a string at the current cursor position."""
        raise NotImplementedError()

    def insertStringW(self,string):
        """Unicode version of insertString"""
        raise NotImplementedError()

    def insertStyle(self,style):
        """Inserts an encoded font style at the current cursor position."""
        raise NotImplementedError()

    def isAtEnd(self):
        """Returns whether the cursor is at the end."""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isLocked(self):
        """Is the multi-line edit control locked? Returns 1 if true otherwise returns 0"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def lock(self):
        """Locks the multi-line edit control so the user cannot make changes."""
        raise NotImplementedError()

    def moveCursor(self,direction):
        """Move the cursor in the specified direction (see PtGUIMultiLineDirection)"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setBufferLimit(self,bufferLimit):
        """Sets the buffer max for the editbox"""
        raise NotImplementedError()

    def setEncodedBuffer(self,bufferObject):
        """Sets the edit control to the encoded buffer in the python buffer object. Do NOT use with a result from getEncodedBufferW."""
        raise NotImplementedError()

    def setEncodedBufferW(self,bufferObject):
        """Unicode version of setEncodedBuffer. Do NOT use with a result from getEncodedBuffer."""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the default font size for the edit control"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setScrollPosition(self,topLine):
        """Sets the what line is the top line."""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setString(self,asciiText):
        """Sets the multi-line edit control string."""
        raise NotImplementedError()

    def setStringW(self,unicodeText):
        """Unicode version of setString."""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

    def unclickable(self):
        """Makes this listbox not clickable by the user.
Useful when just displaying a list that is not really selectable."""
        raise NotImplementedError()

    def unlock(self):
        """Unlocks the multi-line edit control so that the user can make changes."""
        raise NotImplementedError()

class ptGUIControlProgress(ptGUIControlValue):
    """Plasma GUI control for progress bar"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def animateToPercent(self,percent):
        """Sets the value of the control and animates to that point."""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getMax(self):
        """Returns the maximum of the control."""
        raise NotImplementedError()

    def getMin(self):
        """Returns the minimum of the control."""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getStep(self):
        """Returns the step increment of the control."""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def getValue(self):
        """Returns the current value of the control."""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setStep(self,step):
        """Sets the step increment of the control."""
        raise NotImplementedError()

    def setValue(self,value):
        """Sets the current value of the control."""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlRadioGroup(ptGUIControl):
    """Plasma GUI Control Radio Group class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def getValue(self):
        """Returns the current selection of the radio group."""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setValue(self,value):
        """Sets the current selection to 'value'"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlTextBox(ptGUIControl):
    """Plasma GUI Control Textbox class"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the current forecolor"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getString(self):
        """Returns the string that the TextBox is set to (in case you forgot)"""
        raise NotImplementedError()

    def getStringJustify(self):
        """Returns current justify"""
        raise NotImplementedError()

    def getStringW(self):
        """Unicode version of getString"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,color):
        """Sets the text backcolor to 'color', which is a ptColor object."""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,size):
        """Don't use"""
        raise NotImplementedError()

    def setForeColor(self,color):
        """Sets the text forecolor to 'color', which is a ptColor object."""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setString(self,text):
        """Sets the textbox string to 'text'"""
        raise NotImplementedError()

    def setStringJustify(self,justify):
        """Sets current justify"""
        raise NotImplementedError()

    def setStringW(self,text):
        """Unicode version of setString"""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIControlUpDownPair(ptGUIControlValue):
    """Plasma GUI control for up/down pair"""
    def __init__(self,ctrlKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this GUI control"""
        raise NotImplementedError()

    def enable(self,flag=1):
        """Enables this GUI control"""
        raise NotImplementedError()

    def focus(self):
        """Gets focus for this GUI control"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns the ptKey for this GUI control"""
        raise NotImplementedError()

    def getMax(self):
        """Returns the maximum of the control."""
        raise NotImplementedError()

    def getMin(self):
        """Returns the minimum of the control."""
        raise NotImplementedError()

    def getObjectCenter(self):
        """Returns ptPoint3 of the center of the GUI control object"""
        raise NotImplementedError()

    def getOwnerDialog(self):
        """Returns a ptGUIDialog of the dialog that owns this GUI control"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getStep(self):
        """Returns the step increment of the control."""
        raise NotImplementedError()

    def getTagID(self):
        """Returns the Tag ID for this GUI control"""
        raise NotImplementedError()

    def getValue(self):
        """Returns the current value of the control."""
        raise NotImplementedError()

    def hide(self):
        """Hides this GUI control"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this GUI control is enabled"""
        raise NotImplementedError()

    def isFocused(self):
        """Returns whether this GUI control has focus"""
        raise NotImplementedError()

    def isInteresting(self):
        """Returns whether this GUI control is interesting at the moment"""
        raise NotImplementedError()

    def isVisible(self):
        """Returns whether this GUI control is visible"""
        raise NotImplementedError()

    def refresh(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setFocus(self,state):
        """Sets the state of the focus of this GUI control"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setNotifyOnInteresting(self,state):
        """Sets whether this control should send interesting events or not"""
        raise NotImplementedError()

    def setObjectCenter(self,point):
        """Sets the GUI controls object center to 'point'"""
        raise NotImplementedError()

    def setRange(self,minimum,maximum):
        """Sets the minimum and maximum range of the control."""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def setStep(self,step):
        """Sets the step increment of the control."""
        raise NotImplementedError()

    def setValue(self,value):
        """Sets the current value of the control."""
        raise NotImplementedError()

    def setVisible(self,state):
        """Sets the state of visibility of this GUI control"""
        raise NotImplementedError()

    def show(self):
        """Shows this GUI control"""
        raise NotImplementedError()

    def unFocus(self):
        """Releases focus for this GUI control"""
        raise NotImplementedError()

class ptGUIDialog:
    """Plasma GUI dialog class"""
    def __init__(self,dialogKey):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Disables this dialog"""
        raise NotImplementedError()

    def enable(self,enableFlag=1):
        """Enable this dialog"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the back color as a ptColor object"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the select back color as a ptColor object"""
        raise NotImplementedError()

    def getControlFromIndex(self,index):
        """Returns the ptKey of the control with the specified index (not tag ID!)"""
        raise NotImplementedError()

    def getControlFromTag(self,tagID):
        """Returns the ptKey of the control with the specified tag ID"""
        raise NotImplementedError()

    def getFontSize(self):
        """Returns the font size"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the fore color as a ptColor object"""
        raise NotImplementedError()

    def getKey(self):
        """Returns this dialog's ptKey"""
        raise NotImplementedError()

    def getName(self):
        """Returns the dialog's name"""
        raise NotImplementedError()

    def getNumControls(self):
        """Returns the number of controls in this dialog"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the select color as a ptColor object"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns this dialog's tag ID"""
        raise NotImplementedError()

    def getVersion(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def hide(self):
        """Hides the dialog"""
        raise NotImplementedError()

    def isEnabled(self):
        """Is this dialog currently enabled?"""
        raise NotImplementedError()

    def noFocus(self):
        """Makes sure no control has input focus"""
        raise NotImplementedError()

    def refreshAllControls(self):
        """Tells the dialog to redraw all its controls"""
        raise NotImplementedError()

    def setBackColor(self,red,green,blue,alpha):
        """Sets the back color, -1 means don't change"""
        raise NotImplementedError()

    def setBackSelectColor(self,red,green,blue,alpha):
        """Sets the select back color, -1 means don't change"""
        raise NotImplementedError()

    def setFocus(self,ctrlKey):
        """Sets the control that has input focus"""
        raise NotImplementedError()

    def setFontSize(self,fontSize):
        """Sets the font size"""
        raise NotImplementedError()

    def setForeColor(self,red,green,blue,alpha):
        """Sets the fore color, -1 means don't change"""
        raise NotImplementedError()

    def setSelectColor(self,red,green,blue,alpha):
        """Sets the select color, -1 means don't change"""
        raise NotImplementedError()

    def show(self):
        """Shows the dialog"""
        raise NotImplementedError()

    def showNoReset(self):
        """Show dialog without resetting clickables"""
        raise NotImplementedError()

    def updateAllBounds(self):
        """Tells the dialog to recompute all the bounds for its controls"""
        raise NotImplementedError()

class ptGUIPopUpMenu:
    """Takes three diferent argument lists:
gckey
name,screenOriginX,screenOriginY
name,parent,screenOriginX,screenOriginY"""
    def __init__(self,arg1,arg2=None,arg3=None,arg4=None):
        """None"""
        raise NotImplementedError()

    def addConsoleCmdItem(self,name,consoleCmd):
        """Adds a new item to the menu that fires a console command"""
        raise NotImplementedError()

    def addConsoleCmdItemW(self,name,consoleCmd):
        """Unicode version of addConsoleCmdItem"""
        raise NotImplementedError()

    def addNotifyItem(self,name):
        """Adds a new item ot the mneu"""
        raise NotImplementedError()

    def addNotifyItemW(self,name):
        """Unicode version of addNotifyItem"""
        raise NotImplementedError()

    def addSubMenuItem(self,name,subMenu):
        """Adds a submenu to this menu"""
        raise NotImplementedError()

    def addSubMenuItemW(self,name,subMenu):
        """Unicode version of addSubMenuItem"""
        raise NotImplementedError()

    def disable(self):
        """Disables this menu"""
        raise NotImplementedError()

    def enable(self,state=1):
        """Enables/disables this menu"""
        raise NotImplementedError()

    def getBackColor(self):
        """Returns the background color"""
        raise NotImplementedError()

    def getBackSelectColor(self):
        """Returns the background selection color"""
        raise NotImplementedError()

    def getForeColor(self):
        """Returns the foreground color"""
        raise NotImplementedError()

    def getKey(self):
        """Returns this menu's key"""
        raise NotImplementedError()

    def getName(self):
        """Returns this menu's name"""
        raise NotImplementedError()

    def getSelectColor(self):
        """Returns the selection color"""
        raise NotImplementedError()

    def getTagID(self):
        """Returns this menu's tag id"""
        raise NotImplementedError()

    def getVersion(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def hide(self):
        """Hides this menu"""
        raise NotImplementedError()

    def isEnabled(self):
        """Returns whether this menu is enabled or not"""
        raise NotImplementedError()

    def setBackColor(self,r,g,b,a):
        """Sets the background color"""
        raise NotImplementedError()

    def setBackSelectColor(self,r,g,b,a):
        """Sets the selection background color"""
        raise NotImplementedError()

    def setForeColor(self,r,g,b,a):
        """Sets the foreground color"""
        raise NotImplementedError()

    def setSelectColor(self,r,g,b,a):
        """Sets the selection color"""
        raise NotImplementedError()

    def show(self):
        """Shows this menu"""
        raise NotImplementedError()

class ptGUISkin:
    """Plasma GUI Skin object"""
    def __init__(self,key):
        """None"""
        raise NotImplementedError()

    def getKey(self):
        """Returns this object's ptKey"""
        raise NotImplementedError()

class ptGrassShader:
    """Plasma Grass Shader class"""
    def __init__(self,key):
        """None"""
        raise NotImplementedError()

    def getWaveDirection(self,waveNum):
        """Gets the wave waveNum's direction as a tuple of x,y. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        raise NotImplementedError()

    def getWaveDistortion(self,waveNum):
        """Gets the wave waveNum's distortion as a tuple of x,y,z. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        raise NotImplementedError()

    def getWaveSpeed(self,waveNum):
        """Gets the wave waveNum's speed as a float. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        raise NotImplementedError()

    def resetWaves(self):
        """Resets wave data to 0"""
        raise NotImplementedError()

    def setWaveDirection(self,waveNum, direction):
        """Sets the wave waveNum's direction as a tuple of x,y. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        raise NotImplementedError()

    def setWaveDistortion(self,waveNum, distortion):
        """Sets the wave waveNum's distortion as a tuple of x,y,z. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        raise NotImplementedError()

    def setWaveSpeed(self,waveNum, speed):
        """Sets the wave waveNum's speed as a float. waveNum must be between 0 and plGrassShaderMod::kNumWaves-1 (currently 3) inclusive"""
        raise NotImplementedError()

class ptImage:
    """Plasma image class"""
    def __init__(self,imgKey):
        """None"""
        raise NotImplementedError()

    def getColorLoc(self,color):
        """Returns the ptPoint3 where the specified color is located"""
        raise NotImplementedError()

    def getHeight(self):
        """Returns the height of the image"""
        raise NotImplementedError()

    def getPixelColor(self,x,y):
        """Returns the ptColor at the specified location (float from 0 to 1)"""
        raise NotImplementedError()

    def getWidth(self):
        """Returns the width of the image"""
        raise NotImplementedError()

    def saveAsJPEG(self,filename,quality=75):
        """Saves this image to disk as a JPEG file"""
        raise NotImplementedError()

class ptInputInterface:
    """Plasma input interface class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def popTelescope(self):
        """pops off the telescope interface and gos back to previous interface"""
        raise NotImplementedError()

    def pushTelescope(self):
        """pushes on the telescope interface"""
        raise NotImplementedError()

class ptKey:
    """Plasma Key class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """Sends a disable message to whatever this ptKey is pointing to"""
        raise NotImplementedError()

    def enable(self):
        """Sends an enable message to whatever this ptKey is pointing to"""
        raise NotImplementedError()

    def getName(self):
        """Get the name of the object that this ptKey is pointing to"""
        raise NotImplementedError()

    def getParentKey(self):
        """This will return a ptKey object that is the parent of this modifer
However, if the parent is not a modifier or not loaded, then None is returned."""
        raise NotImplementedError()

    def getSceneObject(self):
        """This will return a ptSceneobject object that is associated with this ptKey
However, if this ptKey is _not_ a sceneobject, then unpredicatable results will ensue"""
        raise NotImplementedError()

    def isAttachedToClone(self):
        """Returns whether the python file mod is attached to a clone"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

class ptKeyMap:
    """Accessor class to the Key Mapping functions"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def bindKey(self):
        """Params key1,key2,action
Bind keys to an action"""
        raise NotImplementedError()

    def bindKeyToConsoleCommand(self,keyStr1, command):
        """Binds key to console command"""
        raise NotImplementedError()

    def convertCharToControlCode(self,controlCodeString):
        """Convert string version of control code to number"""
        raise NotImplementedError()

    def convertCharToFlags(self,charString):
        """Convert char string to flags"""
        raise NotImplementedError()

    def convertCharToVKey(self,charString):
        """Convert char string to virtual key"""
        raise NotImplementedError()

    def convertControlCodeToString(self):
        """Params controlCode
Convert control code to character string"""
        raise NotImplementedError()

    def convertVKeyToChar(self,virtualKey,flags):
        """Convert virtual key and shift flags to string"""
        raise NotImplementedError()

    def getBindingFlags1(self):
        """Params controlCode
Returns modifier flags for controlCode"""
        raise NotImplementedError()

    def getBindingFlags2(self):
        """Params controlCode
Returns modifier flags for controlCode"""
        raise NotImplementedError()

    def getBindingFlagsConsole(self,command):
        """Returns modifier flags for the console command mapping"""
        raise NotImplementedError()

    def getBindingKey1(self):
        """Params controlCode
Returns key code for controlCode"""
        raise NotImplementedError()

    def getBindingKey2(self):
        """Params controlCode
Returns key code for controlCode"""
        raise NotImplementedError()

    def getBindingKeyConsole(self,command):
        """Returns key for console command mapping"""
        raise NotImplementedError()

    def writeKeyMap(self):
        """Forces write of the keymap file"""
        raise NotImplementedError()

class ptMarkerMgr:
    """Marker manager accessor class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addMarker(self,x, y, z, id, justCreated):
        """Add a marker in the specified location with the specified id"""
        raise NotImplementedError()

    def areLocalMarkersShowing(self):
        """Returns true if we are showing the markers on this local machine"""
        raise NotImplementedError()

    def captureQuestMarker(self,id, captured):
        """Sets a marker as captured or not"""
        raise NotImplementedError()

    def captureTeamMarker(self,id, team):
        """Sets a marker as captured by the specified team (0 = not captured)"""
        raise NotImplementedError()

    def clearSelectedMarker(self):
        """Unselects the selected marker"""
        raise NotImplementedError()

    def getMarkersRespawn(self):
        """Returns whether markers respawn after being captured, or not"""
        raise NotImplementedError()

    def getSelectedMarker(self):
        """Returns the id of the selected marker"""
        raise NotImplementedError()

    def hideMarkersLocal(self):
        """Hides the markers on your machine, so you can no longer see where they are"""
        raise NotImplementedError()

    def removeAllMarkers(self):
        """Removes all markers"""
        raise NotImplementedError()

    def removeMarker(self,id):
        """Removes the specified marker from the game"""
        raise NotImplementedError()

    def setMarkersRespawn(self,respawn):
        """Sets whether markers respawn after being captured, or not"""
        raise NotImplementedError()

    def setSelectedMarker(self,id):
        """Sets the selected marker to the one with the specified id"""
        raise NotImplementedError()

    def showMarkersLocal(self):
        """Shows the markers on your machine, so you can see where they are"""
        raise NotImplementedError()

class ptMatrix44:
    """Plasma Matrix44 class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def copy(self):
        """Copies the matrix and returns the copy"""
        raise NotImplementedError()

    def getAdjoint(self,adjointMat):
        """Returns the adjoint of the matrix"""
        raise NotImplementedError()

    def getData(self):
        """Returns the matrix in tuple form"""
        raise NotImplementedError()

    def getDeterminant(self):
        """Get the matrix's determinant"""
        raise NotImplementedError()

    def getInverse(self,inverseMat):
        """Returns the inverse of the matrix"""
        raise NotImplementedError()

    def getParity(self):
        """Get the parity of the matrix"""
        raise NotImplementedError()

    def getTranslate(self,vector):
        """Returns the translate vector of the matrix (and sets vector to it as well)"""
        raise NotImplementedError()

    def getTranspose(self,transposeMat):
        """Returns the transpose of the matrix"""
        raise NotImplementedError()

    def make(self,fromPt, atPt, upVec):
        """Creates the matrix from from and at points, and the up vector"""
        raise NotImplementedError()

    def makeRotateMat(self,axis,radians):
        """Makes the matrix a rotation matrix"""
        raise NotImplementedError()

    def makeScaleMat(self,scale):
        """Makes the matrix a scaling matrix"""
        raise NotImplementedError()

    def makeTranslateMat(self,trans):
        """Makes the matrix a translation matrix"""
        raise NotImplementedError()

    def makeUpPreserving(self,fromPt, atPt, upVec):
        """Creates the matrix from from and at points, and the up vector (perserving the up vector)"""
        raise NotImplementedError()

    def reset(self):
        """Reset the matrix to identity"""
        raise NotImplementedError()

    def right(self):
        """Returns the right vector of the matrix"""
        raise NotImplementedError()

    def rotate(self,axis,radians):
        """Rotates the matrix by radians around the axis"""
        raise NotImplementedError()

    def scale(self,scale):
        """Scales the matrix by the vector"""
        raise NotImplementedError()

    def setData(self,mat):
        """Sets the matrix using tuples"""
        raise NotImplementedError()

    def translate(self,vector):
        """Translates the matrix by the vector"""
        raise NotImplementedError()

    def up(self):
        """Returns the up vector of the matrix"""
        raise NotImplementedError()

    def view(self):
        """Returns the view vector of the matrix"""
        raise NotImplementedError()

class ptMoviePlayer:
    """Accessor class to play in the MoviePlayer"""
    def __init__(self,movieName,selfKey):
        """None"""
        raise NotImplementedError()

    def pause(self):
        """Pauses the movie"""
        raise NotImplementedError()

    def play(self):
        """Plays the movie"""
        raise NotImplementedError()

    def playPaused(self):
        """Plays movie, but pauses at first frame"""
        raise NotImplementedError()

    def resume(self):
        """Resumes movie after pausing"""
        raise NotImplementedError()

    def setCenter(self,x,y):
        """Sets the center of the movie"""
        raise NotImplementedError()

    def setColor(self,color):
        """Sets the color of the movie"""
        raise NotImplementedError()

    def setOpacity(self,opacity):
        """Sets the opacity of the movie"""
        raise NotImplementedError()

    def setScale(self,width,height):
        """Sets the width and height scale of the movie"""
        raise NotImplementedError()

    def setVolume(self,volume):
        """Set the volume of the movie"""
        raise NotImplementedError()

    def stop(self):
        """Stops the movie"""
        raise NotImplementedError()

class ptNetLinkingMgr:
    """Constructor to get access to the net link manager"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getCurrAgeLink(self):
        """Get the ptAgeLinkStruct for the current age"""
        raise NotImplementedError()

    def getPrevAgeLink(self):
        """Get the ptAgeLinkStruct for the previous age"""
        raise NotImplementedError()

    def isEnabled(self):
        """True if linking is enabled."""
        raise NotImplementedError()

    def linkPlayerHere(self,pid):
        """link player(pid) to where I am"""
        raise NotImplementedError()

    def linkPlayerToAge(self,ageLink,pid):
        """Link player(pid) to ageLink"""
        raise NotImplementedError()

    def linkToAge(self,ageLink):
        """Links to ageLink (ptAgeLinkStruct)"""
        raise NotImplementedError()

    def linkToMyNeighborhoodAge(self):
        """Link to my Neighborhood Age"""
        raise NotImplementedError()

    def linkToMyPersonalAge(self):
        """Link to my Personal Age"""
        raise NotImplementedError()

    def linkToMyPersonalAgeWithYeeshaBook(self):
        """Link to my Personal Age with the YeeshaBook"""
        raise NotImplementedError()

    def linkToPlayersAge(self,pid):
        """Link me to where player(pid) is"""
        raise NotImplementedError()

    def setEnabled(self,enable):
        """Enable/Disable linking."""
        raise NotImplementedError()

class ptNotify:
    """Creates a Notify message
- selfKey is ptKey of your PythonFile modifier"""
    def __init__(self,selfKey):
        """None"""
        raise NotImplementedError()

    def addActivateEvent(self,activeFlag,activateFlag):
        """Add an activate event record to the notify message"""
        raise NotImplementedError()

    def addCallbackEvent(self,eventNumber):
        """Add a callback event record to the notify message"""
        raise NotImplementedError()

    def addCollisionEvent(self,enterFlag,hitterKey,hitteeKey):
        """Add a collision event record to the Notify message"""
        raise NotImplementedError()

    def addContainerEvent(self,enteringFlag,containerKey,containedKey):
        """Add a container event record to the notify message"""
        raise NotImplementedError()

    def addControlKeyEvent(self,keynumber,downFlag):
        """Add a keyboard event record to the Notify message"""
        raise NotImplementedError()

    def addFacingEvent(self,enabledFlag,facerKey, faceeKey, dotProduct):
        """Add a facing event record to the Notify message"""
        raise NotImplementedError()

    def addPickEvent(self,enabledFlag,pickerKey,pickeeKey,hitPoint):
        """Add a pick event record to the Notify message"""
        raise NotImplementedError()

    def addReceiver(self,key):
        """Add a receivers key to receive this Notify message"""
        raise NotImplementedError()

    def addResponderState(self,state):
        """Add a responder state event record to the notify message"""
        raise NotImplementedError()

    def addVarKey(self,name,key):
        """Add a ptKey variable event record to the Notify message
This event record is used to pass a ptKey variable to another python program"""
        raise NotImplementedError()

    def addVarNumber(self,name,number):
        """Add a number variable event record to the Notify message
Method will try to pick appropriate variable type
This event record is used to pass a number variable to another python program"""
        raise NotImplementedError()
        
    def addVarFloat(self,name,number):
        """Add a float variable event record to the Notify message
This event record is used to pass a number variable to another python program"""
        raise NotImplementedError()
        
    def addVarInt(self,name,number):
        """Add a integer variable event record to the Notify message
This event record is used to pass a number variable to another python program"""
        raise NotImplementedError()
        
    def addVarNull(self,name):
        """Add a null (no data) variable event record to the Notify message
This event record is used to pass a number variable to another python program"""
        raise NotImplementedError()

    def clearReceivers(self):
        """Remove all the receivers that this Notify message has
- receivers are automatically added if from a ptAttribActivator"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

    def netPropagate(self,netFlag):
        """Sets the net propagate flag - default to set"""
        raise NotImplementedError()

    def send(self):
        """Send the notify message"""
        raise NotImplementedError()

    def setActivate(self,state):
        """Set the activate state to true(1.0) or false(0.0)"""
        raise NotImplementedError()

    def setType(self,type):
        """Sets the message type"""
        raise NotImplementedError()

class ptParticle:
    """Plasma particle system class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

    def setGeneratorLife(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setHeightSize(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setInitPitchRange(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setInitYawRange(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setParticleLifeMaximum(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setParticleLifeMinimum(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setParticlesPerSecond(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setScaleMaximum(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setScaleMinimum(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setVelocityMaximum(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setVelocityMinimum(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

    def setWidthSize(self,value):
        """NEEDS DOCSTRING"""
        raise NotImplementedError()

class ptPhysics:
    """Plasma physics class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def angularImpulse(self,impulseVector):
        """Add the given vector (representing a rotation axis and magnitude) to
the attached sceneobject's velocity"""
        raise NotImplementedError()

    def damp(self,damp):
        """Reduce all velocities on the object (0 = all stop, 1 = no effect)"""
        raise NotImplementedError()

    def disable(self):
        """Disables physics on the sceneobject attached"""
        raise NotImplementedError()

    def disableCollision(self):
        """Disables collision detection on the attached sceneobject"""
        raise NotImplementedError()

    def enable(self,state=1):
        """Sets the physics enable state for the sceneobject attached"""
        raise NotImplementedError()

    def enableCollision(self):
        """Enables collision detection on the attached sceneobject"""
        raise NotImplementedError()

    def force(self,forceVector):
        """Applies the specified force to the attached sceneobject"""
        raise NotImplementedError()

    def forceWithOffset(self,forceVector,offsetPt):
        """Applies the specified offsetted force to the attached sceneobject"""
        raise NotImplementedError()

    def impulse(self,impulseVector):
        """Adds the given vector to the attached sceneobject's velocity"""
        raise NotImplementedError()

    def impulseWithOffset(self,impulseVector,offsetPt):
        """Adds the given vector to the attached sceneobject's velocity
with the specified offset"""
        raise NotImplementedError()

    def move(self,direction,distance):
        """Moves the attached sceneobject the specified distance in the specified direction"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object"""
        raise NotImplementedError()

    def rotate(self,radians,axis):
        """Rotates the attached sceneobject the specified radians around the specified axis"""
        raise NotImplementedError()

    def shiftMass(self,offsetVector):
        """Shifts the attached sceneobject's center to mass in the specified direction and distance"""
        raise NotImplementedError()

    def suppress(self,doSuppress):
        """Completely remove the physical, but keep it around so it
can be added back later."""
        raise NotImplementedError()

    def torque(self,torqueVector):
        """Applies the specified torque to the attached sceneobject"""
        raise NotImplementedError()

    def warp(self,position):
        """Warps the sceneobject to a specified location.
'position' can be a ptPoint3 or a ptMatrix44"""
        raise NotImplementedError()

    def warpObj(self,objkey):
        """Warps the sceneobject to match the location and orientation of the specified object"""
        raise NotImplementedError()

class ptPlayer:
    """And optionally __init__(name,playerID)"""
    def __init__(self,avkey,name,playerID,distanceSq):
        """None"""
        raise NotImplementedError()

    def getDistanceSq(self):
        """Returns the distance to remote player from local player"""
        raise NotImplementedError()

    def getPlayerID(self):
        """Returns the unique player ID"""
        raise NotImplementedError()

    def getPlayerName(self):
        """Returns the name of the player"""
        raise NotImplementedError()

    def getPlayerNameW(self):
        """Returns the name of the player as Unicode"""
        raise NotImplementedError()

    def isCCR(self):
        """Is this player a CCR?"""
        raise NotImplementedError()

    def isServer(self):
        """Is this player a server?"""
        raise NotImplementedError()

class ptPoint3:
    """Plasma Point class"""
    def __init__(self,x=0, y=0, z=0):
        """None"""
        raise NotImplementedError()

    def copy(self):
        """Returns a copy of the point in another ptPoint3 object"""
        raise NotImplementedError()

    def distance(self,other):
        """Computes the distance from this point to 'other' point"""
        raise NotImplementedError()

    def distanceSq(self,other):
        """Computes the distance squared from this point to 'other' point
- this function is faster than distance(other)"""
        raise NotImplementedError()

    def getX(self):
        """Returns the 'x' component of the point"""
        raise NotImplementedError()

    def getY(self):
        """Returns the 'y' component of the point"""
        raise NotImplementedError()

    def getZ(self):
        """Returns the 'z' component of the point"""
        raise NotImplementedError()

    def setX(self,x):
        """Sets the 'x' component of the point"""
        raise NotImplementedError()

    def setY(self,y):
        """Sets the 'y' component of the point"""
        raise NotImplementedError()

    def setZ(self,z):
        """Sets the 'z' component of the point"""
        raise NotImplementedError()

    def zero(self):
        """Sets the 'x','y' and the 'z' component to zero"""
        raise NotImplementedError()

class ptSDL:
    """SDL accessor"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def sendToClients(self,key):
        """Sets it so changes to this key are sent to the
server AND the clients. (Normally it just goes
to the server.)"""
        raise NotImplementedError()

    def setDefault(self,key,value):
        """Like setitem, but doesn't broadcast over the net.
Only use for setting defaults that everyone will
already know (from reading it off disk)"""
        raise NotImplementedError()

    def setFlags(self,name,sendImmediate,skipOwnershipCheck):
        """Sets the flags for a variable in this SDL"""
        raise NotImplementedError()

    def setIndex(self,key,idx,value):
        """Sets the value at a specific index in the tuple,
so you don't have to pass the whole thing in"""
        raise NotImplementedError()

    def setIndexNow(self,key,idx,value):
        """Same as setIndex but sends immediately"""
        raise NotImplementedError()

    def setNotify(self,selfkey,key,tolerance):
        """Sets the OnSDLNotify to be called when 'key'
SDL variable changes by 'tolerance' (if number)"""
        raise NotImplementedError()

    def setTagString(self,name,tag):
        """Sets the tag string for a variable"""
        raise NotImplementedError()

class ptSDLStateDataRecord:
    """Basic SDL state data record class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def findVar(self,name):
        """Finds and returns the specified ptSimpleStateVariable"""
        raise NotImplementedError()

    def getName(self):
        """Returns our record's name"""
        raise NotImplementedError()

    def getVarList(self):
        """Returns the names of the vars we hold as a list of strings"""
        raise NotImplementedError()

    def setFromDefaults(self,timeStampNow):
        """Sets all our vars to their defaults"""
        raise NotImplementedError()

class ptSceneobject:
    """Plasma Sceneobject class"""
    def __init__(self,objKey, selfKey):
        """None"""
        raise NotImplementedError()

    def addKey(self,key):
        """Mostly used internally.
Add another sceneobject ptKey"""
        raise NotImplementedError()

    def animate(self):
        """If we can animate, start animating"""
        raise NotImplementedError()

    def avatarVelocity(self):
        """Returns the velocity of the first attached avatar scene object"""
        raise NotImplementedError()

    def fastForwardAttachedResponder(self,state):
        """Fast forward the attached responder to the specified state"""
        raise NotImplementedError()

    def findObject(self,name):
        """Find a particular object in just the sceneobjects that are attached"""
        raise NotImplementedError()

    def getKey(self):
        """Get the ptKey of this sceneobject
If there are more then one attached, get the first one"""
        raise NotImplementedError()

    def getLocalToParent(self):
        """Returns ptMatrix44 of the local to parent transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        raise NotImplementedError()

    def getLocalToWorld(self):
        """Returns ptMatrix44 of the local to world transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        raise NotImplementedError()

    def getName(self):
        """Returns the name of the sceneobject (Max name)
- If there are more than one sceneobject attached, return just the first one"""
        raise NotImplementedError()

    def getParentToLocal(self):
        """Returns ptMatrix44 of the parent to local transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        raise NotImplementedError()

    def getPythonMods(self):
        """Returns list of ptKeys of the python modifiers attached to this sceneobject"""
        raise NotImplementedError()

    def getResponderState(self):
        """Return the responder state (if we are a responder)"""
        raise NotImplementedError()

    def getResponders(self):
        """Returns list of ptKeys of the responders attached to this sceneobject"""
        raise NotImplementedError()

    def getSoundIndex(self,sndComponentName):
        """Get the index of the requested sound component"""
        raise NotImplementedError()

    def getWorldToLocal(self):
        """Returns ptMatrix44 of the world to local transform for this sceneobject
- If there is more than one sceneobject attached, returns just the first one"""
        raise NotImplementedError()

    def isAvatar(self):
        """Returns true if the scene object is an avatar"""
        raise NotImplementedError()

    def isHuman(self):
        """Returns true if the scene object is a human avatar"""
        raise NotImplementedError()

    def isLocallyOwned(self):
        """Returns true(1) if this object is locally owned by this client
or returns false(0) if it is not or don't know"""
        raise NotImplementedError()

    def netForce(self,forceFlag):
        """Specify whether this object needs to use messages that are forced to the network
- This is to be used if your Python program is running on only one client
Such as a game master, only running on the client that owns a particular object
- Setting the netForce flag on a sceneobject will also set the netForce flag on
its draw, physics, avatar, particle objects"""
        raise NotImplementedError()

    def playAnimNamed(self,animName):
        """Play the attached named animation"""
        raise NotImplementedError()

    def popCamera(self,avKey):
        """Pop the camera stack and go back to the previous camera"""
        raise NotImplementedError()

    def popCutsceneCamera(self,avKey):
        """Pop the camera stack and go back to previous camera."""
        raise NotImplementedError()

    def position(self):
        """Returns the scene object's current position"""
        raise NotImplementedError()

    def pushCamera(self,avKey):
        """Switch to this object (if it is a camera)"""
        raise NotImplementedError()

    def pushCameraCut(self,avKey):
        """Switch to this object, cutting the view (if it is a camera)"""
        raise NotImplementedError()

    def pushCutsceneCamera(self,cutFlag,avKey):
        """Switch to this object (assuming that it is actually a camera)"""
        raise NotImplementedError()

    def rewindAnimNamed(self,animName):
        """Rewind the attached named animation"""
        raise NotImplementedError()

    def right(self):
        """Returns the scene object's current right vector"""
        raise NotImplementedError()

    def runAttachedResponder(self,state):
        """Run the attached responder to the specified state"""
        raise NotImplementedError()

    def setSoundFilename(self,index, filename, isCompressed):
        """Sets the sound attached to this sceneobject to use the specified sound file."""
        raise NotImplementedError()

    def setTransform(self,local2world,world2local):
        """Set our current transforms"""
        raise NotImplementedError()

    def stopAnimNamed(self,animName):
        """Stop the attached named animation"""
        raise NotImplementedError()

    def up(self):
        """Returns the scene object's current up vector"""
        raise NotImplementedError()

    def view(self):
        """Returns the scene object's current view vector"""
        raise NotImplementedError()

    def volumeSensorIgnoreExtraEnters(self,ignore):
        """Tells the volume sensor attached to this object to ignore extra enters (default), or not (hack for garrison)."""
        raise NotImplementedError()

    def volumeSensorNoArbitration(self, noArbitration):
        """Tells the volume sensor attached to this object whether or not to negotiate exclusive locks with the server."""
        raise NotImplementedError()


class ptSimpleStateVariable:
    """Basic SDL state data record class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getBool(self,idx=0):
        """Returns a boolean variable's value"""
        raise NotImplementedError()

    def getByte(self,idx=0):
        """Returns a byte variable's value"""
        raise NotImplementedError()

    def getDefault(self):
        """Returns the variable's default"""
        raise NotImplementedError()

    def getDisplayOptions(self):
        """Returns the variable's display options"""
        raise NotImplementedError()

    def getDouble(self,idx=0):
        """Returns a double variable's value"""
        raise NotImplementedError()

    def getFloat(self,idx=0):
        """Returns a float variable's value"""
        raise NotImplementedError()

    def getInt(self,idx=0):
        """Returns an int variable's value"""
        raise NotImplementedError()

    def getShort(self,idx=0):
        """Returns a short variable's value"""
        raise NotImplementedError()

    def getString(self,idx=0):
        """Returns a string variable's value"""
        raise NotImplementedError()

    def getType(self):
        """Returns the variable's type"""
        raise NotImplementedError()

    def isAlwaysNew(self):
        """Is this variable always new?"""
        raise NotImplementedError()

    def isInternal(self):
        """Is this an internal variable?"""
        raise NotImplementedError()

    def isUsed(self):
        """Is this variable used?"""
        raise NotImplementedError()

    def setBool(self,val,idx=0):
        """Sets a boolean variable's value"""
        raise NotImplementedError()

    def setByte(self,val,idx=0):
        """Sets a byte variable's value"""
        raise NotImplementedError()

    def setDouble(self,val,idx=0):
        """Sets a double variable's value"""
        raise NotImplementedError()

    def setFloat(self,val,idx=0):
        """Sets a float variable's value"""
        raise NotImplementedError()

    def setInt(self,val,idx=0):
        """Sets an int variable's value"""
        raise NotImplementedError()

    def setShort(self,val,idx=0):
        """Sets a short variable's value"""
        raise NotImplementedError()

    def setString(self,val,idx=0):
        """Sets a string variable's value"""
        raise NotImplementedError()

class ptSpawnPointInfo:
    """Class to hold spawn point data"""
    def __init__(self,title=None,spawnPt=None):
        """None"""
        raise NotImplementedError()

    def getCameraStack(self):
        """Returns the camera stack for this spawnpoint as a string"""
        raise NotImplementedError()

    def getName(self):
        """Returns the spawnpoint's name"""
        raise NotImplementedError()

    def getTitle(self):
        """Returns the spawnpoint's title"""
        raise NotImplementedError()

    def setCameraStack(self,stack):
        """Sets the spawnpoint's camera stack (as a string)"""
        raise NotImplementedError()

    def setName(self,name):
        """Sets the spawnpoint's name"""
        raise NotImplementedError()

    def setTitle(self,title):
        """Sets the spawnpoint's title"""
        raise NotImplementedError()

class ptSpawnPointInfoRef:
    """Class to hold spawn point data"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def getCameraStack(self):
        """Returns the camera stack for this spawnpoint as a string"""
        raise NotImplementedError()

    def getName(self):
        """Returns the spawnpoint's name"""
        raise NotImplementedError()

    def getTitle(self):
        """Returns the spawnpoint's title"""
        raise NotImplementedError()

    def setCameraStack(self,stack):
        """Sets the spawnpoint's camera stack (as a string)"""
        raise NotImplementedError()

    def setName(self,name):
        """Sets the spawnpoint's name"""
        raise NotImplementedError()

    def setTitle(self,title):
        """Sets the spawnpoint's title"""
        raise NotImplementedError()

class ptStatusLog:
    """A status log class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def close(self):
        """Close the status log file"""
        raise NotImplementedError()

    def isOpen(self):
        """Returns whether the status log is currently opened"""
        raise NotImplementedError()

    def open(self,logName,numLines,flags):
        """Open a status log for writing to
'logname' is the name of the log file (example: special.log)
'numLines' is the number of lines to display on debug screen
'flags' is a PlasmaConstants.PtStatusLogFlags"""
        raise NotImplementedError()

    def write(self,text,color=None):
        """If the status log is open, write 'text' to log
'color' is the display color in debug screen"""
        raise NotImplementedError()

class ptStream:
    """A basic stream class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def close(self):
        """Close the status log file"""
        raise NotImplementedError()

    def isOpen(self):
        """Returns whether the stream file is currently opened"""
        raise NotImplementedError()

    def open(self,fileName,flags):
        """Open a stream file for reading or writing"""
        raise NotImplementedError()

    def readlines(self):
        """Reads a list of strings from the file"""
        raise NotImplementedError()

    def writelines(self,lines):
        """Write a list of strings to the file"""
        raise NotImplementedError()

class ptSwimCurrentInterface:
    """Creates a new ptSwimCurrentInterface"""
    def __init__(self,key):
        """None"""
        raise NotImplementedError()

    def disable(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def enable(self):
        """UNKNOWN"""
        raise NotImplementedError()

class ptVault:
    """Accessor class to the player's vault"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addChronicleEntry(self,entryName,type,string):
        """Adds an entry to the player's chronicle with a value of 'string'."""
        raise NotImplementedError()

    def amAgeCzar(self,ageInfo):
        """Are we the czar (WTH is this?) of the specified age?"""
        raise NotImplementedError()

    def amAgeOwner(self,ageInfo):
        """Are we the owner of the specified age?"""
        raise NotImplementedError()

    def amCzarOfCurrentAge(self):
        """Are we the czar (WTH is this?) of the current age?"""
        raise NotImplementedError()

    def amOwnerOfCurrentAge(self):
        """Are we the owner of the current age?"""
        raise NotImplementedError()

    def createNeighborhood(self):
        """Creates a new neighborhood"""
        raise NotImplementedError()

    def findChronicleEntry(self,entryName):
        """Returns a ptVaultNode of type kNodeTypeChronicle of the current player's chronicle entry by entryName."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Find the node matching the template"""
        raise NotImplementedError()

    def getAgeJournalsFolder(self):
        """Returns a ptVaultFolderNode of the current player's age journals folder."""
        raise NotImplementedError()

    def getAgesICanVisitFolder(self):
        """Returns a ptVaultFolderNode of ages I can visit"""
        raise NotImplementedError()

    def getAgesIOwnFolder(self):
        """Returns a ptVaultFolderNode of ages that I own"""
        raise NotImplementedError()

    def getAllPlayersFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the all players folder."""
        raise NotImplementedError()

    def getAvatarClosetFolder(self):
        """Do not use.
Returns a ptVaultFolderNode of the avatars outfit in their closet."""
        raise NotImplementedError()

    def getAvatarOutfitFolder(self):
        """Do not use.
Returns a ptVaultFolderNode of the avatars outfit."""
        raise NotImplementedError()

    def getBuddyListFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the current player's buddy list folder."""
        raise NotImplementedError()

    def getChronicleFolder(self):
        """Returns a ptVaultFolderNode of the current player's chronicle folder."""
        raise NotImplementedError()

    def getGlobalInbox(self):
        """Returns a ptVaultFolderNode of the global inbox folder."""
        raise NotImplementedError()

    def getIgnoreListFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the current player's ignore list folder."""
        raise NotImplementedError()

    def getInbox(self):
        """Returns a ptVaultFolderNode of the current player's inbox folder."""
        raise NotImplementedError()

    def getInviteFolder(self):
        """Returns a ptVaultFolderNode of invites"""
        raise NotImplementedError()

    def getKIUsage(self):
        """Returns a tuple with usage statistics of the KI (# of pics, # of text notes, # of marker games)"""
        raise NotImplementedError()

    def getLinkToCity(self):
        """Returns a ptVaultAgeLinkNode that will go to the city"""
        raise NotImplementedError()

    def getLinkToMyNeighborhood(self):
        """Returns a ptVaultAgeLinkNode that will go to my neighborhood"""
        raise NotImplementedError()

    def getOwnedAgeLink(self,ageInfo):
        """Returns a ptVaultAgeLinkNode to my owned age(ageInfo)"""
        raise NotImplementedError()

    def getPeopleIKnowAboutFolder(self):
        """Returns a ptVaultPlayerInfoListNode of the current player's people I know about (Recent) list folder."""
        raise NotImplementedError()

    def getPlayerInfo(self):
        """Returns a ptVaultNode of type kNodeTypePlayerInfo of the current player"""
        raise NotImplementedError()

    def getPsnlAgeSDL(self):
        """Returns the personal age SDL"""
        raise NotImplementedError()

    def getVisitAgeLink(self,ageInfo):
        """Returns a ptVaultAgeLinkNode for a visitor to age(ageInfo)"""
        raise NotImplementedError()

    def inMyNeighborhoodAge(self):
        """Are we in the player's neighborhood age?"""
        raise NotImplementedError()

    def inMyPersonalAge(self):
        """Are we in the player's personal age?"""
        raise NotImplementedError()

    def invitePlayerToAge(self,link,playerID):
        """Sends an invitation to visit the age to the specified player"""
        raise NotImplementedError()

    def offerLinkToPlayer(self,link,playerID):
        """Offer a one-time link to the specified player"""
        raise NotImplementedError()

    def registerMTStation(self,stationName,mtSpawnPoint):
        """Registers this player at the specified mass-transit point"""
        raise NotImplementedError()

    def registerOwnedAge(self,link):
        """Registers the specified age as owned by the player"""
        raise NotImplementedError()

    def registerVisitAge(self,link):
        """Register this age as visitable by this player"""
        raise NotImplementedError()

    def sendToDevice(self,node,deviceName):
        """Sends a ptVaultNode object to an Age's device by deviceName."""
        raise NotImplementedError()

    def setAgePublic(self,ageInfo,makePublic):
        """Makes the specified age public or private"""
        raise NotImplementedError()

    def unInvitePlayerToAge(self,guid,playerID):
        """Revokes the invitation to visit the age"""
        raise NotImplementedError()

    def unRegisterOwnedAge(self,ageFilename):
        """Unregisters the specified age so it's no longer owned by this player"""
        raise NotImplementedError()

    def unRegisterVisitAge(self,guid):
        """Unregisters the specified age so it can no longer be visited by this player"""
        raise NotImplementedError()

    def updatePsnlAgeSDL(self,pyrec):
        """Updates the personal age SDL to the specified data"""
        raise NotImplementedError()

class ptVaultNode:
    """Vault node class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultFolderNode(ptVaultNode):
    """Plasma vault folder node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def folderGetName(self):
        """LEGACY
Returns the folder's name"""
        raise NotImplementedError()

    def folderGetType(self):
        """LEGACY
Returns the folder type (of the standard folder types)"""
        raise NotImplementedError()

    def folderSetName(self,name):
        """LEGACY
Set the folder name"""
        raise NotImplementedError()

    def folderSetType(self,type):
        """LEGACY
Set the folder type"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getFolderName(self):
        """Returns the folder's name"""
        raise NotImplementedError()

    def getFolderNameW(self):
        """Unicode version of getFolerName"""
        raise NotImplementedError()

    def getFolderType(self):
        """Returns the folder type (of the standard folder types)"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setFolderName(self,name):
        """Set the folder name"""
        raise NotImplementedError()

    def setFolderNameW(self,name):
        """Unicode version of setFolderName"""
        raise NotImplementedError()

    def setFolderType(self,type):
        """Set the folder type"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultAgeInfoListNode(ptVaultFolderNode):
    """Plasma vault age info list node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addAge(self,ageID):
        """Adds ageID to list of ages"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def folderGetName(self):
        """LEGACY
Returns the folder's name"""
        raise NotImplementedError()

    def folderGetType(self):
        """LEGACY
Returns the folder type (of the standard folder types)"""
        raise NotImplementedError()

    def folderSetName(self,name):
        """LEGACY
Set the folder name"""
        raise NotImplementedError()

    def folderSetType(self,type):
        """LEGACY
Set the folder type"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getFolderName(self):
        """Returns the folder's name"""
        raise NotImplementedError()

    def getFolderNameW(self):
        """Unicode version of getFolerName"""
        raise NotImplementedError()

    def getFolderType(self):
        """Returns the folder type (of the standard folder types)"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasAge(self,ageID):
        """Returns whether ageID is in the list of ages"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAge(self,ageID):
        """Removes ageID from list of ages"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setFolderName(self,name):
        """Set the folder name"""
        raise NotImplementedError()

    def setFolderNameW(self,name):
        """Unicode version of setFolderName"""
        raise NotImplementedError()

    def setFolderType(self,type):
        """Set the folder type"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultAgeInfoNode(ptVaultNode):
    """Plasma vault age info node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def asAgeInfoStruct(self):
        """Returns this ptVaultAgeInfoNode as a ptAgeInfoStruct"""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getAgeDescription(self):
        """Returns the description of the age"""
        raise NotImplementedError()

    def getAgeFilename(self):
        """Returns the age filename"""
        raise NotImplementedError()

    def getAgeID(self):
        """Returns the age ID"""
        raise NotImplementedError()

    def getAgeInstanceGuid(self):
        """Returns the age instance guid"""
        raise NotImplementedError()

    def getAgeInstanceName(self):
        """Returns the instance name of the age"""
        raise NotImplementedError()

    def getAgeLanguage(self):
        """Returns the age's language (integer)"""
        raise NotImplementedError()

    def getAgeOwnersFolder(self):
        """Returns a ptVaultPlayerInfoList of the players that own this age"""
        raise NotImplementedError()

    def getAgeSDL(self):
        """Returns a ptVaultSDLNode of the age's SDL"""
        raise NotImplementedError()

    def getAgeSequenceNumber(self):
        """Returns the sequence number of this instance of the age"""
        raise NotImplementedError()

    def getAgeUserDefinedName(self):
        """Returns the user define part of the age name"""
        raise NotImplementedError()

    def getCanVisitFolder(self):
        """Returns a ptVaultPlayerInfoList of the players that can visit this age"""
        raise NotImplementedError()

    def getChildAgesFolder(self):
        """Returns a ptVaultFolderNode of the child ages of this age"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getCzar(self):
        """Returns ptVaultPlayerInfoNode of the player that is the Czar"""
        raise NotImplementedError()

    def getCzarID(self):
        """Returns the ID of the age's czar"""
        raise NotImplementedError()

    def getDisplayName(self):
        """Returns the displayable version of the age name"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getParentAgeLink(self):
        """Returns ptVaultAgeLinkNode of the age's parent age, or None if not a child age"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def isPublic(self):
        """Returns whether the age is Public or Not"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setAgeDescription(self,description):
        """Sets the description of the age"""
        raise NotImplementedError()

    def setAgeFilename(self,fileName):
        """Sets the filename"""
        raise NotImplementedError()

    def setAgeID(self,ageID):
        """Sets the age ID"""
        raise NotImplementedError()

    def setAgeInstanceGuid(self,guid):
        """Sets the age instance GUID"""
        raise NotImplementedError()

    def setAgeInstanceName(self,instanceName):
        """Sets the instance name"""
        raise NotImplementedError()

    def setAgeLanguage(self,lang):
        """Sets the age's language (integer)"""
        raise NotImplementedError()

    def setAgeSequenceNumber(self,seqNumber):
        """Sets the sequence number"""
        raise NotImplementedError()

    def setAgeUserDefinedName(self,udname):
        """Sets the user defined part of the name"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultAgeLinkNode(ptVaultNode):
    """Plasma vault age link node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def addSpawnPoint(self,point):
        """Adds the specified ptSpawnPointInfo or ptSpawnPointInfoRef"""
        raise NotImplementedError()

    def asAgeLinkStruct(self):
        """Returns this ptVaultAgeLinkNode as a ptAgeLinkStruct"""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getAgeInfo(self):
        """Returns the ageInfo as a ptAgeInfoStruct"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getLocked(self):
        """Returns whether the link is locked or not"""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getSpawnPoints(self):
        """Returns a list of ptSpawnPointInfo objects"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def getVolatile(self):
        """Returns whether the link is volatile or not"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def hasSpawnPoint(self,spawnPtName):
        """Returns true if this link has the specified spawn point"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def removeSpawnPoint(self,point):
        """Removes the specified spawn point based on a ptSpawnPointInfo, ptSpawnPointInfoRef, or string"""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setLocked(self,state):
        """Sets whether the link is locked or not"""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def setVolatile(self,state):
        """Sets the state of the volitility of the link"""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultChronicleNode(ptVaultNode):
    """Plasma vault chronicle node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def chronicleGetName(self):
        """LEGACY: Returns the name of the chronicle node."""
        raise NotImplementedError()

    def chronicleGetType(self):
        """LEGACY: Returns the user defined type of the chronicle node."""
        raise NotImplementedError()

    def chronicleGetValue(self):
        """LEGACY: Returns the value as a string of this chronicle node."""
        raise NotImplementedError()

    def chronicleSetName(self,name):
        """LEGACY: Sets the name of the chronicle node."""
        raise NotImplementedError()

    def chronicleSetType(self,type):
        """LEGACY: Sets this chronicle node to a user defined type."""
        raise NotImplementedError()

    def chronicleSetValue(self,value):
        """LEGACY: Sets the chronicle to a value that is a string"""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getEntryType(self):
        """Returns the user defined type of the chronicle node."""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getName(self):
        """Returns the name of the chronicle node."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def getValue(self):
        """Returns the value as a string of this chronicle node."""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setEntryType(self,type):
        """Sets this chronicle node to a user defined type."""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setName(self,name):
        """Sets the name of the chronicle node."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def setValue(self,value):
        """Sets the chronicle to a value that is a string"""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultImageNode(ptVaultNode):
    """Plasma vault image node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getImage(self):
        """Returns the image(ptImage) of this image node"""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getTitle(self):
        """Returns the title (caption) of this image node"""
        raise NotImplementedError()

    def getTitleW(self):
        """Unicode version of getTitle"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def imageGetImage(self):
        """LEGACY
Returns the image(ptImage) of this image node"""
        raise NotImplementedError()

    def imageGetTitle(self):
        """LEGACY
Returns the title (caption) of this image node"""
        raise NotImplementedError()

    def imageSetImage(self,image):
        """LEGACY
Sets the image(ptImage) of this image node"""
        raise NotImplementedError()

    def imageSetTitle(self,title):
        """LEGACY
Sets the title (caption) of this image node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setImage(self,image):
        """Sets the image(ptImage) of this image node"""
        raise NotImplementedError()

    def setImageFromBuf(self,buf):
        """Sets our image from a buffer"""
        raise NotImplementedError()

    def setImageFromScrShot(self):
        """Grabs a screenshot and stuffs it into this node"""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setTitle(self,title):
        """Sets the title (caption) of this image node"""
        raise NotImplementedError()

    def setTitleW(self,title):
        """Unicode version of setTitle"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultMarkerGameNode(ptVaultNode):
    """Plasma vault age info node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getGameGuid(self):
        """Returns the marker game's guid"""
        raise NotImplementedError()

    def getGameName(self):
        """Returns the marker game's name"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getReward(self):
        """Returns a string representing the reward for completing this game"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setGameGuid(self,guid):
        """Sets the marker game's guid"""
        raise NotImplementedError()

    def setGameName(self,name):
        """Sets marker game's name"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setReward(self,reward):
        """Sets the reward for completing this marker game"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultNodeRef:
    """Vault node relationship pseudo class"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def beenSeen(self):
        """Returns true until we reimplement this"""
        raise NotImplementedError()

    def getChild(self):
        """Returns a ptVaultNode that is the child of this reference"""
        raise NotImplementedError()

    def getChildID(self):
        """Returns id of the child node"""
        raise NotImplementedError()

    def getParent(self):
        """Returns a ptVaultNode that is the parent of the reference"""
        raise NotImplementedError()

    def getParentID(self):
        """Returns id of the parent node"""
        raise NotImplementedError()

    def getSaver(self):
        """Returns a ptVaultPlayerInfoNode of player that created this relationship"""
        raise NotImplementedError()

    def getSaverID(self):
        """Returns id of player that created this relationship"""
        raise NotImplementedError()

    def setSeen(self):
        """Does nothing until we reimplement this"""
        raise NotImplementedError()

class ptVaultPlayerInfoListNode(ptVaultFolderNode):
    """Plasma vault player info list node"""
    def __init__(self,n=0):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def addPlayer(self,playerID):
        """Adds playerID player to this player info list node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def folderGetName(self):
        """LEGACY
Returns the folder's name"""
        raise NotImplementedError()

    def folderGetType(self):
        """LEGACY
Returns the folder type (of the standard folder types)"""
        raise NotImplementedError()

    def folderSetName(self,name):
        """LEGACY
Set the folder name"""
        raise NotImplementedError()

    def folderSetType(self,type):
        """LEGACY
Set the folder type"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getFolderName(self):
        """Returns the folder's name"""
        raise NotImplementedError()

    def getFolderNameW(self):
        """Unicode version of getFolerName"""
        raise NotImplementedError()

    def getFolderType(self):
        """Returns the folder type (of the standard folder types)"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getPlayer(self,playerID):
        """Gets the player info node for the specified player."""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def hasPlayer(self,playerID):
        """Returns whether the 'playerID' is a member of this player info list node."""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def playerlistAddPlayer(self,playerID):
        """LEGACY: Adds playerID player to this player info list node."""
        raise NotImplementedError()

    def playerlistGetPlayer(self,playerID):
        """LEGACY: Gets the player info node for the specified player."""
        raise NotImplementedError()

    def playerlistHasPlayer(self,playerID):
        """LEGACY: Returns whether the 'playerID' is a member of this player info list node."""
        raise NotImplementedError()

    def playerlistRemovePlayer(self,playerID):
        """LEGACY: Removes playerID player from this player info list node."""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def removePlayer(self,playerID):
        """Removes playerID player from this player info list node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setFolderName(self,name):
        """Set the folder name"""
        raise NotImplementedError()

    def setFolderNameW(self,name):
        """Unicode version of setFolderName"""
        raise NotImplementedError()

    def setFolderType(self,type):
        """Set the folder type"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def sort(self):
        """Sorts the player list by some means...?"""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultPlayerInfoNode(ptVaultNode):
    """Plasma vault folder node"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def playerGetAgeGuid(self):
        """Returns the guid as a string of where the player is for this player info node."""
        raise NotImplementedError()

    def playerGetAgeInstanceName(self):
        """Returns the name of the Age where the player is for this player info node."""
        raise NotImplementedError()

    def playerGetCCRLevel(self):
        """Returns the ccr level of the player for this player info node."""
        raise NotImplementedError()

    def playerGetID(self):
        """Returns the player ID for this player info node."""
        raise NotImplementedError()

    def playerGetName(self):
        """Returns the player name of this player info node."""
        raise NotImplementedError()

    def playerIsOnline(self):
        """Returns the online status of the player for this player info node."""
        raise NotImplementedError()

    def playerSetAgeGuid(self,guidString):
        """Not sure this should be used. Sets the guid for this player info node."""
        raise NotImplementedError()

    def playerSetAgeInstanceName(self,name):
        """Not sure this should be used. Sets the name of the age where the player is for this player info node."""
        raise NotImplementedError()

    def playerSetID(self,playerID):
        """Not sure this should be used. Sets the playerID for this player info node."""
        raise NotImplementedError()

    def playerSetName(self,name):
        """Not sure this should be used. Sets the player name of this player info node."""
        raise NotImplementedError()

    def playerSetOnline(self,state):
        """Not sure this should be used. Sets the state of the player online status for this player info node."""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultSDLNode(ptVaultNode):
    """Plasma vault SDL node"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getIdent(self):
        """UNKNOWN"""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getStateDataRecord(self):
        """Returns the ptSDLStateDataRecord associated with this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def initStateDataRecord(self,filename,flags):
        """Read the SDL Rec from File if needed"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setIdent(self,v):
        """UNKNOWN"""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setStateDataRecord(self,rec,writeOptions=0):
        """Sets the ptSDLStateDataRecord"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultSystemNode(ptVaultNode):
    """Plasma vault system node"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of ptVaultNode this is.
See PlasmaVaultTypes.py"""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setType(self,type):
        """Set the type of ptVaultNode this is."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVaultTextNoteNode(ptVaultNode):
    """Plasma vault text note node"""
    def __init__(self):
        """None"""
        raise NotImplementedError()

    def addNode(self,node,cb=None,cbContext=0):
        """Adds 'node'(ptVaultNode) as a child to this node."""
        raise NotImplementedError()

    def findNode(self,templateNode):
        """Returns ptVaultNode if child node found matching template, or None"""
        raise NotImplementedError()

    def getChildNodeCount(self):
        """Returns how many children this node has."""
        raise NotImplementedError()

    def getChildNodeRefList(self):
        """Returns a list of ptVaultNodeRef that are the children of this node."""
        raise NotImplementedError()

    def getClientID(self):
        """Returns the client's ID."""
        raise NotImplementedError()

    def getCreateAgeCoords(self):
        """Returns the location in the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeGuid(self):
        """Returns the guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeName(self):
        """Returns the name of the Age where this node was created."""
        raise NotImplementedError()

    def getCreateAgeTime(self):
        """Returns the time in the Age that the node was created...(?)"""
        raise NotImplementedError()

    def getCreateTime(self):
        """Returns the when this node was created, that is useable by python's time library."""
        raise NotImplementedError()

    def getCreatorNode(self):
        """Returns the creator's node"""
        raise NotImplementedError()

    def getCreatorNodeID(self):
        """Returns the creator's node ID"""
        raise NotImplementedError()

    def getDeviceInbox(self):
        """Returns a ptVaultFolderNode"""
        raise NotImplementedError()

    def getID(self):
        """Returns the unique ID of this ptVaultNode."""
        raise NotImplementedError()

    def getModifyTime(self):
        """Returns the modified time of this node, that is useable by python's time library."""
        raise NotImplementedError()

    def getNode(self,id):
        """Returns ptVaultNodeRef if is a child node, or None"""
        raise NotImplementedError()

    def getOwnerNode(self):
        """Returns a ptVaultNode of the owner of this node"""
        raise NotImplementedError()

    def getOwnerNodeID(self):
        """Returns the node ID of the owner of this node"""
        raise NotImplementedError()

    def getSubType(self):
        """Returns the subtype of this text note node."""
        raise NotImplementedError()

    def getText(self):
        """Returns the text of this text note node."""
        raise NotImplementedError()

    def getTextW(self):
        """Unicode version of getText."""
        raise NotImplementedError()

    def getTitle(self):
        """Returns the title of this text note node."""
        raise NotImplementedError()

    def getTitleW(self):
        """Unicode version of getTitle"""
        raise NotImplementedError()

    def getType(self):
        """Returns the type of text note for this text note node."""
        raise NotImplementedError()

    def hasNode(self,id):
        """Returns true if node if a child node"""
        raise NotImplementedError()

    def linkToNode(self,nodeID,cb=None,cbContext=0):
        """Adds a link to the node designated by nodeID"""
        raise NotImplementedError()

    def noteGetSubType(self):
        """LEGACY
Returns the subtype of this text note node."""
        raise NotImplementedError()

    def noteGetText(self):
        """LEGACY
Returns the text of this text note node."""
        raise NotImplementedError()

    def noteGetTitle(self):
        """LEGACY
Returns the title of this text note node."""
        raise NotImplementedError()

    def noteGetType(self):
        """LEGACY
Returns the type of text note for this text note node."""
        raise NotImplementedError()

    def noteSetSubType(self,subType):
        """LEGACY
Sets the subtype of the this text note node."""
        raise NotImplementedError()

    def noteSetText(self,text):
        """LEGACY
Sets text of the this text note node."""
        raise NotImplementedError()

    def noteSetTitle(self,title):
        """LEGACY
Sets the title of this text note node."""
        raise NotImplementedError()

    def noteSetType(self,type):
        """LEGACY
Sets the type of text note for this text note node."""
        raise NotImplementedError()

    def removeAllNodes(self):
        """Removes all the child nodes on this node."""
        raise NotImplementedError()

    def removeNode(self,node,cb=None,cbContext=0):
        """Removes the child 'node'(ptVaultNode) from this node."""
        raise NotImplementedError()

    def save(self,cb=None,cbContext=0):
        """Save the changes made to this node."""
        raise NotImplementedError()

    def saveAll(self,cb=None,cbContext=0):
        """Saves this node and all its children nodes."""
        raise NotImplementedError()

    def sendTo(self,destID,cb=None,cbContext=0):
        """Send this node to inbox at 'destID'"""
        raise NotImplementedError()

    def setCreateAgeGuid(self,guid):
        """Set guid as a string of the Age where this node was created."""
        raise NotImplementedError()

    def setCreateAgeName(self,name):
        """Set name of the Age where this node was created."""
        raise NotImplementedError()

    def setCreatorNodeID(self,id):
        """Set creator's node ID"""
        raise NotImplementedError()

    def setDeviceInbox(self,inboxName,cb=None,cbContext=0):
        """Sets the device inbox"""
        raise NotImplementedError()

    def setID(self,id):
        """Sets ID of this ptVaultNode."""
        raise NotImplementedError()

    def setOwnerNodeID(self,id):
        """Set node ID of the owner of this node"""
        raise NotImplementedError()

    def setSubType(self,subType):
        """Sets the subtype of the this text note node."""
        raise NotImplementedError()

    def setText(self,text):
        """Sets text of the this text note node."""
        raise NotImplementedError()

    def setTextW(self,text):
        """Unicode version of setText"""
        raise NotImplementedError()

    def setTitle(self,title):
        """Sets the title of this text note node."""
        raise NotImplementedError()

    def setTitleW(self,title):
        """Unicode version of setTitle"""
        raise NotImplementedError()

    def setType(self,type):
        """Sets the type of text note for this text note node."""
        raise NotImplementedError()

    def upcastToAgeInfoListNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoListNode"""
        raise NotImplementedError()

    def upcastToAgeInfoNode(self):
        """Returns this ptVaultNode as ptVaultAgeInfoNode"""
        raise NotImplementedError()

    def upcastToAgeLinkNode(self):
        """Returns this ptVaultNode as ptVaultAgeLinkNode"""
        raise NotImplementedError()

    def upcastToChronicleNode(self):
        """Returns this ptVaultNode as ptVaultChronicleNode"""
        raise NotImplementedError()

    def upcastToFolderNode(self):
        """Returns this ptVaultNode as ptVaultFolderNode"""
        raise NotImplementedError()

    def upcastToImageNode(self):
        """Returns this ptVaultNode as ptVaultImageNode"""
        raise NotImplementedError()

    def upcastToMarkerGameNode(self):
        """Returns this ptVaultNode as ptVaultMarkerNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoListNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoListNode"""
        raise NotImplementedError()

    def upcastToPlayerInfoNode(self):
        """Returns this ptVaultNode as ptVaultPlayerInfoNode"""
        raise NotImplementedError()

    def upcastToPlayerNode(self):
        """Returns this ptVaultNode as a ptVaultPlayerNode"""
        raise NotImplementedError()

    def upcastToSDLNode(self):
        """Returns this ptVaultNode as a ptVaultSDLNode"""
        raise NotImplementedError()

    def upcastToSystemNode(self):
        """Returns this ptVaultNode as a ptVaultSystemNode"""
        raise NotImplementedError()

    def upcastToTextNoteNode(self):
        """Returns this ptVaultNode as ptVaultTextNoteNode"""
        raise NotImplementedError()

class ptVector3:
    """Plasma Point class"""
    def __init__(self,x=0, y=0, z=0):
        """None"""
        raise NotImplementedError()

    def add(self,other):
        """Adds other to the current vector"""
        raise NotImplementedError()

    def copy(self):
        """Copies the vector into another one (which it returns)"""
        raise NotImplementedError()

    def crossProduct(self,other):
        """Finds the cross product between other and this vector"""
        raise NotImplementedError()

    def dotProduct(self,other):
        """Finds the dot product between other and this vector"""
        raise NotImplementedError()

    def getX(self):
        """Returns the 'x' component of the vector"""
        raise NotImplementedError()

    def getY(self):
        """Returns the 'y' component of the vector"""
        raise NotImplementedError()

    def getZ(self):
        """Returns the 'z' component of the vector"""
        raise NotImplementedError()

    def length(self):
        """Returns the length of the vector"""
        raise NotImplementedError()

    def lengthSq(self):
        """Returns the length of the vector, squared
- this function is faster then length(other)"""
        raise NotImplementedError()

    def normalize(self):
        """Normalizes the vector to length 1"""
        raise NotImplementedError()

    def scale(self,scale):
        """Scale the vector by scale"""
        raise NotImplementedError()

    def setX(self,x):
        """Sets the 'x' component of the vector"""
        raise NotImplementedError()

    def setY(self,y):
        """Sets the 'y' component of the vector"""
        raise NotImplementedError()

    def setZ(self,z):
        """Sets the 'z' component of the vector"""
        raise NotImplementedError()

    def subtract(self,other):
        """Subtracts other from the current vector"""
        raise NotImplementedError()

    def zero(self):
        """Zeros the vector's components"""
        raise NotImplementedError()

class ptWaveSet:
    """Creates a new ptWaveSet"""
    def __init__(self,ey):
        """None"""
        raise NotImplementedError()

    def getDepthFalloff(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getEnvCenter(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getEnvRadius(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getGeoAmpOverLen(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getGeoAngleDev(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getGeoChop(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getGeoMaxLength(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getGeoMinLength(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getMaxAtten(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getMinAtten(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getOpacFalloff(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getOpacOffset(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getReflFalloff(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getReflOffset(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getRippleScale(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getSpecularEnd(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getSpecularMute(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getSpecularNoise(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getSpecularStart(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getSpecularTint(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getTexAmpOverLen(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getTexAngleDev(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getTexChop(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getTexMaxLength(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getTexMinLength(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWaterHeight(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWaterOffset(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWaterOpacity(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWaterTint(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWaveFalloff(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWaveOffset(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def getWindDir(self):
        """Returns the attribute's value"""
        raise NotImplementedError()

    def setDepthFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setEnvCenter(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setEnvRadius(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setGeoAmpOverLen(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setGeoAngleDev(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setGeoChop(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setGeoMaxLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setGeoMinLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setMaxAtten(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setMinAtten(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setOpacFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setOpacOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setReflFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setReflOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setRippleScale(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setSpecularEnd(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setSpecularMute(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setSpecularNoise(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setSpecularStart(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setSpecularTint(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setTexAmpOverLen(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setTexAngleDev(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setTexChop(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setTexMaxLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setTexMinLength(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWaterHeight(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWaterOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWaterOpacity(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWaterTint(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWaveFalloff(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWaveOffset(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

    def setWindDir(self,s, secs = 0):
        """Sets the attribute to s over secs time"""
        raise NotImplementedError()

