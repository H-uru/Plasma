/*
  HawkVoice Direct Interface (HVDI) cross platform network voice library
  Copyright (C) 2001 Phil Frisbie, Jr. (phil@hawksoft.com)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
      
  Or go to http://www.gnu.org/copyleft/lgpl.html
*/

#ifndef HVDI_H
#define HVDI_H

#include "blowfish/blowfish.h"

#ifdef __cplusplus
extern "C" {
#endif


#define HVDI_MAJOR_VERSION 0
#define HVDI_MINOR_VERSION 7
#define HVDI_VERSION_STRING "HVDI 0.7 beta"

/* This was copied from nl.h so that it did not need to be included */

#if defined WIN32 || defined WIN64
  #pragma warning (disable:4514) /* disable "unreferenced inline function has
                                    been removed" warning */
  /* The default build for Windows is as a DLL. */
  /* If you want a static library, define WIN_STATIC_LIB. */
  #ifdef WIN_STATIC_LIB
    #define NL_EXP
  #else
    #if defined __LCC__
     #define NL_EXP extern
    #else
     #define NL_EXP __declspec(dllexport)
    #endif
  #endif
  #define NL_APIENTRY __stdcall
  #define NL_CALLBACK __cdecl
  #ifdef __GNUC__
    #define NL_INLINE extern __inline__
  #else
    #define NL_INLINE __inline
  #endif
#else
  #define NL_EXP extern
  #define NL_APIENTRY
  #define NL_CALLBACK
  #ifdef __GNUC__
    #define NL_INLINE extern __inline__
  #else
    #define NL_INLINE inline /* assuming C99 compliant compiler */
  #endif /* __GNUC__ */
#endif /* WIN32 || WIN64 */

#ifndef NL_INVALID
#define NL_INVALID              (-1)
#define NL_FALSE                (0)
#define NL_TRUE                 (1)
#endif


/* We will use HVDI or hvdi to prefix all HawkVoiceDI defines and functions */

/* 
   The internal state of the codec. This is READ ONLY! You can read hvdi_dec_state->codec
   if you want to know what type of codec is being used on the other side, but DO NOT
   write to this structure!! I could have hidden these structures behind an index, but
   this IS low level quick and dirty ;)

   hvdi_enc_state and hvdi_dec_state are defined separately to help the compiler spot
   your mistakes ;)
*/
typedef struct
{
    unsigned char codec;    /* the codec used with the last packet */
    unsigned short sequence;/* the sequence number of the last packet */
    void    *state;         /* the codec state */
} hvdi_enc_state;

typedef struct
{
    unsigned char codec;    /* the codec used with the last packet */
    unsigned short sequence;/* the sequence number of the last packet */
    void    *state;         /* the codec state */
} hvdi_dec_state;

typedef struct
{
    int     rate;           /* HVDI_VOX_FAST, HVDI_VOX_MEDIUM, or HVDI_VOX_SLOW */
    int     noisethreshold; /* 0(always pass) to 1000(never pass), 300 is a good starting point */
    int     samplecount;    /* init to 0; used internally by hvdiVOX */
} hvdi_vox;

typedef struct
{
    unsigned long   lcmrate;		    /* least common multiple of rates */
    unsigned long   inskip, outskip;    /* LCM increments for I & O rates */
    unsigned long   total;
    unsigned long   intot, outtot;      /* total samples in terms of LCM rate */
    long            lastsamp;
} hvdi_rate;

typedef struct
{
    unsigned int    sample_max;
    int             counter;
    float           gain;
    float           peak;
    int             silence_counter;
} hvdi_agc;

/* The basic codecs, from hawkvoice.h */
#define HV_2_4K_CODEC       0x0001  /* LPC-10 2.4 Kbps codec */
#define HV_4_8K_CODEC       0x0002  /* LPC 4.8 Kbps codec */
#define HV_13_2K_CODEC      0x0004  /* GSM 13.2 Kbps codec */
#define HV_32K_CODEC        0x0008  /* Intel/DVI ADPCM 32 Kbps codec */
#define HV_64K_CODEC        0x0010  /* G.711 u-law 64 Kbps codec */
#define HV_1_4K_CODEC       0x0011  /* OpenLPC 1.4 Kbps codec */
#define HV_1_8K_CODEC       0x0012  /* OpenLPC 1.8 Kbps codec */

/* Alternate codec names */
#define HV_LPC10_CODEC      HV_2_4K_CODEC
#define HV_LPC_CODEC        HV_4_8K_CODEC
#define HV_GSM_CODEC        HV_13_2K_CODEC
#define HV_ADPCM_32_CODEC   HV_32K_CODEC
#define HV_PCM_64_CODEC     HV_64K_CODEC
#define HV_G_711_CODEC      HV_64K_CODEC
#define HV_ULAW_CODEC       HV_64K_CODEC
#define HV_LPC_1_4_CODEC    HV_1_4K_CODEC
#define HV_LPC_1_8_CODEC    HV_1_8K_CODEC

/* VOX options */
/* how many samples of silence to wait after voice stops */
#define HVDI_VOX_FAST       4000    /* 1/2 second */
#define HVDI_VOX_MEDIUM     8000    /* 1 second */
#define HVDI_VOX_SLOW      12000    /* 1 1/2 seconds */


/* HawkVoiceDI API */

NL_EXP hvdi_enc_state* NL_APIENTRY hvdiCreateEncoderState(void);

NL_EXP hvdi_dec_state* NL_APIENTRY hvdiCreateDecoderState(void);

NL_EXP void      NL_APIENTRY hvdiFreeEncoderState(hvdi_enc_state *state);

NL_EXP void      NL_APIENTRY hvdiFreeDecoderState(hvdi_dec_state *state);

NL_EXP int       NL_APIENTRY hvdiSetCodec(unsigned char codec, hvdi_enc_state *state);

NL_EXP BF_KEY*   NL_APIENTRY hvdiMakeEncryptionKey(const char *string);

NL_EXP int       NL_APIENTRY hvdiIsVoicePacket(unsigned char *packet, int length);

NL_EXP int       NL_APIENTRY hvdiDecodePacket(unsigned char *packet, int paclen, short *buffer,
                                              int buflen, BF_KEY *key, hvdi_dec_state *state);

NL_EXP int       NL_APIENTRY hvdiEncodePacket(short *buffer, int buflen, unsigned char *packet,
                                              int paclen, BF_KEY *key, hvdi_enc_state *state);

NL_EXP int       NL_APIENTRY hvdiVOX(short *buffer, int buflen, hvdi_vox *vox);

NL_EXP void      NL_APIENTRY hvdiRateInit(hvdi_rate *rate, int inrate, int outrate);

NL_EXP void      NL_APIENTRY hvdiRateFlow(hvdi_rate *rate, short *inbuf, short *outbuf, int *inlen, int *outlen);

NL_EXP void      NL_APIENTRY hvdiAGCInit(hvdi_agc *agc, float level);

NL_EXP void      NL_APIENTRY hvdiAGC(hvdi_agc *agc, short *buffer, int len);

NL_EXP void      NL_APIENTRY hvdiMix(short *outbuf, short **inbuf, int number, int inlen);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* HVDI_H */

