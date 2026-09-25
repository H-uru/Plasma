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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#ifndef plShaderID_inc
#define plShaderID_inc

// When adding to the compiled table, make sure
// you add the include in plShaderTable.cpp, or you'll
// compile fine but have a nil shader (FFP) at runtime.
namespace plShaderID
{
    enum ID
    {
        Unregistered = 0,
        vs_WaveFixedFin6,   //OBSOLETE
        ps_WaveFixed,
        vs_CompCosines,
        ps_CompCosines,     //OBSOLETE
        vs_ShoreLeave6,     //OBSOLETE
        ps_ShoreLeave6,
        vs_WaveRip,         //OBSOLETE
        ps_WaveRip,
        vs_WaveDec1Lay,     //OBSOLETE
        vs_WaveDec2Lay11,   //OBSOLETE
        vs_WaveDec2Lay12,   //OBSOLETE
        vs_WaveDecEnv,      //OBSOLETE
        ps_CbaseAbase,
        ps_CalphaAbase,
        ps_CalphaAMult,
        ps_CalphaAadd,
        ps_CaddAbase,
        ps_CaddAMult,
        ps_CaddAAdd,
        ps_CmultAbase,
        ps_CmultAMult,
        ps_CmultAAdd,
        ps_WaveDecEnv,
        vs_WaveGraph2,
        ps_WaveGraph,
        vs_WaveGridFin,     //OBSOLETE
        ps_WaveGrid,        //OBSOLETE
        vs_BiasNormals,
        ps_BiasNormals,
        vs_ShoreLeave7,
        vs_WaveRip7,
        ps_MoreCosines,
        vs_WaveDec1Lay_7,
        vs_WaveDec2Lay11_7,
        vs_WaveDec2Lay12_7,
        vs_WaveDecEnv_7,
        vs_WaveFixedFin7,
        vs_GrassShader,
        ps_GrassShader,

        kNumShaders
    };
};

#endif // plShaderID_inc
