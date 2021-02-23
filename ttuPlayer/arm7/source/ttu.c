
#include "ndsARM7.h"

#include "defines.h"
#include "memory.h"
#include "bios.h"
#include "shared.h"

#include "sound.h"
#include "ttu.h"


// frequency = TTU_AMIGA_VAL / period
#define TTU_AMIGA_VAL 3579545

// 16777216/AMIGA_VAL, left shifted 17 more bits for accuracy
#define TIMER_VAL	614361

// the TTU we are playing
struct ttu ttuCurrent __attribute__((section(".bss")));

// the ProTracker sine table for vibrato effects
const s16 ttuSineTable[] = {
	0, 24, 49, 74, 97, 120, 141, 161,
	180, 197, 212, 224, 235, 244, 250, 253,
	255, 253, 250, 244, 235, 224, 212, 197,
	180, 161, 141, 120, 97, 74, 49, 24,
	0, -24, -49, -74, -97, -120, -141, -161,
	-180, -197, -212, -224, -235, -244, -250, -253,
	-255, -253, -250, -244, -235, -224, -212, -197,
	-180, -161, -141, -120, -97, -74, -49, -24
};

// random table for "random" waveforms
const s8 ttuRandomTable[] = {
	-111, 43, -13, -65, 50, 83, 114, -58, 
	  34, -42, -101, 127, 74, -22, 25, 120, 
	 -30, -7, 12, 104, -46, 7, 19, -97, 
	  57, -79, -115, 61, -72, 0, 90, -128,
	 -97, -7, 12, 101, -46, 7, -100, 19, 
	  55, -80, -128, 61, -72, 90, 121, -128,
	 -30, 45, -13, 114, -69, 49, 88, -57, 
	  35, -40, -106, 0, 74, -22, 25, 39, 
};

