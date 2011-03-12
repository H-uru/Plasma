/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plWaveSetShaderConsts_inc
#define plWaveSetShaderConsts_inc

// Notice there can be skips for multi-vector consts (e.g. matrices).
namespace plGridVS
{
	enum {
		kLocalToNDC					= 0,
		kProjRow0						= 0,
		kProjRow1						= 1,
		kProjRow2						= 2,
		kProjRow3						= 3,
		kWaterTint					= 4,
		kFrequency					= 5,
		kPhase						= 6,
		kAmplitude					= 7,
		kDirectionX					= 8,
		kDirectionY					= 9,
		kUTransform					= 10,
		kVTransform					= 11,
		kScrunch					= 12,
		kSinConsts					= 13,
		kCosConsts					= 14,
		kPiConsts					= 15,
		kNumericConsts				= 16,
		kCameraPos					= 17,
		kWindRot					= 18,
		kEnvAdjust					= 19,
		kEnvTint					= 20,

		kNumConsts
	};
};

namespace plFixedVS
{
	enum {
		kWorldToNDC					= 0,
		kProjRow0						= 0,
		kProjRow1						= 1,
		kProjRow2						= 2,
		kProjRow3						= 3,
		kWaterTint					= 4,
		kFrequency					= 5,
		kPhase						= 6,
		kAmplitude					= 7,
		kDirectionX					= 8,
		kDirectionY					= 9,
		kUTransform					= 10,
		kVTransform					= 11,
		kScrunch					= 12,
		kSinConsts					= 13,
		kCosConsts					= 14,
		kPiConsts					= 15,
		kNumericConsts				= 16,
		kCameraPos					= 17,
		kWindRot					= 18,
		kEnvAdjust					= 19,
		kEnvTint					= 20,
		kLocalToWorld				= 21,
		kL2WRow0						= 21,
		kL2WRow1						= 22,
		kL2WRow2						= 23,
		kLengths					= 24,
		kWaterLevel					= 25,
		kDepthFalloff				= 26,
		kMinAtten					= 27,
		kFogSet						= 28,

		kNumConsts
	};
};

namespace plFixedVS7
{
	enum {
		kWorldToNDC					= 0,
		kProjRow0						= 0,
		kProjRow1						= 1,
		kProjRow2						= 2,
		kProjRow3						= 3,
		kWaterTint					= 4,
		kFrequency					= 5,
		kPhase						= 6,
		kAmplitude					= 7,
		kDirectionX					= 8,
		kDirectionY					= 9,
		kUVScale					= 10,
		kSpecAtten					= 11,
		kScrunch					= 12, // UNUSED
		kSinConsts					= 13,
		kCosConsts					= 14,
		kPiConsts					= 15,
		kNumericConsts				= 16,
		kCameraPos					= 17,
		kWindRot					= 18,
		kEnvAdjust					= 19,
		kEnvTint					= 20,
		kLocalToWorld				= 21,
		kL2WRow0						= 21,
		kL2WRow1						= 22,
		kL2WRow2						= 23,
		kLengths					= 24,
		kWaterLevel					= 25,
		kDepthFalloff				= 26,
		kMinAtten					= 27,
		kFogSet						= 28,
		kDirXK						= 29,
		kDirYK						= 30,
		kDirXW						= 31,
		kDirYW						= 32,
		kWK							= 33,
		kDirXSqKW					= 34,
		kDirXDirYKW					= 35,
		kDirYSqKW					= 36,

		kNumConsts
	};
};

namespace plShoreVS
{
	enum {
		kWorldToNDC					= 0,
		kProjRow0						= 0,
		kProjRow1						= 1,
		kProjRow2						= 2,
		kProjRow3						= 3,
		kShoreTint					= 4,
		kFrequency					= 5,
		kPhase						= 6,
		kAmplitude					= 7,
		kDirectionX					= 8,
		kDirectionY					= 9,
		kIncline					= 10,
		kFogSet						= 11,
		kScrunch					= 12, // UNUSED
		kSinConsts					= 13,
		kCosConsts					= 14,
		kPiConsts					= 15,
		kNumericConsts				= 16,
		kQADirX						= 17, // Q * Dir.x * A
		kQADirY						= 18, // Q * Dir.y * A
		kTex0Transform				= 19,
		kTex0_Row0						= 19,
		kTex0_Row1						= 20,
		kTex0_Row2						= 21,
		kTex1Transform				= 22, // UNUSED
		kTex1_Row0						= 22,
		kTex1_Row1						= 23,
		kTex1_Row2						= 24,
		kLocalToWorld				= 25,
		kL2WRow0						= 26,
		kL2WRow1						= 27,
		kL2WRow2						= 28,
		kLengths					= 29,
		kWaterLevel					= 30,
		kDepthFalloff				= 31,
		kMinAtten					= 32,

		kNumConsts
	};
};

namespace plGraphVS
{
	enum {
		kNumericConsts				= 0,
		kFrequency					= 1,
		kPhase						= 2,
		kAmplitude					= 3,
		kPiConsts					= 4,
		kCosConsts					= 5,
		kUVWConsts					= 6,
		kColor						= 7,

