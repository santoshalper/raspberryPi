#ifndef __RPIO_H__
#define __RPIO_H_

/* GPIO PIN OUT *********************************************
* J8 CONNECTOR RASPBERRY PI B
*
*   1  3  5  7  9 11 13 15 17 19 21 23 25 27 29 31 33 35 37 39 
*   +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +
*   +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +  +
*   2  4  6  8 10 12 14 16 18 20 22 24 26 28 30 32 34 36 38 40
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
*                     GPIO (PIN) |  ALT0  |  ALT1  |  ALT2  |  ALT3  |  ALT4  |  ALT5  |
* PIN 1  -- 3.3 V DC |0(27)  HIGH|I2C0 SDA|SMI  SA5|DPI  CLK|AVEOUTVC|AVEINVCL|        |
* PIN 2  --   5 V DC |1(28)  HIGH|I2C0 SCL|SMI  SA4|DPI  DEN|AVEOUTDS|AVEINDSY|        |
* PIN 3  --   GPIO 2 |2(03)  HIGH|I2C1 SDA|SMI  SA3|DPIVSYNC|AVEOUTVS|AVEINVSY|        |
* PIN 4  --   5 V DC |3(05)  HIGH|I2C1 SCL|SMI  SA2|DPIHSYNC|AVEOUTHS|AVEINHSY|        |
* PIN 5  --   GPIO 3 |4(07)  HIGH|GPCLK  0|SMI  SA1|DPI   D0|AVEOUTV0|AVEINVI0|JTAG TDI|
* PIN 6  --   GROUND |5(29)  HIGH|GPCLk  1|SMI  SA0|DPI   D1|AVEOUTV1|AVEINVI1|JTAG TDO|
* PIN 7  --   GPIO 4 |6(31)  HIGH|GPCLK  2|SMISOE_N|DPI   D2|AVEOUTV2|AVEINVI2|JTAGRTCK|
* PIN 8  --  GPIO 14 |7(26)  HIGH|SPI0 CE1|SMISWE_N|DPI   D3|AVEOUTV3|AVEINVI3|        |
* PIN 9  --   GROUND |8(24)  HIGH|SPI0 CE0|SMI  SD0|DPI   D4|AVEOUTV4|AVEINVI4|        |
* PIN 10 --  GPIO 15 |9(21)   LOW|SPI0MISO|SMI  SD1|DPI   D5|AVEOUTV5|AVEINVI5|        |
* PIN 11 --  GPIO 17 |10(19)  LOW|SPI0MOSI|SMI  SD2|DPI   D6|AVEOUTV6|AVEINVI6|        |
* PIN 12 --  GPIO 18 |11(23)  LOW|SPI0SCLK|SMI  SD3|DPI   D7|AVEOUTV7|AVEINVI7|        |
* PIN 13 --  GPIO 27 |12(32)  LOW|PWM    0|SMI  SD4|DPI   D8|AVEOUTV8|AVEINVI8|JTAG TMS|
* PIN 14 --   GROUND |13(33)  LOW|PWM    1|SMI  SD5|DPI   D9|AVEOUTV9|AVEINVI9|JTAG TCK|
* PIN 15 --  GPIO 22 |14(08)  LOW|UART0 TX|SMI  SD6|DSI  D10|AVEOTV10|AVEINV10|UART1 TX|
* PIN 16 --  GPIO 23 |15(10)  LOW|UART0 RX|SMI  SD7|DPI  D11|AVEOTV11|AVEINV11|UART1 RX|
* PIN 17 -- 3.3 V DC |16(36)  LOW|FL     0|SMI  SD8|DPI  D12|UART0CTS|SPI1 CE2|UART1CTS|
* PIN 18 --  GPIO 24 |17(11)  LOW|FL     1|SMI  SD9|DPI  D13|UART0RTS|SPI1 CE1|UART1RTS|
* PIN 19 --  GPIO 10 |18(12)  LOW|PCM  CLK|SMI SD10|DPI  D14|I2CSLSDA|SPI1 CE0|PWM    0|
* PIN 20 --   GROUND |19(35)  LOW|PCM   FS|SMI SD11|DPI  D15|I2CSLSCL|SPI1MISO|PWM    1|
* PIN 21 --   GPIO 9 |20(38)  LOW|PCM  DIN|SMI SD12|DPI  D16|I2CSMISO|SPI1MOSI|GPCLK  0|
* PIN 22 --  GPIO 25 |21(40)  LOW|PCM DOUT|SMI SD13|DPI  D17|I2CSL CE|SPI1SCLK|GPCLK  1|
* PIN 23 --  GPIO 11 |22(15)  LOW|SD0  CLK|SMI SD14|DPI  D18|SD1  CLK|JTAGTRST|        |
* PIN 24 --   GPIO 8 |23(16)  LOW|SD0  CMD|SMI SD15|DPI  D19|SD1  CMD|JTAGRTCK|        |
* PIN 25 --   GROUND |24(18)  LOW|SD0 DAT0|SMI SD16|DPI  D20|SD1 DAT0|JTAG TDO|        |
* PIN 26 --   GPIO 7 |25(22)  LOW|SD0 DAT1|SMI SD17|DPI  D21|SD1 DAT1|JTAG TCK|        |
* PIN 27 --   GPIO 0 |26(37)  LOW|SDO DAT2|TE     0|DPI  D22|SD1 DAT2|JTAG TDI|        |
* PIN 28 --   GPIO 1 |27(13)  LOW|SDO DAT3|TE     1|DPI  D23|SD1 DAT3|JTAG TMS|        |   
* PIN 29 --   GPIO 5 |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
* PIN 30 --   GROUND |   IO CONTROL REGISTERS BASE = 0x3f20 0000                                    |
* PIN 31 --   GPIO 6 | GPFSEL0 - FUNC SEL FOR GPIO 0-9  + 0x0000                                    |
* PIN 32 --  GPIO 12 | GPFSEL1 - FUNC SEL FOR GPIO10-19 + 0x0004                                    |
* PIN 33 --  GPIO 13 | GPFSEL2 - FUNC SEL FOR GPIO20-29 + 0x0008                                    |
* PIN 34 --   GROUND | GPSET0 - SET OUTPUT  GPI0 0-31  + 0x001C                                     |
* PIN 35 --  GPIO 19 |  GPCLR0 - CLEAR OUTPUT GPIO 0-31 + 0x0028                                    |
* PIN 36 --  GPIO 16 |  GPLEV0 - RETURN VALUE GPIO 0-31 + 0x0034                                    |
* PIN 37 --  GPIO 26 |  GPEDS0 - EVENT DETECT GPIO 0-31 + 0x0040 (interrupt events)                 |
* PIN 38 --  GPIO 20 |  GPREN0 - RISE  DETECT GPIO 0-31 + 0x004C (event on GPEDS0 on rising edge)   | 
* PIN 39 --   GROUND |  GPFEN0 - FALL  DETECT GPIO 0-31 + 0x0058 (event on GPEDS0 on falling edge)  |
* PIN 40 --  GPIO 21 |  GPHEN0 - HIGH  DETECT GPIO 0-31 + 0x0064 (event on GPEDS0 on high pin)      |
*~~~~~~~~~~~~~~~~~~~~|  GPLEN0 - LOW   DETECT GPIO 0-31 + 0x0070 (event on GPEDS0 on low pin)       |
*                    |  GPAREN0 - ASYN RISE ED GPIO 0-31 + 0x007C (event on GPEDS0 on ASYNC rise)   |
*                    |  GPAFEN0 - ASYN FALL ED GPIO 0-31 + 0x0088 (event on GPEDS0 on ASYNC fall)   |
*                    | 	GPPUD - PIN PULL-UP/DOWN  0-31 + 0x0094 (used allong with GPPUDCLK)         |
*                    |  GPPUDCLK - PIN PUP/PDWN CLK  0-31 + 0x0098 (assert line onto pin)           |
*                     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

unsigned int *tmp; //temp pointer

#define INPUT  0b000
#define OUTPUT 0b001
#define ALT0   0b100
#define ALT1   0b101
#define ALT2   0b110
#define ALT3   0b111
#define ALT4   0b011
#define ALT5   0b010

//IO PERIPHERAL BASE
#define IO_P_BASE  0x3F000000
#define IO_PADS IO_P_BASE + 0x00100000 // set pad power consu

#define IO_CTRL   (IO_P_BASE + 0x00200000) // GPIO  CONTROL

#define UART_CTRL (IO_P_BASE + 0x00201000) // UART CONTROL

#define BLOCK_SIZE (4*1024)
#define PAGE_SIZE (4*1024)

#define IO_SET  7// 0x001C/0x0004
#define IO_CLR  10// 0x0028/0x0004
#define IO_LEV  13// 0x0034/0x0004
#define IO_EDS  16// 0x0040/0x0004
#define IO_RIS  19// 0x004C/0x0004
#define IO_FAL  22// 0x0058/0x0004
#define IO_EHI  25// 0x0064/0x0004
#define IO_ELO  28// 0x0070/0x0004
#define IO_ARIS 31// 0x007C/0x0004
#define IO_AFAL 34// 0x0088/0x0004



#define SETFUNC(a,b) *(gpio+(a/10)) |= (b << ((a%10)*3))
#define SETEVENT(a,b)  *(gpio+b) |= (1 << a) 
#define SETPIN(a) *(gpio+IO_SET) |= (1 << a)
#define CLRPIN(a) *(gpio+IO_CLR) |= (1 << a)
#define CHKPIN(a) (*(gpio+IO_LEV)&(1 << a))
#define CHKEVN(a) (*(gpio+IO_EDS)&(1 << a))
#define CHKEVNS   (*(gpio+IO_EDS)&0xFFFF) 
#define CLREVN(a) *(gpio+IO_EDS) |= (1 << a)


int mem_fd;
void *gpio_map;
volatile unsigned *gpio;

void *uart_map; 
volatile unsigned *uart; 
#define UARTFR  (void*)((unsigned int)uart + 0x18)
#define IBRD  (void*)((unsigned int)uart + 0x24)
#define FBRD  (void*)((unsigned int)uart + 0x28)
#define LCRH  (void*)((unsigned int)uart + 0x2C)
#define UARTCR  (void*)((unsigned int)uart + 0x30)



void GpioInit();
void midi_uart_init();
int midi_uart_out(char *);
int midi_write(char, char, char, char);
void midi_uart_close(); 
#endif
