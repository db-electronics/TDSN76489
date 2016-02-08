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
#define SAMPLERATE 			44118
#define CLOCKSPERSAMPLE		SN76489CLOCK / SAMPLERATE
#define CLOCKSPERUPDATE		SN76489CLOCK / (SAMPLERATE / AUDIO_BLOCK_SAMPLES)	

class AudioTDSN76489 : public AudioStream
{
	public:
		AudioTDSN76489(int type) : AudioStream(0, NULL) { SN76489_Init(void); }
		void init(void);
		void reset(void);
		void config(unsigned int clocks, int preAmp, int boostNoise, int stereo);
		void write(unsigned int clocks, unsigned int data);

		inline bool isPlaying(void) { return playing; }
	
	private:
		volatile bool playing;
		virtual void update(void);
		sn76489_t psg;
		
		/* These constants came from Maxim's core (then doubled). */
		static const int volume_values[16] = { 
    		1784, 1548, 1338, 1150,  984,  834,  702,  584,
     		478,  384,  300,  226,  160,  100,   48,    0
		};	
};

#endif
