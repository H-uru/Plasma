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
Author: Karl Johnson
Date: April 16, 2007
"""

from Plasma import *
from PlasmaTypes import *
from PlasmaKITypes import *
from PlasmaVaultConstants import *
import string

#=============================================================
# define the attributes that will be entered in max
#=============================================================
ImagerName = ptAttribString(1,"Name of the Imager")
FontColor = ptAttribString(2,"Color: Red,Green,Blue,Alpha")
FontSize = ptAttribInt(3,"Font Size")
FontFace = ptAttribString(4,"Font Face")
TextXOffset = ptAttribInt(7,"Text X Offset", 0, (-10000,1000000))
TextYOffset = ptAttribInt(8,"Text Y Offset", 0, (-10000,1000000))
UpdateTime = ptAttribFloat(9,"Update Interval", 0, (0,100))
IncAmount = ptAttribInt(10,"Increment px amount per update.")
ImagerMap1 = ptAttribDynamicMap(11, "The Dynamic Texture Map1")
ImagerMap2 = ptAttribDynamicMap(12, "The Dynamic Texture Map2")
ImagerMap3 = ptAttribDynamicMap(13, "The Dynamic Texture Map3")
ImagerMap4 = ptAttribDynamicMap(14, "The Dynamic Texture Map4")
#----------
# globals
#----------
kFontColor = []
kTextFontSize = 0
kTextFontFace = ""
kTextXStart = 0
kTextYStart = 0
kUpdateTime = 0
kMessage = ""
kTextXPos = []
kIncAmount = 0
kCursorStart = [0,0,0,0]
kCursorEnd = [0,0,0,0]
CurrentMessage = ["","","",""]
kNextChar = [0,0,0,0]
kFirstChar = [0,0,0,0]
kTextWidth = [0,0,0,0]
kLastUpdate = 0
kNewChar = 0


#====================================

class islmMemorialImager(ptModifier):

    def __init__(self):
        global Instance
        ptModifier.__init__(self)
        Instance = self
        self.id = 5105
        self.version = 1
        print "islmMemorialImager: init  version=%d.%d"%(self.version,3)
    ############################
    def OnServerInitComplete(self):
        
        global kTextFontSize
        global kFontColor
        global kTextFontFace
        global kTextXStart
        global kTextYStart
        global kUpdateTime
        global kCharWidth
        global kMessage
        global kTextXPos
        global kIncAmount
        global kCursorStart
        global kCursorEnd
        global CurrentMessage
        global kLastUpdate

#        ageSDL = PtGetAgeSDL()
#        ageSDL.sendToClients("MemorialImagerStartTime")
#        ageSDL.setFlags("MemorialImagerStartTime", 1, 1)
#        ageSDL.setNotify(self.key, "MemorialImagerStartTime", 0.0)

        kTextFontSize = FontSize.value
        kTextFontFace = FontFace.value
        kTextXStart = 0
        kTextYStart = 0
        kUpdateTime = float(UpdateTime.value)
        kTextXPos = [kTextXStart,kTextXStart,kTextXStart,kTextXStart]
        kIncAmount = IncAmount.value

        kFontColor = FontColor.value.split(",")
        #print "islmMemorialImager: Font Color: R:%s,G:%s,B:%s,A:%s" % (float(kFontColor[0]),float(kFontColor[1]),float(kFontColor[2]),float(kFontColor[3]))
        kFontColor = ptColor(red=float(kFontColor[0]),green=float(kFontColor[1]),blue=float(kFontColor[2]),alpha=float(kFontColor[3]))
        self.UpdateMarqueeMessage()
        kLastUpdate = PtGetDniTime()
        if kMessage == "":
            self.setTimerCallback(30)
            return
        self.getMaxChars()
        self.setTimerCallback(kUpdateTime)

    ############################
    def OnTimer(self,id):
        if kMessage == "":
            self.UpdateMarqueeMessage()
            return
        self.UpdateMarqueeStrPosition(kMessage)
        self.UpdateMarquee(ImagerMap1,0)
        self.UpdateMarquee(ImagerMap2,1)
        self.UpdateMarquee(ImagerMap3,2)
        self.UpdateMarquee(ImagerMap4,3)
        self.setTimerCallback(kUpdateTime)
    ############################
    def UpdateMarquee(self, ImagerMap, x):
        imagertext = CurrentMessage[x]
        #print "islmMemorialImager: kTextXPos:%s \t kTextYStart:%s \n imagertext:%s" % (str(kTextXPos[x]),str(kTextYStart),str(imagertext)) 
        ImagerMap.textmap.clearToColor(ptColor(0,0,0,0))
        ImagerMap.textmap.drawText(kTextXPos[x],kTextYStart,imagertext)
        ImagerMap.textmap.flush()
    ############################
    def UpdateMarqueeStrPosition(self, imgMessage):
        global kTextWidth
        global kCursorStart
        global kCursorEnd
        global CurrentMessage
        global kNextChar
        global kFirstChar
        global kLastUpdate
        #print "islmMemorialImager: PtGetDniTime: " + str(PtGetDniTime()) + "kLastUpdate: " + str(kLastUpdate) 
        movedSince = int(((long(PtGetDniTime()) - long(kLastUpdate)) / kUpdateTime) * kIncAmount)
        #print "islmMemorialImager: Distance to move:" , movedSince
        kLastUpdate = PtGetDniTime()
        for x in range(4):
            #print "islmMemorialImager: Imager%d \n kcursorstart:%d \t kcursorend:%d" % (x,kCursorStart[x],kCursorEnd[x])
            self.UpdateMarqueeOffset(x,movedSince)
            #print "islmMemorialImager: kTextXPos[%d]: %d" % (x, kTextXPos[x])
            self.UpdateStartCursor(x,imgMessage)
            self.UpdateEndCursor(x,imgMessage)
            
    ############################
    def UpdateStartCursor(self, x, imgMessage):
        global kCursorStart
        global kFirstChar
        global kNewChar

        if kNewChar == 1:

            CurrentMessage[x] = ""
            if kCursorStart[x] > len(imgMessage) - 1:
                kCursorStart[x] = 1
                if x == 0:
                    self.UpdateMarqueeMessage()                
            else:
                kCursorStart[x] = kCursorStart[x] + 1
            if kCursorStart[x] <= len(imgMessage) - 1:
                (kFirstChar[x],z) = ImagerMap1.textmap.calcTextExtents(kMessage[kCursorStart[x]])
            elif kCursorStart[x] > len(imgMessage) - 1:
                (kFirstChar[x],z) = ImagerMap1.textmap.calcTextExtents(kMessage[0])
            kNewChar = 0

    ############################
    def UpdateEndCursor(self, x, imgMessage):
        global kTextWidth
        global kCursorEnd
        global kNextChar
        global CurrentMessage
        for i in range(len(imgMessage)):
            (kTextWidth[x],z) = ImagerMap1.textmap.calcTextExtents(CurrentMessage[x])
            if (((kTextXPos[x] - kTextXStart) + kTextWidth[x]) + kNextChar[x]) > ImagerMap1.textmap.getWidth(): break
            
            #print "islmMemorialImager: Value of offset plus the nextChar width", ((kTextXPos[x] - kTextXStart[x]) + kTextWidth[x])
            if kCursorEnd[x] > len(imgMessage) - 1:
                kCursorEnd[x] = 0
            else:
                kCursorEnd[x] = kCursorEnd[x] + 1
            
            if kCursorEnd[x] <= len(imgMessage) - 1:
                (kNextChar[x],z) = ImagerMap1.textmap.calcTextExtents(kMessage[kCursorEnd[x]])
            elif kCursorEnd[x] > len(imgMessage) - 1:
                (kNextChar[x],z) = ImagerMap1.textmap.calcTextExtents(kMessage[0])

            if kCursorStart[x] < kCursorEnd[x]:
                CurrentMessage[x] = imgMessage[kCursorStart[x]:kCursorEnd[x]]
            if kCursorStart[x] > kCursorEnd[x]:
                CurrentMessage[x] = imgMessage[kCursorStart[x]:len(imgMessage)] + kMessage[0:kCursorEnd[x]]

    ############################
    def UpdateMarqueeOffset(self, x, movedSince):
        global kTextXPos
        global kNewChar

        if kTextXPos[x] <= (kTextXStart - kFirstChar[x]) + movedSince:
            kTextXPos[x] = kTextXStart - (((kTextXStart - kFirstChar[x]) + movedSince) - kTextXPos[x])
            kNewChar = 1
        else:
            kTextXPos[x] = kTextXPos[x] - movedSince
        #print "islmMemorialImager: Updating Offset to:",kTextXPos[x]
    ############################
    def UpdateMarqueeMessage(self):
        global kMessage
        global kLastUpdate
        
        inbox = ptVault().getGlobalInbox()
        inboxChildList = inbox.getChildNodeRefList()
        
        for child in inboxChildList:
            PtDebugPrint("islmMemorialImager: looking at node " + str(child),level=kDebugDumpLevel)
            node = child.getChild()
            folderNode = node.upcastToFolderNode()
            if type(folderNode) != type(None):
                PtDebugPrint("islmMemorialImager: node is named %s" % (folderNode.getFolderName()),level=kDebugDumpLevel)
                if folderNode.getFolderName() == "MemorialImager":
                    folderNodeChildList = folderNode.getChildNodeRefList()
                    for folderChild in folderNodeChildList:
                        PtDebugPrint("islmMemorialImager: looking at child node " + str(folderChild),level=kDebugDumpLevel)
                        childNode = folderChild.getChild()
                        textNode = childNode.upcastToTextNoteNode()
                        if type(textNode) != type(None):
                            PtDebugPrint("islmMemorialImager: child node is named %s" % (textNode.getTitle()),level=kDebugDumpLevel)
                            if textNode.getTitle() == "MemorialImager":
                                if textNode.getText() == "":
                                    if kMessage == "":
                                        self.setTimerCallback(30)
                                    #print "islmMemorialImager: No message was found."
                                
                                elif kMessage != textNode.getText():
                                    oldmessage = kMessage
                                    kMessage = textNode.getText()
                                    self.setMarquee(ImagerMap1,0)
                                    self.setMarquee(ImagerMap2,1)
                                    self.setMarquee(ImagerMap3,2)
                                    self.setMarquee(ImagerMap4,3)
                                    self.getMaxChars()
                                    if oldmessage == "":
                                        kLastUpdate = PtGetDniTime()
                                        self.setTimerCallback(kUpdateTime)
                                PtDebugPrint("islmMemorialImager: Marquee contents are '%s'" % (kMessage),level=kDebugDumpLevel)
                                return
        self.setTimerCallback(30)
        #print "islmMemorialImager: Message Node not found."
        
        #print "islmMemorialImager: Message Updated"
        #print "islmMemorialImager: Message Length:", len(kMessage)
    ############################
    
    #def SetMemorialSDL(self):
    #    #print "islmMemorialImager: Updating SDL with the new time value."
    #    ageSDL = PtGetAgeSDL()
    #    ageSDL["MemorialImagerStartTime"] = (PtGetDniTime(),)
    
    ############################
#    def syncMessagePosition(self):
#        global kCursorStart
#        global kLastUpdate
#        if kMessage == "":
#            self.UpdateMarqueeMessage()
#            return
#        #print "islmMemorialImager: Synching my imager to the age owners."
#        ageSDL = PtGetAgeSDL()
#        updatetime = long(PtGetDniTime()) - long(ageSDL["MemorialImagerStartTime"][0])
#        movedsince = int((updatetime / kUpdateTime) * kIncAmount)
#        #print "islmMemorialImager: UpdateTime: " + str(updatetime) + " \t MovedSince: " + str(movedsince)
#        textWidth = 0
#        cursor = len(kMessage)
#        #print "islmMemorialImager: Cursor: ", cursor
#        for i in range(len(kMessage)):
#            (textWidth,z) = ImagerMap1.textmap.calcTextExtents(kMessage[0:cursor])
#            if textWidth < movedsince: break
#            cursor = cursor - 1
#            kCursorStart[0] = cursor
#
#        kLastUpdate = PtGetDniTime()
#
    ############################
    def getMaxChars(self):
        global kTextWidth
        global CurrentMessage
        global kCursorEnd
        global kNextChar
        global kFirstChar
        global kCursorStart
        global kMessage
        if not len(kMessage):
            return
        
        textWidth = 0
        startposdistance = int(len(kMessage) / 4)
        i = 1
        for x in range(4):
            StartCursor = kCursorStart[x]
            i = StartCursor
            for g in range(len(kMessage)):

                if i >= StartCursor:
                    testmessage = kMessage[StartCursor:i]
                elif i < StartCursor:
                    testmessage = kMessage[StartCursor:len(kMessage)] + kMessage[0:i-(len(kMessage) * (i / len(kMessage)))]
                
                (textWidth,z) = ImagerMap1.textmap.calcTextExtents(testmessage)
                if textWidth > ImagerMap1.textmap.getWidth(): break
                i = i + 1
                if i > len(kMessage):
                    i = 0

            i = i - 1
            
            if not len(testmessage):
                print "islmMemorialImager: Message Length = 0"
                kMessage = ""
                return
            testmessage = testmessage[0:len(testmessage) - 1]
            CurrentMessage[x] = testmessage
            kCursorEnd[x] = (len(testmessage) + i)-(len(kMessage) * (i / len(kMessage)))
            (kFirstChar[x], z) = ImagerMap1.textmap.calcTextExtents(testmessage[0])
            (kTextWidth[x],z) = ImagerMap1.textmap.calcTextExtents(testmessage)
            (kNextChar[x], z) = ImagerMap1.textmap.calcTextExtents(kMessage[i-(len(kMessage) + 1 * (i / len(kMessage))) + 1])

            if x < 3:
                kCursorStart[x+1] = int(StartCursor + startposdistance) - (len(kMessage) * ((StartCursor + startposdistance) / len(kMessage)))
            kTextXPos[x] = kTextXStart

    ############################
    
    def setMarquee(self, ImagerMap, x):

        ImagerMap.textmap.clearToColor(ptColor(0,0,0,0))  
        ImagerMap.textmap.setTextColor(kFontColor, true)
        ImagerMap.textmap.setFont(kTextFontFace,kTextFontSize)
        ImagerMap.textmap.setJustify(PtJustify.kLeftJustify)  
        ImagerMap.textmap.setLineSpacing(0)
        ImagerMap.textmap.drawText(kTextXPos[x],kTextYStart,kMessage)
        ImagerMap.textmap.flush()
    ############################

    def setTimerCallback(self, timerlength):
        PtClearTimerCallbacks(self.key)
        PtAtTimeCallback(self.key,timerlength,1)