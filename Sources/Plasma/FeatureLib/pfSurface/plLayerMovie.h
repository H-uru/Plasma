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

#ifndef plLayerMovie_inc
#define plLayerMovie_inc

#include "plFileSystem.h"

#include "plSurface/plLayerAnimation.h"

class plMessage;
class hsResMgr;
class hsStream;

class plLayerMovie : public plLayerAnimation
{
protected:
    plFileName                  fMovieName;

    int32_t                     fCurrentFrame;
    float                       fLength;
    uint32_t                    fWidth, fHeight;

    virtual int32_t             ISecsToFrame(float secs) = 0;

    bool                        IGetFault() const { return !fMovieName.IsValid(); }
    bool                        ISetFault(const char* errStr);
    bool                        ICheckBitmap();
    bool                        IMovieIsIdle(); // will call IRelease();
    bool                        ISetupBitmap();
    bool                        ISetSize(int w, int h);
    bool                        ISetLength(float secs);
    bool                        ICurrentFrameDirty(double wSecs);

    virtual bool                IInit() = 0; // Load header etc, must call ISetSize(w, h), ISetLength(s)
    virtual bool                IGetCurrentFrame() = 0; // Load fCurrentFrame into bitmap
    virtual bool                IRelease() = 0; // release any system resources.
public:
    plLayerMovie();
    virtual ~plLayerMovie();

    CLASSNAME_REGISTER( plLayerMovie );
    GETINTERFACE_ANY( plLayerMovie, plLayerAnimation );

    uint32_t        Eval(double secs, uint32_t frame, uint32_t ignore) override;

    void                    Read(hsStream* s, hsResMgr* mgr) override;
    void                    Write(hsStream* s, hsResMgr* mgr) override;

    bool                    IsStopped() { return fTimeConvert.IsStopped(); }

    void                    SetMovieName(const plFileName& n) { fMovieName = n; }
    const plFileName&       GetMovieName() const { return fMovieName; }

    bool                    MsgReceive(plMessage* msg) override;

    // Movie specific
    int                     GetWidth() const;
    int                     GetHeight() const;
    float                   GetLength() const { return fLength; }

    virtual void            DefaultMovie();
};

#endif // plLayerMovie_inc