// the Amiga period table
const u16 ttuPeriodTable[12*5*16] = {
	1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960, 906,
	856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
	428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
	214, 202, 190, 180, 169, 160, 151, 142, 134, 127, 120, 113,
	107, 101, 95, 90, 84, 80, 75, 71, 67, 63, 60, 56,
	1700, 1604, 1514, 1430, 1348, 1274, 1202, 1134, 1070, 1010, 954, 900,
	850, 802, 757, 715, 674, 637, 601, 567, 535, 505, 477, 450,
	425, 401, 378, 357, 337, 318, 300, 283, 267, 252, 238, 225,
	212, 200, 189, 178, 168, 159, 150, 141, 133, 126, 119, 112,
	106, 100, 94, 89, 84, 79, 75, 70, 66, 63, 59, 56,
	1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948, 894,
	844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474, 447,
	422, 398, 376, 354, 335, 316, 298, 281, 266, 251, 237, 223,
	211, 199, 188, 177, 167, 158, 149, 140, 133, 125, 118, 111,
	105, 99, 94, 88, 83, 79, 74, 70, 66, 62, 59, 55,
	1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996, 940, 888,
	838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470, 444,
	419, 395, 373, 352, 332, 314, 296, 279, 264, 249, 235, 222,
	209, 197, 186, 176, 166, 157, 148, 139, 132, 124, 117, 111,
	104, 98, 93, 88, 83, 78, 74, 69, 66, 62, 58, 55,
	1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 990, 934, 882,
	832, 785, 741, 699, 660, 623, 588, 555, 524, 495, 467, 441,
	416, 392, 370, 349, 330, 311, 294, 277, 262, 247, 233, 220,
	208, 196, 185, 174, 165, 155, 147, 138, 131, 123, 116, 110,
	104, 98, 92, 87, 82, 77, 73, 69, 65, 61, 58, 55,
	1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982, 926, 874,
	826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463, 437,
	413, 389, 368, 347, 327, 309, 292, 275, 260, 245, 231, 218,
	206, 194, 184, 173, 163, 154, 146, 137, 130, 122, 115, 109,
	103, 97, 92, 86, 81, 77, 73, 68, 65, 61, 57, 54,
	1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974, 920, 868,
	820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460, 434,
	410, 387, 365, 344, 325, 307, 290, 273, 258, 243, 230, 217,
	205, 193, 182, 172, 162, 153, 145, 136, 129, 121, 115, 108,
	102, 96, 91, 86, 81, 76, 72, 68, 64, 60, 57, 54,
	1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968, 914, 862,
	814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457, 431,
	407, 384, 362, 342, 323, 305, 287, 271, 256, 242, 228, 215,
	203, 192, 181, 171, 161, 152, 143, 135, 128, 121, 114, 107,
	101, 96, 90, 85, 80, 76, 71, 67, 64, 60, 57, 53,
	1814, 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960,
	907, 856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480,
	453, 428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240,
	226, 214, 202, 190, 180, 169, 160, 151, 142, 134, 127, 120,
	113, 107, 101, 95, 90, 84, 80, 75, 71, 67, 63, 60,
	1800, 1700, 1604, 1514, 1430, 1350, 1272, 1202, 1134, 1070, 1010, 954,
	900, 850, 802, 757, 715, 675, 636, 601, 567, 535, 505, 477,
	450, 425, 401, 378, 357, 337, 318, 300, 283, 267, 252, 238,
	225, 212, 200, 189, 178, 168, 159, 150, 141, 133, 126, 119,
	112, 106, 100, 94, 89, 84, 79, 75, 70, 66, 63, 59,
	1788, 1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948,
	894, 844, 796, 752, 709, 670, 632, 597, 563, 532, 502, 474,
	447, 422, 398, 376, 354, 335, 316, 298, 281, 266, 251, 237,
	223, 211, 199, 188, 177, 167, 158, 149, 140, 133, 125, 118,
	111, 105, 99, 94, 88, 83, 79, 74, 70, 66, 62, 59,
	1774, 1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996, 940,
	887, 838, 791, 746, 704, 665, 628, 592, 559, 528, 498, 470,
	443, 419, 395, 373, 352, 332, 314, 296, 279, 264, 249, 235,
	221, 209, 197, 186, 176, 166, 157, 148, 139, 132, 124, 117,
	110, 104, 98, 93, 88, 83, 78, 74, 69, 66, 62, 58,
	1762, 1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 988, 934,
	881, 832, 785, 741, 699, 660, 623, 588, 555, 524, 494, 467,
	440, 416, 392, 370, 349, 330, 311, 294, 277, 262, 247, 233,
	220, 208, 196, 185, 174, 165, 155, 147, 138, 131, 123, 116,
	110, 104, 98, 92, 87, 82, 77, 73, 69, 65, 61, 58,
	1750, 1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982, 926,
	875, 826, 779, 736, 694, 655, 619, 584, 551, 520, 491, 463,
	437, 413, 389, 368, 347, 327, 309, 292, 275, 260, 245, 231,
	218, 206, 194, 184, 173, 163, 154, 146, 137, 130, 122, 115,
	109, 103, 97, 92, 86, 81, 77, 73, 68, 65, 61, 57,
	1736, 1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974, 920,
	868, 820, 774, 730, 689, 651, 614, 580, 547, 516, 487, 460,
	434, 410, 387, 365, 344, 325, 307, 290, 273, 258, 243, 230,
	217, 205, 193, 182, 172, 162, 153, 145, 136, 129, 121, 115,
	108, 102, 96, 91, 86, 81, 76, 72, 68, 64, 60, 57,
	1724, 1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968, 914,
	862, 814, 768, 725, 684, 646, 610, 575, 543, 513, 484, 457,
	431, 407, 384, 362, 342, 323, 305, 287, 271, 256, 242, 228,
	215, 203, 192, 181, 171, 161, 152, 143, 135, 128, 121, 114,
	107, 101, 96, 90, 85, 80, 76, 71, 67, 64, 60, 57
};

