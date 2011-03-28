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
Module: xConcentration Game
Age: global
Date: June 2002
Author: Pete Gage
"""

from Plasma import *
from PlasmaTypes import *
from whrandom import *

import PlasmaControlKeys


# define the attributes that will be entered in max
matrix =  ptAttribInt(1, "matrix size")
puzname =  ptAttribString(2, "puzzle name")
xfeet =  ptAttribInt(3, "grid x-size (in feet)")
yfeet =  ptAttribInt(4, "grid y-size (in feet)")
dtext = ptAttribDynamicMap(5,"Dynamic Text map")
Click = ptAttribActivator(6,"clickable")

# globals

puz = []
whichpick = 0
pick1 = None
blue = ptColor()
blue.blue()
black = ptColor()
black.black()
yellow = ptColor()
yellow.yellow()
white = ptColor()
white.white()
hack=0
image= None

class xConcentration(ptModifier):
        
    def __init__(self):
        ptModifier.__init__(self)
        self.id = 5120
        
        version = 1
        self.version = version
        print "__init__xConcentration v.", version
        
    def OnServerInitComplete(self):
        global puz
        self.SDL.setDefault("puz",(0,))
        puztuple = self.SDL["puz"]
        #puztuple=("xx",)
        #print puztuple[0]
        #puz = list(puztuple)
        print puztuple[0], "Existing Value!"
        if puztuple[0] != puzname.value:
            puz = list(puztuple)
            puz=BuildPuzList()
            puz[0] = puzname.value
            print puz[0],"firstupdate"
            self.SDL["puz"] = tuple(puz) # writes it

        dtext.textmap.clearToColor(ptColor(0,0,0,0))

        
    def Load(self):
        print "load XXXXXXXX"
        global puz
        puz = self.SDL["puz"][1032]
        #if not puz[0]: #thing is empty, else we already have a list.
        puz=BuildPuzList()

    def OnNotify(self,state,id,events):
        dtext.textmap.clearToColor(ptColor(0,0,0,0.5))

        global puz
        global whichpick
        global pick1
        global hack
        global image
        if hack==0:
            image = PtScreenShot()
            hack=1
           
        xtotalsize = float(xfeet.value)
        ytotalsize = float(yfeet.value)
        xsquaresize = float(xtotalsize / float(matrix.value))
        ysquaresize = float(ytotalsize / float(matrix.value))
        #print xsquaresize,"squaresize",ysquaresize
        if state:
            if id == Click.id and PtWasLocallyNotified(self.key):
                for event in events:
                    if event[0] == kPickedEvent:
                        TruePickx = event[5].getX() + float(xtotalsize/2) 
                        TruePicky = event[5].getY() + float(ytotalsize/2) 
                            # gets a positive value. Truepick will be a number from 0 to totalsize
                        xSquarepicked = TruePickx / xsquaresize
                        ySquarepicked = TruePicky / ysquaresize
                            # find the square from the Truepick (not rounded yet)
                        
                        xSquare = round(xSquarepicked+.5)
                        ySquare = round(ySquarepicked+.5)
                            
                        print TruePickx,"Truepickx=xSquarepicked",xSquarepicked,"Rounded:",xSquare
                        print TruePicky,"Truepicky=ySquarepicked",ySquarepicked,"Rounded:",ySquare
                                            
                        pick = (int(ySquare-1) * matrix.value) + int(xSquare) #pick is the array location                     
                        print "Pick=",whichpick," Square:",pick,"value=",puz[pick] 

                        #Now do something with the pick
                        texX = 512
                        texY = 512 #hardcoded for now
                        texXBox = texX/matrix.value
                        texYBox = texY/matrix.value
                        drawAtX = texXBox * (xSquare-1)
                        drawAtY = texXBox * (ySquare-1)
                        dtext.textmap.setFont("arial", 6)
                        dtext.textmap.setTextColor(white)
                        
                        imageSquare = puz[pick] # this is the value of the spot you picked, and tell what square to draw here
                        imageSquareY = round((float(imageSquare / matrix.value))-.49) # rows
                        imageSquareX = imageSquare % matrix.value #columns
                        
                        getFromX = texXBox * (imageSquareX)
                        getFromY = texYBox * (imageSquareY)
                        
                        print imageSquareX,"GETFROM",imageSquareY
                        
                        if puz[pick] == pick: # solved spot already
                            #play moot graphic.
                            print "skippingpick"
                            pass
                        elif whichpick == 0 : #first selection
                            pick1 = pick
                            whichpick = 1

                            # do graphics here on spot!!!!
                            #dtext.textmap.drawImageClipped(drawAtX, drawAtY, image, getFromX, getFromY, texXBox, texYBox,0)
                            dtext.textmap.fillRect( drawAtX, drawAtY, drawAtX+texXBox, drawAtY+texYBox, ptColor(1,0,0,1) )
                            dtext.textmap.drawText( drawAtX, drawAtY, str(puz[pick]))


                        elif whichpick == 1 : # second pick
                            if puz[pick1] == pick: #You actually picked the right spot!
                                puz[pick1] = puz[pick] 
                                puz[pick] = pick # swap numbers
                                self.SDL["puz"] = tuple(puz) # writes it
                                print "MATCH:"
                                print "Square:",pick1,"value=",puz[pick1]
                                print "Square:",pick,"value=",puz[pick]
                                dtext.textmap.clearToColor(black)
                            else:
                                pass
                            #dtext.textmap.fillRect(left, top, right, bottom, color)

                            whichpick = 0
                            #dtext.textmap.drawImageClipped(drawAtX, drawAtY, image, getFromX, getFromY, texXBox, texYBox,0)
                            dtext.textmap.fillRect( drawAtX, drawAtY, drawAtX+texXBox, drawAtY+texYBox, yellow )
                            dtext.textmap.drawText( drawAtX, drawAtY, str(puz[pick]))
                        dtext.textmap.flush()

def BuildPuzList():
    g = whrandom()
    p = []
    #size=matrix.value(self)
    for x in range(1033): # fills the list with a number coinciding with its position
        p.append(x)

    print "start list"    
    for x in range(1,1032): # fill numbers
        p[x] = x
    for x in range(1,1032): # scrambles those numbers
        y=g.randint(1, 1032)
        z=p[x]
        p[x] = p[y]
        p[y] = z
        print x,",",p[x]

    for x in range(1,1032): # makes sure there were no "put backs"
        if p[x] == x:
            y=g.randint(1, 1032)
            z=p[x]
            p[x] = y
            p[y] = z
            print x,",,,",p[x]
    print "end list"
    return p
        
