#include <RPio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void GpioInit(){
        if((mem_fd=open("/dev/mem", O_RDWR|O_SYNC) ) < 0){
	  printf("failed to open memfile.\n");
	  exit(-1);
        }
        gpio_map = mmap(
	       NULL,
	       BLOCK_SIZE,
	       PROT_READ|PROT_WRITE,
	       MAP_SHARED,
	       mem_fd,
               IO_CTRL
	);
	uart_map = mmap(
		NULL,
                BLOCK_SIZE,
                PROT_READ|PROT_WRITE, 
                MAP_SHARED, 
                mem_fd,
                UART_CTRL
	);
        close(mem_fd);
	if(gpio_map == MAP_FAILED) {
	   printf("mmap error %d\n", (int)gpio_map);
	   exit(-1);
        }
        if (uart_map == MAP_FAILED) {
 	   printf("uart map failed : %d\n", (int)uart_map);
	   exit(-1);
	}
	gpio = (volatile unsigned int *)gpio_map;
        uart = (volatile unsigned int *)uart_map;   
 }
