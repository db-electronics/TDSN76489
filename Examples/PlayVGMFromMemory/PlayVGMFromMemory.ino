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

AudioTDSN76489 psgChip;

void setup() {
  // put your setup code here, to run once:
  psgChip.reset(NOISE_BITS_SMS, NOISE_TAPPED_SMS);
  psgChip.write(0x00);
}

void loop() {
  // put your main code here, to run repeatedly:

}
