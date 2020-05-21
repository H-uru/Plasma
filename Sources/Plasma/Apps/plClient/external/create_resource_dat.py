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
import sys
import glob
import struct
from optparse import OptionParser

version = 1

def create_resource_dat(resfilepath, inrespath):
	datHeader = 0xCBBCF00D
	datVersion = 0x00000001

	## Get list of files to archive
	resourceList = glob.glob(os.path.join(inrespath, "*"))
	resourceList.sort()
	try:
		resourceList.remove(os.path.join(inrespath, "Thumbs.db")) # likely to be there on Windows and likely to be unwanted
	except ValueError:
		pass
	if len(resourceList) == 0:
		print("No files found in '{0}'.  Quitting.\n".format(inrespath))
		return False
	print("{0} resources found in '{1}'.".format(len(resourceList), inrespath, ))

	## Write each resource into the output file
	with open(resfilepath, "wb") as datFile:
		datFile.write(struct.pack("<I",datHeader))
		datFile.write(struct.pack("<I",datVersion))
		datFile.write(struct.pack("<I",len(resourceList)))
		for res in resourceList:
			with open(res, "rb") as resFile:
				name = os.path.basename(res).encode("utf-8")
				datFile.write(struct.pack("<I", len(name)))
				datFile.write(name)
				datFile.write(struct.pack("<I", os.path.getsize(res)))
				datFile.write(resFile.read())

		print("{0} resources written to '{1}'.\n".format(len(resourceList), resfilepath))

	return True

if __name__ == '__main__':
	parser = OptionParser(usage="usage: %prog [options]", version="%prog {0}".format(version))
	parser.add_option("-q", "--quiet", dest="verbose", default=True, action="store_false", help="Don't print status messages")
	parser.add_option("-o", "--outfile", dest="outfile", default="resource.dat", help="Sets name for output file")
	parser.add_option("-i", "--inpath", dest="inpath", default=".", help="Sets input path for files to add to resource file")

	(options, args) = parser.parse_args()

	## Send output to OS's null if unwanted
	if not options.verbose:
		sys.stdout = open(os.devnull,"w")
		sys.stderr = open(os.devnull,"w")

	## Compute Paths
	outfile = os.path.expanduser(options.outfile)
	inpath = os.path.expanduser(options.inpath)

	## Do the work!
	print("Creating {0}...".format(outfile))
	create_resource_dat(outfile, inpath)
