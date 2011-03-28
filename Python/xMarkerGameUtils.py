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
Module: xMarkerGameUtils
Age: global
Author: Tye Hooley
Date: Feb. 2007
Contains various functions and data structures to support the new marker game mini-game server
"""

from PlasmaTypes import *
from PlasmaGameConstants import *
from PlasmaGame import *

#---------------------------------------#
#                                       #
#          Utility Functions            #
#                                       #
#---------------------------------------#

def GetGameClient(gameID):
    "Returns the game client for a specified gameID"
    gameClient = PtGetGameCli(gameID)
    if type(gameClient) != type(None) and PtIsMarkerGame(gameClient.gameTypeID()):
        return gameClient.upcastToMarkerGame()
    PtDebugPrint("xMarkerGameManager.GetGameClient():\tERROR: Cannot find marker game!")
    return None


def GameTypeToString(gameType):  
    "Converts enumerated values from PlasmaGameConstants.PtMarkerGameTypes and converts them to human readable strings"
    if gameType == PtMarkerGameTypes.kMarkerGameQuest: 
        gameString = "Quest"
    elif gameType == PtMarkerGameTypes.kMarkerGameCapture:
        gameString = "Capture"
    elif gameType == PtMarkerGameTypes.kMarkerGameCaptureAndHold:
        gameString = "Capture and Hold"
    else:
        gameString = "UNKNOWN"
    return gameString



#---------------------------------------#
#                                       #
#            Data Structures            #
#                                       #
#---------------------------------------#
class MarkerData:
    "A class interface for housing marker data"
    def __init__(self):
        #Setup Default vars
        self.default = {}
        self.default['id']       = -1   # markerID
        self.default['age']      = None # the age filename the marker is within
        self.default['x']        = -1   # x coord
        self.default['y']        = -1   # y coord
        self.default['z']        = -1   # z coord
        self.default['name']     = None # description of the marker (i.e. when you get it)
        self.default['captured'] = 0    # flag If this marker was captured
        
        self.initDefaultValues()

    def initDefaultValues(self):
        "Sets all variables to their default states"
        self.data = {}
        #Must do a deep copy here; otherwise, we'll overwrite the defaults!
        for x in self.default.keys():
            self.data[x] = self.default[x]

    def __str__(self):
        d = self.data
        retStr = "Marker #%s: age = %s, (%s,%s,%s), description = %s, status: %s" % (d['id'], d['age'], d['x'], d['y'], d['z'], d['name'], self.capturedStatus())
        return retStr

    def capturedStatus(self):
        if self.data['captured']:
            return "captured"
        else:
            return "not captured"





class MarkerGameData:
    "A class interface for housing marker game data"
    def __init__(self):
        self.setup()

    def setup(self):
        #Setup Default vars
        self.default = {}
        self.default['svrGameTypeID']      = -1     #The marker game type
        self.default['svrGameTemplateID']  = ""     #The identifier for the game (GUID)
        self.default['CGZGameNum']         = -1     #CGZ game identifier (stored by game number)
        
        self.default['svrGameClientID']    = -1     #This is the handle for the getting the game server
        
        self.default['numMarkers']         =  0     #The number of markers in the game
        self.default['numCapturedMarkers'] =  0     #The number of captured markers
        self.default['isPlayerJoined']     =  0     #Whether or not the player joined the game (don't start the game until this is true!)

        self.default['svrGameName']        = ""     #The Name of user-created marker games
        self.default['svrGameStarted']     =  0     #If the user-created marker game has been started

        self.default['markers']             = []    #Houses all markers for the game
        self.default['timeLimit']           = -1    #Time limit on the game....

        #Init the game data structure
        self.initDefaultValues()

    def initDefaultValues(self):
        "Sets all variables to their default states"
        self.data = {}
        #Must do a deep copy here; otherwise, we'll overwrite the defaults!
        self.copy(self.default)


    def copy(self, src):
        try:
            for x in src.keys():
                self.data[x] = src[x]
        except:
            PtDebugPrint("ERROR: %s.copy():\tCould not copy from source" %self.__class__.__name__)


    def printData(self):
        print "--------------[Start of Marker Game Data]---------------------"
        for x in self.data.keys():
            if x == 'markers':
                print "\tBEGIN Marker List:"
                markers = self.data[x]
                for marker in markers:
                    print "\t\t%s" % marker.__str__()
                print "\tEND of Marker List"
            else:
                print "\t\tdata[%s] = %s" %(x,self.data[x])
        print "--------------[END of Marker Game Data]---------------------"



class chronicleMarkerGameData(MarkerGameData):
    "A class interface for housing CGZ marker game data (has chronicle hooks)"
    kChronMarkerGameData = "MarkerGameData"

    def __init__(self, existingData = None):
        #Setup Default vars (this is done in the base class)
        self.setup()

        #Init the game data structure
        if existingData == None:
            self.load()
        else:
            if type(existingData) == type(self):
                self.copy(existingData.data)
            else:
                self.copy(existingData)

        self.save()


    def load(self):
        "Initializes all variables from the chronicle, if it doesn't exist, defaults are used."
        vault = ptVault()
        entry = vault.findChronicleEntry(self.kChronMarkerGameData)

        #Only retrieve if the chronicle entry exists; otherwise, we'll just save later.
        if type(entry) == type(None) or type(entry.chronicleGetValue()) == type(None) or entry.chronicleGetValue() == "":
            return

        #Here will do a little magic...
        #To protect against versioning issues, we'll initialize whatever data the user has.
        #All other data will remain at default values....
        temp = eval(entry.chronicleGetValue())
        self.copy(temp)

        #self.printData()

    def save(self):
        "Saves the data to the chronicle"
        #We don't want to store the marker data as it will be way too big in most cases
        #So we'll delete it here....
        saveData = self.data.copy()
        del saveData['markers']
        del saveData['timeLimit']  #also delete this as we don't need it in the chronicle

        vault = ptVault()
        entry = vault.findChronicleEntry(self.kChronMarkerGameData)

        if type(entry) == type(None):
            vault.addChronicleEntry(self.kChronMarkerGameData, 1, str(saveData))
        else:
            entry.chronicleSetValue(str(saveData))
            entry.save()

        #~self.printData()            

    def printData(self):
        vault = ptVault()
        entry = vault.findChronicleEntry(self.kChronMarkerGameData)
    
        if type(entry) == type(None) or type(entry.chronicleGetValue()) == type(None):
            PtDebugPrint("chronicleMarkerGameData.printData():\t****ERROR****  Chronicle Entry does not exist, aborting print command")
            return

        temp = eval(entry.chronicleGetValue())

        print "--------------[Start of CZG Marker Data Chronicle Entry]---------------------"
        for x in temp.keys():
            print "\t\tdata[%s] = %s" %(x,temp[x])
        print "--------------[END of CZG Marker Data Chronicle Entry]---------------------"



