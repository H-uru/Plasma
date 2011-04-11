

// Grab noise texture,
// modulate biased version by vtx color 0,
// add to vtx color 1

ps.1.1

tex		t0;
tex		t1;

add		r0.rgb, t0_bias, t1_bias;
+add	r0.a, t0, t1;
//mov		r0, t1_bias;
mad		r0.rgb, r0, v0, v1;
//mov r0, v1;

