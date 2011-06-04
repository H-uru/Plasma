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
#include "hsWindows.h"
#include "hsTimer.h"
#include "hsResMgr.h"
#include "al.h"
#include "alc.h"
#include "plDSoundBuffer.h"
#include "speex.h"
#include "speex_bits.h"
#include "hsGeometry3.h"
#include "plVoiceChat.h"
#include "plAudioSystem.h"
#include "plgDispatch.h"
#include "../plAudible/plWinAudible.h"
#include "../plNetMessage/plNetMessage.h"
#include "../plPipeline/plPlates.h"
#include "hsConfig.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"
#include "hsQuat.h"
#include "../plAudioCore/plAudioCore.h"

// DEBUG for printing to the console
#include "../plMessage/plConsoleMsg.h"
#include "../plPipeline/plDebugText.h"
#include "../plStatusLog/plStatusLog.h"

#define MICROPHONE			121
#define TALKING				122
#define NUM_CHANNELS		1
#define VOICE_STOP_MS		2000
#define MAX_DATA_SIZE		1024 * 4	// 4 KB

hsBool					plVoiceRecorder::fCompress =				true;
hsBool					plVoiceRecorder::fRecording =				true;
hsBool					plVoiceRecorder::fNetVoice =				false;
short					plVoiceRecorder::fSampleRate =				FREQUENCY;
hsScalar				plVoiceRecorder::fRecordThreshhold =		200.0f;
hsBool					plVoiceRecorder::fShowIcons =				true;
hsBool					plVoiceRecorder::fMicAlwaysOpen =			false;
hsBool					plVoicePlayer::fEnabled	=					true;

plVoiceRecorder::plVoiceRecorder()
{
	plPlateManager::Instance().CreatePlate( &fDisabledIcon );
	fDisabledIcon->CreateFromResource( MAKEINTRESOURCE( MICROPHONE ) );
	fDisabledIcon->SetPosition(-0.90, -0.90);
	fDisabledIcon->SetSize(0.0675, 0.09);
	fDisabledIcon->SetVisible(false);

	plPlateManager::Instance().CreatePlate( &fTalkIcon );
	fTalkIcon->CreateFromResource( MAKEINTRESOURCE( TALKING ) );
	fTalkIcon->SetPosition(-0.9,-0.9);
	fTalkIcon->SetSize(0.0675, 0.09);
	fTalkIcon->SetVisible(false);
}

plVoiceRecorder::~plVoiceRecorder()
{
	if(fDisabledIcon)
		plPlateManager::Instance().DestroyPlate( fDisabledIcon);
	fDisabledIcon = nil;
	
	if (fTalkIcon)
		plPlateManager::Instance().DestroyPlate( fTalkIcon );
	fTalkIcon = nil;
}

void plVoiceRecorder::IncreaseRecordingThreshhold()
{
	fRecordThreshhold += (100 * hsTimer::GetDelSysSeconds());
	if (fRecordThreshhold >= 10000.0f)
		fRecordThreshhold = 10000.0f;
	
	plDebugText	&txt = plDebugText::Instance();
	char str[256];
	sprintf(str, "RecordThreshhold %f\n", fRecordThreshhold);
	txt.DrawString(400,300,str);
}

void plVoiceRecorder::DecreaseRecordingThreshhold()
{
	fRecordThreshhold -= (100 * hsTimer::GetDelSysSeconds());
	if (fRecordThreshhold <= 50.0f)
		fRecordThreshhold = 50.0f;
	
	plDebugText	&txt = plDebugText::Instance();
	char str[256];
	sprintf(str, "RecordThreshhold %f\n", fRecordThreshhold);
	txt.DrawString(400,300,str);
}

// Set the quality of speex encoder
void plVoiceRecorder::SetQuality(int quality)
{
	char str[] = "Voice quality setting out of range. Must be between 1 and 10 inclusive";
	if(quality < 1 || quality > 10)
	{
		plConsoleMsg	*cMsg = TRACKED_NEW plConsoleMsg( plConsoleMsg::kAddLine, str );
		plgDispatch::MsgSend( cMsg );
		return;
	}

	if(plSpeex::GetInstance()->IsUsingVBR())
	{
		// Sets average bit rate between 4kb and 13kb
		int AverageBitrate = quality * 1000 + 3000;
		plSpeex::GetInstance()->SetABR(AverageBitrate);
	}
	else
	{
		plSpeex::GetInstance()->SetQuality(quality);		
	}
}

