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
Module: ahnyQuabs
Age: Ahnonay
Date: April, 2007
Author: Derek Odell
Ahnonay Quab control
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaGame import *
from PlasmaGameConstants import *
import xRandom


# define the attributes that will be entered in max
deadZone                    = ptAttribActivator(1, "detector for dead zone")
quabObjects                 = ptAttribSceneobjectList(2, "quab spawners")
SDLQuabs                    = ptAttribString(3, "SDL: quabs")

# globals
byteQuabs = 0
quabList = []
quabKeyList = []

quabBrainList = []
quabVarList = {}

cheater = 0

#====================================
class ahnyQuabs(ptModifier):
    ###########################
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5946
        version = 1
        self.version = version
        print "__init__ahnyQuabs v%d " % (version)
        self.brainsRemaining = 0
        self.brainsReady = 0
        self.gameReady = 0
        self.sdlReady = 0
        self.isAIOwner = 0
        self.quabIsRunning = []
        self.gameId = 0
        self.joinedToGame = 0

    ###########################
    def __del__(self):
        print "ahnyQuabs.del(): will unload %d quabs" % (len(quabKeyList))
        i = 1
        for quabKey in quabKeyList:
            PtUnLoadAvatarModel(quabKey)
            print "ahnyQuabs.del(): unloading quab #%d" % (i)
            i += 1

    ###########################
    def OnFirstUpdate(self):
        global quabList

        for quab in quabObjects.value:
            quabList.append(quab)

        # join the age's common var sync "game"
        self.clientId = PtGetLocalClientID()
        PtJoinCommonVarSyncGame(self.key)

        ###########################
        #def OnServerInitComplete(self):
        global byteQuabs

        try:
            ageSDL = PtGetAgeSDL()
            ageSDL[SDLQuabs.value][0]
        except:
            print "ahnyQuabs.OnServerInitComplete(): ERROR --- Cannot find the Ahnonay Age SDL"
            print SDLQuabs.value
            ageSDL[SDLQuabs.value] = (20,)

        ageSDL.setFlags(SDLQuabs.value,1,1)
        ageSDL.sendToClients(SDLQuabs.value)
        ageSDL.setNotify(self.key,SDLQuabs.value,0.0)

        byteQuabs = ageSDL[SDLQuabs.value][0]

        if byteQuabs < 20 and not len(PtGetPlayerList()):
            updatedTime = ageSDL["ahnyQuabsLastUpdate"][0]
            currentTime = PtGetDniTime()
            amount = (currentTime - updatedTime) / 28800
            print "ahnyQuabs.OnServerInitComplete(): Last Update: %d Current Time: %d means %d quabs returning" % (updatedTime, currentTime, amount)
            if (byteQuabs + amount) > 20:
                byteQuabs = 20
            else:
                byteQuabs += amount
            ageSDL[SDLQuabs.value] = (byteQuabs,)
        elif byteQuabs > 20:
            print "ahnyQuabs.OnServerInitComplete(): I think I have %d quabs for some reason, so I'll reset to 20" % (byteQuabs)
            ageSDL[SDLQuabs.value] = (20,)
            byteQuabs = 20
        ageSDL["ahnyQuabsLastUpdate"] = (PtGetDniTime(),)

        self.sdlReady = 1

        #PtFindSceneobject("Box01", "Ahnonay").physics.suppress(true)
        #PtFindSceneobject("Box01", "Ahnonay").physics.disableCollision()

    ###########################
    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        global byteQuabs

        if VARname == SDLQuabs.value:
            ageSDL = PtGetAgeSDL()
            byteQuabs = ageSDL[SDLQuabs.value][0]

    ###########################
    def OnNotify(self,state,id,events):
        global byteQuabs

        #print "ahnyQuabs.OnNotify: state=%s id=%d events=" % (state, id), events

        if id == deadZone.id:
            if PtFindAvatar(events) != PtGetLocalAvatar():
                quabNum = byteQuabs - 1
                if quabNum < 0:
                    quabNum = 0
                print "ahnyQuabs.OnNotify: quabNum now = %d" % (quabNum)
                ageSDL = PtGetAgeSDL()
                ageSDL[SDLQuabs.value] = (quabNum,)
                ageSDL["ahnyQuabsLastUpdate"] = (PtGetDniTime(),)

    ###########################
    def ISpawnQuabsIfNeeded(self):
        if self.isAIOwner and self.sdlReady and self.gameReady and (byteQuabs > 0):
            print "ahnyQuabs.ISpawnQuabsIfNeeded(): Calling ISpawnQuabs to spawn %d quabs" % (byteQuabs)
            self.ISpawnQuabs()
        elif not self.isAIOwner and self.sdlReady and self.gameReady and (byteQuabs > 0):
            quabs = PtGetAIAvatarsByModelName("Quab") # tuple as [(brain object, "user string")]
            self.brainsRemaining = byteQuabs
            for quab in quabs:
                brainNum = int(quab[1][5:])
                brainName = str(quab[1][:4])
                if brainName == "Quab":
                    print "ahnyQuabs.ISpawnQuabsIfNeeded(): Quab brain #%d was created. Initializing brain." % (brainNum)
                    if len(quabBrainList) <= brainNum:
                        # probably spawned remotely
                        while len(quabBrainList) <= brainNum:
                            quabBrainList.append(None)
                    
                    quabBrainList[brainNum] = quab[0]
                    quab[0].addReceiver(self.key)
                    
                    quab[0].setLocallyControlled(self.isAIOwner)
                    
                    idleBehavior = quab[0].idleBehaviorName()
                    quab[0].addBehavior("Idle02", idleBehavior)
                    quab[0].addBehavior("Idle03", idleBehavior)
                    
                    runBehavior = quab[0].runBehaviorName()
                    quab[0].addBehavior("Run02", runBehavior, randomStartPos = 0)
                    quab[0].addBehavior("Run03", runBehavior, randomStartPos = 0)
                    
                    self.brainsRemaining = self.brainsRemaining - 1
                    if self.brainsRemaining == 0:
                        self.brainsReady = 1
                    
                    #if self.brainsReady and self.gameReady:
                    #    self.IInitInitialAIStates()

    ###########################
    def ISpawnQuabs(self):
        global quabList
        global quabKeyList
        global quabBrainList

        xRandom.shuffle(quabList)
        game = self.IGetVarSyncGameCli()
        if game == None:
            print "ahnyQuabs.ISpawnQuabs(): DANGER!!! No VarSyncGame!"
        print "ahnyQuabs.ISpawnQuabs(): out of %d possible spawn points, will spawn %d quabs" % (len(quabList),byteQuabs)
        i = 1
        self.brainsRemaining = byteQuabs
        while i <= byteQuabs :
            quab = quabList[i-1]
            quabKey = quab.getKey()
            quabKey = PtLoadAvatarModel("Quab", quabKey, "Quab "+str(i - 1))
            quabKeyList.append(quabKey)
            quabBrainList.append(None)

            varName = "QuabRun" + str(i)
            game.createNumericVar(varName, 0)

            print "ahnyQuabs.ISpawnQuabs(): spawned quab #%d" % (i)
            i += 1
    
    ###########################
    def IInitInitialAIStates(self):
        global quabBrainList

        i = 0
        while i < len(quabBrainList):
            print "ahnyQuabs.IInitInitialAIStates(): Setting initial state for brain #%d" % (i)
            brain = quabBrainList[i]
            if brain == None:
                i += 1
                continue
            try:
                if self.quabIsRunning[i]:
                    brain.goToGoal(ptPoint3(0,0,0), 1)
                else:
                    brain.startBehavior(brain.idleBehaviorName())
            except:
                self.quabIsRunning.append(0)
                brain.startBehavior(brain.idleBehaviorName())
            i += 1
    
    ###########################
    def OnUpdate(self, seconds, delta):
        global quabBrainList
        global quabVarList
        global quabKeyList
        global cheater
        
        if cheater:
            return

        ## Check as many conditions as we can so we don't overload processing
        if (not self.brainsReady) or (not self.gameReady):
            return # not ready to run AI yet
        if not self.isAIOwner:
            return # we aren't the ones running the brain, abort
        game = self.IGetVarSyncGameCli()
        if game == None:
            return # no game connection established

        for brain in quabBrainList:
            if brain == None:
                continue

            isRunning = brain.runningBehavior(brain.runBehaviorName())
            isIdling = brain.runningBehavior(brain.idleBehaviorName())

            ## Every three seconds, update goal if we're running
            if PtGetDniTime() % 3 != 0 and isRunning:
                continue

            ## Can we hear anyone?
            playersWeHear = brain.playersICanHear()
            total = ptVector3(0,0,0)
            varName = quabVarList[quabBrainList.index(brain)+1]
            if len(playersWeHear) != 0:
                for avatar in playersWeHear:
                    ## Get the normalized vector to each player we hear
                    vector = brain.vectorToPlayer(avatar)
                    vector.normalize()
                    total = total.add(vector)

                ## Scale it up so it's not a foot away
                escVector = ptVector3(total.getX(),total.getY(),total.getZ())
                escVector = escVector.scale(40)

                index = quabBrainList.index(brain)
                loc = quabKeyList[index].getSceneObject().position()

                ## Find the final position
                target = ptPoint3((escVector.getX() + loc.getX()), (escVector.getY() + loc.getY()) , (escVector.getZ() + loc.getZ()))

                ## Visual representation of goal
                #PtFindSceneobject("Box01", "Ahnonay").physics.warp(target)

                ## Variable Outputs
                #print "Total Vector: %f / %f / %f" % (total.getX(), total.getY(), total.getZ())
                #print "Escape Vector: %f / %f / %f" % (escVector.getX(), escVector.getY(), escVector.getZ())
                #print "Loc: %f / %f / %f" % (loc.getX(), loc.getY(), loc.getZ())
                #print "Target: %f / %f / %f" % (target.getX(), target.getY(), target.getZ())
                #print "Player: %f / %f / %f" % (PtGetLocalAvatar().position().getX(), PtGetLocalAvatar().position().getY(), PtGetLocalAvatar().position().getZ())

                ## Avoid avatars
                brain.goToGoal(target, 0)

                ## Set 'running' var if we need to
                if isIdling:
                    game.setNumericVar(varName, 1)

            else:
                if isRunning:
                    ## Stop running
                    brain.startBehavior(brain.idleBehaviorName())
                    game.setNumericVar(varName, 0)

    ###########################
    def OnAIMsg(self, brain, msgType, userStr, args):
        global quabBrainList
        global quabVarList

        if msgType == PtAIMsgType.kBrainCreated:
            brainNum = int(userStr[5:])
            brainName = str(userStr[:4])
            if brainName == "Quab":
                print "ahnyQuabs.OnAIMsg(): Quab brain #%d was created. Initializing brain." % (brainNum)
                if len(quabBrainList) <= brainNum:
                    # probably spawned remotely
                    while len(quabBrainList) <= brainNum:
                        quabBrainList.append(None)
                
                quabBrainList[brainNum] = brain
                brain.addReceiver(self.key)
                
                brain.setLocallyControlled(self.isAIOwner)
                
                idleBehavior = brain.idleBehaviorName()
                brain.addBehavior("Idle02", idleBehavior)
                brain.addBehavior("Idle03", idleBehavior)
                
                runBehavior = brain.runBehaviorName()
                brain.addBehavior("Run02", runBehavior, randomStartPos = 0)
                brain.addBehavior("Run03", runBehavior, randomStartPos = 0)
                
                self.brainsRemaining = self.brainsRemaining - 1
                if self.brainsRemaining == 0:
                    self.brainsReady = 1
                
                if self.brainsReady and self.gameReady:
                    self.IInitInitialAIStates()
            
            # defaults for the rest should be fine
        elif msgType == PtAIMsgType.kArrivedAtGoal:
            print "ahnyQuabs.Brain arrived at goal, but we don't really care"
    
    ###########################
    def OnGameCliMsg(self,msg):
        global quabBrainList
        global quabVarList

        if msg.getType() == PtGameCliMsgTypes.kGameCliPlayerJoinedMsg:
            joinMsg = msg.upcastToFinalGameCliMsg()
            if joinMsg.playerID() == self.clientId:
                self.gameId = msg.getGameCli().gameID()
                self.joinedToGame = 1
                print "ahnyQuabs.OnGameCliMsg(): Got join reply from the var sync game, we are now an observer for game id %d" % (self.gameId)

        elif msg.getType() == PtGameCliMsgTypes.kGameCliOwnerChangeMsg:
            ownerChangeMsg = msg.upcastToFinalGameCliMsg()
            print "ahnyQuabs.OnGameCliMsg(): Got owner change msg, ownerID = %d, clientId = %d" % (ownerChangeMsg.ownerID(), self.clientId)
            if ownerChangeMsg.ownerID() == self.clientId:
                print "ahnyQuabs.OnGameCliMsg(): We are now the game owner"
                self.isAIOwner = 1
            else:
                self.isAIOwner = 0
            # go through all our brains and tell them whether we are or are not the authoritative client
            for brain in quabBrainList:
                if brain == None:
                    continue
                brain.setLocallyControlled(self.isAIOwner)

        elif msg.getType() == PtGameCliMsgTypes.kGameCliVarSyncMsg:
            varSyncMsg = msg.upcastToGameMsg()
            msgType = varSyncMsg.getVarSyncMsgType()
            finalMsg = varSyncMsg.upcastToFinalVarSyncMsg()
            if msgType == PtVarSyncMsgTypes.kVarSyncAllVarsSent:
                self.gameReady = 1
                self.ISpawnQuabsIfNeeded()
            elif msgType == PtVarSyncMsgTypes.kVarSyncNumericVarCreated:
                name = finalMsg.name()
                id = finalMsg.id()
                idx = name[7:]
                quabVarList[long(idx)] = id
            elif msgType == PtVarSyncMsgTypes.kVarSyncNumericVarChanged:
                id = finalMsg.id()
                value = finalMsg.value()
                if id in quabVarList.keys():
                    index = quabVarList[id]
                    if len(self.quabIsRunning) <= index:
                        # probably spawned remotely
                        while len(self.quabIsRunning) <= index:
                            self.quabIsRunning.append(0)
                    self.quabIsRunning[index] = value
                    if self.isAIOwner:
                        #print "ahnyQuabs.OnGameCliMsg(): Var %s updated but we are AI controller, so we updated it, not adjusting brain." % (name)
                        return # we already have told the brain
                    
                    if index >= len(quabBrainList):
                        #print "ahnyQuabs.OnGameCliMsg(): Var %s updated, but we don't have a brain for that one (possibly too early)." % (name)
                        return # it'll get transferred over when the brains DO get here
                    
                    #print "ahnyQuabs.OnGameCliMsg(): Var %s updated, making sure brain matches." % (name)
                    brain = quabBrainList[index-1]
                    if (brain == None):
                        return
                    isRunning = brain.runningBehavior(brain.runBehaviorName())
                    isIdling = brain.runningBehavior(brain.idleBehaviorName())
                    if value: 
                        if isIdling:
                            # run away!
                            brain.goToGoal(ptPoint3(0,0,0), 1) # avoiding avatars
                    else:
                        if isRunning:
                            # stop running
                            brain.startBehavior(brain.idleBehaviorName())
    
    ###########################
    def IGetVarSyncGameCli(self):
        if not self.joinedToGame:
            print "ahnyQuabs.IGetVarSyncGameCli: Requesting game client before we have become an observer... returning None"
            return None
        
        gameCli = PtGetGameCli(self.gameId)
        if (type(gameCli) != type(None)) and (PtIsVarSyncGame(gameCli.gameTypeID())):
            return gameCli.upcastToVarSyncGame()
        return None

    ###########################
    def OnBackdoorMsg(self, target, param):
        global quabBrainList
        global quabVarList
        global quabKeyList
        global cheater

        if target == "quabs":
            if param == "runaway":
                ## Check as many conditions as we can so we don't overload processing
                if (not self.brainsReady) or (not self.gameReady):
                    return # not ready to run AI yet
                if not self.isAIOwner:
                    return # we aren't the ones running the brain, abort
                game = self.IGetVarSyncGameCli()
                if game == None:
                    return # no game connection established

                cheater = 1

                for brain in quabBrainList:
                    if brain == None:
                        continue

                    brain.goToGoal(ptPoint3(0,-1000,700), 0)

            elif param == "reset":
                cheater = 0



