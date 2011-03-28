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
Module: xAvatarCustomization
Age: global
Author: Mark DeForest
Date: April 18, 2002
This is the python handler for the avatar customization area and GUIs
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
from PlasmaNetConstants import *
from colorsys import *
import string
import PlasmaControlKeys
import time
import os   #used for saving pictures locally

import xACAItems
import xVisitorUtils

# define the attributes that will be entered in max
InRoomActivator         = ptAttribActivator(1, "In the Room Activator")
AvCustGUI               = ptAttribGUIDialog(2,"The AvatarCustomizaion GUI")
ZoomCamera              = ptAttribSceneobject(3, "The Zoom in Camera")
Color1Map               = ptAttribDynamicMap(4, "Color 1 Dynamic Texture Map")
Color2Map               = ptAttribDynamicMap(6, "Color 2 Dynamic Texture Map")
SkinMap                 = ptAttribDynamicMap(7, "Skin Color Dynamic Texture Map")
ColorMaterial           = ptAttribMaterial(8, "Color Material")
HairMaterial            = ptAttribMaterial(9, "Hair Color Material")
SkinMaterial            = ptAttribMaterial(10, "Skin Color Material")
TestMap                 = ptAttribDynamicMap(11, "Test Map")
ZoomResponder           = ptAttribResponder(12, "Zoom responder", ['ZoomIn','ZoomOut'])

# globals
#==============
kAvCustDialogName="AvatarCustomization"
kCalibrationDialogName = "CalibrationGUI"

kCalibrationFadeOutID = 3
kCalibrationFadeOutSeconds = 0.5
kCalibrationFadeInSeconds = 1.0

InAvatarCloset = 0      # assume that we are not in the closet
kAvaCustaIsDone = "InitialAvCustomizationsDone"
kAvaCustaIsDoneType = 0
kCleftSolved = "CleftSolved"

# ==============
# AVCustGUI globals
#----Controls
kPanelsRGID=1
kLinkBackBtnID=50
kQuitBtnID=51
kAvatarResetID=52
kAvatarReadID=53
kAvatarSaveID=54
kAvatarSwitchID=58
kAvatarCameraID=59
#----GlobalControls
kNameEBID=5
kNameTBID=55
kGenderRGID=6
kHeightTBID=7
kMaleCheckbox=56
kFemaleCheckbox=57

# ====================================
# Yes/No dialog globals
#----Controls
kYesNoTextID=12
kYesButtonID=10
kNoButtonID=11

#----Morph sliders
kNumberOfMorphs = 9
kMorphSliderOffset = 200
kWeightKnob = 200
kEyebrowsKnob = 201
kNoseWidthKnob = 202
kNoseLengthKnob = 203
kNoseAngleKnob = 204
kCheeksKnob = 205
kMouthKnob = 206
kChinWidthKnob = 207
kChinAngleKnob = 208

#----TexMorph sliders
kNumberOfTexMorphs = 4
kTexMorphSliderOffset = 250
kAgeTexMorph = 251
kEthnic1TexMorph = 252
kEthnic2TexMorph = 253
kEthnic3TexMorph = 254

#----Zoom button (checkbox)
kZoomButton = 300

#----Color click maps
kColor1ClickMap = 63
kColor2ClickMap = 64
kSkinClickMap = 65
kHairClickMap = 66

#----StandardControls
kClothingDesc=60
kColorbarName1=61
kColorbarName2=62
kFakeColorSlideBtn1=63
kFakeColorSlideBtn2=64
kFakeSkinSlideBtn=65
kFakeHairSlideBtn=66
kSliderMax = 12.0

#----ClothingListboxes
kHairOptionsLB=70
kHeadOptionsLB=71
kUpperBodyOptionsLB=72
kHandsOptionsLB=73
kLwrBodyOptionsLB=74
kFeetOptionsLB=75
kAccessOptionsLB=76
# accessory panel
kAccessoryLBOffset=10
kHairAccLB=80
kHeadAccLB=81
kUpperBodyAccLB=82
kHandsAccLB=83
kLwrBodyAccLB=84
kFeetAccLB=85
kAccessAccLB=86

listboxDict = {}

# add these values to the radio button value to get the list box that is currently showing
panelOptListboxOffset = 70
panelAccListboxOffset = 80

#----Listbox scroll arrows
kIDBtnLeftAccOffset = 320 # add this to the accessory LB ID
kIDBtnHairLeftAccArrow = 400
kIDBtnHeadLeftAccArrow = 401
kIDBtnUpperBodyLeftAccArrow = 402
kIDBtnHandsLeftAccArrow = 403
kIDBtnLwrBodyLeftAccArrow = 404
kIDBtnFeetLeftAccArrow = 405

kIDBtnRightAccOffset = 330 # add this to the accessory LB ID
kIDBtnHairRightAccArrow = 410
kIDBtnHeadRightAccArrow = 411
kIDBtnUpperBodyRightAccArrow = 412
kIDBtnHandsRightAccArrow = 413
kIDBtnLwrBodyRightAccArrow = 414
kIDBtnFeetRightAccArrow = 415

kIDBtnLeftOptOffset = 350 # add this to the option LB ID
kIDBtnHairLeftOptArrow = 420
kIDBtnHeadLeftOptArrow = 421 # not used (yet)
kIDBtnUpperBodyLeftOptArrow = 422
kIDBtnHandsLeftOptArrow = 423
kIDBtnLwrBodyLeftOptArrow = 424
kIDBtnFeetLeftOptArrow = 425

kIDBtnRightOptOffset = 360 # add this to the option LB ID
kIDBtnHairRightOptArrow = 430
kIDBtnHeadRightOptArrow = 431 # not used (yet)
kIDBtnUpperBodyRightOptArrow = 432
kIDBtnHandsRightOptArrow = 433
kIDBtnLwrBodyRightOptArrow = 434
kIDBtnFeetRightOptArrow = 435

#----ClothingLB
kCLBMinWidth=96
kCLBMinHeight=104
kCLBImageX=19
kCLBImageY=18
kCLBImageWidth=91
kCLBImageHeight=97
kPrimaryListBoxSize = 8
kAccessoryLBSize = 8

# The list of wearable clothing (what's in the closet)
TheCloset = None
#=== color types
kColorTypeNone=0
kColorTypeNormal=1
kColorTypeSkin=2
kColorTypeHair=3
#-----Clothing list cross reference (to clothing lists, clothing types and listboxes
#   [0] = clothing_type; [1] = listbox ID; [2] = colorType; [3] = saturation;
#   [4] = number of items per; [5] = Closet'd clothing;
#   [6] = Closet color layer 1; [7] = Closet color layer 2
CLxref = [ [kHairClothingItem,kHairOptionsLB,kColorTypeHair,0.25,1,1,1,1],\
           [kFaceClothingItem,kHeadOptionsLB,kColorTypeSkin,0.25,1,0,1,0],\
           [kShirtClothingItem,kUpperBodyOptionsLB,kColorTypeNormal,0.40,1,1,1,1],\
           [kRightHandClothingItem,kHandsOptionsLB,kColorTypeNone,0.0,2,1,1,1],\
           [kPantsClothingItem,kLwrBodyOptionsLB,kColorTypeNormal,0.25,1,1,1,1],\
           [kRightFootClothingItem,kFeetOptionsLB,kColorTypeNormal,0.25,2,1,1,1] ]
#====
def FindSaturationAndCloset(itemname,itemtype):
    "returns the color type and saturation for a particular clothing item"
    # this returns the default for clothing type, ClothingItem.__init__ will determine local
    for xref in CLxref:
        if xref[0] == itemtype:
            return (xref[2],xref[3],xref[5],xref[6],xref[7])
    return (0,0,1,1,1)

def GetItemName(maxName):
    "returns the localized string for the specified max item name"
    try:
        return PtGetLocalizedString(xACAItems.xClothesXRef[maxName])
    except:
        return U"*" + unicode(maxName) + U"*"

# Timer constants
kTimerUpdateMorphs = 99
kTimerUpdateControls = 100

#------WornList - what's being worn
WornList = []

# The list of stuff we came in with, so we can reset back if the user wants
DefaultClothing = []
DefaultColor1 = []
DefaultColor2 = []
DefaultSkinColor = None
DefaultGeomMorphs = []
DefaultTexMorphs = []
DefaultAgeMorph = None

# list of head accessory items that we can't tint (like the D'ni goggles)
untintableHeadAcc = [ "03_FAccGoggles", "03_MAccGoggles" ]

AvatarChange = 0

#Free/Paid 
IsVisitorPlayer = 1  #use this variable so we only have to check once...


def SetDefaultSettings():
    "Sets our default vars when we enter the age so we can restore them later"
    global DefaultClothing
    global DefaultSkinColor
    global DefaultColor1
    global DefaultColor2
    global DefaultGeomMorphs
    global DefaultTexMorphs
    global DefaultAgeMorph

    # grab the clothing
    avatar = PtGetLocalAvatar()
    worn = avatar.avatar.getAvatarClothingList()
    DefaultClothing = []
    for item in worn:
        colortype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
        DefaultClothing.append(ClothingItem(item,colortype,saturation,inCloset,inClosClr1,inClosClr2))
        DefaultColor1.append(avatar.avatar.getTintClothingItem(item[0],1))
        DefaultColor2.append(avatar.avatar.getTintClothingItem(item[0],2))

    # grab the skin color
    DefaultSkinColor = avatar.avatar.getTintSkin()

    # graph the default geometry morphs
    gender = avatar.avatar.getAvatarClothingGroup()
    geomMorphs = []
    for morphID in range(kNumberOfMorphs):
        morphVal = 0
        try:
            if gender == kFemaleClothingGroup:
                morphVal = avatar.avatar.getMorph("FFace",morphID)
            else:
                morphVal = avatar.avatar.getMorph("MFace",morphID)
            geomMorphs.append(morphVal)
        except:
            pass # we'll get this one later
    if len(geomMorphs) == kNumberOfMorphs:
        # we got them all
        DefaultGeomMorphs = geomMorphs

    # graph the texture morphs
    texMorphs = []
    for texMorphID in range(kNumberOfTexMorphs):
        try:
            morphVal = avatar.avatar.getSkinBlend(texMorphID)
            texMorphs.append(morphVal)
        except:
            pass # we'll grab it later
    if len(texMorphs) == kNumberOfTexMorphs:
        # we grabbed them all
        DefaultTexMorphs = texMorphs
    try:
        morphVal = avatar.avatar.getSkinBlend(4)
        DefaultAgeMorph = morphVal
    except:
        pass



def SaveAvatarToDisk():
    # grab current settings
    avatar = PtGetLocalAvatar()
    worn = avatar.avatar.getAvatarClothingList()
    clothingList = []
    color1 = []
    color2 = []
    for item in worn:
        clothingList.append(item[0])
        color1.append(avatar.avatar.getTintClothingItem(item[0],1))
        color2.append(avatar.avatar.getTintClothingItem(item[0],2))
    skinColor = avatar.avatar.getTintSkin()
    gender = avatar.avatar.getAvatarClothingGroup()
    geomMorphs = []
    for morphID in range(kNumberOfMorphs):
        morphVal = 0
        try:
            if gender == kFemaleClothingGroup:
                morphVal = avatar.avatar.getMorph("FFace",morphID)
            else:
                morphVal = avatar.avatar.getMorph("MFace",morphID)
        except:
            pass
        geomMorphs.append(morphVal)
    texMorphs = []
    for texMorphID in range(kNumberOfTexMorphs):
        morphVal = 0
        try:
            morphVal = avatar.avatar.getSkinBlend(texMorphID)
        except:
            pass
        texMorphs.append(morphVal)
    ageMorph = 0
    try:
        ageMorph = avatar.avatar.getSkinBlend(4)
    except:
        pass
    name = PtGetLocalPlayer().getPlayerName()
    
    # write them to a file on disk
    saveFile = file(name+".avatar.ava",'w')
    saveFile.write(str(len(clothingList))+'\n')
    for i in range(len(clothingList)):
        item = clothingList[i]
        item += ' '+str(color1[i].getRed())+' '+str(color1[i].getGreen())+' '+str(color1[i].getBlue())
        item += ' '+str(color2[i].getRed())+' '+str(color2[i].getGreen())+' '+str(color2[i].getBlue())
        saveFile.write(item+'\n')
    saveFile.write(str(skinColor.getRed())+' '+str(skinColor.getGreen())+' '+str(skinColor.getBlue())+'\n')
    saveFile.write(str(len(geomMorphs))+'\n')
    for i in range(len(geomMorphs)):
        saveFile.write(str(geomMorphs[i])+'\n')
    saveFile.write(str(len(texMorphs))+'\n')
    for i in range(len(texMorphs)):
        saveFile.write(str(texMorphs[i])+'\n')
    saveFile.write(str(ageMorph)+'\n')
    saveFile.close()