// toggle variable bit rate
void plVoiceRecorder::SetVBR(bool vbr)
{
	plSpeex::GetInstance()->VBR(vbr);
	SetQuality(plSpeex::GetInstance()->GetQuality());		// update proper quality param
}

void plVoiceRecorder::SetComplexity(int c)
{
	char str[] = "Voice quality setting out of range. Must be between 1 and 10 inclusive";
	if(c < 1 || c > 10)
	{
		plConsoleMsg	*cMsg = TRACKED_NEW plConsoleMsg( plConsoleMsg::kAddLine, str );
		plgDispatch::MsgSend( cMsg );
		return;
	}
	plSpeex::GetInstance()->SetComplexity((UInt8) c);
}

void plVoiceRecorder::SetENH(hsBool b)
{
	plSpeex::GetInstance()->SetENH(b);
}

void plVoiceRecorder::SetMikeOpen(hsBool b)
{
	ALCdevice *device = plgAudioSys::GetCaptureDevice();
	if (fRecording && device)
	{		
		if (b)
		{
			alcCaptureStart(device);
		}
		else
		{
			alcCaptureStop(device);
		}
		DrawTalkIcon(b);
		fMikeOpen = b;
	}
	else
	{
		DrawDisabledIcon(b);		// voice recording is unavailable or disabled
	}
}

void plVoiceRecorder::DrawDisabledIcon(hsBool b)
{
	if (!fDisabledIcon)
	{
		// at least try and make one here...
		plPlateManager::Instance().CreatePlate( &fDisabledIcon );
		if (fDisabledIcon)
		{
			fDisabledIcon->CreateFromResource( MAKEINTRESOURCE( MICROPHONE ) );
			fDisabledIcon->SetPosition(-0.90, -0.90);
			fDisabledIcon->SetSize(0.0675, 0.09);
			fDisabledIcon->SetVisible(false);
		}
	}

	if (fDisabledIcon)
		fDisabledIcon->SetVisible(b);
}

void plVoiceRecorder::DrawTalkIcon(hsBool b)
{
	if (!fTalkIcon)
	{	
		plPlateManager::Instance().CreatePlate( &fTalkIcon );
		if (fTalkIcon)
		{	fTalkIcon->CreateFromResource( MAKEINTRESOURCE( TALKING ) );
			fTalkIcon->SetPosition(-0.9,-0.9);
			fTalkIcon->SetSize(0.0675, 0.09);
			fTalkIcon->SetVisible(false);
		}	
	}

	if (fTalkIcon)
	{
		fTalkIcon->SetVisible(b);
	}
}

void plVoiceRecorder::Update(double time)
{	
	if(!fRecording)
		return;

	int EncoderFrameSize = plSpeex::GetInstance()->GetFrameSize();
	if(EncoderFrameSize == -1) 
		return;

	ALCdevice *captureDevice = plgAudioSys::GetCaptureDevice();
	if(!captureDevice)
		return;

	unsigned minSamples = EncoderFrameSize * 10;

	ALCint samples;
	alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, sizeof(samples), &samples );
	
	if (samples > 0)
	{
		if (samples >= minSamples)
		{
			int numFrames = (int)(samples / EncoderFrameSize);		// the number of frames that have been captured
			int totalSamples = numFrames * EncoderFrameSize;

			// cap uncompressed data
			if(totalSamples > MAX_DATA_SIZE)
				totalSamples = MAX_DATA_SIZE;

			// convert to correct units:
			short *buffer = TRACKED_NEW short[totalSamples];

			alcCaptureSamples(captureDevice, buffer, totalSamples);

			if (!CompressionEnabled())
			{
				plNetMsgVoice pMsg;
				pMsg.SetNetProtocol(kNetProtocolCli2Game);
				pMsg.SetVoiceData((char *)buffer, totalSamples * sizeof(short));
				// set frame size here;
				pMsg.SetPlayerID(plNetClientApp::GetInstance()->GetPlayerID());
				//if (false) //plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kEchoVoice))
				//	pMsg.SetBit(plNetMessage::kEchoBackToSender);
				plNetClientApp::GetInstance()->SendMsg(&pMsg);
			
			}
			else  // use the speex voice compression lib
			{
				UInt8 *packet = TRACKED_NEW UInt8[totalSamples];	  // packet to send encoded data in
				int packedLength = 0;									  // the size of the packet that will be sent
				hsRAMStream ram;										  // ram stream to hold output data from speex
				UInt8 numFrames = totalSamples / EncoderFrameSize;		  // number of frames to be encoded
				
				// encode the data using speex
				plSpeex::GetInstance()->Encode(buffer, numFrames, &packedLength, &ram);

				if (packedLength)
				{
					// extract data from ram stream into packet
					ram.Rewind();
					ram.Read(packedLength, packet);
					plNetMsgVoice pMsg;
					pMsg.SetNetProtocol(kNetProtocolCli2Game);

					pMsg.SetVoiceData((char *)packet, packedLength);
					pMsg.SetPlayerID(plNetClientApp::GetInstance()->GetPlayerID());
					pMsg.SetFlag(VOICE_ENCODED);	// Set encoded flag
					pMsg.SetNumFrames(numFrames);
					if (plNetClientApp::GetInstance()->GetFlagsBit(plNetClientApp::kEchoVoice))
						pMsg.SetBit(plNetMessage::kEchoBackToSender);

					plNetClientApp::GetInstance()->SendMsg(&pMsg);
				}
				delete[] packet;
			}
			delete[] buffer;
		}
		else if(!fMikeOpen)
		{
			short *buffer = TRACKED_NEW short[samples];
			// the mike has since closed, and there isn't enough data to meet our minimum, so throw this data out
			alcCaptureSamples(captureDevice, buffer, samples);		
			delete[] buffer;
		}
	}
}

