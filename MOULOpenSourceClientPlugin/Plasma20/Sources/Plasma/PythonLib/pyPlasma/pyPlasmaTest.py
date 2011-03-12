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
from pyPlasma import *
from pyPlasmaHelpers import *
from traceback import print_exc

kLogToDebugger = 32
kPeristentNode = 1
kTransientNode = 0
kQuittingGame = 7
kLinkingOut = 8
kExitingLobby = 9

#-------------------------------------

print "BEGIN"

# create client-side networking
net = ptNetClientComm()
# init log. this must be done before creating the vault manager
net.setLog("pyPlasmaTest.log", kLogToDebugger )
# create vault manager
#vault = ptPlayerVNodeMgr(net)
vault = ptAdminVNodeMgr(net)
vault.setWantGlobalSDL(1)
vault.setWantAllPlayers(1)
# create the NetClientMgr.
nc = NetClientMgr(net)
# create the VaultConnectMgr
vc = VaultConnectMgr(vault)

# startup networking
print "Net: starting up..."
net.init()
print "Net: started"

# point to lobby server
net.setActiveServer('ea1-2k',5000)
# set acct username/password
net.setAuthInfo('reseng0221','tooann42')
# specify the name of player we want to use.
nc.setDesiredPlayer('Scooby5',1)

#------------------
success = 0

while 1:
    try:
        # login to the lobby server
        if nc.login(NetClientMgr.kLobby)<0: break
        
        # connect to vault
        if vc.connect()<0: break
        # get root node
        rootNode = vault.getRootNode()
        print rootNode
        # create a template node for finding the global sdl folder node
        tmpNode = vault.createNode(PtVaultNodeTypes.kFolderNode,kTransientNode).upcastToFolderNode()
        tmpNode.setFolderType(PtVaultStandardNodes.kAllAgeGlobalSDLNodesFolder)
        # find global SDL folder
        globalSDLFolder = vault.findNode(tmpNode)
        if globalSDLFolder:
            globalSDLFolder = globalSDLFolder.upcastToFolderNode()
        print globalSDLFolder
        
        # startup an age or three (forces global sdl nodes to initialize)
        ageLink = ptAgeLinkStruct()
    #    ageLink.getAgeInfo().setAgeFilename('Teledahn')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('city')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('Personal')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('Garden')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('BaronCityOffice')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('Kadish')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('Neighborhood')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('Cleft')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    #    ageLink.getAgeInfo().setAgeFilename('Garrison')
    #    nc.startFindingAge(ageLink) # we don't need to wait around for the operation to complete
    
        # spawn a game
        ageLink.getAgeInfo().setAgeFilename('Teledahn')
        ageLink.setLinkingRules(PtLinkingRules.kOriginalBook)
        if nc.findAge(ageLink)<0: break
        serverInfo = nc.fCbArgs[0]
    
        # leave the lobby
        nc.logout(kExitingLobby)
    
        # log into the game server
        net.setActiveServer(serverInfo)
        if nc.login(NetClientMgr.kGame)<0: break
    
        # join the age
        if nc.joinAge()<0: break
    
        # done trying things
        success = 1
        break
    except:
        print_exc()
        break
    
# disconnect from vault
vc.disconnect()
# leave the server
nc.logout(kQuittingGame)
    
#------------------

# shutdown networking. only flush msgs if all went well (not required, but speeds up shutdown on error)
print "Net: shutting down..."
net.fini(success)
print "Net: shut down"


print "END"
raw_input("\npress return")
