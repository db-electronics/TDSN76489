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

#include "TDSN76489.h"

void AudioTDSN76489::reset(uint16_t noise_bits, uint16_t tapped) 
{
    psg.volume[0] = 0xF;
    psg.volume[1] = 0xF;
    psg.volume[2] = 0xF;
    psg.volume[3] = 0xF;

    psg.tone[0] = 0x00;
    psg.tone[1] = 0x00;
    psg.tone[2] = 0x00;
    psg.noise = 0x00;

    psg.latched_reg = LATCH_TONE0;

    psg.counter[0] = 0x00;
    psg.counter[1] = 0x00;
    psg.counter[2] = 0x00;
    psg.counter[3] = 0x00;

    psg.tone_state[0] = 1;
    psg.tone_state[1] = 1;
    psg.tone_state[2] = 1;
    psg.tone_state[3] = 1;

	psg.clockspersample = (SN76489CLOCK / 16.0f / AUDIO_SAMPLE_RATE_EXACT);

    psg.noise_shift = (1 << (noise_bits - 1));
    psg.noise_tapped = tapped;
    psg.noise_bits = noise_bits;
}

void AudioTDSN76489::write(uint8_t data) 
{    
	if(data & 0x80) {
        /* This is a LATCH/DATA byte */
        psg.latched_reg = (data & 0x70);

        switch(psg.latched_reg) {
            case LATCH_TONE0:
                psg.tone[0] = (psg.tone[0] & 0x3F0) | (data & 0x0F);
                break;
            case LATCH_TONE1:
                psg.tone[1] = (psg.tone[1] & 0x3F0) | (data & 0x0F);
                break;
            case LATCH_TONE2:
                psg.tone[2] = (psg.tone[2] & 0x3F0) | (data & 0x0F);
                break;
            case LATCH_NOISE:
                psg.noise = (data & 0x07);
                psg.noise_shift = 1 << (psg.noise_bits - 1);
                break;
            case LATCH_VOL0:
                psg.volume[0] = (data & 0x0F);
                break;
            case LATCH_VOL1:
                psg.volume[1] = (data & 0x0F);
                break;
            case LATCH_VOL2:
                psg.volume[2] = (data & 0x0F);
                break;
            case LATCH_VOL3:
                psg.volume[3] = (data & 0x0F);
                break;
        }
    }
    else {
        /* This is a DATA byte */
        switch(psg.latched_reg) {
            case LATCH_TONE0:
                psg.tone[0] = (psg.tone[0] & 0x000F) | ((data & 0x3F) << 4);
                break;
            case LATCH_TONE1:
                psg.tone[1] = (psg.tone[1] & 0x000F) | ((data & 0x3F) << 4);
                break;
            case LATCH_TONE2:
                psg.tone[2] = (psg.tone[2] & 0x000F) | ((data & 0x3F) << 4);
                break;
            case LATCH_NOISE:
                psg.noise = (data & 0x07);
                psg.noise_shift = 1 << (psg.noise_bits - 1);
                break;
            case LATCH_VOL0:
                psg.volume[0] = (data & 0x0F);
                break;
            case LATCH_VOL1:
                psg.volume[1] = (data & 0x0F);
                break;
            case LATCH_VOL2:
                psg.volume[2] = (data & 0x0F);
                break;
            case LATCH_VOL3:
                psg.volume[3] = (data & 0x0F);
                break;
        }
    }
}

/* This is pretty much taken directly from Maxim's SN76489 document. */
int AudioTDSN76489::parity(int input) 
{
    input ^= input >> 8;
    input ^= input >> 4;
    input ^= input >> 2;
    input ^= input >> 1;
    return input & 1;
}

