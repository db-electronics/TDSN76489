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

//reference http://www.smspower.org/Development/SN76489 throughout

#include "TDSN76489.h"

void AudioTDSN76489::reset(uint16_t noise_bits, uint16_t tapped) 
{
    psg.volume[0] = 0x0F;
    psg.volume[1] = 0x0F;
    psg.volume[2] = 0x0F;
    psg.volume[3] = 0x0F;

    psg.tone[0] = 0;
    psg.tone[1] = 0;
    psg.tone[2] = 0;
    psg.noise = 0;
	psg.noiseType = 0;

    psg.latched_reg = LATCH_TONE0;

    psg.counter[0] = 0;
    psg.counter[1] = 0;
    psg.counter[2] = 0;
    psg.counter[3] = 0;

    psg.tone_state[0] = 1;
    psg.tone_state[1] = 1;
    psg.tone_state[2] = 1;
    psg.tone_state[3] = 1;

	//clocks per sample is 32bit value, psg.counter is also 32bit (max write value of 0x3FF -> 1023 (10bits))
	//for most best precision and best use of 32bits, we can scale the value by 21 bits maximum
	//I'll scale by 16bits to be reasonable
	//FYI, this places the binary point at the sixteenth bit and allows for increased precision to due the 
	//sample rate not being a precise multiple of the PSG clock
	psg.clockspersample = (SN76489CLOCK / 16.0 / AUDIO_SAMPLE_RATE) * (2^16);

    psg.noise_shift = (1 << (noise_bits - 1));
    psg.noise_tapped = tapped;
    psg.noise_bits = noise_bits;
}

