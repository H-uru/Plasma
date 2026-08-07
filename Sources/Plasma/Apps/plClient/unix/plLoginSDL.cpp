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

#include "plLoginSDL.h"

#include "HeadSpin.h"
#include "hsStream.h"
#include "plFileSystem.h"
#include "plProduct.h"

#include "pnNetBase/pnNbConst.h"

#include "plGImage/plFont.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plPNG.h"

#include <SDL3/SDL.h>

#include <string_theory/format>

#include <algorithm>
#include <memory>

//
// The form is drawn at a fixed size and letterboxed into whatever the window
// actually is, so the layout below never has to think about DPI. Everything is
// in these coordinates, mouse hits included.
//
static constexpr int kWindowWidth  = 640;
static constexpr int kWindowHeight = 400;

/** The banner strip across the top; the form is drawn below it. */
static constexpr int kBannerHeight  = 84;
static constexpr int kContentTop    = kBannerHeight;
static constexpr int kContentHeight = kWindowHeight - kContentTop;

static constexpr float kLabelX     = 24.0f;
static constexpr float kFieldX     = 160.0f;
static constexpr float kFieldWidth = 456.0f;
static constexpr float kRowHeight  = 32.0f;
static constexpr float kTextInset  = 8.0f;
static constexpr float kCheckboxSize = 18.0f;
static constexpr float kLanguageRowHeight = 28.0f;
static constexpr size_t kMaxVisibleLanguages = 6;

//
// Colours taken off the banner art, so the form reads as part of the same thing
// rather than as a system dialog that happens to be sitting in front of it.
//
static constexpr uint32_t kColorBackground = 0xff0d0b09;
static constexpr uint32_t kColorField      = 0xff100d0b;
static constexpr uint32_t kColorButton     = 0xff241c15;
static constexpr uint32_t kColorButtonHot  = 0xff3a2d20;
static constexpr uint32_t kColorBorder     = 0xff6b573a;
static constexpr uint32_t kColorFocus      = 0xffd8b46a;
static constexpr uint32_t kColorText       = 0xffe8dcc0;
static constexpr uint32_t kColorDimText    = 0xff9a8866;
static constexpr uint32_t kColorError      = 0xffd2694a;

/** Where each focusable thing lives, indexed by plLoginSDL::Field. */
static constexpr SDL_FRect kFieldRects[] = {
    { kFieldX, 120.0f, kFieldWidth, kRowHeight }, // account
    { kFieldX, 164.0f, kFieldWidth, kRowHeight }, // password
    { kFieldX, 208.0f, kFieldWidth, kRowHeight }, // remember
    { kFieldX, 252.0f, kFieldWidth, kRowHeight }, // language
    {  160.0f, 320.0f,      140.0f,     40.0f }, // log in
    {  320.0f, 320.0f,      140.0f,     40.0f }, // quit
};
static_assert(std::size(kFieldRects) == plLoginSDL::kNumFields,
              "every field needs a rect");

/**
 * The faces to try for the form, best first.
 *
 * These come out of the game's own dat directory, so the login reads in the
 * same typeface as everything after it. A fresh install that has not patched
 * yet will not have them, which is what the debug-font fallback is for.
 */
static const char* const kFontFaces[] = { "Uru", "Sharper", "Atrus", "Arial" };
static constexpr int kFontSize = 16;

/** SDL's built-in font, used only when the game's own fonts are unavailable. */
static constexpr float kDebugCharWidth = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

static void ISetDrawColor(SDL_Renderer* renderer, uint32_t argb)
{
    SDL_SetRenderDrawColor(renderer, (argb >> 16) & 0xff, (argb >> 8) & 0xff,
                           argb & 0xff, (argb >> 24) & 0xff);
}

