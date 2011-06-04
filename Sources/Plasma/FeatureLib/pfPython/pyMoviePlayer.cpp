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

#include "../plMessage/plMovieMsg.h"
#include "../pfMessage/pfMovieEventMsg.h"

pyMoviePlayer::pyMoviePlayer(const char* movieName,pyKey& selfKey)
{
	fMovieName = hsStrcpy(movieName);
	fSelfKey = selfKey.getKey();
	// make the movie
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kMake | plMovieMsg::kAddCallbacks);
		mov->SetSender(fSelfKey);
		pfMovieEventMsg* movCallback = TRACKED_NEW pfMovieEventMsg(fMovieName);
		movCallback->AddReceiver(fSelfKey);
		mov->AddCallback(movCallback);
		mov->Send();
		hsRefCnt_SafeUnRef(movCallback);
	}
}


pyMoviePlayer::~pyMoviePlayer()
{
	Stop();
	delete [] fMovieName;
}

void pyMoviePlayer::MakeMovie(const char* movieName, pyKey& selfKey)
{
	Stop();
	if (fMovieName)
		delete [] fMovieName;
	fMovieName = hsStrcpy(movieName);
	fSelfKey = selfKey.getKey();
	if (fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kMake | plMovieMsg::kAddCallbacks);
		mov->SetSender(fSelfKey);
		pfMovieEventMsg* movCallback = TRACKED_NEW pfMovieEventMsg(fMovieName);
		movCallback->AddReceiver(fSelfKey);
		mov->AddCallback(movCallback);
		mov->Send();
		hsRefCnt_SafeUnRef(movCallback);
	}
}

void pyMoviePlayer::SetCenter(hsScalar x, hsScalar y)
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kMove);
		mov->SetSender(fSelfKey);
		mov->SetCenterX(x);
		mov->SetCenterY(y);
		mov->Send();
	}
}

void pyMoviePlayer::SetScale(hsScalar width, hsScalar height)
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kScale);
		mov->SetSender(fSelfKey);
		mov->SetScaleX(width);
		mov->SetScaleY(height);
		mov->Send();
	}
}

void pyMoviePlayer::SetColor(pyColor color)
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kColor);
		mov->SetSender(fSelfKey);
		mov->SetColor(color.getColor());
		mov->Send();
	}
}

void pyMoviePlayer::SetVolume(hsScalar volume)
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kVolume);
		mov->SetSender(fSelfKey);
		mov->SetVolume(volume);
		mov->Send();
	}
}

void pyMoviePlayer::SetOpacity(hsScalar opacity)
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kOpacity);
		mov->SetSender(fSelfKey);
		mov->SetOpacity(opacity);
		mov->Send();
	}
}


void pyMoviePlayer::Play()
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kStart);
		mov->SetSender(fSelfKey);
		mov->Send();
	}
}

void pyMoviePlayer::PlayPaused()
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kStart | plMovieMsg::kPause);
		mov->SetSender(fSelfKey);
		mov->Send();
	}
}

void pyMoviePlayer::Pause()
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kPause);
		mov->SetSender(fSelfKey);
		mov->Send();
	}
}

void pyMoviePlayer::Resume()
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kResume);
		mov->SetSender(fSelfKey);
		mov->Send();
	}
}

void pyMoviePlayer::Stop()
{
	if ( fMovieName)
	{
		plMovieMsg* mov = TRACKED_NEW plMovieMsg(fMovieName, plMovieMsg::kStop);
		mov->SetSender(fSelfKey);
		mov->Send();
	}
}
