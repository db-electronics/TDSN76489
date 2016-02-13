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
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "SonicTitleScreen.h"

#define ONESAMPLE   ( 1 / AUDIO_SAMPLE_RATE ) * 1000000   // microseconds per audio sample
#define ONE60TH     ( 1 / 60 ) * 1000000
#define ONE50TH     ( 1 / 50 ) * 1000000

//#define DEBUGWAIT   

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

  Serial.begin(9600); // USB is always 12 Mbit/sec

  AudioMemory(10);
  
  psgChip.reset(NOISE_BITS_SMS, NOISE_TAPPED_SMS);
  
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.9);

  delay(4000);
  Serial.println("Play VGM From Memory starting...");
  vgmWait = 0;
  Serial.print("Audio sample rate: ");
  Serial.println(AUDIO_SAMPLE_RATE, DEC);
  Serial.print("Audio block samples: ");
  Serial.println(AUDIO_BLOCK_SAMPLES, DEC);
  Serial.print("PSG clocks per sample: ");
  Serial.println((SN76489CLOCK / 16.0f / AUDIO_SAMPLE_RATE), DEC);
  delay(1000);

  //mute the psg
  psgChip.write(0x9F);  //channel 1 volume off
  psgChip.write(0xBF);  //channel 2 volume off
  psgChip.write(0xDF);  //channel 3 volume off
  psgChip.write(0xFF);  //channel 4 volume off
  psgChip.play(true);
  
  //beep each tone channel for testing
  psgChip.write(0x80);
  psgChip.write(0x0F); //note on channel 1
  psgChip.write(0x91); //volume on channel 1 
  delay(500);
  psgChip.write(0x9F);  //channel 1 volume off
  delay(500);
  psgChip.write(0xA0);
  psgChip.write(0x0C); //note on channel 2
  psgChip.write(0xB1); //volume on channel 2 
  delay(500);
  psgChip.write(0xBF);  //channel 2 volume off 
  delay(500);
  psgChip.write(0xC0);
  psgChip.write(0x08); //note on channel 2
  psgChip.write(0xD1); //volume on channel 2 
  delay(500);
  psgChip.write(0xDF);  //channel 2 volume off
  delay(2000); 
}

void loop() {

  bool doneFrame = false;
  uint8_t dummyRead;

  //return;

  if( vgmTimer < vgmWait ) return;

  //Serial.println("vgmTimer reset");
  vgmTimer = 0;

  while(doneFrame == false)
  {
    switch(*vgmptr)
    {
      case 0x50: // 0x50 dd : PSG (SN76489/SN76496) write value dd
          vgmptr++;
          psgChip.write(*vgmptr);
          //Serial.print("psg write ");
          //Serial.println(*vgmptr, HEX);
          vgmptr++;
        break;
      case 0x61: // 0x61 nn nn : Wait n samples, n can range from 0 to 65535
        vgmptr++;
        vgmWait = (uint16_t)( 0x00ff & *vgmptr);
        vgmptr++;
        vgmWait |= (uint16_t)( 0xff00 & *vgmptr << 8);
        vgmptr++;
        //Serial.print("wait ");
        //Serial.println(vgmWait, DEC);
#ifdef DEBUGWAIT
        while(!Serial.available());
        dummyRead = Serial.read();
#endif
        doneFrame = true;
        break;
        
      case 0x62: // wait 735 samples (60th of a second)
        vgmWait = ONE60TH;
        vgmptr++;
        //Serial.println("wait one 60th");
#ifdef DEBUGWAIT
        if( psgChip.isPlaying() )
        {
          Serial.println("psgChip is playing");
        }else
        {
          Serial.println("psgChip is not playing");
        }
        while(!Serial.available());
        dummyRead = Serial.read();
#endif
        doneFrame = true;
        break;
        
      case 0x63: // wait 882 samples (50th of a second)
        vgmWait = ONE50TH;
        vgmptr++;
        //Serial.println("wait one 50th");
#ifdef DEBUGWAIT
        while(!Serial.available());
        dummyRead = Serial.read();
#endif
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
        //Serial.print("wait ");
        //Serial.println(vgmWait, DEC);
#ifdef DEBUGWAIT
        while(!Serial.available());
        dummyRead = Serial.read();
#endif
        doneFrame = true;
        break;
        
      case 0x66: // 0x66 : end of sound data
        vgmptr = &TitleScreen[0x40];
        //Serial.println("song over");
#ifdef DEBUGWAIT
        while(!Serial.available());
        dummyRead = Serial.read();
#endif
        doneFrame = true;
        break;
        
      default:
        break;
    }
  }
}



