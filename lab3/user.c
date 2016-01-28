#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#define VMODE                   _IOW (0xcc,0,unsigned long)
#define BIND_DMA                _IOW (0xcc,1,unsigned long)
#define UNBIND_DMA              _IOW (0xcc,5,unsigned long)
#define START_DMA               _IOWR(0xcc,2,unsigned long)
#define FIFO_QUEUE              _IOWR(0xcc,3,unsigned long)
#define FIFO_FLUSH              _IO  (0xcc,4)


struct u_kyouko_device{
   unsigned int *u_control_base;
} kyouko3;

#define KYOUKO_CONTROL_SIZE (65535)
#define Device_RAM (0x0020)

unsigned int U_READ_REG(unsigned int rgister){
   return (*(kyouko3.u_control_base+(rgister>>2)));
}

int main()
{
   int fd;
   int result;

   fd = open("/dev/kyouko3", O_RDWR);

   ioctl(fd, VMODE, 1);
   
   kyouko3.u_control_base = (unsigned int *)mmap(0, KYOUKO_CONTROL_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
   result = U_READ_REG(Device_RAM);
   printf("Ram size in MB is: %d\n", result);
   close(fd);
   return 0;
}

