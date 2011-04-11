
vs.1.1

dcl_position v0
dcl_color v5

// Store our input position in world space in r6
m4x3		r6, v0, c21; // v0 * l2w
// Fill out our w (m4x3 doesn't touch w).
mov			r6.w, c16.zzzz;

//

// Input diffuse v5 color is:
// v5.r = overall transparency
// v5.g = reflection strength (transparency)
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
//	c25 = waterlevel + offset
//	c26 = (maxAtten - minAtten) / depthFalloff
//	c27 = minAtten.
// And in particular:
//	c25.w = waterlevel
//	c26.w = 1.f;
//	c27.w = 0;
// So r4.w is the depth of this vertex in feet.

// Dot our position with our direction vectors.
mul		r0, c8, r6.xxxx;
mad		r0, c9, r6.yyyy, r0;

//
//    dist = mad( dist, kFreq.xyzw, kPhase.xyzw);
mul         r0, r0, c5;
add			r0, r0, c6;
//
//    // Now we need dist mod'd into range [-Pi..Pi]
//    dist *= rcp(kTwoPi);
rcp         r4, c15.wwww;
add			r0, r0, c15.zzzz;
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
mul         r0, r1, c15.wwww;
//    dist += -kPi;
sub         r0, r0, c15.zzzz;

//
//    sincos(dist, sinDist, cosDist);
// sin = r0 + r0^3 * vSin.y + r0^5 * vSin.z
// cos = 1 + r0^2 * vCos.y + r0^4 * vCos.z
mul         r1, r0, r0; // r0^2
mul         r2, r1, r0; // r0^3 - probably stall
mul         r3, r1, r1; // r0^4
mul         r4, r1, r2; // r0^5
mul         r5, r2, r3; // r0^7

mul         r1, r1, c14.yyyy;       // r1 = r0^2 * vCos.y
mad         r2, r2, c13.yyyy, r0;   // r2 = r0 + r0^3 * vSin.y
add         r1, r1, c14.xxxx;       // r1 = 1 + r0^2 * vCos.y
mad         r2, r4, c13.zzzz, r2;   // r2 = r0 + r0^3 * vSin.y + r0^5 * vSin.z
mad         r1, r3, c14.zzzz, r1;   // r1 = 1 + r0^2 * vCos.y + r0^4 * vCos.z

// r0^7 & r0^6 terms
mul         r4, r4, r0; // r0^6
mad         r2, r5, c13.wwww, r2;
mad         r1, r4, c14.wwww, r1;

// Calc our depth based filtering here into r4 (because we don't use it again
// after here, and we need our filtering shortly).
sub			r4, c25, r6.zzzz;
mul			r4, r4, c26;
add			r4, r4, c27;
// Clamp .xyz to range [0..1]
min			r4.xyz, r4, c16.zzzz;
max			r4.xyz, r4, c16.xxxx;

// Calc our filter (see above).
mul			r11, v5.wwww, c24;
max			r11, r11, c16.xxxx;
min			r11, r11, c16.zzzz;

//mov    r2, r1;
// r2 == sinDist
// r1 == cosDist
//    sinDist *= filter;
mul         r2, r2, r11;
//    sinDist *= kAmplitude.xyzw
mul         r5, r2, c7;
// r5 is now T = sum(Ai * sin())
//    height = dp4(sinDist, kOne);
//    accumPos.z += height; (but accumPos.z is currently 0).
dp4         r8.x, r5, c16.zzzz;
mul			r8.y, r8.x, r4.z;
add			r8.z, r8.y, c25.w;
max			r6.z, r6.z, r8.z; // CLAMP
// r8.x == wave height relative to 0
// r8.y == dampened wave relative to 0
// r8.z == dampened wave height in world space
// r6.z == wave height clamped to never go beneath ground level
//
//    cosDist *= kAmplitude.xyzw; // Combine?
mul         r7, r1, c7;
//    cosDist *= filter;
mul         r7, r7, r11;
// r7 is now M = sum(Ai * cos())

