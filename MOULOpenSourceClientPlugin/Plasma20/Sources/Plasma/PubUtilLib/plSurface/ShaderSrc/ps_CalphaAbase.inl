
ps.1.1

// Alpha blend layers, output base alpha
//
// Color is t0 * (1 - t1.a) + t1 * t1.a
// Alpha is t0.a

tex		t0
tex		t1

lrp		r0.rgb, t1.a, t1, t0
mov		r0.a, t0;
mul		r0, r0, v0;
