# TDSN76489
SN76489 implementation for Teensyduino Audio Library

Information regarding the implementation of the Sega Master System's variant of the SN76489 can be found at:
http://www.smspower.org/Development/SN76489

Example sketches in this library use VGM files to test the SN76489. VGM files for Master System are commonly found in compressed format (7z) on the web. To use them with this libary you will need to extract them:

Linux users:

7za -e %vgmfile%

Afterwards, if you wish to generate a header file which can be included in a project:

xxd --include %extractedvgmfile% &> %file%


Windows users:
ask google
