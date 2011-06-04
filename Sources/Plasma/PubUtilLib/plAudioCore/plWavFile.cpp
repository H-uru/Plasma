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
#include <stdio.h>
#include "plWavFile.h"

#include <dxerr9.h>
#include <dsound.h>

#include <stdio.h>

#pragma comment(lib, "winmm.lib")
#ifdef PATCHER
#define DXTRACE_ERR(str,hr) hr		// I'm not linking in directx stuff to the just for this
#endif

// if it looks like I lifted this class directly from Microsoft it's because that
// is exactly what I did.  It's okay, though.  Microsoft tells you to go ahead
// and do it in the DX8 documentation.  They are SO nice.

//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.  
//       Then call Read() as needed.  Calling the destructor or Close() 
//       will close the file.  
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_dwSize  = 0;
    m_bIsReadingFromMemory = FALSE;
	fSecsPerSample = 0;

}




//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();

    if( !m_bIsReadingFromMemory )
	{
       delete[] m_pwfx;

	}

	int i;
	for(i = 0 ; i < fMarkers.size() ; i++)
	{
		delete [] fMarkers[i]->fName;
	}
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open(const char *strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    HRESULT hr;

    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

	char fileName[MAX_PATH];
	sprintf(fileName, strFileName);

#ifdef UNICODE
	wchar_t * temp = hsStringToWString(fileName);
	std::wstring wFileName = temp;
	delete [] temp;
#endif

    if( m_dwFlags == WAVEFILE_READ )
    {
        if( strFileName == NULL )
            return E_INVALIDARG;
        delete[] m_pwfx;

#ifdef UNICODE
		m_hmmio = mmioOpen( (wchar_t*)wFileName.c_str(), NULL, MMIO_ALLOCBUF | MMIO_READ );
#else
        m_hmmio = mmioOpen( fileName, NULL, MMIO_ALLOCBUF | MMIO_READ );
#endif

        if( NULL == m_hmmio )
        {
            HRSRC   hResInfo;
            HGLOBAL hResData;
            DWORD   dwSize;
            VOID*   pvRes;

            // Loading it as a file failed, so try it as a resource
#ifdef UNICODE
			if( NULL == ( hResInfo = FindResource( NULL, wFileName.c_str(), TEXT("WAVE") ) ) )
			{
				if( NULL == ( hResInfo = FindResource( NULL, wFileName.c_str(), TEXT("WAV") ) ) )
					return DXTRACE_ERR( TEXT("FindResource"), E_FAIL );
			}
#else
            if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAVE") ) ) )
            {
                if( NULL == ( hResInfo = FindResource( NULL, strFileName, TEXT("WAV") ) ) )
                    return DXTRACE_ERR( TEXT("FindResource"), E_FAIL );
            }
#endif

            if( NULL == ( hResData = LoadResource( NULL, hResInfo ) ) )
                return DXTRACE_ERR( TEXT("LoadResource"), E_FAIL );

            if( 0 == ( dwSize = SizeofResource( NULL, hResInfo ) ) ) 
                return DXTRACE_ERR( TEXT("SizeofResource"), E_FAIL );

            if( NULL == ( pvRes = LockResource( hResData ) ) )
                return DXTRACE_ERR( TEXT("LockResource"), E_FAIL );

            CHAR* pData = TRACKED_NEW CHAR[ dwSize ];
            memcpy( pData, pvRes, dwSize );

            MMIOINFO mmioInfo;
            ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
            mmioInfo.fccIOProc = FOURCC_MEM;
            mmioInfo.cchBuffer = dwSize;
            mmioInfo.pchBuffer = (CHAR*) pData;

            m_hmmio = mmioOpen( NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ );
        }

        if( FAILED( hr = ReadMMIO() ) )
        {
            // ReadMMIO will fail if its an not a wave file
            mmioClose( m_hmmio, 0 );
            return DXTRACE_ERR( TEXT("ReadMMIO"), hr );
        }

        if( FAILED( hr = ResetFile() ) )
            return DXTRACE_ERR( TEXT("ResetFile"), hr );

        // After the reset, the size of the wav file is m_ck.cksize so store it now
		
		
		
		
        m_dwSize = m_ck.cksize;

    }
    else
    {
#ifdef UNICODE
		m_hmmio = mmioOpen( (wchar_t*)wFileName.c_str(), NULL, MMIO_ALLOCBUF  | 
												  MMIO_READWRITE | 
                                                  MMIO_CREATE );
#else
        m_hmmio = mmioOpen( fileName, NULL, MMIO_ALLOCBUF  | 
                                                  MMIO_READWRITE | 
                                                  MMIO_CREATE );
#endif
        if( NULL == m_hmmio )
            return DXTRACE_ERR( TEXT("mmioOpen"), E_FAIL );

        if( FAILED( hr = WriteMMIO( pwfx ) ) )
        {
            mmioClose( m_hmmio, 0 );
            return DXTRACE_ERR( TEXT("WriteMMIO"), hr );
        }
                        
        if( FAILED( hr = ResetFile() ) )
            return DXTRACE_ERR( TEXT("ResetFile"), hr );
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::OpenFromMemory()
// Desc: copy data to CWaveFile member variable from memory
//-----------------------------------------------------------------------------
HRESULT CWaveFile::OpenFromMemory( BYTE* pbData, ULONG ulDataSize, 
                                   WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    m_pwfx       = pwfx;
    m_ulDataSize = ulDataSize;
    m_pbData     = pbData;
    m_pbDataCur  = m_pbData;
    m_bIsReadingFromMemory = TRUE;
    
    if( dwFlags != WAVEFILE_READ )
        return E_NOTIMPL;       
    
    return S_OK;
}



/*

	This defintion for a CuePoint was ripped from the internet somewhere. There are more defs at the end of this file which attempt to document 
	these wave file format extensions for storing markers.

	Cue Point--

	The dwIdentifier field contains a unique number (ie, different than the ID number of any other CuePoint structure). This is used to associate
	a CuePoint structure with other structures used in other chunks which will be described later. 

	The dwPosition field specifies the position of the cue point within the "play order" (as determined by the Playlist chunk. See that chunk for 
	a discussion of the play order).

	The fccChunk field specifies the chunk ID of the Data or Wave List chunk which actually contains the waveform data to which this CuePoint 
	refers. If there is only one Data chunk in the file, then this field is set to the ID 'data'. On the other hand, if the file contains a Wave 
	List (which can contain both 'data' and 'slnt' chunks), then fccChunk will specify 'data' or 'slnt' depending upon in which type of chunk the
	referenced waveform data is found.

	The dwChunkStart and dwBlockStart fields are set to 0 for an uncompressed WAVE file that contains one 'data' chunk. These fields are used 
	only for WAVE files that contain a Wave List (with multiple 'data' and 'slnt' chunks), or for a compressed file containing a 'data' chunk.
	(Actually, in the latter case, dwChunkStart is also set to 0, and only dwBlockStart is used). Again, I want to emphasize that you can avoid
	all of this unnecessary crap if you avoid hassling with compressed files, or Wave Lists, and instead stick to the sensible basics.

	The dwChunkStart field specifies the byte offset of the start of the 'data' or 'slnt' chunk which actually contains the waveform data to
	which this CuePoint refers. This offset is relative to the start of the first chunk within the Wave List. (ie, It's the byte offset, within
	the Wave List, of where the 'data' or 'slnt' chunk of interest appears. The first chunk within the List would be at an offset of 0).

	The dwBlockStart field specifies the byte offset of the start of the block containing the position. This offset is relative to the start of
	the waveform data within the 'data' or 'slnt' chunk.

	The dwSampleOffset field specifies the sample offset of the cue point relative to the start of the block. In an uncompressed file, this 
	equates to simply being the offset within the waveformData array. Unfortunately, the WAVE documentation is much too ambiguous, and doesn't
	define what it means by the term "sample offset". This could mean a byte offset, or it could mean counting the sample points (for example,
	in a 16-bit wave, every 2 bytes would be 1 sample point), or it could even mean sample frames (as the loop offsets in AIFF are specified).
	Who knows? The guy who conjured up the Cue chunk certainly isn't saying. I'm assuming that it's a byte offset, like the above 2 fields.
*/

class CuePoint 
{
public:
  DWORD   dwIdentifier;
  DWORD   dwPosition;
  FOURCC  fccChunk;		
  DWORD   dwChunkStart;
  DWORD   dwBlockStart;
  DWORD   dwSampleOffset;

public:
	CuePoint(DWORD id, DWORD pos, FOURCC chk, DWORD ckSt, DWORD BkSt, DWORD SO) : 
	  dwIdentifier(id), dwPosition(pos), fccChunk(chk), dwChunkStart(ckSt), dwBlockStart(BkSt), dwSampleOffset(SO)
	  {}
	CuePoint(){}
	

};



//
// this struct is used to hold cue pts temporarily while we wait for the labels that match them
//
struct myCuePoint
{
	DWORD fId;
	DWORD fOffset;
};



//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx. 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadMMIO()
{
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.       

    m_pwfx = NULL;

    if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) )
        return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );

    // Check to make sure this is a valid wave file
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
        return DXTRACE_ERR( TEXT("mmioFOURCC"), E_FAIL ); 

    // Search the input file for for the 'fmt ' chunk.
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
        return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
       if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
           return DXTRACE_ERR( TEXT("sizeof(PCMWAVEFORMAT)"), E_FAIL );

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat, 
                  sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
        return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );

    // Allocate the waveformatex, but if its not pcm format, read the next
    // word, and thats how many extra bytes to allocate.
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
    {
        m_pwfx = (WAVEFORMATEX*)( TRACKED_NEW CHAR[ sizeof( WAVEFORMATEX ) ] );
        if( NULL == m_pwfx )
            return DXTRACE_ERR( TEXT("m_pwfx"), E_FAIL );

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    }
    else
    {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
            return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );

        m_pwfx = (WAVEFORMATEX*)( TRACKED_NEW CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ] );
        if( NULL == m_pwfx )
            return DXTRACE_ERR( TEXT("new"), E_FAIL );

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),
                      cbExtraBytes ) != cbExtraBytes )
        {
            delete m_pwfx;
            return DXTRACE_ERR( TEXT("mmioRead"), E_FAIL );
        }
    }



	fSecsPerSample = 1.0/ (double)(pcmWaveFormat.wf.nSamplesPerSec) ; // * (((double)pcmWaveFormat.wBitsPerSample)/8.0);


	// Ascend the input file out of the 'fmt ' chunk.
    if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        delete m_pwfx;
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    }



	//
	// Here is where we attempt to parse sound indicies from the file for loop points.
	// If there is no cue chunk then we just return OK
	//

    ckIn.ckid = mmioFOURCC('c', 'u', 'e', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
        return S_OK; // No Cue Chunck so no point in reading the rest

#if 0
   // Expect the 'cue ' chunk to be at least as large as <PCMWAVEFORMAT>;
   // if there are extra parameters at the end, we'll ignore them
      if( ckIn.cksize < (long) ( sizeof(FOURCC) + 2*(sizeof(DWORD)) + sizeof(CuePoint) ) )
         return DXTRACE_ERR( TEXT("sizeof(CueChunk)"), E_FAIL );
#endif

	DWORD* CueBuff = TRACKED_NEW DWORD[ckIn.cksize];
	DWORD Results;
	Read((BYTE*)CueBuff, ckIn.cksize, &Results);

	std::vector<myCuePoint> myCueList; // Place to hold the cue points

	int numCuePoints = (ckIn.cksize - sizeof(DWORD))/sizeof(CuePoint); // this is how many there should be.
	unsigned int i, j;	
				
	for (i = 1, j = 0; i <= ckIn.cksize && j < numCuePoints ; i += sizeof(CuePoint)/(sizeof(DWORD)), j++)

	{
		myCuePoint p;
		p.fId = CueBuff[i];			// dwIdentifier
		p.fOffset = CueBuff[i+1];	// dwPosition
		myCueList.push_back(p);
	}

	delete[] CueBuff;

	if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        delete m_pwfx;
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    }

	

	// Goal --> Grab the label information below
    ckIn.ckid = mmioFOURCC('a', 'd', 't', 'l');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDLIST ) )
        return DXTRACE_ERR( TEXT("mmioDescend, with list"), E_FAIL );


	plSoundMarker *newMarker;

	BYTE *labelBuf = TRACKED_NEW BYTE [ckIn.cksize];

	// Read the entire lable chunk and then lets parse out the individual lables.
	Read(labelBuf, ckIn.cksize-4, &Results);

	BYTE *bp = labelBuf;
	// Keep looking for labl chunks till we run out.
	while(!strncmp("labl",(char*)bp,4))
	{
		DWORD size = *(DWORD*)(bp + 4);
		DWORD id = *(DWORD*)(bp + 8);
		newMarker = TRACKED_NEW plSoundMarker; // Grab a new label
	
		int i;

		int numPts = myCueList.size();
		//
		// Do we have a matching cue point for this label?
		//
		for(i = 0 ; i < numPts;  i++)
		{
			if(id == myCueList[i].fId)
			{
				newMarker->fOffset = myCueList[i].fOffset * fSecsPerSample;
			}
		}	
		int stringSize = size - sizeof(DWORD); // text string is size of chunck - size of the size word
		newMarker->fName = TRACKED_NEW char[ stringSize];

		strcpy(newMarker->fName, (char*)(bp + 12));
		
		fMarkers.push_back(newMarker);
		bp += size + 8; 

		// crappy fixup hack for odd length label records
		if(size & 1 && !strncmp("labl", (char*)(bp +1), 4))
			bp++;
	 
		fprintf(stderr,"Label name=%s Time =%f\n",newMarker->fName, newMarker->fOffset);
	}

	delete [] labelBuf;

	if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        delete m_pwfx;
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    }


	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::GetSize()