// initial panning tables
const u8 ttuInitialPanning4[] = {
	16, 112, 112, 16
};
const u8 ttuInitialPanning6[] = {
	16, 112, 112, 16, 112, 112
};
const u8 ttuInitialPanning8[] = {
	16, 112, 112, 16, 112, 112, 112, 112
};


void ttuStart(u32 startChannel, u32 ttuChannels, u32 *ttuData, u32 *ttsData) {

	struct channel *channel;
	struct ttu *ttu;
	const u8 *panningTable;
	s32 i, j, *iPtr;

	// the TTU uses too many channels?
	if (ttuData[TTU_CHANNELS_N] > ttuChannels)
		return;

	// init the player
	ttu = &ttuCurrent;

	ttu->ttu = ttuData;
	ttu->tts = ttsData;
	ttu->startChannel = startChannel;
	ttu->channelsN = ttuData[TTU_CHANNELS_N];
	ttu->ordersN = ttuData[TTU_ORDERS_N];
	ttu->patternsN = ttuData[TTU_PATTERNS_N];
	ttu->orders = &ttuData[TTU_ORDERS];
	ttu->patterns = &ttuData[TTU_ORDERS + ttu->ordersN];
	ttu->orderCurrent = 0;
	ttu->orderNext = 1;
	ttu->tick = 5;
	ttu->ticksPerRow = 6;
	// ttu->tickArpeggio = 2; will be done in the first ttuTick() call
	ttu->rowBreak = 0;
	ttu->rowCurrent = -1;
	ttu->rowsPatternDelay = 0;
	ttu->tempo = 125;
	ttu->state = TTU_STATE_PLAYING;

	// default pannings for 4, 6 and 8 channel MODs
	if (ttu->channelsN == 4)
		panningTable = ttuInitialPanning4;
	else if (ttu->channelsN == 6)
		panningTable = ttuInitialPanning6;
	else
		panningTable = ttuInitialPanning8;

	// reset the channels
	for (i = 0; i < ttu->channelsN; i++) {
		channel = &ttu->channels[i];
		iPtr = (s32 *)channel;

		// zero the structure
		for (j = 0; j < (sizeof(struct channel)) >> 2; j++)
			iPtr[j] = 0;

		// set initial values
		channel->volume = 127;
		channel->panning = panningTable[i];
		channel->sample = 0xFF;
		channel->note = 0xFF;
		channel->vibratoWaveform = TTU_WAVEFORM_SINE;
		channel->tremoloWaveform = TTU_WAVEFORM_SINE;

		// reset the hardware channel
		SOUND_CHANNEL_CR(startChannel + i) = 0;
	}

	// start the timer IRQ
	ttuTimerStart(ttu->tempo);
}


void ttuTick(void) {

	struct ttu *ttu;
	s32 i;

	ttu = &ttuCurrent;

	if (ttu->state != TTU_STATE_PLAYING)
		return;

	// tick the arpeggio
	ttu->tickArpeggio++;
	if (ttu->tickArpeggio > 2)
		ttu->tickArpeggio = 0;

	ttu->tick++;
	if (ttu->tick >= ttu->ticksPerRow) {
		// we advance one row
		ttu->tick = 0;
		ttu->tickArpeggio = 0;

		if (ttu->rowsPatternDelay > 0)
			ttu->rowsPatternDelay--;
		else {
			ttu->rowCurrent++;
			if (ttu->rowCurrent >= 64) {
				// we advance one order
				ttu->orderCurrent = ttu->orderNext++;
				if (ttu->orderCurrent >= ttu->ordersN) {
					// out of orders! the end!
					ttu->state = TTU_STATE_END;

					// reset the hardware channels
					for (i = 0; i < ttu->channelsN; i++)
						SOUND_CHANNEL_CR(ttu->startChannel + i) = 0;

					return;
				}

				// reset the row
				ttu->rowCurrent = ttu->rowBreak;
				ttu->rowBreak = 0;
			}

			// execute the row
			ttuTickRow(ttu);
		}
	}
	else {
		// update the effects
		ttuTickEffects(ttu);
	}
}


