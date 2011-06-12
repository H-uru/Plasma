#!/opt/local/bin/python2.7

# Christian Walther 2011-04-21
# Public Domain

# blackwhite2rgba.py <imageonblack> <imageonwhite> <factor> <output>
#
# - Read two opaque image files that have the same picture composited onto a
#   black background and a white background.
# - Scale them down by <factor> (integer), taking into account that adding
#   (averaging) colors must be done in a linear color space, not with the
#   power-law-encoded values stored in the files. (I know of no image
#   processing application that does this right.) Gamma is hardcoded to 2.0,
#   a subjective compromise.
# - Compute color and alpha that will result in the input images when
#   composited onto black or white, as far as possible, and save them as a PNG
#   file <output>. For this step, assume that alpha compositing will be done
#   directly with the raw power-law-encoded values rather than in the linear
#   color space that would be correct, which is the way almost all software
#   will do it, in particular OpenGL/Direct3D. (Photoshop has an option to do
#   it right, maybe other high-end image processing software too.)


from __future__ import division
import sys
import math
import Image


gamma = 2.0


def add(a, b):
    for i, y in enumerate(b):
        a[i] += y

def sub(a, b):
    for i, y in enumerate(b):
        a[i] -= y

def mul(a, b):
    for i, y in enumerate(b):
        a[i] *= y

def mulsc(a, b):
    for i in range(len(a)):
        a[i] *= b

def pixel2linear(p):
    return [math.pow(p[i]/255.0, gamma) for i in range(3)]

def clamp(x):
    return 255 if x > 255 else 0 if x < 0 else x

def linear2pixel(l):
    p = [clamp(int(math.floor(math.pow(l[i], 1.0/gamma)*255 + 0.5))) for i in range(3)]
    if len(l) == 4:
        p.append(clamp(int(math.floor(l[3]*255 + 0.5))))
    return p

def nonlinear2pixel(l):
    return [clamp(int(math.floor(c*255 + 0.5))) for c in l]


bcimg = Image.open(sys.argv[1])
wcimg = Image.open(sys.argv[2])
factor = int(sys.argv[3])
outfilename = sys.argv[4]

assert bcimg.size == wcimg.size

bcpix = bcimg.load()
wcpix = wcimg.load()

outw = bcimg.size[0] // factor
outh = bcimg.size[1] // factor
outimg = Image.new("RGBA", (outw, outh), None)
outpix = outimg.load()

for oy in range(outh):
    for ox in range(outw):
        # scale down in linear color space
        bc = [0.0, 0.0, 0.0]
        wc = [0.0, 0.0, 0.0]
        for j in range(factor):
            for i in range(factor):
                add(bc, pixel2linear(bcpix[ox*factor+i, oy*factor+j]))
                add(wc, pixel2linear(wcpix[ox*factor+i, oy*factor+j]))
        mulsc(bc, 1.0/(factor*factor))
        mulsc(wc, 1.0/(factor*factor))
        
        # go back to gamma-encoded color space, assuming that alpha blending will be done in that
        bcn = [math.pow(l, 1.0/gamma) for l in bc]
        wcn = [math.pow(l, 1.0/gamma) for l in wc]
        
        # compute color and alpha
        sub(wcn, bcn)
        a = 1.0 - (wcn[0] + wcn[1] + wcn[2])/3
        if math.floor(a*255 + 0.5) > 0:
            mulsc(bcn, 1.0/a)
        else:
            bcn = [0.0, 0.0, 0.0]
        outpix[ox, oy] = tuple(nonlinear2pixel(bcn + [a]))

# collect pixels that ended up with alpha 0
transparent = [0]*outh*outw
for oy in range(outh):
    for ox in range(outw):
        if outpix[ox, oy][3] == 0:
            transparent[oy*outw + ox] = 1

# expand neighboring color values from nonzero-alpha pixels into the zero-alpha region twice, so that bilinear interpolation cannot bleed the arbitrary background color (black here) from zero-alpha into nonzero-alpha territory
for i in range(2):
    transp = transparent[:]
    for oy in range(outh):
        for ox in range(outw):
            if transp[oy*outw + ox]:
                count = 0
                sum = [0, 0, 0]
                if ox > 0:
                    if not transp[oy*outw + ox-1]:
                        add(sum, outpix[ox-1, oy][0:3])
                        count += 1
                if ox < outw-1:
                    if not transp[oy*outw + ox+1]:
                        add(sum, outpix[ox+1, oy][0:3])
                        count += 1
                if oy > 0:
                    if not transp[(oy-1)*outw + ox]:
                        add(sum, outpix[ox, oy-1][0:3])
                        count += 1
                if oy < outh-1:
                    if not transp[(oy+1)*outw + ox]:
                        add(sum, outpix[ox, oy+1][0:3])
                        count += 1
                if count > 0:
                    mulsc(sum, 1.0/count)
                    outpix[ox, oy] = tuple(clamp(int(math.floor(c + 0.5))) for c in sum) + (0,)
                    transparent[oy*outw + ox] = 0

outimg.save(outfilename, "PNG")
