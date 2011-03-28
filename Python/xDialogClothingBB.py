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
"""Module: xDialogClothingBB
Age: global
Author: Mark DeForest
Date: March 13, 2002
This is the temporary clothing dialog handler, that resides on the Blackbar
"""

from Plasma import *
from PlasmaTypes import *

# define the attributes that will be entered in max
KIBlackbar = ptAttribGUIDialog(1,"The Blackbar dialog")

# globals
#----Pants
kNoPants = "nopants"
kNoPantsIdx = -1
kPantIdentifier = "pants"
kPantRadioGroupID = 6
kPantNumChBoxes = 2
PantNames = [ ]

#----Shirts
kNoShirt = "noshirt"
kNoShirtIdx = -1
kShirtIdentifier = "shirt"
kShirtRadioGroupID = 7
kShirtNumChBoxes = 3
ShirtNames = []

#----Shirt colors
kTintRadioGroupID = 8
kTintRGSize = 11
kNoTintIdx = -1
Tints = []
# These are the colors that match the color swatches in the BalckBar dialog
Tints.append( ptColor( 1.0, 1.0, 1.0, 1.0 ) )
Tints.append( ptColor( 0.871, 0.125, 0.125, 1.0 ) )
Tints.append( ptColor( 0.871, 0.492, 0.125, 1.0 ) )
Tints.append( ptColor( 0.492, 0.871, 0.125, 1.0 ) )
Tints.append( ptColor( 0.125, 0.871, 0.125, 1.0 ) )
Tints.append( ptColor( 0.125, 0.871, 0.480, 1.0 ) )
Tints.append( ptColor( 0.125, 0.871, 0.871, 1.0 ) )
Tints.append( ptColor( 0.125, 0.492, 0.871, 1.0 ) )
Tints.append( ptColor( 0.125, 0.125, 0.871, 1.0 ) )
Tints.append( ptColor( 0.492, 0.125, 0.871, 1.0 ) )
Tints.append( ptColor( 0.871, 0.125, 0.512, 1.0 ) )


