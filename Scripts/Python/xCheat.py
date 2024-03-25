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
"""
This is the python cheats module
Add your cheat function below:
    Be sure to have one argument, which will be a string of whatever they type after the function name
"""


def ListYeeshaPages(args):
    import Plasma
    import xLinkingBookDefs
    vault = Plasma.ptVault()
    psnlSDL = vault.getPsnlAgeSDL()
    if psnlSDL:
        for sdlvar,page in xLinkingBookDefs.xYeeshaPages:
            FoundValue = psnlSDL.findVar(sdlvar)
            print("%s is %d" % (sdlvar, FoundValue.getInt()))
    else:
        print("Could not find personal age SDL")


def GetAllYeeshaPages(args):
    import Plasma
    import xLinkingBookDefs
    vault = Plasma.ptVault()
    psnlSDL = vault.getPsnlAgeSDL()
    if psnlSDL:
        if args == "0":
            newval = 0
            print("xCheat.GetYeeshaPage(): removing all Yeesha pages...")
        else:
            newval = 1
            print("xCheat.GetYeeshaPage(): adding all Yeesha pages...")
        for sdlvar,page in xLinkingBookDefs.xYeeshaPages:
            FoundValue = psnlSDL.findVar(sdlvar)
            FoundValue.setInt(newval)
        vault.updatePsnlAgeSDL(psnlSDL)
    else:
        print("Could not find personal age SDL")


def GetYeeshaPage(args):
    import Plasma
    import xLinkingBookDefs
    vault = Plasma.ptVault()
    psnlSDL = vault.getPsnlAgeSDL()
    if not args:
        print("xCheat.GetYeeshaPage(): ERROR.  Must provide a Yeesha Page number.")
        return
    thispage = "YeeshaPage" + args
    if psnlSDL:
        for sdlvar,page in xLinkingBookDefs.xYeeshaPages:
            if sdlvar == thispage:
                FoundValue = psnlSDL.findVar(sdlvar)
                FoundValue.setInt(1)
                vault.updatePsnlAgeSDL(psnlSDL)
                print("xCheat.GetYeeshaPage(): Done. Have added: ",sdlvar)
                return
        print("xCheat.GetYeeshaPage(): ERROR.  Could not find Yeesha page: ",thispage)
    else:
        print("xCheat.GetYeeshaPage(): ERROR.  Could not find personal age SDL")


def GetAgeJourneyCloths(args):
    import Plasma

    ageChronNode = None
    ageName = Plasma.PtGetAgeName()
    if ageName == "Gira" or ageName == "Garden":
        ageName = "Eder"
    elif ageName == "Teledahn" or ageName == "Garrison" or ageName == "Kadish" or ageName == "Cleft":
        pass
    else:
        return

    vault = Plasma.ptVault()
    chron = vault.findChronicleEntry("JourneyClothProgress")
    ageChronRefList = chron.getChildNodeRefList()
    
    for ageChron in ageChronRefList:
        ageChild = ageChron.getChild()

        ageChild = ageChild.upcastToChronicleNode()

        if ageChild.getName() == ageName:
            ageChronNode = ageChild
            break

    if ageChronNode is None:
        newNode = Plasma.ptVaultChronicleNode(0)
        newNode.setName(ageName)
        newNode.setValue("abcdefg")
        chron.addNode(newNode)
    else:
        ageChronNode.setValue("abcdefg")
        ageChronNode.save()


def GetFissure(args):
    import Plasma
    import xPsnlVaultSDL

    sdl = xPsnlVaultSDL.xPsnlVaultSDL()
    sdl.BatchSet( [ ("TeledahnPoleState", (8,) ), ("KadishPoleState", (8,) ), ("GarrisonPoleState", (8,) ), ("GardenPoleState", (8,) ) ] )

#    ageName = Plasma.PtGetAgeName()
#    if ageName == "Personal":
#        sdl["CleftVisited"] = (0,)


def _AreListsEquiv(list1, list2):
    import copy
    if list1[0] in list2 and len(list1) == len(list2):
        # rearrange list
        list2Copy = copy.copy(list2)
        while list2Copy[0] != list1[0]:
            list2Copy.append(list2Copy.pop(0))

        # check if all values match up now
        for i in range(4):
            if list2Copy[i] != list1[i]:
                return 0

        return 1
    
    return 0