// Desc: Retuns the size of the read access wave file 
//-----------------------------------------------------------------------------
DWORD CWaveFile::GetSize()
{
    return m_dwSize;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the 
//       beginning of the file again 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile()
{
    if( m_bIsReadingFromMemory )
    {
        m_pbDataCur = m_pbData;
    }
    else 
    {
        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( m_dwFlags == WAVEFILE_READ )
        {
            // Seek to the data
            if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
                            SEEK_SET ) )
                return DXTRACE_ERR( TEXT("mmioSeek"), E_FAIL );

            // Search the input file for the 'data' chunk.
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
              return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );
        }
        else
        {
            // Create the 'data' chunk that holds the waveform samples.  
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            m_ck.cksize = 0;

            if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) ) 
                return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );

            if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
                return DXTRACE_ERR( TEXT("mmioGetInfo"), E_FAIL );
        }
    }
    
    return S_OK;
}


#define	MCN_USE_NEW_READ_METHOD	0

//-----------------------------------------------------------------------------
// Name: CWaveFile::Read()
// Desc: Reads section of data from a wave file into pBuffer and returns 
//       how much read in pdwSizeRead, reading not more than dwSizeToRead.
//       This uses m_ck to determine where to start reading from.  So 
//       subsequent calls will be continue where the last left off unless 
//       Reset() is called.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Read( BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead )
{
    if( m_bIsReadingFromMemory )
    {
        if( m_pbDataCur == NULL )
            return CO_E_NOTINITIALIZED;
        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( (BYTE*)(m_pbDataCur + dwSizeToRead) > 
            (BYTE*)(m_pbData + m_ulDataSize) )
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }
        
        CopyMemory( pBuffer, m_pbDataCur, dwSizeToRead );
        
        if( pdwSizeRead != NULL )
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else 
    {
        MMIOINFO mmioinfoIn; // current status of m_hmmio

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;
        if( pBuffer == NULL || pdwSizeRead == NULL )
            return E_INVALIDARG;

        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return DXTRACE_ERR( TEXT("mmioGetInfo"), E_FAIL );
                
        UINT cbDataIn = dwSizeToRead;
        if( cbDataIn > m_ck.cksize ) 
            cbDataIn = m_ck.cksize;       

        m_ck.cksize -= cbDataIn;
    
#if !(MCN_USE_NEW_READ_METHOD)
        for( DWORD cT = 0; cT < cbDataIn; cT++ )
        {
            // Copy the bytes from the io to the buffer.
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
                    return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
                    return DXTRACE_ERR( TEXT("mmioinfoIn.pchNext"), E_FAIL );
            }

            // Actual copy.
            *((BYTE*)pBuffer+cT) = *((BYTE*)mmioinfoIn.pchNext);
            mmioinfoIn.pchNext++;
        }
#else
		// Attempt to do this a bit faster... 9.12.2001 mcn
        for( DWORD cT = 0; cT < cbDataIn; )
        {
            // Copy the bytes from the io to the buffer.
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
                    return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
                    return DXTRACE_ERR( TEXT("mmioinfoIn.pchNext"), E_FAIL );
            }

            // Actual copy
			DWORD	length = (DWORD)mmioinfoIn.pchEndRead - (DWORD)mmioinfoIn.pchNext;
			if( cT + length > cbDataIn )
				length = cbDataIn - cT;

			memcpy( (BYTE*)pBuffer + cT, mmioinfoIn.pchNext, length );
			mmioinfoIn.pchNext += length;
			cT += length;
        }