def GetClothingWorn():
    "Avatars clothing has changed"
    global WornList
    avatar = PtGetLocalAvatar()
    # update what is being worn (it may have changed)
    worn = avatar.avatar.getAvatarClothingList()
    WornList = []
    for item in worn:
        colortype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
        WornList.append(ClothingItem(item,colortype,saturation,inCloset,inClosClr1,inClosClr2))

def IsWearing(item):
    "check to see if they are wearing a specified clothing item"
    global WornList
    for wornitem in WornList:
        if wornitem.name == item.name:
            return 1
    return 0

def FindWornItem(clothing_type):
    "returns the clothing name of what is worn for clothing type"
    global WornList
    for item in WornList:
        if item.type == clothing_type:
            return item
    return None

# date is a standard time tuple (year = 0, month = 1, day = 2)
# range is a standard time tuple, but the year, month, and day are tuples with two vaules, a min and a max
# the range order is the same as the US (mm/dd/yy, so year = 2, month = 0, day = 1)
def DateInRange(date,range):
    dateDay = date[2]
    dateMonth = date[1]
    dateYear = date[0]
    
    rangeDayMin = range[1][0]
    rangeDayMax = range[1][1]
    rangeMonthMin = range[0][0]
    rangeMonthMax = range[0][1]
    rangeYearMin = range[2][0]
    rangeYearMax = range[2][1]
    
    #dateStr = "%d/%d/%d" % (dateMonth,dateDay,dateYear)
    #rangeStr = "%d-%d/%d-%d/%d-%d" % (rangeMonthMin,rangeMonthMax,rangeDayMin,rangeDayMax,rangeYearMin,rangeYearMax)
    #PtDebugPrint("DateInRange(): Checking to see if the date " + dateStr + " is in the date range " + rangeStr)
    inRange = true
    if not (rangeYearMin == 0 or rangeYearMax == 0): # year of 0 means any year
        if (dateYear < rangeYearMin) or (dateYear > rangeYearMax):
            inRange = false
    if not (rangeMonthMin == 0 or rangeMonthMax == 0): # month of 0 means any month
        if (dateMonth < rangeMonthMin) or (dateMonth > rangeMonthMax):
            inRange = false
    if not (rangeDayMin == 0 or rangeDayMax == 0): # year of 0 means any year
        if (dateDay < rangeDayMin) or (dateDay > rangeDayMax):
            inRange = false
    return inRange

def CanShowSeasonal(clothingItem):
    "returns true if the item can be shown, this checks seasonal only"
    if clothingItem.seasonal:
        if ItemInWardrobe(clothingItem):
            return true # it's in our closet, so we can show this
        showTime = clothingItem.seasonTime
        if PtIsInternalRelease():
            curTime = time.localtime(time.time()) # use the client's clock if we're an internal build
        else:
            curTime = time.localtime(PtGetServerTime()) # otherwise, use the server's clock
        #PtDebugPrint("CanShowSeasonal(): Checking if we can show " + clothingItem.name)
        for timeRange in showTime: # there may be multiple times listed
            if DateInRange(curTime,timeRange):
                return true
        return false
    return true # as far as we know, the item can be shown

def CanShowClothingItem(clothingItem):
    "returns true if this item is elegable for showing"
    
    #if we're a visitor, don't allow paid clothing items
    if IsVisitorPlayer and not clothingItem.free:
        PtDebugPrint("The following item is not allowed to free players: %s" % clothingItem.name)
        return false
    
    # make sure we're not supposed to hide the item
    if (clothingItem.internalOnly and PtIsInternalRelease()) or not clothingItem.internalOnly:
        if (clothingItem.nonStandardItem and ItemInWardrobe(clothingItem)) or not clothingItem.nonStandardItem:
            if (PtIsSinglePlayerMode() and clothingItem.singlePlayer) or not PtIsSinglePlayerMode():
                if CanShowSeasonal(clothingItem):
                    return true
                else:
                    PtDebugPrint("CanShowClothingItem(): Hiding item "+clothingItem.name+" because it is seasonal")
            else:
                PtDebugPrint("CanShowClothingItem(): Hiding item "+clothingItem.name+" because it is a multiplayer-only option")
        else:
            PtDebugPrint("CanShowClothingItem(): Hiding item "+clothingItem.name+" because it is optional and isn't in your closet")
    else:
        PtDebugPrint("CanShowClothingItem(): Hiding item "+clothingItem.name+" because it is an internal-only option")
    return false

def ItemInWardrobe(clothingItem):
    avatar = PtGetLocalAvatar()
    clothingList = avatar.avatar.getWardrobeClothingList()
    for item in clothingList:
        ctype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
        closetItem = ClothingItem(item,ctype,saturation,inCloset,inClosClr1,inClosClr2)
        # hopefully name is enough to determine if it's in the closet, but we can be more accurate if necessary
        if clothingItem.name == closetItem.name:
            return 1
    return 0

def GetAllWithSameGroup(name):
    "returns a list of items in the same group as the one passed in (but not icons)"
    global TheCloset
    global CLxref
    retVal = []
    avatar = PtGetLocalAvatar()
    targetGroup = ""
    clothinglist = []
    groupFound = 0
    for idx in range(len(CLxref)):
        clothingType = CLxref[idx][0]
        clothinglist = avatar.avatar.getClosetClothingList(clothingType)
        for item in clothinglist:
            ctype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
            newitem = ClothingItem(item,ctype,saturation,inCloset,inClosClr1,inClosClr2)
            if newitem.name == name:
                targetGroup = newitem.groupName
                groupFound = 1
                break
        if groupFound:
            break
    for item in clothinglist:
        ctype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
        newitem = ClothingItem(item,ctype,saturation,inCloset,inClosClr1,inClosClr2)
        if newitem.groupName == targetGroup and not newitem.meshicon:
            if CanShowClothingItem(newitem):
                retVal.append(item)
    return retVal

def GroupHasClothing(iconItem):
    name = iconItem.name
    clothingInGroup = GetAllWithSameGroup(name)
    if len(clothingInGroup) > 0:
        return 1
    return 0
    
def UsesSameGroup(name1, name2):
    "check to see if the two clothing items share the same group"
    group = GetAllWithSameGroup(name1)
    for item in group:
        if item[0] == name2:
            return 1
    return 0

# a few small utility functions to help with the management of the scroll buttons
def IsRightArrow(id):
    if (id >= kIDBtnHairRightAccArrow and id <= kIDBtnFeetRightAccArrow) or (id >= kIDBtnHairRightOptArrow and id <= kIDBtnFeetRightOptArrow):
        return 1
    return 0

def IsLeftArrow(id):
    if (id >= kIDBtnHairLeftAccArrow and id <= kIDBtnFeetLeftAccArrow) or (id >= kIDBtnHairLeftOptArrow and id <= kIDBtnFeetLeftOptArrow):
        return 1
    return 0

def IsAccArrow(id):
    if (id >= kIDBtnHairLeftAccArrow and id <= kIDBtnFeetLeftAccArrow) or (id >= kIDBtnHairRightAccArrow and id <= kIDBtnFeetRightAccArrow):
        return 1
    return 0

def IsOptArrow(id):
    if (id >= kIDBtnHairLeftOptArrow and id <= kIDBtnFeetLeftOptArrow) or (id >= kIDBtnHairRightOptArrow and id <= kIDBtnFeetRightOptArrow):
        return 1
    return 0

