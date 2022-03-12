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
#include "plCmdParser.h"
#include "plProduct.h"
#include "hsSystemInfo.h"
#include "hsWindows.h"

#include <algorithm>
#include <string_view>
#include <type_traits>

#include <CommCtrl.h>
#include <shellapi.h>
#include <Vsstyle.h>
#include <vssym32.h>
#include <Uxtheme.h>

#include "resource.h"

#include "plClipboard/plClipboard.h"

#include "pfCrashHandler/plCrashSrv.h"

// ===================================================

static std::vector<plStackEntry> s_stackTrace;

// ===================================================

static inline ST::string IBuildCrashString()
{
    ST::string_stream ss;
    ss << plProduct::ProductString() << '\n';
    ss << hsSystemInfo::AsString() << '\n';
    ss << "\nStack Trace:\n";
    for (size_t i = 0; i < s_stackTrace.size(); ++i) {
        const plStackEntry& entry = s_stackTrace[i];

        ss << ST::format("Level {02}: ", i);
        if (!entry.fModuleName.empty())
            ss << entry.fModuleName << '!';
        ss << entry.fFunctionName;
        if (entry.fFileName.IsValid()) {
            ss << " (File: " << entry.fFileName.GetFileName();
            if (entry.fLine.has_value())
                ss << ':' << entry.fLine.value();
            ss << ')';
        }
        if (i + 1 < s_stackTrace.size())
            ss << '\n';
    }
    return ss.to_string();
}

// ===================================================

enum class RenderMode { kColor, kMonochrome };

template<RenderMode _RenderMode>
HBITMAP IRenderSymbol(HWND hwnd, HDC hDC, HFONT font, COLORREF color, wchar_t symbol)
{
    HDC buffDC = CreateCompatibleDC(hDC);
    HBITMAP buffBitmap;
    if constexpr (_RenderMode == RenderMode::kColor)
        buffBitmap = CreateCompatibleBitmap(hDC, 32, 32);
    else
        buffBitmap = CreateCompatibleBitmap(buffDC, 32, 32);
    SelectObject(buffDC, buffBitmap);
    SelectObject(buffDC, font);
    SetBkMode(buffDC, OPAQUE);
    SetBkColor(buffDC, RGB(255, 255, 255));
    FloodFill(buffDC, 0, 0, RGB(255, 255, 255));
    SetTextColor(buffDC, color);
    TextOutW(buffDC, 0, 0, &symbol, 1);

    DeleteObject(buffDC);
    return buffBitmap;
}

static inline void ISetCommandLinkSymbols(HWND hwnd, HDC hDC, HFONT font, HTHEME theme, std::tuple<int, wchar_t>&& def)
{
    HBITMAP mask = IRenderSymbol<RenderMode::kMonochrome>(hwnd, hDC, font, RGB(0, 0, 0), std::get<1>(def));

    BUTTON_IMAGELIST imgList{};
    imgList.margin = { 0, 0, 24, 24 };
    Button_GetImageList(GetDlgItem(hwnd, std::get<0>(def)), &imgList);
    if (imgList.himl == nullptr)
        imgList.himl = ImageList_Create(24, 24, ILC_COLOR24 | ILC_MASK, 0, 0);

    if (theme) {
        // Matches the offsets of PUSHBUTTONSTATES-1, so we can just insert directly into the himl.
        constexpr int images[] {
            CMDLS_NORMAL,
            CMDLS_HOT,
            CMDLS_PRESSED,
            CMDLS_DISABLED,
            CMDLS_DEFAULTED,
        };

        ImageList_SetImageCount(imgList.himl, std::size(images));

        // Render one icon for each button state
        for (size_t i = 0; i < std::size(images); ++i) {
            COLORREF color;
            if (FAILED(GetThemeColor(theme, BP_COMMANDLINK, images[i], TMT_TEXTCOLOR, &color)))
                color = RGB(0, 0, 0);
            HBITMAP icon = IRenderSymbol<RenderMode::kColor>(hwnd, hDC, font, color, std::get<1>(def));
            ImageList_Replace(imgList.himl, i, icon, mask);
            DeleteObject(icon);
        }
    } else {
        COLORREF color = GetSysColor(COLOR_HIGHLIGHT);
        HBITMAP icon = IRenderSymbol<RenderMode::kColor>(hwnd, hDC, font, color, std::get<1>(def));
        ImageList_SetImageCount(imgList.himl, 1);
        ImageList_Replace(imgList.himl, 0, icon, mask);
        DeleteObject(icon);
    }

    Button_SetImageList(GetDlgItem(hwnd, std::get<0>(def)), &imgList);
    DeleteObject(mask);
}