def GenerateCleftSolution(args):
    import Plasma
    import xRandom
    solutionlist = [3,2,5,0]
    cleftSolList = [3,2,5,0]

    while _AreListsEquiv(solutionlist, cleftSolList):
        solutionlist = []
        while len(solutionlist) < 4:
            newint = xRandom.randint(0,6)
            if not newint in solutionlist:
                solutionlist.append(newint)

    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry("BahroCave")
    entry.removeAllNodes()
    if entry != None:
        agelist = ["Teledahn", "Garrison", "Garden", "Kadish"]
        for v in range(len(agelist)):
            newnode = Plasma.ptVaultChronicleNode(0)
            newnode.setName(agelist[v])
            ageVal = str(solutionlist[v])            
            newnode.setValue("1," + ageVal + "," + str(v + 1))
            #newnode.setValue("1," + str(solutionlist[v]) + "," + str(v + 1))
            entry.addNode(newnode)
            print("%s solution is %s" % (agelist[v], ageVal))


def ResetEnding(args):
    import Plasma
    #import PlasmaTypes
    ageName = Plasma.PtGetAgeName()
    if ageName == "Cleft":
        sdl = Plasma.PtGetAgeSDL()
        sdl["clftSceneYeeshaUnseen"] = (1,)
        sdl["clftSceneBahroUnseen"] = (1,)


def ResetBahro(args):
    import Plasma
    #import PlasmaTypes
    ageName = Plasma.PtGetAgeName()
    if ageName == "Cleft":
        sdl = Plasma.PtGetAgeSDL()
        sdl["clftSceneBahroUnseen"] = (1,)


def SndWhatIsLogTrack(args):
    import xSndLogTracks
    xSndLogTracks.WhatIsLog()


def SndIsLogMode(args):
    import xSndLogTracks
    if xSndLogTracks.IsLogMode():
        print("yes")
    else:
        print("not yet")


def SndSetLogMode(args):
    import xSndLogTracks
    xSndLogTracks.SetLogMode()


def SndUnsetLogMode(args):
    import xSndLogTracks
    xSndLogTracks.UnsetLogMode()


def SndSetLogTrack(args):
    import xSndLogTracks
    xSndLogTracks.InitLogTrack(args)


def SndGetTrack(args):
    import xSndLogTracks
    xSndLogTracks.GetTrack()


def KIPhasedAllOn(args):
    import Plasma
    import PlasmaKITypes
    Plasma.PtSendKIMessage(PlasmaKITypes.kKIPhasedAllOn,0)


#===========
# GZ cheat commands
#----

def GZSetGame(args):
    import Plasma
    import PlasmaKITypes
    if not args or args == "" or args == "1":
        # set a default
        gameString = "1 green:greenlt 0:15"
        level = 1
    elif args == "2":
        gameString = "2 green:greenlt 0:15"
        level = 2
    vault = Plasma.ptVault()
    # is there a chronicle for the GZ games?
    entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleGZGames)
    if entry is not None:
        entry.setValue(gameString)
        entry.save()
    else:
        # if there is none, then just add another entry
        vault.addChronicleEntry(PlasmaKITypes.kChronicleGZGames,PlasmaKITypes.kChronicleGZGamesType,args)
    Plasma.PtSendKIMessageInt(PlasmaKITypes.kUpgradeKIMarkerLevel,level)
    # update the 
    Plasma.PtSendKIMessageInt(PlasmaKITypes.kGZUpdated,30)

def ResetGZGame(args):
    "Removes all progress from the main GZ games (i.e. red/green games)"
    import grtzKIMarkerMachine
    grtzKIMarkerMachine.ResetMarkerGame()


