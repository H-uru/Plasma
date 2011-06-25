#!/usr/bin/env python

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
				name = os.path.basename(res)
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