void ttuTickRow(struct ttu *ttu) {

	u32 i, channel, sample, note, effect, parameter;
	struct channel *c;
	u8 *data;

	// calculate the row pointer
	i = ttu->orders[ttu->orderCurrent];
	i = ((i << 6) + ttu->rowCurrent) * ttu->channelsN;
	data = (u8 *)&(ttu->patterns[i]);

	// go through the channels
	for (channel = 0; channel < ttu->channelsN; channel++) {
		note      = *(data++);
		sample    = *(data++);
		effect    = *(data++);
		parameter = *(data++);

		// get the channel
		c = &ttu->channels[channel];

		// reset the effects
		c->effect = effect;
		c->parameter = parameter;
		c->extraFXTick = 0;

		// no sample, no nothing?
		if (sample != 0xFF || note != 0xFF || effect != 0 || parameter != 0) {
			// sample?
			if (sample != 0xFF) {
				c->sample = sample;
				// yes, reset volume and finetune
				i = ttu->tts[TTS_POINTERS + sample];
				c->volume = ttu->tts[i + 2];
				c->finetune = ttu->tts[i + 1];
			}

			if (effect != 0x3 && effect != 0x5) {
				// note?
				if (note != 0xFF) {
					c->note = note;
					// reset the frequency
					c->period = ttuPeriodTable[(c->finetune*12*5) + note];
					c->frequency = 65536 - ((TIMER_VAL*c->period) >> 17);
				}
			}

			// effects?
			switch (effect) {
			case 0x3:
				// tone portamento
				if (note != 0xFF)
					c->tonePortamentoNote = note;
				if (parameter != 0)
					c->tonePortamentoSpeed = parameter;

				break;

			case 0x4:
				// vibrato
				if ((parameter >> 4) != 0)
					c->vibratoSpeed = parameter >> 4;
				if ((parameter & 0xF) != 0)
					c->vibratoAmplitude = parameter & 0xF;

				// restart the vibrato?
				if (note != 0xFF && c->vibratoWaveform <= 3)
					c->vibratoTick = 0;

				break;

			case 0x5:
				// tone portamento & volume slide
				if (note != 0xFF)
					c->tonePortamentoNote = note;
				if (parameter != 0)
					c->volumeSlideParameter = parameter;

				break;

			case 0x6:
				// vibrato & volume slide
				if (parameter != 0)
					c->volumeSlideParameter = parameter;

				break;

			case 0x7:
				// tremolo
				if ((parameter >> 4) != 0)
					c->tremoloSpeed = parameter >> 4;
				if ((parameter & 0xF) != 0)
					c->tremoloAmplitude = parameter & 0xF;

				// restart the tremolo?
				if (note != 0xFF && c->tremoloWaveform <= 3)
					c->tremoloTick = 0;

				// backup the current volume
				c->tremoloVolumeBackup = c->volume;

				break;

			case 0x8:
				// pan
				c->panning = parameter >> 1;

				break;

			case 0xA:
				// volume slide
				if (parameter != 0)
					c->volumeSlideParameter = parameter;

				break;

			case 0xB:
				// jump to order
				ttu->rowCurrent = 64;
				ttu->orderNext = parameter;

				break;

			case 0xC:
				// volume
				if (parameter > 63)
					parameter = 63;

				// scale (0-63 -> 0-126)
				c->volume = parameter << 1;

				break;

			case 0xD:
				// break to row
				ttu->rowCurrent = 64;
				ttu->rowBreak = (parameter >> 4)*10 + (parameter & 0xF);

				break;

			case 0xE:
				// extra effects
				i = parameter >> 4;
				parameter &= 0xF;

				switch (i) {
				case 0x1:
					// fine pitch up
					c->period -= parameter;
					c->frequency = 65536 - ((TIMER_VAL*c->period) >> 17);

					break;

				case 0x2:
					// fine pitch down
					c->period += parameter;
					c->frequency = 65536 - ((TIMER_VAL*c->period) >> 17);

					break;

				case 0x4:
					// set the vibrato waveform
					c->vibratoWaveform = parameter;

					break;

				case 0x5:
					// set finetune for the current instrument
					if (sample != 0xFF) {
						// get the sample attributes, and pointer to the sample data
						i = ttu->tts[TTS_POINTERS + sample];
						ttu->tts[i + 1] = parameter;
					}

					break;

				case 0x6:
					// pattern loop
					if (parameter == 0)
						c->loopPosition = ttu->rowCurrent;
					else {
						if (c->loopCounter == 0)
							c->loopCounter = parameter + 1;

						c->loopCounter--;
						if (c->loopCounter > 0) {
							ttu->rowCurrent = 64;
							ttu->rowBreak = c->loopPosition;
							ttu->orderNext = ttu->orderCurrent;
						}
					}

					break;

				case 0x7:
					// set the tremolo waveform
					c->tremoloWaveform = parameter;

					break;

				case 0x8:
					// pan
					c->panning = parameter << 4;

					break;

				case 0x9: // retrigger note
				case 0xC: // cut note
				case 0xD: // note delay
					c->extraFXTick = parameter;

					break;

				case 0xA:
					// fine volume up
					c->volume += parameter << 1;

					break;

				case 0xB:
					// fine volume down
					c->volume -= parameter << 1;

				case 0xE:
					// pattern delay
					ttu->rowsPatternDelay = parameter;

					break;
				}

				break;

			case 0xF:
				// set speed / tempo
				if (parameter < 0x20)
					ttu->ticksPerRow = parameter;
				else {
					// tempo
					ttu->tempo = parameter;
					ttuTimerStart(parameter);
				}

				break;
			}

			// 0xED & 0xE9 wait a few ticks before doing anything, the rest don't do a thing here any more
			if (effect == 0xED || effect == 0xE9 || effect == 0x3 || effect == 0x5)
				continue;

			if (sample != 0xFF && note != 0xFF) {
				// we have a real sample -> play
				ttuPlaySample(ttu, channel, sample, note);
			}
			else if (note != 0xFF) {
				// replay the previous sample
				ttuPlaySample(ttu, channel, c->sample, note);
			}

			ttuUpdateChannel(ttu, channel);
		}
	}
}