#endif

        if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return DXTRACE_ERR( TEXT("mmioSetInfo"), E_FAIL );

        if( pdwSizeRead != NULL )
            *pdwSizeRead = cbDataIn;

        return S_OK;
    }
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::AdvanceWithoutRead()
// Desc: Identical to Read(), only doesn't actually read any data in.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::AdvanceWithoutRead( DWORD dwSizeToRead, DWORD* pdwSizeRead )
{
    if( m_bIsReadingFromMemory )
    {
        if( m_pbDataCur == NULL )
            return CO_E_NOTINITIALIZED;
        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( (BYTE*)(m_pbDataCur + dwSizeToRead) > 
            (BYTE*)(m_pbData + m_ulDataSize) )
        {
            dwSizeToRead = m_ulDataSize - (DWORD)(m_pbDataCur - m_pbData);
        }
        
        if( pdwSizeRead != NULL )
            *pdwSizeRead = dwSizeToRead;

        return S_OK;
    }
    else 
    {
        MMIOINFO mmioinfoIn; // current status of m_hmmio

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;
        if( pdwSizeRead == NULL )
            return E_INVALIDARG;

        if( pdwSizeRead != NULL )
            *pdwSizeRead = 0;

        if( 0 != mmioGetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return DXTRACE_ERR( TEXT("mmioGetInfo"), E_FAIL );
                
        UINT cbDataIn = dwSizeToRead;
        if( cbDataIn > m_ck.cksize ) 
            cbDataIn = m_ck.cksize;       

        m_ck.cksize -= cbDataIn;
    
#if !(MCN_USE_NEW_READ_METHOD)
        for( DWORD cT = 0; cT < cbDataIn; cT++ )
        {
            // Copy the bytes from the io to the buffer.
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
                    return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
                    return DXTRACE_ERR( TEXT("mmioinfoIn.pchNext"), E_FAIL );
            }

            mmioinfoIn.pchNext++;
        }
#else
		// Attempt to do this a bit faster... 9.12.2001 mcn
        for( DWORD cT = 0; cT < cbDataIn; )
        {
            if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
            {
                if( 0 != mmioAdvance( m_hmmio, &mmioinfoIn, MMIO_READ ) )
                    return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );

                if( mmioinfoIn.pchNext == mmioinfoIn.pchEndRead )
                    return DXTRACE_ERR( TEXT("mmioinfoIn.pchNext"), E_FAIL );
            }

            // Advance
			DWORD	length = (DWORD)mmioinfoIn.pchEndRead - (DWORD)mmioinfoIn.pchNext;
			if( cT + length > cbDataIn )
				length = cbDataIn - cT;

			mmioinfoIn.pchNext += length;
			cT += length;
        }
#endif

        if( 0 != mmioSetInfo( m_hmmio, &mmioinfoIn, 0 ) )
            return DXTRACE_ERR( TEXT("mmioSetInfo"), E_FAIL );

        if( pdwSizeRead != NULL )
            *pdwSizeRead = cbDataIn;

        return S_OK;
    }
}


