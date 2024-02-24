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

#ifndef plMovieMsg_inc
#define plMovieMsg_inc

#include <vector>

#include "hsColorRGBA.h"
#include "hsPoint2.h"

#include "pnMessage/plMessage.h"
#include "pnKeyedObject/plFixedKey.h"

class plMovieMsg : public plMessage
{
public:
    enum
    {
        kIgnore             = 0x0,
        kStart              = 0x1,
        kPause              = 0x2,
        kResume             = 0x4,
        kStop               = 0x8,
        kMove               = 0x10, // Call SetCenter() or default is 0,0
        kScale              = 0x20, // Call SetScale() or default is 1,1
        kColor              = 0x40, // Call SetColor() or default is 1,1,1
        kVolume             = 0x80, // Call SetVolume() or default is 1
        kOpacity            = 0x100, // Call SetOpacity() or default is 1
        kColorAndOpacity    = 0x200, // Call SetColor() or default is 1,1,1,1
        kMake               = 0x400,        //Installs, but doesn't play until kStart
        kAddCallbacks       = 0x800, // Call AddCallback() for each callback message
        kFadeIn             = 0x1000, // Call SetFadeInSecs() and SetFadeInColor() or defs are 0 and 0,0,0,0
        kFadeOut            = 0x2000 // Call SetFadeOutSecs() and SetFadeOutColor() or defs are 0 and 0,0,0,0
    };
protected:

    hsPoint2    fCenter;

    hsPoint2    fScale;

    hsColorRGBA fColor;

    hsColorRGBA fFadeInColor;
    float       fFadeInSecs;

    hsColorRGBA fFadeOutColor;
    float       fFadeOutSecs;

    float       fVolume;

    ST::string  fFileName;

    uint16_t    fCmd;

    std::vector<plMessage*> fCallbacks;

public:
    plMovieMsg(const ST::string& name, uint16_t cmd)
        : plMessage(nullptr, nullptr, nullptr)
    { 
        fFileName = name;
        SetCmd(cmd).MakeDefault();
    }

    plMovieMsg() : fCmd(kIgnore)
    { 
        MakeDefault();
    }

    virtual ~plMovieMsg()
    {
        for (plMessage* callback : fCallbacks)
        {
            hsRefCnt_SafeUnRef(callback);
        }
    }

    CLASSNAME_REGISTER( plMovieMsg );
    GETINTERFACE_ANY( plMovieMsg, plMessage );

    plMovieMsg& MakeDefault() 
    { 
        SetCenter(0,0);
        SetScale(1.f,1.f);
        SetColor(1.f, 1.f, 1.f, 1.f);
        SetFadeInSecs(0);
        SetFadeInColor(0, 0, 0, 0);
        SetFadeOutSecs(0);
        SetFadeOutColor(0, 0, 0, 0);
        SetVolume(1.f);
        SetBCastFlag(kBCastByType); 
        return *this; 
    }

    // Make sure you set at least one command, and set appropriate params for the command
    uint16_t GetCmd() const { return fCmd; }
    plMovieMsg& SetCmd(uint16_t c) { fCmd = c; return *this; }

    // Center 0,0 is center of screen, 1,1 is Upper-Right, -1,-1 is Lower-Left, etc.
    const hsPoint2& GetCenter() const { return fCenter; }
    float GetCenterX() const { return fCenter.fX; }
    float GetCenterY() const { return fCenter.fY; }

    plMovieMsg& SetCenter(const hsPoint2& p) { fCenter = p; return *this; }
    plMovieMsg& SetCenter(float x, float y) { fCenter.Set(x, y); return *this; }
    plMovieMsg& SetCenterX(float x) { fCenter.fX = x; return *this; }
    plMovieMsg& SetCenterY(float y) { fCenter.fY = y; return *this; }

    // Scale of 1.0 matches movie pixel to screen pixel (whatever the resolution).
    // Scale of 2.0 doubles each movie pixel across 2 screen pixels.
    // Etc.
    const hsPoint2& GetScale() const { return fScale; }
    float GetScaleX() const { return fScale.fX; }
    float GetScaleY() const { return fScale.fY; }

    plMovieMsg& SetScale(const hsPoint2& p) { fScale = p; return *this; }
    plMovieMsg& SetScale(float x, float y) { fScale.Set(x, y); return *this; }
    plMovieMsg& SetScaleX(float x) { fScale.fX = x; return *this; }
    plMovieMsg& SetScaleY(float y) { fScale.fY = y; return *this; }

    // Include the movie folder, e.g. "avi/movie.webm"
    // String is copied, not pointer copy.
    ST::string GetFileName() const { return fFileName; }
    plMovieMsg& SetFileName(const ST::string& name) { fFileName = name; return *this; }

    // Color is mostly useful for alpha fade up and down.
    const hsColorRGBA& GetColor() const { return fColor; }
    plMovieMsg& SetColor(const hsColorRGBA& c) { fColor = c; return *this; }
    plMovieMsg& SetColor(float r, float g, float b, float a) { fColor.Set(r,g,b,a); return *this; }
    plMovieMsg& SetOpacity(float a) { return SetColor(1.f, 1.f, 1.f, a); }

    // Or the auto matic fades
    const hsColorRGBA& GetFadeInColor() const { return fFadeInColor; }
    plMovieMsg& SetFadeInColor(const hsColorRGBA& c) { fFadeInColor = c; return *this; }
    plMovieMsg& SetFadeInColor(float r, float g, float b, float a) { fFadeInColor.Set(r,g,b,a); return *this; }

    float GetFadeInSecs() const { return fFadeInSecs; }
    plMovieMsg& SetFadeInSecs(float s) { fFadeInSecs = s; return *this; }

    const hsColorRGBA& GetFadeOutColor() const { return fFadeOutColor; }
    plMovieMsg& SetFadeOutColor(const hsColorRGBA& c) { fFadeOutColor = c; return *this; }
    plMovieMsg& SetFadeOutColor(float r, float g, float b, float a) { fFadeOutColor.Set(r,g,b,a); return *this; }

    float GetFadeOutSecs() const { return fFadeOutSecs; }
    plMovieMsg& SetFadeOutSecs(float s) { fFadeOutSecs = s; return *this; }

    // Volume is on scale of 0=muted to 1=full
    float GetVolume() const { return fVolume; }
    plMovieMsg& SetVolume(float v) { fVolume = v; return *this; }

    plMovieMsg& AddCallback(plMessage* msg) { hsRefCnt_SafeRef(msg); fCallbacks.emplace_back(msg); return *this; }
    size_t GetNumCallbacks() const { return fCallbacks.size(); }
    plMessage* GetCallback(size_t i) const { return fCallbacks[i]; }

    void Read(hsStream* s, hsResMgr* mgr) override { hsAssert(false, "Not for I/O"); plMessage::IMsgRead(s, mgr); }
    void Write(hsStream* s, hsResMgr* mgr) override { hsAssert(false, "Not for I/O"); plMessage::IMsgWrite(s, mgr); }
};

#endif // plMovieMsg_inc