plVoicePlayer::plVoicePlayer()
{
}

plVoicePlayer::~plVoicePlayer()
{
}

void plVoicePlayer::PlaybackUncompressedVoiceMessage(void* data, unsigned size)
{	
	if(fEnabled)
	{
		if(!fSound.IsPlaying())
		{
			fSound.Play();		
		}
		fSound.AddVoiceData(data, size);
	}
}

void plVoicePlayer::PlaybackVoiceMessage(void* data, unsigned size, int numFramesInBuffer)
{
	if(fEnabled)
	{
		int numBytes;				// the number of bytes that speex decompressed the data to.	
		int bufferSize = numFramesInBuffer * plSpeex::GetInstance()->GetFrameSize();
		short *nBuff = TRACKED_NEW short[bufferSize];
		memset(nBuff, 0, bufferSize);

		// Decode the encoded voice data using speex
		if(!plSpeex::GetInstance()->Decode((UInt8 *)data, size, numFramesInBuffer, &numBytes, nBuff))
		{
			delete[] nBuff;
			return;
		}
		
		BYTE* newBuff;
		newBuff = (BYTE*)nBuff;			// Convert to byte data
		PlaybackUncompressedVoiceMessage(newBuff, numBytes);	// playback uncompressed data
		delete[] nBuff;
	}
}

void plVoicePlayer::SetVelocity(const hsVector3 vel)
{
	fSound.SetVelocity(vel);
}
	 
void plVoicePlayer::SetPosition(const hsPoint3 pos)
{
	fSound.SetPosition(pos);
}
	
void plVoicePlayer::SetOrientation(const hsPoint3 pos)
{
	fSound.SetConeOrientation(pos.fX, pos.fY, pos.fZ);
}


/*****************************************************************************
*
*   plVoiceSound
*
***/
unsigned plVoiceSound::fCount = 0;

plVoiceSound::plVoiceSound() 
{
	fInnerCone = 90;
	fOuterCone = 240;
	fOuterVol = -2000;
	
	fMinFalloff = 15;
	fMaxFalloff = 75;

	fProperties = 0;
	fCurrVolume = 1.0;
	fDesiredVol = 1.0;

	fPriority = 1;
	fType = plgAudioSys::kVoice;

	fEAXSettings.SetRoomParams(-1200, -100, 0, 0);
	fLastUpdate = 0;

	char keyName[32];
	StrPrintf(keyName, arrsize(keyName), "VoiceSound_%d", fCount);
	fCount++;
	hsgResMgr::ResMgr()->NewKey(keyName, this, plLocation::kGlobalFixedLoc);
}

plVoiceSound::~plVoiceSound()
{
}

