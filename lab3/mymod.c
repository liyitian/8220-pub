#include <linux/init.h>
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/mm.h>  
#include <linux/fs.h>  
#include <linux/ioctl.h>  
#include <linux/cdev.h>  
#include <linux/pci.h>

MODULE_LICENSE("Proprietarty");
MODULE_AUTHOR("Chaoran Huang");

struct kyouko3_data{
    unsigned int* p_control_base;
    unsigned int* p_card_ram_base;
    unsigned int* k_control_base;
    unsigned int* k_card_ram_base;
    struct pci_dev *pdev;
} kyouko3;


unsigned int K_READ_REG(unsigned int reg){
    unsigned int  value;
    //delay();
    //rmb();
    value = *(kyouko3.k_control_base+(reg>>2));
    return (value);
}


int kyouko3_open(struct inode *inode, struct file *fp){
    printk(KERN_ALERT "kyouko3_Open device");
    kyouko3.k_control_base = ioremap(kyouko3.p_control_base, 65536);
    unsigned int size = K_READ_REG(0x0020);
    kyouko3.k_card_ram_base = ioremap(kyouko3.p_card_ram_base, size);
    return 0;
}


int kyouko3_release(struct inode *inode, struct file *fp){
    iounmap(kyouko3.k_control_base);
    iounmap(kyouko3.k_card_ram_base);
    printk(KERN_ALERT "BUUH BYE");
    //close(fd);
    return 0;
}


int kyouko3_mmap(struct file *filp, struct vm_area_struct *vma){
    int ret;
    ret = remap_pfn_range(vma, vma->vm_start, (unsigned int)(kyouko3.p_control_base)>>PAGE_SHIFT, (unsigned long)(vma->vm_end-vma->vm_start), vma->vm_page_prot);
    return ret;
                                                              
}

long kyouko3_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
   long ret;

   

   return 0;
}


struct file_operations kyouko3_fops ={
    .open = kyouko3_open,
    .release = kyouko3_release,
    .unlocked_ioctl = kyouko3_ioctl,
    .mmap = kyouko3_mmap,
    .owner = THIS_MODULE
};


struct cdev kyouko3_cdev;

struct pci_device_id kyouko3_dev_ids[]={
    {PCI_DEVICE(0x1234, 0x1113)},
    {0},
};


int kyouko3_probe(struct pci_dev * pdev, const struct pci_dev_id * pciId){
   kyouko3.p_control_base = pci_resource_start(pdev, 1);
    kyouko3.p_card_ram_base = pci_resource_start(pdev, 2);
    pci_enable_device(pdev);
    pci_set_master(pdev);
    kyouko3.pdev = pdev;
}

void kyouko3_remove(struct pci_dev* pdev){
    pci_disable_device(pdev);
}


struct pci_driver kyouko3_pci_drv = {
    .name = "pci_driver",
    .id_table = kyouko3_dev_ids,
    .probe = kyouko3_probe,
    .remove = kyouko3_remove
};


static int my_init_func(void)
{
    cdev_init(&kyouko3_cdev, &kyouko3_fops);
    cdev_add(&kyouko3_cdev, MKDEV(500, 127), 1);
    printk("init_module");
    pci_register_driver(&kyouko3_pci_drv);
    return 0;
}

static void my_exit_func(void)
{
    pci_unregister_driver(&kyouko3_pci_drv);
    cdev_del(&kyouko3_cdev);
    printk("exit_module");

}

module_init(my_init_func);
module_exit(my_exit_func);