def GZGetMarkers(args):
    import Plasma
    import PlasmaKITypes
    try:
        markersToGet = int(args)
    except ValueError:
        markersToGet = 0
    if markersToGet:
        vault = Plasma.ptVault()
        # is there a chronicle for the GZ games?
        entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleGZGames)
        if entry is not None:
            gameString = entry.getValue()
            gargs = gameString.split()
            if len(gargs) == 3:
                try:
                    markerGame = int(gargs[0])
                    colors = gargs[1].split(':')
                    markerGottenColor = colors[0]
                    markerToGetColor = colors[1]
                    outof = gargs[2].split(':')
                    markerGottenNumber = int(outof[0])
                    markerToGetNumber = int(outof[1])
                    newgotten = markerGottenNumber + markersToGet
                    if newgotten > markerToGetNumber:
                        newgotten = markerToGetNumber
                    print("Updating markers gotten to %d from %d" % (newgotten,markerToGetNumber))
                    upstring = "%d %s:%s %d:%d" % (markerGame,markerGottenColor,markerToGetColor,newgotten,markerToGetNumber)
                    entry.setValue(upstring)
                    entry.save()
                    # just pick some marker to have gotten
                    # is there a chronicle for the GZ games?
                    entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleGZMarkersAquired)
                    if entry is not None:
                        markers = entry.getValue()
                        for mnum in range(markersToGet):
                            markerIdx = markers.index(PlasmaKITypes.kGZMarkerAvailable)
                            if markerIdx >= 0 and markerIdx < len(markers):
                                # Set the marker to "captured"
                                if len(markers)-(markerIdx+1) != 0:
                                    markers = markers[:markerIdx] + PlasmaKITypes.kGZMarkerCaptured + markers[-(len(markers)-(markerIdx+1)):]
                                else:
                                    markers = markers[:markerIdx] + PlasmaKITypes.kGZMarkerCaptured
                                print("Update marker #%d - out string is '%s'" % (markerIdx+1,markers))
                                entry.setValue(markers)
                                entry.save()
                    # update the 
                    Plasma.PtSendKIMessage(PlasmaKITypes.kGZUpdated,0)
                    return
                except ValueError:
                    print("xKI:GZ - error trying to read GZGames Chronicle '%s'" % (gameString))
            else:
                print("xKI:GZ - error GZGames string formation error (len=%d)" % (len(gargs)))
        else:
            # if there is none, then error
            print("Error - there is no GZMarker game going!")


def GiveMeMarkerTag(args):
    import Plasma
    import PlasmaKITypes
    Plasma.PtSendKIMessageInt(PlasmaKITypes.kUpgradeKIMarkerLevel,PlasmaKITypes.kKIMarkerFirstLevel)


def GZGiveMeFullAccess(args):
    import Plasma
    import PlasmaKITypes
    Plasma.PtSendKIMessageInt(PlasmaKITypes.kUpgradeKIMarkerLevel,PlasmaKITypes.kKIMarkerNormalLevel)

    #Make sure to update the markers too, as we don't want them to display!!!
    import grtzKIMarkerMachine
    grtzKIMarkerMachine.UpdateGZMarkers(PlasmaKITypes.kGZMarkerUploaded)

    #reset KI's marker display
    resetString = "0 off:off 0:0"
    vault = Plasma.ptVault()
    entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleGZGames)
    if entry is not None:
        entry.setValue(resetString)

    # Finally, update the KI display
    Plasma.PtSendKIMessage(PlasmaKITypes.kGZUpdated,0)


def GZGiveMeGPS(args):
    import Plasma
    import PlasmaKITypes
    vault = Plasma.ptVault()
    psnlSDL = vault.getPsnlAgeSDL()
    if psnlSDL:
        GPSVar = psnlSDL.findVar('GPSEnabled')
        GPSVar.setBool(1)
        vault.updatePsnlAgeSDL(psnlSDL)

def RemoveMarkerTag(args):
    import Plasma
    import PlasmaKITypes
    newlevel = "0"
    vault = Plasma.ptVault()
    # is there a chronicle for the GZ games?
    entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleKIMarkerLevel)
    if entry is not None:
        entry.setValue(newlevel)
        entry.save()
    # is there a chronicle for the GZ games?
    entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleGZGames)
    if entry is not None:
        entry.setValue("0")
        entry.save()
    # is there a chronicle for the GZ games?
    entry = vault.findChronicleEntry(PlasmaKITypes.kChronicleGZMarkersAquired)
    if entry is not None:
        entry.setValue("")
        entry.save()
    Plasma.PtSendKIMessage(PlasmaKITypes.kGZUpdated,0)
    # get rid of the CGZ marker games
    MGs = [ 'MG01','MG02','MG03','MG04','MG05','MG06','MG07','MG08','MG09','MG10','MG11','MG12','MG13','MG14']
    for mg in MGs:
        entry = vault.findChronicleEntry(mg)
        if entry is not None:
            entry.setValue("")
            entry.save()
   
    entry = vault.findChronicleEntry("CGZPlaying")
    if entry is not None:
        entry.setValue("")
        entry.save()
    # remove the marker games from the hidden folder--- later


