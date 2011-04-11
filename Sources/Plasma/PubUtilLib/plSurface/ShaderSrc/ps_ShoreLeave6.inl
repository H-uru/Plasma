
ps.1.1

def c0, 1.0, 1.0, 1.0, 1.0       // Temp Hack

tex		t0;
tex		t1;
tex		t2;

mov		r1.a, t1;
lrp		r0.rgb, r1.a, t1, t0;
+mul	r0.a, 1-t1, 1-t0;
lrp		r0.rgb, t2.a, t2, r0;
+mul	r0.a, 1-t2, r0;
mul		r0.rgb, r0, v0;
+mul		r0.a, 1-r0, v0;

//mov		r0.a, c1;

//mov r0.rgb, t2;
//+mov r0.a, 1-t2;
