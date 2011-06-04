vs.1.1

dcl_position v0
dcl_texcoord0 v7

// Take in a screen space position,
// transform the UVW,
// and spit it out.
// c4 = (0,0.5,1.0,2.0)

//mov r0, v0;
//mov r0.w, c4.zzzz;
//mov oPos, r0;
mov oPos, v0;

dp4 r0.x, v7, c0;
mov r0.yzw, c4.xxxz; // yzw will stay constant (0,0,1);

mov oT0, r0;

dp4 r0.x, v7, c1;

mov oT1, r0;

dp4 r0.x, v7, c2;

mov oT2, r0;

dp4 r0.x, v7, c3;

mov oT3, r0;