plLoginSDL::plLoginSDL()
    : fWindow(), fRenderer(), fBanner(), fSurfaceTexture(), fSurface(), fFont(),
      fRemember(), fFocus(kFieldAccount), fLanguage(plLocalization::GetLanguage()),
      fLanguageHighlight(fLanguage), fLanguageScroll(), fLanguageOpen()
{
    // Created hidden so an empty window never flashes up before Prompt() draws.
    fWindow = SDL_CreateWindow(ST::format("{} Login", plProduct::LongName()).c_str(),
                               kWindowWidth, kWindowHeight, SDL_WINDOW_HIDDEN);
    if (!fWindow) {
        hsStatusMessageF("Could not create the login window: {}", SDL_GetError());
        return;
    }

    fRenderer = SDL_CreateRenderer(fWindow, nullptr);
    if (!fRenderer) {
        hsStatusMessageF("Could not create the login renderer: {}", SDL_GetError());
        SDL_DestroyWindow(fWindow);
        fWindow = nullptr;
        return;
    }

    SDL_SetRenderLogicalPresentation(fRenderer, kWindowWidth, kWindowHeight,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);

    ILoadFont();
    ILoadBanner();

    // The form is composed on the CPU into a plMipmap -- which is what plFont
    // renders into -- and uploaded whole once per frame. plMipmap's ARGB32 is
    // byte-identical to SDL's ARGB8888 on a little-endian host, so the upload
    // is a straight copy.
    fSurface = new plMipmap(kWindowWidth, kContentHeight, plMipmap::kARGB32Config,
                            1, plMipmap::kUncompressed);
    fSurfaceTexture = SDL_CreateTexture(fRenderer, SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        kWindowWidth, kContentHeight);

    SDL_StartTextInput(fWindow);
}

plLoginSDL::~plLoginSDL()
{
    delete fFont;
    delete fSurface;

    if (fBanner)
        SDL_DestroyTexture(fBanner);
    if (fSurfaceTexture)
        SDL_DestroyTexture(fSurfaceTexture);
    if (fWindow)
        SDL_StopTextInput(fWindow);
    if (fRenderer)
        SDL_DestroyRenderer(fRenderer);
    if (fWindow)
        SDL_DestroyWindow(fWindow);
}

void plLoginSDL::ILoadFont()
{
    // plFont reads a .p2f straight off disk with no key and no resource
    // manager, which is what makes this usable before the client is inited.
    for (const char* face : kFontFaces) {
        plFileName path = plFileName::Join("dat", ST::format("{}-{}.p2f", face, kFontSize));
        if (!plFileInfo(path).Exists())
            continue;

        auto font = std::make_unique<plFont>();
        if (!font->LoadFromP2FFile(path))
            continue;

        font->SetRenderYJustify(plFont::kRenderJustYTop);
        fFont = font.release();
        hsStatusMessageF("Login font: {}", path.AsString());
        return;
    }

    hsStatusMessage("No game fonts found for the login window; using the built-in font.");
}

