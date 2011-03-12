
ps.1.1

//def c0, 1.0, 0.0, 0.0, 1.0       // Temp Hack

// Want
// Color: vert.rgb * t0.rgb
// Alpha: vert.a * t0.a * t1.a

tex t0;
//tex t1;

//mul		r0.rgb, v0, t0;
//+mul	r0.a, v0.a, t0.a;
//mul		r0.a, r0.a, t1.a;

//mul r0, t0, t1;

mul	r0, t0, v0;

//mov r0, t0;
