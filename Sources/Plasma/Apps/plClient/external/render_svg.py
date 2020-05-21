#!/usr/bin/env python
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

from __future__ import print_function
from __future__ import with_statement


import os
import math
from xml.dom.minidom import parse
from optparse import OptionParser
import scalergba

try:
	import cairosvg
except ImportError as e:
	print("Rendering SVG resources requires CairoSVG.  Exiting...")
	exit(1)

cursorList = {
	"cursor_up": ["circleOuter"],
	"cursor_poised": ["circleOuter", "circleInnerOpen"],
	"cursor_clicked": ["circleOuter", "circleInnerClosed"],
	"cursor_disabled": ["cross"],

	"cursor_open": ["circleOuter", "arrowTranslucentUpper", "arrowTranslucentLower"],
	"cursor_grab": ["circleOuter", "circleInnerOpen", "arrowTranslucentUpper", "arrowTranslucentLower"],
	"cursor_updown_open": ["circleOuter", "circleInnerOpen", "arrowTranslucentUpper", "arrowTranslucentLower"],
	"cursor_updown_closed": ["circleOuter", "circleInnerClosed", "arrowOpaqueUpper", "arrowOpaqueLower"],

	"cursor_leftright_open": ["circleOuter", "circleInnerOpen", "arrowTranslucentLeft", "arrowTranslucentRight"],
	"cursor_leftright_closed": ["circleOuter", "circleInnerClosed", "arrowOpaqueLeft", "arrowOpaqueRight"],

	"cursor_4way_open": ["circleOuter", "circleInnerOpen", "arrowTranslucentUpper", "arrowTranslucentRight", "arrowTranslucentLower", "arrowTranslucentLeft"],
	"cursor_4way_closed": ["circleOuter", "circleInnerClosed", "arrowOpaqueUpper", "arrowOpaqueRight", "arrowOpaqueLower", "arrowOpaqueLeft"],

	"cursor_upward": ["circleOuter", "arrowOpaqueUpper"],
	"cursor_right": ["circleOuter", "arrowOpaqueRight"],
	"cursor_down": ["circleOuter", "arrowOpaqueLower"],
	"cursor_left": ["circleOuter", "arrowOpaqueLeft"],

	"cursor_book": ["circleOuter", "book"],
	"cursor_book_poised": ["circleOuter", "circleInnerOpen", "book"],
	"cursor_book_clicked": ["circleOuter", "circleInnerClosed", "book"],
}
cursorOffsetList = {
	"cursor_book": [7, 7],
	"cursor_book_poised": [7, 7],
	"cursor_book_clicked": [7, 7]
}

textList = {
	"xLoading_Linking_Text": ["background", "circles", "textLinking"],
	"xLoading_Updating_Text": ["background", "circles", "textUpdating"]
}

voiceList = {
	"ui_speaker": ["speakerGrille", "speakerIndicator", "speakerOuterRing"],
	"ui_microphone": ["microphoneGrille", "microphoneIndicator", "microphoneOuterRing"]
}

def enable_only_layers(layerlist, layers):
	for layer in layers:
		if layer in layerlist:
			layers[layer].setAttribute("style","")
		else:
			layers[layer].setAttribute("style","display:none")
	# sanity check
	for layer in layerlist:
		if layer not in layers:
			print("warning: unknown layer", layer)

def shift_all_layers(layers, shiftx, shifty):
	# note: this assumes that all layers start out with no transform of their own
	for layer in layers:
		layers[layer].setAttribute("transform", "translate(%g,%g)" % (shiftx, shifty))

def rotate_layer(layer, angle, offX, offY):
	# note: this assumes that this layer starts out with no transform of their own
	layer.setAttribute("transform", "rotate(%g %g %g)" % (angle, offX, offY))

def get_layers_from_svg(svgData):
	inkscapeNS = "http://www.inkscape.org/namespaces/inkscape"
	layers = {}

	groups = svgData.getElementsByTagName("g")
	for group in groups:
		if group.getAttributeNS(inkscapeNS,"groupmode") == "layer":
			layers[group.getAttribute("id")] = group

	return layers

