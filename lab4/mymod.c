#include <linux/init.h>
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/mm.h>  
#include <linux/fs.h>  
#include <linux/ioctl.h>  
#include <linux/cdev.h>  
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/mman.h>
#include "reg.h"
#define NUM_BUFS 1

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Yaolobg Yu");
DECLARE_WAIT_QUEUE_HEAD(dma_snooze);

struct cdev kyouko3_cdev;

struct fifo_entry{
	u32 command;
	u32 value;
};

struct __fifo
{
	u64 p_base;
	struct fifo_entry *k_base;
	u32 head;
	u32 tail_cache;
};


struct __k_dmabuff
{
	u64 p_base;
	u64 k_base;
	DMABuffInfo buffInfo; 
};

struct kyouko3_data{
	struct file * fp;
	struct vm_area_struct * vma;
	unsigned int * p_control_base;
	unsigned int * p_card_ram_base;
	struct pci_dev * pdev;
	unsigned int * k_control_base;
	unsigned int * k_card_ram_base;
	struct __fifo fifo;
	struct __k_dmabuff dmabuffs[NUM_BUFS];
	unsigned int dma_fill;
	unsigned int dma_drain;
	unsigned int graphics_on;
} kyouko3;

unsigned int K_READ_REG(unsigned int reg)
{
        unsigned int value;
        udelay(1);
        //rmb();
        value = *(kyouko3.k_control_base+(reg>>2));
        return (value);
}
void K_WRITE_REG(unsigned int reg, unsigned int value){
	udelay(1);
	*(kyouko3.k_control_base+(reg>>2))=value;
}

void FIFO_WRITE(unsigned int reg, unsigned int value)
{
	kyouko3.fifo.k_base[kyouko3.fifo.head].command= reg;
	kyouko3.fifo.k_base[kyouko3.fifo.head].value=value;
	kyouko3.fifo.head++;
	//printk(KERN_ALERT "head: %d\n", kyouko3.fifo.head);
	
	if(kyouko3.fifo.head >= FIFO_ENTRIES)
	{
		kyouko3.fifo.head=0;
	}
}



struct pci_device_id kyouko3_dev_ids[]=
{
	{PCI_DEVICE(0x1234,0x1113)},
	{0}
};

int kyouko3_probe(struct pci_dev * pdev, const struct pci_device_id * pci_id){
	kyouko3.p_control_base=pci_resource_start(pdev,1);
	kyouko3.p_card_ram_base=pci_resource_start(pdev,2);
	
	pci_enable_device(pdev);
	pci_set_master(pdev);

	kyouko3.pdev = pdev;
};

int kyouko3_remove(struct pci_dev * pdev){
	pci_disable_device(pdev);
}




void pause(void){
	while (K_READ_REG(FifoTail)!=0) 
		schedule();//to some emergen guy then return;
}

int init_fifo(void){
	kyouko3.fifo.head=0;
	kyouko3.fifo.tail_cache=0;
	kyouko3.fifo.k_base= pci_alloc_consistent(
		kyouko3.pdev,
		8192u,
		&kyouko3.fifo.p_base
		);
	K_WRITE_REG(FifoStart,kyouko3.fifo.p_base);
	K_WRITE_REG(FifoEnd,kyouko3.fifo.p_base+8192);
	pause();
	return 0;
}

struct pci_driver kyouko3_pci_drv=
{
        .name=				"pci_driver",
        .id_table=			kyouko3_dev_ids,
        .probe=				kyouko3_probe,
        .remove=			kyouko3_remove
};




int kyouko3_open(struct inode *inode, struct file *fp){
	unsigned int ramsize;
	kyouko3.fp = fp;
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
	printk(KERN_ALERT "File Close...");
	return 0;
}
int kyouko3_mmap(struct file *flip, struct vm_area_struct * vma){
	int ret;
	kyouko3.vma = vma;
	if((vma->vm_pgoff)<<PAGE_SHIFT == 0x0){
		printk(KERN_ALERT "I am in controlMMP\n");
		ret = remap_pfn_range(vma, vma->vm_start,(unsigned int )(kyouko3.p_control_base)>>PAGE_SHIFT, (unsigned long)(vma->vm_end-vma->vm_start), vma->vm_page_prot);
		printk(KERN_ALERT "controlMMPret: %d\n", ret);
	}
	else if((vma->vm_pgoff)<<PAGE_SHIFT == 0x80000000){
		printk(KERN_ALERT "I am in framBufferMMP\n");
		ret = remap_pfn_range(vma, vma->vm_start,(unsigned int )(kyouko3.p_card_ram_base)>>PAGE_SHIFT, (unsigned long)(vma->vm_end-vma->vm_start), vma->vm_page_prot);
		printk(KERN_ALERT "ramMMPret: %d\n", ret);
	}else{
		printk(KERN_ALERT "I am in DMABufferMMP\n");
		ret = remap_pfn_range(vma, vma->vm_start,(unsigned int )(kyouko3.dmabuffs[0].p_base)>>PAGE_SHIFT, (unsigned long)(vma->vm_end-vma->vm_start), vma->vm_page_prot);
		printk(KERN_ALERT "DMAMMPret: %d\n", ret);
	}
	return ret;
}