template<typename _ArgT, typename... _ArgsT>
inline void ISetCommandLinkSymbols(HWND hwnd, HDC hDC, HTHEME theme, HFONT font, _ArgT&& arg, _ArgsT&&... args)
{
    static_assert(std::is_same_v<std::tuple<int, wchar_t>, std::decay_t<_ArgT>>, "ISetCommandLinkSymbols expects std::tuple<int, wchar_t>");
    ISetCommandLinkSymbols(hwnd, hDC, font, theme, std::forward<_ArgT>(arg));
    if constexpr (sizeof...(args) > 0)
        ISetCommandLinkSymbols(hwnd, hDC, font, theme, std::forward<_ArgsT>(args)...);
}

template<typename... _ArgsT>
void ISetCommandLinkSymbols(HWND hwnd, _ArgsT&&... args)
{
    // While the Segoe UI Symbol font does, in fact, exist on Windows 7, it lacks
    // the actually useful symbols from Windows 8 that we need. So just bail.
    const RTL_OSVERSIONINFOEXW& winver = hsGetWindowsVersion();
    if (winver.dwMajorVersion < 6 || (winver.dwMajorVersion == 6 && winver.dwMinorVersion < 3))
        return;

    HDC hDC = GetDC(hwnd);
    HFONT font = CreateFontW(
        -MulDiv(14, GetDeviceCaps(hDC, LOGPIXELSY), 72), // cHeight
        0,                                               // cWidth
        0,                                               // cEscapement
        0,                                               // cOrientation
        FW_NORMAL,                                       // cWeight
        FALSE,                                           // bItalic
        FALSE,                                           // bUnderline
        FALSE,                                           // bStrikeOut
        DEFAULT_CHARSET,                                 // iCharSet
        OUT_DEFAULT_PRECIS,                              // iOutPrecision
        CLIP_DEFAULT_PRECIS,                             // iClipPrecision
        CLEARTYPE_QUALITY,                               // iQuality
        DEFAULT_PITCH,                                   // iPitchAndFamily
        L"Segoe UI Symbol"                               // pszFaceName
    );

    if (font) {
        HTHEME theme = OpenThemeData(hwnd, L"BUTTON");
        ISetCommandLinkSymbols(hwnd, hDC, theme, font, std::forward<_ArgsT>(args)...);
        if (theme)
            CloseThemeData(theme);
        DeleteObject(font);
    }

    ReleaseDC(hwnd, hDC);
}

// ===================================================

static inline void IAddColumn(HWND hwnd, int idx, std::wstring_view name, int size)
{
    LVCOLUMNW column{};
    column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    column.fmt = LVCFMT_LEFT;
    column.cx = size;
    column.pszText = const_cast<wchar_t*>(name.data());
    column.cchTextMax = name.size();
    column.iSubItem = idx;
    ListView_InsertColumn(hwnd, idx, &column);
}

static inline void ISetItem(HWND hwnd, int idx, int col, const ST::string& value)
{
    ST::wchar_buffer text = value.to_wchar();
    LV_ITEMW item{};
    item.mask = LVIF_DI_SETITEM | LVIF_TEXT;
    item.iItem = idx;
    item.iSubItem = col;
    item.pszText = text.data();
    item.cchTextMax = text.size();
    // Per MSDN: "You cannot use ListView_InsertItem or LVM_INSERTITEM to insert subitems."
    if (col == 0)
        ListView_InsertItem(hwnd, &item);
    else
        ListView_SetItem(hwnd, &item);
}

// ===================================================