def CGZKillAll(args):
    import Plasma
    import PlasmaKITypes
    vault = Plasma.ptVault()
    # get rid of the CGZ marker games
    MGs = [ 'MG01','MG02','MG03','MG04','MG05','MG06','MG07','MG08','MG09','MG10','MG11','MG12','MG13','MG14']
    for mg in MGs:
        entry = vault.findChronicleEntry(mg)
        if entry is not None:
            entry.setValue("")
            entry.save()
    entry = vault.findChronicleEntry("CGZPlaying")
    if entry is not None:
        entry.setValue("")
        entry.save()

def ShowHiddenFolder(args):
    import Plasma
    # search thru the age journal folders
    vault = Plasma.ptVault()
    jfolder = None
    master_agefolder = vault.getAgeJournalsFolder()
    if master_agefolder is not None:
        agefolderRefs = master_agefolder.getChildNodeRefList()
        for agefolderRef in agefolderRefs:
            agefolder = agefolderRef.getChild()
            agefolder = agefolder.upcastToFolderNode()
            if agefolder is not None:
                # look for the Hidden folder
                if "Hidden" == agefolder.getFolderName():
                    jfolder = agefolder
                    break
    if jfolder:
        # need to try to find the game
        print("Hidden folder contents:")
        folderRefs = jfolder.getChildNodeRefList()
        for jref in folderRefs:
            jnode = jref.getChild()
            jnode = jnode.upcastToMarkerListNode()
            # is it a marker folder list?
            if jnode is not None:
                # is it named the right one?
                print("markerFolder - ",jnode.getFolderName())
    else:
        print("There is no Hidden folder")


def RemoveHiddenContent(args):
    import Plasma
    import Plasma
    # search thru the age journal folders
    vault = Plasma.ptVault()
    jfolder = None
    master_agefolder = vault.getAgeJournalsFolder()
    if master_agefolder is not None:
        agefolderRefs = master_agefolder.getChildNodeRefList()
        for agefolderRef in agefolderRefs:
            agefolder = agefolderRef.getChild()
            agefolder = agefolder.upcastToFolderNode()
            if agefolder is not None:
                # look for the Hidden folder
                if "Hidden" == agefolder.getFolderName():
                    jfolder = agefolder
                    break
    if jfolder:
        print("Removing content")
        jfolder.removeAllNodes()
    else:
        print("There is no Hidden folder")


#=======================================================
# Dump and import MarkerFolder games to/from a text file
#
# DumpMarkers filename folder [markername]
# ImportMarkers filename folder
#=======================================================

def _DumpEm(f,markerfolder,mfNumber):
    lines = 0
    tab = ""
    print("  -dumping %s" % (markerfolder.getFolderName()))
    f.write("#\n")
    f.write("# %s\n" %(markerfolder.getFolderName()))
    f.write("MG%02d = [ " % (mfNumber))
    f.write("%d, " % (markerfolder.getOwnerID()))
    f.write('"%s", ' % (markerfolder.getOwnerName()))
    f.write("%d, " % (markerfolder.getGameType()))
    f.write("%d ,\\\n" % (markerfolder.getRoundLength()))
    f.write("  [\\\n")
    lines += 4
    # loop through all the markers
    markerRefs = markerfolder.getChildNodeRefList()
    numMarkers = 0
    for markerRef in markerRefs:
        marker = markerRef.getChild()
        marker = marker.upcastToMarkerNode()
        if marker is not None:
            f.write('    [ "%s", ' % (marker.markerGetText()))
            pos = marker.markerGetPosition()
            f.write("%g, %g, %g, " % (pos.getX(),pos.getY(),(pos.getZ())))
            f.write('"%s", ' % (marker.markerGetAge()))
            f.write("%d, %d, %d ],\\\n" % (marker.markerGetTorans(),marker.markerGetHSpans(),marker.markerGetVSpans()))
            lines += 1
            numMarkers += 1
    f.write("  ]\\\n]\n")
    lines += 2
    print("      with %d markers" % (numMarkers))
    return lines