class xAvatarCustomization(ptModifier):
    "The Avatar customization python code"
    def __init__(self):
        global IsVisitorPlayer
        
        ptModifier.__init__(self)
        self.id = 198
        self.version = 23
        minorVersion = 2
        PtDebugPrint("__init__xAvatarCustomization v. %d.%d" % (self.version,minorVersion))
        self.morphsLoaded = 0
        self.numTries = 0
        self.dirty = 0 # have we changed the clothing since a reset?
        
        # Set global for visitor players
        IsVisitorPlayer = not PtIsSubscriptionActive()
        PtLoadDialog(xVisitorUtils.kVisitorNagDialog)
        
        #TESTING!  
        #IsVisitorPlayer = 1 #set to test visitor player as an explorer, remove for production!



    def SetupCamera(self):
        "Disable firstperson camera and cursor fade"
        cam = ptCamera()
        cam.undoFirstPerson()
        cam.disableFirstPersonOverride()
        PtDisableAvatarCursorFade()

    def OnFirstUpdate(self):
        "First update, load our dialogs"
        self.SetupCamera()
        self.IInitFirst()

        if not InAvatarCloset and not AvatarChange:
            if PtIsDialogLoaded(kCalibrationDialogName):
                PtLoadDialog(kCalibrationDialogName,self.key)
                PtShowDialog(kCalibrationDialogName)
            # show the ava dialog later
            PtLoadDialog(kAvCustDialogName,self.key)
        else:
            #Came in from the closet...
            if PtIsDialogLoaded(kAvCustDialogName):
                self.IInitAvaCusta()
            else:
                PtLoadDialog(kAvCustDialogName,self.key)

    def OnServerInitComplete(self):
        #This is not being used right now.... as it is called too late if left in OnServerInitComplete()
        #It's been moved to OnFirstUpdate().
        #self.IInitFirst()
        # Put one of these here since the linking code in the client was overriding it, and this seems to get
        # called late enough for the linking code not to stomp all over it
        PtDisableAvatarJump()
        pass
            

    def __del__(self):
        "destructor - get rid of any dialogs that we might have loaded"
        PtUnloadDialog(xVisitorUtils.kVisitorNagDialog)
        
        PtUnloadDialog(kAvCustDialogName)
        cam = ptCamera()
        cam.enableFirstPersonOverride()
        PtEnableAvatarCursorFade()
    
    def ILocalizeStaticText(self):
        "Localizes the static text objects in the dialog"
        # we list the IDs here because no one else in this script should touch these objects
        pass #nothing to do here anymore...

    def OnNotify(self,state,id,events):
        "Activated... we just landed in the AvaCusta Age"
        if state and id == InRoomActivator.id:
            ZoomResponder.run(self.key, state="ZoomOut")
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarCameraID)).disable()
            TestMap.textmap.clearToColor(ptColor(0.92,0.82,0.63,1))
            TestMap.textmap.flush()
            # disable the KI until further notice
            PtSendKIMessage(kDisableKIandBB,0)
            PtEnableMovementKeys()
            ############# Calibration screen check moved to OnFirstUpdate ###############

        elif id == -1:
            # callback from the reset confirmation dialog box
            if state:
                self.dirty = 0 # we are resetting the avatar, so hide the reset button
                ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).hide()
                self.IResetAvatar()

    def OnGUINotify(self,id,control,event):
        "Events from the AvatarCustomize GUI.."
        global InAvatarCloset
        global listboxDict
        global AvatarChange
        global IsVisitorPlayer
        # if we don't what dialog it is... it might be the Calibration screen... 
        if id == -1:
            if event == kAction or event == kValueChanged:
                if isinstance(control,ptGUIControlButton):
                    # must be clicking on button, hide calibration and show AvatarCustomization
                    PtFadeOut(kCalibrationFadeOutSeconds,1)
                    PtAtTimeCallback(self.key, kCalibrationFadeOutSeconds, kCalibrationFadeOutID)
        # make sure this is from the Avatar Customization dialog
        elif id == AvCustGUI.id:
            if event == kDialogLoaded:
                if InAvatarCloset:
                    # set the current avatar settings to be reflected on the dialog
                    self.IInitAvaCusta()
            elif event == kAction or event == kValueChanged:
                tagID = control.getTagID()
                # is it one of the knobbies?
                if isinstance(control,ptGUIControlValue):
                    self.dirty = 1 # we changed something! so show the "reset" button
                    ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).show()
                    if tagID >= kMorphSliderOffset and tagID < kMorphSliderOffset+kNumberOfMorphs:
                        # it is a morph slider
                        self.IMorphItem(tagID)
                    else:
                        self.ITexMorphItem(tagID)
                elif isinstance(control,ptGUIControlClickMap):
                    self.dirty = 1 # we changed something! so show the "reset" button
                    ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).show()
                    self.IColorShowingItem(tagID)
                # is it one of the list boxes, probably one of the clothing listboxes
                elif isinstance(control,ptGUIControlListBox):
                    self.dirty = 1 # we changed something! so show the "reset" button
                    ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).show()
                    # find the clothing group they are working with
                    clothing_group = TheCloset[tagID]
                    if tagID == kUpperBodyOptionsLB or tagID == kLwrBodyOptionsLB:
                        # update the "accessory" list box (where the textures go)
                        itemselect = control.getSelection()
                        if itemselect == -1:
                            avatar = PtGetLocalAvatar()
                            self.ISetWhatWearing(avatar)
                        else:
                            # tell our listbox class to select it so we can get the clothing item the user wants
                            listboxDict[tagID].SelectItem(itemselect)
                            selectedItem = listboxDict[tagID].GetSelectedItem()
                            # setup the "accessory" listbox with our texture options
                            texGroup = TextureGroup(clothing_group.clothingType,selectedItem.name)
                            listboxDict[tagID + kAccessoryLBOffset].SetClothingList(texGroup.clothingItems)
                            listboxDict[tagID + kAccessoryLBOffset].SelectItem(0) # select the first texture option
                            listboxDict[tagID + kAccessoryLBOffset].UpdateScrollArrows()
                            listboxDict[tagID + kAccessoryLBOffset].UpdateListbox()
                            listbox = ptGUIControlListBox(AvCustGUI.dialog.getControlFromTag(tagID+kAccessoryLBOffset))
                            self.OnGUINotify(AvCustGUI.id, listbox, kValueChanged) # fake a mouse down
                    elif type(clothing_group) != type(None):
                        # found list box
                        itemselect = control.getSelection()
                        if itemselect == -1:
                            avatar = PtGetLocalAvatar()
                            self.ISetWhatWearing(avatar)
                        else:
                            # tell our listbox class to select it so we can get the clothing item the user wants
                            listboxDict[tagID].SelectItem(itemselect)
                            newitem = listboxDict[tagID].GetSelectedItem()
                            # get the current worn item to see what color it was
                            lastitem = FindWornItem(clothing_group.clothingType)
                            avatar = PtGetLocalAvatar()
                            if type(lastitem) != type(None):
                                # just get the color straight from the item
                                lastcolor1 = avatar.avatar.getTintClothingItem(lastitem.name,1)
                                lastcolor2 = avatar.avatar.getTintClothingItem(lastitem.name,2)
                            else:
                                lastcolor1 = ptColor().white()
                                lastcolor2 = ptColor().white()
                            # if we need to grab a second object (like a second shoe), then get and wear it
                            if clothing_group.numberItems > 1:
                                matchingItem = avatar.avatar.getMatchingClothingItem(newitem.name)
                                if type(matchingItem) == type([]):
                                    avatar.avatar.wearClothingItem(matchingItem[0],0)
                                    avatar.avatar.tintClothingItem(matchingItem[0],lastcolor1,0)
                                    avatar.avatar.tintClothingItemLayer(matchingItem[0],lastcolor2,2,0)
                            # tint and wear the chosen item
                            avatar.avatar.wearClothingItem(newitem.name,0)
                            avatar.avatar.tintClothingItem(newitem.name,lastcolor1,0)
                            avatar.avatar.tintClothingItemLayer(newitem.name,lastcolor2,2)
                            self.IMorphOneItem(kWeightKnob,newitem.name) # propigate the weight morph to the new clothing item
                            # then set the description box
                            descbox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kClothingDesc))
                            descbox.setStringW(newitem.description)
                            # set up the color pickers
                            colorbar1 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName1))
                            if newitem.colorlabel1 == U"":
                                self.IHideColorPicker(kColor1ClickMap)
                            else:
                                self.IShowColorPicker(kColor1ClickMap)
                                colorbar1.setStringW(newitem.colorlabel1)
                                if newitem.type == kHairClothingItem:
                                    self.IDrawPickerThingy(kHairClickMap,lastcolor1)
                                else:
                                    self.IDrawPickerThingy(kColor1ClickMap,lastcolor1)
                            colorbar2 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName2))
                            if newitem.colorlabel2 == U"":
                                self.IHideColorPicker(kColor2ClickMap)
                            else:
                                self.IShowColorPicker(kColor2ClickMap)
                                colorbar2.setStringW(newitem.colorlabel2)
                                self.IDrawPickerThingy(kColor2ClickMap,lastcolor2)
                    # else it might be one of the accessory listboxes
                    else:
                        clothing_group = TheCloset[tagID-kAccessoryLBOffset]
                        if tagID-kAccessoryLBOffset == kUpperBodyOptionsLB or tagID-kAccessoryLBOffset == kLwrBodyOptionsLB:
                            # this "accessory" listbox is actually a textures listbox, treat as clothing
                            clothing_group = TheCloset[tagID-kAccessoryLBOffset]
                            if type(clothing_group) != type(None):
                                itemselect = control.getSelection()
                                # if there is only one texture, it isn't shown, so fake it into selecting the first item
                                if len(listboxDict[tagID].clothingList) == 1:
                                    itemselect = 0
                                if itemselect == -1:
                                    avatar = PtGetLocalAvatar()
                                    self.ISetWhatWearing(avatar)
                                else:
                                    # set up the listbox so we can grab the selected item
                                    listboxDict[tagID].SelectItem(itemselect)
                                    newitem = listboxDict[tagID].GetSelectedItem()
                                    avatar = PtGetLocalAvatar()
                                    # get the current worn item to see what color it was
                                    lastitem = FindWornItem(clothing_group.clothingType)
                                    if type(lastitem) != type(None):
                                        # just get the color straight from the item
                                        lastcolor1 = avatar.avatar.getTintClothingItem(lastitem.name,1)
                                        lastcolor2 = avatar.avatar.getTintClothingItem(lastitem.name,2)
                                    else:
                                        lastcolor1 = ptColor().white()
                                        lastcolor2 = ptColor().white()
                                    # tint and wear the item
                                    avatar.avatar.wearClothingItem(newitem.name,0)
                                    avatar.avatar.tintClothingItem(newitem.name,lastcolor1,0)
                                    avatar.avatar.tintClothingItemLayer(newitem.name,lastcolor2,2)
                                    self.IMorphOneItem(kWeightKnob,newitem.name) # propigate the weight morph to the new clothing item
                                    # then set the description box
                                    descbox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kClothingDesc))
                                    descbox.setStringW(newitem.description)
                                    # set up the color pickers
                                    colorbar1 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName1))
                                    if newitem.colorlabel1 == U"":
                                        self.IHideColorPicker(kColor1ClickMap)
                                    else:
                                        self.IShowColorPicker(kColor1ClickMap)
                                        colorbar1.setStringW(newitem.colorlabel1)
                                        self.IDrawPickerThingy(kColor1ClickMap,lastcolor1)
                                    colorbar2 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName2))
                                    if newitem.colorlabel2 == U"":
                                        self.IHideColorPicker(kColor2ClickMap)
                                    else:
                                        self.IShowColorPicker(kColor2ClickMap)
                                        colorbar2.setStringW(newitem.colorlabel2)
                                        self.IDrawPickerThingy(kColor2ClickMap,lastcolor2)
                        elif type(clothing_group) != type(None):
                            itemselect = control.getSelection()
                            if itemselect == -1:
                                avatar = PtGetLocalAvatar()
                                self.ISetWhatWearing(avatar)
                            else:
                                # grab the actual selection from our listbox class
                                listboxDict[tagID].SelectItem(itemselect)
                                newitem = listboxDict[tagID].GetSelectedItem()
                                avatar = PtGetLocalAvatar()
                                lastitem = ""
                                # remove all currently worn accessories
                                for aitem in clothing_group.accessories:
                                    if IsWearing(aitem) and aitem.name != newitem.name:
                                        avatar.avatar.removeClothingItem(aitem.name)
                                        lastitem = aitem.name
                                if not newitem.donotwear:
                                    if newitem.coloredAsHair:
                                        # find the hair color and color this item
                                        haircolor = self.IGetHairColor()
                                        if type(haircolor) != type(None):
                                            avatar.avatar.wearClothingItem(newitem.name,0)
                                            avatar.avatar.tintClothingItem(newitem.name,haircolor)
                                        else:
                                            avatar.avatar.wearClothingItem(newitem.name)
                                    else:
                                        # else just wear the thing
                                        avatar.avatar.wearClothingItem(newitem.name)
                                        # if this is a head accessory, we can tint it, so restore last tint
                                        if tagID == kHeadAccLB and not (newitem.name in untintableHeadAcc):
                                            self.IShowColorPicker(kColor2ClickMap)
                                            colorbar2 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName2))
                                            colorbar2.setStringW(GetItemName("Glasses"))
                                            if not lastitem == "":
                                                lastcolor = avatar.avatar.getTintClothingItem(lastitem,1)
                                            else:
                                                lastcolor = ptColor(1,1,1,1)
                                            avatar.avatar.tintClothingItem(newitem.name,lastcolor)
                                            # draw the picker thingy on the accessory color picker
                                            self.IDrawPickerThingy(kColor2ClickMap,lastcolor)
                                        else:
                                            self.IHideColorPicker(kColor2ClickMap)
                                    self.IMorphOneItem(kWeightKnob,newitem.name) # propigate the weight morph to the new clothing item
                                else:
                                    if tagID == kHeadAccLB:
                                        # we are removing the glasses, so hide the second color picker
                                        self.IHideColorPicker(kColor2ClickMap)
                elif isinstance(control,ptGUIControlRadioGroup):
                    if tagID == kPanelsRGID:
                        panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
                        rgVal = panelRG.getValue()
                        zoomBtn = ptGUIControlCheckBox(AvCustGUI.dialog.getControlFromTag(kZoomButton))
                        if rgVal == 1:
                            ZoomResponder.run(self.key, state="ZoomIn")
                            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarCameraID)).enable()
                            # enable the zoom button
                            zoomBtn.show()
                            # zoom in on face automatically
                            zoomBtn.setChecked(true)
                            ZoomCamera.sceneobject.pushCutsceneCamera(1,PtGetLocalAvatar().getKey())
                            panelRG.hide()
                            self.SetupCamera()
                        else:
                            zoomBtn.hide()
                        # unset the description box
                        descbox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kClothingDesc))
                        descbox.setString("")
                        self.ISetStandardControls()
                elif isinstance(control,ptGUIControlButton):
                    btnID = control.getTagID()
                    if  btnID == kLinkBackBtnID:
                        # they want to go back...
                        if not ptNetLinkingMgr().isEnabled():
                            PtDebugPrint("OnGuiNotify():\tAborting linkout attempt because the linking managar isn't ready")
                            return # don't attempt to link out if we can't do so
                        avatar = PtGetLocalAvatar()
                        # save any worn seasonal items to the player's closet
                        self.ISaveSeasonalToCloset()
                        # save the settings to the vault
                        avatar.avatar.saveClothing()
                        PtEnableAvatarJump()
                        if InAvatarCloset:
                            vault = ptVault()
                            entry = vault.findChronicleEntry(kCleftSolved)
                            linkmgr = ptNetLinkingMgr()

                            #Just link back to the closet
                            self.ILinkToCloset()

                            #Disable other logic... no more going to the cleft from the AVC
                            #~if type(entry) != type(None):
                                # player has solved the cleft
                                # just go back to your personal age
                            #~        self.ILinkToCloset()
                            #~else:
                                # player has not solved the cleft, link them to it
                            #~    ageLink = ptAgeLinkStruct()
                            #~    ageInfo = ageLink.getAgeInfo()
                            #~    temp = ptAgeInfoStruct()
                            #~    temp.copyFrom(ageInfo)
                            #~    ageInfo = temp
                            #~    if PtIsDemoMode():
                            #~        ageInfo.setAgeFilename("Demo")
                            #~    else:
                            #~        ageInfo.setAgeFilename("Cleft")
                            #~    ageInfo.setAgeInstanceName("D'ni-Riltagamin")
                            #~    ageLink.setAgeInfo(ageInfo)
                            #~    ageLink.setLinkingRules(PtLinkingRules.kOriginalBook)
                            #~    linkmgr.linkToAge(ageLink)
                            
                        else:
                            # mark the chonicle that we've been here
                            vault = ptVault()
                            vault.addChronicleEntry(kAvaCustaIsDone,kAvaCustaIsDoneType,"1")
                            entry = vault.findChronicleEntry(kCleftSolved)
                            
                            #Link straight to personal, no more going to cleft from the AVC!
                            linkmgr = ptNetLinkingMgr()
                            ageLink = ptAgeLinkStruct()
                            ageInfo = ageLink.getAgeInfo()
                            temp = ptAgeInfoStruct()
                            temp.copyFrom(ageInfo)
                            ageInfo = temp
                            ageInfo.setAgeFilename("Personal")
                            ageLink.setAgeInfo(ageInfo)
                            ageLink.setLinkingRules(PtLinkingRules.kOwnedBook)
                            linkmgr.linkToAge(ageLink)
                            
                            #Disable other logic... no more going to cleft from the AVC!
                            #~if type(entry) != type(None):
                                # player has solved the cleft
                                # just go back to your personal age
                            #~    linkmgr = ptNetLinkingMgr()
                            #~    ageLink = ptAgeLinkStruct()
                                
                            #~    ageInfo = ageLink.getAgeInfo()
                            #~    temp = ptAgeInfoStruct()
                            #~    temp.copyFrom(ageInfo)
                            #~    ageInfo = temp
                            #~    ageInfo.setAgeFilename("Personal")
                            #~    ageLink.setAgeInfo(ageInfo)
                            #~    ageLink.setLinkingRules(PtLinkingRules.kOriginalBook)
                            #~    linkmgr.linkToAge(ageLink)
                            
                            #~else:
                                # this was their first time... go to the cleft
                            #~    ageLink = ptAgeLinkStruct()
                            #~    ageInfo = ageLink.getAgeInfo()
                            #~    temp = ptAgeInfoStruct()
                            #~    temp.copyFrom(ageInfo)
                            #~    ageInfo = temp
                            #~    if PtIsDemoMode():
                            #~        ageInfo.setAgeFilename("Demo")
                            #~    else:
                            #~        ageInfo.setAgeFilename("Cleft")
                            #~    ageInfo.setAgeInstanceName("D'ni-Riltagamin")
                            #~    ageLink.setAgeInfo(ageInfo)
                            #~    ageLink.setLinkingRules(PtLinkingRules.kOriginalBook)
                            #~    linkmgr = ptNetLinkingMgr()
                            #~    linkmgr.linkToAge(ageLink)
                            
                    elif btnID == kQuitBtnID:
                        avatar = PtGetLocalAvatar()
                        # save any seasonal clothing being worn to the closet
                        self.ISaveSeasonalToCloset()
                        # save their changes
                        avatar.avatar.saveClothing()
                        # and ask if they want to quit (KI will quit if they answer yes)
                        PtSendKIMessage(kQuitDialog,0)
                    elif IsAccArrow(btnID):
                        panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
                        rgVal = panelRG.getValue()
                        listboxID = panelAccListboxOffset + rgVal
                        if IsLeftArrow(btnID):
                            listboxDict[listboxID].DecrementOffset()
                        else:
                            listboxDict[listboxID].IncrementOffset()
                        listboxDict[listboxID].UpdateScrollArrows()
                        listboxDict[listboxID].UpdateListbox()
                    elif IsOptArrow(btnID):
                        panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
                        rgVal = panelRG.getValue()
                        listboxID = panelOptListboxOffset + rgVal
                        if IsLeftArrow(btnID):
                            listboxDict[listboxID].DecrementOffset()
                        else:
                            listboxDict[listboxID].IncrementOffset()
                        listboxDict[listboxID].UpdateScrollArrows()
                        listboxDict[listboxID].UpdateListbox()
                    elif btnID == kAvatarResetID:
                        PtDebugPrint("Confirming reset...")
                        PtYesNoDialog(self.key,PtGetLocalizedString("ACA.GUI.ResetConfirm"))
                    elif btnID == kAvatarReadID:
                        if PtIsInternalRelease():
                            self.IRestoreAvatarFromDisk()
                            self.dirty = 1 # we are changing the avatar, so show the reset button
                            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).show()
                    elif btnID == kAvatarSaveID:
                        if PtIsInternalRelease():
                            SaveAvatarToDisk()
                    elif btnID == kAvatarSwitchID:
                        if not InAvatarCloset:
                            AvatarChange = 1
                            avatar = PtGetLocalAvatar()
                            gender = avatar.avatar.getAvatarClothingGroup()
                            if gender == kFemaleClothingGroup:
                                PtChangeAvatar("Male")
                            else:
                                PtChangeAvatar("Female")
                    elif btnID == kAvatarCameraID:
                        print "ACA: Taking a picture!"
                        picCam = ptCamera()
                        PicCamAspect = picCam.getAspectRatio()
                        picCam.setAspectRatio(4/3)
                        picCam.refreshFOV()
                        AvCustGUI.dialog.hide()
                        PtStartScreenCapture(self.key, 1024, 768)
                        picCam.setAspectRatio(PicCamAspect)

                elif isinstance(control,ptGUIControlCheckBox):
                    chkBoxID = control.getTagID()
                    if chkBoxID == kZoomButton:
                        zoomBtn = ptGUIControlCheckBox(AvCustGUI.dialog.getControlFromTag(chkBoxID))
                        radioGroup = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
                        if zoomBtn.isChecked():
                            ZoomResponder.run(self.key, state="ZoomIn")
                            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarCameraID)).enable()
                            ZoomCamera.sceneobject.pushCutsceneCamera(1,PtGetLocalAvatar().getKey())
                            radioGroup.hide()
                            self.SetupCamera()
                        else:
                            ZoomResponder.run(self.key, state="ZoomOut")
                            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarCameraID)).disable()
                            ZoomCamera.sceneobject.popCutsceneCamera(PtGetLocalAvatar().getKey())
                            radioGroup.show()
                            self.SetupCamera()
                else:
                    pass
            else:
                pass
        else:
            pass
    
    def OnScreenCaptureDone(self,image):
        AvCustGUI.dialog.show()
        picCam = ptCamera()
        picCam.refreshFOV()
        avatar = PtGetLocalAvatar()
        currentgender = avatar.avatar.getAvatarClothingGroup()
        if currentgender == 1:
            print "Female Screenshot"
            TestMap.textmap.drawImageClipped(0, 0, image, 55, 200, 512, 512, 0)
        else:
            print "Male Screenshot"
            TestMap.textmap.drawImageClipped(0, 0, image, 55, 60, 512, 512, 0)
        TestMap.textmap.flush()
        PtAtTimeCallback(self.key, 0, 999)

    def IResetAvatar(self):
        "Resets our avatar to the defaults"
        # reset our clothing
        PtDebugPrint("Resetting avatar...")
        avatar = PtGetLocalAvatar()
        # remove all accessories that aren't in our accessory list
        for item in WornList:
            if item.accessoryType >= 0:
                found = 0
                for acc in DefaultClothing:
                    if acc.name == item.name:
                        found = 1
                        break
                if not found:
                    avatar.avatar.removeClothingItem(item.name)
        # add all the default clothing items back
        for i in range(len(DefaultClothing)):
            item = DefaultClothing[i]
            color1 = DefaultColor1[i]
            color2 = DefaultColor2[i]
            avatar.avatar.wearClothingItem(item.name,0)
            avatar.avatar.tintClothingItem(item.name,color1,0)
            avatar.avatar.tintClothingItemLayer(item.name,color2,2)
            # add the matching item, if it exists
            matchingItem = avatar.avatar.getMatchingClothingItem(item.name)
            if type(matchingItem) == type([]):
                avatar.avatar.wearClothingItem(matchingItem[0],0)
                avatar.avatar.tintClothingItem(matchingItem[0],color1,0)
                avatar.avatar.tintClothingItemLayer(matchingItem[0],color2,2,0)

        # reset skin color
        avatar.avatar.tintSkin(DefaultSkinColor)

        # reset geometry morphs
        for morphID in range(len(DefaultGeomMorphs)):
            gender = avatar.avatar.getAvatarClothingGroup()
            if gender == kFemaleClothingGroup:
                avatar.avatar.setMorph("FFace",morphID,DefaultGeomMorphs[morphID])
            else:
                avatar.avatar.setMorph("MFace",morphID,DefaultGeomMorphs[morphID])

        # reset texture morphs
        for morphID in range(len(DefaultTexMorphs)):
            avatar.avatar.setSkinBlend(morphID,DefaultTexMorphs[morphID])
        avatar.avatar.setSkinBlend(4,DefaultAgeMorph)
        
        # give the avatar a bit of time to update before trying to update the controls
        PtAtTimeCallback(self.key, 1, kTimerUpdateControls)
    
    def IRestoreAvatarFromDisk(self):
        # open the file and read in the settings
        name = PtGetLocalPlayer().getPlayerName()
        try:
            saveFile = file(name+".avatar.ava",'r')
            numClothingItems = int(saveFile.readline())
            clothingList = []
            color1 = []
            color2 = []
            for i in range(numClothingItems):
                line = saveFile.readline()
                items = string.split(line)
                clothingList.append(items[0])
                color1.append(ptColor(float(items[1]),float(items[2]),float(items[3])))
                color2.append(ptColor(float(items[4]),float(items[5]),float(items[6])))
            line = saveFile.readline()
            items = string.split(line)
            skinColor = ptColor(float(items[0]),float(items[1]),float(items[2]))
            numMorphs = int(saveFile.readline())
            geomMorphs = []
            for i in range(numMorphs):
                morphVal = float(saveFile.readline())
                geomMorphs.append(morphVal)
            numMorphs = int(saveFile.readline())
            texMorphs = []
            for i in range(numMorphs):
                morphVal = float(saveFile.readline())
                texMorphs.append(morphVal)
            ageMorph = float(saveFile.readline())
            saveFile.close()
    
            # now set up the avatar properly
            # reset our clothing
            avatar = PtGetLocalAvatar()
            # remove all accessories that aren't in our accessory list
            for item in WornList:
                if item.accessoryType >= 0:
                    found = 0
                    for acc in DefaultClothing:
                        if acc.name == item.name:
                            found = 1
                            break
                    if not found:
                        avatar.avatar.removeClothingItem(item.name)
            # add all the clothing items back
            for i in range(len(clothingList)):
                item = clothingList[i]
                clr1 = color1[i]
                clr2 = color2[i]
                avatar.avatar.wearClothingItem(item,0)
                avatar.avatar.tintClothingItem(item,clr1,0)
                avatar.avatar.tintClothingItemLayer(item,clr2,2)
                # add the matching item, if it exists
                matchingItem = avatar.avatar.getMatchingClothingItem(item)
                if type(matchingItem) == type([]):
                    avatar.avatar.wearClothingItem(matchingItem[0],0)
                    avatar.avatar.tintClothingItem(matchingItem[0],clr1,0)
                    avatar.avatar.tintClothingItemLayer(matchingItem[0],clr2,2,0)
            # reset skin color
            avatar.avatar.tintSkin(skinColor)
            # reset geometry morphs
            for morphID in range(len(geomMorphs)):
                gender = avatar.avatar.getAvatarClothingGroup()
                if gender == kFemaleClothingGroup:
                    avatar.avatar.setMorph("FFace",morphID,geomMorphs[morphID])
                else:
                    avatar.avatar.setMorph("MFace",morphID,geomMorphs[morphID])
            # reset texture morphs
            for morphID in range(len(texMorphs)):
                avatar.avatar.setSkinBlend(morphID,texMorphs[morphID])
            avatar.avatar.setSkinBlend(4,ageMorph)
            # give the avatar a bit of time to update before trying to update the controls
            PtAtTimeCallback(self.key, 1, kTimerUpdateControls)
        except IOError:
            pass
    
    def ISaveSeasonalToCloset(self):
        for item in WornList:
            if item.seasonal and not ItemInWardrobe(item):
                PtDebugPrint("Adding seasonal item "+item.name+" to your closet since you are wearing it")
                avatar = PtGetLocalAvatar()
                avatar.avatar.addWardrobeClothingItem(item.name,ptColor().white(),ptColor().white())
    
    def ILinkToCloset(self):
        linkmgr = ptNetLinkingMgr()
        ageLink = ptAgeLinkStruct()
        
        ageInfo = ageLink.getAgeInfo()
        temp = ptAgeInfoStruct()
        temp.copyFrom(ageInfo)
        ageInfo = temp
        ageInfo.setAgeFilename("Personal")
        
        spawnPoint = ageLink.getSpawnPoint()
        spawnPoint.setName("LinkInPointCloset")
        
        ageLink.setAgeInfo(ageInfo)
        ageLink.setSpawnPoint(spawnPoint)
        ageLink.setLinkingRules(PtLinkingRules.kOwnedBook)
        linkmgr.linkToAge(ageLink)

    def IInitFirst(self):
        "Initialization stuff, after the dialog has been loaded"
        global InAvatarCloset
        # determine if this the first time here or not
        vault = ptVault()
        entry = vault.findChronicleEntry(kAvaCustaIsDone)

        InAvatarCloset = 0      # assume never been here before, until proven otherwise
        if type(entry) != type(None):
            InAvatarCloset = 1
        PtDebugPrint("AvaCusta: InAvatarCloset is %d" % (InAvatarCloset))

        entry = vault.findChronicleEntry("GiveYeeshaReward")
        if type(entry) != type(None):
            # we need to give the yeesha clothing (probably an imported char)
            avatar = PtGetLocalAvatar()
            currentgender = avatar.avatar.getAvatarClothingGroup()
            if currentgender == kFemaleClothingGroup:
                clothingName = "02_FTorso11_01"
            else:
                clothingName = "02_MTorso09_01"
            clothingList = avatar.avatar.getWardrobeClothingList()
            if clothingName not in clothingList:
                print "adding Yeesha reward clothing %s to wardrobe" % (clothingName)
                avatar.avatar.addWardrobeClothingItem(clothingName,ptColor().white(),ptColor().black())
            else:
                print "player already has Yeesha reward clothing, doing nothing"
            folder = vault.getChronicleFolder()
            if type(folder) != type(None):
                folder.removeNode(entry)

    def IInitAvaCusta(self):
        "Initialization stuff, after the dialog has been loaded"
        self.IUpdateAllControls()
        SetDefaultSettings() # grab the stuff we came in with
        PtDisableAvatarCursorFade()

        if InAvatarCloset:
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarSwitchID)).hide()
        else:
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarSwitchID)).hide()
        
        AvCustGUI.dialog.show()
        PtDisableAvatarJump()
        self.ILocalizeStaticText()


    def OnClothingUpdate(self):
        "Avatars clothing has changed"
        GetClothingWorn()

    def IGetHairColor(self):
        # find the hair color from the first hair color type item
        global WornList
        avatar = PtGetLocalAvatar()
        for item in WornList:
            # there is hair and there is hair on the face (also), so color both!
            if item.type == kHairClothingItem:
                return avatar.avatar.getTintClothingItem(item.name,1)
        return None
    
    def OnTimer(self, id):
        # if this is the calibration fade out
        if id == kCalibrationFadeOutID:
            PtHideDialog(kCalibrationDialogName)
            self.IInitAvaCusta()
            PtFadeIn(kCalibrationFadeInSeconds,0)
            return

        if id == kTimerUpdateMorphs and not self.morphsLoaded:
            # we don't want to keep trying because it causes the ACA to stutter, so give up after 5 tries
            self.numTries = self.numTries + 1
            if self.numTries < 5:
                self.IUpdateAllControls()
                SetDefaultSettings() # grab the stuff we came in with
        elif id == kTimerUpdateControls:
            # We need to update our current selection (probably because we just reset the avatar)
            self.IUpdateAllControls()

        if id == 999:
            newImage = TestMap.textmap.getImage()
            basePath = PtGetUserPath() + U"\\Avatars\\"
            if not PtCreateDir(basePath):
                print U"xAvatarCustomization::OnTimer(): Unable to create \"" + basePath + "\" directory. Avatar pic is NOT saved."
                return
            filename = basePath + unicode(PtGetLocalPlayer().getPlayerID()) + U".jpg"
            print U"xAvatarCustomization::OnTimer(): Saving avatar pic to \"" + filename + U"\""
            newImage.saveAsJPEG(filename, 90)

    def IUpdateAllControls(self):
        "Update all the dialog controls to reflect what the avatar is currently"
        # do the global settings
        avatar = PtGetLocalAvatar()
        namebox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kNameTBID))
        namebox.show()
        editbox = ptGUIControlEditBox(AvCustGUI.dialog.getControlFromTag(kNameEBID))
        editbox.hide() # don't want people changing their name
        localplayer = PtGetLocalPlayer()
        namebox.setString(localplayer.getPlayerName())
        panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
        clothing_panel = panelRG.getValue()
        zoomBtn = ptGUIControlCheckBox(AvCustGUI.dialog.getControlFromTag(kZoomButton))
        if clothing_panel == 1:
            # enable the zoom button
            zoomBtn.show()
        else:
            zoomBtn.hide()
        # clear out the color pickers
        Color1Map.textmap.clearToColor(ptColor(0,0,0,0))
        Color1Map.textmap.flush()
        Color2Map.textmap.clearToColor(ptColor(0,0,0,0))
        Color2Map.textmap.flush()
        SkinMap.textmap.clearToColor(ptColor(0,0,0,0))
        SkinMap.textmap.flush()
        # get what is currently being worn
        GetClothingWorn()
        # fill in all the clothing listboxes
        self.IUpdateClothingListboxes(avatar)
        self.ISetWhatWearing(avatar)
        # set color controls
        self.ISetStandardControls()

            
    def IUpdateClothingListboxes(self,avatar):
        "Update the clothing listboxes"
        global TheCloset
        global listboxDict
        # assume that there is no accessories
        listbox = ptGUIControlListBox(AvCustGUI.dialog.getControlFromTag(kAccessOptionsLB))
        listbox.hide()
        # get (or re-get) the closet
        TheCloset = ClothingCloset()
        # start from the top and go down Hair thru Feet (and Accessories)
        for listboxID in TheCloset.keys():
            # do the primary clothing listbox
            group = TheCloset[listboxID]
            # add the listbox class if we don't have it already
            if not (listboxID in listboxDict):
                listboxDict[listboxID] = ScrollingListBox()
                listboxDict[listboxID].SetListboxID(listboxID)
            if not (listboxID + kAccessoryLBOffset in listboxDict):
                listboxDict[listboxID + kAccessoryLBOffset] = ScrollingListBox()
                listboxDict[listboxID + kAccessoryLBOffset].SetListboxID(listboxID + kAccessoryLBOffset)
            # set up and display our list box
            listboxDict[listboxID].SetClothingList(group.clothingItems)
            listboxDict[listboxID].UpdateScrollArrows()
            listboxDict[listboxID].UpdateListbox()
            # and configure the accessory panel associated with it
            if listboxID == kUpperBodyOptionsLB or listboxID == kLwrBodyOptionsLB:
                # this is a texture panel
                targetMesh = listboxDict[listboxID].GetSelectedItem()
                texGroup = TextureGroup(group.clothingType,targetMesh.name)
                listboxDict[listboxID + kAccessoryLBOffset].SetClothingList(texGroup.clothingItems)
                listboxDict[listboxID + kAccessoryLBOffset].UpdateScrollArrows()
                listboxDict[listboxID + kAccessoryLBOffset].UpdateListbox()
            else:
                # this is a standard accessory panel
                listboxDict[listboxID + kAccessoryLBOffset].SetClothingList(group.accessories)
                listboxDict[listboxID + kAccessoryLBOffset].UpdateScrollArrows()
                listboxDict[listboxID + kAccessoryLBOffset].UpdateListbox()

    def ISetWhatWearing(self,avatar):
        "Gets whats being worn and sets the dialogs to show what we are wearing"
        # assumes that WornList is already filled out
        global listboxDict
        for id in TheCloset.keys():
            # tell the listboxes to update themselves
            listboxDict[id].SetWhatWearing()
            listboxDict[id].UpdateScrollArrows()
            listboxDict[id].UpdateListbox()
            if id == kUpperBodyOptionsLB or id == kLwrBodyOptionsLB:
                # this is a texture listbox, so update our list of clothing items, and pick the currently worn one
                targetMesh = listboxDict[id].GetSelectedItem()
                group = TheCloset[id]
                texGroup = TextureGroup(group.clothingType,targetMesh.name)
                listboxDict[id + kAccessoryLBOffset].SetClothingList(texGroup.clothingItems)
                listboxDict[id + kAccessoryLBOffset].SetWhatWearing()
                listboxDict[id + kAccessoryLBOffset].UpdateScrollArrows()
                listboxDict[id + kAccessoryLBOffset].UpdateListbox()
            else:
                # standard accessory panel
                listboxDict[id + kAccessoryLBOffset].SetWhatWearing()
                listboxDict[id + kAccessoryLBOffset].UpdateScrollArrows()
                listboxDict[id + kAccessoryLBOffset].UpdateListbox()

    def IColorShowingItem(self,controlID):
        "Color whatever clothing type is selected with color1 slider"
        global CLxref
        global WornList
        global IsVisitorPlayer
        
        # Find what to color
        panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
        clothing_panel = panelRG.getValue()
        foundItem = 0
        if clothing_panel >= 0 and clothing_panel < len(CLxref):
            clothing_type = CLxref[clothing_panel][0]
            # find the item that is worn that is in this clothing type
            wornItem = FindWornItem(clothing_type)
            # did we find the item that is being worn in this clothing group?
            if type(wornItem) != type(None):
                # if the saturation is zero then don't allow tiniting
                if controlID == kColor1ClickMap or controlID == kColor2ClickMap:
                    #Intercept any visitor originated clickMaps and display the nag!
                    if IsVisitorPlayer:
                        PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                        return

                    #Make sure that we're not zoomed in (changing eye color)
                    panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
                    rgVal = panelRG.getValue()
                   
                    # get the color
                    if controlID == kColor2ClickMap:
                        layer = 2
                        colormap = ptGUIControlClickMap(AvCustGUI.dialog.getControlFromTag(kColor2ClickMap)).getLastMouseUpPoint()
                        colorit = ColorMaterial.map.getPixelColor(colormap.getX(),colormap.getY())
                        self.IDrawCrosshair(kColor2ClickMap,colormap.getX(),colormap.getY())
                    else:
                        layer = 1
                        colormap = ptGUIControlClickMap(AvCustGUI.dialog.getControlFromTag(kColor1ClickMap)).getLastMouseUpPoint()
                        colorit = ColorMaterial.map.getPixelColor(colormap.getX(),colormap.getY())
                        self.IDrawCrosshair(kColor1ClickMap,colormap.getX(),colormap.getY())
                    avatar = PtGetLocalAvatar()
                    clothing_group = TheCloset[CLxref[clothing_panel][1]]
                    if clothing_group.numberItems > 1:
                        matchingItem = avatar.avatar.getMatchingClothingItem(wornItem.name)
                        if type(matchingItem) == type([]):
                            avatar.avatar.tintClothingItemLayer(matchingItem[0],colorit,layer,0)
                    avatar.avatar.tintClothingItemLayer(wornItem.name,colorit,layer)
                    # if we are messing with the face, color the accessory too! (only on layer 2 color)
                    if clothing_type == kFaceClothingItem and layer == 2:
                        for aitem in clothing_group.accessories:
                            if IsWearing(aitem):
                                avatar.avatar.tintClothingItem(aitem.name,colorit)
                elif controlID == kSkinClickMap:
                    # then this is a skin tinting affair
                    colormap = ptGUIControlClickMap(AvCustGUI.dialog.getControlFromTag(kSkinClickMap)).getLastMouseUpPoint()
                    colorskin = SkinMaterial.map.getPixelColor(colormap.getX(),colormap.getY())
                    self.IDrawCrosshair(kSkinClickMap,colormap.getX(),colormap.getY())
                    avatar = PtGetLocalAvatar()
                    avatar.avatar.tintSkin(colorskin)
                elif controlID == kHairClickMap:
                    # then this is a hair tinting affair
                    
                    #Hair color changes are not available to visitors
                    if IsVisitorPlayer:
                        PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                        return
                    
                    colormap = ptGUIControlClickMap(AvCustGUI.dialog.getControlFromTag(kHairClickMap)).getLastMouseUpPoint()
                    colorit = HairMaterial.map.getPixelColor(colormap.getX(),colormap.getY())
                    self.IDrawCrosshair(kHairClickMap,colormap.getX(),colormap.getY())
                    avatar = PtGetLocalAvatar()
                    # find all the hair tinting items to be colored as hair
                    for item in WornList:
                        # there is hair and there is hair on the face (also), so color both!
                        if item.type == kHairClothingItem or item.coloredAsHair:
                            avatar.avatar.tintClothingItemLayer(item.name,colorit,1)
    
    def IMorphOneItem(self,knobID,itemName):
        "Morph a specific item"
        global TheCloset
        global IsVisitorPlayer
        
        if knobID < kMorphSliderOffset or knobID >= kMorphSliderOffset+kNumberOfMorphs:
            return
        morphKnob = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(knobID))
        morphVal = self.ISliderToMorph(morphKnob.getValue())
        avatar = PtGetLocalAvatar()
        item = TheCloset.getItemByName(itemName)
        if item == None:
            return
        gender = avatar.avatar.getAvatarClothingGroup()

        #save state
        if gender == kFemaleClothingGroup:
            avatar.avatar.setMorph("FFace",knobID-kMorphSliderOffset,morphVal)
        else:
            avatar.avatar.setMorph("MFace",knobID-kMorphSliderOffset,morphVal)
    
    def IMorphItem(self,knobID):
        "Morph the avatar"
        global WornList
        global IsVisitorPlayer
        
        GetClothingWorn() # update clothing list just in case
        # for now, skip out on an invalid control
        if knobID < kMorphSliderOffset or knobID >= kMorphSliderOffset+kNumberOfMorphs:
            return
        morphKnob = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(knobID))
        morphVal = self.ISliderToMorph(morphKnob.getValue())
        avatar = PtGetLocalAvatar()
        gender = avatar.avatar.getAvatarClothingGroup()

        #Limit visitors to not make any physical adjustments
        if IsVisitorPlayer and knobID != kWeightKnob:
            if gender == kFemaleClothingGroup:
                resetVal = avatar.avatar.getMorph("FFace",knobID)
            else:
                resetVal = avatar.avatar.getMorph("MFace",knobID)
            print resetVal  #This is a hack... for some reason I get errors without it!
            morphKnob.setValue(self.IMorphToSlider(resetVal))
            PtShowDialog(xVisitorUtils.kVisitorNagDialog)
            return

        #Save state
        if gender == kFemaleClothingGroup:
            avatar.avatar.setMorph("FFace",knobID-kMorphSliderOffset,morphVal)
        else:
            avatar.avatar.setMorph("MFace",knobID-kMorphSliderOffset,morphVal)
    
    def ITexMorphItem(self,knobID):
        "Texture morph the avatar"
        global IsVisitorPlayer
        
        if knobID <= kTexMorphSliderOffset or knobID > kTexMorphSliderOffset+kNumberOfTexMorphs:
            return
        morphKnob = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(knobID))
        morphVal = self.ISliderToTexMorph(morphKnob.getValue())
        avatar = PtGetLocalAvatar()
        
        if knobID == kAgeTexMorph:
            # the age morph is independent of the other texture morphs
            avatar.avatar.setSkinBlend(4,morphVal)
            
        else:
            #Limit visitors to not make any physical adjustments
            if IsVisitorPlayer:
                #reset knobs
                morphKnob1 = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kEthnic1TexMorph))
                morphKnob2 = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kEthnic2TexMorph))
                morphKnob3 = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kEthnic3TexMorph))
                
                morphKnob1.setValue(self.ITexMorphToSlider(avatar.avatar.getSkinBlend(1)))
                morphKnob2.setValue(self.ITexMorphToSlider(avatar.avatar.getSkinBlend(2)))
                morphKnob3.setValue(self.ITexMorphToSlider(avatar.avatar.getSkinBlend(3)))
                
                #show nag
                PtShowDialog(xVisitorUtils.kVisitorNagDialog)
                return
            
            morphID1 = 0
            morphID2 = 0
            if knobID == kEthnic1TexMorph:
                morphID1 = kEthnic2TexMorph
                morphID2 = kEthnic3TexMorph
            elif knobID == kEthnic2TexMorph:
                morphID1 = kEthnic1TexMorph
                morphID2 = kEthnic3TexMorph
            else:
                morphID1 = kEthnic1TexMorph
                morphID2 = kEthnic2TexMorph
            morphKnob1 = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(morphID1))
            morphKnob2 = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(morphID2))
            morphVal1 = self.ISliderToTexMorph(morphKnob1.getValue())
            morphVal2 = self.ISliderToTexMorph(morphKnob2.getValue())
            total = morphVal + morphVal1 + morphVal2
            if total > 1.0:
                # adjust sliders so the total is <= 1.0
                adjustment = (total - 1.0)/2.0
                morphVal1 -= adjustment
                morphVal2 -= adjustment
                if morphVal1 < 0.0:
                    # since morphVal1 is negative, we adjust the other slider by that amount, and zero morphVal1
                    morphVal2 += morphVal1
                    morphVal1 = 0.0
                if morphVal2 < 0.0:
                    # since morphVal2 is negative, we adjust the other slider by that amount, and zero morphVal2
                    morphVal1 += morphVal2
                    morphVal2 = 0.0

            morphKnob1.setValue(self.ITexMorphToSlider(morphVal1))
            morphKnob2.setValue(self.ITexMorphToSlider(morphVal2))

            
            avatar.avatar.setSkinBlend(knobID-kTexMorphSliderOffset-1,morphVal)
            avatar.avatar.setSkinBlend(morphID1-kTexMorphSliderOffset-1,morphVal1)
            avatar.avatar.setSkinBlend(morphID2-kTexMorphSliderOffset-1,morphVal2)

    def ISetStandardControls(self):
        "Set the color knobs depending on the color of the clothing item panel showing"
        if self.dirty: # has the clothing changed at all?
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).show() # then show the reset button
        else:
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarResetID)).hide() # otherwise, hide it
        
        if PtIsInternalRelease():
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarReadID)).show()
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarSaveID)).show()
        else:
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarReadID)).hide()
            ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(kAvatarSaveID)).hide()

        foundItem = 0
        # get the panel and the worn item
        panelRG = ptGUIControlRadioGroup(AvCustGUI.dialog.getControlFromTag(kPanelsRGID))
        clothing_panel = panelRG.getValue()
        avatar = PtGetLocalAvatar()
        if clothing_panel >= 0 and clothing_panel < len(CLxref):
            clothing_type = CLxref[clothing_panel][0]
            # find the clothing type in what is being worn
            wornitem = FindWornItem(clothing_type)
            if type(wornitem) != type(None):
                descbox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kClothingDesc))
                descbox.setStringW(wornitem.description)
                colorbar1 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName1))
                if wornitem.colorlabel1 == U"":
                    self.IHideColorPicker(kColor1ClickMap)
                else:
                    self.IShowColorPicker(kColor1ClickMap)
                    colorbar1.setStringW(wornitem.colorlabel1)
                colorbar2 = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName2))
                if wornitem.colorlabel2 == U"":
                    self.IHideColorPicker(kColor2ClickMap)
                else:
                    self.IShowColorPicker(kColor2ClickMap)
                    colorbar2.setStringW(wornitem.colorlabel2)
                # special case the face... its the skin tint
                if clothing_type == kFaceClothingItem:
                    skin = avatar.avatar.getTintSkin()
                    self.IDrawPickerThingy(kSkinClickMap,skin)
                    self.IShowColorPicker(kSkinClickMap)
                if clothing_type == kHairClothingItem:
                    self.IShowColorPicker(kHairClickMap)
                    # the hair color is the same of the tinting on the hair clothing item
                    hair = avatar.avatar.getTintClothingItem(wornitem.name,1)
                    self.IDrawPickerThingy(kHairClickMap,hair)
                    color2 = avatar.avatar.getTintClothingItem(wornitem.name,2)
                    self.IDrawPickerThingy(kColor2ClickMap,color2)
                else:
                    color1 = avatar.avatar.getTintClothingItem(wornitem.name)
                    self.IDrawPickerThingy(kColor1ClickMap,color1)
                    # the face sets this above, so don't set it here
                    if clothing_type == kFaceClothingItem:
                        color2 = ptColor(1,1,1,1)
                        clothing_group = TheCloset[CLxref[clothing_panel][1]]
                        hasaccessory = 0
                        for aitem in clothing_group.accessories:
                            if IsWearing(aitem):
                                hasaccessory = 1
                                color2 = avatar.avatar.getTintClothingItem(aitem.name)
                        if hasaccessory:
                            self.IDrawPickerThingy(kColor2ClickMap,color2)
                        else:
                            self.IHideColorPicker(kColor2ClickMap)
                    else:
                        color2 = avatar.avatar.getTintClothingItem(wornitem.name,2)
                        self.IDrawPickerThingy(kColor2ClickMap,color2)
        allMorphsLoaded = 1
        for morphID in range(kNumberOfMorphs):
            knobID = morphID + kMorphSliderOffset
            morphKnob = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(knobID))
            morphKnob.show()
            morphVal = 0
            gender = avatar.avatar.getAvatarClothingGroup()
            try:
                if gender == kFemaleClothingGroup:
                    morphVal = avatar.avatar.getMorph("FFace",morphID)
                else:
                    morphVal = avatar.avatar.getMorph("MFace",morphID)
                morphKnob.setValue(self.IMorphToSlider(morphVal))
            except:
                PtDebugPrint("Some error occurred while setting morph slider #" + str(morphID) + ", morphs probably haven't loaded yet")
                morphKnob.setValue(self.IMorphToSlider(0.0)) # set it to the default
                allMorphsLoaded = 0
                pass
        for texMorphID in range(1,kNumberOfTexMorphs+1):
            knobID = texMorphID + kTexMorphSliderOffset
            morphKnob = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(knobID))
            morphKnob.show()
            try:
                if (knobID == kAgeTexMorph):
                    morphVal = avatar.avatar.getSkinBlend(4)
                else:
                    morphVal = avatar.avatar.getSkinBlend(texMorphID - 1)
                morphKnob.setValue(self.ITexMorphToSlider(morphVal))
            except:
                PtDebugPrint("Some error occurred while setting tex morph slider #" + str(texMorphID) + ", probably haven't loaded yet")
                allMorphsLoaded = 0
                pass
        self.morphsLoaded = allMorphsLoaded
        if not self.morphsLoaded:
            PtAtTimeCallback(self.key, 1, kTimerUpdateMorphs) # we will try again in a second


    
    def IShowColorPicker(self,id):
        if (id == kColor1ClickMap):
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kColor1ClickMap))
        elif (id == kColor2ClickMap):
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kColor2ClickMap))
        elif (id == kHairClickMap):
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kHairClickMap))
        else:
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kSkinClickMap))

        clickMap.enable()
        clickMap.show()
    
    def IHideColorPicker(self,id):
        textBox = None
        if (id == kColor1ClickMap):
            textBox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName1))
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kColor1ClickMap))
            texMap = Color1Map.textmap
        elif (id == kColor2ClickMap):
            textBox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName2))
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kColor2ClickMap))
            texMap = Color2Map.textmap
        elif (id == kHairClickMap):
            textBox = ptGUIControlTextBox(AvCustGUI.dialog.getControlFromTag(kColorbarName1))
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kHairClickMap))
            texMap = Color1Map.textmap
        else:
            clickMap = ptGUIControlValue(AvCustGUI.dialog.getControlFromTag(kSkinClickMap))
            texMap = SkinMap.textmap
        clickMap.disable()
        clickMap.hide()
        if not textBox == None:
            textBox.setString("")
        texMap.clearToColor(ptColor(0,0,0,0))
        texMap.flush()
    
    def IDrawPickerThingy(self,id,color):
        if (id == kColor1ClickMap):
            location = ColorMaterial.map.getColorLoc(color)
        elif (id == kColor2ClickMap):
            location = ColorMaterial.map.getColorLoc(color)
        elif (id == kHairClickMap):
            location = HairMaterial.map.getColorLoc(color)
        elif (id == kSkinClickMap):
            location = SkinMaterial.map.getColorLoc(color)
        else:
            return
        
        if location == None:
            location = ptPoint3(0,0,0)
        
        self.IDrawCrosshair(id,location.getX(),location.getY())
    
    # draw the crosshair, relX and relY are from 0 to 1
    def IDrawCrosshair(self,id,relX,relY):
        if (id == kColor1ClickMap):
            texMap = Color1Map.textmap
            if not ptGUIControl(AvCustGUI.dialog.getControlFromTag(kColor1ClickMap)).isVisible():
                return
        elif (id == kColor2ClickMap):
            texMap = Color2Map.textmap
            if not ptGUIControl(AvCustGUI.dialog.getControlFromTag(kColor2ClickMap)).isVisible():
                return
        elif (id == kHairClickMap):
            texMap = Color1Map.textmap
            if not ptGUIControl(AvCustGUI.dialog.getControlFromTag(kHairClickMap)).isVisible():
                return
        elif (id == kSkinClickMap):
            texMap = SkinMap.textmap
        else:
            return
        
        width = texMap.getWidth()
        height = texMap.getHeight()
        x = int(relX * width)
        y = int(relY * height)
        texMap.clearToColor(ptColor(0,0,0,0))
        self.IDrawClippedRectangle(texMap,x-8,y-2,6,5,width,height)
        self.IDrawClippedRectangle(texMap,x+3,y-2,6,5,width,height)
        self.IDrawClippedRectangle(texMap,x-2,y-8,5,6,width,height)
        self.IDrawClippedRectangle(texMap,x-2,y+3,5,6,width,height)
        texMap.flush()
    
    def IDrawClippedRectangle(self,texMap,x,y,width,height,texMapWidth,texMapHeight):
        black = ptColor(0,0,0,1)
        white = ptColor(1,1,1,1)
        
        x = min(x,texMapWidth-1)
        y = min(y,texMapHeight-1)
        left = max(x,0)
        top = max(y,0)
        right = min(x+width,texMapWidth-1)
        bottom = min(y+height,texMapHeight-1)
        right = max(right,0)
        bottom = max(bottom,0)
        
        texMap.fillRect(left,top,right,bottom,black)
        texMap.frameRect(left,top,right,bottom,white)
    
    def IMorphToSlider(self,morph):
        "convert morph value (-1.0 to 1.0) to slider value (0.0 to 12.0)"
        morph = -morph # flip it since it seems to be backwards
        slider = morph + 1.0 # convert to 0 to 2 range
        slider *= 6.0 # convert to 0 to 12 range
        return slider
    
    def ISliderToMorph(self,slider):
        "convert slider value (0.0 to 12.0) to morph value (1.0 to 1.0)"
        morph = slider / 6.0 # convert to 0 to 2 range
        morph -= 1.0 # convert to -1 to 1 range
        morph = -morph # flip the value since it seems to be backwards
        return morph
        
    def ITexMorphToSlider(self,texMorph):
        "convert texture morph value (0.0 to 1.0) to a slider value (0.0 to 12.0)"
        slider = texMorph * 12.0
        return slider
    
    def ISliderToTexMorph(self,slider):
        "convert slider value (0.0 to 12.0) to a texture morph value (0.0 to 1.0)"
        texMorph = slider / 12.0
        return texMorph

