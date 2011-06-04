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

#include "hsTypes.h"
#include "plLayerMovie.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "hsUtils.h"

#include "../plMessage/plAnimCmdMsg.h"
#include "../plGImage/plMipmap.h"
#include "../plPipeline/hsGDeviceRef.h"

plLayerMovie::plLayerMovie()
:	fMovieName(nil),
	fCurrentFrame(-1),
	fLength(0),
	fWidth(32),
	fHeight(32)
{
	fOwnedChannels |= kTexture;
	fTexture = TRACKED_NEW plBitmap*;
	*fTexture = nil;
	
//	fTimeConvert.SetOwner(this);
}

plLayerMovie::~plLayerMovie()
{
	delete [] fMovieName;

	delete *fTexture;
}

hsBool plLayerMovie::ISetFault(const char* errStr)
{
#ifdef HS_DEBUGGING
	char buff[256];
	sprintf(buff, "ERROR %s: %s\n", fMovieName, errStr);
	hsStatusMessage(buff);
#endif // HS_DEBUGGING
	*fMovieName = 0;
	return true;
}

hsBool plLayerMovie::ISetLength(hsScalar secs)
{
	fLength = secs;
	return false;
}

int plLayerMovie::GetWidth() const
{
	plMipmap	*mip = plMipmap::ConvertNoRef( GetTexture() );
	return mip ? mip->GetWidth() : 0;
}

int plLayerMovie::GetHeight() const
{
	plMipmap	*mip = plMipmap::ConvertNoRef( GetTexture() );
	return mip ? mip->GetHeight() : 0;
}

hsBool plLayerMovie::ISetSize(int width, int height)
{
	fWidth = width;
	fHeight = height;
	return false;
}

hsBool plLayerMovie::ISetupBitmap()
{
	if( !GetTexture() )
	{
		plMipmap* b = TRACKED_NEW plMipmap( fWidth, fHeight, plMipmap::kARGB32Config, 1 );
		memset(b->GetImage(), 0x10, b->GetHeight() * b->GetRowBytes() );
		b->SetFlags( b->GetFlags() | plMipmap::kDontThrowAwayImage );

		char	name[ 256 ];
		sprintf( name, "%s_BMap", fMovieName );
		hsgResMgr::ResMgr()->NewKey( name, b, plLocation::kGlobalFixedLoc );

		*fTexture = (plBitmap *)b;
	}

	return false;
}

hsBool plLayerMovie::ICheckBitmap()
{
	if( !GetTexture() )
		ISetupBitmap();

	return false;
}

hsBool plLayerMovie::IMovieIsIdle()
{
	IRelease();

	return false;
}

hsBool plLayerMovie::ICurrentFrameDirty(double wSecs)
{
	hsScalar secs = fTimeConvert.WorldToAnimTime(wSecs);
	UInt32 frame = ISecsToFrame(secs);
	if( frame == fCurrentFrame )
		return false;
	fCurrentFrame = frame;

	return true;
}

UInt32 plLayerMovie::Eval(double wSecs, UInt32 frame, UInt32 ignore)
{
	UInt32 dirty = plLayerAnimation::Eval(wSecs, frame, ignore);

	if( !IGetFault() && !(ignore & kTexture) )
	{
		if( ICurrentFrameDirty(wSecs) )
		{			
			if( IGetCurrentFrame() )
				ISetFault("Getting current frame");

			if( GetTexture() )
			{
				hsGDeviceRef* ref = GetTexture()->GetDeviceRef();
				if( ref )
					ref->SetDirty(true);
			}
		}
		else
		if( IsStopped() )
		{
			IMovieIsIdle();
		}
		
		dirty |= kTexture;
	}
	return dirty;
}

void plLayerMovie::Read(hsStream* s, hsResMgr* mgr)
{
	plLayerAnimation::Read(s, mgr);

	delete [] fMovieName;
	int len = s->ReadSwap32();
	if( len )
	{
		fMovieName = TRACKED_NEW char[len+1];
		s->Read(len, fMovieName);
		fMovieName[len] = 0;
	}
	else
	{
		hsAssert(false, "Reading empty string for movie name");
		fMovieName = nil;
	}
}

void plLayerMovie::Write(hsStream* s, hsResMgr* mgr)
{
	plLayerAnimation::Write(s, mgr);

	int len = hsStrlen(fMovieName);
	s->WriteSwap32(len);
	if( len )
		s->Write(len, fMovieName);
}

void plLayerMovie::SetMovieName(const char* n)
{
	delete [] fMovieName;
	fMovieName = hsStrcpy(n);
}

hsBool plLayerMovie::MsgReceive(plMessage* msg)
{	
	return plLayerAnimation::MsgReceive(msg);
}

void plLayerMovie::DefaultMovie()
{
}