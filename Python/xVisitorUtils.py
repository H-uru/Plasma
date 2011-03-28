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
Module: xVisitorUtils.py
Date: May 2006
Written by: Tye Hooley
Provides support for Free players
"""

from Plasma import *

kValidVisitorAges = ("GarrisonNoShare", "Personal", "Nexus", "Neighborhood", "Cleft")
kVisitorNagDialog  = "GUIDialog13"

def IsVisitorAllowedAge(ageName):
    "Returns if the age is available to visitors (i.e. free players)"
    
    #print( "IsVisitorAllowedAge-->Checking Age: %s" % ageName)
    if PtIsSubscriptionActive():
        return 1
    else:
        if ageName in kValidVisitorAges:
            return 1
        else:
            return 0
    