class ClothingItem:
    def __init__(self,clothing,colorType,tintSaturation,inCloset,inClosetColor1,inClosetColor2):
        self.colorType = colorType
        self.tintSaturation1 = tintSaturation
        self.tintSaturation2 = tintSaturation
        self.inCloset = inCloset
        self.inClosetColor1 = inClosetColor1
        self.inClosetColor2 = inClosetColor2
        self.tintValue1 = 1.0
        self.tintValue2 = 1.0
        self.name = ""
        self.type = 0
        self.description = U""
        self.thumbnail = 0
        self.colorlabel1 = U"Color 1"
        self.colorlabel2 = U"Color 2"
        self.accessories = []
        self.meshicon = 0 # is this a category icon and not real clothing?
        self.logofor = "" # is this clothing item a logo for another?
        # parameters for the accessories
        self.groupwith = -1
        self.groupName = ""
        self.accessoryType = -1
        self.wornwith = []
        self.donotwear=0
        self.coloredAsHair=0
        self.lastcolorLayer1=None
        self.lastcolorLayer2=None
        self.nonStandardItem = 0 # does this item need to be in the closet to display?
        self.singlePlayer = 0 # can this item show up in singleplayer?
        self.internalOnly = 0 # is this item only showing up in internal builds?
        self.seasonal = 0 # does this item only show up on certain days?
        # dashes specify a range, commas specify a second, disconnected date
        # seasonTime is a nested tuple. The date: 12/30/03-12/31/03,1/1/04-1/2/04 will be listed as [ [[12,12],[30,31],[03,03]], [[1,1],[1,2],[04,04]] ]
        self.seasonTime = []    # the date that it will show up on (0 for any of these means any, i.e. 0/1/03 means the first day of every month in 2003
                                # while 1/1/0 or 1/1 means January 1st every year
        self.free = 0   # Visitors (free players) are allowed limited clothing.  This is set in the max files by specifying "free" in 
                        # the text option for the Plasma clothing material rollout.
                        
        
        try:
            self.name = clothing[0]
            self.type = clothing[1]
            try:
                if clothing[2] != "":
                    self.description = GetItemName(clothing[2])
            except:
                self.description = U"*"+unicode(clothing[2])+U"*"
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
                    # find what parms there are
                    if ls == "colorlabel1":
                        try:
                            if rs != "":
                                self.colorlabel1 = GetItemName(rs)
                            else:
                                self.colorlabel1 = U""
                        except KeyError:
                            self.colorlabel1 = U"*"+unicode(rs)+U"*"
                    elif ls == "colorlabel2":
                        try:
                            if rs != "":
                                self.colorlabel2 = GetItemName(rs)
                            else:
                                self.colorlabel2 = U""
                        except KeyError:
                            self.colorlabel2 = U"*"+unicode(rs)+U"*"
                    elif ls == "saturationlayer1":
                        self.tintSaturation1 = string.atof(rs)
                    elif ls == "saturationlayer2":
                        self.tintSaturation2 = string.atof(rs)
                    elif ls == "valuelayer1":
                        self.tintValue1 = string.atof(rs)
                    elif ls == "valuelayer2":
                        self.tintValue2 = string.atof(rs)
                    elif ls == "clothingtype":
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
                            PtDebugPrint("AvaCusta: Unknown ClothingType %s" % (rs))
                    elif ls == "accessorytype":
                        self.accessoryType = 0
                    elif ls == "accessory":
                        self.accessoryType = 0
                    elif ls == "incloset":
                        rs = string.lower(rs)
                        if rs == "yes":
                            self.inCloset = 1
                        elif rs == "no":
                            self.inCloset = 0
                        else:
                            PtDebugPrint("AvaCusta: Unknown inCloset type of %s on clothing item %s" % (rs,self.name))
                    elif ls == "inclosetcolor1":
                        rs = string.lower(rs)
                        if rs == "yes":
                            self.inClosetColor1 = 1
                        elif rs == "no":
                            self.inClosetColor1 = 0
                        else:
                            PtDebugPrint("AvaCusta: Unknown inClosetColor1 type of %s on clothing item %s" % (rs,self.name))
                    elif ls == "inclosetcolor2":
                        rs = string.lower(rs)
                        if rs == "yes":
                            self.inClosetColor2 = 1
                        elif rs == "no":
                            self.inClosetColor2 = 0
                        else:
                            PtDebugPrint("AvaCusta: Unknown inClosetColor2 type of %s on clothing item %s" % (rs,self.name))
                    elif ls == "nonstandard":
                        self.nonStandardItem = 1
                    elif ls == "internal":
                        self.internalOnly = 1
                    elif ls == "wornwith":
                        wearlist = rs.split(',')
                        for wearitem in wearlist:
                            self.wornwith.append(wearitem.strip())
                    elif ls == "donotwear":
                        self.donotwear=1
                    elif ls == "coloredashair":
                        self.coloredAsHair=1
                    elif ls == "groupicon":
                        self.meshicon=1
                    elif ls == "islogofor":
                        self.logofor = rs
                    elif ls == "groupname":
                        self.groupName = rs
                    elif ls == "singleplayer":
                        self.singlePlayer = 1
                    elif ls == "needssort":
                        pass # this one was getting annoying
                    elif ls == "free":
                        self.free = 1
                    elif ls == "seasonal":
                        self.seasonal = 1
                        try:
                            ranges = rs.split(',') # split up multiple dates
                            self.seasonTime = []
                            for dateRange in ranges:
                                dates = dateRange.split('-') # split on - for ranges
                                tempRange = []
                                if len(dates) == 1:
                                    # only one date in this one, so we split on / and store it
                                    date = dates[0].split('/')
                                    date = [int(item) for item in date]
                                    if len(date) > 2:
                                        if date[2] < 2000:
                                            date[2] = date[2] + 2000 # convert to 4 digits
                                    else:
                                        date.append(0) # any year (wasn't specified)
                                    tempRange.append(date)
                                else:
                                    # two dates, a min and a max, but we might need to split it up
                                    startDate = dates[0].split('/') # get the starting date
                                    startDate = [int(item) for item in startDate]
                                    if len(startDate) > 2:
                                        if startDate[2] < 2000:
                                            startDate[2] = startDate[2] + 2000 # convert to 4 digits
                                    else:
                                        startDate.append(0) # any year (wasn't specified)
                                    
                                    endDate = dates[1].split('/') # get the ending date
                                    endDate = [int(item) for item in endDate]
                                    if len(endDate) > 2:
                                        if endDate[2] < 2000:
                                            endDate[2] = endDate[2] + 2000 # convert to 4 digits
                                    else:
                                        endDate.append(0) # any year (wasn't specified)
                                    
                                    if (startDate[2] == 0) or (endDate[2] == 0):
                                        startDate[2] = endDate[2] = 0 # make sure that if one year is wild, the other is also
                                    # we don't want wild days or months in a range
                                    if startDate[0] == 0:
                                        startDate[0] = 1
                                    if startDate[1] == 0:
                                        startDate[1] = 1
                                    if endDate[0] == 0:
                                        endDate[0] = 1
                                    if endDate[1] == 0:
                                        endDate[1] = 1
                                    
                                    while not (startDate == endDate):
                                        tempRange.append([date for date in startDate])
                                        # assume every month is 31 days long, it won't really matter when we do our compare
                                        if startDate[1] >= 31: # end of the month
                                            # we have an overlap of some kind, if we are at the beginning of a double, tack this one on a second time
                                            if not (len(tempRange) == len(tempRange)/2*2):
                                                tempRange.append([date for date in startDate])
                                            if startDate[0] >= 12: # end of the year
                                                startDate[0] = 1 # reset to Jan 1st of the next year
                                                startDate[1] = 1
                                                startDate[2] = startDate[2] + 1
                                            else:
                                                startDate[0] = startDate[0] + 1 # reset to the first of the next month
                                                startDate[1] = 1
                                        else:
                                            if startDate[0] == endDate[0] and startDate[2] == endDate[2]: # we have the right month and year, so set to the right day
                                                startDate[1] = endDate[1]
                                            else: # otherwise, advance the day to the end of the month
                                                startDate[1] = 31
                                        # make sure we don't somehow pass the end date
                                        if startDate[2] > endDate[2]:
                                            startDate[2] = endDate[2]
                                    tempRange.append([date for date in endDate])
                                    # if this end date is actually the beginning of a double, tack it on a second time
                                    if not (len(tempRange) == len(tempRange)/2*2):
                                        tempRange.append([date for date in endDate])
                                for i in range(len(tempRange)):
                                    if not (i == i/2*2): # if our i is odd
                                        continue # we skip it, since it is paired with the previous one
                                    startMonth = tempRange[i][0]
                                    startDay = tempRange[i][1]
                                    startYear = tempRange[i][2]
                                    if i+1 >= len(tempRange): # this is the last one
                                        if i == 0: # this was the first one as well
                                            monthRange = [startMonth,startMonth]
                                            dayRange = [startDay,startDay]
                                            yearRange = [startYear,startYear]
                                            self.seasonTime.append([monthRange,dayRange,yearRange])
                                        # otherwise it was the last in our list, which has already been accounted for
                                    else:
                                        endMonth = tempRange[i+1][0]
                                        endDay = tempRange[i+1][1]
                                        endYear = tempRange[i+1][2]
                                        monthRange = [startMonth,endMonth]
                                        dayRange = [startDay,endDay]
                                        yearRange = [startYear,endYear]
                                        self.seasonTime.append([monthRange,dayRange,yearRange])
                            #PtDebugPrint("AvaCusta: The date string "+rs+" was decoded to the following tuple: "+str(self.seasonTime))
                        except:
                            PtDebugPrint("AvaCusta: Malformed date string "+rs+" on clothing "+self.name)
                            self.seasonal = 0
                    else:
                        if ls != "":
                            PtDebugPrint("AvaCusta: Unknown keyword type (%s) on clothing %s" % (ls,self.name))
        except (TypeError, LookupError):
            PtDebugPrint("AvaCusta: some kind of error on clothing " + str(clothing))

