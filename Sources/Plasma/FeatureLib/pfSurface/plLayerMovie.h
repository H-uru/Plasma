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

#include "plSurface/plLayerAnimation.h"
#include "plInterp/plAnimTimeConvert.h"

class plMessage;
class hsStream;
class hsResMgr;

class plLayerMovie : public plLayerAnimation
{
protected:
    char*                       fMovieName;

//  plAnimTimeConvert           fTimeConvert;

    int32_t                       fCurrentFrame;
    float                         fLength;
    uint32_t                      fWidth, fHeight;

    virtual int32_t               ISecsToFrame(float secs) = 0;

    hsBool                      IGetFault() const { return !(fMovieName &&  *fMovieName); }
    hsBool                      ISetFault(const char* errStr);
    hsBool                      ICheckBitmap();
    hsBool                      IMovieIsIdle(); // will call IRelease();
    hsBool                      ISetupBitmap();
    hsBool                      ISetSize(int w, int h);
    hsBool                      ISetLength(float secs);
    hsBool                      ICurrentFrameDirty(double wSecs);

    virtual hsBool              IInit() = 0; // Load header etc, must call ISetSize(w, h), ISetLength(s)
    virtual hsBool              IGetCurrentFrame() = 0; // Load fCurrentFrame into bitmap
    virtual hsBool              IRelease() = 0; // release any system resources.
public:
    plLayerMovie();
    virtual ~plLayerMovie();

    CLASSNAME_REGISTER( plLayerMovie );
    GETINTERFACE_ANY( plLayerMovie, plLayerAnimation );

    virtual uint32_t          Eval(double secs, uint32_t frame, uint32_t ignore);

    virtual void            Read(hsStream* s, hsResMgr* mgr);
    virtual void            Write(hsStream* s, hsResMgr* mgr);

    hsBool                  IsStopped() { return fTimeConvert.IsStopped(); }

    void                    SetMovieName(const char* n);
    const char*             GetMovieName() const { return fMovieName; }

    virtual hsBool          MsgReceive(plMessage* msg);

    // Movie specific
    int                     GetWidth() const;
    int                     GetHeight() const;
    float                   GetLength() const { return fLength; }

    virtual void            DefaultMovie();
};

#endif // plLayerMovie_inc
