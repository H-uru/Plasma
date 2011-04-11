vs.1.1

dcl_position v0

//m4x4 oPos, v0, c0


/*
In fact, I was trying to understand how it was possible to expand FRC into 4
instructions...
Actually, I can do it in 7 instructions :)

EXPP r0.y, r1.xxxx
MOV r0.x, r0.y
EXPP r0.y, r1.zzzz
MOV r0.z, r0.y
EXPP r0.y, r1.wwww
MOV r0.w, r0.y
EXPP r0.y, r1.yyyy
*/

/*
   // Constants for sin and cos. 3 term approximation seems plenty
   // (it's what i used for software sim, and had no visibly different
   // results than the math library functions).
   // When doing sin/cos together, some speedup might be obtained
   // with good pairing of ops doing them simultaneously. Also save
   // an instruction calculating r0^3.
        D3DXVECTOR4 vSin( 1.0f, -1.0f/6.0f, 1.0f/120.0f, -1.0f/5040.0f );
        D3DXVECTOR4 vCos( 1.0f, -1.0f/2.0f, 1.0f/ 24.0f, -1.0f/ 720.0f );
*/

/*
Cos():


  r1 = mul(r0, r0);     // r0^2
  r2 = mul(r1, r1);     // r0^4

  //cos
  r3 = mad( r1, vCos.yyyy, vCos.xxxx );
  r3 = mad( r2, vCos.zzzz, r3 );
*/

/*
Sin();
  r1 = mul(r0, r0);     // r0^3
  r1 = mul(r0, r1);
  r2 = mul(r1, r1);     // r0^6

  r3 = mad( r1, vSin.yyyy, r0 );
  r3 = mad( r2, vSin.zzzz, r3 );
*/

/*
SinCos():

  r1 = mul(r0, r0);     // r0^2
  r2 = mul(r1, r0);     // r0^3 // probably stall
  r3 = mul(r1, r1);     // r0^4
  r4 = mul(r2, r2);     // r0^6

  r5 = mad( r1, vCos.yyyy, vCos.xxxx );
  r6 = mad( r2, vSin.yyyy, r0 );
  r5 = mad( r3, vCos.zzzz, r5 );
  r6 = mad( r4, vSin.zzzz, r6 );

*/

/*
consts
   kOneOverEightNsqPi      = 1.f / ( 8.f * Pi * 4.f * 4.f );
   kPiOverTwo           = Pi / 2.f;
   kTwoPi               = Pi * 2.f;
   kPi                  = Pi;
*/
/*
CONSTANT REGISTERS
VOLATILE CONSTS - change per invocation
C0-C3 local2proj matrix
C4    color
C5    freq vector
C6    phase vector
C7    amplitude vector
C8    center0
C9    center1
C10      center2
C11      center3
C12      scrunch = (scrunch, -scrunch, 0, 1);
CONSTANT CONSTS - forever more
C13      SinConsts = (1.0f, -1.0f/6.0f, 1.0f/120.0f, -1.0f/5040.0f);
C14      CosConsts = (1.0f, -1.0f/2.0f, 1.0f/ 24.0f, -1.0f/ 720.0f);
C15      PiConsts = (1.f / 8*Pi*N^2, Pi/2, Pi, 2*Pi);
C16      numberConsts = (0.f, 0.5f, 1.f, 2.f);
//=====================================
TEMP REGISTERS
r6    accumPos
r7    accumCos
r8    toCenter_Y
r9    toCenter_X
r11      filter
r10      tempFloat
*/
// const float4 kCosConsts = float4(1.0f, -1.0f/2.0f, 1.0f/ 24.0f, -1.0f/ 720.0f);
// const float4 kSinConsts = float4(1.0f, -1.0f/6.0f, 1.0f/120.0f, -1.0f/5040.0f);

// const float4 kPiConsts = float4(1.f / (8.f * 3.1415f * 16f), 3.1415f*0.5f, 3.1415f, 3.1515f*2.f);
// const float4 k0512 = float4(0.f, 0.5f, 1.f, 2.f);

// accumPos = inPos;
   mov         r6, v0;
//
// For each wave
// {
//    // First, we want to filter out waves based on distance from the local origin
//    dist = dp3(inPos, inPos);
   dp3         r0, r6, r6;
//    dist *= kFreqSq.xyzw;
   mul         r0, r0, c5;
   mul         r0, r0, c5;
//    dist *= kOneOverEightNsqPi; // combine this into kFreqSq?
   mul         r0, r0, c15.xxxx;
//    dist = min(dist, kPiOverTwo);
   min         r0, r0, c15.yyyy;
//    filter = cos(dist);
   mul         r1, r0, r0;    // r0^2
   mul         r2, r1, r1;    // r1^2
   mul         r1, r1, c14.yyyy;
   add         r11, r1, c14.xxxx;
   mad         r11, r2, c14.zzzz, r11;


//    filter *= kAmplitude.xyzw;
//   mul         r11, r11, c7;
//    // Notice that if dist is a 4vec, all this can be simultaneously done for 4 waves at a time.
//
//    Find the x/y distances and stuff them into r9(x) and r8(y) respectively
   // toCenter_X.x = dir0.x * pos.x;
   // toCenter_Y.x = dir0.y * pos.y;
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

//mov    r2, r1;
   // r2 == sinDist
   // r1 == cosDist
//    sinDist *= filter;
   mul         r2, r2, r11;
//    sinDist *= kAmplitude.xyzw
   mul         r2, r2, c7;
//    height = dp4(sinDist, kOne);
//    accumPos.z += height; (but accumPos.z is currently 0).
   dp4         r6.z, r2, c16.zzzz;
//
//    cosDist *= kFreq.xyzw;
   mul         r1, r1, c5;
