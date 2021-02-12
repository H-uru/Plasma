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
//  plWin32GroupedSound - Grouped version of a static sound. Lots of short  //
//                        sounds stored in the buffer, all share the same   //
//                        DSound playback buffer and only one plays at a    //
//                        time.                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef plWin32GroupedSound_h
#define plWin32GroupedSound_h

#include "plSoundEvent.h"
#include "plWin32StaticSound.h"

#include <vector>

class hsResMgr;
class plDSoundBuffer;
class plEventCallbackMsg;

class plWin32GroupedSound : public plWin32StaticSound
{
public:
    plWin32GroupedSound()
        : fCurrentSound(), fCurrentSoundLength(), fNumDestChannels(), fNumDestBytesPerSample()
    { }
    ~plWin32GroupedSound() { DeActivate(); }

    CLASSNAME_REGISTER( plWin32GroupedSound );
    GETINTERFACE_ANY( plWin32GroupedSound, plWin32StaticSound );
    
    bool    LoadSound(bool is3D) override;
    bool    MsgReceive(plMessage *pMsg) override;
    void    SetPositionArray(size_t numSounds, const uint32_t *posArray, const float *volumeArray);
    float   GetSoundLength(uint16_t soundIndex);
    double  GetLength() override { return GetSoundLength( fCurrentSound ); }

protected:
    uint16_t              fCurrentSound;
    uint32_t              fCurrentSoundLength;
    std::vector<uint32_t> fStartPositions;    // In bytes
    std::vector<float>    fVolumes;

    // Some extra handy info for us
    uint8_t               fNumDestChannels, fNumDestBytesPerSample;

    void    IDerivedActuallyPlay() override;

    void    IRead(hsStream *s, hsResMgr *mgr) override;
    void    IWrite(hsStream *s, hsResMgr *mgr) override;

    uint32_t          IGetSoundbyteLength(uint16_t soundIndex);
    void            IFillCurrentSound( int16_t newCurrent = -1 );
    
    // Abstracting a few things here for the incidentalMgr
    virtual void *  IGetDataPointer() const;
    virtual uint32_t  IGetDataLength() const;
};

#endif //plWin32GroupedSound_h
