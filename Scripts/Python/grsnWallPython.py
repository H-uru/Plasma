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
from collections import defaultdict
from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from grsnWallConstants import *
import xACAItems

##############################################################
# define the attributes/parameters that we need from the 3dsMax scene
##############################################################

NorthPanelClick = ptAttribActivator(1, "North Panel Clickables")
SouthPanelClick = ptAttribActivator(2, "South Panel Clickables")

NorthPanel = ptAttribSceneobjectList(3, "North Panel Objects", byObject=1)
SouthPanel = ptAttribSceneobjectList(4, "South Panel Objects", byObject=1)

NorthWall = ptAttribSceneobjectList(5, "North Wall", byObject=1)
SouthWall = ptAttribSceneobjectList(6, "South Wall", byObject=1)

NorthChair = ptAttribActivator(7, "North Chair")
SouthChair = ptAttribActivator(8, "South Chair")

NorthLights = ptAttribSceneobjectList(9, "North Panel Lights", byObject=1)
SouthLights = ptAttribSceneobjectList(10, "South Panel Lights", byObject=1)

NorthCountLights = ptAttribSceneobjectList(11, "North Count Lights", byObject=1)
SouthCountLights = ptAttribSceneobjectList(12, "South Count Lights", byObject=1)

upButtonSouth = ptAttribActivator(13, "South up count button")
dnButtonSouth = ptAttribActivator(14, "South down count button")
readyButtonSouth = ptAttribActivator(15, "South ready button")

upButtonNorth = ptAttribActivator(18, "North up count button")
dnButtonNorth = ptAttribActivator(19, "North down count button")
readyButtonNorth = ptAttribActivator(20, "North ready button")

goButtonNorth = ptAttribActivator(21, "North Go Button activator")
goButtonSouth = ptAttribActivator(22, "South Go Button activator")

goBtnNorthObject = ptAttribSceneobject(23, "North Go Button Object")
goBtnSouthObject = ptAttribSceneobject(24, "South Go Button Object")

NorthChairSit = ptAttribActivator(25, "North sit component")
SouthChairSit = ptAttribActivator(26, "South sit component")

fiveBtnNorth = ptAttribActivator(27, "5 btn North")
tenBtnNorth = ptAttribActivator(28, "10 btn North")
fifteenBtnNorth = ptAttribActivator(29, "15 btn North")

fiveBtnSouth = ptAttribActivator(30, "5 btn South")
tenBtnSouth = ptAttribActivator(31, "10 btn South")
fifteenBtnSouth = ptAttribActivator(32, "15 btn South")

SouthTubeOpen = ptAttribNamedResponder(33, "South tube open", netForce=1)
NorthTubeOpen = ptAttribNamedResponder(34, "North tube open", netForce=1)

SouthTubeClose = ptAttribNamedResponder(35, "South tube close", netForce=1)
NorthTubeClose = ptAttribNamedResponder(36, "North tube close", netForce=1)

SouthTubeEntry = ptAttribNamedActivator(37, "South tube entry trigger")
NorthTubeEntry = ptAttribNamedActivator(38, "North tube entry trigger")

SouthTubeMulti = ptAttribBehavior(43, "South tube entry multi", netForce=0)
NorthTubeMulti = ptAttribBehavior(44, "North tube entry multi", netForce=0)

SouthTubeExclude = ptAttribExcludeRegion(45, "South tube exclude")
NorthTubeExclude = ptAttribExcludeRegion(46, "North tube exclude")

SouthTeamWarpPt = ptAttribSceneobject(47, "South team warp point")
NorthTeamWarpPt = ptAttribSceneobject(48, "North team warp point")

SouthTeamWin = ptAttribActivator(49, "South team win")
NorthTeamWin = ptAttribActivator(50, "North team win")

SouthTeamQuit = ptAttribActivator(51, "South team quit")
NorthTeamQuit = ptAttribActivator(52, "North team quit")

SouthTeamWinTeleport = ptAttribSceneobject(53, "South team win point")
NorthTeamWinTeleport = ptAttribSceneobject(54, "North team win point")

NorthQuitBehavior = ptAttribBehavior(55, "North quit behavior", netForce=0)
SouthQuitBehavior = ptAttribBehavior(56, "South quit behavior", netForce=0)