void ttuTickEffects(struct ttu *ttu) {

	s32 channel, effect, parameter, i;
	struct channel *c;

	for (channel = 0; channel < ttu->channelsN; channel++) {
		// get the channel data
		c = &ttu->channels[channel];

		effect = c->effect;
		parameter = c->parameter;

		// no effect?
		if (effect == 0 && parameter == 0)
			continue;

		switch (effect) {
		case 0x0:
			// arpeggio
			i = c->note;
			if (ttu->tickArpeggio == 1)
				i += parameter >> 4;
			else if (ttu->tickArpeggio == 2)
				i += parameter & 0xF;

			// limit the new note
			if (i > 12*5-1)
				i = 12*5-1;

			c->period = ttuPeriodTable[(c->finetune*12*5) + i];
			c->frequency = 65536 - ((TIMER_VAL*c->period) >> 17);

			break;

		case 0x1:
			// portamento up
			c->period -= parameter;
			if (c->period < 54)
				c->period = 54;
			c->frequency = 65536 - ((TIMER_VAL*c->period) >> 17);

			break;

		case 0x2:
			// portamento down
			c->period += parameter;
			if (c->period > 1814)
				c->period = 1814;
			c->frequency = 65536 - ((TIMER_VAL*c->period) >> 17);

			break;

		case 0x3:
			// tone portamento
			ttuFXTonePortamento(c);

			break;

		case 0x4:
			// vibrato
			ttuFXVibrato(c);

			break;

		case 0x5:
			// tone portamento & volume slide

			// tone portamento
			ttuFXTonePortamento(c);

			// volume slide
			ttuFXVolumeSlide(c);

			break;

		case 0x6:
			// vibrato & volume slide

			// vibrato
			ttuFXVibrato(c);

			// volume slide
			ttuFXVolumeSlide(c);

			break;

		case 0x7:
			// tremolo
			c->tremoloTick = (c->tremoloSpeed + c->tremoloTick) & 63;

			i = c->tremoloAmplitude;

			// get the ampitude [-128, 127]
			switch (c->tremoloWaveform & 3) {
			case TTU_WAVEFORM_SINE:
				i = (i * ttuSineTable[c->tremoloTick]) >> 5;
				break;

			case TTU_WAVEFORM_RAMP:
				i = ((32 - c->tremoloTick)*i) >> 2;
				break;

			case TTU_WAVEFORM_SQUARE:
				i <<= 3;
				if (c->tremoloTick >= 32)
					i = -i;
				break;

			case TTU_WAVEFORM_RANDOM:
				i = (ttuRandomTable[c->tremoloTick]*i) >> 4;
				break;
			}

			i += c->tremoloVolumeBackup;
			c->volume = i;

			break;

		case 0xA:
			// volume slide
			ttuFXVolumeSlide(c);

			break;
		}

		// extra FX (0xE*)?
		if (c->extraFXTick > 0) {
			c->extraFXTick--;
			if (c->extraFXTick == 0) {
				i = parameter >> 4;
				if (i == 0x9) {
					// retrigger note
					if (c->sample != 0xFF && c->note != 0xFF)
						ttuPlaySample(ttu, channel, c->sample, c->note);

					// restart the counter
					c->extraFXTick = parameter & 0xF;
				}
				else if (i == 0xC) {
					// note cut
					c->volume = 0;

					// do this just once
					c->effect = 0;
					c->parameter = 0;
				}
				else if (i == 0xD) {
					// note delay
					if (c->sample != 0xFF && c->note != 0xFF)
						ttuPlaySample(ttu, channel, c->sample, c->note);

					// do this just once
					c->effect = 0;
					c->parameter = 0;
				}
			}
		}

		ttuUpdateChannel(ttu, channel);
	}
}


