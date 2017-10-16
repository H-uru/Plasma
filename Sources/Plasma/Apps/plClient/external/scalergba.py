#!/opt/local/bin/python2.7

# Christian Walther 2011-07-20
# Public Domain

# scalergba.py <input> <factor> <output>
#
# Scale image <input> down by <factor> (integer) and save as PNG file <output>.
#
# - Taking into account that adding (averaging) colors must be done in a linear
#   color space, not with the power-law-encoded values stored in the files.
#   (I know of no image processing application that does this right.)
#   Gamma is hardcoded to 2.2.
# - Assuming that alpha compositing will be done directly with the raw
#   power-law-encoded values rather than in the linear color space that would
#   be correct, which is the way almost all software will do it, in particular
#   OpenGL/Direct3D. (Photoshop has an option to do it right, maybe other
#   high-end image processing software too.)


from __future__ import division
import sys
import math
try:
	from PIL import Image
except ImportError:
	print("Scaling requires the Python Imaging Library.")
	raise


gamma = 2.2


def add(a, b):
	for i, y in enumerate(b):
		a[i] += y

def sub(a, b):
	for i, y in enumerate(b):
		a[i] -= y

def subsc(a, b):
	for i in range(3):
		a[i] -= b

def mul(a, b):
	for i, y in enumerate(b):
		a[i] *= y

def mulsc(a, b):
	for i in range(3):
		a[i] *= b

def pixel2linear(p):
	l = [math.pow(p[i]/255.0, gamma) for i in range(3)]
	if len(p) == 4:
		l.append(p[3]/255.0)
	else:
		l.append(1.0)
	return l

def pixel2nonlinear(p):
	return [p[i]/255.0 for i in range(3)], p[3]/255.0 if len(p) > 3 else 1.0

def clamp(x):
	return 255 if x > 255 else 0 if x < 0 else x

def linear2pixel(l):
	p = [clamp(int(math.floor(math.pow(l[i], 1.0/gamma)*255 + 0.5))) for i in range(3)]
	if len(l) == 4:
		p.append(clamp(int(math.floor(l[3]*255 + 0.5))))
	return p

def nonlinear2pixel(l):
	return [clamp(int(math.floor(c*255 + 0.5))) for c in l]


def scale(infilename, outfilename, factor):
	inimg = Image.open(infilename)
	inpix = inimg.load()
	
	outw = inimg.size[0] // factor
	outh = inimg.size[1] // factor
	outimg = Image.new("RGBA", (outw, outh), None)
	outpix = outimg.load()
	
	for oy in range(outh):
		for ox in range(outw):
			# scale down in linear color space to get a tentative color to compute the fixed points from
			sum = [0.0, 0.0, 0.0, 0.0]
			for j in range(factor):
				for i in range(factor):
					l = pixel2linear(inpix[ox*factor+i, oy*factor+j])
					mulsc(l, l[3])
					add(sum, l)
			if sum[3] != 0:
				mulsc(sum, 1.0/sum[3])
			sum[3] /= factor*factor
			
			# determine the two fixed points (background colors on which we will achieve the correct result) per component
			# I used to use constant black and white for that, but later realized that that results in a large error (result too light) on midtones and I can do better by distributing the error more evenly. The dependency of the fixed points on the foreground color is empirical magic that has been experimentally determined to produce visually pleasing results.
			fix1 = [0.04*sum[i] for i in range(3)]
			fix2 = [0.4 + 0.6*sum[i] for i in range(3)]
			fix1n = [math.pow(l, 1.0/gamma) for l in fix1]
			fix2n = [math.pow(l, 1.0/gamma) for l in fix2]
			
			# composite against the fixed points in nonlinear color space as that's what the image expects (only matters in areas of medium alpha), then scale down in linear color space again
			f1c = [0.0, 0.0, 0.0]
			for j in range(factor):
				for i in range(factor):
					c, a = pixel2nonlinear(inpix[ox*factor+i, oy*factor+j])
					mulsc(c, a)
					f = fix1n[:]
					mulsc(f, 1.0 - a)
					add(c, f)
					add(f1c, [math.pow(p, gamma) for p in c])
			mulsc(f1c, 1.0/(factor*factor))
			
			f2c = [0.0, 0.0, 0.0]
			for j in range(factor):
				for i in range(factor):
					c, a = pixel2nonlinear(inpix[ox*factor+i, oy*factor+j])
					mulsc(c, a)
					f = fix2n[:]
					mulsc(f, 1.0 - a)
					add(c, f)
					add(f2c, [math.pow(p, gamma) for p in c])
			mulsc(f2c, 1.0/(factor*factor))
			
			# go back to gamma-encoded color space, assuming that alpha blending will be done in that
			f1cn = [math.pow(l, 1.0/gamma) for l in f1c]
			f2cn = [math.pow(l, 1.0/gamma) for l in f2c]
			
			# compute color and alpha
			# This gives us three alphas, in general different, but we can only output one - the best thing to do with them I can think of is to average them together and leave the color components alone, this ensures that the alpha deviation does not affect the case where background color equals foreground color.
			a = [1.0 - (f2cn[i] - f1cn[i])/(fix2n[i] - fix1n[i]) for i in range(3)]
			c = [(f1cn[i] - (1.0-a[i])*fix1n[i])/a[i] if math.floor(a[i]*255 + 0.5) > 0 else 0.0 for i in range(3)]
			outpix[ox, oy] = tuple(nonlinear2pixel(c + [(a[0] + a[1] + a[2])/3]))
	
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


if __name__ == "__main__":
	scale(sys.argv[1], sys.argv[3], int(sys.argv[2]))