class TextureGroup:
    def __init__(self,clothingType,meshName):
        self.clothingType = clothingType
        if clothingType == kShirtClothingItem:
            self.listboxID = kUpperBodyAccLB
        else:
            self.listboxID = kLwrBodyAccLB
        clothinglist = GetAllWithSameGroup(meshName)
        self.clothingItems = []
        for item in clothinglist:
            ctype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
            newitem = ClothingItem(item,ctype,saturation,inCloset,inClosClr1,inClosClr2)
            # make sure we're not supposed to hide the item
            if CanShowClothingItem(newitem):
                if not newitem.meshicon:
                    self.clothingItems.append(newitem)
        # now sort the list so that logos appear directly after the clothing they are logos for
        sortedList = []
        for item in self.clothingItems[:]:
            if item.logofor == "":
                sortedList.append(item)
                self.clothingItems.remove(item)
        # all the ones that aren't logos are now inserted, so we can now throw in all the ones that are logos
        # in the rights spot
        while len(self.clothingItems) > 0:
            startingLen = len(self.clothingItems)
            # we will continue looping until all the items have been moved over
            for item in self.clothingItems[:]:
                for i in range(len(sortedList)):
                    if item.logofor == sortedList[i].name:
                        # insert it here, after the object it's a logo for
                        sortedList.insert(i+1,item)
                        self.clothingItems.remove(item)
                        break
            if startingLen == len(self.clothingItems): # we didn't add anything
                # someone made a mistake in the clothing items, so we will tack on the rest of the clothing, then exit
                for item in self.clothingItems:
                    sortedList.append(item)
                self.clothingItems = []
        # copy the result
        self.clothingItems = sortedList
    
    def __getitem__(self,key):
        "returns one of the items in the list of clothing items"
        return self.clothingItems[key]
    
    def __getslice__(self,i,j):
        return self.clothingItems[i:j]
    
    def __len__(self):
        return len(self.clothingItems)
    
    def type(self):
        return self.clothingType