//-----------------------------------------------------------------------------
// Name: CWaveFile::IClose()
// Desc: Closes the wave file 
//-----------------------------------------------------------------------------
HRESULT CWaveFile::IClose()
{
    if( m_dwFlags == WAVEFILE_READ )
    {
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }
    else
    {
        m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
            return DXTRACE_ERR( TEXT("mmioSetInfo"), E_FAIL );
    
        // Ascend the output file out of the 'data' chunk -- this will cause
        // the chunk size of the 'data' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
            return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    
        // Do this here instead...
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
        
        mmioSeek( m_hmmio, 0, SEEK_SET );

        if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
            return DXTRACE_ERR( TEXT("mmioDescend"), E_FAIL );
    
        m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) ) 
        {
            DWORD dwSamples = 0;
            mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(DWORD) );
            mmioAscend( m_hmmio, &m_ck, 0 ); 
        }
    
        // Ascend the output file out of the 'RIFF' chunk -- this will cause
        // the chunk size of the 'RIFF' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.  
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.  
//-----------------------------------------------------------------------------
HRESULT CWaveFile::WriteMMIO( WAVEFORMATEX *pwfxDest )
{
    DWORD    dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
    MMCKINFO ckOut1;
    
    dwFactChunk = (DWORD)-1;

    // Create the output file RIFF chunk of form type 'WAVE'.
    m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');       
    m_ckRiff.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) )
        return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );
    
    // We are now descended into the 'RIFF' chunk we just created.
    // Now create the 'fmt ' chunk. Since we know the size of this chunk,
    // specify it in the MMCKINFO structure so MMIO doesn't have to seek
    // back and set the chunk size after ascending from the chunk.
    m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    m_ck.cksize = sizeof(PCMWAVEFORMAT);   

    if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
        return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );
    
    // Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type. 
    if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
    {
        if( mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                       sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
            return DXTRACE_ERR( TEXT("mmioWrite"), E_FAIL );
    }   
    else 
    {
        // Write the variable length size.
        if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest, 
                             sizeof(*pwfxDest) + pwfxDest->cbSize ) != 
                             ( sizeof(*pwfxDest) + pwfxDest->cbSize ) )
            return DXTRACE_ERR( TEXT("mmioWrite"), E_FAIL );
    }  
    
    // Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
    
    // Now create the fact chunk, not required for PCM but nice to have.  This is filled
    // in when the close routine is called.
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) )
        return DXTRACE_ERR( TEXT("mmioCreateChunk"), E_FAIL );
    
    if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) != 
                    sizeof(dwFactChunk) )
         return DXTRACE_ERR( TEXT("mmioWrite"), E_FAIL );
    
    // Now ascend out of the fact chunk...
    if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) )
        return DXTRACE_ERR( TEXT("mmioAscend"), E_FAIL );
       
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CWaveFile::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Write( UINT nSizeToWrite, BYTE* pbSrcData, UINT* pnSizeWrote )
{
    UINT cT;

    if( m_bIsReadingFromMemory )
        return E_NOTIMPL;
    if( m_hmmio == NULL )
        return CO_E_NOTINITIALIZED;
    if( pnSizeWrote == NULL || pbSrcData == NULL )
        return E_INVALIDARG;

    *pnSizeWrote = 0;
    
    for( cT = 0; cT < nSizeToWrite; cT++ )
    {       
        if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite )
        {
            m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
            if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) )
                return DXTRACE_ERR( TEXT("mmioAdvance"), E_FAIL );
        }

        *((BYTE*)m_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData+cT);
        (BYTE*)m_mmioinfoOut.pchNext++;

        (*pnSizeWrote)++;
    }

    return S_OK;
}


