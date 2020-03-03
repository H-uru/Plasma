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

from Plasma import *
from PlasmaTypes import *
import random

# These are helper functions for the special state variables that need custom code.
def Ayhoheek5Man1State(VARname, NewSDLValue):
    PtDebugPrint("nb01EmgrPhase0.Ayhoheek5Man1State(): Attempting to set '{}' to a value of {}".format(VARname, NewSDLValue))

    nb01Ayhoheek5Man1StateMaxINT = 2
    
    if NewSDLValue > nb01Ayhoheek5Man1StateMaxINT:
        PtDebugPrint("ERROR: nb01EmgrPhase0.Ayhoheek5Man1State:\tERROR: Variable '{}'' expected range from  0-{}. Received value of {}.".format(VARname, nb01Ayhoheek5Man1StateMaxINT, NewSDLValue))
        return

    elif NewSDLValue == 0:
        PtDebugPrint("DEBUG: nb01EmgrPhase0.Ayhoheek5Man1State:\tPaging out 5 Man Heek table completely.")
        PtPageOutNode("nb01Ayhoheek5Man1State")
        PtPageOutNode("nb01Ayhoheek5Man1Dead")

    elif NewSDLValue == 1:
        PtDebugPrint("DEBUG: nb01EmgrPhase0.Ayhoheek5Man1State:\tPaging in broken 5 Man Heek table.")
        PtPageInNode("nb01Ayhoheek5Man1Dead")
        PtPageOutNode("nb01Ayhoheek5Man1State")

    elif NewSDLValue == 2:
        PtDebugPrint("DEBUG: nb01EmgrPhase0.Ayhoheek5Man1State:\tPaging in functional 5 Man Heek table.")
        PtPageInNode("nb01Ayhoheek5Man1State")
        PtPageOutNode("nb01Ayhoheek5Man1Dead")
        
    else:
        PtDebugPrint("ERROR: nb01EmgrPhase0.Ayhoheek5Man1State:\tERROR: Invalid value ({}) for variable '{}'.".format(NewSDLValue, VARname))
        return