void plLoginSDL::ILoadBanner()
{
    hsUNIXStream stream;
    if (!stream.Open("banner.png", "rb"))
        return;

    std::unique_ptr<plMipmap> image(plPNG::Instance().ReadFromStream(&stream));
    if (!image)
        return;

    fBanner = SDL_CreateTexture(fRenderer, SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STATIC,
                                image->GetWidth(), image->GetHeight());
    if (!fBanner)
        return;

    SDL_UpdateTexture(fBanner, nullptr, image->GetAddr32(0, 0),
                      image->GetWidth() * sizeof(uint32_t));
    SDL_SetTextureBlendMode(fBanner, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(fBanner, SDL_SCALEMODE_LINEAR);
}

//
// Drawing into the composed surface. Coordinates are in window space; the
// surface starts below the banner, so everything shifts up by kContentTop.
//

void plLoginSDL::IFillRect(const SDL_FRect& rect, uint32_t color)
{
    const int x0 = std::max(0, static_cast<int>(rect.x));
    const int y0 = std::max(0, static_cast<int>(rect.y) - kContentTop);
    const int x1 = std::min<int>(kWindowWidth, static_cast<int>(rect.x + rect.w));
    const int y1 = std::min<int>(kContentHeight, static_cast<int>(rect.y + rect.h) - kContentTop);

    for (int y = y0; y < y1; y++) {
        uint32_t* row = fSurface->GetAddr32(0, y);
        for (int x = x0; x < x1; x++)
            row[x] = color;
    }
}

void plLoginSDL::IFrameRect(const SDL_FRect& rect, uint32_t color)
{
    IFillRect({ rect.x, rect.y, rect.w, 1.0f }, color);
    IFillRect({ rect.x, rect.y + rect.h - 1.0f, rect.w, 1.0f }, color);
    IFillRect({ rect.x, rect.y, 1.0f, rect.h }, color);
    IFillRect({ rect.x + rect.w - 1.0f, rect.y, 1.0f, rect.h }, color);
}

float plLoginSDL::ITextWidth(const ST::string& text) const
{
    if (fFont)
        return fFont->CalcStringWidth(text);
    return text.size() * kDebugCharWidth;
}

void plLoginSDL::IText(float x, float y, const ST::string& text, uint32_t color)
{
    if (text.empty())
        return;

    if (fFont) {
        fFont->SetRenderColor(color);
        fFont->RenderString(fSurface, static_cast<uint16_t>(x),
                            static_cast<uint16_t>(y - kContentTop), text);
        return;
    }

    // Without a game font, the text is drawn over the composed surface after it
    // is blitted, since SDL_RenderDebugText only draws through the renderer.
    fDeferredText.push_back({ x, y, text, color });
}

void plLoginSDL::IDrawField(int index, const ST::string& text, bool masked)
{
    const SDL_FRect& r = kFieldRects[index];
    const bool focused = (fFocus == index);

    IFillRect(r, kColorField);
    IFrameRect(r, focused ? kColorFocus : kColorBorder);

    ST::string shown = masked ? ST::string::fill(text.size(), '*') : text;

    // Long entries scroll rather than spill: the tail is the part being typed,
    // so that is the part worth showing.
    const float room = r.w - kTextInset * 2;
    while (!shown.empty() && ITextWidth(shown + ST_LITERAL("_")) > room)
        shown = shown.substr(1);
    if (focused)
        shown += ST_LITERAL("_");

    IText(r.x + kTextInset, r.y + (r.h - kFontSize) / 2, shown, kColorText);
}

void plLoginSDL::IDrawButton(int index, const ST::string& text)
{
    const SDL_FRect& r = kFieldRects[index];
    const bool focused = (fFocus == index);

    IFillRect(r, focused ? kColorButtonHot : kColorButton);
    IFrameRect(r, focused ? kColorFocus : kColorBorder);
    IText(r.x + (r.w - ITextWidth(text)) / 2, r.y + (r.h - kFontSize) / 2,
          text, focused ? kColorFocus : kColorText);
}

void plLoginSDL::IDrawCheckbox()
{
    const SDL_FRect& row = kFieldRects[kFieldRemember];
    const bool focused = (fFocus == kFieldRemember);
    const SDL_FRect box{
        row.x,
        row.y + (row.h - kCheckboxSize) / 2,
        kCheckboxSize,
        kCheckboxSize,
    };

    IFillRect(box, kColorField);
    IFrameRect(box, focused ? kColorFocus : kColorBorder);

    if (fRemember) {
        // A small stepped tick keeps this independent of any particular font.
        for (int i = 0; i < 5; i++)
            IFillRect({ box.x + 3.0f + i, box.y + 8.0f + i, 3.0f, 2.0f }, kColorFocus);
        for (int i = 0; i < 8; i++)
            IFillRect({ box.x + 7.0f + i, box.y + 12.0f - i, 3.0f, 2.0f }, kColorFocus);
    }

    IText(box.x + box.w + kTextInset,
          row.y + (row.h - kFontSize) / 2,
          ST_LITERAL("Remember password"),
          focused ? kColorFocus : kColorText);
}

void plLoginSDL::IDrawLanguage()
{
    const SDL_FRect& r = kFieldRects[kFieldLanguage];
    const bool focused = (fFocus == kFieldLanguage);

    IFillRect(r, kColorField);
    IFrameRect(r, focused ? kColorFocus : kColorBorder);
    IText(r.x + kTextInset, r.y + (r.h - kFontSize) / 2,
          plLocalization::GetLanguageName(fLanguage), kColorText);

    // Draw a font-independent downward arrow at the right edge.
    const float arrowX = r.x + r.w - 17.0f;
    const float arrowY = r.y + (r.h - 5.0f) / 2;
    for (int i = 0; i < 5; i++) {
        IFillRect({ arrowX + i, arrowY + i,
                    9.0f - static_cast<float>(i * 2), 1.0f },
                  focused ? kColorFocus : kColorText);
    }
}

std::vector<plLocalization::Language> plLoginSDL::IUsableLanguages() const
{
    std::vector<plLocalization::Language> languages;
    for (plLocalization::Language language : plLocalization::GetAllLanguages()) {
        if (plLocalization::IsLanguageUsable(language))
            languages.emplace_back(language);
    }
    return languages;
}

void plLoginSDL::IEnsureLanguageHighlightVisible()
{
    const std::vector<plLocalization::Language> languages = IUsableLanguages();
    auto it = std::find(languages.begin(), languages.end(), fLanguageHighlight);
    if (it == languages.end())
        return;

    const size_t index = std::distance(languages.begin(), it);
    if (index < fLanguageScroll)
        fLanguageScroll = index;
    else if (index >= fLanguageScroll + kMaxVisibleLanguages)
        fLanguageScroll = index - kMaxVisibleLanguages + 1;
}

void plLoginSDL::IOpenLanguageMenu()
{
    const std::vector<plLocalization::Language> languages = IUsableLanguages();
    if (languages.empty())
        return;

    fLanguageOpen = true;
    fLanguageHighlight = std::find(languages.begin(), languages.end(), fLanguage) != languages.end()
                           ? fLanguage
                           : languages.front();
    IEnsureLanguageHighlightVisible();
}

void plLoginSDL::ICloseLanguageMenu(bool accept)
{
    if (accept)
        fLanguage = fLanguageHighlight;
    fLanguageOpen = false;
}

void plLoginSDL::IMoveLanguageHighlight(int delta)
{
    const std::vector<plLocalization::Language> languages = IUsableLanguages();
    if (languages.empty())
        return;

    auto it = std::find(languages.begin(), languages.end(), fLanguageHighlight);
    int index = it == languages.end() ? 0 : static_cast<int>(std::distance(languages.begin(), it));
    index = std::clamp(index + delta, 0, static_cast<int>(languages.size()) - 1);
    fLanguageHighlight = languages[index];
    IEnsureLanguageHighlightVisible();
}

bool plLoginSDL::ILanguageAtPoint(float x, float y,
                                  plLocalization::Language* language) const
{
    const std::vector<plLocalization::Language> languages = IUsableLanguages();
    const size_t visible = std::min(kMaxVisibleLanguages, languages.size());
    if (!visible)
        return false;

    const SDL_FRect& field = kFieldRects[kFieldLanguage];
    const float menuTop = field.y - visible * kLanguageRowHeight;
    if (x < field.x || x >= field.x + field.w || y < menuTop || y >= field.y)
        return false;

    const size_t row = static_cast<size_t>((y - menuTop) / kLanguageRowHeight);
    const size_t index = fLanguageScroll + row;
    if (index >= languages.size())
        return false;

    *language = languages[index];
    return true;
}

void plLoginSDL::IDrawLanguageMenu()
{
    if (!fLanguageOpen)
        return;

    const std::vector<plLocalization::Language> languages = IUsableLanguages();
    const size_t visible = std::min(kMaxVisibleLanguages, languages.size());
    if (!visible)
        return;

    const SDL_FRect& field = kFieldRects[kFieldLanguage];
    const float menuTop = field.y - visible * kLanguageRowHeight;
    const SDL_FRect menu{ field.x, menuTop, field.w,
                          visible * kLanguageRowHeight };

    // The fallback font is rendered after the composed surface. Remove text
    // belonging to covered controls so it cannot show through this overlay;
    // the menu's own text is queued immediately afterwards.
    if (!fFont) {
        fDeferredText.erase(
            std::remove_if(fDeferredText.begin(), fDeferredText.end(),
                           [&menu](const DeferredText& text) {
                               return text.fX >= menu.x && text.fX < menu.x + menu.w &&
                                      text.fY >= menu.y && text.fY < menu.y + menu.h;
                           }),
            fDeferredText.end());
    }

    IFillRect(menu, kColorField);
    for (size_t row = 0; row < visible; row++) {
        const size_t index = fLanguageScroll + row;
        if (index >= languages.size())
            break;

        const SDL_FRect item{ menu.x, menu.y + row * kLanguageRowHeight,
                              menu.w, kLanguageRowHeight };
        const bool highlighted = languages[index] == fLanguageHighlight;
        if (highlighted)
            IFillRect(item, kColorButtonHot);
        IText(item.x + kTextInset, item.y + (item.h - kFontSize) / 2,
              plLocalization::GetLanguageName(languages[index]),
              highlighted ? kColorFocus : kColorText);

        if (row)
            IFillRect({ item.x, item.y, item.w, 1.0f }, kColorBorder);
    }
    IFrameRect(menu, kColorFocus);
}

void plLoginSDL::IDraw(const ST::string& status)
{
    fDeferredText.clear();

    IFillRect({ 0.0f, static_cast<float>(kContentTop),
                static_cast<float>(kWindowWidth), static_cast<float>(kContentHeight) },
              kColorBackground);

    const float labelOffset = (kRowHeight - kFontSize) / 2;
    IText(kLabelX, kFieldRects[kFieldAccount].y + labelOffset, ST_LITERAL("Account"), kColorDimText);
    IText(kLabelX, kFieldRects[kFieldPassword].y + labelOffset, ST_LITERAL("Password"), kColorDimText);
    IText(kLabelX, kFieldRects[kFieldLanguage].y + labelOffset, ST_LITERAL("Language"), kColorDimText);

    IDrawField(kFieldAccount, fAccount, false);
    IDrawField(kFieldPassword, fPassword, true);

    IDrawCheckbox();
    IDrawLanguage();

    IDrawButton(kFieldLogIn, ST_LITERAL("Log In"));
    IDrawButton(kFieldQuit, ST_LITERAL("Quit"));

    if (!status.empty())
        IText(kLabelX, 292.0f, status, kColorError);

    IText(kLabelX, 372.0f,
          fLanguageOpen
              ? ST_LITERAL("Up/Down selects    Enter confirms    Esc closes")
              : ST_LITERAL("Tab switches fields    Enter activates    Esc quits"),
          kColorDimText);

    // The open menu is an overlay and must be the last thing composed into the
    // form surface so it covers the controls behind it.
    IDrawLanguageMenu();

    // Compose: banner on top, then the surface, then anything the fallback font
    // still owes.
    ISetDrawColor(fRenderer, kColorBackground);
    SDL_RenderClear(fRenderer);

    if (fBanner) {
        const SDL_FRect dest{ 0.0f, 0.0f, static_cast<float>(kWindowWidth),
                              static_cast<float>(kBannerHeight) };
        SDL_RenderTexture(fRenderer, fBanner, nullptr, &dest);
    } else {
        IText(kLabelX, 32.0f, plProduct::LongName(), kColorText);
    }

    SDL_UpdateTexture(fSurfaceTexture, nullptr, fSurface->GetAddr32(0, 0),
                      kWindowWidth * sizeof(uint32_t));
    const SDL_FRect surfaceDest{ 0.0f, static_cast<float>(kContentTop),
                                 static_cast<float>(kWindowWidth),
                                 static_cast<float>(kContentHeight) };
    SDL_RenderTexture(fRenderer, fSurfaceTexture, nullptr, &surfaceDest);

    for (const DeferredText& text : fDeferredText) {
        ISetDrawColor(fRenderer, text.fColor);
        SDL_RenderDebugText(fRenderer, text.fX, text.fY, text.fText.c_str());
    }

    SDL_RenderPresent(fRenderer);
}

//
// Input
//

ST::string* plLoginSDL::IEditTarget()
{
    switch (fFocus) {
    case kFieldAccount:
        return &fAccount;
    case kFieldPassword:
        return &fPassword;
    default:
        return nullptr;
    }
}

plLoginSDL::Field plLoginSDL::IFieldAtPoint(float x, float y) const
{
    for (int i = 0; i < kNumFields; i++) {
        const SDL_FRect& r = kFieldRects[i];
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h)
            return static_cast<Field>(i);
    }
    return kNumFields;
}

