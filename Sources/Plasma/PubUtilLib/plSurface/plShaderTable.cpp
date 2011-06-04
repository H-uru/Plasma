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

#include "hsTypes.h"
#include "plShaderTable.h"

#include "plShader.h"

using namespace plShaderID;

///////////////////////////////////////////////////////////////////////
// Includes for compiled shaders
///////////////////////////////////////////////////////////////////////
#include "vs_WaveFixedFin6.h"
#include "ps_WaveFixed.h"
#include "vs_CompCosines.h"
#include "ps_CompCosines.h"
#include "vs_ShoreLeave6.h"
#include "ps_ShoreLeave6.h"
#include "vs_WaveRip.h"
#include "ps_WaveRip.h"
#include "vs_WaveDec1Lay.h"
#include "vs_WaveDec2Lay11.h"
#include "vs_WaveDec2Lay12.h"
#include "vs_WaveDecEnv.h"
#include "ps_CbaseAbase.h"
#include "ps_CalphaAbase.h"
#include "ps_CalphaAMult.h"
#include "ps_CalphaAadd.h"
#include "ps_CaddAbase.h"
#include "ps_CaddAMult.h"
#include "ps_CaddAAdd.h"
#include "ps_CmultAbase.h"
#include "ps_CmultAMult.h"
#include "ps_CmultAAdd.h"
#include "ps_WaveDecEnv.h"
#include "vs_WaveGraph2.h"
#include "ps_WaveGraph.h"
#include "vs_WaveGridFin.h"
#include "ps_WaveGrid.h"
#include "vs_BiasNormals.h"
#include "ps_BiasNormals.h"
#include "vs_ShoreLeave7.h"
#include "vs_WaveRip7.h"
#include "ps_MoreCosines.h"
#include "vs_WaveDec1Lay_7.h"
#include "vs_WaveDec2Lay11_7.h"
#include "vs_WaveDec2Lay12_7.h"
#include "vs_WaveDecEnv_7.h"
#include "vs_WaveFixedFin7.h"
#include "vs_GrassShader.h"
#include "ps_GrassShader.h"


plShaderTableInst::plShaderTableInst()
:	fFlags(0)
{
}

plShaderTableInst::~plShaderTableInst()
{
}

void plShaderTableInst::Register(const plShaderDecl* decl)
{
	hsAssert(decl->GetID() && (decl->GetID() < plShaderID::kNumShaders), "Unexpected registration");
	fTable[decl->GetID()] = decl;
}

plShaderTableInst& plShaderTable::IMakeInstance()
{
	static plShaderTableInst inst;
	fInst = &inst;

	return *fInst;
}


plShaderTableInst* plShaderTable::fInst;

