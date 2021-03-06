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

int main(int argc, const char* argv[])
{

	if(argc<2)
	{
		printf("dma or fifo\n");
		exit(1);
	}

	if(strcmp(argv[1],"fifo") == 0){
		int result;
		unsigned int i;
		fd = open("/dev/kyouko3", O_RDWR);
		kyouko3.u_control_base = mmap(0, KYOUKO_CONTROL_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
		kyouko3.u_framebuffer_base = mmap(0, 1024*768*4, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x80000000);
		ioctl(fd, VMODE, GRAPHICS_ON);
		//draw a line
		for (i = 200*1024; i < 201*1024; ++i)
		{
			U_WRITE_FB(i, 0xff0000);
		}
		printf("address: %x\n", &entry);
		printf("user_queue: %x , %d\n", entry.command, entry.value);
		printf("sizeofentry: %d\n",sizeof(entry));
	
		ioctlQueue(Flush, 0x0);
		sleep(1);
		
		ioctlQueue(Command_Primitive, 1);
	
		float p1[4]={-0.5,-0.5,0.0,1.0};
		float c1[4]={1.0, 0.0, 0.0, 1.0};
		draw_point(p1, c1);
		ioctlQueue(Vertex_Emit, 0x0);

		float p2[4]={0.5,0.0,0.0,1.0};
		float c2[4]={1.0, 1.0, 0.0, 1.0};
		draw_point(p2, c2);
		ioctlQueue(Vertex_Emit, 0x0);
	
		float p3[4]={0.125,0.5,0.0,1.0};
		float c3[4]={1.0, 0.0, 1.0, 1.0};
		draw_point(p3, c3);
		ioctlQueue(Vertex_Emit, 0x0);
		ioctlQueue(Command_Primitive, 0x0);
		ioctlQueue(Flush, 0x0);
		sleep(2);
		ioctl(fd, FIFO_FLUSH);
		sleep(2);
		ioctl(fd, VMODE, GRAPHICS_OFF);
		//result = U_READ_REG(Device_RAM);
		//printf("Ram size in MB is: %d\n", result);
		close(fd);
		return 0;
	}




	if(strcmp(argv[1],"dma") == 0){
		int result;
		unsigned int i;
		fd = open("/dev/kyouko3", O_RDWR);
		
		kyouko3.u_control_base = mmap(0, KYOUKO_CONTROL_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
		kyouko3.u_framebuffer_base = mmap(0, 1024*768*4, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x80000000);
		ioctl(fd, VMODE, GRAPHICS_ON);
		ioctl(fd, BIND_DMA, dmaHeadBuffs);
	
		srand((unsigned int)time(NULL));
		for(i=0; i<NUM_BUFS; ++i){
			printf("dma %d u_address %x\n", i, dmaHeadBuffs[i].u_dma_bufferAddress);
			dmaHeadBuffs[i].dmaHdr.address = 0x1045; //0b 1 0000 0100 0101
			dmaHeadBuffs[i].dmaHdr.count = 3 * TRIANGLE_NUM;
			dmaHeadBuffs[i].dmaHdr.opcode = 0x14;
			dmaHeadBuffs[i].u_dma_bufferAddress[0] = *((unsigned int *)&dmaHeadBuffs[i].dmaHdr);
			printf("dmaHdr: %x\n",*(unsigned int*)&dmaHeadBuffs[i].dmaHdr);	
			//printf("dmaHdrInaddr: %x\n", dmaHeadBuffs[i].u_dma_bufferAddress[0]);
			int j=0;
			float x1,x2,x3,y1,y2,y3;
			for(j=0; j<TRIANGLE_NUM; ++j){
				x1  = rand()*1.0/RAND_MAX*2-1;
				y1 = rand()*1.0/RAND_MAX*2-1;
				x2 = rand()*1.0/RAND_MAX*2-1;
				y2 = rand()*1.0/RAND_MAX*2-1;
				x3 = rand()*1.0/RAND_MAX*2-1;
				y3 = rand()*1.0/RAND_MAX*2-1;
			
				float r = rand()*1.0/RAND_MAX; 
				float g = rand()*1.0/RAND_MAX; 
				float b = rand()*1.0/RAND_MAX; 
				float p0[4]={x1,y1,0.0,1.0};
				float c0[4]={r, g, b, 1.0};
				U_WRITE_DMABufferPoint(i ,3*j+0, p0, c0);
				float p1[4]={x2,y2,0.0,1.0};
				float c1[4]={r, g, b, 1.0};
				U_WRITE_DMABufferPoint(i, 3*j+1, p1, c1);
				float p2[4]={x3,y3,0.0,1.0};
				float c2[4]={r, g, b, 1.0};
				U_WRITE_DMABufferPoint(i, 3*j+2, p2, c2);
			}
		
			ioctlQueue(Flush, 0x0);
			ioctl(fd, START_DMA, 18*TRIANGLE_NUM+1);
			sleep(1);
			ioctlQueue(Flush, 0x0);
		}

		ioctlQueue(Flush, 0x0);
		sleep(1);
		ioctl(fd, UNBIND_DMA);

		sleep(1);

		ioctl(fd, VMODE, GRAPHICS_OFF);

		close(fd);
		return 0;
	}
}
