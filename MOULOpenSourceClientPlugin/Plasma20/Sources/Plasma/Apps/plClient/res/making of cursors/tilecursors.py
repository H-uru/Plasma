#!/opt/local/bin/python2.7

import sys
import struct
import shutil
import Image


cursornames = [
	"up",		"open",			None,				None,			"book",
	"poised",	"updownopen",	"leftrightopen",	"4wayopen",		"book_highlight",
	"clicked",	"updownclosed",	"leftrightclosed",	"4wayclosed",	"book_clicked",
	"disabled",	"upward",		"down",				"left",			"right",
	None,		None,			None,				"hand",			"arrow"
]


img = Image.open(sys.argv[1])
outprefix = sys.argv[2]
assert img.size == (240, 240)
for yi in range(5):
	for xi in range(5):
		name = cursornames[yi*5+xi]
		if name != None:
			tile = img.crop((xi*48, yi*48, (xi+1)*48, (yi+1)*48))
			tilepix = tile.load()
			with open(outprefix + "cursor_" + name + ".raw", "w") as f:
				w, h = tile.size
				f.write(struct.pack("<II", w, h))
				for y in range(h):
					for x in range(w):
						r, g, b, a = tilepix[x, y]
						f.write(struct.pack("BBBB", b, g, r, a))

shutil.copy(outprefix + "cursor_updownopen.raw", outprefix + "cursor_grab.raw")