class xDialogClothingBB(ptModifier):
    "The Clothing dialog modifier, that's on a Blackbar"
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 200

    def OnGUINotify(self,id,control,event):
        "Events from the Blackbar dialog... that it couldn't handle"
        global PantNames
        global ShirtNames
        global Tints
        #PtDebugPrint("dialogClothes::OnGUINotify id=%d, event=%d control=" % (id,event),control )
        # make sure this is from the Blackbar
        if id == KIBlackbar.id:
            # is it one of the buttons?
            if isinstance(control,ptGUIControlRadioGroup):
                rgID = control.getTagID()
                if rgID == kPantRadioGroupID:
                    pantIdx = control.getValue()
                    if pantIdx >= 0 and pantIdx < len(PantNames):
                        self.IWearPants(PantNames[pantIdx])
                elif rgID == kShirtRadioGroupID:
                    shirtIdx = control.getValue()
                    if shirtIdx >= 0 and shirtIdx < len(ShirtNames):
                        self.IWearShirt(ShirtNames[shirtIdx])
                elif rgID == kTintRadioGroupID:
                    colorIdx = control.getValue()
                    #PtDebugPrint("xDialogClothingBB: colorRG says go to colorIdx %d" % (colorIdx))
                    if colorIdx >= 0 and colorIdx < len(Tints):
                        self.IColorShirt(Tints[colorIdx])
                else:
                    PtDebugPrint("xDialogClothingBB: Can't find which radiogroup was hit  rgID=%d" % (rgID))

    def OnClothingUpdate(self):
        "Avatars clothing has changed"
        #PtDebugPrint("xDialogClothingBB: Clothing Update, update clothing dialog")
        avatar = PtGetLocalAvatar()
        self.IUpdateClothingButtons(avatar)

    def IWearPants(self,newPants):
        "Wear some pants"
        global PantNames
        # find our avatar and what they are wearing
        #PtDebugPrint("xDialogClothingBB: Wear pants %s" % (newPants))
        avatar = PtGetLocalAvatar()
        whatsOnIdx = self.IWhatPantsAmIWearing(avatar)
        if whatsOnIdx == kNoPantsIdx or whatsOnIdx >= len(PantNames):
            whatsOn = kNoPants
        else:
            whatsOn = PantNames[whatsOnIdx]
        # Do we need to change pants?
        if newPants != whatsOn:
            # take off the pants we have on, unless we are already not wearing any
            if whatsOn != kNoPants:
                avatar.avatar.removeClothingItem(whatsOn)
            # put on new pants, unless we really want to go without pants
            if newPants != kNoPants:
                # wear the pants
                avatar.avatar.wearClothingItem(newPants)


    def IWearShirt(self,newShirt):
        "Wear a shirt"
        global ShirtNames
        #PtDebugPrint("xDialogClothingBB: Wear shirt %s" % (newShirt))
        # find our avatar and what they are wearing
        avatar = PtGetLocalAvatar()
        whatsOnIdx = self.IWhatShirtAmIWearing(avatar)
        if whatsOnIdx == kNoShirtIdx or whatsOnIdx >= len(ShirtNames):
            whatsOn = kNoShirt
        else:
            whatsOn = ShirtNames[whatsOnIdx]
        # Do we need to change shirt?
        if newShirt != whatsOn:
            # take off the shirt we have on, unless we are already bare chested
            if whatsOn != kNoShirt:
                # save the color
                lastColor = avatar.avatar.getTintClothingItem(whatsOn)
                # if we are going to actually wear another shirt then put it on with the tint we last had
                if newShirt != kNoShirt:
                    avatar.avatar.wearClothingItem(newShirt,0)
                    avatar.avatar.tintClothingItem(newShirt,lastColor)
                else:
                    avatar.avatar.removeClothingItem(whatsOn)
            # else if we did start out barechested then just put on the stupid shirt
            else:
                if newShirt != kNoShirt:
                    # wear the shirt
                    avatar.avatar.wearClothingItem(newShirt)

    def IColorShirt(self,newColor):
        "Colorize the shirt"
        # find our avatar and what they are wearing
        avatar = PtGetLocalAvatar()
        whatsOnIdx = self.IWhatShirtAmIWearing(avatar)
        if whatsOnIdx == kNoShirtIdx or whatsOnIdx >= len(ShirtNames):
            whatsOn = kNoShirt
        else:
            whatsOn = ShirtNames[whatsOnIdx]
        # do we have a shirt?  (Can't color our bare chest!)
        if whatsOn != kNoShirt:
            avatar.avatar.tintClothingItem(whatsOn,newColor)

    def IWhatShirtAmIWearing(self,avatar):
        "Find out what shirt we are already wearing - returns index"
        global ShirtNames
        worn = avatar.avatar.getAvatarClothingList()
        #PtDebugPrint("xDialogClothingBB: I am currently wearing ",worn)
        for item in worn:
            try:
                shirtIdx = ShirtNames.index(item)
                return shirtIdx
            except ValueError:
                # see if its a shirt... maybe they are wearing something that is not in their closet
                if item[-len(kShirtIdentifier):] == kShirtIdentifier:
                    ShirtNames.append(item)
                    try:
                        shirtIdx = ShirtNames.index(item)
                        return shirtIdx
                    except ValueError:
                        pass
        return kNoShirtIdx        # otherwise must not be wearing any shirt
        
    def IWhatPantsAmIWearing(self,avatar):
        "Find out what pants we are already wearing - returns index"
        global PantNames
        worn = avatar.avatar.getAvatarClothingList()
        for item in worn:
            try:
                pantIdx = PantNames.index(item)
                return pantIdx
            except ValueError:
                # see if this is a pant that is not in their closet.... then add it to list of pants
                if item[-len(kPantIdentifier):] == kPantIdentifier:
                    # its a pant, save it
                    PantNames.append(item)
                    try:
                        pantIdx = PantNames.index(item)
                        return pantIdx
                    except ValueError:
                        pass
        return kNoPantsIdx        # otherwise must not be wearing any pants
        
    def IUpdateClothingButtons(self,avatar):
        "Update the Clothing buttons to what is currently available and what is being worn"
        global PantNames
        global ShirtNames
        global Tints
        # load the clothing in the closet into top listbox
        rack = avatar.avatar.getClosetClothingList(0)
        # reset the current set of pants
        PantNames = []
        # search thru our rack to find shirts and pants
        for item in rack:
            if item[-len(kPantIdentifier):] == kPantIdentifier:
                # its a pant, save it
                PantNames.append(item)
        # reset the current set of shirts
        ShirtNames = []
        # search thru our rack to find shirts and pants
        for item in rack:
            if item[-len(kShirtIdentifier):] == kShirtIdentifier:
                # its a shirt, save it
                ShirtNames.append(item)
        #PtDebugPrint("Our closet looks like this:")
        #PtDebugPrint("  pants  -",PantNames)
        #PtDebugPrint(" shirts -",ShirtNames)
        # get the clothing that us being worn
        pantsWornIdx = self.IWhatPantsAmIWearing(avatar)
        shirtWornIdx = self.IWhatShirtAmIWearing(avatar)
        # set the pants radio group
        psRG = ptGUIControlRadioGroup(KIBlackbar.dialog.getControlFromTag(kPantRadioGroupID))
        if pantsWornIdx != psRG.getValue():
            if pantsWornIdx == kNoPantsIdx:
                PtDebugPrint("xDialogClothingBB: currently wearing no pants")
                psRG.setValue(-1)   # there is nothing that is selected for nopant wearers
            else:
                if pantsWornIdx < kPantNumChBoxes:
                    psRG.setValue(pantsWornIdx)
                else:
                    PtDebugPrint("xDialogClothingBB: currently wearing unknown pants %s" % (PantNames[pantsWornIdx]))
                    psRG.setValue(-1)   # there is nothing that is selected for nopant wearers
        # set the pants radio group
        ssRG = ptGUIControlRadioGroup(KIBlackbar.dialog.getControlFromTag(kShirtRadioGroupID))
        if shirtWornIdx != ssRG.getValue():
            if shirtWornIdx == kNoShirtIdx:
                PtDebugPrint("xDialogClothingBB: currently wearing no shirt")
                ssRG.setValue(-1)   # there is nothing that is selected for barechested
            else:
                if shirtWornIdx < kShirtNumChBoxes:
                    ssRG.setValue(shirtWornIdx)
                else:
                    PtDebugPrint("xDialogClothingBB: currently wearing unknown shirt %s" % (ShirtNames[shirtWornIdx]))
                    ssRG.setValue(-1)   # there is nothing that is selected for barechested
        # get current color
        scRG = ptGUIControlRadioGroup(KIBlackbar.dialog.getControlFromTag(kTintRadioGroupID))
        if shirtWornIdx != kNoShirtIdx:
            # make sure that it is enabled
            scRG.setVisible(1)
            # need to set the color to what it already is... if we can find it...?
            coloridx = self.IWhatColorShirt(avatar)
            #PtDebugPrint("Color index found %d" % (coloridx))
            if coloridx != kNoTintIdx:
                scRG.setValue(coloridx)
            else:
                scRG.setValue(-1)   # don't know this color, set no selection
        else:
            # we need to disable the color buttons
            scRG.setVisible(0)
            scRG.setValue(kNoTintIdx)

    def IWhatColorShirt(self,avatar):
        "Find out what color shirt I am was  - returns index"
        global Tints
        global ShirtNames
        # get the shirt that I'm wearing
        shirtIdx = self.IWhatShirtAmIWearing(avatar)
        colorIdx = kNoTintIdx
        if shirtIdx != kNoShirtIdx and shirtIdx < len(ShirtNames):
            shirtColor = avatar.avatar.getTintClothingItem(ShirtNames[shirtIdx])
            # try to match the color with the ones we already know
            try:
                colorIdx = Tints.index(shirtColor)
            except ValueError:
                colorIdx = kNoTintIdx
        return colorIdx
        