// Overloads for plAudioFileReader (we only support writing for CWaveFile for now)
CWaveFile::CWaveFile( const char *path, plAudioCore::ChannelSelect whichChan )
{
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_dwSize  = 0;
    m_bIsReadingFromMemory = FALSE;
	fSecsPerSample = 0;

	// Just a stub--do nothing
}

hsBool	CWaveFile::OpenForWriting( const char *path, plWAVHeader &header )
{
	fHeader = header;

	WAVEFORMATEX	winFormat;
	winFormat.cbSize = 0;
	winFormat.nAvgBytesPerSec = header.fAvgBytesPerSec;
	winFormat.nBlockAlign = header.fBlockAlign;
	winFormat.nChannels = header.fNumChannels;
	winFormat.nSamplesPerSec = header.fNumSamplesPerSec;
	winFormat.wBitsPerSample = header.fBitsPerSample;
	winFormat.wFormatTag = header.fFormatTag;

	if( SUCCEEDED( Open( path, &winFormat, WAVEFILE_WRITE ) ) )
		return true;

	return false;
}

plWAVHeader	&CWaveFile::GetHeader( void )
{
	return fHeader;
}

void	CWaveFile::Close( void )
{
	IClose();
}

UInt32	CWaveFile::GetDataSize( void )
{
	hsAssert( false, "Unsupported" );
	return 0;
}