def DumpMarkers(args):
    "args = 'filename folder [markername]'"
    import Plasma
    arglist = args.split()
    if len(arglist) < 2:
        print('ERROR - not enough arguments - DumpMarkers "filename foldername [markername]")')
        return
    filename = arglist[0]
    dfile = open(filename,'w+')
    lines = 0
    totalmfs = 0
    argresidual = args[len(filename)+1:]
    #== find the folder where the markerfolders might be
    # search thru the age journal folders
    vault = Plasma.ptVault()
    jfolder = None
    master_agefolder = vault.getAgeJournalsFolder()
    if master_agefolder is not None:
        agefolderRefs = master_agefolder.getChildNodeRefList()
        for agefolderRef in agefolderRefs:
            agefolder = agefolderRef.getChild()
            agefolder = agefolder.upcastToFolderNode()
            if agefolder is not None:
                # might be a foldername with spaces! so see if it starts with the name
                if argresidual.startswith(agefolder.getFolderName()):
                    jfolder = agefolder
                    argresidual = argresidual[len(agefolder.getFolderName())+1:]
                    break
        if jfolder:
            # loop thru all the journal entries looking for marker folders
            dfile.write("# [ ownerID,ownerName,type,roundLength,\\\n")
            dfile.write("# [ [markerText,x,y,z,age,torans,hspans,vspans],]]\n")
            lines += 2
            jfolderRefs = jfolder.getChildNodeRefList()
            for jfolderRef in jfolderRefs:
                jentry = jfolderRef.getChild()
                jentry = jentry.upcastToMarkerListNode()
                if jentry is not None:
                    # see if we are looking for a particular markerfolder
                    if len(argresidual) > 0 and argresidual == jentry.getFolderName():
                        # only dump the one
                        lines += _DumpEm(dfile,jentry,totalmfs+1)
                        totalmfs += 1
                        break
                    else:
                        # dump all the markerfolders in the folder
                        lines += _DumpEm(dfile,jentry,totalmfs+1)
                        totalmfs += 1
        else:
            print("Could not find folder")
    else:
        print("Master age journal folders could not be found")
    sess = "s"
    if totalmfs == 1:
        sess = ""
    print("==Wrote %d markerfolder%s. %d lines total==" % (totalmfs,sess,lines))
    dfile.write("mgs = [")
    for mg in range(totalmfs):
        dfile.write("(MG%02d,'MG%02d')," % (mg+1,mg+1))
    dfile.write("]\n")
    dfile.close()


def ImportGames(args):
    "args = 'filename folder'"
    import Plasma
    import PlasmaVaultConstants
    arglist = args.split()
    if len(arglist) < 2:
        print('ERROR - not enough arguments - ImportMarkers "filename foldername")')
        return
    filename = arglist[0]
    try:
        dfile = open(filename+'.py','r')
    except IOError:
        print("ERROR - file %d could not be found" % (filename+'.py'))
        return
    dfile.close()
    argresidual = args[len(filename)+1:]
    #== find the folder where the markerfolders might be
    # search thru the age journal folders
    vault = Plasma.ptVault()
    jfolder = None
    master_agefolder = vault.getAgeJournalsFolder()
    if master_agefolder is not None:
        agefolderRefs = master_agefolder.getChildNodeRefList()
        for agefolderRef in agefolderRefs:
            agefolder = agefolderRef.getChild()
            agefolder = agefolder.upcastToFolderNode()
            if agefolder is not None:
                # might be a foldername with spaces! so see if it starts with the name
                if argresidual.startswith(agefolder.getFolderName()):
                    jfolder = agefolder
                    argresidual = argresidual[len(agefolder.getFolderName())+1:]
    if jfolder:
        from importlib import import_module
        for mg in import_module(filename).mgs:
            nMarkerFolder = Plasma.ptVaultMarkerListNode(PlasmaVaultConstants.PtVaultNodePermissionFlags.kDefaultPermissions)
            nMarkerFolder.setFolderName(mg[1])
            nMarkerFolder.setOwnerID(mg[0][0])
            nMarkerFolder.setOwnerName(mg[0][1])
            nMarkerFolder.setGameType(mg[0][2])
            nMarkerFolder.setRoundLength(mg[0][3])
            jfolder.addNode(nMarkerFolder)