// Okay, here we go:
// W == sum(k w Dir.x^2 A sin())
// V == sum(k w Dir.x Dir.y A sin())
// U == sum(k w Dir.y^2 A sin())
//
// T == sum(A sin())
//
// S == sum(k Dir.x A cos())
// R == sum(k Dir.y A cos())
//
// Q == sum(k w A cos())
//
// M == sum(A cos())
//
// P == sum(w Dir.x A cos())
// N == sum(w Dir.y A cos())
//
// Then:
// Pos = (in.x + S, in.y + R, waterheight + T)
//
// Bin = (1 - W, -V, P)
// Tan = (-V, 1 - U, N)
// Nor = (-P, -N, 1 - Q)
//
// But we want the transpose of that to go into r1-r3

dp4			r10.x, r7, c29;
add			r6.x, r6.x, r10.x;
dp4			r10.x, r7, c30;
add			r6.y, r6.y, r10.x;

dp4			r1.x, r5, -c34;
dp4			r2.x, r5, -c35;
dp4			r3.x, r7, c31;
add			r1.x, r1.xxxx, c16.zzzz;

dp4			r1.y, r5, -c35;
dp4			r2.y, r5, -c36;
dp4			r3.y, r7, c32;
add			r2.y, r2.yyyy, c16.zzzz;

dp4			r1.z, r7, -c31;
dp4			r2.z, r7, -c32;
dp4			r3.z, r5, -c33;
add			r3.z, r3.zzzz, c16.zzzz;


// Calculate our normalized vector from camera to vtx.
// We'll use that a couple of times coming up.
sub			r5, r6, c17;
dp3			r10.x, r5, r5;
rsq			r10.x, r10.x;
mul			r5, r5, r10.xxxx; // r0 = D
rcp			r5.w, r10.x;

// Calculate our specular attenuation from and into r5.w.
// r5.w starts off the distance from vtx to camera.
// Once we've turned it into an attenuation factor, we
// scale the x and y of our normal map (through the transform bases)
// so that in the distance, the normal map is flat. Note that the
// geometry in the distance isn't necessarily flat. We want to apply
// this scale to the normal read from the normal map before it is
// transformed into surface space.
add			r5.w, r5.w, c11.x;
mul			r5.w, r5.w, c11.y;
min			r5.w, r5.w, c16.z;
max			r5.w, r5.w, c16.x;
mul			r5.w, r5.w, r5.w; // Square it to account for perspective
mul			r5.w, r5.w, c11.z;


// Normalize?

// We can either calculate an orthonormal basis from the
// computed normal, with Binormal = (0,1,0) X Normal, Tangent = Normal X (1,0,0),
// or compute our basis directly from the partial derivatives, with
// Binormal = (1, 0, -cosX), Tangent = (0, 1, -cosY), Normal = (cosX, cosY, 1)
//
// These work out to identically the same result, so we'll compute directly
// from the partials because it takes 2 fewer instructions.
//
// Note that our basis is NOT orthonormal. The Normal is equal to
// Binormal X Tangent, but Dot(Binormal, Tangent) != 0. The Binormal and Tangents
// are both correct tangents to the surface, and their projections on the XY plane
// are 90 degrees apart, but in 3-space, they are not orthogonal. Practical implications?
// Not really. I'm actually not really sure which is more "proper" for bump mapping.
//
// Note also that we add when we should subtract and subtract when we should
// add, so that r1, r2, r3 aren't Binormal, Tangent, Normal, but the rows
// of our transform, (Bx, Tx, Nx), (By, Ty, Ny), (Bz, Tz, Nz). See below for
// explanation.
//
// Binormal = Y % Normal
// Cross product3 is:
//	mul		res.xyz, a.yzx, b.zxy
//	mad		res.xyz, -a.zxy, b.yzx, res.xyz
//   mul			r1.xyz, c16.zxx, r3.zxy;
//   mad			r1.xyz, -c16.xxz, r3.yzx, r1.xyz;

