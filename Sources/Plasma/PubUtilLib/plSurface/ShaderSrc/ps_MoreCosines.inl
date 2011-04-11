

// Composite the cosines together.
// Input map is cosine(pix) for each of
// the 4 waves.
//
// The constants are set up so:
//		Nx = -freq * amp * dirX * cos(pix);
//		Ny = -freq * amp * dirY * cos(pix);
//	So c[i].x = -freq[i] * amp[i] * dirX[i]
//	etc.
// All textures are:
//		(r,g,b,a) = (cos(), cos(), 1, 1)
//
// Here all c[i].z = 0, because we're accumulating ontop
// of layers that have been primed with z = 1.
// Note also the c4 used for biasing back at the end.

ps.1.1

tex		t0;
tex		t1;
tex		t2;
tex		t3;

mul		r0, t0_bx2, c0;
mad		r0, t1_bx2, c1, r0;
mad		r0, t2_bx2, c2, r0;
mad		r0, t3_bx2, c3, r0;

// Now bias it back into range [0..1] for output.
mul		r0.rgb, r0, c4;
+mov	r0.a, c4;
add		r0.rgb, r0, c5;
//mov		r0, c4;