class nb01EmgrPhase0(ptResponder):

    # State options for Neighborhood decoration randomization
    HoodDecorations = {
        "nb01ClockVis": (0, 1),
        "nb01GardenFungusVis": (0, 1),
        "nb01DestructionCracksVis": (0, 1),
        "nb01LanternsVis": (0, 1),
        "nb01LampOption01Vis": (0, 1),
        "nb01OldImager01Vis": (0, 1),
        "nb01OldImager02Vis": (0, 1),
        "nb01WaterfallTorchesVis": (0, 1),
        "nb01ResidenceAdditionsVis": (0, 1),
        "nb01StainedWindowOption": (0, 1, 2)
    }

    # These booleans will page in/out the page sharing their name.
    SimplePagingVars = [
        "nb01LinkBookGarrisonVis",
    ]

    # This maps a state variable to a helper function which
    # will be called on updates to that variable.
    #   Calling convention is func(varName, newValue)
    SpecialPagingVars = {
        'nb01Ayhoheek5Man1State': Ayhoheek5Man1State,
    }

    # Link Room Pedestal randomization options
    Pedestals = {
        "Left": {
            #  ----
            #"sdlState": "nb01LinkBookGarrisonVis",
            #"sdlGlass": "nb01StainedWindowOption",
            #"Books" : {
            #    1: {"Name": "Gahreesen", "Glasses": (1, 2)},
            #}
            # ----
            # The Gahreesen Stained Glass doesn't observe the proper on/off rules
            # So we won't try to regulate the Left position
        },
        "Center": {
            "sdlState": "nb01LinkBookEderToggle",
            "sdlGlass": "nb01StainedGlassEders",
            "Books" : {
                #1: Kemo -- deprecated
                2: {"Name": "EderDelin", "Glasses": (1, 2, 3)},
                3: {"Name": "EderTsogal", "Glasses": (4, 5, 6)},
            }
        },
        "Right": {
            "sdlState": "nb01LinkBookGZVis",
            "sdlGlass": "nb01StainedGlassGZ",
            "Books" : {
                1: {"Name": "GreatZero", "Glasses": (1, 2, 3)},
            }
        }
    }

    def __init__(self):
        ptResponder.__init__(self)
        random.seed()
        self.id = 5222

        self.version = 7
        PtDebugPrint("__init__nb01EmgrPhase0 v.{}".format(self.version))

        self.PedestalSDL = []
        self._pedestalSDLValues = {}
        self._ageVault = None
        self._updateAgeSDL = False

        # Store these in a list for convenience
        for position in self.Pedestals:
            if self.Pedestals[position]:
                self.PedestalSDL.append(self.Pedestals[position]["sdlState"])
                self.PedestalSDL.append(self.Pedestals[position]["sdlGlass"])

    def OnServerInitComplete(self):
        ageSDL = PtGetAgeSDL()
        if not ageSDL:
            PtDebugPrint("nb01EmgrPhase0.OnServerInitComplete:\tNo ageSDL. We're done here.")
            return

        for sdlVar in self.PedestalSDL:
            ageSDL.setFlags(sdlVar, 1, 1)
            ageSDL.sendToClients(sdlVar)
            ageSDL.setNotify(self.key, sdlVar, 0.0)
            self._pedestalSDLValues[sdlVar] = ageSDL[sdlVar][0]
            PtDebugPrint("nb01EmgrPhase0.OnServerInitComplete(): Pedestal SDL variable {} = {}".format(sdlVar, self._pedestalSDLValues[sdlVar]))

        for sdlVar in self.SimplePagingVars:
            ageSDL.setFlags(sdlVar, 1, 1)
            ageSDL.sendToClients(sdlVar)
            ageSDL.setNotify(self.key, sdlVar, 0.0)
            self.IManageSimplePagingVar(sdlVar)

        for sdlVar in self.SpecialPagingVars:
            ageSDL.setFlags(sdlVar, 1, 1)
            ageSDL.sendToClients(sdlVar)
            ageSDL.setNotify(self.key, sdlVar, 0.0)
            self.SpecialPagingVars[sdlVar](sdlVar, ageSDL[sdlVar][0])

        # Set up the linking room pedestals.
        if self.sceneobject.isLocallyOwned():
            PtDebugPrint("nb01EmgrPhase0.OnServerInitComplete(): Check all pedestal books and stained glasses for sanity.")
            for position in self.Pedestals:
                if self.Pedestals[position]:
                    self.IManageLinkPedestal(position)

        # Age State Randomization
        # This used to be in the Nexus, but that would never affect server-generated hoods
        if self.sceneobject.isLocallyOwned():
            self._RandomizeNeighborhood()

        # Save the AgeVaultSDL if we need to...
        self._SaveSDL()

    def OnSDLNotify(self, VARname, SDLname, PlayerID, tag):
        ageSDL = PtGetAgeSDL()
        PtDebugPrint("nb01EmgrPhase0.OnSDLNotify():\tVARname: {}, SDLname: {}, tag: {}, value: {}".format(VARname, SDLname, tag, ageSDL[VARname][0]))

        if VARname in self.SimplePagingVars:
            self.IManageSimplePagingVar(VARname)

        elif VARname in self.SpecialPagingVars.keys():
            NewSDLValue = ageSDL[VARname][0]
            self.SpecialPagingVars[VARname](VARname, NewSDLValue)

        elif VARname in self.PedestalSDL:
            self._pedestalSDLValues[VARname] = ageSDL[VARname][0]
            PtDebugPrint("nb01EmgrPhase0.OnSDLNotify():\t{} = {}".format(VARname, self._pedestalSDLValues[VARname]))

        else:
            PtDebugPrint("ERROR: nb01EmgrPhase0.OnSDLNotify():\tVariable '{}' was not recognized.".format(VARname))
            return

    def IManageLinkPedestal(self, position):
        """Manage Pedestal link books and stained glass"""
        PtDebugPrint("nb01EmgrPhase0.IManageLinkPedestal(): PedestalPosition = {}".format(position))
        if position in self.Pedestals:
            pedInfo = self.Pedestals[position]
            state = self._pedestalSDLValues[pedInfo["sdlState"]]
            if state:
                if state in pedInfo["Books"]:
                    # We have this book...
                    book = pedInfo["Books"][state]
                    if self._pedestalSDLValues[pedInfo["sdlGlass"]] not in book["Glasses"]:
                        # ...but the glass is wrong.
                        self.IPickPedestalGlass(position, book)
                else:
                    # This is not a valid book for this pedestal.  Get a new one.
                    self.IPickPedestalBook(position)
            else:
                # No book, clear the glass if it's set.
                self.IPickPedestalGlass(position, 0)
        else:
            PtDebugPrint("nb01EmgrPhase0.IManageLinkPedestal(): No pedestal named '{}'.  Ignoring.".format(position))

    def IPickPedestalBook(self, position):
        if position in self.Pedestals:
            # Select a random book from this pedestal's list.
            newBookChoice = random.choice(list(self.Pedestals[position]["Books"].keys()))
            self._UpdateVaultSDL(self.Pedestals[position]["sdlState"], newBookChoice)

            newBook = self.Pedestals[position]["Books"][newBookChoice]
            PtDebugPrint("nb01EmgrPhase0.IPickPedestalBook():\tSelecting {} as new book for pedestal.".format(newBook["Name"]))

            # Update the stained glass to match.
            self.IPickPedestalGlass(position, newBook)
        else:
            PtDebugPrint("nb01EmgrPhase0.IPickPedestalBook(): No pedestal named '{}'.  Ignoring.".format(position))

    def IPickPedestalGlass(self, position, bookInfo):
        if position in self.Pedestals:
            newGlass = 0
            if bookInfo:
                # Select a random glass from this book's list.
                newGlass = bookInfo["Glasses"][random.randint(0, len(bookInfo["Glasses"]) - 1)]
                PtDebugPrint("nb01EmgrPhase0.IPickPedestalGlass():\tSelecting stained glass #{} to match book for {}.".format(newGlass, bookInfo["Name"]))
            else:
                PtDebugPrint("nb01EmgrPhase0.IPickPedestalGlass():\tNo book.  Clearing stained glass.")
            self._UpdateVaultSDL(self.Pedestals[position]["sdlGlass"], newGlass)
        else:
            PtDebugPrint("nb01EmgrPhase0.IPickPedestalGlass(): No pedestal named '{}'.  Ignoring.".format(position))

    def _SaveSDL(self):
        if self._updateAgeSDL:
            self._ageVault.updateAgeSDL(self._ageSDL)
            del self._ageSDL
            self._updateAgeSDL = False

    def _UpdateSimpleStateVar(self, ageSDL, sdlName, value, overwrite=True):
        if isinstance(ageSDL, ptSDLStateDataRecord):
            var = ageSDL.findVar(sdlName)
            if var is None:
                PtDebugPrint("nb01EmgrPhase0._UpdateSimpleStateVar(): '{}' not found!".format(sdlName))
                return
        elif isinstance(ageSDL, ptSimpleStateVariable):
            var = ageSDL
        else:
            raise TypeError("ageSDL must be a ptSDLStateDataRecord or a ptSimpleStateVariable")

        if not overwrite and var.isUsed():
            PtDebugPrint("nb01EmgrPhase0._UpdateSimpleStateVar(): Skipping update of '{}'".format(sdlName), level=kDebugDumpLevel)
            return

        if var.getType() == PtSDLVarType.kBool:
            var.setBool(value)
        elif var.getType() in (PtSDLVarType.kInt, PtSDLVarType.kByte):
            var.setInt(value)
        elif var.getType() == PtSDLVarType.kString32:
            var.setString(value)
        else:
            PtDebugPrint("nb01EmgrPhase0._UpdateSimpleStateVar(): '{}' has an unknown type!".format(sdlName))
            return
        PtDebugPrint("nb01EmgrPhase0._UpdateSimpleStateVar(): '{}' = {}".format(sdlName, value))
        self._updateAgeSDL = True

    def _UpdateVaultSDL(self, sdlVar, value, overwrite=True):
        if not self._ageVault:
            self._ageVault = ptAgeVault()
            if not self._ageVault:
                PtDebugPrint("nb01EmgrPhase0._UpdateVaultSDL():\tNo AgeVault?!")
                return
            self._ageSDL = self._ageVault.getAgeSDL()
            if not self._ageSDL:
                PtDebugPrint("nb01EmgrPhase0._UpdateVaultSDL():\tVaultSDL is null?!")
                return

        if sdlVar and value:
            self._UpdateSimpleStateVar(self._ageSDL, sdlVar, value, overwrite)

    def _RandomizeNeighborhood(self):
        """Does initial state scrambling for the Neighborhood.
           This makes all hoods have a slightly different appearance (hopefully)"""
        PtDebugPrint("nb01EmgrPhase0._RandomizeNeighborhood(): ---Attempting to Randomize SDL---")
        for name, values in self.HoodDecorations.iteritems():
            self._UpdateVaultSDL(name, random.choice(values), overwrite=False)
        PtDebugPrint("nb01EmgrPhase0._RandomizeNeighborhood(): ---SDL Randomized!---")

    def IManageSimplePagingVar(self, VARname):
        """Load or unload a page based on the value of the specified SDL"""
        ageSDL = PtGetAgeSDL()
        try:
            if ageSDL[VARname][0] == 1: # Are we paging things in?
                PtDebugPrint("DEBUG: nb01EmgrPhase0.IManageSimplePagingVar:\tPaging in {}".format(VARname))
                PtPageInNode(VARname)
            elif ageSDL[VARname][0] == 0:  # Are we paging things out?
                PtDebugPrint("DEBUG: nb01EmgrPhase0.IManageSimplePagingVar:\tPaging out {}".format(VARname))
                PtPageOutNode(VARname)
            else:
                PtDebugPrint("ERROR: nb01EmgrPhase0.IManageSimplePagingVar:\tVariable '{}' had unexpected value of {} (should be a boolean).".format(VARname, ageSDL[VARname][0]))
        except:
            PtDebugPrint("ERROR: nb01EmgrPhase0.IManageSimplePagingVar:\tError managing variable '{}'".format(VARname))
            return
