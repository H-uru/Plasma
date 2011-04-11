

vs.1.1

dcl_position v0
dcl_texcoord0 v7

// Take in a screen space position,
// transform the UVW,
// and spit it out.
// c0 = uvXform0[0]
// c1 = uvXform0[1]
// c2 = uvXform1[0]
// c3 = uvXform1[1]
// c4 = (0,0.5,1.0,2.0)
// c5 = (noiseScale, bias, 0, 1)

mov oPos, v0;

mov r0.zw, c4.xxxz; // yzw will stay constant (0,0,1);

dp4 r0.x, v7, c0;
dp4 r0.y, v7, c1;

mov oT0, r0;

dp4 r0.x, v7, c2;
dp4 r0.y, v7, c3;

mov oT1, r0;

mov oD0, c5.xxzz;
mov oD1, c5.yyzz;