//write an 8bit value to the SN76489, this simulates an 8bit write to the physical chip
void AudioTDSN76489::write(uint8_t data) 
{    
	//If bit 7 is 1 then the byte is a LATCH/DATA byte
	if(data & 0x80) 
	{
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
				//check state of bit2
				psg.noiseType = (data & 0x04);
				//reset shift register
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
	//If bit 7 is 0 then the byte is a DATA byte
    }else
	{
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
				//check state of bit2
				psg.noiseType = (data & 0x04);
				//reset shift register
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

//Mute all PSG channels
void AudioTDSN76489::muteAllChannels(void)
{
	write(0x9F);  //channel 0 volume off
	write(0xBF);  //channel 1 volume off
	write(0xDF);  //channel 2 volume off
	write(0xFF);  //channel 3 volume off
}

//Set Channel Volume, 4bits, 0 is loudest, F is off
void AudioTDSN76489::setVolume(uint32_t channel, uint8_t value)
{
	value &= 0x0F;
	switch(channel)
	{
		case 0:
			write( 0x90 | value );
			break;
		case 1:
			write( 0xB0 | value );
			break;
		case 2:
			write( 0xD0 | value );
			break;
		case 3:
			write( 0xF0 | value );
			break; 
		default:
			break;
	}	
}

//Set Tone Counter 10bit value
void AudioTDSN76489::setToneCounter(uint32_t channel, uint16_t value)
{
	value &= 0x03FF;
	switch(channel)
	{
		case 0:
			write( (uint8_t)(0x80 | (value & 0x0F)) );
			write( (uint8_t)(value>>4 & 0x3F) );
			break;
		case 1:
			write( (uint8_t)(0xA0 | (value & 0x0F)) );
			write( (uint8_t)(value>>4 & 0x3F) );
			break;
		case 2:
			write( (uint8_t)(0xC0 | (value & 0x0F)) );
			write( (uint8_t)(value>>4 & 0x3F) );
			break; 
		default:
			break;
	}
}

//Set Tone Counter value by specifying a MIDI note number (defined in MIDINOTES.h)
void AudioTDSN76489::setNote(uint32_t channel, uint8_t midiNoteNum)
{
	if( midiNoteNum > 108 ) return;

	uint16_t value = midi[midiNoteNum];
	switch(channel)
	{
		case 0:
			write( (uint8_t)(0x80 | (value & 0x0F)) );
			write( (uint8_t)(value>>4 & 0x3F) );
			break;
		case 1:
			write( (uint8_t)(0xA0 | (value & 0x0F)) );
			write( (uint8_t)(value>>4 & 0x3F) );
			break;
		case 2:
			write( (uint8_t)(0xC0 | (value & 0x0F)) );
			write( (uint8_t)(value>>4 & 0x3F) );
			break; 
		default:
			break;
	}
}

//parity function taken straight from http://www.smspower.org/Development/SN76489
inline int AudioTDSN76489::parity(int input) 
{
    input ^= input >> 8;
    input ^= input >> 4;
    input ^= input >> 2;
    input ^= input >> 1;
    return input & 1;
}

//Update function is called by the Teensy audio libray, this is what fills the audio output buffer
void AudioTDSN76489::update(void)
{
	audio_block_t *block;
    int16_t channels[4];
    uint32_t sampleCount, chNum;

	// only update if we're playing
	if (!playing) return;

	// allocate the audio blocks to transmit
	block = allocate();
	if (block == NULL) return;

    for(sampleCount = 0; sampleCount < AUDIO_BLOCK_SAMPLES; sampleCount++) 
	{
		//process the three tone channels
        for(chNum = 0; chNum < 3; chNum++) 
		{
			//decrement counter by CLOCKSPERSAMPLE	
            psg.counter[chNum] -= psg.clockspersample;

			//have we reached count 0
            if(psg.counter[chNum] <= 0) {
                if(psg.tone[chNum] < 7) {
                    /* The PSG doesn't change states if the tone isn't at least
                       7, this fixes the "Sega" at the beginning of Sonic The
                       Hedgehog 2 for the Game Gear. */
                    psg.tone_state[chNum] = 1;
                }else 
				{
					//invert the output bit
                    psg.tone_state[chNum] = -psg.tone_state[chNum];
                }
				//reset counter, binary point at bit 16
                psg.counter[chNum] += (psg.tone[chNum] * (2^16));
            }
			//essentially -1 or 1 * channel volume			
            channels[chNum] = psg.tone_state[chNum] * volume_values[psg.volume[chNum]];
        }

		//process the noise channel
		//decrement counter by CLOCKSPERSAMPLE
        psg.counter[3] -= psg.clockspersample;
        
        if(psg.counter[3] < 0) 
		{
			//invert the output bit
            psg.tone_state[3] = -psg.tone_state[3];
            
			//are we using channel 2's counter as source
			if((psg.noise & 0x03) == 0x03)
			{
				//reset to channel 2 counter value
                psg.counter[3] = psg.counter[2];
            }else 
			{
				//reset to 0x10, 0x20 or 0x40, binary point at bit 16
                psg.counter[3] += (0x10 << (psg.noise & 0x03)) * (2^16) ;
            }

			//only shift when the array transitions from 0 to 1 (i.e. every other time)
            if(psg.tone_state[3] == 1) 
			{
				//check noise type
                if(psg.noiseType) 
				{
					//white noise
                    psg.noise_shift = (psg.noise_shift >> 1) | (parity(psg.noise_shift & psg.noise_tapped) << (psg.noise_bits - 1));
                }else 
				{
					//periodic noise
                    psg.noise_shift = (psg.noise_shift >> 1) | ((psg.noise_shift & 0x01) << (psg.noise_bits - 1));
                }

				/* from SMSPower
				ShiftRegister = (ShiftRegister >> 1) | ((WhiteNoise ? parity(ShiftRegister & TappedBits) : ShiftRegister & 1) << 15 );
				*/
            }
        }

		//essentially 0 or 1 * channel volume	
		//channels[3] = (psg.noise_shift & 0x01) * volume_values[psg.volume[3]];

		/* to try, seems more right to me since the other channels use -1 and +1 */
		if( psg.noise_shift & 0x01 )
		{
			channels[3] = volume_values[psg.volume[3]];
		}else
		{
			channels[3] = -volume_values[psg.volume[3]];
		}
		
        block->data[sampleCount] = ( channels[0] + channels[1] + channels[2] + channels[3] );
    }	

	transmit(block);
	release(block);
}