void ttuFXVibrato(struct channel *c) {
	
	s32 i;

	i = c->vibratoAmplitude;
	c->vibratoTick = (c->vibratoTick + c->vibratoSpeed) & 63;

	// get the amplitude [-32, 31]
	switch (c->vibratoWaveform & 3) {
	case TTU_WAVEFORM_SINE:
		i = (i * ttuSineTable[c->vibratoTick]) >> 7;
		break;

	case TTU_WAVEFORM_RAMP:
		i = ((32 - c->vibratoTick)*i) >> 4;
		break;

	case TTU_WAVEFORM_SQUARE:
		i <<= 1;
		if (c->vibratoTick >= 32)
			i = -i;
		break;

	case TTU_WAVEFORM_RANDOM:
		i = (ttuRandomTable[c->vibratoTick]*i) >> 6;
		break;
	}

	i += c->period;
	if (i < 54)
		i = 54;
	if (i > 1814)
		i = 1814;
	c->frequency = 65536 - ((TIMER_VAL*i) >> 17);
}


void ttuFXTonePortamento(struct channel *c) {

	s32 i, j;

	i = c->period;
	j = ttuPeriodTable[(c->finetune*12*5) + c->tonePortamentoNote];

	if (i > j) {
		i -= c->tonePortamentoSpeed;
		if (i < j)
			i = j;
	}
	else if (i < j) {
		i += c->tonePortamentoSpeed;
		if (i > j)
			i = j;
	}

	c->period = i;
	c->frequency = 65536 - ((TIMER_VAL*i) >> 17);
}


