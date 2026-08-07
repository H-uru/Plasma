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

#ifndef plLoginSDL_inc
#define plLoginSDL_inc

#include "plResMgr/plLocalization.h"

#include <string_theory/string>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_FRect;
class plFont;
class plMipmap;

/**
 * The login form, drawn with SDL.
 *
 * This runs before the pipeline exists, so it cannot use the engine's own GUI:
 * pfGameGUIMgr only loads dialogs authored into .prp data, and there is no
 * login dialog in the GUI age to load. It draws instead with SDL's built-in
 * 8x8 debug font, which costs nothing and pulls in no font dependency, in its
 * own small window that is destroyed once the login is done.
 *
 * Construction can fail -- check IsValid() -- in which case the caller is
 * expected to fall back to the terminal prompt.
 */
class plLoginSDL
{
public:
    /** Everything that can hold focus, in tab order. */
    enum Field
    {
        kFieldAccount,
        kFieldPassword,
        kFieldRemember,
        kFieldLanguage,
        kFieldLogIn,
        kFieldQuit,
        kNumFields
    };

    plLoginSDL();
    ~plLoginSDL();

    plLoginSDL(const plLoginSDL&) = delete;
    plLoginSDL& operator=(const plLoginSDL&) = delete;

    /** Whether the window and renderer were actually created. */
    bool IsValid() const { return fRenderer != nullptr; }

    /**
     * Runs the form until the user submits or quits.
     *
     * The account, password and remember flag are in/out: what is passed in is
     * what the fields start out holding. Returns false if the user asked to
     * quit, in which case the outputs are left alone.
     */
    bool Prompt(ST::string* account, ST::string* password, bool* remember);

    /**
     * Draws one frame showing a status line, and services the window's events.
     *
     * Meant to be called from inside the authentication wait loop so the window
     * does not go unresponsive while the server is thinking.
     */
    void Tick(const ST::string& status);

    /** Sets the message shown by the next Prompt(), used for auth failures. */
    void SetMessage(ST::string message) { fMessage = std::move(message); }

private:
    /** A run of text owed to the fallback font, drawn after the surface blit. */
    struct DeferredText
    {
        float      fX;
        float      fY;
        ST::string fText;
        uint32_t   fColor;
    };

    void ILoadFont();
    void ILoadBanner();

    void  IFillRect(const SDL_FRect& rect, uint32_t color);
    void  IFrameRect(const SDL_FRect& rect, uint32_t color);
    void  IText(float x, float y, const ST::string& text, uint32_t color);
    float ITextWidth(const ST::string& text) const;

    void IDraw(const ST::string& status);
    void IDrawField(int index, const ST::string& text, bool masked);
    void IDrawButton(int index, const ST::string& text);
    void IDrawCheckbox();
    void IDrawLanguage();
    void IDrawLanguageMenu();
    bool IHandleKey(const struct SDL_KeyboardEvent& key, bool* done, bool* accepted);
    void IActivate(Field field, bool* done, bool* accepted);
    Field IFieldAtPoint(float x, float y) const;
    ST::string* IEditTarget();

    std::vector<plLocalization::Language> IUsableLanguages() const;
    void IOpenLanguageMenu();
    void ICloseLanguageMenu(bool accept);
    void IMoveLanguageHighlight(int delta);
    void IEnsureLanguageHighlightVisible();
    bool ILanguageAtPoint(float x, float y, plLocalization::Language* language) const;

    SDL_Window*   fWindow;
    SDL_Renderer* fRenderer;

    /** The banner art, and the CPU-composed form below it. */
    SDL_Texture* fBanner;
    SDL_Texture* fSurfaceTexture;
    plMipmap*    fSurface;
    plFont*      fFont;

    std::vector<DeferredText> fDeferredText;

    ST::string fAccount;
    ST::string fPassword;
    ST::string fMessage;
    bool       fRemember;
    Field      fFocus;

    plLocalization::Language fLanguage;
    plLocalization::Language fLanguageHighlight;
    size_t                   fLanguageScroll;
    bool                     fLanguageOpen;
};

#endif // plLoginSDL_inc