class ClothingGroup:
    def __init__(self,clothingType,listboxID,numberItems):
        self.clothingType = clothingType
        self.listboxID = listboxID
        self.numberItems = numberItems
        self.accessories = []
        # get the clothing items for this group
        avatar = PtGetLocalAvatar()
        clothinglist = avatar.avatar.getClosetClothingList(clothingType)
        # build the list of clothing items
        self.clothingItems = []
        for item in clothinglist:
            ctype,saturation,inCloset,inClosClr1,inClosClr2 = FindSaturationAndCloset(item[0],item[1])
            newitem = ClothingItem(item,ctype,saturation,inCloset,inClosClr1,inClosClr2)
            # make sure we're not supposed to hide the item
            if CanShowClothingItem(newitem):
                if (listboxID == kUpperBodyOptionsLB or listboxID == kLwrBodyOptionsLB) and newitem.meshicon and GroupHasClothing(newitem):
                    self.clothingItems.append(newitem)
                elif not (listboxID == kUpperBodyOptionsLB or listboxID == kLwrBodyOptionsLB):
                    # only show the object if we are not in the closet, or if we are wearing it
                    self.clothingItems.append(newitem)

    def __getitem__(self,key):
        "returns one of the items in the list of clothing items"
        return self.clothingItems[key]

    def __getslice__(self,i,j):
        return self.clothingItems[i:j]

    def __len__(self):
        return len(self.clothingItems)

    def type(self):
        return self.clothingType

