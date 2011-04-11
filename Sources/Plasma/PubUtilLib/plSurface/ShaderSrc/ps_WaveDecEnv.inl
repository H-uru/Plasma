
// Very simular to ps_WaveFixed.inl. Only the final coloring is different.
// Even though so far they are identical.

ps.1.1

//def c0, 1.0, 0.0, 0.0, 1.0       // Temp Hack


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
// is to multiply t3 by v0 into r0 and we're done.
mul			r0.rgb, t3, v0;
+mul		r0.a, t0, v0;

// mov r0, t0;

/*
tex t0;
texcoord t1;
texcoord t2;
texcoord t3;

mov	r0.rgb, t3;
+mov r0.a, c0;
*/