hsBool plVoiceSound::LoadSound( hsBool is3D )
{
	if( fFailed )
		return false;
	if( !plgAudioSys::Active() || fDSoundBuffer )
		return false;

	if( fPriority > plgAudioSys::GetPriorityCutoff() )
		return false;	// Don't set the failed flag, just return

	plWAVHeader	header;
	header.fFormatTag = WAVE_FORMAT_PCM;
	header.fBitsPerSample  = 16;
	header.fNumChannels = 1;
	header.fNumSamplesPerSec = FREQUENCY;
	header.fBlockAlign = header.fNumChannels * header.fBitsPerSample / 2;
	header.fAvgBytesPerSec = header.fNumSamplesPerSec * header.fBlockAlign;

	fDSoundBuffer = TRACKED_NEW plDSoundBuffer(0, header, true, false, false, true);
	if(!fDSoundBuffer)
		return false;
	fDSoundBuffer->SetupVoiceSource();
	
	IRefreshParams();
	IRefreshEAXSettings( true );
	fDSoundBuffer->SetScalarVolume(1.0);
	return true;
}

void plVoiceSound::Play()
{
	fPlaying = true;
	if( IWillBeAbleToPlay() )
	{
		IRefreshParams();
		SetVolume( fDesiredVol );
		IActuallyPlay();
	}
}

void plVoiceSound::IDerivedActuallyPlay( void )
{
	if( !fReallyPlaying )
	{
		fDSoundBuffer->Play(); 
		fReallyPlaying = true;
	}
}

void plVoiceSound::AddVoiceData(void *data, unsigned bytes)
{	
	unsigned size;
	unsigned bufferId;
	if(!fDSoundBuffer)
	{
		if(!LoadSound(true))
		{
			return;
		}
	}
	
	fDSoundBuffer->UnQueueVoiceBuffers();		// attempt to unque any buffers that have finished
	while(bytes > 0)
	{
		size = bytes < STREAM_BUFFER_SIZE ? bytes : STREAM_BUFFER_SIZE;
		if(!fDSoundBuffer->GetAvailableBufferId(&bufferId))
			break;		// if there isn't any room for the data, it is currently thrown out 

		fDSoundBuffer->VoiceFillBuffer(data, size, bufferId);
		bytes -= size;
	}
	fLastUpdate = hsTimer::GetMilliSeconds();
}

void plVoiceSound::Update()
{
	if(IsPlaying())
	{
		if((hsTimer::GetMilliSeconds() - fLastUpdate) > VOICE_STOP_MS)
		{
			Stop();	// terminating case for playback. Wait for x number of milliseconds, and stop.
		}
	}
}

void plVoiceSound::IRefreshParams()
{
	plSound::IRefreshParams();
}


/*****************************************************************************
*
*   Speex Voice Encoding/Decoding
*
***/

plSpeex::plSpeex() :
fBits(nil),
fEncoderState(nil),
fDecoderState(nil),
fSampleRate(plVoiceRecorder::GetSampleRate()),
fFrameSize(-1),
fQuality(7),
fVBR(true),					// variable bit rate on		
fAverageBitrate(8000),		// 8kb bitrate
fComplexity(3),
fENH(false),
fInitialized(false)
{
	fBits = TRACKED_NEW SpeexBits;
	Init(kNarrowband);		// if no one initialized us initialize using a narrowband encoder
}

plSpeex::~plSpeex()
{
	Shutdown();
	delete fBits;
	fBits = nil;
}
	
hsBool plSpeex::Init(Mode mode) 
{
	int enh = 1;
	
	// setup speex
	speex_bits_init(fBits);
	fBitsInit = true;

	if(mode == kNarrowband)
	{
		fEncoderState = speex_encoder_init(&speex_nb_mode);							// narrowband
		fDecoderState = speex_decoder_init(&speex_nb_mode);
	}
	else if(mode == kWideband)
	{
		fEncoderState = speex_encoder_init(&speex_wb_mode);
		fDecoderState = speex_decoder_init(&speex_wb_mode);
	}
	
	speex_encoder_ctl(fEncoderState, SPEEX_GET_FRAME_SIZE, &fFrameSize);			// get frame size
    speex_encoder_ctl(fEncoderState, SPEEX_SET_COMPLEXITY, &fComplexity);			// 3
    speex_encoder_ctl(fEncoderState, SPEEX_SET_SAMPLING_RATE, &fSampleRate);		// 8 khz
	speex_encoder_ctl(fEncoderState, SPEEX_SET_VBR_QUALITY, &fQuality);				// 7
	speex_encoder_ctl(fEncoderState, SPEEX_SET_VBR, &fVBR);							// use variable bit rate
	speex_encoder_ctl(fEncoderState, SPEEX_SET_ABR, &fAverageBitrate);				// default to 8kb
	
	speex_decoder_ctl(fDecoderState, SPEEX_SET_ENH, &fENH);							// perceptual enhancement				

	fInitialized = true;

	return true;
}

