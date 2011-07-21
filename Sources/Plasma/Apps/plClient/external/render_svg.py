#!/usr/bin/env python

from __future__ import print_function
from __future__ import with_statement


import os
import math
from xml.dom.minidom import parse
from optparse import OptionParser

try:
	import rsvg
	import cairo
except ImportError as e:
	print("Rendering SVG resources requires PyGTK.  Exiting...")
	exit(1)

cursorList = {
	"cursor_up": ["circleOuter"],
	"cursor_poised": ["circleOuter", "circleInnerOpen"],
	"cursor_clicked": ["circleOuter", "circleInnerClosed"],
	"cursor_disabled": ["circleOuter", "cross"],

	"cursor_open": ["circleOuter", "arrowGreyUpper", "arrowGreyLower"],
	"cursor_grab": ["circleOuter", "circleInnerClosed", "arrowGreyUpper", "arrowGreyLower"],
	"cursor_updown_open": ["circleOuter", "circleInnerClosed", "arrowGreyUpper", "arrowGreyLower"],
	"cursor_updown_closed": ["circleOuter", "circleInnerClosed", "arrowWhiteUpper", "arrowWhiteLower"],

	"cursor_leftright_open": ["circleOuter", "circleInnerClosed", "arrowGreyRight", "arrowGreyLeft"],
	"cursor_leftright_closed": ["circleOuter", "circleInnerClosed", "arrowWhiteRight", "arrowWhiteLeft"],

	"cursor_4way_open": ["circleOuter", "circleInnerClosed", "arrowGreyUpper", "arrowGreyRight", "arrowGreyLower", "arrowGreyLeft"],
	"cursor_4way_closed": ["circleOuter", "circleInnerClosed", "arrowWhiteUpper", "arrowWhiteRight", "arrowWhiteLower", "arrowWhiteLeft"],

	"cursor_upward": ["circleOuter", "arrowWhiteUpper"],
	"cursor_right": ["circleOuter", "arrowWhiteRight"],
	"cursor_down": ["circleOuter", "arrowWhiteLower"],
	"cursor_left": ["circleOuter", "arrowWhiteLeft"],

	"cursor_book": ["circleOuter", "book"],
	"cursor_book_poised": ["circleOuter", "circleInnerOpen", "book"],
	"cursor_book_clicked": ["circleOuter", "circleInnerClosed", "book"],
}
cursorOffsetList = {
	"book": [8, 8]
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

def get_layers_from_svg(svgData):
	inkscapeNS = "http://www.inkscape.org/namespaces/inkscape"
	layers = {}

	groups = svgData.getElementsByTagName("g")
	for group in groups:
		if group.getAttributeNS(inkscapeNS,"groupmode") == "layer":
			layers[group.getAttribute("id")] = group

	return layers

def render_cursors(inpath, outpath):
	resSize = {"width":32, "height":32}
	with open(os.path.join(inpath,"Cursor_Base.svg"), "r") as svgFile:
		cursorSVG = parse(svgFile)
		layers = get_layers_from_svg(cursorSVG)
		ratioW = resSize["width"] / float(cursorSVG.documentElement.getAttribute("width"))
		ratioH = resSize["height"] / float(cursorSVG.documentElement.getAttribute("height"))
		surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, resSize["width"], resSize["height"])

		for cursor in cursorList:
			ctx = cairo.Context(surface)
			ctx.save()
			ctx.set_operator(cairo.OPERATOR_CLEAR)
			ctx.paint()
			ctx.restore()

			enable_only_layers(cursorList[cursor], layers)

			for layerName in cursorOffsetList:
				if layerName in cursorList[cursor]:
					ctx.translate(*cursorOffsetList[layerName])
			svg = rsvg.Handle(data=cursorSVG.toxml())
			ctx.scale(ratioW, ratioH)
			svg.render_cairo(ctx)

			surface.write_to_png(os.path.join(outpath, cursor + ".png"))

def render_loading_books(inpath, outpath):
	resSize = {"width":256, "height":256}
	with open(os.path.join(inpath,"Linking_Book.svg"), "r") as svgFile:
		bookSVG = parse(svgFile)
		layers = get_layers_from_svg(bookSVG)
		ratioW = resSize["width"] / float(bookSVG.documentElement.getAttribute("width"))
		ratioH = resSize["height"] / float(bookSVG.documentElement.getAttribute("height"))
		surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, resSize["width"], resSize["height"])

		for angle in range(0, 18):
			ctx = cairo.Context(surface)

			# Draw Book and Black Background
			enable_only_layers(["background", "book"],layers)
			svg = rsvg.Handle(data=bookSVG.toxml())
			ctx.save()
			ctx.scale(ratioW, ratioH)
			svg.render_cairo(ctx)
			ctx.restore()

			# Draw Circles at appropriate angle
			enable_only_layers(["circles"],layers)
			svg = rsvg.Handle(data=bookSVG.toxml())
			ctx.translate(resSize["height"] / 2, resSize["width"] / 2)
			ctx.rotate(math.radians(angle*(5)))
			ctx.translate(-resSize["width"] / 2, -resSize["height"] / 2)
			ctx.scale(ratioW, ratioH)
			svg.render_cairo(ctx)

			surface.write_to_png(os.path.join(outpath, "xLoading_Linking.{0:02}.png".format(angle)))

def render_loading_text(inpath, outpath):
	resSize = {"width":192, "height":41}
	with open(os.path.join(inpath,"Loading_Text_rasterfont.svg"), "r") as svgFile:
		textSVG = parse(svgFile)
		layers = get_layers_from_svg(textSVG)
		ratioW = resSize["width"] / float(textSVG.documentElement.getAttribute("width"))
		ratioH = resSize["height"] / float(textSVG.documentElement.getAttribute("height"))
		surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, resSize["width"], resSize["height"])

		for textEntry in textList:
			ctx = cairo.Context(surface)
			ctx.save()
			ctx.set_operator(cairo.OPERATOR_CLEAR)
			ctx.paint()
			ctx.restore()
			enable_only_layers(textList[textEntry], layers)
			svg = rsvg.Handle(data=textSVG.toxml())
			ctx.scale(ratioW, ratioH)
			svg.render_cairo(ctx)
			surface.write_to_png(os.path.join(outpath, textEntry + ".png"))

def render_voice_icons(inpath, outpath):
	resSize = {"width":32, "height":32}
	with open(os.path.join(inpath,"Voice_Chat.svg"), "r") as svgFile:
		uiSVG = parse(svgFile)
		layers = get_layers_from_svg(uiSVG)
		ratioW = resSize["width"] / float(uiSVG.documentElement.getAttribute("width"))
		ratioH = resSize["height"] / float(uiSVG.documentElement.getAttribute("height"))
		surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, resSize["width"], resSize["height"])

		for voiceUI in voiceList:
			ctx = cairo.Context(surface)
			ctx.save()
			ctx.set_operator(cairo.OPERATOR_CLEAR)
			ctx.paint()
			ctx.restore()

			enable_only_layers(voiceList[voiceUI], layers)

			svg = rsvg.Handle(data=uiSVG.toxml())
			ctx.scale(ratioW, ratioH)
			svg.render_cairo(ctx)

			surface.write_to_png(os.path.join(outpath, voiceUI + ".png"))

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
		os.mkdir(outpath)

	## Do the work!
	print("Rendering SVGs...")
	render_cursors(inpath, outpath)
	render_loading_books(inpath, outpath)
	render_loading_text(inpath, outpath)
	render_voice_icons(inpath, outpath)
