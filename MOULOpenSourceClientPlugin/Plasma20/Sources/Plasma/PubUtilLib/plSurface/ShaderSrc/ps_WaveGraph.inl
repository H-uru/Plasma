
ps.1.1

// Have a couple extra textures to burn here. Only thing
// I've thought of is to have an additional texture to
// make the front of the wave solid. So it's UVW would be
// the same as the base texture, but the texture itself would
// be just a thin horizontal band of alpha. Then just add that
// alpha to the output alpha.
//
// Let's get the first cut running first.

tex		t0;
tex		t1;
tex		t2;

//mul		r0, v0, t0;
//mul		r0, r0, t1;
//add		r0.a, r0, t2;

// 1.0 mov		r0, t0;
// 1.0 mul		r0, r0, t1;
mul		r0, t0, t1;
// TEST add		r0.a, r0, t2; // TEST
add		r0, r0, t2; // TEST
mul		r0, r0, v0;

//mul		r0.rgb, r0, r0.a; // TEST

//mov r0, t1;