// Tangent = Normal % X
//   mul			r2.xyz, r3.yzx, c16.xzx;
//   mad			r2.xyz, -r3.zxy, c16.xxz, r2;

//mad			r1, r5.wwww, c16.zxxx, r7.zzxz;
//mad			r2, r5.wwww, c16.xzxx, r7.zzyz;
//mul			r3.xy, r3.xy, r5.wwww;


// Note that we're swapping z and y to match our environment map tools in max.
// We do this through our normal map transform (oT1, oT2, oT3), making it
// a concatenation of:
//
//	rotate about Z (blue) to turn our map into the wind
//	windRot =	|	dirY	-dirX	0 |
//				|	dirX	dirY	0 |
//				|	0		0		1 |
//
//	swap our Y and Z axes to match our environment map
//	swapYZ	=	|	1		0		0 |
//				|	0		0		1 |
//				|	0		1		0 |
//
//	rotate the normal into the surface's tangent space basis
//	basis	=	|	Bx		Tx		Nx |
//				|	By		Ty		Ny |
//				|	Bz		Tz		Nz |
//
//	Note that we've constucted the basis by taking advantage of the
//	matrix being a pure rotation, as noted below, so r1, r2 and r3
//	are actually constructed as:
//	basis	=	|	Bx		-By		-Bz |
//				|	-Tx		Ty		-Tz |
//				|	-Nx		-Ny		-Nz |
//
//	Then the final normal map transform is:
//
//		basis * swapYZ * windRot [ * normal ]


//   sub         r1.w, c17.x, r6.x;
//   sub         r2.w, c17.z, r6.z;
//   sub         r3.w, c17.y, r6.y;

// Big note here. All this math can blow up if the camera position
// is outside the environment sphere. It's assumed that's dealt
// with in the app setting up the constants. For that reason, the
// camera position used here might not be the real local camera position,
// which is needed for the angular attenuation, so we burn another constant
// with our pseudo-camera position. To restrain the pseudo-camera from
// leaving the sphere, we make:
//	pseudoPos = envCenter + (realPos - envCenter) * dist * R / (dist + R)
// where dist = |realPos - envCenter|

// So, our "finitized" eyeray is:
//	camPos + D * t - envCenter = D * t - (envCenter - camPos)
// with
//	D = (pos - camPos) / |pos - camPos| // normalized usual eyeray
// and
//	t = D dot F + sqrt( (D dot F)^2 - G )
// with
//	F = (envCenter - camPos)	=> c19.xyz
//	G = F^2 - R^2				=> c19.w
//	R = environment radius.		=> unused
//
// This all derives from the positive root of equation
//	(camPos + (pos - camPos) * t - envCenter)^2 = R^2,
// In other words, where on a sphere of radius R centered about envCenter
// does the ray from the real camera position through this point hit.
//
// Note that F, G, and R are all constants (one point, two scalars).
//
// So first we calculate D into r0,
// then D dot F into r10.x,
// then (D dot F)^2 - G into r10.y
// then rsq( (D dot F)^2 - G ) into r9.x;
// then t = r10.z = r10.x + r10.y * r9.x;
// and
// r0 = D * t - (envCenter - camPos)
//		= r0 * r10.zzzz - F;
//
mov			r0, r5; // r0 = D

dp3			r10.x, r0, c19; // r10.x = D dot F
mad			r10.y, r10.x, r10.x, -c19.w; // r10.y = (D dot F)^2 - G

rsq			r9.x, r10.y; // r9.x = 1/SQRT((D dot F)^2 - G)

mad			r10.z, r10.y, r9.x, r10.x; // r10.z = D dot F + SQRT((D dot F)^2 - G)

mad			r0.xyz, r0, r10.zzz, -c19.xyz; // r0.xyz = D * t - (envCenter - camPos)

// ATI 9000 is having trouble with eyeVec as computed. Normalizing seems to get it over the hump.
dp3			r10.x, r0, r0;
rsq			r9.x, r10.x;
mul			r0.xyz, r0.xyz, r9.xxx;

