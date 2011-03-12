vs.1.1

// Grass shader. Moves verts according sine waves seeded by position
// Based on the article "Animated Grass with Pixel and Vertex Shaders"
// by John Isidoro and Drew Card, in the book
// "Direct3D ShaderX Vertex and Pixel Shader Tips and Tricks"

// c0 = Local2NDC
// c4 = (0.0, 0.5, 1.0, 2.0)
// c5 = (time, X, X, X)
// c6 = Pi constants
// c7 = Sin constants (-1/3!, 1/!5, -1/7!, 1/9!)
// c8 = waveDistortX
// c9 = waveDistortY
// c10 = waveDistortZ
// c11 = waveDirX (0.25, 0.0, -0.7, -0.8)
// c12 = waveDirY (0.0, 0.15, -0.7, 0.1)
// c13 = waveSpeed (0.2, 0.15, 0.4, 0.4)

dcl_position v0
dcl_color v5
dcl_texcoord0 v7

mul r0, c11, v0.x		// pos X,Y input to waves
mad r0, c12, v0.y, r0

mov r1, c5.x			// time
mad r0, r1, c13, r0		// scale by speed and add to X,Y input
frc r0.xy, r0
frc r1.xy, r0.zwzw
mov r0.zw, r1.xyxy

sub r0, r0, c4.y		// - 0.5
mul r1, r0, c6.w		// *= 2 pi

mul r2, r1, r1			// ^2
mul r3, r2, r1			// ^3
mul r5, r3, r2			// ^5
mul r7, r5, r2			// ^7
mul r9, r7, r2			// ^9

mad r0, r3, c7.x, r1	// - r1^3 / 3!
mad r0, r5, c7.y, r0	// + r1^5 / 5!
mad r0, r7, c7.z, r0	// - r1^7 / 7!
mad r0, r9, c7.w, r0	// + r1^9 / 9!

dp4 r3.x, r0, c8
dp4 r3.y, r0, c9
dp4 r3.zw, r0, c10

sub r4, c4.z, v7.y
mul r3, r3, r4		// mult by Y tex coord. So the waves only affect the top verts
mov r2.w, v0			//
add r2.xyz, r3, v0		// add offset to position

m4x4 oPos, r2, c0		// trans to NDC

mov oFog, c4.z		// no fog
mov oD0, v5
mov oT0, v7
