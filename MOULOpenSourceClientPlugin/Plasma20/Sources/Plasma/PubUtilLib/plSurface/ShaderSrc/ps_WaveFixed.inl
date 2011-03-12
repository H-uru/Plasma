//ps.1.1

// def c0, 1.0, 0.0, 0.0, 1.0

// mov r0, c0

// Short pixel shader. Use the texm3x3vspec to do a per-pixel
// reflected lookup into our environment map.
// Input:
//    t0    - Normal map in tangent space. Apply _bx2 modifier to shift
//             [0..255] -> [-1..1]
//    t1    - UVW = tangent + eye2pos.x, map ignored.
//    t2    - UVW = binormal + eye2pos.y, map ignored
//    t3    - UVW = normal + eye2pos.z, map = environment cube map
//    v0    - attenuating color/alpha.
//    See docs on texm3x3vspec for explanation of the eye2pos wackiness.
// Output:
//    r0 = reflected lookup from environment map X input v0.
//    Since environment map has alpha = 255, the output of this
//    shader can be used for either alpha or additive blending,
//    as long as v0 is fed in appropriately.

ps.1.1

def c0, 1.0, 0.0, 0.0, 1.0       // Temp Hack
/*
def c1, 0.0, 1.0, 0.0, 1.0
def c2, 0.0, 0.0, 1.0, 1.0
*/


tex t0                  // Bind texture in stage 0 to register t0.
texm3x3pad   t1,  t0_bx2   // First row of matrix multiply.
texm3x3pad   t2,  t0_bx2   // Second row of matrix multiply.
texm3x3vspec t3,  t0_bx2   // Third row of matrix multiply to get a 3-vector.
                      // Reflect 3-vector by the eye-ray vector.
                      // Use reflected vector to do a texture lookup
                      // at stage 3.

// t3 now has our reflected environment map value
// We've (presumably) attenuated the effect on a vertex basis
// and have our color w/ attenuated alpha in v0. So all we need
// is to multiply t3 by v0 into r0, add our base color from v1 and we're done.
mad			r0.rgb, t3, v0, v1;
/* HACKAGE
//+mul			r0.a, v1, v0;
HACKAGE */
mov r0.a, v0; //HACKAGE
/*
mov	r0.rgb, v0;
mov r0.a, v0;
*/

/*
tex t0;
texcoord t1;
texcoord t2;
texcoord t3;

mov r0.rgb, t3;

+mov	r0.a, c0;
*/





/*
tex t0;
texcoord t1;
texcoord t2;
texcoord t3;

mul r0.rgb, t0_bx2, c1;
+mov r0.a, c2;
*/
