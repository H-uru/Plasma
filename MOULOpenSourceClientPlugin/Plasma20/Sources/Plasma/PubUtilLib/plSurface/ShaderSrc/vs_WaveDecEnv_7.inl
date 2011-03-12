

vs.1.0

dcl_position v0
dcl_color v5
dcl_texcoord0 v7
dcl_texcoord1 v8
dcl_texcoord2 v9

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
//m4x4     oPos, r6, c0; // ADDFOG
m4x4		r9, r6, c0;
add			r10.x, r9.w, c29.x;
mul			oFog, r10.x, c29.y;
//mov			oFog, c13.y;
mov			oPos, r9;

// Now onto texture coordinate generation.
//
// First is the usual texture transform
mov		r11.zw, c13.zzzz;
dp4		r11.x, v7, c14;
dp4		r11.y, v7, c15;
mov		oT0, r11;

// Calculate our basis vectors as input into our tex3x3vspec
// First we get our basis set off our surface. This is
// Okay, here we go:
// W == sum(k w Dir.x^2 A sin()) x
// V == sum(k w Dir.x Dir.y A sin()) x
// U == sum(k w Dir.y^2 A sin()) x
//
// T == sum(A sin())
//
// S == sum(k Dir.x A cos())
// R == sum(k Dir.y A cos())
//
// Q == sum(k w A cos()) x
//
// M == sum(A cos())
//
// P == sum(w Dir.x A cos()) x
// N == sum(w Dir.y A cos()) x
//
// Then:
// Pos = (in.x + S, in.y + R, waterheight + T) // Already done above.
//
// Bin = (1 - W, -V, P)
// Tan = (-V, 1 - U, N)
// Nor = (-P, -N, 1 - Q)
//
// The matrix
//		|Bx, Tx, Nx|
//		|By, Ty, Ny|
//		|Bz, Tz, Nz|
// is surface2world, but we still need to fold in
// texture2surface. We'll go with the generalized
// (not assuming a flat surface) partials of dPos/dU and dPos/dV
// as coming in as uv coords v8 and v9.
// Then, if r5 = v8 X v9, then texture to surface is
//		|v8.x, v9.x, r5.x|
//		|v8.y, v9.y, r5.y|
//		|v8.z, v9.z, r5.z|
//
// So, let's say we calc 3 vectors,
//		r7 = (Bx, Tx, Nx)
//		r8 = (By, Ty, Ny)
//		r9 = (Bz, Tz, Nz)
//
// Then surface2world * texture2surface =
//		|r7 dot v8, r7 dot v9, r7 dot r5|
//		|r8 dot v8, r8 dot v9, r8 dot r5|
//		|r9 dot v8, r9 dot v9, r9 dot r5|
//
// We will need r5 as v8 X v9
mov			r7, v8;
mul			r5.xyz, r7.yzx, v9.zxy;
mad			r5.xyz, r7.zxy, -v9.yzx, r5.xyz;

// Okay, r1 currently has the vector of cosines, and r2 has vector of sines.
// Everything will want that times amplitude, so go ahead and fold that in.
mul			r1, r1, c6; // r1 = A cos() = M
// Sines already have amplitude folded in, so r2 = A sin() = T.
// Now just compute r7-9 one element at a time.
dp4			r7.x, r2, -c35; // r7.x = -W
dp4			r7.y, r2, -c36; // r7.y = -V
dp4			r7.z, r1, -c32; // r7.z = -P
add			r7.x, r7.x, c13.z; // r7.x = 1 - W;

dp4			r8.x, r2, -c36; // r8.x = -V
dp4			r8.y, r2, -c37; // r8.y = -U
dp4			r8.z, r1, -c33; // r8.z = -N
add			r8.y, r8.y, c13.z; // r8.y = 1 - U

dp4			r9.z, r2, -c34; // r9.z = -Q
mov			r9.x, -r7.z; // r9.x = P = -r7.z
mov			r9.y, -r8.z; // r9.y = N = -r8.z
add			r9.z, r9.z, c13.z; // r9.z = 1 - Q

// Okay, got everything we need, construct r1-3 as surface2world*texture2surface.
dp3			r1.x, r7, v8;
dp3			r1.y, r7, v9;
dp3			r1.z, r7, r5;

dp3			r2.x, r8, v8;
dp3			r2.y, r8, v9;
dp3			r2.z, r8, r5;

dp3			r3.x, r9, v8;
dp3			r3.y, r9, v9;
dp3			r3.z, r9, r5;

// Following section is debug only to skip the per-vert tangent space axes.
//add r1, c13.zxxx, r7.zzxw;
//add r2, c13.xzxx, r7.zzyw;
//
//mov r3.x, -r7.x;
//mov r3.y, -r7.y;
//mov r3.zw, c13.zz;

// See vs_WaveFixedFin6.inl for derivation of the following
sub			r0, r6, c27; // c27 is camera position.
dp3			r10.x, r0, r0;
rsq			r10.x, r10.x;
mul			r0, r0, r10.xxxx;

dp3			r10.x, r0, c28; // c28 is kEnvAdjust
mad			r10.y, r10.x, r10.x, -c28.w;

rsq			r9.x, r10.y;

mad			r10.z, r10.y, r9.x, r10.x;

mad			r0.xyz, r0, r10.zzz, -c28.xyz;

// ATI 9000 is having trouble with eyeVec as computed. Normalizing seems to get it over the hump.
dp3			r10.x, r0, r0;
rsq			r9.x, r10.x;
mul			r0.xyz, r0.xyz, r9.xxx;

mov			r1.w, -r0.x;
mov			r2.w, -r0.y;
mov			r3.w, -r0.z;


// Now r1-r3 are texture2world, with the eye-ray vector in .w. We just
// need to normalize them and bung them into output UV's 1-3.
// Note we're accounting for our environment map being flipped from
// D3D (and all rational thought) by putting r2 into UV3 and r3 into UV2.
mov r10.w, c13.z;
dp3			r10.x, r1, r1;
rsq			r10.x, r10.x;
mul			oT1, r1, r10.xxxw;

dp3			r10.x, r3, r3;
rsq			r10.x, r10.x;
mul			oT2, r3, r10.xxxw;
//mul			oT3, r3, r10.xxxw; // YZHACK

dp3			r10.x, r2, r2;
rsq			r10.x, r10.x;
mul			oT3, r2, r10.xxxw;
//mul			oT2, r2, r10.xxxw;

// Output color is vertex green
// Output alpha is vertex red (vtx alpha is used for wave filtering)
// Whole thing modulated by material color/opacity.
mul		oD0, v5.yyyx, c26;

