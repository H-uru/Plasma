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
#include "plBumpMtl.h"
#include "plPassBaseParamIDs.h"

#include "plPassAnimDlgProc.h"
#include "plAnimStealthNode.h"

#include "MaxMain/MaxCompat.h"

using namespace plPassBaseParamIDs;

static ParamBlockDesc2 gBumpAnimPB
(
    plBumpMtl::kBlkAnim, _T("anim"), IDS_PASS_ANIM, GetBumpMtlDesc(),//nullptr,
    P_AUTO_CONSTRUCT + P_AUTO_UI + P_CALLSETS_ON_LOAD, plBumpMtl::kRefAnim,

    // UI
    IDD_PASS_ANIM, IDS_PASS_ANIM, 0, 0, &plPassAnimDlgProc::Get(),

#ifdef MCN_UPGRADE_OLD_ANIM_BLOCKS
    // THE FOLLOWING ARE ALL OLD PARAMETERS AND SHOULD NO LONGER BE USED. The only reason
    // they're here is so we can convert old paramBlocks into the new plAnimStealthNode format
    kPBAnimName,        _T("animName"),     TYPE_STRING,        0, 0,
        p_end,

    kPBAnimAutoStart,   _T("autoStart"),    TYPE_BOOL,          0, 0,
        p_end,

    kPBAnimLoop,        _T("loop"),         TYPE_BOOL,          0, 0,
        p_end,
    kPBAnimLoopName,    _T("loopName"),     TYPE_STRING,        0, 0,
        p_end,

    // Anim Ease
    kPBAnimEaseInType,  _T("easeInType"),   TYPE_INT,       0, 0,
        p_end,
    kPBAnimEaseInLength,    _T("easeInLength"), TYPE_FLOAT,     0, 0,   
        p_end,
    kPBAnimEaseInMin,       _T("easeInMin"),    TYPE_FLOAT,     0, 0,   
        p_end,
    kPBAnimEaseInMax,   _T("easeInMax"),    TYPE_FLOAT,     0, 0,   
        p_end,

    kPBAnimEaseOutType, _T("easeOutType"),  TYPE_INT,       0, 0,
        p_end,
    kPBAnimEaseOutLength,   _T("easeOutLength"),    TYPE_FLOAT,     0, 0,   
        p_end,
    kPBAnimEaseOutMin,      _T("easeOutMin"),   TYPE_FLOAT,     0, 0,   
        p_end,
    kPBAnimEaseOutMax,  _T("easeOutMax"),   TYPE_FLOAT,     0, 0,   
        p_end,

#endif // MCN_UPGRADE_OLD_ANIM_BLOCKS

    kPBAnimUseGlobal,       _T("UseGlobal"),    TYPE_BOOL,  0, 0,
        p_default,  FALSE,
        p_ui,   TYPE_SINGLECHEKBOX, IDC_MTL_USE_GLOBAL,
        p_end,
    kPBAnimGlobalName,  _T("GlobalName"),   TYPE_STRING,    0,  0,
        p_default, _T(""),
        p_end,
        
    kPBAnimStealthNodes, _T( "testing" ), TYPE_REFTARG_TAB, 0, 0, 0,
        p_accessor, &plStealthNodeAccessor::GetInstance(),
        p_end,

    p_end
);
ParamBlockDesc2 *GetBumpAnimPB() { return &gBumpAnimPB; }
