#!/usr/bin/env python

import os
import sys
import glob
import subprocess
from optparse import OptionParser


if __name__ == '__main__':
	parser = OptionParser(usage="usage: %prog [options]")
	parser.add_option("-q", "--quiet", dest="verbose", default=True, action="store_false", help="Don't print status messages")
	parser.add_option("-r", "--render", dest="render", default=False, action="store_true", help="Perform SVG Render to images")
	parser.add_option("-p", "--package", dest="package", default=False, action="store_true", help="Perform packaging into resource container")
	parser.add_option("-z", "--optimize", dest="optimize", default=False, action="store_true", help="Perform PNGCrush optimization on PNG resources")
	parser.add_option("-o", "--outpath", dest="outpath", default=".", help="Sets output path for resource container")
	parser.add_option("-i", "--inpath", dest="inpath", default=".", help="Sets input path for files to add to resource file")

	(options, args) = parser.parse_args()

	## Send output to OS's null if unwanted
	if not options.verbose:
		sys.stdout = open(os.devnull,"w")
		sys.stderr = open(os.devnull,"w")

	## Compute Paths
	outpath = os.path.expanduser(options.outpath)
	inpath = os.path.expanduser(options.inpath)

	## Do the work!
	if options.render:
		ret = subprocess.call(["python", os.path.join(inpath, "render_svg.py"), "-i", inpath, "-o", os.path.join(outpath, "render")], stdout=sys.stdout, stderr=sys.stderr)
		if ret != 0:
			print("An error has occurred.  Aborting.")
			exit(1)

	if options.optimize:
		print("Optimizing PNGs with pngcrush...")
		for png in glob.glob(os.path.join("render", "*.png")):
			#print("pngcrushing - {0}".format(png))
			ret = subprocess.call(["pngcrush", "-q", "-l 9", "-brute", png, "temp.png"], stdout=sys.stdout, stderr=sys.stderr)
			if ret != 0:
				print("An error has occurred.  Aborting.")
				exit(1)
			os.remove(png)
			os.rename("temp.png", png)

	if options.package:
		ret = subprocess.call(["python", os.path.join(inpath, "create_resource_dat.py"), "-i", os.path.join(outpath, "render"), "-o", "resource.dat"], stdout=sys.stdout, stderr=sys.stderr)
		if ret != 0:
			print("An error has occurred.  Aborting.")
			exit(1)