static inline void ILayoutStackTrace(HWND hwnd)
{
    enum { kLevel, kFunction, kFile, kLine };
    bool haveFiles = std::any_of(s_stackTrace.cbegin(), s_stackTrace.cend(), [](const auto& i) { return i.fFileName.IsValid(); });
    bool haveLines = std::any_of(s_stackTrace.cbegin(), s_stackTrace.cend(), [](const auto& i) { return i.fLine.has_value(); });
    bool haveOffsets = !haveLines && std::any_of(s_stackTrace.cbegin(), s_stackTrace.cend(), [](const auto& i) { return i.fOffset != 0; });

    constexpr int kLevelColSize = 50;
    constexpr int kSymbolColSize = 220;
    constexpr int kFileColSize = 90;
    constexpr int kLineColSize = 50;
    constexpr int kOffsetColSize = 90;

    HWND list = GetDlgItem(hwnd, IDC_CRASHLIST);
    IAddColumn(list, kLevel, L"Level", kLevelColSize);
    int symcol = kSymbolColSize;
    if (!haveFiles)
        symcol += kFileColSize;
    if (haveOffsets)
        symcol -= (kOffsetColSize - kLineColSize);
    IAddColumn(list, kFunction, L"Symbol Name", symcol);
    if (haveFiles)
        IAddColumn(list, kFile, L"File", kFileColSize);
    if (haveLines)
        IAddColumn(list, kLine, L"Line", kLineColSize);
    else if (haveOffsets)
        IAddColumn(list, haveFiles ? kLine : kFile, L"Offset", kOffsetColSize);

    ListView_SetItemCount(list, s_stackTrace.size());
    for (size_t i = 0; i < s_stackTrace.size(); ++i) {
        const plStackEntry& entry = s_stackTrace[i];

        ISetItem(list, i, kLevel, ST::string::from_uint64(i));
        ST::string symbol = (!entry.fModuleName.empty() ?
            ST::format("{}!{}", entry.fModuleName, entry.fFunctionName) :
            entry.fFunctionName
        );
        ISetItem(list, i, kFunction, symbol);
        if (entry.fFileName.IsValid())
            ISetItem(list, i, kFile, entry.fFileName.GetFileName());
        if (entry.fLine.has_value())
            ISetItem(list, i, kLine, ST::string::from_uint(entry.fLine.value()));
        else if (haveOffsets)
            ISetItem(list, i, haveFiles ? kLine : kFile, ST::format("+{08X}", entry.fOffset));
    }
}

static inline void IDrawCommandLinkIcons(HWND hwnd)
{
    ISetCommandLinkSymbols(
        hwnd,
        std::make_tuple(IDC_COPYBUTTON, L'\uE16F'),
        std::make_tuple(IDC_QUITBUTTON, L'\uE126')
    );
}

static void ILayoutDialog(HWND dialog)
{
    SetDlgItemTextW(dialog, IDC_PRODUCTSTRING, plProduct::ProductString().to_wchar().data());
    ILayoutStackTrace(dialog);
    IDrawCommandLinkIcons(dialog);
}

INT_PTR CALLBACK CrashDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        ILayoutDialog(hwndDlg);
        break;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam)) {
            case IDC_QUITBUTTON:
                PostQuitMessage(0);
                break;
            case IDC_COPYBUTTON:
                // Build the crash string so that when pasting into markup-like environments,
                // eg Discord or GitHub, the entire trace will be monospaced and surrounded
                // by a box.
                plClipboard::GetInstance().SetClipboardText(ST::format("```\n{}\n```", IBuildCrashString()));
                break;
            }
        }
        break;
    case WM_NCHITTEST:
        SetWindowLongPtrW(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)HTCAPTION);
        return TRUE;
    case WM_THEMECHANGED:
        // re-render the button icons for great justice
        IDrawCommandLinkIcons(hwndDlg);
        break;
    }

    return DefWindowProcW(hwndDlg, uMsg, wParam, lParam);
}


static void IShowCrashDialog(HINSTANCE hInstance)
{
    HWND dialog = CreateDialogW(hInstance, MAKEINTRESOURCEW(IDD_DIALOG), nullptr, CrashDialogProc);
    BringWindowToTop(dialog);

    FLASHWINFO flash{};
    flash.cbSize = sizeof(flash);
    flash.hwnd = dialog;
    flash.dwFlags = FLASHW_TRAY;
    flash.uCount = 5;
    FlashWindowEx(&flash);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessageW(dialog, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
}

// ===================================================

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    enum { kArgCrashFile, kArgForceShow };
    const plCmdArgDef cmdLineArgs[] = {
        { kCmdArgRequired | kCmdTypeString, "file", kArgCrashFile },
        { kCmdArgFlagged | kCmdTypeBool, "force", kArgForceShow },
    };

    // Don't use pCmdLine because plCmdParser doesn't like it.
    std::vector<ST::string> args;
    args.reserve(__argc);
    for (size_t i = 0; i < __argc; i++)
        args.push_back(ST::string::from_wchar(__wargv[i]));

    plCmdParser parser(cmdLineArgs, std::size(cmdLineArgs));
    if (!parser.Parse(args)) {
        if (parser.GetBool(kArgForceShow)) {
            IShowCrashDialog(hInstance);
            return 0;
        } else {
            hsMessageBox("You should never run this manually.", "Error", hsMessageBoxNormal, hsMessageBoxIconExclamation);
            return 1;
        }
    }

    plCrashSrv srv(parser.GetString(kArgCrashFile));
    plCrashResult result;
    std::tie(result, s_stackTrace) = srv.HandleCrash();
    if (result == plCrashResult::kClientCrashed)
        IShowCrashDialog(hInstance);

    return 0;
}