def ImportMarkers(args):
    "args = 'filename folder'"
    import Plasma
    import PlasmaVaultConstants
    arglist = args.split()
    if len(arglist) < 2:
        print('ERROR - not enough arguments - ImportMarkers "filename foldername")')
        return
    filename = arglist[0]
    try:
        dfile = open(filename+'.py','r')
    except IOError:
        print("ERROR - file %d could not be found" % (filename+'.py'))
        return
    dfile.close()
    argresidual = args[len(filename)+1:]
    #== find the folder where the markerfolders might be
    # search thru the age journal folders
    vault = Plasma.ptVault()
    jfolder = None
    master_agefolder = vault.getAgeJournalsFolder()
    if master_agefolder is not None:
        agefolderRefs = master_agefolder.getChildNodeRefList()
        for agefolderRef in agefolderRefs:
            agefolder = agefolderRef.getChild()
            agefolder = agefolder.upcastToFolderNode()
            if agefolder is not None:
                # might be a foldername with spaces! so see if it starts with the name
                if argresidual.startswith(agefolder.getFolderName()):
                    jfolder = agefolder
                    argresidual = argresidual[len(agefolder.getFolderName())+1:]
    if jfolder:
        from importlib import import_module
        for mg in import_module(filename).mgs:
            # need to try to find the game to stick these in
            jfolderRefs = jfolder.getChildNodeRefList()
            for jref in jfolderRefs:
                jnode = jref.getChild()
                jnode = jnode.upcastToMarkerListNode()
                # is it a marker folder list?
                if jnode is not None:
                    # is it named the right one?
                    if jnode.getFolderName() == mg[1]:
                        # yes, add the markers to this game
                        for marker in mg[0][4]:
                            nMarker = Plasma.ptVaultMarkerNode(PlasmaVaultConstants.PtVaultNodePermissionFlags.kDefaultPermissions)
                            nMarker.markerSetText(marker[0])
                            pos = Plasma.ptPoint3(marker[1],marker[2],marker[3])
                            nMarker.markerSetPosition(pos)
                            nMarker.markerSetAge(marker[4])
                            nMarker.markerSetGPS(marker[5],marker[6],marker[7])
                            jnode.addNode(nMarker)


def GetChildInfo(args):
    import Plasma
    vault = Plasma.ptVault()
    hoodGUID = vault.getLinkToMyNeighborhood().getAgeInfo().getAgeInstanceGuid()
    print("hoodGUID: ",hoodGUID)

    parentname = None
    agevault = Plasma.ptAgeVault()
    ageinfo = agevault.getAgeInfo()
    #agerules = agevault.getLinkingRules()
    parent = ageinfo.getParentAgeLink()
    if parent == None:
        agename = ageinfo.getAgeFilename()
        print("not a child age.  age = ",agename)
        #print("with linking rules: ",agerules)
        return
    parentinfo = parent.getAgeInfo()
    parentname = parentinfo.getAgeFilename()
    parentGUID = parentinfo.getAgeInstanceGuid()
    print("parentGUID: ",parentGUID)
    
    if parentname == "Neighborhood":
        if hoodGUID == parentGUID:
            print("child of hood:  yes")
        else:
            print("child of hood:  different")
    else:
        print("child of hood:  no")
    #print("linking rules: ",agerules)


def GetSDL(varName):
    """
    GetSDL is used to get the value of an Age SDL variable by name.
    Expects one argument:
     (string) VariableName
    """
    import Plasma

    if not varName:
        print("xCheat.GetSDL(): GetSDL takes one argument: SDL variable name is required.\n Use 'all' to list all variables for the current Age.")
        return

    ageName = Plasma.PtGetAgeName()
    try:
        ageSDL = Plasma.PtGetAgeSDL()
    except:
        print(("xCheat.GetSDL(): Unable to retrieve SDL for '{}'.".format(ageName)))
        return

    varList = []
    if varName == "all":
        if ageName == "Personal":
            varRecord = Plasma.ptVault().getPsnlAgeSDL()
            if varRecord:
                varList = varRecord.getVarList()
        else:
            vault = Plasma.ptAgeVault()
            if vault:
                varRecord = vault.getAgeSDL()
                if varRecord:
                    varList = varRecord.getVarList()

        if not varList:
            print("xCheat.GetSDL(): Couldn't retrieve SDL list.")
            return

        maxlen = len(max(varList, key=len))
        for var in varList:
            try:
                if len(ageSDL[var]) == 0:
                    val = ""
                else:
                    val = ageSDL[var][0]
                print(("xCheat.GetSDL(): {:>{width}}  =  {}".format(var, val, width=maxlen)))
            except:
                print(("xCheat.GetSDL(): Error retrieving value for '{}'.".format(var)))
    else:
        try:
            if len(ageSDL[varName]) == 0:
                print(("xCheat.GetSDL():  SDL variable '{}' is not set.".format(varName)))
            else:
                print(("xCheat.GetSDL(): {}  =  {}".format(varName, ageSDL[varName][0])))
        except:
            print(("xCheat.GetSDL(): SDL variable '{}' not found.".format(varName)))
            return


