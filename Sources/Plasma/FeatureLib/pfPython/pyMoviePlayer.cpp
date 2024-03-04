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
//////////////////////////////////////////////////////////////////////
//
// pyMoviePlayer   - a wrapper class all the Movie functions
//
//////////////////////////////////////////////////////////////////////

#include "pyMoviePlayer.h"

#include "plMessage/plMovieMsg.h"

#include "pfMessage/pfMovieEventMsg.h"

#include "pyColor.h"
#include "pyKey.h"

pyMoviePlayer::pyMoviePlayer(const ST::string& movieName, pyKey& selfKey)
{
    fMovieName = movieName;
    fSelfKey = selfKey.getKey();
    // make the movie
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kMake | plMovieMsg::kAddCallbacks);
        mov->SetSender(fSelfKey);
        pfMovieEventMsg* movCallback = new pfMovieEventMsg(fMovieName);
        movCallback->AddReceiver(fSelfKey);
        mov->AddCallback(movCallback);
        mov->Send();
        hsRefCnt_SafeUnRef(movCallback);
    }
}


pyMoviePlayer::~pyMoviePlayer()
{
    Stop();
}

void pyMoviePlayer::MakeMovie(const ST::string& movieName, pyKey& selfKey)
{
    Stop();
    fMovieName = movieName;
    fSelfKey = selfKey.getKey();
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kMake | plMovieMsg::kAddCallbacks);
        mov->SetSender(fSelfKey);
        pfMovieEventMsg* movCallback = new pfMovieEventMsg(fMovieName);
        movCallback->AddReceiver(fSelfKey);
        mov->AddCallback(movCallback);
        mov->Send();
        hsRefCnt_SafeUnRef(movCallback);
    }
}

void pyMoviePlayer::SetCenter(float x, float y)
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kMove);
        mov->SetSender(fSelfKey);
        mov->SetCenterX(x);
        mov->SetCenterY(y);
        mov->Send();
    }
}

void pyMoviePlayer::SetScale(float width, float height)
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kScale);
        mov->SetSender(fSelfKey);
        mov->SetScaleX(width);
        mov->SetScaleY(height);
        mov->Send();
    }
}

void pyMoviePlayer::SetColor(pyColor color)
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kColor);
        mov->SetSender(fSelfKey);
        mov->SetColor(color.getColor());
        mov->Send();
    }
}

void pyMoviePlayer::SetVolume(float volume)
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kVolume);
        mov->SetSender(fSelfKey);
        mov->SetVolume(volume);
        mov->Send();
    }
}

void pyMoviePlayer::SetOpacity(float opacity)
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kOpacity);
        mov->SetSender(fSelfKey);
        mov->SetOpacity(opacity);
        mov->Send();
    }
}


void pyMoviePlayer::Play()
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kStart);
        mov->SetSender(fSelfKey);
        mov->Send();
    }
}

void pyMoviePlayer::PlayPaused()
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kStart | plMovieMsg::kPause);
        mov->SetSender(fSelfKey);
        mov->Send();
    }
}

void pyMoviePlayer::Pause()
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kPause);
        mov->SetSender(fSelfKey);
        mov->Send();
    }
}

void pyMoviePlayer::Resume()
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kResume);
        mov->SetSender(fSelfKey);
        mov->Send();
    }
}

void pyMoviePlayer::Stop()
{
    if (!fMovieName.empty())
    {
        plMovieMsg* mov = new plMovieMsg(fMovieName, plMovieMsg::kStop);
        mov->SetSender(fSelfKey);
        mov->Send();
    }
}
