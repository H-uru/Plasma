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
#ifndef plWavFile_H
#define plWavFile_H

#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

#include "hsTypes.h"
#include "hsWindows.h"
#include "hsStlUtils.h"
#include <mmsystem.h>
#include "../plAudioCore/plAudioFileReader.h"


struct plSoundMarker
{
	char *fName;
	double fOffset; // in Secs

	plSoundMarker () { fName = NULL;fOffset = 0.0; }

};


//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------



class CWaveFile : public plAudioFileReader
{
public:
    CWaveFile();
    ~CWaveFile();

    HRESULT Open(const char *strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags );
    HRESULT OpenFromMemory( BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags );

    HRESULT Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT AdvanceWithoutRead( DWORD dwSizeToRead, DWORD* pdwSizeRead );
    HRESULT Write( UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote );

    DWORD   GetSize();
    HRESULT ResetFile();
    WAVEFORMATEX* GetFormat() { return m_pwfx; };

	DWORD GetNumMarkers() { return fMarkers.size() ; };
	plSoundMarker *GetSoundMarker(int i) { return fMarkers[i]; }


	// Overloads for plAudioFileReader
	CWaveFile( const char *path, plAudioCore::ChannelSelect whichChan );
	virtual hsBool	OpenForWriting( const char *path, plWAVHeader &header );
	virtual plWAVHeader	&GetHeader( void );
	virtual void	Close( void );
	virtual UInt32	GetDataSize( void );
	virtual float	GetLengthInSecs( void );

	virtual hsBool	SetPosition( UInt32 numBytes );
	virtual hsBool	Read( UInt32 numBytes, void *buffer );
	virtual UInt32	NumBytesLeft( void );
	virtual UInt32	Write( UInt32 bytes, void *buffer );

	virtual hsBool	IsValid( void );
    WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure
    HMMIO         m_hmmio;       // MM I/O handle for the WAVE
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file
    MMIOINFO      m_mmioinfoOut;
    DWORD         m_dwFlags;
    BOOL          m_bIsReadingFromMemory;
    BYTE*         m_pbData;
    BYTE*         m_pbDataCur;
    ULONG         m_ulDataSize;
	
	plWAVHeader		fHeader;

	std::vector<plSoundMarker*>	 fMarkers;
	double		fSecsPerSample;

protected:
    HRESULT ReadMMIO();
    HRESULT WriteMMIO( WAVEFORMATEX *pwfxDest );
    HRESULT IClose();
};

#endif // plWavFile_H