		kNumConsts
	};
};

namespace plGraphPS
{
	enum {

		kNumConsts
	};
};

namespace plFixedPS
{
	enum {
		kDir0						= 0,
		kDir1						= 1,
		kDir2						= 2,
		kDir3						= 3,

		kNumConsts
	};
};

namespace plBumpPS
{
	enum {
		kWave0						= 0,
		kWave1						= 1,
		kWave2						= 2,
		kWave3						= 3,
		kHalfOne					= 4,
		kBias						= 5,

		kNumConsts
	};
};

namespace plBumpVS
{
	enum {
		kUXform0			= 0,
		kUXform1			= 1,
		kUXform2			= 2,
		kUXform3			= 3,

		kNumbers			= 4,

		kNumConsts
	};
};

namespace plBiasPS
{
	enum {

		kNumConsts
	};
};

namespace plBiasVS
{
	enum {
		kTexU0			= 0,
		kTexV0			= 1,

		kTexU1			= 2,
		kTexV1			= 3,

		kNumbers		= 4,

		kScaleBias		= 5,

		kNumConsts
	};
};

namespace plCompPS
{
	enum {


		kNumConsts
	};
};

namespace plCompVS
{
	enum {
		kTex0Transform				= 0,
		kTex0_Row0						= 0,
		kTex0_Row1						= 1,
		kTex1Transform				= 2,
		kTex1_Row0						= 2,
		kTex1_Row1						= 3,
		kTex2Transform				= 4,
		kTex2_Row0						= 4,
		kTex2_Row1						= 5,
		kTex3Transform				= 6,
		kTex3_Row0						= 6,
		kTex3_Row1						= 7,

		kNumbers			= 8,

		kNumConsts
	};
};

namespace plRipVS // closely related to plFixedVS and plShoreVS
{
	enum {
		kWorldToNDC					= 0,
		kProjRow0						= 0,
		kProjRow1						= 1,
		kProjRow2						= 2,
		kProjRow3						= 3,
		kFogSet						= 4,
		kFrequency					= 5,
		kPhase						= 6,
		kAmplitude					= 7,
		kDirectionX					= 8,
		kDirectionY					= 9,
		kQADirX						= 10,
		kQADirY						= 11,
		kScrunch					= 12, // UNUSED
		kSinConsts					= 13,
		kCosConsts					= 14,
		kPiConsts					= 15,
		kNumericConsts				= 16,
		kCameraPos					= 17,
		kWindRot					= 18, // UNUSED
		kTex0Transform				= 19, // UNUSED
		kTex0_Row0						= 19,
		kTex0_Row1						= 20,
		kTex0_Row2						= 21,
		kTex1Transform				= 22, // UNUSED
		kTex1_Row0						= 22,
		kTex1_Row1						= 23,
		kTex1_Row2						= 24,
		kLocalToWorld				= 25,
		kL2WRow0						= 26,
		kL2WRow1						= 27,
		kL2WRow2						= 28,
		kLengths					= 29,
		kWaterLevel					= 30,
		kDepthFalloff				= 31,
		kMinAtten					= 32,
		kTexConsts					= 33,
		kLifeConsts					= 34,
		kRampBias					= 35,

		kNumConsts
	};
};

namespace plWaveDecVS // closely related to plFixedVS and plShoreVS
{
	enum {
		kWorldToNDC					= 0,
		kProjRow0						= 0,
		kProjRow1						= 1,
		kProjRow2						= 2,
		kProjRow3						= 3,
		kFrequency					= 4,
		kPhase						= 5,
		kAmplitude					= 6,
		kDirectionX					= 7,
		kDirectionY					= 8,
		kScrunch					= 9, // UNUSED
		kSinConsts					= 10,
		kCosConsts					= 11,
		kPiConsts					= 12,
		kNumericConsts				= 13,
		kTex0Transform				= 14, 
		kTex0_Row0						= 14,
		kTex0_Row1						= 15,
		kTex1Transform				= 16,
		kTex1_Row0						= 16,
		kTex1_Row1						= 17,
		kLocalToWorld				= 18,
		kL2WRow0						= 18,
		kL2WRow1						= 19,
		kL2WRow2						= 20,
		kLengths					= 21,
		kWaterLevel					= 22,
		kDepthFalloff				= 23,
		kMinAtten					= 24,
		kBias						= 25, // Only using one slot
		kMatColor					= 26,
		kCameraPos					= 27, // Only used by DecalEnv
		kEnvAdjust					= 28, // Only used by DecalEnv
		kFogSet						= 29,
		kQADirX						= 30,
		kQADirY						= 31,

		kDirXW						= 32, // Only used by DecalEnv
		kDirYW						= 33, // Only used by DecalEnv
		kWK							= 34, // Only used by DecalEnv
		kDirXSqKW					= 35, // Only used by DecalEnv
		kDirXDirYKW					= 36, // Only used by DecalEnv
		kDirYSqKW					= 37, // Only used by DecalEnv

		kNumConsts
	};
};


#endif // plWaveSetShaderConsts_inc
