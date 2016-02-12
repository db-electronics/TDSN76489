 /*
    Title:          PlayVGMFromMemory.ino
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

#include <TDSN76489.h>
#include <Audio.h>
#include <SPI.h>
#include <SD.h>
#include "SonicTitleScreen.h"

#define ONESAMPLE   ( 1 / AUDIO_SAMPLE_RATE_EXACT ) * 1000000   // microseconds per audio sample
#define ONE60TH     ( 1 / 60 ) * 1000000
#define ONE50TH     ( 1 / 50 ) * 1000000

elapsedMicros vgmTimer;
uint16_t vgmWait;
//hard code to start at 0x40 since I know this is a vgm 1.10 file
uint8_t *vgmptr = &TitleScreen[0x40];

AudioTDSN76489           psgChip;  //xy=189,110
AudioOutputI2S           i2s1;           //xy=366,111
AudioConnection          patchCord1(psgChip, 0, i2s1, 0);
AudioConnection          patchCord2(psgChip, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=354,176

void setup() {

  AudioMemory(10);
  
  psgChip.reset(NOISE_BITS_SMS, NOISE_TAPPED_SMS);
  
  SPI.setSCK(14);      
  SPI.setMOSI(7);
  SPI.setMISO(12);
  SPI.begin();
  
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.9);
  sgtl5000_1.enhanceBassEnable();
  sgtl5000_1.enhanceBass(0.5, 2.5);

  delay(2000);

  vgmWait = 0;

}

void loop() {

  bool doneFrame = false;

  if( vgmTimer < vgmWait ) return;

  vgmTimer = 0;

  while(doneFrame == false)
  {
    switch(*vgmptr)
    {
      case 0x50: // 0x50 dd : PSG (SN76489/SN76496) write value dd
          vgmptr++;
          psgChip.write(*vgmptr);
          vgmptr++;
        break;
      case 0x61: // 0x61 nn nn : Wait n samples, n can range from 0 to 65535
        vgmptr++;
        vgmWait = (uint16_t)( 0x00ff & *vgmptr);
        vgmptr++;
        vgmWait |= (uint16_t)( 0xff00 & *vgmptr << 8);
        vgmptr++;
        doneFrame = true;
        break;
        
      case 0x62: // wait 735 samples (60th of a second)
        vgmWait = ONE60TH;
        vgmptr++;
        doneFrame = true;
        break;
        
      case 0x63: // wait 882 samples (50th of a second)
        vgmWait = ONE50TH;
        vgmptr++;
        doneFrame = true;
        break;
        
      case 0x70: // 0x7n : wait n+1 samples, n can range from 0 to 15
      case 0x71:
      case 0x72:
      case 0x73:
      case 0x74:
      case 0x75:
      case 0x76:
      case 0x77:
      case 0x78:
      case 0x79:
      case 0x7A:
      case 0x7B:
      case 0x7C:
      case 0x7D:
      case 0x7E:
      case 0x7F:
        vgmWait = (ONESAMPLE * (*vgmptr & 0x0f));
        vgmptr++;
        doneFrame = true;
        break;
        
      case 0x66: // 0x66 : end of sound data
        vgmptr = &TitleScreen[0x40];
        doneFrame = true;
        break;
        
      default:
        break;
    }
  }
}