float	CWaveFile::GetLengthInSecs( void )
{
	hsAssert( false, "Unsupported" );
	return 0.f;
}

hsBool	CWaveFile::SetPosition( UInt32 numBytes )
{
	hsAssert( false, "Unsupported" );
	return false;
}

hsBool	CWaveFile::Read( UInt32 numBytes, void *buffer )
{
	hsAssert( false, "Unsupported" );
	return false;
}

UInt32	CWaveFile::NumBytesLeft( void )
{
	hsAssert( false, "Unsupported" );
	return 0;
}

UInt32	CWaveFile::Write( UInt32 bytes, void *buffer )
{
	UINT written;
	Write( (DWORD)bytes, (BYTE *)buffer, &written );
	return (UInt32)written;
}

hsBool	CWaveFile::IsValid( void )
{
	return true;
}


#if 0

THIS IS MORE STUFF having to do with WAV FILE format. It is just sitting here for documentation purposes.
/*
	Cue Chunk--

	The ID is always 'cue '. chunkSize is the number of bytes in the chunk, not counting the 8 bytes used by ID and Size fields. 
	The dwCuePoints field is the number of CuePoint structures in the Cue Chunk. If dwCuePoints is not 0, it is followed by that many 
	CuePoint structures, one after the other. Because all fields in a CuePoint structure are an even number of bytes, the length of any 
	CuePoint will always be even. Thus, CuePoints are packed together with no unused bytes between them. The CuePoints need not be placed 
	in any particular order.

	The Cue chunk is optional. No more than one Cue chunk can appear in a WAVE.
*/

