#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/msi.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>  
#include <linux/delay.h>  
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/init.h>  
#include <linux/mm.h>  
#include <linux/fs.h>  
#include <linux/types.h>  
#include <linux/delay.h>  
#include <linux/moduleparam.h>  
#include <linux/slab.h>  
#include <linux/errno.h>  
#include <linux/ioctl.h>  
#include <linux/cdev.h>  
#include <linux/string.h>  
#include <linux/list.h>  
#include <linux/pci.h>  
#include <linux/gpio.h>

#include "reg.h"

#define FIFO_ENTRIES 1024
#define GRAPHICS_ON 1
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Chaoran Huang");

int fd=0;
struct cdev kyouko3_cdev;

struct fifo_entry{
	u32 command;
	u32 value;
};

struct fifo
{
	u64 p_base;
	struct fifo_entry *k_base;
	u32 head;
	u32 tail_cache;
};


struct kyouko3_data{
	unsigned int * p_control_base;
	unsigned int * p_card_ram_base;
	struct pci_dev * pdev;
	unsigned int * k_control_base;
	unsigned int * k_card_ram_base;
//	void __iomem * ioaddr;
	struct fifo fifo;
	unsigned int graphics_on;
} kyouko3;

struct pci_device_id kyouko3_dev_ids[]=
{
	{PCI_DEVICE(0x1234,0x1113)},
	{0}
};

int kyouko3_probe(struct pci_dev * pci_dev, const struct pci_device_id * pci_id){
	kyouko3.p_control_base=pci_resource_start(pci_dev,1);
	kyouko3.p_card_ram_base=pci_resource_start(pci_dev,2);
	kyouko3.pdev=pci_dev->irq;
	pci_enable_device(pci_dev);
	pci_set_master(pci_dev);
};

int kyouko3_remove(struct pci_dev * pci_dev){
	pci_disable_device(pci_dev);
}

unsigned int K_READ_REG(unsigned int reg)
{
        unsigned int value;
        //delay();
        //rmb();
        value = *(kyouko3.k_control_base+(reg>>2));
        return (value);
}

void K_WRITE_REG(unsigned int reg, unsigned int value){
	//delay();
	*(kyouko3.k_control_base+(reg>>2))=value;
}

void pause(void){
	while (K_READ_REG(FifoTail)!=0) 
		schedule();//to some emergen guy then return;
	//return (1);
}

int init_fifo(void){
	kyouko3.fifo.k_base= pci_alloc_consistent(
		kyouko3.pdev,
		8192u,
		&kyouko3.fifo.p_base
		);
	K_WRITE_REG(FifoStart,kyouko3.fifo.p_base);
	K_WRITE_REG(FifoEnd,kyouko3.fifo.p_base+8192);
	kyouko3.fifo.head=0;
	kyouko3.fifo.tail_cache=0;
	pause();
}

struct pci_driver kyouko3_pci_drv=
{
        .name="Kyouko3",
        .id_table=kyouko3_dev_ids,
        .probe=kyouko3_probe,
        .remove=kyouko3_remove
};


void FIFO_WRITE(unsigned int reg, unsigned int value)
{
	kyouko3.fifo.k_base[kyouko3.fifo.head].command= reg;
	kyouko3.fifo.k_base[kyouko3.fifo.head].value=value;
	kyouko3.fifo.head++;
	if(kyouko3.fifo.head >= FIFO_ENTRIES)
	{
		kyouko3.fifo.head=0;
	}
}


int kyouko3_open(struct inode *inode, struct file *fp){
	unsigned int ramsize;
	kyouko3.k_control_base=ioremap(kyouko3.p_control_base,65536);
	ramsize = K_READ_REG(Device_RAM);
	ramsize *=(1024*1024);
	kyouko3.k_card_ram_base=ioremap(kyouko3.p_card_ram_base,ramsize);
	init_fifo();
	printk(KERN_ALERT "Opened Kyouko3...");
	return 0;
}