void plLoginSDL::IActivate(Field field, bool* done, bool* accepted)
{
    switch (field) {
    case kFieldRemember:
        fRemember = !fRemember;
        break;
    case kFieldLanguage:
        IOpenLanguageMenu();
        break;
    case kFieldQuit:
        *done = true;
        *accepted = false;
        break;
    case kFieldLogIn:
    default:
        // An empty field is not worth sending to the server, so treat Enter on
        // one as a request to go fill it in.
        if (fAccount.empty()) {
            fFocus = kFieldAccount;
            fMessage = ST_LITERAL("Please enter an account name.");
        } else if (fPassword.empty()) {
            fFocus = kFieldPassword;
            fMessage = ST_LITERAL("Please enter a password.");
        } else {
            *done = true;
            *accepted = true;
        }
        break;
    }
}

bool plLoginSDL::IHandleKey(const SDL_KeyboardEvent& key, bool* done, bool* accepted)
{
    ST::string* target = IEditTarget();

    if (fLanguageOpen) {
        switch (key.key) {
        case SDLK_UP:
            IMoveLanguageHighlight(-1);
            return true;
        case SDLK_DOWN:
            IMoveLanguageHighlight(1);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            ICloseLanguageMenu(true);
            return true;
        case SDLK_ESCAPE:
            ICloseLanguageMenu(false);
            return true;
        case SDLK_TAB:
            ICloseLanguageMenu(false);
            if (key.mod & SDL_KMOD_SHIFT)
                fFocus = static_cast<Field>((fFocus + kNumFields - 1) % kNumFields);
            else
                fFocus = static_cast<Field>((fFocus + 1) % kNumFields);
            return true;
        default:
            return true;
        }
    }

    switch (key.key) {
    case SDLK_TAB:
        if (key.mod & SDL_KMOD_SHIFT)
            fFocus = static_cast<Field>((fFocus + kNumFields - 1) % kNumFields);
        else
            fFocus = static_cast<Field>((fFocus + 1) % kNumFields);
        return true;

    case SDLK_UP:
        fFocus = static_cast<Field>((fFocus + kNumFields - 1) % kNumFields);
        return true;

    case SDLK_DOWN:
        fFocus = static_cast<Field>((fFocus + 1) % kNumFields);
        return true;

    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        if (fFocus == kFieldLanguage) {
            IOpenLanguageMenu();
            return true;
        }
        // Enter anywhere except the Quit button means "log in", which is what
        // someone who just typed their password expects it to mean.
        IActivate(fFocus == kFieldQuit ? kFieldQuit : kFieldLogIn, done, accepted);
        return true;

    case SDLK_SPACE:
        if (fFocus == kFieldRemember || fFocus == kFieldLanguage) {
            IActivate(fFocus, done, accepted);
            return true;
        }
        return false;

    case SDLK_ESCAPE:
        *done = true;
        *accepted = false;
        return true;

    case SDLK_BACKSPACE:
        if (target && !target->empty())
            *target = target->left(target->size() - 1);
        return true;

    case SDLK_V:
        if ((key.mod & SDL_KMOD_CTRL) && target) {
            char* clip = SDL_GetClipboardText();
            if (clip) {
                *target += ST::string::from_utf8(clip).before_first('\n');
                SDL_free(clip);
            }
            return true;
        }
        return false;

    default:
        return false;
    }
}

