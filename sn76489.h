#ifndef TDSN76489_H
#define TDSN76489_H

#include <AudioStream.h>

#define SAMPLERATE 44118
#define CLOCKFREQ (22.5 * SAMPLERATE) //nearest int to 985248

typedef struct sn76489_struct {
    uint8 volume[4];
    uint16 tone[3];
    uint8 noise;

    uint16 noise_shift;
    uint16 noise_bits;
    uint16 noise_tapped;

    int8 tone_state[4];

    uint8 latched_reg;

    float counter[4];

    uint8 enabled_channels;

    uint8 output_channels;
    uint32 channel_masks[2][4];

    float clocks_per_sample;
} sn76489_t;

#define LATCH_TONE0 0x00
#define LATCH_TONE1 0x20
#define LATCH_TONE2 0x40
#define LATCH_NOISE 0x60

#define LATCH_VOL0 0x10
#define LATCH_VOL1 0x30
#define LATCH_VOL2 0x50
#define LATCH_VOL3 0x70

#define ENABLE_TONE0 0x01
#define ENABLE_TONE1 0x02
#define ENABLE_TONE2 0x04
#define ENABLE_NOISE 0x08

/* Channel outputs */
#define TONE0_RIGHT 0x01
#define TONE1_RIGHT 0x02
#define TONE2_RIGHT 0x04
#define NOISE_RIGHT 0x08
#define TONE0_LEFT  0x10
#define TONE1_LEFT  0x20
#define TONE2_LEFT  0x40
#define NOISE_LEFT  0x80

/* Default settings */
#define SN76489_NOISE_TAPPED_NORMAL 0x0006
#define SN76489_NOISE_BITS_NORMAL   15

#define SN76489_NOISE_TAPPED_SMS    0x0009
#define SN76489_NOISE_BITS_SMS      16

#define SN76489_NOISE_TAPPED_SG1000 SN76489_NOISE_TAPPED_NORMAL
#define SN76489_NOISE_BITS_SG1000   SN76489_NOISE_BITS_NORMAL

int sn76489_init(
		sn76489_t *psg, 
		float clock, 
		float sample_rate,
     	uint16 noise_bits, 
		uint16 tapped);

int sn76489_reset(
		sn76489_t *psg, 
		float clock, 
		float sample_rate,
      	uint16 noise_bits, 
		uint16 tapped);

void sn76489_write(
		sn76489_t *psg, 
		uint8 byte);

void sn76489_execute_samples(
		sn76489_t *psg, 
		int16 *buf,
     	uint32 samples);

void sn76489_set_output_channels(
		sn76489_t *psg, 
		uint8 data);

#endif