//    cosDist *= kAmplitude.xyzw; // Combine?
   mul         r1, r1, c7;
//    cosDist *= filter;
   mul         r1, r1, r11;
//
// accumCos = (0, 0, 0, 0);
   mov         r7, c16.xxxx;
//    temp = dp4( cosDist, toCenter_X );
//    accumCos.x += temp.xxxx; (but accumCos = (0,0,0,0)
   dp4         r7.x, r1, -c8
//
//    temp = dp4( cosDist, toCenter_Y );
//    accumCos.y += temp.xxxx;
   dp4         r7.y, r1, -c9
//
// }
//
// accumBin = (1, 0, -accumCos.x);
// accumTan = (0, 1, -accumCos.y);
// accumNorm = (accumCos.x, accumCos.y, 1);
   mov         r11, c16.xxzx;
   add         r11, r11, r7;
   dp3         r10.x, r11, r11;
   rsq         r10.x, r10.x;
   mul         r11, r11, r10.xxxx;

//
// // Scrunch in based on computed (normalized) normal
// temp = mul( accumNorm, kNegScrunchScale ); // kNegScrunchScale = (-scrunchScale, -scrunchScale, 0, 0);
// accumPos += temp;
   dp3			r10.x, r11, c18.zxw; // winddir.x, winddir.y, 0, 0
   // r10.x tells us whether our normal is opposed to the wind.
   // If opposed, r10.x = 0, else r10.x = 1.f;
   // We'll use this to kill the Scrunch on the back sides of waves.
   // We use it for position right here, and then again for the
   // normal just down a bit further.
   slt			r10.x, r10.x, c16.x;
   mul			r9, r10.xxxx, r11;

   mad         r6, r9, c12.yyzz, r6;

//   mul			r6.z, r6.z, r10.xxxx; DEBUG

//   mad         r6, r11, c12.yyzz, r6;

// accumNorm = mul (accumNorm, kScrunchScale ); // kScrunchScale = (scrunchScale, scrunchScale, 1, 1);
   // accumCos *= (scrunchScale, scrunchScale, 0, 0);

   mul			r2.x, r6.z, c12.x;
   mul			r2.x, r2.x, r10.x; // ???
   add			r2.x, r2.x, c16.z;

//   mul         r7, r7, c12.xxzz;
   mul			r7.xy, r7.xy, r2.xx;

// This is actually wrong, but useful right now for visualizing the generated coords.
// See below for correct version.

   sub			r3, c16.xxzx, r7.xyzz;

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

   add			r1, c16.zxxx, r7.zzxz;
   add			r2, c16.xzxx, r7.zzyz;

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
   sub			r0, r6, c17;
   dp3			r10.x, r0, r0;
   rsq			r10.x, r10.x;
   mul			r0, r0, r10.xxxx;

   dp3			r10.x, r0, c19;
   mad			r10.y, r10.x, r10.x, -c19.w;

   rsq			r9.x, r10.y;

   mad			r10.z, r10.y, r9.x, r10.x;

   mad			r0.xyz, r0, r10.zzz, -c19.xyz;

   mov			r1.w, -r0.x;
   mov			r2.w, -r0.y;
   mov			r3.w, -r0.z;

   // Now rotate our basis vectors into the wind
	dp3		r0.x, r1, c18.xyww;
	dp3		r0.y, r1, c18.zxww;
	mov		r1.xy, r0;

	dp3		r0.x, r2, c18.xyww;
	dp3		r0.y, r2, c18.zxww;
	mov		r2.xy, r0;

	dp3		r0.x, r3, c18.xyww;
	dp3		r0.y, r3, c18.zxww;
	mov		r3.xy, r0;

   mov			r0.w, c16.zzzz;

   dp3         r0.x, r1, r1;
   rsq         r0.x, r0.x;
   mul         oT1, r1.xyzw, r0.xxxw;
//   mul			r8, r1.xyzw, r0.xxxw; // VISUAL

   dp3         r0.x, r2, r2;
   rsq         r0.x, r0.x;
   mul         oT3, r2.xyzw, r0.xxxw;
//   mul			r9, r2.xyzw, r0.xxxw; // VISUAL

   dp3         r0.x, r3, r3;
   rsq         r0.x, r0.x;
   mul         oT2, r3.xyzw, r0.xxxw;
//   mul			r9, r3.xyzw, r0.xxxw; // VISUAL

//	mul		   r3, r3.xzyw, r0.xxxw;
//	mul			r3.xy, r3, -c16.zzzz;

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
   m4x4     oPos, r6, c0;

// Still need to attenuate based on position
   mov         oD0, c4;

// This should be in local space after xforming v0
   dp4			r0.x, v0, c10;
   dp4			r0.y, v0, c11;
   mov			r0.zw, c16.xxxz;
   mov			oT0, r0
//   mov			oT0, v7;

// Questionble attenuation follows
	// Find vector from this point to camera and normalize
	sub			r0, c17, r6;
	dp3			r1.x, r0, r0;
    rsq			r1.x, r1.x;
	mul			r0, r0, r1.xxxx;
	// Dot that with the computed normal
	dp3			r1.x, r0, r11;
//	dp3			r1.x, r0, r3; // if you want the adjusted normal, you'll need to normalize/swizzle r3
	// Map dot=1 => 0, dot=0 => 1
	sub			r1.xyzw, c16.zzzz, r1.xxxx;
	add			r1.w, r1.wwww, c16.zzzz;
	mul			r1.w, r1.wwww, c16.yyyy;
	// No need to clamp, since the destination register (in the pixel shader)
	// will saturate [0..1] anyway.
	mul			oD1, r1, c20;
//	mov			oD1, r9;
//	mov			oD1, r8.xzyw;
