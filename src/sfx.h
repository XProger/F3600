/*********************************************************
 *
 * File: Sfx.h
 *
 * Header file to sfx.c
 *
 **********************************************************
 *
 *********************************************************/

#ifndef _SFX_H_INCLUDED_
#define _SFX_H_INCLUDED_

#include "game.h"

/*
 * Card Types
 */
#define NO_CARD 106
#define SOUNDBLASTER 1
#define ULTRASOUND 2
#define AUTO_DETECT 9

/*
 * Mix Types
 */
#define MONO 1
#define STEREO 2
#define PHASED 3
#define QSOUND 4
#define DOLBY 5

/*
 * Sample Rates
 */
#define SR_11K 11025
#define SR_22K 22050

/*
 * Pitch defines
 */
#define OCTAVE_ABOVE 512
#define NORMAL_PITCH 256
#define OCTAVE_BELOW 128

/*
 * Flags
 */
#define ALL_OFF 0
#define LOOP_ON 1
#define SURROUND_ON 2
#define PROLOGIC_ON 4

uint16 SfxInit(uint16 wCard, uint16 wMixType, uint16 wSampRate);
void SfxFree(void);

uint16 SfxInitReverb(uint16 wDelay);
void SfxReverbOn(void);
void SfxReverbOff(void);
void SfxSetReverbDepth(uint8 bDepth);

uint16 SfxSetMaxEffects(uint16 wNum);

uint16 SfxGetFxBank(char* filename);
int16 SfxOpenFxBank(int16* pwFirstId);

void SfxSetVolume(uint8 bVolume);
void SfxSetCdVolume(uint8 bVolume);
void SfxPlay(int16 wSampleId, uint8 bVolume, uint8 bLRPan, uint8 bFBPan, uint16 wPitch, uint8 bFlags);
int16 SfxPlayID(int16 wSampleId, uint8 bVolume, uint8 bLRPan, uint8 bFBPan, uint16 wPitch, uint8 bFlags);

void SfxStop(int16 wPlayId);
void SfxStopAll(void);
void SfxEffectVol(int16 wPlayId, uint8 bVolume);
void SfxEffectPitch(int16 wPlayId, uint16 wPitch);
void SfxEffectPan(int16 wPlayId, uint16 bPan);

#endif /* _SFX_H_INCLUDED_ */