class ClothingCloset:
    def __init__(self):
        global CLxref
        # clothing Groups mapped by listbox ID number
        self.clothingGroups = {}
        for xref in CLxref:
            self.clothingGroups[xref[1]] = ClothingGroup(xref[0],xref[1],xref[4])
        # get the accessories
        avatar = PtGetLocalAvatar()
        acclist = avatar.avatar.getClosetClothingList(kAccessoryClothingItem)
        for accitem in acclist:
            accCI = ClothingItem(accitem,0,0.0,1,0,0) # default is inCloset
            if CanShowClothingItem(accCI):
                group = self.findGroup(accCI.groupwith)
                if type(group) != type(None):
                    # if this is not donotwear, then append at end
                    if not accCI.donotwear:
                        group.accessories.append(accCI)
                    # else insert at the start
                    else:
                        group.accessories.insert(0,accCI)
                else:
                    PtDebugPrint("AvaCusta: no group set for accessory %s" % (accCI.name))

    def __getitem__(self,key):
        try:
            return self.clothingGroups[key]
        except LookupError:
            return None

    def __len__(self):
        return len(self.clothingGroups)

    def keys(self):
        return self.clothingGroups.keys()

    def findGroup(self,clothing_type):
        for group in self.clothingGroups.values():
            if group.clothingType == clothing_type:
                return group
        return None

    def findClothingItem(self,finditem):
        "find the clothing item in the closet, return group and index"
        for group in self.clothingGroups.values():
            # search through the normal clothing
            for idx in range(len(group)):
                if group[idx].name == finditem.name:
                    return (group,idx,0)
            # now search throught the accessories
            for idx in range(len(group.accessories)):
                if group.accessories[idx].name == finditem.name:
                    return (group,idx,1)
        return (None,-1,-1)
    
    def getItemByName(self,itemName):
        "return the clothing item with the desired name"
        for group in self.clothingGroups.values():
            # search through the normal clothing
            for idx in range(len(group)):
                if group[idx].name == itemName:
                    return group[idx]
            # now search throught the accessories
            for idx in range(len(group.accessories)):
                if group.accessories[idx].name == itemName:
                    return group.accessories[idx]
        # didn't find it? search its texture group
        texGroup = TextureGroup(kShirtClothingItem,itemName) # clothing type doesn't matter since we don't use it here
        for idx in range(len(texGroup)):
            if texGroup[idx].name == itemName:
                return texGroup[idx]
        return None

