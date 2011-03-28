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
Module: xTakableClothing
Age: global
Date: December 2003
Author: Jeff Lundin
Allows you to click on a clothing object, have it removed from the scene and added to your closet. Your avatar
also wears the piece of clothing you just picked up
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaVaultConstants import *
from xPsnlVaultSDL import *
import string
import xRandom

stringVarName = ptAttribString(1,"Show/hide age SDL var name")
boolShowOnTrue = ptAttribBoolean(2,"Show on true",1)
actClickable = ptAttribActivator(3,"Clothing clickable")
stringFClothingName = ptAttribString(4,"Female clothing name")
stringMClothingName = ptAttribString(5,"Male clothing name")
boolHasHairColor = ptAttribBoolean(6,"Has hair color",0)
stringChanceSDLName = ptAttribString(7,"Chance SDL var name")
intTint1Red = ptAttribInt(8,"Tint 1 Red",255,(0,255))
intTint1Green = ptAttribInt(9,"Tint 1 Green",255,(0,255))
intTint1Blue = ptAttribInt(10,"Tint 1 Blue",255,(0,255))
intTint2Red = ptAttribInt(11,"Tint 2 Red",255,(0,255))
intTint2Green = ptAttribInt(12,"Tint 2 Green",255,(0,255))
intTint2Blue = ptAttribInt(13,"Tint 2 Blue",255,(0,255))
boolStayVisible = ptAttribBoolean(14, "Stay visible after click", 0)
boolFirstUpdate = ptAttribBoolean(15,"Eval On First Update?",0)
AgeStartedIn = None

hairColor = ptColor().white()

kEnableClothingTimer = 99
kDisableClothingTimer = 100
kGetHairColorTimer = 101
kRollDiceTimer = 102

# the base is the one that is added to the closet, all contains all items that are worn
baseFClothing = ""
baseMClothing = ""
allFClothing = []
allMClothing = []

clothingTypeList = [kHairClothingItem,kFaceClothingItem,kShirtClothingItem,kRightHandClothingItem,kPantsClothingItem,kRightFootClothingItem]

removedSets = [] # the sets we've already removed, so we don't do it multiple times
guildSDLValues = {'MTorso_GuildBlue':1,'FTorso_GuildBlue':1,'MTorso_GuildGreen':2,'FTorso_GuildGreen':2,'MTorso_GuildRed':3,'FTorso_GuildRed':3,'MTorso_GuildYellow':4,'FTorso_GuildYellow':4,'MTorso_GuildWhite':5,'FTorso_GuildWhite':5}

