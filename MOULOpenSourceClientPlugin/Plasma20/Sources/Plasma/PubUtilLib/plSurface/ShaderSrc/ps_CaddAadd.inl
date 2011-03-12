
ps.1.1

// Add blend color, output sum of alpha

// Color is t0 + t1
// Alpha is t0.a + t1.a

tex		t0;
tex		t1;

add		r0.rgb, t0, t1;
+add		r0.a, t0, t1;
mul		r0, r0, v0;