int kyouko3_release(struct inode *inode, struct file *fp){

	iounmap(kyouko3.k_control_base);
	iounmap(kyouko3.k_card_ram_base);
	pci_free_consistent(
		kyouko3.pdev,
		8192u,
		kyouko3.fifo.k_base,
		kyouko3.fifo.p_base
		);
	printk(KERN_ALERT "BUUH BYE...");
	return 0;
}
int kyouko3_mmap(struct file *flip, struct vm_area_struct * vma){
	int ret;
	ret = remap_pfn_range(vma, vma->vm_start,(unsigned int )(kyouko3.p_control_base)>>PAGE_SHIFT, (unsigned long)(vma->vm_end-vma->vm_start), vma->vm_page_prot);
	return ret;
}

kyouko3_ioctl(struct file *fp, unsigned int cmd, unsigned int arg)
{
	switch (cmd){
		case VMODE:{
			if (((int)(arg))==GRAPHICS_ON){
				FIFO_WRITE(Frame_Objects+_FColumns,1024);
				FIFO_WRITE(Frame_Objects+_FRows,768);
				FIFO_WRITE(Frame_Objects+_FRowPitch,1024*4);
				FIFO_WRITE(Frame_Objects+_FFormat,0xf888);
				FIFO_WRITE(Frame_Objects+_FAddress,0);
				//set acceleration bitmask to 0x40000000
				FIFO_WRITE(Acceleration,0x40000000);
				//set DAC
				FIFO_WRITE(DAC_Objects+_DWidth,1024);
				FIFO_WRITE(DAC_Objects+_DHeight,768);
				FIFO_WRITE(DAC_Objects+_DVirtX,0);
				FIFO_WRITE(DAC_Objects+_DVirtY,0);
				FIFO_WRITE(DAC_Objects+_DFrame,0);
				//set ModeSet
				FIFO_WRITE(ModeSet,1);			
				msleep(10);
				float *RGBA=(float *)arg;
				unsigned int uintR=*(unsigned int *) &RGBA[0];
				unsigned int uintG=*(unsigned int *) &RGBA[1];
				unsigned int uintB=*(unsigned int *) &RGBA[2];
				unsigned int uintA=*(unsigned int *) &RGBA[3];
				FIFO_WRITE(Clear_Color,uintR);
				FIFO_WRITE(Clear_Color+4,uintG);
				FIFO_WRITE(Clear_Color+8,uintB);
				FIFO_WRITE(Clear_Color+12,uintA);
				
				FIFO_WRITE(Clear_Buffer,0x03);
				FIFO_WRITE(Flush,0x0);
				
				kyouko3.graphics_on=1;
			}
			else{//off
				FIFO_WRITE(Acceleration,0);
				FIFO_WRITE(ModeSet,0);
				kyouko3.graphics_on=0;
			}			
			break;
		}
		case FIFO_QUEUE:{
			unsigned int ret;
			struct fifo_entry entry;
			ret=copy_from_user(
				&entry, 
				(struct fifo_entry *) arg,
				sizeof(struct fifo_entry)
				);
			FIFO_WRITE(entry.command, entry.value);
			break;
		}
		case FIFO_FLUSH:{
			K_WRITE_REG(FifoHead,kyouko3.fifo.head);
			while(kyouko3.fifo.tail_cache != kyouko3.fifo.head){
				kyouko3.fifo.tail_cache= K_READ_REG(FifoTail);
				schedule();
			}
			break;
		}
	}
}


struct file_operations kyouko3_fops = {
	.open= 		kyouko3_open,
	.release=	kyouko3_release,
	.mmap=		kyouko3_mmap,
	.owner=		THIS_MODULE
};

int my_init_function(void)
{
	cdev_init(&kyouko3_cdev, &kyouko3_fops);
	cdev_add(&kyouko3_cdev, MKDEV(500,127),1);
	pci_register_driver(&kyouko3_pci_drv);
	printk(KERN_ALERT "Initializing...");
	return 0;
}
void my_exit_function(void)
{	
	cdev_del(&kyouko3_cdev);
	pci_unregister_driver(&kyouko3_pci_drv);
	printk(KERN_ALERT "Exiting...");
}

module_init(my_init_function);
module_exit(my_exit_function);
