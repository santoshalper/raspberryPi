#ifndef MIDI_H
#define MIDI_H
/*
	   MIDI COMMANDS
	   -------------------------------------------------------------------
	   name                 status      param 1          param 2
	   -------------------------------------------------------------------
	   note off             0x80+C       key #            velocity
	   note on              0x90+C       key #            velocity
	   poly key pressure    0xA0+C       key #            pressure value
	   control change       0xB0+C       control #        control value
	   program change       0xC0+C       program #        --
	   mono key pressure    0xD0+C       pressure value   --
	   pitch bend           0xE0+C       range (LSB)      range (MSB)
	   system               0xF0+C       manufacturer     model
	   -------------------------------------------------------------------
	   C is the channel number, from 0 to 15;
	   -------------------------------------------------------------------
	   source: http://ftp.ec.vanderbilt.edu/computermusic/musc216site/MIDI.Commands.html
	
	   In this program the pitch bend range will be transmitter as 
	   one single 8-bit number. So the end result is that MIDI commands 
	   will be transmitted as 3 bytes, starting with the operation byte:
	
	   buf[0] --> operation/channel
	   buf[1] --> param1
	   buf[2] --> param2        (param2 not transmitted on program change or key press)
   */

#define NOTE_ON  (char)0x90	
#define NOTE_OFF (char)0x80
#define POLY_CHG (char)0xA0
#define CTRL_CHG (char)0xB0
#define PRGM_CHG (char)0xC0
#define MONO_CHG (char)0xD0
#define PTCH_BND (char)0xE0
#define SYST_SHT (char)0xF0
#define DATA_MSK (char)0x7F
#define EMTY_DAT (char)0xFF

#define C   0x00
#define CS  0x01
#define D   0x02
#define DS  0x03
#define E   0x04
#define F   0x05
#define FS  0x06
#define G   0x07
#define GS  0x08
#define A   0x09
#define AS  0x0A
#define B   0x0B

#define NOTE(a,b) (char)((a+(b*0x0C))&0X7F)

#endif
