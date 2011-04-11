
ps.1.1

// Alpha blend color, output product of alphas

// Color is t0 * (1 - t1.a) + t1 * t1.a
// Alpha is t0.a * t1.a

tex		t0
tex		t1

lrp		r0.rgb, t1.a, t1, t0
mul		r0.a, t0, t1;
mul		r0, r0, v0;
