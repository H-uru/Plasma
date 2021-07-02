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

#include "HeadSpin.h"
#include "MaxAPI.h"

#include "pnKeyedObject/plKey.h"

#include "resource.h"

#include <set>

#include "plMaxUtils.h"
#include "plResMgr/plPageInfo.h"
#include "pnKeyedObject/plUoid.h"
#include "pnFactory/plFactory.h"

class MaxUtilsClassDesc : public plMaxClassDesc<ClassDesc>
{
public:
    int             IsPublic() override             { return TRUE; }
    void*           Create(BOOL loading) override   { return &plMaxUtils::Instance(); }
    const MCHAR*    ClassName() override            { return _M("Plasma Debug Utils"); }
    SClass_ID       SuperClassID() override         { return UTILITY_CLASS_ID; }
    Class_ID        ClassID() override              { return Class_ID(0x316610ee, 0xebe62c3); }
    const MCHAR*    Category() override             { return _M(""); }
};

static MaxUtilsClassDesc theMaxUtilsClassDesc;
ClassDesc* GetMaxUtilsDesc() { return &theMaxUtilsClassDesc; }

plMaxUtils::plMaxUtils() : fhPanel(), fhResDlg()
{
}

plMaxUtils& plMaxUtils::Instance()
{
    static plMaxUtils theInstance;
    return theInstance;
}

void plMaxUtils::BeginEditParams(Interface *ip, IUtil *iu)
{
    fhPanel = GetCOREInterface()->AddRollupPage(hInstance,
                                                MAKEINTRESOURCE(IDD_UTILS),
                                                ForwardDlgProc,
                                                _M("Plasma Debug Utils"));
}

void plMaxUtils::EndEditParams(Interface *ip, IUtil *iu)
{
    GetCOREInterface()->DeleteRollupPage(fhPanel);
}

INT_PTR CALLBACK plMaxUtils::ForwardDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Instance().DlgProc(hDlg, msg, wParam, lParam);
}

int ClearTextureIds();

INT_PTR plMaxUtils::DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_RES)
        {
            int numCleared = ClearTextureIds();
            TCHAR buf[256];
            _sntprintf(buf, std::size(buf), _T("Cleared %d texture ids"), numCleared);
            plMaxMessageBox(nullptr, buf, _T("AssetMan Clear"), MB_OK);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

#include "plMtlCollector.h"
#include "MaxPlasmaMtls/Layers/plPlasmaMAXLayer.h"
#ifdef MAXASS_AVAILABLE
#   include "../../AssetMan/PublicInterface/AssManTypes.h"
#endif

int ClearTextureIds()
{
#ifdef MAXASS_AVAILABLE
    int numCleared = 0;

    TexSet texmaps;
    plMtlCollector::GetMtls(nullptr, &texmaps);

    TexSet::iterator texIt = texmaps.begin();
    for (; texIt != texmaps.end(); texIt++)
    {
        Texmap* texmap = (*texIt);

        plPlasmaMAXLayer* layer = plPlasmaMAXLayer::GetPlasmaMAXLayer(texmap);
        if (layer)
        {
            int numBitmaps = layer->GetNumBitmaps();
            for (int i = 0; i < numBitmaps; i++)
            {
                jvUniqueId assetId;
                layer->GetBitmapAssetId(assetId, i);
                if (!assetId.IsEmpty())
                {
                    assetId.SetEmpty();
                    layer->SetBitmapAssetId(assetId, i);
                    numCleared++;
                }
            }
        }
    }

    return numCleared;
#else
    return 0;
#endif
}