void ttuFXVolumeSlide(struct channel *c) {

	s32	parameter;

	parameter = c->volumeSlideParameter;

	if ((parameter & 0xF) == 0) {
		// slide up
		c->volume += (parameter >> 4) << 1;
	}
	else if ((parameter >> 4) == 0) {
		// slide down
		c->volume -= parameter << 1;
	}
}


void ttuUpdateChannel(struct ttu* ttu, s32 channel) {

	s32 channelDS, enable;
	struct channel *c;

	enable = 0;

	// get the DS channel
	channelDS = channel + ttu->startChannel;

	// get the channel data
	c = &ttu->channels[channel];

	// fix the volume
	if (c->volume > 127)
		c->volume = 127;
	if (c->volume < 0)
		c->volume = 0;

	// update frequency
	SOUND_CHANNEL_TIMER(channelDS) = c->frequency;

	// volume > 0?
	if (c->volume > 0)
		enable = BIT_SOUNDxCNT_ENABLE;

	// update the parameters
	SOUND_CHANNEL_CR(channelDS) = enable | SOUNDxCNT_VOLUME(c->volume) | SOUNDxCNT_PANNING(c->panning) | c->sampleType | SOUNDxCNT_FORMAT_PCM8;
}


void ttuPlaySample(struct ttu *ttu, s32 channel, s32 sample, s32 note) {

	u32 sampleStart, length, loopStart, loopLength, channelDS;
	struct channel *c;

	// get the DS channel
	channelDS = channel + ttu->startChannel;

	// get the channel data
	c = &ttu->channels[channel];

	// get the sample attributes, and pointer to the sample data
	sampleStart = ttu->tts[TTS_POINTERS + sample];

	length     = ttu->tts[sampleStart + 0];
	loopStart  = ttu->tts[sampleStart + 3];
	loopLength = ttu->tts[sampleStart + 4];

	// stop the current sample
	SOUND_CHANNEL_CR(channelDS) = 0;

	if (loopLength == 0) {
		// one shot
		ttu->channels[channel].sampleType = SOUNDxCNT_REPEAT_ONE_SHOT;
		SOUND_CHANNEL_LENGTH(channelDS) = length;
		SOUND_CHANNEL_REPEAT_POINT(channelDS) = 0;
	}
	else {
		// looping
		ttu->channels[channel].sampleType = SOUNDxCNT_REPEAT_LOOP;
		SOUND_CHANNEL_LENGTH(channelDS) = loopLength;
		SOUND_CHANNEL_REPEAT_POINT(channelDS) = loopStart;
	}

	SOUND_CHANNEL_SOURCE(channelDS) = (u32)&ttu->tts[sampleStart];
}


void ttuStop(void) {

	struct ttu *ttu;
	s32 i;

	ttu = &ttuCurrent;

	ttuTimerStop();

	// reset the hardware channels
	for (i = 0; i < ttu->channelsN; i++)
		SOUND_CHANNEL_CR(ttu->startChannel + i) = 0;
}


void ttuTimerStart(s32 tempo) {

	u16 ime;

	ime = REG_IME;
	REG_IME = 0;

	// stop the timer
	REG_TM0CNT_H = 0;

	// 1 << 25 ~ ARM7 freq, clock divided by 64, / tempo*2/5
	REG_TM0CNT_L = 65536 - ((((1 << 25) >> 6) >> 1)*5 / tempo);

	// restart the timer
	REG_TM0CNT_H = TMxCNT_H_ENABLE | TMxCNT_H_PRESCALER_64 | TMxCNT_H_IRQ_ON_OVERFLOW;

	REG_IME = ime;
}


void ttuTimerStop(void) {

	u16 ime;

	ime = REG_IME;
	REG_IME = 0;

	REG_TM0CNT_H = 0;

	REG_IME = ime;
}