def render_cursors(inpath, outpath):
	scalefactor = 4
	with open(os.path.join(inpath,"Cursor_Base.svg"), "r") as svgFile:
		cursorSVG = parse(svgFile)
		layers = get_layers_from_svg(cursorSVG)
		svgwidth = float(cursorSVG.documentElement.getAttribute("width"))
		svgheight = float(cursorSVG.documentElement.getAttribute("height"))

		for cursor in cursorList:
			enabledlayers = cursorList[cursor]
			enabledlayers = enabledlayers + [l + "Shadow" for l in enabledlayers]
			enable_only_layers(enabledlayers, layers)

			shift_all_layers(layers, *cursorOffsetList.get(cursor, [0, 0]))

			outfile = os.path.join(outpath, cursor + ".png")
			cairosvg.svg2png(bytestring=cursorSVG.toxml().encode('utf-8'), write_to=outfile, 
				parent_width=svgwidth, parent_height=svgheight, scale=scalefactor)
			scalergba.scale(outfile, outfile, scalefactor)

def render_loading_books(inpath, outpath):
	resSize = {"width":256, "height":256}
	with open(os.path.join(inpath,"Linking_Book.svg"), "r") as svgFile:
		bookSVG = parse(svgFile)
		layers = get_layers_from_svg(bookSVG)
		cX = float(bookSVG.documentElement.getAttribute("width")) / 2
		cY = float(bookSVG.documentElement.getAttribute("height")) / 2

		for angle in range(0, 18):
			# Draw Book, Black Background, and rotating Circles
			enable_only_layers(["background", "book", "circles"],layers)

			# Rotate Circles at appropriate angle
			rotate_layer(layers["circles"], angle*5, cX, cY)

			cairosvg.svg2png(bytestring=bookSVG.toxml().encode('utf-8'),
				write_to=os.path.join(outpath, "xLoading_Linking.{0:02}.png".format(angle)), 
				parent_width=resSize["width"], parent_height=resSize["height"])

def render_loading_text(inpath, outpath):
	resSize = {"width":192, "height":41}
	with open(os.path.join(inpath,"Loading_Text_rasterfont.svg"), "r") as svgFile:
		textSVG = parse(svgFile)
		layers = get_layers_from_svg(textSVG)

		for textEntry in textList:
			enable_only_layers(textList[textEntry], layers)
			cairosvg.svg2png(bytestring=textSVG.toxml().encode('utf-8'),
				write_to=os.path.join(outpath, textEntry + ".png"), 
				parent_width=resSize["width"], parent_height=resSize["height"])

def render_voice_icons(inpath, outpath):
	resSize = {"width":32, "height":32}
	with open(os.path.join(inpath,"Voice_Chat.svg"), "r") as svgFile:
		uiSVG = parse(svgFile)
		layers = get_layers_from_svg(uiSVG)

		for voiceUI in voiceList:
			enable_only_layers(voiceList[voiceUI], layers)

			cairosvg.svg2png(bytestring=uiSVG.toxml().encode('utf-8'),
				write_to=os.path.join(outpath, voiceUI + ".png"), 
				parent_width=resSize["width"], parent_height=resSize["height"])

if __name__ == '__main__':
	parser = OptionParser(usage="usage: %prog [options]")
	parser.add_option("-q", "--quiet", dest="verbose", default=True, action="store_false", help="Don't print status messages")
	parser.add_option("-o", "--outpath", dest="outpath", default="./out", help="Sets output path for rendered images")
	parser.add_option("-i", "--inpath", dest="inpath", default=".", help="Sets input path for SVG files")

	(options, args) = parser.parse_args()

	## Send output to OS's null if unwanted
	if not options.verbose:
		sys.stdout = open(os.devnull,"w")
		sys.stderr = open(os.devnull,"w")

	## Compute Paths
	outpath = os.path.expanduser(options.outpath)
	inpath = os.path.expanduser(options.inpath)

	if not os.path.exists(outpath):
		os.makedirs(outpath)

	## Do the work!
	print("Rendering SVGs...")
	render_cursors(inpath, outpath)
	render_loading_books(inpath, outpath)
	render_loading_text(inpath, outpath)
	render_voice_icons(inpath, outpath)