hsBool plSpeex::Shutdown()
{
	//shutdown speex
	if(fDecoderState)
	{
		speex_decoder_destroy(fDecoderState);
		fDecoderState = nil;
	}

	if(fEncoderState)
	{
		speex_encoder_destroy(fEncoderState);
		fEncoderState = nil;
	}

	if(fBitsInit)
	{
		speex_bits_destroy(fBits);
		fBitsInit = false;
	}
	fInitialized = false;

	return true;
}

hsBool plSpeex::Encode(short *data, int numFrames, int *packedLength, hsRAMStream *out)
{
	*packedLength = 0;
	
	short *pData = data;						// pointer to input data
	float *input = TRACKED_NEW float[fFrameSize];		// input to speex - used as am intermediate array since speex requires float data
	BYTE frameLength;							// number of bytes speex compressed frame to
	BYTE *frameData = TRACKED_NEW BYTE[fFrameSize];		// holds one frame of encoded data
	
	// encode data
	for( int i = 0; i < numFrames; i++ )
	{
		// convert input data to floats
		for( int j = 0; j < fFrameSize; j++ )
		{
			input[j] = pData[j];
		}

		speex_bits_reset(fBits);				// reset bit structure

		// encode data using speex
		speex_encode(fEncoderState, input, fBits);
		frameLength = speex_bits_write(fBits, (char *)frameData, fFrameSize);

		// write data - length and bytes
		out->WriteSwap(frameLength);
		*packedLength += sizeof(frameLength);	// add length of encoded frame						
		out->Write(frameLength, frameData);
		*packedLength += frameLength;			// update length

		pData += fFrameSize;					// move input pointer
	}
	
	delete[] frameData;	
	delete[] input;
	return true;
}

hsBool plSpeex::Decode(UInt8 *data, int size, int numFrames, int *numOutputBytes, short *out)
{
	if(!fInitialized) return false;
	*numOutputBytes = 0;

	hsReadOnlyStream stream( size, data );
	float *speexOutput = TRACKED_NEW float[fFrameSize];		// holds output from speex
	short *pOut = out;								// pointer to output short buffer
	
	// create buffer for input data
	BYTE *frameData = TRACKED_NEW BYTE[fFrameSize];			// holds the current frames data to be decoded
	BYTE frameLen;									// holds the length of the current frame being decoded.
	

	// Decode data
	for (int i = 0; i < numFrames; i++)
	{
		stream.ReadSwap( &frameLen );			// read the length of the current frame to be decoded
		stream.Read( frameLen, frameData );		// read the data

		memset(speexOutput, 0, fFrameSize * sizeof(float));
		speex_bits_read_from(fBits, (char *)frameData, frameLen);	// give data to speex
		speex_decode(fDecoderState, fBits, speexOutput);					// decode data 

		for(int j = 0; j < fFrameSize; j++)
		{
			pOut[j] = (short)(speexOutput[j]);			// convert floats to shorts
		}
		
		pOut += fFrameSize;					 
	}
	
	delete[] frameData;
	delete[] speexOutput;
	
	*numOutputBytes = (numFrames * fFrameSize) * sizeof(short);		// length of decoded voice data(out) in bytes
	if(*numOutputBytes == 0) 
		return false;

	return true;
}
		
// Sets variable bit rate on/off
void plSpeex::VBR(hsBool b)
{
	fVBR = b;
	speex_encoder_ctl(fEncoderState, SPEEX_SET_VBR, &fVBR);
}


// Sets the average bit rate
void plSpeex::SetABR(UInt32 abr) 
{
	fAverageBitrate = abr;
	speex_encoder_ctl(fEncoderState, SPEEX_SET_ABR, &fAverageBitrate); 
}

// Sets the quality of encoding
void plSpeex::SetQuality(UInt32 quality) 
{ 
	fQuality = quality;
	speex_encoder_ctl(fEncoderState, SPEEX_SET_QUALITY, &fQuality); 
}

void plSpeex::SetENH(hsBool b)
{
	fENH = b;
	speex_decoder_ctl(fDecoderState, SPEEX_SET_ENH, &fENH);	
}

void plSpeex::SetComplexity(UInt8 c)
{
	fComplexity = c;
	speex_encoder_ctl(fEncoderState, SPEEX_SET_COMPLEXITY, &fComplexity);	
}