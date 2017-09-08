#include <RPio.h>
#include <midi.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h> 
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

struct termios opt;
int uart_file;

void midi_uart_init(){
	unsigned int brd;
	SETFUNC(14,ALT0);
	SETFUNC(15,ALT0);
        uart_file = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
        if (uart_file == -1) {
	   printf("Error - Unable to open UART\n");
	   exit(-1);
	}
	tcgetattr(uart_file, &opt);
	opt.c_lflag &= ~ISIG;
	opt.c_lflag &= ~ICANON;
	opt.c_lflag &= ~ECHO;
	opt.c_lflag &= ~NOFLSH;
	opt.c_lflag &= ~IEXTEN;
	opt.c_cc[VMIN] = 0;
	opt.c_cc[VTIME] = 0;
	tcsetattr(uart_file, TCSANOW, &opt);

	brd = 0x0B00;
	memcpy(UARTCR,&brd,4);
	memcpy(&brd,UARTFR,4);
	while(brd & 0x08)
	  memcpy(&brd,UARTFR,4);
	brd = 96;	
	memcpy(IBRD,&brd,4);
	brd = 0;
	memcpy(FBRD,&brd,4);
	brd = 0x60;
	memcpy(LCRH,&brd,4);
	brd = 0x0B01;
	memcpy(UARTCR,&brd,4);
	munmap(uart_map,BLOCK_SIZE);
}

int midi_uart_out(char *mess) {
	int length, chk,byte;
	byte = 0;
	if(*(mess+2) == EMTY_DAT)
	  length = 2;
        else length = 3; 
        if (uart_file != -1) {
         while(byte<length) {
           chk = write(uart_file, mess+byte, 1);
	   byte++;
           if (chk<0) {
	      printf("UART tx error : %d\n",errno);
	      return -1;
           }
	  }
	}
        else {
	   printf("uart_file not open bro\n");
	   return -1;
        }
	return 0;
}

int midi_write(char type, char chnl, char data1, char data2) {
	char buf[2];
        buf[0] = type+chnl;
	buf[1] = data1&DATA_MSK;
        if(data2 != EMTY_DAT) 
          buf[2] = data2&DATA_MSK;
	else buf[2] = EMTY_DAT;
	if(midi_uart_out(buf) == -1){
	   printf("uart failled b\n");
	   return -1;
        }
        else return 0;
}
void midi_uart_close() {
	close(uart_file);
}
