
#ifndef _TTU_H
#define _TTU_H

// the TTU file format (in u32s)
#define TTU_HEADER     0
#define TTU_CHANNELS_N 1
#define TTU_PATTERNS_N 2
#define TTU_ORDERS_N   3
#define TTU_ORDERS     4

// the TTU states
#define TTU_STATE_STOPPED 0
#define TTU_STATE_PLAYING 1
#define TTU_STATE_END     2

// waveforms
#define TTU_WAVEFORM_SINE   0
#define TTU_WAVEFORM_RAMP   1
#define TTU_WAVEFORM_SQUARE 2
#define TTU_WAVEFORM_RANDOM 3

struct channel {
	s32 volume;
	s32 panning;
	s32 sample;
	s32 note;
	s32 finetune;
	s32 frequency;
	s32 sampleType;
	s32 effect;
	s32 parameter;
	s32 period;
	s32 volumeSlideParameter;
	s32 vibratoWaveform;
	s32 vibratoTick;
	s32 vibratoSpeed;
	s32 vibratoAmplitude;
	s32 tremoloWaveform;
	s32 tremoloTick;
	s32 tremoloSpeed;
	s32 tremoloAmplitude;
	s32 tremoloVolumeBackup;
	s32 tonePortamentoNote;
	s32 tonePortamentoSpeed;
	s32 extraFXTick;
	s32 loopPosition;
	s32 loopCounter;
};

struct ttu {
	u32 state;
	u32 *ttu;
	u32 *tts;
	s32 startChannel;
	s32 channelsN;
	s32 patternsN;
	u32 *patterns;
	s32 ordersN;
	u32 *orders;
	s32 orderCurrent;
	s32 orderNext;
	s32 tick;
	s32 ticksPerRow;
	s32 tickArpeggio;
	s32 rowCurrent;
	s32 rowBreak;
	s32 tempo;
	s32 rowsPatternDelay;
	struct channel channels[8];
};

// the TTS file format (in u32s)
#define TTS_HEADER    0
#define TTS_SAMPLES_N 1
#define TTS_POINTERS  2

void ttuStart(u32 startChannel, u32 ttuChannels, u32 *ttuData, u32 *ttsData);
void ttuStop(void);
void ttuTick(void);
void ttuTimerStart(s32 ticksPerRow);
void ttuTimerStop(void);
void ttuTickRow(struct ttu *ttu);
void ttuTickEffects(struct ttu *ttu);
void ttuPlaySample(struct ttu *ttu, s32 channel, s32 sample, s32 note);
void ttuUpdateChannel(struct ttu *ttu, s32 channel);

void ttuFXVibrato(struct channel *c);
void ttuFXVolumeSlide(struct channel *c);
void ttuFXTonePortamento(struct channel *c);

#endif
