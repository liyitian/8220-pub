#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include "reg.h"

struct u_kyouko_device{
unsigned int *u_control_base;
unsigned int *u_framebuffer_base;
} kyouko3;

#define KYOUKO_CONTROL_SIZE (65535)

struct fifo_entry{
   unsigned int command;
   unsigned int value;
};

unsigned int U_READ_REG(unsigned int rgister){

	   return (*(kyouko3.u_control_base+(rgister>>2)));
}


void U_WRITE_FB(unsigned int address, unsigned int value){
	*(kyouko3.u_framebuffer_base +address) = value;
}

int main()
{

	int fd;
	int result;
	unsigned int i;
	struct fifo_entry entry = {
	Flush,
	0
	};
	fd = open("/dev/kyouko3", O_RDWR);
	
	kyouko3.u_control_base = mmap(0, KYOUKO_CONTROL_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	kyouko3.u_framebuffer_base = mmap(0, 1024*768*4, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x80000000);

	
	ioctl(fd, VMODE, GRAPHICS_ON);
		
	for (i = 200*1024; i < 201*1024; ++i)
	{
		U_WRITE_FB(i, 0xff000000);
	}


	//entry.command = Flush;
	//entry.value = 0;
	printf("address: %x\n", &entry);
	printf("user_queue: %x , %d\n", entry.command, entry.value);
	printf("sizeofentry: %d\n",sizeof(entry));
	ioctl(fd, FIFO_QUEUE, &entry);
	
	ioctl(fd, FIFO_FLUSH);
	
	
	sleep(5);

	ioctl(fd, VMODE, GRAPHICS_OFF);
	
	result = U_READ_REG(Device_RAM);
   	printf("Ram size in MB is: %d\n", result);

	close(fd);
	return 0;
}