/*
unsigned int initiate_transfer(void)
{
	unsigned int flags;
	local_irq_save(flags);
	if(kyouko3.dma.fill == kyouko3.dma.drain){
		local_irq_restore(flags);
		fill = (fill + 1) % NUM_BUFS;
		FIFO_WRITE(BufferA_Address, kyouko3.dma.p_base);
		FIFO_WRITE(BufferA_Config, 124*1024 / NUM_BUFS);
		//kick_fifo .......
		K_WRITE_REG(FifoHead,kyouko3.fifo.head);
		return fill;
	}
	//????
	dmabuf[fill.stored_count] = count;
	fill = (fill + 1) % NUM_BUFS;
	wait_event_interruptable(dma_snooze, fill != drain);
	local_irq_restore(flags);
	return fill;

}
*/
long kyouko3_ioctl(struct file *fp, unsigned int cmd, unsigned int arg)
{
	switch (cmd){
		case VMODE:{
			if (((int)(arg))==GRAPHICS_ON){
				printk(KERN_ALERT "arg: %x\n", arg);
				K_WRITE_REG(Frame_Objects+_FColumns,1024);
				K_WRITE_REG(Frame_Objects+_FRows,768);
				K_WRITE_REG(Frame_Objects+_FRowPitch,1024*4);
				K_WRITE_REG(Frame_Objects+_FFormat,0xf888);
				K_WRITE_REG(Frame_Objects+_FAddress,0);
				//set acceleration bitmask to 0x40000000
				K_WRITE_REG(Acceleration,0x40000000);
				//set DAC
				K_WRITE_REG(DAC_Objects+_DWidth,1024);
				K_WRITE_REG(DAC_Objects+_DHeight,768);
				K_WRITE_REG(DAC_Objects+_DVirtX,0);
				K_WRITE_REG(DAC_Objects+_DVirtY,0);
				K_WRITE_REG(DAC_Objects+_DFrame,0);
				//set ModeSet
				K_WRITE_REG(ModeSet,1);			
				msleep(10);
				float one = 1.0;
				unsigned int one_as_int = *(unsigned int *) &one;
				FIFO_WRITE(Clear_Color,   0);
				FIFO_WRITE(Clear_Color+4, 0);
				FIFO_WRITE(Clear_Color+8, 0);
				FIFO_WRITE(Clear_Color+12,one_as_int);
				
				FIFO_WRITE(Clear_Buffer,0x03);
				FIFO_WRITE(Flush,0x0);
				
				kyouko3.graphics_on=1;
			}
			else{//off
				printk(KERN_ALERT "arg: %x\n", arg);
				K_WRITE_REG(Acceleration,0x80000000);
				K_WRITE_REG(ModeSet,0);
				kyouko3.graphics_on=0;
			}			
			break;
		}
		case FIFO_QUEUE:{
			printk(KERN_ALERT "in QUEUE\n");
			unsigned int ret;
			struct fifo_entry entry;
			ret=copy_from_user(&entry, (unsigned int*)arg, sizeof(struct fifo_entry));

			printk(KERN_ALERT "ret: %d", ret);
			printk(KERN_ALERT "queue_arg: %x\n", arg);
			printk(KERN_ALERT "KernelQueue: %x, %d\n", entry.command, entry.value);
			FIFO_WRITE(entry.command, entry.value);
			break;
		}
		case FIFO_FLUSH:{  
			printk(KERN_ALERT "in FLUSH\n");
			K_WRITE_REG(FifoHead,kyouko3.fifo.head);
			while(kyouko3.fifo.tail_cache != kyouko3.fifo.head){
				kyouko3.fifo.tail_cache= K_READ_REG(FifoTail);
				schedule();
			 	//printk(KERN_ALERT "Still in FLUSH\n");
			}
			break;
		}

		case BIND_DMA:{
			unsigned int i=0;
			for(i=0;i<NUM_BUFS;++i){
				kyouko3.dmabuffs[i].k_base= pci_alloc_consistent(
					kyouko3.pdev,
					124*1024,
					&kyouko3.dmabuffs[i].p_base
					);
				vm_mmap(kyouko3.fp, 0, 124*1024, PROT_READ|PROT_WRITE, MAP_SHARED, 0x90000000);
				((DMABuffInfo *)arg + i) ->u_address = kyouko3.vma->vm_start; 
			}
			break;
		}

		case START_DMA:{
				
			break;
		}

		case UNBIND_DMA:{

			break;
		}



	}
	return 0;
}


struct file_operations kyouko3_fops = {
	.open= 		kyouko3_open,
	.release=	kyouko3_release,
	.mmap=		kyouko3_mmap,
	.unlocked_ioctl=	kyouko3_ioctl,	
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

