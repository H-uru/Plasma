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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plOGGCodec - Plasma codec support for the OGG/Vorbis file format.       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _plOGGCodec_h
#define _plOGGCodec_h

#include "plAudioFileReader.h"
#include "plFileSystem.h"


//// Class Definition ////////////////////////////////////////////////////////

struct OggVorbis_File;
namespace ST { class string; }

class plOGGCodec : public plAudioFileReader
{
public:

    plOGGCodec( const plFileName &path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll );
    virtual ~plOGGCodec();

    enum DecodeFormat
    {
        k8bitUnsigned,
        k16bitSigned
    };

    enum DecodeFlags
    {
        kFastSeeking = 0x01
    };
    
    plWAVHeader &GetHeader() override;

    void    Close() override;

    uint32_t  GetDataSize() override { return fDataSize / fChannelAdjust; }
    float   GetLengthInSecs() override;

    bool    SetPosition(uint32_t numBytes) override;
    bool    Read(uint32_t numBytes, void *buffer) override;
    uint32_t  NumBytesLeft() override;

    bool    IsValid() override { return (fOggFile != nullptr); }

    static void     SetDecodeFormat( DecodeFormat f ) { fDecodeFormat = f; }
    static void     SetDecodeFlag( uint8_t flag, bool on ) { if( on ) fDecodeFlags |= flag; else fDecodeFlags &= ~flag; }
    static uint8_t  GetDecodeFlags() { return fDecodeFlags; }
    void            ResetWaveHeaderRef() { fCurHeaderPos = 0; }
    void            BuildActualWaveHeader();
    bool            ReadFromHeader(int numBytes, void *data); // read from Actual wave header

protected:

    enum
    {
        kPCMFormatTag = 1
    };

    plFileName      fFilename;
    FILE           *fFileHandle;
    OggVorbis_File *fOggFile;

    plWAVHeader     fHeader, fFakeHeader;
    uint32_t        fDataStartPos, fCurrDataPos, fDataSize;

    plAudioCore::ChannelSelect  fWhichChannel;
    uint32_t                    fChannelAdjust, fChannelOffset;

    static DecodeFormat fDecodeFormat;
    static uint8_t      fDecodeFlags;
    uint8_t *           fHeadBuf;
    int                 fCurHeaderPos;

    void IError(int vorbisError, const ST::string& message);
    void    IOpen( const plFileName &path, plAudioCore::ChannelSelect whichChan = plAudioCore::kAll );
};

#endif //_plOGGCodec_h
