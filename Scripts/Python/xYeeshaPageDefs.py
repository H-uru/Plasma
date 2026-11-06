# /*==LICENSE==*
#
# CyanWorlds.com Engine - MMOG client, server and tools
# Copyright (C) 2011  Cyan Worlds, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Additional permissions under GNU GPL version 3 section 7
#
# If you modify this Program, or any covered work, by linking or
# combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
# NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
# JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
# (or a modified version of those libraries),
# containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
# PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
# JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
# licensors of this Program grant you additional
# permission to convey the resulting work. Corresponding Source for a
# non-source form of such a combination shall include the source code for
# the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
# work.
#
# You can contact Cyan Worlds, Inc. by email legal@cyan.com
#  or by snail mail at:
#       Cyan Worlds, Inc.
#       14617 N Newport Hwy
#       Mead, WA   99021
#
# *==LICENSE==*/

# This dictionary controls the information needed to display Relto Pages.
# Add yeesha page textures to be loaded by name here.
# Just like link panels, textures should probably be in GUI_District_BkBookImages.prp or derivatives thereof.
# The number is the number of the corresponding Personal SDL variable for the page.
# The order of entries in this dictionary will be the order of pages in the Relto book.
YeeshaPages = {
    1: "xyeeshapagealphasketch01*1#0.hsm", # sun and moon
    2: "xyeeshapagealphasketch02*1#0.hsm", # waterfall
    3: "xyeeshapagealphasketch03*1#0.hsm", # hut decal / interior rug
    4: "xyeeshapagealphasketch04*1#0.hsm", # hut roof (swap)
    5: "xyeeshapagealphasketch07*1#0.hsm", # jumping pinnacles
    6: "xyeeshapagealphasketch06*1#0.hsm", # dock
    7: "xyeeshapagealphasketch05*1#0.hsm", # kickable physicals
    8: "xyeeshapagealphasketch12*1#0.hsm", # Rain
    9: "xyeeshapagealphasketch09*1#0.hsm", # music player
    10: "xyeeshapagealphasketch10*1#0.hsm", # the tree
    12: "xyeeshapagealphasketch08*1#0.hsm", # imager addition
    13: "xyeeshapagealphasketch13*1#0.hsm", # butterflies
    14: "xyeeshapagealphasketchfireplace*1#0.hsm", # fireplace
    15: "xyeeshapagealphasketch15*1#0.hsm", # bench
    16: "xyeeshapagealphasketchfiremarbles*1#0.hsm", # firemarbles
    17: "xyeeshapagealphasketchlushrelto*1#0.hsm", # lush
    18: "xyeeshapagealphasketchclock*1#0.hsm", # clock
    19: "xyeeshapagealphasketchbirds*1#0.hsm", # birds
    20: "xyeeshapagealphasketchcalendar*1#0.hsm", # calendar/bridge
    21: "xyeeshapagealphasketchleaf*1#0.hsm", # maple trees
    22: "xyeeshapagealphasketchgrass*1#0.hsm", # grass
    23: "xyeeshapagealphasketchercaplant*1#0.hsm", # ercana plants
    24: "xyeeshapagealphasketchstorm*1#0.hsm", # thunderstorm
    25: "xyeeshapagealphasketch14*1#0.hsm", # Bahro poles
    26: "xYeeshaPageVeelay7.dds", # VeeTsah Sky
    32: "xYeeshaPageGreenBushes.dds", # Green Bushes
}

###############################################################################

YeeshaPageStartID = 3 # used to map pages in the Relto book

# Compile our yeesha pages from above into the information needed for the Relto Book
# Array of Tuples: (SDL Variable, PageHTML)
YeeshaPageBookDefs = [
    (
        f'YeeshaPage{page}',
        f'<pb><img src="{sketch}" align=center check=00ff18,00800c,%d link={YeeshaPageStartID + idx}>'
    )
    for idx, (page, sketch) in enumerate(YeeshaPages.items())
]
