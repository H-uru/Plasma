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
#ifndef plVoiceChat_h
#define plVoiceChat_h

#include "hsTemplates.h"
#include "plWin32Sound.h"
#include "hsThread.h"

// voice flags
#define VOICE_ENCODED		( 1 << 0 )
#define VOICE_NARROWBAND	( 1 << 1 )
#define VOICE_ENH			( 1 << 2 )
#define BUFFER_LEN_SECONDS		4
#define FREQUENCY				8000

struct hsVector3;
struct SpeexBits;
class  plWinAudible;
class  plPlate;
class  plStatusLog;
class  plSpeex;
typedef struct ALCdevice_struct ALCdevice;


// Sound used for playing back dynamic voice chat data. this allows us to hook voice chat into the audio system
class plVoiceSound : public plWin32Sound
{
public:
	plVoiceSound();
	~plVoiceSound();
	hsBool LoadSound( hsBool is3D );
	void AddVoiceData(void *data, unsigned bytes);
	void Update();
	void Play();
	virtual void SetStartPos(unsigned bytes){}
	
private:
	virtual bool	ILoadDataBuffer( void ){ return true; }
	virtual void	IUnloadDataBuffer( void ){}

	virtual void	IDerivedActuallyPlay( void );
	virtual void	ISetActualTime( double t ){}
	virtual float	GetActualTimeSec() { return 0.0f; }
	virtual void	IRefreshParams( void );
	static unsigned fCount;
	double	 fLastUpdate;
};

class plVoicePlayer
{
public:
	plVoicePlayer();
	~plVoicePlayer();
	void PlaybackVoiceMessage(void* data, unsigned size, int numFramesInBuffer);
	void PlaybackUncompressedVoiceMessage(void* data, unsigned size);
	void SetVelocity(const hsVector3 vel);
	void SetPosition(const hsPoint3 pos);
	void SetOrientation(const hsPoint3 pos);
	
	void SetTalkIcon(int index, UInt32 str){}
	void ClearTalkIcon(){}
	plVoiceSound *GetSoundPtr() { return &fSound; }
	static void Enable(hsBool enable) { fEnabled = enable; }

private:
	plVoiceSound fSound;
	static hsBool fEnabled;
};

class plVoiceRecorder
{
public:
	plVoiceRecorder();
	~plVoiceRecorder();

	void Update(double time);
	void SetMikeOpen(hsBool b);
	void DrawTalkIcon(hsBool b);
	void DrawDisabledIcon(hsBool b);
	
	void	SetTalkIcon(int index, UInt32 str);
	void	ClearTalkIcon();

	static hsBool	RecordingEnabled() { return fRecording; }
	static hsBool	NetVoiceEnabled() { return fNetVoice; }
	static hsBool	CompressionEnabled() { return fCompress; }
	static void		EnablePushToTalk(hsBool b) { fMicAlwaysOpen = !b; }
	static void		EnableIcons(hsBool b) { fShowIcons = b; }
	static void		EnableRecording(hsBool b) { fRecording = b; }
	static void		EnableNetVoice(hsBool b) { fNetVoice = b; }
	static void		EnableCompression(hsBool b) { fCompress = b; }
	static void		SetSampleRate(short s) { fSampleRate = s; }
	static void		SetSquelch(hsScalar f) { fRecordThreshhold = f; }

	static void IncreaseRecordingThreshhold();
	static void DecreaseRecordingThreshhold();

	static void SetQuality(int quality);	// sets the quality of encoding
	static void SetMode(int mode);	// sets nb or wb mode
	static void SetVBR(bool vbr);
	static void SetComplexity(int c);
	static void SetENH(hsBool b);
	static short GetSampleRate() { return fSampleRate; }
	
private:
	
	hsBool					fMikeOpen;
	hsBool					fMikeJustClosed;
	static hsBool			fMicAlwaysOpen;
	static hsBool			fShowIcons;
	static hsBool			fCompress;
	static hsBool			fNetVoice;
	static hsBool			fRecording;
	static short			fSampleRate;
	plPlate*				fDisabledIcon;
	plPlate*				fTalkIcon;
	static hsScalar			fRecordThreshhold;
};


// Speex voice encoder/decoder class
class plSpeex 
{
public:
	~plSpeex();
	
	enum Mode
	{
		kNarrowband,
		kWideband,
		kUltraWideband
	};
	static plSpeex *GetInstance()	
	{
		static plSpeex instance;
		return &instance;
	}

	hsBool Init(Mode mode);
	hsBool Shutdown();
	hsBool Encode(short *data, int numFrames, int *packedLength, hsRAMStream *out);
	hsBool Decode(UInt8 *data, int size, int numFrames, int *numOutputBytes, short *out);
	int	   GetFrameSize() { return fFrameSize; }
	void   VBR(hsBool b);									// turn variable bit rate on/off
	void   SetVBR(UInt32 vbr);								// Set variable bit rate quality
	void   ABR(hsBool b);									// turn average bit rate on/off
	void   SetABR(UInt32 abr);								// Set average bit rate quality
	void   SetQuality(UInt32 quality);						// Set encoder quality
	hsBool IsUsingVBR()			{ return fVBR; }
	int    GetQuality()			{ return fQuality; }
	void   SetENH(hsBool b);
	void   SetComplexity(UInt8 c);

	hsBool Initialized() { return fInitialized; }
	
private:
	plSpeex();
	SpeexBits*					fBits;					// main speex structure
	hsBool						fBitsInit;
	void*						fEncoderState;
	void*						fDecoderState;
	int							fSampleRate;
	int							fFrameSize;				// frame size from speex - 160 for nb
	int							fQuality;				// 0-10 speex encode quality
	hsBool						fVBR;					// toggle variable bit rate
	int							fAverageBitrate;		// n-bits per second
	UInt8						fComplexity;			// 1-10 sets cpu resources allowed for encoder
	hsBool						fENH;					// perceptual enhancement
	hsBool						fInitialized;			
};

#endif //plVoiceChat_h