# list boxes will arrange their items as follows: (example assumes 14 items)
# 0  1  2  3  4  5  6
# 7  8  9 10 11 12 13
# but the list box can only display 8 items at a time (4 if the facial options), so the mapping for
# list box to array is as follows (offset is the index of the item to display first, columns is the
# number of total columns needed to display all the items in a 2xN matrix):
# offset,         offset+1,         offset+2,         offset+3
# offset+columns, offset+columns+1, offset+columns+2, offset+columns+3

class ScrollingListBox:
    def __init__(self):
        self.offset = 0 # our current offset
        self.selection = 0 # our current selection (index in our clothing list)
        self.columns = 0 # the number of columns required to show our items
        self.rows = 2 # the number of rows in our listbox (all except for the face accessories have 2 rows)
        self.clothingList = [] # the list of clothing we are managing
        self.listboxID = 0 # the listbox we are managing
    
    def IShowLeftArrow(self):
        if self.offset > 0:
            return 1
        else:
            return 0
    
    def IShowRightArrow(self):
        if self.offset < self.columns - 4:
            return 1
        else:
            return 0
    
    def SetOffset(self,offset):
        if offset >= 0 and offset <= self.columns - 4:
            self.offset = offset
    
    def IncrementOffset(self):
        self.SetOffset(self.offset+1)
    
    def DecrementOffset(self):
        self.SetOffset(self.offset-1)
    
    # sets the internal selection based on the listbox index clicked on, adjusts for the offset
    def SelectItem(self,listboxIndex):
        selection = -1
        # is it one of the top row?
        if listboxIndex >= 0 and listboxIndex < 4:
            selection = listboxIndex + self.offset # add in the offset
        elif self.rows == 2 and listboxIndex >= 4 and listboxIndex < 8: # we are in the second row
            if len(self.clothingList) <= 8: # columns is invalid if we have less then 8 items
                selection = listboxIndex + self.offset
            else:
                selection = listboxIndex + self.offset - 4 + self.columns # add the offset, subtract 4, then add columns to get the right value
        if selection >= 0 and selection < len(self.clothingList):
            self.selection = selection
    
    # sets our selection to what we are wearing
    def SetWhatWearing(self):
        # assumes that WornList is already filled out
        global WornList
        global TheCloset
        for wornitem in WornList:
            # are we a mesh listbox?
            if self.listboxID == kUpperBodyOptionsLB or self.listboxID == kLwrBodyOptionsLB:
                # we are going to need to match up the selection with it's mesh representation
                for idx in range(len(self.clothingList)):
                    if UsesSameGroup(self.clothingList[idx].name, wornitem.name):
                        self.selection = idx
                        return
                self.selection = -1 # can't find it, so don't select anything
            else:
                for idx in range(len(self.clothingList)):
                    if self.clothingList[idx].name == wornitem.name:
                        self.selection = idx
                        return
                self.selection = -1 # can't find it, so don't select anything
    
    # return the selected clothing item
    def GetSelectedItem(self):
        return self.clothingList[self.selection]
    
    def SetClothingList(self,clothingList):
        self.clothingList = clothingList
        if self.rows == 1:
            self.columns = len(self.clothingList)
        else:
            self.columns = int(round(float(len(self.clothingList))/2.0)) # half of the number of items, rounded up
        self.offset = 0
        self.selection = 0
    
    def SetListboxID(self,id):
        if id >= kHairOptionsLB and id <= kAccessOptionsLB:
            self.listboxID = id
        elif id >= kHairAccLB and id <= kAccessAccLB:
            self.listboxID = id
    
        if id == kHeadAccLB: # this list box has only one row of 4, the rest have 2 rows of 4
            self.rows = 1
            self.columns = len(self.clothingList)
        else:
            self.rows = 2
            self.columns = int(round(float(len(self.clothingList))/2.0))
    
    def GetListboxID(self):
        return self.listboxID
    
    def UpdateScrollArrows(self):
        if self.listboxID >= kHairAccLB and self.listboxID <= kAccessAccLB:
            leftArrow = ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(self.listboxID+kIDBtnLeftAccOffset))
            rightArrow = ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(self.listboxID+kIDBtnRightAccOffset))
        else:
            if self.listboxID == kHeadOptionsLB:
                return # there aren't any head options, just accessories
            leftArrow = ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(self.listboxID+kIDBtnLeftOptOffset))
            rightArrow = ptGUIControlButton(AvCustGUI.dialog.getControlFromTag(self.listboxID+kIDBtnRightOptOffset))
        if self.IShowLeftArrow():
            leftArrow.show()
        else:
            leftArrow.hide()
        if self.IShowRightArrow():
            rightArrow.show()
        else:
            rightArrow.hide()
        pass
    
    def UpdateListbox(self):
        listbox = ptGUIControlListBox(AvCustGUI.dialog.getControlFromTag(self.listboxID))
        listbox.clearAllElements()
        listbox.hide()
        listbox.disallowNoSelect()
        if self.listboxID == kUpperBodyAccLB or self.listboxID == kLwrBodyAccLB: # we don't display anything if only one texture option is available
            if len(self.clothingList) == 1:
                return
        if self.listboxID >= kHairAccLB and self.listboxID <= kAccessAccLB:
            if len(self.clothingList) == 0: # no accessories
                return
        listbox.show()
        listbox.enable()
        displayItems = []
        # check to see if we can just copy it over
        if self.rows == 1 and len(self.clothingList) <= 4:
            displayItems = self.clothingList
        elif self.rows == 2 and len(self.clothingList) <= 8:
            displayItems = self.clothingList
        else:
            for i in range(4):
                displayItems.append(self.clothingList[i+self.offset]) # add the first row of items to display
            if self.rows == 2:
                for i in range(4):
                    try:
                        displayItems.append(self.clothingList[i+self.offset+self.columns]) # add the second row
                    except:
                        pass # we probably went to far, so just don't add the non-existant item
        # displayItems now has the correct 8 (or 4) items to show
        for item in displayItems:
            if type(item.thumbnail) != type(0):
                listbox.addImageInBox(item.thumbnail,kCLBImageX,kCLBImageY,kCLBImageWidth,kCLBImageHeight,1)
            else:
                listbox.addStringInBox(item.name,kCLBMinWidth,kCLBMinHeight)
        # now select the correct one (if in view)
        LBIndex = -1
        if (self.rows == 1 and len(self.clothingList) <= 4) or (self.rows == 2 and len(self.clothingList) <= 8):
            # looks like we fit without scrolling, so columns is probably wrong, and we can just use the selection number directly!
            LBIndex = self.selection
        else:
            if self.selection >= self.offset and self.selection <= self.offset+3: # on the top row
                LBIndex = self.selection - self.offset
            elif self.rows == 2 and self.selection >= self.offset+self.columns and self.selection <= self.offset+self.columns+3: # on the bottom row
                LBIndex = self.selection - self.offset - self.columns + 4
        if LBIndex >= 0:
            listbox.setSelection(LBIndex)
