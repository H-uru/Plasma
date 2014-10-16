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
	parser.add_option("-w", "--workpath", dest="workpath", default=".", help="Sets working output path for image renders")
	parser.add_option("-o", "--outpath", dest="outpath", default=".", help="Sets output path for resource container")
	parser.add_option("-i", "--inpath", dest="inpath", default=".", help="Sets input path for files to add to resource file")

	(options, args) = parser.parse_args()

	## Send output to OS's null if unwanted
	if not options.verbose:
		sys.stdout = open(os.devnull,"w")
		sys.stderr = open(os.devnull,"w")

	## Compute Paths
	workpath = os.path.expanduser(options.workpath)
	outpath = os.path.expanduser(options.outpath)
	inpath = os.path.expanduser(options.inpath)

	## Do the work!
	if options.render:
		ret = subprocess.call(["python", os.path.join(inpath, "render_svg.py"), "-i", inpath, "-o", os.path.join(workpath, "render")], stdout=sys.stdout, stderr=sys.stderr)
		if ret != 0:
			print("An error has occurred.  Aborting.")
			exit(1)

	if options.optimize:
		print("Optimizing PNGs with pngcrush...")
		for png in glob.glob(os.path.join(workpath, "render", "*.png")):
			#print("pngcrushing - {0}".format(png))
			ret = subprocess.call(["pngcrush", "-q", "-l", "9", "-brute", png, "temp.png"], stdout=sys.stdout, stderr=sys.stderr)
			if ret != 0:
				print("An error has occurred.  Aborting.")
				exit(1)
			os.remove(png)
			os.rename("temp.png", png)

	if options.package:
		ret = subprocess.call(["python", os.path.join(inpath, "create_resource_dat.py"), "-i", os.path.join(workpath, "render"), "-o", os.path.join(outpath, "resource.dat")], stdout=sys.stdout, stderr=sys.stderr)
		if ret != 0:
			print("An error has occurred.  Aborting.")
			exit(1)