#sfx responders
NorthPanelSound = ptAttribResponder(57,"North panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=0)
SouthPanelSound = ptAttribResponder(58,"South panel sound",['main','up','down','select','blockerOn','blockerOff','gameStart','denied'],netForce=0)

panelLightAnimSouth = ptAttribMaterialAnimation(61, "South Panel Light Animation")
panelLightAnimNorth = ptAttribMaterialAnimation(62, "North Panel Light Animation")
sitPanelLightsNorth = ptAttribSceneobjectList(63, "North sitting indicators")
sitPanelLightsSouth = ptAttribSceneobjectList(64, "South sitting indicators")
confirmPanelLightsNorth = ptAttribSceneobjectList(65, "North ready indicators")
confirmPanelLightsSouth = ptAttribSceneobjectList(66, "South ready indicators")

EscapeArtist = ptAttribActivator(67, "Escape Artist Catchall")

MaleSuit = ('03_MLegs_Suit','03_MTorso_Suit','03_MLHand_Suit','03_MRHand_Suit',None,'03_MHAcc_SuitHelmet','03_MLFoot_Suit','03_MRFoot_Suit')
FemaleSuit = ('03_FLegs_Suit','03_FTorso_Suit','03_FLHand_Suit','03_FRHand_Suit',None,'03_FHair_SuitHelmet','03_FLFoot_Suit','03_FRFoot_Suit')
    
wornItem = []
DefaultColor1 = {}
DefaultColor2 = {}

clothingTypeList = [kHairClothingItem,kFaceClothingItem,kShirtClothingItem,kRightHandClothingItem,kPantsClothingItem,kRightFootClothingItem]

kWearOriginalClothes = 0
kLinkToNorthNexus = 1
kLinkToSouthNexus = 2

PanelClick = [NorthPanelClick, SouthPanelClick]
Panel = [NorthPanel, SouthPanel]
Wall = [NorthWall, SouthWall]
Chair = [NorthChair, SouthChair]
Lights = [NorthLights, SouthLights]
CountLights = [NorthCountLights, SouthCountLights]
upButton = [upButtonNorth, upButtonSouth]
dnButton = [dnButtonNorth, dnButtonSouth]
readyButton = [readyButtonNorth, readyButtonSouth]
goButton = [goButtonNorth, goButtonSouth]
goBtnObject = [goBtnNorthObject, goBtnSouthObject]
ChairSit = [NorthChairSit, SouthChairSit]
fiveBtn = [fiveBtnNorth, fiveBtnSouth]
tenBtn = [tenBtnNorth, tenBtnSouth]
fifteenBtn = [fifteenBtnNorth, fifteenBtnSouth]
TubeOpen = [NorthTubeOpen, SouthTubeOpen]
TubeClose = [NorthTubeClose, SouthTubeClose]
TubeEntry = [NorthTubeEntry, SouthTubeEntry]
TubeMulti = [NorthTubeMulti, SouthTubeMulti]
TubeExclude = [NorthTubeExclude, SouthTubeExclude]
TeamWarpPt = [NorthTeamWarpPt, SouthTeamWarpPt]
TeamWin = [NorthTeamWin, SouthTeamWin]
TeamQuit = [NorthTeamQuit, SouthTeamQuit]
TeamWinTeleport = [NorthTeamWinTeleport, SouthTeamWinTeleport]
QuitBehavior = [NorthQuitBehavior, SouthQuitBehavior]
PanelSound = [NorthPanelSound, SouthPanelSound]
panelLightAnim = [panelLightAnimNorth, panelLightAnimSouth]
sitPanelLights = [sitPanelLightsNorth, sitPanelLightsSouth]
confirmPanelLights = [confirmPanelLightsNorth, confirmPanelLightsSouth]

##############################################################
# grsnWallPython - Valid Blocker Configuration Helper
##############################################################
# Numbered Nodes:
# 01 = starting platform
# 84 = ending panels at the top center of the wall
# 76|77|78|79|--|--|80|81|82|83
# 66|67|68|69|70|71|72|73|74|75
# 56|57|58|59|60|61|62|63|64|65
# 46|47|48|49|50|51|52|53|54|55
# -\|38|39|40|41|42|43|44|45|/-
# --|30|31|32|33|34|35|36|37|--
# --|22|23|24|25|26|27|28|29|--
# --|14|15|16|17|18|19|20|21|--
# -/|06|07|08|09|10|11|12|13|\-
# --|--|--|02|03|04|05|--|--|--

WALL_START_NODE = 1
WALL_END_NODE = 84

# Edges are numbered Node-to-Node based on map above
edgeList = [(1,3),(1,4),(2,3),(2,8),(3,4),(3,9),(4,5),(4,10),(5,11),(6,7),(6,14),\
            (7,8),(7,15),(8,9),(8,16),(9,10),(9,17),(10,11),(10,18),(11,12),(11,19),\
            (12,13),(12,20),(13,21),(14,15),(14,22),(15,16),(15,23),(16,17),(16,24),\
            (17,18),(17,25),(18,19),(18,26),(19,20),(19,27),(20,21),(20,28),(21,29),\
            (22,23),(22,30),(23,24),(23,31),(24,25),(24,32),(25,26),(25,33),(26,27),\
            (26,34),(27,28),(27,35),(28,29),(28,36),(29,37),(30,31),(30,38),(31,32),\
            (31,39),(32,33),(32,40),(33,34),(33,41),(34,35),(34,42),(35,36),(35,43),\
            (36,37),(36,44),(37,45),(38,39),(38,47),(39,40),(39,48),(40,41),(40,49),\
            (41,42),(41,50),(42,43),(42,51),(43,44),(43,52),(44,45),(44,53),(45,54),\
            (46,47),(46,56),(47,48),(47,57),(48,49),(48,58),(49,50),(49,59),(50,51),\
            (50,60),(51,52),(51,61),(52,53),(52,62),(53,54),(53,63),(54,55),(54,64),\
            (55,65),(56,57),(56,66),(57,58),(57,67),(58,59),(58,68),(59,60),(59,69),\
            (60,61),(60,70),(61,62),(61,71),(62,63),(62,72),(63,64),(63,73),(64,65),\
            (64,74),(65,75),(66,67),(66,76),(67,68),(67,77),(68,69),(68,78),(69,70),\
            (69,79),(70,71),(70,84),(71,72),(71,84),(72,73),(72,80),(73,74),(73,81),\
            (74,75),(74,82),(75,83),(76,77),(77,78),(78,79),(79,84),(80,81),(80,84),\
            (81,82),(82,83)]

# Blockers are numbered top-down with horizontal blockers first then vertical blockers
blockerToEdgeDict = {0:None,1:None,2:None,3:None,4:None,5:None,6:None,7:None,8:(76,77),\
                    9:(77,78),10:(78,79),11:(79,84),12:(80,84),13:(80,81),14:(81,82),\
                    15:(82,83),16:(66,76),17:(67,77),18:(68,78),19:(69,79),20:(70,84),\
                    21:(71,84),22:(72,80),23:(73,81),24:(74,82),25:(75,83),26:(66,67),\
                    27:(67,68),28:(68,69),29:(69,70),30:(70,71),31:(71,72),32:(72,73),\
                    33:(73,74),34:(74,75),35:(56,66),36:(57,67),37:(58,68),38:(59,69),\
                    39:(60,70),40:(61,71),41:(62,72),42:(63,73),43:(64,74),44:(65,75),\
                    45:(56,57),46:(57,58),47:(58,59),48:(59,60),49:(60,61),50:(61,62),\
                    51:(62,63),52:(63,64),53:(64,65),54:(46,56),55:(47,57),56:(48,58),\
                    57:(49,59),58:(50,60),59:(51,61),60:(52,62),61:(53,63),62:(54,64),\
                    63:(55,65),64:(46,47),65:(47,48),66:(48,49),67:(49,50),68:(50,51),\
                    69:(51,52),70:(52,53),71:(53,54),72:(54,55),73:None,74:(38,47),\
                    75:(39,48),76:(40,49),77:(41,50),78:(42,51),79:(43,52),80:(44,53),\
                    81:(45,54),82:None,83:None,84:(38,39),85:(39,40),86:(40,41),87:(41,42),\
                    88:(42,43),89:(43,44),90:(44,45),91:None,92:(30,38),93:(31,39),\
                    94:(32,40),95:(33,41),96:(34,42),97:(35,43),98:(36,44),99:(37,45),\
                    100:None,101:(30,31),102:(31,32),103:(32,33),104:(33,34),105:(34,35),\
                    106:(35,36),107:(36,37),108:None,109:(22,30),110:(23,31),111:(24,32),\
                    112:(25,33),113:(26,34),114:(27,35),115:(28,36),116:(29,37),117:None,\
                    118:(22,23),119:(23,24),120:(24,25),121:(25,26),122:(26,27),123:(27,28),\
                    124:(28,29),125:None,126:(14,22),127:(15,23),128:(16,24),129:(17,25),\
                    130:(18,26),131:(19,27),132:(20,28),133:(21,29),134:None,135:(14,15),\
                    136:(15,16),137:(16,17),138:(17,18),139:(18,19),140:(19,20),141:(20,21),\
                    142:None,143:(6,14),144:(7,15),145:(8,16),146:(9,17),147:(10,18),\
                    148:(11,19),149:(12,20),150:(13,21),151:None,152:(6,7),153:(7,8),\
                    154:(8,9),155:(9,10),156:(10,11),157:(11,12),158:(12,13),159:None,\
                    160:None,161:(2,8),162:(3,9),163:(4,10),164:(5,11),165:None,166:None,\
                    167:(2,3),168:(3,4),169:(4,5),170:None}

# find if there is a path from start node to end node
def DepthFirstSearch(adjacencyList, startNode, endNode, visitedNodes=None): 
    # initialize for root
    if visitedNodes is None:
        visitedNodes = []

    visitedNodes.append(startNode)

    if startNode == endNode:
        return True

    if startNode in adjacencyList:
        for adjNode in adjacencyList[startNode]:
            if adjNode not in visitedNodes:
                val = DepthFirstSearch(adjacencyList, adjNode, endNode, visitedNodes)
                if val:
                    return True

    return False

##############################################################
# grsnWallPython
##############################################################

class grsnWallPython(ptResponder):

    def __init__(self):
        PtDebugPrint("grsnWallPython::init")
        ptResponder.__init__(self)
        self.id = 52392
        self.version = 5
        self.BlockerCount = 0
        self.oldCount = [0, 0]

    def IAmMaster(self):
        return (self.sceneobject.isLocallyOwned())

    def OnServerInitComplete(self):
        PtDebugPrint("grsnWallPython::OnServerInitComplete")
        solo = not bool(PtGetPlayerList())
        ageSDL = PtGetAgeSDL()

        #Prepare SDLs (n-North, s-South)
        ageSDL.setFlags("nChairOccupant",1,1)
        ageSDL.setFlags("sChairOccupant",1,1)
        ageSDL.setFlags("nWallPlayer",1,1)
        ageSDL.setFlags("sWallPlayer",1,1)
        ageSDL.setFlags("nState",1,1)
        ageSDL.setFlags("sState",1,1)
        ageSDL.setFlags("NumBlockers",1,1)
        ageSDL.setFlags("northWall",1,1)
        ageSDL.setFlags("southWall",1,1)

        ageSDL.setNotify(self.key,"nState",0.0)
        ageSDL.setNotify(self.key,"sState",0.0)
        ageSDL.setNotify(self.key,"NumBlockers",0.0)
        ageSDL.setNotify(self.key,"northWall",0.0)
        ageSDL.setNotify(self.key,"southWall",0.0)
        ageSDL.setNotify(self.key,"nWallPlayer",0.0)
        ageSDL.setNotify(self.key,"sWallPlayer",0.0)

        ageSDL.sendToClients("nChairOccupant")
        ageSDL.sendToClients("sChairOccupant")
        ageSDL.sendToClients("nWallPlayer")
        ageSDL.sendToClients("sWallPlayer")
        ageSDL.sendToClients("nState")
        ageSDL.sendToClients("sState")
        ageSDL.sendToClients("NumBlockers")
        ageSDL.sendToClients("northWall")
        ageSDL.sendToClients("southWall")

        if(solo):
            PtDebugPrint("grsnWallPython::OnServerInitComplete: I am alone :(")
            self.ResetWall()
        else:
            PtDebugPrint("grsnWallPython::OnServerInitComplete: There is already another Game Master - Request Game Update")
            self.RequestGameUpdate()

    def OnNotify(self,state,id,events):
        global wornItem
        global DefaultColor1
        global DefaultColor2
        cID = PtGetLocalClientID()
        ageSDL = PtGetAgeSDL()
        
        ### Sit down ###
        if(id == NorthChair.id and state):
            if(ageSDL["nState"][0] == kStandby or ageSDL["nState"][0] == kEnd):
                self.ChangeGameState(kNorth, kSit)
            NorthChair.disable()
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                ageSDL["nChairOccupant"] = (cID,)
            return
        if(id == SouthChair.id and state):
            if(ageSDL["sState"][0] == kStandby or ageSDL["sState"][0] == kEnd):
                self.ChangeGameState(kSouth, kSit)
            SouthChair.disable()
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                ageSDL["sChairOccupant"] = (cID,)
            return
        ### Stand up ###
        if(id == NorthChairSit.id):
            for event in events:
                if(event[0] == kContainedEvent and event[1] == 1 and not state):
                    NorthChair.enable()
                    if(ageSDL["nState"][0] == kSit):
                        self.ChangeGameState(kNorth, kStandby)
                        self.SetPanelMode(kNorth, disableAll=True)
                    if(self.IAmMaster()):
                        ageSDL["nChairOccupant"] = (-1,)
            return
        if(id == SouthChairSit.id):
            for event in events:
                if(event[0] == kContainedEvent and event[1] == 1 and not state):
                    SouthChair.enable()
                    if(ageSDL["sState"][0] == kSit):
                        self.ChangeGameState(kSouth, kStandby)
                        self.SetPanelMode(kSouth, disableAll=True)
                    if(self.IAmMaster()):
                        ageSDL["sChairOccupant"] = (-1,)
            return
        ### Press Go Button ###
        if(id == goButtonNorth.id and not state):
            ### Start Game ###
            if(ageSDL["nState"][0] == kSit):
                self.ResetWall(resetState=False)
                self.ChangeGameState(kNorth, kSelectCount)
                #NorthPanelSound.run(self.key, state='main')
                if(eventHandler):
                    eventHandler.Handle(kEventInit)
            ### Confirm Blocker ###
            elif(ageSDL["nState"][0] == kSetBlocker):
                if(self.GetNumBlockerSet(kNorth) < ageSDL["NumBlockers"][0] or ageSDL["sState"][0] < kSetBlocker):
                    NorthPanelSound.run(self.key, state='denied')
                else:
                    self.ChangeGameState(kNorth, kWait)
                    NorthTubeOpen.run(self.key)
                    NorthPanelSound.run(self.key, state='gameStart')
            return
        if(id == goButtonSouth.id and not state):
            ### Start Game ###
            if(ageSDL["sState"][0] == kSit):
                self.ResetWall(resetState=False)
                self.ChangeGameState(kSouth, kSelectCount)
                #SouthPanelSound.run(self.key, state='main')
                if(eventHandler):
                    eventHandler.Handle(kEventInit)
            ### Confirm Blocker ###
            elif(ageSDL["sState"][0] == kSetBlocker):
                if(self.GetNumBlockerSet(kSouth) < ageSDL["NumBlockers"][0] or ageSDL["nState"][0] < kSetBlocker):
                    SouthPanelSound.run(self.key, state='denied')
                else:
                    self.ChangeGameState(kSouth, kWait)
                    SouthTubeOpen.run(self.key)
                    SouthPanelSound.run(self.key, state='gameStart')
            return
        ### Tube open Animation Finished ###
        if(id == NorthTubeOpen.id):
            NorthTubeExclude.release(self.key)
            return
        if(id == SouthTubeOpen.id):
            SouthTubeExclude.release(self.key)
            return
        ### Blocker Count Buttons ###
        if(id == fiveBtnNorth.id and state and ageSDL["nState"][0] == kSelectCount):
            NorthPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(5)
            return
        if(id == tenBtnNorth.id and state and ageSDL["nState"][0] == kSelectCount):
            NorthPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(10)
            return
        if(id == fifteenBtnNorth.id and state and ageSDL["nState"][0] == kSelectCount):
            NorthPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(15)
            return
        if(id == upButtonNorth.id and state and ageSDL["nState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] >= 20):
                NorthPanelSound.run(self.key, state='denied')
            else:
                NorthPanelSound.run(self.key, state='up')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] + 1)
            return
        if(id == dnButtonNorth.id and state and ageSDL["nState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] <= 0):
                NorthPanelSound.run(self.key, state='denied')
            else:
                NorthPanelSound.run(self.key, state='down')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] - 1)
            return
        #South
        if(id == fiveBtnSouth.id and state and ageSDL["sState"][0] == kSelectCount):
            SouthPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(5)
            return
        if(id == tenBtnSouth.id and state and ageSDL["sState"][0] == kSelectCount):
            SouthPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(10)
            return
        if(id == fifteenBtnSouth.id and state and ageSDL["sState"][0] == kSelectCount):
            SouthPanelSound.run(self.key, state='up')
            self.ChangeBlockerCount(15)
            return
        if(id == upButtonSouth.id and state and ageSDL["sState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] >= 20):
                SouthPanelSound.run(self.key, state='denied')
            else:
                SouthPanelSound.run(self.key, state='up')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] + 1)
            return
        if(id == dnButtonSouth.id and state and ageSDL["sState"][0] == kSelectCount):
            if(ageSDL["NumBlockers"][0] <= 0):
                SouthPanelSound.run(self.key, state='denied')
            else:
                SouthPanelSound.run(self.key, state='down')
                self.ChangeBlockerCount(ageSDL["NumBlockers"][0] - 1)
            return
        ### Confirm count Button ###
        if(id == readyButtonNorth.id and not state and ageSDL["nState"][0] == kSelectCount):
            self.ChangeGameState(kNorth, kSetBlocker)
            NorthPanelSound.run(self.key, state='up')
            return
        if(id == readyButtonSouth.id and not state and ageSDL["sState"][0] == kSelectCount):
            self.ChangeGameState(kSouth, kSetBlocker)
            SouthPanelSound.run(self.key, state='up')
            return
        ### Tube Entry trigger ###
        if(id == NorthTubeEntry.id and ageSDL["nState"][0] >= kWait and ageSDL["nState"][0] != kEnd):
            if ageSDL["nState"][0] == kWait:
                self.ChangeGameState(kNorth, kEntry)
            if(ageSDL["nWallPlayer"] == (-1,)):
                self.ChangeChuteState(kNorth, PtGetClientIDFromAvatarKey(PtFindAvatar(events).getKey()))
                if(PtFindAvatar(events) == PtGetLocalAvatar() and cID == ageSDL["sWallPlayer"][0]):
                    ageSDL["sWallPlayer"] = (-1,)
            return
        if(id == SouthTubeEntry.id and ageSDL["sState"][0] >= kWait and ageSDL["sState"][0] != kEnd):
            if ageSDL["sState"][0] == kWait:
                self.ChangeGameState(kSouth, kEntry)
            if(ageSDL["sWallPlayer"] == (-1,)):
                self.ChangeChuteState(kSouth, PtGetClientIDFromAvatarKey(PtFindAvatar(events).getKey()))
                if(PtFindAvatar(events) == PtGetLocalAvatar() and cID == ageSDL["nWallPlayer"][0]):
                    ageSDL["nWallPlayer"] = (-1,)
            return
        ### Tube Multibehaviors ###
        if(id == NorthTubeMulti.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    #Smart seek complete
                    NorthTubeClose.run(self.key)
                    if(PtFindAvatar(events) != PtGetLocalAvatar()):
                        NorthTubeExclude.clear(self.key)
                elif(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage):
                    #Multistage complete
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        avatar = PtGetLocalAvatar()
                        currentgender = avatar.avatar.getAvatarClothingGroup()
                        if currentgender == kFemaleClothingGroup:
                            clothing = FemaleSuit
                        else:
                            clothing = MaleSuit
                        if not wornItem:
                            wornItem = avatar.avatar.getAvatarClothingList()
                            DefaultColor1 = {item[1]: avatar.avatar.getTintClothingItem(item[0],1) for item in wornItem}
                            DefaultColor2 = {item[1]: avatar.avatar.getTintClothingItem(item[0],2) for item in wornItem}
                        for index, item in enumerate(clothing):
                            if item == None:
                                continue
                            avatar.avatar.netForce(1)
                            avatar.avatar.wearClothingItem(item, 0)
                            avatar.avatar.tintClothingItem(item, DefaultColor1[index], 0)
                            avatar.avatar.tintClothingItemLayer(item, DefaultColor2[index], 2, 1)
                        PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                        PtGetLocalAvatar().physics.warpObj(SouthTeamWarpPt.value.getKey())
                        ageSDL["nWallPlayer"] = (-1,)
                    if (ageSDL["nState"][0] == kEntry):
                        self.ChangeGameState(kNorth, kGameInProgress)
                        self.ChangeGameState(kSouth, kGameInProgress)
                    NorthTubeOpen.run(self.key)
            return
        if(id == SouthTubeMulti.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage):
                    #Smart seek complete
                    SouthTubeClose.run(self.key)
                    if(PtFindAvatar(events) != PtGetLocalAvatar()):
                        SouthTubeExclude.clear(self.key)
                elif(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kAdvanceNextStage):
                    #Multistage complete
                    if(PtFindAvatar(events) == PtGetLocalAvatar()):
                        avatar = PtGetLocalAvatar()
                        currentgender = avatar.avatar.getAvatarClothingGroup()
                        if currentgender == kFemaleClothingGroup:
                            clothing = FemaleSuit
                        else:
                            clothing = MaleSuit
                        if not wornItem:
                            wornItem = avatar.avatar.getAvatarClothingList()
                            DefaultColor1 = {item[1]: avatar.avatar.getTintClothingItem(item[0],1) for item in wornItem}
                            DefaultColor2 = {item[1]: avatar.avatar.getTintClothingItem(item[0],2) for item in wornItem}
                        for index, item in enumerate(clothing):
                            if item == None:
                                continue
                            avatar.avatar.netForce(1)
                            avatar.avatar.wearClothingItem(item, 0)
                            avatar.avatar.tintClothingItem(item, DefaultColor1[index], 0)
                            avatar.avatar.tintClothingItemLayer(item, DefaultColor2[index], 2, 1)
                        PtSendKIMessage(kDisableEntireYeeshaBook, 0)
                        PtGetLocalAvatar().physics.warpObj(NorthTeamWarpPt.value.getKey())
                        ageSDL["sWallPlayer"] = (-1,)
                    if (ageSDL["sState"][0] == kEntry):
                        self.ChangeGameState(kNorth, kGameInProgress)
                        self.ChangeGameState(kSouth, kGameInProgress)
                    SouthTubeOpen.run(self.key)
            return
        ### Win region ###
        if(id == NorthTeamWin.id):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtAtTimeCallback(self.key, 3.0, kLinkToNorthNexus)
                if (ageSDL["grsnGrantMaintainerSuit"][0] and ageSDL["nState"][0] != kEnd and PtGetPlayerList()):
                    avatar = PtGetLocalAvatar()
                    currentgender = avatar.avatar.getAvatarClothingGroup()
                    if currentgender == kFemaleClothingGroup:
                        clothing = FemaleSuit
                    else:
                        clothing = MaleSuit
                    for item in filter(None, clothing):
                        if (not self.IItemInCloset(avatar, item)):
                            PtDebugPrint(f'DEBUG: grsnWallPython.OnNotify():  Adding {item} to your closet and wearing it.')
                            avatar.avatar.addWardrobeClothingItem(item, ptColor().white(), ptColor().white())
                            avatar.avatar.saveClothing()
                            wornItem = []
                            item = self.IGetItem(item)
                            if hasattr(item, "description"):
                                PtSendKIMessage(kKILocalChatStatusMsg,PtGetLocalizedString("KI.Messages.NewClothing", [item.description]))
            if (ageSDL["nState"][0] != kEnd):
                if(eventHandler):
                    eventHandler.Handle(kEventNorthWin)
                self.ChangeGameState(kNorth, kEnd)
                self.ChangeGameState(kSouth, kEnd)
            return
        if(id == SouthTeamWin.id):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtAtTimeCallback(self.key, 3.0, kLinkToSouthNexus)
                if (ageSDL["grsnGrantMaintainerSuit"][0] and ageSDL["sState"][0] != kEnd and PtGetPlayerList()):
                    avatar = PtGetLocalAvatar()
                    currentgender = avatar.avatar.getAvatarClothingGroup()
                    if currentgender == kFemaleClothingGroup:
                        clothing = FemaleSuit
                    else:
                        clothing = MaleSuit
                    for item in filter(None, clothing):
                        if (not self.IItemInCloset(avatar, item)):
                            PtDebugPrint(f'DEBUG: grsnWallPython.OnNotify():  Adding {item} to your closet and wearing it.')
                            avatar.avatar.addWardrobeClothingItem(item, ptColor().white(), ptColor().white())
                            avatar.avatar.saveClothing()
                            wornItem = []
                            item = self.IGetItem(item)
                            if hasattr(item, "description"):
                                PtSendKIMessage(kKILocalChatStatusMsg,PtGetLocalizedString("KI.Messages.NewClothing", [item.description]))
            if (ageSDL["sState"][0] != kEnd):
                if(eventHandler):
                    eventHandler.Handle(kEventSouthWin)             
                self.ChangeGameState(kNorth, kEnd)
                self.ChangeGameState(kSouth, kEnd)
            return
        ### Quit button ###
        if(id == NorthTeamQuit.id and state):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(NorthQuitBehavior.value, self.key, NorthQuitBehavior.netForce)
            if (ageSDL["nState"][0] != kEnd):
                if(eventHandler):
                    eventHandler.Handle(kEventNorthQuit)
                self.ChangeGameState(kNorth, kEnd)
                self.ChangeGameState(kSouth, kEnd)
            return
        if(id == SouthTeamQuit.id and state):
            if(PtFindAvatar(events) == PtGetLocalAvatar()):
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(SouthQuitBehavior.value, self.key, SouthQuitBehavior.netForce)
            if (ageSDL["sState"][0] != kEnd):
                if(eventHandler):
                    eventHandler.Handle(kEventSouthQuit)
                self.ChangeGameState(kNorth, kEnd)
                self.ChangeGameState(kSouth, kEnd)
            return
        if(id == NorthQuitBehavior.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and PtFindAvatar(events) == PtGetLocalAvatar()):
                    #Touched the Crystal
                    PtDebugPrint("Trigger North Crystal")
                    PtAtTimeCallback(self.key, 3.0, kLinkToNorthNexus)
            return
        if(id == SouthQuitBehavior.id):
            for event in events:
                if(event[0] == kMultiStageEvent and event[1] == 0 and event[2] == kEnterStage and PtFindAvatar(events) == PtGetLocalAvatar()):
                    #Touched the Crystal
                    PtDebugPrint("Trigger South Crystal")
                    PtAtTimeCallback(self.key, 3.0, kLinkToSouthNexus)
            return
        #Check for crafty individuals who try to escape the building while still wearing a suit
        if(id == EscapeArtist.id and PtFindAvatar(events) == PtGetLocalAvatar()):
            PtAtTimeCallback(self.key, 2.0, kWearOriginalClothes)
            PtSendKIMessage(kEnableEntireYeeshaBook, 0)
            return

        ### Blocker ###
        for event in events:
            if(event[0] == kPickedEvent and event[1] == 1):
                pickedBlockerObj = event[3]
                team = kNorth
                try:
                    index = NorthPanel.value.index(pickedBlockerObj)
                except:
                    try:
                        index = SouthPanel.value.index(pickedBlockerObj)
                        team = kSouth
                    except:
                        PtDebugPrint("grsnWallPython::OnNotify: Blocker not found on either panel")
                        return
                if(team == kNorth and ageSDL["nState"][0] == kSetBlocker and cID == ageSDL["nChairOccupant"][0]):
                    self.SetPanelBlocker(kNorth, index, not self.FindBlocker(kNorth, index))
                if(team == kSouth and ageSDL["sState"][0] == kSetBlocker and cID == ageSDL["sChairOccupant"][0]):
                    self.SetPanelBlocker(kSouth, index, not self.FindBlocker(kSouth, index))


    def IItemInCloset(self, avatar, clothingName):
        clothingList = avatar.avatar.getWardrobeClothingList()
        for item in clothingList:
            if clothingName == item[0]:
                return True
        return False

    def OnTimer(self,id):
        avatar = PtGetLocalAvatar()
        if(id == kWearOriginalClothes and wornItem):
            for item in wornItem:
                avatar.avatar.netForce(True)
                avatar.avatar.wearClothingItem(item[0], 0)
                avatar.avatar.tintClothingItem(item[0], DefaultColor1[item[1]], 0)
                avatar.avatar.tintClothingItemLayer(item[0], DefaultColor2[item[1]], 2, 1)
        elif(id == kLinkToNorthNexus):
            PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), NorthTeamWinTeleport.value.getKey())
            PtFadeOut(1,True,True)
        elif(id == kLinkToSouthNexus):
            PtFakeLinkAvatarToObject(PtGetLocalAvatar().getKey(), SouthTeamWinTeleport.value.getKey())
            PtFadeOut(1,True,True)

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        ageSDL = PtGetAgeSDL()
        value = ageSDL[VARname][0]
        cID = PtGetLocalClientID()
        PtDebugPrint("grsn::OnSDLNotify: received SDL '%s' with value %d" % (VARname, value))
        if(VARname == "NumBlockers"):
            if(ageSDL["nState"][0] == kSetBlocker):
                #South wants another count - set North back to kSelectCount
                self.ChangeGameState(kNorth, kSelectCount)
            if(ageSDL["sState"][0] == kSetBlocker):
                #same thing
                self.ChangeGameState(kSouth, kSelectCount)
            self.SetPanelMode(kNorth,onlNorthLights=True)
            self.SetPanelMode(kSouth,onlNorthLights=True)
        ### BlockerCount ###
        if(VARname == "northWall"):
            self.SetPanelMode(kNorth, onlNorthLights=True)
        if(VARname == "southWall"):
            self.SetPanelMode(kSouth, onlNorthLights=True)
        ### nState ###
        if(VARname == "nState"):
            self.PanelLight()
            if(value == kStandby):
                self.SetPanelMode(kNorth)
            if(value == kSit):
                self.SetPanelMode(kNorth)
            if(value == kSelectCount):
                if(ageSDL["sState"][0] != kSelectCount):
                    #Force South to keep up
                    self.ChangeGameState(kSouth, kSelectCount)
                self.SetPanelMode(kNorth)
            if(value == kSetBlocker and ageSDL["sState"][0] == kSetBlocker):
                #Both players are ok with the blocker count
                NorthPanelSound.run(self.key, state='select')
                SouthPanelSound.run(self.key, state='select')
                self.SetPanelMode(kNorth)
                self.SetPanelMode(kSouth)
            if(value == kWait):
                #North player is ready - transmit blockers to the wall
                for blocker in ageSDL["northWall"]:
                    NorthWall.value[blocker].physics.enable()
                if(ageSDL["sState"][0] >= kWait and eventHandler):
                    eventHandler.Handle(kEventEntry)

        if(VARname == "sState"):
            self.PanelLight()
            if(value == kStandby):
                self.SetPanelMode(kSouth)
            if(value == kSit):
                self.SetPanelMode(kSouth)
            if(value == kSelectCount):
                if(ageSDL["nState"][0] != kSelectCount):
                    #Force North to keep up
                    self.ChangeGameState(kNorth, kSelectCount)
                self.SetPanelMode(kSouth)
            if(value == kSetBlocker and ageSDL["nState"][0] == kSetBlocker):
                #Both players are ok with the blocker count
                NorthPanelSound.run(self.key, state='select')
                SouthPanelSound.run(self.key, state='select')
                self.SetPanelMode(kNorth)
                self.SetPanelMode(kSouth)
            if(value == kWait):
                #South player is ready - transmit blockers to the wall
                for blocker in ageSDL["southWall"]:
                    SouthWall.value[blocker].physics.enable()
                if(ageSDL["nState"][0] >= kWait and eventHandler):
                    eventHandler.Handle(kEventEntry)
                    
        if(VARname == "nWallPlayer" and ageSDL["nWallPlayer"] != (-1,)):
            if(ageSDL["sState"][0] == kEntry):
                if(eventHandler):
                    eventHandler.Handle(kEventStart)
                #both players are in the Tube - flush them down
                if(cID == ageSDL["nWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(NorthTubeMulti.value, self.key, NorthTubeMulti.netForce)
                if(cID == ageSDL["sWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(SouthTubeMulti.value, self.key, SouthTubeMulti.netForce)
            elif(ageSDL["nState"][0] == kGameInProgress and cID == ageSDL["nWallPlayer"][0]):
                #additional player joining Game In Progress
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(NorthTubeMulti.value, self.key, NorthTubeMulti.netForce)

        if(VARname == "sWallPlayer" and ageSDL["sWallPlayer"] != (-1,)):
            if(ageSDL["nState"][0] == kEntry):
                if(eventHandler):
                    eventHandler.Handle(kEventStart)
                #both players are in the Tube - flush them down
                if(cID == ageSDL["nWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(NorthTubeMulti.value, self.key, NorthTubeMulti.netForce)
                if(cID == ageSDL["sWallPlayer"][0]):
                    PtGetLocalAvatar().avatar.runBehaviorSetNotify(SouthTubeMulti.value, self.key, SouthTubeMulti.netForce)
            elif(ageSDL["sState"][0] == kGameInProgress and cID == ageSDL["sWallPlayer"][0]):
                #additional player joining Game In Progress
                PtGetLocalAvatar().avatar.runBehaviorSetNotify(SouthTubeMulti.value, self.key, SouthTubeMulti.netForce)

    def FindBlocker(self,team,id):
        ageSDL = PtGetAgeSDL()
        blockerFound = False
        if team == kNorth:
            for blocker in ageSDL["northWall"]:
                if(blocker == id):
                    blockerFound = True
                    break
        elif team == kSouth:
            for blocker in ageSDL["southWall"]:
                if(blocker==id):
                    blockerFound = True
                    break
        return blockerFound

    def SetPanelMode(self,team,disableAll=False,onlNorthLights=False):
        ageSDL = PtGetAgeSDL()
        if(team == kNorth):
            state = ageSDL["nState"][0]
        elif(team == kSouth):
            state = ageSDL["sState"][0]
        if(onlNorthLights):
            if(team == kNorth):
                sdl = "northWall"
            elif(team == kSouth):
                sdl = "southWall"

            if(state == kSelectCount):
                for i in range(0,20):
                    if(i < ageSDL["NumBlockers"][0]):
                        CountLights[team].value[i].runAttachedResponder(kRedFlash)
                    elif((team == kNorth and i < self.oldCount[team]) or (team == kSouth and i < self.oldCount[team])):
                        CountLights[team].value[i].runAttachedResponder(kRedOff)
                self.oldCount[team] = ageSDL["NumBlockers"][0]
            elif(state >= kSetBlocker):
                for i in range(0,ageSDL["NumBlockers"][0]):
                    if(i < self.GetNumBlockerSet(team)):
                        CountLights[team].value[i].runAttachedResponder(kTeamLightsOn)
                    else:
                        CountLights[team].value[i].runAttachedResponder(kRedOn)
            return

        PtDebugPrint("grsnWallPython::SetPanelMode: %s - %d" % (team, state))
        if(state == kStandby or state == kEnd or disableAll):
            for i in range(0,20):
                CountLights[team].value[i].runAttachedResponder(kRedOff)
            goBtnObject[team].value.runAttachedResponder(kDim)
            goButton[team].disable()
            upButton[team].disable()
            dnButton[team].disable()
            readyButton[team].disable()
            fiveBtn[team].disable()
            tenBtn[team].disable()
            fifteenBtn[team].disable()
        elif(state == kSit):
            goBtnObject[team].value.runAttachedResponder(kBright)
            goButton[team].enable()
            upButton[team].disable()
            dnButton[team].disable()
            readyButton[team].disable()
            fiveBtn[team].disable()
            tenBtn[team].disable()
            fifteenBtn[team].disable()
        elif(state == kSelectCount):
            for i in range(0,171):
                self.SetPanelBlocker(team, i, False, onlyLight=True)
                Panel[team].value[i].physics.disable()
            goBtnObject[team].value.runAttachedResponder(kDim)
            goButton[team].disable()
            upButton[team].enable()
            dnButton[team].enable()
            readyButton[team].enable()
            fiveBtn[team].enable()
            tenBtn[team].enable()
            fifteenBtn[team].enable()
            for i in range(0,20):
                if(i < ageSDL["NumBlockers"][0]):
                    CountLights[team].value[i].runAttachedResponder(kRedFlash)
                else:
                    CountLights[team].value[i].runAttachedResponder(kRedOff)
        elif(state == kSetBlocker):
            goBtnObject[team].value.runAttachedResponder(kPulse)
            goButton[team].enable()
            upButton[team].disable()
            dnButton[team].disable()
            readyButton[team].disable()
            fiveBtn[team].disable()
            tenBtn[team].disable()
            fifteenBtn[team].disable()
            for i in range(0,ageSDL["NumBlockers"][0]):
                if(i < self.GetNumBlockerSet(team)):
                    CountLights[team].value[i].runAttachedResponder(kTeamLightsOn)
                else:
                    CountLights[team].value[i].runAttachedResponder(kRedOn)
            for i in range(0,171):
                Panel[team].value[i].physics.enable()
        else:
            goBtnObject[team].value.runAttachedResponder(kDim)
            goButton[team].disable()
            for i in range(0,171):
                Panel[team].value[i].physics.disable()

    def ChangeGameState(self,team,state):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            PtDebugPrint("grsnWallPython::ChangeGameState: New State %d for team %s" % (state, team))
            if(team == kNorth):
                ageSDL["nState"] = (state,)
            elif(team == kSouth):
                ageSDL["sState"] = (state,)

    def ChangeChuteState(self,team,cID):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            if(team == kNorth and ageSDL["nWallPlayer"] != cID):
                PtDebugPrint("grsnWallPython::ChangeChuteState: New PlayerID %d for nWallPlayer" % (cID))
                ageSDL["nWallPlayer"] = (cID,)
            elif(team == kSouth and ageSDL["sWallPlayer"] != cID):
                PtDebugPrint("grsnWallPython::ChangeChuteState: New PlayerID %d for sWallPlayer" % (cID))
                ageSDL["sWallPlayer"] = (cID,)

    def PanelLight(self):
        ageSDL = PtGetAgeSDL()
        nState = ageSDL["nState"][0]
        sState = ageSDL["sState"][0]

        panelLightAnimSouth.animation.speed(0.25)
        panelLightAnimSouth.animation.stop()
        panelLightAnimSouth.animation.skipToTime(0.16)
        panelLightAnimNorth.animation.speed(0.25)
        panelLightAnimNorth.animation.stop()
        panelLightAnimNorth.animation.skipToTime(0.16)

        if(nState == kSetBlocker):
            panelLightAnimNorth.animation.play()
        if(sState == kSetBlocker):
            panelLightAnimSouth.animation.play()

        for i in range(0,2):
            if(nState == kSit or nState == kSelectCount):
                sitPanelLightsNorth.value[i].draw.enable()
                confirmPanelLightsNorth.value[i].draw.disable()
            elif(nState == kWait or nState == kEntry or nState == kGameInProgress):
                sitPanelLightsNorth.value[i].draw.enable()
                confirmPanelLightsNorth.value[i].draw.enable()
            elif(nState == kStandby or nState == kEnd):
                sitPanelLightsNorth.value[i].draw.disable()
                confirmPanelLightsNorth.value[i].draw.disable()

            if(sState == kSit or sState == kSelectCount):
                sitPanelLightsSouth.value[i].draw.enable()
                confirmPanelLightsSouth.value[i].draw.disable()
            elif(sState == kWait or sState == kEntry or sState == kGameInProgress):
                sitPanelLightsSouth.value[i].draw.enable()
                confirmPanelLightsSouth.value[i].draw.enable()
            elif(sState == kStandby or sState == kEnd):
                sitPanelLightsSouth.value[i].draw.disable()
                confirmPanelLightsSouth.value[i].draw.disable()

    def ChangeBlockerCount(self,num):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            ageSDL["NumBlockers"] = (num,)

    def RequestGameUpdate(self):
        ageSDL = PtGetAgeSDL()
        self.SetPanelMode(kNorth)
        self.SetPanelMode(kSouth)
        for i in range(0,171):
            NorthWall.value[i].physics.disable()
            SouthWall.value[i].physics.disable()
        for blocker in ageSDL["northWall"]:
            if(blocker != -1):
                self.SetPanelBlocker(kNorth, blocker, True, onlyLight=True)
                NorthWall.value[blocker].physics.enable()
        for blocker in ageSDL["southWall"]:
            if(blocker != -1):
                self.SetPanelBlocker(kSouth, blocker, True, onlyLight=True)
                SouthWall.value[blocker].physics.enable()
        self.oldCount[0] = ageSDL["NumBlockers"][0]
        self.oldCount[1] = ageSDL["NumBlockers"][0]

        NorthTubeExclude.clear(self.key)
        SouthTubeExclude.clear(self.key)

        if(ageSDL["nState"][0] == kWait or ageSDL["nState"][0] == kEntry or ageSDL["nState"][0] == kGameInProgress):
            NorthTubeOpen.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        else:
            NorthTubeClose.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        if(ageSDL["sState"][0] == kWait or ageSDL["sState"][0] == kEntry or ageSDL["sState"][0] == kGameInProgress):
            SouthTubeOpen.run(self.key, netForce=0, netPropagate=0, fastforward=1)
        else:
            SouthTubeClose.run(self.key, netForce=0, netPropagate=0, fastforward=1)

        self.PanelLight()

    def SetPanelBlocker(self,team,id,on,onlyLight=False):
        ageSDL = PtGetAgeSDL()
        if(onlyLight):
            if(on):
                Lights[team].value[id].runAttachedResponder(kTeamLightsOn)
            else:
                Lights[team].value[id].runAttachedResponder(kTeamLightsOff)
            return

        if(team == kNorth):
            sdl = "northWall"
        else:
            sdl = "southWall"

        if(on):
            idx = self.GetNumBlockerSet(team) #push it to the end
            if(idx >= ageSDL["NumBlockers"][0]): #all Blockers set
                PanelSound[team].run(self.key, state='denied')
            else:
                ageSDL.setIndex(sdl,idx,id)

                if self.CheckBlockersValid(sdl,ageSDL):
                    PanelSound[team].run(self.key, state='blockerOn')
                    self.SetPanelBlocker(team, id, True, onlyLight=True)
                else:
                    PanelSound[team].run(self.key, state='denied')
                    ageSDL.setIndex(sdl,idx,-1)
        else:
            wall = list(ageSDL[sdl])
            try:
                idx = wall.index(id)
            except ValueError:
                PtDebugPrint("grsnWallPython::SetPanelBlocker: Blocker %d not found in %s" % (id, sdl))
            else:
                # Remove this blocker from the list and append an unset one to the end of the list.
                # This ensures all blockers are contiguous at the start of the list.
                wall.pop(idx)
                wall.append(-1)
                if(self.IAmMaster()):
                    ageSDL[sdl] = tuple(wall)
                PanelSound[team].run(self.key, state='blockerOff')
                self.SetPanelBlocker(team, id, False, onlyLight=True)

    def GetNumBlockerSet(self,team):
        ageSDL = PtGetAgeSDL()
        if(team == kNorth):
            sdl = "northWall"
        elif(team == kSouth):
            sdl = "southWall"

        try:
            return list(ageSDL[sdl]).index(-1)
        except ValueError:
             return 20

    def ResetWall(self,resetState=True):
        if(self.IAmMaster()):
            ageSDL = PtGetAgeSDL()
            ageSDL["northWall"] = (-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,)
            ageSDL["southWall"] = (-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,)

            ageSDL["nWallPlayer"] = (-1,)
            ageSDL["sWallPlayer"] = (-1,)
            ageSDL["NumBlockers"] = (0,)

            if(resetState):
                ageSDL["nState"] = (kStandby,)
                ageSDL["sState"] = (kStandby,)

        for i in range(0,171):
            NorthWall.value[i].physics.disable()
            SouthWall.value[i].physics.disable()
            self.SetPanelBlocker(kNorth, i, False, onlyLight=True)
            self.SetPanelBlocker(kSouth, i, False, onlyLight=True)

        self.SetPanelMode(kNorth)
        self.SetPanelMode(kSouth)

        NorthTubeExclude.clear(self.key)
        SouthTubeExclude.clear(self.key)
        NorthTubeClose.run(self.key,fastforward=1)
        SouthTubeClose.run(self.key,fastforward=1)

        self.oldCount[0] = 0
        self.oldCount[1] = 0

        if(eventHandler):
            eventHandler.Reset()

        self.PanelLight()

    def OnBackdoorMsg(self, target, param):
        ageSDL = PtGetAgeSDL()
        if target == "wallreset":
            self.ResetWall()
        elif target == "wallprep":
            ageSDL["NumBlockers"] = (int(param),)
        elif target == "wallstart":
            ageSDL["nState"] = (kSetBlocker,)
            ageSDL["sState"] = (kSetBlocker,)

    def CheckBlockersValid(self,team,ageSDL):

        # create shallow copy of full edge array
        myEdges = list(edgeList)

        for blockerId in ageSDL[team]:
            if blockerId == -1:
                # reached the end of set blocker list
                break
            elif blockerToEdgeDict[blockerId] is not None:
                myEdges.remove(blockerToEdgeDict[blockerId])
            else:
                # this blocker has no effect on edges, it's a throwaway
                pass

        adjacencyList = defaultdict(list)
        for nodeA, nodeB in myEdges:
            adjacencyList[nodeA].append(nodeB)
            adjacencyList[nodeB].append(nodeA)

        return DepthFirstSearch(adjacencyList, WALL_START_NODE, WALL_END_NODE)

    def IGetItem(self,name):
        avatar = PtGetLocalAvatar()
        # we will return the item corresponding to the name passed in
        for clothingType in clothingTypeList:
            clothingList = avatar.avatar.getClosetClothingList(clothingType)
            for clothingItem in clothingList:
                tempItem = ClothingItem(clothingItem)
                if tempItem.name == name:
                    return tempItem
        accList = avatar.avatar.getClosetClothingList(kAccessoryClothingItem)
        for accItem in accList:
            tempAcc = ClothingItem(accItem)
            if tempAcc.name == name:
                return tempAcc
        return None
        
class ClothingItem:
    def __init__(self,clothing):
        global clothingSets
        self.name = ""
        self.type = 0
        # parameters for the accessories
        self.groupwith = -1
        self.accessoryType = -1
        self.wornwith = []
        self.donotwear=0
        self.coloredAsHair=0
        self.isClothingSet = 0 # are we part of a set of clothing? (clothing that is worn and unworn as a group)
        self.clothingSet = "" # the set of clothing we belong to
        try:
            self.name = clothing[0]
            self.type = clothing[1]
            try:
                if clothing[2] != "":
                    self.description = PtGetLocalizedString(xACAItems.xClothesXRef[clothing[2]])
            except:
                self.description = "*"+clothing[2]+"*"
            self.thumbnail = clothing[3]
            # determine more from the custom string
            if len(clothing[4]) > 0:
                parts = clothing[4].split(';')
                for part in parts:
                    parm = part.split('=')
                    try:
                        ls = parm[0].strip().lower()
                    except LookupError:
                        ls = ""
                    try:
                        rs = parm[1].strip()
                    except LookupError:
                        rs = ""
                    if ls == "clothingtype":
                        # change clothing group name into clothingtype
                        rs = rs.lower()
                        if rs == "pants":
                            self.groupwith = kPantsClothingItem
                        elif rs == "shirt":
                            self.groupwith = kShirtClothingItem
                        elif rs == "hands":
                            self.groupwith = kRightHandClothingItem
                        elif rs == "face":
                            self.groupwith = kFaceClothingItem
                        elif rs == "hair":
                            self.groupwith = kHairClothingItem
                        elif rs == "feet":
                            self.groupwith = kRightFootClothingItem
                        else:
                            PtDebugPrint("grsnWallPython: Unknown ClothingType %s" % (rs))
                    elif ls == "accessorytype":
                        self.accessoryType = 0
                    elif ls == "accessory":
                        self.accessoryType = 0
                    elif ls == "wornwith":
                        wearlist = rs.split(',')
                        for wearitem in wearlist:
                            self.wornwith.append(wearitem.strip())
                    elif ls == "donotwear":
                        self.donotwear=1
                    elif ls == "coloredashair":
                        self.coloredAsHair=1
                    elif ls == "clothingset":
                        self.isClothingSet = 1
                        self.clothingSet = rs
                    else:
                        pass
        except (TypeError, LookupError):
            PtDebugPrint("grsnWallPython: some kind of error on clothing " + str(clothing))