bool plLoginSDL::Prompt(ST::string* account, ST::string* password, bool* remember)
{
    hsAssert(IsValid(), "Prompt() on a login window that was never created");

    fAccount = *account;
    fPassword = *password;
    fRemember = *remember;
    fLanguageOpen = false;
    fLanguageHighlight = fLanguage;
    fLanguageScroll = 0;

    // Whichever field is not filled in yet is the one being asked about. With
    // nothing left to fill in, the button is -- there is only one thing left to
    // do, and Enter should do it. Same rule as LoadUserPass, winmain.cpp:705-711.
    if (fAccount.empty())
        fFocus = kFieldAccount;
    else if (fPassword.empty())
        fFocus = kFieldPassword;
    else
        fFocus = kFieldLogIn;

    SDL_ShowWindow(fWindow);
    SDL_RaiseWindow(fWindow);

    bool done = false;
    bool accepted = false;
    while (!done) {
        IDraw(fMessage);

        SDL_Event evt;
        if (!SDL_WaitEventTimeout(&evt, 250))
            continue;

        // Mouse coordinates arrive in window space; the layout is in logical
        // space, and only the renderer knows how the two relate.
        SDL_ConvertEventToRenderCoordinates(fRenderer, &evt);

        switch (evt.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            done = true;
            accepted = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            IHandleKey(evt.key, &done, &accepted);
            break;

        case SDL_EVENT_TEXT_INPUT:
            if (ST::string* target = IEditTarget()) {
                const size_t limit = (fFocus == kFieldAccount) ? kMaxAccountNameLength
                                                               : kMaxPasswordLength;
                if (target->size() < limit)
                    *target += ST::string::from_utf8(evt.text.text);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                if (fLanguageOpen) {
                    plLocalization::Language language;
                    if (ILanguageAtPoint(evt.button.x, evt.button.y, &language)) {
                        fLanguageHighlight = language;
                        ICloseLanguageMenu(true);
                    } else {
                        ICloseLanguageMenu(false);
                    }
                    break;
                }

                Field hit = IFieldAtPoint(evt.button.x, evt.button.y);
                if (hit != kNumFields) {
                    fFocus = hit;
                    // Clicking a text field only moves focus there; clicking
                    // anything else is the same as pressing it.
                    if (hit != kFieldAccount && hit != kFieldPassword)
                        IActivate(hit, &done, &accepted);
                }
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (fLanguageOpen) {
                plLocalization::Language language;
                if (ILanguageAtPoint(evt.motion.x, evt.motion.y, &language))
                    fLanguageHighlight = language;
            }
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            if (fLanguageOpen && evt.wheel.y != 0.0f)
                IMoveLanguageHighlight(evt.wheel.y > 0.0f ? -1 : 1);
            break;

        default:
            break;
        }
    }

    if (!accepted)
        return false;

    plLocalization::SetLanguage(fLanguage);

    *account = fAccount;
    *password = fPassword;
    *remember = fRemember;

    // The password is not worth keeping on screen any longer than it has to be.
    fPassword = ST::string();
    fMessage = ST::string();
    return true;
}

void plLoginSDL::Tick(const ST::string& status)
{
    if (!IsValid())
        return;

    // Only drain what the window itself needs. Anything else in the queue is
    // for the game window and is the main loop's business, not this one's.
    // Consuming these is what keeps the compositor from deciding the window has
    // stopped answering while the server takes its time.
    SDL_PumpEvents();
    SDL_Event evt;
    while (SDL_PeepEvents(&evt, 1, SDL_GETEVENT, SDL_EVENT_WINDOW_FIRST,
                          SDL_EVENT_WINDOW_LAST) > 0) {
    }

    IDraw(status);
}