def SetSDL(varNameAndVal):
    """
    SetSDL is used to set an Age SDL variable by name. It expects two arguments:
    (string) VariableName, (int) NewValue
    """
    import Plasma

    if not varNameAndVal:
        print("xCheat.SetSDL(): SetSDL takes two arguments: SDL variable name and new value are required.")
        return

    varNameAndValList = varNameAndVal.split("_")
    if (len(varNameAndValList) < 2) or varNameAndValList[1] == "":
        print("xCheat.SetSDL(): No new value specified.")
        return
    varName = varNameAndValList[0]
    try:
        newval = int(varNameAndValList[1])
    except ValueError:
        print(("xCheat.SetSDL(): Can't use '{}'. Only numerical SDL values are supported.".format(varNameAndValList[1])))
        return

    ageName = Plasma.PtGetAgeName()
    try:
        ageSDL = Plasma.PtGetAgeSDL()
    except:
        print(("xCheat.SetSDL(): Unable to retrieve SDL for '{}'.".format(ageName)))
        return

    try:
        oldval = ageSDL[varName][0]
    except KeyError:
        print(("xCheat.SetSDL(): SDL variable '{}' not found.".format(varName)))
        return

    if newval == oldval:
        print(("xCheat.SetSDL(): Not changing value, '{}' is already {}.".format(varName, newval)))
        return

    if ageName == "Personal":
        vault = Plasma.ptVault()
        psnlSDL = vault.getPsnlAgeSDL()
        FoundValue = psnlSDL.findVar(varName)
        FoundValue.setInt(newval)
        vault.updatePsnlAgeSDL(psnlSDL)
    else:
        ageSDL.setFlags(varName, 1, 1)
        ageSDL.sendToClients(varName)
        ageSDL[varName] = (newval,)

    print(("xCheat.SetSDL(): Setting '{}' to {} (was {}).".format(varName, newval, oldval)))


def InstaPellets(args):
    import Plasma
    ageName = Plasma.PtGetAgeName()
    if ageName == "Ercana":
        sdl = Plasma.PtGetAgeSDL()
        if args == "":
            print("xCheat.InstantPellets: ERROR.  Must specify a recipe value as argument.")
        else:
            PelletsPresent = sdl["ercaPelletMachine"][0]
            if PelletsPresent:
                print("xCheat.InstantPellets: ERROR.  Must flush current pellets before using this cheat.")
            else:
                iarg = int(args)
                if iarg == 0:
                    iarg = 1
                recipeSDL = (iarg + 300)
                if recipeSDL < 1:
                    recipeSDL = 1
                print("xCheat.InstantPellets: 5 pellets now created with Recipe of %d." % (iarg))
                sdl["ercaPellet1"] = (recipeSDL,)
                sdl["ercaPellet2"] = (recipeSDL,)
                sdl["ercaPellet3"] = (recipeSDL,)
                sdl["ercaPellet4"] = (recipeSDL,)
                sdl["ercaPellet5"] = (recipeSDL,)


def CheckRecipe(args):
    import Plasma
    ageName = Plasma.PtGetAgeName()
    if ageName == "Ercana":
        sdl = Plasma.PtGetAgeSDL()
        Pellet1 = sdl["ercaPellet1"][0]
        Pellet2 = sdl["ercaPellet2"][0]
        Pellet3 = sdl["ercaPellet3"][0]
        Pellet4 = sdl["ercaPellet4"][0]
        Pellet5 = sdl["ercaPellet5"][0]
        pelletList = [Pellet1,Pellet2,Pellet3,Pellet4,Pellet5]
        testVal = 0
        for recipeSDL in pelletList:
            if recipeSDL > 0:
                testVal = 1
                break
        if testVal:
            recipe = (recipeSDL - 300)
            print("xCheat.CheckRecipe: Recipe of current pellet(s) = %d." % (recipe))
        else:
            print("xCheat.CheckRecipe: No pellets currently exist, therefore no recipe.")


def GetPlayerID(self):
    import Plasma
    playerID = Plasma.PtGetLocalPlayer().getPlayerID()
    print("playerID = ",playerID)
