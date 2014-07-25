#include <iostream>
#include <fstream>

#include "hsResMgr.h"
#include "plFormat.h"

#include "plDSoundBuffer.h"

#include "plWin32VideoSound.h"

static int uniqueID = 0;
plWin32VideoSound::plWin32VideoSound() : plWin32Sound()
{
    plWAVHeader header;
    header.fFormatTag = 0x1;
    header.fNumChannels = 1;
    header.fBitsPerSample = 16;
    fDSoundBuffer = new plDSoundBuffer(0, header, false, false, false, true);
    fDSoundBuffer->SetupVoiceSource();
    uniqueID++;
    hsgResMgr::ResMgr()->NewKey(plFormat("videosound#{}", uniqueID), this, plLocation::kGlobalFixedLoc);
    fSoftVolume = 1.0f;
}

plWin32VideoSound::~plWin32VideoSound()
{
    delete fDSoundBuffer;
}

void plWin32VideoSound::UpdateSoundBuffer(unsigned char* buffer, size_t size)
{
    unsigned int bufferID = 0;

    if (fDSoundBuffer->GetAvailableBufferId(&bufferID))
    {
        fDSoundBuffer->VoiceFillBuffer(buffer, size, bufferID);
        IActuallyPlay();
    }
}

void plWin32VideoSound::IDerivedActuallyPlay()
{
    if (!fDSoundBuffer->IsPlaying())
        fDSoundBuffer->Play();
}

bool plWin32VideoSound::LoadSound(bool is3D)
{
    hsAssert(false, "unimplemented cause unnecessary for this class");
    return false;
}

void plWin32VideoSound::SetStartPos(unsigned bytes)
{
    
    //do nothing
    hsAssert(false, "unimplemented cause unnecessary for this class");
}

float plWin32VideoSound::GetActualTimeSec()
{
    hsAssert(false, "unimplemented cause unnecessary for this class");
    return 0;
}

void plWin32VideoSound::ISetActualTime(double t)
{
    hsAssert(false, "unimplemented cause unnecessary for this class");
}