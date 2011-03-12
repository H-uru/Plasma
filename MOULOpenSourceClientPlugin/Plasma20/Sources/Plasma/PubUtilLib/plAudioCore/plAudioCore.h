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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plAudioCore - Core, common stuff that all of the audio system needs		//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plAudioCore_h
#define _plAudioCore_h

//// plWAVHeader Class ///////////////////////////////////////////////////////
//	Just a small info class about WAV sound

class plWAVHeader
{
	public:
		UInt16	fFormatTag;
		UInt16	fNumChannels;
		UInt32	fNumSamplesPerSec;
		UInt32	fAvgBytesPerSec;
		UInt16	fBlockAlign;
		UInt16	fBitsPerSample;

		enum
		{
			kPCMFormatTag = 1
		};
};

//// plAudioCore Konstants ///////////////////////////////////////////////////

namespace plAudioCore
{
	enum ChannelSelect
	{
		kAll = 0,
		kLeft,
		kRight
	};

};


#endif //_plAudioCore_h
