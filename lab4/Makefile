obj-m += mymod.o

default:
	$(MAKE) -C /usr/src/linux M=$(PWD) modules
clean: 
	rm -rf *.ko *.o *.mod.c *.symvers *.order .* user
	