void AudioTDSN76489::update(void)
{
	audio_block_t *block;
    int16_t channels[4];
    uint32_t sampleNum, chNum;

	// only update if we're playing
	if (!playing) return;


	// allocate the audio blocks to transmit
	block = allocate();
	if (block == NULL) return;
	
    for(sampleNum = 0; sampleNum < AUDIO_BLOCK_SAMPLES; sampleNum++) {
        for(chNum = 0; chNum < 2; chNum++) 
		{
			//decrement counter by CLOCKPERSAMPLE	
            psg.counter[chNum] -= psg.clockspersample;

			//essentially -1 or 1 * channel volume			
            channels[chNum] = psg.tone_state[chNum] * volume_values[psg.volume[chNum]];
			
			//have we reached count 0
            if(psg.counter[chNum] <= 0.0f) {
                if(psg.tone[chNum] < 7) {
                    /* The PSG doesn't change states if the tone isn't at least
                       7, this fixes the "Sega" at the beginning of Sonic The
                       Hedgehog 2 for the Game Gear. */
                    psg.tone_state[chNum] = 1;
                }
                else {
                    psg.tone_state[chNum] = -psg.tone_state[chNum];
                }
				//reset counter
                psg.counter[chNum] += psg.tone[chNum];
            }
        }

        channels[3] = (psg.noise_shift & 0x01) * volume_values[psg.volume[3]];

        psg.counter[3] -= psg.clockspersample;
        
        if(psg.counter[3] < 0.0f) {
            psg.tone_state[3] = -psg.tone_state[3];
            if((psg.noise & 0x03) == 0x03) {
                psg.counter[3] = psg.counter[2];
            }
            else {
                psg.counter[3] += 0x10 << (psg.noise & 0x03);
            }

            if(psg.tone_state[3] == 1) {
                if(psg.noise & 0x04) {
                    psg.noise_shift = (psg.noise_shift >> 1) |
                        (parity(psg.noise_shift & psg.noise_tapped) <<
                        (psg.noise_bits - 1));
                }
                else {
                    psg.noise_shift = (psg.noise_shift >> 1) |
                        ((psg.noise_shift & 0x01) << (psg.noise_bits - 1));
                }
            }
        }
        block->data[sampleNum] = ( channels[0] + channels[1] + channels[2] );
    }	

	//execute((short int)*block->data, AUDIO_BLOCK_SAMPLES);
	transmit(block);
	release(block);
}

void AudioTDSN76489::execute(int16_t * buf, uint32_t samples)
{
    int32_t channels[4];
    uint32_t sampleNum, chNum;

    for(sampleNum = 0; sampleNum < samples; sampleNum++) {
        for(chNum = 0; chNum < 2; chNum++) 
		{
			//decrement counter by CLOCKPERSAMPLE	
            psg.counter[chNum] -= psg.clockspersample;

			//essentially -1 or 1 * channel volume			
            channels[chNum] = psg.tone_state[chNum] * volume_values[psg.volume[chNum]];
			
			//have we reached count 0
            if(psg.counter[chNum] <= 0.0f) {
                if(psg.tone[chNum] < 7) {
                    /* The PSG doesn't change states if the tone isn't at least
                       7, this fixes the "Sega" at the beginning of Sonic The
                       Hedgehog 2 for the Game Gear. */
                    psg.tone_state[chNum] = 1;
                }
                else {
                    psg.tone_state[chNum] = -psg.tone_state[chNum];
                }
				//reset counter
                psg.counter[chNum] += psg.tone[chNum];
            }
        }

        channels[3] = (psg.noise_shift & 0x01) * volume_values[psg.volume[3]];

        psg.counter[3] -= psg.clockspersample;
        
        if(psg.counter[3] < 0.0f) {
            psg.tone_state[3] = -psg.tone_state[3];
            if((psg.noise & 0x03) == 0x03) {
                psg.counter[3] = psg.counter[2];
            }
            else {
                psg.counter[3] += 0x10 << (psg.noise & 0x03);
            }

            if(psg.tone_state[3] == 1) {
                if(psg.noise & 0x04) {
                    psg.noise_shift = (psg.noise_shift >> 1) |
                        (parity(psg.noise_shift & psg.noise_tapped) <<
                        (psg.noise_bits - 1));
                }
                else {
                    psg.noise_shift = (psg.noise_shift >> 1) |
                        ((psg.noise_shift & 0x01) << (psg.noise_bits - 1));
                }
            }
        }

        buf[sampleNum] = ( channels[0] + channels[1] + channels[2] );
    }
}