class CueChunk 
{
 
public:
  FOURCC					chunkID;
  DWORD						chunkSize;
  DWORD						dwCuePoints;
  std::vector<CuePoint*>	points;

public:
	CueChunk(DWORD ChunkSize)
	{
		chunkID = mmioFOURCC('c','u','e',' ');
		chunkSize = ChunkSize;
		dwCuePoints = (ChunkSize - (sizeof(DWORD)*1))/(sizeof(CuePoint));
		//points = NULL;
		//points = TRACKED_NEW CuePoint[dwCuePoints];
		
	}
	//Cue

	~CueChunk() {} //for(int i = 0; i < (int) dwCuePoints; i++) points.erase(i);  }
};


/*
	LabelChunk--

	The ID is always 'labl'. chunkSize is the number of bytes in the chunk, not counting the 8 bytes used by ID and Size fields nor any possible 
	pad byte needed to make the chunk an even size (ie, chunkSize is the number of remaining bytes in the chunk after the chunkSize field, not
	counting any trailing pad byte). 
	The dwIdentifier field contains a unique number (ie, different than the ID number of any other Label chunk). This field should correspond 
	with the dwIndentifier field of some CuePoint stored in the Cue chunk. In other words, this Label chunk contains the text label associated
	with that CuePoint structure with the same ID number.

	The dwText array contains the text label. It should be a null-terminated string. (The null byte is included in the chunkSize, therefore the 
	length of the string, including the null byte, is chunkSize - 4).
*/


class LabelChunk
{

public:
  FOURCC     chunkID;
  DWORD		chunkSize;

  DWORD    dwIdentifier;
  char*    dwText;
public:
	LabelChunk(DWORD ChunkSize) 
	{
		chunkID = mmioFOURCC('l','a','b','l');
		chunkSize = ChunkSize;
		dwIdentifier = 0;
		dwText = NULL;
	}

	LabelChunk() 
	{
		chunkID = mmioFOURCC('l','a','b','l');
		chunkSize = 0;
		dwIdentifier = 0;
		dwText = NULL;
	}


	~LabelChunk() { delete[] dwText; }

};

#endif