mov			r1.w, -r0.x;
mov			r2.w, -r0.y;
mov			r3.w, -r0.z;

mov			r0.zw, c16.zzxz;

dp3         r0.x, r1, r1;
rsq         r0.xy, r0.x;
mul			r0.x, r0.x, r5.w;
mul         oT1, r1.xyzw, r0.xxyw;
//   mul			r8, r1.xyzw, r0.xxxw; // VISUAL
mul			r11.x, r1.z, r0.y;


dp3         r0.x, r2, r2;
rsq         r0.xy, r0.x;
mul			r0.x, r0.x, r5.w;
mul         oT3, r2.xyzw, r0.xxyw;
//   mul			r9, r2.xyzw, r0.xxxw; // VISUAL
mul			r11.y, r2.z, r0.y;

dp3         r0.x, r3, r3;
rsq         r0.xy, r0.x;
mul			r0.x, r0.x, r5.w;
mul         oT2, r3.xyzw, r0.xxyw;
//   mul			r9, r3.xyzw, r0.xxxw; // VISUAL
mul			r11.z, r3.z, r0.y;


/*
// Want:
//    oT1 = (BIN.x, TAN.x, NORM.x, view2pos.x)
//    oT2 = (BIN.y, TAN.y, NORM.y, view2pos.y)
//    ot3 = (BIN.z, TAN.z, NORM.z, view2pos.z)
// with BIN, TAN, and NORM normalized.
// Unnormalized, we have
//    BIN = (1, 0, -r7.x) where r7 == accumCos
//    TAN = (0, 1, -r7.y)
//    NORM= (r7.x, r7.y, 1)
// So, unnormalized, we have
//    oT1 = (1, 0, r7.x, view2pos.x)
//    oT2 = (0, 1, r7.y, view2pos.y)
//    oT3 = (-r7.x, -r7.y, 1, view2pos.z)
// which is just reversing the signs on the accumCos
// terms above. So the normalized version is just
// reversing the signs on the normalized version above.
*/
//mov oT3, r4;

//
// // Transform position to screen
//
//
//m4x3	r6, v0, c21; // HACKAGE
//mov		r6.w, c16.z; // HACKAGE
//m4x4     oPos, r6, c0; // ADDFOG
m4x4		r9, r6, c0;
add			r10.x, r9.w, c28.x;
mul			oFog, r10.x, c28.y;
//mov			oFog, c16.zzzz; // TESTFOGHACK
mov			oPos, r9;

// Transform our uvw
mul			r0.x, v0.xxxx, c10.xxxx;
mul			r0.y, v0.yyyy, c10.xxxx;

//mov			r0.zw, c16.xxxz;
mov			oT0, r0

// Questionble attenuation follows
// vector from this point to camera and normalize stashed in r5
// Dot that with the computed normal
dp3			r1.x, -r5, r11;
mul			r1.x, r1.x, v5.z;
//	dp3			r1.x, r5, r3; // if you want the adjusted normal, you'll need to normalize/swizzle r3
// Map dot=1 => 0, dot=0 => 1
sub			r1.xyzw, c16.zzzz, r1.xxxx;
add			r1.w, r1.wwww, c16.zzzz;
mul			r1.w, r1.wwww, c16.yyyy;
// No need to clamp, since the destination register (in the pixel shader)
// will saturate [0..1] anyway.
//%%% mul			r1.w, r1.w, r4.x;
//%%% mul			r1.xyz, r1.xyz, r4.yyy;
mul r1, r1, r4.yyyx; // HACKTESTCOLOR
//mul	r1.xyz, r1, r8.xxx; // WAVEFACE
mul r1.w,	r1.wwww, v5.xxxx;
mul r1.w,	r1.wwww, c4.wwww;
mul			oD0, r1, c20;

mov         oD1, c4; // SEENORM
//mov oD1, c16.xxxx;
// mov oD1, r4.yyyy;

//mov			oD1, c16.zzzz; // HACKAGE
//	mov			oD1, r9;
//	mov			oD1, r8.xzyw;
