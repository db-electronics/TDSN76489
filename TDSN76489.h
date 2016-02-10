 /*
    Title:          TDSN76489.h
    Author:         René Richard
    Description:
        
    Target Hardware:
        Teensy 3.2 + Audio Adapter
    Arduino IDE settings:
        Board Type  - Teensy 3.2
        USB Type    - Serial
 LICENSE
 
    This file is part of TDSN76489.
    TDSN76489 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with TDSN76489.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TDSN76489_h_
#define TDSN76489_h_

#include <AudioStream.h>
#include <inttypes.h>

#define SN76489CLOCK		3579545
#define CLOCKSPERSAMPLE		SN76489CLOCK / AUDIO_SAMPLE_RATE_EXACT
#define CLOCKSPERUPDATE		SN76489CLOCK / (AUDIO_SAMPLE_RATE_EXACT / AUDIO_BLOCK_SAMPLES)	

/* Default settings */
#define NOISE_TAPPED_NORMAL 0x0006
#define NOISE_BITS_NORMAL   15
#define NOISE_TAPPED_SMS    0x0009
#define NOISE_BITS_SMS      16

/* Registers */
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

class AudioTDSN76489 : public AudioStream
{
	public:
		AudioTDSN76489(void) : AudioStream(0, NULL) { reset(NOISE_BITS_SMS, NOISE_TAPPED_SMS); }
		AudioTDSN76489(uint16_t noise_bits, uint16_t tapped) : AudioStream(0, NULL) { reset(noise_bits, tapped); }
		void reset(uint16_t noise_bits, uint16_t tapped);
		void write(uint8_t data);
		virtual void update(void);
		void setOutput(uint8_t data); 
		inline bool isPlaying(void) { return playing; }
	
	private:

		volatile bool playing;
		typedef struct sn76489_struct {
			uint8_t volume[4];
			uint16_t tone[3];
			uint8_t noise;
			uint16_t noise_shift;
			uint16_t noise_bits;
			uint16_t noise_tapped;
			int8_t tone_state[4];
			uint8_t latched_reg;
			float counter[4];
			uint8_t enabled_channels;
			uint8_t output_channels;
			uint32_t channel_masks[2][4];
			float clocks_per_sample;
		} psg;
		
		void AudioTDSN76489::execute(uint16_t *buf, uint32_t samples);

		/* These constants came from Maxim's core (then doubled). */
		static const uint16_t volume_values[16] = { 
    		1784, 1548, 1338, 1150,  984,  834,  702,  584,
     		478,  384,  300,  226,  160,  100,   48,    0
		};	
};

#endif
