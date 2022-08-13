#include "game.h"

int WhatChannel(enum SfxType id) { return 0; }
int IsPlaying(int channel) { return 0; }
void InitNotes(void) {}
void KillNote(enum SfxType id) {}
void KillNotes(void) {}
int PlayNote(enum SfxType id) { return 0; }
void SetPitch(enum SfxType id, int32 pitch) {}
void SetVolume(enum SfxType id, int32 volL, int32 volR) {}
void FillNotes(int16* samples, int32 count) {}

int MusicIsPlaying(void) { return 0; }
void MusicPlay(int track) {}
void MusicStop(void) {}
