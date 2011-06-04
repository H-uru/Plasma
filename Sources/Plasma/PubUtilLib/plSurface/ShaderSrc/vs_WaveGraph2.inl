
vs.1.1

dcl_position v0
dcl_normal v3

// c0 = (0,0.5,1.0,2.0) (aka NumericConsts)
// c1 = frequencies
// c2 = phases
// c3 = amplitudes

// c4 = PiConsts = (1/(2PI), PI/2, PI, 2*PI) // NOTE THIS IS DIFFERENT
//		because we don't need oonsqpi here but do want 1/2Pi.
// c5 = cosConsts = (1.0f, -1.0f/2.0f, 1.0f/ 24.0f, -1.0f/ 720.0f);

// c6 = ((cMax - cMin), cMin, 2ndLayerVOffset, 2ndLayerScale);
// c7 = overall color, including current opacity. Will
//		probably only use the opacity, which we could stuff into
//		the free slot of c6, but we're a wuss.

// First, "move" the position to oPos
mov r0, v0;
//mov r0.y, -r0.yyyy;
mov r0.w, c0.zzzz;
mov oPos, r0;

// Now the tricky part.

// The base layer defines the shape of the incoming wave
// The next layer has bubbles (noise) and moves in when the
//		wave is moving in, moves out when wave is moving out.
// So calculate uvw for first layer, second uvw shares u val
//		and v val is const

// The .x component of the normal
// tells us how much to shift this vert based on the
// cumulative cosine wave.

// Figure c = Sigma((cosine(v0.x * freq + phase) + 1) * amp);
// Note that range c must be [0..1]
// Also, c(-1) must equal c(1) so it will wrap.
// That implies freq = k * 2 * PI, where k is an integer.
// To keep c >= 0, we can add 1 to each term in the sigma BEFORE
// modulating by the amplitude.
// That puts our range at [0..2*sigma(amp)], so as long as
// sigma(amp) <= 0.5, we're fine.

// Get our input to cosine value (v0.x * freq + phase).
add		r0, v0.xxxx, c0.zzzz;
mul		r0, r0, c1;
add		r0, r0, c2;

// Get it into range [-Pi..Pi]
// First divide out the 2PI
// add			r0, r0, c4.zzzz; HACKOUT
mul         r0, r0, c4.xxxx;

// Do an integer mod
expp		r1.y, r0.xxxx
mov			r1.x, r1.yyyy
expp		r1.y, r0.zzzz
mov			r1.z, r1.yyyy
expp		r1.y, r0.wwww
mov			r1.w, r1.yyyy
expp		r1.y, r0.yyyy

//mov oD1, r1; // HACKTEST
//mov oD1.w, c0.zzzz; // HACKTEST

// Move back into PI space, w/ *= 2P, -= PI
mul         r0, r1, c4.wwww;
sub         r0, r0, c4.zzzz;

// Okay, compute cosine here.
// cos = 1 + r0^2 * kCos.y + r0^4 * kCos.Z + r0^6 * kCos.w
// Note: could pare off an instr by putting 1/kCos.w in kCos.x,
// then doing a mad to get r3=(1/kCos.w + r0^6), then mad that
// into the accum by kCos.w to get (1 + r0^6*kCos.x). But who cares.
mul			r1, r0, r0; // r0^2
mul			r2, r1, r1; // r0^4
mul			r3, r1, r2; // r0^6

mov			r4, c5.xxxx;			// r4 = 1
mad			r4, r1, c5.yyyy, r4;	// r4 += r0^2 * kCos.y
mad			r4, r2, c5.zzzz, r4;	// r4 += r0^4 * kCos.z
mad			r4, r3, c5.wwww, r4;	// r4 += r0^6 * kCos.w

add			r4, r4, c0.zzzz;	// shift from [-1..1] to [0..2]
//mov	r4, c0.xxxx; // HACKLAST
mul			r4, r4, c3;			// times amplitude

dp4			r5.y, r4, c0.zzzz; // r5.x = sigma((cos() + 1) * amp);

// V calculation, goes something like:
// For layers 0 and 2:
//		V = { 1 + c6.z	<= r5.y = 0 } * norm.x // norm.x == v3.x
//			{ 1 + 0		<= r5.y = 1 }
// For layer 1:
//		V = (norm.x + c6.z) * c6.w // Scaled like U
//
// Another way to formulate that is
// baseV = cMin + sinAge * (cMax-cMin) where
//		cMin = 2
//		cMax = 1
//		sinAge = color.a = c7.w
// delV = sigma(cos) = r5.y
// Then
//		V0 = V2 = (baseV + delV) * v3.x
//		V1 = (norm.x + baseV + delV) * c6.w
//
// If we're sure we want cMin = 2 and cMax = 1, then it simplifies to:
//	baseV = 2 - sinAge = c0.w - c7.w
//	delV = r5.y
//  (baseV + delV) = c0.w - c7.w + r5.y
//
// If we want to stay general, then
//	baseV = c6.x * c7.w + c6.y
//	delV = -r5.y
//	(baseV + delV) = constant + r5.y
//

// make r5.y = (baseV + delV)
add			r5.y, c6.xxxx, r5.yyyy;

//mov oD1, r5.yyyy; // HACKLAST
//mov oD1.w, c0.zzzz; // HACKLAST

// U is input U (or v0.x * 0.5f + 0.5f)
mul			r5.x, v0.x, c0.y;
add			r5.x, r5.x, c0.y;

// Fill out wq.
mov			r5.zw, c0.xz;

mul			oT0, r5, v3.wxww;
// mov oD1, r5.yyyw; // HACKTEST
mul			oT2, r5, v3.wxww;

// Second uv shares u, but v is norm.x + c6.x;
// Then we scale it.
// If we want the bubble texture to move with the
// wave front, we want the second UV calc (RESCALE1).
// But it looks better to have the bubbles moving
// slightly faster than the wave front. RESCALE0
// happens to do that, because we're scaling the
// texture by a factor of 2, but we should probably
// supply an independent scale of the motion vs. the
// scale of the texture.
// Let's move c6 to r6 for ease of use.
mov				r6, c6;
// add			r5.x, r5.x, c6.y;
// add			r5.y, c6.xxxx, v3.xxxx; // RESCALE0
// mul			r5.xy, r5, c6.wwww;		// RESCALE0
add			r5.x, r5.x, r6.y;	// RESCALE1 // offset U
mov			r5.y, v3.xx;		// RESCALE1 // Init V to value stashed in normal.x
mul			r5.xy, r5, r6.wwww;	// RESCALE1 // scale them by single scale value
mad			r5.y, r6.xx, r6.zz, r5.yy;	// RESCALE1 // add in our scaled V offset (sinage * vScale)
mov			oT1, r5;

//mov	oT0, v7; // HACKTEST
//mov oT1, v7; // HACKTEST
//mov oT2, v7; // HACKTEST

// Just slam in the constant color (includes our current opacity).
mov			oD0, c7;
//mov	oD0, c0.zzzz; // HACKTEST
