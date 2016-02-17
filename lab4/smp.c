#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#include <linux/ioctl.h>
#include <time.h>
#include <stdlib.h>
#include "reg.h"

#define TRIANGLE_NUM 100
int fd;
struct u_kyouko_device{
unsigned int *u_control_base;
unsigned int *u_framebuffer_base;
} kyouko3;

dmaInfo dmaHeadBuffs[NUM_BUFS];

#define KYOUKO_CONTROL_SIZE (65535)

struct f_entry{
   unsigned int command;
   unsigned int value;
};

struct f_entry entry;	

unsigned int U_READ_REG(unsigned int rgister){

	   return (*(kyouko3.u_control_base+(rgister>>2)));
}


void U_WRITE_FB(unsigned int pixel, unsigned int value){
	kyouko3.u_framebuffer_base[pixel] = value;
}



unsigned int float_to_uint(float v)
{
	return *(unsigned int *) &v; 
}

void ioctlQueue(unsigned int reg, unsigned int value)
{
	entry.command = reg;
	entry.value = value; 
	ioctl(fd, FIFO_QUEUE, &entry);
}

void draw_point(float* xyzw, float* rgba){


	ioctlQueue(Vertex_Coordinate, float_to_uint(xyzw[0]));
	ioctlQueue(Vertex_Color, float_to_uint(rgba[0]));
	
	ioctlQueue(Vertex_Coordinate+4, float_to_uint(xyzw[1]));
	ioctlQueue(Vertex_Color+4, float_to_uint(rgba[1]));
	
	ioctlQueue(Vertex_Coordinate+8, float_to_uint(xyzw[2]));
	ioctlQueue(Vertex_Color+8, float_to_uint(rgba[2]));
	
	ioctlQueue(Vertex_Coordinate+12, float_to_uint(xyzw[3]));
	ioctlQueue(Vertex_Color+12, float_to_uint(rgba[3]));

}

void U_WRITE_DMABufferPoint(unsigned int index,  unsigned int i, float* xyz, float *rgb){
	//XYZRGB 
	dmaHeadBuffs[index].u_dma_bufferAddress[i*6+1] = float_to_uint(rgb[0]);
	dmaHeadBuffs[index].u_dma_bufferAddress[i*6+2] = float_to_uint(rgb[1]);
	dmaHeadBuffs[index].u_dma_bufferAddress[i*6+3] = float_to_uint(rgb[2]);
	dmaHeadBuffs[index].u_dma_bufferAddress[i*6+4] = float_to_uint(xyz[0]);
	dmaHeadBuffs[index].u_dma_bufferAddress[i*6+5] = float_to_uint(xyz[1]);
	dmaHeadBuffs[index].u_dma_bufferAddress[i*6+6] = float_to_uint(xyz[2]);

}

int main()
{
	int pid;
	fd = open("/dev/kyouko3", O_RDWR);
	kyouko3.u_control_base = mmap(0, KYOUKO_CONTROL_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	kyouko3.u_framebuffer_base = mmap(0, 1024*768*4, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x80000000);
	ioctl(fd, VMODE, GRAPHICS_ON);
	int flag = 0;
	switch(pid=fork()){
		case 0:{
				unsigned int i;
				ioctl(fd, BIND_DMA, dmaHeadBuffs);
				for(i=0; i<NUM_BUFS; ++i){
					printf("dma %d u_address %x\n", i, dmaHeadBuffs[i].u_dma_bufferAddress);
					dmaHeadBuffs[i].dmaHdr.address = 0x1045; //0b 1 0000 0100 0101
					dmaHeadBuffs[i].dmaHdr.count = 3 * TRIANGLE_NUM;
					dmaHeadBuffs[i].dmaHdr.opcode = 0x14;
					dmaHeadBuffs[i].u_dma_bufferAddress[0] = *((unsigned int *)&dmaHeadBuffs[i].dmaHdr);
		 			//printf("dmaHdr: %x\n",*(unsigned int*)&dmaHeadBuffs[i].dmaHdr);
				}
				++flag;
				break;
		}
		default:{
					unsigned int i;
					ioctl(fd, BIND_DMA, dmaHeadBuffs);
					for(i=0; i<NUM_BUFS; ++i){
						printf("dma %d u_address %x\n", i, dmaHeadBuffs[i].u_dma_bufferAddress);
						dmaHeadBuffs[i].dmaHdr.address = 0x1045; //0b 1 0000 0100 0101
						dmaHeadBuffs[i].dmaHdr.count = 3 * TRIANGLE_NUM;
						dmaHeadBuffs[i].dmaHdr.opcode = 0x14;
						dmaHeadBuffs[i].u_dma_bufferAddress[0] = *((unsigned int *)&dmaHeadBuffs[i].dmaHdr);
						//printf("dmaHdr: %x\n",*(unsigned int*)&dmaHeadBuffs[i].dmaHdr);
					}
				}		
				++flag;
		 		break;
	}
	while(flag != 2);
	ioctl(fd, UNBIND_DMA);
	sleep(4);

	ioctl(fd, VMODE, GRAPHICS_OFF);
	

	close(fd);
	return 0;
}

