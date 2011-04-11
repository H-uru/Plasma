
vs.1.1

dcl_position v0
dcl_color v5
dcl_texcoord0 v7

// Store our input position in world space in r6
m4x3		r6, v0, c18; // v0 * l2w
// Fill out our w (m4x3 doesn't touch w).
mov			r6.w, c13.z;

//

// Input diffuse v5 color is:
// v5.r = overall transparency
// v5.g = illumination
// v5.b = overall wave scaling
//
// v5.a is:
// v5.w = 1/(2.f * edge length)
// So per wave filtering is:
// min(max( (waveLen * v5.wwww) - 1), 0), 1.f);
// So a wave effect starts dying out when the wave is 4 times the sampling frequency,
// and is completely filtered at 2 times sampling frequency.

// We'd like to make this autocalculated based on the depth of the water.
// The frequency filtering (v5.w) still needs to be calculated offline, because
// it's dependent on edge length, but the first 3 filterings can be calculated
// based on this vertex.
// Basically, we want the transparency, reflection strength, and wave scaling
// to go to zero as the water depth goes to zero. Linear falloffs are as good
// a place to start as any.
//
// depth = waterlevel - r6.z		=> depth in feet (may be negative)
// depthNorm = depth / depthFalloff	=> zero at watertable, one at depthFalloff beneath
// atten = minAtten + depthNorm * (maxAtten - minAtten);
// These are all vector ops.
// This provides separate ramp ups for each of the channels (they reach full unfiltered
// values at different depths), but doesn't provide separate controls for where they
// go to zero (they all go to zero at zero depth). For that we need an offset. An offset
// in feet (depth) is probably the most intuitive. So that changes the first calculation
// of depth to:
// depth = waterlevel - r6.z + offset
//		= (waterlevel + offset) - r6.z
// And since we only need offsets for 3 channels, we can make the waterlevel constant
// waterlevel[chan] = watertableheight + offset[chan],
// with waterlevel.w = watertableheight.
//
// So:
//	c22 = waterlevel + offset
//	c23 = (maxAtten - minAtten) / depthFalloff
//	c24 = minAtten.
// And in particular:
//	c22.w = waterlevel
//	c23.w = 1.f;
//	c24.w = 0;
// So r4.w is the depth of this vertex in feet.

// Dot our position with our direction vectors.
mul		r0, c7, r6.xxxx;
mad		r0, c8, r6.yyyy, r0;

//
//    dist = mad( dist, kFreq.xyzw, kPhase.xyzw);
mul         r0, r0, c4;
add			r0, r0, c5;
//
//    // Now we need dist mod'd into range [-Pi..Pi]
//    dist *= rcp(kTwoPi);
rcp         r4, c12.wwww;
add			r0, r0, c12.zzzz;
mul         r0, r0, r4;
//    dist = frac(dist);
expp     r1.y, r0.xxxx
mov      r1.x, r1.yyyy
expp     r1.y, r0.zzzz
mov      r1.z, r1.yyyy
expp     r1.y, r0.wwww
mov      r1.w, r1.yyyy
expp     r1.y, r0.yyyy
//    dist *= kTwoPi;
mul         r0, r1, c12.wwww;
//    dist += -kPi;
sub         r0, r0, c12.zzzz;

//
//    sincos(dist, sinDist, cosDist);
// sin = r0 + r0^3 * vSin.y + r0^5 * vSin.z
// cos = 1 + r0^2 * vCos.y + r0^4 * vCos.z
mul         r1, r0, r0; // r0^2
mul         r2, r1, r0; // r0^3 - probably stall
mul         r3, r1, r1; // r0^4
mul         r4, r1, r2; // r0^5
mul         r5, r2, r3; // r0^7

mul         r1, r1, c11.yyyy;       // r1 = r0^2 * vCos.y
mad         r2, r2, c10.yyyy, r0;   // r2 = r0 + r0^3 * vSin.y
add         r1, r1, c11.xxxx;       // r1 = 1 + r0^2 * vCos.y
mad         r2, r4, c10.zzzz, r2;   // r2 = r0 + r0^3 * vSin.y + r0^5 * vSin.z
mad         r1, r3, c11.zzzz, r1;   // r1 = 1 + r0^2 * vCos.y + r0^4 * vCos.z

// r0^7 & r0^6 terms
mul         r4, r4, r0; // r0^6
mad         r2, r5, c10.wwww, r2;
mad         r1, r4, c11.wwww, r1;

// Calc our depth based filtering here into r4 (because we don't use it again
// after here, and we need our filtering shortly).
sub			r4, c22, r6.zzzz;
mul			r4, r4, c23;
add			r4, r4, c24;
// Clamp .xyz to range [0..1]
min			r4.xyz, r4, c13.zzzz;
max			r4.xyz, r4, c13.xxxx;
//mov r4.xyz, c13.xxx; // HACKTEST

// Calc our filter (see above).
mul			r11, v5.wwww, c21;
max			r11, r11, c13.xxxx;
min			r11, r11, c13.zzzz;

//mov    r2, r1;
// r2 == sinDist
// r1 == cosDist
//    sinDist *= filter;
mul         r2, r2, r11;
//    sinDist *= kAmplitude.xyzw
mul         r2, r2, c6;
//    height = dp4(sinDist, kOne);
//    accumPos.z += height; (but accumPos.z is currently 0).
dp4         r8.x, r2, c13.zzzz;
mul			r8.y, r8.x, r4.z;
add			r8.z, r8.y, c22.w;
max			r6.z, r6.z, r8.z;
// r8.x == wave height relative to 0
// r8.y == dampened wave relative to 0
// r8.z == dampened wave height in world space
// r6.z == wave height clamped to never go beneath ground level
//
//    cosDist *= filter;
mul         r1, r1, r11;

// Pos = (in.x + S, in.y + R, r6.z)
// S = sum(k Dir.x A cos())
// R = sum(k Dir.y A cos())
// c30 = k Dir.x A
// c31 = k Dir.y A
//    S = sum(cosDist * c30);
dp4         r7.x, r1, c30;
//    R = sum(cosDist * c31);
dp4			r7.y, r1, c31;

add			r6.xy, r6.xy, r7.xy;

// Bias our vert up a bit to compensate for precision errors.
// In particular, our filter coefficients are coming in as
// interpolated bytes, so there's bound to be a lot of slop
// from that. We've got a free slot in c25.x, so we'll use that.
// A better implementation would be to bias and scale our screen
// vert, effectively pushing the vert toward the camera without
// actually moving it, but this is easier and might work just
// as well.
add			r6.z, r6.z, c25.x;

//
// // Transform position to screen
//
//
//m4x3	r6, v0, c18; // HACKAGE
//mov		r6.w, c13.z; // HACKAGE
//m4x4     oPos, r6, c0; // ADDFOG
m4x4		r9, r6, c0;
add			r10.x, r9.w, c29.x;
mul			oFog, r10.x, c29.y;
mov			oPos, r9;

// Output color is vertex green
// Output alpha is vertex red (vtx alpha is used for wave filtering)
// Whole thing modulated by material color/opacity.
mul		oD0, v5.yyyx, c26;

// Usual texture transform
mov		r11.zw, c13.zzzz;
dp4		r11.x, v7, c14;
dp4		r11.y, v7, c15;
mov		oT0, r11;

dp4		r11.x, v7, c16;
dp4		r11.y, v7, c17;
mov		oT1, r11;