class xTakableClothing(ptModifier):

    def __init__(self):
        ptModifier.__init__(self)
        self.id = 211
        self.version = 3

    def OnFirstUpdate(self):
        global AgeStartedIn
        global allFClothing
        global allMClothing
        global baseFClothing
        global baseMClothing
        AgeStartedIn = PtGetAgeName()
        self.useChance = true
        self.shown = false
        if not (type(stringVarName.value) == type("") and stringVarName.value != ""):
            PtDebugPrint("ERROR: xTakableClothing.OnFirstUpdate():\tERROR: missing SDL var name on %s" % self.sceneobject.getName())
        if not (type(stringFClothingName.value) == type("") and stringFClothingName.value != ""):
            PtDebugPrint("ERROR: xTakableClothing.OnFirstUpdate():\tERROR: missing female clothing name on %s" % self.sceneobject.getName())
        if not (type(stringMClothingName.value) == type("") and stringMClothingName.value != ""):
            PtDebugPrint("ERROR: xTakableClothing.OnFirstUpdate():\tERROR: missing male clothing name on %s" % self.sceneobject.getName())
        if not (type(stringChanceSDLName.value) == type("") and stringChanceSDLName.value != ""):
            PtDebugPrint("DEBUG: xTakableClothing.OnFirstUpdate(): Chance SDL var name is empty, so we will not use chance rolls for showing %s" % self.sceneobject.getName())
            self.useChance = false
        allFClothing = stringFClothingName.value.split(";")
        for i in range(len(allFClothing)):
            allFClothing[i] = allFClothing[i].strip()
        baseFClothing = allFClothing[0]
        allMClothing = stringMClothingName.value.split(";")
        for i in range(len(allMClothing)):
            allMClothing[i] = allMClothing[i].strip()
        baseMClothing = allMClothing[0]

        if boolFirstUpdate.value:
            self.Initialize()

    def OnServerInitComplete(self):
        if not boolFirstUpdate.value:
            self.Initialize()

    def Initialize(self):
        if AgeStartedIn == PtGetAgeName():
            ageSDL = PtGetAgeSDL()
            if type(stringVarName.value) == type("") and stringVarName.value != "":
                try:
                    ageSDL.setNotify(self.key,stringVarName.value,0.0)
                    if not (ageSDL[stringVarName.value][0] ^ boolShowOnTrue.value):
                        PtAtTimeCallback(self.key, 1, kEnableClothingTimer) # we will handle the clickables in a second, since handling them now doesn't work
                        self.shown = true
                    else:
                        PtAtTimeCallback(self.key, 1, kDisableClothingTimer)
                except:
                    PtDebugPrint("ERROR: xTakableClothing.OnServerInitComplete():\tERROR accessing ageSDL on %s so we're disabling" % self.sceneobject.getName())
                    PtAtTimeCallback(self.key, 1, kDisableClothingTimer)
                    pass
            else:
                PtDebugPrint("ERROR: xTakableClothing.OnServerInitComplete():\tERROR: missing SDL var name on %s so we're disabling" % self.sceneobject.getName())
                PtAtTimeCallback(self.key, 1, kDisableClothingTimer)
                pass
            
            if self.useChance and not self.shown: # only attempt to show if it's currently hidden
                try:
                    self.chanceAppearing = int(ageSDL[stringChanceSDLName.value][0])
                    PtDebugPrint("DEBUG: xTakableClothing.OnServerInitComplete(): Chance of %s appearing is %d percent" % (self.sceneobject.getName(),self.chanceAppearing))
                    PtAtTimeCallback(self.key, 1.5, kRollDiceTimer) # make sure this fires after our initial hide/show code runs
                except:
                    PtDebugPrint("ERROR: xTakableClothing.OnServerInitComplete():\tERROR missing SDL var %s so we will not use chance rolls on %s" % (stringChanceSDLName.value,self.sceneobject.getName()))
                    self.useChance = false
                    self.chanceAppearing = 0
            else:
                self.chanceAppearing = 0
        if boolHasHairColor.value:
            PtAtTimeCallback(self.key, 1, kGetHairColorTimer)
    
    def OnTimer(self, id):
        global hairColor
        # are we trying to enable or disable the item?
        if id == kEnableClothingTimer:
            self.IEnableClothing()
        elif id == kDisableClothingTimer:
            self.IDisableClothing()
        # we need to grab the hair color
        elif id == kGetHairColorTimer:
            hairColor = self.IGetHairColor()
        # we need to roll dice to show the object
        elif id == kRollDiceTimer:
            rint = xRandom.randint(1, 100)
            PtDebugPrint("DEBUG: xTakableClothing.OnTimer(): Rolled a %d against a DC of %d on %s, success on <=" % (rint,self.chanceAppearing,self.sceneobject.getName()))
            if rint <= self.chanceAppearing:
                ageSDL = PtGetAgeSDL() # show the object
                ageSDL[stringVarName.value] = (boolShowOnTrue.value, )
                self.IEnableClothing()

    def OnSDLNotify(self,VARname,SDLname,playerID,tag):
        if VARname != stringVarName.value:
            return
        # make sure that we are in the age we think we're in
        if AgeStartedIn != PtGetAgeName():
            return
        ageSDL = PtGetAgeSDL()
        try:
            if not (ageSDL[stringVarName.value][0] ^ boolShowOnTrue.value):
                self.IEnableClothing()
            else:
                self.IDisableClothing()
        except:
            PtDebugPrint("ERROR: xTakableClothing.OnSDLNotify():\tERROR reading age SDL on %s so we're disabling" % self.sceneobject.getName())
            self.IDisableClothing()
            pass
    
    def IItemInCloset(self):
        avatar = PtGetLocalAvatar()
        clothingList = avatar.avatar.getWardrobeClothingList()
        currentgender = avatar.avatar.getAvatarClothingGroup()
        if currentgender == kFemaleClothingGroup:
            clothingName = baseFClothing
        else:
            clothingName = baseMClothing
        for item in clothingList:
            if clothingName == item[0]:
                return 1
        return 0
    
    def IGetHairColor(self):
        avatar = PtGetLocalAvatar()
        worn = avatar.avatar.getAvatarClothingList()
        # look for the existing hair clothing to grab its color
        for item in worn:
            name = item[0]
            type = item[1]
            if type == kHairClothingItem:
                PtDebugPrint("DEBUG: xTakableClothing.IGetHairColor():  Found current hair item: "+name)
                color = avatar.avatar.getTintClothingItem(name,1)
                PtDebugPrint("DEBUG: xTakableClothing.IGetHairColor():  Hair color (r,g,b) = (%d,%d,%d)" % (color.getRed(), color.getGreen(), color.getBlue()))
                return color
        PtDebugPrint("ERROR: xTakableClothing.IGetHairColor():  Couldn't find the currently worn hair item, defaulting to white")
        return ptColor().white()
    
    def IGetTint(self,oneOrTwo):
        if oneOrTwo == 1:
            red = intTint1Red.value
            green = intTint1Green.value
            blue = intTint1Blue.value
        else:
            red = intTint2Red.value
            green = intTint2Green.value
            blue = intTint2Blue.value
        # convert from 0-255 to 0-1
        red = float(red)/float(255)
        green = float(green)/float(255.0)
        blue = float(blue)/float(255.0)
        PtDebugPrint("Tint "+str(oneOrTwo)+" is ("+str(red)+","+str(green)+","+str(blue)+")")
        return ptColor(red,green,blue,1)
    
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
    
    def IConflictsWithSet(self,name):
        # we will return the name of the set that the item conflicts with if a set is worn
        # if it doesn't conflict with any set, we return ""
        clothingItem = self.IGetItem(name)
        if clothingItem:
            avatar = PtGetLocalAvatar()
            worn = avatar.avatar.getAvatarClothingList()
            for item in worn:
                tempItem = ClothingItem(item)
                if tempItem.type == clothingItem.type:
                    # this is the worn item we need to check
                    if tempItem.isClothingSet:
                        return tempItem.clothingSet # return the set we conflict with
        return ""
    
    def IRemoveOtherGuildShirt (self):
        playerCNode = ptVault().getAvatarClosetFolder()
        print "xTakableClothing: getAvatarClosetFolder Type = " + str(playerCNode.getType())
        print "xTakableClothing: getAvatarClosetFolder Child Node Count = " + str(playerCNode.getChildNodeCount())
        if playerCNode.getChildNodeCount() > 0:
            playerCNodeList = playerCNode.getChildNodeRefList()
            for folderChild in playerCNodeList:
                PtDebugPrint("xTakableClothing: looking at child node " + str(folderChild),level=kDebugDumpLevel)
                childNode = folderChild.getChild()
                if childNode != type(None):
                    print "xTakableClothing: Child Node Node ID"
                    SDLNode = childNode.upcastToSDLNode()
                    if type(SDLNode) != type(None):
                        rec = SDLNode.getStateDataRecord()
                        print "xTakableClothing: getStateDataRecord().getName(): " + str(rec.getName())
                        SDLVarList = rec.getVarList()
                        for var in SDLVarList:
                            varnode = rec.findVar(var)
                            if varnode:
                                if varnode.getType() == 4:
                                    varKey = varnode.getKey()
                                    varName = varKey.getName()
                                    print "xTakableClothing: VarNode.getName(): ", varName
                                    if varName.find('Torso_GuildBlue') != -1 or varName.find('Torso_GuildGreen') != -1 or varName.find('Torso_GuildRed') != -1 or varName.find('Torso_GuildYellow') != -1 or varName.find('Torso_GuildWhite') !=  -1:
                                        print "xTakableClothing: Found Other Guild Shirt. Deleting Old Guild Shirt."
                                        if playerCNode.removeNode(childNode):
                                            print "xTakableClothing: Delete was a success."
                                        else:
                                            print "xTakableClothing: Delete failed."
                                        return

    def IRemoveWornSet(self,setName):
        global removedSets
        if setName == "":
            return
        if setName in removedSets:
            PtDebugPrint("xTakableClothing: Set "+setName+" already removed, skipping")
            return
        PtDebugPrint("xTakableClothing: Removing worn set "+setName)
        removedSets.append(setName) # add this set to our list of removed sets
        avatar = PtGetLocalAvatar()
        worn = avatar.avatar.getAvatarClothingList()
        # replace all set pieces with the default ones
        typesToReplace = []
        for item in worn:
            tempItem = ClothingItem(item)
            if tempItem.isClothingSet and tempItem.clothingSet == setName:
                if not (tempItem.accessoryType == -1):
                    avatar.avatar.removeClothingItem(tempItem.name) # remove any set accessories
                else:
                    typesToReplace.append(tempItem.type)
        
        if len(typesToReplace) > 0:
            for clothingType in typesToReplace:
                PtWearDefaultClothingType(avatar.getKey(),clothingType)
    
    def OnNotify(self,state,id,events):
        # in no other cases do we want to take action on state = 0 events
        if not state:
            return

        if not PtWasLocallyNotified(self.key):
            PtDebugPrint("DEBUG: xTakableClothing.OnNotify(): Message didn't come from our player, ignoring")
            return

        if id==actClickable.id:
            avatar = PtGetLocalAvatar()
            currentgender = avatar.avatar.getAvatarClothingGroup()
            if currentgender == kFemaleClothingGroup:
                clothingNames = allFClothing
                base = baseFClothing
            else:
                clothingNames = allMClothing
                base = baseMClothing
            if not self.IItemInCloset():
                color1 = self.IGetTint(1)
                color2 = self.IGetTint(2)
                if boolHasHairColor.value:
                    PtDebugPrint("DEBUG: xTakableClothing.OnNotify():  Using existing hair color since this is a hair item")
                    color1 = hairColor
                if not boolStayVisible.value:
                    ageSDL = PtGetAgeSDL()
                    ageSDL[stringVarName.value] = (not (boolShowOnTrue.value), )
                self.IRemoveWornSet(self.IConflictsWithSet(base))
                if base.find('Torso_GuildBlue') != -1 or base.find('Torso_GuildGreen') != -1 or base.find('Torso_GuildRed') != -1 or base.find('Torso_GuildYellow') != -1 or base.find('Torso_GuildWhite') !=  -1:
                    self.IRemoveOtherGuildShirt()
                    psnlSDL = xPsnlVaultSDL()
                    psnlSDL["guildAlliance"] = (guildSDLValues[base],)
                    print "xTakableClothing: Guild set to:", guildSDLValues[base]
                avatar.avatar.addWardrobeClothingItem(base,ptColor().white(),ptColor().white())
                acclist = avatar.avatar.getClosetClothingList(kAccessoryClothingItem)
                accnamelist = []
                for accessory in acclist:
                    if accessory[0][4:14] == "AccGlasses" or accessory[0][1:] == "Reward_Goggles":
                    	accnamelist.append(accessory[0])
                worn = avatar.avatar.getAvatarClothingList()
                wornnamelist = []
                for wornitem in worn:
                	wornnamelist.append(wornitem[0])
                for name in clothingNames:
                    self.IRemoveWornSet(self.IConflictsWithSet(name))
                    if name in accnamelist:
                        for aitem in accnamelist:
                            if aitem in wornnamelist and aitem != name:
                                avatar.avatar.removeClothingItem(aitem)
                    PtDebugPrint("DEBUG: xTakableClothing.OnNotify():  Wearing "+name)
                    avatar.avatar.netForce(1)
                    avatar.avatar.wearClothingItem(name,0)
                    avatar.avatar.tintClothingItem(name,color1,0)
                    avatar.avatar.tintClothingItemLayer(name,color2,2,1)
                    matchingItem = avatar.avatar.getMatchingClothingItem(name)
                    if type(matchingItem) == type([]):
                        avatar.avatar.wearClothingItem(matchingItem[0],0)
                        avatar.avatar.tintClothingItem(matchingItem[0],color1,0)

                        #START-->Hard Hat color fix
                        if (matchingItem[0] == 'MReward_HardHat') or (matchingItem[0] == 'FRewardHardHat'):
                            avatar.avatar.tintClothingItem(matchingItem[0],ptColor().orange(), 2, 1)
                        else:                                      
                            avatar.avatar.tintClothingItemLayer(matchingItem[0],color2,2,1)
                        #END-->Hard Hat color fix
                    avatar.avatar.saveClothing()
            else:
                PtDebugPrint("DEBUG: xTakableClothing.OnNotify():  You already have "+base+" so I'm not going to give it to you again")

    def IEnableClothing(self):
        if self.IItemInCloset():
            # no need to disable a clickable if it's a guild shirt, and it'd be a pain to reenable later...
            if not "Guild" in stringFClothingName.value:
                PtDebugPrint("DEBUG: xTakableClothing.IEnableClothing():  Disabling clickable on %s because we already have it..." % self.sceneobject.getName())
                actClickable.disable() # don't let them take it again if they already have it
        else:
            PtDebugPrint("DEBUG: xTakableClothing.IEnableClothing():  Enabling clickable on %s..." % self.sceneobject.getName())
            actClickable.enable()

    def IDisableClothing(self):
        PtDebugPrint("DEBUG: xTakableClothing.IDisableClothing():  Disabling clickable on %s..." % self.sceneobject.getName())
        actClickable.disable()

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
                    self.description = xLocalization.xACA.xClothesXRef[clothing[2]]
            except:
                self.description = "*"+clothing[2]+"*"
            self.thumbnail = clothing[3]
            # determine more from the custom string
            if len(clothing[4]) > 0:
                parts = clothing[4].split(';')
                for part in parts:
                    parm = part.split('=')
                    try:
                        ls = string.lower(parm[0].strip())
                    except LookupError:
                        ls = ""
                    try:
                        rs = parm[1].strip()
                    except LookupError:
                        rs = ""
                    if ls == "clothingtype":
                        # change clothing group name into clothingtype
                        rs = string.lower(rs)
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
                            PtDebugPrint("xTakableClothing: Unknown ClothingType %s" % (rs))
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
            PtDebugPrint("xTakableClothing: some kind of error on clothing " + str(clothing))
